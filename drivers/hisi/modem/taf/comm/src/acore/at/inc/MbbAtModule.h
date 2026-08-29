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



#ifndef __MBB_AT_MODULE_H__
#define __MBB_AT_MODULE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"

#include "MbbAtMtaInterface.h"
#include "GasNvInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "mdrv_version.h"

#pragma pack(4)  /*4字节对齐*/

#if (FEATURE_ON == MBB_WPG_NETMONITOR)
#define AT_NETMON_PLMN_STRING_MAX_LENGTH            (10)
#define AT_NETMON_GSM_RX_QUALITY_INVALID_VALUE      (99)
#define AT_NETMON_GSM_RSSI_INVALID_VALUE            (-500)
#define AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE      (0)
#define AT_NETMON_UTRAN_FDD_RSSI_INVALID_VALUE      (0)
#define AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE      (0)
#define AT_NETMON_UTRAN_TDD_DRX_MAX_VALUE           (9)
#define AT_NETMON_UTRAN_TDD_DRX_MIN_VALUE           (6)
#define AT_NETMON_EUTRAN_RSSI_INVALID_VALUE         (-32768)
#endif /* FEATURE_ON == MBB_WPG_NETMONITOR */

#if (FEATURE_ON == MBB_WPG_MODULE_AT)
#define HCSTUB_NV_MAX_NUM     (4)
#define MAX_ASCII_LENGTH 3

#define PORT_VALUE_RESERVED 0xFF
#define INITIAL_PORT_VALUE 0x00
#define PORT_VALUE_PCSC 0x04
#define PORT_VALUE_GPS 0x14

#define MAX_CONFIG_NUM 0x03
#define MULTI_CONFIG_FIRST 0x01
#define MULTI_CONFIG_SECOND 0x02
#define MULTI_CONFIG_THIRD 0x03
enum A2fCount_ENUM
{
    AT_MBB_CMD_A_LEN_0                  = 0,
    AT_MBB_CMD_A_LEN_1,                  
    AT_MBB_CMD_A_LEN_2,
    AT_MBB_CMD_A_LEN_3,
    AT_MBB_CMD_A_LEN_4 
};
enum AT_DVCFG_PRIORITY_ENUM
{
    AT_DVCFG_CALL_PRIORITY = 0,
    AT_DVCFG_DATA_PRIORITY
};
typedef VOS_UINT32 AT_DVCFG_PRIORITY_UINT32;

#define UTRAN_SWITCH_MODE_AUTO                   (2)                     /* 当前自动切换模式 */
#define AT_SIM_PLMN_ID_LEN                       (3)                     /* SIM卡中PLMN ID的长度 */
#define AT_SIM_PLMN_ID_NUM_MAX                   (6)                     /* 最大支持6组PLMN ID*/
#define AT_OCTET_MOVE_FOUR_BITS                  (0x04)                  /* 将一个字节移动4位 */
#define AT_OCTET_LOW_FOUR_BITS                   (0x0f)                  /* 获取一个字节中的低4位 */
#define AT_OCTET_HIGH_FOUR_BITS                  (0xf0)                  /* 获取一个字节中的高4位 */

#if (FEATURE_ON == MBB_EUICC)
#define AT_CARD_SERVIC_ABSENT           0

#define EID_ECASD_MAXLEN               (4)
#define EID_SIN_MAXLEN                (28)
#define EID_SDIN_MAXLEN               (20)
#define EID_MAXLEN                    (32)

/*****************************************************************************
 结构体名  : USIMM_EUICC_ID_STRU
 结构说明  : EUICC ID 的结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8      sinData[EID_SIN_MAXLEN];       //get sin的数据包
    VOS_UINT8      sdinData[EID_SDIN_MAXLEN];     //get sdin 的数据包
    VOS_UINT8      eidData[EID_MAXLEN];           //组成的eid的数据
    VOS_UINT32     EID_LEN;
    VOS_BOOL       ECASD_FLAG;
    VOS_BOOL       EID_FLAG;
}USIMM_EID_STRU;

/*****************************************************************************
 结构体名  : USIMM_EID_RECEIVE_STRU
 结构说明  : EUICC ID 的结构
*****************************************************************************/
typedef struct
{
   VOS_MSG_HEADER
   VOS_UINT32                 MsgName;
   USIMM_EID_STRU             EIDInfo;   
}USIMM_EID_RECEIVE_STRU;

typedef struct
{
    USIMM_CMDHEADER_REQ_STRU            stMsgHeader;
} USIMM_ACTIVEID_REQ_STRU;

