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


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mdrv_hkadc_common.h"
#include "AtCheckFunc.h"
#include "LPsNvInterface.h"
#include "AtTestParaCmd.h"
#include "AtSetParaCmd.h"
#include "AtDeviceCmd.h"
#include "GasNvInterface.h"
#include "mdrv_temp_cfg.h"
#include "bsp_shared_ddr.h"
#include "at_lte_common.h"
#include "mdrv_chg.h"
#include "MbbAtDeviceCmd.h"
#include "product_nv_id.h"
#include "drv_nv_id.h"
#include "AtCmdMsgProc.h"
#include "mbb_process_start.h"
/*add by wanghaijie for mbb_leds*/


#include "mdrv_nfc.h"


#include <linux/mlog_lib.h>

/*****************************************************************************
  1 宏定义
*****************************************************************************/
/*修改wifi校准温度返回值按照AT规范返回*/

#define   WICALTEMPNUM             (1)


#define    THIS_FILE_ID        PS_FILE_ID_AT_DEVICECMD_C

#define                           SetOemOprLen       (5)
#define                           SetSimlockOprLen   (9)

#define OUTPUT_BUF_LENGTH                               (64)    /*数组长度*/


typedef enum {
    REVERT_FAIL = 0,
    REVERT_SUCCESS = 1
}REVERT_NV_FLAG;
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
VOS_UINT32              g_SetTseLrfParValue = 0;
/*LTE测试时，选择的分集天线号*/
VOS_UINT32 g_SetLteTseLrfSubParValue = 0;


/*记录当前是否下发过AT^FTYRESET设置命令*/
static SET_FTYRESET_TYPE g_set_ftyreset_flag = NOT_SET;





/*AT^VERSION查询版本号处理函数指针*/
typedef VOS_INT32 (*AT_VERSION_QRY_PROC_FUN)(VOS_UINT8 *pt_version);
typedef struct
{
    VOS_UINT32 index;
    VOS_UINT8* version_type;
    AT_VERSION_QRY_PROC_FUN pFunc;
    VOS_UINT32 reserved;
}AT_VERSION_TAB_STRU;
extern VOS_UINT32 AT_SetTmodeAutoPowerOff(VOS_UINT8 ucIndex);
extern TAF_UINT8 At_GetDspLoadMode(VOS_UINT32 ulRatMode);

extern VOS_UINT32                 g_ulNVRD;
extern VOS_UINT32                 g_ulNVWR;

extern VOS_VOID AT_GetNvRdDebug(VOS_VOID);
static VOS_UINT32 at_version_get_bdt(VOS_UINT8* bdt_ver);
static VOS_UINT32 at_version_get_exts(VOS_UINT8* exts_ver);
static VOS_UINT32 at_version_get_ints(VOS_UINT8* ints_ver);
static VOS_UINT32 at_version_get_extd(VOS_UINT8* extd_ver);
static VOS_UINT32 at_version_get_intd(VOS_UINT8* intd_ver);
static VOS_UINT32 at_version_get_exth(VOS_UINT8* exth_ver);
static VOS_UINT32 at_version_get_inth(VOS_UINT8* inth_ver);
static VOS_UINT32 at_version_get_extu(VOS_UINT8* extu_ver);
static VOS_UINT32 at_version_get_intu(VOS_UINT8* intu_ver);
static VOS_UINT32 at_version_get_cfg(VOS_UINT8* cfg_ver);
static VOS_UINT32 at_version_get_prl(VOS_UINT8* prl_ver);
static VOS_UINT32 at_version_get_ini(VOS_UINT8* ini_ver);
VOS_UINT8 gportTypeNum[MAXPORTTYPENUM] = {0};

extern TAF_UINT32 At_MacConvertWithColon(TAF_UINT8 *pMacDst, TAF_UINT8 *pMacSrc, TAF_UINT16 usSrcLen);
extern void report_power_key_up_for_suspend(void);
extern void report_power_key_down_for_suspend(void);


AT_VERSION_TAB_STRU g_at_versiont_config_tab[] =
{
    /*lint -e64*/
    {0, "BDT", at_version_get_bdt, 0},
    {1, "EXTS", at_version_get_exts, 0},
    {2, "INTS", at_version_get_ints, 0},
    {3, "EXTD", at_version_get_extd, 0},
    {4, "INTD", at_version_get_intd, 0},
    {5, "EXTH", at_version_get_exth, 0},
    {6, "INTH", at_version_get_inth, 0},
    {7, "EXTU", at_version_get_extu, 0},
    {8, "INTU", at_version_get_intu, 0},
    {9, "CFG", at_version_get_cfg, 0},
    {10, "PRL", at_version_get_prl, 0},
    {12, "INI", at_version_get_ini, 0},
    /*lint +e64*/
};
static REVERT_NV_FLAG g_revert_flag = REVERT_FAIL;
/*****************************************************************************
  3 函数定义
*****************************************************************************/

VOS_UINT32 At_TestFlnaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_ERROR;
    }
    
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-5 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:6,0,1,2,3,4,5",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* WDSP LNA等级取值为0-3 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:4,0,1,2,3",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    /* 参数LNA等级取值范围检查 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-2 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:3,0,1,2",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    if (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* WDSP LNA等级取值为0-3 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:256,(0-255)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32  At_SetVersionPara(VOS_UINT8 ucIndex )
{
    VOS_UINT8 iniVersion[AT_VERSION_INI_LEN] = {0};
    
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_ERROR;
    }
    
    /* 版本号类型目前只支持INI写入 */
    if (3 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }
    
    if (gastAtParaList[1].usParaLen >= AT_VERSION_INI_LEN)
    {
        return AT_ERROR;
    }
    
    /*lint -e64*/
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "INI", gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }
    /*lint +e64*/
    
    MBB_MEM_CPY_S(iniVersion, AT_VERSION_INI_LEN, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
    iniVersion[gastAtParaList[1].usParaLen] = '\0';
    
    if (NV_OK != NV_Write(en_NV_Item_PRI_VERSION, iniVersion, sizeof(iniVersion)))
    {
        AT_ERR_LOG("At_SetVersion: Write NV fail");
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


void version_info_build(VOS_UINT8 ucIndex, VOS_UINT8  *pucKey, VOS_UINT8  *pucValue)
{
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%s:%s%s\r\n",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pucKey,
                                          pucValue,
                                          gaucAtCrLf);
}


void version_info_fill(VOS_UINT8  *pucDes, VOS_UINT8  *pucSrc)
{
    MBB_MEM_SET_S(pucDes, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    MBB_STR_NCPY_S(pucDes, TAF_MAX_VERSION_VALUE_LEN,
                pucSrc, MBB_STR_NLEN(pucSrc, TAF_MAX_VERSION_VALUE_LEN)); /*lint !e64*/
}




VOS_UINT32 At_TestTmmiPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* TMMI支持的测试模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s=%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    AT_MMI_TEST_SUPPORT_MANUAL);
    return AT_OK;
}






VOS_UINT32 At_TestWiFiModePara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&strBuf, sizeof(strBuf), 0, sizeof(strBuf));
    
    /* 读取WIFI模式 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiModeSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 At_TestWiFiBandPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&strBuf, sizeof(strBuf), 0, sizeof(strBuf));

    /* 读取WIFI支持带宽 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiBandSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWifiCalPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCal(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ucWifiCal = 0;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准模式信息 */
    ucWifiCal = WlanATGetWifiCal();

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCal);
	return AT_OK;
}


VOS_UINT32 AT_TestWifiCalPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ucWifiCalSupport = 0;
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /*获取wical支持参数*/
    ucWifiCalSupport = WlanATGetWifiCalSupport();

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalSupport);
    return AT_OK;
}


VOS_UINT32 AT_SetWifiCalDataPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALDATA_STRU auWiCalDataTemp;
    MBB_MEM_SET_S(&auWiCalDataTemp, sizeof(auWiCalDataTemp), 0, sizeof(auWiCalDataTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALDATA_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /*DATALOCK解锁判断*/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }



    auWiCalDataTemp.type = (WLAN_AT_WICALDATA_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalDataTemp.group = (VOS_INT32)gastAtParaList[1].ulParaValue;
    auWiCalDataTemp.mode = (WLAN_AT_WIMODE_TYPE)gastAtParaList[2].ulParaValue;
    auWiCalDataTemp.band = (WLAN_AT_WIFREQ_TYPE)gastAtParaList[3].ulParaValue;
    auWiCalDataTemp.bandwidth = (WLAN_AT_WIBAND_TYPE)gastAtParaList[4].ulParaValue;
    auWiCalDataTemp.freq = (VOS_INT32)gastAtParaList[5].ulParaValue;
    PS_MEM_CPY(auWiCalDataTemp.data,
                   gastAtParaList[6].aucPara,
                   WICALDATA_DATA_PARA_MAX);

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalData(&auWiCalDataTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalDataPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALDATA_STRU ucWifiCalDataTemp;
    MBB_MEM_SET_S(&ucWifiCalDataTemp, sizeof(ucWifiCalDataTemp), 0, sizeof(ucWifiCalDataTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalData(&ucWifiCalDataTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d,%d,%d,%d,%d,%s%s%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalDataTemp.type,ucWifiCalDataTemp.group,
                                                    ucWifiCalDataTemp.mode,ucWifiCalDataTemp.band,
                                                    ucWifiCalDataTemp.bandwidth,ucWifiCalDataTemp.freq,
                                                    gaucAtQuotation,ucWifiCalDataTemp.data,gaucAtQuotation);
	return AT_OK;
}


VOS_UINT32 AT_SetWifiCalTempPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALTEMP_STRU auWiCalTempTemp;
    MBB_MEM_SET_S(&auWiCalTempTemp, sizeof(auWiCalTempTemp), 0, sizeof(auWiCalTempTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALTEMP_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalTempTemp.index = (VOS_INT32)gastAtParaList[0].ulParaValue;
    auWiCalTempTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalTemp(&auWiCalTempTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalTempPara(VOS_UINT8 ucIndex)
{
#define   WICALTEMPNUM             (1)
    WLAN_AT_WICALTEMP_STRU ucWifiCalTempTemp;
    MBB_MEM_SET_S(&ucWifiCalTempTemp, sizeof(ucWifiCalTempTemp), 0, sizeof(ucWifiCalTempTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalTemp(&ucWifiCalTempTemp))
    {
        return AT_ERROR;
    }
    /*按照规范返回WIFI校准温度*/
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d%s%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    WICALTEMPNUM,
                                                    gaucAtCrLf,
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalTempTemp.index,ucWifiCalTempTemp.value);
	return AT_OK;
}


VOS_UINT32 AT_SetWifiCalFreqPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALFREQ_STRU auWiCalFreqTemp;
    MBB_MEM_SET_S(&auWiCalFreqTemp, sizeof(auWiCalFreqTemp), 0, sizeof(auWiCalFreqTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALFREQ_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalFreqTemp.type = (WLAN_AT_WICALFREQ_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalFreqTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalFreq(&auWiCalFreqTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalFreqPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALFREQ_STRU ucWifiCalFreqTemp;
    MBB_MEM_SET_S(&ucWifiCalFreqTemp, sizeof(ucWifiCalFreqTemp), 0, sizeof(ucWifiCalFreqTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalFreq(&ucWifiCalFreqTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalFreqTemp.type,ucWifiCalFreqTemp.value);
	return AT_OK;

}


VOS_UINT32 AT_SetWifiCalPowPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALPOW_STRU auWiCalPowTemp;
    MBB_MEM_SET_S(&auWiCalPowTemp, sizeof(auWiCalPowTemp), 0, sizeof(auWiCalPowTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALPOW_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalPowTemp.type = (WLAN_AT_WICALPOW_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalPowTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalPOW(&auWiCalPowTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalPowPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALPOW_STRU ucWifiCalPowTemp;
    MBB_MEM_SET_S(&ucWifiCalPowTemp, sizeof(ucWifiCalPowTemp), 0, sizeof(ucWifiCalPowTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalPOW(&ucWifiCalPowTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalPowTemp.type,ucWifiCalPowTemp.value);
    return AT_OK;

}


VOS_UINT32 AT_SetNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 At_TestNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_SetNavEnablePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_SetNavFreqPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavFreqPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavRxPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}




VOS_UINT32  AT_SetTnetPortPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR; 
}


VOS_UINT32 AT_QryTnetPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLen = 0; 
    VOS_UINT32 cur_index = 1;
    NET_PORT_ST test_port_para = {0};
    unsigned char port_ip_add[] = {'1','9','2','.','1','6','8','.','8','.','1'}; 
    

    if (-1 == PhyATQryPortPara(&test_port_para))
    {
        return AT_ERROR;
    }

    if(0 == MBB_STR_NLEN(test_port_para.ip_add, sizeof(test_port_para.ip_add))) /*lint !e64*/
    {
        MBB_MEM_CPY_S(test_port_para.ip_add, sizeof(test_port_para.ip_add), port_ip_add, sizeof(port_ip_add));
    }
    
    usLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            "%s:%d%s",
                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                            test_port_para.total_port,gaucAtCrLf); 
 
    for(cur_index = 0; cur_index < test_port_para.total_port; cur_index++) 
    {
        usLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                "%s:%d,%s%s",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                cur_index,
                                test_port_para.ip_add,gaucAtCrLf); 
    
        usLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                "%s:%d%s",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                test_port_para.port_rate,
                                gaucAtCrLf); 
     }
    
    gstAtSendData.usBufLen = usLen;
   
    return AT_OK;
}


VOS_UINT32 At_QryExtChargePara(VOS_UINT8 ucIndex)
{

    VOS_INT8 testRes;
    
    /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    testRes = chg_extchg_mmi_test();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%d",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 testRes);

	return AT_OK;
}


VOS_UINT32 AT_SetWebSitePara(VOS_UINT8 ucIndex)
{
    VOS_UINT8    aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1] = {0};
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度过长 */
    if (gastAtParaList[0].usParaLen > AT_WEBUI_SITE_WR_LEN_MAX)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 参数长度过短 */
    if (3 > gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    MBB_MEM_SET_S(aucWebSiteTmp, sizeof(aucWebSiteTmp), 0, sizeof(aucWebSiteTmp));
    MBB_MEM_CPY_S(aucWebSiteTmp, sizeof(aucWebSiteTmp), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /* 写入WEB SITE对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_SITE , aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWebSitePara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWebSitePara(VOS_UINT8 ucIndex )
{
   VOS_CHAR     aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1];

   /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    MBB_MEM_SET_S(aucWebSiteTmp, sizeof(aucWebSiteTmp), 0, sizeof(aucWebSiteTmp));

    if (NV_OK != NV_Read(NV_ID_WEB_SITE, aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebsite ERROR: NVIM Read en_NV_Item_Web_Site falied!");
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebSiteTmp,
                                                 gaucAtQuotation);
                    
    return AT_OK;
}



VOS_INT AT_WiFiPinChecksum(VOS_CHAR *acInputStr)
{
    VOS_UINT32  uiAccum  = 0;
    VOS_UINT32  ulValTmp = 0;
    VOS_INT16   iIndStr  = 0;

    if(NULL == acInputStr)
    {
        return VOS_ERROR;
    }
    
    if(AT_WIFI_8BIT_PIN_LEN != MBB_STR_NLEN(acInputStr, AT_WIFI_PIN_NV_LEN_MAX))
    {
        /* vos_printf("WiFiPINChecksum: ERROR, 8 != VOS_StrLen(acInputStr)!!\n"); */
        return VOS_ERROR;
    }
    
    for(iIndStr = 0; iIndStr < AT_WIFI_8BIT_PIN_LEN; iIndStr++)
    {
        if((acInputStr[iIndStr] < '0')
           || (acInputStr[iIndStr] > '9'))
        {
            /* vos_printf("WiFiPINChecksum: ERROR, acInputStr[iIndStr] = #%c# is not digit num!!\n", acInputStr[iIndStr]); */
            return VOS_ERROR;
        }
        
        ulValTmp = (VOS_UINT32)((char)acInputStr[iIndStr] - '0');/*lint !e571 */
        
        if(0 == (iIndStr % 2))
        {
            uiAccum = uiAccum + 3 * (ulValTmp % 10);
        }
        else
        {
            uiAccum = uiAccum + (ulValTmp % 10);
        }
    }
    
    if(0 == (uiAccum % 10))
    {
        return VOS_OK;
    }
    
    /* vos_printf("WiFiPINChecksum: ERROR, checksum failed!!\n"); */
    return VOS_ERROR;
}


VOS_UINT32 AT_SetWiFiPinPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8    aucWiFiPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度检查, WPS PIN 长度必须为8 或者 4 */
    if (AT_WIFI_8BIT_PIN_LEN != gastAtParaList[0].usParaLen && AT_WIFI_4BIT_PIN_LEN != gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    MBB_MEM_SET_S(aucWiFiPinTmp, sizeof(aucWiFiPinTmp), 0, sizeof(aucWiFiPinTmp));
    MBB_MEM_CPY_S(aucWiFiPinTmp, sizeof(aucWiFiPinTmp), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /**8-Bit PIN Need to checksum**/
    if (AT_WIFI_8BIT_PIN_LEN == gastAtParaList[0].usParaLen)
    {
        if(VOS_OK != AT_WiFiPinChecksum((VOS_CHAR *)aucWiFiPinTmp))
        {
            AT_WARN_LOG("AT_SetWiFiPINPara: PIN checksum failed!");/*lint !e64 */
            return AT_ERROR;
        }
    }
    
    /* 写入WiFi PIN对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WPS_PIN , aucWiFiPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWiFiPINPara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWiFiPinPara(VOS_UINT8 ucIndex )
{
   VOS_CHAR     aucWebPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];

   /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
        
    MBB_MEM_SET_S(aucWebPinTmp, sizeof(aucWebPinTmp), 0, sizeof(aucWebPinTmp));

    if (NV_OK != NV_Read(NV_ID_WPS_PIN, aucWebPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebsite ERROR: NVIM Read en_NV_Item_Web_Site falied!");
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebPinTmp,
                                                 gaucAtQuotation);
    
    return AT_OK;
}


VOS_UINT32 AT_SetWebUserPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8  aucWebUserTmp[AT_WEBUI_USER_NV_LEN_MAX + 1];
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度过长 */
    if (AT_WEBUI_USER_WR_LEN_MAX < gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (0 == gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /**数据锁保护**/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    MBB_MEM_SET_S(aucWebUserTmp, sizeof(aucWebUserTmp), 0, sizeof(aucWebUserTmp));
    MBB_MEM_CPY_S(aucWebUserTmp, sizeof(aucWebUserTmp), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /* 写入WEB USER对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_USER_NAME , aucWebUserTmp, AT_WEBUI_USER_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWebUserPara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWebUserPara(VOS_UINT8 ucIndex )
{
    VOS_CHAR     aucWebUsermp[AT_WEBUI_SITE_NV_LEN_MAX + 1];

    /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }

    MBB_MEM_SET_S(aucWebUsermp, sizeof(aucWebUsermp), 0, sizeof(aucWebUsermp));
    
    if (NV_OK != NV_Read(NV_ID_WEB_USER_NAME, aucWebUsermp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebUserPara ERROR: NVIM Read NV_ID_WEB_USER_NAME falied!");
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebUsermp,
                                                 gaucAtQuotation);

    return AT_OK;
}


#define PARA_NUM_4    (4)
#define PARA_LEN_STR    "16"
VOS_UINT32 At_SetPortLockPara(VOS_UINT8 ucIndex)
{
        return AT_OK;
}


#define PORT_LOCKED    (2)
#define PORT_UNLOCKED    (1)
#define PORT_NO_NEED_LOCK    (0)
VOS_UINT32 At_QryPortLockPara(VOS_UINT8 ucIndex)
{

    return AT_OK;
}


VOS_UINT32 AT_SetTbatDataPara(VOS_UINT8 ucIndex)
{

    AT_TBATDATA_BATTERY_ADC_INFO_STRU          stBatdata = {0};
    
    /* 参数有效性检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if ((0 == gastAtParaList[0].usParaLen)
        || (0 == gastAtParaList[1].usParaLen)
        || (0 == gastAtParaList[2].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*功能判断:若产品不支持电池，则直接返回ERROR*/
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE))
    {
        return AT_ERROR;
    }
    
    if (AT_TBATDATA_VOLTAGE == gastAtParaList[0].ulParaValue)
    {
        /* 调用底软接口设置电池电压 */
        if(AT_TBATDATA_INDEX0 == gastAtParaList[1].ulParaValue)
        {
            stBatdata.usMinAdc = gastAtParaList[2].ulParaValue;
            stBatdata.usMaxAdc = TBAT_CHECK_INVALID;
        }
        else
        {
            stBatdata.usMinAdc = TBAT_CHECK_INVALID;
            stBatdata.usMaxAdc = gastAtParaList[2].ulParaValue;
        }
        
        /* 调用^TBAT命令数字电压写接口*/
        if (CHG_OK != chg_tbat_write(CHG_AT_BATTERY_CHECK, &stBatdata))
        {
            return AT_ERROR;
        }
        
        return AT_OK;
    }
    else if (AT_TBATDATA_CURRENT == gastAtParaList[0].ulParaValue)
    {
        /* 电流校准接口有待实现 */
        return AT_ERROR;
    }
    else
    {
        return AT_ERROR;
    }


}


VOS_UINT32 At_QryTbatDataPara(VOS_UINT8 ucIndex)
{

    AT_TBAT_BATTERY_ADC_INFO_STRU stAdcInfo;
    VOS_UINT16 usLen = 0;

    /*命令状态类型检查*/
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /*功能判断:若产品不支持电池，则直接返回ERROR*/
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE) )
    {
        return AT_ERROR;
    }
    
    MBB_MEM_SET_S(&stAdcInfo, sizeof(stAdcInfo), 0, sizeof(stAdcInfo));
    
    if (CHG_OK != chg_tbat_read(CHG_AT_BATTERY_CHECK, &stAdcInfo))          
    {              
        return AT_ERROR;         
    }
    
    usLen = (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   "%s:%d,%d%s",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   VOLT_CALC_NUM_MAX,
                                   CURRENT_CALC_NUM_MAX,
                                   gaucAtCrLf);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%d,%d%s",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   AT_TBATDATA_VOLTAGE,
                                   AT_TBATDATA_INDEX0,
                                   stAdcInfo.usMinAdc,
                                   gaucAtCrLf);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%d,%d",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   AT_TBATDATA_VOLTAGE,
                                   AT_TBATDATA_INDEX1,
                                   stAdcInfo.usMaxAdc);
 
    gstAtSendData.usBufLen = usLen;

    return AT_OK;
}


VOS_UINT32 AT_QryWiFiPlatformPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIPLATFORM_TYPE   ucWifiPlatform;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    ucWifiPlatform = WlanATGetWifiPlatform();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiPlatform);
    return AT_OK;
}


ante_switch_handle ante_switch_array[SUPPORT_RF_BAND_FOR_ANTE] = 
{
        /*  射频通道             天线主分集       内置还是外置      软件支持类型      */
        {GSM_BAND_FOR_ANTE,       MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WCDMA_PRI_BAND_FOR_ANTE, PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WCDMA_SEC_BAND_FOR_ANTE, SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WIFI_BAND_FOR_ANTE,      MIMO_ANTE,      Antenna_Set_IN,   NO_SUPPORT},
        {FDD_LTE_PRI_FOR_ANTE,    PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {FDD_LTE_SEC_FOR_ANTE,    SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {FDD_LTE_MIMO_FOR_ANTE,   MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_PRI_FOR_ANTE,    PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_SEC_FOR_ANTE,    SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_MIMO_FOR_ANTE,   MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
};

VOS_UINT32 AT_SetAntennaPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
    
}


VOS_UINT32 AT_QryAntennaPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
													0
													);
    return AT_OK;
}


VOS_UINT32 At_TestAntenna(VOS_UINT8 ucIndex)
{
    TAF_CHAR  *supportedMode = "0,0";
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s", 
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 
                                                 supportedMode);

    return AT_OK;

}

#define TEMPERATURE_MAGIC         0x5A5A5A5A
#define TEMPINIT                  (-25)
#define TEMPINIT_HISI             (-250)


VOS_UINT32 AT_QryTempInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT16   usLength = 0;
    VOS_UINT8    sim_chan = 0xFF;      /*SIM卡ADC通道*/
    VOS_UINT8    usb_chan = 0xFF;      /*USBADC通道*/
    VOS_INT16    index_chan = 0;
    VOS_INT16    sim_Temp = TEMPINIT;      /*SIM卡温度*/
    VOS_INT16    usb_Temp = TEMPINIT;      /*USB温度*/
    VOS_UINT8    bat_chan = 0xFF;       /*电池ADC通道*/
    VOS_INT16    bat_Temp = TEMPINIT;       /*电池温度*/
    VOS_UINT8    xo_chan = 0xFF;       /*DCXO通道*/
    VOS_INT16    xo_Temp = TEMPINIT_HISI;       /*DCXO温度*/
    VOS_UINT8    pa_chan = 0xFF;       /*PAO通道*/
    VOS_INT16    pa_Temp = TEMPINIT_HISI;       /*PAO温度*/
    DRV_HKADC_DATA_AREA *p_area = (DRV_HKADC_DATA_AREA *)(SHM_BASE_ADDR + SHM_OFFSET_TEMPERATURE); /*lint !e124*/
    VOS_UINT8 *phy_tbl = p_area->phy_tbl;
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
 
    if((TEMPERATURE_MAGIC != p_area->magic_start) || (TEMPERATURE_MAGIC != p_area->magic_end))
    {
        (VOS_VOID)vos_printf("AT_QryTempInfo ERROR:tem mem is writed by others.\n");
        return AT_ERROR;
    }

    /* 根据对照表查询SIM卡温度、电池温度、USB温度、面壳温度对应的CHAN */
    for(index_chan = 0; index_chan < HKADC_CHAN_MAX; index_chan++)
    {
        switch(index_chan)
        {
            case HKADC_TEMP_SIM_CARD:
                sim_chan = phy_tbl[index_chan];
                sim_Temp = p_area->chan_out[sim_chan].temp_l * 10;
                break;
            case HKADC_TEMP_USB: /*lint !e142*/
                usb_chan = phy_tbl[index_chan];
                usb_Temp = p_area->chan_out[usb_chan].temp_l * 10;
                break;
            case HKADC_TEMP_BATTERY: /*lint !e142*/
                bat_chan = phy_tbl[index_chan];
                bat_Temp = p_area->chan_out[bat_chan].temp_l * 10;
                break;
            case HKADC_TEMP_XO0: /*lint !e142*/
                xo_chan = phy_tbl[index_chan];
                xo_Temp = p_area->chan_out[xo_chan].temp_l; 
                break;
              
            case HKADC_TEMP_PA0: /*lint !e142*/
                pa_chan = phy_tbl[index_chan];
                pa_Temp = p_area->chan_out[pa_chan].temp_l; 
                break;
            default:
                break;
        }
    }

    if(BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE))
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s:%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"BAT",bat_Temp, gaucAtCrLf);
    }
    else
    {
         usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"BAT", gaucAtCrLf);
    }
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s:%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"SIM",sim_Temp, gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"SD", gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s:%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"XO",xo_Temp ,gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"WPA", gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"GPA", gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"CPA", gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s:%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"LPA", pa_Temp, gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s:%d%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"USB",usb_Temp, gaucAtCrLf);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                           "%s:%s%s",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,"SHELL", gaucAtCrLf);
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}







VOS_INT32 AT_QryOemLockEnable(VOS_VOID)
{
    VOS_INT32 ulRet ;
    NV_AUHT_OEMLOCK_STWICH_SRTU         OEMLOCK;
    PS_MEM_SET(&OEMLOCK, 0, sizeof(OEMLOCK));

    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR; /*lint !e124*/
    if (NULL == smem_data)
    {
        (VOS_VOID)vos_printf("DRV_SEC_HASH_HWOEMLOCK_CODE smem_confidential_nv_opr_flag malloc fail!\n");
        return VOS_ERROR;
    }
    /*设置纪要nv授权标记，授权读取nv*/
    smem_data->smem_confidential_nv_opr_flag = SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM;

    ulRet = NV_ReadEx(MODEM_ID_0, NV_HUAWEI_OEMLOCK_I, &OEMLOCK, sizeof(OEMLOCK));
    if (NV_OK != ulRet)
    {
        (VOS_VOID)vos_printf("AT_PhyNumIsNull: Fail to read NV_HUAWEI_OEMLOCK_I");
        return VOS_ERROR;
    }
    if (DRV_OEM_SIMLOCK_ENABLE == OEMLOCK.reserved[0])
    {
        
        ulRet = VOS_ERROR;
    }
    else
    {
        
        ulRet = VOS_OK;
    }
    return ulRet;    

}



