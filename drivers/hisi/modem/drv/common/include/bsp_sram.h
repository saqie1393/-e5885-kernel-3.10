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
 
#ifndef __BSP_MEMORY_H__
#define __BSP_MEMORY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_memmap.h"
#include "bsp_s_memory.h"
#include "soc_onchiprom.h"
#include "product_config.h"
#include "bsp_shared_ddr.h"

#ifndef __ASSEMBLY__

struct uart_infor
{
	unsigned int flag;          /*flag which denotes wether this uart will be used*/
	unsigned int base_addr;     /*the register base address*/
	unsigned int interrupt_num; /*interrupt number*/
};
#if (FEATURE_ON == MBB_HWTEST) || (FEATURE_ON == MBB_DRV_M2M_AT) 
/*hardware test*/
typedef  struct
{
    unsigned int smem_hw_mode;             /* 自检模式*/
    unsigned int smem_hw_option;           /* 测试项选择*/
    unsigned int smem_reserved;            /* 保留*/
}hwtest_smem_info;
#define SRAM_HWTEST_SIZE    12
#define HW_TEST_MAGIC_NUM    (0x33557799)
#endif

#if (FEATURE_ON == MBB_DRV_M2M_AT)  
/*dev test*/
typedef  struct
{
    unsigned int   error_type;  /**< Error type */
    unsigned int   error_addr;  /**< Address where error occurs */
    unsigned int   error_data;  /**< Data pattern written where error occurs */
#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
    unsigned int   error_read_data;   /**< Data pattern read where error occurs */
#endif
    unsigned int   ddr_status;  /*DDR 测试状态*/
    unsigned int   bitflip_status;/*bitflip 测试状态*/
#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
    unsigned int   reserved[5];
#endif
}hwtest_boot_test_info_stype;
#define SRAM_DEVTEST_SIZE    20
#endif

typedef struct
{
    int            error_temperature;  /**< error_temperature where error occurs */
    unsigned int   error_count;  /**< error_count where error occurs */
}hwtest_boot_test_info_plus_stype;
#define SRAM_DEVTEST_PLUS_SIZE    8

#define SRAM_DEVTEST_PLUS2_SIZE    44        /**< memtest SRAM_DECTEST_SIZE, add error_read_data*/

#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define MAX_TEMPS 10
#define RESERVED_LEN 4
struct ss_coul_nv_info{
    int charge_cycles;
    int r_pcb; // uohm
    int v_offset_a;
    int v_offset_b;
    int c_offset_a;
    int c_offset_b;
    short temp[MAX_TEMPS];
    short real_fcc[MAX_TEMPS];
    short calc_ocv_reg_v;
    short calc_ocv_reg_c;
    short hkadc_batt_temp;
    short hkadc_batt_id_voltage;
    int start_cc;
    int ocv_temp;
    int limit_fcc;
    int reserved[RESERVED_LEN];
};

#define SRAM_COUL_SIZE    sizeof(struct ss_coul_nv_info)
#endif
#if (FEATURE_ON == MBB_CHG_BQ27510)
struct battery_info{
    unsigned int batt_id_voltage;
    unsigned int reserved2;
    unsigned int reserved3;
    unsigned int reserved4;
};

#define SRAM_BATTERY_INFO_SIZE    sizeof(struct battery_info)
#endif
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define CONFIDENTIAL_NV_OPR_SIZE    (12)
typedef  struct
{
    unsigned int smem_confidential_nv_opr_flag;   /* datalock,simlock NV操作允许 标志*/
    unsigned int smem_datalock_state;                  /* datalock 状态*/
    unsigned int smem_reserve;
}confidential_nv_opr_info;
#endif
#if (MBB_COMMON == FEATURE_ON)
#define  DEBUG_SAVE_LEN  (8)       
#endif

#if (MBB_IPL == FEATURE_ON)
#define SMEM_SIZE_IPL_FOTA    (4)
typedef  struct
{
    unsigned int smem_huawei_ipl_magic_run_parition;    /*image index*/
    unsigned int smem_vote_modem_watchdog;              /* datalock 状态*/
    unsigned int smem_kernel_suspend_process;           /* appcore挂起进程或者激活进程时通知modem */
    unsigned int smem_reserve1;
}smem_huawei_ipl_fota_type;
#endif /*#if (MBB_IPL == FEATURE_ON)*/

