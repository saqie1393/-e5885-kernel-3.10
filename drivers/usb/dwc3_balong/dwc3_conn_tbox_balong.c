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
 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/slab.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/interrupt.h>

#include <linux/usb/balong_usb_nv.h>
#include <linux/gpio.h>
#include "dwc3_conn_ctrl_balong.h"
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#include "mdrv_usb.h"

/* 定义TBOX USB 设备 */
#define DRIVER_NAME "tbox_conn_pdev"

extern void huawei_pm_set_pin_status(int pin_status );
extern void wakeup_in_state_report(unsigned char state);
extern unsigned char huawei_pm_pstandby_state_get(void);
extern void huawei_pm_handle_usb_resume_complete(void);
extern void bsp_usb_nv_init(void);
extern void huawei_pm_remote_shake_complete(void);
extern void syssc_usb_powerdown_hsp(int value);
extern void syssc_usb_powerdown_ssp(int value);

/* wakeup in gpio event 上报延时任务 */
static struct delayed_work wakeupin_gpio_report_work;

/******************************************************************************
Function:       usb_phy_suspend
Description:    A核进休眠时关闭USB phy电源
Input:          platform设备
Output:         None
Return:         int
Others:         None
******************************************************************************/
static int usb_phy_suspend(struct device *dev);

/******************************************************************************
Function:       usb_phy_resume
Description:    A核唤醒时打开USB phy电源
Input:          platform设备
Output:         None
Return:         int
Others:         None
******************************************************************************/
static int usb_phy_resume(struct device *dev);

/* platform 设备suspend以及resume函数指针赋值 */
static const struct dev_pm_ops usb_phy_pm_ops = {
    .suspend = usb_phy_suspend,
    .resume = usb_phy_resume,
};

/* platform 设备休眠唤醒pm函数回调 */
#define BALONG_DEV_PM_OPS (&usb_phy_pm_ops)

/* platform 驱动函数初始化 */
static struct platform_driver tbox_conn_driver = {
    .probe = NULL,
    .remove = NULL,
    .driver = {
        .name = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .pm     = BALONG_DEV_PM_OPS,
    },
};
#endif
struct tbox_conn_device{
    struct device *dev;
    struct dwc3 *dwc;
    unsigned int wakeup_gpio;

    /*wakeupin gpio state 
    0:low level, board can sleep
    other:high level, board should remain wake*/
    int last_wakeup_stat;
	
    /*usb connect state 
    0:disconnect;
    1:connect; */
    usb_conn_stat_t dwc3_conn_stat;
	
    /*debug count*/
    unsigned int tbox_conn_dbg_gpio_total_event;
	
    unsigned int tbox_conn_dbg_gpio_down_event;
    unsigned int tbox_conn_dbg_gpio_down_disconn;
    unsigned int tbox_conn_dbg_gpio_down_conn;

    unsigned int tbox_conn_dbg_gpio_up_event;
    unsigned int tbox_conn_dbg_gpio_up_disconn;
    unsigned int tbox_conn_dbg_gpio_up_conn;

    unsigned int tbox_conn_dbg_gpio_fake_event;

    unsigned int tbox_conn_dbg_gpio_wakeup_fail;
    unsigned int tbox_conn_dbg_gpio_sleep_fail;
};

struct tbox_conn_device *balong_dwc3_tbox_conn = NULL;

static const struct of_device_id balong_tbox_conn_match[] = {
			{.compatible = "hisilicon,usb3"},
			{},
};

void dwc3_tbox_conn_stat_set(usb_conn_stat_t dwc3_conn_flag)
{
    if(unlikely(NULL == balong_dwc3_tbox_conn)){
        printk("tbox conn stat set fail! No device\n");
        return;
    }
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* USB接入时将USB 状态置为resume激活态 */
    if (USB_GADGET_CONNECT == dwc3_conn_flag)
    {
        huawei_pm_handle_usb_resume_complete();
    }
#endif

    balong_dwc3_tbox_conn->dwc3_conn_stat = dwc3_conn_flag;

}


int dwc3_tbox_wakeup_gpio_stat_get(void)
{
    int value = 0;
    if(unlikely(NULL == balong_dwc3_tbox_conn)){
        printk("tbox gpio stat get fail! No device\n");
        return value;
    }

    value = gpio_get_value(balong_dwc3_tbox_conn->wakeup_gpio);
    return value;
}

