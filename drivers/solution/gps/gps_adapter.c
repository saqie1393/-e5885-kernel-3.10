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
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>

#include "bsp_pmu.h"
#include "bsp_version.h"
#include "gps_adapter.h"
//#include "gps_hwctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (FEATURE_ON == MBB_FEATURE_GPS)

/******************************************************************************
  1 模块私有 (宏、枚举、结构体、自定义数据类型) 定义区:
******************************************************************************/
extern void ATNL_StopGPSCB( void );

/******************************************************************************
  2 全局变量定义区:
******************************************************************************/
static struct regulator *gps_vcc = NULL;
static struct wake_lock  gps_lock;
static struct wake_lock  gps_msg_lock;
static struct delayed_work gps_disable_isr_work;/*lint !e86*/


/******************************************************************************
  3 函数定义区:
******************************************************************************/
/*lint -e2 -e10 -e26*/
/********************************************************
*函 数 名   : gps_pmu_power_switch
*函数功能: GPS电源开关，用于开启关闭GPS芯片电源
*输入参数: GPS_POWER_STATE enPara
*输出参数: 无
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
/*int gps_pmu_power_switch(GPS_POWER_STATE enPara)
{
    int ret = 0;

    if(GPS_POWER_ON == enPara)
    {
        ret = regulator_enable(gps_vcc);
    }
    else if(GPS_POWER_OFF == enPara)
    {
        ret = regulator_disable(gps_vcc);
    }

    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_enable GPS_VCC failed, ret %d \n", ret);
    }

    return ret;

}
*/
/********************************************************
*函 数 名   : gps_32k_clk_switch
*函数功能: GPS 32k时钟开关，用于开启关闭GPS 32k时钟
*输入参数: GPS_POWER_STATE enPara
*输出参数: 无
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
/*int gps_32k_clk_switch(GPS_POWER_STATE enPara)
{
    int ret = 0;
    if(GPS_32K_CLK_OFF == enPara)
    {
        ret = bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);
    }
    else if(GPS_32K_CLK_ON == enPara)
    {
        ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_B);
    }

    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: enable gps 32k clock failed, ret %d \n",ret);
        return -1;
    }

    GPS_PRINT(KERN_INFO, "gps_adapter_init successful out\n");

    return 0;

}
*/
/********************************************************
*函 数 名   : gps_disable_pin_level_qury
*函数功能: 查询GPS disable管脚状态
*输入参数: 
*输出参数: unsigned int *enPara
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
int gps_disable_pin_level_qury(unsigned int *enPara)
{
    unsigned int uclevel = 0;
    uclevel = !!gpio_get_value(GPS_DISABLE_PIN);
    GPS_PRINT(KERN_ERR, "gps disable pin level is %d \n",uclevel);

    if(copy_to_user(enPara, &uclevel, sizeof(unsigned int)))
    {
        GPS_PRINT(KERN_ERR, "copy data to user OK **\n ");
    }

    return 0;
}

/********************************************************
*函 数 名   : gps_hwcust_pin_level_qury
*函数功能: 查询GPS hwcust管脚状态
*输入参数: 
*输出参数: unsigned int *enPara
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
/*int gps_hwcust_pin_level_qury(unsigned int *enPara)
{
    unsigned int uclevel = 0;
#if 0  //todo
    uclevel = !!gpio_get_value(GPS_DISABLE_PIN);
    GPS_PRINT(KERN_ERR, "gps disable pin level is %d \n",uclevel);
    if(copy_to_user(enPara, &uclevel, sizeof(unsigned int)))
    {
        GPS_PRINT(KERN_ERR "copy data to user OK **\n ");
    }
#endif

    return 0;
}
*/
/********************************************************
*函 数 名   : gps_pm_lock_ctrl
*函数功能: GPS向电源管理投票开关，用于电源管理投票
*输入参数: GPS_LOCK_STATE enPara
*输出参数: 无
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
void gps_pm_lock_ctrl(GPS_LOCK_STATE enPara)
{
    if(GPS_PM_UNLOCK == enPara)
    {
        wake_unlock(&gps_lock);
    }
    else if(GPS_PM_LOCK == enPara)
    {
        wake_lock(&gps_lock);
    }
    else
    {
        GPS_PRINT(KERN_ERR, "para error !!! \n ");
    }
}

/*===========================================================================
FUNCTION 
    gps_disablepin_event_isr
DESCRIPTION
    The interrupt handler for GPS_DISABLE interrupt.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
static void gps_disablepin_event_isr_handle(struct work_struct* data)
{
    unsigned int uclevel = 0;
    
    uclevel = gpio_get_value(GPS_DISABLE_PIN);

    if ( uclevel )
    {
        GPS_PRINT(KERN_ERR, " high level irq happen !\n");
    }
    else
    {
        GPS_PRINT(KERN_ERR, " low level irq happen !\n");
        ATNL_StopGPSCB();
    }

}
/*===========================================================================
FUNCTION 
    gps_disablepin_event_isr
DESCRIPTION
    The interrupt handler for GPS_DISABLE interrupt.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
static irqreturn_t gps_disablepin_event_isr(void)
{
    wake_lock_timeout(&gps_msg_lock, (long)msecs_to_jiffies(PM_GPS_MSG_TIME));
    
    /*Start timer*/
    schedule_delayed_work(&gps_disable_isr_work, msecs_to_jiffies(PM_GPS_WORK_DELAY_TIME));/*lint !e64*/
    
    return IRQ_HANDLED;
}