VOS_VOID AT_GetEIDInfo(USIMM_EID_RECEIVE_STRU *EIDInfo);
#endif/*FEATURE_ON == MBB_EUICC*/



typedef struct {
    VOS_UINT8                           aucSimPlmn[AT_SIM_PLMN_ID_LEN];
    VOS_UINT8                           aucReserve[1];
}AT_SIM_FORMAT_PLMN_ID;

typedef struct
{
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
    VOS_UINT32                          ulMnc;                                  /* MNC,2 or 3 bytes */
}AT_PLMN_ID_STRU;

typedef struct
{
    VOS_UINT32 MsoIndexClient;
    VOS_UINT8 MsoFlag;
    VOS_UINT8 res[3];              /*align*/
    TAF_MMA_PHONE_MODE_PARA_STRU MsoMode;
}AT_MSO_STRU;

typedef struct
{
    HTIMER                              hTimer;            /* Timer句柄 */
    VOS_UINT16                          usName;            
    VOS_UINT16                          usPara;             
    VOS_UINT8                           ucMode;             /* timer work mode
                                                               VOS_RELTIMER_LOOP   -- start periodically
                                                               VOS_RELTIMER_NOLOOP -- start once time */
    VOS_UINT8                           ucResv[3];              /*align*/
    VOS_UINT32                          ulTimerLen;         /* Timer时长(UNIT: ms) */
}MSO_TIMER_STRU;


typedef struct
{
    VOS_UINT8                           ucUsed;                                 /*0：未使用，1：使用*/
    VOS_UINT8                           aucReserved[3];                         /*align*/
    TAF_PDP_AUTHDATA_STRU               stAuthData;                             /*鉴权参数*/
} TAF_NDIS_AUTHDATA_TABLE_STRU;

extern AT_MSO_STRU g_stMsoInfo;
extern MSO_TIMER_STRU g_MsoTimer;

VOS_UINT32 AT_TestCfunPara(VOS_UINT8 ucIndex);
TAF_UINT8 At_SetCfunSecPara(AT_PARSE_PARA_TYPE_STRU *stAtParaList, TAF_UINT8   ucIndexNum, TAF_UINT8 ucIndex);
VOS_UINT32 At_SetGetSPara(VOS_UINT8 ucIndex);
TAF_UINT32 At_SetAndVPara(TAF_UINT8 ucIndex);
TAF_UINT32 At_SetAmpWPara(TAF_UINT8 ucIndex);
TAF_UINT32 At_MBB_SetVPara(TAF_UINT8 ucIndex);
VOS_UINT32 At_DisconnectPsCall(VOS_UINT8   ucClient);
VOS_UINT32 At_DisconnectCsCall(VOS_UINT8 ucIndex);
TAF_UINT32 At_MBB_SetZPara(TAF_UINT8 ucIndex);
TAF_UINT32 At_MBB_SetCeerPara(TAF_UINT8 ucIndex);
VOS_UINT8 AT_MBB_CheckCsca_Param(VOS_UINT8 *ptScaType);
VOS_UINT32 AT_MBB_TestDhcpPara(VOS_UINT8 ucIndex);
VOS_UINT8 AT_MBB_CUSD_USSDMode_Check
(
        VOS_UINT16                   usUssdTransMode,
        TAF_SS_DATA_CODING_SCHEME    DatacodingScheme
);
TAF_UINT32  At_MBB_CheckUssdNumLen(
    AT_MODEM_SS_CTX_STRU                *pstSsCtx,
    TAF_SS_DATA_CODING_SCHEME           dcs,
    TAF_UINT16                          usLen
);
VOS_VOID At_NidsdupGetParafromAuthdata (TAF_PS_DIAL_PARA_STRU *stDialParaInfo);


