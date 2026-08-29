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



#include "carrier_adapter.h"
#include "drv_comm.h"
#include "bsp_version.h"
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <bsp_nvim.h>
#include <linux/mtd/flash_huawei_dload.h>
#include <bsp_icc.h>
#if ((FEATURE_ON == MBB_DLOAD) || (FEATURE_ON == MBB_IPL))
#include <bsp_sram.h>
#endif /* (FEATURE_ON == MBB_DLOAD) || (FEATURE_ON == MBB_IPL) */
#if (FEATURE_ON == MBB_DRV_SYSREBOOT_REPORT)
#include "mbb_drv_reboot_report.h"
#endif

#define UNINIT_INDEX    (VOS_UINT32)0xFFFFFFFF
/*****************************************************************************
  *
  * index和与之对应的定制NV 路径， 添加新运营商后必须
  * 同步修改头文件中的枚举，和本数组成员中的信息
  *
  ****************************************************************************/
const CA_MATCH_XML_STRU g_ca_match_xml_list[CARRIER_INDEX_MAX] =
{
    /* 美国AT&T 定制NV 信息 */
    {USA_ATT,                USA_ATT_CUST_NV_PATH},
    /* 墨西哥AT&T 定制NV 信息 */
    {MEXICO_ATT,             MEX_ATT_CUST_NV_PATH},
    /* 加拿大AT&T 定制NV 信息 */
    {CANADA_ATT,             CA_ATT_CUST_NV_PATH},
    /* 北美PTCRB 认证定制NV 信息 */
    {NORTH_AMERICA_PTCRB,    NA_PTRB_CUST_NV_PATH},
    /* 韩国AT&T 定制NV 信息 */
    {KOREA_ATT,              KR_ATT_CUST_NV_PATH},
    /* 日本AT&T 定制NV 信息 */
    {JAPAN_ATT,              JP_ATT_CUST_NV_PATH},
    /* 中国联通定制NV 信息 */
    {CHINA_UNICOM,           CHN_UNICOM_CUST_NV_PATH},
    /* 全球沃达丰 定制NV 信息 */
    {GLOBAL_VODAFONE,        GBL_VODAFONE_CUST_NV_PATH},
    /* 俄罗斯AT&T 定制NV 信息 */
    {RUSSIA_ATT,             RUS_ATT_CUST_NV_PATH},
    /* 日本软银定制NV 信息 */
    {JAPAN_SOFTBANK,         JP_SOFTBANK_CUST_NV_PATH},
    /* 韩国KT 定制NV 信息 */
    {KOREA_KT,               KR_KT_CUST_NV_PATH},
};

/* PLMN和index对应关系表 */
const CA_PLMN_INFO_STRU g_ca_support_plmn_list[CARRIER_INDEX_MAX] =
{
    {
        USA_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        MEXICO_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        CANADA_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        NORTH_AMERICA_PTCRB,
        1,
        {"00101", }
    },
    {
        KOREA_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        JAPAN_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        CHINA_UNICOM,
        2,
        {"46001", "46009", }
    },
    {
        GLOBAL_VODAFONE,
        1,
        {"20404", }
    },
    {
        RUSSIA_ATT,
        9,
        {"310170", "310150", "310380", "310410", "310280", "310030", "310560", "310950", "311180", }
    },
    {
        KOREA_KT,
        6,
        {"45008", }
    },
    {
        JAPAN_SOFTBANK,
        6,
        {"44020", }
    },
};

/* 新增国家或地区，在此添加MCC 对应的默认运营商名称 */
const MCC_MATCH_CARRIER_INFO_STRU g_mcc_default_carrier_name[MCC_SUPPORT_COUNT_MAX] =
{
    /* MCC 为460 ，默认为联通卡 */
    {"460", CHINA_UNICOM},
    /* MCC 为450，默认为KT卡 */
    {"450", KOREA_KT},
    /* MCC 为420，默认为softbank卡 */
    {"440", JAPAN_SOFTBANK},
    /* MCC 为310，默认为AT&T卡 (北美子产品)*/
    {"310", USA_ATT},
    /* MCC 为310，默认为AT&T卡 (亚太子产品)*/
    {"310", KOREA_ATT},
    /* MCC 为310，默认为AT&T卡 (欧洲子产品)*/
    {"310", RUSSIA_ATT},
    /* MCC 为311，默认为AT&T卡 */
    {"311", USA_ATT},
    /* MCC为204，默认为沃达丰卡 */
    {"204", GLOBAL_VODAFONE},
};

/* 用于存储C核发送过来的PLMN信息 */
static CA_HPLMN_WITH_MNC_LEN_STRU g_plmn_info_stru;
static struct semaphore carrier_adapter_sem;
static struct task_struct *carrier_adapter_tsk = NULL;
static struct wake_lock carrier_adapter_lock;
/* 避免出现while(1)，导致在CUnit中的测试用例无法结束 */
static int TASK_KEEP_RUNNING = TRUE;
/* NV切换信号量 */
static struct semaphore nv_switch_task_sem;

