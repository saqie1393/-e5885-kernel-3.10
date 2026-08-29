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


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "chg_config.h"
#if ( FEATURE_ON == MBB_MLOG )
#include <linux/mlog_lib.h>
#endif
#include "bsp_coul.h"
#include "platform/hisi/chg_chip_platform.h"

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
#if (FEATURE_ON == MBB_CHG_BQ25892)
#include "drv/fsa9688/fsa9688.h"
#endif
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#include <bsp_hkadc.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include "drv/aportextcharge/extcharge_monitor.h"
#include "hi_gpio.h"
#endif

#ifndef CHG_STUB
#include <linux/usb/usb_interface_external.h>
#else
enum chg_current
{
    CHG_CURRENT_HS = 0,     //usb2.0 for 500mA
    CHG_CURRENT_SS ,        //usb3.0 for 900mA
    CHG_CURRENT_NO,         //invalid usb
};
static int usb_speed_work_mode(void)  // 定义与 usb_interface_external.h 有重复，此次添加 static
{
    return 0;
}

#define USB_OTG_CONNECT_DP_DM               (0x0001)
#define USB_OTG_DISCONNECT_DP_DM            (0x0002)  //直连基带，拉低HS_ID
#define USB_OTG_ID_PULL_OUT                 (0x0003)  //直连基带，拉高HS_ID
#define USB_OTG_DISABLE_ID_IRQ              (0x0004)
#define USB_OTG_ENABLE_ID_IRQ               (0x0005)
#endif/*CHG_STUB*/

/*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define TEMP_INITIAL_VALUE                  (0xFFFF)  /*温度初始化值*/
#define USB_TEMP_DETECT_COUNT               (4)        /*USB温度连续检测次数+1*/
#endif

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern struct chargeIC_chip *g_chip;
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
extern struct softtimer_list g_chg_sleep_timer;
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
extern struct softtimer_list g_extchg_shutoff_timer;
#endif

/*硬件产品版本号*/
extern uint32_t  g_hardware_version_id;
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
extern chg_hardware_data_dtsi g_chg_hardpara_dtsi;
extern chg_batt_data_dtsi *g_battery_para_dtsi;
#endif
/*不同充电器充电参数*/
extern const chg_hw_param_t chg_std_1A_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_std_2A_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_std_2A_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
extern const chg_hw_param_t chg_5v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_9v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST];
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
extern const chg_hw_param_t chg_usb_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb3_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb3_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_weak_chgr_hw_paras_def[CHG_STM_MAX_ST];
#if (MBB_CHG_WIRELESS == FEATURE_ON)
extern const chg_hw_param_t chg_wireless_chgr_hw_paras_def[CHG_STM_MAX_ST];
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
extern const chg_hw_param_t chg_wireless_poweroff_warmchg_paras;
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
#endif /* MBB_CHG_WIRELESS == FEATURE_ON */
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern int battery_monitor_blocking_notifier_call_chain(unsigned long val, unsigned long v);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
#define EXT_INSERT  (0x0005)
#define EXT_REMOVE  (0x0004)
BLOCKING_NOTIFIER_HEAD(g_extchg_pciesave_notifier_list);
#endif

struct chg_hardware_data *g_chg_hardware_data = NULL;

struct chg_batt_data *g_chg_batt_data = NULL;

#define BATTERY_DATA_ARRY_SIZE sizeof(chg_batt_data_array) / sizeof(chg_batt_data_array[0])

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
/*0:-Charge Transition State TRIO function.*/
static void chg_transit_state_entry_func(void);
static void chg_transit_state_period_func(void);
static void chg_transit_state_exit_func(void);

/*1:-Fast Charge State TRIO function.*/
static void chg_fastchg_state_entry_func(void);
static void chg_fastchg_state_period_func(void);
static void chg_fastchg_state_exit_func(void);

/*2:-Battery Maintenance State TRIO function.*/
static void chg_maint_state_entry_func(void);
static void chg_maint_state_period_func(void);
static void chg_maint_state_exit_func(void);

/*3:-Invalid Charge Temperature State TRIO function.*/
static void chg_invalid_chg_temp_state_entry_func(void);
static void chg_invalid_chg_temp_state_period_func(void);
static void chg_invalid_chg_temp_state_exit_func(void);

/*4:-Battery Only State TRIO function.*/
static void chg_batt_only_state_entry_func(void);
static void chg_batt_only_state_period_func(void);
static void chg_batt_only_state_exit_func(void);

static void chg_stm_set_cur_state(chg_stm_state_type new_state);

/*function declaration of a-port ext-charge*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
int32_t chg_batt_temp_revise(int32_t batt_temp);
void extchg_device_insert_proc(void);
void extchg_device_remove_proc(void);
void extchg_status_update_in_sleep_mode();
void extchg_monitor_func_in_sleep_mode(int32_t bat_vol, int32_t bat_temp);
int32_t batt_revise_calc_average_volt_value(int32_t new_data);
int32_t batt_revise_calc_average_temp_value(int32_t new_data);
int32_t batt_revise_calc_average_curr_value(int32_t new_data);
#endif/*FEATURE_ON == MBB_CHG_APORT_EXTCHG*/

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*5:-Warmchg State TRIO function.*/
static void chg_warmchg_state_entry_func(void);
static void chg_warmchg_state_period_func(void);
static void chg_warmchg_state_exit_func(void);
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
static void chg_set_recharge_threshold(void);
#endif/*BSP_CONFIG_BOARD_E5885Ls_93a*/

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
/*6:-HVDCP Charge State TRIO function.*/
static void chg_hvdcpchg_state_entry_func(void);
static void chg_hvdcpchg_state_period_func(void);
static void chg_hvdcpchg_state_exit_func(void);
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
void chg_sleep_batt_check_timer(void);
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
static uint32_t g_real_factory_mode;                  //是否工厂模式
static boolean chg_batt_high_temp_58_flag = FALSE;    //高温58度标志
static boolean chg_batt_low_battery_flag = FALSE;     //低电标志
static boolean chg_batt_condition_error_flag = FALSE; //电池异常标志
static boolean chg_limit_supply_current_flag = FALSE; //温保限流标识
static boolean chg_powdown_charging_time_expired_flag = FALSE;/*关机充电超时标志位*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
static boolean chg_battery_protect_flag = FALSE;     //电池保护标识
static boolean g_chg_over_temp_volt_protect_flag = FALSE;  //过温前过压保护标识
static boolean g_chg_longtime_nocharge_protect_flag = FALSE; //满电停充电源长时间在位状态标识
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define BATT_EXPAND_CAPACITY_RECHG 60
static boolean g_chg_battery_expand_change_normal_rechg_flag = FALSE;
#endif
#endif
/*烧片版本上对外充电检测开始的标识位*/
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
static int32_t g_extchg_start = 0;
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
/*the current of battery*/
static int32_t battery_current = 0;
/*the last current of battery*/
static int32_t battery_current_prev = 0;
/*the voltage of battery*/
int32_t battery_voltage = 0xFFFF;
/*the temperature of battery*/
int32_t battery_temperature = 0;
/*the time of short circult detections*/
static int32_t count_for_short_det = 0;
/*the flag of short circult*/
static boolean extchg_short_flag = FALSE;
/*the time of loop of short detect*/
static int32_t poll_round_for_det = 0;
/*the last state of ext-charge*/
static EXTCHG_ILIM ext_chg_status_old = RE_ILIM_NA;
/*the state of ext-charge*/
static EXTCHG_ILIM ext_chg_status_new = RE_ILIM_STOP;
/*the temporary state of ext-charge*/
static EXTCHG_ILIM ext_chg_status_temp = RE_ILIM_NA;
/*the flag of charge loop*/
static boolean usb_err_flag = FALSE;
/*the define of interruption device ID*/
#define DEV_OTG_AB            0xBB
/*the variable of interruption device ID*/
int ext_dev_id = DEV_OTG_AB;
/*the work queue of ext-charge plug in*/
struct work_struct extchg_plug_in_work;
/*the work queue of ext-charge plug out*/
struct work_struct extchg_plug_out_work;
#endif

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
/*高温充电时保存复充门限*/
int32_t g_chg_rechg_threshod = BATT_CAPACITY_RECHG;
/*关机外充时单板能否关机标志位*/
boolean g_extchg_shutoff_flag = TRUE;
#endif

/*充放电相关NV*/
CHG_BATTERY_OVER_TEMP_PROTECT_NV   g_chgBattOverTempPeotect;  //NV50016
CHG_BATTERY_LOW_TEMP_PROTECT_NV    g_chgBattLowTempPeotect;   //NV52005
CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE   g_chgShutOffTempProtect;   //NV50385
CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE   g_chgBattVoltProtect;      //NV50386
POWERUP_MODE_TYPE                  g_real_powerup_mode_value; //NV50364

/***************************begin NV50016 数据宏定义 ************************/
/*SHUTOFF 高温关机使能开关*/
#define SHUTOFF_OVER_TEMP_PROTECT_ENABLE g_chgBattOverTempPeotect.ulIsEnable
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD g_chgBattOverTempPeotect.lCloseAdcThreshold
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_CHECK_TIMES g_chgBattOverTempPeotect.ulTempOverCount
/***************************end   NV50016 数据宏定义 ************************/

/***************************begin NV52005 数据宏定义 ************************/
/*SHUTOFF 高温关机使能开关*/
#define SHUTOFF_LOW_TEMP_PROTECT_ENABLE g_chgBattLowTempPeotect.ulIsEnable
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD g_chgBattLowTempPeotect.lCloseAdcThreshold
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_CHECK_TIMES g_chgBattLowTempPeotect.ulTempLowCount
/***************************end  NV52005 数据宏定义 ************************/


/************************************begin NV50385 数据宏定义 *******************************/
/*CHG 充电温保护使能开关*/
#define CHG_TEMP_PROTECT_ENABLE g_chgShutOffTempProtect.ulChargeIsEnable
/*CHG 充电高温/低温保护门限*/
#define CHG_OVER_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.overTempchgStopThreshold
#define CHG_LOW_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.lowTempChgStopThreshold
/*CHG 充电高温/低温恢复温度门限*/
#define CHG_OVER_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.overTempChgResumeThreshold
#define CHG_LOW_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.lowTempChgResumeThreshold
/*CHG 充电停充轮询次数*/
#define CHG_TEMP_PROTECT_CHECK_TIMES g_chgShutOffTempProtect.chgTempProtectCheckTimes
/*CHG 充电复充轮询次数*/
#define CHG_TEMP_RESUME_CHECK_TIMES g_chgShutOffTempProtect.chgTempResumeCheckTimes
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*进入高温充电电池温度门限*/
#define CHG_WARM_CHARGE_ENTER_THRESHOLD g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold
/*由高温充电恢复到常温充电电池温度门限*/
#define CHG_WARM_CHARGE_EXIT_THRESHOLD g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
/**********************************end NV50385 数据宏定义 *******************************/

/************************************begin NV50386 数据宏定义 *******************************/
/*开机电压门限*/
#define BATT_VOLT_POWER_ON_THR g_chgBattVoltProtect.battVoltPowerOnThreshold
/*关机电压门限*/
#define BATT_VOLT_POWER_OFF_THR g_chgBattVoltProtect.battVoltPowerOffThreshold
/*充电过压保护门限(平滑值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_THR g_chgBattVoltProtect.battOverVoltProtectThreshold
/*充电过压保护门限(单次采集值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_ONE_THR g_chgBattVoltProtect.battOverVoltProtectOneThreshold
/*超时停充判断门限*/
#define BATT_CHG_TEMP_MAINT_THR g_chgBattVoltProtect.battChgTempMaintThreshold
/*高温充电二次复充门限*/
#define BATT_HIGH_TEMP_RECHARGE_THR g_chgBattVoltProtect.battChgRechargeThreshold

/*低电上限门限*/
#define BATT_VOLT_LEVELLOW_MAX g_chgBattVoltProtect.VbatLevelLow_MAX
/*0格电压上限门限*/
#define BATT_VOLT_LEVEL0_MAX g_chgBattVoltProtect.VbatLevel0_MAX
/*1格电压上限门限*/
#define BATT_VOLT_LEVEL1_MAX g_chgBattVoltProtect.VbatLevel1_MAX
/*2格电压上限门限*/
#define BATT_VOLT_LEVEL2_MAX g_chgBattVoltProtect.VbatLevel2_MAX
/*3格电压上限门限 */
#define BATT_VOLT_LEVEL3_MAX g_chgBattVoltProtect.VbatLevel3_MAX
/*判断插入充电器是否充电的门限*/
#define BATT_INSERT_CHARGE_THR g_chgBattVoltProtect.battInsertChargeThreshold
/*常温充电的复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR g_chgBattVoltProtect.battNormalTempRechergeThreshold

/************************************end NV50386 数据宏定义 *******************************/

/*当前是否为工厂模式: true:工厂模式 false :非工厂模式*/
static boolean chg_current_ftm_mode = FALSE;
/*记录充电温保护标志,初始化为温度正常**/
static uint32_t chg_temp_protect_flag = FALSE;
static uint32_t fact_release_flag = FALSE;
/*记录电池温度状态*/
TEMP_EVENT chg_batt_temp_state = TEMP_BATT_NORMAL;

/*记录实际的充电状态信息**/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
/*add the average current sending to app*/
CHG_PROCESS_INFO chg_real_info = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, BATT_LEVEL_0, 0,0,0xFFFF,0xFFFF,0xFFFF};
#else
CHG_PROCESS_INFO chg_real_info = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, BATT_LEVEL_0, 0,0,0xFFFF,0xFFFF};
#endif
/*记录USB端口温度保护信息*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
static USB_TEMP_PROTECT_INFO chg_usb_temp_info = {TEMP_INITIAL_VALUE, FALSE, FALSE};
#endif

static chg_stm_type chg_stm_state_machine[CHG_STM_MAX_ST + 1] =
{
    /* 0 Transition State, which is also the default after system boot-up.*/
    {
        (chg_stm_func_type)chg_transit_state_entry_func,
        (chg_stm_func_type)chg_transit_state_period_func,
        (chg_stm_func_type)chg_transit_state_exit_func,
    },

    /* 1 Fast Charge State, battery is charged during this state.*/
    {
        (chg_stm_func_type)chg_fastchg_state_entry_func,
        (chg_stm_func_type)chg_fastchg_state_period_func,
        (chg_stm_func_type)chg_fastchg_state_exit_func,
    },

    /* 2 Maintenance State, battery has been charged to full, system was supplied by
         external charger preferentially.*/
    {
        (chg_stm_func_type)chg_maint_state_entry_func,
        (chg_stm_func_type)chg_maint_state_period_func,
        (chg_stm_func_type)chg_maint_state_exit_func,
    },

    /* 3 Invalid Charge Temperature State, external charger is present, while battery
         is too hot/cold to charge.*/
    {
        (chg_stm_func_type)chg_invalid_chg_temp_state_entry_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_period_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_exit_func,
    },

    /* 4 Battery Only State, external charger is absent, or removed, system was supplied
         by battery.*/
    {
        (chg_stm_func_type)chg_batt_only_state_entry_func,
        (chg_stm_func_type)chg_batt_only_state_period_func,
        (chg_stm_func_type)chg_batt_only_state_exit_func,
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /* 5 warm chg State, battery is charged during this state when the battery temp is in
      warm chg area*/
    {
        (chg_stm_func_type)chg_warmchg_state_entry_func,
        (chg_stm_func_type)chg_warmchg_state_period_func,
        (chg_stm_func_type)chg_warmchg_state_exit_func,
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    /*6:-HVDCP Charge State TRIO function.*/
    {
        (chg_stm_func_type)chg_hvdcpchg_state_entry_func,
        (chg_stm_func_type)chg_hvdcpchg_state_period_func,
        (chg_stm_func_type)chg_hvdcpchg_state_exit_func,
    },
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    {
        NULL,NULL,NULL
    }
};
/*----------------------------------------------*
 * 全局变量                                   *
 *----------------------------------------------*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT ) && (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0,0};
#elif ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0,0};
#elif (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0};
#else
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0};
#endif

/* 充电使能标志提供给USB驱动用于弱充检测1:使能充电 0:没有使能*/
int chg_en_flag = 0;
unsigned long chg_en_timestamp = 0;
#define CHG_EN_TIME_SLOT       (50)   //充电使能时间
int g_default_dpm_volt = 0;           //DPM值
int g_timeout_to_poweroff_reason = 0; //电池异常超时关机原因

static const chg_hw_param_t* chg_std_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_usb_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_usb3_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_weak_chgr_hw_paras = NULL;
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
static const chg_hw_param_t* chg_5v_hvdcp_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_9v_hvdcp_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_12v_hvdcp_chgr_hw_paras = NULL;
#endif/*FEATURE_ON == MBB_CHG_HVDCP_CHARGE*/
#if (MBB_CHG_WIRELESS == FEATURE_ON)
static const chg_hw_param_t* chg_wireless_chgr_hw_paras = NULL;
/*无线充电器在位标志，用于无线充电和对外充电互斥切换*/
boolean g_wireless_online_flag = OFFLINE;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
#define HVDCP_CHARGE_SET_CV_COMPARE_TH    (4100)
#define HVDCP_CHARGE_CV_4200MV             (4200)
#define HVDCP_CHARGE_CV_4350MV             (4350)
#define HVDCP_CHARGE_0P5_CURRENT_TH       (1200)
#define HVDCP_CHARGE_CURRENT_500MA        (500)
/*4.2V-4.35V 0.5C充电限流标志*/
boolean g_hvdcp_charge_currnt_limit_flag = FALSE;
chg_hvdcp_type_value g_hvdcp_init_vol = CHG_HVDCP_9V;
#endif/*FEATURE_ON == MBB_CHG_HVDCP_CHARGE*/

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/* 0 - 10 ℃充电相关参数设定*/
int chg_sub_low_temp_changed = -1;
extern const charger_current_limit_st charger_current_limit_paras[CHG_CHGR_INVALID];
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

/*0:-PwrOff Charging; 1:-Normal Charging.*/
/*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
static const uint32_t chg_fastchg_timeout_value_in_sec[2][2] =
{
    /*Power-off charge.*/
    {
        CHG_POWEROFF_FAST_USB_TIMER_VALUE,    //USB/NoStd Chgr
        CHG_POWEROFF_FAST_CHG_TIMER_VALUE,    //Wall/Standard Chgr
    },
    /*Normal charge.*/
    {
        CHG_FAST_USB_TIMER_VALUE,             //USB/NoStd Chgr
        CHG_FAST_CHG_TIMER_VALUE,             //Wall/Standard Chgr
    },
};

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
static const char* chg_stm_name_table[CHG_STM_MAX_ST] =
{
    "STM_TRANSIT_ST",                   //过渡状态
    "STM_FAST_CHARGE_ST",               //快充状态
    "STM_MAINT_ST",                     //满电状态
    "STM_INVALID_CHG_TEMP_ST",          //过温状态
    "STM_BATTERY_ONLY",                 //电池单独在位状态
    "STM_WARMCHG_ST",                   //高温充电状态
    "STM_HVDCP_CHARGE_ST"               //高压快充状态
};

static const char* chg_chgr_type_name_table[CHG_CHGR_INVALID + 1] =
{
    "CHG_CHGR_UNKNOWN",                 //充电器类型未知
    "CHG_WALL_CHGR",                    //标准充电器
    "CHG_USB_HOST_PC",                  //USB
    "CHG_NONSTD_CHGR",                  //第三方充电器
    "CHG_WIRELESS_CHGR",                //无线充电器
    "CHG_EXGCHG_CHGR",                  //对外充电器
    "CHG_500MA_WALL_CHGR",              //弱充
    "CHG_USB_OTG_CRADLE",               //cradle
    "CHG_HVDCP_CHGR",                   //高压充电器
    "CHG_CHGR_INVALID"                  //充电器不可用
};

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*Lookup charge state machine name from chg_stm_name_table.*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed (CHG_STM_INIT_ST, CHG_STM_MAX_ST) range.*/
#define TO_STM_NAME(i)   ((i >= CHG_STM_TRANSIT_ST && i < CHG_STM_MAX_ST) \
                         ? chg_stm_name_table[i] : "NULL")

/*Lookup charger type name from chg_chgr_type_name_table*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed [CHG_CHGR_UNKNOWN, CHG_CHGR_INVALID] range.*/
#define TO_CHGR_NAME(i)  ((i >= CHG_CHGR_UNKNOWN && i <= CHG_CHGR_INVALID) \
                         ? chg_chgr_type_name_table[i] : "NULL")

/*电池温度平滑的长度个数:快轮询长度为30，满轮询长度为5*/
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST   (30)
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW   (5)

/*电池电压平滑的长度个数:快轮询长度为30，满轮询长度为5*/
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST    (30)
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW    (5)

/*过温关机底层等待时间，若APP在该时间段内仍未关机，则底层执行关机*/
#define  OVER_TEMP_SHUTOFF_DURATION         (45000)
/*低电关机底层等待时间，若APP在该时间段内仍未关机，则底层执行关机*/
#define  LOW_BATT_SHUTOFF_DURATION          (45000)

/*开机后异常电池检测*/
/* 开机后坏电池检测的电压门限***/

/*高温关机的温度告警回滞带，在达到关机门限前的该范围，提示高温*/
#define SHUTOFF_HIGH_TEMP_WARN_LEN          (2)
/*高温提示的恢复回滞带，在温度恢复到关机门限的该范围以下，撤销高温告警提示*/
#define SHUTOFF_HIGH_TEMP_RESUME_LEN        (5)

#define LIMIT_SUPPLY_CURR_TEMP              (58)
#define LIMIT_SUPPLY_CURR__RESUME_LEN       (1)

/*满电停充且充电器在位进入suspend的时间门限*/
#define CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS (16 * 60 * 60)

/*满电停充且充电器长时间在位状态常温复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE    (4000)
/*保存进入 满电停充且充电器长时间在位状态 前常温复充门限*/
unsigned int g_batt_normal_temp_recherge_threshold = 0;
/*电池过温后给应用上报过温事件的次数*/
#define BATTERY_EVENT_REPORT_TIMES          (5)

static int32_t g_chg_revise_count = 0;
/*开始充电前的电池电压，用于充电时的电压补偿防反转*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4280) //充电电池电压补偿上限
#else
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4150) //充电电池电压补偿上限
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
#define CHG_BATT_VOLT_REVISE_LIMIT_DOWN     (3350)//放电电池电压补偿下限

#define CHG_BATT_VOLT_REVISE_WINDOW         (35)  //电压虚高变化太小则不补补偿
#define CHG_BATT_VOLT_REVISE_LIMIT          (200) //虚高补偿限制值
#define CHG_BATT_VOLT_REVISE_COUNT          (15)  //首次进行电池电压虚高虚低值计算的次数

/* 需要上报事件给应用时延时5秒休眠，保证应用能够收到事件*/
#define ALARM_REPORT_WAKELOCK_TIMEOUT (5000)
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define SHUTDOWN_WAKELOCK_TIMEOUT (1000 * 120)
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
#define EXTCHG_OVER_TEMP_STOP_THRESHOLD     (55)
#define EXTCHG_OVER_TEMP_RESUME_THRESHOLD   (51)
#if (defined(BSP_CONFIG_BOARD_E5785Lh_92a) || defined(BSP_CONFIG_BOARD_E5785Lh_22c) \
    || defined(BSP_CONFIG_BOARD_E5787Ph_92a) || defined(BSP_CONFIG_BOARD_E5787Ph_67a) \
    || defined(BSP_CONFIG_BOARD_E5785Lh_67a))
#define EXTCHG_OVER_TEMP_LIMIT_500MA        (50)
#endif
#define EXTCHG_LOW_VOLTAGE_RESUME_THRESHOLD (3550)
static boolean is_otg_extchg    = FALSE;
static boolean is_extchg_ovtemp = FALSE;
static boolean is_batt_volt_low = FALSE;
#if defined(BSP_CONFIG_BOARD_E5785Lh_92a)
static uint32 g_covert_to_otg_counter = 0;
#define OTG_OVP_DETECT_DELAY_CONTER    (5)
#endif
static int32_t g_extchg_revise_count = 0;
#define EXTCHG_BATT_VOLT_REVISE_LIMIT       (200)
#define EXTCHG_THRESHOLD_PATH   "/data/userdata/extchg_config/extchg_threshold"
#define EXTCHG_DISABLE_PATH     "/data/userdata/extchg_config/extchg_disable_st"
#define EXTCHG_DEFAULT_STOP_THRESHOLD       (5)
#define EXTCHG_STOP_CAPACITY_TEN            (10)
#define EXTCHG_STOP_CAPACITY_TWENTY         (20)
#define EXTCHG_STOP_CAPACITY_THIRTY         (30)
#define EXTCHG_STOP_CAPACITY_FORTY          (40)
#define EXTCHG_STOP_CAPACITY_FIFTY          (50)
#define EXTCHG_STOP_CAPACITY_SIXTY          (60)
#define EXTCHG_STOP_CAPACITY_SEVENTY        (70)
#define EXTCHG_STOP_CAPACITY_EIGHTY         (80)
#define EXTCHG_STOP_CAPACITY_NINETY         (90)
#define EXTCHG_STOP_CAPACITY_HUNDRED        (100)

#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3470)
#define EXTCHG_STOP_VOLTAGE_TEN             (3579)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3637)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3670)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3696)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3728)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3769)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3830)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (3902)
#define EXTCHG_STOP_VOLTAGE_NINETY          (3985)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4130)

/*停止对外充电门电池电量值默认为低电5%->3.55V*/
int32_t g_extchg_voltage_threshold = 0;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
int32_t g_extchg_stop_soc_threshold = 0;
#endif
/*是否永久停止对外充电标志1:永久停止 0:永久开启*/
int32_t g_extchg_diable_st = 0;
/*记录用户上次设置的永久停止对外充电标志1：永久停止，0：永久开启*/
static int32_t g_last_extchg_diable_st = 0;
#define EXTCHG_TEMP_COMPENSATE_VALUE        (4)
/*用户通过TOUCH UI选择的对外充电的模式，1:仅对外充电 2:对外充电加数据业务*/
int32_t g_ui_choose_exchg_mode = 0;
boolean g_exchg_enable_flag = FALSE;
/*对外充电USB ID线在位标志，用于对外充电和无线充电的互斥切换*/
boolean g_exchg_online_flag = OFFLINE;

