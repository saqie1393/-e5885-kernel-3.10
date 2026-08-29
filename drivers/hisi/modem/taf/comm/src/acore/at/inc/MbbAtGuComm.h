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



#ifndef _MBB_AT_GU_COMMON_H__
#define _MBB_AT_GU_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "product_config.h"
#include "mdrv.h"
#include "at_lte_common.h"
#include "AtEventReport.h"
#include "LPsNvInterface.h"

#include "AtDataProc.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)

#ifdef __MBB_LINT__
#ifndef VOS_CHAR
typedef char VOS_CHAR;
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / (sizeof((a)[0])))
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define AT_HCSQ_RAT_NAME_MAX             (255)
#define AT_HCSQ_RSSI_VALUE_MIN           (-120)
#define AT_HCSQ_RSSI_VALUE_MAX           (-25)
#define AT_HCSQ_LEVEL_MIN                      (0)
#define AT_HCSQ_RSSI_LEVEL_MAX            (96)   
#define AT_HCSQ_RSCP_VALUE_MIN           (-120)
#define AT_HCSQ_RSCP_VALUE_MAX           (-25)
#define AT_HCSQ_RSCP_LEVEL_MAX            (96 )
#define AT_HCSQ_ECIO_VALUE_MIN           (-32)
#define AT_HCSQ_ECIO_VALUE_MAX           (0)
#define AT_HCSQ_ECIO_LEVEL_MAX           (65)
#define AT_HCSQ_VALUE_INVALID             (255)
#define MMA_PLMN_ID_LEN                         (6)
#define AT_COPN_LEN_AND_RT                  (7)
#define AT_MSG_7BIT_MASK                       (0x7f)

#define TIME_INFO_DEBUG_VAR      (3000)

#define SYSCFGEX_MODE_AUTO       (0x00)
#define SYSCFGEX_MODE_GSM        (0x01)
#define SYSCFGEX_MODE_WCDMA      (0x02)
#define SYSCFGEX_MODE_LTE        (0x03)
#define SYSCFGEX_MODE_CDMA       (0x04)
#define SYSCFGEX_MODE_TDSCDMA    (0x05)
#define SYSCFGEX_MODE_WIMAX      (0x06)
#define SYSCFGEX_MODE_NOT_CHANGE (0x99)
#define SYSCFGEX_MODE_INVALID    (0xFF)
#define SYSCFGEX_MAX_RAT_STRNUM  (7) /*gul每个模式2个字符再加上\0*/

extern AT_DEBUG_INFO_STRU g_stAtDebugInfo;

#if (FEATURE_ON == MBB_WPG_AT)
#define AT_Q_NORMAL_MODE            0
#define AT_Q_QUIET_MODE             1
#endif /* (FEATURE_ON == MBB_WPG_AT) */

#if(FEATURE_ON == MBB_WPG_DIAL)
#define MAX_NDIS_NET                                     (8)
#define DEFAULT_MIN_NDIS_NET                             (1)
extern UDI_HANDLE g_ulAtUdiNdisMpdpHdl[MAX_NDIS_NET];
extern VOS_UINT8  g_ucPcuiPsCallFlg[TAF_MAX_CID + 1];

#endif/*FEATURE_ON == MBB_WPG_DIAL*/

#define AT_DATACLASS_MAX                  AT_DATACLASS_DC_HSPAPLUS
#define AT_DATACLASS_BASE_VALUE           (0X01)
#define AT_DATACLASSLTE_MAX                  (0X01)
#define AT_DATACLASS_LTE           (0X01)
/* 系统模式名称的字符串长度，暂定为16 */
#define AT_DATACLASS_NAME_LEN_MAX         (16)
#define AT_DATACLASS_NOT_SUPPORT          (0)
#define AT_DATACLASS_SUPPORT              (1)
#define AT_DATACLASS_HSPASTATUS_ACTIVED   (1)
#define AT_DATACLASS_ENASRELINDICATOR_R5  (2)
#define AT_DATACLASS_ENASRELINDICATOR_R6  (3)
#define AT_DATACLASS_ENASRELINDICATOR_R7  (4)
#define AT_DATACLASS_ENASRELINDICATOR_R8  (4)
#if (FEATURE_ON == MBB_WPG_COMMON)
#define AT_IPV6_FIRST_VERSION         (1)
#define AT_IPV6_SECOND_VERSION        (2)
#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if(FEATURE_ON == MBB_WPG_PCM)
#define AT_CMD_CLVL_CONST               (13)
#endif /* FEATURE_ON == MBB_WPG_PCM */