/*****************************************************************************
函 数 名  : set_default_data
功能描述  : 查询随卡匹配运营商ID时，当ID为-1即不插卡或者插入不支持的SIM卡，将返回值设置为默认运营商ID
输入参数  : index:随卡匹配查询的运营商ID，仅当ID为-1时调用该函数
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void set_default_data(VOS_UINT32* index)
{
    u32 board_id = 0;
    const BSP_VERSION_INFO_S *board_info = NULL;
    if (NULL == index)
    {
        carrier_adapter_log(LOG_ERR, "index is NULL!!\r\n");
        return;
    }
    /* 从共享内存中获取单板boardid */
    board_info = bsp_get_version_info();
    board_id = board_info->board_id;
    switch (board_id)
    {
        case HW_VER_PRODUCT_ME919Bs_127bN_A:
        case HW_VER_PRODUCT_ME919Bs_127bN_B:
        case HW_VER_PRODUCT_ME919Bs_127bN_C:
        case HW_VER_PRODUCT_ME919Bs_127bN_D:
            *index = GLOBAL_VODAFONE;
            break;
        case HW_VER_PRODUCT_ME919Bs_567bN_A:
        case HW_VER_PRODUCT_ME919Bs_567bN_B:
        case HW_VER_PRODUCT_ME919Bs_567bN_C:
        case HW_VER_PRODUCT_ME919Bs_567bN_D:
            *index = USA_ATT;
            break;
        case HW_VER_PRODUCT_ME919Bs_821bN_A:
        case HW_VER_PRODUCT_ME919Bs_821bN_B:
        case HW_VER_PRODUCT_ME919Bs_821bN_C:
        case HW_VER_PRODUCT_ME919Bs_821bN_D:
            *index = KOREA_ATT;
            break;
        case HW_VER_PRODUCT_ME919Bs_823a_A:
        case HW_VER_PRODUCT_ME919Bs_823a_B:
        case HW_VER_PRODUCT_ME919Bs_823a_C:
        case HW_VER_PRODUCT_ME919Bs_823a_D:
            *index = CHINA_UNICOM;
            break;
        default:
            carrier_adapter_log(LOG_ERR, "Get default index fail!!");
    }
}

