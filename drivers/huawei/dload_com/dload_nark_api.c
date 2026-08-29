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


#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include "dload_product.h"
#include "dload_mtd_adapter.h"
#include "dload_nark_api.h"
#include "dload_sec_sign.h"
#include "bsp_sram.h"
#include <linux/mtd/flash_huawei_dload.h>
#if (FEATURE_ON == MBB_IPL)
#include "bsp_nvim.h"
#endif /* #if (FEATURE_ON == MBB_IPL) */

#include "dload_comm.h"

#if (FEATURE_ON == MBB_LED_LCD_UNIFORM)
#include <bsp_version.h>
#endif/*MBB_LED_LCD_UNIFORM*/
#if (FEATURE_ON == MBB_DLOAD_COPY_SHM)
/* C30 版本CPE的共享内存的物理起始地址 */
#define LEAGACY_SHM_PHY_ADDR  (0xa5180000)
#endif /* MBB_DLOAD_COPY_SHM */

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
  1 模块私有 (宏、枚举、结构体、自定义数据类型) 定义区:
******************************************************************************/
#define DLOAD_PRINT(level, fmt, ...)  (printk(level"[*DLOAD_NARK_API*] %s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__))

/* 消息映射结构定义 */
typedef struct _IOCRL_MSG_MAP
{
    uint32      map_index;  /* 映射索引ID */
    msg_func_t  msg_func;   /* 映射处理函数 */
} ioctl_map_t;

/******************************************************************************
  2 模块私有 (全局变量) 定义区:
******************************************************************************/
/* 平台信息 */
STATIC platform_info_t      platinfo_st;
/* 共享内存 */
STATIC dload_smem_info_t    smem_info_st;

STATIC const dload_part_name_mapping_t g_dload_part_name_mapping[] =
{
#if (FEATURE_ON == MBB_DLOAD_TELEMATIC_AUDI)
    /* 奥迪分区名映射表 */
    {"online", "cache", {0}},
    {"kernel", "boot", "bootbk"},
#elif (FEATURE_ON == MBB_DLOAD_TELEMATIC_HARMAN)
    /* 哈曼分区名映射表 */
    {"kernel", "boot", "bootbk"},
#else
    {{0}, {0}, {0}}
#endif
};

/*SD卡升级，充电提供的获取电池格数的设备节点*/
#define DLOAD_CHG_DEVNAME  "/proc/dload_chg"