/*the Macro of a-port ext-charge*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#define CHG_BAT_TEMP_REVISE_SAMPLE          (3)
#define EXTCHG_ILIM_1A_CAPACITY             (25)
#define EXTCHG_SHUTDOWN_CHG_STOP_THRESHOLD  (2920)
#define DELAY_TIME_OF_DEBOUNCE              (200)
#define TIME_OF_SHORT_DET                   (10)
#define NUM_OF_SHORT_DET_POLL               (3)

#define EXTCHG_OVER_HEAT_STOP_THRESHOLD     (60)
#define EXTCHG_OVER_HEAT_RESUME_THRESHOLD   (57)
#define EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD  (53)
#define EXTCHG_WARM_CHARGE_RESUME_THRESHOLD (50)
#define EXTCHG_HEAT_CHARGE_LIMIT_THRESHOLD  (57)
#define EXTCHG_HEAT_CHARGE_RESUME_THRESHOLD (54)
#define EXTCHG_LOW_TEMP_STOP_THRESHOLD      (0)
#define EXTCHG_LOW_TEMP_RESUME_THRESHOLD    (3)
#define EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD   (-20)
#define EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD (-17)

#define EXTCHG_TEMP_REVISE_CAPACITY_THIRTY           (30)
#define EXTCHG_TEMP_REVISE_CAPACITY_FIFTY            (50)
#define EXTCHG_TEMP_REVISE_CAPACITY_SEVENTYFIVE      (75)
#define EXTCHG_TEMP_REVIEW_CURRENT_2800MA            (-2800)
#define EXTCHG_TEMP_REVIEW_CURRENT_3000MA            (-3000)
#define EXTCHG_TEMP_REVIEW_CURRENT_3200MA            (-3200)

/*charge termimation voltage*/
#define CHG_TERMI_VOLT_FOR_COUL                      (4165)
/*charge termimation voltage in warm-charge*/
#define CHG_TERMI_WARMCHG_VOLT_FOR_COUL              (4065)
/*charge termimation current*/
#define CHG_TERMI_CURR_FOR_COUL                      (256)
#endif /*FEATURE_ON == MBB_CHG_APORT_EXTCHG*/
#endif /*MBB_CHG_EXTCHG == FEATURE_ON */

#if ((MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON))
/*下边函数不同平台实现方式不同，需要进行适配*/

extern void usb_notify_event(unsigned long val, void *v);
#endif/*(MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON)*/

#if ( FEATURE_ON == MBB_MLOG )
extern void mlog_set_statis_info(char *item_name,unsigned int item_value);
#endif

void load_ftm_mode_init(void)
{
    if(CHG_OK == chg_config_para_read(NV_FACTORY_MODE_I, (char *) &g_real_factory_mode, sizeof(uint32_t)))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:read factory mode ok,mode =  %d\n ", g_real_factory_mode);

        if(0 == g_real_factory_mode)
        {
            chg_current_ftm_mode = TRUE;
        }
        else
        {
            chg_current_ftm_mode = FALSE;
        }
    }
    else/*若NV读失败，则默认为升级版本*/
    {
#ifndef MBB_FACTORY_FEATURE
        chg_current_ftm_mode = FALSE;
#else
        chg_current_ftm_mode = TRUE;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM: In ftm mode now \n ");
#endif
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read factory mode nv fail \n ");
    }
}


void chg_batt_temp_init(void)
{
    if(CHG_OK != chg_config_para_read(NV_OVER_TEMP_SHUTOFF_PROTECT,&g_chgBattOverTempPeotect, \
    sizeof(CHG_BATTERY_OVER_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattOverTempPeotect.ulIsEnable =  SHUTOFF_OVER_TEMP_PROTECT_ENABLE_DEFAULT;
        g_chgBattOverTempPeotect.lCloseAdcThreshold = SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD_DEFAULT; /*power off temp +1 */
        g_chgBattOverTempPeotect.ulTempOverCount = SHUTOFF_OVER_TEMP_SHUTOFF_CHECK_TIMES_DEFAULT;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_LOW_TEMP_SHUTOFF_PROTECT,&g_chgBattLowTempPeotect,\
    sizeof(CHG_BATTERY_LOW_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattLowTempPeotect.ulIsEnable =  SHUTOFF_LOW_TEMP_PROTECT_ENABLE_DEFAULT;
        g_chgBattLowTempPeotect.lCloseAdcThreshold = SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD_DEFAULT;
        g_chgBattLowTempPeotect.ulTempLowCount = SHUTOFF_LOW_TEMP_SHUTOFF_CHECK_TIMES_DEFAULT;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_BATT_TEMP_PROTECT_I,&g_chgShutOffTempProtect,\
    sizeof(CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_BATT_TEMP_PROTECT_I read fail \n ");
        g_chgShutOffTempProtect.ulChargeIsEnable = CHG_TEMP_PROTECT_ENABLE_DEFAULT;                   //充电温保护使能
        g_chgShutOffTempProtect.overTempchgStopThreshold = CHG_OVER_TEMP_STOP_THRESHOLD_DEFAULT;          //充电高温保护门限
        g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold = CHG_WARM_CHARGE_ENTER_THRESHOLD_DEFAULT;   //高温充电入口电池温度门限
        g_chgShutOffTempProtect.lowTempChgStopThreshold = CHG_LOW_TEMP_STOP_THRESHOLD_DEFAULT;            //充电低温保护门限
        g_chgShutOffTempProtect.overTempChgResumeThreshold = CHG_OVER_TEMP_RESUME_THRESHOLD_DEFAULT;        //充电高温恢复温度门限
        g_chgShutOffTempProtect.lowTempChgResumeThreshold = CHG_LOW_TEMP_RESUME_THRESHOLD_DEFAULT;          //充电低温恢复温度门限
        g_chgShutOffTempProtect.chgTempProtectCheckTimes = CHG_TEMP_PROTECT_CHECK_TIMES_DEFAULT;           //充电停充轮询次数
        g_chgShutOffTempProtect.chgTempResumeCheckTimes = CHG_TEMP_RESUME_CHECK_TIMES_DEFAULT;           //充电复充轮询次数
        g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold = CHG_WARM_CHARGE_EXIT_THRESHOLD_DEFAULT;   //高温充电恢复到常温充电温度门限
        g_chgShutOffTempProtect.reserved2 = CHG_TEMP_PROTECT_RESERVED2_DEFAULT;                          //预留
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_BATT_TEMP_PROTECT_I read success \n ");
    }

    if (CHG_BATT_ID_DEF == g_chg_batt_data->batt_id)
    {
        /*非标准电池45度以上停止充电，小于42度时恢复充电*/
        g_chgShutOffTempProtect.overTempChgResumeThreshold = CHG_DEF_BATT_OVER_TEMP_RESUME_THRESHOLD;
        g_chgShutOffTempProtect.overTempchgStopThreshold = CHG_DEF_BATT_OVER_TEMP_STOP_THRESHOLD;
    }
}


void load_on_off_mode_parameter(void)
{
    int32_t ret_val = CHG_ERROR;
    memset( (void *)&g_real_powerup_mode_value, 0, sizeof(POWERUP_MODE_TYPE) );

    /* 从nv中读取硬件测试开机模式标志*/
    /* 如果读取失败不设置，设置默认值*/
    ret_val = chg_config_para_read(NV_POWERUP_MODE,&g_real_powerup_mode_value, \
        sizeof(POWERUP_MODE_TYPE) );
    if ( CHG_OK == ret_val )
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:\r\nno_battery_powerup_enable=%d,\r\nexception_poweroff_poweron_enable=%d,\
            \r\nlow_battery_poweroff_disable=%d,\r\n ",\
            g_real_powerup_mode_value.no_battery_powerup_enable,
            g_real_powerup_mode_value.exception_poweroff_poweron_enable,
            g_real_powerup_mode_value.low_battery_poweroff_disable);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read hw test powerup mode nv error, ret_val = %d \n ", ret_val, 0, 0);

        g_real_powerup_mode_value.no_battery_powerup_enable = FALSE;
        g_real_powerup_mode_value.exception_poweroff_poweron_enable = FALSE;
        g_real_powerup_mode_value.low_battery_poweroff_disable = FALSE;
    }
}


void chg_batt_volt_init(void)
{
    if(CHG_OK != chg_config_para_read(NV_BATT_VOLT_PROTECT_I,&g_chgBattVoltProtect,\
        sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_BATT_VOLT_PROTECT_I read fail \n ");

        g_chgBattVoltProtect.battVoltPowerOnThreshold = BATT_VOLT_POWER_ON_THR_DEFAULT;     //开机电压门限
        g_chgBattVoltProtect.battVoltPowerOffThreshold = BATT_VOLT_POWER_OFF_THR_DEFAULT;    //关机电压门限
        g_chgBattVoltProtect.battOverVoltProtectThreshold = BATT_CHG_OVER_VOLT_PROTECT_THR_DEFAULT; //平滑充电过压保护门限(平滑值)
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold = BATT_CHG_OVER_VOLT_PROTECT_ONE_THR_DEFAULT; //单次充电过压保护门限(单次值)
        g_chgBattVoltProtect.battChgTempMaintThreshold = BATT_CHG_TEMP_MAINT_THR_DEFAULT;    //超时停充判断门限
        g_chgBattVoltProtect.battChgRechargeThreshold = BATT_HIGH_TEMP_RECHARGE_THR_DEFAULT;     //高温充电二次复充门限
        g_chgBattVoltProtect.VbatLevelLow_MAX = BATT_VOLT_LEVELLOW_MAX_DEFAULT;             //低电上限门限
        g_chgBattVoltProtect.VbatLevel0_MAX = BATT_VOLT_LEVEL0_MAX_DEFAULT;               //0格电压上限门限
        g_chgBattVoltProtect.VbatLevel1_MAX = BATT_VOLT_LEVEL1_MAX_DEFAULT;               //1格电压上限门限
        g_chgBattVoltProtect.VbatLevel2_MAX = BATT_VOLT_LEVEL2_MAX_DEFAULT;               //2格电压上限门限
        g_chgBattVoltProtect.VbatLevel3_MAX = BATT_VOLT_LEVEL3_MAX_DEFAULT;               //3格电压上限门限
        g_chgBattVoltProtect.battInsertChargeThreshold = BATT_INSERT_CHARGE_THR_DEFAULT;    //插入充电器判断是否要充电的门限
        g_chgBattVoltProtect.battNormalTempRechergeThreshold = BATT_NORMAL_TEMP_RECHARGE_THR_DEFAULT; //常温充电二次复充门限
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_BATT_VOLT_PROTECT_I read success \n ");
    }
}

boolean chg_is_no_battery_powerup_enable(void)
{
#ifdef CHG_STUB
    g_real_powerup_mode_value.no_battery_powerup_enable = TRUE;
#endif/*CHG_STUB*/

#ifdef HUAWEI_USB_POWERON
    static_smem_vendor0 *smem_data = NULL;
    smem_data = (static_smem_vendor0 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0 , sizeof(static_smem_vendor0 ));
    if(NULL == smem_data)
    {
        pr_err("%s: SMEM_HUAWEI_ALLOC ERROR.\n",__func__);
    }
    if ( STATUS_POWER_ON_USB == smem_data->smem_huawei_poweroff_chg )
    {
        g_real_powerup_mode_value.no_battery_powerup_enable = TRUE;
    }
#endif  /*HUAWEI_USB_POWERON*/

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_no_battery_powerup_enable %d !\n",
                            g_real_powerup_mode_value.no_battery_powerup_enable);
    return g_real_powerup_mode_value.no_battery_powerup_enable;
}

boolean chg_is_low_battery_poweroff_disable(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_low_battery_poweroff_disable %d !\n",
                            g_real_powerup_mode_value.low_battery_poweroff_disable);
    return g_real_powerup_mode_value.low_battery_poweroff_disable;
}


boolean chg_is_exception_poweroff_poweron_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_exception_poweroff_poweron_mode %d !\n",
                            g_real_powerup_mode_value.exception_poweroff_poweron_enable);
    return g_real_powerup_mode_value.exception_poweroff_poweron_enable;
}


BATT_LEVEL_ENUM chg_get_batt_level(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_get_batt_level=%d !\n",chg_real_info.bat_volt_lvl);

    return chg_real_info.bat_volt_lvl;
}


boolean chg_is_ftm_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_ftm_mode=%d !\n",chg_current_ftm_mode);
    return chg_current_ftm_mode;
}


void chg_batt_error_handle(void)
{
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_batt_error_handle->chg_set_power_off\n");
#if (FEATURE_ON == MBB_CHG_COULOMETER)
    /*通知内置库仑计模块电池移除后进行相应处理*/
    hisi_battery_removal_handle();
#endif
    chg_set_power_off(DRV_SHUTDOWN_BATTERY_ERROR);
    return;
}


boolean chg_get_batt_id_valid(void)
{
    int32_t batt_temp = 0;
    boolean chg_ftm_mode = FALSE;
    boolean chg_no_battery_powerup_mode = FALSE;

    /* 工厂模式下一直认为电池为合法电池**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_ftm_mode=%d ,no_battery_powerup_mode=%d!\n",
                            chg_ftm_mode,chg_no_battery_powerup_mode);

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return TRUE;
    }

    /* 使用电池温度判断目前电池是否为拔除**/
    batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >= batt_temp)
    {
        batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
        if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >=  batt_temp)
        {
            batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
            if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >= batt_temp)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_temp below 30 degree bat not present!\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean chg_is_emergency_state(void)
{
    if((chg_batt_high_temp_58_flag == TRUE) || (chg_batt_low_battery_flag == TRUE))
    {
        chg_batt_condition_error_flag = TRUE;
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:is_emergency_state!\n");
    }
    else
    {
        chg_batt_condition_error_flag = FALSE;
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:is not emergency state!\n");
    }

    return chg_batt_condition_error_flag;
}

void chg_batt_error_detect_temp(void)
{
    if(FALSE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:batt invalid system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery ts error detected.\n");

        chg_batt_error_handle();
    }
    return;
}

void chg_batt_error_detect_volt(void)
{
    int32_t  batt_volt = 0;
    boolean  batt_err_flag = FALSE;
    boolean  chg_ftm_mode = FALSE;
    boolean  chg_no_battery_powerup_mode = FALSE;

    /* 工厂模式下一直认为电池为合法电池**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return;
    }

    /* 查询电池电压,如果电池电压低于规定值,判定电池损坏*/
    batt_volt = chg_get_batt_volt_value();
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
    if (g_chg_hardpara_dtsi.batt_bad_volt > batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();

        if (g_chg_hardpara_dtsi.batt_bad_volt > batt_volt)
        {
            batt_volt = chg_get_batt_volt_value();
            if (g_chg_hardpara_dtsi.batt_bad_volt > batt_volt)
            {
                batt_err_flag = TRUE;
            }
        }
    }
#else
    if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();

        if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
        {
            batt_volt = chg_get_batt_volt_value();
            if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
            {
                batt_err_flag = TRUE;
            }
        }
    }
#endif

    if(TRUE == batt_err_flag)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt shorted system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery short error detected.\n");
        chg_batt_error_handle();
    }

    return;
}

static int32_t chg_calc_average_temp_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_temp_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static int32_t record_avg_num_fast = 0;
    static int32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = {0};

    /*查询当前轮询模式*/
    new_poll_mode = chg_poll_timer_get();
    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*由慢轮询切换到快轮询*/
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_temp;
            }
            record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_temp;
            }
            record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:new_poll_mode==old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)/*快轮询模式*/
    {
        /*数组中当前元素标号小于30*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于30个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else/*慢轮询模式*/
    {
        /*数组中当前元素标号小于5*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于5个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
            }
        }
    }

    return bat_temp_avg;
}

int32_t chg_huawei_set_temp(int32_t temp)
{
    return temp;
}

void chg_temp_is_too_hot_or_too_cold_for_chg ( void )
{
    static uint32_t up_over_temp_flag = FALSE;
    static uint32_t low_over_temp_flag = FALSE;

    /*充电温保护使能关闭*/
    if(0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_temp_protect_flag = FALSE;
        return;
    }
    else
    {
         chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is enable!\n");
    }

    /*高温检测:充电器不在位时不对充电温度做异常判断*/
    if (FALSE == chg_is_charger_present())
    {
        up_over_temp_flag = FALSE;
    }
    else if(FALSE == up_over_temp_flag)
    {   /*1、带高温充电功能产品通过NV50385将电池温度门限设置为55度
          2、不带高温充电功能产品通过NV50385将电池温度门限设置为45度*/
        if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            up_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't up_over_temp!\n");
        }
    }
    else
    {
          /*1、带高温充电功能产品通过NV50385将电池温度门限设置为52度
          2、不带高温充电功能产品通过NV50385将电池温度门限设置为42度*/
        if(CHG_OVER_TEMP_RESUME_THRESHOLD > chg_real_info.battery_temp)
        {
            up_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery up temp don't resume nomal!\n");
        }
    }

    /*低温检测:充电器不在位时不对充电温度做异常判断*/
    if (FALSE == chg_is_charger_present())
    {
        low_over_temp_flag = FALSE;
    }
    else if(FALSE == low_over_temp_flag)
    {
        /*归一化规格电池温度小于0度低温停充*/
        if(CHG_LOW_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp)
        {
            low_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't low_over_temp!\n");
        }
    }
    else
    {
        /*归一化规格电池温度大于等于3度恢复常温充电*/
        if(CHG_LOW_TEMP_RESUME_THRESHOLD <= chg_real_info.battery_temp)
        {
            low_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery low temp don't resume nomal!\n");
        }
    }

    /*根据高低温检测结果判断充电温保护状态*/
    if((up_over_temp_flag == TRUE) || (low_over_temp_flag == TRUE))
    {
        chg_temp_protect_flag = TRUE;
    }
    else
    {
        chg_temp_protect_flag = FALSE;
    }
}

void chg_set_supply_current_by_temp(void)
{
    uint8_t need_resume_supply = 0;
    uint8_t curr_limit = 0;

    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_set_supply_current_by_temp check begin!\n");
    /*支持可维可测*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable()
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        return;
    }

    if( TRUE == chg_is_powdown_charging())/*关机充电模式*/
    {
        return;
    }

    if(TRUE == is_chg_charger_removed())
    {
        if(TRUE == chg_limit_supply_current_flag)
        {
            chg_limit_supply_current_flag = FALSE;
        }
        return;
    }

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    if((chg_real_info.battery_temp >= LIMIT_SUPPLY_CURR_TEMP)
        && (FALSE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = TRUE;
    }
    else if((chg_real_info.battery_temp <= LIMIT_SUPPLY_CURR_TEMP - LIMIT_SUPPLY_CURR__RESUME_LEN)
        && (TRUE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = FALSE;
        need_resume_supply = 1;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current in high temp!\n");
    }

    if(TRUE == chg_limit_supply_current_flag)
    {
        curr_limit = chg_get_supply_limit();
#if (MBB_CHG_BQ24196 == FEATURE_ON)
        if(BQ24192_IINLIMIT_100 != curr_limit)
        {
            chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set supply current in high temp!\n");
        }
#elif (MBB_CHG_SMB1351 == FEATURE_ON)
        if(SMB1351_DCIN_CUR_LIMIT_ST_500MA != curr_limit)
        {
            chg_set_supply_limit(CHG_IINPUT_LIMIT_500MA);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set supply current in high temp!\n");
        }

#endif/*MBB_CHG_SMB1351 == FEATURE_ON*/
    }
    else
    {
        if(need_resume_supply)
        {
            chg_set_supply_limit_by_stm_stat();
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_set_supply_current_by_temp check finish!\n");
}


boolean chg_temp_protect_eixt_suspend_mode(void)
{
    boolean ret = TRUE;
    /*是否满足退出suspend条件*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_temp_protect_eixt_suspend_mode chg_battery_protect_flag is %d!\r\n", chg_battery_protect_flag);
#endif

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_usb_temp_info.usb_temp_protect_cur_stat chg_battery_protect_flag is %d!\r\n",\
        chg_usb_temp_info.usb_temp_protect_cur_stat);
#endif

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT ) && (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if ( (FALSE == chg_battery_protect_flag) && (FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat) )
    {
        ret = chg_set_suspend_mode(FALSE);
    }
#else
    ret = chg_set_suspend_mode(FALSE);
#endif

    return ret;
}

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

static void chg_usb_over_temp_protect_enter_process(void)
{
    /*1.设置充电芯片进入suspend。2.更新POWER_SUPPLY节点。3.mlog。*/
    if ( TRUE != chg_set_suspend_mode(TRUE) )
    {
        chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend fail in usb high temp!\n");
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Set charge IC to suspend fail in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
        return;
    }
    chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
    chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_DEAD;
    chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_HIGH);
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend in usb high temp!\n");
#if ( FEATURE_ON == MBB_MLOG )
    printk(KERN_ERR "CHG_STM:USBPortOverTempCnt\n");
    mlog_set_statis_info("USBPortOverTempCnt",1);/*USB温保前段限流次数+1*/

    mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Entry usb temp protect in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif/*MBB_MLOG*/
    return;
}


static void chg_usb_over_temp_protect_exit_process(void)
{
    /*1.设置充电芯片退出suspend*/
    if ( TRUE != chg_temp_protect_eixt_suspend_mode() )
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend is fail in usb high temp!\n");
#if (FEATURE_ON == MBB_MLOG)
        mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Set charge IC to suspend is fail in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
        return;
    }

    /*1.根据当前状态恢复输入电流。2.更新POWER_SEPPLY节点。3.mlog。*/
    chg_set_supply_limit_by_stm_stat();
    chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
    chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_GOOD;
    chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_NORMAL);
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Resume charge IC from suspend in usb high temp!\n");
#if (FEATURE_ON == MBB_MLOG)
    mlog_print(MLOG_CHG, mlog_lv_error, "CHG_STM: Resume usb temp protect in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
    return;
}


void chg_usb_temp_protect_proc(void)
{
    int32_t         usb_temp = TEMP_INITIAL_VALUE;
    int32_t         usb_temp_detect_count_t = USB_TEMP_DETECT_COUNT;

    /*获取当前温度值*/
    usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    chg_usb_temp_info.usb_cur_temp = usb_temp;

    /*充电温保护使能关闭*/
    if(0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        return;
    }

    /*判断三次温度值是否符合触发和恢复门限，并做相应处理.*/
    if( FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat)
    {
        while(--usb_temp_detect_count_t)
        {
            if( CHG_USB_TEMP_LIMIT > usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = TRUE;
        }
    }
    else
    {
        while(--usb_temp_detect_count_t)
        {
            if( CHG_USB_TEMP_RESUME < usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        }
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_cur_temp=%d\n",chg_usb_temp_info.usb_cur_temp);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_temp_protect_stat=%d\n",
            chg_usb_temp_info.usb_temp_protect_cur_stat);
    /*判断当前所处状态,并作相应处理*/
    if( chg_usb_temp_info.usb_temp_protect_cur_stat != chg_usb_temp_info.usb_temp_protect_pre_stat )
    {
        if( TRUE == chg_usb_temp_info.usb_temp_protect_cur_stat )
        {
            chg_usb_over_temp_protect_enter_process();
        }
        else
        {
            chg_usb_over_temp_protect_exit_process();
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Set usb port stat by temp finish!\n");
}


int32_t chg_get_usb_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.usb_heath_type;
}

int32_t chg_get_usb_cur_temp(void)
{
    if( TEMP_INITIAL_VALUE == chg_usb_temp_info.usb_cur_temp )
    {
        chg_usb_temp_info.usb_cur_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    }
    return chg_usb_temp_info.usb_cur_temp;
}

boolean chg_get_usb_temp_protect_stat(void)
{
    return chg_usb_temp_info.usb_temp_protect_cur_stat;
}
#endif

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)

static void chg_battery_protect_enter_process(void)
{
    /*设置charger 24196 进入suspend模式*/
    if (FALSE == chg_get_suspend_status())
    {
        (void)chg_set_suspend_mode(TRUE);
    }
}


static void chg_battery_protect_exit_process(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    chg_stm_state_type curr_state = chg_stm_get_cur_state();

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if ((curr_state <= CHG_STM_INIT_ST) || (curr_state >= CHG_STM_MAX_ST))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    cur_chgr_type = chg_stm_get_chgr_type();
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:battery protect resume!\n");
    switch(cur_chgr_type)
    {
        case CHG_WALL_CHGR:
        {
            chg_set_hardware_parameter(&chg_std_chgr_hw_paras[curr_state]);
            break;
        }
        case CHG_USB_HOST_PC:
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[curr_state]);
            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        {
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[curr_state]);
            break;
        }
        default:
        {
            break;
        }
    }
    /*退出suspend模式*/
    if (TRUE == chg_get_suspend_status())
    {
        chg_temp_protect_eixt_suspend_mode();
    }
}


uint32_t chg_stm_get_no_charging_charger_lasted_time(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:charger_lasted_without_charging_in_seconds=%u\n",\
                    chg_stm_state_info.charger_lasted_without_charging_in_seconds);
    /*1. Return the current time.*/
    return chg_stm_state_info.charger_lasted_without_charging_in_seconds;
}


static void chg_poll_volt_temp_protect_state(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();

    /*电池膨胀保护TBAT>45°and VBAT >=4.1V 状态轮询*/
    if ((FALSE == g_chg_over_temp_volt_protect_flag)
        && (chg_get_sys_batt_temp () > CHG_BATTERY_PROTECT_TEMP)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_over_temp_volt_protect_flag = TRUE;
        /*高温高压触发电池膨胀保护+1*/
        mlog_set_statis_info("BattExpandOverTempVoltCnt",1);
    }
    else if ((TRUE == g_chg_over_temp_volt_protect_flag)
            && ((chg_get_sys_batt_temp () < CHG_BATTERY_PROTECT_RESUME_TEMP)
            || (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE)))
    {
        g_chg_over_temp_volt_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
}


void set_long_time_no_charge_protect_recharge_volt(void)
{
    /*长时间电源在位不充电进入保护后，修改常温复充门限为4.0v*/
    g_chgBattVoltProtect.battNormalTempRechergeThreshold = BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    g_chg_battery_expand_change_normal_rechg_flag = TRUE;
#endif
}


void resume_long_time_no_charge_protect_recharge_volt(void)
{
    /*电源拔出后，恢复常温复充门限为4.2v*/
    g_chgBattVoltProtect.battNormalTempRechergeThreshold = g_batt_normal_temp_recherge_threshold;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    /*电源拔出后，恢复常温复充门限*/
    g_chg_battery_expand_change_normal_rechg_flag = FALSE;
#endif
}


static void chg_poll_long_time_no_charge_state(void)
{
    uint32_t charger_lasted_time = 0;
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();
    /*停充且充电器在位的时间*/
    charger_lasted_time = chg_stm_get_no_charging_charger_lasted_time();

    /*电池膨胀保护充电器在位持续>=16H且不充电状态轮询*/
    if ((FALSE == g_chg_longtime_nocharge_protect_flag)
        && (charger_lasted_time >= CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = TRUE;
        /*长时间电源在位不充电进入保护后，修改常温复充门限为4.0v*/
        (void)set_long_time_no_charge_protect_recharge_volt();
        /*USB长时间在位触发电池膨胀保护+1*/
        mlog_set_statis_info("BattExpandLongTimeCnt",1);
    }
    else if ((TRUE == g_chg_longtime_nocharge_protect_flag)
            && (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
}


static void chg_battery_protect_proc(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    chg_stm_state_type curr_state = CHG_STM_INIT_ST;
    cur_chgr_type = chg_stm_get_chgr_type();
    curr_state = chg_stm_get_cur_state();
    /*支持可维可测*/
    if ( (TRUE == chg_is_ftm_mode() ) \
        || (TRUE == chg_is_no_battery_powerup_enable() ) \
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In FTM mode do not enable battery protect!\n");
        return;
    }

    /*电源拔出后，退出电池膨胀保护*/
    if (TRUE == is_chg_charger_removed())
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            g_chg_over_temp_volt_protect_flag = FALSE;
            g_chg_longtime_nocharge_protect_flag = FALSE;
            (void)chg_battery_protect_exit_process();
        }
        (void)resume_long_time_no_charge_protect_recharge_volt();
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charger remove disenable battery protect!\n");
        return;
    }

    if (FALSE == chg_battery_protect_flag)
    {
        /*满电停充后才进入Suspend/输入限流100mA*/
        if ( (curr_state != CHG_STM_MAINT_ST) || (BATT_CAPACITY_FULL != chg_real_info.bat_capacity) )
        {
                chg_print_level_message(CHG_MSG_INFO, 
                    "CHG_STM:battery is not in full state, no need to protect!\n");
                return;
        }
    }

    /*轮询保护状态*/
    (void)chg_poll_volt_temp_protect_state();
    (void)chg_poll_long_time_no_charge_state();

    /*两种保护有一个触发将进入保护*/
    if ( (TRUE == g_chg_over_temp_volt_protect_flag)
        || (TRUE == g_chg_longtime_nocharge_protect_flag) )
    {
        if (FALSE == chg_battery_protect_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:meet battery protect condition do battery protect!\n");
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_over_temp_volt_protect_flag is %d!\r\n",
                g_chg_over_temp_volt_protect_flag);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_longtime_nocharge_protect_flag is %d!\r\n",
                g_chg_longtime_nocharge_protect_flag);
            chg_battery_protect_enter_process();
            chg_battery_protect_flag = TRUE;
            /*进入电池膨胀保护+1*/
            mlog_set_statis_info("BattExpandProtectCnt",1);
            mlog_print(MLOG_CHG, mlog_lv_error,
                    "CHG_STM: Entry battery expand protect by OverTempVolt:%d LongTime %d.\n",
                            g_chg_over_temp_volt_protect_flag,g_chg_longtime_nocharge_protect_flag);
        }
    }
    else
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            chg_battery_protect_exit_process();
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:To resume from battery expand protect.\n");
            mlog_print(MLOG_CHG, mlog_lv_error,"CHG_STM: To resume from battery expand protect.\n");
        }
    }
}
#endif

