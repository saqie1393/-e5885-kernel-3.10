/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <product_config.h>
#include <osl_types.h>

#include <linux/sched.h>
#include <asm/thread_notify.h>
#include <linux/stacktrace.h>
#include <linux/irq.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>

#include <bsp_softtimer.h>
#include <bsp_adump.h>
#include "bsp_slice.h"
#include "bsp_nvim.h"

#include "adump_baseinfo.h"
#include "adump_area.h"
#include "adump_field.h"
#include "adump_sche_record.h"
#include "adump_queue.h"
#include "adump_debug.h"


struct adump_sche_ctrl_info_s g_adump_sche_ctrl;


/********************task tcb info start**********************/
void adump_save_all_tasktcb(void)
{
    struct task_struct  *pTid = NULL;
    struct thread_info  *pThread = NULL;
    dump_tasktcb_info_s *task_info = NULL;
    int ulTaskNum = 0;
    struct stack_trace trace;
    int i=0;
    unsigned long trace_entry[16];
    dump_tasktcb_info_s*  tasktcb_addr = (dump_tasktcb_info_s*)g_adump_sche_ctrl.tasktcb_addr;

    if(tasktcb_addr == NULL)
    {
        adump_err("tasktcb_addr is NULL\n");
        return;
    }
    for_each_process(pTid)
    {
        if(ulTaskNum >=  DUMP_LINUX_TASK_NUM_MAX)
        {
            return;
        }

        pThread = (struct thread_info*)pTid->stack;
        task_info = &tasktcb_addr[ulTaskNum];

        task_info->pid = (u32)PID_PPID_GET(pTid);
        task_info->entry = (u32)NULL;                        // linux暂不支持
        task_info->status = pTid->state;
        task_info->policy = pTid->policy;
        task_info->priority = pTid->prio;
        task_info->stack_base = (uintptr_t)((uintptr_t)pTid->stack + THREAD_SIZE);
        task_info->stack_end = (uintptr_t)end_of_stack(pTid);
        task_info->stack_high = 0;                          // linux暂不支持
        /* coverity[buffer_size_warning] */
        strncpy((char *)task_info->name, pTid->comm, 16);
        task_info->regs[0] = 0;                             // 前四个通用寄存器无意义
        task_info->regs[1] = 0;
        task_info->regs[2] = 0;
        task_info->regs[3] = 0;
        memcpy(&task_info->regs[4], &pThread->cpu_context, 12*sizeof(u32));
        task_info->offset = 0;
        /* coverity[secure_coding] */
        memset(&trace,0,sizeof(trace));
        /* coverity[secure_coding] */
        memset(trace_entry,0,sizeof(trace_entry));
        trace.max_entries = 16;
        trace.entries     = trace_entry;

        save_stack_trace_tsk(pTid, &trace);

        for(i=0;i<trace.nr_entries;i++){
            if((trace.entries[i] == ULONG_MAX)||(!core_kernel_text(trace.entries[i])))
                break;
            if((DUMP_KERNEL_TASK_INFO_STACK_SIZE - task_info->offset) < (strlen((void*)trace.entries[i])+12))
                break;
            task_info->offset += sprintf((char *)task_info->dump_stack+task_info->offset, "[%08lx]%pS\n", \
                (unsigned long)trace.entries[i], (void *)trace.entries[i]);
        }
        ulTaskNum++;
    }
    adump_err("ok !\n");
    return;
}


s32 adump_tasktcb_info_init(void)
{
    u32 i = 0;
    dump_tasktcb_info_s*  tasktcb_addr= NULL;

    tasktcb_addr = (dump_tasktcb_info_s*)bsp_adump_register_field(DUMP_KERNEL_ALLTASK_TCB, "ALLTASK_TCB", NULL,NULL,DUMP_KERNEL_ALLTASK_TCB_SIZE,0);
    if(!tasktcb_addr)
    {
        adump_err("alloc task tcb buffer fail\n");
        return ADUMP_ERR;
    }
    /* coverity[secure_coding] */
    memset((void*)tasktcb_addr, 0, DUMP_KERNEL_ALLTASK_TCB_SIZE);

    for(i = 0; i < DUMP_LINUX_TASK_NUM_MAX; i++)
    {
        tasktcb_addr[i].pid = 0xffffffff;
    }

    g_adump_sche_ctrl.tasktcb_addr = (void*)tasktcb_addr;
    adump_err("ok\n");
    return ADUMP_OK;
}

/*********************task tcb info end***********************/


/*********************task name save start********************/
static void adump_task_timer_handler(u32 param)
{
    adump_save_task_name();
    bsp_softtimer_add(&g_adump_sche_ctrl.taskname_timer);
}

s32 adump_start_task_name_save_timer(u32 time_out)
{
    s32 ret = 0;

    g_adump_sche_ctrl.taskname_timer.func = (softtimer_func)adump_task_timer_handler;
    g_adump_sche_ctrl.taskname_timer.para = 0;
    g_adump_sche_ctrl.taskname_timer.timeout = time_out;
    g_adump_sche_ctrl.taskname_timer.wake_type = SOFTTIMER_NOWAKE;

    ret =  bsp_softtimer_create(&g_adump_sche_ctrl.taskname_timer);
    if(ret)
    {
        adump_err("create dump timer err!\n");
        return ADUMP_ERR;
    }

    bsp_softtimer_add(&g_adump_sche_ctrl.taskname_timer);

    return ADUMP_OK;
}


