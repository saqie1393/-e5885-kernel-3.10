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

#ifndef _CHG_CHARGE_CUST_H
#define _CHG_CHARGE_CUST_H
/*----------------------------------------------*
 * 包含头文件                                   *
*----------------------------------------------*/
#include "product_config.h"
#include "product_nv_def.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
*----------------------------------------------*/
/*----------------------------------------------*
 * 外部函数原型说明                             *
*----------------------------------------------*/
/*----------------------------------------------*
 * 内部函数原型说明                             *
*----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
*----------------------------------------------*/
/*----------------------------------------------*
 * 常量定义                                     *
*----------------------------------------------*/
/*----------------------------------------------*
 * 宏定义                                       *
*----------------------------------------------*/
/*----------------------------------------------*
* 简单数据类型定义说明                         *
*----------------------------------------------*/
/*=============以下宏定义是高通专用，巴龙采用exl自动生成===============*/
#ifndef FEATURE_ON
#define FEATURE_ON  1
#endif/*FEATURE_ON*/

#ifndef FEATURE_OFF
#define FEATURE_OFF 0
#endif/*FEATURE_OFF*/
/*************************BEGIN:产品功能特性宏定义***********************/
/*充放电功能总控制宏,编译控制宏，外部编译如果定义此处不再定义*/
//#define    MBB_CHARGE
/*巴龙LINUX平台打开此平台特性宏*/
#ifndef   MBB_CHG_PLATFORM_BALONG
#define   MBB_CHG_PLATFORM_BALONG          FEATURE_OFF
#endif/*HUAWEI_CHG_PLATFORM_BALONG*/

/*高通linux平台打开此平台特性宏*/
#ifndef   MBB_CHG_PLATFORM_QUALCOMM
#define   MBB_CHG_PLATFORM_QUALCOMM        FEATURE_OFF
#endif/*HUAWEI_CHG_PLATFORM_QUALCOMM*/

/*LCD产品特性宏*/
#ifndef   MBB_CHG_LCD
#define   MBB_CHG_LCD                      FEATURE_OFF
#endif/*MBB_CHG_LCD*/

/*LED产品特性宏*/
#ifndef   MBB_CHG_LED
#define   MBB_CHG_LED                      FEATURE_OFF
#endif/*MBB_CHG_LED*/

#ifndef   MBB_CHG_OLED
#define   MBB_CHG_OLED                     FEATURE_OFF
#endif/*MBB_CHG_OLED*/

/*外置库仑计BQ27510特性宏*/
#ifndef   MBB_CHG_BQ27510
#define   MBB_CHG_BQ27510                  FEATURE_OFF
#endif/*MBB_CHG_BQ27510*/

/*BQ24196/BQ24192/BQ24296充电芯片特性宏*/
#ifndef   MBB_CHG_BQ24196
#define   MBB_CHG_BQ24196                  FEATURE_OFF
#endif/*MBB_CHG_BQ24196*/

/*BQ25892充电芯片特性宏*/
#ifndef   MBB_CHG_BQ25892
#define   MBB_CHG_BQ25892                  FEATURE_OFF
#endif/*MBB_CHG_BQ25892*/

/*SMB1351充电芯片特性宏*/
#ifndef   MBB_CHG_SMB1351
#define   MBB_CHG_SMB1351                FEATURE_OFF
#endif/*MBB_CHG_SMB1351*/

/*高压快充特性宏*/
#ifndef   MBB_CHG_HVDCP_CHARGE
#define   MBB_CHG_HVDCP_CHARGE             FEATURE_OFF
#endif/*MBB_CHG_HVDCP_CHARGE*/

/*对外充电特性宏*/
#ifndef   MBB_CHG_EXTCHG
#define   MBB_CHG_EXTCHG                   FEATURE_OFF
#endif/*MBB_CHG_EXTCHG*/

/*无线充电特性宏*/
#ifndef   MBB_CHG_WIRELESS
#define   MBB_CHG_WIRELESS                 FEATURE_OFF
#endif/*MBB_CHG_WIRELESS*/

/*工厂补电特性宏*/
#ifndef   MBB_CHG_COMPENSATE
#define   MBB_CHG_COMPENSATE               FEATURE_OFF
#endif/*MBB_CHG_COMPENSATE*/

/*高温充电特性宏*/
#ifndef   MBB_CHG_WARM_CHARGE
#define   MBB_CHG_WARM_CHARGE              FEATURE_OFF
#endif/*MBB_CHG_WARM_CHARGE*/

/*power supply特性宏*/
#ifndef   MBB_CHG_POWER_SUPPLY
#define   MBB_CHG_POWER_SUPPLY             FEATURE_OFF
#endif/*MBB_CHG_POWER_SUPPLY*/

/*库仑计特性宏*/
#ifndef   MBB_CHG_COULOMETER
#define   MBB_CHG_COULOMETER              FEATURE_OFF
#endif/*MBB_CHG_COULOMETER*/

/*高压电池特性宏*/
#ifndef   MBB_CHG_HIGH_VOLT_BATT
#define   MBB_CHG_HIGH_VOLT_BATT          FEATURE_OFF
#endif/*MBB_CHG_HIGH_VOLT_BATT*/

/*可拆卸电池特性宏，用于支持产线AT查询，可拆卸电池打开此宏，非可拆卸电池关闭此宏*/
#ifndef   MBB_CHG_BAT_KNOCK_DOWN
#define   MBB_CHG_BAT_KNOCK_DOWN          FEATURE_OFF
#endif/*MBB_CHG_BAT_KNOCK_DOWN*/