VOS_UINT32  At_SetHWLock(VOS_UINT8 ucIndex )
{
    VOS_INT32 ret = 0;
    AT_TAF_SET_HWLOCK_REQ_STRU  HWLOCK_REQ ;
    PS_MEM_SET(&HWLOCK_REQ, '\0', sizeof(AT_TAF_SET_HWLOCK_REQ_STRU));
    
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;/*是否转化为内部错误码,现在是按照装备规范只能返回OK or ERROR*/
    }
    /*参数长度判断*/
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        (VOS_VOID)vos_printf(" para len is error\n");
        return AT_ERROR;
    }
    /* 版本号类型目前只支持OEM和SIMLOCK的写入 */
    if ((SetOemOprLen != gastAtParaList[0].usParaLen) && (SetSimlockOprLen != gastAtParaList[0].usParaLen))
    {
        (VOS_VOID)vos_printf(" compare len is error\n");
        return AT_ERROR;
    }
    /*解锁密码必须为16位*/
    if (gastAtParaList[1].usParaLen != AT_HWLOCK_PARA_LEN )
    {
        return AT_ERROR;
    }

    /*lint -e64*/
    /* 密码类型目前只支持"OEM"和"SIMLOCK"的写入 */
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen) && \
        VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"SIMLOCK\"", gastAtParaList[0].usParaLen))
    
    {        
        return AT_ERROR;        
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    if(VOS_OK == VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen))
    {
        HWLOCK_REQ.HWLOCKTYPE = AT_TAF_HWLOCKOEMTYPE; 
    }
    else /*lint !e830*/
    {
        HWLOCK_REQ.HWLOCKTYPE =  AT_TAF_HWLOCKSIMLOCKTYPE;
    }
    HWLOCK_REQ.usPara1Len = gastAtParaList[1].usParaLen;    
    PS_MEM_CPY((VOS_VOID*)HWLOCK_REQ.HWLOCKPARA,(VOS_VOID*)gastAtParaList[1].aucPara, HWLOCK_REQ.usPara1Len);
    HWLOCK_REQ.HWLOCKPARA[AT_HWLOCK_PARA_LEN] = '\0';
    ret = hw_lock_set_proc(HWLOCK_REQ.HWLOCKPARA, AT_HWLOCK_PARA_LEN, HWLOCK_REQ.HWLOCKTYPE);
    if ( 0 == ret)
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32  At_TestHWlock(VOS_UINT8 ucIndex )
{
    VOS_INT32 ret = -1;
    AT_TAF_SET_HWLOCK_REQ_STRU  HWLOCK_QURY_REQ ;

    PS_MEM_SET(&HWLOCK_QURY_REQ, '\0', sizeof(AT_TAF_SET_HWLOCK_REQ_STRU));
     
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    /*参数长度*/
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_ERROR;
    }
    /* 版本号类型目前只支持"OEM"和"SIMLOCK"的查询 */
    if ((SetOemOprLen != gastAtParaList[0].usParaLen) && (SetSimlockOprLen != gastAtParaList[0].usParaLen))
    {
        (VOS_VOID)vos_printf(" compare len is error\n");
        return AT_ERROR;
    }
    /*解锁码必须是16位*/
    if (gastAtParaList[1].usParaLen != AT_HWLOCK_PARA_LEN )
    {
        return AT_ERROR;
    }
    
    /*lint -e64*/
    /* 密码类型目前只支持OEM和SIMLOCK的查询 */
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen) && \
        VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"SIMLOCK\"", gastAtParaList[0].usParaLen))
    
    {        
        return AT_ERROR;        
    }
    /*受datalock保护*/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    if(VOS_OK == VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen))
    {
        HWLOCK_QURY_REQ.HWLOCKTYPE = AT_TAF_HWLOCKOEMTYPE; 
    }
    else /*lint !e830*/
    {
        HWLOCK_QURY_REQ.HWLOCKTYPE =  AT_TAF_HWLOCKSIMLOCKTYPE;
    }
    
    
    HWLOCK_QURY_REQ.usPara1Len = gastAtParaList[1].usParaLen;
    PS_MEM_CPY((VOS_VOID*)HWLOCK_QURY_REQ.HWLOCKPARA, (VOS_VOID*)gastAtParaList[1].aucPara, HWLOCK_QURY_REQ.usPara1Len);
    HWLOCK_QURY_REQ.HWLOCKPARA[AT_HWLOCK_PARA_LEN] = '\0';
    ret = hw_lock_verify_proc(HWLOCK_QURY_REQ.HWLOCKPARA, gastAtParaList[1].usParaLen, HWLOCK_QURY_REQ.HWLOCKTYPE);
    if ( 0 == ret)
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}

/*end  add by wanghaijie for simlock 3.0*/








VOS_UINT32 At_TestSfm(VOS_UINT8 ucIndex)
{
    TAF_CHAR  *supportedMode = "(0,1,2)";
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s", 
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 
                                                 supportedMode);

    return AT_OK;
}





VOS_UINT32 At_CheckWiwepHex()
{
    int wepIndex = 0;
    VOS_UINT8 *PaucPara = gastAtParaList[1].aucPara;
    for(wepIndex = 0; wepIndex < gastAtParaList[1].usParaLen ; wepIndex++)
    {
        if(((PaucPara[wepIndex] >= '0') && (PaucPara[wepIndex] <= '9')) 
           || ((PaucPara[wepIndex] >= 'a') && (PaucPara[wepIndex] <= 'f')) 
           || ((PaucPara[wepIndex] >= 'A') && (PaucPara[wepIndex] <= 'F'))) 
        {
            //do nothing;
        }
        else
        {
            return AT_ERROR;
        }
    }

    if(wepIndex == gastAtParaList[1].usParaLen)
    {
        return AT_OK;
    }
    
    return AT_ERROR;
}


int set_key_press_event()
{
    struct file *file_handle = NULL;/*lint !e565*/
    char lock_buf[LOCK_BUF_LEN];
    int len = -1;
    mm_segment_t old_fs = get_fs(); /*lint !e10 !e522*/
    
    MBB_MEM_SET_S(lock_buf, sizeof(lock_buf), 0x0, sizeof(lock_buf));/*lint !e506*/
    
    set_fs(KERNEL_DS);

    file_handle = (struct file*)filp_open(screen_state_path, O_RDWR, S_IRWXU);
    if(IS_ERR(file_handle))
    {
        printk("%s: fatal error opening \"%s\".\n", __func__,screen_state_path);
        set_fs(old_fs);
        return AT_ERROR;
    }

    len = file_handle->f_op->read(file_handle, lock_buf, len, &file_handle->f_pos); /*lint !e10 !e115*/
 
    if(len < 0) 
    {
        printk("%s: fatal error read \"%s\".\n", __func__,screen_state_path);
        (void)filp_close(file_handle, NULL);
        set_fs(old_fs);
        return AT_ERROR;
    }

    printk("%s: before keypress autosleep state is \"%s\".\n", __func__,lock_buf);

    /*如果autosleep 不是mem，则上报一次按键事件*/ 
    if (strncmp(lock_buf, "mem\n", sizeof("mem\n") - 1)) 
    {
        report_power_key_down_for_suspend();
     
        report_power_key_up_for_suspend();
    }
    
    (void)filp_close(file_handle, NULL);
    set_fs(old_fs);

    return AT_OK;
}





int set_screen_state(app_main_lock_e on)
{
    struct file *file_handle = NULL;
    char lock_buf[LOCK_BUF_LEN];
    int len = -1;
    mm_segment_t old_fs = get_fs(); /*lint !e10 !e522*/
    
    MBB_MEM_SET_S(lock_buf, sizeof(lock_buf), 0x0, sizeof(lock_buf));/*lint !e506*/
    
    if(APP_MAIN_LOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", on_state);
    }
    else if(APP_MAIN_UNLOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", off_state);
    }
    else
    {
        return AT_ERROR;
    }

    set_fs(KERNEL_DS);

    file_handle = (struct file*)filp_open(screen_state_path, O_RDWR, S_IRWXU);
    if(IS_ERR(file_handle))
    {
        printk("%s: fatal error opening \"%s\".\n", __func__,screen_state_path);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    len = file_handle->f_op->write(file_handle, lock_buf, len, &file_handle->f_pos); /*lint !e10 !e115*/
    if(len < 0) 
    {
        printk("%s: fatal error writing \"%s\".\n", __func__,screen_state_path);
        (void)filp_close(file_handle, NULL);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    (void)filp_close(file_handle, NULL);
    set_fs(old_fs);

    return AT_OK;
}


VOS_UINT32 At_WriteVersionINIToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
)
{
    VOS_UINT8 iniVersion[AT_VERSION_INI_LEN];
    
    PS_MEM_SET(iniVersion, 0, sizeof(iniVersion));
    /* 写入VERSION INI对应的NV项 */
    if (VOS_OK != NV_Write(en_NV_Item_PRI_VERSION , iniVersion, sizeof(iniVersion)))
    {
        AT_WARN_LOG("At_WriteVersionINIToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    return VOS_OK;
}


VOS_UINT32 At_WriteWebCustToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
)
{
    VOS_UINT8    aucWebUserTmp[AT_WEBUI_USER_NV_LEN_MAX + 1];
    VOS_UINT8    aucWiFiPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];
    VOS_UINT8    aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1];
    VOS_UINT8    aucWebPwdTmp[AT_WEBUI_PWD_MAX + 1];
    
    MBB_MEM_SET_S(aucWebUserTmp, sizeof(aucWebUserTmp), 0, sizeof(aucWebUserTmp));
    /* 写入WEB USER对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_USER_NAME , aucWebUserTmp, AT_WEBUI_USER_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    MBB_MEM_SET_S(aucWiFiPinTmp, sizeof(aucWiFiPinTmp), 0, sizeof(aucWiFiPinTmp));
    /* 写入WPS PIN对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WPS_PIN , aucWiFiPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    MBB_MEM_SET_S(aucWebPwdTmp, sizeof(aucWebPwdTmp), 0, sizeof(aucWebPwdTmp));
    /* 写WEBPWD对应的NV项 */
    if (VOS_OK != NV_Write(en_NV_Item_WEB_ADMIN_PASSWORD_NEW_I, aucWebPwdTmp, AT_WEBUI_PWD_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    MBB_MEM_SET_S(aucWebSiteTmp, sizeof(aucWebSiteTmp), 0, sizeof(aucWebSiteTmp));
    /* 写入WEB SITE对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_SITE , aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    return VOS_OK;
}



/*****************************************************************************
 函 数 名  : at_get_ftyreset_set_flag
 功能描述  : 查询当前是否设置过AT^FTYRESET=0命令
 输入参数  : NA
 输出参数  : 无
 返 回 值  : SET_FTYRESET_TYPE;
 调用函数  :
 被调函数  :

 修改历史      :
*****************************************************************************/
SET_FTYRESET_TYPE at_get_ftyreset_set_flag(void)
{
    return g_set_ftyreset_flag;
}

/*****************************************************************************
 函 数 名  : AT_SetFtyResetPara
 功能描述  : 恢复出厂设置操作AT^FTYRESET设置命令处理
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :

*****************************************************************************/
VOS_UINT32 AT_SetFtyResetPara(VOS_UINT8 ucIndex)
{

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    if (0 != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

    /*如果已下发过一次恢复出厂设置命令，直接返回ok*/
    if ( SET_DONE == g_set_ftyreset_flag )
    {
        return AT_OK;
    }


    /*设置已下发恢复出厂设置标记*/
    g_set_ftyreset_flag = SET_DONE;
    return AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_QryFtyResetPara
 功能描述  : 恢复出厂设置操作AT^FTYRESET查询命令处理
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :

*****************************************************************************/
VOS_UINT32 AT_QryFtyResetPara(VOS_UINT8 ucIndex)
{
    RESTORE_STATE_TYPE restore_state = RESTORE_FAIL;
    NODE_STATE_TYPE node_state = APP_START_INVALID; /*lint !e578*/
    VOS_UINT32 ulRst = AT_OK;
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /*获取节点状态*/
    node_state = drv_get_ftyreset_node_state();

    if ( SET_DONE == at_get_ftyreset_set_flag() )
    {
        restore_state = RESTORE_PROCESSING;

        /*如果恢复出厂设置完成*/
        if (APP_RESTORE_OK == node_state)
        {
            restore_state = RESTORE_OK;
        }

        /*如果应用启动ok，上报字串^NORSTFACT字串*/
        if (APP_START_READY == node_state)
        {
            if (REVERT_FAIL == g_revert_flag)
            {
                ulRst = NVM_RevertFNV();
                if (ulRst == AT_SUCCESS)
                {
                    g_revert_flag = REVERT_SUCCESS;
                }
                else
                {
                    return AT_ERROR;
                }
            }
            AT_PhSendRestoreFactParmNoReset();
        }
    }
    else
    {
        restore_state = RESTORE_FAIL;
    }
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    restore_state);
    return AT_OK;
}


VOS_UINT32 At_QrySnPara(VOS_UINT8 ucIndex)
{
    TAF_PH_SERIAL_NUM_STRU stSerialNum;
    TAF_UINT16            usLength = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 从NV中读取 Serial Num,上报显示，返回 */

    PS_MEM_SET(&stSerialNum, 0, sizeof(TAF_PH_SERIAL_NUM_STRU));

    usLength = TAF_SERIAL_NUM_NV_LEN;
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Serial_Num, stSerialNum.aucSerialNum, usLength))
    {
        AT_WARN_LOG("At_QrySnPara:WARNING:NVIM Read en_NV_Item_Serial_Num falied!");
        return AT_ERROR;
    }
    else
    {
        /*将sn的buf的后四位清0，目前仅使用前16位*/
        PS_MEM_SET((stSerialNum.aucSerialNum + TAF_SERIAL_NUM_LEN), 0, 4 * sizeof(stSerialNum.aucSerialNum[0]));
        usLength = 0;
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, \
            (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%s:", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, \
            (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%s", stSerialNum.aucSerialNum);
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}



VOS_UINT32 AT_QryLteAntInfo(TAF_UINT8 ucIndex)
{
    LTE_ANT_INFO_STRU lte_ant_info;
    VOS_UINT8 output_buf[MAX_ANT_NUM][OUTPUT_BUF_LENGTH] = {"ANT0",
              "ANT1", "ANT2", "ANT3", "ANT4", "ANT5", "ANT6", "ANT7"};
    VOS_UINT32 iRet            = 0;
    VOS_UINT8 ant_num_index    = 0;
    VOS_UINT8 band_num_index   = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    MBB_MEM_SET_S(&lte_ant_info, sizeof(lte_ant_info), 0, sizeof(lte_ant_info));

    iRet = NV_Read(NV_ID_LTE_ANT_INFO, &lte_ant_info, sizeof(LTE_ANT_INFO_STRU));

    if(NV_OK != iRet)
    {
        (VOS_VOID)vos_printf("AT_QryLteAntInfo: read nv %d failed!\n", NV_ID_LTE_ANT_INFO);
        return AT_ERROR;
    }
    
    if (lte_ant_info.ant_num > MAX_ANT_NUM)
    {
        (VOS_VOID)vos_printf("AT_QryLteAntInfo: NV %d: ANT NUM ERROR!\n", NV_ID_LTE_ANT_INFO);
        return AT_ERROR;
    }

    for (ant_num_index = 0; ant_num_index < lte_ant_info.ant_num; ant_num_index++)
    {
        for (band_num_index = 0; band_num_index < lte_ant_info.ant_info[ant_num_index].band_num_main_div_flag.band_num;
                                 band_num_index++)
        {
            (VOS_VOID)snprintf((VOS_INT8*)(&output_buf[ant_num_index][0]) + AT_STRLEN((VOS_INT8*)(&output_buf[ant_num_index][0])),
                        OUTPUT_BUF_LENGTH - AT_STRLEN((VOS_INT8*)(&output_buf[ant_num_index][0])),
                        ",B%d", lte_ant_info.ant_info[ant_num_index].band_idx[band_num_index]);
        }
    }

    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      lte_ant_info.ant_num,
                                      gaucAtCrLf);

    for (ant_num_index = 0; ant_num_index < lte_ant_info.ant_num; ant_num_index++)
    {
       gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%s%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          output_buf[ant_num_index],
                                          gaucAtCrLf);
    }
    return AT_OK;
}


TAF_UINT32 At_MacConvertWithColon(TAF_UINT8 *pMacDst, TAF_UINT8 *pMacSrc, TAF_UINT16 usSrcLen)
{
    TAF_UINT16 ulLoop = 0;
    VOS_UINT32                          ulDstMacOffset = 0;
    VOS_UINT32                          ulSrcMacOffset = 0;
    
    if(NULL == pMacDst || NULL == pMacSrc || 0 == usSrcLen)
    {
        return AT_FAILURE;
    }
    
    /* MAC地址长度检查: 必须12位 */
    if (AT_PHYNUM_MAC_LEN != usSrcLen)
    {
        return AT_FAILURE;
    }
    
    /* MAC地址格式匹配: 7AFEE22111E4=>7A:FE:E2:21:11:E4*/
    for (ulLoop = 0; ulLoop < (AT_PHYNUM_MAC_COLON_NUM + 1); ulLoop++)
    {
        PS_MEM_CPY(&pMacDst[ulDstMacOffset], &pMacSrc[ulSrcMacOffset], AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);
        
        ulDstMacOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        ulSrcMacOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        
        pMacDst[ulDstMacOffset] = ':';
        
        ulDstMacOffset++;
    }
    
    pMacDst[AT_PHYNUM_MAC_LEN + AT_PHYNUM_MAC_COLON_NUM] = '\0';
    
    return AT_SUCCESS;
}



VOS_UINT32 Mbb_AT_SetWiFiRxPara(VOS_VOID)
{
    WLAN_AT_WIRX_STRU    wifiRxStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 > gucAtParaIndex || 3 < gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 参数值判断 */
    if(AT_FEATURE_DISABLE != gastAtParaList[0].ulParaValue && AT_FEATURE_ENABLE != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    MBB_MEM_SET_S(&wifiRxStru, sizeof(wifiRxStru), 0, sizeof(wifiRxStru));
    
    if(1 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        wifiRxStru.src_mac[0] = '\0';
        wifiRxStru.dst_mac[0] = '\0';
    }
    else if(2 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.src_mac, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen))
        {
            return AT_ERROR;
        }
        
        wifiRxStru.dst_mac[0] = '\0';
    }
    else if(3 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.src_mac, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen))
        {
            return AT_ERROR;
        }
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.dst_mac, gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen))
        {
            return AT_ERROR;
        }
    }
    
    /* 设置WIFI接收机 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiRX(&wifiRxStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}




VOS_UINT32 Mbb_AT_SetWiFiEnable(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* 参数取值判断 */
    if(gastAtParaList[0].ulParaValue > AT_WIENABLE_TEST)
    {
        return AT_ERROR;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiEnable((WLAN_AT_WIENABLE_TYPE)gastAtParaList[0].ulParaValue)) /*lint !e830*/
    {
        return AT_ERROR;
    }
	return AT_OK; /*lint !e539*/
}



