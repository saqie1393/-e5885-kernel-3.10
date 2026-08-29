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




#ifndef __MBB_ATCMDCALLPROC_H__
#define __MBB_ATCMDCALLPROC_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-5, begin */
#include "AtMnInterface.h"
/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-5, end */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)  /*4字节对齐*/

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
#define AT_CMD_CMIC_VAL_MAX             (12)
enum AT_CMD_CMIC_VALUE_ENUM
{
    AT_CMD_CMIC_LEV_1                 = 64,    /*第1档音量值*/
    AT_CMD_CMIC_LEV_2                 = 67,    /*第2档音量值*/
    AT_CMD_CMIC_LEV_3                 = 70,    /*第3档音量值*/
    AT_CMD_CMIC_LEV_4                 = 73,    /*第4档音量值*/
    AT_CMD_CMIC_LEV_5                 = 76,    /*第5档音量值*/
    AT_CMD_CMIC_LEV_6                 = 79,    /*第6档音量值*/
    AT_CMD_CMIC_LEV_7                 = 82,    /*第7档音量值*/
    AT_CMD_CMIC_LEV_8                 = 84,    /*第8档音量值*/
    AT_CMD_CMIC_LEV_9                 = 86,    /*第9档音量值*/
    AT_CMD_CMIC_LEV_10                = 88,    /*第10档音量值*/
    AT_CMD_CMIC_LEV_11                = 90,    /*第11档音量值*/
    AT_CMD_CMIC_LEV_12                = 92,    /*第12档音量值*/
};

enum AT_CMD_MODEM_LOOP_ENUM
{
    MODEM_LOOP_OFF = 0,    /*modemloop属于关闭状态*/
    MODEM_LOOP_ON  = 1,    /*modemloop属于打开状态*/
};
typedef VOS_UINT32  AT_CMD_MODEM_LOOP_ENUM_UINT32;

#endif /* FEATURE_ON == MBB_WPG_PCM */


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
#if (FEATURE_ON == MBB_WPG_M2M_DTMF)
extern VOS_VOID AT_RcvVCSetDdtmfCfgCnf(MN_AT_IND_EVT_STRU *pstData);
extern VOS_VOID AT_RcvVCQryDdtmfCfgCnf( MN_AT_IND_EVT_STRU  *pstData);
extern VOS_VOID AT_RcvTafCallDtmfOverInd(MN_AT_IND_EVT_STRU *pstData);

extern VOS_UINT32 At_SetDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_SetVtsexPara(VOS_UINT8 ucIndex);
#endif

#if(FEATURE_ON == MBB_WPG_PCM)

VOS_UINT32 AT_QryModemloopStatePara(VOS_UINT8 ucIndex);

VOS_UINT32 At_QryEccListPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_SetPcmfrPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_QryPcmFrPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_TestPcmFrPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_IsModemLoopOn(VOS_VOID);

VOS_UINT32 AT_IsCallOn(VOS_UINT8 ucIndex);

VOS_UINT32 At_DeltaSetPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 函 数 名  : AT_SetCvhuPara
 功能描述  : +CVHU 设置CVHU参数
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_SetCvhuPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 函 数 名  : AT_QryCvhuPara
 功能描述  : +CVHU  查询CVHU值
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_QryCvhuPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 函 数 名  : AT_TestCvhuPara
 功能描述  : +CVHU 查询CVHU参数范围
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_TestCvhuPara(VOS_UINT8 ucIndex);

VOS_VOID AT_APP_VC_CallStatus(TAF_UINT8 ucIndex);
VOS_UINT32 AT_MBB_SetCmutPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_MBB_QryCmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_SetSmutPara
 功能描述  : ^SMUT=<n> 查询静音状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_SetSmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_QrySmutPara
 功能描述  : ^SMUT? 查询静音状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_QrySmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_SetCpcmPara
 功能描述  : ^CPCM=<n> 查询静音状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_SetCpcmPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_QryCpcmPara
 功能描述  : ^CPCM? 查询静音状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_QryCpcmPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_SetStnPara
 功能描述  : ^STN=<n> 设置侧音开关状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_SetStnPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_QryStnPara
 功能描述  : ^STN? 查询侧音开关状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_QryStnPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_SetNsswitchPara
 功能描述  : ^NSSWITCH=<n> 设置噪音抑制开关状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_SetNsswitchPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_QryNsswitchPara
 功能描述  : ^NSSWITCH? 查询噪音抑制开关状态
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_QryNsswitchPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 函 数 名  : At_SetCmicPara
 功能描述  : ^CMIC=<n> 设置上行音量
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 At_SetCmicPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 函 数 名  : At_QryCmicPara
 功能描述  : ^CMIC? 查询上行音量
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 At_QryCmicPara(VOS_UINT8 ucIndex);
/******************************************************************************
 函 数 名  : At_SetEchoexPara
 功能描述  : ^ECHOEX=<n>[,<value>[,<tmode>[,<tmodepara1>,<tmodepara2>]]] 
设置回声抑制开关及参数
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 At_SetEchoexPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : At_QryEchoexPara
 功能描述  : ^ECHOEX? 查询回声抑制开关及参数
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 At_QryEchoexPara(VOS_UINT8 ucIndex);

#if (FEATURE_ON == MBB_TELEMATIC_AUDIO)
/******************************************************************************
 函 数 名  : AT_SetEchoswitchPara
 功能描述  : ^ECHOSWITCH=<n> 设置回声消除开关
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_SetEchoswitchPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 函 数 名  : AT_QryEchoswitchPara
 功能描述  : ^ECHOSWITCH? 查询回声ECHOSWITCH开关
 输入参数  : ucIndex --- 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_QryEchoswitchPara(VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_TELEMATIC_AUDIO */

