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
#include <osl_sem.h>
#include <osl_thread.h>
#include <linux/hisi/rdr_pub.h>
#include <bsp_slice.h>
#include <bsp_onoff.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <bsp_nvim.h>
#include <bsp_coresight.h>
#include <bsp_adump.h>

#include "adump_baseinfo.h"
#include "adump_field.h"
#include "adump_core.h"
#include "adump_notifier.h"
#include "adump_sche_record.h"
#include "adump_lastkmsg.h"
#include "adump_exception.h"
#include "adump_debug.h"


struct adump_core_ctrl_s            g_adump_core_ctrl;

/* RDR异常类型定义 */
struct rdr_exception_info_s g_adump_exc_info[] = {
    {
        .e_modid            = RDR_AP_DUMP_ARM_RESET_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_RESET_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc reset",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_UNDEF_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_UNDEF_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc undef",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_PREFETCH_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_PREFETCH_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc prefectch",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_DATA_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_DATA_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc databort",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_FIQ_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_FIQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc fiq",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_IRQ_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_IRQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc irq",
    },
    {
        .e_modid            = RDR_AP_DUMP_NORMAL_EXC_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_NORMAL_EXC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_SFTRESET,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap normal soft reset",
    },
    {
        .e_modid            = RDR_AP_DUMP_PANIC_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_PANIC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC_INT,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm painc in int",
    },
    {
        .e_modid            = RDR_AP_DUMP_AP_WDT_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_AP_WDT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_WDT_TIMEOUT,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP WDT",
        .e_desc             = "ap wdt exc",
    },
};

void adump_set_panic_flag(void)
{
    g_adump_core_ctrl.in_panic = true;
}
EXPORT_SYMBOL(adump_set_panic_flag);
void adump_save_usr_data(char *data, u32 length)
{
    s32 len = 0;
    dump_field_map_t* pfield = NULL;

    if ((data) && (length))
    {
        pfield = (dump_field_map_t*)bsp_adump_get_field_map(DUMP_KERNEL_USER_DATA);
        len = (length > DUMP_KERNEL_USER_DATA_SIZE) ? DUMP_KERNEL_USER_DATA_SIZE : length;

        if(g_adump_core_ctrl.user_data != NULL)
        {
             memcpy((void *)g_adump_core_ctrl.user_data, (const void * )data, (size_t)len); /* [false alarm]:屏蔽Fortify错误 */
        }

        if(pfield)
        {
            pfield->length = len;
        }
        adump_err("ok!\n");
    }
    else
    {
        adump_err("ok![no data to save]\n");
    }
    return;
}
EXPORT_SYMBOL(adump_save_usr_data);

__inline__ void adump_save_arm_regs(u32 addr)
{
    asm volatile(
        "str r0, [r0,#0x00]\n"
        "str r1, [r0,#0x04]\n"
        "str r2, [r0,#0x08]\n"
        "str r3, [r0,#0x0C]\n"
        "str r4, [r0,#0x10]\n"
        "str r5, [r0,#0x14]\n"
        "str r6, [r0,#0x18]\n"
        "str r7, [r0,#0x1C]\n"
        "str r8, [r0,#0x20]\n"
        "str r9, [r0,#0x24]\n"
        "str r10, [r0,#0x28]\n"
        "str r11, [r0,#0x2C]\n"
        "str r12, [r0,#0x30]\n"
        "str r14, [r0,#0x38]\n"
        "push {r1}\n"
        "str r13, [r0,#0x34]\n"
        "mov r1, pc\n"
        "str r1, [r0,#0x3C]\n"
        "mrs r1, cpsr\n"
        "str r1, [r0,#0x40]\n"
        "pop {r1}\n"
    );
}
void adump_save_exc_stack(u32 addr)
{
    struct task_struct *task = get_current();

    adump_save_arm_regs(addr);

    adump_fill_arm_register((u8*)addr);

    adump_fill_current_task((u32)task);

    if(g_adump_core_ctrl.exc_stack != NULL)
    {
        memcpy((void * )g_adump_core_ctrl.exc_stack , (const void * )task->stack, (size_t )THREAD_SIZE);
    }

    adump_err("ok !\n");
    return;
}