void adump_stop_task_name_timer(void)
{
    if(g_adump_sche_ctrl.taskname_timer.init_flags != TIMER_INIT_FLAG)
    {
        adump_err("timer is null\n");
        return;
    }
    (void)bsp_softtimer_delete(&g_adump_sche_ctrl.taskname_timer);
    (void)bsp_softtimer_free(&g_adump_sche_ctrl.taskname_timer);
}

void adump_save_task_name(void)
{
    struct task_struct *pTid = NULL;
    int ulTaskNum = 0x00;
    u32 pid_ppid = 0;
    void* task_name_addr = 0;
    char idle_task_name[12] = {"swapper"};

    task_name_addr = g_adump_sche_ctrl.taskname_addr;
    if(NULL == task_name_addr )
    {
        adump_err("task name buffer is NULL\n");
        return;
    }
    /* 任务切换正在记录，直接返回 */
    if(g_adump_sche_ctrl.taskname_flag == true)
    {
        return;
    }
    /* 开始记录 */
    g_adump_sche_ctrl.taskname_flag = true;

    for_each_process(pTid)
    {
        if(ulTaskNum >=  DUMP_LINUX_TASK_NUM_MAX)
        {
            break;
        }

        pid_ppid = (u32)PID_PPID_GET(pTid);
        queue_loop_enter((queue_t *)task_name_addr, pid_ppid);
        queue_loop_enter((queue_t *)task_name_addr, *((int *)(pTid->comm)));
        queue_loop_enter((queue_t *)task_name_addr, *((int *)((pTid->comm)+4)));
        queue_loop_enter((queue_t *)task_name_addr, *((int *)((pTid->comm)+8)));

        ulTaskNum++;
    }

    queue_loop_enter((queue_t *)task_name_addr, 0);
    queue_loop_enter((queue_t *)task_name_addr, *((int *)(idle_task_name)));
    queue_loop_enter((queue_t *)task_name_addr, *((int *)(idle_task_name+4)));
    queue_loop_enter((queue_t *)task_name_addr, *((int *)(idle_task_name+8)));

    g_adump_sche_ctrl.taskname_flag = false;
}

/*task name 记录内核任务名，依赖timer的初始化，需要放到timer初始化之后*/
s32 adump_task_name_init(void)
{
    void* addr;
    addr = bsp_adump_register_field(DUMP_KERNEL_TASK_NAME,"TASK_NAME", NULL,NULL,DUMP_KERNEL_TASK_NAME_SIZE,0);
    if(addr == NULL)
    {
        adump_err("alloc task name buffer fail\n");
        return ADUMP_ERR;
    }
    queue_init((queue_t *)addr, (DUMP_KERNEL_TASK_NAME_SIZE-0x10) / 0x4);
    g_adump_sche_ctrl.taskname_addr = addr;

    adump_err("ok!\n");
    return ADUMP_OK;
}
s32 __init adump_task_name_timer_init(void)
{
    adump_start_task_name_save_timer(60000);
    return ADUMP_OK;
}
arch_initcall_sync(adump_task_name_timer_init);
/*******************task name save end************************/


/*******************task int switch start*********************/

void adump_stop_taskswitch_record(void)
{
    g_adump_sche_ctrl.taskSwitch = false;
}
EXPORT_SYMBOL(adump_stop_taskswitch_record);

void adump_start_taskswitch_record(void)
{
    g_adump_sche_ctrl.taskSwitch = true;
}
EXPORT_SYMBOL(adump_start_taskswitch_record);

void adump_stop_intswitch_record(void)
{
    g_adump_sche_ctrl.intSwitch = false;
}
EXPORT_SYMBOL(adump_stop_intswitch_record);

void adump_start_intswitch_record(void)
{
    g_adump_sche_ctrl.intSwitch = true;
}
EXPORT_SYMBOL(adump_start_intswitch_record);

void adump_int_switch_hook(u32 dir, u32 newVec)
{
    void* addr = NULL;
    unsigned long lock_flag = 0;

    addr = g_adump_sche_ctrl.scheRecord_addr;
    if((false == g_adump_sche_ctrl.intSwitch)||(NULL == addr))
        return;

    spin_lock_irqsave(&g_adump_sche_ctrl.taskswitch_spinlock, lock_flag);

    if (0 == dir)/*IN*/
    {
        queue_loop_enter((queue_t *)addr, (((u32)DUMP_INT_IN_FLAG<<16)|newVec));
    }
    else/*EXIT*/
    {
        queue_loop_enter((queue_t *)addr, (((u32)DUMP_INT_EXIT_FLAG<<16)|newVec));
    }
    queue_loop_enter((queue_t *)addr, bsp_get_slice_value());

    spin_unlock_irqrestore(&g_adump_sche_ctrl.taskswitch_spinlock, lock_flag);

    adump_fill_current_int(newVec);

    return;
}


