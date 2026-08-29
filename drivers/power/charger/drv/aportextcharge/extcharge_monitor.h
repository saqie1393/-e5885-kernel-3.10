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

#ifndef _EXTCHG_MONITOR_H
#define _EXTCHG_MONITOR_H
 /*----------------------------------------------*
 * Head file                                   *
 *----------------------------------------------*/
 #include "chg_charge_cust.h"
 #include "chg_config.h"
/*----------------------------------------------*
 * External variable declaration               *
 *----------------------------------------------*/
/*----------------------------------------------*
 * External function declaration              *
 *----------------------------------------------*/
/*----------------------------------------------*
 * Internal function declaration               *
 *----------------------------------------------*/
/*----------------------------------------------*
 * Global variable                             *
 *----------------------------------------------*/

typedef enum
{
    RE_ILIM_NA = -1,
    RE_ILIM_STOP,
    RE_ILIM_500mA,
    RE_ILIM_1A,
    RE_ILIM_2A,
}EXTCHG_ILIM;

/*----------------------------------------------*
 * ∫Í∂®“Â                                       *
 *----------------------------------------------*/
#define EXTCHG_SHORT_VOLT_CHAN              (1)
#define RSIISTANCE_CENT_COUNT               (7)
#define EXTCHG_SHORT_VOLT_THRESHOLD         (2000)
#define DELAY_TIME_OF_DEBOUNCE              (200)
#define EXTCHG_OFFLINE                      (1)
#define EXTCHG_ONLINE                       (0)
#define USB_SWITCH_APORT_LEVEL              (1)
#define USB_SWITCH_MPORT_LEVEL              (0)
#define DELAY_TIME_20MS                     (20)
 
  
extern int extchg_is_extcharge_device_on(void);
 
 
extern int extchg_set_charge_enable(boolean enable);


extern void extchg_set_cur_level(EXTCHG_ILIM curr);


extern boolean extchg_is_perph_circult_short(void);


chg_chgr_type_t extchg_charger_type_detect(void);


boolean extchg_is_charge_status_error(void);
#endif