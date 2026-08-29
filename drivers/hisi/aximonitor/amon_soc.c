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

#include "osl_types.h"
#include "of.h"
#include "mdrv_public.h"
#include "bsp_dump.h"
#include "bsp_om_enum.h"
#include "bsp_sysctrl.h"
#include <linux/clk.h>
#include "amon_soc.h"
#include "amon_cfg.h"
#include "amon_balong.h"
#include "amon_cnt.h"
#include "amon_reg.h"

AMON_CFG_STRU     g_amon_hw_cfg;


AXI_STATE amon_soc_get_state(AXI_REQ_STATE state_req)
{
    u32 reg_value = 0;

    AXI_REG_READ(AXI_MON_CNT_STATE_INT, &reg_value);
    if(AXI_GET_RUN_STATE_REQ == state_req)
    {
        reg_value = reg_value & AXI_RUN_STATE_MASK;
    }
    else if(AXI_GET_RESET_STATE_REQ == state_req)
    {
        reg_value = reg_value & AXI_RESET_STATE_MASK;
    }
    else
    {
        return (AXI_STATE)AXI_STATE_BUTT;
    }

    return reg_value;
}


s32 axi_soc_state_check(void)
{
    AXI_STATE axi_state = AXI_STATE_BUTT;

    axi_state = amon_soc_get_state(AXI_GET_RUN_STATE_REQ);
    /* AXI monitor正在运行 */
    if(axi_state != AXI_IDLE && axi_state != AXI_STOP)
    {
        return MDRV_ERROR;
    }

    return MDRV_OK;
}


void amon_soc_soft_reset(void)
{
    void* base_addr =  g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_EN].base_addr;
    u32 start_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_EN].start_bit;
    u32 end_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_EN].end_bit;

    if(AMON_IS_VALID_ADDR(base_addr))
	{
        amon_set_reg(1, base_addr, 0, start_bit, end_bit);
    }
}


void amon_soc_rls_reset(void)
{
    void* base_addr =  g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_DIS].base_addr;
    u32 start_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_DIS].start_bit;
    u32 end_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_SRST_DIS].end_bit;

    if(AMON_IS_VALID_ADDR(base_addr))
    {
        amon_set_reg(1, base_addr, 0, start_bit, end_bit);
    }
}


s32 amon_soc_start(void)
{
    void* base_addr =  g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].base_addr;
    u32 start_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].start_bit;
    u32 end_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].end_bit;
    u32 i=0;
    AXI_STATE  axi_state = AXI_STATE_BUTT; 

    if(AMON_IS_VALID_ADDR(base_addr))
    {
        if(amon_get_reg(base_addr, 0, start_bit, end_bit))
        {
            amon_set_reg(0, base_addr, 0, start_bit, end_bit);
        }
        amon_set_reg(1, base_addr, 0, start_bit, end_bit);
    }

    /* 启动结束判定，等待启动标志置位 */
    do
    {
        axi_state = amon_soc_get_state(AXI_GET_RUN_STATE_REQ);
        if(AXI_UNWIN_RUNNING == axi_state || AXI_WIN_RUNNING == axi_state)
        {
            return MDRV_OK;
        }
    }while(i++ < AXI_WAIT_CNT);

    amon_error("start time out\n");
    return MDRV_OK;
}


s32 amon_soc_stop(void)
{
    void* base_addr =  g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].base_addr;
    u32 start_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].start_bit;
    u32 end_bit = g_amon_hw_cfg.amon_sysctrl[AMON_SOC_MONITOR_START].end_bit;
    u32 i = 0;
    AXI_STATE axi_state = AXI_STATE_BUTT;
    
    if(AMON_IS_VALID_ADDR(base_addr))
	{
        amon_set_reg(0, base_addr, 0, start_bit, end_bit);
    }
    
    do
    {
        axi_state = amon_soc_get_state( AXI_GET_RUN_STATE_REQ);
        if(AXI_STOP == axi_state)
        {
            /* 停止之后，强制进入IDLE态 */
            AXI_REG_WRITE(AXI_MON_CNT_RESET, AXI_RESET_TO_IDLE);
            return BSP_OK;
        }
        if(AXI_IDLE == axi_state)
        {
            return BSP_OK;
        }
    }while(i++ < AXI_WAIT_CNT);

    amon_error("stop time out\n");
    return BSP_ERROR;
}



