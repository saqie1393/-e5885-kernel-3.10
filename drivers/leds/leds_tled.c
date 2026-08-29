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

#include <linux/platform_device.h>
#include <mdrv_leds.h>
#include <bsp_nvim.h>
#include <bsp_icc.h>
#include <bsp_softtimer.h>
#include <bsp_leds.h>
#include "leds_balong.h"
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#include <linux/gpio.h>
#include <mbb_flight_mode.h>
#endif /*FEATURE_ON == MBB_FEATURE_M2M_LED*/

struct led_param g_led_para = {0};                  /* led parameter */
struct led_tled_arg g_arg;                          /* 专门为do_led_threecolor_flush()传参 */
unsigned int g_led_old_state = LED_LIGHT_OFFLINE;   /* to store old status of three-color-light */
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define LED_ON    1
#define LED_OFF   0
void do_led_status(void);
extern struct softtimer_list ce_led_softtimer;
#endif /*FEATURE_ON == MBB_FEATURE_M2M_LED*/
int led_threecolor_flush(u32 channel_id , u32 len, void* context);
int do_led_threecolor_flush(void);

extern struct platform_device balong_led_dev;
extern struct softtimer_list led_softtimer;        /* soft timer */
extern struct nv_led g_nv_led;                      /* store nv */
extern LED_CONTROL_NV_STRU g_led_state_str_lte[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE]; /* 不同的状态对应的闪灯方案，功能与led nv一样 */

EXPORT_SYMBOL_GPL(g_arg);

/*******************************************************************************
 * FUNC NAME:
 * led_on()
 * Function     : turn on a led
 * Arguments
 *      input   : @led_id - led id
 *      output  : null
 *
 * Return       : null
 * Decription   :
 ************************************************************************/
void led_on(unsigned int led_id)
{
    unsigned long full_on = ALWAYS_ON_OFF_TIME_DR345* 1000;
    unsigned long full_off = 0;
    unsigned long fade_on = 0;
    unsigned long fade_off = 0;

    struct balong_led_device *led = platform_get_drvdata(&balong_led_dev);
    if(!led)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"get drvdata failed\n");
        return;
    }

    balong_led_breath_set(&led[led_id].cdev, &full_on, &full_off, &fade_on, &fade_off);
    balong_led_brightness_set(&led[led_id].cdev, LED_HALF);

    return;
}
EXPORT_SYMBOL_GPL(led_on);

/*******************************************************************************
 * FUNC NAME:
 * led_on()
 * Function     : turn on a led
 * Arguments
 *      input   : @led_id - led id
 *      output  : null
 *
 * Return       : null
 * Decription   :
 ************************************************************************/
void led_off(unsigned int led_id)
{
    struct balong_led_device *led = platform_get_drvdata(&balong_led_dev);
    if(!led)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"get drvdata failed\n");
        return;
    }

    balong_led_brightness_set(&led[led_id].cdev, LED_OFF);

    return;
}
EXPORT_SYMBOL_GPL(led_off);

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
void led_state_switch(unsigned int led_onoff)
{
    if ( LED_ON != led_onoff && LED_OFF != led_onoff )
    {
        return;
    }

    if ( LED_ON == led_onoff )    
    {
        M2M_LED_ON;
    }
    else
    {
        M2M_LED_OFF;
    }

    return;
}
extern g_atnl_gps_status;
/****************************************************************************************
 函 数 名  : led_gps_state
 功能描述  : 获取GPS状态
 输入参数  :
 输出参数  : 无
 返 回 值  : 返回GPS状态
 注意事项  : 无

****************************************************************************************/
int led_gps_state(void)
{
    return g_atnl_gps_status;
}

