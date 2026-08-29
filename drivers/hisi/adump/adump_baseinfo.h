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
#ifndef __DUMP_BASE_H__
#define __DUMP_BASE_H__

#include <osl_types.h>
#include <linux/hisi/rdr_pub.h>
#include <bsp_adump.h>


/**************************************************************************
  宏定义
**************************************************************************/

#define DUMP_KERNEL_BASE_INFO_SIZE                 (0x180)
#define ARM_REGS_NUM                        17

/*  CPSR R16
  31 30  29  28   27  26    7   6   5   4    3    2    1    0
----------------------------------------------------------------
| N | Z | C | V | Q | RAZ | I | F | T | M4 | M3 | M2 | M1 | M0 |
---------------------------------------------------------------- */
/*  REG
R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13/SP R14/LR R15/PC R16/CPSR
*/

typedef struct
{
    u32 reboot_context;     /*0x00  */
    u32 reboot_task;        /*0x04  */
    u8  taskName[16];       /*0x08  */
    u32 reboot_int;         /*0x18  */

    u32 modId;              /*0x1c  */
    u32 arg1;               /*0x20  */
    u32 arg2;               /*0x24  */
    u32 arg3;               /*0x28  */
    u32 arg3_length;        /*0x2c  */

    u32 vec;                /*0x30  */
    u32 cpsr;               /*0x34  */
    u32 regSet[17];         /*0x38  */

    u32 current_task;       /*0x7c */
    u32 current_int;        /*0x80 */

    u32 cpu_usage;          /*0x84 */
    u32 mem_free;          /*0x88 */

    u32 axi_dlock_reg[3];   /*0x8C --- AXI总线挂死寄存器，0x624,0x628,0x658*/
    u32 int_stack_base;     /*0x98 */
    u32 int_stack_end;      /*0x9C */
    u8  version[32];        /*0xA0 */
    u8  compile_time[32];   /*0xB0 */
    u32 current_task_id;    /*0x88 */
    u32 reboot_reason;
}adump_base_info_s;



void adump_fill_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);
void adump_fill_exc_vec(u32 vec);
void adump_fill_current_task(u32 current_task);
void adump_fill_current_int(u32 current_int);
void adump_fill_arm_register(u8* regs);
void adump_fill_exc_vec(u32 vec);

s32 adump_baseinfo_init(void);


#endif