/*高温限制从USB/充电器取电特性宏*/
#ifndef   MBB_CHG_CURRENT_SUPPLY_LIMIT
#define   MBB_CHG_CURRENT_SUPPLY_LIMIT     FEATURE_OFF
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT*/

/*特殊电池充电限流标定如 20%标定特性宏*/
#ifndef   MBB_CHG_CHARGE_CURRENT_LIMIT
#define   MBB_CHG_CHARGE_CURRENT_LIMIT     FEATURE_OFF
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT*/
/*=============以上宏定义是高通专用，巴龙采用exl自动生成===============*/

/*=============以下是数据结构定义，适配时不涉及修改===================*/
#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif
#ifndef uint8_t
typedef unsigned char    uint8_t;
#endif

#ifndef int8_t
typedef signed char    int8_t;
#endif

#ifndef boolean
typedef uint8_t    boolean;
#endif

/*温度对应电压结构,巴龙由于CHG_TEMP_ADC_TYPE 在hisi基线有定义且数据结构
不同，采用CHG_TEMP2ADC_TYPE定义VT表结构*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
typedef struct
{
    int   temperature;
    int   voltage;
}CHG_TEMP2ADC_TYPE;
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
typedef struct
{
    int   temperature;
    int   voltage;
}CHG_TEMP_ADC_TYPE;
#endif

/*电池ID信息索引*/
typedef enum
{
    CHG_BATT_ID_DEF                     = 0 ,
    CHG_BATT_ID_FEIMAOTUI_2300MAH       = 1 ,
    CHG_BATT_ID_XINGWANGDA_1900MAH      = 2 ,
    CHG_BATT_ID_FEIMAOTUI_1900MAH       = 3 ,
    CHG_BATT_ID_XINGWANGDA_1500MAH      = 4 ,
    CHG_BATT_ID_FEIMAOTUI_1500MAH       = 5 ,
    CHG_BATT_ID_LISHEN_1500MAH          = 6 ,
    CHG_BATT_ID_XINWANGDA_3000MAH       = 7 ,
    CHG_BATT_ID_FEIMAOTUI_4800MAH       = 8 ,
    CHG_BATT_ID_XINGWANGDA_4800MAH      = 9 ,
    CHG_BATT_ID_FEIMAOTUI_3000MAH       = 10 ,
    CHG_BATT_ID_FEIMAOTUI_6400MAH       = 11 ,
    CHG_BATT_ID_XINGWANGDA_6400MAH      = 12 ,
    CHG_BATT_ID_MAX
}CHG_BATT_ID_TYPE;

/*电池相关参数结构体*/
struct chg_batt_data {
    unsigned int        id_voltage_min;         //高压电池ID识别的最小电压根据参考电压不同电压不同
    unsigned int        id_voltage_max;         //高压电池ID识别的最大电压根据参考电压不同电压不同
    boolean             is_20pct_calib;         //是否需要充电电流20%标定标志
    CHG_BATT_ID_TYPE    batt_id;
    CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE    chg_batt_volt_paras;
};

/*=================================================================
 CHG_CHGR_UNKNOWN: Chgr type has not been check completely from USB module.
 CHG_WALL_CHGR   : Wall standard charger, which D+/D- was short.
 CHG_USB_HOST_PC : USB HOST PC or laptop or pad, etc.
 CHG_NONSTD_CHGR : D+/D- wasn't short and USB enumeration failed.
 CHG_CHGR_INVALID: External Charger invalid or absent.
 ==================================================================*/
typedef enum
{
    /*未知类型*/
    CHG_CHGR_UNKNOWN        = 0 ,
    /*标准充电器*/
    CHG_WALL_CHGR           = 1 ,
    /*USB*/
    CHG_USB_HOST_PC         = 2 ,
    /*非标准充电器*/
    CHG_NONSTD_CHGR         = 3 ,
    /*无线充电器*/
    CHG_WIRELESS_CHGR       = 4 ,
    /*对外充电器*/
    CHG_EXGCHG_CHGR         = 5 ,
    /*弱充*/
    CHG_500MA_WALL_CHGR     = 6 ,
    /*cradle*/
    CHG_USB_OTG_CRADLE      = 7 ,
    /*高压充电器*/
    CHG_HVDCP_CHGR          = 8 ,
    /*充电器不可用*/
    CHG_CHGR_INVALID,
}chg_chgr_type_t;

/*高压电池0-10℃之间限流结构体*/
typedef struct
{
    /*充电器类型*/
    chg_chgr_type_t chgr_type;
    /*限流参数*/
    unsigned int current_limit;
    /*I_USB限流参数*/
    unsigned int current_limit_usb;
}charger_current_limit_st;

typedef struct
{
    unsigned int pwr_supply_current_limit_in_mA;
    unsigned int chg_current_limit_in_mA;
    unsigned int chg_CV_volt_setting_in_mV;
    unsigned int chg_taper_current_in_mA;
    boolean  chg_is_enabled;
}chg_hw_param_t;

typedef enum
{
    /*CHG_STM_INIT_ST: NOT real state, just for initialize.*/
    CHG_STM_INIT_ST = -1,
    CHG_STM_TRANSIT_ST = 0,
    CHG_STM_FAST_CHARGE_ST,
    CHG_STM_MAINT_ST,
    CHG_STM_INVALID_CHG_TEMP_ST,
    CHG_STM_BATTERY_ONLY,
    CHG_STM_WARMCHG_ST,
    CHG_STM_HVDCP_CHARGE_ST,
    CHG_STM_MAX_ST,
}chg_stm_state_type;

typedef struct
{
    int tem_min;
    int tem_max;
    int revise_tem;
}extchg_batt_temp_revise;

/*=============以上是数据结构定义，适配时不涉及修改===================*/

/*=============以下是充电相关参数，适配时需要根据硬件参数修改=======*/
/*定义充电芯片I2C addr，不同充电芯片有差异*/
#define I2C_CHARGER_CHIP_SLAVE_ADDR    (0x6B)
/*SMB1351 需要定义为0x1D*/
//#define I2C_CHARGER_CHIP_SLAVE_ADDR    (0x1D)

#define    CHG_ENABLE_GPIO                     (GPIO_SLEEP_0_4)   /* charge enable */
#define    CHG_BATT_LOW_INT                    (GPIO_NULL)        /* batt low int */
#define    CHG_BATT_ID_CHAN                    (3)                /* batt id hkadc channel */
#define    CHG_BATT_TEMP_CHAN                  (0)                /* batt temp hkadc channel */
#define    CHG_BATT_VOLT_CHAN                  (0)                /* batt volt hkadc channel */
#define    CHG_VBUS_VOLT_CHAN                  (11)               /* vbus volt hkadc channel */
#define    CHG_USB_TEMP_CHAN                   (12)                /* usb temp hkadc channel */
#define    EXTCHG_CHG_ENABLE                   (GPIO_SLEEP_0_7)   /* A-port ext-charge enable */
#define    SHORT_PROTECT_EN                    (GPIO_SLEEP_1_0)   /* A-port ext-charge short protect enable */
#define    RE_ILIM_1A_GPIO                     (GPIO_SLEEP_0_2)   /* A-port ext-charge current limit */
#define    RE_ILIM_2A_GPIO                     (GPIO_SLEEP_0_3)   /* A-port ext-charge current limit */
#define    EXTCHG_SHORT_VOLT_CHAN              (1)                /* A-port ext-charge short detect */
#define    OTG_ON_CTRL_GPIO                    (GPIO_SLEEP_1_7)   /* A-port ext-charge release power-key */
#define    EXTCHG_OTG_DET_GPIO                 (GPIO_0_6)         /* A-port ext-charge detect */
#define    GPIO_USB_SELECT                     (GPIO_SLEEP_1_3)   /* A-port M-port select */
#define    USB_GPIO_DM                         (GPIO_5_1)         /* A-port DM detect */
#define    USB_GPIO_DP                         (GPIO_5_2)         /* A-port DP detect */
#define    HW_SYS_GPIO_SHUTDOWN                (GPIO_6_6)         /* hardware system shutdown gpio */

/*电池在位检测温度门限*/
#define  BATT_NOT_PRESENCE_TEMP    (-30)
/*开机后异常电池检测*/
#define CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN      (-30)

/*电池短路电压门限*/
#define BAT_CHECK_SHORTED_VOLT    (1000)

/*退出涓充电池电压门限*/
#define BAT_CHECK_JUDGMENT_VOLT    (3000)

/*低电禁止开机电池电压门限*/
#define BAT_CHECK_POWERON_VOLT    (3300)

/*kernle坏电池电压门限*/
#define CHG_SHORT_CIRC_BATTERY_THRES    (2700)

#define CHG_SUB_LOW_TEMP_TOP    (10)
#define CHG_TEMP_RESUM    (3)

#if (MBB_CHG_COMPENSATE == FEATURE_ON)
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/*电池补电阈值40%电量*/
#define TBAT_SUPPLY_VOLT                        (3780)
/*电池放电阈值80%电量*/
#define TBAT_DISCHG_VOLT                        (4085)
/*电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT                   (3825)
/*电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (4050)
#else
/*电池补电阈值40%电量*/
#define TBAT_SUPPLY_VOLT                        (3350)
/*电池放电阈值80%电量*/
#define TBAT_DISCHG_VOLT                        (3970)
/*电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT                   (3500)
/*电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (3865)
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
/*飞毛腿电池补电阈值*/
#define TBAT_SUPPLY_VOLT_FMT                    (3350)
/*飞毛腿电池放电阈值*/
#define TBAT_DISCHG_VOLT_FMT                    (3700)
/*飞毛腿电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT_FMT               (3400)
/*飞毛腿电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT_FMT               (3580)
/*欣旺达电池补电阈值*/
#define TBAT_SUPPLY_VOLT_XWD                    (3350)
/*欣旺达电池放电阈值*/
#define TBAT_DISCHG_VOLT_XWD                    (3700)
/*欣旺达电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT_XWD               (3400)
/*欣旺达电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT_XWD               (3580)
#endif
#endif/*HUAWEI_CHG_HIGH_VOLT_BATT*/
#endif/*(MBB_CHG_COMPENSATE == FEATURE_ON)*/

/* 电池校准的最小及最大电压**/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
#define CHG_BATT_CALI_MIN_VOLT  (3450)/*电压校准门限*/
#define CHG_BATT_CALI_MAX_VOLT  (4350)/*电压校准门限*/
#else
#define CHG_BATT_CALI_MIN_VOLT  (3400)/*电压校准门限*/
#define CHG_BATT_CALI_MAX_VOLT  (4200)/*电压校准门限*/
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