#if(FEATURE_ON == MBB_WPG_AC)
#define  AC_REPORT_DISABLED             (0)        /* 关闭 AC 主动上报 */
#define  AC_REPORT_ENABLED              (1)        /* 开启 AC 主动上报 */
#endif/* FEATURE_ON == MBB_WPG_AC */

#if(FEATURE_ON == MBB_NOISETOOL)
#define NOISECFG_PWR_OFFSET  (70)
#define NOISECFG_MAX_PARA_NUM (7)
#define NOISECFG_LTE_MAX_TX_PWR (23)
#define NOISECFG_W_MAX_TX_PWR (24)
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/
#if (FEATURE_ON == MBB_WPG_ROAM)
#define ROAM_STATUS_NOCHANGE         (2)
#define ROAM_MODULE_TO_BL(A)\
    do{\
        A = (A == 1 ? TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON\
               : TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF);\
    }while(0)
#define ROAM_BL_TO_MODULE(B)\
    do{\
        B = (B == TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON ? 1 :0);\
    }while(0)
#endif /* FEATURE_ON == MBB_WPG_ROAM */  

extern AT_SEND_DATA_BUFFER_STRU                gstAtSendData;   /* 单个命令的返回信息存储区 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_FD)

enum AT_RABM_M2M_FASTDORM_OPERATION_ENUM
{
    AT_RABM_M2M_SINGLE_FASTDORM = 1,                                          /* 单次触发 */
    AT_RABM_M2M_AUTOM_FASTDORM,                                                 /* 启动自动触发 */
    AT_RABM_M2M_STOP_FASTDORM,                                                   /* 停止自动触发 */
    AT_RABM_M2M_FASTDORM_START_BUTT
};
#endif/*FEATURE_ON == MBB_WPG_FD*/

#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
enum AT_HFEATURE_ID_ENUM
{
    AT_HFEATURE_SINGLE_PDN = 1,/*目前仅支持SINGLE PDN 其他特性待后续扩展*/
    AT_HFEATURE_BUTT,
};
enum AT_HFEATURE_STATE_ENUM
{
    AT_HFEATURE_NOT_OPEN = 0,/*特性未激活*/
    AT_HFEATURE_OPEN,/*特性激活*/
};
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/

#if (FEATURE_ON == MBB_WPG_PDPSTATUS)
enum AT_REPORT_PDPSTATUS_ENUM
{
    AT_REPORT_PDPDEACTIVATE_NW   = 0,
    AT_REPORT_PSDETACH_NW,
    AT_REPORT_MSDEACTIVATE_MS
};
typedef VOS_UINT32 AT_REPORT_PDPSTATUS_ENUM_UINT32;

#endif/*FEATURE_ON == MBB_WPG_PDPSTATUS*/
#if (FEATURE_ON == MBB_WPG_DIAL)
enum AT_DIALTYPE_ENUM
{
    AT_DIALTYPE_NONE                    = 0,
    AT_DIALTYPE_APP,
    AT_DIALTYPE_NDIS,
    AT_DIALTYPE_MODEM,
    AT_DIALTYPE_BUTT,
};
typedef VOS_UINT32 AT_DIALTYPE_ENUM_UINT32;

enum AT_CONNSTATUS_ENUM
{
    AT_DCONNSTAT_TEST                    = 0,
    AT_NDISSTATQRY_TEST,
    AT_CONNSTAT_BUTT,
};
typedef VOS_UINT32 AT_CONNSTATUS_ENUM_UINT32;
#endif/*FEATURE_ON == MBB_WPG_DIAL*/
/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/
typedef struct
{
    TAF_UINT16   MCC;
    TAF_INT8   Zone;
    TAF_UINT8   Reserved;
}MCC_ZONE_INFO_STRU;


typedef struct
{
    TAF_UINT8  ucLteSupport;
    TAF_UINT8  ucWcdmaSupport;
    TAF_UINT8  ucGsmSupport;
    TAF_UINT8  aucAutoAcqorder[SYSCFGEX_MAX_RAT_STRNUM];
    TAF_UINT8  reserve1;
    TAF_UINT8  reserve2;
}MBB_RAT_SUPPORT_STRU;