static irqreturn_t dwc3_tbox_wakeupin_interrupt(int irq, void *dev_id)
{
    struct tbox_conn_device *tbox_conn_dev = (struct tbox_conn_device *)dev_id;
    int cur_wakeup_stat = 0;

    if(unlikely(NULL == tbox_conn_dev)){
        printk("tbox conn gpio intr, dev fail not init \n");
        return IRQ_HANDLED;
    }
    cur_wakeup_stat = dwc3_tbox_wakeup_gpio_stat_get();
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* wakeupin中断处理函数中设置wakeupin 管脚状态，cancle掉已经在队列当中的wakeupin上报延时任务 */
    cancel_delayed_work(&wakeupin_gpio_report_work);
    huawei_pm_set_pin_status(cur_wakeup_stat);
#endif
    tbox_conn_dev->tbox_conn_dbg_gpio_total_event++;
	
    /*Gpio pull down, board go to sleep. */
    if(tbox_conn_dev->last_wakeup_stat > cur_wakeup_stat){
#if(FEATURE_OFF == MBB_COMMON)
        printk("[tbox dbg]gpio pull downterrupt \n");
#endif

        tbox_conn_dev->tbox_conn_dbg_gpio_down_event++;

        /*disconnect event calling connect ctrl module func 
         only when usb alreay disconnect */
        if(USB_GADGET_DISCONNECT == tbox_conn_dev->dwc3_conn_stat){
            tbox_conn_dev->tbox_conn_dbg_gpio_down_disconn++;
            dwc3_conn_usb_disable();
        }else{
		tbox_conn_dev->tbox_conn_dbg_gpio_down_conn++;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if (FEATURE_ON == MBB_FACTORY)
        /* 烧片版本进入pstandby模式后，wakeupin拉低后不走usb disconnect流程，避免冲突 */
        if(0 == huawei_pm_pstandby_state_get())
        {
            dwc3_gadget_disconnect_cb();
        }
#endif
#endif
        }
    }
    /*Gpio pull up, wakeup board. */
    else if(tbox_conn_dev->last_wakeup_stat < cur_wakeup_stat){
#if(FEATURE_OFF == MBB_COMMON)
        printk("[tbox dbg]gpio pull up interrupt \n");
#endif
        tbox_conn_dev->tbox_conn_dbg_gpio_up_event++;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
        /* wakeupin拉高投票禁止休眠 */
        bsp_wakeupin_lock_wakelock();
#endif
        if(USB_GADGET_DISCONNECT == tbox_conn_dev->dwc3_conn_stat){
            tbox_conn_dev->tbox_conn_dbg_gpio_up_disconn++;
            dwc3_conn_usb_connect();
        }else{
            tbox_conn_dev->tbox_conn_dbg_gpio_up_conn++;
        }
    }

    /*Gpio no change but there is an interrupt*/
    else{
        tbox_conn_dev->tbox_conn_dbg_gpio_fake_event ++;
    }
    tbox_conn_dev->last_wakeup_stat = cur_wakeup_stat;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* wakeupin中断处理函数中启动wakeupin上报延时任务 */
    schedule_delayed_work(&wakeupin_gpio_report_work, 0);
#endif
    return IRQ_HANDLED;
}

static int dwc3_tbox_wakeupin_gpio_init(unsigned int tbox_gpio, 
	struct tbox_conn_device *tbox_conn_dev)
{
    int ret = 0;

    ret = gpio_request(tbox_gpio, "usb_tbox_conn_dect");
    if(ret){
        printk("usb tbox conn detect:request gpio%d is error!\n", tbox_gpio);
        return ret;
    }

    ret = gpio_direction_input(tbox_gpio);
    if(ret){
        printk("usb tbox conn detect:gpio direction input error!\n");
        goto dir_set_err;
    }
    tbox_conn_dev->last_wakeup_stat = dwc3_tbox_wakeup_gpio_stat_get();
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* wakeupin gpio初始化管脚状态和根据管脚状态判断是否需要持休眠锁 */
    huawei_pm_set_pin_status(tbox_conn_dev->last_wakeup_stat);
    if (0 == tbox_conn_dev->last_wakeup_stat)
    {
        bsp_wakeupin_unlock_wakelock();
    }
    else
    {
        bsp_wakeupin_lock_wakelock();
    }
#else
    if(tbox_conn_dev->last_wakeup_stat)
        bsp_usb_lock_wakelock();
#endif
    /*conncet tbox wakeup interrupt*/
    ret = request_irq(gpio_to_irq(tbox_gpio), dwc3_tbox_wakeupin_interrupt, 
		IRQF_NO_SUSPEND| IRQF_SHARED | IRQF_TRIGGER_RISING  | IRQF_TRIGGER_FALLING, 
		"usb_tbox_conn", tbox_conn_dev);

    if(ret){
        printk("usb tbox conn detect:request usb tbox gpio irq is error!\n");
        goto irq_err;
    }
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 导出wakeupin GPIO到用户空间使用 */
    ret = gpio_export(tbox_gpio,0);
    if (0 != ret)
    {
        printk("export WAKEUP_IN to userspace fail  \n");
    }
#endif
    return 0;
dir_set_err:
irq_err:
    gpio_free(tbox_gpio);

