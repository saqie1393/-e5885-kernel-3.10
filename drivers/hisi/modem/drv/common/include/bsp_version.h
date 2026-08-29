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


#ifndef __BSP_VERSION_H__
#define __BSP_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__) || defined(__KERNEL__)) ||defined(__OS_RTOSCK_SMP__)
#include <mdrv_version.h>
#endif
#include "product_config.h"
#include <osl_types.h>
#include <drv_nv_def.h>
#include <bsp_trace.h>
#include <bsp_shared_ddr.h>

#ifndef VER_ERROR
#define VER_ERROR (-1)
#endif

#ifndef VER_OK
#define VER_OK 0
#endif

#if (FEATURE_ON == MBB_DLOAD)
#define DLOAD_VERSION      "2.0"                   /* 下载协议版本 */
#define  ver_print_info(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#endif

#define  ver_print_error(fmt, ...) (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

#define HW_VER_INVALID                          (BSP_U32)0XFFFFFFFF
#define HW_VER_UDP_MASK				(BSP_U32)0XFF000000 	/*UDP单板掩码*/
#define HW_VER_UDP_UNMASK			(BSP_U32)(~HW_VER_UDP_MASK) /*UDP掩码取反*/

#define HW_VER_V711_UDP				(BSP_U32)0X71000000 /*V711 UDP*/
#define HW_VER_V750_UDP				(BSP_U32)0X75000000 /*V7R5 UDP*/
#define HW_VER_V722_UDP				(BSP_U32)0X72000000 /*V722 UDP*/
#define HW_VER_K3V5_UDP				(BSP_U32)0X35000000 /*K3V5 UDP*/
#define HW_VER_PXXX                                 (BSP_U32)0XFF000000 /*P532*/

#define HW_VER_PRODUCT_E5785Lh_92a           (BSP_U32)0X00000001 /*V722 E5785Lh-92a*/
#define HW_VER_PRODUCT_E5783h_92a            (BSP_U32)0X00000017 /*V722 E5783h-92a*/
#define HW_VER_PRODUCT_B520s_93a           (BSP_U32)0X00040021 /*V722 B520s-93a*/
/*Telematic ME919Bs-127a 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_127a_A  (BSP_U32)0X00050000 /*V722 ME919Bs-127a A版本*/
#define HW_VER_PRODUCT_ME919Bs_127a_B  (BSP_U32)0X00050001 /*V722 ME919Bs-127a B版本*/  
#define HW_VER_PRODUCT_ME919Bs_127a_C  (BSP_U32)0X00050002 /*V722 ME919Bs-127a C版本*/  
#define HW_VER_PRODUCT_ME919Bs_127a_D  (BSP_U32)0X00050003 /*V722 ME919Bs-127a D版本*/ 

/*Telematic ME919Bs-567a 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_567a_A  (BSP_U32)0X00050004 /*V722 ME919Bs-567a A版本*/
#define HW_VER_PRODUCT_ME919Bs_567a_B  (BSP_U32)0X00050005 /*V722 ME919Bs-567a B版本*/
#define HW_VER_PRODUCT_ME919Bs_567a_C  (BSP_U32)0X00050006 /*V722 ME919Bs-567a C版本*/
#define HW_VER_PRODUCT_ME919Bs_567a_D  (BSP_U32)0X00050007 /*V722 ME919Bs-567a D版本*/

/*Telematic ME919Bs-823a 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_823a_A  (BSP_U32)0X00050008 /*V722 ME919Bs-823a A版本*/
#define HW_VER_PRODUCT_ME919Bs_823a_B  (BSP_U32)0X00050009 /*V722 ME919Bs-823a B版本*/
#define HW_VER_PRODUCT_ME919Bs_823a_C  (BSP_U32)0X00050010 /*V722 ME919Bs-823a C版本*/
#define HW_VER_PRODUCT_ME919Bs_823a_D  (BSP_U32)0X00050011 /*V722 ME919Bs-823a D版本*/

/*Telematic ME919Bs-727a 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_727a_A  (BSP_U32)0X00050012 /*V722 ME919Bs-727a A版本*/
#define HW_VER_PRODUCT_ME919Bs_727a_B  (BSP_U32)0X00050013 /*V722 ME919Bs-727a B版本*/
#define HW_VER_PRODUCT_ME919Bs_727a_C  (BSP_U32)0X00050014 /*V722 ME919Bs-727a C版本*/
#define HW_VER_PRODUCT_ME919Bs_727a_D  (BSP_U32)0X00050015 /*V722 ME919Bs-727a D版本*/

