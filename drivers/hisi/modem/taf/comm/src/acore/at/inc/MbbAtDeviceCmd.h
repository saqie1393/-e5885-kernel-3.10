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



#ifndef __MBB_AT_DEVICE_CMD_H__
#define __MBB_AT_DEVICE_CMD_H__

/*****************************************************************************
  1 头文件
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#if(FEATURE_ON == MBB_COMMON)


/* ^PSTANDBY命令解MAIN锁接口 */
#define LOCK_BUF_LEN       32

typedef enum
{
    APP_MAIN_LOCK,
    APP_MAIN_UNLOCK,
} app_main_lock_e;

/* 命令包含的参数个数 */
typedef enum
{
    PARA_NUMBER_ONE        = 1,
    PARA_NUMBER_TWO        = 2,
    PARA_NUMBER_THREE      = 3,
    PARA_NUMBER_FOUR       = 4,
    PARA_NUMBER_TWELVE     = 12,
    PARA_NUMBER_THIRTEEN   = 13,
}guc_At_Para_Index;

/* 记录设置或是读取nv时返回的状态 */
typedef enum
{
    CHECK_FIX_NUMBER_ONE        = 1,
    CHECK_FIX_NUMBER_TWO        = 2,
    CHECK_FIX_NUMBER_THREE      = 3,
    CHECK_FIX_NUMBER_FIVE       = 5,
    CHECK_FIX_NUMBER_SIX        = 6,
    CHECK_FIX_NUMBER_SEVEN      = 7,
    CHECK_FIX_NUMBER_EIGHT      = 8,
    CHECK_FIX_NUMBER_NIGN       = 9,
    CHECK_FIX_NUMBER_TEN        = 10,
}wifi_Rf_Chk_Fix;

typedef enum
{
    CHECK_FIX_LEN_TWO          = 2,
    CHECK_FIX_LEN_SIX          = 6,
    CHECK_FIX_LEN_TWENTYFOUR   = 24,
    CHECK_FIX_LEN_FORTYEIGHT   = 48,
}wifi_Rf_Chk_Fix_Len;

static const char *off_state = "mem";
static const char *on_state = "on";
static const char *screen_state_path = "/sys/power/autosleep";


#if(FEATURE_ON == MBB_USB)
typedef enum _port_Type
{
    MODEMNUM       = 0,
    NDISNUM        = 1,
    PCUINUM,
    GPSNUM,
    MAXPORTTYPENUM
}port_Type;

extern VOS_UINT8 gportTypeNum[MAXPORTTYPENUM];
#endif


/*****************************************************************************
  对外接口
*****************************************************************************/

#if(FEATURE_ON == MBB_SIMLOCK_FOUR)
VOS_INT32 AT_QryOemLockEnable(VOS_VOID);
#endif

VOS_UINT32 At_RegisterDeviceMbbCmdTable(VOS_VOID);

VOS_UINT32 At_WriteVersionINIToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
);

VOS_UINT32 At_WriteWebCustToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
);

VOS_UINT32 At_CheckWiwepHex();

void version_info_build(VOS_UINT8 ucIndex, VOS_UINT8  *pucKey, VOS_UINT8  *pucValue);
void version_info_fill(VOS_UINT8  *pucDes, VOS_UINT8  *pucSrc);

int set_screen_state(app_main_lock_e on);
int set_key_press_event(VOS_VOID);

VOS_UINT32 Mbb_AT_SetWiFiRxPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiEnable(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiModePara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiBandPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiFreqPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiRatePara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiPowerPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiTxPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiPacketPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWifiInfoPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWifiPaRangePara(VOS_VOID);

VOS_UINT32 Mbb_AT_SetTmmiPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetChrgEnablePara(VOS_VOID);

#if (FEATURE_ON == MBB_MODULE_PM)
TAF_UINT32   Mbb_At_SetCurcPara(TAF_UINT8 ucIndex);
#endif
VOS_UINT32  Mbb_AT_SetPhyNumPara(AT_PHYNUM_TYPE_ENUM_UINT32 enSetType, MODEM_ID_ENUM_UINT16 enModemId);

#if(FEATURE_ON == MBB_USB)
VOS_VOID At_SecCheckSamePortNUM(VOS_UINT32  ucTempnum);
VOS_VOID clearportTypeNum(VOS_VOID);
#endif

TAF_UINT32  Mbb_AT_SetFDac_Para_Valid(TAF_VOID);

VOS_UINT32 Mbb_AT_QryTmmiPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryChrgEnablePara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWifiPaRangePara (VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiPacketPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiRxPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiRatePara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiFreqPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiBandPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiModePara(VOS_UINT8 ucIndex);
VOS_UINT32  Mbb_At_QryVersion(VOS_UINT8 ucIndex );
TAF_UINT32 Mbb_At_QryCurcPara(TAF_UINT8 ucIndex);
#define WIFI_2G_NV_ID  50601
#define WIFI_5G_NV_ID  50602
VOS_UINT32 SetWiFiSsidParaCheck(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetWiFiSsidPara(VOS_UINT8 ucIndex);
VOS_UINT32 SetWiFiKeyParaCheck(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetWiFiKeyPara(VOS_UINT8 ucIndex);
#if (FEUATRE_ON == MBB_NFC)
VOS_UINT32 AT_SetNFCCFGPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryNFCCFGPavars(VOS_UINT8 ucIndex);
VOS_UINT32 At_TestNFCCFGPara(VOS_UINT8 ucIndex);
#endif

#if(FEATURE_ON == MBB_HSUART_CMUX)
VOS_UINT32 SetCmuxParaOperating_mode(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaSubset(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaPort_speed(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaMax_frame_size_N1(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaResponse_timer_T1(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaMax_cmd_num_tx_times_N2(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaResponse_timer_T2(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaResponse_timer_T3(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
VOS_UINT32 SetCmuxParaWindow_size_k(cmux_info_type *AtCmuxPara, VOS_UINT16 usParaLen, VOS_UINT32 ulParaValue);
#endif/*#if(FEATURE_ON == MBB_HSUART_CMUX)*/

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)

VOS_UINT32 At_SetSysDown(VOS_UINT8 ucIndex);
#endif


#endif/*#if(FEATURE_ON == MBB_COMMON)*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__MBB_AT_DEVICE_CMD_H__*/