extern MBB_RAT_SUPPORT_STRU g_MbbIsRatSupport;

enum AT_DATACLASS_ENUM
{
    AT_DATACLASS_GSM,
    AT_DATACLASS_GPRS,
    AT_DATACLASS_EDGE,
    AT_DATACLASS_WCDMA,
    AT_DATACLASS_HSDPA,
    AT_DATACLASS_HSUPA,
    AT_DATACLASS_HSPA,
    AT_DATACLASS_HSPAPLUS,
    AT_DATACLASS_DC_HSPAPLUS,
};
#if(FEATURE_ON == MBB_NOISETOOL)

typedef struct
{
    VOS_BOOL            bPending;         /*当前是否已经启动测量*/
    VOS_UINT16         usMode;             /*UE 模式*/
    VOS_UINT16         usBand;             /*频段*/
    VOS_UINT16         usDLStFreq;       /*下行扫频起始频点*/
    VOS_UINT16         usDLEdFreq;       /*下行扫频结束频点*/
    VOS_UINT16         usFreqStep;       /*下行扫频步长*/
    VOS_INT16           usWTxPwr;        /*w模上行发射功率*/
    VOS_INT16           usLTxPwr;         /*L模上行发射功率*/
    VOS_UINT16         usBandWidth;       /*物理带宽*/
    VOS_UINT8           ucIndex;
    VOS_UINT8           usRsv[3];         /*字节对齐*/
}AT_NOISE_TOOL_RF_CFG_STRU;

typedef struct
{
    VOS_UINT16         usMode;             /*UE 模式*/
    VOS_UINT32         usStFreq;           /*扫频起始频点*/
    VOS_UINT32         usEdFreq;          /*扫频结束频点*/
    VOS_UINT16         usFreqStep;       /*扫频步长*/
    VOS_UINT8           ucIndex;
    VOS_BOOL            bPending;         /*当前是否已经启动测量*/
    VOS_UINT16         usRsv;
}AT_NOISE_TOOL_GPS_CFG_STRU;

typedef struct
{
    AT_NOISE_TOOL_RF_CFG_STRU      stRfCfg;
    AT_NOISE_TOOL_GPS_CFG_STRU    stGpsCfg;
}AT_NOISE_TOOL_CFG_STRU;
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/

#if (FEATURE_ON == MBB_WPG_M2M_XCELL)
VOS_UINT32 AT_RcvMtaSetXCellInfoCnf(VOS_VOID *pmsg);
VOS_UINT32 AT_RcvMtaQryXCellInfoCnf(VOS_VOID *pmsg);
VOS_UINT32 AT_RcvMtaRptXCellInfoInd(VOS_VOID *pmsg);
#endif /* (FEATURE_ON == MBB_WPG_M2M_XCELL) */

#if(FEATURE_ON == MBB_WPG_WIRELESSPARAM)
VOS_UINT32 AT_RcvMtaTxPowerQryCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvMtaMcsSetCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvMtaTddQryCnf(VOS_VOID *pMsg);
#endif /* FEATURE_ON == MBB_WPG_WIRELESSPARAM */

#if (FEATURE_ON == MBB_WPG_CCLK)
typedef struct
{
    TAF_UINT32    tv_sec;            /* seconds */
    TAF_UINT32    tv_nsec;           /* nanoseconds */
}AT_TIMESPEC;
#endif

#if(FEATURE_ON == MBB_WPG_DIAL)
/*****************************************************************************
 结构名    : AT_AUTH_FALLBACK_STRU
 结构说明  : 鉴权回退全局变量结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             AuthFallbackFlag;   //是否进行鉴权回退
    VOS_UINT16                            AuthNum;            //保存鉴权回退次数
    VOS_UINT16                            AuthType ;          //保存首次拨号的鉴权类型
}AT_AUTH_FALLBACK_STRU;
/*****************************************************************************
 结构名    : AT_NV_AUTHFALLBACK_STRU
 结构说明  : 鉴权回退NV全局变量结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             NvAuthActFlag;        //鉴权nv是否开启
    VOS_UINT16                            NvAuthType;         //NV中保存的鉴权类型
}AT_NV_AUTHFALLBACK_STRU;

#endif

/*****************************************************************************
  7 UNION定义
*****************************************************************************/


