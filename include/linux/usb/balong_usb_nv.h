 /*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2016. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
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

#ifndef __BALONG_USB_NV_H__
#define __BALONG_USB_NV_H__

#ifdef __cplusplus
extern "C" { /* allow C++ to use these headers */
#endif /* __cplusplus */

#include "osl_types.h"

/* usb dbg module */
#define USB_DBG_NV  (NV_ID_DRV_USB_DBG)

typedef enum usb_pc_driver
{
    JUNGO_DRIVER,
    HUAWEI_PC_DRIVER,
    HUAWEI_MODULE_DRIVER
} usb_pc_driver_t;

typedef enum usb_u1u2_enable
{
    USB_U1U2_DISABLE,
    USB_U1U2_ENABLE_WITH_WORKAROUND,
    USB_U1U2_ENABLE_WITHOUT_WORKAROUND
} usb_u1u2_enable_t;

/*0=pmu detect; 
1=no detect(for fpga); 
2=car module vbus detect, disconnect detect by chip, connect detect by gpio 
*/
typedef enum usb_vbus_detect_mode
{
    USB_PMU_DETECT,
    USB_NO_DETECT,
    USB_TBOX_DETECT
} usb_vbus_detect_mode_t;


typedef enum usb_otg_detect_detect_mode
{
    USB_OTG_GPIO_DETECT = 1,
} usb_otg_detect_detect_mode_t;


typedef enum usb_mode_support
{
    USB_DEVICE,
    USB_HOST,
    USB_DRD
} usb_mode_support_t;


/* usb nv feature functions */
void bsp_usb_nv_init(void);

int bsp_usb_is_force_highspeed(void);

int bsp_usb_is_support_phy_apd(void);

int bsp_usb_is_support_hibernation(void);

int bsp_usb_is_enable_u1u2_workaround(void);

int bsp_usb_is_ncm_bypass_mode(void);

int bsp_usb_is_vbus_connect(void);

int bsp_usb_vbus_detect_mode(void);

/*Host, Device, DRD*/
int bsp_usb_mode_support(void);

/* 1: support, 0:not support */
int bsp_usb_is_support_wwan(void);
int bsp_usb_otg_gpio_detect(void);
int bsp_usb_is_support_shell(void);

/*get driver information from nv*/
u16 bsp_usb_pc_driver_id_get(void);
u8 bsp_usb_pc_driver_subclass_get(void);


#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif    /* End of __BSP_USB_H__ */

