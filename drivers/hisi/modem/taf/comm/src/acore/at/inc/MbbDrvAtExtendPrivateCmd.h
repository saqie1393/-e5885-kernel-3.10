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


#ifndef __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__
#define __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if(FEATURE_ON == MBB_COMMON)


/*****************************************************************************
  特性公共 函数申明
*****************************************************************************/
/* 注册函数，对外接口 */
VOS_UINT32 At_RegisterMbbDrvPrivateCmdTable(VOS_VOID);

#if (FEATURE_ON == MBB_FEATURE_BODYSAR)
VOS_UINT32 Mbb_AT_QryBodySarOnPara(VOS_UINT8 ucIndex);
#endif

#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
VOS_UINT32 Mbb_AT_QryChipTempPara(VOS_UINT8 ucIndex);
#endif

#if  (FEATURE_ON == MBB_FEATURE_OFFLINE_LOG)
VOS_UINT32 Mbb_AT_QryRsfrLogFs(VOS_CHAR* cSubName, VOS_CHAR* cName);
#endif

#if (FEATURE_ON == MBB_NFBBCHK)
/*****************************************************************************
 函 数 名  : Mbb_AT_GetCheckResultFilefd
 功能描述  : 获取检测结果文件的文件描述符resultfd
 输入参数  : 检测结果文件的路径.
 输出参数  : 无
 返 回 值     : 成功:File_Path对应的resultfd;     失败:-1
 调用函数  : Mbb_AT_SetNandBadBlockInfo
 被调函数  : sys_open

 修改历史  ：新增函数
*****************************************************************************/
VOS_INT32 Mbb_AT_GetCheckResultFilefd(VOS_VOID);

/*****************************************************************************
 函 数 名  : Mbb_AT_CloseCheckResultFilefd
 功能描述  : 关闭检测结果文件的文件描述符resultfd
 输入参数  : resultfd
 输出参数  : 无
 返 回 值     : 成功:0 ;     失败:-1
 调用函数  : Mbb_AT_SetNandBadBlockInfo
 被调函数  : sys_close

 修改历史  ：新增函数
*****************************************************************************/
VOS_INT32 Mbb_AT_CloseCheckResultFilefd(VOS_INT32 resultfd);

/*****************************************************************************
 函 数 名  : Mbb_AT_SetNandBadBlockInfo
 功能描述  : 根据分区信息表一个分区一个分区的检测，
                          任何分区出现不合格的情况都表示此NAND FLASH不合格!
 输入参数  : 无
 输出参数  : 无
 返 回 值  : FLASH的检测结果,返回0:合格，1:不合格,-1:出错!
 调用函数  :AT_SetNandBadBlockInfo
 被调函数  :Mtd_AT_Query_BadBlock,Mbb_AT_EmptyCheckResultFile,Mbb_AT_EmptyCheckResultFile

 修改历史      :新增函数
*****************************************************************************/
VOS_INT32 Mbb_AT_SetNandBadBlockInfo(VOS_VOID);
#endif /* MBB_NFBBCHK */

#endif/*#if(FEATURE_ON == MBB_COMMON)*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /* __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__ */