/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


#if(FEATURE_ON == MBB_NOISETOOL)
extern AT_NOISE_TOOL_CFG_STRU        g_stAtNoiseToolCfg;
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/

VOS_UINT32 AT_GetTimeInfoDebugFlag(VOS_VOID);
#define AT_EVENT_REPORT_LOG_1(str1, var1) \
{\
    if (VOS_TRUE == AT_GetTimeInfoDebugFlag())\
    {\
        (VOS_VOID)vos_printf("======>%s,%d, %s=%x\n", __func__, __LINE__, str1, var1);\
    }\
}


#if (FEATURE_ON == MBB_MLOG)
#define AT_HIGH_QULITY_RSCP_FDD_MIN      (-95)
#define AT_HIGH_QULITY_RSCP_TDD_MIN      (-84)
#define AT_HIGH_QULITY_RSSI_MIN                (-85)
#define MBB_LOG_RSSI_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSSI_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rssi is %d.\n", var1);\
    }\
}

#define MBB_LOG_FDD_RSCP_INFO(var1,var2)\
{\
    if(AT_HIGH_QULITY_RSCP_FDD_MIN > var1)\
    {\
        mlog_print("at",mlog_lv_error,"rscp is %d, ecio is %d.\n",var1, var2);\
    }\
}

#define MBB_LOG_TDD_RSCP_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSCP_TDD_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rscp is %d.\n",var1);\
    }\
}
#else
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#endif

extern VOS_UINT32 AT_Hex2AsciiStrForIccId(
           VOS_UINT32                          ulMaxLength,
           VOS_INT8                            *pcHeadaddr,
           VOS_UINT8                           *pucDst,
           VOS_UINT8                           *pucSrc,
           VOS_UINT16                          usSrcLen
       );


#if(FEATURE_ON == MBB_WPG_DIAL)
#define    MAX_AUTH_NUM        3  //回退最大次数
#define   AT_DIAL_Exist_User_Password()                (g_ucUserPasswordExist)
#endif
#if (FEATURE_ON == MBB_WPG_EONS)
extern VOS_VOID At_FormatAndSndEons0(TAF_UINT8 ucIndex, VOS_UINT32 RcvNwNameflag);
extern VOS_UINT32 At_CheckEonsTypeMoudlePara(AT_TAF_PLMN_ID* ptrPlmnID);
#endif/*FEATURE_ON == MBB_WPG_EONS*/

extern VOS_UINT32 At_MbbMatchAtCmdName(VOS_CHAR *pszCmdName);
extern VOS_VOID    At_MbbMatchAtInit(VOS_VOID);

#if(FEATURE_ON == MBB_WPG_FD)
extern VOS_VOID    MatchFastDormOperationType(VOS_UINT32* pulFastDormOperationType);
extern VOS_VOID    FastDormTypeDisplay(VOS_UINT16* usLength,
                                                                        VOS_UINT32 enFastDormOperationType,
                                                                        VOS_UINT32 ulTimeLen);
