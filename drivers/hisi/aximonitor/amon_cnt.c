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

#include "osl_bio.h"
#include "osl_irq.h"
#include "osl_common.h"
#include "drv_nv_id.h"
#include "drv_nv_def.h"
#include <of.h>
#include "bsp_memmap.h"
#include "bsp_version.h"
#include "bsp_sysctrl.h"
#include "bsp_adump.h"
#include "bsp_nvim.h"
#include "bsp_dump.h"
#include <bsp_shared_ddr.h>
#include <bsp_slice.h>
#include <bsp_module.h>
#include "amon_balong.h"
#include "amon_soc.h"
#include "amon_cfg.h"



u32   g_bus_stress_time[AMON_BUS_STRESS_TIME_BUTT];
DRV_AMON_CNT_CONFIG_STRU g_amon_cnt_config = {};
amon_stat_t g_amon_stat;
amon_buff_info_t  g_amon_buff;


void amon_debug_reset(void)
{
    int i;

    for(i=0; i<AXI_MAX_CONFIG_ID; i++)
    {
        g_amon_stat.soc_rd_cnt[i] = 0;
        g_amon_stat.soc_wr_cnt[i] = 0;

    }
}

void amon_debug_show(void)
{
    u32 i = 0;
    u32 reg_value_low = 0;
    u32 reg_value_high = 0;

    amon_soc_clk_enable();
    amon_error("************SOC STAT************\n");
    for(i=0; i<AXI_MAX_CONFIG_ID; i++)
    {
        AXI_REG_READ(  AXI_MON_RD_BYTES_ID_LOW(i), &reg_value_low);
        AXI_REG_READ(  AXI_MON_RD_BYTES_ID_HIGH(i), &reg_value_high);
        amon_error("=======ID %d statistics=======\n", i);
        amon_error("rd int cnt       : 0x%x\n", g_amon_stat.soc_rd_cnt[i]);
        amon_error("wr int cnt       : 0x%x\n", g_amon_stat.soc_wr_cnt[i]);
        amon_error("rd total bytes(h): 0x%x\n", reg_value_high);
        amon_error("rd total bytes(l): 0x%x\n", reg_value_low);
        AXI_REG_READ(  AXI_MON_WR_BYTES_ID_LOW(i), &reg_value_low);
        AXI_REG_READ(  AXI_MON_WR_BYTES_ID_HIGH(i), &reg_value_high);
        amon_error("wr total bytes(h): 0x%x\n", reg_value_high);
        amon_error("wr total bytes(l): 0x%x\n", reg_value_low);
    }
    amon_soc_clk_disable();
}



void amon_cnt_set_port_cfg(u32 id ,u32 port,u32 master_id,u32 addr_enable,u32 start_addr,u32 end_addr,u32 opt_type)
{
    u32 reg_value = 0xffff;
    u32 id_cfg = 0;

    if(BSP_ERROR == amon_get_cnt_cfg(port,master_id,&id_cfg))
    {
        amon_error("get master cfg fail\n");
        return;
    }

    AXI_REG_SETBITS(AXI_MON_PORT_SEL, id*3, 3,port&0x7);
    AXI_REG_WRITE(AXI_MON_CNT_ID(id), id_cfg);

    if(addr_enable == 1)
    {
        AXI_REG_SETBITS(AXI_MON_CNT_ID(id), 30, 1,1);
        AXI_REG_WRITE(AXI_MON_ID_ADDR_DES(id), start_addr);
        AXI_REG_WRITE(AXI_MON_ID_ADDR_DES_M(id), end_addr);
    }
    else
    {
        AXI_REG_SETBITS(AXI_MON_CNT_ID(id), 30, 1,0);
    }

    /* ID i监控读操作 */
    if(opt_type & AMON_OPT_READ)
    {
        reg_value = reg_value & (~(u32)(1<<(id*2+1)));
    }
    /* ID i监控写操作 */
    if(opt_type & AMON_OPT_WRITE)
    {
        reg_value = reg_value & (~(u32)(1<<(id*2)));
    }
    AXI_REG_WRITE(AXI_ID_INT_MASK, reg_value);

}



