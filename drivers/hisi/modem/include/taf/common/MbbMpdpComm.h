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

/******************************************************************************
 */
/*************************************************************************************
*************************************************************************************/



#ifndef _MBB_MPDP_COMM_H__
#define _MBB_MPDP_COMM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"
#include "TafAppMma.h"
#include "TafApsApi.h"
#include "mbb_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if(FEATURE_ON == MBB_WPG_DIAL)
enum AT_CH_DATA_CHANNEL_ENUM
{
    AT_CH_DATA_CHANNEL_ID_1             = 1,
    AT_CH_DATA_CHANNEL_ID_2,
    AT_CH_DATA_CHANNEL_ID_3,
    AT_CH_DATA_CHANNEL_ID_4,
    AT_CH_DATA_CHANNEL_ID_5,
    AT_CH_DATA_CHANNEL_ID_6,
    AT_CH_DATA_CHANNEL_ID_7,
    AT_CH_DATA_CHANNEL_ID_8,
    AT_CH_DATA_CHANNEL_BUTT
};
typedef VOS_UINT32 AT_CH_DATA_CHANNEL_ENUM_UINT32;


enum NDIS_RM_NET_ID_ENUM
{
    NDIS_RM_NET_ID_0,                        /*网卡1*/
    NDIS_RM_NET_ID_1,                        /*网卡2*/
    NDIS_RM_NET_ID_2,                        /*网卡3*/
    NDIS_RM_NET_ID_3,                        /*网卡4*/
    NDIS_RM_NET_ID_4,                        /*网卡5*/
    NDIS_RM_NET_ID_5,                        /*网卡6*/
    NDIS_RM_NET_ID_6,                        /*网卡7*/
    NDIS_RM_NET_ID_7,                        /*网卡8*/
    NDIS_RM_NET_ID_BUTT
};
typedef VOS_UINT8 NDIS_RM_NET_ID_ENUM_UINT8;



typedef struct
{
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enChdataValue;
    NDIS_RM_NET_ID_ENUM_UINT8           enNdisRmNetId;
    VOS_UINT8                           aucReserved[3];
}AT_CHDATA_NDIS_RMNET_ID_STRU;

/*****************************************************************************
 函 数 名  : AT_DeRegNdisFCPoint
 功能描述  : 去注册NDIS端口流控点。
 输入参数  : VOS_UINT8                           ucRabId
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_DeRegNdisFCPointEx(VOS_UINT8 ucRabId,
    VOS_UINT16                enModemId,
    VOS_UINT8                   enFcId);

/*通过信道ID获取到RmnetId*/
VOS_UINT32 AT_PS_GetNdisRmNetIdFromChDataValue(VOS_UINT8  ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId,
    RNIC_RMNET_ID_ENUM_UINT8          *penNdisRmNetId);

/*拨号成功之后，激活网卡的处理*/
VOS_VOID AT_PS_SndNdisPdpActInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU        *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
/*拨号成功之后，去激活网卡的处理*/
VOS_VOID AT_PS_SndNdisPdpDeactInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
/*拨号成功之后，注册流控的处理*/
VOS_VOID AT_PS_RegNdisFCPoint(VOS_UINT8 ucCid, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
/*拨号成功之后，去注册流控的处理*/
VOS_VOID AT_PS_DeRegNdisFCPoint(VOS_UINT8 ucCid, TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent);

extern VOS_UINT8 AT_GetDefaultFcID(VOS_UINT8 ucUserType, VOS_UINT32 ulRmNetId);
extern VOS_VOID AT_PS_UpdateUserDialDnsInfo(VOS_UINT8 ucIndex, VOS_VOID* stUsrDialParam);
extern VOS_INT AT_UsbCtrlBrkReqCBMpdp(VOS_VOID);
extern VOS_VOID AT_OpenUsbNdisMpdp(VOS_VOID* stParam);
extern VOS_VOID AT_CloseUsbNdisMpdp(VOS_VOID);
extern VOS_UINT8 AT_GetUsbNetNum(VOS_VOID);
extern VOS_VOID AT_IncreaseNumWhenAct(VOS_VOID);
extern VOS_VOID AT_DecreaseNumWhenDeact(VOS_VOID);
extern VOS_UINT8 AT_PS_FindDialCid(VOS_UINT16 ucIndex, const VOS_UINT8 ucCid);
extern VOS_UINT32 AT_PS_ProcDialCmdMpdp(VOS_UINT32* ulResult, VOS_UINT8 ucIndex);
extern VOS_VOID AT_SetCurrentNdisUdiHandle(VOS_UINT16 usClientId, VOS_UINT8 ucUserCid);
extern UDI_HANDLE AT_GetCurrentNdisUdiHandle(VOS_VOID);
extern VOS_UINT32 AT_SendNdisIPv4PdnInfoCfgReq(
    AT_CLIENT_TAB_INDEX_UINT8           usClientId,
    AT_IPV4_DHCP_PARAM_STRU            *pstIPv4DhcpParam
);
#endif/*FEATURE_ON == MBB_WPG_DIAL*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif


