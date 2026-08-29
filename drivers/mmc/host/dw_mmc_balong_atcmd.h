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
 
#ifndef _DW_MMC_BALONG_ATCMD_H_
#define _DW_MMC_BALONG_ATCMD_H_

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if (FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)
extern int SdioATTestEnable(unsigned int *size);
extern void AT_RegisterSdioATTestEnable(void *Handle);
#endif
#endif

#endif 