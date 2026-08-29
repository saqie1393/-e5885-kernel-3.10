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



#ifndef __USB_VENDOR_H__
#define __USB_VENDOR_H__

#ifdef __cplusplus
extern "C" { /* allow C++ to use these headers */
#endif /* __cplusplus */
#include "usb_platform_comm.h"
#include <linux/notifier.h>
#include "usb_hotplug.h"
#include "hw_pnp.h"
#include "hw_pnp_api.h"

#define USB_NOTIF_PRIO_ADP      0           /* adp has lowest priority */
#define USB_NOTIF_PRIO_FD       100         /* function drvier */
#define USB_NOTIF_PRIO_CORE     200         /* usb core */
#define USB_NOTIF_PRIO_HAL      300         /* hardware has highest priority */

/*
 * usd to support usb event notify
 * connect pc:
 * insert(1) -> chargertype iden (2) -> enum_done(3) -> remove(0)/disable(-1) +
 *   ^___________________________________________________________|
 *
 * connect charger (must pmu support):
 * insert(1) -> remove(0) +
 *   ^____________________|
 *
 * connect peripheral(such as udisk):
 * perip_insert(4) -> perip_remove(5)
 *   ^_______________________^
 */
#define USB_BALONG_DEVICE_REMOVE    0
#define USB_BALONG_DEVICE_INSERT    1
#define USB_BALONG_CHARGER_IDEN     2
#define USB_BALONG_DEV_ENUM_DONE    3
#define USB_BALONG_PERIP_INSERT     4
#define USB_BALONG_PERIP_REMOVE     5
#define USB_BALONG_HOST_ENUM_DONE   6

/* if the version is not support pmu detect
 * and all the device is disable, we assume that the usb is remove,
 */
#define USB_BALONG_DEVICE_DISABLE   (0xF1)

#define USB_BALONG_HOTPLUG_IDLE     0
#define USB_BALONG_HOTPLUG_INSERT   1
#define USB_BALONG_HOTPLUG_REMOVE   2

/*
 * charger type define
 */
#define USB_CHARGER_TYPE_HUAWEI         1
#define USB_CHARGER_TYPE_NOT_HUAWEI     2
#define USB_CHARGER_TYPE_INVALID        0
#define USB_CHARGER_TYPE_CRADLE         8

#define USB_IS_SUPPORT_NV               1
#define USB_IS_SUPPORT_OM_DUMP          1
#define USB_IS_SUPPORT_PMU              1
#define USB_IS_SUPPORT_VSHELL           1
#define USB_IS_SUPPORT_WAKELOCK         1
#define USB_IS_SUPPORT_DYN_INIT         1


#define USB_FD_DEVICE_MAX               32
#define USB_CDEV_NAME_MAX               64
#define USB_ENABLE_CB_MAX               32

//#define GNET_MAX_NUM       (4)
//#define GNET_USED_NUM      (1)


#if !defined(CONFIG_PC_USE_HUAWEI_DRIVER)
#define USB_IF_SUBCLASS_BALONG      0x02
#else /* PC_USE_HUAWEI_DRIVER */
#define USB_IF_SUBCLASS_BALONG      0x03
#endif

#define USB_CDC_NCM_DWC_CACHE_TRB_NUM   (CONFIG_USB_CDC_NCM_DWC_CACHE_TRB_NUM)
#define USB_CDC_ACM_DWC_CACHE_TRB_NUM   (CONFIG_USB_CDC_NCM_DWC_CACHE_TRB_NUM)

#define DUMP_OFFSET_OF(type, member) ((u32) (&((type *)0)->member))

typedef struct usb_enum_stat
{
    char* fd_name;                  /* function drvier file name */
    unsigned usb_intf_id;           /* usb interface id */
    unsigned is_enum;               /* whether the dev is enum */
    unsigned cfg_idx;
} usb_enum_stat_t;

/* charger type identify interface */
#if(FEATURE_ON == MBB_TELE_WWAN)
char wwan_read_nv(void);
USB_INT wwan_write_nv(char flags);
#endif
void bsp_usb_set_charger_type(int type);
int bsp_usb_get_charger_type(void);

int power_on_dwc3_usb(void);
int power_off_dwc3_usb(void);
int bsp_usb_adapter_init(void);

int bsp_usb_is_support_charger(void);
int bsp_usb_is_support_pmu_detect(void);
void bsp_usb_wait_cdev_created(void);
void bsp_usb_clear_last_cdev_name(void);

/* notify interface */
void bsp_usb_register_notify(struct notifier_block* nb);
void bsp_usb_unregister_notify(struct notifier_block* nb);

/* usb status change interface*/
void bsp_usb_status_change(int status);
//int adp_usb_queue_delay_work(struct delayed_work *dwork, unsigned long delay);
int usb_balong_init(void);
void usb_balong_exit(int is_otg);
int bsp_usb_console_init(void);


/* usb enum done interface */
void bsp_usb_init_enum_stat(void);
int bsp_usb_is_all_enum(void);
int bsp_usb_is_all_disable(void);
#define bsp_usb_add_setup_dev(intf_id) \
    bsp_usb_add_setup_dev_fdname(intf_id, __FILE__)
void bsp_usb_del_setup_dev(unsigned intf_id);
void bsp_usb_add_setup_dev_fdname(unsigned intf_id, char* fd_name);
void bsp_usb_set_enum_stat(unsigned intf_id, int enum_stat);
void bsp_usb_set_last_cdev_name(char* dev_name);

extern u32 usb_get_curr_cfg_idx(void);
#ifdef BSP_CONFIG_BOARD_TELEMATIC
extern void dwc3_gadget_disconnect_cb(void);
#endif
/* usb nv feature functions */
void bsp_usb_nv_init(void);
/* 1: support, 0:not support */
int bsp_usb_is_support_wwan(void);
int bsp_usb_is_support_shell(void);
int bsp_usb_is_support_hibernation(void);
int bsp_usb_is_support_phy_apd(void);
int bsp_usb_is_ncm_bypass_mode(void);
int bsp_usb_is_sys_err_on_disable(void);
int bsp_usb_is_enable_u1u2_workaround(void);
int bsp_usb_is_vbus_connect(void);
int bsp_usb_vbus_detect_mode(void);
int bsp_usb_otg_gpio_detect(void);
int bsp_usb_mode_support(void);
/*get driver information from nv*/
u16 bsp_usb_pc_driver_id_get(void);
u8 bsp_usb_pc_driver_subclass_get(void);

/* ncm host assemble function */
int ncm_set_host_assemble_param(int net_id, unsigned long host_timeout);

#ifdef CONFIG_GADGET_SUPPORT_LPM_L2	
void 	bsp_usb_unlock_wakelock(void);
void 	bsp_usb_lock_wakelock(void);

int power_on_dwc3_usb(void);
int power_off_dwc3_usb(void);
#endif 
#ifdef USB_CHARGE_EXT
extern int otg_dev_id_state();
#endif

#if (FEATURE_ON == MBB_USB)
USB_INT usb_get_vbus_status(USB_VOID);
USB_INT usb_set_vbus_status(USB_INT value);
void set_value_usb_setup_num(unsigned char value);
#ifdef USB_CHARGE
USB_ULONG get_chg_insert_time(void);
USB_ULONG get_chg_remove_time(void);
#endif
#endif

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif    /* End of __USB_VENDOR_H__ */