/*****************************************************************************
 函 数 名  : get_oem_carrier_name
 功能描述  : 获取oeminfo分区存储的index信息
 输入参数  : 无
 输出参数  : index: 获取到的oeminfo中存储的index
 返 回 值  : BSP_OK: 处理成功
                       BSP_ERROR: 处理失败
*****************************************************************************/
int get_oem_carrier_name(unsigned int *index)
{
    CA_OEM_INFO_STRU oeminfo;
    bool bret = FALSE;
#if (FEATURE_ON == MBB_IPL)
    smem_huawei_ipl_fota_type *smem_data = NULL;
#endif

    /* 入参判断 */
    if (NULL == index)
    {
        carrier_adapter_log(LOG_ERR, "NULL pointer!");
        return BSP_ERROR;
    }

    /* 初始化 */
    (void)memset(&oeminfo, 0, sizeof(oeminfo));
    /* 读取oeminfo 分区存储的当前nv 的index信息 */

    /* 从oeminfo分区读取数据 */
    bret = flash_get_share_region_info(RGN_CARRIER_ADAPTER_NV_FLAG, &oeminfo, \
                                       sizeof(CA_OEM_INFO_STRU));

    if (!bret)
    {
        carrier_adapter_log(LOG_ERR, "failed to get oeminfo region");
        return BSP_ERROR;
    }

#if (FEATURE_ON == MBB_IPL)
    /* 获取smem 共享内存 */
    smem_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;

    if (NULL == smem_data)
    {
        carrier_adapter_log(LOG_ERR, "get share memory failed");
        return BSP_ERROR;
    }

    /* 获取当前启动分区对应的oeminfo 中的PLMN 信息 */
    if (HUAWEI_IPL_IMAGE_FIRST == smem_data->smem_huawei_ipl_magic_run_parition)
    {
        *index = oeminfo.index;
    }
    else
    {
        *index = oeminfo.index_bk;
    }

#else
    *index = oeminfo.index;
#endif

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : set_oem_carrier_name
 功能描述  : 设置oeminfo分区存储的index信息
 输入参数  : index: 要设置的index值
 输出参数  : 无
 返 回 值  : BSP_OK: 处理成功
                       BSP_ERROR: 处理失败
*****************************************************************************/
int set_oem_carrier_name(unsigned int index)
{
    CA_OEM_INFO_STRU oeminfo;
    bool bret = FALSE;
#if (FEATURE_ON == MBB_IPL)
    smem_huawei_ipl_fota_type *smem_data = NULL;
#endif

    /* 入参判断 */
    if (CARRIER_INDEX_MAX <= index)
    {
        carrier_adapter_log(LOG_ERR, "invalid index to set!");
        return BSP_ERROR;
    }

    /* 初始化 */
    (void)memset(&oeminfo, 0, sizeof(oeminfo));
    /* 读取oeminfo 分区存储的当前nv 的index信息 */

    carrier_adapter_log(LOG_INFO, "start to set index %d to oeminfo", index);

    /* 从oeminfo分区读取当前数据 */
    bret = flash_get_share_region_info(RGN_CARRIER_ADAPTER_NV_FLAG, &oeminfo, \
                                       sizeof(CA_OEM_INFO_STRU));

    if (!bret)
    {
        carrier_adapter_log(LOG_ERR, "failed to get oeminfo region");
        return BSP_ERROR;
    }

#if (FEATURE_ON == MBB_IPL)
    /* 获取smem 共享内存 */
    smem_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;

    if (NULL == smem_data)
    {
        carrier_adapter_log(LOG_ERR, "get share memory failed");
        return BSP_ERROR;
    }

    /* 刷新当前启动分区对应的oeminfo 中的index信息 */
    if (HUAWEI_IPL_IMAGE_FIRST == smem_data->smem_huawei_ipl_magic_run_parition)
    {
        oeminfo.index = index;
    }
    else
    {
        oeminfo.index_bk = index;
    }

#else
    oeminfo.index = index;
#endif

    /* 更新oeminfo 分区数据 */
    bret = flash_update_share_region_info(RGN_CARRIER_ADAPTER_NV_FLAG, &oeminfo, sizeof(CA_OEM_INFO_STRU));

    if (!bret)
    {
        carrier_adapter_log(LOG_ERR, "failed to set oeminfo region");
        return BSP_ERROR;
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : ca_decode_plmn_info
 功能描述  : 根据plmn 信息结构体，解析出plmn (字符串形式)
 输入参数  : stHplmn: 原始PLMN 信息结构体
 输出参数  : sim_plmn: plmn 字符串
 返 回 值  :  BSP_OK: 解析成功
                        BSP_ERROR: 解析失败
*****************************************************************************/
static int ca_decode_plmn_info(CA_HPLMN_WITH_MNC_LEN_STRU *stHplmn, char *sim_plmn)
{
    char plmn_temp[PLMN_SIZE_MAX] = {0};
    char basic_zero = '0';

    if ((NULL == stHplmn) || (NULL == sim_plmn))
    {
        carrier_adapter_log(LOG_ERR, "NULL pointer!");
        return BSP_ERROR;
    }

    /* 获取MCC的第1个数值 */
    plmn_temp[0] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mcc) >> FIRST_BIT_SHIFT) \
                   + basic_zero;
    /* 获取MCC的第2个数值 */
    plmn_temp[1] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mcc) >> SECOND_BIT_SHIFT) \
                   + basic_zero;
    /* 获取MCC的第3个数值 */
    plmn_temp[2] = ((THIRD_BIT_MASK & stHplmn->stHplmn.Mcc) >> THIRD_BIT_SHIFT) \
                   + basic_zero;

    /*2位mnc*/
    if (2 == stHplmn->ucHplmnMncLen)
    {
        /* 获取MNC的第1个数值 */
        plmn_temp[3] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mnc) >> FIRST_BIT_SHIFT) \
                       + basic_zero;
        /* 获取MNC的第2个数值 */
        plmn_temp[4] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mnc) >> SECOND_BIT_SHIFT) \
                       + basic_zero;
    }
    /*3位mnc*/
    else if (3 == stHplmn->ucHplmnMncLen)
    {
        /* 获取MNC的第1个数值 */
        plmn_temp[3] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mnc) >> FIRST_BIT_SHIFT) \
                       + basic_zero;
        /* 获取MNC的第2个数值 */
        plmn_temp[4] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mnc) >> SECOND_BIT_SHIFT) \
                       + basic_zero;
        /* 获取MNC的第3个数值 */
        plmn_temp[5] = ((THIRD_BIT_MASK & stHplmn->stHplmn.Mnc) >> THIRD_BIT_SHIFT) \
                       + basic_zero;
    }
    else
    {
        carrier_adapter_log(LOG_ERR, "HPLMN MNC LEN INVAILID");
        return BSP_ERROR;
    }

    (void)memset(sim_plmn, 0, PLMN_SIZE_MAX);

    (void)strncpy(sim_plmn, plmn_temp, strlen(plmn_temp));
    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_icc_cb
 功能描述  : SIM卡初始化后向A核发icc消息的回调函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : BSP_OK: 处理成功
                       BSP_ERROR: 处理失败
*****************************************************************************/
static signed int carrier_adapter_icc_cb(void)
{
    int read_len = 0;
    int ret = BSP_ERROR;
    NV_CARRIER_ADAPTER_CONFIG_STRU ca_mode;

    /* 随卡匹配的消息ID */
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_SIM_CA;

    /* 初始化 */
    (void)memset(&ca_mode, 0, sizeof(ca_mode));

    /* 读取C 核发送的PLMN 信息 */
    read_len = bsp_icc_read(channel_id, (unsigned char *)&g_plmn_info_stru, \
                            sizeof(g_plmn_info_stru));

    if (sizeof(g_plmn_info_stru) != read_len)
    {
        carrier_adapter_log(LOG_ERR, "bsp_icc_read len is %d.", read_len);
        return BSP_ERROR;
    }

    /* 读随卡匹配模式NV 50635 */
    ret = bsp_nvm_read(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "nv read err");
        return BSP_ERROR;
    }

    /* 当前为自动模式，唤醒NV切换进程 */
    if (CA_NV_SWITCH_MODE_AUTO == ca_mode.switch_mode)
    {
        up(&carrier_adapter_sem);
    }
    else
    {
        carrier_adapter_log(LOG_INFO, "switch mode is manual, still sleeping");
    }

    carrier_adapter_log(LOG_INFO, "carrier_adapter_icc_cb proccess finish");

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : flush_nv_data_to_partition
 功能描述  : 刷新内存中的NV数据到分区中
 输入参数  : 无
 输出参数  : 无
 返 回 值  : BSP_OK: 成功
             BSP_ERROR: 失败
