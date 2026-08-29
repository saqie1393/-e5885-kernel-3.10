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



#ifndef _MBB_AT_GU_COMMON_H__
#define _MBB_AT_GU_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "product_config.h"
#include "mdrv.h"
#include "at_lte_common.h"
#include "AtEventReport.h"
#include "LPsNvInterface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_COMMON)

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
VOS_VOID BSP_CombineLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

VOS_VOID BSP_PartLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

VOS_VOID BSP_M2M_LedUpdate(VOS_VOID *pstLedNvStru);
#endif

#if (FEATURE_ON == MBB_DRV_M2M_AT)
/*
<op>: 
0：设置标记，返回OK，重启单板进入flash检查模式，启动扫描
1：全部回写
2：回写<blkid>指定的块
3：退出flash检查模式。（当前状态为正在扫描或者正在回写，返回ERROR）
*/

typedef enum
{
    BITFLIP_OP_SCAN,
    BITFLIP_OP_WB_ALL,
    BITFLIP_OP_WB_BLK,
    BITFLIP_OP_EXIT,
    BITFLIP_OP_MAX,
} bitflip_op_type;
/*
<op_status>：
0：正在扫描
1：扫描完成，没有位翻转
2：扫描完成，有位翻转
3：正在回写
4：回写完成
5：回写失败
6：其它错误
*/
typedef enum
{
    BITFLIP_STATUS_SCAN,
    BITFLIP_STATUS_SCAN_NO_FLIP,
    BITFLIP_STATUS_SCAN_HAS_FLIP,
    BITFLIP_STATUS_WB,
    BITFLIP_STATUS_WB_SUC,
    BITFLIP_STATUS_WB_FAI,
    BITFLIP_STATUS_ERROR,
    BITFLIP_STATUS_MAX,
} bitflip_status_type;

typedef enum
{
    DDR_TEST_ERR_NONE,                    /**< No error */
    DDR_TEST_ERR_UNATTACHED,              /**< No DDR attached */
    DDR_TEST_ERR_MEM_MAP,                 /**< Memory map error */
    DDR_TEST_ERR_DATA_LINES,              /**< Data line error */
    DDR_TEST_ERR_ADDR_LINES,              /**< Address line error */
    DDR_TEST_ERR_OWN_ADDR,                /**< Own address error */
    DDR_TEST_ERR_WALKING_ONES,            /**< Walking ones error */
    DDR_TEST_ERR_SELF_REFRESH,            /**< Self refresh error */
    DDR_TEST_ERR_DEEP_POWER_DOWN,         /**< Deep power down error */
    DDR_TEST_ERR_MEM_CHECK_NINE_SETP,     /**< mem nine step check error */
    DDR_TEST_ERR_MEM_CHECK_BOARD,         /**< mem check board error */
    DDR_TEST_ERR_MEM_PRBS_DATA_ALIGN,     /**< mem prbs data align error */
#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
    DDR_TEST_ERR_HALF_CACHE_LINE,         /*half cache line error*/
    DDR_TEST_ERR_ROW_AUTO_PATTERN,        /*row auto patten error*/
    DDR_TEST_ERR_ROW_BIT_EQUALIZING,      /*Bit equalizing error*/
    DDR_TEST_ERR_ROW_BIT_WORKING,           /*bit working error*/
    DDR_TEST_ERR_ROW_256,                  /*row 256 error*/
    DDR_TEST_ERR_ROW_256_RC,               /*row 256 rc error*/
    DDR_TEST_ERR_FRAME_MARCH_LA,           /*frame march la error*/
    DDR_TEST_ERR_FRAME_MARCH_RAW,           /*frame march raw error*/
#endif
    DDR_TEST_ERR_MAX = 0x7FFFFFFF,        /**< Force enum to 32 bits */
} mbb_at_ddr_test_err_type;

typedef enum
{
    HARDWARE_TEST_OPT_MEM = 0,
    HARDWARE_TEST_OPT_FLASH,
    HARDWARE_TEST_OPT_PERI,
    HARDWARE_TEST_OPT_ALL,
    FLASH_BITFLIP_OPT = 4,
    DDRTEST_OPT = 5,
#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
    DDR_SCREEN_TEST_OPT_CONSERVATIVE_SHORT = 6,         /*保守短时间*/
    DDR_SCREEN_TEST_OPT_CONSERVATIVE_LONG = 7,         /*保守长时间*/
    DDR_SCREEN_TEST_OPT_TOUGH_SHORT = 8,/*严酷短时间*/
    DDR_SCREEN_TEST_OPT_TOUGH_LONG = 9,/*严酷长时间*/
#endif
    HARDWARE_TEST_OPT_MAX,
}MBB_AT_HARDWARE_TEST_OPT;

typedef struct
{
    bitflip_op_type bitflip_op;             /* bitflip命令 */
    bitflip_status_type bitflip_status;     /* bitflip命令状态 */
    int blkaddr_wb;                           /* 回写的块ID*/
}bitflip_cmd_result_type;


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
}devtest_test_info_stype;

#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
typedef struct
{
    int            error_temperature;  /**< error_temperature where error occurs */
    unsigned int   error_count;  /**< error_count where error occurs */
}devtest_test_info_plus_stype;
#endif
typedef  struct
{
    unsigned int smem_hw_mode;             /* 自检模式*/
    unsigned int smem_hw_option;           /* 测试项选择*/
    unsigned int smem_reserved;            /* 保留*/
}at_devtest_info;

typedef enum
{
  DDR_TEST_IS_RUNNING,    
  DDR_TEST_IS_DONE_OK,
  DDR_TEST_IS_DONE_ERR,
  DDR_TEST_OTHER_ERR,
  DDR_TEST_MAX,
} ddr_test_sta;

#define HDTESTMODE  1
#define NOTHDTESTMODE  1
#define FLASH_BITFLIP_OPT  4
#define DDRTEST_OPT  5

#if ((FEATURE_ON == MBB_DDR_SCREEN) && (FEATURE_ON == MBB_FACTORY))
#define    DDR_SCREEN_TEST_OPT_CONSERVATIVE_SHORT   6         /*保守短时间*/
#define    DDR_SCREEN_TEST_OPT_CONSERVATIVE_LONG    7         /*保守长时间*/
#define    DDR_SCREEN_TEST_OPT_TOUGH_SHORT          8         /*严酷短时间*/
#define    DDR_SCREEN_TEST_OPT_TOUGH_LONG           9         /*严酷长时间*/
#define    HARDWARE_TEST_DDR_SCREEN_RESULT_PLUS     (14)
#endif

#define HARDWARE_TEST_MODE_GET 4
#define DEV_TEST_BOOT_DDR_TEST_INFO_GET  (11)
#define DEV_TEST_MODE_GET (4)
#define DEV_TEST_DEV_NAME "/dev/hardwaretest"    /*硬件自检驱动节点*/

#define RET_OK      0
#define RET_FAIL    (-1)
#endif

#endif   /* (FEATURE_ON == MBB_COMMON) */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__AT_H__
 */






