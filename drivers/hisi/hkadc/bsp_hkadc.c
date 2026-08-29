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




#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <osl_sem.h>
#include <bsp_trace.h>
#include <bsp_icc.h>
#include <bsp_hkadc.h>
#include <bsp_slice.h>


#include <bsp_temperature.h>
#include <mdrv_temp_cfg.h>

static bool g_hkadc_msg_print_flag = false;

static int bsp_hkadc_acore_init(void);

static u16 g_hkadc_voltage;
static osl_sem_id g_hkadc_icc_sem_id;
static osl_sem_id g_hkadc_value_sem_id;
HKADC_DEBUG_INFO hkadc_debug_info;

int bsp_hkadc_convert(enum HKADC_CHANNEL_ID channel, u16* value)
{
    int ret;
    u32 hkadc_channel_id = (u32)channel;
    u32 icc_channel_id = ICC_CHN_MCORE_ACORE << 16 | MCORE_ACORE_FUNC_HKADC;

    osl_sem_down(&g_hkadc_icc_sem_id);

    ret = bsp_icc_send(ICC_CPU_MCU, icc_channel_id,
                       (u8*)&hkadc_channel_id, sizeof(hkadc_channel_id));
    if (ret != (int)sizeof(hkadc_channel_id))
    {
        osl_sem_up(&g_hkadc_icc_sem_id);
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_HKADC, "icc send error, error code: 0x%x\r\n", ret);
        return ret;
    }
    hkadc_debug_info.icc_send_stamp = bsp_get_slice_value();

    /*coverity[lock] */
    osl_sem_down(&g_hkadc_value_sem_id);

    if (0xFFFF == g_hkadc_voltage)
    {
        ret = -1;
    }
    else
    {
        ret = 0;
        *value = g_hkadc_voltage;
    }

    osl_sem_up(&g_hkadc_icc_sem_id);

    /*coverity[missing_unlock] */
    return ret;
}
EXPORT_SYMBOL_GPL(bsp_hkadc_convert);

static int bsp_hkadc_icc_callback(u32 icc_channel_id , u32 len, void* context)
{
    int ret;
    u16 voltage = 0;

    ret = bsp_icc_read(icc_channel_id, (u8*)&voltage, len);
    if (ret != (int)sizeof(voltage))
    {
        g_hkadc_voltage = 0xFFFF;
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_HKADC, "hkadc icc read error, error code: 0x%x\r\n", ret);
    }
    else
    {
        g_hkadc_voltage = voltage;
    }
    hkadc_debug_info.icc_callback_stamp = bsp_get_slice_value();

    osl_sem_up(&g_hkadc_value_sem_id);

    return 0;
}


/*****************************************************************************
 函 数 名     : bsp_hkadc_print_set
 功能描述     : 设置HKADC打印使能
 输入参数     : u8 state 打印状态
               非0 ： 使能打印
               0   ： 关闭打印
 输出参数     : 无
 返 回 值     : 无
 修改历史     :
 函数生成日期 : 2014年8月1日
*****************************************************************************/
void bsp_hkadc_print_set(u8 flag)
{
    g_hkadc_msg_print_flag = !!flag;
}

/********************************************************
*函数名   : bsp_hkadc_get_temp
*函数功能 : 通过共享内存获取给定通道的温度值，该方式不会阻塞。
*输入参数 : chan: 物理ADC通道值
*输出参数 : 无
*返回值   : TEMPERATURE_ERROR（-1）：表示获取出错。
            ELSE：有效的温度值
*修改历史 :
*           2015-8-31 韩久平 移植自V7R2平台
********************************************************/
short bsp_hkadc_get_temp(int chan)
{
    short temp = 0;
    DRV_HKADC_DATA_AREA *p_area = (DRV_HKADC_DATA_AREA *)TEMPERATURE_VIRT_ADDR;
    if(NULL == p_area)
    {
        pr_err("p_area is NULL!!!\n");
        return TEMPERATURE_ERROR;
    }
    if(TEMPERATURE_MAGIC_DATA != p_area->magic_start 
        || (TEMPERATURE_MAGIC_DATA != p_area->magic_end))
    {
        pr_err("temp mem is writed by others.\n");
        return TEMPERATURE_ERROR;
    }
    if((chan > HKADC_CHANNEL_MAX) || (chan < HKADC_CHANNEL_MIN))
    {
        pr_err("hkadc channel %d is error!!\n", chan);
        return TEMPERATURE_ERROR;
    }
    temp = p_area->chan_out[chan].temp_l;

    /*Set to emergency level to show the msg always.*/
    if(true == g_hkadc_msg_print_flag)
    {
        printk(KERN_EMERG "The temperature of channel %d is %d'C.\n", chan, temp);
    }

    return temp;
}

/********************************************************
*函数名   : bsp_hkadc_get_volt
*函数功能 : 通过共享内存获取给定通道的电压值，该方式不会阻塞。
*输入参数 : chan: 物理ADC通道值
*输出参数 : 无
*返回值   : TEMPERATURE_ERROR（-1）：表示获取出错。
            ELSE：有效的电压值
*修改历史 :
*           2015-8-31 韩久平 移植自V7R2平台
********************************************************/
int bsp_hkadc_get_volt(int chan)
{
    int volt = 0;
    DRV_HKADC_DATA_AREA *p_area = (DRV_HKADC_DATA_AREA *)TEMPERATURE_VIRT_ADDR;
    if(NULL == p_area)
    {
        pr_err("p_area is NULL!!!\n");
        return TEMPERATURE_ERROR;
    }
    if(TEMPERATURE_MAGIC_DATA != p_area->magic_start 
        || (TEMPERATURE_MAGIC_DATA != p_area->magic_end))
    {
        pr_err("tem mem is writed by others.\n");
        return TEMPERATURE_ERROR;
    }
    if((chan > HKADC_CHANNEL_MAX) || (chan < HKADC_CHANNEL_MIN))
    {
        pr_err("hkadc channel %d is error!!\n", chan);
        return TEMPERATURE_ERROR;
    }
    volt = p_area->chan_out[chan].volt_l;

    /*Set to emergency level to show the msg always.*/
    if(true == g_hkadc_msg_print_flag)
    {
        printk(KERN_EMERG "The voltage of channel %d is %dmV.\n", chan, volt);    
    }

    return volt;
}

static int bsp_hkadc_acore_init(void)
{
    int ret = 0;
    u32 icc_channel_id = ICC_CHN_MCORE_ACORE << 16 | MCORE_CCORE_FUNC_HKADC;

    osl_sem_init(1, &g_hkadc_icc_sem_id);
    osl_sem_init(0, &g_hkadc_value_sem_id);

    hkadc_debug_info.icc_callback_stamp = 0;
    hkadc_debug_info.icc_send_stamp     = 0;

    ret |= bsp_icc_event_register(icc_channel_id,
                                  (read_cb_func)bsp_hkadc_icc_callback, NULL, NULL, NULL);
    if (ret)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_HKADC, "hkadc init error, error code: 0x%x\r\n", ret);
    }

    return ret;
}


module_init(bsp_hkadc_acore_init);

MODULE_AUTHOR("z00227143@huawei.com");
MODULE_DESCRIPTION("HIS Balong V7R2 HKADC");
MODULE_LICENSE("GPL");