*****************************************************************************/
static int flush_nv_data_to_partition(void)
{
    u32 ret = NV_ERROR;

    /* 刷新img分区数据 */
    ret = nv_img_flush_all();

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "write back to img partition failed");
        return BSP_ERROR;
    }

    /* 刷新backlte分区数据 */
    ret = bsp_nvm_backup(NV_FLAG_NO_CRC);

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "write back to backlte partition failed");
        return BSP_ERROR;
    }

    /* 刷新sys分区数据 */
    ret = bsp_nvm_flushSys();

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "write back to sys partition failed");
        return BSP_ERROR;
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_name_match_cust_nv_path
 功能描述  : 根据运营商名称匹配运营商定制nv 的路径
 输入参数  : carrier_name: 运营商名称
 输出参数  : cust_nv_path: 运营商定制NV 路径
 返 回 值  : NV_OK:  执行成功
             NV_ERROR: 执行失败
*****************************************************************************/
unsigned int carrier_name_match_cust_nv_path(unsigned int carrier_name, char *cust_nv_path)
{
    u32 ret = NV_ERROR;
    u32 len = 0;
    u32 i = 0;

    if (NULL == cust_nv_path)
    {
        return ret;
    }

    /* 在列表中匹配运营商定制NV */
    for (i = 0; i < CARRIER_INDEX_MAX; i++)
    {
        if (carrier_name == g_ca_match_xml_list[i].carrier_name)
        {
            len = strlen(g_ca_match_xml_list[i].cust_nv_path);

            if ((0 < len) && (CA_CUST_PATH_MAX_LENGTH > len))
            {
                (void)snprintf(cust_nv_path, (CA_CUST_PATH_MAX_LENGTH - 1), "%s", \
                               g_ca_match_xml_list[i].cust_nv_path);
                ret = NV_OK;
            }
            else
            {
                carrier_adapter_log(LOG_ERR, "cann't find cust_nv_path for this plmn\n");
                ret = NV_ERROR;
            }

            break;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_update_nv
 功能描述  : 切换nv 主流程
 输入参数  : carrier_name: 运营商名称
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_update_nv(unsigned int carrier_name)
{
    int ret = BSP_ERROR;
    char cust_nv_path[CA_CUST_PATH_MAX_LENGTH] = {0};

    /* 判断当前函数是否被两个进程重入，如果获取信号量失败则立即返回ERROR */
    if (0 != down_trylock(&nv_switch_task_sem))
    {
        carrier_adapter_log(LOG_ERR, "carrier_adapter NV Switch process is busying");
        return BSP_ERROR;
    }

    do
    {
        /* 匹配plmn 对应的准入NV 文件路径 */
        ret = carrier_name_match_cust_nv_path(carrier_name, cust_nv_path);

        if (NV_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "can not find cust nv path");
            ret = BSP_ERROR;
            break;
        }

        /* 解析xml 文件 */
        ret = bsp_nvm_ca_xml_decode(cust_nv_path);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_WARN, "decode xml error ret = %d", ret);
            ret = BSP_ERROR;
            break;
        }

        /* ddr 中的NV 数据做CRC 校验*/
        ret = nv_crc_make_ddr();

        if (NV_OK != ret)
        {
            carrier_adapter_log(LOG_WARN, "make nv crc error");
            ret = BSP_ERROR;
            break;
        }

        /*将最新数据写入各个分区*/
        ret = flush_nv_data_to_partition();

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "flush nv data to flash failed");
            ret = BSP_ERROR;
            break;
        }

        /* 所有流程执行成功，NV切换完成，返回BSP_OK */
        ret = BSP_OK;
    } while (0);

    /* NV切换流程结束，释放信号量 */
    up(&nv_switch_task_sem);

    carrier_adapter_log(LOG_INFO, "nv switch action finished, ret = %d", ret);

    return ret;
}