/****************************************************************************
  Function:         dload_get_charge_voltage
  Description:      获取电池当前电量值
  Input:            ret_volt 获取的电池电量值
  Output:           void
  Return:           电池当前电量格数
  Others:           NA
****************************************************************************/
STATIC int32 dload_get_charge_voltage(uint32 *ret_volt)
{
    int32 fd = -1;
    int8 read_buf = '0';
    int32 len = -1;
    mm_segment_t fs_old = get_fs();

    if(NULL == ret_volt)
    {
        DLOAD_PRINT(KERN_ERR, "dload_get_charge_voltage para error.");
        return DLOAD_ERR;
    }

    /*改变内核访问权限 */
    set_fs(KERNEL_DS);

    fd = sys_open(DLOAD_CHG_DEVNAME, O_RDONLY, (S_IRUSR | S_IRGRP));
    if(0 > fd)
    {
        DLOAD_PRINT(KERN_ERR,"dload_get_charge_voltage open %s error.", DLOAD_CHG_DEVNAME);
        set_fs(fs_old);
        return DLOAD_ERR;
    }

    /*获取电池当前电量值 */
    len = sys_read(fd,&read_buf,sizeof(read_buf));
    if(sizeof(read_buf) != len)
    {
        DLOAD_PRINT(KERN_ERR, "read %s chg error.", DLOAD_CHG_DEVNAME);
        set_fs(fs_old);
        sys_close(fd);
        return DLOAD_ERR;
    }

    *ret_volt = (uint8)(read_buf - '0');
    DLOAD_PRINT(KERN_ERR, "get chg volt ret_volt = %lu !", *ret_volt);
    set_fs(fs_old);
    sys_close(fd);
    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : get_platform_information
  函数描述  : 获取平台信息
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
int32 get_platform_information(uint8 * data_buffer, uint32 data_len)
{
    memcpy((void *)data_buffer, (void *)&platinfo_st, sizeof(platform_info_t));

    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : get_battery_level
  函数描述  : 获取电池电量格数
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR : 失败
********************************************************************/
STATIC int32 get_battery_level(uint8 * data_buffer, uint32 data_len)
{
    uint32  bat_volt_level = -1;
    int32   ret = DLOAD_ERR;

    ret = dload_get_charge_voltage(&bat_volt_level);
    if(DLOAD_ERR == ret)
    {
        DLOAD_PRINT(KERN_ERR, "get_battery_level error.");
        return  DLOAD_ERR;
    }

    memcpy((void *)data_buffer, (void *)&bat_volt_level, data_len);

    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : get_mtd_fs_type
  函数描述  : 获取MTD设备文件系统类型
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 get_mtd_fs_type(uint8 * data_buffer, uint32 data_len)
{
    mtd_fs_type fs_type = MTD_UNKNOW_FS;

    DLOAD_PRINT(KERN_DEBUG, "mtd name = %s.", data_buffer);

    fs_type = check_mtdchar_fs_part(data_buffer);

    return fs_type;
}


/*******************************************************************
  函数名称  : get_show_path_info_size
  函数描述  :   获取path_info空间大小
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 get_show_path_info_size(uint8 * data_buffer, uint32 data_len)
{
    uint32 info_size = 0;

    DLOAD_PRINT(KERN_DEBUG, "fetch path list size.");

    if(sizeof(info_size) != data_len)
    {
        DLOAD_PRINT(KERN_ERR, "data_len = %lu not suitable.", data_len);
        return DLOAD_ERR;
    }

    get_show_path_info(NULL, &info_size);

    memcpy((void *)data_buffer, (void *)&info_size, sizeof(info_size));

    return DLOAD_OK;
}


/*******************************************************************
  函数名称  : get_show_information
  函数描述  : 获取path_info
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 get_show_path_information(uint8 * data_buffer, uint32 data_len)
{
    path_info_t *ppath_info = NULL;

    DLOAD_PRINT(KERN_DEBUG, "fetch path list.");

    get_show_path_info(&ppath_info, NULL);

    memcpy((void *)data_buffer, (void *)ppath_info, data_len);

    return DLOAD_OK;
}


/*******************************************************************
  函数名称  : get_show_info_size
  函数描述  :   获取LCD/LED配置占的空间大小
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 get_show_info_size(uint8 * data_buffer, uint32 data_len)
{
    uint32 info_size = 0;

    DLOAD_PRINT(KERN_DEBUG, "fetch run state list size.");

    if(sizeof(info_size) != data_len)
    {
        DLOAD_PRINT(KERN_ERR, "data_len = %lu not suitable.", data_len);
        return DLOAD_ERR;
    }

    get_show_info(NULL, &info_size);

    memcpy((void *)data_buffer, (void *)&info_size, sizeof(info_size));

    return DLOAD_OK;
}


/*******************************************************************
  函数名称  : get_show_information
  函数描述  : 获取升级时 LCD/LED 的配置
  函数入参  : data_buffer : 数据缓存指针
              data_len    : 数据长度
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 get_show_information(uint8 * data_buffer, uint32 data_len)
{
    upstate_info_t *pupstate_info = NULL;

    DLOAD_PRINT(KERN_DEBUG, "fetch run state list.");

    get_show_info(&pupstate_info, NULL);

    memcpy((void *)data_buffer, (void *)pupstate_info, data_len);

    return DLOAD_OK;
}


/*******************************************************************
  函数名称  : dload_mtd_flush
  函数描述  : 根据最新的分区表刷新MTD设备
  函数入参  :
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR : 失败
********************************************************************/
STATIC int32 dload_mtd_flush(uint8 * data_buffer, uint32 data_len)
{
    return dload_flush_partion();
}

#if (FEATURE_ON == MBB_DLOAD_COPY_SHM)
#ifdef CONFIG_DDR_SECURITY
#else
/*******************************************************************
  函数名称  : dload_copy_shared_memory
  函数描述  : 将C60共享内存中的内容拷贝到C30的共享内存地址，实现C60向C30升级的平滑过渡
  函数入参  : void
  函数输出  : C60共享内存中的内容拷贝到C30的共享内存
  函数返回值: void
********************************************************************/
static void dload_copy_shared_memory(void)
{
    void *phy_addr  = NULL;
    void *virt_addr = NULL;

    phy_addr = (void *)MDDR_FAMA(LEAGACY_SHM_PHY_ADDR);
    virt_addr = (void *)ioremap_wc((unsigned long)phy_addr, HI_SHARED_DDR_SIZE);
    if (NULL == virt_addr)
    {
        printk(KERN_ERR "dload_copy_shared_memory, ioremap_wc failed\n");
        return ;
    }

    memcpy((void *)(((SRAM_SMALL_SECTIONS*)(virt_addr + SHM_OFFSET_SRAM_TO_DDR
           + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DLOAD), SRAM_DLOAD_ADDR, SRAM_DLOAD_SIZE);

    iounmap(virt_addr);
}
#endif
#endif /* MBB_DLOAD_COPY_SHM */

/*******************************************************************
  函数名称  : visit_dload_smem
  函数描述  : 共享内存访问接口
  函数入参  :
  函数输出  : NA
  函数返回值: TRUE  : 成功
              FALSE : 失败
********************************************************************/
int32 visit_dload_smem(uint8 * data_buffer, uint32 data_len)
{
    uint32 visit_flag;
    uint32 visit_value;

    memcpy((void *)&visit_flag, (void *)data_buffer, data_len);
    DLOAD_PRINT(KERN_DEBUG, "dload visit smem  vist = %lu.", visit_flag);

    switch(visit_flag)
    {
        case SET_SMEM_DLOAD_MODE_MAGIC:
            {
                if(NULL == smem_info_st.dload_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.dload_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.dload_mode_addr)) = SMEM_DLOAD_FLAG_NUM;
            }
            break;
        case SET_SMEM_DLOAD_MODE_ZERO:
            {
                if(NULL == smem_info_st.dload_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.dload_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.dload_mode_addr)) = 0;
#if ( FEATURE_ON == MBB_MLOG)
                *((uint32 *)(smem_info_st.dload_times_addr)) = 1;
#endif
#if (FEATURE_ON == MBB_DLOAD_COPY_SHM)
#ifdef CONFIG_DDR_SECURITY
                /* operation always fail in this case */
#else
                dload_copy_shared_memory();
#endif
#endif /* MBB_DLOAD_COPY_SHM */
            }
            break;
        case GET_SMEM_IS_DLOAD_MODE:
            {
               if(NULL == smem_info_st.dload_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.dload_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.dload_mode_addr));
                if(SMEM_DLOAD_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_SCARD_DLOAD_MAGIC:
            {
                if(NULL == smem_info_st.sd_upgrade_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.sd_upgrade_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.sd_upgrade_addr)) = SMEM_SDUP_FLAG_NUM;
            }
            break;
        case SET_SMEM_SCARD_DLOAD_ZERO:
            {
                if(NULL == smem_info_st.sd_upgrade_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.sd_upgrade_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.sd_upgrade_addr)) = 0;
            }
            break;
        case GET_SMEM_IS_SCARD_DLOAD:
            {
                if(NULL == smem_info_st.sd_upgrade_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.sd_upgrade_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.sd_upgrade_addr));
                if(SMEM_SDUP_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_SWITCH_PORT_MAGIC:
            {
                if(NULL == smem_info_st.usbport_switch_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.usbport_switch_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.usbport_switch_addr)) = SMEM_SWITCH_PUCI_FLAG_NUM;
            }
            break;
        case SET_SMEM_SWITCH_PORT_ZERO:
            {
                if(NULL == smem_info_st.usbport_switch_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.usbport_switch_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.usbport_switch_addr)) = 0;
            }
            break;
        case GET_SMEM_IS_SWITCH_PORT:
            {
                if(NULL == smem_info_st.usbport_switch_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.usbport_switch_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.usbport_switch_addr));
                if(SMEM_SWITCH_PUCI_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_ONL_RESTORE_REBOOT_MAGIC:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = SMEM_ONNR_FLAG_NUM;
            }
            break;
        case SET_SMEM_ONL_RESTORE_REBOOT_ZERO:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = 0;
            }
            break;
        case GET_SMEM_ONL_IS_RESTORE_REBOOT:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.app_inactivate_addr));
                if(SMEM_ONNR_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_SD_RESTORE_REBOOT_MAGIC:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = SMEM_SDNR_FLAG_NUM;
            }
            break;
        case SET_SMEM_SD_RESTORE_REBOOT_ZERO:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = 0;
            }
            break;
        case GET_SMEM_SD_IS_RESTORE_REBOOT:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.app_inactivate_addr));
                if(SMEM_SDNR_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_EXCEP_REBOOT_INTO_ONL_MAGIC:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = SMEM_ONNR_FLAG_NUM;
            }
            break;
        case SET_SMEM_EXCEP_REBOOT_INTO_ONL_ZERO:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.app_inactivate_addr)) = 0;
            }
            break;
        case GET_SMEM_IS_EXCEP_REBOOT_INTO_ONL:
            {
                if(NULL == smem_info_st.app_inactivate_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.app_inactivate_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.app_inactivate_addr));
                if(SMEM_ONNR_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_FORCE_MODE_MAGIC:
            {
                if(NULL == smem_info_st.force_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.force_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.force_mode_addr)) = SMEM_FORCELOAD_FLAG_NUM;
            }
            break;
        case SET_SMEM_FORCE_MODE_ZERO:
            {
                if(NULL == smem_info_st.force_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.force_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.force_mode_addr)) = 0;
            }
            break;
        case SET_SMEM_FORCE_SUCCESS_MAGIC:
            {
                if(NULL == smem_info_st.force_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.force_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.force_mode_addr)) = SMEM_FORCELOAD_SUCCESS_NUM;
            }
            break;
        case GET_SMEM_IS_FORCE_MODE:
            {
                if(NULL == smem_info_st.force_mode_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.force_mode_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.force_mode_addr));
                if(SMEM_FORCELOAD_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_UT_DLOAD_MAGIC:
            {
                if(NULL == smem_info_st.ut_dload_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.ut_dload_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.ut_dload_addr)) = SMEM_BURN_UPDATE_FLAG_NUM;
            }
            break;
        case SET_SMEM_UT_DLOAD_ZERO:
            {
                if(NULL == smem_info_st.ut_dload_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.ut_dload_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.ut_dload_addr)) = 0;
            }
            break;
        case GET_SMEM_IS_UT_DLOAD:
            {
                if(NULL == smem_info_st.ut_dload_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.ut_dload_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.ut_dload_addr));
                if(SMEM_BURN_UPDATE_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_MULTIUPG_DLOAD_MAGIC:
            {
                if(NULL == smem_info_st.multiupg_dload_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.multiupg_dload_addr is NULL.");
                    return DLOAD_ERR;
                }

                *((uint32 *)(smem_info_st.multiupg_dload_addr)) = SMEM_MULTIUPG_FLAG_NUM;
            }
            break;
        case GET_SMEM_DATALOCK_STATUS_MAGIC:
            {
                if(NULL == smem_info_st.datalock_status_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.datalock_status_addr is NULL.");
                    return DLOAD_ERR;
                }

                visit_value = *((uint32 *)(smem_info_st.datalock_status_addr));
                DLOAD_PRINT(KERN_ERR, "smem_info_st.datalock_status is 0x%x.", visit_value);
                if(SMEM_DATALOCK_STATUS_FLAG_NUM == visit_value)
                {
                    return DLOAD_OK;
                }
                else
                {
                    return DLOAD_ERR;
                }
            }
            break;
        case SET_SMEM_DATALOCK_MODE_ZERO:
            {
                if(NULL == smem_info_st.datalock_status_addr)
                {
                    DLOAD_PRINT(KERN_ERR, "smem_info_st.datalock_status_addr is NULL.");
                    return DLOAD_ERR;
                }
                *((uint32 *)(smem_info_st.datalock_status_addr)) = 0;
            }
            break;
        default:
            {
                DLOAD_PRINT(KERN_ERR, "visit smem enum ERROR.");
                return DLOAD_ERR;
            }
            break;
    }

    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : dload_mtd_flush
  函数描述  : 根据最新的分区表刷新MTD设备
  函数入参  :
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
             DLOAD_ERR : 失败