/*电池电压ADC采集分压值*/
#define BAT_VOL_MUIT_NUMBER        804 / 200  /*此处宏定义不要加括号*/
/*VPH_PWR电压ADC采集分压值*/
#define VPH_PWR_VOL_MUIT_NUMBER    804 / 200  /*此处宏定义不要加括号*/

#define CHG_FAST_CHG_TIMER_VALUE                (12 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (12 * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (24 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (24 * SECOND_IN_HOUR)

/*电池格数百分比定义*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define    BATT_CAPACITY_SHUTOFF     (2)
#define    BATT_CAPACITY_LEVELLOW    (10)
#define    BATT_CAPACITY_LEVEL1      (11)
#define    BATT_CAPACITY_LEVEL2      (20)
#define    BATT_CAPACITY_LEVEL3      (30)
#define    BATT_CAPACITY_LEVEL4      (60)
#define    BATT_CAPACITY_WARMRECHG   (87)
#define    BATT_CAPACITY_RECHG       (95)
#define    BATT_CAPACITY_NINTY_NINE  (99)
#define    BATT_CAPACITY_FULL        (100)
#define    BATT_CAPACITY_INTERVAL    (1)
#define    BATT_CAPACITY_LOW_BATT    (3)
#else
#define    BATT_CAPACITY_SHUTOFF     (0)
#define    BATT_CAPACITY_LEVELLOW    (3)
#define    BATT_CAPACITY_LEVEL1      (10)
#define    BATT_CAPACITY_LEVEL2      (30)
#define    BATT_CAPACITY_LEVEL3      (50)
#define    BATT_CAPACITY_LEVEL4      (80)
#define    BATT_CAPACITY_RECHG       (95)
#define    BATT_CAPACITY_FULL        (100)
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if (FEATURE_ON == MBB_CHG_COULOMETER)
/*根据硬件提供的等效电阻计算得到*/
/*
计算方法:
平均等效阻值为10.43021185mohm，相比理想的10mohm电阻偏大了4.3021185%，
即等效阻值为理想阻值的104.3021185%，取倒数为0.958753  ，
该值即为增益校准因子a。
把算出的c_offset_a（斜率）乘以1000000即为该值
*/
#define TEN_MOHM_RESISTANCE_CORRECT    (980392)
/*内阻大小需要根据电池容量和厂家来区分*/
#define R_BATT    (34)
#endif

/*=====================begin NV50016 数据默认值宏定义 =========================*/
/*SHUTOFF 高温关机使能开关*/
#define SHUTOFF_OVER_TEMP_PROTECT_ENABLE_DEFAULT    (1)
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD_DEFAULT    (61)/*power off temp +1 */
/*SHUTOFF 高温关机超过温度门限次数限制*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_CHECK_TIMES_DEFAULT    (1)
/*=====================end NV50016 数据默认值宏定义 ==========================*/

/*=====================begin NV52005 数据默认值宏定义 =========================*/
/*SHUTOFF 低温关机使能开关*/
#define SHUTOFF_LOW_TEMP_PROTECT_ENABLE_DEFAULT    (1)
/*SHUTOFF 低温关机温度门限*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD_DEFAULT    (-20)
/*SHUTOFF 低温关机超过温度门限次数限制*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_CHECK_TIMES_DEFAULT     (1)
/*=====================end  NV52005 数据默认值宏定义 ==========================*/


/*=====================begin NV50385 数据默认值宏定义==========================*/
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*CHG 充电温保护使能开关*/
#define CHG_TEMP_PROTECT_ENABLE_DEFAULT    (1)
/*CHG 充电高温保护停充温度门限*/
#define CHG_OVER_TEMP_STOP_THRESHOLD_DEFAULT    (55)
/*进入高温充电电池温度门限*/
#define CHG_WARM_CHARGE_ENTER_THRESHOLD_DEFAULT    (45)
/*CHG 充电低温保护停充温度门限*/
#define CHG_LOW_TEMP_STOP_THRESHOLD_DEFAULT    (0)
/*CHG 充电高温恢复温度门限*/
#define CHG_OVER_TEMP_RESUME_THRESHOLD_DEFAULT    (52)
/*CHG 充电低温恢复温度门限*/
#define CHG_LOW_TEMP_RESUME_THRESHOLD_DEFAULT    (3)
/*CHG 充电停充轮询次数*/
#define CHG_TEMP_PROTECT_CHECK_TIMES_DEFAULT    (1)
/*CHG 充电复充轮询次数*/
#define CHG_TEMP_RESUME_CHECK_TIMES_DEFAULT    (1)
/*由高温充电恢复到常温充电电池温度门限*/
#define CHG_WARM_CHARGE_EXIT_THRESHOLD_DEFAULT    (42)
#define CHG_TEMP_PROTECT_RESERVED2_DEFAULT    (0)
#else
/*CHG 充电温保护使能开关*/
#define CHG_TEMP_PROTECT_ENABLE_DEFAULT    (1)
/*CHG 充电高温保护停充温度门限*/
#define CHG_OVER_TEMP_STOP_THRESHOLD_DEFAULT    (45)
/*进入高温充电电池温度门限*/
#define CHG_WARM_CHARGE_ENTER_THRESHOLD_DEFAULT    (38)
/*CHG 充电低温保护停充温度门限*/
#define CHG_LOW_TEMP_STOP_THRESHOLD_DEFAULT    (0)
/*CHG 充电高温恢复温度门限*/
#define CHG_OVER_TEMP_RESUME_THRESHOLD_DEFAULT    (42)
/*CHG 充电低温恢复温度门限*/
#define CHG_LOW_TEMP_RESUME_THRESHOLD_DEFAULT    (3)
/*CHG 充电停充轮询次数*/
#define CHG_TEMP_PROTECT_CHECK_TIMES_DEFAULT    (1)
/*CHG 充电复充轮询次数*/
#define CHG_TEMP_RESUME_CHECK_TIMES_DEFAULT    (1)
/*由高温充电恢复到常温充电电池温度门限*/
#define CHG_WARM_CHARGE_EXIT_THRESHOLD_DEFAULT    (0)
#define CHG_TEMP_PROTECT_RESERVED2_DEFAULT    (0)
#endif
/*======================end NV50385 数据默认值宏定义==========================*/

