/*
 *
 * All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses. You may choose this file to be licensed under the terms
 * of the GNU General Public License (GPL) Version 2 or the 2-clause
 * BSD license listed below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


#include "product_config.h"

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "ppp_public.h"

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 Prototype      : PPP_PullPacketEvent
 Description    : 从R接口接收数据，发送到PPP处理
 Input          : ---
 Output         : ---
 Return Value   : PS_SUCC   --- 成功
                  PS_FAIL   --- 失败
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2008-07-17
    Author      : L47619
    Modification: Created function
*****************************************************************************/
VOS_UINT32 PPP_PullPacketEvent(VOS_UINT16 usPppId, PPP_ZC_STRU *pstImmZc)
{
    return PS_SUCC;
}

/*****************************************************************************
 Prototype      : PPP_PullRawEvent
 Description    : 从R接口接收数据，发送到PPP处理，PPP类型的PDP激活情况下使用，
 Input          : ---
 Output         : ---
 Return Value   : PS_SUCC   --- 成功
                  PS_FAIL   --- 失败
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2008-07-17
    Author      : L47619
    Modification: Created function
*****************************************************************************/
VOS_UINT32 PPP_PullRawDataEvent(VOS_UINT16 usPppId, PPP_ZC_STRU *pstImmZc)
{
    return PS_SUCC;
}