    return ret;

}
#ifdef BSP_CONFIG_BOARD_TELEMATIC
/******************************************************************************
Function:       huawei_pm_wakeupin_cb
Description:    wakupin 中断回调函数
Input:          延时任务调度
Output:         None
Return:         None
Others:         None
******************************************************************************/
static void huawei_pm_wakeupin_cb(struct work_struct *w)
{
    int voltage = 0;
    HW_PM_INIT_STATUS status = huawei_pm_status_get();

    voltage = dwc3_tbox_wakeup_gpio_stat_get();
    /* 电源管理未初始化完成不上报^wakeupin主动上报 */
    if (HUAWEI_PM_INIT_OK == status)
    {
        wakeup_in_state_report(voltage);
    }

    if (0 == voltage)
    {
        printk("[tbox dbg]gpio pull downterrupt \n");
        bsp_wakeupin_unlock_wakelock();
    }
    else
    {
        printk("[tbox dbg]gpio pull upterrupt \n");
        if (HUAWEI_PM_INIT_OK == status)
        {
            huawei_pm_remote_shake_complete();
        }
        else
        {
            /* 暂时不上报缓存等待电源管理模块初始化完成 */;
        }
    }
}
#endif
#ifdef BSP_CONFIG_BOARD_TELEMATIC
/******************************************************************************
Function:       usb_phy_suspend
Description:    A核进休眠时关闭USB phy电源
Input:          platform设备
Output:         None
Return:         int
Others:         None
******************************************************************************/
static int usb_phy_suspend(struct device *dev)
{
    syssc_usb_powerdown_hsp(1);
    syssc_usb_powerdown_ssp(1);
    return 0;
}
/******************************************************************************
Function:       usb_phy_resume
Description:    A核唤醒时打开USB phy电源
Input:          platform设备
Output:         None
Return:         int
Others:         None
******************************************************************************/
static int usb_phy_resume(struct device *dev)
{
    syssc_usb_powerdown_hsp(0);
    syssc_usb_powerdown_ssp(0);
    return 0;
}

#endif
static int dwc3_tbox_conn_balong_init(void)
{
    struct device_node * usb3_node = NULL;
    struct tbox_conn_device *tbox_conn_dev = NULL;
    struct conn_detect_ops reg_ops;
    struct platform_device	*tbox_conn_pdev;
    u32 para_value[1] = {0};
    int ret;
	
/*fixme:after full fix connect disconnect process use usb dwc3_balong device*/
    if(USB_TBOX_DETECT != bsp_usb_vbus_detect_mode()){
        return 0;
    }

    tbox_conn_pdev = platform_device_alloc("tbox_conn_pdev", PLATFORM_DEVID_AUTO);
    if (!tbox_conn_pdev) {
        printk("usb tbox conn detect:couldn't allocate tbox_conn platform device\n");
        ret = -ENOMEM;
        goto err0;
    }
	
    ret = platform_device_add(tbox_conn_pdev);
    if (ret) {
        printk("usb tbox conn detect:failed to register xHCI device\n");
        goto err1;
    }

/*end fixme*/

#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 注册tbox usb platform设备 */
    ret = platform_driver_register(&tbox_conn_driver);
    if (0 != ret)
    {
        goto err2;
    }
#endif

    tbox_conn_dev = devm_kzalloc(&tbox_conn_pdev->dev, sizeof(struct tbox_conn_device), GFP_KERNEL);
    if(NULL == tbox_conn_dev){
        printk("usb tbox conn detect:Alloc usb tbox connect detect device fail. \n");
        ret = -ENOMEM;
        goto err2;
    }

    tbox_conn_dev->dev = &tbox_conn_pdev->dev;
    tbox_conn_dev->dwc = NULL;

    usb3_node = of_find_compatible_node(NULL, NULL, balong_tbox_conn_match[0].compatible);
    if(NULL == usb3_node){
        printk("usb tbox conn detect:dwc3 platform device node not found \n");
        ret = -EINVAL;
        goto err3;
    }

    ret = of_property_read_u32_array(usb3_node, "tbox_wakeupin_gpio", para_value, 1);
    if(ret){
        printk("usb tbox conn detect:get tbox gpio from dts fail. \n");
        goto err3;
    }

    tbox_conn_dev->wakeup_gpio = para_value[0]; 
    balong_dwc3_tbox_conn = tbox_conn_dev;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 初始化wakeupin event上报延时任务 */
    INIT_DELAYED_WORK(&wakeupin_gpio_report_work, 
                        huawei_pm_wakeupin_cb);
#endif
    ret = dwc3_tbox_wakeupin_gpio_init(tbox_conn_dev->wakeup_gpio, tbox_conn_dev);
    if(ret){
        printk("usb tbox conn detect:dwc3_wakeupin_gpio_init fail. \n");
        goto err3;
    }

    reg_ops.tbox_conn_stat_set_cb = dwc3_tbox_conn_stat_set;
    reg_ops.tbox_wakeup_stat_get_cb = dwc3_tbox_wakeup_gpio_stat_get;
    ret = dwc3_conn_register_ops(&reg_ops);
    if(ret){
        printk("usb tbox conn detect:dwc3_conn_register_ops fail. \n");
        goto err3;
    }
    
    return 0;

err3:
	/*mem alloced by devm_alloc will be free when device release. */
err2:
    platform_device_del(tbox_conn_pdev);

err1:
    platform_device_put(tbox_conn_pdev);

err0:
	return ret;

}

module_init(dwc3_tbox_conn_balong_init);


MODULE_AUTHOR("BALONG USBNET GROUP");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("BALONG USB3 DRD Controller Driver");
