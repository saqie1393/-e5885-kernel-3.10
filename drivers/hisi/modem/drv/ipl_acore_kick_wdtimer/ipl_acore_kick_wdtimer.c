


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <asm-generic/uaccess.h>
#include <mdrv_ipl_kick_wdtime.h>
#include "mbb_process_start.h"
#include <product_config.h>
#include <bsp_sram.h>

/*****************************************************************************
函 数 名 : kick_wdt_acore_wuc
功能描述 : minidump时继续喂狗
输入参数 : 无
返 回 值 : ret
调用函数 :
修改内容 : 新生成函数
*****************************************************************************/
void kick_wdt_acore_wuc()
{
    int val = 0;

    /* 获取当前wdt gpio的电平; 高则拉低,低则拉高 */
    val = gpio_get_value(IPL_KICK_WDT_GPIO);
    if (IPL_WDT_GPIO_VAL_HIGH == val)
    {
        gpio_set_value(IPL_KICK_WDT_GPIO, IPL_WDT_GPIO_VAL_LOW);
    }
    else if (IPL_WDT_GPIO_VAL_LOW == val)
    {
        gpio_set_value(IPL_KICK_WDT_GPIO, IPL_WDT_GPIO_VAL_HIGH);
    }

    return;
}

/*****************************************************************************
函 数 名 : stop_wdt_atDload
功能描述 : 一键升级时停止喂狗
输入参数 : 无
返 回 值 : ret
调用函数 : sys_newstat
修改内容 : 新生成函数
*****************************************************************************/
int stop_wdt_atDload(void)
{
    int ret = -1;
    char *exe_name = "flashwuc";
    char *exe_para = "--watchdog 0";

    /* 执行flashwuc命令，在一键升级时停止喂狗 */
    ret = drv_start_user_process(exe_name, exe_para, NEED_RESULT, WAIT_TIME_5S);
    printk("drv_start_user_process ret = %d  \n"  ,ret);

    return ret ;
}

/*****************************************************************************
 函 数 名  : ipl_fota_enter_freeze_processes_set
 功能描述  : A核进入睡眠标志，进入睡眠时执行
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
void ipl_fota_enter_freeze_processes_set(void)
{
    smem_huawei_ipl_fota_type *smem_data = NULL;

    /* 获取smem共享内存 */
    smem_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR"Share mem err %s() line = %d\n", __FUNCTION__, __LINE__);
        return;
    }

    /* 更新共享内存标志位，进入睡眠 */
    smem_data->smem_kernel_suspend_process = ENTER_FREEZE_PROCESSES;
    printk(KERN_INFO"%s()   enter sleep... \n", __FUNCTION__);

    /*
     * 进入睡眠时手动投票一次，保证watchdog gpio在唤醒后
     * 第一次进入定时器中断时能够正常翻转
     * 解决模块睡眠唤醒前后的两次watchdog gpio翻转间隔可
     * 能为两倍模块预设时间的问题；
     * 如果客户设置的WUC看门狗超时时间不大于2倍的模块
     * watchdog gpio翻转周期，可能因此导致模块被复位重启
     */
    smem_data->smem_vote_modem_watchdog = IPL_APPCORE_PROCESS_ALIVE;

    return;
}
EXPORT_SYMBOL(ipl_fota_enter_freeze_processes_set);

/*****************************************************************************
 函 数 名  : ipl_fota_exit_freeze_processes_set
 功能描述  : A核退出睡眠标志，退出睡眠时执行
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
void ipl_fota_exit_freeze_processes_set(void)
{
    smem_huawei_ipl_fota_type *smem_data = NULL;

    /* 获取共享内存 */
    smem_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR"Share mem err %s() line = %d\n", __FUNCTION__, __LINE__);
        return;
    }

    /* 更新共享内存标志位，退出睡眠 */
    smem_data->smem_kernel_suspend_process = EXIT_FREEZE_PROCESSES;
    printk(KERN_INFO"%s()   exit sleep... \n", __FUNCTION__);

    return;
}
EXPORT_SYMBOL(ipl_fota_exit_freeze_processes_set);