********************************************************************/
STATIC int32 dload_get_dload_feature(uint8 * data_buffer, uint32 data_len)
{
    dload_feature_t*  local_dload_feature_st = NULL;

    local_dload_feature_st = get_dload_feature();

    if(NULL == local_dload_feature_st)
    {
        return DLOAD_ERR;
    }

    memcpy((void *)data_buffer, (void *)local_dload_feature_st, sizeof(dload_feature_t));

    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : get_smem_info
  函数描述  : 对外提供获取共享内存接口
  函数入参  : NA
  函数输出  : NA
  函数返回值: 共享内存指针
********************************************************************/
dload_smem_info_t* get_smem_info(void)
{
    return &smem_info_st;
}

/*******************************************************************
  函数名称  : nark_api_init
  函数描述  : API模块初始化
  函数入参  : NA
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR   : 失败
********************************************************************/
int32 nark_api_init(void)
{
    huawei_smem_info *smem_data = NULL;
    product_info_t* product_info_st = NULL;
#if (FEATURE_ON == MBB_LED_LCD_UNIFORM)
    unsigned int product_id = 0;
    int32 ret = DLOAD_ERR;
    bool dload_lcd_feature_support = FALSE;
    bool dload_led_feature_support = FALSE;
#endif/*MBB_LED_LCD_UNIFORM*/

    DLOAD_PRINT(KERN_DEBUG, "nark api module init.");
#if (FEATURE_ON == MBB_LED_LCD_UNIFORM)
    product_id = bsp_get_version_info()->board_id;/*获取硬件ID*/
#endif/*MBB_LED_LCD_UNIFORM*/
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "nark api smem error.");
        return DLOAD_ERR;
    }

    /* 共享内存指针初始化 */
    smem_info_st.dload_mode_addr        = (void *)&(smem_data->smem_dload_flag);
    smem_info_st.sd_upgrade_addr        = (void *)&(smem_data->smem_sd_upgrade);
    smem_info_st.usbport_switch_addr    = (void *)&(smem_data->smem_switch_pcui_flag);
    smem_info_st.app_inactivate_addr    = (void *)&(smem_data->smem_online_upgrade_flag);
    smem_info_st.force_mode_addr        = (void *)&(smem_data->smem_forceload_flag);
    smem_info_st.ut_dload_addr          = (void *)&(smem_data->smem_burn_update_flag);
    smem_info_st.multiupg_dload_addr    = (void *)&(smem_data->smem_multiupg_flag);
    smem_info_st.dload_times_addr       = (void *)&(smem_data->smem_update_times);
    smem_info_st.datalock_status_addr   = (void *)&(smem_data->smem_datalock_status);
    smem_info_st.dt_dload_addr          = (void *)&(smem_data->smem_dt_update_flag);
    smem_info_st.smem_reserve4          = (void *)NULL;
    smem_info_st.smem_reserve3          = (void *)NULL;
    smem_info_st.smem_reserve2          = (void *)NULL;
    smem_info_st.smem_reserve1          = (void *)NULL;


    /* 平台信息初始化 */
    product_info_st = get_product_info();
    if(NULL == product_info_st)
    {
        return DLOAD_ERR;
    }

    memcpy((void *)&platinfo_st.product_info_st, (void *)product_info_st, sizeof(product_info_t));
