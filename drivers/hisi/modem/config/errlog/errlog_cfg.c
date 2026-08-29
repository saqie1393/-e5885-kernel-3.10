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
#include "errlog_cfg.h"


//#define    THIS_FILE_ID        PS_FILE_ID_ERRLOG_CFG_C

/*****************************************************************************
  2 全局变量声明
*****************************************************************************/


/*****************************************************************************
  3 函数申明
*****************************************************************************/


VOS_INT32 ErrLog_GetErrReptAddrAndSize(
    VOS_UINT32                           ulModemId,
    VOS_UINT_PTR                        *pulErrLogAddr,
    VOS_UINT32                          *pulsize
)
{
    return VOS_ERR;


}


VOS_INT32 ErrLog_GetPidAndAlarmId(
    VOS_UINT32                          ulModemId,
    VOS_UINT32                          ulFaultId,
    ERR_LOG_ALARM_STRU                 *paustAlarmArray,
    VOS_UINT32                         *pulAlarmNum
)
{
    VOS_UINT_PTR                        ulErrLogAddr = VOS_NULL;
    VOS_UINT32                          ulsize;
    VOS_UINT32                          ulCount = 0;
    VOS_UINT32                          i;
    VOS_UINT32                          (*pulErrLogArry)[3];

    if (VOS_NULL_PTR == paustAlarmArray)
    {
        return VOS_ERR;
    }

    if (VOS_OK != ErrLog_GetErrReptAddrAndSize(ulModemId, &ulErrLogAddr, &ulsize))
    {
        return VOS_ERR;
    }

    pulErrLogArry = (VOS_UINT32(*)[])ulErrLogAddr;

    for (i=0; i < (ulsize / (3 * sizeof(VOS_UINT32))); i++)
    {
        if (ulFaultId == pulErrLogArry[i][0])
        {
            if (FAULT_ID_CONTAIN_ALARM_ID_MAX_NUM <= ulCount)
            {
                return VOS_ERR;
            }

            paustAlarmArray[ulCount].ulPid      = pulErrLogArry[i][1];
            paustAlarmArray[ulCount].ulAlarmID  = pulErrLogArry[i][2];
            ulCount++;
        }
    }

    *pulAlarmNum = ulCount;

    return VOS_OK;
}


VOS_BOOL ErrLog_IsContainPID(
    VOS_UINT32                         *PaulPidArray,
    VOS_UINT32                          ulPidArrayLen,
    VOS_UINT32                          ulDstPid
)
{
    VOS_UINT32                          i;

    for (i=0; i < ulPidArrayLen; i++)
    {
        if (ulDstPid == PaulPidArray[i])
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_INT32 ErrLog_GetErrlogPid(
    VOS_UINT32                          ulModemId,
    VOS_UINT32                         *PaulPidArray,
    VOS_UINT32                         *pulPidNum
)
{
    VOS_UINT_PTR                        ulErrLogAddr = VOS_NULL;
    VOS_UINT32                          ulsize;
    VOS_UINT32                          ulCount = 0;
    VOS_UINT32                          i;
    VOS_UINT32                          (*pulErrLogArry)[3];

    if (VOS_NULL_PTR == PaulPidArray)
    {
        return VOS_ERR;
    }

    if (VOS_OK != ErrLog_GetErrReptAddrAndSize(ulModemId, &ulErrLogAddr, &ulsize))
    {
        return VOS_ERR;
    }

    pulErrLogArry = (VOS_UINT32(*)[])ulErrLogAddr;

    for (i=0; i < (ulsize / (3 * sizeof(VOS_UINT32))); i++)
    {
        if (0 == i)
        {
            PaulPidArray[0] = pulErrLogArry[i][1];
            ulCount++;
            continue;
        }

        if (VOS_TRUE != ErrLog_IsContainPID(PaulPidArray, ulCount, pulErrLogArry[i][1]))
        {
            if (ERR_LOG_PID_MAX_NUM <= ulCount)
            {
                return VOS_ERR;
            }

            PaulPidArray[ulCount] = pulErrLogArry[i][1];
            ulCount++;
        }
    }

    *pulPidNum = ulCount;

    return VOS_OK;
}



