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



#ifndef HUAWEI_PM_H
#define HUAWEI_PM_H

#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include "bsp_softtimer.h"
#include "huawei_pm_at_cache.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "bsp_sram.h"
#include "bsp_nvim.h"
#include "mdrv_nvim_comm.h"

#ifndef FALSE
#define FALSE	    (0)
#endif

#ifndef TRUE
#define TRUE	    (1)  
#endif

#ifndef BSP_OK
#define BSP_OK       (0)
#endif

#ifndef BSP_ERROR
#define BSP_ERROR    (-1)
#endif

#ifndef BSP_VOID
typedef void       BSP_VOID;
#endif

/********************909s配置*****begin*********************/
/*Define WAKEUP_IN*/
#define LABEL_WAKEUP_IN         "wakeup_in"
/*Define SLEEP_STATUS*/
#define LABEL_SLEEP_STATUS      "sleep_status"
/*Define WAKEUP_OUT*/
#define LABEL_WAKEUP_OUT        "wakeup_out"
/********************909s配置*****end*********************/


/********************906s配置*****begin*********************/

/********************906s配置*****end*********************/

/*WAKEUP_OUT PIN expire time*/
#define WAKEUP_OUT_TIMEOUT          (1000)      

#define GPIO_LOW_VALUE        0
#define GPIO_HIGH_VALUE       1
/* wakupin 定时器超时时间 */
#define WAKEUPIN_N_TIMER_LENGTH      (200)
#define PRINT_ACORE_LOG(print_onoff, fmt, ...)  do { \
    if((print_onoff) == PM_MSG_PRINT_ON) { \
        printk("[module_pm]: <%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

/*WAKEUP_OUT PIN timer struct*/
typedef struct huawei_pm_timer_type
{
    boolean is_start;                   /*Timer start flag*/  
    struct delayed_work work;           /*timer_type*/
}huawei_pm_timer_type;

/*Define huawei PM struct*/
typedef struct huawei_pm_struct
{
    boolean is_usb_suspend;              /*USB suspend flag*/
    boolean is_usb_enum;                 /*USB enum flag*/
    boolean is_pm_pin_disable;           /*Disable pm pin flag*/
    uint32 rmt_wake_host_mask;           /*Mask for USB&PIN channel*/
    huawei_pm_timer_type pin_timer;      /*Timer for PIN wakeup*/ 
    remote_wakeup_info_type rmt_wk_cfg;  /*Remote wakeup config param*/
}huawei_pm_type;

typedef struct 
{
    unsigned int  wakeup_temp;   /*gpio中断来的时候第一次记录的gpio值*/
    unsigned int  wakeup_level;  /*当前GPIO的值*/
    spinlock_t    lock;
    struct softtimer_list irq_timer;
}WAKEUPIN_T_CTRL_S;

#if (FEATURE_ON == MBB_ATNLPROXY)
extern int huawei_pm_dispatch_uspace_data(void *buf, int32 buf_len, int32 port_idx);
#endif
/*===========================================================================
FUNCTION 
    huawei_pm_remote_wakeupcfg_init
DESCRIPTION
    Init Remote Wakuep Configuration. If NV read failure then use the default
    setting.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_remote_wakeupcfg_init(void);
/*===========================================================================
 
FUNCTION 
    huawei_pm_get_wakeup_channel_status
 
DESCRIPTION
    Get wakeup channels status.
 
DEPENDENCIES
    None
 
RETURN VALUE
    TRUE:   permit
    FALSE:  forbid
  
SIDE EFFECTS
    None
 
===========================================================================*/
boolean huawei_pm_get_wakeup_channel_status(rmt_wk_chl_type channel);
/******************************************************************************
Function:       huawei_pm_remote_wakuep_update
Description:   update remotewakeup info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_remote_wakuep_update(remote_wakeup_info_type src_config);
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_save_curc_info(dsat_curc_cfg_type *src_curc);
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_get_curc_info(dsat_curc_cfg_type *dst_curc);

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_time_update
* 功能描述  : 更新睡眠唤醒的握手超时时间
* 输入参数  : timeValue -- 设置时间
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_time_update(unsigned int timeValue);

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_time_get
* 功能描述  : 获取睡眠唤醒的握手超时时间值
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : remote_wakeup_shake_time -- 超时时间
*****************************************************************************/
unsigned int huawei_pm_remote_wakeup_time_get(void);

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_timer
* 功能描述  : 握手完成的处理函数
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_shake_complete(void);
/*===========================================================================
FUNCTION 
    huawei_pm_update_wakeup_channel_status
DESCRIPTION
    Update wakeup channels status.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_update_wakeup_channel_status(void);
/*===========================================================================
* 函 数 名  : huawei_pm_get_pin_suspended_status
* 功能描述  : 通过PIN握手方式获取上位机的睡眠状态
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : 1-上位机睡眠；0-上位机非睡眠
===========================================================================*/
unsigned int huawei_pm_get_pin_suspended_status(void);

/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_change_handle
* 功能描述  : 电源管理模块给USB提供的处理函数
* 输入参数  : enType --- 1   表示USB  进入S3/S4
                        0  表示USB  退出S3/S4
* 输出参数  :
* 返  回  值   :  NA
***************************************************************/
void huawei_pm_usb_s34_state_change_handle(unsigned char enType);
/******************************************************************************
Function:       huawei_pm_wakeup_remote_host
Description:   
Input:           rmt_wk_src_type rmt_wk_src
Output:         None
Return:          boolean wether the src can wakeup host
Others:          None
******************************************************************************/
boolean huawei_pm_wakeup_remote_host(rmt_wk_src_type rmt_wk_src);
/*===========================================================================
FUNCTION 
    huawei_pm_get_usb_suspended_status
DESCRIPTION
    Get USB Suspend Status.
DEPENDENCIES
    None
RETURN VALUE
    if USB has enter SUSPEND then return TRUE,
    else return FALSE;
SIDE EFFECTS
    None
===========================================================================*/
boolean huawei_pm_get_usb_suspended_status(void);
/*===========================================================================
FUNCTION 
    huawei_pm_request_wakeup_pin
DESCRIPTION
    Wakeup the Host through WAKEUP_OUT PIN
DEPENDENCIES
    None
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_request_wakeup_pin(void);
/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_timer
* 功能描述  : 起握手超时定时器
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_timer(void);

/******************************************************************************
Function:       huawei_pm_get_curc_status
Description:   
Input:           huawei_pm_rsp_id_type send_id
Output:         None
Return:          dsat_curc_status
Others:          None
******************************************************************************/
dsat_curc_status huawei_pm_get_curc_status(void);
#ifdef BSP_CONFIG_BOARD_TELEMATIC

#if(FEATURE_ON == MBB_FACTORY) 
/*===========================================================================
FUNCTION     :   huawei_pm_factory_unlock
DESCRIPTION  :   allow factory software to sleep.
DEPENDENCIES :   None
RETURN VALUE :   None
SIDE EFFECTS :   None
===========================================================================*/
void huawei_pm_factory_unlock(void);
/*************************************************************
* 函数名  : huawei_pm_factory_lock
* 功能描述: 防止烧片睡眠
* 输入参数: NA
* 输出参数:
* 返 回 值: NA
**************************************************************/
void huawei_pm_factory_lock(void);
#endif
#endif
/*************************************************************
* 函数名  : huawei_pm_pstandby_entry_handle
* 功能描述: 进入产线待机状态，通知A C核关闭远程唤醒
* 输入参数: NA
* 输出参数:
* 返 回 值: NA
**************************************************************/
void huawei_pm_pstandby_entry_handle(void);
/*************************************************************
* 函数名  : huawei_pm_pstandby_state_get
* 功能描述: 读取是否处于产线待机模式
* 输入参数: NA
* 输出参数:
* 返 回 值: NA
**************************************************************/
unsigned char huawei_pm_pstandby_state_get(void);

#endif /*HUAWEI_PM_H*/