/****************************************************************************************
 函 数 名  : led_pin_control
 功能描述  : 控制GPIO_2_06电平值的接口函数
 输入参数  : highlowvalue
 输出参数  : 无
 返 回 值  : 无
 注意事项  : 无

****************************************************************************************/
void led_pin_control(int highlowvalue)
{
    gpio_direction_output(70,highlowvalue); /*70表示GPIO_2_06*/
    return;
}
/****************************************************************************************
 函 数 名  : do_led_status
 功能描述  : 处理led状态
 输入参数  : 
 输出参数  : 无
 返 回 值  : 无
 注意事项  : 无

****************************************************************************************/
void do_led_status(void)
{
    int get_gps_status = 0;
    int level_value = 0;
    int rfswitch_value = 0;
    get_gps_status = led_gps_state();
    rfswitch_value = RfSwitch_State_Get();
    if ((1 == get_gps_status) || ((1 == rfswitch_value) && (1 == g_led_para.led_color_id)))
    {
         level_value =LED_ON;
    }
    else
    {
        level_value =LED_OFF;
    }
    led_pin_control(level_value);
    bsp_softtimer_add(&ce_led_softtimer);

    return;
}
#endif /*FEATURE_ON == MBB_FEATURE_M2M_LED*/
/*******************************************************************************
 * FUNC NAME:
 * led_threecolor_state_switch() - three color light status change
 * Function     : update three color light status
 * Arguments
 *      input   : @new_color - new color
 *              : @old_color - old color
 *      output  : null
 *
 * Return       : null
 * Decription   : before call this function, make sure that new_state is ligal and nv is gotten
 ************************************************************************/
void led_threecolor_state_switch(unsigned char new_color, unsigned char old_color)
{
    LED_TRACE(LED_DEBUG_LEVEL(ERROR),"tled status change, %d(old) to %d(new)\n", old_color, new_color);

    if ((old_color & LED_RED) ^ (new_color & LED_RED))
    {
        if (new_color & LED_RED)
        {
            RED_ON;
        }
        else
        {
            RED_OFF;
        }
    }

    if ((old_color & LED_GREEN) ^ (new_color & LED_GREEN))
    {
        if (new_color & LED_GREEN)
        {
            GREEN_ON;
        }
        else
        {
            GREEN_OFF;
        }
    }

    if ((old_color & LED_BLUE) ^ (new_color & LED_BLUE))
    {
        if (new_color & LED_BLUE)
        {
            BLUE_ON;
        }
        else
        {
            BLUE_OFF;
        }
    }
    return;
}
EXPORT_SYMBOL_GPL(led_threecolor_state_switch);

/*******************************************************************************
 * FUNC NAME:
 * led_softtimer_modify()
 * Function     : modify softtimer time
 * Arguments
 *      input   : new_time  -   new time to set
 *      output  : null
 *
 * Return       : null
 * Decription   : null
 ************************************************************************/
void led_softtimer_modify_and_add(unsigned int new_time)
{
    int ret = LED_OK;
    static int i = 0;

    g_arg.ctl = MNTN_LED_TIMER_OCCURE;

    ret = bsp_softtimer_modify(&led_softtimer, new_time * LED_TIME_BASE_UNIT);
    i++;
    if(ret != LED_OK)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"ERROR: softtimer modify failed, ret = 0x%x, i = %d\n",
                  ret, i);
        dump_stack();
        return;
    }
    bsp_softtimer_add(&led_softtimer);

    return;
}
/*******************************************************************************
 * FUNC NAME:
 * tled_need_flash() -
 * Function     :
 * Arguments
 *      input   : null,该函数使用g_led_param为参数
 *      output  : null
 *
 * Return       : 0 - false
 *              : else - true
 * Decription   : 判断特定的闪灯方案需不需要启动timer
 ************************************************************************/
