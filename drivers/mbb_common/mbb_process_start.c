/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 /* 注意:
  * 应用态读取节点的文件名为:process_start_app.c，若要增加可执行文件的调用，请
  * 同步在该文件里的表格里添加，并向相关责任人申请。
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include "linux/wakelock.h"
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <osl_sem.h>
#include <linux/kthread.h>
#include "mbb_process_start.h"

#ifdef __cplusplus
extern "C" {
#endif

#define procs_err(fmt, ...)  (printk(KERN_ERR "%s: "fmt, __func__, ##__VA_ARGS__))

#define NAME_MAX_LEN   (64)
#define PARA_MAX_LEN   (128)
#define BUFF_MAX_LEN   (256)

#define PROC_NAME      "process_start"
#define CHAR_CR_LR     "\r\n"

typedef enum
{
    PROCESS_STATE_WAIT    = 0,  /*该进程正在等待被执行*/
    PROCESS_STATE_EXECULT = 1,  /*该进程正在执行*/
    PROCESS_STATE_DONE    = 2,  /*该进程执行结束，可以获取结果*/
}process_state_e;

typedef struct process_node
{
    char name[NAME_MAX_LEN];    /*进程名称*/
    char para[PARA_MAX_LEN];    /*进程需要的参数，没有可为空*/
    int           result;       /*保存应用返回值*/
    unsigned int  need_result;  /*是否需要返回值*/
    unsigned int  state;        /*当前状态*/
    struct list_head  stlist;   
}process_node_str;

typedef struct process_ctrl
{
    unsigned int inited;         /*是否初始化标志*/
    unsigned int count;          /*启动进程计数*/
    spinlock_t   lock;
    osl_sem_id   rw_sem;
    wait_queue_head_t pollwait;  /*poll函数的休眠等待队列*/
    wait_queue_head_t listwait; /*需要执行结果的等待队列*/
    struct list_head  stlist;    /*list 头*/
}process_ctl_str;

static process_ctl_str  g_process_info;  


/*调试接口，用于查看当前链表里的进程及其状态，供手动调用*/
void show_process_list_info(void)
{
    struct list_head* me  = NULL;
    process_node_str* cur = NULL;

    list_for_each(me,&g_process_info.stlist)
    {
        cur = list_entry(me, process_node_str, stlist);
        if(NULL != cur)
        {
            procs_err("name:%s, para:%s, state:%d, need_result:%d, result:%d\n",
                cur->name, cur->para, cur->state, cur->need_result, cur->result);
        }
    }
    procs_err("==========list info show end=============\n");
    return;
}
     
/*****************************************************************************
 函 数 名  : process_read
 功能描述  : 应用读节点操作接口
 输入参数  : file:  文件句柄，对应proc节点；
             buf:   应用态数据存放地址；
             count:要写的长度;
             ppos:  当前写的文件位置
 输出参数  : 无
 返 回 值  : 读取长度
 说    明  : 
*****************************************************************************/
static ssize_t process_read(struct file *file, __user char *buffer, size_t count,loff_t *ppos)
{
    int ret = 0;
    unsigned int  len = 0;
    unsigned long cpy_len = 0;
    unsigned long flag = 0;
    char buf_temp[BUFF_MAX_LEN] = {0};
    struct list_head* me  = NULL;
    process_node_str* cur = NULL;
    
      /*参数检查*/
    if(NULL == file || NULL == buffer || NULL == ppos)
    {
        procs_err("para is null\n");
        return -1;
    }

    if(0 == g_process_info.count)
    {
        procs_err("there is no process to read.\n");
        return 0;
    }

    osl_sem_down(&g_process_info.rw_sem);
    
    (void)memset(buf_temp, '\0', BUFF_MAX_LEN);
    
    /*找到需要启动的进程链表节点*/
    list_for_each(me,&g_process_info.stlist)
    {
        cur = list_entry(me, process_node_str, stlist);
        if(NULL != cur && PROCESS_STATE_WAIT == cur->state)
        {
            break;
        }
    }

    if(&g_process_info.stlist == me)
    {
        procs_err("there is nothing in node to read\n");
        osl_sem_up(&g_process_info.rw_sem);
        return -1;
    }
    
    /*按格式组装数据*/
    len  = snprintf(buf_temp, BUFF_MAX_LEN, "%s%s%d%s%s", 
                        cur->name, CHAR_CR_LR, 
                        cur->need_result, CHAR_CR_LR,
                        cur->para);
    
    /*拷贝数据给应用*/
    cpy_len = count > len ? len : count;
    ret = copy_to_user(buffer, buf_temp, cpy_len);
    if(0 != ret)
    {
        procs_err("msg copy to user failed!\n");
        osl_sem_up(&g_process_info.rw_sem);
        return -1;
    }
    else
    {
        *ppos += cpy_len;
    }
    
    g_process_info.count--;

    if(1 != cur->need_result)
    {
        spin_lock_irqsave(&g_process_info.lock, flag);
        list_del(&cur->stlist);
        kfree(cur);
        cur = NULL;
        spin_unlock_irqrestore(&g_process_info.lock, flag);
    }
    else
    {
        cur->state = PROCESS_STATE_EXECULT;
    }

    osl_sem_up(&g_process_info.rw_sem);
    return cpy_len;
    
}