void amon_soc_clk_enable(void)
{  
    if(g_amon_hw_cfg.soc_clk != NULL )
    {
       clk_prepare_enable(g_amon_hw_cfg.soc_clk);
    }

}


void amon_soc_clk_disable(void)
{    
    if(g_amon_hw_cfg.soc_clk != NULL )
    {
        clk_disable_unprepare(g_amon_hw_cfg.soc_clk);
    }
}

s32 amon_soc_get_clk_node(void)
{
    g_amon_hw_cfg.soc_clk = clk_get(NULL, "amon_soc_clk");
    if(IS_ERR(g_amon_hw_cfg.soc_clk))
    {
        amon_error("get amon_soc_clk fail\n");
        return MDRV_ERROR;
    }
    return MDRV_OK;
}

s32 amon_soc_self_reset(void)
{
    AXI_STATE axi_state = AXI_STATE_BUTT;
    int i = 0;

    AMON_SOC_CHECK_STATE();

    AXI_REG_WRITE(AXI_MON_CNT_RESET, AXI_SOFT_RESET);
    do
    {
        axi_state = amon_soc_get_state(AXI_GET_RESET_STATE_REQ);
        if(AXI_RESET_FINISH == axi_state)
        {
            return MDRV_OK;
        }
    }while(i++ < AXI_WAIT_CNT);

    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_AMON, "%s: time out\n", __FUNCTION__);
    return MDRV_ERROR;
}


__init s32 amon_soc_parse_dts(void)
{
	struct device_node *dev = NULL;
	const char *name = "hisilicon,amon_soc_balong";
    char* soc_reg_name[AMON_SOC_DTS_BUTT] = {"amon_soc_srst_en","amon_soc_srst_dis", "amon_soc_monitor_start"};
    u32 i = 0;
    u32 reg_offset[3] = {0,0,0};
    u32 amon_cfg_addr = 0;
    
    if(g_amon_hw_cfg.base_addr != NULL)
    {
        return MDRV_OK;
    }

    dev = of_find_compatible_node(NULL, NULL, name);
    if(NULL == dev)
    {
        amon_error("device node not found\n");
        return MDRV_ERROR;
    }
    
    g_amon_hw_cfg.base_addr = of_iomap(dev, 0);
    if (NULL == g_amon_hw_cfg.base_addr)
    {
        amon_error("remap soc base addr fail\n");
        return MDRV_ERROR;
    }

    g_amon_hw_cfg.irq_num = irq_of_parse_and_map(dev, 0);

    if(of_property_read_u32(dev, "amon_soc_cfg_base", &amon_cfg_addr))
    {
        amon_error("Get amon_soc_cfg_base fail\n");
        return MDRV_ERROR;
    }

    for(i = 0; i < AMON_SOC_DTS_BUTT; i++)
    {
        if(of_property_read_u32_array(dev, soc_reg_name[i], reg_offset, 3))
        {
            amon_error("Get amon dts fail, i = 0x%x\n", i);
            return MDRV_ERROR;
        }
        g_amon_hw_cfg.amon_sysctrl[i].base_addr = bsp_sysctrl_addr_get((void*)amon_cfg_addr) + reg_offset[AMON_BASE_ADDR];
        g_amon_hw_cfg.amon_sysctrl[i].start_bit = reg_offset[AMON_START_BIT];
        g_amon_hw_cfg.amon_sysctrl[i].end_bit = reg_offset[AMON_END_BIT];
    }

    return MDRV_OK;
}



__init s32 amon_soc_init(void)
{
    s32 ret = BSP_ERROR;
    if(g_amon_hw_cfg.init_flag == 1)
    {
        return BSP_OK;
    }
    
    ret = amon_soc_parse_dts();
    if(MDRV_ERROR == ret)
    {
        amon_error("parse dts fail\n");
        return MDRV_ERROR;
    }
    
    ret = amon_soc_get_clk_node();
    if(MDRV_ERROR == ret)
    {
        amon_error("amon_get_clk_node fail\n");
        return ret;
    }
    
    ret = amon_parse_port_cfg();
    if(MDRV_ERROR == ret)
    {
        amon_error("amon_get_clk_node fail\n");
        return ret;
    }
 
    g_amon_hw_cfg.init_flag = 1;

    amon_error("init ok\n", __FUNCTION__);

    return MDRV_OK;
}


AMON_CFG_STRU* amon_soc_get_cfg(void)
{
    return &g_amon_hw_cfg;
}