VOS_UINT32 Mbb_AT_SetWiFiModePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiMode((WLAN_AT_WIMODE_TYPE)gastAtParaList[0].ulParaValue)) /*lint !e830*/
    {
        return AT_ERROR;
    }
	return AT_OK; /*lint !e539*/
}


VOS_UINT32 Mbb_AT_SetWiFiBandPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiBand((WLAN_AT_WIBAND_TYPE)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiFreqPara(VOS_VOID)
{
    WLAN_AT_WIFREQ_STRU wifiReqStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    MBB_MEM_SET_S(&wifiReqStru, sizeof(wifiReqStru), 0, sizeof(wifiReqStru));
    
    if(1 == gucAtParaIndex)
    {
        wifiReqStru.value = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        wifiReqStru.offset = 0;
    }
    else
    {
        wifiReqStru.value = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        wifiReqStru.offset = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiFreq(&wifiReqStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiRatePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiDataRate(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiPowerPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI功率 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiPOW(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiTxPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 参数值判断 */
    if(AT_FEATURE_DISABLE != gastAtParaList[0].ulParaValue && AT_FEATURE_ENABLE != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI功率 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiTX((WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiPacketPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 清除WIFI统计包为0 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiRPCKG((VOS_INT32)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWifiInfoPara(VOS_VOID)
{
    VOS_UINT16  usLen = 0;
    VOS_CHAR  *resultBuffer = VOS_NULL;
    WLAN_AT_WIINFO_STRU   wifiInfoStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if(1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    MBB_MEM_SET_S(&wifiInfoStru, sizeof(wifiInfoStru), 0, sizeof(wifiInfoStru));
    
    if(1 == gucAtParaIndex)
    {
        if (0 == gastAtParaList[0].usParaLen)
        {
            return AT_ERROR;
        }
        
        wifiInfoStru.type = (WLAN_AT_WIINFO_TYPE_ENUM)gastAtParaList[0].ulParaValue;
        wifiInfoStru.member.group = 0;//默认值
    }
    else if(2 == gucAtParaIndex)
    {
        if (0 == gastAtParaList[0].usParaLen
            || 0 == gastAtParaList[1].usParaLen)
        {
            return AT_ERROR;
        }
        
        wifiInfoStru.type = (WLAN_AT_WIINFO_TYPE_ENUM)gastAtParaList[0].ulParaValue;
        wifiInfoStru.member.group = (VOS_INT32)gastAtParaList[1].ulParaValue;
    }
    
    /* 获取WIFI信息 */
    if (AT_RETURN_SUCCESS != WlanATGetWifiInfo(&wifiInfoStru))
    {
        return AT_ERROR;
    }
    
    resultBuffer = (VOS_CHAR *)wifiInfoStru.member.content;
    while(0 != MBB_STR_NLEN(resultBuffer, sizeof(wifiInfoStru.member.content)))
    {
        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                        "^WIINFO:%s%s",
                                        resultBuffer, gaucAtCrLf);
                                        
        resultBuffer += MBB_STR_NLEN(resultBuffer, sizeof(wifiInfoStru.member.content)) + 1;
    }
    
    gstAtSendData.usBufLen = usLen - (VOS_UINT16)AT_CRLF_MAX_LEN;
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWifiPaRangePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* 参数个数判断 */
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 小写字母转换 */
    gastAtParaList[0].aucPara[gastAtParaList[0].usParaLen] = '\0';
    (void)VOS_StrToLower((VOS_CHAR*)gastAtParaList[0].aucPara);
    
    /* 参数值判断 */
    if('l' != gastAtParaList[0].aucPara[0] && 'h' != gastAtParaList[0].aucPara[0])
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI PA模式 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiParange((WLAN_AT_WiPARANGE_TYPE)gastAtParaList[0].aucPara[0]))
    {
        return AT_ERROR;
    }
    return AT_OK;
}

#define TMMI_PARA_LEN_MAX    (5) /* MMI test */

VOS_UINT32 Mbb_AT_SetTmmiPara(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           aucFacInfo[AT_FACTORY_INFO_LEN];
    VOS_UINT32 temp_mmi = 0;
    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return  AT_TOO_MANY_PARA;
    }


    /*参数长度过长*/
    if (TMMI_PARA_LEN_MAX < gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    MBB_MEM_SET_S(aucFacInfo, AT_FACTORY_INFO_LEN, 0x00, AT_FACTORY_INFO_LEN);

    /* 写入en_NV_Item_Factory_Info，需偏移24个字节，长度4为四个字节，因此需要先读 */
    ulResult = NV_ReadEx(MODEM_ID_0, en_NV_Item_Factory_Info, aucFacInfo, AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        AT_ERR_LOG("AT_SetTmmiPara: NV Read Fail!");
        return AT_ERROR;
    }

    if (gastAtParaList[0].ulParaValue > AT_MMI_RESULT_MAX)
    {
        return AT_ERROR;
    }

    temp_mmi = gastAtParaList[0].ulParaValue;

    MBB_MEM_CPY_S(&aucFacInfo[AT_MMI_TEST_FLAG_OFFSET], (AT_FACTORY_INFO_LEN - AT_MMI_TEST_FLAG_OFFSET),
                    &temp_mmi, AT_MMI_TEST_FLAG_LEN);

    ulResult = NV_WriteEx(MODEM_ID_0, en_NV_Item_Factory_Info, aucFacInfo, AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 Mbb_AT_SetChrgEnablePara(VOS_VOID)
{
    /* 输入参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*参数长度过长*/
    if (0 == gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        if(2 == gucAtParaIndex)
        {
            if (0 == gastAtParaList[1].usParaLen)
            {
                return  AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }
    
    /* 是否支持电池 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE))
    {
        return AT_ERROR;
    }
    
    if(2 == gucAtParaIndex)
    {
        /* 调用充电接口 */
        if(CHG_OK != chg_tbat_charge_mode_set(gastAtParaList[0].ulParaValue, gastAtParaList[1].ulParaValue))
        {
            return AT_ERROR;
        }
    }
    else
    {
        /* 调用放电和补电接口 */
        if(CHG_OK != chg_tbat_charge_mode_set(gastAtParaList[0].ulParaValue, 0))
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}



VOS_UINT32  Mbb_AT_SetPhyNumPara(AT_PHYNUM_TYPE_ENUM_UINT32 enSetType, MODEM_ID_ENUM_UINT16 enModemId)
{
    VOS_UINT32                          ulRet;
	
    switch(enSetType)
    {
        case AT_PHYNUM_TYPE_IMEI:
            ulRet = AT_UpdateImei(enModemId, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            break;
            
        case AT_PHYNUM_TYPE_SVN:
            ulRet = AT_UpdateSvn(enModemId, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            break;

        case AT_PHYNUM_TYPE_MAC:
            if (TRUE == get_lan_support())
            {
                ulRet = AT_UpdateMacPara(gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            }
            else
            {
                ulRet = AT_ERROR;
            }
            break;
        case AT_PHYNUM_TYPE_MACWLAN:
            if (BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
            {
                if (TRUE == get_lan_support())
                {
                   ulRet = AT_ERROR;
                }
                else               
                {
                   ulRet = AT_UpdateMacPara(gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
                }
            }
            else
            {
                ulRet = AT_ERROR;
            }
            break;
         case AT_PHYNUM_TYPE_ESN:
            ulRet = AT_ERROR;
            break;
            
         case AT_PHYNUM_TYPE_UMID:
            ulRet = AT_ERROR;
            break;   

         case AT_PHYNUM_TYPE_BUTT:
            ulRet = AT_ERROR;
            break;   

         default:
            ulRet = AT_ERROR;
            break;
    }
	
	return ulRet;
}

/*****************************************************************************
 函 数 名  : At_SecCheckSamePortNUM
 功能描述  :不能同时设置两个以上的PCUI、MODEM、NDIS及GPS接口，
            并且有且只能有一个PCUI、和NDIS接口
 输入参数  : 
 输出参数  : 无
 返 回 值  : VOS_UINT8 PortNum
 调用函数  :
 被调函数  :

*****************************************************************************/
VOS_VOID At_SecCheckSamePortNUM(VOS_UINT32  ucTempnum)
{

    /*记录设置的每类端口个数*/
    switch(ucTempnum)
    {
        case AT_DEV_MODEM:
        case AT_DEV_4G_MODEM:
        {
            gportTypeNum[MODEMNUM]++;
            break;
        }

        case AT_DEV_PCUI:
        case AT_DEV_4G_PCUI:
        {
            gportTypeNum[PCUINUM]++;
            break;
        }

        case AT_DEV_GPS:
        case AT_DEV_GPS_CONTROL:
        case AT_DEV_4G_GPS:
        {
           gportTypeNum[GPSNUM]++;
            break;
        }

        case AT_DEV_NDIS:
        case AT_DEV_4G_NDIS:
        case AT_DEV_NCM:
        {
           gportTypeNum[NDISNUM]++;
            break;
        }

        default :
            break;
    }
}

VOS_VOID clearportTypeNum(VOS_VOID)
{
    VOS_UINT8 i = 0;
    for(i = 0; i < MAXPORTTYPENUM; i++)
    {
        gportTypeNum[i] = 0;
    }
}


TAF_UINT32  Mbb_AT_SetFDac_Para_Valid(TAF_VOID)
{
	TAF_UINT16                           usDAC;

    /*调用 LTE 模的接口分支*/
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_ERROR;
    }
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_ERROR;
    }
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_ERROR;
    }

    usDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (usDAC > WDSP_MAX_TX_AGC)
        {
            return AT_ERROR;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
			return AT_OK;
        }
    }
    else
    {
        if (usDAC > GDSP_MAX_TX_VPA)
        {
            return AT_ERROR;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
			return AT_OK;
        }
    }
}


VOS_UINT32 Mbb_AT_QryTmmiPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           aucFacInfo[AT_FACTORY_INFO_LEN];
    VOS_UINT32                           ucMmiFlag;
    VOS_UINT32                          ulResult;
    VOS_UINT32 temp_mmi = 0;
    ulResult = NV_ReadEx(MODEM_ID_0, en_NV_Item_Factory_Info,
                       aucFacInfo,
                       AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        return AT_ERROR;
    }

    MBB_MEM_CPY_S(&temp_mmi, sizeof(temp_mmi),  &aucFacInfo[AT_MMI_TEST_FLAG_OFFSET], AT_MMI_TEST_FLAG_LEN);
    if (AT_MMI_RESULT_MAX < temp_mmi)
    {
        ucMmiFlag = 0;
    }
    else
    {
        ucMmiFlag = temp_mmi;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ucMmiFlag);

    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryChrgEnablePara(VOS_UINT8 ucIndex)
{
    CHG_TCHRENABLE_TYPE chrenable_state;
    
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE) )
    {
        return AT_ERROR;
    }

    MBB_MEM_SET_S(&chrenable_state, sizeof(chrenable_state), 0, sizeof(chrenable_state));
    
    if(CHG_OK != chg_tbat_get_tchrenable_status(&chrenable_state))
    {
        return AT_ERROR;
    }
    
    if(AT_TCHRENABEL_SWITCH_CHARG_OPEN == chrenable_state.chg_state)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:%d,%d",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       chrenable_state.chg_state,
                                                       chrenable_state.chg_mode);
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:%d",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       chrenable_state.chg_state);
    }

    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWifiPaRangePara (VOS_UINT8 ucIndex)
{

    WLAN_AT_WiPARANGE_TYPE   ucWifiPaType;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    ucWifiPaType = WlanATGetWifiParange();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%c",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiPaType );
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiPacketPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIRPCKG_STRU   wifiPckStru;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&wifiPckStru, sizeof(wifiPckStru), 0, sizeof(wifiPckStru));

    /* 获取WIFI接收机误包码 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiRPCKG(&wifiPckStru))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiPckStru.good_result, wifiPckStru.bad_result);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiRxPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIRX_STRU   wifiRxStru;
    MBB_MEM_SET_S(&wifiRxStru, sizeof(wifiRxStru), 0, sizeof(wifiRxStru));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI RX信息 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiRX(&wifiRxStru))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiRxStru.onoff);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiRatePara(VOS_UINT8 ucIndex)
{

    VOS_UINT32 wifiDataRate;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 读取WIFI速率 */
    wifiDataRate = WlanATGetWifiDataRate();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiDataRate);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiFreqPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIFREQ_STRU wifiReqStru;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&wifiReqStru, sizeof(wifiReqStru), 0, sizeof(wifiReqStru));
    
    if (AT_RETURN_SUCCESS != WlanATGetWifiFreq(&wifiReqStru))
    {
        return AT_ERROR;
    }

    /* 查询设置值 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiReqStru.value);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiBandPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&strBuf, sizeof(strBuf), 0, sizeof(strBuf));
    
    /* 读取WIFI带宽 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiBand(&strBuf))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiModePara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    MBB_MEM_SET_S(&strBuf, sizeof(strBuf), 0, sizeof(strBuf));

    /* 读取WIFI模式 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiMode(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}


static VOS_UINT32 at_version_get_bdt(VOS_UINT8* bdt_ver)
{
    VOS_UINT8 *pInfoTemp = NULL;

    if (NULL == bdt_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_bdt bdt_ver is NULL!\r\n");
        return AT_ERROR;
    }
    /* 1. 读取编译时间 */
    pInfoTemp = NULL;
    pInfoTemp = (char *)bsp_version_get_build_date_time();
    if(NULL == pInfoTemp)
    {
        (VOS_VOID)vos_printf("%s: build time read error.", __func__);
        return AT_ERROR;
    }

    version_info_fill(bdt_ver, pInfoTemp);
    /*版本编译时间格式转换，将时间戳中连续两个空格的后一个空格用0替换 */
    (void)At_ZeroReplaceBlankInString(bdt_ver, VOS_StrNLen(bdt_ver, TAF_MAX_VERSION_VALUE_LEN));
    return AT_OK;
}


static VOS_UINT32 at_version_get_exts(VOS_UINT8* exts_ver)
{
    VOS_UINT8 *pInfoTemp = NULL;

    if (NULL == exts_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_inth exts_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 2. 读取软件外部版本号 */
    pInfoTemp = bsp_version_get_firmware();
    if (NULL == pInfoTemp)
    {
        (VOS_VOID)vos_printf("%s: software version read error.", __func__);
        return AT_ERROR;
    }
    version_info_fill(exts_ver, pInfoTemp);
    return AT_OK;
}


static VOS_UINT32 at_version_get_ints(VOS_UINT8* ints_ver)
{
    VOS_UINT8 *pInfoTemp = NULL;
    if (NULL == ints_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_ints ints_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 3. 读取软件内部版本号 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        pInfoTemp = bsp_version_get_firmware();
        if(NULL == pInfoTemp)
        {
            (VOS_VOID)vos_printf("%s: software version read error.", __func__);
            return AT_ERROR;
        }
        version_info_fill(ints_ver, pInfoTemp);
    }
    else
    {
        MBB_MEM_SET_S(ints_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    }
    return AT_OK;
}


static  VOS_UINT32 at_version_get_extd(VOS_UINT8* extd_ver)
{
    VOS_UINT16 ulLen = 0;
    if (NULL == extd_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_extd extd_ver is NULL!\r\n");
        return AT_ERROR;
    }
    /* 4. 获取WEBUI或者ISO版本号 */
    MBB_MEM_SET_S(extd_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    ulLen = TAF_MAX_VERSION_VALUE_LEN;
    if (VOS_OK != mdrv_dload_getwebuiver(extd_ver, TAF_MAX_VERSION_VALUE_LEN)) /*722平台非STICK形态产品获取WEBUI版本号*/
    {
        (VOS_VOID)vos_printf("%s: outer extd_ver read error.", __func__);
        return AT_ERROR;
    }
    (void)At_DelCtlAndBlankCharWithEndPadding(extd_ver, &ulLen);
    return AT_OK;
}


static VOS_UINT32 at_version_get_intd(VOS_UINT8* intd_ver)
{
    VOS_UINT16 ulLen = 0;
    if (NULL == intd_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_intd intd_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 5. 获取WEBUI或者ISO版本号 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        MBB_MEM_SET_S(intd_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
        ulLen = TAF_MAX_VERSION_VALUE_LEN;
        if (VOS_OK != mdrv_dload_getwebuiver(intd_ver, TAF_MAX_VERSION_VALUE_LEN))  /*722平台非STICK形态产品获取WEBUI版本号*/
        {
            (VOS_VOID)vos_printf("%s: outer iso ver read error.", __func__);
            return AT_ERROR;
        }
        (void)At_DelCtlAndBlankCharWithEndPadding(intd_ver, &ulLen);
    }
    else
    {
        MBB_MEM_SET_S(intd_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    }
    return AT_OK;
}


static VOS_UINT32 at_version_get_exth(VOS_UINT8* exth_ver)
{
    if (NULL == exth_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_exth exth_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 6. 获取外部硬件版本号  */
    MBB_MEM_SET_S(exth_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (VOS_OK != BSP_HwGetHwVersion((char *)exth_ver, TAF_MAX_VERSION_VALUE_LEN))
    {
        (VOS_VOID)vos_printf("%s: outer hardware version read error.", __func__);
        return AT_ERROR;
    }

    return AT_OK;
}


static VOS_UINT32 at_version_get_inth(VOS_UINT8* inth_ver)
{
    if (NULL == inth_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_inth inth_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 7. 获取内部硬件版本号  */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        MBB_MEM_SET_S(inth_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
        if (VOS_OK != BSP_HwGetHwVersion((VOS_UINT8 *)(inth_ver), TAF_MAX_VERSION_VALUE_LEN))
        {
            (VOS_VOID)vos_printf("%s: inner hardware version read error.", __func__);
            return AT_ERROR;
        }
    }
    else
    {
        MBB_MEM_SET_S(inth_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    }
    return AT_OK;
}


static VOS_UINT32 at_version_get_extu(VOS_UINT8* extu_ver)
{
    if (NULL == extu_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_extu extu_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 8. 获取外部产品名 */
    MBB_MEM_SET_S(extu_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (VOS_OK != mdrv_dload_get_productname(extu_ver, TAF_MAX_VERSION_VALUE_LEN))
    {
        (VOS_VOID)vos_printf("%s: outer product name read error.", __func__);
        return AT_ERROR;
    }

    return AT_OK;
}


static VOS_UINT32 at_version_get_intu(VOS_UINT8* intu_ver)
{
    if (NULL == intu_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_intu inth_ver is NULL!\r\n");
        return AT_ERROR;
    }

    /* 9. 获取内部产品名 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        MBB_MEM_SET_S(intu_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
        if (VOS_OK != BSP_MNTN_GetProductIdInter(intu_ver, TAF_MAX_VERSION_VALUE_LEN))
        {
            (VOS_VOID)vos_printf("%s: inner product name read error.", __func__);
            return AT_ERROR;
        }
    }
    else
    {
        MBB_MEM_SET_S(intu_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    }
    return AT_OK;
}


static VOS_UINT32 at_version_get_cfg(VOS_UINT8* cfg_ver)
{
    TAF_NVIM_CS_VER_STRU stCsver = {0};

    if (NULL == cfg_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_cfg cfg_ver is NULL!\r\n");
        return AT_ERROR;
    }

    MBB_MEM_SET_S(cfg_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Csver, &(stCsver.usCsver), sizeof(stCsver.usCsver)))
    {
        (VOS_VOID)vos_printf("%s: en_NV_Item_Csver read error.", __func__);
        return AT_ERROR;
    }
    (void)snprintf(cfg_ver, TAF_MAX_VERSION_VALUE_LEN, "%d", stCsver.usCsver);
    return AT_OK;
}


static VOS_UINT32 at_version_get_prl(VOS_UINT8* prl_ver)
{
    if (NULL == prl_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_prl prl_ver is NULL!\r\n");
        return AT_ERROR;
    }
    /* 11. PRL版本号 */
    MBB_MEM_SET_S(prl_ver, TAF_MAX_VERSION_VALUE_LEN, 0, TAF_MAX_VERSION_VALUE_LEN);
    return  AT_OK;
}


static VOS_UINT32 at_version_get_ini(VOS_UINT8* ini_ver)
{
    VOS_UINT8 iniVersion[AT_VERSION_INI_LEN] = {0};

    if (NULL == ini_ver)
    {
        (VOS_VOID)vos_printf("at_version_get_ini ini_ver is NULL!\r\n");
        return AT_ERROR;
    }
    /* 13. INI配置文件版本号 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_PRI_VERSION, iniVersion, sizeof(iniVersion)))
    {
        (VOS_VOID)vos_printf("%s: en_NV_Item_PRI_VERSION read error.", __func__);
        return AT_ERROR;
    }

    version_info_fill(ini_ver, iniVersion);
    return AT_OK;
}


VOS_UINT32  Mbb_At_QryVersion(VOS_UINT8 ucIndex )
{
    VOS_UINT8 tempInfoArray[TAF_MAX_VERSION_VALUE_LEN] = {0};
    VOS_UINT32 ret = AT_ERROR;
    VOS_UINT8 loop = 0;

    gstAtSendData.usBufLen = 0;
    for (loop =  0; loop < sizeof(g_at_versiont_config_tab) / sizeof(AT_VERSION_TAB_STRU); loop++)
    {
        if (NULL == g_at_versiont_config_tab[loop].pFunc)
        {
            (VOS_VOID)vos_printf("g_at_versiont_config_tab[%d].pFunc is null!\r\n", loop);
            return AT_ERROR;
        }

        if (NULL == g_at_versiont_config_tab[loop].version_type)
        {
            (VOS_VOID)vos_printf("g_at_versiont_config_tab[%d].version_type is null!\r\n", loop);
            return AT_ERROR;
        }

        ret = g_at_versiont_config_tab[loop].pFunc(tempInfoArray);
        if (AT_OK == ret)
        {
            version_info_build(ucIndex, g_at_versiont_config_tab[loop].version_type, tempInfoArray);
        }
        else
        {
            (VOS_VOID)vos_printf("g_at_versiont_config_tab[%d].pFunc retrun ret=%d!\r\n", loop, ret);
            return AT_ERROR; 
        }
    }

    if (0 < gstAtSendData.usBufLen)
    {
        gstAtSendData.usBufLen = gstAtSendData.usBufLen - (VOS_UINT16)MBB_STR_NLEN(gaucAtCrLf, AT_CRLF_MAX_LEN);
    }

    return AT_OK;
}


#define SB_SERIAL_NUM_LEN    (15)

TAF_UINT32 AT_SetSbSnPara(TAF_UINT8 ucIndex)
{
    SB_SERIAL_NUM_STRU stSerialNum;
    PS_MEM_SET((char*)(&stSerialNum), 0, sizeof(stSerialNum));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:ucCmdOptType err!");
        return AT_ERROR;
    }

    /* 参数个数不为1 */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:ucCmdOpt num err!");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 如果参数长度不等于15，直接返回错误 */
    if (SB_SERIAL_NUM_LEN != gastAtParaList[0].usParaLen)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:sn len err!");
        return AT_SN_LENGTH_ERROR;
    }

    /* 检查当前参数是否为数字字母字符串,不是则直接返回错误 */
    if (AT_FAILURE == At_CheckNumCharString(gastAtParaList[0].aucPara,
                                            gastAtParaList[0].usParaLen))
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:sn str err!");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 拷贝设置的15位SN参数到结构体变量stSerialNum.aucSerialNum中 */
    PS_MEM_CPY(stSerialNum.serial_num, gastAtParaList[0].aucPara, SB_SERIAL_NUM_LEN);

    if (NV_OK != NV_WriteEx(MODEM_ID_0, NV_SB_SERIAL_NUM,
                          stSerialNum.serial_num,
                          TAF_SERIAL_NUM_NV_LEN))
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:NVIM Write NV_SB_SERIAL_NUM_STRU failed!");
        return AT_DEVICE_OTHER_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_QrySbSnPara(VOS_UINT8 ucIndex)
{
    SB_SERIAL_NUM_STRU stSerialNum;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    PS_MEM_SET((char*)(&stSerialNum), '\0', sizeof(stSerialNum));

    if (NV_OK != NV_ReadEx(MODEM_ID_0, NV_SB_SERIAL_NUM, stSerialNum.serial_num, SB_SERIAL_NUM_LEN))
    {
        AT_WARN_LOG("AT_QrySbSnPara:WARNING:NVIM Read NV_SB_SERIAL_NUM_STRU falied!");
        return AT_ERROR;
    }
    else
    {
        /*设置第16字节为字串结束符*/

        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 stSerialNum.serial_num,
                                                 gaucAtQuotation);
    }
    return AT_OK;
}



VOS_UINT32 AT_SetTledSwitchPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}

VOS_UINT32 At_QryTledSwitchPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 At_TestTledSwitchPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 at_mac_format(VOS_UINT8 *aucMac)
{
    /* MAC地址格式匹配: 7A:FE:E2:21:11:E4=>7AFEE22111E4 */
    VOS_UINT8 mac_buf[AT_MAC_ADDR_LEN + 1] = {0}; /* MAC地址*/
    VOS_UINT32 mac_offset         = 0;
    VOS_UINT32 nv_mac_offset = 0;
    VOS_UINT32 loop = 0;

    if (NULL == aucMac)
    {
        (VOS_VOID)vos_printf("at_mac_format auMac is null!\r\n");
        return AT_ERROR;
    }

    MBB_MEM_CPY_S(&mac_buf[nv_mac_offset], (sizeof(mac_buf) - nv_mac_offset), &aucMac[mac_offset], AT_MAC_ADDR_LEN);
    MBB_MEM_SET_S(&aucMac[mac_offset], (sizeof(mac_buf) - mac_offset), '\0', MBB_STR_NLEN(aucMac, AT_MAC_ADDR_LEN) + 1);

    for (loop = 0; loop < (1 + AT_PHYNUM_MAC_COLON_NUM); loop++)
    {
        MBB_MEM_CPY_S(&aucMac[mac_offset],
                            (AT_MAC_ADDR_LEN - mac_offset),
                            &mac_buf[nv_mac_offset],
                            AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);

        mac_offset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        nv_mac_offset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS + sizeof(":") - 1;
    }

    return  AT_OK;
}

VOS_UINT32 mbb_type_mac_output(VOS_UINT8 index, VOS_UINT16 *length, AT_MAC_TYPE_ENUM mac_type)
{
    VOS_INT8 aucMac[AT_MAC_ADDR_LEN] = {0}; /* MAC地址*/
    VOS_UINT8 *pt_type = NULL;
    VOS_UINT8 *pt_mac = "MAC";
    VOS_UINT8 *pt_macwlan = "MACWLAN";
    VOS_UINT8 *pt_macbt = "MACBT";
    VOS_UINT8   mac_num = 0;
    VOS_INT32 ret = 0;

    if (NULL == length)
    {
        (VOS_VOID)vos_printf("mbb_mac_output: length is null!\r\n.");
        return AT_ERROR;
    }

    /*mac type 合法性检查*/
    if (TYPE_LAN_MAC == mac_type)
    {
        pt_type = pt_mac;
    }
    else if (TYPE_MAC_ADDR == mac_type)
    {
        pt_type = pt_macwlan;
    }
    else if (TYPE_NV_HUAWEI_BT_INFO_I == mac_type)
    {
        pt_type = pt_macbt;
    }
    else
    {
        (VOS_VOID)vos_printf("mbb_mac_output: mac_type err!\r\n.");
        return AT_ERROR;
    }
    /*获取mac_type类型mac地址及mac个数*/
    ret = get_type_mac_addr(mac_type, (char *)(&aucMac[0]), &mac_num);
    if (0 != ret)
    {
        (VOS_VOID)vos_printf("get_type_mac_addr: ret = %d err!\r\n.", ret);
        return AT_ERROR;
    }

    /*如果该类型的mac_num=0,认为不支持该类型的mac*/
    if (0 == mac_num)
    {
        return AT_OK;
    }

    /*7A:FE:E2:21:11:E4=>7AFEE22111E4 ,MAC 地址格式后输出*/
    ret = (VOS_INT32)at_mac_format((char *)(&aucMac[0]));
    if (AT_OK != ret)
    {
        (VOS_VOID)vos_printf("at_mac_format: ret = %d err!\r\n.", ret);
        return AT_ERROR;
    }

    if( ('\0' != aucMac[0]) && (mac_num > 1) )
    {
        /* MAC地址输出 */
        (*length) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + (*length),
                                               "%s:%s,%s,%d,%d%s",
                                               g_stParseContext[index].pstCmdElement->pszCmdName,
                                               pt_type,
                                               aucMac,
                                               0,
                                               mac_num - 1,
                                               gaucAtCrLf);
    }
    else
    {
        (*length) += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + (*length),
                                            "%s:%s,%s%s",
                                            g_stParseContext[index].pstCmdElement->pszCmdName,
                                            pt_type,
                                            aucMac,
                                            gaucAtCrLf);
    }

    return AT_OK;
}



#define ANT_2G_NUM                               (2)
#define ANT_5G_NUM                               (3)
#define WIFI_FIRST_RF                            (0)
#define WIFI_SECOND_RF                           (1)
#define WIFI_THIRD_RF                            (2)


VOS_UINT32 AT_QryWiFi2GPavars(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;
    VOS_INT32               ulRet = 0 ;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    ulRet = mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI);
    if (BSP_MODULE_UNSUPPORT == ulRet )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void*)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的2.4g pavars执行的参数 */
    ulRet = WlanATGetWifi2GPavars(&strBuf);
    if (AT_RETURN_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }

    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             "%s:\n%s",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                             strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWiFi2GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_PAVARS2G_STRU    wifiPavars2gStru;
    VOS_INT32                ulRet = 0 ;
    int i = 0;

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

     /* 参数个数判断 */
    if (PARA_NUMBER_FOUR != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    ulRet = mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI);
    if (BSP_MODULE_UNSUPPORT == ulRet)
    {
        return AT_ERROR;
    }
    /* 参数值判断，因为只有两根天线0,1，故参数不能大于等于2 */
    if (ANT_2G_NUM <= gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

   /* 初始化 */
    (void)VOS_MemSet((void *)&wifiPavars2gStru, 0, sizeof(wifiPavars2gStru));

    wifiPavars2gStru.ANT_Index = gastAtParaList[0].ulParaValue;
    for ( i = 0 ; i < PARA_NUMBER_THREE ; i++ )
    {
        (void)VOS_MemCpy((void *)wifiPavars2gStru.data[i], (void *)gastAtParaList[i + 1].aucPara, gastAtParaList[i + 1].usParaLen);
    }

    /* WIFI模块射频参数 */
    ulRet = WlanATSetWifi2GPavars(&wifiPavars2gStru);
    if (AT_RETURN_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }
    return AT_OK;
}

VOS_UINT32 AT_QryWiFi5GPavars(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;
    VOS_INT32               ulRet = 0 ;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    ulRet = mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI);
    if (BSP_MODULE_UNSUPPORT == ulRet)
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的5g pavars执行的参数 */
    ulRet =  WlanATGetWifi5GPavars(&strBuf);
    if (AT_RETURN_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }
    
    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:\n%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWiFi5GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_PAVARS5G_STRU    wifiPavars5gStru;
    VOS_INT32  i = 0;
    VOS_INT32  ulRet = 0 ;

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (PARA_NUMBER_THIRTEEN != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    ulRet = mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI);
    if (BSP_MODULE_UNSUPPORT == ulRet)
    {
        return AT_ERROR;
    }

    /* 参数值判断,5Gwifi只有三根天线 */
    if (ANT_5G_NUM <= gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void*)&wifiPavars5gStru, 0, sizeof(wifiPavars5gStru));

    wifiPavars5gStru.ANT_Index = gastAtParaList[0].ulParaValue;

    for ( i = 0 ; i < PARA_NUMBER_TWELVE ; i++ )
    {
        VOS_MemCpy(wifiPavars5gStru.data[i], gastAtParaList[i + 1].aucPara,
                   gastAtParaList[i + 1].usParaLen);
    }

    /* WIFI模块射频参数 */
    ulRet = WlanATSetWifi5GPavars(&wifiPavars5gStru);
    if (AT_RETURN_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }

    return AT_OK;
}



VOS_UINT32 AT_SetNVReadWiFi2GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId  = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          i       = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          ulRet = 0 ;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_ONE;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > PARA_NUMBER_ONE)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        usNvId = NV_ID_WIFI_2G_RFCAL;
        ulNvLen = CHECK_FIX_LEN_SIX;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, ulNvLen);

    if (VOS_NULL_PTR == pucData)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_FIVE;
        return AT_ERROR;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVReadPara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_EIGHT;
        return AT_ERROR;
    }

    if (0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_ReadEx(enModemId, usNvId, (VOS_VOID*)pucData, ulNvLen);
    }
    else
    {
        ulRet = NV_ReadPartEx(enModemId, usNvId, CHECK_FIX_LEN_SIX, (VOS_VOID*)pucData, ulNvLen);
    }

    if (VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVRD = CHECK_FIX_NUMBER_SIX;
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    if (0 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                 "pa2ga0=");
    }
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                 "pa2ga1=");
    }
    else
    {
        g_ulNVRD = CHECK_FIX_NUMBER_THREE;
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < ulNvLen; i++)
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "0X%02X", pucData[i++]);

        if (i == (ulNvLen - 1))
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "%02X", pucData[i]);
            break;
        }

        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "%02X,", pucData[i]);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVRD = CHECK_FIX_NUMBER_SEVEN;
    return AT_OK;
}

VOS_UINT32 AT_SetNVWriteWiFi2GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId = NV_ID_WIFI_2G_RFCAL;
    VOS_UINT16                          usNvTotleLen = CHECK_FIX_LEN_SIX;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulNvNum = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulRet = 0 ;
    VOS_UINT32                          ulTmp = 0;
    VOS_UINT32                          j = 0;
    VOS_UINT8                           *pu8Start   = VOS_NULL_PTR;

    gstAtSendData.usBufLen = 0;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVWR = CHECK_FIX_NUMBER_ONE;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > PARA_NUMBER_FOUR || gucAtParaIndex < PARA_NUMBER_ONE )
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (ANT_2G_NUM <= gastAtParaList[0].ulParaValue)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usNvTotleLen); /*lint !e516*/

    if (VOS_NULL_PTR == pucData)
    {
        g_ulNVWR = CHECK_FIX_NUMBER_THREE;
        return AT_ERROR;
    }

    for (i = 0; i < PARA_NUMBER_THREE; i++)
    {
        pu8Start = gastAtParaList[i + 1].aucPara;

        for (j = 0; j < PARA_NUMBER_TWO ; j++)
        {
            pu8Start += CHECK_FIX_LEN_TWO;
            ulRet = AT_String2Hex(pu8Start , CHECK_FIX_LEN_TWO , &ulTmp);

            if ((VOS_OK != ulRet) || (ulTmp > 0xff))
            {
                PS_MEM_FREE(WUEPS_PID_AT, pucData);
                return AT_ERROR;
            }

            *(pucData + ulNvNum) = (VOS_UINT8)ulTmp;
            ulNvNum++;
        }
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVWritePara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_EIGHT;
        return AT_ERROR;
    }

    if (0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WriteEx(enModemId, usNvId, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, CHECK_FIX_LEN_SIX, (VOS_VOID*)pucData, usNvTotleLen);
    }

    if (VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_NIGN;
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVWR = CHECK_FIX_NUMBER_TEN;
    return AT_OK;
}