/*执行LCD与LED归一化代码，进行升级特性支持、升级显示控制路径、闪灯方案初始化*/
#if (FEATURE_ON == MBB_LED_LCD_UNIFORM)
    ret = dload_uniform_init(product_id,&dload_lcd_feature_support,&dload_led_feature_support);
    if(DLOAD_OK == ret)
    {
        /*配置升级支持LCD还是LED*/
        platinfo_st.platform_feature.led_feature  = dload_led_feature_support;
        platinfo_st.platform_feature.lcd_feature  = dload_lcd_feature_support;
    }
    else
    {
        DLOAD_PRINT(KERN_ERR, "dload_uniform_init fail!\n");
        platinfo_st.platform_feature.led_feature  = FALSE;
        platinfo_st.platform_feature.lcd_feature  = FALSE; 
    }
    return ret;
#else
#if (FEATURE_ON == MBB_DLOAD_LED)
    platinfo_st.platform_feature.led_feature  = TRUE;
#else
    platinfo_st.platform_feature.led_feature  = FALSE;
#endif

#if (FEATURE_ON == MBB_DLOAD_LCD)
    platinfo_st.platform_feature.lcd_feature  = TRUE;
#else
    platinfo_st.platform_feature.lcd_feature  = FALSE;
#endif

    return DLOAD_OK;
#endif/*MBB_LED_LCD_UNIFORM*/
}

/*******************************************************************
  函数名称  : dload_get_public_key
  函数描述  : 获取产品镜像对应的公钥
  函数入参  : NA
  函数输出  : NA
  函数返回值: 公钥的长度  : 成功
              DLOAD_ERR   : 失败
********************************************************************/
STATIC int32 dload_get_public_key(uint8 * data_buffer, uint32 data_len)
{
    if(NULL == data_buffer || data_len < PUBLIC_KEY_LEN)
    {
        DLOAD_PRINT(KERN_DEBUG, "data_buffer is NULL or data_len=%ud is too long.\n", data_len);
        return DLOAD_ERR;
    }

    memcpy((void *)data_buffer, (void *)pubkeybuf, sizeof(pubkeybuf));
    return sizeof(pubkeybuf);
}

