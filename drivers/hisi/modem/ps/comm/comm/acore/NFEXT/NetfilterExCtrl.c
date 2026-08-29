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
   1 头文件包含
******************************************************************************/
#include "v_typdef.h"
#include "PsTypeDef.h"
#include "IpsMntn.h"
#include "TtfOamInterface.h"
#include "TTFComm.h"
#include "NetfilterEx.h"
#include "TTFUtil.h"

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_ACPU_NFEX_CTRL_C


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 全局变量声明
*****************************************************************************/
NF_EXT_ENTITY_STRU                  g_stExEntity            = {0};
VOS_UINT32                          g_ulNFExtTaskId         = 0;
VOS_UINT32                          g_ulNFExtInitFlag       = 0;

#if(NF_EXT_DBG == DBG_ON)
NF_EXT_STATS_STRU                   g_stNfExtStats = {{0}};
#endif


/******************************************************************************
 Prototype       : NFExt_SndDataNotify
 Description     : 发送数据处理指示NFEXT_DATA_PROC_NOTIFY
 Input           :
 Output          : NONE
 Return Value    : PS_SUCC   --- 成功
                   PS_FAIL   --- 失败
 History         :
   1.Date        : 2008-07-16
     Author      : l47619
     Modification: 增加PPP数据队列处理机制
******************************************************************************/
VOS_VOID NFExt_SndDataNotify(VOS_VOID)
{
    (VOS_VOID)VOS_EventWrite(g_ulNFExtTaskId, NFEXT_DATA_PROC_NOTIFY);

    return;
} /* NFExt_SndDataNotify */


/*lint -e{550,438} */
VOS_INT NFExt_RingBufferPut( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT nbytes )
{
    VOS_ULONG   ulFlags = 0UL;
    VOS_INT     iRst;

    iRst = 0;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    if ((VOS_UINT32)OM_RingBufferFreeBytes(g_stExEntity.pRingBufferId) >= sizeof(NF_EXT_DATA_RING_BUF_STRU) )
    {
        iRst = OM_RingBufferPut(rngId, buffer, nbytes);
    }
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    return iRst;
}


/*lint -e{550,438} */
VOS_INT NFExt_RingBufferGet( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT maxbytes )
{
    VOS_ULONG   ulFlags = 0UL;
    VOS_INT     iRst;

    iRst = 0;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    if (!OM_RingBufferIsEmpty(rngId))
    {
        iRst = OM_RingBufferGet(rngId, buffer, maxbytes );
    }
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    return iRst;
}


/*lint -e{550,438} */
VOS_VOID NFExt_FlushRingBuffer(OM_RING_ID rngId)
{
    NF_EXT_DATA_RING_BUF_STRU   stData;
    VOS_ULONG                   ulFlags = 0UL;
    VOS_INT                     iRst = 0;

    /* 初始化 */
    PSACORE_MEM_SET(&stData, sizeof(stData), 0x0, sizeof(stData));

    while (!OM_RingBufferIsEmpty(rngId))
    {
        iRst = NFExt_RingBufferGet(rngId, (VOS_CHAR*)(&stData), sizeof(NF_EXT_DATA_RING_BUF_STRU));
        if (iRst == sizeof(NF_EXT_DATA_RING_BUF_STRU))
        {
            NF_EXT_MEM_FREE(ACPU_PID_NFEXT, stData.pData);
        }
        else
        {
            TTF_LOG1(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                    "NFExt_FlushRingBuffer : ERROR : Get data error from ring buffer!", iRst);

            break;
        }
    }

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    OM_RingBufferFlush(rngId);
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);
}



VOS_UINT32 NFExt_AddDataToRingBuf(NF_EXT_DATA_RING_BUF_STRU *pstData)
{
    VOS_UINT32                  ulRst           = VOS_OK;
    VOS_UINT32                  ulNeedWakeUp    = VOS_FALSE;
    VOS_INT                     iRst;
    NF_EXT_DATA_RING_BUF_STRU   stData;

    if ( VOS_NULL_PTR == g_stExEntity.pRingBufferId )
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"NFExt_AddDataToRingBuf: Warning : g_stExEntity.pRingBufferId is null!\n");
        return VOS_ERR;
    }

    /* 空到非空，唤醒任务处理勾包 */
    if (OM_RingBufferIsEmpty(g_stExEntity.pRingBufferId))
    {
        ulNeedWakeUp = VOS_TRUE;
    }

    iRst = NFExt_RingBufferPut(g_stExEntity.pRingBufferId, (VOS_CHAR *)pstData, (VOS_INT)(sizeof(NF_EXT_DATA_RING_BUF_STRU)));
    if (sizeof(NF_EXT_DATA_RING_BUF_STRU) == iRst)
    {
        if (VOS_TRUE == ulNeedWakeUp)
        {
            NFExt_SndDataNotify();
        }

        ulRst = VOS_OK;
    }
    else if (0 == iRst)
    {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_BUF_FULL_DROP);

        /* 队列满，唤醒任务处理勾包 */
        NFExt_SndDataNotify();

        ulRst = VOS_ERR;
    }
    else
    {
        TTF_LOG2(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,
                "RingBufferPut Fail found ulRst = %u, sizeof=%u \r\n", iRst, sizeof(NF_EXT_DATA_RING_BUF_STRU));

        (VOS_VOID)NFExt_RingBufferGet(g_stExEntity.pRingBufferId, (VOS_CHAR *)(&stData), iRst);

        NF_EXT_STATS_INC(1, NF_EXT_STATS_PUT_BUF_FAIL);

        ulRst = VOS_ERR;
    }

    return ulRst;
}


VOS_VOID NFExt_RcvNfExtInfoCfgReq(VOS_VOID *pMsg)
{
    OM_IPS_MNTN_INFO_CONFIG_REQ_STRU    *pstNfExtCfgReq;
    IPS_OM_MNTN_INFO_CONFIG_CNF_STRU    stNfExtCfgCnf;
    IPS_MNTN_RESULT_TYPE_ENUM_UINT32    enResult;

    pstNfExtCfgReq  = (OM_IPS_MNTN_INFO_CONFIG_REQ_STRU *)pMsg ;

    enResult        = IPS_MNTN_RESULT_OK;

    /*================================*/
    /*构建回复消息*/
    /*================================*/

    /* Fill DIAG trans msg header */
    stNfExtCfgCnf.stDiagHdr.ulSenderCpuId   = VOS_LOCAL_CPUID;
    stNfExtCfgCnf.stDiagHdr.ulSenderPid     = ACPU_PID_NFEXT;
    stNfExtCfgCnf.stDiagHdr.ulReceiverCpuId = VOS_LOCAL_CPUID;
    stNfExtCfgCnf.stDiagHdr.ulReceiverPid   = MSP_PID_DIAG_APP_AGENT;   /* 把应答消息发送给DIAG，由DIAG把透传命令的处理结果发送给HIDS工具*/
    stNfExtCfgCnf.stDiagHdr.ulLength        = sizeof(IPS_OM_MNTN_INFO_CONFIG_CNF_STRU) - VOS_MSG_HEAD_LENGTH;

    stNfExtCfgCnf.stDiagHdr.ulMsgId         = ID_IPS_OM_MNTN_INFO_CONFIG_CNF;

    /* DIAG透传命令中的特定信息*/
    stNfExtCfgCnf.stDiagHdr.usOriginalId  = pstNfExtCfgReq->stDiagHdr.usOriginalId;
    stNfExtCfgCnf.stDiagHdr.usTerminalId  = pstNfExtCfgReq->stDiagHdr.usTerminalId;
    stNfExtCfgCnf.stDiagHdr.ulTimeStamp   = pstNfExtCfgReq->stDiagHdr.ulTimeStamp;
    stNfExtCfgCnf.stDiagHdr.ulSN          = pstNfExtCfgReq->stDiagHdr.ulSN;

    /* 填充回复OM申请的确认信息 */
    stNfExtCfgCnf.stIpsMntnCfgCnf.enCommand  = pstNfExtCfgReq->stIpsMntnCfgReq.enCommand;
    stNfExtCfgCnf.stIpsMntnCfgCnf.enRslt     = enResult;

    /* 发送OM透明消息 */
    IPS_MNTN_SndCfgCnf2Om( ID_IPS_OM_MNTN_INFO_CONFIG_CNF,
        sizeof(IPS_OM_MNTN_INFO_CONFIG_CNF_STRU), &stNfExtCfgCnf );

    return;

}