/*======================begin NV50386 数据宏定义================================*/
/*开机电压门限*/
#define BATT_VOLT_POWER_ON_THR_DEFAULT    (3300)
/*关机电压门限*/
#define BATT_VOLT_POWER_OFF_THR_DEFAULT    (3300)
/*充电过压保护门限(平滑值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_THR_DEFAULT    (4220)
/*充电过压保护门限(单次采集值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_ONE_THR_DEFAULT    (4240)
/*超时停充判断门限*/
#define BATT_CHG_TEMP_MAINT_THR_DEFAULT    (4100)
/*高温充电二次复充门限*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
#define BATT_HIGH_TEMP_RECHARGE_THR_DEFAULT    (4000)
#else
#define BATT_HIGH_TEMP_RECHARGE_THR_DEFAULT    (4050)
#endif
/*低电上限门限*/
#define BATT_VOLT_LEVELLOW_MAX_DEFAULT    (3550)
/*0格电压上限门限*/
#define BATT_VOLT_LEVEL0_MAX_DEFAULT    (3550)
/*1格电压上限门限*/
#define BATT_VOLT_LEVEL1_MAX_DEFAULT    (3610)
/*2格电压上限门限*/
#define BATT_VOLT_LEVEL2_MAX_DEFAULT    (3670)
/*3格电压上限门限 */
#define BATT_VOLT_LEVEL3_MAX_DEFAULT    (3770)
/*判断插入充电器是否充电的门限*/
#define BATT_INSERT_CHARGE_THR_DEFAULT    (4150)
/*常温充电的复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR_DEFAULT    (4100)
/*=======================end NV50386 数据宏定义================================*/

/*电池膨胀保护方案参数*/
/*参数说明*/
/*
    区分应用场景，只要充电器在位且电池温度超过45°时就不让电池电压超过4.1V：
    1、充电器在位and电池温度>=45°and电池电压>=4.1V 三个条件同时满足，则USB进入Suspend模式；
    2、电池电压<4.05V（需要有滞回区间）或者电池温度<42°，两个条件有一个满足则USB退出Suspend模式；
    3、高温复充门限统一由4.05V修改至4.0V（为了避免出现刚取消充电芯片输入限流100mA则出现复充问题）
*/
#define CHG_BATTERY_PROTECT_TEMP            (45)  /*电池保护温度门限*/
#define CHG_BATTERY_PROTECT_RESUME_TEMP     (42)  /*退出电池保护温度门限*/
#define CHG_BATTERY_PROTECT_VOLTAGE         (4100)  /*温度超过电池保护门限时，电池电池电压限制*/
#define CHG_BATTERY_PROTECT_RESUME_VOLTAGE  (4050)  /*退出电池保护电压门限*/
#define CHG_BATTERY_PROTECT_CV_VOLTAGE      (4100)  /*电池保护截止电压门限*/

/*USB温保门限值,硬件测试完成后提供*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define CHG_USB_TEMP_LIMIT      (120)     /*USB温保机制触发门限*/
#define CHG_USB_TEMP_RESUME     (100)    /*USB温度机制恢复门限*/
#endif