/*占用空间较小的部分*/
#define SRAM_TEMP_PROTECT_SIZE 32
#define SRAM_DLOAD_SIZE 64
#define SRAM_DSP_MNTN_SIZE 32
#define SRAM_CDSP_MNTN_SIZE 32
struct dfs_ddr_cfg
{
    unsigned int ddr_freq;
    unsigned int ddr_reg_83c;
    unsigned int ddr_reg_abc;
    unsigned int ddr_reg_b3c;
    unsigned int ddr_reg_bbc;
    unsigned int ddr_reg_c3c;
};
struct hpm_tem_print
{
    unsigned int hpm_hvt_opc;
	unsigned int hpm_uhvt_opc;
	unsigned int hpm_svt_opc;
	signed int	 temperature;
	unsigned char up_volt;
};
typedef struct MULTIPINS
{
    unsigned int MULTIPINS_FLAG;
    unsigned int UART_SPI;
    unsigned int RGMII_MII;
    unsigned int I2S_PCM;
    unsigned int reserve[5];  //保留项
} multipins_share;
typedef struct 
{
    unsigned int        SRAM_NOCARD_VALUE ;             /* 存放无卡注册的设置值 */
    unsigned int        SRAM_NOCARD_SET_FLAG ;          /* 无卡注册是否已经设置的标志 */
}SRAM_NOCARD_STR;

/* drv module  support module struct  define */
typedef struct
{   /* 1: support; 0: not support */
    unsigned int lcd          : 1;
    unsigned int oled         : 1;
    unsigned int gpio_led     : 1;
    unsigned int dr_led       : 1;
    unsigned int battery      : 1;
    unsigned int menu_key     : 1;
    unsigned int reset_key    : 1;
    unsigned int power_key    : 1;
    unsigned int reserved     : 24;
} SMEM_DRV_MODULE_SUPPORT_STRU;