/*Telematic ME909Bs-566N 硬件版本号定义*/
#define HW_VER_PRODUCT_ME909Bs_566N_A  (BSP_U32)0X00050016 /*V722 ME909Bs-566N A版本*/
#define HW_VER_PRODUCT_ME909Bs_566N_B  (BSP_U32)0X00050017 /*V722 ME909Bs-566N B版本*/
#define HW_VER_PRODUCT_ME909Bs_566N_C  (BSP_U32)0X00050018 /*V722 ME909Bs-566N C版本*/
#define HW_VER_PRODUCT_ME909Bs_566N_D  (BSP_U32)0X00050019 /*V722 ME909Bs-566N D版本*/

/*Telematic ME909Bs-123N 硬件版本号定义*/
#define HW_VER_PRODUCT_ME909Bs_123N_A  (BSP_U32)0X00050020 /*V722 ME909Bs-123N A版本*/
#define HW_VER_PRODUCT_ME909Bs_123N_B  (BSP_U32)0X00050021 /*V722 ME909Bs-123N B版本*/
#define HW_VER_PRODUCT_ME909Bs_123N_C  (BSP_U32)0X00050022 /*V722 ME909Bs-123N C版本*/
#define HW_VER_PRODUCT_ME909Bs_123N_D  (BSP_U32)0X00050023 /*V722 ME909Bs-123N D版本*/

/*Telematic ME909Bs-827N 硬件版本号定义*/
#define HW_VER_PRODUCT_ME909Bs_827N_A  (BSP_U32)0X00050024 /*V722 ME909Bs-827N A版本*/
#define HW_VER_PRODUCT_ME909Bs_827N_B  (BSP_U32)0X00050025 /*V722 ME909Bs-827N B版本*/
#define HW_VER_PRODUCT_ME909Bs_827N_C  (BSP_U32)0X00050026 /*V722 ME909Bs-827N C版本*/
#define HW_VER_PRODUCT_ME909Bs_827N_D  (BSP_U32)0X00050027 /*V722 ME909Bs-827N D版本*/

/*Telematic ME919Bs_127bN 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_127bN_A  (BSP_U32)0X00050032 /*V722 ME919Bs-127bN A版本*/
#define HW_VER_PRODUCT_ME919Bs_127bN_B  (BSP_U32)0X00050033 /*V722 ME919Bs-127bN B版本*/
#define HW_VER_PRODUCT_ME919Bs_127bN_C  (BSP_U32)0X00050034 /*V722 ME919Bs-127bN C版本*/
#define HW_VER_PRODUCT_ME919Bs_127bN_D  (BSP_U32)0X00050035 /*V722 ME919Bs-127bN D版本*/

/*Telematic ME919Bs-567bN 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_567bN_A  (BSP_U32)0X00050036 /*V722 ME919Bs-567bN A版本*/
#define HW_VER_PRODUCT_ME919Bs_567bN_B  (BSP_U32)0X00050037 /*V722 ME919Bs-567bN B版本*/
#define HW_VER_PRODUCT_ME919Bs_567bN_C  (BSP_U32)0X00050038 /*V722 ME919Bs-567bN C版本*/
#define HW_VER_PRODUCT_ME919Bs_567bN_D  (BSP_U32)0X00050039 /*V722 ME919Bs-567bN D版本*/

/*Telematic ME919Bs-821bN 硬件版本号定义*/
#define HW_VER_PRODUCT_ME919Bs_821bN_A  (BSP_U32)0X00050040 /*V722 ME919Bs-821bN A版本*/
#define HW_VER_PRODUCT_ME919Bs_821bN_B  (BSP_U32)0X00050041 /*V722 ME919Bs-821bN B版本*/
#define HW_VER_PRODUCT_ME919Bs_821bN_C  (BSP_U32)0X00050042 /*V722 ME919Bs-821bN C版本*/
#define HW_VER_PRODUCT_ME919Bs_821bN_D  (BSP_U32)0X00050043 /*V722 ME919Bs-821bN D版本*/