/*----------------------------------------------*
 * 全局变量                                     *
*----------------------------------------------*/
/*电池温度电压表(V/T表)*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
static const  CHG_TEMP2ADC_TYPE  g_adc_batt_therm_map[] =
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
static const  CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map[] =
#endif
{
    /*该表由硬件提供*/
    {-30,      1654},   /*vol to temp*/
    {-29,      1647},   /*vol to temp*/
    {-28,      1639},   /*vol to temp*/
    {-27,      1632},   /*vol to temp*/
    {-26,      1624},   /*vol to temp*/
    {-25,      1615},   /*vol to temp*/
    {-24,      1607},   /*vol to temp*/
    {-23,      1598},   /*vol to temp*/
    {-22,      1589},   /*vol to temp*/
    {-21,      1580},   /*vol to temp*/
    {-20,      1570},   /*vol to temp*/
    {-19,      1560},   /*vol to temp*/
    {-18,      1550},   /*vol to temp*/
    {-17,      1539},   /*vol to temp*/
    {-16,      1528},   /*vol to temp*/
    {-15,      1517},   /*vol to temp*/
    {-14,      1506},   /*vol to temp*/
    {-13,      1494},   /*vol to temp*/
    {-12,      1482},   /*vol to temp*/
    {-11,      1470},   /*vol to temp*/
    {-10,      1457},    /*vol to temp*/
    {-9,       1444},    /*vol to temp*/
    {-8,       1431},    /*vol to temp*/
    {-7,       1418},    /*vol to temp*/
    {-6,       1404},    /*vol to temp*/
    {-5,       1390},    /*vol to temp*/
    {-4,       1376},    /*vol to temp*/
    {-3,       1361},    /*vol to temp*/
    {-2,       1346},    /*vol to temp*/
    {-1,       1332},    /*vol to temp*/
    {0,        1316},    /*vol to temp*/
    {1,        1301},    /*vol to temp*/
    {2,        1286},    /*vol to temp*/
    {3,        1270},    /*vol to temp*/
    {4,        1254},    /*vol to temp*/
    {5,        1238},    /*vol to temp*/
    {6,        1222},    /*vol to temp*/
    {7,        1205},    /*vol to temp*/
    {8,        1189},    /*vol to temp*/
    {9,        1172},    /*vol to temp*/
    {10,       1155},    /*vol to temp*/
    {11,       1139},    /*vol to temp*/
    {12,       1122},    /*vol to temp*/
    {13,       1105},    /*vol to temp*/
    {14,       1088},    /*vol to temp*/
    {15,       1070},    /*vol to temp*/
    {16,       1053},    /*vol to temp*/
    {17,       1036},    /*vol to temp*/
    {18,       1019},    /*vol to temp*/
    {19,       1002},    /*vol to temp*/
    {20,       985},    /*vol to temp*/
    {21,       968},    /*vol to temp*/
    {22,       951},    /*vol to temp*/
    {23,       934},    /*vol to temp*/
    {24,       917},    /*vol to temp*/
    {25,       900},    /*vol to temp*/
    {26,       883},    /*vol to temp*/
    {27,       867},    /*vol to temp*/
    {28,       850},    /*vol to temp*/
    {29,       817},    /*vol to temp*/
    {30,       801},    /*vol to temp*/
    {31,       785},    /*vol to temp*/
    {32,       769},    /*vol to temp*/
    {33,       753},    /*vol to temp*/
    {34,       738},    /*vol to temp*/
    {35,       723},    /*vol to temp*/
    {36,       707},    /*vol to temp*/
    {37,       693},    /*vol to temp*/
    {38,       663},    /*vol to temp*/
    {39,       649},    /*vol to temp*/
    {40,       635},    /*vol to temp*/
    {41,       621},    /*vol to temp*/
    {42,       607},    /*vol to temp*/
    {43,       593},    /*vol to temp*/
    {44,       580},    /*vol to temp*/
    {45,       567},    /*vol to temp*/
    {46,       554},    /*vol to temp*/
    {47,       541},    /*vol to temp*/
    {48,       529},    /*vol to temp*/
    {49,       517},    /*vol to temp*/
    {50,       505},    /*vol to temp*/
    {51,       493},    /*vol to temp*/
    {52,       481},    /*vol to temp*/
    {53,       470},    /*vol to temp*/
    {54,       459},    /*vol to temp*/
    {55,       454},    /*vol to temp*/
    {56,       448},    /*vol to temp*/
    {57,       438},    /*vol to temp*/
    {58,       427},    /*vol to temp*/
    {59,       417},    /*vol to temp*/
    {60,       407},     /*vol to temp*/
    {61,       397},     /*vol to temp*/
    {62,       388},     /*vol to temp*/
    {63,       379},     /*vol to temp*/
    {64,       370},     /*vol to temp*/
    {65,       361},     /*vol to temp*/
    {66,       353},     /*vol to temp*/
    {67,       344},     /*vol to temp*/
    {68,       336},     /*vol to temp*/
    {69,       328},     /*vol to temp*/
    {70,       320},     /*vol to temp*/
    {71,       312},     /*vol to temp*/
    {72,       305},     /*vol to temp*/
    {73,       298},     /*vol to temp*/
    {74,       291},     /*vol to temp*/
    {75,       284},     /*vol to temp*/
    {76,       277},     /*vol to temp*/
    {77,       270},     /*vol to temp*/
    {78,       264},     /*vol to temp*/
    {79,       257},     /*vol to temp*/
    {80,       251},     /*vol to temp*/
    {81,       245},     /*vol to temp*/
    {82,       240},     /*vol to temp*/
    {83,       234},     /*vol to temp*/
    {84,       228},     /*vol to temp*/
    {85,       223},     /*vol to temp*/
};