/******************************************************************************
Function:       huawei_pm_wakeupin_gpio_init
Description:   Initialize the GPS_DISABLE gpio
Input:           None
Output:         None
Return:         int, 0 = success, -1 = failed
Others:         None
******************************************************************************/
int gps_disable_gpio_init(void)
{
    int ret = 0;
    unsigned int int_num;

    ret = gpio_request(GPS_DISABLE_PIN, LABEL_GPS_DISABLE);
    if(ret)
    {
        GPS_PRINT(KERN_ERR, "request GPS_DISABLE_PIN fail !!! \n");
        goto fail1;
    }

    /*设置GPIO为输入*/
    gpio_direction_input(GPS_DISABLE_PIN);

    int_num = gpio_to_irq(GPS_DISABLE_PIN);
    /*挂载中断号对应的回调函数*/
    ret = request_irq(int_num, (irq_handler_t) gps_disablepin_event_isr,
        IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, LABEL_GPS_DISABLE, "gps_disable");
    
    if (ret) 
    {
        GPS_PRINT(KERN_ERR," request GPS_DISABLE irq fail !!! \n");
        goto fail2;
    }
    
    return 0;

fail2:
    gpio_free(GPS_DISABLE_PIN);
fail1:
    return -1;
}


/********************************************************
*函 数 名   : gpsControlIoctl
*函数功能: GPS驱动ioctl函数，用于处理用户态程序下发的指令
*输入参数: 
*输出参数: 无
*返 回 值   : 执行成功返回0，失败返回非0值
********************************************************/
long gpsControlIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if(NULL == file )
    {
        GPS_PRINT(KERN_ERR, "file is NULL!\r\n");
        return -1;
    }

    //GPS_PRINT(KERN_ERR, " cmd=0x%x, data = 0x%x !\r\n",cmd,arg);

    switch(cmd)
    {
        case GPS_POWER_CTRL_CMD:
        {
            /*gps电源控制*/
            //ret = gps_pmu_power_switch((GPS_POWER_STATE)arg);
            break;
        }
        case GPS_CLOCK_CTRL_CMD:
        {
            /*gps的32K时钟控制*/
           // ret = gps_32k_clk_switch((GPS_32K_CLK_STATE)arg);
            break;
        }
        case GPS_DISABLE_QUY_CMD:
        {
            /*gps disable pin脚电平状态查询*/
            ret = gps_disable_pin_level_qury((unsigned int *)arg);
            break;
        }
        case GPS_HWCUST_PIN_SET_CMD:
        {
            /*gps disable pin脚电平状态查询*/
            break;
        }
        case GPS_HWCUST_PIN_QUY_CMD:
        {
            /*gps disable pin脚电平状态查询*/
            break;
        }
        case GPS_LOCK_CTRL_CMD:
        {
            /*gps休眠唤醒控制接口*/
            gps_pm_lock_ctrl((GPS_LOCK_STATE)arg);
            break;
        }
        default:
            /*驱动不支持该命令*/
            return ( - ENOTTY);
    }
    
    return ret;
}

static const struct file_operations gpsCtrlFops = {
    .owner         = THIS_MODULE,
    .unlocked_ioctl = gpsControlIoctl,
};