VOS_VOID At_Ndis_PS_CALL_Info(TAF_PS_DIAL_PARA_STRU *pstCallDialParam);
TAF_UINT32 At_SetHcstubPara (TAF_UINT8 ucIndex);
VOS_UINT32 At_TestHcstubState(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryHcstubPara(VOS_UINT8 ucIndex);
VOS_VOID AT_ReportCssuHoldCallReleased(
    VOS_UINT8                           ucIndex,
    MN_CALL_INFO_STRU                  *pstCallInfo
);
TAF_VOID At_DvcfgConditionHangupCallingProc(
    TAF_UINT8                           ucIndex,
    MN_CALL_EVENT_ENUM_U32              enEvent,
    MN_CALL_INFO_STRU                   *pstCallInfo
);
VOS_VOID  AT_MBB_Report_CPBF(VOS_UINT8 ucIndex, VOS_UINT16 *usBufLen);
TAF_UINT32 At_MBB_SetQPara(TAF_UINT8 ucIndex);
VOS_UINT32 At_MBB_QryS0Para(TAF_UINT8 ucIndex);
VOS_VOID AT_MBB_CopyNotSupportRetValueNV(VOS_UINT32 ulRetLen, VOS_CHAR *acRetVal);
VOS_UINT32 AT_TestCfunPara(VOS_UINT8 ucIndex);
TAF_UINT32 At_QryCpbwPara(TAF_UINT8 ucIndex);
VOS_UINT8 AT_MBB_CNMICheckType(AT_CNMI_MODE_TYPE CnmiTmpModeType, AT_CNMI_MT_TYPE CnmiTmpMtType);
VOS_UINT32 At_TestDvcfgPriority(VOS_UINT8 ucIndex);
TAF_UINT32 At_SetDvcfgPara (TAF_UINT8 ucIndex);
VOS_UINT32 At_QryDvcfgPara(VOS_UINT8 ucIndex);
VOS_VOID AT_Report_Cend(VOS_UINT16 *usLength, MN_CALL_INFO_STRU *pstCallInfo);
TAF_UINT32 At_MBB_SetFPara(TAF_UINT8 ucIndex);
VOS_VOID  AT_ProcCsRspEvtAlerting(
    TAF_UINT8                           ucIndex,
    MN_CALL_INFO_STRU                  *pstCallInfo
);
VOS_UINT32 AT_Cgact_DialDownProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT16                          usClientId
);
VOS_UINT32 AT_CGACT_MODEM_HangupCall(VOS_UINT8 ucIndex);
TAF_UINT32 AT_Cgact_DownProc(TAF_UINT8 ucIndex);
TAF_UINT32 At_MBB_SetCgact0(TAF_UINT8 ucIndex);
VOS_VOID AT_Modem_EventProc(
    NAS_OM_EVENT_ID_ENUM_UINT16 enEventId,
    VOS_UINT8                   ucIndex
);
VOS_VOID AT_GetApnFromPdpContext(
    TAF_PDP_PRIM_CONTEXT_EXT_STRU      *pstPdpPrimContextEExt,
    VOS_UINT8                           ucCid
);
VOS_VOID AT_GetApnFromPdpContext(TAF_PDP_PRIM_CONTEXT_EXT_STRU *pstPdpPrimContextEExt, VOS_UINT8 ucCid);
VOS_VOID AT_MsoStatusStartTimer(TAF_UINT8 ucIndex, VOS_UINT32 ulTimer, VOS_UINT32 ulLength);
TAF_UINT32 At_SetMsoPara(TAF_UINT8 ucIndex);
VOS_UINT32 At_TestMsoPara(TAF_UINT8 ucIndex);

VOS_VOID AT_ProcSimresetInd(
    VOS_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU           *pEvent
);
VOS_VOID  AT_ProcCsRspEvtAlerting(
    TAF_UINT8                           ucIndex,
    MN_CALL_INFO_STRU                  *pstCallInfo
);

/*引入的外部函数*/
extern VOS_UINT32 AT_CheckRptCmdStatus(VOS_UINT8 *pucRptCfg,
                                       AT_CMD_RPT_CTRL_TYPE_ENUM_UINT8 enRptCtrlType,
                                       AT_RPT_CMD_INDEX_ENUM_UINT8 enRptCmdIndex);
extern AT_PS_USER_INFO_STRU* AT_PS_GetUserInfo(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);
extern VOS_UINT32 AT_PS_HangupCall(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);
extern VOS_UINT32 At_SetDhcpPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryDhcpPara(VOS_UINT8 ucIndex);
extern TAF_VOID At_CsIncomingEvtOfIncomeStateIndProc(
    TAF_UINT8                           ucIndex,
    MN_CALL_EVENT_ENUM_U32              enEvent,
    MN_CALL_INFO_STRU                   *pstCallInfo);
MDRV_VER_SOLUTION_TYPE mdrv_ver_get_solution_type(void);
#endif /* FEATURE_ON == MBB_WPG_MODULE_AT */