VOS_UINT32 AT_SetNVReadWiFi5GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId  = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          i       = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          ulRet = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_ONE;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > PARA_NUMBER_ONE)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        usNvId = NV_ID_WIFI_5G_RFCAL;
        ulNvLen = CHECK_FIX_LEN_TWENTYFOUR;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, ulNvLen);

    if (VOS_NULL_PTR == pucData)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_FIVE;
        return AT_ERROR;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVReadPara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_EIGHT;
        return AT_ERROR;
    }

    if (WIFI_FIRST_RF == gastAtParaList[0].ulParaValue)        /* 参数值为0时,表示第一根天线 */
    {
        ulRet = NV_ReadEx(enModemId, usNvId, (VOS_VOID*)pucData, ulNvLen);
    }
    else if (WIFI_SECOND_RF == gastAtParaList[0].ulParaValue)   /* 第二根天线 */
    {
        ulRet = NV_ReadPartEx(enModemId, usNvId, CHECK_FIX_LEN_TWENTYFOUR, (VOS_VOID*)pucData, ulNvLen);
    }
    else if (WIFI_THIRD_RF == gastAtParaList[0].ulParaValue)   /* 第三根天线 */
    {
        ulRet = NV_ReadPartEx(enModemId, usNvId, CHECK_FIX_LEN_FORTYEIGHT, (VOS_VOID*)pucData, ulNvLen);
    }
    else
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        return AT_ERROR;
    }

    if (VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVRD = CHECK_FIX_NUMBER_SIX;
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    if (0 == gastAtParaList[0].ulParaValue)            /* 参数值为0时,表示第一根天线 */
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                 "pa5ga0=");
    }
    else if (1 == gastAtParaList[0].ulParaValue)       /* 第二根天线 */
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                 "pa5ga1=");
    }
    else                                               /* 第三根天线 */
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                 "pa5ga2=");

    }

    for (i = 0; i < ulNvLen; i++)
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "0X%02X", pucData[i++]);

        if (i == (ulNvLen - 1))
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "%02X", pucData[i]);
            break;
        }

        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen, "%02X,", pucData[i]);

    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVRD = CHECK_FIX_NUMBER_SEVEN;
    return AT_OK;
}

