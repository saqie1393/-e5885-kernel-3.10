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


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include <linux/io.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include "bsp_wp.h"
#include "watchpoint.h"


struct wp_struct g_wbp_table[ARM_MAX_WRP];

u32 current_regs[18];

static void enable_single_step(struct perf_event *bp,u32 addr,u32 mismatch)
{
    arch_uninstall_hw_breakpoint(bp);
	bp->hw.info.step_ctrl.mismatch  = mismatch;
	bp->hw.info.step_ctrl.len	    = ARM_BREAKPOINT_LEN_4;
	bp->hw.info.step_ctrl.type	    = ARM_BREAKPOINT_EXECUTE;
	bp->hw.info.step_ctrl.privilege     = bp->hw.info.ctrl.privilege;
	bp->hw.info.step_ctrl.enabled	    = 1;
	bp->hw.info.trigger		            = addr;
    arch_install_hw_breakpoint(bp);
}

static void breakpoint_proc(struct perf_event *bp,
			   struct perf_sample_data *data, struct pt_regs *regs)
{
    struct arch_hw_breakpoint *info = counter_arch_bp(bp);
	int i;
	struct perf_event **pevent = NULL;
    u32 trigger;
    u32 mismatch = 0;

    for(i=0;i<ARM_MAX_WRP;i++){
        if(!g_wbp_table[i].isSet){
            continue;
        }
		pevent = this_cpu_ptr(g_wbp_table[i].wp_pe);
        if(bp == *pevent){
            break;
        }
        pevent = NULL;
    }

    trigger = info->trigger;

    if((pevent) && (g_wbp_table[i].wp_way & WP_EVENT_INFO_HANDLER))
    {
        printk(KERN_ERR"==============================================\n");
        printk(KERN_ERR"PC is at %pS\n",(void*)regs->ARM_pc);
        printk(KERN_ERR"LR is at %pS\n",(void*)regs->ARM_lr);

    	printk(KERN_ERR"pc : [<%08lx>]    lr : [<%08lx>]    psr: %08lx\n", \
            regs->ARM_pc, regs->ARM_lr, regs->ARM_cpsr);
        printk(KERN_ERR"sp : %08lx  ip : %08lx  fp : %08lx\n", \
            regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
    	printk(KERN_ERR"r10: %08lx  r9 : %08lx  r8 : %08lx\n", \
            regs->ARM_r10,regs->ARM_r9, regs->ARM_r8);
    	printk(KERN_ERR"r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",\
            regs->ARM_r7, regs->ARM_r6,regs->ARM_r5, regs->ARM_r4);
    	printk(KERN_ERR"r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",\
            regs->ARM_r3, regs->ARM_r2,regs->ARM_r1, regs->ARM_r0);
        printk(KERN_ERR"++++++++++++++++++++++++++++++++++++++++++++++\n");
        printk(KERN_ERR"access addr         :[0x%x]\n",trigger);
        printk(KERN_ERR"access symbol       :[%pS]\n",(void*)trigger);
        printk(KERN_ERR"==============================================\n");
    }

    if(NULL == pevent){
        show_stack(current,NULL);
    }else if(g_wbp_table[i].event_cb){
        g_wbp_table[i].event_cb((void*)regs);
    } else{
        show_stack(current,NULL);
    }

    if((pevent)&&(g_wbp_table[i].wp_way & WP_EVENT_SUSPEND_HANDLER)){
        memcpy(current_regs,regs,sizeof(current_regs));
        mismatch = 1;
    }

    if(NULL == pevent){
        enable_single_step(bp,regs->ARM_pc-4,mismatch);
    }else if((uintptr_t)g_wbp_table[i].wp_addr == regs->ARM_pc){
        enable_single_step(bp,regs->ARM_pc+4,mismatch);
    }else{
        enable_single_step(bp,regs->ARM_pc-4,mismatch);
        return;
    }
    if ((pevent) && (g_wbp_table[i].wp_way & WP_EVENT_RESET_HANDLER))
    {
        die("breakpoint",regs,0);
        return;
    }
}
static void watchpoint_proc(struct perf_event *bp,
			   struct perf_sample_data *data, struct pt_regs *regs)
{
    struct arch_hw_breakpoint *info = counter_arch_bp(bp);
	int i;
	struct perf_event **pevent = NULL;
    u32 mismatch = 0;