typedef struct tag_SRAM_SMALL_SECTIONS
{
	unsigned int        SRAM_USB_ASHELL;
	struct uart_infor   UART_INFORMATION[3];                            /*three uarts:0/1/2*/
	unsigned int        SRAM_ONOFF[8];
	unsigned int        SRAM_DICC[8];				                    /*GU使用的DICC*/
	unsigned int        SRAM_DSP_DRV;
	unsigned int        SRAM_PCIE_INFO[64];			                    /*DSP镜像加载时使用*/
	unsigned int        SRAM_RESERVED[65];
	unsigned int        SRAM_WDT_AM_FLAG;	                            /*连仿真器停狗标志*/
	unsigned int        SRAM_WDT_CM_FLAG;
	unsigned int        SRAM_BUCK3_ACORE_ONOFF_FLAG;                    /*BUCK3上下电标志*/
	unsigned int        SRAM_BUCK3_CCORE_ONOFF_FLAG;
	unsigned int        SRAM_CUR_CPUFREQ_PROFILE;                       /* current profile */
	unsigned int        SRAM_MAX_CPUFREQ_PROFILE;                       /* max profile */
	unsigned int        SRAM_MIN_CPUFREQ_PROFILE;                       /* min profile */
	unsigned int        SRAM_CPUFREQ_DOWN_FLAG[2];
	unsigned int        SRAM_CPUFREQ_DOWN_PROFILE[2];
#if (MBB_COMMON == FEATURE_ON)
    unsigned int        SRAM_MBB_COMMON;          /* for MBB COMMON driver */
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    unsigned int        SRAM_COUL[SRAM_COUL_SIZE];/*库仑计NV信息存储*/
#endif
#endif
	unsigned int        SRAM_REBOOT_INFO[8];		                    /* E5 开机信息区 不可以被修改*/
	unsigned int        SRAM_TEMP_PROTECT[SRAM_TEMP_PROTECT_SIZE];		/*温保使用的该地址不能被修改*/
	unsigned char       SRAM_DLOAD[SRAM_DLOAD_SIZE];			        /*升级模块使用，不可以被修改*/
	struct tagOcrShareData  SRAM_SEC_SHARE;			                    /*onchiprom启动时存放信息的标志位，放在SRAM的高地址处,不可修改*/

	unsigned char       SRAM_DSP_MNTN_INFO[SRAM_DSP_MNTN_SIZE];		    /* DSP邮箱异常时的可维可测信息 */
	struct dfs_ddr_cfg  SRAM_DFS_DDRC_CFG[2];
	unsigned int 	    SRAM_DUMP_POWER_OFF_FLAG;
	unsigned int 	    SRAM_PM_CHECK_ADDR;
    unsigned int        SRAM_CDSP_DRV;
	unsigned char       SRAM_CDSP_MNTN_INFO[SRAM_CDSP_MNTN_SIZE];		    /* CDSP邮箱异常时的可维可测信息 */
 	unsigned int        SRAM_SEC_ROOTCA[ROOT_CA_LEN/4];
    struct hpm_tem_print hpm_tem;
#ifdef FEATURE_E5_ONOFF
    unsigned int        SRAM_MBB_PD_CHARGE;          /* for MBB PDCHARGE */
#endif
#if (FEATURE_ON == MBB_HWTEST) || (FEATURE_ON == MBB_DRV_M2M_AT)  
    unsigned char   SRAM_HWTEST[SRAM_HWTEST_SIZE];   /*硬件自检*/
#endif

#if (FEATURE_ON == MBB_DRV_M2M_AT)  
    unsigned char   SRAM_DEVTEST[SRAM_DEVTEST_SIZE];   /*DEV TEST*/
#endif
#if (FEATURE_ON == MBB_MLOG)  
    unsigned int   SRAM_ABNORAL_REBOOT;   /*MLOG异常重启*/
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
    unsigned int SRAM_SOLUTION_TYPE;  /*模块使用,用以区分模块下的子产品形态(M2M/CE/车载)*/
#endif
#if (FEATURE_ON == MBB_MODULE_PM)
    unsigned int  SRAM_MODULE_PM[2];
#endif
#if (FEATURE_ON == MBB_MLOG)
    unsigned int   SRAM_MLOG_INIT_FLAG;   /*MLOG模块初始化成功标志*/
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
    unsigned char SRAM_CONFIDENTIAL_NV_OPR[CONFIDENTIAL_NV_OPR_SIZE];
#endif
#if (FEATURE_ON == MBB_DLOAD)
    unsigned int SRAM_COMMON_DPART_FLAG;  /*双备份使用,本功能使用通用宏定义*/
#endif
#if (MBB_COMMON == FEATURE_ON)
    unsigned int        SRAM_MBB_DEBUG[DEBUG_SAVE_LEN];
#endif
#if (MBB_IPL == FEATURE_ON)
    unsigned int        SRAM_MBB_IPL_FOTA_FLAG[SMEM_SIZE_IPL_FOTA];
#endif
    unsigned int       SRAM_DEVTEST_PLUS[SRAM_DEVTEST_PLUS_SIZE];/*devtest结果的新选项*/
    multipins_share    SARM_MULTIPINS_SHARE;  
    SRAM_NOCARD_STR    SRAM_NOCARDFLAG;
    unsigned char      SRAM_DEVTEST_PLUS2[SRAM_DEVTEST_PLUS2_SIZE];/*devtest结果的新选项2*/
    unsigned int       SRAM_PERIPHERAL_SUPPORT_FLAG;
#if (FEATURE_ON == MBB_CHG_BQ27510)
    unsigned int       SRAM_BATTERY_INFO[SRAM_BATTERY_INFO_SIZE];
#endif
}SRAM_SMALL_SECTIONS;

#define MULTIPINS_STATUS 0x9A9F9B90
#define SARM_MULTIPINS_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SARM_MULTIPINS_SHARE)
#define SRAM_DLOAD_ADDR   (void *)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DLOAD)

#define SRAM_PERIPHERAL_SUPPORT_ADDR   (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_PERIPHERAL_SUPPORT_FLAG)

