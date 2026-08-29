/*
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
 *
 */

#ifndef __MBB_PROCESS_START_H__
#define __MBB_PROCESS_START_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define SEM_UP  (1)

enum{
    NO_NEED_RESULT = 0,   /*不需要返回值*/
    NEED_RESULT,          /*需要返回值*/
    NEED_MAX
};


enum{
    WAIT_TIME_1S = 1000,   /*等待时间1s*/
    WAIT_TIME_2S = 2000,
    WAIT_TIME_3S = 3000,
    WAIT_TIME_4S = 4000,
    WAIT_TIME_5S = 5000,
    
    WAIT_TIME_1M = 60000,         /*等待时间1分钟*/
    WAIT_TIME_MAX = 300*1000,     /*最高等待时间300s，5分钟*/
};

/*****************************************************************************
 函 数 名  : drv_start_user_process
 功能描述  : 底层调用应用态进程通用接口
 输入参数  : name: 进程名；
             para: 进程的参数，没有的话为null
             need_result:  是否需要返回值
             wait_time:超时等待时间，单位ms
 输出参数  : 无
 返 回 值  : 0--进程启动成功或者执行结果为ok；
             other--失败；
 说    明  :如果执行的进程不会退出，一直执行，need_result参数请传入0，wait_time为0；
            执行的进程会退出，need_result参数请传入1，wait_time最大为5分钟，单位为ms；
            此时该接口会阻塞等待执行结果，直至超时时间到。
*****************************************************************************/
int  drv_start_user_process(char* name, char* para, unsigned int need_result, unsigned int wait_time);


#ifdef __cplusplus
}
#endif

#endif

 
