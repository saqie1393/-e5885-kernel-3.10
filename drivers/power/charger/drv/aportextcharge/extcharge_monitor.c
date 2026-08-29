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


 /*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "chg_config.h"
#include <bsp_hkadc.h>
#include <linux/gpio.h>
#include "extcharge_monitor.h"
#include "hi_gpio.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
 
 
int extchg_is_extcharge_device_on(void)
{
    int ret = 0;

    ret = gpio_get_value(EXTCHG_OTG_DET_GPIO);
    if (ret < 0)
    {
        return EXTCHG_OFFLINE;
    }

    return ret;
}


int extchg_set_charge_enable(boolean enable)
{
    int ret = 0;

    if (TRUE == enable)
    {
        ret = gpio_direction_output(SHORT_PROTECT_EN, 0);
        ret |= gpio_direction_output(EXTCHG_CHG_ENABLE, 1);
        mdelay(DELAY_TIME_OF_DEBOUNCE);
        ret |= gpio_direction_output(EXTCHG_CHG_ENABLE, 0);
    }
    else
    {
        ret = gpio_direction_output(SHORT_PROTECT_EN, 1);
    }
    if(ret)
    {
        chg_print_level_message(CHG_MSG_ERR,"EXTCHG: %s EXTCHG_CHG_ENABLE failed!!\r\n ",\
                                             (enable?"Enable":"Disable"));
        return -1;
    }

    chg_print_level_message(CHG_MSG_ERR,"EXTCHG: %s EXTCHG_CHG_ENABLE success!!\r\n ",\
                                           (enable?"Enable":"Disable"));
    return 0;
}


void extchg_set_cur_level(EXTCHG_ILIM curr)
{
    if (RE_ILIM_500mA == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 0);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 500mA!!\r\n ");
    }
    else if(RE_ILIM_1A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 1000mA!!\r\n ");
    }
    else if(RE_ILIM_2A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 1);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 2000mA!!\r\n ");
    }
    else
    {
        //do nothing
    }
}


boolean extchg_is_perph_circult_short(void)
{
    int32_t volt_of_short = 0;
    int32_t volt_of_short_original = 0;
    int ret = 0;

    ret = bsp_hkadc_convert(EXTCHG_SHORT_VOLT_CHAN, &volt_of_short);
    if(ret)
    {
        chg_print_level_message( CHG_MSG_ERR,"EXTCHG: detect volt_of_short failed, treat as short!!!\r\n ",0,0,0 );
        return TRUE;
    }
    chg_print_level_message( CHG_MSG_DEBUG, \
        "EXTCHG:volt_of_get = %d\n", volt_of_short,0,0);

    volt_of_short_original = (volt_of_short * RSIISTANCE_CENT_COUNT);//采集的电压为分压后的值，还原为原始值
    if (volt_of_short_original <= EXTCHG_SHORT_VOLT_THRESHOLD)
    {
        chg_print_level_message( CHG_MSG_ERR, \
                "EXTCHG: perph_circult is short! valtage = %d\n", volt_of_short_original,0,0);
        return TRUE;
    }
    else
    {
        chg_print_level_message( CHG_MSG_DEBUG,"EXTCHG: perph_circult is ok!\\n", 0,0,0);
        return FALSE;
    }
}


chg_chgr_type_t extchg_charger_type_detect(void)
{
    int ret = 0;

    gpio_direction_input(USB_GPIO_DM);
    gpio_direction_input(USB_GPIO_DP);
    mdelay(100);

    /*首先检测DP是否为低，若为低则是PC HOST*/
    ret = gpio_get_value(USB_GPIO_DP);
    if (0 == ret)
    {
        return CHG_USB_HOST_PC;
    }
    else
    {
        /*其次将DM拉低，DP设为输入，读取DP的值，
        若为低则DP、DM短接，充电器为标准充电器
        若为高则DP、DM不短接，为非标充电器*/
        gpio_direction_output(USB_GPIO_DM, 0);
        ret = gpio_get_value(USB_GPIO_DP);
        if (0 == ret)
        {
            return CHG_WALL_CHGR;
        }
        else
        {
            return CHG_NONSTD_CHGR;
        }
    }
}


boolean extchg_is_charge_status_error(void)
{
    chg_chgr_type_t charge_type = CHG_CHGR_UNKNOWN;

    if(EXTCHG_ONLINE == extchg_is_extcharge_device_on())
    {
        //切换开关端口
        gpio_direction_output(GPIO_USB_SELECT, USB_SWITCH_APORT_LEVEL);
        mdelay(DELAY_TIME_20MS);
        //使用GPIO检测充电器类型
        charge_type = extchg_charger_type_detect();
        if (charge_type == CHG_NONSTD_CHGR)
        {
            if (TRUE == chg_is_charger_present())
            {
                chg_print_level_message(CHG_MSG_ERR, \
                    "CHG_PLT:CHARGING TO ITSELF, SUSPEND CHARGE!\n ");
                gpio_direction_output(GPIO_USB_SELECT,USB_SWITCH_MPORT_LEVEL);
                return TRUE;
            }
        }
        gpio_direction_output(GPIO_USB_SELECT,USB_SWITCH_MPORT_LEVEL);
        return FALSE;
    }
    return FALSE;
}