/*****************************************************************************
 函 数 名  : process_write
 功能描述  : 应用写节点操作接口
 输入参数  : file:  文件句柄，对应proc节点；
             buf:   应用态数据存放地址；
             count:要写的长度;
             ppos:  当前写的文件位置
 输出参数  : 无
 返 回 值  : 写入长度
 说    明  : 
*****************************************************************************/
static ssize_t process_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int result = 0;
    unsigned long cpy_len = 0;
    char name[BUFF_MAX_LEN] = {0};
    char buf_temp[BUFF_MAX_LEN] = {0};
    struct list_head* me  = NULL;
    process_node_str* cur = NULL;
    
     /*参数检查*/
    if(NULL == file || NULL == buf || NULL == ppos)
    {
        procs_err("para is null\n");
        return -1;
    }
    
    osl_sem_down(&g_process_info.rw_sem);
    
    (void)memset(buf_temp, 0x0, BUFF_MAX_LEN);

    /*从节点读取数据*/
    cpy_len = count > BUFF_MAX_LEN ? BUFF_MAX_LEN - 1 : count;
    ret = copy_from_user(buf_temp, buf, cpy_len);
    if(0 != ret)
    {
        procs_err("msg copy from user failed!\n");
        osl_sem_up(&g_process_info.rw_sem);
        return -1;
    }
    *ppos += cpy_len;

    /*将进程名和结果解析出来*/
    (void)sscanf(buf_temp, "%s\r\n%d", name, &result);

    /*遍历链表，找到对应的node*/
    list_for_each(me,&g_process_info.stlist)
    {
        cur = list_entry(me, process_node_str, stlist);
        if(NULL != cur && PROCESS_STATE_EXECULT == cur->state)
        {
            if(0 == strncmp(cur->name, name, strlen(cur->name)))
            {
                cur->result = result;
                cur->state = PROCESS_STATE_DONE;
                wake_up_interruptible(&g_process_info.listwait);
                break;
            }
        }
    }

    if(&g_process_info.stlist == me)
    {
        procs_err("there is no process matched,name:%s!\n",name);
    }
    
    osl_sem_up(&g_process_info.rw_sem);
    return cpy_len;
}

