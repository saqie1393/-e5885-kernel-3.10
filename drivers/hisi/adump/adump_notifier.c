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
#include <osl_malloc.h>

#include "adump_notifier.h"
#include "adump_debug.h"

static LIST_HEAD(dump_notifiler_list);
static DEFINE_SPINLOCK(dump_notifiler_spinlock);

dump_handle bsp_adump_register_hook(char * name, dump_hook func)
{
    struct dump_notifier_info_s* pfieldhook = NULL;
    unsigned long flags = 0;

    pfieldhook = (struct dump_notifier_info_s*)osl_malloc(sizeof(struct dump_notifier_info_s));
    if(pfieldhook == NULL)
    {
        return ADUMP_ERR;
    }

    pfieldhook->pfunc = func;
    /* coverity[secure_coding] */
    memset(pfieldhook->name,'\0',sizeof(pfieldhook->name));
    memcpy(pfieldhook->name,name,strlen(name));

    spin_lock_irqsave(&dump_notifiler_spinlock, flags);
    list_add_tail(&pfieldhook->hook_list, &dump_notifiler_list);
    spin_unlock_irqrestore(&dump_notifiler_spinlock, flags);

    return (dump_handle)((unsigned long)pfieldhook);
}
EXPORT_SYMBOL(bsp_adump_register_hook);


s32 bsp_adump_unregister_hook(dump_handle handle)
{
    struct dump_notifier_info_s * pfieldhook= NULL;
    struct dump_notifier_info_s * hook_node = NULL;
    unsigned long flags = 0;

    if(handle == 0)
    {
        return ADUMP_ERR;
    }

    spin_lock_irqsave(&dump_notifiler_spinlock, flags);

    list_for_each_entry(pfieldhook, &dump_notifiler_list, hook_list)
    {
        if((dump_handle)((unsigned long)pfieldhook) == handle)
        {
            hook_node = pfieldhook;
        }
    }

    if(hook_node == NULL)
    {
        spin_unlock_irqrestore(&dump_notifiler_spinlock, flags);
        return ADUMP_ERR;
    }

    list_del(&hook_node->hook_list);
    osl_free(hook_node);
    spin_unlock_irqrestore(&dump_notifiler_spinlock, flags);

    return ADUMP_OK;
}
EXPORT_SYMBOL(bsp_adump_unregister_hook);


void adump_notify_call_chain(void)
{
    struct list_head *p,*n;
    struct dump_notifier_info_s* pfieldhook = NULL;

    list_for_each_safe(p,n, &dump_notifiler_list)
    {
        pfieldhook = list_entry(p,struct dump_notifier_info_s, hook_list);
        if(pfieldhook->pfunc)
        {
            pfieldhook->pfunc();
        }
    }
}

EXPORT_SYMBOL(adump_notify_call_chain);

void dump_show_notifier(void)
{
    struct list_head* cur = NULL;
    struct dump_notifier_info_s* pfieldhook = NULL;

    list_for_each(cur, &dump_notifiler_list)
    {
        pfieldhook = list_entry(cur, struct dump_notifier_info_s, hook_list);
        if(pfieldhook->pfunc)
        {
            (void)printk(KERN_ERR"name      :%s Function  :%pS\n", pfieldhook->name, pfieldhook->pfunc);
        }
    }
}
EXPORT_SYMBOL(dump_show_notifier);