VOS_UINT32 AT_SetNVWriteWiFi5GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId = NV_ID_WIFI_5G_RFCAL;
    VOS_UINT16                          usNvTotleLen = CHECK_FIX_LEN_TWENTYFOUR;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulNvNum = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulTmp = 0;
    VOS_UINT32                          j = 0;
    VOS_UINT8                           *pu8Start   = VOS_NULL_PTR;

    gstAtSendData.usBufLen = 0;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVWR = CHECK_FIX_NUMBER_ONE;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > PARA_NUMBER_THIRTEEN || gucAtParaIndex < PARA_NUMBER_ONE )
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (ANT_5G_NUM <= gastAtParaList[0].ulParaValue)
    {
        g_ulNVRD = CHECK_FIX_NUMBER_TWO;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usNvTotleLen); /*lint !e516*/

    if (VOS_NULL_PTR == pucData)
    {
        g_ulNVWR = CHECK_FIX_NUMBER_THREE;
        return AT_ERROR;
    }

    for (i = 0; i < PARA_NUMBER_TWELVE; i++)
    {
        pu8Start = gastAtParaList[i + 1].aucPara;

        for (j = 0; j < PARA_NUMBER_TWO; j++)
        {
            pu8Start += CHECK_FIX_LEN_TWO;
            ulRet = AT_String2Hex(pu8Start , CHECK_FIX_LEN_TWO , &ulTmp);

            if ((VOS_OK != ulRet) || (ulTmp > 0xff))
            {
                PS_MEM_FREE(WUEPS_PID_AT, pucData);
                return AT_ERROR;
            }

            *(pucData + ulNvNum) = (VOS_UINT8)ulTmp;
            ulNvNum++;
        }
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVWritePara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_EIGHT;
        return AT_ERROR;
    }

    if (0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WriteEx(enModemId, usNvId, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, CHECK_FIX_LEN_TWENTYFOUR, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, CHECK_FIX_LEN_FORTYEIGHT, (VOS_VOID*)pucData, usNvTotleLen);
    }

    if (VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR = CHECK_FIX_NUMBER_NIGN;
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVWR = CHECK_FIX_NUMBER_TEN;
    return AT_OK;
}
/*****************************************************************************
 函 数 名  : SetWiFiSsidParaCheck
 功能描述  : check Wifi的ssid参数输入
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 SetWiFiSsidParaCheck(VOS_UINT8 ucIndex)
{

    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :wifi not suppt\n");
        return AT_ERROR;
    }

    /* SSID最多4组 */
    if (gastAtParaList[0].ulParaValue >= AT_WIFI_MAX_SSID_NUM)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :g_bAtDataLocked not unlock\n");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_SetWiFiSsidPara(VOS_UINT8 ucIndex)
{
    TAF_AT_MULTI_WIFI_SSID_STRU         stWifiSsid;
    VOS_UINT8                           ucGroup;
    VOS_UINT32 wifissidcheckresult = AT_ERROR;

    /* 参数过多*/
    if (2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*参数长度过长*/
    if (gastAtParaList[1].usParaLen >= AT_WIFI_SSID_LEN_MAX)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    wifissidcheckresult = SetWiFiSsidParaCheck(ucIndex);
    if(AT_OK != wifissidcheckresult)
    {
        return wifissidcheckresult;
    }
    else
    {
        /*do nothing*/
    }

    PS_MEM_SET(&stWifiSsid, 0, sizeof(stWifiSsid));

    ucGroup = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /*读取WIFI KEY对应的NV项*/
    if (VOS_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_STATUS_SSID,&stWifiSsid, sizeof(TAF_AT_MULTI_WIFI_SSID_STRU)))
    {
        AT_WARN_LOG("AT_SetWiFiSsidPara:READ NV ERROR");
        return AT_ERROR;
    }
    else
    {
        PS_MEM_CPY(&(stWifiSsid.aucWifiSsid[ucGroup][0]), gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
        stWifiSsid.aucWifiSsid[ucGroup][gastAtParaList[1].usParaLen] = '\0';

        /*写入WIFI SSID对应的NV项*/
        if (VOS_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_STATUS_SSID,&stWifiSsid, sizeof(TAF_AT_MULTI_WIFI_SSID_STRU)))
        {
            AT_WARN_LOG("AT_SetWiFiSsidPara:WRITE NV ERROR");
            return AT_ERROR;
        }
    }

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : SetWiFiKeyParaCheck
 功能描述  : check Wifi的key参数输入
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 SetWiFiKeyParaCheck(VOS_UINT8 ucIndex)
{

    VOS_UINT8  pindex;
    VOS_UINT8 *paucPara = NULL;

    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :wifi not support\n");
        return AT_ERROR;
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :not unlock\n");
        return AT_ERROR;
    }
    /* AT^WIKEY 协议支持 8-63位ASCII码或64位16进制*/

    /*AT^NFCCFG与AT^SSID都会调用该接口写SSID,且均为传入参数的最后一个参数，
    因此采用[gucAtParaIndex - 1]来获取SSID 参数进行有效性判断*/

    if (AT_WIFI_KEY_LEN_MAX < gastAtParaList[gucAtParaIndex - 1].usParaLen ||
        AT_WIFI_KEY_LEN_MIN > gastAtParaList[gucAtParaIndex - 1].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else if(AT_WIFI_KEY_LEN_MAX == gastAtParaList[gucAtParaIndex - 1].usParaLen)
    {
        paucPara = gastAtParaList[gucAtParaIndex - 1].aucPara;
        /*wikey长度为64时，只支持16进制*/
        for(pindex = 0; pindex < gastAtParaList[gucAtParaIndex - 1].usParaLen ; pindex++)
        {
            if(((paucPara[pindex] >= '0') && (paucPara[pindex] <= '9'))
               || ((paucPara[pindex] >= 'a') && (paucPara[pindex] <= 'f'))
               || ((paucPara[pindex] >= 'A') && (paucPara[pindex] <= 'F')))
            {
                //do nothing;
            }
            else
            {
                AT_WARN_LOG("SetWiFiSsidParaCheck :Para error\n");
                return AT_ERROR;
            }
        }
    }
    else
    {
        //for lint;
    }
    /* 最多4组SSID */
    if (gastAtParaList[0].ulParaValue >= AT_WIFI_MAX_SSID_NUM)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;

}


VOS_UINT32 AT_SetWiFiKeyPara(VOS_UINT8 ucIndex)
{
    TAF_AT_MULTI_WIFI_SEC_STRU          stWifiKey;
    VOS_UINT8                           ucGroup;
    VOS_UINT32 wifikeycheckresult = AT_ERROR;

    /* 参数过多*/
    if (2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度过长 */
    if (gastAtParaList[1].usParaLen > AT_WIFI_WLWPAPSK_LEN)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }


    wifikeycheckresult = SetWiFiKeyParaCheck(ucIndex);
    if(AT_OK != wifikeycheckresult)
    {
        return wifikeycheckresult;
    }
    else
    {
        /*do nothing*/
    }


    PS_MEM_SET(&stWifiKey, 0, sizeof(stWifiKey));

    ucGroup = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 读取WIFI KEY对应的NV项 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY,&stWifiKey, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU)))
    {
        AT_WARN_LOG("AT_SetWiFiKeyPara:READ NV ERROR");
        return AT_ERROR;
    }
    else
    {
        /* 写入KEY */
        PS_MEM_CPY(&(stWifiKey.aucWifiWpapsk[ucGroup][0]), gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
        stWifiKey.aucWifiWpapsk[ucGroup][gastAtParaList[1].usParaLen] = '\0';


        if (NV_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY, &stWifiKey, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU)))
        {
            AT_WARN_LOG("AT_SetWiFiKeyPara:WRITE NV ERROR");
            return AT_ERROR;
        }
    }


    return AT_OK;
}
/* Modified by f62575 for AT Project, 2011-10-28, end */
/*****************************************************************************
 函 数 名  : AT_SetNFCCFGPara
 功能描述  : 设置NFC WIFI KEY与WIFI SSID
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetNFCCFGPara(VOS_UINT8 ucIndex)
{

    VOS_UINT32                wifikeycheckresult = AT_ERROR;
    VOS_UINT32                wifissidcheckresult = AT_ERROR;
    VOS_UINT16                nfcwriteresult = 0;
    VOS_UINT16                nfcstatus = 0;
    huawei_nfc_token          nfc_info = {0};

    /* 参数过多*/
    if (3 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        /*do thing*/
    }

    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen
        || 0 == gastAtParaList[2].usParaLen)/*^NFCCFG写入时候传入3个参数，第1,2,3个参数的长度均不能为0*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*参数长度过长*/
    if ((gastAtParaList[1].usParaLen >= AT_WIFI_SSID_LEN_MAX) || (gastAtParaList[2].usParaLen >= AT_WIFI_WLWPAPSK_LEN))
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        /*do thing*/
    }

    wifissidcheckresult = SetWiFiSsidParaCheck(ucIndex);

    if(AT_OK != wifissidcheckresult)
    {
        AT_WARN_LOG("AT_SetNFCCFGPara:SetWiFiSsidParaCheck fail");
        return wifissidcheckresult;
    }
    else
    {
        /*do nothing*/
    }


    wifikeycheckresult = SetWiFiKeyParaCheck(ucIndex);
    if(AT_OK != wifikeycheckresult)
    {
        AT_WARN_LOG("AT_SetNFCCFGPara:SetWiFiKeyParaCheck fail");
        return wifikeycheckresult;
    }
    else
    {
        /*do nothing*/
    }

    /*在此需要调用NFC 驱动接口，判断NFC 是否已经启动*/
    nfcstatus = huawei_nfc_is_initialized();
    if(0 == nfcstatus)
    {
        AT_WARN_LOG("AT_SetNFCCFGPara:huawei_nfc_is_initialized fail");
        return AT_ERROR;
    }
    else
    {
        /*do nothing*/
    }

    /*内存清0*/
    PS_MEM_SET(&nfc_info, 0, sizeof(nfc_info));

    /*拷贝WIFI SSID */
    PS_MEM_CPY(&(nfc_info.NetworkSSID[0]), gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
    nfc_info.NetworkSSID[gastAtParaList[1].usParaLen] = '\0';

    /*拷贝WIFI KEY 传入的第3个参数，下标2*/
    PS_MEM_CPY(&(nfc_info.NetworkKey[0]), gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen);
    /*拷贝WIFI KEY 传入的第3个参数，下标2，结尾进行'\0'，补齐*/
    nfc_info.NetworkKey[gastAtParaList[2].usParaLen] = '\0';

    nfcwriteresult = huawei_nfc_write_info(&nfc_info);
    /*关键信息，使用结束后清0*/
    PS_MEM_SET(&nfc_info, 0, sizeof(nfc_info));
    if(0 == nfcwriteresult)
    {
        AT_WARN_LOG("AT_SetNFCCFGPara:huawei_nfc_write_info fail");
        return AT_ERROR;
    }
    else
    {
        /*do nothing*/
    }

    return AT_OK;

}
/*****************************************************************************
 函 数 名  : AT_QryNFCCFGPavars
 功能描述  : 查询NFC WIFI KEY与WIFI SSID
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryNFCCFGPavars(VOS_UINT8 ucIndex)
{
    VOS_UINT16                       usLen = 0;
    VOS_UINT16                       nfcreadesult = 0;
    VOS_UINT32                       nfcstatus = 0;
    huawei_nfc_token                 nfc_info = {0};

    /*在此需要调用NFC 驱动接口，判断NFC 是否已经启动*/
    nfcstatus = huawei_nfc_is_initialized();
    if(0 == nfcstatus)
    {
        AT_WARN_LOG("AT_QryNFCCFGPavars:huawei_nfc_is_initialized fail");
        return AT_ERROR;
    }
    else
    {
        //do nothig
    }

    /*数组清0*/
    PS_MEM_SET(&nfc_info, 0 ,sizeof(nfc_info));

    nfcreadesult = huawei_nfc_read_info(&nfc_info);
    if(0 == nfcreadesult)
    {
        AT_WARN_LOG("AT_QryNFCCFGPavars:huawei_nfc_read_info fail");
        return AT_ERROR;
    }

    usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                    (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                    "%s:%d,%s%s%s,%s%s%s%s",
                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                    0,
                                    gaucAtQuotation,nfc_info.NetworkSSID,gaucAtQuotation,
                                    gaucAtQuotation,nfc_info.NetworkKey,gaucAtQuotation,gaucAtCrLf);

    /* 去掉字符串结尾回车换行 */
    gstAtSendData.usBufLen = usLen - (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : At_TestNFCCFGPara
 功能描述  : 查询NFC 是否启动
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 At_TestNFCCFGPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                       usLen = 0;
    VOS_UINT32                       nfcstatus = 0;

    nfcstatus = huawei_nfc_is_initialized();

    usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                    (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                    "%s:%d%s",
                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                    nfcstatus,gaucAtCrLf);

    gstAtSendData.usBufLen = usLen - (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);

    return AT_OK;
}


