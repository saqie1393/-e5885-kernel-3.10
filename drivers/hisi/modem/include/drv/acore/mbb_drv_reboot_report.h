/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2016. All rights reserved.
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


#ifndef __MBB_DRV_REBOOT_REPORT_H__
#define __MBB_DRV_REBOOT_REPORT_H__

/* 请求重启的原因枚举 */
typedef enum
{
    SYS_REBOOT_REASON_INVALID = 0,  /* 无效的请求原因，不可用 */
    SYS_REBOOT_REASON_CRNVSWT = 1,  /* 请求重启原因为随卡匹配NV切换完成 */
    SYS_REBOOT_REASON_MAX,
} AT_SYSREBOOT_REASON;

/* 请求重启，主动上报接口 */
void request_sys_reboot_report(AT_SYSREBOOT_REASON reason);

#endif  /* __MBB_DRV_REBOOT_REPORT_H__ */