void chg_temp_is_too_hot_or_too_cold_for_shutoff(void)
{
    static uint32_t countNum = 0;
    static uint32_t high_temp_58_flag = FALSE;
    static uint32_t high_timer_flag = FALSE;
    static uint32_t low_timer_flag = FALSE;
    static uint32_t up_over_temp_shutoff_falg = FALSE;
    static uint32_t low_over_temp_shutoff_falg = FALSE;

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_is_too_hot_or_too_cold_for_shutoff check begin!\n");
    /*烧片版本不做处理*/
    /*可维可测:无电池开机使能*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }

    /*当单板在开机状态下，温度高于等于(关机温度-2度)时上报给应用，告警提示*/
    if( FALSE == chg_is_powdown_charging())/*开机模式*/
    {
        if(chg_real_info.battery_temp >= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN))
        {
            if(FALSE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = TRUE;
                /*切换轮询方式，高温进入快轮询模式*/
                chg_poll_timer_set(FAST_POLL_CYCLE);
                high_temp_58_flag = TRUE;
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT over 58 degree!\n");
            }
            /*上报APP，高温告警提示，上报5次，避免开机即高温，APP启动较慢收不到消息*/
            if(BATTERY_EVENT_REPORT_TIMES >= countNum)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_HIGH \n ");

                chg_batt_temp_state = TEMP_BATT_HIGH;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery over-heated WARNING!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH event report below 5 times!\n");
            }

            countNum++;
        }
        /*当单板在开机状态下，温度低于等于(关机温度-5度)时上报给应用，取消告警*/
        else if((chg_real_info.battery_temp <= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_RESUME_LEN)))
        {
            if(TRUE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_NORMAL \n ");
                chg_batt_temp_state = TEMP_BATT_NORMAL;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
                high_temp_58_flag = FALSE;
                countNum = 0;
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH!\n");
        }
    }
    else
    {
        /*关机状态不做处理*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System in power down chging do nothing!\n");
    }


    /*温度关机检测------高温检测*/
    if(SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_OVER_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*开机模式*/
            {
                up_over_temp_shutoff_falg = TRUE;

                /*todo 上报APP执行高温关机操作，以按键关机形式上报*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for high temp power off  \n ");
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery Over-heated, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == high_timer_flag)
                {
                    high_timer_flag = TRUE;

                    /*启动定时器，回调函数为关机函数，定时时长为45秒；*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for high temp power off \n ");
                    g_timeout_to_poweroff_reason = DRV_SHUTDOWN_TEMPERATURE_PROTECT;
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Already start power off timer!\n");
                }
            }
            else //关机模式
            {
                /*关机模式下的温度高于关机门限，不做处理*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:high temp but power down chging do nothing!\n");
            }
        }
        else
        {
             chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable high temp protect!\n");
        }
    }

    /*温度关机检测------低温检测*/
    if(SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD >= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*开机模式*/
            {
                low_over_temp_shutoff_falg = TRUE;
                /*上报APP执行低温关机操作*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low temp power off  \n ");

                chg_batt_temp_state = TEMP_BATT_LOW;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery too Cold, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == low_timer_flag )
                {
                    low_timer_flag = TRUE;
                    /*起定时器计时25秒，当45秒时间到后APP仍未关机，则底层自己执行关机操作；*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for low temp power off \n ");
                    g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_TEMP_PROTECT;
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);

                }
            }
            else
            {
                /*关机模式下的温度低于关机门限，不做处理*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low temp but power down chging do nothing!\n");
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable low temp protect!\n");
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_is_too_hot_or_too_cold_for_shutoff check over!\n");
}


void chg_poll_batt_temp(void)
{
    int32_t new_one_batt_temp = 0;
    int32_t new_batt_temp = 0;
    static uint32_t init_flag = FALSE;

    /*坏电池检测处理函数*/
    chg_batt_error_detect_temp();

    /*调用参数获取接口函数获取电压值*/
    new_one_batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    /*cause temperature collection from battery NTC has a large deviation
    when current is high, the temperature should revise*/
    new_one_batt_temp = chg_batt_temp_revise(new_one_batt_temp);
#endif

    /*保存单次采集的温度值到全局变量中*/
    chg_real_info.battery_one_temp = new_one_batt_temp;

    /*电池平滑温度初始化为首次采集值*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_temp = chg_real_info.battery_one_temp;
        init_flag = TRUE;
    }
    /*调用温度补偿函数对单次采集的温度进行补偿，不同平台对温度补偿要求不同
      有的要求补偿平滑后的温度，有的要求补偿平滑前的温度，9X25对外充电对单次
      电池温度进行补偿，因此删除下边的平滑温度补偿*/
    new_one_batt_temp = chg_huawei_set_temp(new_one_batt_temp);

    /*调用平滑算法对当前电池温度进行平滑运算*/
    new_batt_temp = chg_calc_average_temp_value(new_one_batt_temp);

    /*调用温度补偿函数进行对平滑后的温度进行补偿*/
    //new_batt_temp = chg_huawei_set_temp(new_batt_temp);

    /*保存最终获取的温度值到全局变量中*/
    chg_real_info.battery_temp = new_batt_temp;

    /*调用充电温保护的温度检测函数执行*/
    chg_temp_is_too_hot_or_too_cold_for_chg ( );

    /* log打印接口，可维可测讨论内容*/

    chg_print_level_message(CHG_MSG_DEBUG, "**********CHG_STM: chg_poll_batt_temp  begin *********\n");

    /*打印充电温保护使能开关*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*高、低温温保护充电停充门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_thr = %d,chg_low_thr = %d\n", \
        CHG_OVER_TEMP_STOP_THRESHOLD, CHG_LOW_TEMP_STOP_THRESHOLD);

    /*高、低温温保护充电复充门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:rechg_high_thr = %d,rechg_low_thr = %d\n", \
        CHG_OVER_TEMP_RESUME_THRESHOLD, CHG_LOW_TEMP_RESUME_THRESHOLD);

    /*打印高/低温保护关机使能开关；*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff over temp protect enable flag = %d\n",
                            SHUTOFF_OVER_TEMP_PROTECT_ENABLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff low  temp protect enable flag = %d\n",
                            SHUTOFF_LOW_TEMP_PROTECT_ENABLE);

    /*高、低温温保护关机门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_high_thr = %d,shutoff_low_thr = %d\n", \
        SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD, SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD);

    /*打印单次电池温度和处理后的电池温度*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_temp = %d\n", chg_real_info.battery_one_temp);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_temp = %d\n", chg_real_info.battery_temp);

    /*打印当前的温保护状态chg_temp_protect_flag的值*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_protect_flag = %d\n", chg_temp_protect_flag);

    /*调用温度关机条件检测执行*/
    chg_temp_is_too_hot_or_too_cold_for_shutoff( );

#if (MBB_CHG_CURRENT_SUPPLY_LIMIT == FEATURE_ON)
    chg_set_supply_current_by_temp();
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT == FEATURE_ON*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    /*电池膨胀保护处理*/
    chg_battery_protect_proc();
#endif
    /*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_usb_temp_protect_proc();
#endif
    chg_print_level_message(CHG_MSG_INFO, "**********CHG_STM: chg_poll_batt_temp  end  *********\n");
}

int32_t chg_calc_average_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_volt;
            }
            record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_volt;
            }
            record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_calc_average_volt_value new_poll_mode=old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于30*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于30个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于5*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于5个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
            }
        }

    }

    return bat_volt_avg;
}


int32_t chg_calc_average_vph_pwr_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    vph_pwr_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_volt;
            }
            record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_volt;
            }
            record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_calc_average_vph_pwr_volt_value new_poll_mode=old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于30*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            vph_pwr_volt_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于30个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            vph_pwr_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于5*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            vph_pwr_volt_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于5个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            vph_pwr_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
            }
        }

    }

    return vph_pwr_volt_avg;
}

void chg_volt_level_to_capacity(BATT_LEVEL_ENUM bat_volt_level)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
    switch(bat_volt_level)
    {
        case BATT_LOW_POWER:
        {
            /*未充电场景才允许将电量设置为低电低电电量，防止上报低电电量后应用在充电场景进行低电提示*/
            if(FALSE == chg_get_charging_status())
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
            }
            break;
        }
        case BATT_LEVEL_1:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL1);
            break;
        }
        case BATT_LEVEL_2:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL2);
            break;
        }
        case BATT_LEVEL_3:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL3);
            break;
        }
        case BATT_LEVEL_4:
        {
            /*满电停充设置电池电量为100*/
            if(CHG_STM_MAINT_ST == cur_stat)
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity-->CHG_STM_MAINT_ST!\n ");
            }
            else
            {
                /*非满电停充但电池电压大于4.1V也设置电池电量为100*/
                if(chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR)
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity VBAT>=4.1V!\n ");
                }
                else
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL4);
                }
            }

            break;
        }

        default:
            break;
    }

}

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

BATT_LEVEL_ENUM chg_soc2level(int soc)
{
    BATT_LEVEL_ENUM volt_level;

    if ( (soc > BATT_CAPACITY_FULL) || (soc < 0) )
    {
        chg_print_level_message(CHG_MSG_ERR,\
            "chg_percent2level param %d error!! not in [0 ~ 100]\n ", soc);
    }

    if (soc < BATT_CAPACITY_LEVEL1)
    {
        volt_level = BATT_LOW_POWER;
    }
    else if (soc < BATT_CAPACITY_LEVEL2)
    {
        volt_level = BATT_LEVEL_1;
    }
    else if (soc < BATT_CAPACITY_LEVEL3)
    {
        volt_level = BATT_LEVEL_2;
    }
    else if (soc < BATT_CAPACITY_LEVEL4)
    {
        volt_level = BATT_LEVEL_3;
    }
    else
    {
        volt_level = BATT_LEVEL_4;
    }

    return volt_level;
}

void chg_set_battery_level(void)
{
    int soc = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    BATT_LEVEL_ENUM pre_bat_level = BATT_LEVEL_MAX;
    soc = chg_get_sys_batt_capacity();
    bat_volt_level = chg_soc2level(soc);
    pre_bat_level = chg_real_info.bat_volt_lvl;

   /*非充电状态，不允许电池电压反转*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state volt_lvl decline to %d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,\
        "CHG_STM:chargin state volt_lvl increase to %d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
    }

    if(BATT_LOW_POWER == chg_real_info.bat_volt_lvl)
    {
        chg_batt_low_battery_flag = TRUE;
        if(FAST_POLL_CYCLE != chg_poll_timer_get())
        {
            chg_poll_timer_set(FAST_POLL_CYCLE);
        }
    }

    if(pre_bat_level != chg_real_info.bat_volt_lvl)
    {
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,\
                            (uint32_t)CHG_EVENT_NONEED_CARE);
    }

}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

void chg_set_battery_volt_level(void)
{
    static uint32_t count = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    int32_t batt_volt = chg_real_info.battery_volt;

    if(batt_volt < BATT_VOLT_LEVELLOW_MAX)
    {

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /*对外充电器在位且用户选择了仅对外充电或者对外充电加数据业务则不上报低电*/
        if(g_ui_choose_exchg_mode <= 0)
        {
            bat_volt_level = BATT_LOW_POWER;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:EXTCHG do not report Lowpower!\n");
        }
#else
        bat_volt_level = BATT_LOW_POWER;
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/
    }

    else if(batt_volt < BATT_VOLT_LEVEL1_MAX)
    {
        bat_volt_level = BATT_LEVEL_1;
    }
    else if(batt_volt < BATT_VOLT_LEVEL2_MAX)
    {
        bat_volt_level = BATT_LEVEL_2;
    }
    else if(batt_volt < BATT_VOLT_LEVEL3_MAX)
    {
        bat_volt_level = BATT_LEVEL_3;
    }
    else
    {
        bat_volt_level = BATT_LEVEL_4;
    }

   /*非充电状态，不允许电池电压反转*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_volt_level_to_capacity(bat_volt_level);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state volt_lvl decline to LEVEL_%d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chargin state volt_lvl increase to LEVEL_%d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
        chg_volt_level_to_capacity(bat_volt_level);
    }

    /*低电非充电,且非关机充电状态下，上报APP低电事件*/
    if((BATT_LOW_POWER == chg_real_info.bat_volt_lvl) \
       && (FALSE == chg_is_powdown_charging()) \
       && (FALSE == chg_get_charging_status()))
    {
        /* 上报5次 */
        if(BATTERY_EVENT_REPORT_TIMES > count)
        {
            /*调用接口函数上报APP低电事件*/
            chg_batt_low_battery_flag = TRUE;
            chg_poll_timer_set(FAST_POLL_CYCLE);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for show low power \n ");
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWER);
            count++;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low power event report less than 5 times!\n");
        }
    }
    else
    {
        count = 0;
    }
}


void chg_detect_batt_volt_for_shutoff(void)
{
    static uint32_t timer_flag = FALSE;

    /*烧片版本不做处理*/
    /*可维可测:低电关机/无电池开机使能*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_low_battery_poweroff_disable() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }
    /*电池低电关机检测*/
    if(BATT_VOLT_POWER_OFF_THR > chg_real_info.battery_volt)
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:PowerOffThreshold=%d\n",BATT_VOLT_POWER_OFF_THR);
        if( FALSE == chg_is_powdown_charging() )/*开机模式*/
        {
            /*todo 上报APP执行低电关机操作*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low battery power off  \n ");
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
            /*对于支持库仑计的产品，需要根据电压门限来关机，同时需要coul的soc值平滑到2%*/
            if((FALSE == timer_flag) && (BATT_CAPACITY_SHUTOFF == chg_get_sys_batt_capacity()))
            {
                timer_flag = TRUE;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
                /*启动定时器，回调函数为关机函数，定时时长为45秒；*/
                chg_print_level_message(CHG_MSG_ERR, \
                "CHG_STM:begin 45s timer for low battery power off \n ");
                g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_BATTERY;
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, \
                chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
            }
#else

            /*开机模式电池电压低于3.45V给应用上报电量为0，关机充电模式不上报防止反复开关机*/
            chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
#ifdef  MBB_MLOG
            mlog_print(MLOG_CHG, mlog_lv_warn, "Battery volt too low, report system down message!!!\n");
#endif/*MBB_MLOG*/
            if(FALSE == timer_flag)
            {
                timer_flag = TRUE;
                /*启动定时器，回调函数为关机函数，定时时长为45秒；*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for low battery power off \n ");
#ifdef  MBB_MLOG
                mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:begin 45s timer for low battery power off!!!\n");
#endif/*MBB_MLOG*/
                g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_BATTERY;
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, \
                chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
            }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else/*关机模式*/
        {
            /*关机模式下的电压低于关机门限，不做处理，本身为关机状态*/
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:power off voltage but in powdown_charging do nothing!\n");
        }
    }
}


void chg_detect_batt_chg_for_shutoff(void)
{
    static int32_t charge_remove_check_count = 0;

    if(TRUE == chg_is_powdown_charging())
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        /*有无线充电功能时，关机充电如果连续两次检测到无线充电器和无线充电器均不在位则关机*/
        if((FALSE == chg_is_charger_present()) && (FALSE == chg_stm_get_wireless_online_st()))
#elif (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
        /*add the detection of ext-charge when shutoff*/
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
        if ((FALSE == chg_is_charger_present()) && (EXTCHG_OFFLINE == extchg_is_extcharge_device_on()) \
            && (TRUE == g_extchg_shutoff_flag))
#else
        if ((FALSE == chg_is_charger_present()) && (EXTCHG_OFFLINE == extchg_is_extcharge_device_on()))
#endif
#else
        /*无无线充电功能时，关机充电如果连续两次检测到无线充电和无线充电器均不在位则关机*/
        if(FALSE == chg_is_charger_present())
#endif/*MBB_CHG_WIRELESS*/
        {
            if(CHARGE_REMOVE_CHECK_MAX <= charge_remove_check_count)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:POWER OFF FOR CHARGER REMOVE !\n ");
                chg_set_power_off(DRV_SHUTDOWN_CHARGE_REMOVE);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO,"CHG_STM:charge_remove_check_count = %d\n ",charge_remove_check_count);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
                /*延时1s后再次检测充电器是否在位*/
                chg_delay_ms(EXTCHG_DETECT_DELAY_TIME);
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
                if ((FALSE == chg_is_charger_present()) && (EXTCHG_OFFLINE == extchg_is_extcharge_device_on()) \
                    && (TRUE == g_extchg_shutoff_flag))
#else
                if ((FALSE == chg_is_charger_present()) && (EXTCHG_OFFLINE == extchg_is_extcharge_device_on()))
#endif
                {
                    charge_remove_check_count++;
                }
#else
                charge_remove_check_count++;
#endif
            }
        }
        else
        {
            charge_remove_check_count = 0;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM: power on chging do noting!\n");
    }
}


void chg_set_revise_value(int32_t value)
{
    chg_real_info.volt_revise_value = value;
}

int32_t chg_get_revise_value(void)
{
    return chg_real_info.volt_revise_value;
}

#if (MBB_CHG_EXTCHG == FEATURE_ON)

int32_t chg_get_extchg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }

    /*放电电补偿上限*/
    if(chg_get_revise_value() < EXTCHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = chg_get_revise_value();
    }
    else
    {
        revise_val = EXTCHG_BATT_VOLT_REVISE_LIMIT;
    }

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg revise_val = %d\n", revise_val);
    return revise_val;
}

boolean is_need_extchg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    /*电池端口电压低于3.35v则不再补偿*/
    if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt=%d below 3.35V no need revise!\n", batt_volt);
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_extchg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start ext revise!\n",
                    (batt_volt_average - batt_volt));
                tmp_volt_revise_val = batt_volt_average - batt_volt;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_extchg_revise_count++ ;
    }
    else
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            /*对外充电线在位但是没有外接被充电设备虚低消除*/
            if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,stop ext revise!\n",
                        (batt_volt - batt_volt_average));
                    g_extchg_revise_count = 0;
                    is_need_revise = FALSE;
                }
            }
            /*虚低更新*/
            else
            {
                int32_t temp_revise_val = chg_get_revise_value();
                if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    batt_volt = chg_get_batt_volt_value();
                    if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                    {
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:extchg batt volt change=%d,dynamic ext revise!\n",
                            (batt_volt_average - batt_volt));
                        tmp_volt_revise_val = batt_volt_average - batt_volt;
                        chg_set_revise_value(tmp_volt_revise_val);

                    }
                }
            }
        }
    }
    return  is_need_revise;
}

#endif/*defined(MBB_CHG_EXTCHG*/


int32_t chg_get_chg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }

    /*充电补偿上限*/
    if(chg_get_revise_value() < CHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = 0 - chg_get_revise_value();
    }
    else
    {
        revise_val = 0 - CHG_BATT_VOLT_REVISE_LIMIT;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg revise_val = %d\n", revise_val);
    return revise_val;
}


boolean is_need_chg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    /*电池端口电压高于补偿上限则不再补偿*/
    if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt = %d\n", batt_volt);
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_chg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop charging,no need revise!\n");
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start revise!\n",
                    (batt_volt - batt_volt_average));
                tmp_volt_revise_val = batt_volt - batt_volt_average;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_chg_revise_count++ ;
    }
    else
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop charging ,stop revise!\n",
                (batt_volt - batt_volt_average));
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            int32_t temp_revise_val = chg_get_revise_value ();
            if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    /*动态更新补偿值*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,dynamic revise!\n",
                    (batt_volt - batt_volt_average));
                    tmp_volt_revise_val = batt_volt - batt_volt_average;
                    chg_set_revise_value(tmp_volt_revise_val);
                }
            }
        }
    }
    return  is_need_revise;
}


int32_t chg_batt_volt_revise(int32_t batt_volt)
{
    int32_t revised_batt_volt = 0;
    int32_t revise_val = 0;
    if(TRUE == is_need_chg_revise())
    {
        revise_val = chg_get_chg_revise();
        revised_batt_volt = batt_volt + revise_val;
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need chg revise batt_volt_revised=%d\n",revised_batt_volt);
    }
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    else if(TRUE == is_need_extchg_revise())
    {
        revised_batt_volt = batt_volt + chg_get_extchg_revise();
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need extchg revise batt_volt_revised=%d\n",revised_batt_volt);
    }
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/
    else
    {
        revised_batt_volt = batt_volt;
        chg_set_revise_value(0);

        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:no need revise batt_volt_revised=%d\n",revised_batt_volt);
    }
    chg_print_level_message(CHG_MSG_INFO, "VOLTAGE_COMPENCATE:batt_volt=%d,revised_value=%d,batt_volt_revised=%d,real_revise_val=%d\n",\
                           batt_volt, revise_val, revised_batt_volt, chg_get_revise_value());

    return revised_batt_volt;
}
#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*电流采集次数*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*Hi6421内置库仑计支持10组有效采样数据*/
#define CURRENT_RECORD_NUM      (10)
#else
#define CURRENT_RECORD_NUM      (20)
#endif /*(MBB_CHG_COULOMETER == FEATURE_ON)*/

/*电流采集最大值边界值*/
#define CURRENT_LIMIT_UP_MAX    (2000)
/*电流采集最大值边界值*/
#define CURRENT_LIMIT_UP_MIN    (-2000)
/*电流采集最小值边界值*/
#define CURRENT_LIMIT_DOWN_MAX  (50)
/*电流采集最小值边界值*/
#define CURRENT_LIMIT_DOWN_MIN  (-50)
/*单位换算*/
#define CURRENT_UNIT_MA2UA      (1000)


int32_t chg_get_average_batt_current(void)
{
    int32_t i, used, current_ma, totalcur;

    used = 0;
    totalcur = 0;
    current_ma = 0;
    for (i = 0; i < CURRENT_RECORD_NUM; i++)
    {
        current_ma = bsp_coul_current_before(i);
        if((current_ma > CURRENT_LIMIT_UP_MAX) || (current_ma < CURRENT_LIMIT_UP_MIN))
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:invalid current = %d ma\n", current_ma);
            continue;
        }
        totalcur += current_ma;
        used++;
    }
    if(used > 0)
    {
        current_ma = totalcur / used;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:current_ma = %d\n", current_ma);
    return current_ma;
}