VOS_VOID At_RcvMnCallEccListIndProc(
    MN_AT_IND_EVT_STRU                 *pstData);
VOS_VOID At_RcvEccListQryCnf(
    MN_AT_IND_EVT_STRU                 *pstData);
extern VOS_UINT32 At_ProcVcSetSmutStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetSmutStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcSetStnStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcGetStnStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcSetNsswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcGetNsswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetUpVolumeEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvt
);
extern VOS_UINT32 At_ProcVcSetEchoStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetEchoexEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
#if (FEATURE_ON == MBB_TELEMATIC_AUDIO)
/*****************************************************************************
 函 数 名  : At_ProcVcSetEchoswitchStatusEvent
 功能描述  : 处理APP_VC_EVT_SET_ECHOSWITCH事件
 输入参数  : ucIndex      - 客户端索引
             pstVcEvtInfo - 事件
 输出参数  : 无
 返 回 值  : VOS_OK       - 处理正常
             VOS_ERR      - 处理异常
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_ProcVcSetEchoswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
/*****************************************************************************
 函 数 名  : At_ProcVcGetEchoswitchStatusEvent
 功能描述  : 处理APP_VC_EVT_GET_ECHOSWITCH事件
 输入参数  : ucIndex      - 客户端索引
             pstVcEvtInfo - 事件
 输出参数  : 无
 返 回 值  : VOS_OK       - 处理正常
             VOS_ERR      - 处理异常
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_ProcVcGetEchoswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
#endif /* FEATURE_ON == MBB_TELEMATIC_AUDIO */
extern VOS_UINT32 At_ProcVcSetPcmFrStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetPcmFrEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
#endif /* FEATURE_ON == MBB_WPG_PCM */

#if (FEATURE_ON == MBB_WPG_NWTIME)
VOS_UINT32 At_GetLiveTime( NAS_MM_INFO_IND_STRU *pstATtime, 
            NAS_MM_INFO_IND_STRU *pstNewTime );
#endif
#if (FEATURE_ON == MBB_WPG_CCLK)
extern NAS_MM_INFO_IND_STRU    gstCCLKInfo;
#endif

#if (FEATURE_ON == MBB_WPG_ECALL)
extern VOS_UINT32 AT_SetEclAbortPara(VOS_UINT8 ucIndex);
extern VOS_UINT8 AT_GetAppCtrlEcallFlag(VOS_VOID);
extern VOS_VOID AT_ReadHuaweiAppCtrlEcallFeatureNv(VOS_VOID);
extern VOS_UINT32 At_MatchEcallCmdName(VOS_CHAR *pszCmdName);
extern VOS_UINT32 AT_MbbSetEclstartPara(VOS_UINT8 ucIndex);
VOS_VOID AT_EcallStatusError(VOS_VOID);
VOS_UINT32 AT_CheckCtrUser(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_MatchEcallCmdName(VOS_CHAR *pszCmdName);
extern VOS_UINT8 At_HaveEcall(TAF_UINT8 ucIndex);
extern VOS_VOID AT_EcallRedial(VOS_VOID);
extern VOS_VOID EcallRedailInfoSave(VOS_UINT8 ucIndex, MN_CALL_ORIG_PARAM_STRU *pECallParam);
extern VOS_UINT32 EcallRedailClear(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_QryEclListPara(VOS_UINT8 ucIndex);
extern VOS_VOID At_EcallAlackDisplay(AT_ECALL_ALACK_VALUE_STRU    EcallAlackInfo, VOS_UINT16   *usLength);
extern AT_ECALL_ALACK_INFO_STRU* AT_GetEcallAlAckInfoAddr(VOS_VOID);
extern VOS_UINT32 At_ProcVcReportEcallAlackEvent(VOS_UINT8   ucIndex, APP_VC_EVENT_INFO_STRU   *pstVcEvtInfo);
extern VOS_VOID AT_EcallAlAckListDisplay(VOS_VOID);
extern VOS_VOID At_RcvAmrcfgQryCnf(MN_AT_IND_EVT_STRU  *pstData);
extern VOS_VOID AT_RcvTafCallArmcfgCnf(MN_AT_IND_EVT_STRU    *pstData);
extern VOS_UINT32 At_SetAmrcfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryAmrcfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_RcvVcMsgEcallPushCnfProc(MN_AT_IND_EVT_STRU  *pstData);
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AtCmdCallProc.h */
