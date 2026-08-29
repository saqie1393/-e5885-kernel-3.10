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

#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include <linux/io.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include "bsp_wp.h"

enum
{
    WP_SET_MONITOR          =0,
    WP_DISABLE_MONITOR,
    WP_ENABLE_MONITOR,
    WP_CLEAR_MONITOR,
    WP_MODIFY_MONITOR,
    WP_QUERY_MONITOR,
};


typedef struct
{
    unsigned int id;
    unsigned int choice;
    unsigned int magic;
    void*        addr;
    unsigned int len;
    unsigned int way;
    unsigned int type;
    unsigned int enable;
    unsigned int scope;
}wbp_cfg_t;

struct wp_struct {
    bool                isSet;
    bool                isEnable;
	void                *wp_addr;
	unsigned int        wp_len;
	unsigned int        wp_type;
    unsigned int        wp_way;
    unsigned int        wp_magic;
    unsigned int        wp_scope;
	struct perf_event *__percpu *wp_pe;
    wp_event_callback   event_cb;
};

extern struct wp_struct g_wbp_table[ARM_MAX_WRP];

#define wp_err(fmt,...)        printk(KERN_ERR"[watchpoint]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)

#define wp_check_index(idx)\
    do{\
        if (idx >= ARM_MAX_WRP)\
        {\
            wp_err("index is large than arm max value %d!\n",idx);\
            return -1;\
        }\
    }while(0)

#define wp_check_set_flag(idx)\
    do{\
        if (!g_wbp_table[idx].isSet)\
        {\
            wp_err("this watchpoint is not set %d!\n",idx);\
            return -1;\
        }\
    }while(0)

#define wp_check_pe_param(idx)\
    do{\
        if (IS_ERR((void __force *)g_wbp_table[idx].wp_pe))\
        {\
            wp_err("this watchpoint perf event is err %d!\n",idx);\
            return -1;\
        }\
    }while(0)


#endif


