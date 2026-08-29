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

#include <linux/wakelock.h>
#include <linux/usb/balong_usb_nv.h>
#include "dwc3_conn_ctrl_balong.h"

#if (FEATURE_ON == MBB_USB)
#include "usb_workqueue.h"
#endif
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#include "usb_event.h"
#endif
extern void syssc_usb_powerdown_hsp(int value);
extern void syssc_usb_powerdown_ssp(int value);
extern void dwc3_sysctrl_init(void);
extern void dwc3_sysctrl_exit(void);

static struct wake_lock g_dwc_wakelock;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
/* wakeupin 休眠锁 */
static struct wake_lock wakeup_in_lock;
extern void huawei_pm_handle_usb_suspend(void);
#endif
static struct conn_detect_ops ops;
static struct dwc3 *conn_dwc;
static struct workqueue_struct *usb_conn_wq;
static struct delayed_work usb_conn_wk;


void bsp_usb_unlock_wakelock(void)
{

#if (FEATURE_ON == MBB_USB)
    usb_wake_unlock();
#else
    wake_unlock(&g_dwc_wakelock);
#endif

}
#ifdef BSP_CONFIG_BOARD_TELEMATIC
/******************************************************************************
Function:       bsp_wakeupin_unlock_wakelock
Description:    wakeupin拉低释放休眠锁
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void bsp_wakeupin_unlock_wakelock(void)
{
    wake_unlock(&wakeup_in_lock);
    return;
}
/******************************************************************************
Function:       bsp_wakeupin_lock_wakelock
Description:    wakeupin拉高持休眠锁
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void bsp_wakeupin_lock_wakelock(void)
{
    wake_lock(&wakeup_in_lock);
    return;
}
#endif
void bsp_usb_lock_wakelock(void)
{

#if (FEATURE_ON == MBB_USB)
    usb_wake_lock();
#else
    wake_lock(&g_dwc_wakelock);
#endif

}

int dwc3_conn_register_ops(struct conn_detect_ops *reg_ops)
{
    if(NULL == reg_ops){
        printk("dwc3_conn_register_ops detcet dwc3 register fail. \n");
        return -ENXIO;
    }

    ops.tbox_wakeup_stat_get_cb = reg_ops->tbox_wakeup_stat_get_cb;
    ops.tbox_conn_stat_set_cb = reg_ops->tbox_conn_stat_set_cb;
    return 0;
}

void dwc3_conn_usb_set_dwc_cb(struct dwc3 *dwc)
{
    if(NULL == dwc){
        printk("dwc3_conn_usb_set_dwc_cb fail. \n");
    }
    conn_dwc = dwc;
}

/*usb gadget hw disable secquence called by tbox.*/
void dwc3_conn_usb_disable(void)
{
    if(NULL == conn_dwc){
        printk("dwc3_conn_ctrl:conn_dwc is null. \n");
        return;
    }
	
    dwc3_gadget_dev_init_disconnect(conn_dwc);
		
    /*USB core, USB phy, USB bus clock disable*/
    dwc3_sysctrl_exit();
	
    bsp_usb_unlock_wakelock();
}


/*new disconnect event respond sequence, without remove usb device
In tbox mode: Called by gadget.c as usb_disconnect_cb call back function. */
void dwc3_conn_usb_disconnect(struct dwc3 *dwc)
{
    unsigned long flags;
    unsigned long timeout = 0;
    usb_vbus_detect_mode_t usb_detect_mode;
    
    usb_detect_mode = bsp_usb_vbus_detect_mode();
	
    switch (usb_detect_mode) {
    case USB_PMU_DETECT:
        /*do nothing for now*/
        break;
		
    case USB_TBOX_DETECT:

        if(NULL != ops.tbox_conn_stat_set_cb)
            ops.tbox_conn_stat_set_cb(USB_GADGET_DISCONNECT);
        else
		printk("[tbox dbg]tbox_conn_stat_set_cb NULL \n");
		
        if(unlikely(NULL == ops.tbox_wakeup_stat_get_cb)){
            printk("tbox disconnect fail, no call back function. \n");
            break;
        }
#ifdef BSP_CONFIG_BOARD_TELEMATIC
        /* USB断开消息上报用户空间并将USB状态置为suspend休眠态 */
        usb_notify_syswatch(EVENT_DEVICE_USB, USB_REMOVE_EVENT);
        huawei_pm_handle_usb_suspend();
#endif
        spin_lock_irqsave(&dwc->lock, flags);
        if(ops.tbox_wakeup_stat_get_cb()){
#ifdef BSP_CONFIG_BOARD_TELEMATIC
            /* 释放USB休眠锁 */
            bsp_usb_unlock_wakelock(); 
#endif
            /*This branch is for gpio still high, board stay wake
            the device attempts to reconnect to the host,
            at which time a USB Reset and Connect Done event occurs.*/
            spin_unlock_irqrestore(&dwc->lock, flags);
            printk("[tbox dbg]dwc3_conn_usb_disconnect gpio still high return \n");

            break;			
        }else{
            /*This branch is for gpio low, board deep sleep in progress
            the application does not want to attempt to reconnect to the host.*/
            printk("[tbox dbg]dwc3_conn_usb_disconnect gpio low call usb disable \n");

            queue_delayed_work(usb_conn_wq, &usb_conn_wk, timeout);

            spin_unlock_irqrestore(&dwc->lock, flags);
            break;
        }

    case USB_NO_DETECT:
        /*return without unlock wake lock*/
        return;
    default:
        printk("%s, invalid connect detect mode:%d\n",__FUNCTION__, usb_detect_mode);
        break;
    }    

}