/*Telematic MU809Bs-2 硬件版本号定义*/
#define HW_VER_PRODUCT_MU809Bs_2_A     (BSP_U32)0X00050028 /*V722 MU809Bs-2 A版本*/
#define HW_VER_PRODUCT_MU809Bs_2_B     (BSP_U32)0X00050029 /*V722 MU809Bs-2 B版本*/
#define HW_VER_PRODUCT_MU809Bs_2_C     (BSP_U32)0X00050030 /*V722 MU809Bs-2 C版本*/
#define HW_VER_PRODUCT_MU809Bs_2_D     (BSP_U32)0X00050031 /*V722 MU809Bs-2 D版本*/

/*E5787Ph-92a 硬件版本号定义*/
#define HW_VER_PRODUCT_E5787Ph_92a     (BSP_U32)0X00000005 /*V722 E5787Ph-92a*/
#define HW_VER_PRODUCT_E5785Lh_22c     (BSP_U32)0X00000003 /*V722 E5785Lh-22c*/
#define HW_VER_PRODUCT_E5787Ph_67a     (BSP_U32)0X00000011 /*V722 E5787Ph-67a*/
#define HW_VER_PRODUCT_B525s_23a       (BSP_U32)0X00040024 /*V722 B525s-23a*/
#define HW_VER_PRODUCT_B528s_23a       (BSP_U32)0X00040026 /*V722 B528s-23a*/
#define HW_VER_PRODUCT_E5785Ch_22c     (BSP_U32)0X00000007 /*V722 E5785Ch-22c*/
#define HW_VER_PRODUCT_B525s_95a       (BSP_U32)0X00040022 /*V722 B525s-95a*/
#define HW_VER_PRODUCT_B529s_23a       (BSP_U32)0X00040027 /*V722 B529s-23a*/
#define HW_VER_PRODUCT_E5785Lh_67a     (BSP_U32)0X00000015 /*V722 E5785Lh-67a*/
#define HW_VER_PRODUCT_R227h           (BSP_U32)0X00000023 /*V722 R227*/
#define HW_VER_PRODUCT_E5785Lh_23c     (BSP_U32)0X00000021 /*V722 E5785Lh-23c*/
/* B525S-65A 硬件版本号定义 */
#define HW_VER_PRODUCT_B525s_65a       (BSP_U32)0X00040025 /* V722 B525s-65a */
#define HW_VER_PRODUCT_E5885Ls_93a     (BSP_U32)0X00000013 /*V722 E5885Ls-93a*/
typedef enum
{
	CHIP_P531 = 0x0530,
	CHIP_P532 = 0x0532,
	CHIP_K3V3 = 0x3630,
	CHIP_K3V5 = 0x3650,
	CHIP_K3V6 = 0x3660,
	CHIP_K970 = 0x3670,
	CHIP_V8R5 = 0x6250,
	CHIP_V711 = 0x6921,
	CHIP_V722 = 0x6932,
	CHIP_V750 = 0x6950
}VERSION_CHIP_TYPE_E;

typedef enum
{
	PLAT_ASIC= 0x0,
	PLAT_FPGA = 0xa,
	PLAT_EMU = 0xe
}VERSION_PLAT_TYPE_E;

typedef enum{
	 BSP_BOARD_TYPE_BBIT    = 0,
	 BSP_BOARD_TYPE_SFT,
	 BSP_BOARD_TYPE_ASIC,
	 BSP_BOARD_TYPE_SOC,
	 BSP_BOARD_TYPE_MAX
}VERSION_BOARD_TYPE_E;

typedef enum
{
	DALLAS_BBIT = 0x1,
	V722_BBIT = 0x2,
	CHICAGO_BBIT = 0x3,
	BOSTON_BBIT = 0x4
}VERSION_BBIT_TYPE_E;

typedef enum
{
	PRODUCT_MBB= 0x0,
	PRODUCT_PHONE = 0x1,
	PRODUCT_ERROR = 0x2
}VERSION_PRODUCT_TYPE_E;

typedef enum
{
	PRODUCT_AUSTIN = 0x3650,
	PRODUCT_CHICAGO = 0x3660,
	PRODUCT_BOSTON = 0x3670,
	PRODUCT_DALLAS = 0x6250,
	PRODUCT_722 = 0x6932,
	PRODUCT_750 = 0x6950,
	PRODUCT_NOT_SUPPORT = 0xFFFF
}VERSION_PRODUCT_NAME_E;

