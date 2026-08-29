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

#ifndef __MBB_GPIO_H__
#define __MBB_GPIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if ((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) 
#define PRODUCT_GPIO_RANGE         "000000000000000000000000000-111111111111111111111111111"
#define GPIO_CONTROL_NUMBER           (27)
#else
#if (FEATURE_ON == MBB_IOCFG_HERMES)
#define PRODUCT_GPIO_RANGE         "000000000000000-111111111111111"
#define GPIO_CONTROL_NUMBER           (15)
#elif (FEATURE_ON == MBB_IOCFG_AUDI)
#define PRODUCT_GPIO_RANGE         "00000000000-11111111111"
#define GPIO_CONTROL_NUMBER           (11)
#else
#define PRODUCT_GPIO_RANGE           "0-1"
#define GPIO_CONTROL_NUMBER           (1)
#endif
#endif  /*MBB_FACTORY, MBB_AGING_TEST*/
#else
#define PRODUCT_GPIO_RANGE         "0000000-1111111"
#define GPIO_CONTROL_NUMBER           (7)
#endif  /*BSP_CONFIG_BOARD_TELEMATIC*/

typedef enum{
    GPIO_CMD_SET = 0,
    GPIO_CMD_GET = 1,
    GPIO_CMD_MAX,
}GPIO_CMD_E;

/*环回测试结果枚举定义*/
typedef enum
{
    GPIOLOOP_SUCCESS = 0,
    GPIOLOOP_FAIL,
}GPIOLOOP_RESULT_ENUM;

/*支持环回测试的GPIO最大个数*/
#define GPIO_TEST_NUM_MAX    (32)

/*环回测试结果数据结构定义*/
typedef struct
{
    GPIOLOOP_RESULT_ENUM test_result;
    unsigned int fail_num;
    unsigned char fail_list[GPIO_TEST_NUM_MAX];
}GPIOLOOP_RST_STRU;

/*****************************************************************************
 函 数 名  : drv_gpio_ioctrl_cmd
 功能描述  : 供AT^IOCTRL调用，设置或者获取提供给客户的gpio方向和电平
 输入参数  : cmd:0--SET,1--GET;
             *gpio:需要设置的gpio，如"0110111",当cmd为1时此参数可忽略；
             *direction:需要设置或者获取到的gpio方向，1为输出，0为输入；
             *value: 需要设置或获取到的gpio电平，1为高电平，0为低电平；
 输出参数  : 
 返 回 值  :  0--成功，1--失败
 说    明  : 调用者需要保证传入的参数有效或者接受的地址范围足够大。
*****************************************************************************/
int drv_gpio_ioctrl_cmd(GPIO_CMD_E cmd, unsigned char *gpio, unsigned char *direction, unsigned char *value);


extern int drv_gpioloop_test_process(GPIOLOOP_RST_STRU *p_ret);


extern int drv_get_support_gpioloop_info(unsigned int *pt_support, unsigned char *pt_num);


extern void drv_gpioloop_set_function_to_gpio(void);


extern void drv_gpioloop_set_gpio_to_default(void);
#ifdef __cplusplus
}
#endif

#endif