void dwc3_conn_usb_connect_done(struct dwc3 *dwc)
{
    usb_vbus_detect_mode_t usb_detect_mode;

    usb_detect_mode = bsp_usb_vbus_detect_mode();

    switch (usb_detect_mode) {
    case USB_PMU_DETECT:
        /*do nothing for now*/
        break;
    case USB_TBOX_DETECT:
        bsp_usb_lock_wakelock();
        if(NULL != ops.tbox_conn_stat_set_cb)
            ops.tbox_conn_stat_set_cb(USB_GADGET_CONNECT);
        else
		printk("[tbox dbg]tbox_conn_stat_set_cb NULL \n");
        break;
    case USB_NO_DETECT:
        /*return without unlock wake lock*/
        break;
    default:
        printk("%s, invalid connect detect mode:%d\n",__FUNCTION__, usb_detect_mode);
    }   
    return;
}

void dwc3_conn_usb_connect(void)
{
    int ret;
    usb_vbus_detect_mode_t usb_detect_mode;

    usb_detect_mode = bsp_usb_vbus_detect_mode();

    switch (usb_detect_mode) {
    case USB_PMU_DETECT:
        /*do nothing for now*/
        break;
    case USB_TBOX_DETECT:
        if(NULL == conn_dwc){
            break;
        }
#ifndef BSP_CONFIG_BOARD_TELEMATIC
        /* 添加USB休眠锁 */
        bsp_usb_lock_wakelock();
#endif
        dwc3_sysctrl_init();
        conn_dwc->usb_core_powerdown = false;
        ret = dwc3_gadget_reinit(conn_dwc);
        if(ret){
            printk("dwc3_conn_usb_connect usb reset fail. \n");
        }
        break;
    case USB_NO_DETECT:
        /*return without unlock wake lock*/
        break;
    default:
        printk("%s, invalid connect detect mode:%d\n",__FUNCTION__, usb_detect_mode);
    }    
    return;
}

static void usb_conn_handler(struct work_struct *work)
{
    dwc3_conn_usb_disable();
}

 

static int dwc3_conn_detect_ctrl_init(void)
{
    struct dwc3_gadget_conn_ops reg_ops;
    usb_vbus_detect_mode_t usb_detect_mode;
    int ret = 0;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* USB NV初始化,可以从NV中读取USB VBUS使能状态 */
    bsp_usb_nv_init();
#endif
    usb_detect_mode = bsp_usb_vbus_detect_mode();
	

#if (FEATURE_ON == MBB_USB)
#else
    wake_lock_init(&g_dwc_wakelock, WAKE_LOCK_SUSPEND, "dwc3-wakelock");
#endif
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* USB VBUS开启模式，因为wakeupin流程和usb上下电是耦合的，所以就在VBUS初始化流程中初始化wakeupin投票 */
    if (USB_TBOX_DETECT == usb_detect_mode)
    {
        wake_lock_init(&wakeup_in_lock, WAKE_LOCK_SUSPEND, "pm_wakeupin");
    }
#endif

    switch (usb_detect_mode) {
		
    case USB_PMU_DETECT:
        break;
		
    case USB_TBOX_DETECT:

        reg_ops.usb_conndone_cb = dwc3_conn_usb_connect_done;
        reg_ops.usb_disconnect_cb = dwc3_conn_usb_disconnect;
        reg_ops.usb_set_dwc_cb = dwc3_conn_usb_set_dwc_cb;
        ret = dwc3_gadget_register_conn_ops(&reg_ops);
        if(ret){
            printk("dwc3_gadget_register_conn_ops fail. \n");
            return -EINVAL;
        }
        break;
		
    case USB_NO_DETECT:
        break;

    default:
        printk("%s, invalid connect detect mode:%d\n",__FUNCTION__, usb_detect_mode);
        return -EINVAL;
    }    

    usb_conn_wq = create_singlethread_workqueue("usb_conn_ctrl");
    if (!usb_conn_wq) {
        printk("%s: create_singlethread_workqueue fail\n", __FUNCTION__);
        return -ENOMEM;
    }
    INIT_DELAYED_WORK(&usb_conn_wk, (void *)usb_conn_handler);


	
    return ret;
}

module_init(dwc3_conn_detect_ctrl_init);

MODULE_AUTHOR("BALONG USBNET GROUP");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("BALONG USB3 DRD Controller Driver");
