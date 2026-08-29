 /****************************************************************************
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
 ******************************************************************************/


#ifndef __MDRV_NFC__H__
#define __MDRV_NFC__H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define NFC_IC_DRVNAME                      ("m24sr02")
#define NFC_COMPATIBLE_NAME                 ("st,m24sr02")
#define HUAWEI_NFC_URL_MESSAGE              ("consumer.huawei.com/minisite/mobilewifiapp/download.html\0")

#define TRUE   (1)
#define FALSE  (0)

#define HUAWEI_NFC_MAX_SSID_SIZE        (32)
#define HUAWEI_NFC_MAX_PWD_SIZE         (64)
#define HUAWEI_NFC_MAX_SEC_SIZE         (8)
#define HUAWEI_NFC_MAX_URL_SIZE         (64)

#define HUAWEI_NFC_ENABLE_ON            (1)
#define HUAWEI_NFC_ENABLE_OFF           (0)

#define HUAWEI_NFC_ENABLE_VALUE         (0x01)
#define HUAWEI_NFC_DISABLE_VALUE        (0x00)

#define HUWAEI_NFC_ENABLE_BUFF_SIZE     (2)

#define HUAWEI_NFC_RF_DISABLE_OFFSET     (0x0006)
#define HUAWEI_NFC_READ_RF_DISABLE_BYTES (0x01)


/*----------------------------------------------*
 * 结构体定义                                   *
 *----------------------------------------------*/
struct huawei_nfc_device
{
    struct class*    nfc_class;
    struct device*   nfc_dev;
    struct i2c_client* nfc_i2c_client;
    struct work_struct  work;
    int nfc_int_gpio;
    struct wake_lock nfc_wake_lock;
};

typedef struct 
{
    char NetworkSSID[HUAWEI_NFC_MAX_SSID_SIZE + 1];
    char NetworkKey[HUAWEI_NFC_MAX_PWD_SIZE + 1];
    char NetSecType[HUAWEI_NFC_MAX_SEC_SIZE + 1];
    char URL[HUAWEI_NFC_MAX_URL_SIZE + 1];
    char reserved;
}huawei_nfc_token;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
struct i2c_client* huawei_nfc_get_handle(void);
unsigned short huawei_nfc_write_info(huawei_nfc_token *nfc_info);
unsigned short huawei_nfc_read_info(huawei_nfc_token *nfc_info);
int huawei_nfc_rf_enable(unsigned char rf_enable);
int huawei_nfc_is_initialized(void);


 #endif /*__MDRV_NFC__H__*/