/*****************************************************************************
 函 数 名  : is_carrier_name_valid
 功能描述  : 检查是否是有效的运营商
 输入参数  : carrier_name: 运营商名称
 输出参数  : 无
 返 回 值  : TRUE:有效  FALSE:无效
*****************************************************************************/
static bool is_carrier_name_valid(unsigned int carrier_name)
{
    bool ret = FALSE;
    u32 board_id = 0;
    const BSP_VERSION_INFO_S *board_info = NULL;

    board_info = bsp_get_version_info();

    if (NULL == board_info)
    {
        carrier_adapter_log(LOG_ERR, "get board info failed");
        return FALSE;
    }

    board_id = board_info->board_id;

    switch (board_id)
    {
        /* ME909Bs-566N和ME919Bs-567bN支持北美AT&T和北美PTCRB */
        case HW_VER_PRODUCT_ME909Bs_566N_A:
        case HW_VER_PRODUCT_ME909Bs_566N_B:
        case HW_VER_PRODUCT_ME909Bs_566N_C:
        case HW_VER_PRODUCT_ME909Bs_566N_D:
        case HW_VER_PRODUCT_ME919Bs_567bN_A:
        case HW_VER_PRODUCT_ME919Bs_567bN_B:
        case HW_VER_PRODUCT_ME919Bs_567bN_C:
        case HW_VER_PRODUCT_ME919Bs_567bN_D:
        {
            if ((USA_ATT == carrier_name)
                || (CANADA_ATT == carrier_name)
                || (MEXICO_ATT == carrier_name)
                || (NORTH_AMERICA_PTCRB == carrier_name))
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }

            break;
        }

        /* ME919Bs-127bN支持全球沃达丰和俄罗斯AT&T */
        case HW_VER_PRODUCT_ME919Bs_127bN_A:
        case HW_VER_PRODUCT_ME919Bs_127bN_B:
        case HW_VER_PRODUCT_ME919Bs_127bN_C:
        case HW_VER_PRODUCT_ME919Bs_127bN_D:
        {
            if ((GLOBAL_VODAFONE == carrier_name)
                || (RUSSIA_ATT == carrier_name))
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }

            break;
        }

        /* ME919Bs-821bN支持韩国AT&T、日本AT&T和中国联通 */
        case HW_VER_PRODUCT_ME919Bs_821bN_A:
        case HW_VER_PRODUCT_ME919Bs_821bN_B:
        case HW_VER_PRODUCT_ME919Bs_821bN_C:
        case HW_VER_PRODUCT_ME919Bs_821bN_D:
        {
            if ((KOREA_ATT == carrier_name)
                || (JAPAN_ATT == carrier_name)
                || (CHINA_UNICOM == carrier_name))
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }

            break;
        }

        /* ME919Bs-823a支持，KT、SOFTBANK、UNICOM */
        case HW_VER_PRODUCT_ME919Bs_823a_A:
        case HW_VER_PRODUCT_ME919Bs_823a_B:
        case HW_VER_PRODUCT_ME919Bs_823a_C:
        case HW_VER_PRODUCT_ME919Bs_823a_D:
        {
            if ((KOREA_KT == carrier_name)
                || (JAPAN_SOFTBANK == carrier_name)
                || (CHINA_UNICOM == carrier_name))
            {
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : plmn_get_carrier_name
 功能描述  : 获取plmn 所属的运营商
 输入参数  : plmn: plmn字符串信息
 输出参数  : out: 运营商名称
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int plmn_get_carrier_name(unsigned int *out, const char *plmn)
{
    int ret = BSP_ERROR;
    bool bret = FALSE;
    int i = 0;
    int j = 0;
    char sim_mcc_str[MCC_STR_SIZE] = {0};

    if (NULL == out || NULL == plmn)
    {
        return ret;
    }

    /* 查表匹配PLMN 对应的运营商名称 */
    for (i = 0; i < CARRIER_INDEX_MAX; i++)
    {
        for (j = 0; j < g_ca_support_plmn_list[i].count; j++)
        {
            ret = strcmp(g_ca_support_plmn_list[i].plmn[j], plmn);

            if (0 == ret)
            {
                /* 匹配成功且支持，返回结果，匹配成功且不支持继续查找 */
                bret = is_carrier_name_valid(g_ca_support_plmn_list[i].carrier_name);

                if (bret)
                {
                    /* PLMN匹配运营商成功，且允许切换到该运营商 */
                    *out = g_ca_support_plmn_list[i].carrier_name;
                    return BSP_OK;
                }
            }
        }
    }

    /* 如果用 plmn 匹配失败了才会走到下面, 使用MCC匹配默认运营商 */
    carrier_adapter_log(LOG_WARN, "SIM Hplmn doesn't match, try to match MCC");
    /* 从plmn 中提取mcc */
    (void)strncpy(sim_mcc_str, plmn, (MCC_STR_SIZE - 1));

    /* 查表匹配MCC 对应的运营商名称 */
    for (i = 0; i < MCC_SUPPORT_COUNT_MAX; i++)
    {
        ret = strcmp(g_mcc_default_carrier_name[i].mcc_str, sim_mcc_str);

        if (0 == ret)
        {
            /* 匹配成功且支持，返回结果，匹配成功且不支持继续查找 */
            bret = is_carrier_name_valid(g_mcc_default_carrier_name[i].def_carrier_name);

            if (bret)
            {
                /* MCC匹配默认运营商成功，且允许切换到该运营商 */
                *out = g_mcc_default_carrier_name[i].def_carrier_name;
                return BSP_OK;
            }
        }
    }

    return BSP_ERROR;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_task
 功能描述  : 随卡匹配处理流程入口函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_task(void *args)
{
    int ret = BSP_ERROR;
    char sim_plmn[PLMN_SIZE_MAX] = {0};
    unsigned int oem_carrier_name = CARRIER_INDEX_MAX;
    unsigned int sim_carrier_name = CARRIER_INDEX_MAX;

    /* 要求此任务的优先级比较高,唤醒了就执行*/
    set_user_nice(current, 10);

    do
    {
        /* 一直等待直到获得信号量*/
        if (0 != down_interruptible(&carrier_adapter_sem))
        {
            carrier_adapter_log(LOG_INFO, "waiting for sema timeout");
            continue;
        }

        /* 切换前延迟系统 休眠5s ，防止模块休眠切换失效 */
        wake_lock_timeout(&carrier_adapter_lock, NV_SWITCH_WAIT_TIME);

        /* 解析PLMN 信息为字符串 */
        ret = ca_decode_plmn_info(&g_plmn_info_stru, sim_plmn);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "decode plmn info failed!");
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "SIM PLMN is %s", sim_plmn);
        }

        /* 根据SIM 卡的PLMN 获取运营商名称 */
        ret = plmn_get_carrier_name(&sim_carrier_name, sim_plmn);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_WARN, "SIM carrier name is unknown");
            continue;
        }

        /* 读取oeminfo 分区存储的当前nv 的index信息 */
        ret = get_oem_carrier_name(&oem_carrier_name);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "failed to get oeminfo index");
            continue;
        }

        carrier_adapter_log(LOG_INFO, "oeminfo carrier index is %d", oem_carrier_name);

        /* oeminfo 的运营商 和sim 卡的运营商不同, 或者获取
               * oeminfo 分区运营商名称出错，都会启动nv切换流程 */
        if (sim_carrier_name == oem_carrier_name)
        {
            carrier_adapter_log(LOG_INFO, "don't need switch carrier NV");
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "SIM Carrier changed, start to switch carrier NV");
        }

        /* 切换NV */
        ret = carrier_adapter_update_nv(sim_carrier_name);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "switch carrier nv failed, ret= %d", ret);
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "NV switch finish");
        }

        /* 更新oeminfo分区信息 */
        ret = set_oem_carrier_name(sim_carrier_name);

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "update oeminfo failed");
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "update oeminfo successfully");

