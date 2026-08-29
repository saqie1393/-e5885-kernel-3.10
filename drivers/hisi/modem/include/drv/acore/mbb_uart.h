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

#ifndef __MBB_UART_H__
#define __MBB_UART_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#if (FEATURE_ON == MBB_COMMON)
/**************************************************************************
  头文件包含                            
**************************************************************************/

/**************************************************************************
  宏定义 
**************************************************************************/

/**************************************************************************
  变量、结构定义 
**************************************************************************/

/**************************************************************************
  函数定义 
**************************************************************************/
#if  ((FEATURE_ON == MBB_PHY_UART_TEST) && (FEATURE_ON == MBB_FACTORY))

int mbb_phy_uart_test(void);
#endif
#endif
#ifdef __cplusplus
}
#endif

#endif

