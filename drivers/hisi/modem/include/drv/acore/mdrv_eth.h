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

#ifndef __MDRV_ACORE_ETH_H__
#define __MDRV_ACORE_ETH_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum tagETH_MIRROR_SWITCH_E
{
    ETH_MIRROR_DISENABLE = 0,
    ETH_MIRROR_ENABLE = 1,
}ETH_MIRROR_SWITCH_E;

typedef struct tagETH_MIRROR_STATUS_S
{
    int port;
    int status;
}ETH_MIRROR_STATUS_S;


#if (FEATURE_ON == MBB_FEATURE_ETH)
#if ((FEATURE_ON == MBB_BUILD_DEBUG) || (FEATURE_ON == MBB_FEATURE_ETH_WAN_MIRROR)) 

/*****************************************************************************
函数名：   DRV_ETH_MIRROR_SET_MODE
功能描述:  提供给打开网口驱动镜像抓包功能设置的接口
输入参数： enable   打开或者关闭镜像抓包功能。
          port     数据包镜像到哪个口上
输出参数:  无
返回值：   无
*****************************************************************************/
int RNIC_WANMirror(unsigned int enable, unsigned int port);
#define DRV_ETH_MIRROR_SET_MODE(enable,port) RNIC_WANMirror(enable,port)

/*****************************************************************************
函数名：   DRV_ETH_MIRROR_GET_MODE
功能描述:  提供给查询网口驱动镜像抓包开关的接口
输入参数： enable   打开或者关闭镜像抓包功能。
          port     数据包镜像到哪个口上
输出参数:  无
返回值：   无
*****************************************************************************/
int RNIC_GetWanMirrorStatus(ETH_MIRROR_STATUS_S *status);
#define DRV_ETH_MIRROR_GET_MODE(status) RNIC_GetWanMirrorStatus(status)

#else
static inline int DRV_ETH_MIRROR_SET_MODE(unsigned int enable, unsigned int port)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline int DRV_ETH_MIRROR_GET_MODE(ETH_MIRROR_STATUS_S *status)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif
