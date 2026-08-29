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
**/


#ifndef __MBB_COMBINE_AT_CORE_H__
#define __MBB_COMBINE_AT_CORE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "msp_diag_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4) /*4字节对齐*/

#if (FEATURE_ON == MBB_WPG_COMBINE_AT)
/*目前暂定为三个，在不够用时请扩展*/
#define     AT_COMBINE_TABLE_CMD_NUM                 3

/*ATAGENT使用的AT通道，请选择一个AT通道给ATAGENT专用，没有其他人使用的通道*/
#define     AT_COMBINE_SPEC_CLIENT_INDEX    AT_CLIENT_ID_APP1

#define     IS_AT_COMBINE_SPEC_CLIENT_INDEX(ucIndex)       (AT_COMBINE_SPEC_CLIENT_INDEX == ucIndex)

#define     AT_COMBINE_OPTION_MASK        (((VOS_UINT32)AT_CMD_COMBINE_SET_CMDGROUP) << 16)
#define     SET_AT_COMBINE_CMD_OPTION(ucIndex,cmdIndex)   \
                gastAtClientTab[ucIndex].CmdCurrentOpt = (AT_CMD_CURRENT_OPT_ENUM)(AT_COMBINE_OPTION_MASK | (cmdIndex & 0xffff)) /*高位MASK,低位命令号*/
#define     GET_AT_COMBINE_CMD_OPTION(ucIndex)      gastAtClientTab[ucIndex].CmdCurrentOpt

#define     IS_CUR_AT_COMBINE_CMD_OPTION(ucIndex)    \
                (AT_COMBINE_OPTION_MASK == (((VOS_UINT32)gastAtClientTab[ucIndex].CmdCurrentOpt) & 0xffff0000)) /*判断高位MASK*/

#define     GET_AT_COMBINE_CMD_OPTION_CMDIDX(ucIndex)    \
                (((VOS_UINT32)gastAtClientTab[ucIndex].CmdCurrentOpt) & 0x0000ffff) /*取低位放置的命令号*/

#define     AT_COMBINE_LOCAL_BUFFER_CNT             2
#define     AT_COMBINE_LOCAL_BUFFER_LEN             256

#define     AT_COMBINE_BUFFER_SIZE_128               128
#define     AT_COMBINE_BUFFER_SIZE_256               256

#define     AT_COMBINE_STATUS_INVALID                0x00
#define     AT_COMBINE_STATUS_CMD_FOUND              0x01
#define     AT_COMBINE_STATUS_PREPARSE_OK            0x02
#define     AT_COMBINE_STATUS_CMD_SEND               0x03

#define     GET_AT_COMBINE_STATUS()                    (gstAtAgentCtx.enCurStatus)
#define     GET_AT_COMBINE_CLIENT_INDEX()              gstAtAgentCtx.ucIndex
#define     GET_AT_COMBINE_CMD_TYPE()                  gstAtAgentCtx.ucCmdOptType
#define     GET_AT_COMBINE_CMD_PARAM_COUNT()           gstAtAgentCtx.ucParaCount
#define     GET_AT_COMBINE_CMD_INDEX()                 gstAtAgentCtx.ulCmdIndex
#define     GET_AT_COMBINE_TABLE_INDEX()               gstAtAgentCtx.ulCmdTableIndex
#define     GET_AT_COMBINE_MAINTABLE_INDEX()           gstAtAgentCtx.ulMainActionTableIndex
#define     GET_AT_COMBINE_LOCAL_BUFFER(bufIndex)      &gstAtAgentCtx.aucTempData[bufIndex % AT_COMBINE_LOCAL_BUFFER_CNT][0]
#define     GET_AT_COMBINE_PRIVATE_BUFFER()            gstAtAgentCtx.pucPrivateData
#define     IS_AT_COMBINE_STATUS_VALID()               (AT_COMBINE_STATUS_INVALID != gstAtAgentCtx.enCurStatus)
#define     GET_AT_COMBINE_FLAGS()                     gstAtAgentCtx.ulFlags
#define     CHECK_AT_COMBINE_FLAGS(bits)               (0 != (gstAtAgentCtx.ulFlags & (bits)))

#define     SET_AT_COMBINE_STATUS(enStatus)             gstAtAgentCtx.enCurStatus  = enStatus
#define     SET_AT_COMBINE_CLIENT_INDEX(ucIdx)          gstAtAgentCtx.ucIndex      = ucIdx
#define     SET_AT_COMBINE_CMD_TYPE(ucCmdType)          gstAtAgentCtx.ucCmdOptType = ucCmdType
#define     SET_AT_COMBINE_CMD_PARAM_COUNT(ucCount)     gstAtAgentCtx.ucParaCount  = ucCount
#define     SET_AT_COMBINE_CMD_INDEX(ulCmdIndex)        gstAtAgentCtx.ulCmdIndex   = ulCmdIndex
#define     SET_AT_COMBINE_TABLE_INDEX(ulIndex)         gstAtAgentCtx.ulCmdTableIndex        = ulIndex
#define     SET_AT_COMBINE_MAINTABLE_INDEX(ulIndex)     gstAtAgentCtx.ulMainActionTableIndex = ulIndex
#define     ADD_AT_COMBINE_FLAGS(bits)                  gstAtAgentCtx.ulFlags |= bits
#define     RESET_AT_COMBINE_FLAGS()                    gstAtAgentCtx.ulFlags = 0