/*USB温保，USBV-T,硬件提供*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
static const  CHG_TEMP2ADC_TYPE  g_adc_usb_therm_map[] =
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
static const  CHG_TEMP_ADC_TYPE  g_adc_usb_therm_map[] =
#endif
{
    {-10,      1665},   /*vol to temp*/
    {0,        1575},   /*vol to temp*/
    {5,        1516},   /*vol to temp*/
    {10,       1448},    /*vol to temp*/
    {15,       1370},    /*vol to temp*/
    {20,       1285},    /*vol to temp*/
    {25,       1192},    /*vol to temp*/
    {30,       1095},    /*vol to temp*/
    {35,       996},    /*vol to temp*/
    {40,       897},    /*vol to temp*/
    {45,       801},    /*vol to temp*/
    {50,       710},    /*vol to temp*/
    {55,       624},    /*vol to temp*/
    {60,       546},    /*vol to temp*/
    {65,       476},    /*vol to temp*/
    {66,       463},    /*vol to temp*/
    {67,       450},    /*vol to temp*/
    {68,       437},    /*vol to temp*/
    {69,       425},    /*vol to temp*/
    {70,       413},    /*vol to temp*/
    {71,       401},    /*vol to temp*/
    {72,       390},    /*vol to temp*/
    {73,       379},    /*vol to temp*/
    {74,       368},    /*vol to temp*/
    {75,       358},    /*vol to temp*/
    {76,       347},    /*vol to temp*/
    {77,       337},    /*vol to temp*/
    {78,       328},    /*vol to temp*/
    {79,       318},    /*vol to temp*/
    {80,       309},     /*vol to temp*/
    {81,       300},     /*vol to temp*/
    {82,       292},     /*vol to temp*/
    {83,       283},     /*vol to temp*/
    {84,       275},     /*vol to temp*/
    {85,       267},     /*vol to temp*/
    {86,       259},     /*vol to temp*/
    {87,       252},     /*vol to temp*/
    {88,       244},     /*vol to temp*/
    {89,       237},     /*vol to temp*/
    {90,       231},     /*vol to temp*/
    {91,       224},     /*vol to temp*/
    {92,       217},     /*vol to temp*/
    {93,       211},     /*vol to temp*/
    {94,       205},     /*vol to temp*/
    {95,       199},     /*vol to temp*/
    {96,       193},     /*vol to temp*/
    {97,       188},     /*vol to temp*/
    {98,       182},     /*vol to temp*/
    {99,       177},     /*vol to temp*/
    {100,      172},     /*vol to temp*/
    {101,      167},     /*vol to temp*/
    {102,      162},     /*vol to temp*/
    {103,      158},     /*vol to temp*/
    {104,      153},     /*vol to temp*/
    {105,      149},     /*vol to temp*/
    {106,      145},     /*vol to temp*/
    {107,      141},     /*vol to temp*/
    {108,      137},     /*vol to temp*/
    {109,      133},     /*vol to temp*/
    {110,      129},     /*vol to temp*/
    {111,      125},     /*vol to temp*/
    {112,      122},     /*vol to temp*/
    {113,      118},     /*vol to temp*/
    {114,      115},     /*vol to temp*/
    {115,      112},     /*vol to temp*/
    {116,      109},     /*vol to temp*/
    {117,      106},     /*vol to temp*/
    {118,      103},     /*vol to temp*/
    {119,      100},     /*vol to temp*/
    {120,      98},     /*vol to temp*/
    {121,      95},     /*vol to temp*/
    {122,      92},     /*vol to temp*/
    {123,      90},     /*vol to temp*/
    {124,      87},     /*vol to temp*/
    {125,      85},     /*vol to temp*/
};
#endif

/*温度补偿方案，单内充在位温度补偿表*/
static const extchg_batt_temp_revise g_batt_tem_revise_map_only_chg[] =
{
    {-30,  1,   0},
    {  1,  50,  3},
    { 50,  85,  2},
};

/*温度补偿方案，单外充在位大电流时温度补偿表*/
static const extchg_batt_temp_revise g_batt_tem_revise_map_only_extchg_high_crt[] =
{
    {-30,  36, -1},
    { 36,  41, -4},
    { 41,  46, -7},
    { 46,  51, -9},
    { 51,  56, -10},
    { 56,  61, -11},
    { 61,  66, -13},
    { 66,  71, -14},
    { 71,  75, -16},
    { 75,  80, -18},
    { 80,  82, -20},
    { 82,  83, -21},
    { 83,  85, -22},
};

/*温度补偿方案，单外充在位小电流时温度补偿表*/
static const extchg_batt_temp_revise g_batt_tem_revise_map_only_extchg_low_crt[] =
{
    {-30,   1, -2},
    {  1,  36,  1},
    { 36,  41, -1},
    { 41,  46, -2},
    { 46,  51, -3},
    { 51,  56, -4},
    { 56,  61, -5},
    { 61,  62, -4},
    { 62,  66, -3},
    { 66,  71, -6},
    { 71,  85, -9},
};

/*温度补偿方案，内外充同时在位温度补偿表*/
static const extchg_batt_temp_revise g_batt_tem_revise_map_chg_extchg[] =
{
    {-30,  41,  2},
    { 41,  56,  1},
    { 56,  61, -2},
    { 61,  62, -3},
    { 62,  85, -4},
};

/*温度补偿方案，单电池在位温度补偿表*/
static const extchg_batt_temp_revise g_batt_tem_revise_map_only_batt[] =
{
    {-30,  54,  0},
    { 54,  85, -1},
};

/*以下为不同规格电池的参数*/
/*普通电池*/
static struct chg_batt_data chg_batt_data_default = {
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_DEF,
        .chg_batt_volt_paras    = {3450,3450,4220,4240,4100,4000,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
};

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
/*
static struct chg_batt_data chg_batt_data_feimaotui_1500mah = {
        .id_voltage_min = 887,  //batt id volt
        .id_voltage_max = 1137, //batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_FEIMAOTUI_1500MAH,
        .chg_batt_volt_paras    = {3450,3450,4370,4390,4150,4030,3620,3620,3708,3779,4030,4250,4200},//batt data


};
*/
/*飞毛腿2300mah高压电池*/
/*
static struct chg_batt_data chg_batt_data_feimaotui_2300mah = {
        .id_voltage_min = 852, //batt id volt
        .id_voltage_max = 1032,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_2300MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4050,3600,3600,3690,3803,3948,4280,4200},//batt data
};
*/

/*飞毛腿3000mah高压电池*/
static struct chg_batt_data chg_batt_data_feimaotui_3000mah = {
        .id_voltage_min = 1110, //batt id volt
        .id_voltage_max = 1300,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_3000MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4000,3630,3630,3720,3800,4030,4280,4200},/*batt data*/
};