/*****************************************************************************
 函 数 名  : ant_poll
 功能描述  : 文件poll接口，当达到一定条件(有gpio插拔时)向应用通知事件
 输入参数  : file:文件句柄，对应proc节点；
             wait:没有事件发生时，挂载的等待队列表
 输出参数  : 无
 返 回 值  : 事件类型
 说    明  : 
*****************************************************************************/
static unsigned int process_poll(struct file *file, poll_table *wait)
{
    int ret = 0;
        /*参数检查*/
    if (NULL == file || NULL == wait)
    {
        procs_err("file or wait is null.");
        return  -1;
    }
   
    poll_wait(file, &g_process_info.pollwait, wait);

    if (0 != g_process_info.count)
    {
        ret = POLLIN | POLLRDNORM;
    }
    return ret;
}
/*****************************************************************************
 函 数 名  : drv_start_user_process
 功能描述  : 底层调用应用态进程通用接口
 输入参数  : name: 进程名；
             para: 进程的参数，没有的话为null
             need_result:  是否需要返回值
             wait_time:超时等待时间，单位ms
 输出参数  : 无
 返 回 值  : 0--进程启动成功或者执行结果为ok；
             other--失败；
 说    明  :如果执行的进程不会退出，一直执行，need_result参数请传入0，wait_time为0；
            执行的进程会退出，need_result参数请传入1，wait_time最大为5分钟，单位为ms；
            此时该接口会阻塞等待执行结果，直至wait_time超时时间到。
*****************************************************************************/
int  drv_start_user_process(char* name, char* para, unsigned int need_result, unsigned int wait_time)
{
    int  result   = -1;
    int  timeleft =  0;
    unsigned int  wtime = 0;
    unsigned long flag = 0;
    struct list_head* me  = NULL;
    process_node_str* cur = NULL;

    if (1 != g_process_info.inited)
    {
        procs_err("this module is not inited,pls call it later.\n");
        return -1;
    }

    if(NULL == name)
    {
        procs_err("name is null.\n");
        return -1;
    }

    /*check 进程是否已经在链表中等待启动或者正在被执行*/
    list_for_each(me,&g_process_info.stlist)
    {
        cur = list_entry(me, process_node_str, stlist);
        if(NULL != cur && 0 == strncmp(cur->name, name, strlen(cur->name)))
        {
            procs_err("the process:%s is exist,pls wait and retry\n",name);
            return -1;
        }
    }

    /*不在链表中，可以启动*/
    cur = (process_node_str *)kmalloc(sizeof(process_node_str), GFP_KERNEL);
    if(NULL == cur)
    {
        procs_err("malloc list node:%s failed\n",name);
        return -1;
    }
    (void)memset(cur, '\0', sizeof(process_node_str));

    /*保存参数到链表节点*/
    (void)strncpy(cur->name , name, NAME_MAX_LEN -1);
    if(NULL != para)
    {
        (void)strncpy(cur->para , para, PARA_MAX_LEN - 1);
    }
    cur->need_result = need_result;
    cur->state = PROCESS_STATE_WAIT;
    cur->result = -1;

    spin_lock_irqsave(&g_process_info.lock, flag);
    /*加入链表，唤醒poll*/
    list_add(&cur->stlist, &g_process_info.stlist);
    g_process_info.count++;
    wake_up_interruptible(&g_process_info.pollwait);
    
    spin_unlock_irqrestore(&g_process_info.lock, flag);

    if(NEED_RESULT == cur->need_result)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        wtime = wait_time > WAIT_TIME_MAX ? WAIT_TIME_MAX : wait_time;
        timeleft = wait_event_interruptible_timeout(g_process_info.listwait, 
                    PROCESS_STATE_DONE == cur->state, msecs_to_jiffies(wtime));

        if(0 == timeleft)
        {
            procs_err("timeout,the app run process:%s may be failed\n",name);
        }

        result = cur->result;
        procs_err("process:%s ,result is :%d\n",cur->name, result);
        
        spin_lock_irqsave(&g_process_info.lock, flag);
        list_del(&cur->stlist);
        kfree(cur);
        cur = NULL;
        spin_unlock_irqrestore(&g_process_info.lock, flag);

        return result;
    }

    return 0;

}
EXPORT_SYMBOL_GPL(drv_start_user_process);


static struct file_operations process_start_ops = {
    .read  = process_read,
    .write = process_write,
    .poll  = process_poll,
};

/*****************************************************************************
 函 数 名  : process_proc_init
 功能描述  : 创建相应的proc节点
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int process_proc_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;
   
    proc_entry = proc_create(PROC_NAME, 0600, NULL, &process_start_ops);
    if(NULL == proc_entry)
    {
        procs_err("can't create /proc/%s \n", PROC_NAME);
        return -EFAULT;
    }
    spin_lock_init(&g_process_info.lock);
    osl_sem_init(SEM_UP, &g_process_info.rw_sem);

    init_waitqueue_head(&g_process_info.pollwait);
    init_waitqueue_head(&g_process_info.listwait);
    
    INIT_LIST_HEAD(&g_process_info.stlist);
    g_process_info.count = 0;
    g_process_info.inited = 1;

    return 0;
}

rootfs_initcall(process_proc_init);
MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Mbb Common Driver");

#ifdef __cplusplus
}
#endif