#if (MBB_COMMON == FEATURE_ON)
#define SRAM_MBB_COMMON_NUM              (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_COMMON)
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define SRAM_COUL_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_COUL)/*库仑计共享内存地址*/
#endif
#endif
#ifdef FEATURE_E5_ONOFF
#define SRAM_MBB_PD_CHARGE_ON     0x77778888
#define SRAM_MBB_PD_CHARGE_OFF    0x00000000
#define SRAM_MBB_PD_CHARGE_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_PD_CHARGE)
#endif
#if (FEATURE_ON == MBB_HWTEST)  
#define SRAM_HWTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_HWTEST)/*硬件自检共享内存地址*/
#endif
#if (FEATURE_ON == MBB_MLOG)
#define SRAM_ABNORAL_REBOOT_NUM     0x6F545550
#define SRAM_ABNORAL_REBOOT_CLEAR   0x00000000
#define SRAM_ABNORMAL_REBOOT_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_ABNORAL_REBOOT)/*异常重启共享内存地址*/
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
/*模块子产品形态(M2M/CE/车载)共享内存地址*/
#define SRAM_SOLUTION_TYPE_ADDR  (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_SOLUTION_TYPE)
#endif
#if (FEATURE_ON == MBB_MODULE_PM)
#define USB_SUSPEND_STATUS          (0x12345678)
#define USB_RESUME_STATUS           (0x87654321)
#define PIN_SUSPEND_STATUS          (0x12345678)
#define PIN_RESUME_STATUS           (0x87654321)
#define SRAM_MODULE_PM_ADDR  (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MODULE_PM)/*模块电源管理共享内存地址*/
#endif
#if (FEATURE_ON == MBB_DRV_M2M_AT)
#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
/**< redirect to the SRAM_DEVTEST_PLUS2_ADDR, the old SRAM_DEVTEST_ADDR can be used for further developing*/
#define SRAM_DEVTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DEVTEST_PLUS2)/*硬件自检结果新选项共享内存地址*/
#else
#define SRAM_DEVTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DEVTEST)/*ddr 自检共享内存地址*/
#endif
#endif
#define SRAM_DEVTEST_PLUS_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DEVTEST_PLUS)/*硬件自检结果新选项共享内存地址*/

#if (FEATURE_ON == MBB_MLOG)
#define SRAM_MLOG_INIT_VALUE      0x5A5AAA55
#define SRAM_MLOG_INIT_ADDR    (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MLOG_INIT_FLAG)
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define SMEM_DATALOCK_STATE_NUM   0X8D79F60A  /*DATALOCK解锁状态魔术字 */
#define SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM   0X9C7C021D  /*允许读取DATALOCK\SIMLOCK NV魔术字 */
#define SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR   0x00000000
#define SRAM_CONFIDENTIAL_NV_OPR_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_CONFIDENTIAL_NV_OPR)
#endif
#define SRAM_NOCARD_FLAG_NUM     0xAA77BB88/*NOCARD 已经被设置过的标志魔术字*/
#define NOCARD_OFF   (0)
#define NOCARD_ON    (1)
#define SRAM_NOCARD_VALUE ((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_NOCARDFLAG.SRAM_NOCARD_VALUE
#define SRAM_NOCARD_FLAG ((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_NOCARDFLAG.SRAM_NOCARD_SET_FLAG
#if(FEATURE_ON == MBB_DLOAD)
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SDUP_FLAG_NUM           0X53445550  /*SDUP*/
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SUOK_FLAG_NUM           0X53554F4B  /*SUOK  Scard Upgrade OK*/
#define SMEM_SUFL_FLAG_NUM           0X5355464C  /*SUFL   Scard Upgrade Fail*/
#define SMEM_SDNR_FLAG_NUM           0X53444E52  /*SDNR SD卡升级 NV自动恢复阶段  魔术字*/
#define SMEM_ONNR_FLAG_NUM           0X4F4E4E52  /*ONNR  在线 升级 NV自动恢复阶段  魔术字*/
#define SMEM_RECA_FLAG_NUM           0X5245434A  /*RECA*/
#define SMEM_RECB_FLAG_NUM           0X5245434B  /*RECB*/
#define SMEM_SWITCH_PUCI_FLAG_NUM    0X444E5350  /*NV自动恢复阶段PCUI端口形态*/
#define SMEM_FORCELOAD_FLAG_NUM      0X46434C44  /*FCLD Modem逃生标志 魔术字*/
#define SMEM_FORCELOAD_SUCCESS_NUM  0X4643534E /*FCSN  逃生成功魔术字，    
                                                       在线升级自动恢复也替换为该魔术字,   
                                                       后续下面两个自动恢复相关的魔术字不再使用 */