#define     SET_AT_COMBINE_PRIVATE_BUFFER(pbuffer, beMalloc)    do{ \
                                                                    gstAtAgentCtx.pucPrivateData = pbuffer;\
                                                                    gstAtAgentCtx.ucMallocPrivData = beMalloc;\
                                                                }while(0)

#define     AT_COMBINE_TABLE_SIZE(tableName)                    (sizeof(tableName) / sizeof(tableName[0]))

#define   AT_COMBINE_LOG(Level, String) \
          (void)DIAG_LogReport( DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (Level)), \
                          (WUEPS_PID_AT), __FILE__, __LINE__, "%s", (String) )

#define   AT_COMBINE_LOG1(Level, String, Para1) \
          (void)DIAG_LogReport( DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (Level)), \
                          (WUEPS_PID_AT), __FILE__, __LINE__, "%s %d", (String), (VOS_INT32)(Para1) )

#define   AT_COMBINE_LOG2(Level, String, Para1, Para2) \
          (void)DIAG_LogReport( DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (Level)), \
                          (WUEPS_PID_AT), __FILE__, __LINE__, "%s %d %d", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2) )

typedef struct{
    VOS_UINT32        enCurStatus;
    VOS_UINT8         ucIndex;
    VOS_UINT8         ucCmdOptType;
    VOS_UINT8         ucParaCount;
    VOS_UINT8         ucMallocPrivData;
    VOS_UINT32        ulFlags;
    VOS_UINT32        ulCmdIndex;
    VOS_UINT32        ulCmdTableIndex;
    VOS_UINT32        ulMainActionTableIndex;
    VOS_UINT16        usSaveBroadCastDataLen;
    VOS_UINT16        usSaveBroadCastBufIndex;
    AT_PARSE_PARA_TYPE_STRU        *pstParaBuffer;
    VOS_VOID                       *pucPrivateData;
    VOS_UINT8                       aucTempData[AT_COMBINE_LOCAL_BUFFER_CNT][AT_COMBINE_LOCAL_BUFFER_LEN];
}AT_COMBINE_MAIN_CTX;

extern AT_COMBINE_MAIN_CTX        gstAtAgentCtx;

typedef enum{
    DATA_TYPE_RESPONSE     = 0,
    DATA_TYPE_BROADCASE    = 1,
}AT_COMBINE_RSP_DATA_TYPE_ENUM;
typedef VOS_UINT8    AT_COMBINE_RSP_DATA_TYPE_ENUM_UINT8;

typedef enum{
    AT_COMBINE_CMD_TYPE_SET    = 0,
    AT_COMBINE_CMD_TYPE_QUERY,
    AT_COMBINE_CMD_TYPE_TEST,
}AT_COMBINE_CMD_TYPE;
typedef VOS_UINT32    AT_COMBINE_CMD_TYPE_ENUM_UINT32;

typedef enum{
    AT_COMBINE_ERR_SEND_CMD      = 0,
    AT_COMBINE_ERR_MATCH_DATA,
    AT_COMBINE_ERR_TIMEOUT,
    AT_COMBINE_ERR_ABORT,
    AT_COMBINE_ERR_TABLE_INDEX,
    AT_COMBINE_ERR_NO_RESPONSE,
    AT_COMBINE_ERR_MATCH_OK,
}AT_COMBINE_ERR_TYPE;
typedef VOS_UINT32    AT_COMBINE_ERR_TYPE_ENUM_UINT32;

typedef enum{
    AT_COMBINE_PARA_BASE   = 0,
    AT_COMBINE_PARA_1,
    AT_COMBINE_PARA_2,
    AT_COMBINE_PARA_3,
    AT_COMBINE_PARA_4,
    AT_COMBINE_PARA_5,
    AT_COMBINE_PARA_6,
    AT_COMBINE_PARA_7,
    AT_COMBINE_PARA_8,
    AT_COMBINE_PARA_BUTT,
}AT_COMBINE_PARAM_ENUM;
typedef VOS_UINT32    AT_COMBINE_PARAM_ENUM_UINT32;

typedef VOS_UINT32    (*pfATCombine_sendCmd)(VOS_UINT8 ucIndex, const VOS_UINT8 *pucCmdData);
typedef VOS_UINT32    (*pfATAgent_CmdPreParse)(VOS_UINT8 ucIndex);
typedef VOS_UINT32    (*pfATAgent_CmdOnError)(VOS_UINT8 ucIndex, AT_COMBINE_ERR_TYPE_ENUM_UINT32 enType);
typedef VOS_UINT32    (*pfATCombine_RspMatch)(
    VOS_UINT8                            ucIndex,
    AT_COMBINE_RSP_DATA_TYPE_ENUM_UINT8    eRspType,
    VOS_UINT16                           usRspDataLen,
    const VOS_UINT8                     *pucRspData,
    const VOS_UINT8                     *pucMatchData
);