#if (FEATURE_ON == MBB_DRV_SYSREBOOT_REPORT)
            /* 主动上报 */
            request_sys_reboot_report(SYS_REBOOT_REASON_CRNVSWT);
            carrier_adapter_log(LOG_INFO, "kernel do report finish");
#endif
        }
    } while (TASK_KEEP_RUNNING);

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : manual_switch_ca_cust_nv
 功能描述  : 手动切换到index指定的运营商定制nv
 输入参数  : index: 要切换的目标运营商index
 输出参数  :  无
 返 回 值  :  BSP_OK:执行成功 BSP_ERROR:执行失败
*****************************************************************************/
int manual_switch_ca_cust_nv(unsigned int index)
{
    int ret = BSP_ERROR;
    unsigned int oem_carrier_name = CARRIER_INDEX_MAX;
    NV_CARRIER_ADAPTER_CONFIG_STRU ca_mode;
    MTCSWT_AUTO_MANUL_STATUS_STRU ca_switch;

    /* 初始化 */
    (void)memset(&ca_mode, 0, sizeof(ca_mode));
    (void)memset(&ca_switch, 0, sizeof(ca_switch));

    /* 参数有效性检查 */
    if (CARRIER_INDEX_MAX <= index)
    {
        carrier_adapter_log(LOG_WARN, "unknown carrier index: %d", index);
        return BSP_ERROR;
    }

    /* 读取随卡匹配开关NV */
    ret = bsp_nvm_read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, (u8 *)&ca_switch, sizeof(ca_switch));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "read ca_switch nv error");
        return BSP_ERROR;
    }

    /* 如果随卡匹配开关关闭，直接返回失败 */
    if (CA_SWITCH_ON != ca_switch.carrier_adapter_switch)
    {
        carrier_adapter_log(LOG_ERR, "this feature is ont support");
        return BSP_ERROR;
    }

    /* 读取oeminfo 分区存储的当前nv 的plmn 信息 */
    ret = get_oem_carrier_name(&oem_carrier_name);

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "failed to get oeminfo index");
        return BSP_ERROR;
    }
    else
    {
        carrier_adapter_log(LOG_INFO, "oeminfo carrier index is %d", oem_carrier_name);
    }

    /* oeminfo 的运营商 和index指定的运营商不同, 或者获取
         * oeminfo 分区运营商名称出错，都会启动nv切换流程 */
    if (index == oem_carrier_name)
    {
        carrier_adapter_log(LOG_WARN, "carrier NV already match index: %d", index);
        return BSP_OK;
    }
    else
    {
        carrier_adapter_log(LOG_INFO, "start to switch carrier NV");
    }

    /* 开始NV切换 */
    ret = carrier_adapter_update_nv(index);

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_WARN, "ca nv switch failed");
        return BSP_ERROR;
    }

    /* 读NV 50635，获取当前随卡匹配切换模式 */
    ret = bsp_nvm_read(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_INFO, "nv read error, ret = %d", ret);
        return BSP_ERROR;
    }

    /* 如果已经是手动模式，不处理 */
    if (CA_NV_SWITCH_MODE_MANUAL == ca_mode.switch_mode)
    {
        carrier_adapter_log(LOG_INFO, "switch mode is manual_mode");
    }
    /* 如果为自动模式，则设置为手动模式 */
    else
    {
        if (CA_NV_SWITCH_MODE_AUTO == ca_mode.switch_mode)
        {
            carrier_adapter_log(LOG_INFO, "switch mode is auto_mode, change to manual_mode");
        }
        else
        {
            carrier_adapter_log(LOG_WARN, "current NV config is unknown, change to manual_mode");
        }

        /* 写NV为手动模式 */
        ca_mode.switch_mode = CA_NV_SWITCH_MODE_MANUAL;
        ret = bsp_nvm_write(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

        if (NV_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "nv write error, ret = %d", ret);
            return BSP_ERROR;
        }
    }

    /* 新数据更新到oeminfo分区 */
    ret = set_oem_carrier_name(index);

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "update oeminfo failed");
        return BSP_ERROR;
    }
    else
    {
        carrier_adapter_log(LOG_INFO, "update oeminfo successfully");
    }

