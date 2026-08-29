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


#include <product_config.h>
#include <osl_types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <asm/memory.h>
#include <bsp_version.h>
#include <bsp_om.h>
#include <bsp_om_enum.h>
#include <bsp_dump.h>
#include "adump_baseinfo.h"
#include "adump_exception.h"
#include "adump_debug.h"

adump_base_info_s        *g_dump_base_info;

void adump_fill_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{

    g_dump_base_info->modId = mod_id;
    g_dump_base_info->arg1 = arg1;
    g_dump_base_info->arg2 = arg2;
    g_dump_base_info->arg3 = (u32)data;
    g_dump_base_info->arg3_length = length;

    if(in_interrupt())
    {
        g_dump_base_info->reboot_task = (u32)(-1);
        /* coverity[secure_coding] */
        memset(g_dump_base_info->taskName, 0, sizeof(g_dump_base_info->taskName));
        g_dump_base_info->reboot_int = g_dump_base_info->current_int;
        g_dump_base_info->reboot_context = DUMP_CTX_INT;
    }
    else
    {
        g_dump_base_info->reboot_task = g_dump_base_info->current_task;
        if(g_dump_base_info->reboot_task)
        {
            /* coverity[buffer_size_warning] */
            strncpy((char*)g_dump_base_info->taskName,((struct task_struct *)(g_dump_base_info->reboot_task))->comm,16);
        }
        g_dump_base_info->reboot_int = (u32)(-1);
        g_dump_base_info->reboot_context = DUMP_CTX_TASK;
    }
    adump_err("ok!\n");
    return;
}

void adump_fill_current_task(u32 current_task)
{
    if(g_dump_base_info)
    {
        g_dump_base_info->current_task = current_task;
    }
}

void adump_fill_current_int(u32 current_int)
{
    if(g_dump_base_info)
    {
        g_dump_base_info->current_int = current_int;
    }
}

void adump_fill_exc_vec(u32 vec)
{
    if(g_dump_base_info)
    {
        g_dump_base_info->vec= vec;
    }
}

void adump_fill_arm_register(u8* regs)
{
    if(g_dump_base_info)
    {
        memcpy(g_dump_base_info->regSet,regs,sizeof(g_dump_base_info->regSet));
    }
}


s32 adump_baseinfo_init(void)
{
    if(g_dump_base_info)
    {
        adump_err("dump base info has already init ,do not init twice!\n");
        return ADUMP_OK;
    }

    g_dump_base_info = (adump_base_info_s*)bsp_adump_register_field(DUMP_KERNEL_BASE_INFO, "BASE_INFO", NULL,NULL,DUMP_KERNEL_BASE_INFO_SIZE,0);
    if(NULL == g_dump_base_info)
    {
        adump_err("register base info err!\n");
        return ADUMP_ERR;
    }
    /* coverity[secure_coding] */
    memset(g_dump_base_info, 0, DUMP_KERNEL_BASE_INFO_SIZE);

    g_dump_base_info->vec = DUMP_ARM_VEC_UNKNOW;

    /*coverity[buffer_size_warning] */
    (void)strncpy((char*)g_dump_base_info->version,(char*)bsp_version_get_firmware(),sizeof(g_dump_base_info->version));
    /*coverity[buffer_size_warning] */
    (void)strncpy((char*)g_dump_base_info->compile_time,(char*)bsp_version_get_build_date_time(),sizeof(g_dump_base_info->compile_time));

    adump_err("ok!\n");
    return ADUMP_OK;
}

EXPORT_SYMBOL(adump_baseinfo_init);