void amon_cnt_cfg_all_ids(void)
{
    u32 i = 0;
 
    for(i=0; i<AXI_MAX_CONFIG_ID; i++)
    {
        if(g_amon_cnt_config.config[i].id_enable == 1)
        {

            amon_cnt_set_port_cfg(i,g_amon_cnt_config.config[i].port,
                                    g_amon_cnt_config.config[i].master_id,
                                    g_amon_cnt_config.config[i].addr_enable,
                                    g_amon_cnt_config.config[i].start_addr,
                                    g_amon_cnt_config.config[i].end_addr,
                                    g_amon_cnt_config.config[i].cnt_type);
        }
    }
}

s32 amon_cnt_record_init(void)
{
    g_amon_buff.buff = (char *)bsp_adump_register_field(DUMP_KERNEL_AMON, "AMON_SOC", 0, 0, 1024, (AMON_MAIN_VERSION << 8) + AMON_MINOR_VERSION);
    if(BSP_NULL == g_amon_buff.buff)
    {
        amon_error("get buffer fail\n");
        return MDRV_ERROR;
    }
    g_amon_buff.buff_size = 1024;

    g_amon_buff.write_offset = 0;
    /* coverity[HUAWEI DEFECT] */
    (void)memset(g_amon_buff.buff,0, g_amon_buff.buff_size);
    return MDRV_OK;
}

void amon_cnt_save_log(u32 id, u32 opt_type, AMON_CNT_CONFIG_T * config)
{

    u32 * data_wr = (u32 *)(g_amon_buff.buff + g_amon_buff.write_offset);

    if(g_amon_buff.buff == NULL)
    {
        return;
    }

    if(!config)
    {
         amon_error("config info is NULL\n");
         return;
    }
    if( g_amon_buff.buff_size - g_amon_buff.write_offset < 0x20)
    {
        data_wr = (u32*)g_amon_buff.buff;
        g_amon_buff.write_offset = 0;
    }

    *data_wr     = bsp_get_slice_value();
    *(data_wr+1) = id;
    *(data_wr+2) = config->port;
    *(data_wr+3) = config->master_id;
    *(data_wr+4) = opt_type;
    *(data_wr+5) = config->start_addr;
    *(data_wr+6) = config->end_addr;
    g_amon_buff.write_offset = (g_amon_buff.write_offset + 0x20)%(g_amon_buff.buff_size);
}


void amon_cnt_handler_port_int(u32 id,AMON_CNT_CONFIG_T* amon_cnt_cfg ,u32 reg_value)
{
    if(amon_cnt_cfg == NULL)
    {
        return;
    }
 
    /* 写中断 */
    if(reg_value & (1<<(2 * id)))
    {
        g_amon_stat.soc_wr_cnt[id]++;
        amon_cnt_save_log(id, AMON_OPT_WRITE, amon_cnt_cfg);

        switch(amon_cnt_cfg->cnt_opt)
        {
        case CNT_BOOT:
            amon_error("soc id 0x%x wr hit, reboot\n", id);
            AXI_REG_WRITE(AXI_ID_INT_MASK, 0xffff);
            system_error(DRV_ERRNO_AMON_SOC_WR, amon_cnt_cfg->port,  amon_cnt_cfg->master_id, 0, 0);
            break;
        case CNT_ONCE:
            AXI_REG_SETBITS(AXI_ID_INT_MASK, (u32)id*2, 1, 1);
            break;
        case CNT_ON:
            AXI_REG_SETBITS(AXI_ID_INT_MASK, (u32)id*2, 1, 0);
            break;
        default:
            break;
        }

    }
    /* 读中断 */
    if(reg_value & (1<<(id*2+1)))
    {
        g_amon_stat.soc_rd_cnt[id]++;
        amon_cnt_save_log(id, AMON_OPT_READ,amon_cnt_cfg);
        AXI_REG_SETBITS(AXI_ID_INT_MASK, (u32)id*2+1, 1, 0);

        switch(amon_cnt_cfg->cnt_opt)
        {
        case CNT_BOOT:
            amon_error("cpufast id 0x%x rd hit, reboot\n",id);
            AXI_REG_WRITE( AXI_ID_INT_MASK, 0xffff);
            system_error(DRV_ERRNO_AMON_SOC_RD, (u32)id, 3, 0, 0);
            return;
        case CNT_ONCE:
            AXI_REG_SETBITS(AXI_ID_INT_MASK, (u32)id*2+1, 1, 1);
            break;
        case CNT_ON:
            AXI_REG_SETBITS(AXI_ID_INT_MASK, (u32)id*2+1, 1, 0);
            break;
        default:
            break;
        }

    }

}