/*****************************************************************************
 函 数 名  : smem_vote_modem_watchdog_write
 功能描述  : 写ipl wdt 共享内存，供A5投票使用
 输入参数  : file:  文件句柄，对应proc节点；
             buffer:   应用态数据存放地址；
             count:要写的长度;
             ppos:  当前写的文件位置
 输出参数  : 共享内存的魔术字
 返 回 值  : 写入数值的长度
 调用函数  :
*****************************************************************************/
#ifdef CONFIG_PROC_FS
ssize_t smem_vote_modem_watchdog_write
(
    struct file *file
    , const char __user *buffer
    , size_t count
    , loff_t *ppos
)
{
    smem_huawei_ipl_fota_type *smem_data = NULL;
    unsigned char ipl_wdtimer_num[IPL_WDTIMER_FLAG_MAX_LEN + 1] = {0};
    unsigned int val = 0;

    /* 获取共享内存 */
    smem_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR"Share mem err %s() line = %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* 判断入参是否合法 */
    if ((count > (IPL_WDTIMER_FLAG_MAX_LEN + 1)) || (NULL == buffer))
    {
        printk(KERN_ERR"Invalid param buf(%p) size(%u)\n", buffer, count);
        return -1;
    }

    if (copy_from_user((void *)ipl_wdtimer_num, buffer, count))
    {
        return -1;
    }

    /* 判断传入参数是1或0 */
    if (('0' == ipl_wdtimer_num[0]) || ( '1' == ipl_wdtimer_num[0]))
    {
        val = ipl_wdtimer_num[0] - '0';
    }
    else
    {
        printk("input %d is not available\n", ipl_wdtimer_num[0]);
        return -1;
    }

    /* 将投票数据写入共享内存 */
    if (1 == val)
    {
        smem_data->smem_vote_modem_watchdog = IPL_APPCORE_PROCESS_ALIVE;
    }
    else
    {
        smem_data->smem_vote_modem_watchdog = IPL_APPCORE_PROCESS_NOT_ALIVE;
    }

    return count;
}

/*****************************************************************************
 数据结构名  : ipl_kick_wdtimer_fops
 功能描述    : 定义共享内存的读写接口
*****************************************************************************/
static struct file_operations ipl_kick_wdtimer_fops = {
    .write      = smem_vote_modem_watchdog_write,
};
#endif

/*****************************************************************************
 函 数 名  : create_smem_vote_modem_watchdog_file
 功能描述  : 创建proc文件，提供共享内存读写接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
static int create_smem_vote_modem_watchdog_file(void)
{
#ifdef CONFIG_PROC_FS
    struct proc_dir_entry *ent = NULL;

    /* 创建proc节点 */
    ent = proc_create(SMEM_VOTE_MODEM_WATCHDOG, S_IWUGO, NULL, &ipl_kick_wdtimer_fops);
    if (NULL == ent)
    {
        printk(KERN_ALERT"%s: create proc entry for wdt file failed\n", __FUNCTION__);
        return -1;
    }
#endif

    return 0;
}

/*****************************************************************************
 函 数 名  : remove_smem_vote_modem_watchdog_file
 功能描述  : 删除proc文件，卸载共享内存读写接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
static void remove_smem_vote_modem_watchdog_file(void)
{
#ifdef CONFIG_PROC_FS
    remove_proc_entry(SMEM_VOTE_MODEM_WATCHDOG, NULL);
#endif

    return;
}
/*****************************************************************************
 函 数 名  : ipl_kick_wdtimer_smem_probe
 功能描述  : 驱动初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
static int ipl_kick_wdtimer_smem_probe(void)
{
    return create_smem_vote_modem_watchdog_file();
}

/*****************************************************************************
 函 数 名  : ipl_kick_wdtimer_smem_remove
 功能描述  : 驱动卸载
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  : 无
*****************************************************************************/
static void ipl_kick_wdtimer_smem_remove(void)
{
    remove_smem_vote_modem_watchdog_file();
    return;
}

module_init(ipl_kick_wdtimer_smem_probe);
module_exit(ipl_kick_wdtimer_smem_remove);

MODULE_DESCRIPTION("HUAWEI ipl_kick_wdtimer_smem Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("HUAIWEI: Ipl_kick_wdtimer_smem Driver");