    for(i=0;i<ARM_MAX_WRP;i++){
        if(!g_wbp_table[i].isSet){
            continue;
        }
		pevent = this_cpu_ptr(g_wbp_table[i].wp_pe);
        if(bp == *pevent){
            break;
        }
        pevent = NULL;
    }

    if(NULL == pevent)
    {
       show_stack(current,NULL);
       mismatch = 0;
       goto mismatch;
    }

    /*如果不是所需要的监控范围*/
    if((info->trigger < (uintptr_t)g_wbp_table[i].wp_addr) ||
       (info->trigger > (uintptr_t)(g_wbp_table[i].wp_addr+g_wbp_table[i].wp_len-1))){
       mismatch = 0;
       goto mismatch;
    }

    if((pevent)&&(g_wbp_table[i].wp_way & WP_EVENT_INFO_HANDLER))
    {
        printk(KERN_ERR"==============================================\n");
        printk(KERN_ERR"PC is at %pS\n",(void*)regs->ARM_pc);
        printk(KERN_ERR"LR is at %pS\n",(void*)regs->ARM_lr);

    	printk(KERN_ERR"pc : [<%08lx>]    lr : [<%08lx>]    psr: %08lx\n", \
            regs->ARM_pc, regs->ARM_lr, regs->ARM_cpsr);
        printk(KERN_ERR"sp : %08lx  ip : %08lx  fp : %08lx\n", \
            regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
    	printk(KERN_ERR"r10: %08lx  r9 : %08lx  r8 : %08lx\n", \
            regs->ARM_r10,regs->ARM_r9, regs->ARM_r8);
    	printk(KERN_ERR"r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",\
            regs->ARM_r7, regs->ARM_r6,regs->ARM_r5, regs->ARM_r4);
    	printk(KERN_ERR"r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",\
            regs->ARM_r3, regs->ARM_r2,regs->ARM_r1, regs->ARM_r0);
        printk(KERN_ERR"++++++++++++++++++++++++++++++++++++++++++++++\n");
        printk(KERN_ERR"access addr         :[0x%x]\n",info->trigger);
        printk(KERN_ERR"access type         :[%s]\n",((info->access & HW_BREAKPOINT_RW) == HW_BREAKPOINT_R)?"read":"write");
        printk(KERN_ERR"monitor addr range  :0x%p~0x%p\n",g_wbp_table[i].wp_addr,(g_wbp_table[i].wp_addr+g_wbp_table[i].wp_len-1));
        printk(KERN_ERR"==============================================\n");
    }


    if(NULL == pevent){
        show_stack(current,NULL);
    }else if(g_wbp_table[i].event_cb){
        g_wbp_table[i].event_cb((void*)regs);
    } else{
        show_stack(current,NULL);
    }
    if((pevent)&&(g_wbp_table[i].wp_way & WP_EVENT_SUSPEND_HANDLER)){
        memcpy(current_regs,regs,sizeof(current_regs));
        mismatch = 1;
    }

mismatch:
    enable_single_step(bp,regs->ARM_pc+4,mismatch);

    if ((pevent) && (g_wbp_table[i].wp_way & WP_EVENT_RESET_HANDLER))
    {
        die("watchpoint",regs,0);
    }
    return;
}

static void balong_wp_handler(struct perf_event *bp,
			   struct perf_sample_data *data, struct pt_regs *regs)
{
    struct arch_hw_breakpoint *info = counter_arch_bp(bp);
    if(info->ctrl.type == ARM_BREAKPOINT_EXECUTE){
        breakpoint_proc(bp,data,regs);
    }else{
        watchpoint_proc(bp,data,regs);
    }
    return;
}

int bsp_wp_register(void *addr, unsigned int len, unsigned int type,unsigned int way,unsigned int scope,wp_event_callback cb)
{
	struct perf_event *__percpu *pe;
	struct perf_event_attr attr;
    struct perf_event **pevent;
    struct arch_hw_breakpoint *info;
	int i;
    int cpu;

    if((!addr) || (!len))
    {
        wp_err("invalid parameters! \n");
        return -EINVAL;
    }

	hw_breakpoint_init(&attr);
	attr.bp_addr = (__u64) ((uintptr_t)addr);
	attr.bp_len = len;
    if((attr.bp_addr%4) || (attr.bp_len%4))
    {
        wp_err("invalid monitor addr & length,addr 0x%llx, len 0x%llx\n",attr.bp_addr,attr.bp_len);
        return -EINVAL;
    }
	attr.bp_type = type ;

	pe = register_wide_hw_breakpoint(&attr, balong_wp_handler, NULL);
	if (IS_ERR((void __force *)pe)) {
		wp_err("register failed, addr=0x%p ,err num :%d\n", addr,(int)PTR_ERR((void __force *)pe));
		return (int)PTR_ERR((void __force *)pe);
	}

	for(i = 0;i < ARM_MAX_WRP;i++){
        if(!g_wbp_table[i].isSet){
            g_wbp_table[i].isSet         = true;
            g_wbp_table[i].isEnable      = true;
            g_wbp_table[i].wp_pe         = pe;
            g_wbp_table[i].wp_addr       = addr;
            g_wbp_table[i].wp_len        = len;
            g_wbp_table[i].wp_type       = type;
            g_wbp_table[i].event_cb      = cb;
            g_wbp_table[i].wp_way        = way?way:WP_EVENT_INFO_HANDLER;
            g_wbp_table[i].wp_scope      = scope & 0x3;
            if(type == WATCHPOINT_X)
                printk(KERN_ERR"{breakpoint:id<%1d>,address<%p>}\n",i,addr);
            else
                printk(KERN_ERR"{watchpoint:id<%1d>,address<%p>,length<0x%x>,type<%s>}\n",i,addr,len,\
                        ((type&WATCHPOINT_RW)==WATCHPOINT_RW)?"read/write":(type==WATCHPOINT_R?"read":"write"));

        	for_each_possible_cpu(cpu) {
        		pevent = per_cpu_ptr(g_wbp_table[i].wp_pe, cpu);
                arch_uninstall_hw_breakpoint(*pevent);
        		info = counter_arch_bp(*pevent);
            	info->ctrl.scope = scope&0x3;
        		arch_install_hw_breakpoint(*pevent);
        	}
            return i;
        }
	}
    wp_err("no space for new monitor to register!\n");
    unregister_wide_hw_breakpoint(pe);
	return -ENOSPC;
}
EXPORT_SYMBOL(bsp_wp_register);
int bsp_wp_enable(int wp_id)
{
	int cpu;
	struct perf_event **pevent;

    wp_check_index(wp_id);
    wp_check_set_flag(wp_id);
    wp_check_pe_param(wp_id);

    if(g_wbp_table[wp_id].isEnable){
        return 0;
    }

	for_each_possible_cpu(cpu) {
		pevent = per_cpu_ptr(g_wbp_table[wp_id].wp_pe, cpu);
		arch_install_hw_breakpoint(*pevent);
	}
    g_wbp_table[wp_id].isEnable = true;
    return 0;
}
EXPORT_SYMBOL(bsp_wp_enable);

int bsp_wp_disable(int wp_id)
{
	int cpu;
	struct perf_event **pevent;

    wp_check_index(wp_id);
    wp_check_set_flag(wp_id);
    wp_check_pe_param(wp_id);

    if(!g_wbp_table[wp_id].isEnable)
    {
        wp_err("watchpoint %d is alread disable,no need disable twice!\n",wp_id);
        return 0;
    }

	for_each_possible_cpu(cpu) {
		pevent = per_cpu_ptr(g_wbp_table[wp_id].wp_pe, cpu);
		arch_uninstall_hw_breakpoint(*pevent);
	}

    g_wbp_table[wp_id].isEnable = false;

    return 0;
}
EXPORT_SYMBOL(bsp_wp_disable);
int bsp_wp_unregister(int wp_id)
{
    wp_check_index(wp_id);
    wp_check_set_flag(wp_id);
    wp_check_pe_param(wp_id);

    unregister_wide_hw_breakpoint(g_wbp_table[wp_id].wp_pe);
    memset(&g_wbp_table[wp_id],0,sizeof(g_wbp_table[wp_id]));

	return 0;
}
EXPORT_SYMBOL(bsp_wp_unregister);
static struct notifier_block wbp_suspend_notifier;
static int wp_suspend_nb(struct notifier_block *this,
			 unsigned long event, void *ptr)
{
	struct perf_event *__percpu *pe;
	struct perf_event_attr attr;
	int i,cpu;
    struct perf_event **pevent;
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		wp_err("resume +\n");
        for(i=0;i< ARM_MAX_WRP;i++){
            pe = g_wbp_table[i].wp_pe;
            if(NULL != pe)
                continue;
            if(!g_wbp_table[i].isSet)
                continue;
            hw_breakpoint_init(&attr);
        	attr.bp_addr = (__u64) ((uintptr_t)g_wbp_table[i].wp_addr);
        	attr.bp_len = g_wbp_table[i].wp_len;
        	attr.bp_type = g_wbp_table[i].wp_type ;
        	pe = register_wide_hw_breakpoint(&attr, balong_wp_handler, NULL);
        	if (IS_ERR((void __force *)pe)) {
        		wp_err("register failed,err num :%d\n",(int)PTR_ERR((void __force *)pe));
        		continue;
        	}
            g_wbp_table[i].wp_pe = pe;
            if(!g_wbp_table[i].isEnable){
            	for_each_possible_cpu(cpu) {
            		pevent = per_cpu_ptr(g_wbp_table[i].wp_pe, cpu);
            		arch_uninstall_hw_breakpoint(*pevent);
            	}
            }
        }
		wp_err("resume -\n");
		break;

	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		wp_err("suspend +\n");
        for(i=0;i< ARM_MAX_WRP;i++){
            pe = g_wbp_table[i].wp_pe;
            if(NULL == pe)
                continue;
            unregister_wide_hw_breakpoint(g_wbp_table[i].wp_pe);
            g_wbp_table[i].wp_pe = NULL;
        }
		wp_err("suspend -\n");
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}
int __init bsp_wp_init(void)
{
    DRV_WATCHPOINT_CFG_STRU cfg;
    int i = 0;

    /* coverity[secure_coding] */
    memset(&cfg,0,sizeof(cfg));

    /*pm notifier*/
	/* Register to get PM events */
	wbp_suspend_notifier.notifier_call = wp_suspend_nb;
	wbp_suspend_notifier.priority = -1;
	if (register_pm_notifier(&wbp_suspend_notifier))
    {
		wp_err("Failed to register for PM events\n");
	}


    if(bsp_nvm_read(NV_ID_DRV_WATCHPOINT, (u8 *)&cfg, sizeof(DRV_WATCHPOINT_CFG_STRU)))
    {
        wp_err("read nv err,do not to register the cfg\n");
        return 0;
    }

    for(i=0;i<4;i++)
    {
        if(cfg.ap_cfg[i].enable){
            (void)bsp_wp_register((void*)cfg.ap_cfg[i].start_addr,cfg.ap_cfg[i].start_addr-cfg.ap_cfg[i].end_addr+1, \
                cfg.ap_cfg[i].type,0,WP_TRIGGER_NO_SECURE_STATE,NULL);
        }
    }
    wp_err("ok!\n");
    return 0;
}

module_init(bsp_wp_init);

void wp_help(void)
{
    int i;

    for(i=0;i<ARM_MAX_WRP;i++)
    {
        if(g_wbp_table[i].isSet)
        {
            printk(KERN_ERR"monitor magic       :0x%x\n",g_wbp_table[i].wp_magic);
            printk(KERN_ERR"monitor type        :%d\n",g_wbp_table[i].wp_type);
            printk(KERN_ERR"monitor range       :[%p]~[%p]\n",g_wbp_table[i].wp_addr,(g_wbp_table[i].wp_addr+g_wbp_table[i].wp_len-1));
            printk(KERN_ERR"monitor callback    :%pS\n",g_wbp_table[i].event_cb);
            printk(KERN_ERR"monitor event       :%d\n",g_wbp_table[i].wp_way);
        }
    }
}

void hw_breakpoint_read(void* addr)
{
    u32 val;
    val = readl(addr);
    printk(KERN_ERR"value : 0x%x\n",val);
}

void hw_breakpoint_write(void* addr,u32 value)
{
    writel(value,addr);
}


