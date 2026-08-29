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

#ifndef __MBB_SEC_H__
#define __MBB_SEC_H__

#include "vos.h"
#include "TafTypeDef.h"

#ifdef _EXPORT_PRIVATE
#define MBB_MEM_SET_S(pAddr, DstLen, ucData, len)    memset(pAddr, ucData, len)
#define MBB_MEM_CPY_S(pDst, DstLen, pSrc, len)       memcpy(pDst, pSrc, len)
#define MBB_MEM_MOVE_S(pDst, DstLen, pSrc, len)      memmove(pDst, pSrc, len)
#define MBB_MEM_FREE(ulPid, pAddr)                   free(pAddr)
#define MBB_MEM_ALLOC(ulPid, ulSize)                 malloc(ulSize)
#define MBB_STR_NLEN(pAddr, len)                     strnlen((const char*)pAddr, len)
#define MBB_STR_NCPY_S(pAddr,DstLen,pSrc, Srclen)    strncpy_s((char*)pAddr, DstLen, pSrc, Srclen)

#else
#define MBB_MEM_CPY_S(pDst, DstLen, pSrc, len) { \
        if (VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)(pDst), DstLen, (const VOS_VOID *)(pSrc), len)) \
        {\
            mdrv_om_system_error(TAF_REBOOT_MOD_ID_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0 ); \
        }\
    }

#define MBB_MEM_SET_S(pAddr, DstLen, ucData, len) { \
        if (VOS_NULL_PTR == VOS_MemSet_s((VOS_VOID *)(pAddr), DstLen, (VOS_CHAR)(ucData), len)) \
        { \
            mdrv_om_system_error(TAF_REBOOT_MOD_ID_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0 ); \
        } \
    }

#define MBB_MEM_MOVE_S(pDst, DstLen, pSrc, len) { \
        if (VOS_NULL_PTR == VOS_MemMove_s((VOS_VOID *)(pDst), DstLen, (const VOS_VOID *)(pSrc), len )) \
        { \
            mdrv_om_system_error(TAF_REBOOT_MOD_ID_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0 ); \
        } \
    }

#define MBB_STR_NCPY_S(pDst, DstLen, pSrc, len) { \
        if (VOS_NULL_PTR == VOS_StrNCpy_s((VOS_CHAR *)(pDst), DstLen, (VOS_CHAR *)(pSrc), len )) \
        { \
            mdrv_om_system_error(TAF_REBOOT_MOD_ID_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0 ); \
        } \
    }

#define MBB_STR_NLEN(pAddr, len)                     VOS_StrNLen((VOS_CHAR *)(pAddr), len)

#define MBB_MEM_ALLOC(ulPid, ulSize)                 PS_MEM_ALLOC(ulPid, ulSize)
#define MBB_MEM_FREE(ulPid, pAddr)                   PS_MEM_FREE(ulPid,pAddr)

#endif /* #ifdef _EXPORT_PRIVATE */


#ifdef __cplusplus
extern "C"
{
#endif




#ifdef __cplusplus
}
#endif
#endif /*#ifndef __MBB_SEC_H__*/
