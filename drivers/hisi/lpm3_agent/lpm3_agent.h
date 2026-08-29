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

#ifndef __LPM3_AGENT_H__
#define __LPM3_AGENT_H__


#include <product_config.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <linux/hisi/rdr_pub.h>
#include "bsp_ipc.h"
#include "mdrv_ipc_enum.h"
#include "bsp_slice.h"
#include "drv_comm.h"
#include "bsp_coresight.h"
#include "bsp_wdt.h"
#include "mntn_interface.h"
#include "bsp_dump_mem.h"

#define DUMP_SAVE_SUCCESS                   0xA4A4A4A4
#define HISI_MNTN_EXC_LPM3_MOD_ID           (0x85000001)
#define HISI_MNTN_EXC_LPM3_WDT_MOD_ID       (0x85000002)
#define LPM3_WDT_EXC                        (0xA4B5C6D7)

typedef enum _m3_dump_reboot_reason_e
{
    LPM3_EXC        = 0x1,
    AP_WDT_EXC      = 0x2,
    TEEOS_WDT_EXC   = 0x3,
}m3_dump_reboot_reason_e;


struct lpm3_agent_ctrl_info
{
    bool        ulInitstate;
    osl_sem_id task_sem;
    OSL_TASK_ID task_id;

    u32     phy_addr;
    u32     length;
    void*   virt_addr;

    bool    is_lpm3exc;
};


struct lpm3_rdr_exc_info
{
    u32     modid;
    u64     coreid;
    pfn_cb_dump_done    dump_done;
};

#define lpm3_err(fmt,...)        printk(KERN_ERR"[lpm3 agent]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define lpm3_fetal(fmt,...)      printk(KERN_ALERT"[lpm3 agent]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)

#endif