#define SMEM_ONLINE_AUTO_UPDATE_ENTER_NUM   0x4F415545 /*OAUE 在线升级自动恢复进入魔术字*/
#define SMEM_ONLINE_AUTO_UPDATE_SUCCESS_NUM 0x4F415553 /*OAUS 在线升级自动恢复成功魔术字*/
#define SMEM_BURN_UPDATE_FLAG_NUM           0x4255464E /*BUFN 烧片版本升级魔术字*/
#define SMEM_MULTIUPG_FLAG_NUM                  0x4D545550 /* MTUP 组播升级魔术字*/
#define SMEM_ONUP_FLAG_NUM           0x53555246    /* 在线升级魔术字 */
#define SMEM_DATALOCK_STATUS_FLAG_NUM   0X444C554C  /* DATALOCK 解锁状态魔术字 */
#define SMEM_DT_UPDATE_FLAG_NUM       0x44545546   /*DT加载标志DTUF*/

typedef  struct
{
    unsigned int smem_dload_flag;                  /* 标记是否进入下载模式 */
    unsigned int smem_new_recovery_flag;           /* 标识启动最新需要加载的recovery flag*/
    unsigned int smem_sd_upgrade;                  /* 记录SD卡升级升级标识位，升级时，从SD卡读取升级文件 */
    unsigned int smem_switch_pcui_flag;            /* 标识NV自动恢复阶段，启动时USB端口形态设置*/
    unsigned int smem_online_upgrade_flag;         /* 记录在线升级/SD卡升级NV 自动恢复阶段，APP不启动 */
    unsigned int smem_forceload_flag;              /* 标记是否进入逃生模式 */
    unsigned int smem_online_auto_updata_flag;     /* 标识在线升级自动恢复开始，成功*/ 
    unsigned int smem_burn_update_flag;            /* 烧片版本升级标识*/        
    unsigned int smem_multiupg_flag;               /* 组播升级标识*/ 
    unsigned int smem_update_times;                /* mlog升级次数统计标识*/
    unsigned int smem_datalock_status;             /* datalock解锁状态 */
    unsigned int smem_dt_update_flag;             /*dt升级共享内存标志*/
    unsigned int smem_reserve7;                   /*7 -- 11 暂时保留*/
    unsigned int smem_reserve8;
    unsigned int smem_reserve9;
    unsigned int smem_reserve10;
}huawei_smem_info;

typedef struct _smem_dpart_flag
{
    unsigned int dpart_enable   :1;  /*bit[0] -- 双备份功能启用标志*/
    unsigned int dpart_fastboot_fail  :1;  /*bit[1] -- 加载fastboot失败标志,用来在启动后判断是否切换启动标志*/
    unsigned int dpart_force_dload   :1;  /*bit[2] -- 强制升级标志(开机直接进入升级模式时不启动切换)*/
    unsigned int dpart_reserve  :29;  /*bit[3]-bit[31] -- 预留*/
}smem_dpart_flag_s;
#define SRAM_COMMON_DPART_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_COMMON_DPART_FLAG)
#endif  /*MBB_DLOAD*/

#if (MBB_COMMON == FEATURE_ON)
#define       SRAM_MBB_DEBUG_ADDR  (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_DEBUG)
#endif

#if (MBB_IPL == FEATURE_ON)
#define       SRAM_MBB_IPL_FOTA_FLAG_ADDR  (unsigned int)(((SRAM_SMALL_SECTIONS*)((unsigned long)SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_IPL_FOTA_FLAG)
#endif /*#if (MBB_IPL == FEATURE_ON)*/

