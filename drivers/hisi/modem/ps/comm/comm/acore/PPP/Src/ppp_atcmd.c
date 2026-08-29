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



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767  修改人: z57034; 检视人: g45205 原因简述: 打点日志文件宏ID定义 */
#define    THIS_FILE_ID        PS_FILE_ID_PPP_ATCMD_C
/*lint +e767  修改人: z57034; 检视人: g45205 */

#include "product_config.h"

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtPppInterface.h"

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 Prototype      : Ppp_CreatePppReq
 Description    : 为AT模块"创建PPP链路"提供对应的API函数。

 Input          : ---
 Output         : ---创建成功后返回的PPP ID
 Return Value   : ---VOS_UINT32
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-11-18
    Author      : ---
    Modification: Created function
*****************************************************************************/
VOS_UINT32 Ppp_CreatePppReq ( VOS_UINT16 *pusPppId)
{
    return VOS_OK;
}


/*****************************************************************************
 Prototype      : Ppp_CreateRawDataPppReq
 Description    : 创建PDP类型为PPP的PPP实体，但不做链路管理，只作数据的封装和解封装

 Input          : ---
 Output         : ---创建成功后返回的PPP ID
 Return Value   : ---VOS_UINT32
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-11-18
    Author      : ---
    Modification: Created function
*****************************************************************************/
VOS_UINT32 Ppp_CreateRawDataPppReq ( VOS_UINT16 *pusPppId)
{
    return VOS_OK;
}


/*****************************************************************************
 Prototype      : Ppp_RcvConfigInfoInd
 Description    : 为AT模块"PPP模块接收网侧指示的配置信息"提供对应的API函数。
                  当AT向GGSN认证成功后，调用此函数向PPP发指示。

 Input          : usPppId---要发指示的PPP链路所在的PPP ID
                  pPppIndConfigInfo---从GGSN发来的该PPP链路的IP地址等信息
 Output         : ---
 Return Value   : ---VOS_UINT32
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-11-18
    Author      : ---
    Modification: Created function
*****************************************************************************/
VOS_UINT32 Ppp_RcvConfigInfoInd
(
    PPP_ID usPppId,
    AT_PPP_IND_CONFIG_INFO_STRU         *pstAtPppIndConfigInfo
)
{
    return VOS_OK;
}


/*****************************************************************************
 Prototype      : PPP_RcvAtCtrlOperEvent
 Description    : 为AT模块"向PPP发送控制操作"提供对应的API函数。
 Input          : usPppId---要发指示的PPP链路所在的PPP ID
                  ulCtrlOperType---AT向PPP发送的控制操作类型
 Output         : ---
 Return Value   : VOS_OK        --操作成功
                  VOS_ERR       --操作失败
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2008-04-17
    Author      : L47619
    Modification: Created function
*****************************************************************************/
VOS_UINT32 PPP_RcvAtCtrlOperEvent(VOS_UINT16 usPppId, VOS_UINT32 ulCtrlOperType)
{
    return VOS_OK;
}

/*****************************************************************************
 Prototype      : Ppp_RegDlDataCallback
 Description    : 为AT模块提供注册下行发送数据的API

 Input          : usPppId---要发指示的PPP链路所在的PPP ID
 Output         : ---
 Return Value   : ---VOS_UINT32
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2013-06-04
    Author      : ---
    Modification: Created function
*****************************************************************************/
VOS_UINT32 Ppp_RegDlDataCallback(PPP_ID usPppId)
{
    return VOS_OK;
}