#if (FEATURE_ON == MBB_WPG_SIMSQ)
extern NV_HUAWEI_MBB_FEATURE_STRU* At_GetMbbFeature(VOS_VOID);
extern VOS_UINT8 At_GetMbbFeatureSimsqEnable(VOS_VOID);
extern VOS_VOID At_SetMbbFeatureSimsqEnable(VOS_UINT8 ucSimsqEnable);
VOS_VOID AT_ProcReportSimSqInfo( VOS_UINT8 ucIndex, VOS_UINT8 ucSimsq);
VOS_VOID AT_ProcSimsqInd(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pstEvent);
VOS_UINT32 AT_QryParaRspSimsqPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetParaRspSimsqPara(TAF_UINT8 ucIndex);
#endif/*FEATURE_ON == MBB_WPG_SIMSQ*/

#if (FEATURE_ON == MBB_WPG_CELLLOCK)
VOS_VOID AT_ProcSystemInfoCellLockInd(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pstEvent);
VOS_BOOL AT_IsCellLockForbidCmdProc(AT_PAR_CMD_ELEMENT_STRU* pstCmdElement );
VOS_VOID AT_InitCellLockCtx(VOS_VOID);
VOS_UINT32 AT_IsCellLockAllowedAreaFlag(
    AT_CELLLOCK_STATUS_INFO            *pstCurStatus,
    AT_CELLLOCK_CONFIG_STRU            *pstCellLockPara);
VOS_UINT32 At_SetCellLockPara(TAF_UINT8 ucIndex);
VOS_UINT32 AT_QryCellLockPara(TAF_UINT8 ucIndex);
VOS_UINT32 AT_TestCellLockPara(VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_WPG_CELLLOCK */
#if (FEATURE_ON == MBB_WPG_M2M_XCELL)
VOS_UINT32  AT_SetXCellInfoPara (VOS_UINT8 ucIndex);
VOS_UINT32  AT_QryXCellInfoPara (VOS_UINT8 ucIndex);
#endif
#if (FEATURE_ON == MBB_WPG_NETMONITOR)
VOS_UINT32 At_SetNetMonSCellPara(VOS_UINT8 ucIndex);
VOS_UINT32 At_SetNetMonNCellPara(VOS_UINT8 ucIndex);
UINT32 AT_RcvMtaSetNetMonSCellCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvMtaSetNetMonNCellCnf(VOS_VOID *pMsg);
VOS_VOID    AT_NetMonFmtGsmSCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstSCellInfo, VOS_UINT16 *pusLength);
VOS_VOID    AT_NetMonFmtUtranFddSCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstSCellInfo, VOS_UINT16 *pusLength);
VOS_VOID    AT_NetMonFmtGsmNCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstNCellInfo, VOS_UINT16 usInLen,VOS_UINT16 *pusOutLen);
VOS_VOID    AT_NetMonFmtUtranFddNCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstNCellInfo, VOS_UINT16 usInLen,VOS_UINT16 *pusOutLen);
VOS_VOID    AT_NetMonFmtPlmnId(VOS_UINT32 ulMcc, VOS_UINT32 ulMnc, VOS_CHAR *pstrPlmn, VOS_UINT16 *pusLength);
VOS_VOID    AT_NetMonFmtUtranTddSCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstSCellInfo, VOS_UINT16 *pusLength);
VOS_VOID    AT_NetMonFmtUtranTddNCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstNCellInfo, VOS_UINT16 usInLen,VOS_UINT16 *pusOutLen);
VOS_VOID    AT_NetMonFmtEutranSCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstSCellInfo, VOS_UINT16 *pusLength);
VOS_VOID    AT_NetMonFmtEutranNCellData(MTA_AT_NETMON_CELL_INFO_STRU *pstNCellInfo, VOS_UINT16 usInLen,VOS_UINT16 *pusOutLen);
#endif /* FEATURE_ON == MBB_WPG_NETMONITOR */

#if (FEATURE_ON == MBB_EUICC)
TAF_VOID AT_MsgProcFromUsimm(VOS_VOID  *pstMsg);
VOS_VOID AT_GetEIDFromUsimm(VOS_VOID *pMsg);
VOS_VOID AT_InitEIDInfo(VOS_VOID);
#endif

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define AT_LED_ALWAYS_ON_OFF_PARA_NUM                       (3)
#define AT_LED_FLICKER_SINGLE_PARA_NUM                      (5)
#define AT_LED_FLICKER_DOUBLE_PARA_NUM                      (7)
#define LED_FLICKER_MAX_NUM_PER_CYCLE                       (2)
#define LED_DEFAULT_FLICKER_TIME                            (0xFF)
#define LED_COLOR_NULL                                      (0)
#define AT_SERVICE_STAT_ANY                                 (0x3FFFFFFF)
#define LED_SERVICE_STATE_DEFAUNT                           (0x00007FFF)
#define AT_LED_SERVICE_STATE_NUM                            (15)