/*******************************************************************
  函数名称  : dload_get_partition_exchange_flag
  函数描述  : 获取逻辑分区和物理分区是否交换映射的标志
  函数入参  : parti_exchange: 需要获取的逻辑分区和物理分区是否交换映射的标志
  函数输出  : NA
  函数返回值: DLOAD_OK   : 成功
              DLOAD_ERR  : 失败
********************************************************************/
#if (FEATURE_ON == MBB_DLOAD_PARTITION_MAPPING)

STATIC int32 dload_get_partition_exchange_flag(uint32 *parti_exchange)
{
    uint32 smem_boot_flag = 0; /* 共享内存中的物理分区启动标志 */
#if (FEATURE_ON == MBB_IPL)
    smem_huawei_ipl_fota_type *smem_ipl_data = NULL;
#else
    huawei_smem_info *smem_data = NULL;
#endif /* #if (FEATURE_ON == MBB_IPL) */

    /* 入参检查 */
    if (NULL == parti_exchange)
    {
        DLOAD_PRINT(KERN_ERR, "parti_exchange is NULL!");
        return DLOAD_ERR;
    }

#if (FEATURE_ON == MBB_IPL)
    /*lint -e124*/
    smem_ipl_data = (smem_huawei_ipl_fota_type *)SRAM_MBB_IPL_FOTA_FLAG_ADDR;
    if (NULL == smem_ipl_data)
    {
        DLOAD_PRINT(KERN_ERR, "smem_ipl_data is NULL!");
        return DLOAD_ERR;
    }
    /*lint +e124*/

    /* IPL FOTA特性使能时，获取共享内存中的分区启动标志 */
    smem_boot_flag = (HUAWEI_IPL_IMAGE_SECOND == smem_ipl_data->smem_huawei_ipl_magic_run_parition) ? 1 : 0;

    DLOAD_PRINT(KERN_ERR, "smem_huawei_ipl_magic_run_parition=0x%X.",
                smem_ipl_data->smem_huawei_ipl_magic_run_parition);
#else
    /*lint -e124*/
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "smem_data is NULL!");
        return DLOAD_ERR;
    }
    /*lint +e124*/

    /* IPL FOTA特性禁用时，获取共享内存中的分区启动标志 */
    smem_boot_flag = (SMEM_RECB_FLAG_NUM == smem_data->smem_new_recovery_flag) ? 1 : 0;

    DLOAD_PRINT(KERN_ERR, "smem_new_recovery_flag=0x%X.", smem_data->smem_new_recovery_flag);
#endif /* #if (FEATURE_ON == MBB_IPL) */

    /* 使能逻辑分区和物理分区交换映射特性,加载IMAGEA则直接映射，加载IMAGEB则交换映射 */
    *parti_exchange = (1 == smem_boot_flag) ? 1 : 0;

    return DLOAD_OK;
}

#else

STATIC int32 dload_get_partition_exchange_flag(uint32 *parti_exchange)
{
    /* 入参检查 */
    if (NULL == parti_exchange)
    {
        DLOAD_PRINT(KERN_ERR, "parti_exchange is NULL!");
        return DLOAD_ERR;
    }

    /* 禁用逻辑分区和物理分区交换映射特性， 逻辑分区和物理分区是直接映射 */
    *parti_exchange = 0;

    return DLOAD_OK;
}

#endif /* #if (FEATURE_ON == MBB_DLOAD_PARTITION_MAPPING) */


/*******************************************************************
  函数名称  : dload_set_recovery_flag
  函数描述  : 设置双备份下次启动的recovery启动标志
  函数入参  : new_recovery_flag: 需要设置新的recovery启动标志
  函数输出  : NA
  函数返回值: DLOAD_OK   : 成功
              DLOAD_ERR  : 失败
********************************************************************/
STATIC int32 dload_set_recovery_flag(recovery_type_flag *new_recovery_flag)
{
    boolean ret = FALSE;
    recovery_type_flag  old_recovery_flag = {0}; /* 读取旧的OEMINFO分区中的物理分区启动标志 */

    /* 入参检查 */
    if (NULL == new_recovery_flag)
    {
        DLOAD_PRINT(KERN_ERR, "new_recovery_flag is NULL!");
        return DLOAD_ERR;
    }

    /* 参数有效性检查 */
    if ((DLOAD_RECOVERY_FLAG_MAGIC_NUM != new_recovery_flag->magic_number)
        || ((SMEM_RECA_FLAG_NUM != new_recovery_flag->new_recovery_flag)
            && (SMEM_RECB_FLAG_NUM != new_recovery_flag->new_recovery_flag)))
    {
        DLOAD_PRINT(KERN_INFO, "magic_number[0x%X] or new_recovery_flag[0x%X] is not correct!",
                      new_recovery_flag->magic_number, new_recovery_flag->new_recovery_flag);
        return DLOAD_ERR;
    }

    /* 读取OEMINFO分区的分区启动标志 */
    ret = flash_get_share_region_info(RGN_DLOAD_RECOVERY_FLAG, (void *)&old_recovery_flag, sizeof(recovery_type_flag));
    if (ret)
    {
        /* 写入标志和读取标志相同，则直接返回 */
        if ((new_recovery_flag->magic_number == old_recovery_flag.magic_number)
            && (new_recovery_flag->new_recovery_flag == old_recovery_flag.new_recovery_flag))
        {
            DLOAD_PRINT(KERN_INFO, "set the recovery_flag as before:magic_number[0x%X], new_recovery_flag[0x%X].",
                        new_recovery_flag->magic_number, new_recovery_flag->new_recovery_flag);
            return DLOAD_OK;
        }
    }

    /* 更新OEMINFO分区的分区启动标志 */
    ret = flash_update_share_region_info(RGN_DLOAD_RECOVERY_FLAG, (void *)new_recovery_flag, sizeof(recovery_type_flag));
    if (!ret)
    {
        DLOAD_PRINT(KERN_INFO, "update dload recovery flag failure !");
        return DLOAD_ERR;
    }

    return DLOAD_OK;
}

