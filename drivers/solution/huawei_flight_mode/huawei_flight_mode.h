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

#ifndef __HUAWEI_FLIGHT_MODE_H__
#define __HUAWEI_FLIGHT_MODE_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/spinlock.h>
#include "bsp_softtimer.h"


#define WDIS_DRIVER_NAME         "W-DISABLE"
#define RF_STATUS_DRIVER_NAME    "RF_STATUS"
#define W_DISABLE_PIN            GPIO_0_4
#define RF_STATUS_PIN            GPIO_2_6
#define WDIS_N_TIMER_LENGTH      (200)
#define TRIG_QUEUE_LENGTH        (30)
typedef struct 
{
    unsigned int  hw_temp;   /*gpio中断来的时候第一次记录的gpio值*/
    unsigned int  hw_state;  /*当前硬件gpio状态*/
    unsigned int  sw_state;  /*当前软件状态*/
    spinlock_t    lock;
    struct softtimer_list irq_timer;
}RFSWITCH_T_CTRL_S;

/*飞行模式切换触发类型存储队列中元素类型定义*/
typedef struct
{
    unsigned int is_set;      /*状态标识对应的触发类型是否有效*/
    unsigned int trig_type;   /*飞行模式切换触发类型*/
}queue_trig;

#ifdef __cplusplus
}
#endif
#endif


