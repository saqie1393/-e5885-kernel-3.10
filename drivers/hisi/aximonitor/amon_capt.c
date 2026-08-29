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
#include "mdrv_errno.h"
#include "bsp_dump.h"
#include "bsp_amon.h"
#include "amon_balong.h"
#include "amon_cfg.h"
#include "amon_capt.h"
#include "amon_soc.h"

AMON_CAPT_CFG g_amon_capt_cfg = {};



s32 amon_capt_start(void)
{
    u32 cfg_id = 0;
    amon_soc_clk_enable();

    amon_soc_soft_reset();
    amon_soc_rls_reset(); 

    if(amon_soc_self_reset())
    {
        amon_error("soft reset error\n");
    }
    /* 检查运行状态 */

    if(amon_get_capt_cfg(g_amon_capt_cfg.port, g_amon_capt_cfg.master_id,&cfg_id) == BSP_ERROR)
    {
        amon_error("amon_get_capt_cfg fail\n");
        return BSP_ERROR;
    }

    /*配置循环数采模式*/
    AXI_REG_SETBITS(AXI_CAPT_TRAN_CONFIG, 8, 1, 1) ;

    AXI_REG_SETBITS(AXI_CAPT_TRAN_CONFIG, 0, 3, g_amon_capt_cfg.port) ;
    AXI_REG_SETBITS(AXI_CAPT_TRAN_CONFIG, 0, 1, g_amon_capt_cfg.opt_type) ;
    
    AXI_REG_WRITE(AXI_CAPT_ID(g_amon_capt_cfg.id), cfg_id);
    AXI_REG_SETBITS(AXI_CAPT_ID_EN, g_amon_capt_cfg.id, 1, 1) ;

    /* 清中断 */
    AXI_REG_WRITE(AXI_CAPT_INT_CLR, 0xFFFFFFFF);
    AXI_REG_WRITE(AXI_CAPT_INT_MASK, 0xFFFFFFFF);
    AXI_REG_SETBITS(AXI_CAPT_CTRL, 0, 2, 0x1) ;

    return BSP_OK;

}



s32 amon_capt_stop(void)
{
    AXI_REG_SETBITS(AXI_CAPT_CTRL, 0, 2, 0x2) ;

    if(BSP_ERROR == amon_soc_stop())
    {
        return MDRV_ERROR;
    }
    return MDRV_OK;

}

s32 amon_capt_export(void)
{
    u32 value = 0;
    u32 i = 0;

    amon_soc_clk_enable();
    amon_capt_stop();
    for(;;)
    {
        AXI_REG_GETBITS(AXI_CAPT_INT_SRC, 3, 1, &value);
        if(value != 1)
        {
            if(i < 82*1024)
            {
                AXI_REG_READ(AXI_CAPT_FIFO_ADDR, &value);
                //memcpy(g_buf+i*4,&value,4);
                i++;
            }
        }
        else
        {
            break;
        }
    }
    return BSP_OK;

}

s32 amon_capt_cfg (u32 id, u32 port,u32 master_id,u32 opt_type)
{
    if(id >= AXI_MAX_CONFIG_ID || port >= AXI_MAX_PORT_CNT || opt_type > 1)
    {
        amon_error("input param error\n");
        return BSP_ERROR;
    }
    amon_soc_clk_enable();
    g_amon_capt_cfg.id = id;
    g_amon_capt_cfg.port = port;
    g_amon_capt_cfg.master_id = master_id;
    g_amon_capt_cfg.opt_type = opt_type;

    return BSP_OK;
}
