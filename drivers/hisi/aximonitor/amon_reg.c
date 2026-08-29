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
#include "osl_bio.h"
#include "amon_balong.h"
#include "amon_soc.h"


void axi_reg_read(u32 reg, u32 * value)
{
    AMON_CFG_STRU* hw_cfg =  amon_soc_get_cfg();
    if(hw_cfg == NULL || hw_cfg->base_addr == NULL)
    {
        return;
    }

    *value = readl(hw_cfg->base_addr+ reg);
}


void axi_reg_write( u32 reg, u32 value)
{
    AMON_CFG_STRU* hw_cfg =  amon_soc_get_cfg();
    if(hw_cfg == NULL || hw_cfg->base_addr == NULL)
    {
        return;
    }
    writel(value, hw_cfg->base_addr + reg);
}


void axi_reg_getbits(u32 reg, u32 pos, u32 bits, u32 * value)
{
    u32 reg_value = 0;

    axi_reg_read(reg, &reg_value);
    *value = (reg_value >> pos) & (((u32)1 << (bits)) - 1);
}


void axi_reg_setbits(u32 reg, u32 pos, u32 bits, u32 value)
{
    u32 reg_value = 0;

    axi_reg_read(reg, &reg_value);
    reg_value = (reg_value & (~((((u32)1 << (bits)) - 1) << (pos)))) | ((u32)((value) & (((u32)1 << (bits)) - 1)) << (pos));
    axi_reg_write(reg, reg_value);
}



void amon_set_reg(u32 value,void* base,u32 offset,u32 lowbit,u32 highbit)
{
    void* reg    = base + offset;
    unsigned int temp   = 0;
    unsigned int mask   = 0;

    temp   = readl(reg);
    mask   = ((1 << (highbit - lowbit + 1)) - 1) << lowbit;
    value  = (temp & (~mask)) | ((value <<lowbit) & mask);
    writel(value  ,reg);

}

u32 amon_get_reg(void* base,u32 offset,u32 lowbit,u32 highbit)
{
    unsigned int mask = 0;
    unsigned int temp = 0;
    void*  reg  = base + offset;
    u32 value = 0;

    temp   = readl(reg);
    mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
    value = temp & mask;
    value = (value) >> lowbit;

    return value;
}

