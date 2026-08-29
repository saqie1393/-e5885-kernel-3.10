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

#ifndef __DUMP_SCHE_RECORD_H__
#define __DUMP_SCHE_RECORD_H__


#include <product_config.h>
#include <osl_types.h>

#include <bsp_softtimer.h>


#define DUMP_INT_IN_FLAG                    0xAAAA
#define DUMP_INT_EXIT_FLAG                  0xBBBB


#define DUMP_KERNEL_TASK_SWITCH_SIZE               (0x10000)
#define DUMP_KERNEL_INTLOCK_SIZE                   (0x1000)

#define DUMP_KERNEL_TASK_INFO_SIZE                 0x200
#define DUMP_KERNEL_TASK_INFO_STACK_SIZE           (DUMP_KERNEL_TASK_INFO_SIZE - 32*4)

#define DUMP_KERNEL_INT_STACK_SIZE                 (0x0)
#define DUMP_KERNEL_TASK_NAME_SIZE                 (0x800)
#define DUMP_KERNEL_ALLTASK_TCB_SIZE               (0x10000)


#define PID_PPID_GET(taskTCB)  ((((struct task_struct *)taskTCB)->pid & 0xffff)| \
                                 ((((struct task_struct *)taskTCB)->real_parent->pid & 0xffff)<< 16))

#define DUMP_T_TASK_ERROR(mod_id)      (mod_id & (1<<24))
#define DUMP_LINUX_TASK_NUM_MAX         128

typedef struct
{
    u32 pid;
    u32 entry;
    u32 status;
    u32 policy;
    u32 priority;
    u32 stack_base;
    u32 stack_end;
    u32 stack_high;
    u32 stack_current;
    u8  name[16];
    u32 regs[17];
    u32 offset;
    u32 rsv[1];
    char dump_stack[DUMP_KERNEL_TASK_INFO_STACK_SIZE];
} dump_tasktcb_info_s;




struct adump_sche_ctrl_info_s
{
    /*int/task switch*/
   void*                   scheRecord_addr;/*switch record*/
   spinlock_t              taskswitch_spinlock;/*task switch lock*/
   bool                    taskSwitch;/*task switch switch*/
   bool                    intSwitch;/*int switch switch*/

    /*all task name*/
   void*                   taskname_addr;
   struct softtimer_list   taskname_timer;
   bool                    taskname_flag;

    /*all task tcb*/
    void*                   tasktcb_addr;
};



void adump_save_all_tasktcb(void);
void adump_save_task_name(void);
s32 adump_tasktcb_info_init(void);
void adump_stop_taskswitch_record(void);
void adump_start_taskswitch_record(void);
void adump_stop_intswitch_record(void);
void adump_start_intswitch_record(void);
void adump_stop_sche_record(void);
void adump_save_all_taskname(void);
s32 adump_start_task_name_save_timer(u32 time_out);
void adump_stop_task_name_timer(void);
s32 adump_task_name_init(void);
s32 adump_sche_record_init(void);


#endif