static struct miscdevice gpsmiscdevice = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name    = "gps_control",
    .fops    = &gpsCtrlFops
};
/*end add for debug*/

static int __init gps_adapter_init(void)
{
    int ret = 0;
    SOLUTION_PRODUCT_TYPE product_type = PRODUCT_TYPE_INVALID;

    GPS_PRINT(KERN_INFO, "gps_adapter_init IN\n");

    product_type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE != product_type)
    {
        GPS_PRINT(KERN_INFO, "gps_adapter_init OUT, !CE product\n");
        return ret;
    }
    
    INIT_DELAYED_WORK(&gps_disable_isr_work, 
                     gps_disablepin_event_isr_handle);

    /* 用于gps工作锁的初始化*/
    wake_lock_init(&gps_lock, WAKE_LOCK_SUSPEND, "pm_gps");
    wake_lock_init(&gps_msg_lock, WAKE_LOCK_SUSPEND, "pm_gps_msg");

    /*1. 设置GPS电源*/
    gps_vcc = regulator_get(NULL, "gps-vcc");
    if (IS_ERR(gps_vcc))
    {
        GPS_PRINT(KERN_ERR, "ERROR: GPS_VCC regulator_get error:%d!/n", 
        (int)PTR_ERR(gps_vcc));
        return -1;
    }

    ret = regulator_set_voltage(gps_vcc, GPS_VOLTAGE_UV, GPS_VOLTAGE_UV);
    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_set_voltage failed, ret = %d\n", ret);
        goto Error5;
    }

    ret = regulator_enable(gps_vcc);
    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_enable GPS_VCC failed, ret %d \n", ret);
        goto Error5;
    }

    /*gps standby GPIO控制*/
    /*增加GPIO_2_25作为GPS standby 管脚，输出，初始为低电平*/
    ret = gpio_request(GPS_GPIO, "gpsstandby");
    if (ret < 0)
    {
        GPS_PRINT(KERN_ERR, "ERROR: request gps standy gpio failed, ret %d \n", ret);
        goto Error4;
    }

    /*将standby管脚export到应用态，供Broadcom进程调用，我们自己不操作此管脚*/
    ret = gpio_export(GPS_GPIO, true);
    if (ret < 0)
    {
        GPS_PRINT(KERN_ERR, "ERROR: export gps standy gpio failed, ret %d \n", ret);
        goto Error3;
    }
    /*standby管脚上电默认为低电平状态*/
    gpio_direction_output(GPS_GPIO, 0);

    /*2. 输出32K时钟*/
    ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_B);
    if (0 > ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: enable gps 32k clock failed, ret %d \n", ret);
        goto Error3;
    }

    /*3. 注册混杂设备*/
    ret = misc_register(&gpsmiscdevice);
    if (0 > ret)
    {
        GPS_PRINT(KERN_ERR,"ERROR: gpsmiscdevice register failed !!! \r\n");
        goto Error2;
    }

    ret = gps_disable_gpio_init();
    if (ret)
    {
        GPS_PRINT(KERN_ERR,"ERROR: gps_disable_gpio_init failed !!! \r\n");
        goto Error1;
    }

    GPS_PRINT(KERN_ERR, "gps_adapter_init successful out\n");

    return 0;

Error1:
    misc_deregister(&gpsmiscdevice);

Error2:
    bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);

Error3:
    gpio_free(GPS_GPIO);

Error4:
    regulator_disable(gps_vcc);

Error5:
    regulator_put(gps_vcc);

    return ret;
}
void gps_adapter_exit_handle(void)
{
    SOLUTION_PRODUCT_TYPE product_type = PRODUCT_TYPE_INVALID;

    product_type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE != product_type)
    {
        GPS_PRINT(KERN_INFO, "gps_adapter_init OUT, !CE product\n");
        return;
    }

    misc_deregister(&gpsmiscdevice);
    bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);
    /*standby管脚上电默认为低电平状态*/
    gpio_direction_output(GPS_GPIO, 0);
    regulator_disable(gps_vcc);
    regulator_put(gps_vcc);
}


static void __exit gps_adapter_exit(void)
{
    gps_adapter_exit_handle();
}
module_init(gps_adapter_init);
module_exit(gps_adapter_exit);
/*lint +e2 +e10 +e26*/
#endif

#ifdef __cplusplus
}
#endif
