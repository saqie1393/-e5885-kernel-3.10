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
 
#ifndef __MBB_FLIGHT_MODE_H__
#define __MBB_FLIGHT_MODE_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#define RF_OFF              (0)
#define RF_ON               (1)
#define ENTER_FLIGHT_MODE   (1)
#define EXIT_FLIGHT_MODE    (0)

typedef enum
{
    RFSWITCH_SW_TRIG = 0,
    RFSWITCH_HW_TRIG,
    RFSWITCH_INIT_TRIG,
    RFSWITCH_NONE_TRIG
}rfswitch_trig_type;

typedef enum
{
    RFSWITCH_RF_INIT = 0,
    RFSWITCH_RF_ON,
    RFSWITCH_RF_OFF,
    RFSWITCH_RF_NONE
}rfswitch_ccore_op_type;


unsigned int rfswitch_therm_state_get(void);
void rfswitch_state_set(unsigned int rf_state);
unsigned int rfswitch_state_get(void);
void rfswitch_sw_set(unsigned int swstate);
unsigned int rfswitch_sw_get(void);
unsigned int rfswitch_hw_get(void);

rfswitch_trig_type rfswitch_trig_queue_get(void);
void rfswitch_trig_queue_in(rfswitch_trig_type trig_type);
void rfswitch_trig_queue_out(void);


void rfswitch_switch_end_operation(rfswitch_trig_type trig_type, unsigned int sw_op_mode);

#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
void rfswitch_state_info_therm(unsigned int rf_state);
#endif
void rfswitch_change(unsigned int rf_state);
void rfswitch_state_report(unsigned int sw_state,unsigned int hw_state);


#ifdef __cplusplus
}
#endif
#endif