/*******************************************************************
  函数名称  : dload_set_boot_flag
  函数描述  : 升级模式下，设置双备份下次启动的逻辑分区标志(以当前启动的逻辑分区视图为基准)
  函数入参  : data_buffer: 设置逻辑分区启动标志的缓存指针
              data_len   : 设置逻辑分区启动标志的缓存数据长度
  函数输出  : NA
  函数返回值: Others     : 实际设置标志的数据长度
              DLOAD_ERR  : 失败
********************************************************************/
STATIC int32 dload_set_boot_flag(uint8 *data_buffer, uint32 data_len)
{
    int32 ret = DLOAD_ERR;
    uint32 parti_exchange = 0; /* 逻辑分区和物理分区的映射关系：0是直接映射；1是交换映射 */
    uint32 logic_boot_flag = 0; /* 设置的逻辑分区的启动标志 */
    recovery_type_flag  new_recovery_flag = {0}; /* 设置新的OEMINFO分区中的物理分区启动标志 */

    DLOAD_PRINT(KERN_ERR, "dload_set_boot_flag!");
    /* 入参检查 */
    if ((NULL == data_buffer) || (sizeof(uint32) != data_len))
    {
        DLOAD_PRINT(KERN_ERR, "data_buffer[%p] is NULL or data_len[%u] is too long!", data_buffer, data_len);
        return DLOAD_ERR;
    }

    ret  =  dload_get_partition_exchange_flag(&parti_exchange);
    if (DLOAD_OK != ret)
    {
        /* 获取逻辑分区与物理分区映射关系标志失败 */
        DLOAD_PRINT(KERN_ERR, "dload get partition exchange flag  error!");
        return DLOAD_ERR;
    }

    (void)memcpy((void *)&logic_boot_flag, (void *)data_buffer, sizeof(uint32));

    /* 根据逻辑分区启动标志和分区映射交换标志，设置物理分区启动标志 */
    if (DLOAD_BOOT_A_FLAG_NUM == logic_boot_flag)
    {
        new_recovery_flag.new_recovery_flag = (1 == parti_exchange) ? SMEM_RECB_FLAG_NUM : SMEM_RECA_FLAG_NUM;
    }
    else if (DLOAD_BOOT_B_FLAG_NUM == logic_boot_flag)
    {
        new_recovery_flag.new_recovery_flag = (1 == parti_exchange) ? SMEM_RECA_FLAG_NUM : SMEM_RECB_FLAG_NUM;
    }
    else
    {
        /* 设置的逻辑分区启动标志值不正确 */
        DLOAD_PRINT(KERN_ERR, "dload_set_boot_flag data_buffer error: 0x%X!", logic_boot_flag);
        return DLOAD_ERR;
    }

    DLOAD_PRINT(KERN_ERR, "set logic_boot_flag=0x%X, recovery_flag=0x%X.",
                logic_boot_flag, new_recovery_flag.new_recovery_flag);

    new_recovery_flag.magic_number = DLOAD_RECOVERY_FLAG_MAGIC_NUM;

    ret = dload_set_recovery_flag(&new_recovery_flag);
    if (DLOAD_OK != ret)
    {
        DLOAD_PRINT(KERN_ERR, "dload_set_recovery_flag RGN_DLOAD_RECOVERY_FLAG error!");
        return DLOAD_ERR;
    }

    return data_len;
}