void adump_task_switch_hook(void *old_tcb, void *new_tcb)
{
    u32 pid_ppid = 0;
    void* addr = NULL;
    unsigned long lock_flag;
    void* task_name_addr = NULL;

    addr = g_adump_sche_ctrl.scheRecord_addr;
    if((false == g_adump_sche_ctrl.taskSwitch)||(NULL == addr))
        return;

    pid_ppid = (u32)PID_PPID_GET(new_tcb);
    spin_lock_irqsave(&g_adump_sche_ctrl.taskswitch_spinlock, lock_flag);
    queue_loop_enter((queue_t *)addr, (u32)pid_ppid);
    queue_loop_enter((queue_t *)addr, bsp_get_slice_value());
    spin_unlock_irqrestore(&g_adump_sche_ctrl.taskswitch_spinlock, lock_flag);

    adump_fill_current_task((u32)new_tcb);

    /* 定时器超时，正在记录任务名，任务切换不做记录 */
    if(g_adump_sche_ctrl.taskname_flag == true)
    {
        return;
    }

    /* 开始记录 */
    g_adump_sche_ctrl.taskname_flag = true;

    /* 因为kthreadd派生出来的任务，第一次运行时，任务名都叫kthreadd，所以任务第二次进入时，才记录 */
    /* dump_magic字段，嵌入式修改内核 */
    if(((struct task_struct*)new_tcb)->dump_magic == (int)new_tcb)
    {
        g_adump_sche_ctrl.taskname_flag = false;
        return;
    }
    else if(((struct task_struct*)new_tcb)->dump_magic == (int)new_tcb + 1)
    {
        task_name_addr = g_adump_sche_ctrl.taskname_addr;
        if(NULL == task_name_addr )
        {
            g_adump_sche_ctrl.taskname_flag = false;
            return;
        }
        queue_loop_enter((queue_t *)task_name_addr, pid_ppid);
        queue_loop_enter((queue_t *)task_name_addr, *((u32 *)(((struct task_struct *)(new_tcb))->comm)));
        queue_loop_enter((queue_t *)task_name_addr, *((u32 *)((((struct task_struct *)new_tcb)->comm)+4)));
        queue_loop_enter((queue_t *)task_name_addr, *((u32 *)((((struct task_struct *)new_tcb)->comm)+8)));
        ((struct task_struct*)new_tcb)->dump_magic = (int)new_tcb;
    }
    else
    {
        ((struct task_struct*)new_tcb)->dump_magic = (int)new_tcb + 1;
    }

    g_adump_sche_ctrl.taskname_flag = false;
}


int adump_task_switch_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    struct thread_info *thread = data;

    if (action != THREAD_NOTIFY_SWITCH)
    {
        return NOTIFY_DONE;
    }

    adump_task_switch_hook(NULL, thread->task);

    return NOTIFY_OK;
}



/* dump task switch notifier */
static struct notifier_block adump_task_switch_notifier =
{
    .notifier_call = adump_task_switch_callback,
    .priority      = 0,
};

s32 adump_sche_switch_init(void)
{
    NV_DUMP_STRU            dump_cfg = {{0}};
    spin_lock_init(&g_adump_sche_ctrl.taskswitch_spinlock);
    if(bsp_nvm_read(NV_ID_DRV_DUMP, (u8*)&dump_cfg, sizeof(NV_DUMP_STRU)))
    {
        adump_err("read cfg nv fail register hook fail\n ");
    }

    g_adump_sche_ctrl.scheRecord_addr = bsp_adump_register_field(DUMP_KERNEL_TASK_SWITCH, "TASK_SWITCH", NULL,NULL,DUMP_KERNEL_TASK_SWITCH_SIZE,0);
    if(g_adump_sche_ctrl.scheRecord_addr == NULL)
    {
        adump_err("alloc task switch fail\n");
        return ADUMP_ERR;
    }
    queue_init((queue_t *)(g_adump_sche_ctrl.scheRecord_addr), (DUMP_KERNEL_TASK_SWITCH_SIZE - 0x10) / 0x4);

    /* register task switch notifier */
    if(dump_cfg.dump_cfg.Bits.taskSwitch)
    {
        adump_start_taskswitch_record();
        thread_register_notifier(&adump_task_switch_notifier);
    }

    if(dump_cfg.dump_cfg.Bits.intSwitch)
    {
        adump_start_intswitch_record();
        register_int_notifier(adump_int_switch_hook);
    }
    adump_err("ok!\n");
    return ADUMP_OK;
}

void adump_stop_sche_record(void)
{
    adump_stop_taskswitch_record();
    adump_stop_intswitch_record();
    return;
}

/*******************task int switch end***********************/

s32 adump_sche_record_init(void)
{
    /*all task name init*/
    (void)adump_task_name_init();

    /*int/task switch*/
    (void)adump_sche_switch_init();

    /*all task tcb*/
    (void)adump_tasktcb_info_init();

    return ADUMP_OK;
}