typedef struct
{
	u32 board_id;                    /*硬件版本号，通过hkadc读取。NV和dts在用*/
	u32 board_id_udp_masked;         /*屏蔽扣板信息的硬件版本号。ioshare在用*/
    u32 chip_version;                /*芯片版本号*/
    u16 chip_type;                   /*芯片类型，如CHIP_V711=0x6921*/
	u8  plat_type;                   /*平台类型，如asic/proting/emu*/
	u8  board_type;                  /*平台类型，如BBIT SOC ASIC SFT*/
	u8  bbit_type;                   /*bbit 平台，如dalass bbit/722 BBIT/chicago bbit*/
	u8  product_type;                /*产品类型，如MBB/PHONE*/
    u16 product_name;                /*产品名称，如PRODUCT_722，将722 porting/bbit/sft/udp统一归类为722*/
/*在版本号信息结构中添加一项，用于表示当前单板从HKADC中读取的真实
  硬件子版本号： NV通过board_id无法同时支持多个硬件子版本。*/
#if (FEATURE_ON == MBB_COMMON)
    u8  real_hwIdSub;
#endif
    u32 reserved;
}BSP_VERSION_INFO_S;
#define VERSION_ISO_MAX_LEN  128
#define VERSION_WEBUI_MAX_LEN  128



/*****************************************************************************
*                                                                                                                               *
*            以下提供给version_balong.c(a/c)                                                          *
*                                                                                                                               *
******************************************************************************/

#if defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__)
#define StrParamType (unsigned int)
#define MemparamType (int)
#else
#define StrParamType (int)
#define MemparamType (unsigned int)
#endif

#define VERSION_MAX_LEN 32



/*****************************************************************************
*                                                                                                                               *
*            以下提供给version.c(fastboot)                                                       *
*                                                                                                                               *
******************************************************************************/

typedef struct
{
    u16 vol_low;
    u16 vol_high;
}voltage_range;

#if (FEATURE_ON == MBB_COMMON)
#define HW_VER_PRODUCT_TYPE_MASK	(BSP_U32)0x00FF0000
#define HW_VER_PRODUCT_TYPE_OS		16

/*MBB的产品，通常有Ver.A/B/C 2个硬件子版本。*/
#define HW_MBB_SUB_VER_NR  (2)

/*M2M的产品，通常有Ver.A/B/C/D 4个硬件子版本。*/
#define HW_M2M_SUB_VER_NR  (4)
#endif
/* E5产品硬件版本号定义 */
#ifdef BSP_CONFIG_BOARD_SOLUTION
/*产品形态(包括产品子形态)枚举,共享内存存放值*/
typedef enum{
    PRODUCT_TYPE_M2M       = 0xFAAFFA01,  /*模块m2m产品形态*/
    PRODUCT_TYPE_CE        = 0xFAAFFA02,  /*模块ce产品形态*/
    PRODUCT_TYPE_TELEMATIC = 0xFAAFFA03,  /*模块车载产品形态*/

    PRODUCT_TYPE_INVALID = 0xFAAFFAAF  /*无效产品形态魔数*/
}SOLUTION_PRODUCT_TYPE;
/*产品形态与硬件ID对照表结构体*/
typedef struct _SOLUTION_PRODUCT_INFO_T{
    SOLUTION_PRODUCT_TYPE product_type;
    unsigned int product_id;
}SOLUTION_PRODUCT_INFO_T;
#endif
/*****************************************************************************
*                                                                                                                               *
*            以下提供给adp_version.c(a/c)                                                              *
*                                                                                                                               *
******************************************************************************/

#ifndef isdigit
#define isdigit(c)      (((c) >= '0') && ((c) <= '9'))
#endif

#define CHIP_TYPE_MASK 0xffff0000
#define PLAT_TYPE_MASK 0x0000f000

#define MAX_VER_SECTION 8
#define VER_PART_LEN 3
#define VERC_PART_LEN 2

#ifndef VER_MAX_LENGTH
#define VER_MAX_LENGTH                  30
#endif

typedef struct
{
    unsigned char CompId;                                           /* 组件号：参见COMP_TYPE */
    unsigned char CompVer[VER_MAX_LENGTH+1];         /* 最大版本长度 30 字符+ \0 */
}VERSIONINFO;



/*****************************************************************************
*                                                                                                                               *
*            以下提供给virtual boardid功能                                                          *
*                                                                                                                               *
******************************************************************************/

#define VIRTUAL_BOARDID_SET_OK      0x12345000
#define VIRTUAL_BOARDID_NO_SET      0x12345001
#define VIRTUAL_BOARDID_CMD_NULL    0x12345002
#define VIRTUAL_BOARDID_ERR_FORMAT  0x12345003
#define VIRTUAL_BOARDID_NV_NOBURN   0x12345004
#define VIRTUAL_BOARDID_SET_FLAG    0x12345005