/*******************************************************************
  函数名称  : dload_get_boot_flag
  函数描述  : 升级模式下，读取双备份下次启动逻辑分区标志(以当前启动的逻辑分区视图为基准)
  函数入参  : data_len   : 读取逻辑分区启动标志的缓存数据长度
  函数输出  : data_buffer: 读取逻辑分区启动标志的缓存指针
  函数返回值: Others     : 实际读取标志的数据长度
              DLOAD_ERR  : 失败
********************************************************************/
STATIC int32 dload_get_boot_flag(uint8 *data_buffer, uint32 data_len)
{
    int32 ret = DLOAD_ERR;
    boolean result = FALSE;
    uint32 logic_boot_flag = 0; /* 逻辑分区的启动标志 */
    uint32 parti_exchange = 0; /* 逻辑分区和物理分区的映射关系：0是直接映射；1是交换映射 */
    recovery_type_flag recovery_flag = {0}; /* 读取的OEMINFO分区中物理分区的启动标志 */

    /* 入参检查 */
    if ((NULL == data_buffer) || (sizeof(uint32) != data_len))
    {
        DLOAD_PRINT(KERN_ERR, "data_buffer[%p] is NULL or data_len[%u] is too long!", data_buffer, data_len);
        return DLOAD_ERR;
    }

    ret = dload_get_partition_exchange_flag(&parti_exchange);
    if (DLOAD_OK != ret)
    {
        /* 获取逻辑分区与物理分区映射关系标志失败 */
        DLOAD_PRINT(KERN_ERR, "dload get partition exchange flag  error!");
        return DLOAD_ERR;
    }

    /* 读取OEMINFO分区的分区启动标志 */
    result = flash_get_share_region_info(RGN_DLOAD_RECOVERY_FLAG, (void *)&recovery_flag, sizeof(recovery_type_flag));
    if (!result)
    {
        DLOAD_PRINT(KERN_ERR, "flash_get_share_region_info RGN_DLOAD_RECOVERY_FLAG error!");
        return DLOAD_ERR;
    }

    /* 根据物理分区启动标志和分区映射交换标志，设置逻辑分区启动标志 */
    if (DLOAD_RECOVERY_FLAG_MAGIC_NUM != recovery_flag.magic_number)
    {
        /* 分区启动标志的魔术字不正确 */
        DLOAD_PRINT(KERN_ERR, "recovery_flag magic[0x%X] is not correct!", recovery_flag.magic_number);
        return DLOAD_ERR;
    }

    if (SMEM_RECB_FLAG_NUM == recovery_flag.new_recovery_flag)
    {
        /* 读取OEMINFO分区的分区启动标志为物理分区ImageB启动 */
        logic_boot_flag = (1 == parti_exchange) ? DLOAD_BOOT_A_FLAG_NUM : DLOAD_BOOT_B_FLAG_NUM;
    }
    else if (SMEM_RECA_FLAG_NUM == recovery_flag.new_recovery_flag)
    {
        /* 读取OEMINFO分区的分区启动标志为物理分区ImageA启动 */
        logic_boot_flag = (1 == parti_exchange) ? DLOAD_BOOT_B_FLAG_NUM : DLOAD_BOOT_A_FLAG_NUM;
    }
    else
    {
        DLOAD_PRINT(KERN_ERR, "recovery_flag[0x%X] is not correct!",
                    recovery_flag.new_recovery_flag);
        return DLOAD_ERR;
    }

    DLOAD_PRINT(KERN_ERR, "get logic_boot_flag=0x%X, recovery_flag=0x%X.",
                logic_boot_flag, recovery_flag.new_recovery_flag);

    (void)memcpy((void *)data_buffer, (void *)&logic_boot_flag, sizeof(uint32));

    return data_len;
}

/*******************************************************************
  函数名称  : dload_set_update_over_flag
  函数描述  : recovery升级完，并在清除MISC分区的“boot-recovery”启动标志前，清除共享内存中的下载模式标志
  函数入参  : data_buffer: 设置共享内存中的下载模式标志值的缓存指针
              data_len   : 设置共享内存中的下载模式标志值的缓存数据长度
  函数输出  : NA
  函数返回值: Others     : 实际共享内存中的下载模式标志值的数据长度
              DLOAD_ERR  : 失败
********************************************************************/
STATIC int32 dload_set_update_over_flag(uint8 *data_buffer, uint32 data_len)
{
    uint32 dload_flag = 0; /* 保存要设置共享内存中的下载模式标志的值 */
    huawei_smem_info *smem_data = NULL;

    /* 入参检查 */
    if ((NULL == data_buffer) || (sizeof(uint32) != data_len))
    {
        DLOAD_PRINT(KERN_ERR, "data_buffer[%p] is NULL or data_len[%u] is too long!", data_buffer, data_len);
        return DLOAD_ERR;
    }

    (void)memcpy((void *)&dload_flag, (void *)data_buffer, sizeof(uint32));
    if (0 != dload_flag)
    {
        /* 设置的标志值非法，不为0 */
        DLOAD_PRINT(KERN_ERR, "dload_flag[%u] is not correct!", dload_flag);
        return DLOAD_ERR;
    }

    /*lint -e124*/
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "smem_data is NULL!");
        return DLOAD_ERR;
    }
    /*lint +e124*/

    /* 清除共享内存中的下载模式标志 */
    smem_data->smem_dload_flag = dload_flag;

    return data_len;
}

/*******************************************************************
  函数名称  : get_dload_part_name_mapping_len
  函数描述  : 获取分区配置信息
  函数入参  : data_len, 数据长度
  函数输出  : data_buffer, 保存分区映射表的长度
  函数返回值: DLOAD_ERR, 获取表失败
              其他，数据长度
********************************************************************/
STATIC int32 get_dload_part_name_mapping_len(uint8 *data_buffer, uint32 data_len)
{
    uint32 length = sizeof(g_dload_part_name_mapping) / sizeof(g_dload_part_name_mapping[0]);

    /* 入参检查 */
    if ((NULL == data_buffer) || (sizeof(uint32) != data_len))
    {
        DLOAD_PRINT(KERN_ERR, "\ndata_buffer[%p] is NULL or data_len[%u] is wrong!", data_buffer, data_len);
        return DLOAD_ERR;
    }

    (void)memcpy((void *)data_buffer, (void *)&length, data_len);

    return data_len;
}