int tled_need_flash(void)
{
    unsigned char new_state = g_led_para.led_state_id;

    /* 常暗/常亮时，闪灯序列的第2项时间和颜色都为0 */
    if(0 == g_nv_led.g_led_state_str_om[new_state][1].ucTimeLength
            && 0 == g_nv_led.g_led_state_str_om[new_state][1].ucLedColor)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*******************************************************************************
 * FUNC NAME:
 * do_led_threecolor_flush() - three color light status flush
 * Function     : update three color light status
 * Arguments
 *      input   : null,该函数使用g_arg和g_led_param为参数
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : before call this function, make sure that new_state is ligal and NV has been gotten
 ************************************************************************/
int do_led_threecolor_flush(void)
{
    unsigned char old_color = g_led_para.led_color_id;
    unsigned char new_state = g_arg.new_state;
    unsigned int ctl = g_arg.ctl;
    int ret = LED_ERROR;

    if(MNTN_LED_STATUS_FLUSH == ctl)    /* 接受从c核发来的新状态，并根据NV值确定新的闪灯方案 */
    {
        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "tled status flash\n");

        g_led_para.led_state_id = new_state;
        g_led_para.led_config_id = 0;
        g_led_para.led_color_id = g_nv_led.g_led_state_str_om[new_state][0].ucLedColor;
        g_led_para.led_time = g_nv_led.g_led_state_str_om[new_state][0].ucTimeLength;

        /* timer can't be on-list before modified*/
        if(!list_empty(&led_softtimer.entry))
        {
            ret = bsp_softtimer_delete(&led_softtimer);
            if (ret)
            {
                LED_TRACE(LED_DEBUG_LEVEL(ERROR),"ERROR: softtimer delete failed, ret = %d\n", ret);
                return LED_ERROR;
            }
        }

        /* 闪灯方案中有亮灭的变化，才启动timer */
        if(tled_need_flash())
        {
            led_softtimer_modify_and_add(g_led_para.led_time);
        }

    }
    else if(MNTN_LED_TIMER_OCCURE == ctl)/* 同一个闪灯方案里，timer触发灯的状态(颜色、亮灭等)改变 */
    {
        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "tled timer occure\n");

        if((g_led_para.led_config_id >= (LED_CONFIG_MAX_LTE - 1))
                || (0 == g_nv_led.g_led_state_str_om[g_led_para.led_state_id][g_led_para.led_config_id + 1].ucTimeLength))
        {
            g_led_para.led_config_id = 0;
        }
        else
        {

            /* save old color */
            old_color = g_nv_led.g_led_state_str_om[g_led_para.led_state_id][g_led_para.led_config_id].ucLedColor;

            g_led_para.led_config_id++;
        }

        g_led_para.led_color_id = g_nv_led.g_led_state_str_om[g_led_para.led_state_id][g_led_para.led_config_id].ucLedColor;
        g_led_para.led_time = g_nv_led.g_led_state_str_om[g_led_para.led_state_id][g_led_para.led_config_id].ucTimeLength;

        led_softtimer_modify_and_add(g_led_para.led_time);

        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "new color %d, new time(*100 ms) %d\n",
                  g_led_para.led_color_id, g_led_para.led_time);

    }
    else
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "unknown ctrl: %d\n", ctl);
        return LED_ERROR;
    }

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
    led_state_switch(g_led_para.led_color_id);
#else /*FEATURE_ON == MBB_FEATURE_M2M_LED*/
    led_threecolor_state_switch(g_led_para.led_color_id, old_color);
#endif /*FEATURE_ON == MBB_FEATURE_M2M_LED*/

    return LED_OK;
}
EXPORT_SYMBOL_GPL(do_led_threecolor_flush);

/*******************************************************************************
 * FUNC NAME:
 * led_threecolor_flush() - three color light flush
 * Function     : update three color light status
 * Arguments
 *      input   : @newstatus - new status
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : prepare for update led status
 ************************************************************************/
int led_threecolor_flush(u32 channel_id , u32 len, void* context)
{
    int ret = LED_ERROR;
    int length = 0;
    unsigned char new_state;

    LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "LED acore begin to flush state\n");

    /* get new status from c core */
    length = bsp_icc_read(LED_ICC_CHN_ID, &new_state, sizeof(unsigned char));
    if(length != (int)sizeof(unsigned char))
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "icc read failed, length = 0x%x\n", length);
        return LED_ERROR;
    }

    LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "LED icc read OK, new state %d\n", new_state);

    /* check whether new status is illigal */
    if(new_state >= LED_LIGHT_STATE_MAX)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "illigal new status, new_state = %d!\n", new_state);
        return LED_ERROR;
    }

    /* update led status */
    /* 这里do_led_threecolor_flush要作为timer的注册函数，只能有一个unsigned int 类型的参数，故将参数封装一个结构体 */
    g_arg.new_state = new_state;
    g_arg.ctl = MNTN_LED_STATUS_FLUSH;
    ret = do_led_threecolor_flush();
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "three color led status update failed, ret = 0x%x\n", ret);
        return LED_ERROR;
    }

    /* return */
    return ret;
}