#if (FEATURE_ON == MBB_DRV_SYSREBOOT_REPORT)
    /* 主动上报需要重启 */
    request_sys_reboot_report(SYS_REBOOT_REASON_CRNVSWT);
    carrier_adapter_log(LOG_INFO, "system reboot report finish");
#endif

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : query_ca_status_info
 功能描述  : 查询当前随卡匹配切换模式及NV对应的index
 输入参数  : 无
 输出参数  :  mode: 随卡匹配切换模式
                           index: 当前运行NV对应的运营商index
 返 回 值  :  BSP_OK:执行成功 BSP_ERROR:执行失败
*****************************************************************************/
int query_ca_status_info(unsigned int *mode, unsigned int *index)
{
    int ret = BSP_ERROR;
    NV_CARRIER_ADAPTER_CONFIG_STRU ca_mode;
    MTCSWT_AUTO_MANUL_STATUS_STRU ca_switch;

    /* 初始化 */
    (void)memset(&ca_mode, 0, sizeof(ca_mode));
    (void)memset(&ca_switch, 0, sizeof(ca_switch));

    /* 入参判断 */
    if ((NULL == mode) || (NULL == index))
    {
        carrier_adapter_log(LOG_ERR, "NULL pointer");
        return BSP_ERROR;
    }

    /* 读取随卡匹配开关NV */
    ret = bsp_nvm_read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, (u8 *)&ca_switch, sizeof(ca_switch));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "read ca_switch nv error");
        return BSP_ERROR;
    }

    /* 如果随卡匹配开关关闭，直接返回失败 */
    if (CA_SWITCH_ON != ca_switch.carrier_adapter_switch)
    {
        carrier_adapter_log(LOG_ERR, "this feature is ont support");
        return BSP_ERROR;
    }

    /* 获取当前随卡匹配切换模式 */
    ret = bsp_nvm_read(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_INFO, "nv read error, ret = %d", ret);
        return BSP_ERROR;
    }

    *mode = ca_mode.switch_mode;

    /* 获取当前NV对应的运营商index */
    ret = get_oem_carrier_name(index);

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "read oeminfo err");
        return BSP_ERROR;
    }
    /* 读取运营商ID未初始化时，将运营商ID设置为默认运营商ID */
    if (UNINIT_INDEX == *index)
    {
        set_default_data(index);
    }
    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : set_ca_nv_auto_switch
 功能描述  : 设置随卡匹配切换模式为自动切换
 输入参数  : 无
 输出参数  :  无
 返 回 值  :  BSP_OK:执行成功 BSP_ERROR:执行失败
