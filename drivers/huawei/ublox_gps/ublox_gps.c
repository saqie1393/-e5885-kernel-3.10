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

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>

#include "product_config.h"
#include "pmic_volt.h"
#include "bsp_version.h"
#include "ublox_gps.h"
#include "bsp_pmu.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ( FEATURE_ON == MBB_FEATURE_GPS_UBLOX )
/******************************************************************************
  1 模块私有 (宏、枚举、结构体、自定义数据类型) 定义区:
******************************************************************************/

/******************************************************************************
  2 全局变量定义区:
******************************************************************************/

/******************************************************************************
  3 函数定义区:
******************************************************************************/
/*lint -e2 -e10 -e26*/
/********************************************************
*函 数 名   : ubloxGpsIoctl
*函数功能: GPS驱动ioctl函数，用于处理用户态程序下发的指令
*输入参数:
*输出参数: 无
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
long ubloxGpsIoctl( struct file *file, unsigned int cmd, unsigned long arg )
{
    int ret = 0;

    if ( NULL == file )
    {
        GPS_PRINT( KERN_ERR, "file is NULL !\r\n" );
        return -1;
    }

    switch( cmd )
    {
        case GPS_POWER_ON_CMD:
        {
            /*ublox gps芯片上电*/
            ret = pmic_volt_enable( GPS_UBX_CHIP_POWER );
            break;
        }
        case GPS_POWER_OFF_CMD:
        {
            /*ublox gps芯片下电*/
            ret = pmic_volt_disable( GPS_UBX_CHIP_POWER );
            break;
        }
#if ( FEATURE_ON == MBB_FACTORY )
        case GPS_UPDATE_GPIO_SET_CMD:
        {
            if ( (0 != arg) && (1 != arg) )
            {
                GPS_PRINT( KERN_ERR, "parameter invalid !\r\n" );
                return ( - EINVAL );
            }

            ret = gpio_request( GPS_UPDATE, "ublox_gps");

            if ( (-EINVAL) == ret )
            {
                GPS_PRINT( KERN_ERR, "gpio request failed !\r\n" );
                return ( - EINVAL );
            }

            /*gps update gpio电平状态设置*/
            ret = gpio_direction_output( GPS_UPDATE, arg );
            break;
        }
#endif /* FEATURE_ON == MBB_FACTORY */
        default:
            /*驱动不支持该命令*/
            GPS_PRINT( KERN_ERR, "command no support !\r\n" );
            return ( - ENOTTY );
    }

    return ret;
}

static const struct file_operations gpsCtrlFops = {
    .owner         = THIS_MODULE,
    .unlocked_ioctl = ubloxGpsIoctl,
};

static struct miscdevice gpsmiscdevice = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name    = "ublox_gps",
    .fops    = &gpsCtrlFops
};

static int __init ublox_gps_init(void)
{
    int ret = 0;
    SOLUTION_PRODUCT_TYPE product_type = PRODUCT_TYPE_INVALID;

    GPS_PRINT( KERN_INFO, "ublox_gps_init IN\n" );

    product_type = bsp_get_solution_type();
    if ( PRODUCT_TYPE_TELEMATIC != product_type )
    {
        GPS_PRINT( KERN_ERR, "ublox_gps_init OUT, !TELEMATIC product\n" );
        return ret;
    }

    /*使能32k clk*/
    ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_B);
    if (ret)
    {
        GPS_PRINT( KERN_ERR, "enable 32k clock failed!\n");
        return ret;
    }

    /*注册混杂设备*/
    ret = misc_register( &gpsmiscdevice );
    if (0 > ret)
    {
        GPS_PRINT( KERN_ERR,"ERROR: gpsmiscdevice register failed !!! \r\n" );
        return ret;
    }

    GPS_PRINT( KERN_INFO, "ublox_gps_init successful OUT\n" );

    return ret;
}

static void __exit ublox_gps_exit(void)
{
    SOLUTION_PRODUCT_TYPE product_type = PRODUCT_TYPE_INVALID;

    product_type = bsp_get_solution_type();
    if ( PRODUCT_TYPE_TELEMATIC != product_type )
    {
        GPS_PRINT( KERN_INFO, "gps_adapter_init OUT, !TELEMATIC product\n" );
        return;
    }

#if ( FEATURE_ON == MBB_FACTORY )
    gpio_free( GPS_UPDATE );
#endif /* FEATURE_ON == MBB_FACTORY */
    misc_deregister( &gpsmiscdevice );
}

module_init( ublox_gps_init );
module_exit( ublox_gps_exit );

MODULE_AUTHOR( "Huawei Device" );
MODULE_DESCRIPTION( "ublox gps" );
MODULE_LICENSE( "GPL" );
/*lint +e2 +e10 +e26*/
#endif /*FEATURE_ON == MBB_FEATURE_GPS_UBLOX*/

#ifdef __cplusplus
}
#endif
