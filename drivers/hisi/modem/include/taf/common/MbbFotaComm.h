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

/******************************问题单修改记录**********************************
    日期          修改人               问题单号                 修改内容
******************************************************************************/

#ifndef __MBB_FOTA_COMM_H__
#define __MBB_FOTA_COMM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_WPG_FOTA)

/*FOTA安全方案整改宏定义*/

#ifndef FOTA_ECDSA_DATA_LEN
#define FOTA_ECDSA_DATA_LEN             (76) /*ECDSA待签名数据最大长度*/
#endif
#define FOTA_ECDSA_SIGN_LEN             (64) /*ECDSA签名长度*/
#define FOTA_ECDSA_SIGN_MODE            (0) /*^HWECDSA索取签名模式*/
#define FOTA_ECDSA_VERIFY_MODE          (1) /*^HWECDSA验证签名模式*/

#define FOTA_MN_MSG_MAX_LEN             (255) /*FOTA新方案MN_MSG最大长度*/
#define FOTA_MN_MAX_ASCII_ADDRESS_NUM   (20) /*FOTA新方案ASCII地址最大长度*/
#define TAF_FOTA_ECDSA_SIGN_LEN         (64)  /*FOTA签名数据长度*/

enum FOTA_PROFILE_LEN_ENUM
{
    FOTA_PROFILE_PDPTYPE_AUTH_LEN        = 1,  /*PDP类型长度*/
    FOTA_PROFILE_APN_USER_PW_LEN         = 99,  /*用户名密码长度*/
    FOTA_PROFILE_LEN
};

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16 usMsgName;
    VOS_UINT8  aucApn[FOTA_PROFILE_APN_USER_PW_LEN];
    VOS_UINT8  aucUserName[FOTA_PROFILE_APN_USER_PW_LEN];
    VOS_UINT8  aucPassWord[FOTA_PROFILE_APN_USER_PW_LEN];
    VOS_UINT8  ucApnLen;
    VOS_UINT8  ucUserNameLen;
    VOS_UINT8  ucPasswordLen;
    VOS_UINT8  ucPdpType;
    VOS_UINT8  ucAuthType;
    VOS_UINT8  ucNetworkMode;
    VOS_UINT8  ucPdpTypeLen;
    VOS_UINT8  ucAuthTypeLen;
    VOS_UINT8  ucNetworkModeLen;
}MN_MSG_FOTA_PROFILE_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16 usMsgName;
    VOS_UINT8  ucMode;
    VOS_UINT8  aucData[FOTA_ECDSA_DATA_LEN + 1];
    VOS_UINT8  aucSign[FOTA_ECDSA_SIGN_LEN + 1];
    VOS_UINT8  ucDatalen;
}MN_MSG_FOTA_ECDSA_STRU;


typedef struct
{
    VOS_UINT8  ucMode;
    VOS_UINT8  aucSign[TAF_FOTA_ECDSA_SIGN_LEN + 1];
    VOS_UINT8  aucReserve[2];                            /*字节对齐*/
}TAF_MMA_FOTA_ECDSA_STRU;

VOS_UINT32 Taf_SetECDSA(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    TAF_MMA_FOTA_ECDSA_STRU             *pstECDSA
);

#endif /* FEATURE_ON == MBB_WPG_FOTA */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__MBB_FOTA_COMM_H__ */