*****************************************************************************/
int set_ca_nv_auto_switch(void)
{
    unsigned int ret = NV_ERROR;
    NV_CARRIER_ADAPTER_CONFIG_STRU ca_mode;
    MTCSWT_AUTO_MANUL_STATUS_STRU ca_switch;

    /* 初始化 */
    (void)memset(&ca_mode, 0, sizeof(ca_mode));
    (void)memset(&ca_switch, 0, sizeof(ca_switch));

    /* 读取随卡匹配开关NV */
    ret = bsp_nvm_read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, (u8 *)&ca_switch, sizeof(ca_switch));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "read ca_switch nv error");
        return BSP_ERROR;
    }

    /* 如果随卡匹配开关关闭，直接返回失败 */
    if (CA_SWITCH_ON != ca_switch.carrier_adapter_switch)
    {
        carrier_adapter_log(LOG_ERR, "this feature is not support");
        return BSP_ERROR;
    }

    /* 读NV 50635，获取当前随卡匹配切换模式 */
    ret = bsp_nvm_read(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_INFO, "nv read error, ret = %d", ret);
        return BSP_ERROR;
    }

    /* 如果已经是自动模式，返回成功 */
    if (CA_NV_SWITCH_MODE_AUTO == ca_mode.switch_mode)
    {
        carrier_adapter_log(LOG_WARN, "switch mode is already auto_mode");
        return BSP_OK;
    }
    else
    {
        if (CA_NV_SWITCH_MODE_MANUAL == ca_mode.switch_mode)
        {
            carrier_adapter_log(LOG_INFO, "switch mode is manual_mode, change to auto_mode");
        }
        else
        {
            carrier_adapter_log(LOG_WARN, "current NV config is unknown, change to auto_mode");
        }

        /* 如果为手动模式或配置异常，则设置为自动模式 */
        ca_mode.switch_mode = CA_NV_SWITCH_MODE_AUTO;
        ret = bsp_nvm_write(NV_CARRIER_ADAPTER_CONFIG, (u8 *)&ca_mode, sizeof(ca_mode));

        if (NV_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "nv write error, ret = %d", ret);
            return BSP_ERROR;
        }

        /* 判断PLMN信息是否存在 */
        if (0 != g_plmn_info_stru.ucHplmnMncLen)
        {
            /* 唤醒随卡匹配自动切换进程 */
            up(&carrier_adapter_sem);
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "MNC length is 0, NO PLMN information recieved");
        }
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : get_crnv_test_string
 功能描述  : 测试AT命令取值范围
 输入参数  : out: 输出字符串
 输出参数  :  无
 返 回 值  :  执行成功：返回字符串实际长度len; 执行失败:0
*****************************************************************************/
unsigned int get_crnv_test_string(char *out)
{
    unsigned int len = 0;
    unsigned int ret = NV_ERROR;
    MTCSWT_AUTO_MANUL_STATUS_STRU ca_switch;

    /* 入参判断 */
    if (NULL == out)
    {
        carrier_adapter_log(LOG_ERR, "NULL pointer!");
        return 0;
    }

    /* 初始化 */
    (void)memset(&ca_switch, 0, sizeof(ca_switch));

    /* 读取随卡匹配开关NV */
    ret = bsp_nvm_read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, (u8 *)&ca_switch, sizeof(ca_switch));

    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "read ca_switch nv error");
        return 0;
    }

    /* 如果随卡匹配开关关闭，直接返回失败 */
    if (CA_SWITCH_ON != ca_switch.carrier_adapter_switch)
    {
        carrier_adapter_log(LOG_ERR, "this feature is not support");
        return 0;
    }

    (void)snprintf(out, (CRNV_TEST_STR_LEN - 1), "(0,1),(0,%d)", (CARRIER_INDEX_MAX - 1));

    /* 计算输出字符串长度 */
    len = strlen(out);
    carrier_adapter_log(LOG_INFO, "CRNV TEST string is '%s', len is %d", out, len);

    return len;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_task_init
 功能描述  : 创建随卡匹配任务
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_task_init(void)
{
    /* 创建切换任务 */
    if (NULL == carrier_adapter_tsk)
    {
        carrier_adapter_tsk = kthread_run(carrier_adapter_task, NULL, CA_TSK_NAME);

        if (IS_ERR(carrier_adapter_tsk))
        {
            carrier_adapter_log(LOG_ERR, "fork failed for carrier_adapter_task");
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_init
 功能描述  : 随卡匹配模块初始化函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  其他: 错误码
*****************************************************************************/
static int __init carrier_adapter_init(void)
{
    int ret = BSP_ERROR;
    /* 随卡匹配的消息ID */
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_SIM_CA;
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
#endif

    /* 初始化本模块全局变量 */
    (void)memset(&g_plmn_info_stru, 0, sizeof(g_plmn_info_stru));

#if (FEATURE_ON == MBB_DLOAD)
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        carrier_adapter_log(LOG_INFO, "carrier_adapter_init: smem_data is NULL");
        return ret;
    }

    if (SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽本模块的启动*/
        carrier_adapter_log(LOG_INFO, "update entry, not init carrier_adapter !");
        return ret;
    }

#endif /* MBB_DLOAD */

    /* 初始化系统锁 */
    wake_lock_init(&carrier_adapter_lock, WAKE_LOCK_SUSPEND, CA_MODULE_NAME);

    /* 信号量初始化函数 */
    sema_init(&carrier_adapter_sem, 0);
    sema_init(&nv_switch_task_sem, 1);

    /* 创建随卡匹配NV 切换任务 */
    ret = carrier_adapter_task_init();

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "running carrier adapter task failed");
        return ret;
    }

    /*注册icc回调函数，用来获取SIM 卡的PLMN 信息*/
    ret = bsp_icc_event_register(channel_id, \
                                 (read_cb_func)carrier_adapter_icc_cb, NULL, NULL, NULL);

    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "icc event register failed.");
        return BSP_ERROR;
    }

    carrier_adapter_log(LOG_INFO, "Init OK");
    return BSP_OK;
}

module_init(carrier_adapter_init);