/*充电电流为负，有虚高；放电电流为正，有虚低*/
int32_t chg_batt_volt_coul_revise(int32_t batt_volt)
{
    int32_t current_ma = 0,revise_val = 0;
    int32_t revised_batt_volt = 0;

    revised_batt_volt = batt_volt;
    current_ma = chg_get_average_batt_current();
    if((CURRENT_LIMIT_DOWN_MIN < current_ma) && (CURRENT_LIMIT_DOWN_MAX > current_ma))
    {
        return revised_batt_volt;
    }
    revise_val = (current_ma * R_BATT) / CURRENT_UNIT_MA2UA;/*单位转换，结果转换为mV*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:revise_val = %d\n", revise_val);
    revised_batt_volt = batt_volt + revise_val;
    return revised_batt_volt;
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_poll_bat_level(void)
{
    int32_t new_one_batt_volt = 0;
    int32_t new_batt_volt = 0;
    int32_t new_one_vph_pwr_volt = 0;
    int32_t new_avg_vph_pwr_volt = 0;
    static uint32_t init_flag = FALSE;

    /*坏电池检测处理函数*/
    chg_batt_error_detect_volt();

    /*读取电池ADC采集电压*/
    new_one_batt_volt = chg_get_batt_volt_value();

#if (MBB_CHG_COULOMETER == FEATURE_ON)
    new_one_batt_volt = chg_batt_volt_coul_revise(new_one_batt_volt);
#elif (MBB_CHG_BQ27510 == FEATURE_ON)
    /*no need to revise when using bq27510*/
#else
    /*对单次电压采集值进行虚高虚低校准*/
    new_one_batt_volt = chg_batt_volt_revise(new_one_batt_volt);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

    /*保存电池单次采集电压到全局变量*/
    chg_real_info.battery_one_volt = new_one_batt_volt;

    /*读取VPH_PWR ADC采集电压*/
    new_one_vph_pwr_volt = chg_get_vph_pwr_volt_value();

    /*保存VPH_PWR单次采集电压到全局变量*/
    chg_real_info.vph_pwr_one_volt = new_one_vph_pwr_volt;

    /*电池平滑电压初始化为首次采集值*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_volt = chg_real_info.battery_one_volt;
        chg_real_info.vph_pwr_avg_volt = chg_real_info.vph_pwr_one_volt;
        chg_real_info.bat_volt_lvl = BATT_LEVEL_MAX;
        init_flag = TRUE;
    }

    /*对电池电压进行平滑算法处理*/
    new_batt_volt = chg_calc_average_volt_value(new_one_batt_volt);

    /*保存电池电压平滑值到全局变量*/
    chg_real_info.battery_volt = new_batt_volt;

    /*电池电量格数处理*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    chg_set_battery_level();
#else
    chg_set_battery_volt_level();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    /*对VPH_PWR电压进行平滑算法处理*/
    new_avg_vph_pwr_volt = chg_calc_average_vph_pwr_volt_value(new_one_vph_pwr_volt);

    /*保存VPH_PWR平滑电压到全局变量*/
    chg_real_info.vph_pwr_avg_volt = new_avg_vph_pwr_volt;

    /* log打印接口，可维可测讨论内容*/
    chg_print_level_message(CHG_MSG_DEBUG, "***********CHG_STM: chg_poll_bat_level  begin **********\n");

    /*打印充电温保护使能开关*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*开机电压门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_batt_volt_thr= %d\n", BATT_VOLT_POWER_ON_THR);

    /*打印电池单次采集电压*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_volt = %d\n", chg_real_info.battery_one_volt);
    /*打印电池电压平滑值*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_volt = %d\n", chg_real_info.battery_volt);
    /*打印打印电池电量格数*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.bat_volt_level = %d\n", chg_real_info.bat_volt_lvl);

    /*打印VPH_PWR单次采集电压*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.vph_pwr_one_volt = %d\n", chg_real_info.vph_pwr_one_volt);
    /*打印VPH_PWR电压平滑值*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.vph_pwr_avg_volt = %d\n", chg_real_info.vph_pwr_avg_volt);

    /*检测充电器移除关机*/
    chg_detect_batt_chg_for_shutoff();
    chg_detect_batt_volt_for_shutoff();
    chg_print_level_message(CHG_MSG_INFO, "*********** CHG_STM: chg_poll_bat_level  end **********\n");
}
#if (MBB_CHG_COULOMETER == FEATURE_ON)

void chg_poll_batt_soc(void)
{
    int32_t input_soc = 0;
    int32_t last_soc = chg_get_sys_batt_capacity();
    int32_t bat_stat_t = chg_get_bat_status();

    input_soc = smartstar_battery_capacity();
    if(input_soc - last_soc >= 1)
    {
        /*非充电状态，不允许百分比上升*/
        if (bat_stat_t == POWER_SUPPLY_STATUS_NOT_CHARGING)
        {
            input_soc = last_soc;
        }
        else
        {
            input_soc = last_soc + 1;
        }
    }
    else if(last_soc - input_soc >= 1)
    {
        input_soc = last_soc - 1;
    }
    else
    {
    }

    chg_set_sys_batt_capacity(input_soc);
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

int32_t chg_notify_app_charging_state(int32_t batt_state)
{
    if (NULL == g_chip)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG:NULL pointer,batt_state=%d\n ",batt_state);
        return -1;
    }
    g_chip->bat_stat = batt_state;
    power_supply_changed(&g_chip->bat);
    return 0;
}


void chg_poll_batt_charging_state_for_coul(void)
{
    static CHG_COUL_EVENT_TYPE charge_status = VCHRG_STOP_CHARGING_EVENT;
    chg_stm_state_type cur_stm_state;
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging old state=%d\n ",charge_status);
    switch (charge_status)
    {
        case VCHRG_NOT_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            else
            {
                if(!is_chg_charger_removed())
                {
                     /*IC满电停充，但是soc不是100%,需要模拟一次充停过程*/
                    /*目前状态机会出现，IC已经停充，但是充电状态机依旧停止在快充或者高温充电状态，
                      因此需要在这两种情况下通知库仑计状态保持在对应状态*/
                    if ( ( (CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                        || (CHG_STM_FAST_CHARGE_ST == chg_stm_get_cur_state())
                        || (CHG_STM_WARMCHG_ST == chg_stm_get_cur_state()) )
                    && (FALSE == chg_is_batt_full()))
                    {
                        chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                        hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                        charge_status = VCHRG_START_CHARGING_EVENT;
                    }
                }
            }

            break;
        case VCHRG_START_CHARGING_EVENT:
            if (!chg_is_IC_charging())
            {
                if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                || (CHG_STOP_COMPLETE == chg_get_stop_charging_reason()))
                {
                    hisi_coul_charger_event_rcv(VCHRG_CHARGE_DONE_EVENT);
                    charge_status = VCHRG_CHARGE_DONE_EVENT;
                }
                else
                {
                    hisi_coul_charger_event_rcv(VCHRG_STOP_CHARGING_EVENT);
                    charge_status = VCHRG_STOP_CHARGING_EVENT;
                }
            }
            break;
        case VCHRG_STOP_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            /*目前状态机会出现，IC已经停充，但是充电状态机依旧停止在快充或者高温充电状态，
              因此需要在这两种情况下通知库仑计状态保持在对应状态*/
            else if (!is_chg_charger_removed())
            {
                /*IC满电停充，但是soc不是100%,需要模拟一次充停过程*/
                if ( ( (CHG_STM_MAINT_ST == chg_stm_get_cur_state()) 
                    || (CHG_STM_FAST_CHARGE_ST == chg_stm_get_cur_state())
                    || (CHG_STM_WARMCHG_ST == chg_stm_get_cur_state()) )
                    && (FALSE == chg_is_batt_full()) )
                {
                    chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                    hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                    charge_status = VCHRG_START_CHARGING_EVENT;
                }
            }
            else
            {
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;
            }
            break;
        case VCHRG_CHARGE_DONE_EVENT:
            if(is_chg_charger_removed())
            {
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;
            }
            else
            {
                if (chg_is_IC_charging())
                {
                    hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                    charge_status = VCHRG_START_CHARGING_EVENT;
                }

                /*IC满电停充，但是soc不是100%,需要显示充电动画，平滑百分比*/
                else if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                    && (FALSE == chg_is_batt_full()))
                {
                    chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                }
            }
            break;
        default:
            break;
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging new state=%d\n ",charge_status);
}

#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

int32_t chg_tbat_status_get(void)
{
    int32_t tbat_v = 0;
    int32_t ret = FALSE;
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    uint32_t batt_id = 0;
    int32_t tbat_dischg_volt = 0;
    int32_t tbat_supply_volt = 0;
#endif
    (void)chg_set_charge_enable(FALSE);
    chg_delay_ms(CHG_DELAY_COUNTER_SIZE);

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    batt_id = chg_get_batt_id();
    if (CHG_BATT_ID_FEIMAOTUI_6400MAH == batt_id)
    {
        tbat_supply_volt = TBAT_SUPPLY_VOLT_FMT;
        tbat_dischg_volt = TBAT_DISCHG_VOLT_FMT;
    }
    else if (CHG_BATT_ID_XINGWANGDA_6400MAH == batt_id)
    {
        tbat_supply_volt = TBAT_SUPPLY_VOLT_XWD;
        tbat_dischg_volt = TBAT_DISCHG_VOLT_XWD;
    }
    else
    {
        tbat_supply_volt = TBAT_SUPPLY_VOLT;
        tbat_dischg_volt = TBAT_DISCHG_VOLT;
    }

    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();

    /*判断是否满足放电或补电，连续读3次*/
    if((tbat_v > tbat_dischg_volt) || (tbat_v < tbat_supply_volt))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v > tbat_dischg_volt) || (tbat_v < tbat_supply_volt))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v > tbat_dischg_volt) || (tbat_v < tbat_supply_volt))
            {
                ret = TRUE;
            }
        }
    }
    (void)chg_set_charge_enable(TRUE);
    chg_print_level_message(CHG_MSG_ERR,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ",tbat_v);
    return ret;
#else
    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();

    /*判断是否满足放电或补电，连续读3次*/
    if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
            {
                ret = TRUE;
            }
        }
    }
    (void)chg_set_charge_enable(TRUE);
    chg_print_level_message(CHG_MSG_ERR,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ",tbat_v);
    return ret;
#endif
}


boolean chg_is_sply_finish(void)
{
    uint32_t tbat_v = 0;
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    uint32_t batt_id = 0;
    int32_t tbat_dischg_stop_volt = 0;
    int32_t tbat_supply_stop_volt = 0;

    batt_id = chg_get_batt_id();
    if (CHG_BATT_ID_FEIMAOTUI_6400MAH == batt_id)
    {
        tbat_supply_stop_volt = TBAT_SUPPLY_STOP_VOLT_FMT;
        tbat_dischg_stop_volt = TBAT_DISCHG_STOP_VOLT_FMT;
    }
    else if (CHG_BATT_ID_XINGWANGDA_6400MAH == batt_id)
    {
        tbat_supply_stop_volt = TBAT_SUPPLY_STOP_VOLT_XWD;
        tbat_dischg_stop_volt = TBAT_DISCHG_STOP_VOLT_XWD;
    }
    else
    {
        tbat_supply_stop_volt = TBAT_SUPPLY_VOLT;
        tbat_dischg_stop_volt = TBAT_DISCHG_VOLT;
    }

    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY LEVEL:%d\n ",tbat_v);
    /*判断是否满足放电或补电，连续读3次*/
    if((tbat_v < tbat_dischg_stop_volt) && (tbat_v > tbat_supply_stop_volt))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v < tbat_dischg_stop_volt) && (tbat_v > tbat_supply_stop_volt))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v < tbat_dischg_stop_volt) && (tbat_v > tbat_supply_stop_volt))
            {
                if( TRUE == fact_release_flag )
                {
                    chg_set_fact_release_mode(FALSE);
                    fact_release_flag = FALSE;
                }
                chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
                /*通知应用补电完成*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ", tbat_v);
                chg_sply_unlock();

                return TRUE;
            }
        }
    }
    return FALSE;
#else
    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY LEVEL:%d\n ",tbat_v);
    /*判断是否满足放电或补电，连续读3次*/
    if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
            {
                if( TRUE == fact_release_flag )
                {
                    chg_set_fact_release_mode(FALSE);
                    fact_release_flag = FALSE;
                }
                chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
                /*通知应用补电完成*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ", tbat_v);
                chg_sply_unlock();

                return TRUE;
            }
        }
    }
    return FALSE;
#endif
}


int32_t chg_batt_supply_proc(void *task_data)
{
    int32_t tbat_v = 0;
    uint32_t wait_idx = 0;
    /*补电标志 0:表示充电，1: 表示放电*/
    boolean chg_flag = 0;
    uint32_t  tc_on = TBAT_SUPLY_DELAY_COUNTER;
    uint32_t  tc_off = TBAT_STOP_DELAY_COUNTER;

    if ( NULL == task_data )
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:supply_proc task_data is NULL !\n");
    }

    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:START DISCHARGE OR CHARGE\n ");

    /* 放电删除充电任务*/
    chg_task_delete();

    chg_sply_lock();
    chg_set_cur_chg_mode(CHG_SUPPLY_MODE);

    chg_stm_set_cur_state(CHG_STM_MAX_ST);

    chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_NEED_SUPPLY;
    /*通知应用需要补电*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);

    do{
        tbat_v = chg_get_batt_volt_value();
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CURRENT LEVEL:%d\n ",tbat_v);

        /*如果电池电量大于放电阈值*/
        if(tbat_v > TBAT_DISCHG_STOP_VOLT)
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING BEGIN\n");
            chg_flag = 1;

            (void)chg_set_charge_enable(FALSE);
            (void)chg_set_suspend_mode(TRUE);
            chg_set_fact_release_mode(TRUE);
            fact_release_flag = TRUE;
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            /*脉冲方式*/
            (void)chg_set_suspend_mode(FALSE);

            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING END\n");
        }
        /*充电*/
        else if(tbat_v < TBAT_SUPPLY_STOP_VOLT)
        {
            /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING BEGIN\n");
            chg_flag = 0;

            //补电参数设置为fast充电参数
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
            (void)chg_set_charge_enable(TRUE);
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }
            /*脉冲方式*/
            (void)chg_set_charge_enable(FALSE);
            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING END\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING OR DISCHARGING EXIT:%d\n ",tbat_v);
        }
    }while(!chg_is_sply_finish());
    /* 打开使能 */
    (void)chg_set_charge_enable(FALSE);

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*LCD状态更新*/
    chg_display_interface( CHG_DISP_OK );
#endif/*MBB_CHG_POWER_SUPPLY == FEATURE_OFF*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY EXIT LEVEL:%d\n ",tbat_v);
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY CHARGE or DISCHARGER FINISH\n ");

    return TBAT_SUPPLY_CURR_SUCCESS;
}


int32_t chg_tbat_chg_sply(void)
{
    /* 调用归一化接口启动补电任务**/
    chg_sply_task_create();
/***************Note:平台相关代码，根据平台按需要添加，有的平台如V7R1需要
              移植人员根据需要，添加或者移除下边函数调用***************************/

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*补电开始显示补电未完成图标*/
    chg_display_interface( CHG_DISP_FAIL );
#endif/*MBB_CHG_POWER_SUPPLY == FEATURE_OFF*/
    return TBAT_SUPPLY_CURR_SUCCESS;
}
#endif /* MBB_CHG_COMPENSATE == FEATURE_ON */

static int32_t chg_get_cur_batt_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_one_volt )
    {
        chg_real_info.battery_one_volt = chg_get_batt_volt_value();
    }

    return chg_real_info.battery_one_volt;
}


int32_t chg_get_sys_batt_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_volt )
    {
        chg_real_info.battery_volt = chg_get_batt_volt_value();
    }
    return chg_real_info.battery_volt;
}


int32_t chg_get_avg_vph_pwr_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_volt )
    {
        chg_real_info.vph_pwr_avg_volt = chg_get_vph_pwr_volt_value();
    }
    return chg_real_info.vph_pwr_avg_volt;
}


int chg_get_cur_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_one_temp )
    {
        chg_real_info.battery_one_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_one_temp;
}
EXPORT_SYMBOL_GPL(chg_get_cur_batt_temp);


int chg_get_sys_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_temp )
    {
        chg_real_info.battery_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_temp;
}
EXPORT_SYMBOL_GPL(chg_get_sys_batt_temp);


void chg_set_sys_batt_capacity(int32_t capacity)
{
    /*电量发生变化时才进行上报如果上次上报电量跟本次电量不同时才给应用上报*/
    if(chg_real_info.bat_capacity != capacity)
    {
        chg_real_info.bat_capacity = capacity;
        /*电池电量百分比发生变化通知应用查询*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_capacity=%d\n",capacity);
    }
}


int32_t chg_get_sys_batt_capacity(void)
{
    if(chg_is_no_battery_powerup_enable())
    {
        return BATT_CAPACITY_FULL;
    }
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    if(0 == chg_real_info.bat_capacity)
    {
        return smartstar_battery_capacity();
    }
    /*使能禁止低电关机后，达到低电关机门限时，返回关机门限+1*/
    if( unlikely( TRUE == chg_is_low_battery_poweroff_disable() ) )
    {
        if( BATT_CAPACITY_SHUTOFF >= chg_real_info.bat_capacity )
        {
            chg_real_info.bat_capacity = BATT_CAPACITY_SHUTOFF + 1;
        }
    }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/    
#if (MBB_CHG_BQ27510 == FEATURE_ON)
    if(0 == chg_real_info.bat_capacity)
    {
        return hisi_battery_capacity();
    }
#endif
    return chg_real_info.bat_capacity;
}

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

boolean chg_is_batt_in_state_of_emergency(void)
{
    /*仅在开机状态下判断是否需要关机*/
    return (!chg_is_powdown_charging() && BATT_VOLT_POWER_OFF_THR > chg_get_sys_batt_volt());
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_set_batt_time_to_full(int32_t time_to_full)
{
    chg_real_info.bat_time_to_full = time_to_full;
    /*通知应用剩余充电时间发生变化*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_time_to_full=%d\n",time_to_full);
}


int32_t chg_get_batt_time_to_full(void)
{
    return chg_real_info.bat_time_to_full;
}


boolean chg_is_batt_full(void)
{
/*支持库仑计的产品，满电门限定义为95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL == chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}


boolean chg_is_need_insert_charge(void)
{
/*支持库仑计的产品，满电门限定义为95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL != chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() < BATT_INSERT_CHARGE_THR);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}


static boolean chg_is_batt_need_rechg(void)
{
/*充电器在位时，只要百分比低于100%，则启动充电*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    else
    {
        if (TRUE == g_chg_battery_expand_change_normal_rechg_flag)
        {
            /*复充条件增加对电量的判断，避免因库仑计电量精度问题导致电量一直下降。*/
            return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR
                || chg_get_sys_batt_capacity() <= BATT_EXPAND_CAPACITY_RECHG);
        }
    }
#endif
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    /*复充门限增加对实际电量的判断*/
    return ((chg_get_sys_batt_capacity() <= BATT_CAPACITY_RECHG) &&
        (hisi_battery_capacity() <= chg_get_recharge_threshold()));
#endif
        return (chg_get_sys_batt_capacity() <= BATT_CAPACITY_RECHG);
#else
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    else
    {
        return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR);
    }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}

static boolean chg_is_batt_ovp(void)
{
    return (boolean)((chg_get_sys_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_THR)
            || (chg_get_cur_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_ONE_THR));
}

static boolean chg_is_batt_temp_valid(void)
{
    return !chg_temp_protect_flag;
}


TEMP_EVENT chg_get_batt_temp_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_temp_state=%d\n",chg_batt_temp_state);
    return chg_batt_temp_state;
}


void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type)
{
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
    if(EXTCHG_NOSUPPORT == g_chg_hardpara_dtsi.chg_extchg_support && CHG_EXGCHG_CHGR == chgr_type)
    {
        return;
    }
#endif
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = CHG_HVDCP_INVALID;
    hvdcp_type = chg_get_hvdcp_type();
    /*判断是否为高压充电器*/
    if (CHG_HVDCP_INVALID == hvdcp_type)
    {
        if (CHG_WALL_CHGR == fsa9688_get_charger_type())
        {
            chg_stm_state_info.cur_chgr_type = CHG_WALL_CHGR;
        }
        else
        {
            chg_stm_state_info.cur_chgr_type = chgr_type;
        }
    }
    else
    {
       chg_stm_state_info.cur_chgr_type = CHG_HVDCP_CHGR;
    }
#else
    chg_stm_state_info.cur_chgr_type = chgr_type;
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_chgr_type=%d\n",chg_stm_state_info.cur_chgr_type);
}


chg_chgr_type_t chg_stm_get_chgr_type(void)
{

    if(TRUE == chg_is_no_battery_powerup_enable())
    {
        chg_stm_state_info.cur_chgr_type = CHG_WALL_CHGR;
    }
    /*1. Return the current external charger type.*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_chgr_type=%d\n",chg_stm_state_info.cur_chgr_type);
    return chg_stm_state_info.cur_chgr_type;
}


boolean is_chg_charger_removed(void)
{
    boolean is_chgr_present      = chg_is_charger_present();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
    if ((FALSE == is_chgr_present) && (FALSE == is_wireless_online))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: is_chgr_present=%d, is_wireless_online=%d.\n",
                    is_chgr_present,is_wireless_online);
        return TRUE;
    }
#else
    /*1. External charger removed, swith to battery only state.*/
    if (FALSE == is_chgr_present)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: is_chgr_present=%d !\n",is_chgr_present);
        return TRUE;
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: charger attached !\n");
        return FALSE;
    }
}

#if (MBB_CHG_WIRELESS == FEATURE_ON)

void chg_stm_set_wireless_online_st(boolean online)
{
    chg_stm_state_info.wireless_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_wireless_online_st=%d\n",online);
}


boolean chg_stm_get_wireless_online_st(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:get_wireless_online_st=%d\n",\
                chg_stm_state_info.wireless_online_st);

    return chg_stm_state_info.wireless_online_st;
}
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

void chg_stm_set_extchg_online_st(boolean online)
{
    chg_stm_state_info.extchg_online_st = online;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_extchg_online_st=%d\n",online);
}


boolean chg_stm_get_extchg_online_st(void)
{
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
    if(EXTCHG_NOSUPPORT == g_chg_hardpara_dtsi.chg_extchg_support)
    {
        /*  若产品不支持外充，则外充状态置为 0 再返回 */
        chg_stm_state_info.extchg_online_st = FALSE;
    }
#endif
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_extchg_online_st=%d\n",\
                            chg_stm_state_info.extchg_online_st);
    return chg_stm_state_info.extchg_online_st;
}
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


static void chg_stm_set_cur_state(chg_stm_state_type new_state)
{
    /*1. Update the current state machine state.*/
    chg_stm_state_info.cur_stm_state = new_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_cur_state=%d\n",new_state);
}


chg_stm_state_type chg_stm_get_cur_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_state=%d\n",\
                    chg_stm_state_info.cur_stm_state);
    /*1. Return the current state.*/
    return chg_stm_state_info.cur_stm_state;
}


void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode)
{
    chg_stm_state_info.cur_chg_mode = chg_mode;
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set_cur_chg_mode=%d\n",chg_mode);
}


CHG_MODE_ENUM chg_get_cur_chg_mode(void)
{

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_chg_mode =%d\n",\
                  chg_stm_state_info.cur_chg_mode);

    /*1. Return the current state.*/
    return chg_stm_state_info.cur_chg_mode;
}


boolean chg_get_charging_status(void)
{
    chg_stm_state_type chg_stm_cur_state = CHG_STM_INIT_ST;
    chg_stm_cur_state = chg_stm_get_cur_state();
    /*1. Return the current state.*/
    if((CHG_STM_FAST_CHARGE_ST == chg_stm_cur_state) \
       || (CHG_STM_WARMCHG_ST == chg_stm_cur_state) \
       || (CHG_STM_HVDCP_CHARGE_ST == chg_stm_cur_state) )
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is charging!\n");
        return TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is not charging!\n");
        return FALSE;
    }
}


