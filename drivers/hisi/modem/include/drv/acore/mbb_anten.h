/******************************************************************************
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
******************************************************************************/
#ifndef __MBB_ANTEN_H__
#define __MBB_ANTEN_H__

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************
 函 数 名  : anten_switch_get
 功能描述  : 提供给AT模块获取天线切换状态的接口
 输入参数  : ant_type : 天线类型，0--主天线，1--辅天线
 输出参数  : 无
 返 回 值  : 当前天线的状态
 说    明  : 此接口只给at使用，应用获取状态通过节点操作
*****************************************************************************/
int anten_switch_get(unsigned int ant_type);


/*****************************************************************************
 函 数 名  : anten_switch_set
 功能描述  : 提供给AT模块操作天线切换的接口
 输入参数  : ant_type : 天线类型，0--主天线，1--辅天线，2--所有天线
             in_or_out: 使用内置还是外部天线
 输出参数  : 无
 返 回 值  : 0--成功，其他--失败
 说    明  : 此接口只给at使用，应用切换天线通过节点操作
*****************************************************************************/
int anten_switch_set(unsigned int ant_type,unsigned int in_or_out);



#ifdef __cplusplus
}
#endif

#endif /*__MBB_ANTEN_H__*/