VOS_VOID NFExt_RcvOmMsg(VOS_VOID *pMsg)
{
    VOS_UINT16          usMsgId;

    usMsgId = (VOS_UINT16)(*((VOS_UINT32 *)((VOS_UINT8 *)(pMsg) + VOS_MSG_HEAD_LENGTH)));

    switch ( usMsgId )
    {
        case ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ:
            IPS_MNTN_TraceAdvancedCfgReq(pMsg);
            break;

        case ID_OM_IPS_MNTN_INFO_CONFIG_REQ:
            NFExt_RcvNfExtInfoCfgReq(pMsg);
            break;

        default:
            TTF_LOG1(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,
                "NFExt_RcvConfig:Receive Unkown Type Message !\n", usMsgId);
            break;
    }

    return;
}

/******************************************************************************
 Prototype       : NFExt_BindToCpu
 Description     : 绑定Task到指定CPU上面
 Input           :
 Output          : NONE
 Return Value    : PS_SUCC   --- 成功
                   PS_FAIL   --- 失败
 History         :
   1.Date        : 2016-06-16
     Author      :
     Modification:
******************************************************************************/
VOS_VOID NFExt_BindToCpu(VOS_VOID)
{
    VOS_LONG            ret;
    pid_t               target_pid;
    VOS_INT             cpu;

    /* 获取当前线程的Pid */
    target_pid = current->pid;

    /* 获取当前线程的affinity */
    ret = sched_getaffinity(target_pid, &(g_stExEntity.orig_mask));
    if (ret < 0)
    {
        PS_PRINTF("warning: unable to get cpu affinity\n");
        return;
    }

    PSACORE_MEM_SET(&(g_stExEntity.curr_mask), cpumask_size(), 0, cpumask_size());

    /* 设置当前线程的affinity */
    for_each_cpu(cpu, &(g_stExEntity.orig_mask))
    {
        /* 去绑定CPU0 */
        if ((0 < cpu) && (cpumask_test_cpu(cpu, &(g_stExEntity.orig_mask))))
        {
            cpumask_set_cpu((unsigned int)cpu, &(g_stExEntity.curr_mask));
        }
    }

    if (0 == cpumask_weight(&(g_stExEntity.curr_mask)))
    {
        cpumask_set_cpu(0, &(g_stExEntity.curr_mask));
        return;
    }

    ret = sched_setaffinity(target_pid, &(g_stExEntity.curr_mask));
    if (ret < 0)
    {
        PS_PRINTF("warning: unable to set cpu affinity\n");
        return;
    }

    return;
}


VOS_VOID NFExt_ProcDataNotify(VOS_VOID)
{
    NF_EXT_DATA_RING_BUF_STRU   stData;
    VOS_INT                     iRst;
    DIAG_TRANS_IND_STRU        *pstDiagTransData;
    VOS_UINT32                  ulDealCntOnce;

    if (VOS_NULL_PTR == g_stExEntity.pRingBufferId)
    {
        TTF_LOG(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
            "NFExt_ProcDataNotify : ERROR : pRingBufferId is NULL!" );
        return;
    }

    ulDealCntOnce = 0;

    while (!OM_RingBufferIsEmpty(g_stExEntity.pRingBufferId))
    {
        /* 一次任务调度，最多处理200个勾包 */
        if (NF_ONCE_DEAL_MAX_CNT <= ulDealCntOnce)
        {
            NFExt_SndDataNotify();
            break;
        }

        iRst = NFExt_RingBufferGet(g_stExEntity.pRingBufferId, (VOS_CHAR *)&stData, sizeof(NF_EXT_DATA_RING_BUF_STRU));
        if (sizeof(NF_EXT_DATA_RING_BUF_STRU) == iRst)
        {
            pstDiagTransData = (DIAG_TRANS_IND_STRU *)(stData.pData);

            if ( VOS_OK != DIAG_TransReport(pstDiagTransData))
            {
                TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_ERROR, "NFExt_ProcDataNotify, ERROR, Call DIAG_TransReport fail!");
            }

            NF_EXT_MEM_FREE(ACPU_PID_NFEXT, stData.pData);
        }
        else if (0 == iRst)
        {
            TTF_LOG(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                "NFExt_ProcDataNotify : ERROR : Get null from ring buffer!");

            break;
        }
        else
        {
            TTF_LOG2(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                "NFExt_ProcDataNotify : ERROR : Get data error from ring buffer!", iRst, sizeof(NF_EXT_DATA_RING_BUF_STRU));

            NF_EXT_STATS_INC(1, NF_EXT_STATS_GET_BUF_FAIL);

            NFExt_FlushRingBuffer(g_stExEntity.pRingBufferId);

            break;
        }

        ulDealCntOnce++;
    }
}


VOS_VOID NFExt_EventProc(VOS_UINT32 ulEvent)
{
    if (ulEvent & NFEXT_DATA_PROC_NOTIFY)
    {
        NFExt_ProcDataNotify();
    }

    return;
}


VOS_VOID NFExt_MsgProc( struct MsgCB * pMsg )
{
    if ( VOS_NULL_PTR == pMsg )
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"NFExt_MsgProc: Message is NULL !" );
        return;
    }

    switch ( pMsg->ulSenderPid )
    {
        case MSP_PID_DIAG_APP_AGENT:      /* 来自OM的透传消息处理 */
            NFExt_RcvOmMsg( (void *)pMsg );
            break;

        default:
            break;
    }

    return;
}


VOS_VOID NFExt_FidTask(VOS_VOID)
{
    MsgBlock                           *pMsg          = VOS_NULL_PTR;
    VOS_UINT32                          ulEvent       = 0;
    VOS_UINT32                          ulTaskID      = 0;
    VOS_UINT32                          ulRtn         = VOS_ERR;
    VOS_UINT32                          ulEventMask   = 0;
    VOS_UINT32                          ulExpectEvent = 0;

    ulTaskID = VOS_GetCurrentTaskID();
    if (PS_NULL_UINT32 == ulTaskID)
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: TaskID is invalid.");
        return;
    }

    if (VOS_OK != VOS_CreateEvent(ulTaskID))
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: create event fail.");
        return;
    }

    g_ulNFExtTaskId = ulTaskID;

    NFExt_BindToCpu();

    ulExpectEvent = NFEXT_DATA_PROC_NOTIFY | VOS_MSG_SYNC_EVENT;
    ulEventMask   = VOS_EVENT_ANY | VOS_EVENT_WAIT;

    for ( ; ; )
    {
        ulRtn = VOS_EventRead(ulExpectEvent, ulEventMask, 0, &ulEvent);
        if (VOS_OK != ulRtn)
        {
            TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: read event error.");

            continue;
        }

        /* 事件处理 */
        if (VOS_MSG_SYNC_EVENT != ulEvent)
        {
            NFExt_EventProc(ulEvent);

            continue;
        }

        pMsg = (MsgBlock *)VOS_GetMsg(ulTaskID);
        if (VOS_NULL_PTR != pMsg)
        {
            if (ACPU_PID_NFEXT == pMsg->ulReceiverPid)
            {
                NFExt_MsgProc(pMsg);
            }

            (VOS_VOID)VOS_FreeMsg(ACPU_PID_NFEXT, pMsg);
        }
    }
}


VOS_UINT32 NFExt_PidInit( enum VOS_INIT_PHASE_DEFINE ip )
{
    switch ( ip )
    {
        case VOS_IP_LOAD_CONFIG:
            /* 申请RingBuffer */
            g_stExEntity.pRingBufferId = OM_RingBufferCreate(NF_EXT_RING_BUF_SIZE);
            if ( VOS_NULL_PTR == g_stExEntity.pRingBufferId )
            {
                PS_PRINTF("NFExt_PidInit : ERROR : Create ring buffer Failed!" );
                return VOS_ERR;
            }

            /* 初始化锁 */
            VOS_SpinLockInit(&(g_stExEntity.stLockTxTask));
            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
            break;
        default:
            break;
    }

    return VOS_OK;
}



VOS_UINT32 NFExt_FidInit ( enum VOS_INIT_PHASE_DEFINE ip )
{
    VOS_UINT32                          ulRslt;

    switch ( ip )
    {
        case   VOS_IP_LOAD_CONFIG:

            /* 可维可测模块注册PID */
            ulRslt = VOS_RegisterPIDInfo(ACPU_PID_NFEXT,
                                (Init_Fun_Type)NFExt_PidInit,
                                (Msg_Fun_Type)NFExt_MsgProc);
            if( VOS_OK != ulRslt )
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterPIDInfo FAIL!\n");
                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskEntry(ACPU_FID_NFEXT, (VOS_VOIDFUNCPTR)NFExt_FidTask);
            if (VOS_OK != ulRslt)
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterMsgTaskEntry fail!\n");
                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskPrio(ACPU_FID_NFEXT, VOS_PRIORITY_M4);
            if( VOS_OK != ulRslt )
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterTaskPrio Failed!\n");
                return VOS_ERR;
            }

            break;

        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;

        default:
            break;
    }

    return PS_SUCC;
}


