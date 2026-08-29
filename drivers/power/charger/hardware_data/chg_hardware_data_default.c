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
#include "chg_hardware_data.h"

static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4200,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/

    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        500,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static CHG_TEMP2ADC adc2temp_map = {
    .adc_batt_therm_map = &g_adc_batt_therm_map,
    .size = sizeof(g_adc_batt_therm_map) / sizeof(g_adc_batt_therm_map[0]),
};

static const uint32_t chg_fastchg_timeout_value_in_sec[2][2] =
{
    /*Power-off charge.*/
    {
        CHG_POWEROFF_FAST_USB_TIMER_VALUE,           /*USB/NoStd Chgr*/
        CHG_POWEROFF_FAST_CHG_TIMER_VALUE,           /*Wall/Standard Chgr*/
    },
    /*Normal charge.*/
    {
        CHG_FAST_USB_TIMER_VALUE,                    /*USB/NoStd Chgr*/
        CHG_FAST_CHG_TIMER_VALUE,                    /*Wall/Standard Chgr*/
    },
};

static struct chg_hardware_data chg_hardware_data_default = {
        .id_voltage_min = 666,
        .id_voltage_max = 888,
        .chg_std_chgr_hw_paras        = &chg_std_chgr_hw_paras,
        .chg_usb_chgr_hw_paras        = &chg_usb_chgr_hw_paras,
        .chg_usb3_chgr_hw_paras       = &chg_usb3_chgr_hw_paras,
        .adc2temp_map    = &adc2temp_map,
        .batt_volt_hkadc_id = 1,
        .vbus_volt_hkadc_id = 2,
        .batt_temp_hkadc_id = 10,
        .batt_cali_volt_min = 3400,
        .batt_cali_volt_max = 4200,
        .batt_bad_volt = 2700,
        .batt_valid_temp = -30,
        .extchg_stop_temp = 55,
        .extchg_resume_temp = 51,    
        .chg_fastchg_timeout_value_in_sec    = &chg_fastchg_timeout_value_in_sec,
        .chg_ic_en_gpio = 92,

};