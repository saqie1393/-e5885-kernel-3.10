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



#ifndef __MBB_CARRIER_ADAPTER_H__
#define __MBB_CARRIER_ADAPTER_H__

#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PLMN_SIZE_MAX               (8)
#define CA_CUST_PATH_MAX_LENGTH     (64)
#define CRNV_TEST_STR_LEN           (16)

#define CARRIER_ADAPTER_COMMON_NV_PATH   "/modem_fw/ca_cust_nv/me919bs_common_nv.xml"

typedef enum
{
    CA_NV_SWITCH_MODE_AUTO = 0,     /* 随卡匹配自动切换模式 */
    CA_NV_SWITCH_MODE_MANUAL = 1,   /* 随卡匹配手动切换模式 */
} CA_NV_SWITCH_MODE;

/* OEMINFO分区的index信息，分别存表示正常分区和bk分区的index */
typedef struct
{
    unsigned int index;
    unsigned int index_bk;
} CA_OEM_INFO_STRU;

/* 在这里增加随卡匹配特性支持的新运营商的名字 */
typedef enum
{
    USA_ATT = 0,                 /* 美国AT&T定制 */
    MEXICO_ATT = 1,              /* 墨西哥AT&T定制 */
    CANADA_ATT = 2,              /* 加拿大AT&T定制 */
    NORTH_AMERICA_PTCRB = 3,     /* 北美PTCRB定制 */
    KOREA_ATT = 4,               /* 韩国AT&T定制 */
    JAPAN_ATT = 5,               /* 日本AT&T定制 */
    CHINA_UNICOM = 6,             /* 中国联通定制 */
    GLOBAL_VODAFONE = 7,         /* 全球沃达丰定制 */
    RUSSIA_ATT = 8,              /* 俄罗斯AT&T定制 */
    KOREA_KT = 9,                /* 韩国KT定制 */
    JAPAN_SOFTBANK = 10,          /* 日本软银定制 */
    CARRIER_INDEX_MAX,
} CARRIER_NAME_ENUM;

/* PLMN和运营商定制NV XML文件对应列表数据结构 */
typedef struct
{
    unsigned int carrier_name;
    char cust_nv_path[CA_CUST_PATH_MAX_LENGTH];
} CA_MATCH_XML_STRU;

/* 根据运营商名称匹配运营商定制nv 的路径 */
unsigned int carrier_name_match_cust_nv_path(unsigned int carrier_name, char *cust_nv_path);
/* 查询当前随卡匹配切换模式及NV对应的index */
int query_ca_status_info(unsigned int *mode, unsigned int *index);
/* 设置随卡匹配切换模式为自动切换 */
int set_ca_nv_auto_switch(void);
/* 运营商定制NV手动切换 */
int manual_switch_ca_cust_nv(unsigned int index);
/* 获取oeminfo分区的index */
int get_oem_carrier_name(unsigned int *index);
/* 设置oeminfo分区的index */
int set_oem_carrier_name(unsigned int index);
/* 测试AT命令取值范围 */
unsigned int get_crnv_test_string(char *out);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* __MBB_CARRIER_ADAPTER_H__ */

