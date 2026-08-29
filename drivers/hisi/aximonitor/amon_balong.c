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
#include "osl_irq.h"
#include "osl_thread.h"
#include "osl_malloc.h"
#include "osl_malloc.h"
#include "of.h"
#include "bsp_memmap.h"
#include "bsp_sysctrl.h"
#include "bsp_version.h"
#include "bsp_module.h"
#include "amon_balong.h"
#include "amon_soc.h"
#include "amon_cfg.h"
#include "amon_cnt.h"
#include "amon_capt.h"


__init s32 bsp_amon_soc_init(void)
{
    s32 ret = BSP_ERROR;
 
    ret = amon_soc_init();
    if(ret == BSP_ERROR)
    {
        amon_error("amon_soc_init\n");
    }
    return BSP_OK;
}

module_init(bsp_amon_soc_init);


s32 bsp_axi_mon_cnt_cfg(u32 id,u32 port,u32 master_id,u32 addr_enable,u32 start_addr,u32 end_addr,u32 opt_type,CNT_OPT cnt_opt)
{
    return amon_cnt_set_cfg(id,port,master_id,addr_enable,start_addr,end_addr,opt_type,cnt_opt);
}          
s32 bsp_axi_cnt_start(void)
{
    return amon_cnt_start();

}
s32 bsp_axi_cnt_stop(void)
{
    return amon_cnt_stop();

}
s32  bsp_axi_capt_cfg(u32 id, u32 port,u32 master_id,u32 opt_type)
{
    return amon_capt_cfg(id,port,master_id,opt_type);
} 
s32  bsp_axi_capt_start(void)
{
   return amon_capt_start();
}
s32  bsp_axi_capt_stop(void)
{
    return amon_capt_stop();
}
s32  bsp_axi_capt_export(void)
{
    return amon_capt_export();
}