/*电池信息共享内存地址*/
#if (FEATURE_ON == MBB_CHG_BQ27510)
#define SRAM_BATTERY_INFO_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_BATTERY_INFO)
#endif

#endif/*__ASSEMBLY__*/


#if !defined(__KERNEL__) && !defined(__OS_VXWORKS__) && !defined(__OS_RTOSCK__)

#ifdef CONFIG_SRAM_SECURE
#define SRAM_BASE_ADDR                  (SHM_BASE_ADDR+SHM_OFFSET_SRAM_TO_DDR)
#define SRAM_MEM_TOP_ADDR               (SRAM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#else
#define SRAM_MEM_TOP_ADDR				(HI_SRAM_MEM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#define SRAM_BASE_ADDR                  (HI_SRAM_MEM_BASE_ADDR)
#endif

#else

#ifdef CONFIG_SRAM_SECURE
#define SRAM_BASE_ADDR                  (void *)((unsigned long)SHM_BASE_ADDR+SHM_OFFSET_SRAM_TO_DDR)
#define SRAM_MEM_TOP_ADDR               (void *)((unsigned long)SRAM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#define SRAM_V2P(addr)                  SHD_DDR_V2P(addr)
#define SRAM_P2V(addr)                  SHD_DDR_P2V(addr)
#else
#define SRAM_MEM_TOP_ADDR				(void *)((unsigned long)g_mem_ctrl.sram_virt_addr + g_mem_ctrl.sram_mem_size)
#define SRAM_BASE_ADDR                  (g_mem_ctrl.sram_virt_addr)
#define SRAM_V2P(addr)                  (void *)((unsigned long)(addr) - (unsigned long)g_mem_ctrl.sram_virt_addr + (unsigned long)g_mem_ctrl.sram_phy_addr)
#define SRAM_P2V(addr)                  (void *)((unsigned long)(addr) - (unsigned long)g_mem_ctrl.sram_phy_addr + (unsigned long)g_mem_ctrl.sram_virt_addr)
#endif

#endif

#define SRAM_OFFSET_SMALL_SECTIONS      (0)
#define SRAM_SIZE_SMALL_SECTIONS		(2*1024) /*预留2k大小，暂为用完*/

/* MCU PM 1K */
#define SRAM_OFFSET_MCU_RESERVE         (SRAM_OFFSET_SMALL_SECTIONS + SRAM_SIZE_SMALL_SECTIONS)
#define SRAM_SIZE_MCU_RESERVE           (1*1024)

/* ICC通道(mcore与acore; mcore与ccore), 各2KB, 共4KB */
#define SRAM_OFFSET_ICC                 (SRAM_OFFSET_MCU_RESERVE + SRAM_SIZE_MCU_RESERVE)
#define SRAM_SIZE_ICC                   (4*1024)


/*LDSP 使用 包括rtt 浅睡标志/ 动态调频调压
第一个4字节为浅睡标志  置为1表示此次睡眠是dsp 浅睡*/
#define SRAM_OFFSET_TLDSP_SHARED        (SRAM_OFFSET_ICC + SRAM_SIZE_ICC)
#define SRAM_SIZE_TLDSP_SHARED          (96)

#define SRAM_OFFSET_GU_MAC_HEADER       (SRAM_OFFSET_TLDSP_SHARED + SRAM_SIZE_TLDSP_SHARED)
#ifndef SRAM_SIZE_GU_MAC_HEADER
#define SRAM_SIZE_GU_MAC_HEADER         (56*1024)
#endif

/*SRAM动态区*/
#define SRAM_OFFSET_DYNAMIC_SEC         (SRAM_OFFSET_GU_MAC_HEADER + SRAM_SIZE_GU_MAC_HEADER)
#define SRAM_SIZE_DYNAMIC_SEC           ((unsigned long)SRAM_MEM_TOP_ADDR - ((unsigned long)SRAM_BASE_ADDR + SRAM_OFFSET_DYNAMIC_SEC))

#ifdef __cplusplus
}
#endif

#endif

