
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

#ifndef CHG_CHARGE_STM_H
#define CHG_CHARGE_STM_H
#include "chg_charge_cust.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES       (12)

#define CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS        (2000)

/*Fast charge protection timer, in second. */
#define MS_IN_SECOND                            (1000)
#define SECOND_IN_HOUR                          (60 * 60)

/*Time Interval for toggle CEN.*/
#define CHG_TOGGLE_CEN_INTVAL_IN_MS             (100)

/*Time Interval for switch to SLOW POLLING while battery only.*/
#define CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC (60)

/*Indicate battery charging start/stop flag.*/
#define CHG_UI_START_CHARGING                   (1)
#define CHG_UI_STOP_CHARGING                    (0)
/* 延时500ms */
#define EXTCHG_DELAY_COUNTER_SIZE               (500)

/* 若定义补电宏 */
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
/* 延时10ms */
#define CHG_DELAY_COUNTER_SIZE                  (10)


/*补电成功*/
#define TBAT_SUPPLY_CURR_SUCCESS                (0x0)
/*不需要补电*/
#define TBAT_NO_NEED_SUPPLY_CURR                (0x1)
/* 补电停止时间 */
#define TBAT_STOP_DELAY_COUNTER                 (100)
/* 补电启动时间 */
#define TBAT_SUPLY_DELAY_COUNTER                (2300)
#endif /*MBB_CHG_COMPENSATE == FEATURE_ON */

/*关机充电关机检测次数**/
#define CHARGE_REMOVE_CHECK_MAX                 (1)

/*非标电池过温停充门限*/
#define CHG_DEF_BATT_OVER_TEMP_STOP_THRESHOLD    (45)
/*非标电池恢复充电温度门限*/
#define CHG_DEF_BATT_OVER_TEMP_RESUME_THRESHOLD    (42)

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#define EXTCHG_DETECT_DELAY_TIME                 (1000)
#define EXTCHG_RESET_DELAY_TIME                  (200)
#define EXTCHG_DETECT_INTERVAL                   (500)
#define EXTCHG_DETECT_TIME                       (3)
#endif
/*----------------------------------------------*
 * 结构定义                                      *
 *----------------------------------------------*/
typedef  void (*chg_stm_func_type )(void);
typedef struct
{
  chg_stm_func_type        chg_stm_entry_func;
  chg_stm_func_type        chg_stm_period_func;
  chg_stm_func_type        chg_stm_exit_func;
}chg_stm_type;

/*高温关机温度参数NV50016结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;             //高温关机使能开关
    int32_t       lCloseAdcThreshold;     //高温关机温度门限
    uint32_t      ulTempOverCount;        //高温关机温度检测次数
}CHG_BATTERY_OVER_TEMP_PROTECT_NV;

/*高温关机温度参数NV52005结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;
    int32_t       lCloseAdcThreshold;
    uint32_t      ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/


void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param);


void chg_poll_bat_level(void);

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

void chg_poll_batt_soc(void);


void chg_poll_batt_charging_state_for_coul(void);


boolean chg_is_batt_in_state_of_emergency();


void chg_low_battery_event_handler(void);

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)

int32_t chg_get_recharge_threshold(void);
#endif
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_poll_batt_temp(void);


void chg_batt_volt_init(void);


void chg_batt_temp_init(void);


extern chg_stm_state_type chg_stm_get_cur_state(void);


extern void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode);


extern chg_chgr_type_t chg_stm_get_chgr_type(void);


extern CHG_MODE_ENUM chg_get_cur_chg_mode(void);


void chg_check_and_update_hw_param_per_chgr_type(void);


extern boolean chg_get_batt_id_valid(void);


extern BATT_LEVEL_ENUM chg_get_batt_level(void);


extern int32_t chg_get_sys_batt_capacity(void);


extern void chg_set_sys_batt_capacity(int32_t capacity);


extern void chg_set_batt_time_to_full(int32_t time_to_full);


extern int32_t chg_get_batt_time_to_full(void);


extern boolean chg_is_batt_full(void);


extern int32_t chg_get_bat_status(void);


extern int32_t chg_get_bat_health(void);

void chg_set_extchg_status(int32_t extchg_status);


extern int32_t chg_get_extchg_status(void);


TEMP_EVENT chg_get_batt_temp_state(void);


extern void chg_stm_switch_state(chg_stm_state_type new_state);


void chg_stm_periodic_checking_func(void);


void chg_charge_paras_init(void);


int32_t chg_stm_init(void);


boolean chg_get_charging_status(void);


extern boolean chg_is_ftm_mode(void);


extern void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type);


extern void chg_start_chgr_type_checking(void);

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state);


chg_stm_state_type chg_stm_get_pre_state(void);


boolean is_batttemp_in_warm_chg_area( void );
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

int32_t chg_tbat_status_get(void);


boolean chg_is_sply_finish(void);


int32_t chg_batt_supply_proc(void *task_data);


int32_t chg_tbat_chg_sply(void);
#endif /* MBB_CHG_COMPENSATE == FEATURE_ON */

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)

extern int extchg_gpio_isr_init(void);


extern void extchg_monitor_func(void);


int32_t chg_batt_temp_revise(int32_t batt_temp);
#endif


extern boolean chg_get_charging_status(void);


extern boolean chg_is_exception_poweroff_poweron_mode(void);


extern int chg_get_cur_batt_temp(void);


extern int chg_get_sys_batt_temp(void);


extern int32_t chg_get_sys_batt_volt(void);


extern int32_t chg_get_avg_vph_pwr_volt(void);


void load_on_off_mode_parameter(void);


void load_ftm_mode_init(void);


void chg_detect_batt_chg_for_shutoff(void);


extern void chg_update_power_suply_info(void);


void chg_print_test_view_info(void);


void chg_charger_insert_proc(chg_chgr_type_t chg_type);


void chg_charger_remove_proc(chg_chgr_type_t chg_type);


int32_t chg_get_batt_id_volt(void);

#ifdef CONFIG_MBB_FAST_ON_OFF

extern void chg_get_system_suspend_status(ulong64_t suspend_status);
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)

extern void chg_stm_set_wireless_online_st(boolean online);


extern boolean chg_stm_get_wireless_online_st(void);
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

extern void chg_stm_set_extchg_online_st(boolean online);


boolean chg_stm_get_extchg_online_st(void);


void chg_extchg_config_data_init(void);


extern boolean chg_get_extchg_online_status(void);


extern void chg_extchg_insert_proc(void);


extern void chg_extchg_remove_proc(void);


extern void chg_extchg_monitor_func(void);

#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


boolean chg_get_usb_online_status(void);


void chg_set_usb_online_status(boolean online);


boolean chg_get_ac_online_status(void);


void chg_set_ac_online_status(boolean online);


extern boolean is_chg_charger_removed(void);


extern struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage);


extern int32_t chg_batt_volt_paras_init(void);


extern uint32_t chg_get_batt_id(void);

extern boolean chg_is_no_battery_powerup_enable(void);

void chg_batt_data_dump(void);


void chg_set_supply_limit_by_stm_stat(void);

#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))

int32_t  chg_get_extchg_start(void);

void chg_check_vbus_volt(void);
#endif
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

int32_t chg_get_usb_health(void);

int32_t chg_get_usb_cur_temp(void);

boolean chg_get_usb_temp_protect_stat(void);

void chg_test_set_usb_temp_limit_and_resume(int32_t limit,int32_t resume);
#endif
#endif /*CHG_CHARGE_STM_H*/