#define MISC_VERSION_OFFSET 100

typedef struct  {
    unsigned err_code;
    unsigned virtual_boardid;
    unsigned timestamp;
    unsigned set_ok_flag;
} misc_ptn_version_info;

typedef enum {
	VIRTUAL_BOARDID_MISC_OK = 0,
	VIRTUAL_BOARDID_MISC_ERROR,
}virtual_boardid_misc_return_type;



/*****************************************************************************
*                                                                                                                               *
*            以下为对外头文件声明                                                              *
*                                                                                                                               *
******************************************************************************/

char * bsp_version_get_hardware(void);
char * bsp_version_get_product_inner_name(void);
char * bsp_version_get_product_out_name(void);

char * bsp_version_get_build_date_time(void);
char * bsp_version_get_chip(void);
char * bsp_version_get_firmware(void);
char * bsp_version_get_release(void);

int bsp_version_acore_init(void);
int bsp_version_ccore_init(void);
void bsp_version_ddr_init(void);
int bsp_version_init(void);
void mdrv_ver_init(void);

VERSION_CHIP_TYPE_E bsp_version_get_chip_type(void);
const BSP_VERSION_INFO_S* bsp_get_version_info(void);

int bsp_version_debug(void);

void update_version_boardid(void);
void set_virtual_boardid(char *virtual_boardid);
void show_virtual_boardid(void);
void clear_virtual_boardid(void);
#ifdef BSP_CONFIG_BOARD_SOLUTION
/*****************************************************************************
* 函数  : bsp_get_solution_type
* 功能  : 获取模块子产品形态(独立对外接口)
      fastboot中该接口通过遍历对照表查找
      A/C/M各内核中使用时直接读取共享内存
* 输入  : void
* 输出  : void
* 返回  : MBB_PRODUCT_TYPE
*****************************************************************************/
SOLUTION_PRODUCT_TYPE bsp_get_solution_type(void);

/*****************************************************************************
* 函数  : m2m_get_board_status
* 功能  : 车载模块ME919Bs/ME909Bs产品根据硬件子版本号获取VBus相关信息
* 输入  : isEc 调用指针，EC前赋值为0，EC后赋值为1
*         isVbus 调用指针，不支持VBus赋值为0，支持VBus赋值为1
* 输出  : huawei_product_info.hwIdSub 根据HKADC子版本号刷新硬件子版本号信息
*****************************************************************************/
void m2m_get_board_status(int *isEC, int *isVbus);
#endif

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函数  : mbb_version_get_board_type
* 功能  : 产品线获取产品硬件ID使用接口
                     产品线新增代码中不能使用上面的海思接口
* 输入  : void
* 输出  : void
* 返回  : BOARD_TYPE_E
*****************************************************************************/
u32 mbb_version_get_board_type(void);


/*****************************************************************************
* 函 数 名  : bsp_get_web_version
*
* 功能描述  : 获取webui_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : webui_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
void bsp_get_web_version(char *str,int len);


void bsp_get_iso_version(char *str, int len);

/*****************************************************************************
* 函 数 名   : bsp_version_get_hardware_no_subver
*
* 功能描述   : 获取硬件版本号
*
* 输入参数   : 无
* 输出参数   : 硬件版本号字符串指针
*
* 返 回 值   : 0获取成功
             -1获取失败
*
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_hardware_no_subver(void);

/*****************************************************************************
* 函数	: huawei_set_update_info
* 功能	: set update info 
* 输入	: void
* 输出	: void
* 返回	: void
*
* 其它       : 无
*
*****************************************************************************/
void huawei_set_update_info(char* str);

/*****************************************************************************
* 函数	: huawei_get_update_info_times
* 功能	: get update info times
* 输入	: NA 
* 输出	: times: num of upgrade 
* 返回	: void 
*
* 其它       : 无
*
*****************************************************************************/
void huawei_get_update_info_times(s32* times);

/*****************************************************************************
* 函数	: huawei_get_spec_num_upinfo
* 功能	: get info of once update 
* 输入	: num : index of upgrade
* 输出	: void
* 返回	: 0获取成功/-1获取失败
*
* 其它       : 无
*
*****************************************************************************/
s32 huawei_get_spec_num_upinfo(char* str, s32 str_len, s32 num);
#endif
#ifdef __cplusplus
}
#endif

#endif