irqreturn_t amon_cnt_int_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    u32 i =0;
    u32 reg_value = 0;
    u32 mask = 0;
    amon_soc_clk_enable();

    AXI_REG_READ(AXI_ID_INT_STAT, &reg_value);
    AXI_REG_READ(AXI_ID_INT_MASK, &mask);
    AXI_REG_WRITE(AXI_ID_INT_MASK, (mask | reg_value));
    AXI_REG_WRITE(AXI_MON_INT_CLR, reg_value<<16);
    
    for(i=0; i< AXI_MAX_CONFIG_ID; i++)
    {
        amon_cnt_handler_port_int(i,&(g_amon_cnt_config.config[i]),reg_value);
    }
    return IRQ_HANDLED;
}



s32 amon_cnt_start(void)
{
    AMON_CFG_STRU* amon_hw_cfg = amon_soc_get_cfg();

    amon_soc_soft_reset();
    
    amon_soc_rls_reset(); 
    
    if(amon_soc_self_reset())
    {
        amon_error("soft reset error \n");
        return BSP_ERROR;
    }

    AMON_SOC_CHECK_STATE();

    AXI_REG_WRITE(AXI_MON_INT_CLR, 0xFFFFFFFF);

    if(g_amon_cnt_config.en_flag == 0)
    {
        enable_irq(amon_hw_cfg->irq_num);
    }

    if(amon_soc_start())
    {
        amon_error("amon_soc_start error \n");
        return BSP_ERROR;
    }
    
    g_bus_stress_time[AMON_BUS_STRESS_START_TIME] = bsp_get_slice_value_hrt();

    return MDRV_OK;
}


s32 amon_cnt_stop(void)
{
    AMON_CFG_STRU* amon_hw_cfg = amon_soc_get_cfg();

    AXI_REG_WRITE(AXI_ID_INT_MASK, 0xFFFFF);

    if(g_amon_cnt_config.en_flag == 0)
    {
        disable_irq(amon_hw_cfg->irq_num);
    }
    
    if(BSP_ERROR == amon_soc_stop() )
    {    
        amon_error("amon_soc_start error \n");
        return BSP_ERROR;
    }
    g_bus_stress_time[AMON_BUS_STRESS_END_TIME]  = bsp_get_slice_value_hrt();
    
    return BSP_ERROR;
}

s32 amon_cnt_get_info(amon_count_stru *count)
{
    u32 i = 0;
    u32 timeDelta = 0;
    u32 startTime = 0;
    u32 endTime = 0;
    u32 temp = 0;
    u32 timeFreq = 0;

    if(amon_cnt_stop())
    {
        amon_error("amon stop fail\n");
        return MDRV_ERROR;
    }

    for(i = 0; i < AXI_MAX_CONFIG_ID; i++)
    {
        AXI_REG_READ(AXI_MON_RD_BYTES_ID_LOW(i), &(count->read_count[i].low_count));
        AXI_REG_READ(AXI_MON_RD_BYTES_ID_HIGH(i),&(count->read_count[i].hig_count));
        AXI_REG_READ(AXI_MON_WR_BYTES_ID_LOW(i), &(count->write_count[i].low_count));
        AXI_REG_READ(AXI_MON_WR_BYTES_ID_HIGH(i),&(count->write_count[i].hig_count));
    }
    /*计算从上一次到本次之间该master 口可容纳的访问的总量*/
    startTime = g_bus_stress_time[AMON_BUS_STRESS_START_TIME];
    endTime = g_bus_stress_time[AMON_BUS_STRESS_END_TIME];
    timeDelta = endTime > startTime ? (endTime - startTime):((0xFFFFFFFF - startTime) + endTime);
    temp = (u32)((u32)timeDelta * (u32)MODEM_PORT_WITH);
    timeFreq = (u64)STRESS_TIME_FREQ;
    count->global_access_count = temp/timeFreq;
    count->monitor_time_delta = timeDelta;
    amon_error("timeDelta = 0x%x\n", timeDelta);
    
    if(MDRV_OK != amon_cnt_start())
    {
        amon_error("soc start fail\n");
        return MDRV_ERROR;
    }
    return MDRV_OK;

}