VOS_UINT32 AT_SetWifiDebug(VOS_UINT8 ucIndex)
{
    VOS_INT32 ret = 0;
    VOS_UINT32 idx = 0;
    VOS_UINT32 reg_addr = 0;
    VOS_UINT32 reg_val = 0;
    VOS_UINT32 *pul_cali_info = NULL;
    BSP_MODULE_SUPPORT_E bSupport = BSP_MODULE_SUPPORT;
    HI1151_EQUIP_CALI_INFO_STRU g_st_equip_cali_info;

    MBB_MEM_SET_S(&g_st_equip_cali_info, sizeof(g_st_equip_cali_info), 0, sizeof(g_st_equip_cali_info));

    bSupport = mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI);
    if (BSP_MODULE_UNSUPPORT == bSupport)
    {
        AT_WARN_LOG("AT^WIFIDEBUG MODULE_TYPE_WIFI not support!");
        return AT_ERROR;
    }

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT^WIFIDEBUG format error!");
        return AT_ERROR;
    }

    if (AT_WIDEBUG_REG_READ == gastAtParaList[0].ulParaValue)
    {
        /* 检测输入参数是否为3个 */
        if (3 != gucAtParaIndex)
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 2代表第3个输入的参数,每次最多读100个寄存器 */
        if ((1 > gastAtParaList[2].ulParaValue) || (100 < gastAtParaList[2].ulParaValue))
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 读寄存器 */
        reg_addr = gastAtParaList[1].ulParaValue;   /* 1代表输入第2个的参数,待读取寄存器的地址 */
        reg_val = 0;

        /* 2代表第3个输入的参数,循环读取 */
        for (idx = 0; idx < gastAtParaList[2].ulParaValue; idx++)
        {
            AT_WARN_LOG("Read register");
            ret = WlanATSetWifiDebug(AT_WIDEBUG_REG_READ, reg_addr, &reg_val);
            if (AT_RETURN_SUCCESS != ret)
            {
                AT_WARN_LOG("Read register fail!");
                return AT_ERROR;
            }

            /* 打印寄存器值到AT端口 */
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                  "Reg addr : 0x%08X,  value : 0x%X\r\n",
                                                  reg_addr, reg_val);

            /* 每次读4个字节,读完地址自增 */
            reg_addr += 4; 
        }
    }
    else if (AT_WIDEBUG_REG_WRITE == gastAtParaList[0].ulParaValue)
    {
        /* 检测输入参数是否为3个 */
        if (3 != gucAtParaIndex)
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 写寄存器 */
        reg_addr = gastAtParaList[1].ulParaValue;   /* 1代表输入第2个的参数,得到待写入寄存器的地址 */
        reg_val = gastAtParaList[2].ulParaValue;    /* 2代表输入第3个的参数,得到待写入的值 */

        AT_WARN_LOG("Write register");
        ret = WlanATSetWifiDebug(AT_WIDEBUG_REG_WRITE, reg_addr, &reg_val);
        if (AT_RETURN_SUCCESS != ret)
        {
            AT_WARN_LOG("Write register fail!");
            return AT_ERROR;
        }
    }
    else if (AT_WIDEBUG_REG_FILE == gastAtParaList[0].ulParaValue)
    {
        /* 检测输入参数是否为1个 */
        if (1 != gucAtParaIndex)
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 写寄存器值到文件 */
        AT_WARN_LOG("Register value write to file");
        ret = WlanATSetWifiDebug(AT_WIDEBUG_REG_FILE, 0, NULL);
        if (AT_RETURN_SUCCESS != ret)
        {
            AT_WARN_LOG("Register value write to file fail!");
            return AT_ERROR;
        }
    }
    else if (AT_WIDEBUG_TEM_COM_SWTICH == gastAtParaList[0].ulParaValue)
    {
        /* 检测输入参数是否为2个 */
        if (2 != gucAtParaIndex)
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 切换温度补偿 */
        AT_WARN_LOG("Switch temperature compensate");
        ret = WlanATSetWifiDebug(AT_WIDEBUG_TEM_COM_SWTICH, gastAtParaList[1].ulParaValue, NULL);
        if (AT_RETURN_SUCCESS != ret)
        {
            AT_WARN_LOG("Switch temperature compensate fail!");
            return AT_ERROR;
        }
    }
    else if(AT_WIDEBUG_CALI_INTO_READ == gastAtParaList[0].ulParaValue)
    {
         /* 检测输入参数是否为1个 */
        if (1 != gucAtParaIndex)
        {
            AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
            return AT_ERROR;
        }

        /* 读取校准信息 */
        AT_WARN_LOG("Read cali info");
        ret = WlanATSetWifiDebug(AT_WIDEBUG_CALI_INTO_READ, 0, (VOS_UINT32 *)&g_st_equip_cali_info);
        if (AT_RETURN_SUCCESS != ret)
        {
            AT_WARN_LOG("Read register fail!");
            return AT_ERROR;
        }

        for (idx = 0; idx < (sizeof(HI1151_EQUIP_CALI_INFO_STRU)/sizeof(VOS_UINT32)); idx++)
        {
            /* 打印寄存器值到AT端口 */
            pul_cali_info = (VOS_UINT32 *)((VOS_UINT32 *)&g_st_equip_cali_info + idx);
            gstAtSendData.usBufLen += At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
            "Cali addr : %04d,  value : 0x%08X\r\n", idx, *pul_cali_info);
        }
    }
    else
    {
        AT_WARN_LOG("AT^WIFIDEBUG input param not accepted!");
        return AT_ERROR;
    }

    return AT_OK;
}