void chg_set_supply_limit_by_stm_stat(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    switch(cur_chgr_type)
    {
        case CHG_WALL_CHGR:
        {
            chg_set_supply_limit(chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
        case CHG_USB_HOST_PC:
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                chg_set_supply_limit(chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else
            {
                chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        case CHG_CHGR_UNKNOWN:
        {
            chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
        case CHG_HVDCP_CHGR:
        {
            if(CHG_HVDCP_5V == hvdcp_type)
            {
                chg_set_supply_limit(chg_5v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_5v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_supply_limit(chg_9v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_9v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else if(CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_supply_limit(chg_12v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_12v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else
            {
                //reserve
            }
            break;
        }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
        default:
            break;
    }
}


void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param)
{
    uint32_t ret_code = 0;

    /* ASSERT(ptr_hw_param != NULL); */
    if (NULL == ptr_hw_param)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NULL pointer in chg_set_hardware_parameter!!\n");
        return;
    }
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:In battery protect condition do not update parameter!!\n");
        return;
    }
#endif
    /*Dump all the parameters to set.*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ILIMIT]: %dmA, [ICHG]: %dmA, [VBATREG]: %dmV.\n",
               ptr_hw_param->pwr_supply_current_limit_in_mA,
               ptr_hw_param->chg_current_limit_in_mA,
               ptr_hw_param->chg_CV_volt_setting_in_mV);

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ITERM]: %dmA, [CHARGE]: %s.\n",
               ptr_hw_param->chg_taper_current_in_mA,
               ptr_hw_param->chg_is_enabled ? "enabled" : "disabled");

    /*1. Set power supply front-end current limit.*/
    if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
    {
        /*If error occured, set 1st bit of ret_code.*/
        ret_code |= (1 << 1);
    }

    /*2. Set charge current.*/
    if ( FALSE == chg_set_cur_level(ptr_hw_param->chg_current_limit_in_mA))
    {
        /*If error occured, set 2nd bit of ret_code.*/
        ret_code |= (1 << 2);
    }

    /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
    if ( FALSE == chg_set_vreg_level(ptr_hw_param->chg_CV_volt_setting_in_mV))
        /*If error occured, set 3rd bit of ret_code.*/
    {
            /*If error occured, set 3rd bit of ret_code.*/
            ret_code |= (1 << 3);
    }

    /*4. Set taper(terminate) current. Also IC type dependent, may not work on
         some IC, e.g. max8903c.*/
    if ( FALSE == chg_set_term_current(ptr_hw_param->chg_taper_current_in_mA))
    {
        /*If error occured, set 4th bit of ret_code.*/
        ret_code |= (1 << 4);
    }

    /*5. Enable/Disable Charge.*/
    if ( FALSE == chg_set_charge_enable(ptr_hw_param->chg_is_enabled))
    {
        /*If error occured, set 5th bit of ret_code.*/
        ret_code |= (1 << 5);
    }

    if (ret_code)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC hw parameter error 0x%x.\n", ret_code);
    }

}


static void chg_chgr_type_checking_timer_cb(chg_timer_para data)
{
    (void)data;

    (void)chg_send_msg_to_main(CHG_CHGR_TYPE_CHECKING_EVENT);

     chg_print_level_message(CHG_MSG_INFO, "CHG_STM: chg_send_msg_to_main CHG_CHGR_TYPE_CHECKING_EVENT!\n ");
}


void chg_start_chgr_type_checking(void)
{
    chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,chg_chgr_type_checking_timer_cb);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM: chg_start_chgr_type_checking!\n ");
}


void chg_check_and_update_hw_param_per_chgr_type(void)
{
    static uint8_t chgr_type_checking_cnter = 0;
    chg_chgr_type_t chgr_type_from_usb = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type =  chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*1. If charger removal detected, give warning msg, return.*/
    if (FALSE == chg_is_charger_present())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr has been removed, type checking ignored.\n");
        return;
    }

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    if(CHG_HVDCP_INVALID != hvdcp_type)
    {
        chg_stm_set_chgr_type(CHG_HVDCP_CHGR);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    /*如果是高压电池在0-10℃已经做过限流则不再更新充电参数*/
    if( 0 == chg_sub_low_temp_changed )
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]High Volt Batt below 10 degree did`t update parameter!\n", __func__);
        return;
    }
#endif/*MBB__CHG_HIGH_VOLT_BATT == FEATURE_ON*/
    if (CHG_CHGR_UNKNOWN == chgr_type_from_usb)
    {
        if (chgr_type_checking_cnter >= CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type check timeout failed," \
                       " treat as non-standard.\n");
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Charger type check timeout failed," \
                       " treat as non-standard.\n");
            chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
            /*清零计数器*/
            chgr_type_checking_cnter = 0;
        }
        else
        {
            chgr_type_checking_cnter++;
            chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,
                              chg_chgr_type_checking_timer_cb);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Raise %d time chgr type checking.\n",
                       chgr_type_checking_cnter);
        }

        return;
    }
    else if((CHG_WALL_CHGR == chgr_type_from_usb) || (CHG_USB_HOST_PC == chgr_type_from_usb)
             || (CHG_NONSTD_CHGR == chgr_type_from_usb))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type %d attached after %d times polling.\n",
            chgr_type_from_usb, chgr_type_checking_cnter + 1);
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG_STM:Charger type %d attached " \
                   "after %d times polling.\n",
                   chgr_type_from_usb, chgr_type_checking_cnter + 1);
        if (CHG_WALL_CHGR != chgr_type_from_usb)
        {
            if ((CHG_USB_HOST_PC == chgr_type_from_usb) && (CHG_CURRENT_SS == usb_speed_work_mode()))
            {
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_check_and_update_hw_param_per_chgr_type]set usb 3.0 charge parameter\n\r");
                chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[curr_state]);
            }
            else
            {
                 chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]USB/3rd chgr, no need update hw paras.\n", __func__);
            }
        }
        else
        {
            if (curr_state == CHG_STM_FAST_CHARGE_ST
                || curr_state == CHG_STM_WARMCHG_ST
                || curr_state == CHG_STM_MAINT_ST
                || curr_state == CHG_STM_INVALID_CHG_TEMP_ST)
            {
                chg_en_flag = 1;
                chg_en_timestamp = jiffies;
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]set std_chgr charge parameter!\n", __func__);
                chg_set_hardware_parameter(&chg_std_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:[%s]Invalid stm_state=%d, "    \
                           "chgr_type=%d.\n", __func__, curr_state, chgr_type_from_usb);
            }
        }

        /*清零计数器*/
        chgr_type_checking_cnter = 0;
        return;
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == chgr_type_from_usb)
    {
        if(CHG_STM_HVDCP_CHARGE_ST == curr_state)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]set HVDCP_chgr charge parameter!\n", __func__);
            /*Set charging hardware parameter according to HVDCP voltage type.*/
            if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[curr_state]);
            }
            else if( CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[curr_state]);
            }
        }
        else
        {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:[%s]Invalid stm_state=%d, "    \
                           "chgr_type=%d.\n", __func__, curr_state, chgr_type_from_usb);
        }

        /*清零计数器*/
        chgr_type_checking_cnter = 0;
        return;
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if(CHG_500MA_WALL_CHGR == chgr_type_from_usb)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[curr_state]);

        /*清零计数器*/
        chgr_type_checking_cnter = 0;
        return;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chgr_type_from_usb)
    {
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[curr_state]);
        mlog_print(MLOG_CHG, mlog_lv_info, "Wireless charger detected in polling check.\n");

        /*清零计数器*/
        chgr_type_checking_cnter = 0;
        return;
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid chgr_type!\n");
        /*清零计数器*/
        chgr_type_checking_cnter = 0;
    }
}


void chg_stm_switch_state(chg_stm_state_type new_state)
{
    chg_stm_state_type cur_state = chg_stm_get_cur_state();
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_switch_state begin!\n");
    if((cur_state >= CHG_STM_MAX_ST) || (cur_state <= CHG_STM_INIT_ST))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: cur_state Invalid \n");
        return;
    }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    chg_stm_set_pre_state(cur_state);
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

    if (new_state >= CHG_STM_MAX_ST || new_state <= CHG_STM_INIT_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: Invalid state %d.\n", new_state);
        return;
    }

    chg_stm_set_cur_state(new_state);
    if (cur_state != new_state)
    {
        if (NULL != chg_stm_state_machine[cur_state].chg_stm_exit_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state exit %d state.\n", cur_state);
            chg_stm_state_machine[cur_state].chg_stm_exit_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Cur_state %d exit func doesN'T exist.\n", cur_state);
        }

        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %d to %d state.\n",
            cur_state, new_state);
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));


        if (NULL != chg_stm_state_machine[new_state].chg_stm_entry_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state entry %d state.\n", new_state);
            chg_stm_state_machine[new_state].chg_stm_entry_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:New_state %d entry func doesN'T exist.\n", new_state);
        }

        /*added by ligang to shorten ui display time*/
        if ((CHG_STM_TRANSIT_ST == new_state)
            && (NULL != chg_stm_state_machine[new_state].chg_stm_period_func))
        {
            /*切换前进行一次电池电压轮询查询**/
            chg_poll_bat_level();
            /* 切换前进行一次电池温度轮询查询**/
            chg_poll_batt_temp();
            chg_stm_state_machine[new_state].chg_stm_period_func();
        }

        if (CHG_STM_INVALID_CHG_TEMP_ST == new_state)
        {
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG STM switch from %s to invalid chg temp state. "
                "tBat = %d'C, tBat_sys = %d'C.\n",
                (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
                chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        }

        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));
        (void)chg_dump_ic_hwinfo();
        mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
            "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_get_cur_batt_volt(), chg_get_sys_batt_volt(),
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
#if ( FEATURE_ON == MBB_MLOG )
        if(((CHG_STM_FAST_CHARGE_ST == new_state) && (CHG_STM_WARMCHG_ST != cur_state)) || 
            ((CHG_STM_WARMCHG_ST == new_state) && (CHG_STM_FAST_CHARGE_ST != cur_state)))
        {
            printk(KERN_ERR "CHG_STM:charge_count\n");
            mlog_set_statis_info("charge_count",1);//充电总次数 加1
        }
        if(CHG_STM_WARMCHG_ST == new_state)
        {
            printk(KERN_ERR "CHG_STM:overtemp_charge_count\n");
            mlog_set_statis_info("overtemp_charge_count",1);//高温充电次数加1
        }
        if(CHG_STM_INVALID_CHG_TEMP_ST == new_state) 
        {
            printk(KERN_ERR "CHG_STM:overtemp_charge_stop_count\n");
            mlog_set_statis_info("overtemp_charge_stop_count",1); //高温停充次数加1
        }
#endif        
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state: charge has already be %s[%d] state, " \
                         "no need switch.\n", TO_STM_NAME(cur_state), cur_state);
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_switch_state over!\n");
}


void chg_stm_periodic_checking_func(void)
{
    chg_stm_state_type chg_current_st = CHG_STM_INIT_ST;
    chg_chgr_type_t    chg_cur_chgr   = CHG_CHGR_UNKNOWN;

    uint32_t chg_prt_timer_val = 0;
    int32_t  cur_poll_timer_period  = 0;
    /*0:-PwrOff Charging; 1:-Normal Charging.*/
    uint8_t  is_normal_chg_mode     = !chg_is_powdown_charging();
    /*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
    uint8_t  is_std_wall_chgr       = 0;

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_periodic_checking_func begin!!\n");
    if((CHG_WALL_CHGR == chg_stm_get_chgr_type())
     || (CHG_HVDCP_CHGR == chg_stm_get_chgr_type()))
    {
        is_std_wall_chgr = 1;
    }

    /* Get the current battery charging state */
    chg_current_st = chg_stm_get_cur_state();

    if(1 == chg_en_flag)
    {
        unsigned long time_now = jiffies;
        if((time_now - chg_en_timestamp) > CHG_EN_TIME_SLOT)
        {
            chg_en_flag = 0;
        }
    }
    else
    {
        //for lint
    }

    /*Make sure that we are within the charger state machine
      configuration table*/
    if (chg_current_st > CHG_STM_INIT_ST && chg_current_st < CHG_STM_MAX_ST)
    {
        /* If we have a valid state function, call it. */
        if (NULL != chg_stm_state_machine[chg_current_st].chg_stm_period_func)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stm_periodic_checking_func polling at %d state\n", chg_current_st);
            chg_stm_state_machine[chg_current_st].chg_stm_period_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:State %d haveNOT period func.\n", chg_current_st);
        }

        /*Obtain current_st once again, since chg state may be updated in period_func*/
        chg_current_st = chg_stm_get_cur_state();
        chg_cur_chgr   = chg_stm_get_chgr_type();

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:stm periodic checking: Charge stay at %s[%d] state now.\n",
            TO_STM_NAME(chg_current_st), chg_current_st);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:stm periodic checking: Current chgr is %s[%d].\n",
            TO_CHGR_NAME(chg_cur_chgr), chg_cur_chgr);

        if(TRUE == chg_get_charging_status())
        {
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
            chg_prt_timer_val = g_chg_hardpara_dtsi.chg_fastchg_timeout_value_in_sec[is_normal_chg_mode][is_std_wall_chgr];
#else
            chg_prt_timer_val = chg_fastchg_timeout_value_in_sec[is_normal_chg_mode][is_std_wall_chgr];
#endif
            if (chg_stm_state_info.charging_lasted_in_sconds < chg_prt_timer_val)
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at charging state.\n");

                cur_poll_timer_period = chg_poll_timer_get();
                if (CHG_ERROR == cur_poll_timer_period)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");
                }
                else
                {
                    /*充电未超时标志位置为false*/
                    chg_powdown_charging_time_expired_flag = FALSE;
                    chg_stm_state_info.charging_lasted_in_sconds +=
                                   ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging has lasted for %d Seconds.\n",
                                            chg_stm_state_info.charging_lasted_in_sconds);
                }
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());

                /*If battery is full already, or at pwroff chg mode, goto maint state.*/
                if (TRUE == chg_is_batt_full() || FALSE == is_normal_chg_mode)
                {
                   /*4. Reset fast charge counter.and switch to manit state,disable chg*/
                   /*关机状态充电超时，将标志位置为true，复充前判断此标志位*/
                   if (FALSE == is_normal_chg_mode)
                   {
                       chg_powdown_charging_time_expired_flag = TRUE;
                   }
                   chg_stm_switch_state(CHG_STM_MAINT_ST);
                   chg_stm_state_info.charging_lasted_in_sconds = 0;
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Reset charging counter, " \
                           "continue to charge.\n");
                 /*Reset fast charge counter and continue to charge */
                    chg_stm_state_info.charging_lasted_in_sconds = 0;
                }

                mlog_print(MLOG_CHG, mlog_lv_warn, "Charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());
                mlog_print(MLOG_CHG, mlog_lv_warn, "current state: %s, current charger:%s.\n",
                    TO_STM_NAME(chg_current_st), TO_CHGR_NAME(chg_cur_chgr));
            }
        }
        else
        {
            chg_stm_state_info.charging_lasted_in_sconds = 0;
        }
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        /*检测停充但充电器在位的时间*/
        if ((TRUE ==  chg_is_charger_present()) 
            && (FALSE == chg_get_charging_status())
            && (BATT_CAPACITY_FULL == chg_real_info.bat_capacity))
        {
            cur_poll_timer_period = chg_poll_timer_get();
            if (CHG_ERROR == cur_poll_timer_period)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");
            }
            else
            {
                chg_stm_state_info.charger_lasted_without_charging_in_seconds +=
                               ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                chg_print_level_message(CHG_MSG_INFO, 
                                        "CHG_STM:charger has existed without charging for %d Seconds.\n",
                                        chg_stm_state_info.charging_lasted_in_sconds);
            }
        }
        else
        {
            chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
        }
#endif
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in period checking.\n", chg_current_st);
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_periodic_checking_func over!!\n");
}


static void chg_notify_UI_charging_state(uint8_t bc_state)
{
    /*Don't display battery charging as default.*/
    uint8_t   new_bc_state         = !!bc_state;
    uint8_t   is_pwr_off_charging  = chg_is_powdown_charging();
    BATTERY_EVENT  event_to_send   = BAT_EVENT_MAX;
    static uint8_t cur_ui_bc_state = 0;

    if (new_bc_state != cur_ui_bc_state)
    {
        if (new_bc_state)
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_OFF_START;
            }
            else
            {
                event_to_send = BAT_CHARGING_ON_START;
            }
        }
        else
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_DOWN_STOP;
            }
            else
            {
                event_to_send = BAT_CHARGING_UP_STOP;
            }
        }
        cur_ui_bc_state = new_bc_state;

        /*Send the event to UI module.*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, event_to_send);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Send battery event %d to APP.\n", event_to_send);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:UI bc state is already %d.\n", cur_ui_bc_state);
    }
}


static void chg_transit_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into transition state.\n");

    /*1. Reset charger type to INVALID will be done at:
         a. chg_stm_init() when system just powered up.
         b. Entry function of battery only state when normal running.
     */

    /*2. Don't change UI battery charging state, just KEEP is OK.*/

    /*3. Set poll timer mode, FAST or CHGR_INPUT, will be done:
         a. When system just powered up: chg_stm_init or other module init function.
         b. For CHGR_INPUT: period func.
     */
    chg_poll_timer_set(FAST_POLL_CYCLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:transit_state_entry_func poll_timer_set FAST_POLL_CYCLE.\n");

    /*4. IF chgr_is_present
       ->Y: 1. Read chgr type from USB, Set chgr_type
            IF chgr type still unknown
          ->Y: Raise chgr type checking timer.
            2. Set charging hardware parameter per charger type
     */
        /*Config charge hardware parameter according to chgr_type.*/
    /*标充*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    /*高压充电器*/
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else if(CHG_HVDCP_9V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else if(CHG_HVDCP_12V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else
        {
            //reserve
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    //usb 3.0类型
    else if ((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_transit_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else if ((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }

#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        /*一直打开无线充电芯片使能，无线充电过程中个状态的使能由BQ24196控制*/
        chg_set_wireless_chg_enable(TRUE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_transit_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_TRANSIT_ST]);

        mlog_print(MLOG_CHG, mlog_lv_info, "wireless charger detected.\n", cur_chgr_type);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type unknown use usb para and raise checking timer.\n");
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else
    {
        if(TRUE == is_chg_charger_removed())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force transit_state_entry to batt_only_st!\n");
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:transit_state_entry Chgr type invaild!\n");
        }
    }
    mlog_print(MLOG_CHG, mlog_lv_info, "charger type %d insertion detected.\n", cur_chgr_type);
}


static void chg_transit_state_period_func(void)
{
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
     /*1. IF charger_remove
             Switch to batttery only state.
       */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force transit_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

    }

    /*2. IF vBat > BATT_INSERT_CHARGE_THR || batt OVP
            Switch to maintenance state.
     */
    else if (FALSE == chg_is_need_insert_charge() || TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
                   chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*3. IF Battery is too hot or too cold
            Switch to invalid charge battery temperature state.
     */
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Invalid batt-temp, tBat = %d'C, " \
                   "tBat_sys = %d'C.\n", chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {
         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Switch transit state to warmchg state, tBat = %d'C, tBat_sys = %d'C.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_INVALID != hvdcp_type)
    {
         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Switch transit state to hvdcpchg state\n");
         chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*4. ELSE Switch to fast chg state.*/
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:form transit_state to fast_stste!\n");
        chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
    }
}


static void chg_transit_state_exit_func(void)
{
   chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from transition state.\n");
}

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)

void chg_set_high_volt_batt_current(chg_chgr_type_t cur_chgr_type )
{
    uint32_t index_num = 0;


    for(index_num = 0; index_num < CHG_CHGR_INVALID; index_num++)
    {
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
        if(g_chg_hardpara_dtsi.charger_current_limit_paras[index_num].chgr_type == cur_chgr_type)
#else
        if(charger_current_limit_paras[index_num].chgr_type == cur_chgr_type)
#endif
        {
#if (MBB_CHG_HVDCP_CHARGE == FEATURE_ON)
            switch(g_chg_batt_data->batt_id)
            {
                case CHG_BATT_ID_FEIMAOTUI_2300MAH:
                {
                    chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
#if (MBB_CHG_SMB1351 == FEATURE_OFF)
                    chg_set_supply_limit(HVDCP_CHARGE_CURRENT_500MA);
#endif/*MBB_CHG_SMB1351 == FEATURE_OFF*/
                    break;
                }

                default:
                    chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
#if (MBB_CHG_SMB1351 == FEATURE_OFF)
                    chg_set_supply_limit(HVDCP_CHARGE_CURRENT_500MA);
#endif/*MBB_CHG_SMB1351 == FEATURE_OFF*/
                    break;
            }
#endif/*MBB_CHG_HVDCP_CHARGE == FEATURE_ON*/
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
            chg_set_current_level(g_chg_hardpara_dtsi.charger_current_limit_paras[index_num].current_limit,
                g_chg_batt_data->is_20pct_calib);
            chg_set_supply_limit(g_chg_hardpara_dtsi.charger_current_limit_paras[index_num].current_limit_usb);
#else
            chg_set_current_level(charger_current_limit_paras[index_num].current_limit,
                g_chg_batt_data->is_20pct_calib);
            chg_set_supply_limit(charger_current_limit_paras[index_num].current_limit_usb);
#endif

/*chg_set_current_level在其他充电IC驱动中未定义*/
            break;
        }
    }
}


void chg_high_volt_batt_entry_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_volt_batt_entry_func begin!\n");
    /*充电器类型未识别出来之前不限流防止识别出来后被重新设置*/
    if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        return;
    }

    chg_sub_low_temp_changed = -1;
    if (battry_temp < CHG_SUB_LOW_TEMP_TOP)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_volt_batt_entry_func over!\n");
}


void chg_high_volt_batt_period_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = CHG_HVDCP_5V;
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*充电器类型未识别出来之前不限流防止识别出来后被重新设置*/
    if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        return;
    }

    /* 根据温度区间转换设置充电电流 */
    if (battry_temp < CHG_SUB_LOW_TEMP_TOP && -1 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
    else if (battry_temp >= (CHG_SUB_LOW_TEMP_TOP + CHG_TEMP_RESUM) && 0 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = -1;
        if(CHG_WALL_CHGR == cur_chgr_type)
        {
            chg_set_cur_level(chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal std_chgr current_limit\n");
        }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
        else if(CHG_HVDCP_CHGR == cur_chgr_type)
        {
            (void)chg_set_hvdcp_adpter_vol(g_hvdcp_init_vol);
            chg_delay_ms(1);
            hvdcp_type = chg_get_hvdcp_type();

            if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_supply_limit(chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].pwr_supply_current_limit_in_mA);
                chg_set_cur_level(chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].chg_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal 9v_hvdcp current_limit\n");
            }
            else if( CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_supply_limit(chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].pwr_supply_current_limit_in_mA);
                chg_set_cur_level(chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].chg_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal 12v_hvdcp current_limit\n");
            }
            else
            {
                chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
            }
        }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
        else if(CHG_USB_HOST_PC == cur_chgr_type)
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                chg_set_cur_level(chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal usb3 current_limit\n");

            }
            else
            {
                chg_set_cur_level(chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal usb current_limit\n");
            }
        }
        else
        {
            chg_set_cur_level(chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C weak or nonstd chgr use usb current_limit\n");
        }
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume high batt current in 0-10c!\n");
    }
}
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/


static void chg_fastchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into fast-charge state.\n");

    /*1. Notify UI that battery charging started.*/
    chg_notify_UI_charging_state(CHG_UI_START_CHARGING);

    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*使能充电，设置标志*/
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set std_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }

    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set chg_weak_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_fastchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_fastchg_state_entry_func enable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set usb_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else
    {
        if(TRUE == is_chg_charger_removed())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force fastchg_state_entry to batt_only_st!\n");
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry Chgr type invaild!\n");
        }
    }
    /*3. Enable IC Charge function.*/
    /*Already done in chg_set_hardware_parameter.*/

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_high_volt_batt_entry_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

}


static void chg_fastchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force fastchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(hvdcp_type != CHG_HVDCP_INVALID)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:HVDCP attach fastchg_st to hvdcp_charge_st!\n");
        chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
                chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON)
            if(TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, but soc not 100, need adjust.\n");
            }
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
            /*记录停充时的实际电量*/
            chg_set_recharge_threshold();
#endif
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }

    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while fastchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }
    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Switch fastchg state to warmchg state, tBat=%d'C,tBat_sys=%dC.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    else
    {
        /* 非标准充电器充电电流快充阶段均一致 */
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
        chg_high_volt_batt_period_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
        /*the boost IC affect the charge IC's detection of current,
        so stop charge complete when the voltage and current fulfil requirement*/
        if ((chg_get_sys_batt_volt() >= CHG_TERMI_VOLT_FOR_COUL) && \
            (hisi_battery_current() <= CHG_TERMI_CURR_FOR_COUL))
        {
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            chg_stm_state_info.charging_lasted_in_sconds = 0;
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
            /*记录停充时的实际电量*/
            chg_set_recharge_threshold();
#endif
        }
#endif
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System keep staying at fast charging state.\n");
    }
}


static void chg_fastchg_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from fast-charge state.\n");
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_sub_low_temp_changed = -1;      /*0-10度充电相关参数初始化*/
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
}


static void chg_maint_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into maintenance state.\n");

    /*1. Notify UI that battery charging stopped.*/
#if (MBB_CHG_BQ27510 == FEATURE_ON)
    if (TRUE == chg_is_batt_full())
    {
        chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);
    }
#else
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);
#endif

    /*2. Update charge parameter, set maximal PS. current limit.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else if(CHG_HVDCP_9V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else if(CHG_HVDCP_12V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else
        {
            //reserve
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_maint_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_maint_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_maint_state_period_func(void)
{
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force maint_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    /*if the charger and ext-charge are online at the same time in shutdown charge
    the ext-charge will exhaust the capacity,
    so enable the function of recharge in shutdown charge mode to 
    avoid exhausting the capacity*/
    else if (TRUE == chg_is_batt_need_rechg())
#else
    /*若关机充电超时则不进行复充*/
    else if (TRUE == chg_is_batt_need_rechg() && (FALSE == chg_powdown_charging_time_expired_flag))
#endif
    {
        if (FALSE == chg_is_batt_temp_valid())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery temperature %d invlaid for charge.\n",  \
                       chg_get_sys_batt_temp());
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt_need_rechg force to TRANSIT_ST.\n");
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Keep staying at maint_state.\n");
    }
}


static void chg_maint_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from maintenance state.\n");
}


static void chg_invalid_chg_temp_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into invalid chg temp state.\n");

    /*1. Notify UI that battery charging stopped.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    /*2. Update charge parameter, set maximal PS. current limit.*/
    /*3. Stop charging.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {

        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
        }
        else
        {
           (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
            chg_delay_ms(1);
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:chg_invalid_chg_temp_state_entry_funcset usb 3.0 parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_invalid_chg_temp_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_invalid_chg_temp_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_invalid_chg_temp_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_invalid_chg_temp_state_period_func(void)
{
    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force invalid_chg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    /*2. If the battery temperature resumed?
     ->Y a. If battery OVP detected, switch to maintenance state.
            else switch to fast charge state.
     ->N b. Keep staying at invalid charge temperature state.
     */
    else if (TRUE == chg_is_batt_temp_valid())
    {
        if (chg_is_batt_ovp())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery OVP, batt-volt %dmV while temp resume.\n", \
                       chg_get_sys_batt_volt());
            chg_stm_switch_state(CHG_STM_MAINT_ST);
        }
        else
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Battery temp not resumed, staying at invalid temp.\n");
    }

}


static void chg_invalid_chg_temp_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from invalid chg temp state.\n");
}


static void chg_batt_only_state_entry_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into batt-only state.\n");
    /*1. Notify UI to stop battery charging display.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
    /*2. Set current charger type to invalid, means charger absent.*/
    chg_stm_set_chgr_type(CHG_CHGR_INVALID);

    /*如果已经在切换到单电池状态了，检测到USB或者标准充电器还处于ONLINE，就降其置为OFFLINE*/
    if(TRUE == chg_get_usb_online_status())
    {
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System enter batt-only state USB already OFFLINE!.\n");
    }

    if(TRUE == chg_get_ac_online_status())
    {
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System enter batt-only state AC already OFFLINE!.\n");
    }
    /*3. Config charge hw parameter, disable charge, PS current limit min.*/
    chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_BATTERY_ONLY]);
}


static void chg_batt_only_state_period_func(void)
{
    static uint32_t chg_poll_timer_switch_cnter = 0;
    int32_t         curr_polling_timer_mode     = chg_poll_timer_get();
    boolean         is_emergency_state          = chg_is_emergency_state();
    boolean is_chgr_present      = chg_is_charger_present();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_only_state_period is_chgr_present=%d.\n",
                            is_chgr_present);

    /*Get poll timer period error, do nothing, return.*/
    if (CHG_ERROR == curr_polling_timer_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Get poll timer period error.\n");
        return;
    }

    /*1. If we stay at batt_only state more than CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC
         seconds, switch poll timer to slow mode.
         Notice: DON'T switch to slow timer mode if system in emergency state. e.g.,
                 low power, battery hot/cold, etc, supported by sampling sub-module.*/
    if (FAST_POLL_CYCLE == curr_polling_timer_mode && !is_emergency_state)
    {
        if (chg_poll_timer_switch_cnter >= CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System stay at batt-only state" \
                " more than %d seconds, switch to slow timer mode.\n",
                CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC);
            /*Clear cnter for next count.*/
            chg_poll_timer_switch_cnter = 0;
            chg_poll_timer_set(SLOW_POLL_CYCLE);
        }
        else
        {
            chg_poll_timer_switch_cnter += ((uint32_t)curr_polling_timer_mode / MS_IN_SECOND);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Poll switch cnter = %d.\n", chg_poll_timer_switch_cnter);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:%s polling mode, system %s at emergency State.\n",
            (curr_polling_timer_mode == FAST_POLL_CYCLE) ? "Fast" : "Slow",
            (is_emergency_state) ? "is" : "isn't");

        /*Clear cnter for next count.*/
        chg_poll_timer_switch_cnter = 0;
    }

#ifdef CHG_STUB
    /*充放电代码移植初期，如果USB驱动充电器类型检测没有实现
      将充电器类型写死，同时在单电池状态通过检测电源芯片来判断
      充电器是否在位*/
    if( TRUE == is_chgr_present )
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:CHG_STUB:batt_only_state_period: CHGR IS PRESENT switch BATT_ONLY to TRANSIT_ST!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
    }
#endif/*CHG_STUB*/


#if (MBB_CHG_WIRELESS == FEATURE_ON)
    if(ONLINE == g_wireless_online_flag)
    {
        chg_stm_set_wireless_online_st(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
        chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:wireless online change to CHG_WIRELESS_MODE!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System staying at batt-only state.\n");
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_batt_only_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from batt-only state.\n");
}

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state)
{
    chg_stm_state_info.pre_stm_state = pre_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_pre_stm_state=%d\n",pre_state);
}


chg_stm_state_type chg_stm_get_pre_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_pre_stm_state=%d\n",\
                        chg_stm_state_info.pre_stm_state);
    return chg_stm_state_info.pre_stm_state;
}


boolean is_batttemp_in_warm_chg_area( void )
{
    boolean rtnValue = FALSE;
    static boolean last_rtnValue = FALSE;

    /*如果前一个状态是电池单独在位状态，说明发生了插拔外电源事件，
       将过温状态记录变量恢复到默认值不过温*/
    if (CHG_STM_BATTERY_ONLY == chg_stm_get_pre_state())
    {
        last_rtnValue = FALSE;
    }

    if ( FALSE == last_rtnValue )
    {
        /*电池温度大于等于45度小于55度为高温充电区*/
        if ( ( CHG_WARM_CHARGE_ENTER_THRESHOLD <= chg_real_info.battery_temp ) \
             && ( CHG_OVER_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp ) )
        {
            rtnValue = TRUE;
        }
        else
        {
            rtnValue = FALSE;
        }
    }
    else
    {
        /*小于42度或者大于55度单板不在高温充电区间 */
        if ( (CHG_WARM_CHARGE_EXIT_THRESHOLD > chg_real_info.battery_temp)\
             || (CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp))
        {
            rtnValue = FALSE;
        }
        else
        {
            rtnValue = TRUE;
        }
    }

    last_rtnValue = rtnValue;
    return rtnValue;
}


static void chg_warmchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into warm-charge state.\n");

    /*1.If the pre chg state is not CHG_STM_FAST_CHARGE_ST Notify UI
       that battery charging started.*/
    if ( CHG_STM_FAST_CHARGE_ST != chg_stm_get_pre_state())
    {
        chg_notify_UI_charging_state(CHG_UI_START_CHARGING);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
        else
        {
           (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
            chg_delay_ms(1);
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*使能充电，设置标志*/
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;

        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]:set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_warmchg_state_entry_func enable wireless chg!\n");
        /*开机无线充电设置正常的高温充电参数*/
        if(FALSE == chg_is_powdown_charging())
        {
            chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
        /*关机无线充电设置关机的高温充电参数，前端限流900MA,充电限流1024MA,恒压电压4.1V
            截止电流256MA*/
        else
        {
            chg_set_hardware_parameter(&chg_wireless_poweroff_warmchg_paras);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_warmchg_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_warmchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;

    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force warmchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }

    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON)
            if (TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
            /*高温充电停充时保存停充时的电量，并计算出复充门限*/
            chg_set_recharge_threshold();
#endif
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }
    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while warmchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while warmchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
    /*5.battery temp resume switch to fastchg state*/
    else if(FALSE == is_batttemp_in_warm_chg_area())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt-temp resume form warmchg to fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
        chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        if(CHG_HVDCP_CHGR == cur_chgr_type)
        {
            chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
        }
        else
        {
            chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
        }
    }
    else
    {
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
        /*the boost IC affect the charge IC's detection of current,
        so stop charge complete when the voltage and current fulfil requirement*/
        if ((chg_get_sys_batt_volt() >= CHG_TERMI_WARMCHG_VOLT_FOR_COUL) && \
            (hisi_battery_current() <= CHG_TERMI_CURR_FOR_COUL))
        {
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            chg_stm_state_info.charging_lasted_in_sconds = 0;
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
            /*高温充电停充时保存停充时的电量，并计算出复充门限*/
            chg_set_recharge_threshold();
#endif
        }
#endif
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at warmchg state.\n");
    }

}


static void chg_warmchg_state_exit_func(void)
{
     chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from warmchg state.\n");

}
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)

static void chg_set_recharge_threshold(void)
{
    g_chg_rechg_threshod = hisi_battery_capacity() - (BATT_CAPACITY_FULL - BATT_CAPACITY_RECHG);
}


int32_t chg_get_recharge_threshold(void)
{
    return g_chg_rechg_threshod;
}
#endif /*BSP_CONFIG_BOARD_E5885Ls_93a*/

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)

static void chg_hvdcpchg_state_entry_func(void)
{
    int32_t cur_batt_volt =  chg_get_cur_batt_volt();
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into hvdcp-charge state.\n");

    /*1. Notify UI that battery charging started.*/
    chg_notify_UI_charging_state(CHG_UI_START_CHARGING);

    /*2. Set HVDCP adapter voltage.*/
    (void)chg_set_hvdcp_adpter_vol(g_hvdcp_init_vol);
    chg_delay_ms(1);
    hvdcp_type = chg_get_hvdcp_type();

    /*3. Set charging hardware parameter according to HVDCP voltage type.*/
    if(CHG_HVDCP_9V == hvdcp_type)
    {
        chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }
    else if( CHG_HVDCP_12V == hvdcp_type)
    {
        chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }


    if(cur_batt_volt <= HVDCP_CHARGE_SET_CV_COMPARE_TH)
    {
        chg_set_vreg_level(HVDCP_CHARGE_CV_4200MV);
    }
    else
    {
        chg_set_vreg_level(HVDCP_CHARGE_CV_4350MV);
        chg_set_cur_level(HVDCP_CHARGE_0P5_CURRENT_TH);

        g_hvdcp_charge_currnt_limit_flag = TRUE;
    }

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_high_volt_batt_entry_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
}


static void chg_hvdcpchg_state_period_func(void)
{

    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;
    uint32_t charge_current         = chg_get_hvdcp_charge_current();

    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chgr removed force hvdcpchg_state to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    else if((charge_current <= HVDCP_CHARGE_0P5_CURRENT_TH)
        && (FALSE == g_hvdcp_charge_currnt_limit_flag))
    {
        g_hvdcp_charge_currnt_limit_flag = TRUE;
        chg_set_vreg_level(HVDCP_CHARGE_CV_4350MV);
        chg_set_cur_level(HVDCP_CHARGE_0P5_CURRENT_TH);
/*chg_set_current_level在其他充电IC驱动中未定义*/
    }
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
                chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
            chg_stm_switch_state(CHG_STM_MAINT_ST);
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }

    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while hvdcpchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }
    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while hvdcpchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Switch hvdcpchg state to warmchg state, tBat=%d'C,tBat_sys=%dC.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    else
    {
        /* 非标准充电器充电电流快充阶段均一致 */
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
        chg_high_volt_batt_period_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System keep staying at hvdcpchg charging state.\n");
    }

}


static void chg_hvdcpchg_state_exit_func(void)
{
    g_hvdcp_charge_currnt_limit_flag = FALSE;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from hvdcp-charge state.\n");
}
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)

void extchg_stop_threshold_to_voltage(int32_t capacity)
{
    switch (capacity)
    {
        case EXTCHG_DEFAULT_STOP_THRESHOLD:
            g_extchg_voltage_threshold = EXTCHG_DEFAULT_STOP_VOLTAGE;
            break;
        case EXTCHG_STOP_CAPACITY_TEN:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TEN;
            break;
        case EXTCHG_STOP_CAPACITY_TWENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TWENTY;
            break;
        case EXTCHG_STOP_CAPACITY_THIRTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_THIRTY;
            break;
        case EXTCHG_STOP_CAPACITY_FORTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FORTY;
            break;
        case EXTCHG_STOP_CAPACITY_FIFTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FIFTY;
            break;
        case EXTCHG_STOP_CAPACITY_SIXTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SIXTY;
            break;
        case EXTCHG_STOP_CAPACITY_SEVENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SEVENTY;
            break;
        case EXTCHG_STOP_CAPACITY_EIGHTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_EIGHTY;
            break;
        case EXTCHG_STOP_CAPACITY_NINETY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_NINETY;
            break;
        case EXTCHG_STOP_CAPACITY_HUNDRED:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_HUNDRED;
            break;
        default:
            break;
    }
}


void chg_extchg_config_data_init(void)
{
    int32_t fd    = 0;
    mm_segment_t fs;
    char extchg_threshold_temp = 0;
    char extchg_diable_st_temp = 0;
    int32_t extchg_capacity_threshold = 0;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fd = sys_open(EXTCHG_THRESHOLD_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_threshold_temp, 1);
        extchg_capacity_threshold = extchg_threshold_temp;
        chg_print_level_message(CHG_MSG_INFO,"extchg_capacity_threshold=%d.\n", extchg_capacity_threshold);
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        sys_close(fd);
    }
    else
    {
        extchg_capacity_threshold = EXTCHG_DEFAULT_STOP_THRESHOLD;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_threshold read FAIL use default value 5!\n");
    }


    fd = sys_open(EXTCHG_DISABLE_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_diable_st_temp, 1);
        g_extchg_diable_st = extchg_diable_st_temp;
        chg_print_level_message(CHG_MSG_INFO,"g_extchg_diable_st=%d.\n", g_extchg_diable_st);
        sys_close(fd);
    }
    else
    {
        g_extchg_diable_st = 0;
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_diable_st read FAIL use default value 0!\n");
    }
    set_fs(fs);
}


boolean chg_get_extchg_online_status(void)
{
    int32_t vbus_volt = 0;

    //TO DO:1:调用USB驱动接口短接D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);

    /*使能无线充电*/
    chg_set_extchg_chg_enable(TRUE);

    /*延时500MS,防止使能充电后因为电路延迟导致采集电池电压异常*/
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:chg_delay_ms 500MS! \n");
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
    chg_print_level_message( CHG_MSG_INFO,"CHG_DRV:get_extchg_online vbus_volt = %d\r\n ",vbus_volt);
    if(vbus_volt > VBUS_JUDGEMENT_THRESHOLD)
    {
        return ONLINE;
    }
    else
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: VBUS detect failure extchg is not online\r\n ");
        return OFFLINE;
    }
}

#endif /* MBB_CHG_EXTCHG */


int32_t chg_get_batt_id_volt(void)
{
    if(0 == chg_real_info.battery_id_volt)
    {
        chg_real_info.battery_id_volt = chg_get_volt_from_adc(CHG_PARAMETER__BATTERY_ID_VOLT);
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:chg_get_batt_id_volt id_volt=%d\n",chg_real_info.battery_id_volt);
    return chg_real_info.battery_id_volt;
}


int32_t chg_get_bat_status(void)
{
    int32_t bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
    chg_stm_state_type stm_status = CHG_STM_MAX_ST;

    stm_status = chg_stm_get_cur_state();
    if((CHG_STM_INIT_ST == stm_status) || (CHG_STM_TRANSIT_ST == stm_status)
        || (CHG_STM_INVALID_CHG_TEMP_ST == stm_status)
        || (CHG_STM_BATTERY_ONLY == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_NOT_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NOT_CHARGING\n");
    }
    else if ((CHG_STM_FAST_CHARGE_ST == stm_status)
            || (CHG_STM_WARMCHG_ST == stm_status)
            || (CHG_STM_HVDCP_CHARGE_ST == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=CHARGING\n");
    }
    else if(CHG_STM_MAINT_ST == stm_status)
    {
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        if(BATT_CAPACITY_FULL == chg_get_sys_batt_capacity())
        {
            bat_stat_t = POWER_SUPPLY_STATUS_FULL;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
        }
#if (MBB_CHG_BQ27510 == FEATURE_ON)
        else
        {
            bat_stat_t = POWER_SUPPLY_STATUS_CHARGING;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=CHARGING\n");
        }
#endif
#else
        bat_stat_t = POWER_SUPPLY_STATUS_FULL;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    }
    else
    {
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
        if(POWER_SUPPLY_STATUS_NEED_SUPPLY == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_NEED_SUPPLY;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NEED_SUPPLY\n");
        }
        else if(POWER_SUPPLY_STATUS_SUPPLY_SUCCESS == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=SUPPLY_SUCCESS\n");
        }
        else
        {
            bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
        }
#else
        bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
#endif/*MBB_CHG_COMPENSATE == FEATURE_ON*/
    }
    return bat_stat_t;
}


int32_t chg_get_bat_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.bat_heath_type;
}


void chg_set_extchg_status(int32_t extchg_status)
{
    chg_stm_state_info.extchg_status = extchg_status;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg_status=%d\n",chg_stm_state_info.extchg_status);
}
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))

void chg_check_vbus_volt(void)
{

    int32_t checkstarttime = 0;
    int32_t vbus_volt = 0;
    int32_t checkvbustime = 0;

    msleep(COUL_CHECK_CURRENT_WAIT_MS);
    /*最大循环读取Vbus电压10次，总计5s*/
    while(COUL_CHECK_CURRENT_TIME_MAX > checkstarttime)
    {
        checkstarttime++;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:checkstarttime=%d\n",checkstarttime);
        msleep(COUL_CHECK_CURRENT_DELAY_MS);
        /*检测到Vbus电压拉高后循环读取3次，防止电压跳变误测*/
        while(checkvbustime < COUL_CHECK_VBUS_TIME_MAX)
        {
            checkvbustime++;
            vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:vbus_volt=%d\n",vbus_volt);
            if(VBUS_JUDGEMENT_THRESHOLD >= vbus_volt)
            {
                break;
            }
            else
            {
                //do nothing
            }
            msleep(COUL_CHECK_VBUS_DELAY_MS);
        }
        /*判断3次Vbus电压是否都是被正确拉高*/
        if(COUL_CHECK_VBUS_TIME_MAX == checkvbustime)
        {
            g_extchg_start = 1;
            return;
        }
        else
        {
            g_extchg_start = 0;
        }
    }
}


int32_t chg_get_extchg_start(void)
{
    return g_extchg_start;
}
#endif


int32_t chg_get_extchg_status(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    return chg_stm_state_info.extchg_status;
}


void chg_charge_paras_init(void)
{
    switch(g_hardware_version_id)
    {
        /*产品根据版本号适配对应电池的充电参数，其他和产品硬件相关的参数均可在这里配置*/
        case HW_VER_PRODUCT_E5785Lh_92a:
        case HW_VER_PRODUCT_E5785Lh_22c:
        case HW_VER_PRODUCT_E5787Ph_67a:
        case HW_VER_PRODUCT_R227h:
        case HW_VER_PRODUCT_E5785Lh_23c:
        case HW_VER_PRODUCT_E5785Lh_67a:
        {
            chg_std_chgr_hw_paras  = chg_std_2A_chgr_hw_paras_def;
            chg_usb_chgr_hw_paras  = chg_usb_chgr_hw_paras_def;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_def;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
            g_default_dpm_volt     = 4520;/*配置默认DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read E5785Lh_92a chg parameter.\n");
            break;
        }
        case HW_VER_PRODUCT_E5885Ls_93a:
        {
            chg_std_chgr_hw_paras  = chg_std_2A_chgr_hw_paras_def;
            chg_usb_chgr_hw_paras  = chg_usb_chgr_hw_paras_def;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_def;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
            g_default_dpm_volt     = 4520;/*配置默认DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read E5885Ls_93a chg parameter.\n");
            break;
        }
        case HW_VER_PRODUCT_E5787Ph_92a:
        {
            chg_std_chgr_hw_paras  = chg_std_2A_chgr_hw_paras_4800mAh_batt;
            chg_usb_chgr_hw_paras  = chg_usb_chgr_hw_paras_4800mAh_batt;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_4800mAh_batt;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
            g_default_dpm_volt     = 4520;/*配置默认DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read E5785Ph_92a chg parameter.\n");
            break;
        }
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
/*电池二进制归一的产品要使用设备树中充电参数，在特性宏下添加产品分支*/
        case HW_VER_PRODUCT_E5783h_92a:
        {
            chg_std_chgr_hw_paras  = g_chg_hardpara_dtsi.chg_std_chgr_hw_paras;
            chg_usb_chgr_hw_paras  = g_chg_hardpara_dtsi.chg_usb_chgr_hw_paras;
            chg_usb3_chgr_hw_paras = g_chg_hardpara_dtsi.chg_usb3_chgr_hw_paras;
            chg_weak_chgr_hw_paras = g_chg_hardpara_dtsi.chg_weak_chgr_hw_paras;
            g_default_dpm_volt     = g_chg_hardpara_dtsi.g_default_dpm_volt;/*配置默认DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read E5783h_92a chg parameter \n");
            break;
        }
#endif
        default:
        {
            chg_std_chgr_hw_paras = chg_std_1A_chgr_hw_paras_def;
            chg_usb_chgr_hw_paras = chg_usb_chgr_hw_paras_def;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_def;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
#if (MBB_CHG_WIRELESS == FEATURE_ON)
            chg_wireless_chgr_hw_paras = chg_wireless_chgr_hw_paras_def;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
            g_default_dpm_volt = 4520;/*配置默认DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Use default chg parameter!.\n");
            break;
        }
    }
}


int32_t chg_stm_init(void)
{
    static boolean chg_stm_inited = FALSE;
    chg_stm_state_type cur_chg_state = chg_stm_get_cur_state();
    chg_chgr_type_t cur_chg_type = chg_stm_get_chgr_type();
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
    chg_stm_state_type pre_chg_state = chg_stm_get_pre_state();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    boolean is_extchg_online   = chg_stm_get_extchg_online_st();
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

    if (FALSE == chg_stm_inited)
    {
        /*Clear fast charge sw protection timer(cnter).*/

        chg_stm_state_info.charging_lasted_in_sconds = 0;
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
#endif
        /*Initialize default chgr type and stm state. */
        if((cur_chg_mode <= CHG_MODE_INIT) || (cur_chg_mode >= CHG_MODE_UNKNOW))
        {
            chg_set_cur_chg_mode(CHG_MODE_UNKNOW);
        }
        if((cur_chg_type <= CHG_CHGR_UNKNOWN) || (cur_chg_type >= CHG_CHGR_INVALID))
        {
            chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        }
        if((cur_chg_state <= CHG_STM_INIT_ST) || (cur_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_cur_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
        if((pre_chg_state <= CHG_STM_INIT_ST) || (pre_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_pre_state(CHG_STM_TRANSIT_ST);
        }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        if(TRUE != is_extchg_online)
        {
            chg_stm_set_extchg_online_st(FALSE);
        }
#endif /* MBB_CHG_EXTCHG == FEATURE_ON */

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE != is_wireless_online)
        {
            chg_stm_set_wireless_online_st(FALSE);
        }
#endif /* MBB_CHG_WIRELESS == FEATURE_ON */

        if(CHG_STM_INIT_ST == cur_chg_state)
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM initial state:\n");
        mlog_print(MLOG_CHG, mlog_lv_info, "current charge state: %d, pre-chargestate: %d.\n",
                   cur_chg_state, pre_chg_state);
        mlog_print(MLOG_CHG, mlog_lv_info, "charger type: %d, charge mode: %d.\n",
                   cur_chg_type, cur_chg_mode);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "extchg_online: %d.\n", is_extchg_online);
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "wireless_online: %d.\n", is_wireless_online);
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge State Machine init successfully.\n");
        chg_stm_inited = TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge STM init be called more than one time.\n");
    }

    return CHG_OK;
}


void chg_set_usb_online_status(boolean online)
{
    chg_stm_state_info.usb_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.usb_online_st=%d\n",online);
}


boolean chg_get_usb_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_usb_online_status=%d\n",\
                chg_stm_state_info.usb_online_st);
    return chg_stm_state_info.usb_online_st;
}


void chg_set_ac_online_status(boolean online)
{
    chg_stm_state_info.ac_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.ac_online_st=%d\n",online);
}


boolean chg_get_ac_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_ac_online_status=%d\n",\
                chg_stm_state_info.ac_online_st);
    return chg_stm_state_info.ac_online_st;
}


boolean chg_get_bat_present_status(void)
{
    if(TRUE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=PRESENT\n");
        return PRESENT;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=UNPRESENT\n");
        return UNPRESENT;
    }
    return PRESENT;
}


void chg_update_power_suply_info(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "******CHG_STM:update_power_suply_info begin!******\n");
    if (NULL == g_chip)
    {
        return;
    }
    g_chip->ac_online = chg_get_ac_online_status();
    g_chip->usb_online = chg_get_usb_online_status();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    g_chip->wireless_online = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    g_chip->extchg_online = chg_stm_get_extchg_online_st();
    g_chip->extchg_status = chg_get_extchg_status();
#if(FEATURE_ON == MBB_FACTORY) && (FEATURE_ON == MBB_CHG_COULOMETER)
    g_chip->extchg_start = chg_get_extchg_start();
#endif
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
    g_chip->bat_present = chg_get_bat_present_status();
    g_chip->bat_stat = chg_get_bat_status();
    g_chip->bat_health = chg_get_bat_health();
    g_chip->bat_technology = POWER_SUPPLY_TECHNOLOGY_LION;
    g_chip->bat_avg_voltage = chg_get_sys_batt_volt();
    g_chip->bat_avg_temp = chg_get_sys_batt_temp();
    g_chip->bat_capacity = chg_get_sys_batt_capacity();
    g_chip->bat_time_to_full = chg_get_batt_time_to_full();
    /*USB温保*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    g_chip->usb_health = chg_get_usb_health();
#endif

    chg_print_level_message(CHG_MSG_INFO, "******CHG_STM:update_power_suply_info successfully!******\n");
}


void chg_print_test_view_info(void)
{
    static uint8_t count = 0;

    /*快轮询4个周期打印一次充电信息，慢轮询每周期打印一次*/
    if ((FAST_POLL_CYCLE == chg_poll_timer_get()) && (count < 3))
    {
        count++;
        return;
    }
    
    count = 0;
    chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info Begin**********\n");
    /*打印当前所处的充电状态*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_stm_state=%d\n",
                         chg_stm_state_info.cur_stm_state);
    /*打印当前充电器类型*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charger_type=%d\n",
                        chg_stm_state_info.cur_chgr_type);
    /*打印当前所处的充电模式，有线，无线还是对外充电*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_chg_mode=%d\n",
                        chg_stm_state_info.cur_chg_mode);
    /*打印当前充电持续时间*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charging_lasted_time=%d\n",
                        chg_stm_state_info.charging_lasted_in_sconds);
    /*打印单次电池温度*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_temp = %d\n",
                        chg_real_info.battery_one_temp);
     /*打印平滑后的电池温度*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_temp = %d\n",
                        chg_real_info.battery_temp);

    /*打印电池单次采集电压*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_volt = %d\n",
                        chg_real_info.battery_one_volt);
    /*打印电池电压平滑值*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_volt = %d\n",
                        chg_real_info.battery_volt);
    /*打印VPH_PWR单次采集电压*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.vph_pwr_one_volt=%d\n",
                        chg_real_info.vph_pwr_one_volt);
    /*打印VPH_PWR电压平滑值*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.vph_pwr_avg_volt= %d\n",
                        chg_real_info.vph_pwr_avg_volt);
    /*打印电池电量百分比*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.bat_capacity = %d\n",
                        chg_real_info.bat_capacity);
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    /*Dump current surface temperature.*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_DCM: TSurf: %d'C, TSurf_sys = %d'C.\n",
        g_surf_temp_state.cur_surf_temp, g_surf_temp_state.sys_surf_temp);
#endif
    /*打印USB当前温度*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_usb_cur_temp = %d'C.\n",
                        chg_usb_temp_info.usb_cur_temp);
#endif
    chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info End**********\n");
}

#ifdef CONFIG_MBB_FAST_ON_OFF
#if(MBB_REB == FEATURE_ON)

unsigned int chg_is_bat_only(void)
{
    return ( CHG_BAT_ONLY_MODE == chg_stm_state_info.cur_chg_mode );
}


int get_low_bat_level(void)
{
    /*要在充电模块初始化以后调用*/
    return g_chgBattVoltProtect.VbatLevelLow_MAX;
}
#endif/*MBB_REB*/


void chg_get_system_suspend_status(ulong64_t suspend_status)
{
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    if(suspend_status)
    {
        /*进入假关机，模拟USB ID拔出*/
        if(TRUE == chg_stm_get_extchg_online_st())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg online stop extchg!\n");
            /*模拟USB ID拔出*/
            (void)battery_monitor_blocking_notifier_call_chain(0, CHG_EXGCHG_CHGR);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg offline do nothing!\n");
        }
    }
    else
    {
        /*退出假关机，由USB重新上报 ID中断，充电模块不做处理*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Exit Fastboot extchg offline do nothing!\n");
    }
#endif
}
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*低电中断误报门限*/
#define LOW_BATT_DET_THRESHOLD    (3450)

void chg_low_battery_event_handler(void)
{
    int32_t batt_volt = chg_get_batt_volt_value();

    if(TRUE == chg_is_powdown_charging()
        || TRUE == chg_is_low_battery_poweroff_disable())
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:no need to care，set bypass!\n");
        return;
    }
    if(LOW_BATT_DET_THRESHOLD < batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();
        if(LOW_BATT_DET_THRESHOLD < batt_volt)
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:invalid event，set bypass!\n");
            return;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:battery is low power，system will shutdown!\n");
    wake_lock_timeout(&g_chip->alarm_wake_lock,(long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
    chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, chg_set_power_off, DRV_SHUTDOWN_LOW_BATTERY);
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)

void chg_sleep_batt_check_timer(void)
{
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;

    wake_lock(&g_chip->chg_wake_lock);
    #ifdef CONFIG_COUL
    /*进行一次库仑计强制校准*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bsp_coul_cail_on COUL_CAIL_ON");
    bsp_coul_cail_on();
    msleep(COUL_READY_DELAY_MS);
    #endif
    batt_volt = (int32_t)chg_get_batt_volt_value();
    batt_temp = (int32_t)chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    /*添加温度补偿，保证温度采集的准确性*/
    batt_temp = chg_batt_temp_revise(batt_temp);
#endif

    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback, batt_volt %d,batt_temp %d!\n",
           batt_volt, batt_temp);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*有线/无线/对外充电器在位不进行低电或者过温处理*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    /*if the ext-charge is online, need to check the capacity and temperature*/
    if (FALSE == is_chg_charger_removed())
#else
    if ((FALSE == is_chg_charger_removed()) || (TRUE == chg_stm_get_extchg_online_st()))
#endif
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#else
    if (FALSE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#endif/*defined(MBB_CHG_EXTCHG)*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    /*轮询soc，以便应用低电提示*/
    chg_poll_batt_soc();
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
    {
        /*低电关机紧急状态，系统稍后关机*/
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,\
        "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }
#elif (MBB_CHG_BQ27510 == FEATURE_ON)
    /*just monitor batt volt*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if((batt_volt < BATT_VOLT_POWER_OFF_THR) && (FALSE == chg_is_powdown_charging()))
#else
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
#endif
    {
        /*低电关机紧急状态，系统稍后关机*/
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,\
        "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }
#else
    /* 如果电池电压大于低电门限则什么都不做，否则将相关事件上报应用 */
    if (BATT_VOLT_LEVELLOW_MAX < batt_volt)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_volt > low battery threshold do nothing!*****\n");
    }
    else if ((BATT_VOLT_POWER_OFF_THR < batt_volt) && (BATT_VOLT_LEVELLOW_MAX >= batt_volt))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_volt is in 3450-3550mV report low battery*****\n");
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show low power! \n ");
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
        chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    }
#endif
    /* 如果温度保护不使能 或者 电池温度在正常范围则什么都不做，否则将相关事件上报应用 */
    if ((((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) > batt_temp)
        && (SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD < batt_temp))
        || (FALSE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is in -20-58 deg do nothing*****\n");
    }
    else if ((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) <= batt_temp)
    {
        if (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= batt_temp)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is more than 60 deg, power off*****\n");
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is in 58-60 deg, report overheat*****\n");
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show overheat! \n ");
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is less than -20 deg, report cold*****\n");
        chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
    }

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    extchg_status_update_in_sleep_mode(batt_volt, batt_temp);
#endif

    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:wait next modem time expire!\n");

end:
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    bsp_softtimer_add(&g_chg_sleep_timer);
    wake_unlock(&g_chip->chg_wake_lock);
#endif
}
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

void chg_extchg_info_dump(void)
{
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_diable_st=%d\n",g_extchg_diable_st);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:ui_mode=%d\n",g_ui_choose_exchg_mode);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:otg_extchg=%d\n",is_otg_extchg);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is_extchg_ovtemp=%d\n",is_extchg_ovtemp);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is_batt_volt_low=%d\n",is_batt_volt_low);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_online_st=%d\n",chg_stm_state_info.extchg_online_st);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:exchg_enable=%d\n",g_exchg_enable_flag);
}


static void chg_set_charge_otg_mode(void)
{
    /*将充电芯片设置为 非OTG模式，5V 下电*/
    chg_set_charge_otg_enable(FALSE);
    /*延时500MS*/
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    /*为了避免在芯片下电前至VBUS 5V下电前再次发生OCP，故下电后重新读取寄存器REG09确保在芯片重新上电前BAT_FAULT位清零*/
    (void)chg_extchg_ocp_detect();
    /*TO DO:调用USB驱动接口直连基带，拉低HS_ID*/
    usb_notify_event(USB_OTG_DISCONNECT_DP_DM,NULL);
    /*延时500MS*/
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    /*将充电芯片设置为OTG模式5V上电*/
    chg_set_charge_otg_enable(TRUE);
    /*至此对外充电切换为OTG 500MA充电模式*/
#if defined(BSP_CONFIG_BOARD_E5785Lh_92a)
    /*切换到OTG模式计数清零*/
    g_covert_to_otg_counter = 0;
#endif
    is_otg_extchg = TRUE;
}

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)

int32_t chg_get_revise_temp_from_map(int32_t batt_temp, extchg_batt_temp_revise* tem_revise_map, uint32_t map_size)
{
    int32_t loop = 0;

    if (NULL == tem_revise_map)
    {
        chg_print_level_message( CHG_MSG_ERR, \
                                 "EXTCHG: get tem revise map failed!!\r\n ");
        return batt_temp;
    }

    for (loop = 0; loop < map_size; loop++)
    {
        if ((batt_temp >= tem_revise_map[loop].tem_min) && (batt_temp <= tem_revise_map[loop].tem_max))
        {
            batt_temp += tem_revise_map[loop].revise_tem;
            return batt_temp;
        }
    }

    return batt_temp;
}


int32_t chg_batt_temp_revise(int32_t batt_temp)
{
    boolean charger_status = FALSE;
    int extcharge_status = EXTCHG_OFFLINE;
    int batt_capacity = chg_get_sys_batt_capacity();

    charger_status = is_chg_charger_removed();
    extcharge_status = extchg_is_extcharge_device_on();

    battery_current = batt_revise_calc_average_curr_value(hisi_battery_current());

    /*单对内充电在位的场景*/
    if ((FALSE == charger_status) && (EXTCHG_OFFLINE == extcharge_status))
    {
        batt_temp = chg_get_revise_temp_from_map(batt_temp, g_batt_tem_revise_map_only_chg, \
                    sizeof(g_batt_tem_revise_map_only_chg) / sizeof(g_batt_tem_revise_map_only_chg[0]));
    }
    /*单对外充电在位的场景*/
    else if ((TRUE == charger_status) && (EXTCHG_ONLINE == extcharge_status))
    {
        if (((batt_capacity >= EXTCHG_TEMP_REVISE_CAPACITY_SEVENTYFIVE) &&
             (battery_current <= EXTCHG_TEMP_REVIEW_CURRENT_2800MA)) ||
            ((batt_capacity >= EXTCHG_TEMP_REVISE_CAPACITY_FIFTY) &&
             (batt_capacity < EXTCHG_TEMP_REVISE_CAPACITY_SEVENTYFIVE) &&
             (battery_current <= EXTCHG_TEMP_REVIEW_CURRENT_3000MA)) ||
            ((batt_capacity >= EXTCHG_TEMP_REVISE_CAPACITY_THIRTY) &&
             (batt_capacity < EXTCHG_TEMP_REVISE_CAPACITY_FIFTY) &&
             (battery_current <= EXTCHG_TEMP_REVIEW_CURRENT_3200MA)))
        {
            batt_temp = chg_get_revise_temp_from_map(batt_temp, g_batt_tem_revise_map_only_extchg_high_crt, \
                        sizeof(g_batt_tem_revise_map_only_extchg_high_crt) / sizeof(g_batt_tem_revise_map_only_extchg_high_crt[0]));
        }
        else
        {
            batt_temp = chg_get_revise_temp_from_map(batt_temp, g_batt_tem_revise_map_only_extchg_low_crt, \
                        sizeof(g_batt_tem_revise_map_only_extchg_low_crt) / sizeof(g_batt_tem_revise_map_only_extchg_low_crt[0]));
        }
    }
    /*对内对外充电同时在位的场景*/
    else if ((FALSE == charger_status) && (EXTCHG_ONLINE == extcharge_status))
    {
        batt_temp = chg_get_revise_temp_from_map(batt_temp, g_batt_tem_revise_map_chg_extchg, \
                    sizeof(g_batt_tem_revise_map_chg_extchg) / sizeof(g_batt_tem_revise_map_chg_extchg[0]));
    }
    /*对内对外充电都不在位的场景*/
    else
    {
        batt_temp = chg_get_revise_temp_from_map(batt_temp, g_batt_tem_revise_map_only_batt, \
                    sizeof(g_batt_tem_revise_map_only_batt) / sizeof(g_batt_tem_revise_map_only_batt[0]));
    }

    return batt_temp;
}


int32_t batt_revise_calc_average_curr_value(int32_t new_data)
{
    int32_t index = 0;
    int32_t sum = 0;
    int32_t bat_avg_curr = 0;
    static uint32_t record_avg_num = 0;
    static int32_t record_value[CHG_BAT_TEMP_REVISE_SAMPLE] = {0};

    if(CHG_BAT_TEMP_REVISE_SAMPLE > record_avg_num)
    {
        record_value[record_avg_num] = new_data;
        record_avg_num++;

        for(index = 0;index < record_avg_num; index++)
        {
            sum += record_value[index];
        }

        bat_avg_curr = sum / record_avg_num;
    }
    else
    {
        record_value[record_avg_num % CHG_BAT_TEMP_REVISE_SAMPLE] 
            = new_data;
        record_avg_num++;

        for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE; index++)
        {
            sum += record_value[index];
        }

        bat_avg_curr = sum / CHG_BAT_TEMP_REVISE_SAMPLE;

        if(CHG_BAT_TEMP_REVISE_SAMPLE * 2 == record_avg_num)
        {
            record_avg_num = CHG_BAT_TEMP_REVISE_SAMPLE;
        }
    }

    return bat_avg_curr;
}


int32_t batt_revise_calc_average_volt_value(int32_t new_data)
{
    int32_t index = 0;
    int32_t sum = 0;
    int32_t bat_avg_volt = 0;
    static uint32_t record_avg_num = 0;
    static int32_t record_value[CHG_BAT_TEMP_REVISE_SAMPLE] = {0};

    if(CHG_BAT_TEMP_REVISE_SAMPLE > record_avg_num)
    {
        record_value[record_avg_num] = new_data;
        record_avg_num++;

        for(index = 0;index < record_avg_num; index++)
        {
            sum += record_value[index];
        }

        bat_avg_volt = sum / record_avg_num;
    }
    else
    {
        record_value[record_avg_num % CHG_BAT_TEMP_REVISE_SAMPLE] 
            = new_data;
        record_avg_num++;

        for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE; index++)
        {
            sum += record_value[index];
        }

        bat_avg_volt = sum / CHG_BAT_TEMP_REVISE_SAMPLE;

        if(CHG_BAT_TEMP_REVISE_SAMPLE * 2 == record_avg_num)
        {
            record_avg_num = CHG_BAT_TEMP_REVISE_SAMPLE;
        }
    }

    return bat_avg_volt;
}


int32_t batt_revise_calc_average_temp_value(int32_t new_data)
{
    int32_t index = 0;
    int32_t sum = 0;
    int32_t bat_avg_temp = 0;
    static uint32_t record_avg_num = 0;
    static int32_t record_value[CHG_BAT_TEMP_REVISE_SAMPLE] = {0};

    if(CHG_BAT_TEMP_REVISE_SAMPLE > record_avg_num)
    {
        record_value[record_avg_num] = new_data;
        record_avg_num++;

        for(index = 0;index < record_avg_num; index++)
        {
            sum += record_value[index];
        }

        bat_avg_temp = sum / record_avg_num;
    }
    else
    {
        record_value[record_avg_num % CHG_BAT_TEMP_REVISE_SAMPLE] 
            = new_data;
        record_avg_num++;

        for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE; index++)
        {
            sum += record_value[index];
        }

        bat_avg_temp = sum / CHG_BAT_TEMP_REVISE_SAMPLE;

        if(CHG_BAT_TEMP_REVISE_SAMPLE * 2 == record_avg_num)
        {
            record_avg_num = CHG_BAT_TEMP_REVISE_SAMPLE;
        }
    }

    return bat_avg_temp;
}


static irqreturn_t extchg_plug_in_out_isr(int ext_dev_id)
{
    if(EXTCHG_ONLINE == extchg_is_extcharge_device_on())
    {
        schedule_work(&extchg_plug_in_work);
    }
    else
    {
        schedule_work(&extchg_plug_out_work);
    }

    return IRQ_HANDLED;
}

int extchg_gpio_isr_init(void)
{
    int ret = 0;

    battery_current = hisi_battery_current();
    chg_real_info.bat_current_avg = battery_current;

    INIT_WORK(&extchg_plug_in_work, extchg_device_insert_proc);
    INIT_WORK(&extchg_plug_out_work, extchg_device_remove_proc);

    ret = gpio_direction_input(EXTCHG_OTG_DET_GPIO);
    if(ret)
    {
        chg_print_level_message( CHG_MSG_ERR, \
            "EXTCHG: EXTCHG_OTG_DET_GPIO gpio_direction_input error!!\r\n ");
        gpio_free(EXTCHG_OTG_DET_GPIO);
        return -1;
    }

    ret = request_irq((unsigned int)gpio_to_irq(EXTCHG_OTG_DET_GPIO), \
            extchg_plug_in_out_isr,\
                      IRQF_NO_SUSPEND | IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ext_a_chg_gpio_irq", &ext_dev_id);
    if (ret) {
        chg_print_level_message( CHG_MSG_ERR, \
            "EXTCHG: OTG_DET_GPIO request_irq error, errno = %d!!\r\n ", ret);
        free_irq((unsigned int)gpio_to_irq(EXTCHG_OTG_DET_GPIO), NULL);
        gpio_free(EXTCHG_OTG_DET_GPIO);
        return -1;
    }

    enable_irq_wake(gpio_to_irq(EXTCHG_OTG_DET_GPIO));

    if(FALSE == chg_is_ftm_mode())
    {
        if (EXTCHG_ONLINE == extchg_is_extcharge_device_on())
        {
            /*when ext-charge plug in, call insert_proc and start to detect the state*/
            chg_print_level_message( CHG_MSG_ERR, "extchg_monitor_work start, in %s!!!\n", __func__);
            /*初始化完成后重启外充，防止被充设备将充电电流保护在500mA*/
            extchg_set_charge_enable(FALSE);
            chg_delay_ms(EXTCHG_RESET_DELAY_TIME);
            extchg_set_charge_enable(TRUE);
            extchg_device_insert_proc();
        }
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }

    return 0;
}


EXTCHG_ILIM ext_chg_limit_current_low_batt(int32_t bat_vol, int32_t batt_soc)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_NA;
    static EXTCHG_ILIM ext_chg_status_prev = RE_ILIM_NA;

    if(batt_soc > EXTCHG_STOP_CAPACITY_FIFTY)
    {
        /*  soc > 50，limit to 2A, charger online limit to 1A  */
        if(FALSE == is_chg_charger_removed())
        {
             ext_chg_status = RE_ILIM_1A;
        }
        else
        {
            ext_chg_status = RE_ILIM_2A;
        }   
    }
    else if((batt_soc <= EXTCHG_STOP_CAPACITY_FIFTY) && (batt_soc > EXTCHG_ILIM_1A_CAPACITY))
    {
        /*  25 < soc <= 50, limit to 1A  */
        ext_chg_status = RE_ILIM_1A;
    }
    else
    {  /*  0 < soc <= 25, limit to 500mA  */
        ext_chg_status = RE_ILIM_500mA;
    }

    /*  when powerdonw charge, ext-charge need the process of stop and recharge  */
    if (TRUE == chg_is_powdown_charging())
    {
        chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is in pd charge!\n ");

        if(bat_vol < EXTCHG_SHUTDOWN_CHG_STOP_THRESHOLD)
        {
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
            /*电压低于3.1v时关闭热保护电路，防止单板漏电流过点导致电池过放
            对内充电器在位的情况下不关闭热保护电路，只停掉外充*/
            if (TRUE == is_chg_charger_removed())
            {
                hw_sys_shut_down();
            }
#endif
            ext_chg_status = RE_ILIM_STOP;
        }
        else if((ext_chg_status_prev == RE_ILIM_STOP) && (batt_soc <= EXTCHG_STOP_CAPACITY_TEN))
        {
            ext_chg_status = RE_ILIM_STOP;
        }
        else if((ext_chg_status_prev == RE_ILIM_STOP) && (batt_soc > EXTCHG_STOP_CAPACITY_TEN))
        {
            ext_chg_status = RE_ILIM_500mA;
        } 
        else
        {
            /* nothing */
        }
    }

    ext_chg_status_prev = ext_chg_status;
    chg_print_level_message(CHG_MSG_ERR, "ext_limit_batt ->status = %d,soc=%d,volt=%d\n",\
        ext_chg_status,batt_soc,bat_vol);
    return ext_chg_status;
}


EXTCHG_ILIM ext_chg_limit_current_temp(int32_t bat_temp)
{
    static EXTCHG_TEMP_ILIM ext_chg_status = TEMP_ILIM_2A;
    static EXTCHG_TEMP_ILIM ext_chg_status_pre = TEMP_ILIM_2A;
    if (bat_temp < EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD)  
    { /*  < -20 */
        ext_chg_status = LOWTEMP_ILIM_STOP;
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD) \     
        && (bat_temp < EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD))
    {    /* [-20,-17)*/
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
            break;
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_STOP:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = LOWTEMP_ILIM_1A;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_STOP_THRESHOLD))
    {  /* [-17,0)*/
        ext_chg_status = LOWTEMP_ILIM_1A;
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_STOP_THRESHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_RESUME_THRESHOLD))
    {  /* [0,3)*/
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
            ext_chg_status = LOWTEMP_ILIM_1A;
            break;
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_STOP:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = TEMP_ILIM_2A;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_RESUME_THRESHOLD) \
        && (bat_temp <= EXTCHG_WARM_CHARGE_RESUME_THRESHOLD))
    { /* [3,50] */
        ext_chg_status = TEMP_ILIM_2A;
    }
    else if ((bat_temp >  EXTCHG_WARM_CHARGE_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD))
    {    /* (50,53)*/ 
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
        case TEMP_ILIM_2A:
            ext_chg_status = TEMP_ILIM_2A;
            break;
        case OVERTEMP_ILIM_1A:
        case OVERTEMP_ILIM_500mA:
        case OVERTEMP_ILIM_STOP:
            ext_chg_status = OVERTEMP_ILIM_1A;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD) \
        && (bat_temp <= EXTCHG_HEAT_CHARGE_RESUME_THRESHOLD))
    {   /* [53,54] */
        ext_chg_status = OVERTEMP_ILIM_1A;
    }
    else if ((bat_temp > EXTCHG_HEAT_CHARGE_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_HEAT_CHARGE_LIMIT_THRESHOLD))
    {   /* (54,57) */ 
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = OVERTEMP_ILIM_1A;
            break;
        case OVERTEMP_ILIM_500mA:
        case OVERTEMP_ILIM_STOP:
            ext_chg_status = OVERTEMP_ILIM_500mA;
            break;
        default:
            break;
        }    
    }
    else if ((bat_temp >= EXTCHG_OVER_HEAT_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_OVER_HEAT_STOP_THRESHOLD))
    {   /* [57,60) */
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_1A:
        case OVERTEMP_ILIM_500mA:
            ext_chg_status = OVERTEMP_ILIM_500mA;
            break;
        case OVERTEMP_ILIM_STOP:
            ext_chg_status = OVERTEMP_ILIM_STOP;
            break;
        default:
            break;
        }
    }
    else if (bat_temp >= EXTCHG_OVER_HEAT_STOP_THRESHOLD)
    {  /* [60,....)*/ 
        ext_chg_status = OVERTEMP_ILIM_STOP;
    }
    ext_chg_status_pre = ext_chg_status;
    if((ext_chg_status == OVERTEMP_ILIM_STOP) || (ext_chg_status == LOWTEMP_ILIM_STOP))
    {
        chg_print_level_message(CHG_MSG_ERR, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_STOP,bat_temp);
        return RE_ILIM_STOP;
    }
    else if((ext_chg_status == OVERTEMP_ILIM_1A) || (ext_chg_status == LOWTEMP_ILIM_1A))
    {
        chg_print_level_message(CHG_MSG_ERR, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_1A,bat_temp);
        return  RE_ILIM_1A;
    }
    else if (ext_chg_status == OVERTEMP_ILIM_500mA)
    {
        chg_print_level_message(CHG_MSG_ERR, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_500mA, bat_temp);
        return  RE_ILIM_500mA;
    }
    else
    {  
        chg_print_level_message(CHG_MSG_ERR, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_2A,bat_temp);
        return  RE_ILIM_2A;
    }   

}


void extchg_status_update_in_sleep_mode(int32_t batt_volt, int32_t batt_temp)
{
    static int32_t init_flag = TRUE;

    if (g_exchg_online_flag == ONLINE)
    {
        if(TRUE == init_flag)
        {
            battery_voltage = batt_volt;
            battery_temperature = batt_temp;
            battery_current = hisi_battery_current();
            init_flag = FALSE;
        }

        /*calculate the average of voltage in sleep*/
        battery_voltage = batt_revise_calc_average_volt_value(batt_volt);
        //chg_real_info.battery_volt = battery_voltage;
        /*calculate the average of temperature in sleep*/
        battery_temperature = batt_revise_calc_average_temp_value(batt_temp);
        /*calculate the average of current in sleep*/
        battery_current = batt_revise_calc_average_curr_value(hisi_battery_current());

        /*send the current to app, which is used to calculate the last time of the battery*/
        if (battery_current_prev != battery_current)
        {
            battery_current_prev = battery_current;
            chg_real_info.bat_current_avg = battery_current;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
                (uint32_t)CHG_EVENT_NONEED_CARE);
        }

        extchg_monitor_func_in_sleep_mode(batt_volt, batt_temp);
    }

    return;
}


void extchg_monitor_func_in_sleep_mode(int32_t bat_vol, int32_t bat_temp)
{
    boolean need_to_change_timer = FALSE;
    int32_t batt_soc = chg_get_sys_batt_capacity();

    count_for_short_det = 0;
    poll_round_for_det = 0;

    /*short circult detecting*/
    if (TRUE == extchg_short_flag)
    {
        return;
    }
    if (RE_ILIM_STOP != ext_chg_status_new)
    {
        if (extchg_is_perph_circult_short())
        {
            wake_lock(&g_chip->alarm_wake_lock);
            /*disable ext-charge when detect short in sleep mode*/
            extchg_set_charge_enable(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
                (uint32_t)CHG_EVENT_NONEED_CARE);
            chg_print_level_message(CHG_MSG_ERR, \
                "EXTCHG: extcharge circult is short in sleep mode, \
                    and extcharge is short protected!!\n");
            return;
        }
    }

    /*limit the current of ext-charge according to voltage and temperature*/
    ext_chg_status_new = ext_chg_limit_current_low_batt(battery_voltage, batt_soc);
    ext_chg_status_temp = ext_chg_status_new;
    ext_chg_status_new = ext_chg_limit_current_temp(battery_temperature);
    ext_chg_status_new = (ext_chg_status_temp < ext_chg_status_new) \
        ? ext_chg_status_temp : ext_chg_status_new;

    if (ext_chg_status_old != ext_chg_status_new)
    {
        ext_chg_status_old = ext_chg_status_new;
        switch(ext_chg_status_new)
        {
            case RE_ILIM_STOP:
                extchg_set_charge_enable(FALSE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_charge_enable(FALSE)!!\n");
                break;
            case RE_ILIM_2A:
                extchg_set_cur_level(RE_ILIM_2A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_2A)!!\n");
                break;
            case RE_ILIM_1A:
                extchg_set_cur_level(RE_ILIM_1A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_1A)!!\n");
                break;
            case RE_ILIM_500mA:
                extchg_set_cur_level(RE_ILIM_500mA);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_500mA)!!\n");
                break;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge status not change, do nothing!!\n");
    }

    return;
}


void extchg_device_insert_proc(void)
{
    wake_lock_timeout(&g_chip->alarm_wake_lock,
        (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));

    /*it will detect short circult 3 times,
    so when flag is TRUE return directly avoid to repeat*/
    if (TRUE == extchg_short_flag)
    {
        return;
    }

    /*Dejitter processing*/
    mdelay(DELAY_TIME_OF_DEBOUNCE);
    if (EXTCHG_OFFLINE == extchg_is_extcharge_device_on())
    {
        return;
    }

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    /*关机外充时，检测到外充插入后删除定时器*/
    if (TRUE == chg_is_powdown_charging())
    {
        g_extchg_shutoff_flag = TRUE;
        if (TIMER_INIT_FLAG == g_extchg_shutoff_timer.init_flags)
        {
            (void)bsp_softtimer_delete(&g_extchg_shutoff_timer);
        }
    }
#endif

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN!\n ");
    extchg_short_flag = FALSE;
    /*the state of ext-charge sending to app*/
    chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
    /*the online flag of ext-charge*/
    g_exchg_online_flag = ONLINE;
    chg_stm_set_extchg_online_st(TRUE);
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN, schedule_delayed_work!\n ");
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN, schedule_delayed_work!!\n ");
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
    /*modify the timeout of timer of sleep checking*/
    (void)bsp_softtimer_delete(&g_chg_sleep_timer);
    bsp_softtimer_modify(&g_chg_sleep_timer, SLEEP_POLL_CYCLE_OF_EXTCHG);
    bsp_softtimer_add(&g_chg_sleep_timer);
}


void extchg_device_remove_proc(void)
{
    int count = 0;
    int loop = 0;
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    int ret = 0;
#endif
    wake_lock_timeout(&g_chip->alarm_wake_lock,
        (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));

    /*it will detect short circult 3 times,
    so when flag is TRUE return directly avoid to repeat*/
    if (TRUE == extchg_short_flag)
    {
        return;
    }

    count_for_short_det = 0;
    poll_round_for_det = 0;
    /*Dejitter processing*/
    mdelay(DELAY_TIME_OF_DEBOUNCE);
    if (EXTCHG_ONLINE == extchg_is_extcharge_device_on())
    {
        return;
    }

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG OUT!\n ");
    g_exchg_online_flag = OFFLINE;
    ext_chg_status_old = RE_ILIM_NA;
    chg_stm_set_extchg_online_st(FALSE);

    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);

    /*if ext-charge plug out when powerdown charge, go to shutdown the device*/
    if (TRUE == chg_is_powdown_charging())
    {
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
        if (TIMER_INIT_FLAG == g_extchg_shutoff_timer.init_flags)
        {
            /*若已启动定时器，则删除之前的定时器，启动新的定时器*/
            (void)bsp_softtimer_delete(&g_extchg_shutoff_timer);
            g_extchg_shutoff_flag = FALSE;
            bsp_softtimer_add(&g_extchg_shutoff_timer);
        }
        else
        {
            /*关机外充时外充拔出，创建定时器，定时器超时后再次判断外充是否在位*/
            ret = bsp_softtimer_create(&g_extchg_shutoff_timer);
            if(ret)
            {
                chg_print_level_message(CHG_MSG_ERR,
                    "CHG_STM: %s: g_extchg_shutoff_timer create error!!!\r\n ", __func__);
                return;
            }
            g_extchg_shutoff_flag = FALSE;
            bsp_softtimer_add(&g_extchg_shutoff_timer);
        }
#else
        for(count = 0; count <= 1; count++)
        {
            chg_detect_batt_chg_for_shutoff();
        }
#endif
    }
    /*modify the timeout of timer of sleep checking*/
    (void)bsp_softtimer_delete(&g_chg_sleep_timer);
    bsp_softtimer_modify(&g_chg_sleep_timer, SLEEP_POLL_CYCLE);
    bsp_softtimer_add(&g_chg_sleep_timer);
}


void extchg_monitor_func(void)
{
    int32_t bat_temp = chg_get_sys_batt_temp();
    int32_t bat_vol = chg_get_sys_batt_volt();
    int32_t batt_soc = chg_get_sys_batt_capacity();

    /*short circult detect*/
    if (TRUE == extchg_short_flag)
    {
        if (TIME_OF_SHORT_DET == count_for_short_det)
        {
            /*enable ext-charge and detect the short circult*/
            extchg_set_charge_enable(TRUE);
            mdelay(50);
            if (extchg_is_perph_circult_short())
            {
                count_for_short_det = 0;
                count_for_short_det++;
                poll_round_for_det++;
                /*short is detected then disable ext-charge*/
                extchg_set_charge_enable(FALSE);
            }
            else
            {
                ext_chg_status_new = RE_ILIM_2A;
                extchg_short_flag = FALSE;
                count_for_short_det = 0;
                poll_round_for_det = 0;
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
                    (uint32_t)CHG_EVENT_NONEED_CARE);
                if (EXTCHG_OFFLINE == extchg_is_extcharge_device_on())
                {
                    g_exchg_online_flag = OFFLINE;
                    ext_chg_status_old = RE_ILIM_NA;
                    chg_stm_set_extchg_online_st(FALSE);
                    /*ext-charge plug out, modify the timer of sleep*/
                    (void)bsp_softtimer_delete(&g_chg_sleep_timer);
                    bsp_softtimer_modify(&g_chg_sleep_timer, SLEEP_POLL_CYCLE);
                    bsp_softtimer_add(&g_chg_sleep_timer);
                    wake_unlock(&g_chip->alarm_wake_lock);
                    goto out;
                }
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extcharge is not short anymore!\n");
                bsp_softtimer_add(&g_chg_sleep_timer);
                wake_unlock(&g_chip->alarm_wake_lock);
                goto out;
            }
        }
        else
        {
            count_for_short_det++;
        }
        if (NUM_OF_SHORT_DET_POLL == poll_round_for_det)
        {
            poll_round_for_det = 0;
            count_for_short_det = 0;
            extchg_short_flag = FALSE;
            g_exchg_online_flag = OFFLINE;
            ext_chg_status_old = RE_ILIM_NA;
            ext_chg_status_new = RE_ILIM_STOP;
            chg_stm_set_extchg_online_st(FALSE);
            chg_print_level_message(CHG_MSG_ERR, \
                "EXTCHG: extcharge is short protected!!!\n");
            /*short is detected for 3 times, then disable ext-charge*/
            (void)bsp_softtimer_delete(&g_chg_sleep_timer);
            bsp_softtimer_modify(&g_chg_sleep_timer, SLEEP_POLL_CYCLE);
            bsp_softtimer_add(&g_chg_sleep_timer);
            wake_unlock(&g_chip->alarm_wake_lock);
            return;
        }
        goto out;
    }
    else
    {
        if (RE_ILIM_STOP != ext_chg_status_new)
        {
            if (extchg_is_perph_circult_short())
            {
                wake_lock(&g_chip->alarm_wake_lock);
                (void)bsp_softtimer_delete(&g_chg_sleep_timer);
                ext_chg_status_new = RE_ILIM_STOP;
                extchg_short_flag = TRUE;
                count_for_short_det++;
                extchg_set_charge_enable(FALSE);
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
                    (uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extcharge circult is short!!\n");
                if (TRUE == chg_is_powdown_charging())
                {
                    chg_stm_set_extchg_online_st(FALSE);
                    return;
                }
                goto out;
            }
        }
    }

    /*limit the current of ext-charge according to voltage and temperature*/
    ext_chg_status_new = ext_chg_limit_current_low_batt(bat_vol, batt_soc);
    ext_chg_status_temp = ext_chg_status_new;
    ext_chg_status_new = ext_chg_limit_current_temp(bat_temp);

    ext_chg_status_new = (ext_chg_status_temp < ext_chg_status_new) \
        ? ext_chg_status_temp : ext_chg_status_new;

    if (ext_chg_status_old != ext_chg_status_new)
    {
        ext_chg_status_old = ext_chg_status_new;
        switch(ext_chg_status_new)
        {
            case RE_ILIM_STOP:
                extchg_set_charge_enable(FALSE);
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extchg_set_charge_enable(FALSE)!!\n");
                break;
            case RE_ILIM_2A:
                extchg_set_cur_level(RE_ILIM_2A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_2A)!!\n");
                break;
            case RE_ILIM_1A:
                extchg_set_cur_level(RE_ILIM_1A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_1A)!!\n");
                break;
            case RE_ILIM_500mA:
                extchg_set_cur_level(RE_ILIM_500mA);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_500mA)!!\n");
                break;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, \
            "EXTCHG: extcharge status not change, do nothing!!\n");
    }
out:
    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(2000));

    return;
}
#endif


void chg_extchg_insert_proc(void)
{
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
    int32_t batt_volt = chg_get_sys_batt_volt();
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    int32_t batt_soc = chg_get_sys_batt_capacity();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    int ret = 0;
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN!\n ");
    if(OFFLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = ONLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*如果UI设置永久禁止对外充电此处屏蔽USB ID 插入中断*/
    if(1 == g_extchg_diable_st)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*无线充电场景不处理对外充电事件*/
    if(CHG_WIRELESS_MODE == cur_chg_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_mode in CHG_WIRELESS_MODE,do nothing!\n ");
        return;
    }

    chg_stm_set_extchg_online_st(TRUE);
    /*电池低电停止对外充电*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    if((batt_soc <= g_extchg_stop_soc_threshold) && (FALSE == is_batt_volt_low))
#else
    if((batt_volt <= g_extchg_voltage_threshold) && (FALSE == is_batt_volt_low))
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    {
        chg_set_extchg_chg_enable(FALSE);
        is_batt_volt_low = TRUE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
        /*通知应用低电停止对外充电*/
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG INSERT LOW VOLTAGE STOP EXTGHG!\n");
    }
    chg_set_cur_chg_mode(CHG_EXTCHG_MODE);
    //TO DO:1:调用USB驱动接口短接D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG, (uint32_t)CHG_EVENT_NONEED_CARE);
    ret = blocking_notifier_call_chain(&g_extchg_pciesave_notifier_list,EXT_INSERT,NULL);
#if ( FEATURE_ON == MBB_MLOG )
    printk(KERN_ERR "CHG_STM:otg_charge_count\n");
    mlog_set_statis_info("otg_charge_count",1);//对外充电总次数 加1
#endif/*MBB_MLOG*/
    /*启动对外充电监控work*/
    if(NULL != g_chip)
    {
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }
}


void chg_extchg_remove_proc(void)
{
    int ret = 0;
    if(ONLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = OFFLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG OUT!\n ");
    /*终止运行的work*/
    cancel_delayed_work_sync(&g_chip->extchg_monitor_work);

    /*对外充电USB ID线拔出将温度补偿的标志位清除*/
    g_ui_choose_exchg_mode = 0;
    g_last_extchg_diable_st = 0;
    is_otg_extchg = FALSE;
    is_extchg_ovtemp = FALSE;
    is_batt_volt_low = FALSE;
#if defined(BSP_CONFIG_BOARD_E5785Lh_92a)
    g_covert_to_otg_counter = 0;
#endif
    chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
    chg_set_extchg_chg_enable(FALSE);
    chg_stm_set_extchg_online_st(FALSE);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
    chg_stm_set_chgr_type(CHG_CHGR_INVALID);
    ret = blocking_notifier_call_chain(&g_extchg_pciesave_notifier_list,EXT_REMOVE,NULL);

}


void chg_extchg_monitor_func(void)
{
    int32_t bat_temp = chg_get_sys_batt_temp();
    int32_t bat_vol = chg_get_sys_batt_volt();
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    int32_t batt_soc = chg_get_sys_batt_capacity();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

    if(OFFLINE == g_exchg_online_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:extchg device not exit,do nothing!\n");
        return;
    }
    /*更新UI配置*/
    chg_extchg_config_data_init();
#if defined(BSP_CONFIG_BOARD_E5785Lh_92a)
    if (TRUE == is_otg_extchg)
    {
        if (OTG_OVP_DETECT_DELAY_CONTER > g_covert_to_otg_counter)
        {
            g_covert_to_otg_counter++;
        }
    }
#endif

    /*状态监控*/
    /*如果对外充电过流且为非OTG模式，则通过5V上下电reset usb模块*/
    if(TRUE == chg_extchg_ocp_detect())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:EXTCHG OCP happened is_otg_extchg=%d!\n",
            is_otg_extchg);

        if(FALSE == is_otg_extchg)
        {
            chg_set_charge_otg_mode();
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:1A EXTCHG OCP change to USB OTG 500MA!\n");
        }
        /*如果为OTG模式检测到过流直接停止对外充电*/
        else
        {
#if defined(BSP_CONFIG_BOARD_E5785Lh_92a)
            if (OTG_OVP_DETECT_DELAY_CONTER <= g_covert_to_otg_counter)
            {
                chg_set_extchg_chg_enable(FALSE);
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
                /*通知应用不满足对外充电条件停止对外充电*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:USB OTG 500MA EXTCHG OCP STOP EXTGHG!\n");
            }
#else
            chg_set_extchg_chg_enable(FALSE);
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
            /*通知应用不满足对外充电条件停止对外充电*/
            chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:USB OTG 500MA EXTCHG OCP STOP EXTGHG!\n");
#endif
        }
    }
#if (defined(BSP_CONFIG_BOARD_E5785Lh_92a) || defined(BSP_CONFIG_BOARD_E5785Lh_22c) \
    || defined(BSP_CONFIG_BOARD_E5787Ph_92a) || defined(BSP_CONFIG_BOARD_E5787Ph_67a) \
    || defined(BSP_CONFIG_BOARD_E5785Lh_67a))
    /*电池温度过温且为非OTG模式，限制对外充电电流500MA*/
    else if( (bat_temp >= EXTCHG_OVER_TEMP_LIMIT_500MA) && (FALSE == is_otg_extchg)
        /*电池温度超过过温停冲门限，不进入500MA充电*/
        && (FALSE == is_extchg_ovtemp)
        )
    {
        chg_set_charge_otg_mode();
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:1A EXTCHG battery over temp change to USB OTG 500MA!\n");
    }
#endif
    /*电池过温停止对外充电*/
    else if((bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD) && (FALSE == is_extchg_ovtemp))
    {
        /*过温停止对外充电*/
        chg_set_extchg_chg_enable(FALSE);
        is_extchg_ovtemp = TRUE;

        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_OVERHEAT_STOP_CHARGING;
        /*通知应用温度异常停止对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG OVER TEMP STOP EXTGHG!\n");
    }
    /*电池温度恢复使能对外充电*/
    else if((bat_temp <= EXTCHG_OVER_TEMP_RESUME_THRESHOLD) && (TRUE == is_extchg_ovtemp))
    {
        chg_set_extchg_chg_enable(TRUE);
        is_extchg_ovtemp = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_START_CHARGING;
        /*温度恢复正常通知应用开始对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG TEMP NORMAL RESUME EXTGHG!\n");
    }
   /*电池低电停止对外充电*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    else if((batt_soc <= g_extchg_stop_soc_threshold) && (FALSE == is_batt_volt_low))
#else
    else if((bat_vol <= g_extchg_voltage_threshold) && (FALSE == is_batt_volt_low))
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    {
        chg_set_extchg_chg_enable(FALSE);
        is_batt_volt_low = TRUE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
        /*通知应用低电停止对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG LOW VOLTAGE STOP EXTGHG!\n");
    }
    /*正在对外充电过程中如果检测到用户通过TOUCH UI设置永久停止对外充电则直接停止对外充电*/
    else if((1 == g_extchg_diable_st) && (0 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 1;
        chg_set_extchg_chg_enable(FALSE);
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set disable extchg permanently!\n");
    }
    /*正在对外充电过程中如果检测到用户通过TOUCH UI设置永久停止对外充电则直接停止对外充电后
       如果用户再设置打开*/
    else if((0 == g_extchg_diable_st) && (1 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 0;
        /*重新使能后将STA节点状态改为UNKNOWN,是选择框弹出来由用户选择后决定是否对外充电*/
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set enable extchg permanently!\n");
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:staying at extchg_state !\n");
    }
    chg_extchg_info_dump();
    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(2000));
}

#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


void chg_charger_insert_proc(chg_chgr_type_t chg_type)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();

    /* 有外电源插入,则先清掉低电标志 */
    chg_batt_low_battery_flag = FALSE;

    /*USB插入*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_USB_HOST_PC PLUG IN force to TRANSIT_ST!\n ");
        if(FALSE == chg_get_usb_online_status())
        {
            chg_set_ac_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
            chg_set_usb_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        //chg_stm_set_chgr_type(CHG_USB_HOST_PC);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*USB插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
    /*usb插入，自增一次*/
    if(CHG_CURRENT_SS == usb_speed_work_mode())
    {
        /*usb3.0插入*/
        mlog_set_statis_info("usb3_charge_insert_count",1);
    }
    else
    {
        /*usb2.0插入*/
        mlog_set_statis_info("usb2_charge_insert_count",1);
    }
#endif/*FEATURE_ON == MBB_MLOG*/
        return ;
    }
    /*标充/CRADLE/HVDCP插入*/
    else if((CHG_WALL_CHGR == chg_type)
        || (CHG_USB_OTG_CRADLE == chg_type)
        || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
        //if(CHG_USB_OTG_CRADLE == chg_type)
        //{
        //    chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG IN set Dpm.\n ");
        //    chg_set_dpm_val(CHG_AF18_DPM_VOLT);
        //}
        /*如果检测到充电器类型是标充就将 USB 设置为OFFINE,将AC设置为ONLINE*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
        /*check if the  device is charging to itself, if so, suspend the charge*/
        if (TRUE == extchg_is_charge_status_error())
        {
            usb_err_flag = TRUE;
            chg_stm_state_info.extchg_status = \
                POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
            chg_stm_set_extchg_online_st(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, \
                (uint32_t)CHG_EVENT_NONEED_CARE);
            (void)chg_set_suspend_mode(TRUE);
        }
#endif
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_WALL_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*标充插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                 过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "std charger insert!\n ");
            mlog_set_statis_info("std_charge_insert_count",1);
#endif
        return ;
    }
    /*弱充插入*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_en_flag = 0;
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
        /*如果检测到充电器类型是弱充就将 USB 设置为OFFINE,将AC设置为ONLINE*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);

        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_500MA_WALL_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*弱充插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                 过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
        return ;
    }
    /*非标充插入*/
    else if((CHG_NONSTD_CHGR == chg_type) || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_NONSTD_CHGR PLUG IN force to TRANSIT_ST!\n ");
        //非标充插入时，认为USB在位
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);

        chg_set_usb_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*三方充电器插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);

        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "no_std charger insert!\n ");
            mlog_set_statis_info("no_std_charge_insert_count",1);
#endif
        return ;
    }

    /*充电器类型未知，在此判断是无线充电还是有线充电设备插入*/
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
        /*如果是有线充电器插入先设置AC ONLINE保证应用可以迅速点屏
              如果检测到充电器类型是usb或第三方充电器就将AC 设置为OFFINE,将USB设置为ONLINE*/
        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        /*有线充电器插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
        过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        chg_stm_set_wireless_online_st(FALSE);
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
        //chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        chg_print_level_message( CHG_MSG_INFO,"CHG_PLT: wired charger but chg type unknow force to TRANSIT_ST!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*通知USB驱动无线充电器插入,防止USB驱动未检测出充电器类型给充电模块
        上报第三方充电器，导致充电流程混乱*/
        g_wireless_online_flag = ONLINE;
        usb_notify_event(USB_WIRELESS_CHGR_DET,NULL);
        if(0 == g_ui_choose_exchg_mode)
        {
            (void)chg_set_dpm_val(CHG_WIRELESS_DPM_VOLT);
            chg_stm_set_wireless_online_st(TRUE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_WIRELESS_CHGR PLUG IN force to TRANSIT_ST!\n ");
            /*检测出来是无线充电器后就控制无线充电芯片GPIO使能无线充电，充电过程中的停复充有BQ24196控制*/
            chg_set_wireless_chg_enable(TRUE);
            //chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
            chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:WIRELESS_CHGR PLUG IN but current not CHG_BAT_ONLY_MODE!\n");
            return ;
        }
#if ( FEATURE_ON == MBB_MLOG )        
            chg_print_level_message(CHG_MSG_ERR, "wireless charger insert!\n ");
            mlog_set_statis_info("wireless_charge_insert_count",1);
#endif
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:PLUG IN CHG TYPE UNKNOW!\n ");
    }
}


void chg_charger_remove_proc(chg_chgr_type_t chg_type)
{
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if (TRUE == chg_battery_protect_flag)
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
        (void)chg_battery_protect_exit_process();
    }
    (void)resume_long_time_no_charge_protect_recharge_volt();
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    /*if the charge loop plug out, resume the charge*/
    if (TRUE == usb_err_flag)
    {
        usb_err_flag = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        chg_stm_set_extchg_online_st(FALSE);
        chg_temp_protect_eixt_suspend_mode();
        chg_print_level_message(CHG_MSG_ERR, "self charge plug out, exit suspend!\n ");
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, \
            (uint32_t)CHG_EVENT_NONEED_CARE);
    }
#endif

    /*USB 拔出*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_USB_HOST_PC PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*标充/cradle/HVDCP拔出*/
    else if((CHG_WALL_CHGR == chg_type)
        || (CHG_USB_OTG_CRADLE == chg_type)
        || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        //AF18 拔出后恢复默认DPM电压值
        if(CHG_USB_OTG_CRADLE == chg_type)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG out set Dpm.\n ");
            chg_set_dpm_val(CHG_DEFAULT_DPM_VOLT);
        }
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*弱充拔出*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*非标充/HVDCP*/
    else if((CHG_NONSTD_CHGR == chg_type) || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_NONSTD_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        /*非标充拔出按照USB驱动拔出给应用上报*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);

        return ;
    }
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_CHGR_UNKNOWN PLUG OUT force to BATTERY_ONLY_st!\n ");
        if (TRUE == chg_get_usb_online_status())
        {
            chg_set_usb_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        if(TRUE == chg_get_ac_online_status())
        {
            chg_set_ac_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if (TRUE == chg_stm_get_wireless_online_st())
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
#if (FEATURE_OFF == MBB_CHG_APORT_EXTCHG)
        /*the charger and the ext-charge can be online at the same time,
        so the charger's plugging out will not influence the ext-charge */
        if (TRUE == chg_stm_get_extchg_online_st())
        {
            chg_stm_set_extchg_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    /*无线充电器拔出*/
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*如果当前是无线充电模式，无线充电器不在位后才进行状态切换否则只将无限充电器
            ONLINE节点设置为OFFLINE,不进行状态切换*/
        g_wireless_online_flag = OFFLINE;
        if((CHG_WIRELESS_MODE == cur_chg_mode) || (TRUE == chg_is_powdown_charging()))
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_WIRELESS_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
            chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
            chg_stm_set_chgr_type(CHG_CHGR_INVALID);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }

    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else
    {
        if( FALSE == chg_is_charger_present() && CHG_STM_BATTERY_ONLY != cur_stat )
        {
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        }
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:PLUG OUT CHG TYPE UNKNOW!\n ");
    }
}


struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage)
{
    int i;
#if (FEATURE_ON == MBB_CHG_BATT_UNIFORM)
    /*如果ID电压为0说明为普通电池则使用普通电池的参数*/
    if((0 == id_voltage) || (NULL == g_battery_para_dtsi))
    {
        return chg_batt_data_array[0];
    }
    /*高压电池类型判断*/
    for (i = (g_battery_para_dtsi->count - 1); i > 0; i--)
    {
        if ((id_voltage >= g_battery_para_dtsi->chg_batt_data_paras[i]->id_voltage_min)
            && (id_voltage <= g_battery_para_dtsi->chg_batt_data_paras[i]->id_voltage_max))
        {
            break;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"bat_id_index=%d\n",i);
    return g_battery_para_dtsi->chg_batt_data_paras[i];
#else
    /*如果ID电压为0说明为普通电池则使用普通电池的参数*/
    if(0 == id_voltage)
    {
        return chg_batt_data_array[0];
    }
    /*高压电池类型判断*/
    for (i = (BATTERY_DATA_ARRY_SIZE - 1); i > 0; i--){
        if ((id_voltage >= chg_batt_data_array[i]->id_voltage_min)
            && (id_voltage <= chg_batt_data_array[i]->id_voltage_max))
        {
            break;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"bat_id_index=%d\n",i);
    return chg_batt_data_array[i];
#endif
}


int32_t chg_batt_volt_paras_init(void)
{
    int32_t batt_id = 0;
    batt_id = chg_get_batt_id_volt();
    g_chg_batt_data = chg_get_batt_data(batt_id);
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail use default NV parameter!!\n");
        /*电池膨胀方案，记录常温复充中间变量初始化*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        g_batt_normal_temp_recherge_threshold = g_chgBattVoltProtect.battNormalTempRechergeThreshold;
#endif
        return -1;
    }

    if (CHG_BATT_ID_DEF == g_chg_batt_data->batt_id)
    {
        mlog_print(MLOG_CHG, mlog_lv_info, 
            "Unknown battery type detected,use default charge parameters.\n");
        chg_print_level_message(CHG_MSG_INFO, "Unknown battery type detected.\n");
    }

    //init batt para
    memcpy(&g_chgBattVoltProtect, (uint8_t*)&g_chg_batt_data->chg_batt_volt_paras, sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE));
    /*电池膨胀方案，记录常温复充中间变量初始化*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    g_batt_normal_temp_recherge_threshold = g_chgBattVoltProtect.battNormalTempRechergeThreshold;
#endif
    chg_batt_data_dump();
    return 0;
}


uint32_t chg_get_batt_id(void)
{
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail!!\n");
        return CHG_BATT_ID_DEF;
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG: batt brand is %d\n",g_chg_batt_data->batt_id);

    return g_chg_batt_data->batt_id;
}


void chg_batt_data_dump(void)
{
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOnThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOnThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOffThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOffThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectOneThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battChgTempMaintThreshold=%d\n",
        g_chgBattVoltProtect.battChgTempMaintThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battChgRechargeThreshold=%d\n",
        g_chgBattVoltProtect.battChgRechargeThreshold);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevelLow_MAX=%d\n",g_chgBattVoltProtect.VbatLevelLow_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel0_MAX=%d\n",g_chgBattVoltProtect.VbatLevel0_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel1_MAX=%d\n",g_chgBattVoltProtect.VbatLevel1_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel2_MAX=%d\n",g_chgBattVoltProtect.VbatLevel2_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel3_MAX=%d\n",g_chgBattVoltProtect.VbatLevel3_MAX);
    chg_print_level_message(CHG_MSG_ERR,"battInsertChargeThreshold=%d\n",
    g_chgBattVoltProtect.battInsertChargeThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battNormalTempRechergeThreshold=%d\n",
    g_chgBattVoltProtect.battNormalTempRechergeThreshold);
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    chg_print_level_message(CHG_MSG_ERR,"g_batt_normal_temp_recherge_threshold=%d\n",
        g_batt_normal_temp_recherge_threshold);
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    chg_print_level_message(CHG_MSG_ERR,"g_chg_battery_expand_change_normal_rechg_flag=%d\n",
        g_chg_battery_expand_change_normal_rechg_flag);
#endif
#endif
}