typedef struct{
    const pfATCombine_sendCmd     pfCmdAction;
    const VOS_CHAR                     *pucCmdData;
    const pfATCombine_RspMatch    pfResponseMatch;
    const VOS_CHAR                     *pucMatchData;
}AT_COMBINE_ACTION_MAIN_TABLE_ITEM_ST;

typedef struct{
    VOS_UINT32                                 ulCmdIndex;
    AT_COMBINE_CMD_TYPE_ENUM_UINT32            enCmdType;
    pfATAgent_CmdPreParse                      pstPreProc;
    pfATAgent_CmdOnError                      pstOnErrProc;
    AT_COMBINE_ACTION_MAIN_TABLE_ITEM_ST      *pstMainTable;
    VOS_UINT32                                 ulTableSize;
}AT_COMBINE_CMD_TABLE_ST;

VOS_UINT32 AT_Combine_CmdProc(VOS_UINT8 ucIndex);
VOS_BOOL AT_Combine_ResultDataProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pData,
    VOS_UINT16                          usLen
);
VOS_VOID AT_Combine_TimeOutProc(VOS_UINT8 ucIndex, VOS_UINT32 ulName);
VOS_BOOL CombineAt_IsRunning(VOS_UINT8 ucIndex);
VOS_BOOL CombineAt_IsCmdRunning(VOS_UINT32 ulCmdIndex);
VOS_VOID CombineAt_DoAbort(VOS_UINT8 ucIndex);

VOS_VOID AT_Combine_SendDebugData(VOS_CHAR *str_title, VOS_CHAR *str_var, VOS_UINT32 int_var);

VOS_UINT32 AT_Combine_SaveBroadCastDataToLocal(
    const VOS_UINT8 *pucBroadCastData,
    VOS_UINT16 usCastDataLen,
    VOS_UINT8   ucLocalBufIndex
);
VOS_UINT32 AT_Combine_SendAtCmd(
    VOS_UINT8   ucIndex,
    VOS_UINT8 *pucCmdData,
    VOS_UINT16 usCmdLength
);

VOS_BOOL AT_Combine_MatchData(
    VOS_UINT8                            ucIndex,
    AT_COMBINE_RSP_DATA_TYPE_ENUM_UINT8    eRspType,
    VOS_UINT16                           usRspDataLen,
    const VOS_UINT8                     *pucRspData,
    const VOS_UINT8                     *pucMatchData
);
VOS_BOOL AT_Combine_MatchRspOk(
    VOS_UINT8                            ucIndex,
    AT_COMBINE_RSP_DATA_TYPE_ENUM_UINT8    eRspType,
    VOS_UINT16                           usRspDataLen,
    const VOS_UINT8                     *pucRspData,
    const VOS_UINT8                     *pucMatchData
);
VOS_UINT32  AT_CombineGetParam(
    const VOS_UINT8 *pucData,
    VOS_UINT16 usDataLen,
    VOS_UINT32 ulParaIndex,
    VOS_UINT8 *pucOutParam,
    VOS_UINT16 usOutBufferLen
);
VOS_VOID AT_Combine_GetCurStep(VOS_UINT32 *pulStatus, VOS_UINT32 *pulActionIndex);
VOS_VOID AT_Combine_SkipNextStep(VOS_UINT32 ulStipStep);
VOS_BOOL AT_Combine_PreParseOk(VOS_UINT8 ucIndex);
VOS_UINT32 AT_Combine_SendCmdData(VOS_UINT8 ucIndex, const VOS_UINT8 *pucCmdData);
VOS_VOID AT_Combine_CmdMsgResponse(VOS_UINT8 ucIndex, VOS_UINT32 ulResult, VOS_UINT16 usLength, VOS_UINT8 *pucData);
VOS_UINT32 AT_Combine_ATMsgRspDataSend(VOS_UINT8 ucIndex, VOS_UINT16 usLength, VOS_UINT8 *pucData);
VOS_UINT32 AT_Combine_ATMsgRspSend(VOS_UINT8 ucIndex, VOS_UINT32 ulResultCode);
VOS_UINT32 AT_Combine_ATMsgIndSend(VOS_UINT8 ucIndex, VOS_UINT16 usLength, VOS_UINT8 *pucData);
VOS_UINT32 AT_Combine_DoOriCmd(VOS_UINT8 ucIndex, const VOS_UINT8 *pucCmdData);
VOS_UINT32 AT_Combine_SaveParaList(VOS_UINT8 ucIndex);
VOS_UINT32 AT_Combine_RegisterCmdTable(AT_COMBINE_CMD_TABLE_ST *pstCombineCmdTable, VOS_UINT32 ulCount);

#endif/* FEATURE_ON == MBB_WPG_COMBINE_AT */


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