/*欣旺达3000mah高压电池*/
static struct chg_batt_data chg_batt_data_xinwangda_3000mah = {
        .id_voltage_min = 840, //batt id volt
        .id_voltage_max = 1030,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINWANGDA_3000MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4000,3630,3630,3720,3800,4030,4280,4200},/*batt data*/
};
/*飞毛腿4800mah高压电池*/
static struct chg_batt_data chg_batt_data_feimaotui_4800mah = {
        .id_voltage_min = 220, //batt id volt
        .id_voltage_max = 420,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_4800MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4000,3604,3450,3716,3824,4022,4280,4200},/*batt data*/
};

/*欣旺达4800mah高压电池*/
static struct chg_batt_data chg_batt_data_xinwangda_4800mah = {
        .id_voltage_min = 1530, //batt id volt
        .id_voltage_max = 1730,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_4800MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4000,3604,3450,3716,3824,4022,4280,4200},/*batt data*/
};

/*飞毛腿6400mah电池*/
static struct chg_batt_data chg_batt_data_feimaotui_6400mah = {
        .id_voltage_min = 1100, //batt id volt
        .id_voltage_max = 1300,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_6400MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4000,3630,3630,3720,3800,4030,4280,4200},/*batt data*/
};

/*欣旺达6400mah电池*/
static struct chg_batt_data chg_batt_data_xinwangda_6400mah = {
        .id_voltage_min = 620, //batt id volt
        .id_voltage_max = 830,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_6400MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4000,3630,3630,3720,3800,4030,4280,4200},/*batt data*/
};

/*产品支持的电池规格list*/
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xinwangda_6400mah,
    &chg_batt_data_feimaotui_6400mah,
};

#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
/*飞毛腿2300mah高压电池*/
static struct chg_batt_data chg_batt_data_feimaotui_2300mah = {
        .id_voltage_min = 852, //batt id volt
        .id_voltage_max = 1032,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_2300MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4000,3600,3600,3690,3803,3948,4280,4200},/*batt data*/
};

/*欣旺达1900mah高压电池*/
static struct chg_batt_data chg_batt_data_xingwangda_1900mah = {
        .id_voltage_min = 697,//batt id volt
        .id_voltage_max = 878,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_1900MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4000,3570,3570,3638,3721,3870,4280,4200},/*batt data*/
};
/*飞毛腿1900mah高压电池*/
static struct chg_batt_data chg_batt_data_feimaotui_1900mah = {
        .id_voltage_min = 187,//batt id volt
        .id_voltage_max = 367,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_1900MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4000,3563,3563,3620,3708,3839,4280,4200},/*batt data*/
};

/*产品支持的电池规格list*/
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_feimaotui_2300mah,
    &chg_batt_data_xingwangda_1900mah,
    &chg_batt_data_feimaotui_1900mah,
};
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

/*以下是充电芯片设置参数，根据硬件提供的参数适配*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/*设置0--10度充电电流*/
static const charger_current_limit_st charger_current_limit_paras[CHG_CHGR_INVALID] =
{
    {
        CHG_WALL_CHGR,       /*标充*/
        896,                 /*mA,Charge Current limit*/
        2000,                /*mA,USB Current limit*/
    },
    {
        CHG_USB_HOST_PC,    /*USB2.0/USB3.0*/
        576,                /*mA,Charge Current limit*/
        500,                /*mA,USB Current limit*/
    },
    {
        CHG_WIRELESS_CHGR,    /*非标*/
        576,                /*mA,Charge Current limit*/
        500,                /*mA,USB Current limit*/
    },
    {
        CHG_HVDCP_CHGR,     /*高压充电器*/
        576,
        2000,                /*mA,USB Current limit*/
    },
};
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

static const chg_hw_param_t chg_std_1A_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4208,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};
static const chg_hw_param_t chg_std_2A_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4208,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         896,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

static const chg_hw_param_t chg_std_2A_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        896,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

static const chg_hw_param_t chg_usb_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

static const chg_hw_param_t chg_usb_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4150,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

/*usb3.0限流参数*/
static const chg_hw_param_t chg_usb3_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         896,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4150,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

static const chg_hw_param_t chg_weak_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

#if (MBB_CHG_WIRELESS == FEATURE_ON)
static const chg_hw_param_t chg_wireless_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

/*关机无线充电的限流参数*/
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
static const chg_hw_param_t chg_wireless_poweroff_warmchg_paras =
{
     900,  /*mA, Power Supply front-end Current limit.*/
    1024,  /*mA, Charge Current limit.*/
    4100,  /*mV, CV Voltage setting.*/
     128,  /*mA, Taper(Terminate) current.*/
    TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
};
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#endif /* MBB_CHG_WIRELESS == FEATURE_ON */
/*=============以上是充电相关参数，适配时需要根据硬件参数修改=======*/

#endif/*#define _CHG_CHARGE_CUST_H*/