/*******************************************************************
  函数名称  : get_dload_part_name_mapping_table
  函数描述  : 获取分区到路径的配置信息
  函数入参  : data_len, 数据长度
  函数输出  : data_buffer, 保存分区到路径映射表
  函数返回值: DLOAD_ERR, 获取表失败
              DLOAD_OK，获取表成功
********************************************************************/
STATIC int32 get_dload_part_name_mapping_table(uint8 *data_buffer, uint32 data_len)
{

    /* 入参检查 */
    if ((NULL == data_buffer) || (sizeof(g_dload_part_name_mapping) != data_len))
    {
        DLOAD_PRINT(KERN_ERR, "data_buffer[%p] is NULL or data_len[%u] is wrong!", data_buffer, data_len);
        return DLOAD_ERR;
    }

    (void)memcpy((void *)data_buffer, (void *)&g_dload_part_name_mapping[0], data_len);

    return data_len;
}

/* 消息函数映射表 */
STATIC ioctl_map_t nark_msg_map[] =
{
    {
        /* 获取平台产品特性信息 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_PLATFORMINFO_MSG_ID),
        get_platform_information,
    },
    {
        /* 访问共享内存 */
        MAP_INDEX(DLOAD_MAIN_MODULE, VISIT_DLOAD_SMEM_MSG_ID),
        visit_dload_smem,
    },
    {
         /* 刷新MTD设备 */
        MAP_INDEX(DLOAD_MAIN_MODULE, MTD_FLUSH_MSG_ID),
        dload_mtd_flush,
    },
    {
        /* 获取电池电量格数 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_BAT_LEVEL_MSG_ID),
        get_battery_level,
    },
    {
        /* 获取平台特性信息 */
        MAP_INDEX(DLOAD_SHOW_MODULE, GET_PLATFORMINFO_MSG_ID),
        get_platform_information,
    },
    {
        /* 获取MTD设备文件系统类型 */
        MAP_INDEX(DLOAD_FLASH_MODULE, GET_MTD_FS_TYPE_MSG_ID),
        get_mtd_fs_type,
    },
    {
        /* 访问共享内存 */
        MAP_INDEX(DLOAD_ONLINE_MODULE, VISIT_DLOAD_SMEM_MSG_ID),
        visit_dload_smem,
    },
    {
        /* 主模块获取升级特性信息 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_DLOAD_FEATURE_MSG_ID),
        dload_get_dload_feature,
    },
    {
        /* AT模块获取升级特性信息 */
        MAP_INDEX(DLOAD_AT_MODULE, GET_DLOAD_FEATURE_MSG_ID),
        dload_get_dload_feature,
    },
    {
        /* 安全校验模块获取升级特性信息 */
        MAP_INDEX(DLOAD_SEC_MODULE, GET_DLOAD_FEATURE_MSG_ID),
        dload_get_dload_feature,
    },
    {
        /* 获取path_info_st占的空间大*/
        MAP_INDEX(DLOAD_SHOW_MODULE, GET_PATH_INFO_SIZE_MSG_ID),
        get_show_path_info_size,
    },
    {
        /*获取path_info_st 内容*/
        MAP_INDEX(DLOAD_SHOW_MODULE, GET_PATH_INFO_MSG_ID),
        get_show_path_information,
    },
    {
       /* 获取LCD/LED配置占的空间大小*/
        MAP_INDEX(DLOAD_SHOW_MODULE, GET_SHOW_INFO_SIZE_MSG_ID),
        get_show_info_size,
    },
    {
        /* 获取各升级状态下的LCD/LED配置*/
        MAP_INDEX(DLOAD_SHOW_MODULE, GET_SHOW_INFO_MSG_ID),
        get_show_information,
    },
    {
        /* 获取产品镜像签名对应的公钥 */
        MAP_INDEX(DLOAD_SEC_MODULE, GET_IMG_PUB_KEY_ID),
        dload_get_public_key,
    },
    {
        /* 获取双备份当前启动分区标志 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_BOOT_FLAG),
        dload_get_boot_flag,
    },
    {
        /* 设置双备份当前启动分区标志 */
        MAP_INDEX(DLOAD_MAIN_MODULE, SET_BOOT_FLAG),
        dload_set_boot_flag,
    },
    {
        /* 设置升级完成标志 */
        MAP_INDEX(DLOAD_MAIN_MODULE, SET_UPDATE_OVER_FLAG),
        dload_set_update_over_flag,
    },
    {
        /* 获取分区到路径映射表的大小 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_PART_NAME_MAPPING_LEN),
        get_dload_part_name_mapping_len,
    },
    {
        /* 获取分区到路径映射表 */
        MAP_INDEX(DLOAD_MAIN_MODULE, GET_PART_NAME_MAPPING_TABLE),
        get_dload_part_name_mapping_table,
    },
};

/*******************************************************************
  函数名称  : search_map_callback
  函数描述  : 根据映射索引查找回调函数
  函数入参  : msg_map_index : 映射索引
  函数输出  : NV
  函数返回值: msg_func_t    : 回调函数
********************************************************************/
msg_func_t search_map_callback(uint32  msg_map_index)
{
    int32   index = 0;
    int32   total = 0;

    total = sizeof(nark_msg_map) / sizeof(ioctl_map_t);
    for(index = 0; index < total; index ++)
    {
        if(msg_map_index == nark_msg_map[index].map_index)
        {
            DLOAD_PRINT(KERN_DEBUG, "map_index = %lu : msg_func = %lu.", msg_map_index, (uint32)nark_msg_map[index].msg_func);
            return nark_msg_map[index].msg_func;
        }
    }

    if(index == total)
    {
        DLOAD_PRINT(KERN_ERR, "msg_type = %lu  Ioctl msg_type ERROR.", msg_map_index);
        return NULL;
    }

    return NULL;
}

#ifdef __cplusplus
}
#endif