extern VOS_UINT32    MbbSetFastDormPara(VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_FD*/
#if(FEATURE_ON == MBB_WPG_NWNAME)
extern VOS_VOID At_ReportNWName(TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID At_ReportMMInfo(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID AT_RcvMmInfoInd(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
#endif /* FEATURE_ON == MBB_WPG_NWNAME */

#if (FEATURE_ON == MBB_WPG_AT)
extern VOS_VOID At_RunQryParaRspProcCus(TAF_UINT8 ucIndex,TAF_UINT8 OpId, TAF_VOID *pPara, TAF_PARA_TYPE QueryType);
#endif/*FEATURE_ON == MBB_WPG_AT*/

#if(FEATURE_ON == MBB_WPG_CPBS)
extern VOS_VOID At_Pb_VodafoneCPBSCus(TAF_UINT16* usLength, TAF_UINT8 ucIndex);
extern VOS_UINT16 AT_IsVodafoneCustommed(VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_CPBS*/

#if (FEATURE_ON == MBB_WPG_HCSQ)
extern VOS_VOID AT_RptHcsqChangeInfo(TAF_UINT8 ucIndex,TAF_MMA_RSSI_INFO_IND_STRU *pEvent);
extern VOS_BOOL AT_HdlHcsqCmdResult(VOS_UINT32 *ulResult, VOS_VOID *pstMsg);
#endif/*FEATURE_ON == MBB_WPG_HCSQ*/

#if (FEATURE_ON == MBB_WPG_DUAL_IMSI)
extern VOS_VOID AT_ProcSimRefreshInd(VOS_UINT8 ucIndex, const TAF_PHONE_EVENT_INFO_STRU *pstEvent);
#endif/*FEATURE_ON == MBB_WPG_DUAL_IMSI*/

#if (FEATURE_ON == MBB_WPG_CSIM)
extern VOS_UINT16 AT_IsCSIMCustommed(VOS_VOID);
#endif/*FEATURE_OFF == MBB_WPG_CSIM*/

#if (FEATURE_ON == MBB_WPG_ROAM)
VOS_VOID At_EventReportRoamStatus(VOS_UINT8 *ucRoam);
VOS_UINT32 AT_OperRoamStatus(VOS_UINT8* ucRoam);
#endif /* FEATURE_ON == MBB_WPG_ROAM */
#if (FEATURE_ON == MBB_WPG_DIAL)
extern VOS_UINT32 AT_MBB_ConnStatReport(VOS_UINT8 ucIndex, AT_CONNSTATUS_ENUM_UINT32 enConnStatus);
extern VOS_UINT8 AT_GetModemStat(VOS_UINT8  ucCid);
extern VOS_VOID AT_Mbb_SetPcuiCallFlag(VOS_UINT8 ucCid);
extern VOS_UINT32 AT_PS_SetCgactState(
VOS_UINT8   ucIndex,
TAF_CID_LIST_STATE_STRU stCidListStateInfo);
extern VOS_UINT32 AT_PS_ReportDefaultDhcp(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultDhcpV6(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultApraInfo(TAF_UINT8 ucIndex);
extern VOS_VOID AT_SetIPv6VerFlag(VOS_UINT8  ucCid, VOS_UINT8 ucFlag );
extern VOS_UINT32 AT_PS_ValidateDialParamEx(VOS_UINT8 ucIndex);
extern VOS_VOID AT_PS_SndIPV4FailedResult(VOS_UINT8 ucCallId, VOS_UINT16 usClientId);
extern VOS_VOID AT_PS_ProcIpv4CallRejectEx(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent);
extern VOS_VOID AT_PS_ParseUsrDialApn(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam);
extern VOS_UINT32 AT_PS_ParseUsrDialParamEx(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam, 
    TAF_PDP_PRIM_CONTEXT_STRU* stPdpCtxInfo);
extern TAF_UINT32 AT_SetDsFlowQryParaEx(TAF_UINT8 ucIndex);
extern VOS_UINT32 AT_GetNdisDialParamEx(TAF_PS_DIAL_PARA_STRU *pstDialParaInfo, VOS_UINT8 ucIndex);
/*向PC上报连接状态的at函数*/
extern VOS_VOID AT_PS_ReportDendNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause);
/*向PC上报断开状态的at函数*/
extern VOS_VOID AT_PS_ReportDconnNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType);
extern VOS_UINT32 AT_ReportNdisStatPara( VOS_UINT8 ucIndex, VOS_UINT8 ucUserCid);
extern VOS_VOID  AT_PS_MbbProcCallConnectedEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
VOS_VOID  AT_PS_MbbProcCallEndEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent);
extern VOS_VOID  AT_PS_MbbProcDeactiveInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent);

extern VOS_VOID  AT_PS_ModemMbbProcCallRejEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent);
extern VOS_UINT32 Is_Ipv6CapabilityValid(VOS_UINT8 ucIpv6Capability);
extern VOS_UINT32 AT_PS_MbbSetupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    AT_DIAL_PARAM_STRU                  *pstCallDialParam
);
extern VOS_VOID AT_SetModemStat(VOS_UINT8  ucCid, AT_PDP_STATE_ENUM_U8 enPdpState );

extern VOS_UINT32 At_MBB_SetDconnShadowPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_MBB_QryDconnShadowPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_MBB_CheckDconnShadowDialDown(VOS_UINT8 ucIndex);

#endif/*FEATURE_ON == MBB_WPG_DIAL*/
#if (FEATURE_ON == MBB_WPG_SYSCFGEX)
extern VOS_VOID At_FormatSyscfgMbb
(
    AT_MODEM_NET_CTX_STRU *pstNetCtx,
    TAF_MMA_SYSCFG_TEST_CNF_STRU* pstSysCfgTestCnf,
    VOS_UINT8 ucIndex
);
extern VOS_VOID At_FormatSysinfoExMbb(VOS_UINT16 *usLength, TAF_PH_SYSINFO_STRU *stSysInfo);
extern VOS_UINT32 AT_PS_CheckSyscfgexModeRestrictPara(VOS_UINT32 *ulRst, AT_SYSCFGEX_RAT_ORDER_STRU *stSyscfgExRatOrder);
extern VOS_VOID AT_MBBConverAutoMode(TAF_MMA_RAT_ORDER_STRU    *pstSysCfgRatOrder);
#endif /*FEATURE_ON == MBB_WPG_SYSCFGEX*/

#if(FEATURE_ON == MBB_WPG_CCLK)
extern VOS_VOID AT_UpdateCclkInfo(TAF_MMA_TIME_CHANGE_IND_STRU *pStMmInfo);
#endif/*FEATURE_ON == MBB_WPG_CCLK*/


extern VOS_UINT32 At_RegisterExPrivateMbbCmdTable(VOS_VOID);
extern VOS_VOID AT_ReadNvMbbCustorm(VOS_VOID);
extern VOS_UINT32 TAF_AGENT_GetSysMode(VOS_UINT16 usClientId,  TAF_AGENT_SYS_MODE_STRU *pstSysMode);

#if (FEATURE_ON == MBB_WPG_PDPSTATUS)
VOS_VOID AT_PS_ReportPDPSTATUS(
    AT_REPORT_PDPSTATUS_ENUM_UINT32     enStat,
    VOS_UINT8                           ucPortIndex
);
#endif/*FEATURE_ON == MBB_WPG_PDPSTATUS*/
#if (FEATURE_ON == MBB_WPG_3GPP_STK)
TAF_VOID  At_StkCusatpIndPrint(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent);
TAF_VOID  At_StkCusatmIndPrint(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent);
TAF_VOID At_PrintSTKStandardCMDRsp(
    TAF_UINT8                           ucIndex,
    SI_STK_EVENT_INFO_STRU             *pstEvent
);

#endif /* FEATURE_ON == MBB_WPG_3GPP_STK */

/* MBB_WPG_COMMON */
extern VOS_VOID AT_CLCC_Report(VOS_UINT8 numType, VOS_UINT16 *usLength, VOS_UINT8 *aucAsciiNum);
extern VOS_UINT32 At_TestCgdcont_IP(VOS_UINT8 ucIndex);
#if (FEATURE_ON == MBB_WPG_DIAL)
extern VOS_VOID AT_PS_SavePSCallAuthRecord(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
extern VOS_UINT32 AT_PS_ProcAuthCallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             ucPdpType
);
extern VOS_VOID AT_PS_GenCallDialParam(
    AT_DIAL_PARAM_STRU                 *pstCallDialParam,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
extern VOS_VOID AT_PS_ParseUsrDialAuthtype(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam);
#endif/*FEATURE_ON == MBB_WPG_DIAL*/


#if(FEATURE_ON == MBB_NOISETOOL)
extern VOS_UINT32 AT_SetNoiseCfgPara(VOS_UINT8 ucIndex);
extern VOS_VOID At_NoiseRssi_Report
(
    VOS_UINT16 usDLStFreq,
    VOS_UINT16 usDLRssiNum,
    VOS_INT16 *usDLPriRssi,
    VOS_INT16 *usDLDivRssi
);
extern TAF_UINT32 At_NoiseToolSendRxOnOffToHPA
(
    AT_NOISE_TOOL_RF_CFG_STRU *pstNoiseCfg
);
extern TAF_UINT32 At_NoiseToolSendRxOnOffToGHPA
(
    TAF_UINT8 ucIndex, 
    AT_NOISE_TOOL_RF_CFG_STRU *pstNoiseCfg
);
extern TAF_UINT32 AT_SetRfonoffPara(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_QryRFonoffStatePara(TAF_UINT8 ucIndex);
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/

extern VOS_UINT32  At_QryLteCatEx(VOS_UINT8 ucIndex);

typedef struct
{
    VOS_UINT32    UeCapabilitybitUeEutraCapV940Present                     : 1;
    VOS_UINT32    UeCapabilitybitUeEutraCapV1020Present                   : 1;
    VOS_UINT32    UeCapV1020bitUeCatgV1020Present                             : 1;
    VOS_UINT32    UeCapV1020bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1060bitNonCritiExtPresent                             : 1; 
    VOS_UINT32    UeCapV1090bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1130bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1170bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1170bitucCategoryPresent                             : 1;
    VOS_UINT32    UeCapV1180bitNonCritiExtPresent                             : 1;    
    VOS_UINT32    UeCapV11A0bitUeCatgV11a0Present                         : 1;
    VOS_UINT32    UebitSpare                                              : 21;  /*bits补齐*/

    VOS_UINT8     UeCapabilityUeCatg;
    VOS_UINT8     UeCapV1020UeCatgV1020;
    VOS_UINT8     UeCapV1170UeCategoryV1170; 
    VOS_UINT8     UeCapV11A0UeCatgV11a0;
}Lte_Cat_Info_STRU;

typedef struct 
{
    VOS_UINT32 ulNvId; 
    VOS_UINT32 ulNvLen;    
}Lte_Cat_Nv_Info_STRU;



/* 需要重点check代码 */
extern VOS_BOOL At_CmdAddDoubleQuotes(VOS_UINT8** pHead, VOS_UINT16 * usCount, VOS_CHAR * pAtcmdString,
                               VOS_UINT16 uiLocation1, VOS_UINT16 uiLocation2);
#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if (FEATURE_ON == MBB_WPG_CPBREADY)
VOS_VOID AT_ProcPbreadyInd(VOS_UINT8   ucIndex);
#endif/*FEATURE_ON == MBB_WPG_CPBREADY*/

#if (FEATURE_OFF == MBB_WPG_COMMON)
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#define AP_CONN_DEBUG_STR_VAR(str, var)
#define AP_CONN_DEBUG()
#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if(FEATURE_ON == MBB_WPG_AC)

extern TAF_VOID AT_ACReportMsgProc(TAF_MMA_MSG_AC_REPORT_STRU *pMsg);
#endif/* FEATURE_ON == MBB_WPG_AC */

#if(FEATURE_ON == MBB_WPG_SIM_SWITCH)
extern TAF_VOID      At_QryParaRspSimSwitchProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
);
extern TAF_VOID      At_SimSwitchMsgProc(MN_MSG_SIM_SWITCH_STRU* pMsg);
extern VOS_VOID      AT_ReadSimSwitchFlag( VOS_VOID );
#endif/* FEATURE_ON == MBB_WPG_SIM_SWITCH */

#if(FEATURE_ON == MBB_MLOG)
typedef struct
{
    VOS_INT16      sRsrpValue;/* 范围：(-141,-44), 99为无效 */
    VOS_INT16      sRsrqValue;/* 范围：(-40, -6) , 99为无效 */
    VOS_INT32      lSINRValue;/* SINR */
    VOS_INT16      sRscpValue;  /* 小区信号质量用于3g下*/
    VOS_INT16      sEcioValue;  /* 小区信噪比用于3g*/
    VOS_INT16      sRssiValue;   /* 小区信号质量用于2g下^cerssi上报使用,2g没有rscp的概念用的是rssi */
    VOS_UINT8      aucReserve1[2];  /*reserv*/
}SIGNAL_INFO_STRU;

typedef struct
{
    TAF_PH_INFO_RAT_TYPE               ucRatType;           /* 接入技术   */
    TAF_SYS_SUBMODE_ENUM_UINT8         ucSysSubMode;        /* 系统子模式 */
} AT_SYS_MODE_STRU;

extern SIGNAL_INFO_STRU       g_stSignalInfo;
extern AT_SYS_MODE_STRU     g_stAtSysMode;
extern VOS_VOID   AT_MBB_RecordMlogDialFail(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU   *pstEvent);
extern VOS_VOID   AT_MBB_MlogPrintPdpInfo(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU   *pEvtInfo);
#endif

#if (FEATURE_ON == MBB_WPG_SBB_RRCSTAT)
extern VOS_UINT32 AT_RcvMmaRrcStatQueryCnf(VOS_VOID *pstMsg);
#endif /*(FEATURE_ON == MBB_WPG_SBB_RRCSTAT)*/
#if (FEATURE_ON == MBB_WPG_AT_ABORT)
extern VOS_UINT32 At_AbortCmgsPara(
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 At_AbortCmssPara(
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 At_AbortCcwaPara(
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 At_AbortClipPara(
    VOS_UINT8                           ucIndex
);

#endif  /*FEATURE_ON == MBB_WPG_AT_ABORT*/
/*#if (FEATURE_ON == MBB_WPG_TDD_FDD_CHANGE_SBB), 为适配UT工程，这里使用COMMON宏控制，
宏定义和结构体定义不影响其他模块的逻辑，故可以用COMMON宏来控制*/
#if (FEATURE_ON == MBB_WPG_COMMON)
#define LTE_BAND1                (1)
#define LTE_BAND41              (41)
#define LTE_TDD_ONLY          (1)
#define LTE_FDD_ONLY          (2)
#define FGI_DISABLE              (0)
#define FGI_ENABLE               (1)
#define FDD_FGI_V1020           (0x00000008)//从低位到高位依次为FGI BIT1, FGI BIT2......
#define TDD_FGI_V1020           (0x000003FC)//从低位到高位依次为FGI BIT1, FGI BIT2......
#define FDD_FGI_V9                (0x011BB03A)//从低位到高位依次为FGI BIT1, FGI BIT2......
#define TDD_FGI_V9                (0x011BB03B)//从低位到高位依次为FGI BIT1, FGI BIT2......
#define AT_CMD_STR_LBANDSWITCHSBB    "^LBANDSWITCHSBB"
typedef union {
    LRRC_NV_UE_EUTRA_CAP_STRU stUeCapR9;
    RRC_UE_EUTRA_CAP_V1020_IES_STRU stUeCapV1020;
    RRC_UE_EUTRA_CAP_V1170_IES_STRU stUeCapV1170;
}LTE_CAPA_INFO;
#endif
/*#endif*/

#if (FEATURE_ON == MBB_WPG_TRUST_SNUM) 
TAF_UINT32 At_CheckTrustNumPara(TAF_UINT8 *pData, TAF_UINT16 pusLen,VOS_UINT8 ucIndex);
#endif/*#FEATURE_ON == MBB_WPG_TRUST_SNUM*/
#if (FEATURE_ON == MBB_WPG_NWTIME)
extern VOS_UINT32 BSP_GetSeconds(VOS_VOID);

#if (FEATURE_ON == MBB_WPG_CCLK)
extern int do_sys_settimeofday(const struct timespec *tv, const struct timezone *tz);
extern unsigned long mktime (unsigned int year, unsigned int mon,
    unsigned int day, unsigned int hour,
    unsigned int minute, unsigned int sec);
#endif/*FEATURE_ON == MBB_WPG_CCLK*/

#endif/*FEATURE_ON == MBB_WPG_NWTIME*/

MN_PS_EVT_FUNC AT_MbbGetTafPsEventProcFunc(VOS_UINT32 ulEventId);

#if (FEATURE_ON == MBB_WPG_DIAL) 
AT_PDP_STATUS_ENUM_UINT32 AT_NdisGetConnStatus(AT_PDP_STATE_ENUM_U8 enPdpState);
#endif/*FEATURE_ON == MBB_WPG_DIAL*/

#if (FEATURE_ON == MBB_WPG_MODULE_AT_HSMF) 
VOS_UINT32 AT_SetAppMemStatusPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryAppMemStatusPara(VOS_UINT8 ucIndex);
#endif

#if ( FEATURE_ON == MBB_WPG_MODULE_AT_LOPCNL )
TAF_VOID At_QryParaLopcnlProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
);
#endif

#if defined(__UT__) && defined(WIN32)
/* UT编译添加,只在编译UT时生效 */
VOS_UINT32 VOS_SmCreate( VOS_CHAR Sm_Name[4],
                         VOS_UINT32 Sm_Init,
                         VOS_UINT32 Flags,
                         VOS_SEM * Sm_ID );
VOS_UINT32 VOS_SmP( VOS_SEM Sm_ID, VOS_UINT32 ulTimeOutInMillSec );
VOS_UINT32 VOS_SmV( VOS_SEM Sm_ID );
#endif /* _EXPORT_PRIVATE */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__AT_H__
 */