void ap_system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    u32 regSet[17] = {0,};

    if(mod_id > RDR_AP_DUMP_ARM_MOD_ID_END || mod_id < RDR_AP_DUMP_ARM_MOD_ID_START)
    {
        adump_err("AP mode id from [0x%x] to [0x%x],0x%x is not in this scope!\n",RDR_AP_DUMP_ARM_MOD_ID_START,\
            RDR_AP_DUMP_ARM_MOD_ID_END,mod_id);
        return;
    }
    if(g_adump_core_ctrl.exc_flag == true)
    {
        adump_err("exception has happened can not deal new exception\n");
        return;
    }
    g_adump_core_ctrl.exc_flag = true;

    adump_err("[0x%x]================ acore enter system error! ================\n", bsp_get_slice_value());
    adump_err("mod_id=0x%x arg1=0x%x arg2=0x%x data=0x%p len=0x%x\n", mod_id, arg1, arg2, data, length);

    if (DUMP_INIT_FLAG != g_adump_core_ctrl.init_flag)
    {
        adump_err("dump not init,exit system_error process\n");
        return;
    }

    adump_save_exc_stack((uintptr_t)regSet);

    bsp_coresight_disable();

    adump_stop_sche_record();

    adump_fill_base_info(mod_id,arg1,arg2,data,length);

    adump_save_usr_data(data, length);

    if(g_adump_core_ctrl.in_panic)
    {
        adump_err("kernel enter panic !\n");
        rdr_fetal_system_error(RDR_AP_DUMP_PANIC_MOD_ID, arg1, arg2);
        return;
    }

    rdr_system_error(mod_id, arg1, arg2);

    return;
}
EXPORT_SYMBOL(ap_system_error);

void adump_callback( u32 modid, u32 etype, u64 coreid,char* logpath, pfn_cb_dump_done fndone)
{
    adump_err("enter dump callback, mod id:0x%x\n", modid);

    adump_save_all_tasktcb();

    adump_stop_task_name_timer();

    adump_save_task_name();

    adump_save_self_addr();

    adump_notify_call_chain();

    adump_save_lastkmsg();

    if(fndone){
        adump_err("dump done!\n");
        fndone(modid,coreid);
    }
    return ;
}
EXPORT_SYMBOL(adump_callback);


static void adump_reset(u32 modid, u32 etype, u64 coreid)
{
    s32 ret;
    NV_DUMP_STRU       dump_cfg;

    adump_err("enter dump reset, mod id:0x%x\n", modid);

    ret = bsp_nvm_read(NV_ID_DRV_DUMP, (u8*)&dump_cfg, sizeof(NV_DUMP_STRU));
    if(ret)
    {
        adump_err("read nv error,not reset");
        return;
    }

    if(dump_cfg.dump_cfg.Bits.sysErrReboot)
    {
        adump_err("enter drv reboot process\n");
        adump_save_lastkmsg();
        bsp_drv_power_reboot_direct();
    }

}

/*提供给其它子系统用于保存当前A核异常现场*/
void bsp_adump_save_exc_scene(u32 mod_id, u32 arg1, u32 arg2)
{
    u32 regSet[17];

    if(g_adump_core_ctrl.exc_flag)
        return;

    bsp_coresight_disable();
    adump_stop_sche_record();

    adump_save_exc_stack((uintptr_t)regSet);
    adump_fill_base_info(mod_id,0,0,NULL,0);
}
EXPORT_SYMBOL(bsp_adump_save_exc_scene);

s32 adump_core_init(void)
{
    struct rdr_module_ops_pub   soc_ops;
    struct rdr_register_module_result soc_rst;
    int i = 0;

    g_adump_core_ctrl.exc_stack = bsp_adump_register_field(DUMP_KERNEL_TASK_STACK,  "TASK_STACK",  NULL,NULL,DUMP_KERNEL_TASK_STACK_SIZE,0);
    if(g_adump_core_ctrl.exc_stack)
    {
        adump_err("alloc exc task statck buffer success\n");
    }

    g_adump_core_ctrl.user_data = bsp_adump_register_field(DUMP_KERNEL_USER_DATA,   "USER_DATA", NULL,NULL,DUMP_KERNEL_USER_DATA_SIZE,0);
    if(g_adump_core_ctrl.user_data)
    {
        adump_err("alloc user data buffer success\n");
    }
    /* coverity[secure_coding] */
    memset(&soc_ops,0,sizeof(soc_ops));
    /* coverity[secure_coding] */
    memset(&soc_rst,0,sizeof(soc_rst));
    /*register rdr exception*/
    for(i=0; i<sizeof(g_adump_exc_info)/sizeof(struct rdr_exception_info_s); i++)
    {
        (void)rdr_register_exception(&g_adump_exc_info[i]);
    }

    /*register rdr moudule ops*/
    soc_ops.ops_dump  = (pfn_dump)adump_callback;
    soc_ops.ops_reset = (pfn_reset)adump_reset;

    (void)rdr_register_module_ops(RDR_AP, &soc_ops, &soc_rst);

    g_adump_core_ctrl.init_flag = DUMP_INIT_FLAG;

    adump_err("ok!\n");
    return ADUMP_OK;
}

s32 __init adump_init(void)
{
    /*dump base init*/
    (void)adump_baseinfo_init();

    /*dump sche record init*/
    (void)adump_sche_record_init();

    /*dump lastkmsg init*/
    (void)adump_lastkmsg_init();

    /*dump core init*/
    (void)adump_core_init();

    /*dump exception init*/
    (void)adump_exception_init();

    adump_err("ok!\n");
    return ADUMP_OK;
}
core_initcall_sync(adump_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Balong Drv_I/Msp Team");
MODULE_DESCRIPTION("Kernel mntn mananger Module");