enum AT_M2M_LED_MODE_ENUM
{
    AT_M2M_LED_MODE_CLOSED  = 0,                                                /* 禁用闪灯 */
    AT_M2M_LED_MODE_DEFAULT = 1,                                                /* 华为默认闪灯方案 */
    AT_M2M_LED_MODE_USER    = 2,                                                /* 用户自定义闪灯方案 */
    AT_M2M_LED_MODE_BUTT
};
typedef VOS_UINT8 AT_M2M_LED_MODE_ENUM_UINT8;


enum AT_M2M_LED_FLICKER_TYPE_ENUM
{
    AT_LED_FLICKER_TYPE_ALWAYS  = 0,                                                /* 常亮、常灭 */
    AT_LED_FLICKER_TYPE_SINGLE  = 1,                                                /* 单闪 */
    AT_LED_FLICKER_TYPE_DOUBLE  = 2,                                                /* 双闪 */
    AT_LED_FLICKER_TYPE_BUTT
};
typedef VOS_UINT8 AT_LED_FLICKER_TYPE_ENUM_UINT8;

typedef struct
{
    VOS_UINT8                           ucOnDuration;                          /* 闪灯维持灯亮的时长 */
    VOS_UINT8                           ucOffDuration;                         /* 闪灯维持灯灭的时长 */
}AT_M2M_LED_FLICKER_TIME_STRU;


typedef struct
{
    VOS_UINT8                           ucLedFlickerType;                       /* 闪灯闪烁方式 */
    VOS_UINT8                           ucRsv[3];                               /* 保留位 */
    AT_M2M_LED_FLICKER_TIME_STRU        stLedFlickerTime[2];                    /* 闪灯亮灭时长 */
}AT_M2M_LED_FLICKER_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          UlLedStateMask;                         /* 服务状态位域 */
    VOS_UINT8                           ucLedIndex;                             /* 闪灯GPIO管脚配置 */
    VOS_UINT8                           ucRsv[3];                               /* 保留位 */
    AT_M2M_LED_FLICKER_CTRL_STRU        stLedCtrl;                              /* 闪灯的闪烁方式 */
}AT_M2M_LED_CONTROL_STRU;


typedef struct
{
    VOS_UINT8                           ucLedConfigNum;                             /* 用户闪灯配置组数 */
    VOS_UINT8                           ucRsv[3];                                   /* 保留位 */
    AT_M2M_LED_CONTROL_STRU             stLedConfig[AT_LED_SERVICE_STATE_NUM];      /* 用户配置的闪灯模式 */
}AT_M2M_LED_CONFIG_STRU;

VOS_UINT32 At_TestLedCtrlPara(VOS_UINT8 ucIndex);
VOS_UINT32 At_SetLedCtrlPara(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryLedCtrlPara(VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_FEATURE_M2M_LED */
#if(FEATURE_ON == MBB_WPG_SIM_SWITCH)
TAF_UINT32 TAF_SetSimSwitch(
    MN_CLIENT_ID_T                     ClientId,
    MN_OPERATION_ID_T                  OpId,
    TAF_UINT32                         ulSetValue
);
#endif/* FEATURE_ON == MBB_WPG_SIM_SWITCH */

#if(FEATURE_ON == MBB_WPG_SHARE_APN)
VOS_UINT32  AT_SetAppFilterPara (VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_WPG_SHARE_APN */

#if (FEATURE_ON == MBB_WPG_MODULE_AT)
VOS_UINT32 At_QryLteDplxPara(TAF_UINT8 ucIndex);

VOS_VOID At_QryParaLteDplxProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara);

VOS_VOID At_MBBCmdProcCharA(AT_PARSE_CONTEXT_STRU* pstClientContext, VOS_UINT8* pData, VOS_UINT32 ulLen);
TAF_UINT32 TAF_ProcessUnstructuredSSTranModeReq(MN_CLIENT_ID_T ClientId, MN_OPERATION_ID_T OpId,
                                                TAF_SS_PROCESS_USS_REQ_STRU *pPara, VOS_UINT16 usUssdTransMode);

#endif /* FEATURE_ON == MBB_WPG_MODULE_AT */

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

#endif /* end of AtCmdImsProc.h */