__init s32 amon_cnt_init(void)
{   
    AMON_CFG_STRU* amon_hw_cfg = amon_soc_get_cfg();

    if(amon_hw_cfg == NULL)
    {
        amon_error("amon_hw_cfg is NULL\n");
        return BSP_ERROR;
    }
    
    if(amon_hw_cfg->init_flag == 0)
    {
        if(amon_soc_init() == BSP_ERROR)
        {
            amon_error("amon_soc_init fail\n");
            return BSP_ERROR;
        }
    }
    
    amon_cnt_record_init();

    if(MDRV_OK != request_irq(amon_hw_cfg->irq_num, (irq_handler_t)amon_cnt_int_handler, IRQF_DISABLED, "AXI_MON_IRQ", NULL))
    {
        amon_error("INT connect fail\n");
        return MDRV_ERROR;
    }
    if(MDRV_OK != bsp_nvm_read(NV_ID_DRV_AMON_SOC, (u8*)&g_amon_cnt_config, sizeof(DRV_AMON_CNT_CONFIG_STRU)))
    {
        amon_error("read nv 0x%x fail\n", NV_ID_DRV_AMON_SOC);
        return MDRV_ERROR;
    }

    if(g_amon_cnt_config.en_flag == 0)
    {
        disable_irq(amon_hw_cfg->irq_num);
        amon_error("amon_soc cnt init ok\n");
        return MDRV_OK;
    }

    if(g_amon_cnt_config.en_flag)
    {
        amon_soc_clk_enable();

        amon_soc_soft_reset();
        
        amon_soc_rls_reset();
        
        if(MDRV_OK != amon_soc_self_reset())
        {
            amon_error("soc reset fail\n");
            return MDRV_ERROR;
        }
        
        AXI_REG_WRITE(AXI_ID_INT_MASK, 0xFFFFFFF0);
        AXI_REG_WRITE(AXI_ID_INT_MASK, 0xFFFF);
        AXI_REG_SETBITS(AXI_MON_CNT_CTRL, 2, 2, 0x3);
        AXI_REG_WRITE(AXI_CAPT_INT_MASK, 0xF);
        amon_cnt_cfg_all_ids();

        if(g_amon_cnt_config.en_flag)
        {
            if(MDRV_OK != amon_cnt_start())
            {
                amon_error("soc start fail\n");
                return MDRV_ERROR;
            }
        }
    }
    amon_error("amon_soc cnt init ok\n");
    return MDRV_OK;
}

module_init(amon_cnt_init);


s32 amon_cnt_set_cfg(u32 id,u32 port,u32 master_id,u32 addr_enable,u32 start_addr,u32 end_addr,u32 opt_type,CNT_OPT cnt_opt)
{
    if(id >= AXI_MAX_CONFIG_ID || port >= AXI_MAX_PORT_CNT)
    {
        amon_error("input param error\n");
        return BSP_ERROR;
    }
    
    amon_soc_clk_enable();
  
    if(BSP_ERROR == amon_soc_stop())
    {
        amon_error("amon stop fail id = 0x%x\n", AXI_SOC_CONFIG);
        return MDRV_ERROR;
    }
    if(id >= AXI_MAX_CONFIG_ID)
    {
        amon_error("id too big\n");
        return MDRV_ERROR;
    }
    if(port >= AXI_MAX_PORT_CNT)
    {
        amon_error("port too big\n");
        return MDRV_ERROR;
    }
    amon_soc_soft_reset();
    amon_soc_rls_reset();
    
    if(MDRV_OK != amon_soc_self_reset())
    {
        amon_error("cpufast reset fail\n");
        return MDRV_ERROR;
    }
    
    AXI_REG_WRITE(AXI_ID_INT_MASK, 0xFFFF); 
    AXI_REG_SETBITS(AXI_MON_CNT_CTRL, 2, 2, 0x3);
    AXI_REG_WRITE(AXI_CAPT_INT_MASK, 0xF);

    
    g_amon_cnt_config.config[id].id_enable = 1;
    g_amon_cnt_config.config[id].port = port;
    g_amon_cnt_config.config[id].master_id = master_id;
    g_amon_cnt_config.config[id].addr_enable= addr_enable;
    g_amon_cnt_config.config[id].start_addr = start_addr;
    g_amon_cnt_config.config[id].end_addr = end_addr;
    g_amon_cnt_config.config[id].cnt_type= opt_type;
    g_amon_cnt_config.config[id].cnt_opt= cnt_opt;
    
    amon_cnt_set_port_cfg(id,port,master_id,addr_enable,start_addr, end_addr,opt_type);

    return BSP_OK;
}