/******************************************************************************
                       MBB AT Device list
******************************************************************************/
AT_PAR_CMD_ELEMENT_STRU g_astMbbAtDeviceCmdTbl[] = {

	{AT_CMD_FLNA,
    At_SetFlnaPara,      AT_SET_PARA_TIME,   At_QryFlnaPara,        AT_QRY_PARA_TIME,   At_TestFlnaPara ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR,     CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FLNA",     (VOS_UINT8*)"(0-255)"},

	{AT_CMD_VERSION,
    At_SetVersionPara,        AT_NOT_SET_TIME,    At_QryVersion,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^VERSION",  VOS_NULL_PTR},

    {AT_CMD_TMMI,
    AT_SetTmmiPara,      AT_NOT_SET_TIME,    AT_QryTmmiPara,        AT_NOT_SET_TIME,   At_TestTmmiPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TMMI",  (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_WIMODE,
    AT_SetWiFiModePara,   AT_NOT_SET_TIME, AT_QryWiFiModePara,   AT_NOT_SET_TIME, At_TestWiFiModePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIMODE",  (VOS_UINT8*)"(0,1,2,3,4,5)"},
    
    {AT_CMD_WIBAND,
    AT_SetWiFiBandPara,   AT_NOT_SET_TIME, AT_QryWiFiBandPara,   AT_NOT_SET_TIME, At_TestWiFiBandPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIBAND",  (VOS_UINT8*)"(0,1,2,3)"},

    {AT_CMD_WICAL,
    AT_SetWifiCalPara, AT_NOT_SET_TIME, AT_QryWifiCalPara, AT_NOT_SET_TIME, AT_TestWifiCalPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICAL",  (VOS_UINT8*)"(0,1)"},

    {AT_CMD_WICALDATA,
    AT_SetWifiCalDataPara, AT_NOT_SET_TIME, AT_QryWifiCalDataPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALDATA",  (VOS_UINT8*)"(0-255),(0-32767),(0-32767),(0-32767),(0-32767),(0-32767),(@data)"},

    {AT_CMD_WICALTEMP,
    AT_SetWifiCalTempPara, AT_NOT_SET_TIME, AT_QryWifiCalTempPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALTEMP",  (VOS_UINT8*)"(0-255),(0-32767)"},

    {AT_CMD_WICALFREQ,
    AT_SetWifiCalFreqPara, AT_NOT_SET_TIME, AT_QryWifiCalFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALFREQ",  (VOS_UINT8*)"(0-1),(0-32767)"},

    {AT_CMD_WICALPOW,
    AT_SetWifiCalPowPara, AT_NOT_SET_TIME, AT_QryWifiCalPowPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALPOW",  (VOS_UINT8*)"(0-1),(0-32767)"},


    {AT_CMD_NAVTYPE,
    AT_SetNavTypePara, AT_NOT_SET_TIME, AT_QryNavTypePara, AT_NOT_SET_TIME, At_TestNavTypePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVTYPE",  (VOS_UINT8*)"(0-255)"},

    {AT_CMD_NAVENABLE,
    AT_SetNavEnablePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVENABLE",  (VOS_UINT8*)"(0-1)"},

    {AT_CMD_NAVFREQ,
    AT_SetNavFreqPara, AT_NOT_SET_TIME, AT_QryNavFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVFREQ",  (VOS_UINT8*)"(0-65535),(0-999),(0-999)"},

    {AT_CMD_NAVRX,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNavRxPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVRX",  VOS_NULL_PTR},


    {AT_CMD_SN,
    At_SetSnPara,        AT_NOT_SET_TIME,    At_QrySnPara,           AT_NOT_SET_TIME,    At_CmdTestProcOK, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_DEVICE_OTHER_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^SN",       VOS_NULL_PTR},

    {AT_CMD_LTEANTINFO,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryLteAntInfo,       AT_NOT_SET_TIME,    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^LTEANTINFO",       VOS_NULL_PTR},

    {AT_CMD_TNETPORT,
    AT_SetTnetPortPara,    AT_NOT_SET_TIME,    AT_QryTnetPortPara,      AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TNETPORT",   VOS_NULL_PTR},

    {AT_CMD_EXTCHARGE,
    VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryExtChargePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^EXTCHARGE",   VOS_NULL_PTR},
    
    {AT_CMD_WUSITE,
    AT_SetWebSitePara,   AT_NOT_SET_TIME,    AT_QryWebSitePara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUSITE", (VOS_UINT8*)"(@WUSITE)"},

    {AT_CMD_WIPIN,
    AT_SetWiFiPinPara,   AT_NOT_SET_TIME,    AT_QryWiFiPinPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPIN", (VOS_UINT8*)"(@WIPIN)"},

    {AT_CMD_WUUSER,
    AT_SetWebUserPara,   AT_NOT_SET_TIME,    AT_QryWebUserPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUUSER", (VOS_UINT8*)"(@WUUSER)"},

    {AT_CMD_PORTLOCK,
    At_SetPortLockPara, AT_NOT_SET_TIME, At_QryPortLockPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PORTLOCK",   VOS_NULL_PTR},

    {AT_CMD_TBATDATA,
    AT_SetTbatDataPara, AT_NOT_SET_TIME, At_QryTbatDataPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TBATDATA",   (VOS_UINT8 *)"(0,1),(0-255),(0-65535)"},

    {AT_CMD_WIPLATFORM,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryWiFiPlatformPara,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPLATFORM",  VOS_NULL_PTR},

    {AT_CMD_ANTENNA,
    AT_SetAntennaPara,     AT_NOT_SET_TIME, AT_QryAntennaPara,     AT_NOT_SET_TIME, At_TestAntenna, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^ANTENNA",  (VOS_UINT8 *)"(0,1,2)"},

    {AT_CMD_TEMPINFO,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryTempInfo,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TEMPINFO",  VOS_NULL_PTR},
	
    {AT_CMD_FTYRESET,
    AT_SetFtyResetPara, AT_NOT_SET_TIME, AT_QryFtyResetPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FTYRESET",  (VOS_UINT8 *)"(0,1,2)"},

    {AT_CMD_SBSN,
    AT_SetSbSnPara, AT_NOT_SET_TIME, AT_QrySbSnPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SBSN",  VOS_NULL_PTR},



    {AT_CMD_SETHWLOCK,
    At_SetHWLock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SETHWLOCK",  VOS_NULL_PTR},
    
    {AT_CMD_TESTHWLOCK,
    At_TestHWlock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TESTHWLOCK",  VOS_NULL_PTR},



	{AT_CMD_SFM,
    At_SetSfm,          AT_SET_PARA_TIME,   At_QrySfm,            AT_NOT_SET_TIME,    At_TestSfm , AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SFM",     (VOS_UINT8*)"(0,1,2)"},



    {AT_CMD_TLEDSWITCH,
    AT_SetTledSwitchPara, AT_SET_PARA_TIME,  At_QryTledSwitchPara,  AT_QRY_PARA_TIME,    At_TestTledSwitchPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TLEDSWITCH",    (VOS_UINT8*)"(0-255),(0-255)"},

    /*sleep and wakeup*/
/*lint -e553*/

    {AT_CMD_WIPAVARS2G,
    AT_SetWiFi2GPavars, AT_SET_PARA_TIME,  AT_QryWiFi2GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPAVARS2G",    (VOS_UINT8*)"(0-1),(@data),(@data),(@data)"},

    {AT_CMD_WIPAVARS5G,
    AT_SetWiFi5GPavars, AT_SET_PARA_TIME,  AT_QryWiFi5GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPAVARS5G",    (VOS_UINT8*)"(0-2),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},

    {AT_CMD_NVRDWIFI2G,
    AT_SetNVReadWiFi2GPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVRDWI2G",(VOS_UINT8*)"(0-1)"},

    {AT_CMD_NVWRWIFI2G,
    AT_SetNVWriteWiFi2GPara,    AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWRWI2G",(VOS_UINT8*)"(0-1),(@data),(@data),(@data)"},
    
    {AT_CMD_NVRDWIFI5G,
    AT_SetNVReadWiFi5GPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVRDWI5G",(VOS_UINT8*)"(0-2)"},

    {AT_CMD_NVWRWIFI5G,
    AT_SetNVWriteWiFi5GPara,    AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWRWI5G",(VOS_UINT8*)"(0-2),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},

    {AT_CMD_WIFIDEBUG,
    AT_SetWifiDebug,    AT_NOT_SET_TIME,    VOS_NULL_PTR,    AT_NOT_SET_TIME,    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,       AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8 *)"^WIFIDEBUG", (VOS_UINT32*)"(1-5), (0-0xFFFFFFFF), (0-0xFFFFFFFF)"},
    /* 5代表5种命令格式 */

    {AT_CMD_NFCCFG,
    AT_SetNFCCFGPara,    AT_SET_PARA_TIME,    AT_QryNFCCFGPavars,    AT_QRY_PARA_TIME,    At_TestNFCCFGPara,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NFCCFG",    (VOS_UINT8*)"(0-3),(@WIKEY),(@SSID)"},
};/*g_astMbbAtDeviceCmdTbl*/


VOS_UINT32 At_RegisterDeviceMbbCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astMbbAtDeviceCmdTbl, sizeof(g_astMbbAtDeviceCmdTbl)/sizeof(g_astMbbAtDeviceCmdTbl[0]));
}



