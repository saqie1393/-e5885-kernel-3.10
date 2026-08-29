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


#include <linux/gpio.h>
#include "mbb_gpio.h"
#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(BSP_CONFIG_BOARD_TELEMATIC))
#include "product_config.h"
#include "bsp_version.h"
#include "mbb_bsp_reg_def.h"
#include "hi_gpio.h"
#include "ios_ao_drv_macro.h"
#include "ios_pd_drv_macro.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define gpio_print_err(fmt, ...)  (printk(KERN_ERR "[%s]:"fmt"\n", __func__, ##__VA_ARGS__))

typedef struct GPIO_CTRL_STRU_E
{
    unsigned int          gpio_pin;        /*参考AT文档 对应GPIO PIN 如1*/
    unsigned int          gpio_num;        /*设置GPIO电平或者方向的参数，通过x*8 + y所得，如GPIO_1_7 对应的值为1*8+7=15*/
    const unsigned char*  gpio_label;      /*硬件编码如GPIO_6_4*/
    const unsigned char*  gpio_desc;       /*对客户的编号*/
}GPIO_CTRL_STRU;

/*环回测试GPIO数据结构定义*/
typedef struct
{
    unsigned int pin_num;
    unsigned int gpio_num;
    const unsigned char* gpio_req_label;
    unsigned int request_state;
}GPIO_INFO_STRU;

/*环回类型枚举定义*/
typedef enum
{
    A_OUT_B_IN = 0,/*a TO b环回*/
    BI_DIRECTIONAL_LOOP,/*双向环回*/
}GPIOLOOP_TYPE_ENUM;

/*环回测试数据结构定义*/
typedef struct
{
    GPIO_INFO_STRU test_a;
    GPIO_INFO_STRU test_b;
    GPIOLOOP_TYPE_ENUM test_type;
    GPIOLOOP_RESULT_ENUM result;
}AT_GPIO_LOOP_STRU;

#ifdef BSP_CONFIG_BOARD_TELEMATIC
/*车载模块BC工位测试GPIO适配下面的参数，指定测试17个GPIO*/
#if ((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST))
GPIO_CTRL_STRU g_gpio_ctrl_info_tb[GPIO_CONTROL_NUMBER] = {
    {1, 37, (unsigned char*)"GPIO_4_5" ,(unsigned char*)"client_gpio1"},      /*第1个GPIO，硬件编码如GPIO_4_5*/
    {2, 60, (unsigned char*)"GPIO_7_4" ,(unsigned char*)"client_gpio2"},      /*第2个GPIO，硬件编码如GPIO_7_4*/
    {3, 59, (unsigned char*)"GPIO_7_3" ,(unsigned char*)"client_gpio3"},      /*第3个GPIO，硬件编码如GPIO_7_3*/
    {4, 58, (unsigned char*)"GPIO_7_2" ,(unsigned char*)"client_gpio4"},      /*第4个GPIO，硬件编码如GPIO_7_2*/
    {5, 1, (unsigned char*)"GPIO_0_1" ,(unsigned char*)"client_gpio5"},       /*第5个GPIO，硬件编码如GPIO_0_1*/
    {6, 22, (unsigned char*)"GPIO_2_6" ,(unsigned char*)"client_gpio6"},      /*第6个GPIO，硬件编码如GPIO_2_6*/
    {7, 47, (unsigned char*)"GPIO_5_7" ,(unsigned char*)"client_gpio7"},      /*第7个GPIO，硬件编码如GPIO_5_7*/
    {8, 43, (unsigned char*)"GPIO_5_3" ,(unsigned char*)"client_gpio8"},      /*第8个GPIO，硬件编码如GPIO_5_3*/
    {9, 42, (unsigned char*)"GPIO_5_2" ,(unsigned char*)"client_gpio9"},      /*第9个GPIO，硬件编码如GPIO_5_2*/
    {10, 45, (unsigned char*)"GPIO_5_5" ,(unsigned char*)"client_gpio10"},   /*第10个GPIO，硬件编码如GPIO_5_5*/
    {11, 41, (unsigned char*)"GPIO_5_1" ,(unsigned char*)"client_gpio11"},   /*第11个GPIO，硬件编码如GPIO_5_1*/
    {12, 63, (unsigned char*)"GPIO_7_7" ,(unsigned char*)"client_gpio12"},   /*第12个GPIO，硬件编码如GPIO_7_7*/
    {13, 94, (unsigned char*)"GPIO_11_6" ,(unsigned char*)"client_gpio13"}, /*第13个GPIO，硬件编码如GPIO_11_6*/
    {14, 6, (unsigned char*)"GPIO_0_6" ,(unsigned char*)"client_gpio14"},    /*第14个GPIO，硬件编码如GPIO_0_6*/
    {15, 93, (unsigned char*)"GPIO_11_5" ,(unsigned char*)"client_gpio15"}, /*第15个GPIO，硬件编码如GPIO_11_5*/
    {16, 50, (unsigned char*)"GPIO_6_2" ,(unsigned char*)"client_gpio16"},   /*第16个GPIO，硬件编码如GPIO_6_2*/
    {17, 51, (unsigned char*)"GPIO_6_3" ,(unsigned char*)"client_gpio17"},   /*第17个GPIO，硬件编码如GPIO_6_3*/
    {18, 31, (unsigned char*)"GPIO_3_7" ,(unsigned char*)"client_gpio18"},     /*第18个GPIO，硬件编码如GPIO_3_7*/
    {19, 32, (unsigned char*)"GPIO_4_0" ,(unsigned char*)"client_gpio19"},     /*第19个GPIO，硬件编码如GPIO_4_0*/
    {20, 52, (unsigned char*)"GPIO_6_4" ,(unsigned char*)"client_gpio20"},     /*第20个GPIO，硬件编码如GPIO_6_4*/
    {21, 53, (unsigned char*)"GPIO_6_5" ,(unsigned char*)"client_gpio21"},     /*第21个GPIO，硬件编码如GPIO_6_5*/
    {22, 2, (unsigned char*)"GPIO_0_2" ,(unsigned char*)"client_gpio22"},      /*第22个GPIO，硬件编码如GPIO_0_2*/
    {23, 92, (unsigned char*)"GPIO_11_4" ,(unsigned char*)"client_gpio23"},    /*第23个GPIO，硬件编码如GPIO_11_4*/
    {24, 7, (unsigned char*)"GPIO_0_7" ,(unsigned char*)"client_gpio24"},      /*第24个GPIO，硬件编码如GPIO_0_7*/
    {25, 91, (unsigned char*)"GPIO_11_3" ,(unsigned char*)"client_gpio25"},    /*第25个GPIO，硬件编码如GPIO_11_3*/
    {26, 0, (unsigned char*)"GPIO_0_0" ,(unsigned char*)"client_gpio26"},      /*第26个GPIO，硬件编码如GPIO_0_0*/
    {27, 5, (unsigned char*)"GPIO_0_5" ,(unsigned char*)"client_gpio27"},      /*第27个GPIO，硬件编码如GPIO_0_0*/
};
/*车载模块给客户预留的gpio*/
#else
#if (FEATURE_ON == MBB_IOCFG_HERMES)
GPIO_CTRL_STRU g_gpio_ctrl_info_tb[GPIO_CONTROL_NUMBER] = {
    {1, 37, (unsigned char*)"GPIO_4_5" ,(unsigned char*)"client_gpio1"},       /*第1个GPIO，硬件编码如GPIO_4_5*/
    {2, 60, (unsigned char*)"GPIO_7_4" ,(unsigned char*)"client_gpio2"},       /*第2个GPIO，硬件编码如GPIO_7_4*/
    {3, 59, (unsigned char*)"GPIO_7_3" ,(unsigned char*)"client_gpio3"},       /*第3个GPIO，硬件编码如GPIO_7_3*/
    {4, 58, (unsigned char*)"GPIO_7_2" ,(unsigned char*)"client_gpio4"},       /*第4个GPIO，硬件编码如GPIO_7_2*/
    {5, 1, (unsigned char*)"GPIO_0_1" ,(unsigned char*)"client_gpio5"},        /*第5个GPIO，硬件编码如GPIO_0_1*/
    {6, 22, (unsigned char*)"GPIO_2_6" ,(unsigned char*)"client_gpio6"},       /*第6个GPIO，硬件编码如GPIO_2_6*/
    {7, 47, (unsigned char*)"GPIO_5_7" ,(unsigned char*)"client_gpio7"},       /*第7个GPIO，硬件编码如GPIO_5_7*/
    {8, 43, (unsigned char*)"GPIO_5_3" ,(unsigned char*)"client_gpio8"},       /*第8个GPIO，硬件编码如GPIO_5_3*/
    {9, 42, (unsigned char*)"GPIO_5_2" ,(unsigned char*)"client_gpio9"},       /*第9个GPIO，硬件编码如GPIO_5_2*/
    {10, 45, (unsigned char*)"GPIO_5_5" ,(unsigned char*)"client_gpio10"},    /*第10个GPIO，硬件编码如GPIO_5_5*/
    {11, 94, (unsigned char*)"GPIO_11_6" ,(unsigned char*)"client_gpio11"},  /*第11个GPIO，硬件编码如GPIO_11_6*/
    {12, 6, (unsigned char*)"GPIO_0_6" ,(unsigned char*)"client_gpio12"},     /*第12个GPIO，硬件编码如GPIO_0_6*/
    {13, 93, (unsigned char*)"GPIO_11_5" ,(unsigned char*)"client_gpio13"},  /*第13个GPIO，硬件编码如GPIO_11_5*/
    {14, 50, (unsigned char*)"GPIO_6_2" ,(unsigned char*)"client_gpio14"},    /*第14个GPIO，硬件编码如GPIO_6_2*/
    {15, 51, (unsigned char*)"GPIO_6_3" ,(unsigned char*)"client_gpio15"},    /*第15个GPIO，硬件编码如GPIO_6_3*/
};
#elif (FEATURE_ON == MBB_IOCFG_AUDI)
GPIO_CTRL_STRU g_gpio_ctrl_info_tb[GPIO_CONTROL_NUMBER] =
{
    {1, GPIO_4_5, (unsigned char *)"GPIO_4_5", (unsigned char *)"client_gpio1"},          /* 第1个GPIO，硬件编码如GPIO_4_5 */
    {2, GPIO_7_4, (unsigned char *)"GPIO_7_4", (unsigned char *)"client_gpio2"},          /* 第2个GPIO，硬件编码如GPIO_7_4 */
    {3, GPIO_7_3, (unsigned char *)"GPIO_7_3", (unsigned char *)"client_gpio3"},          /* 第3个GPIO，硬件编码如GPIO_7_3 */
    {4, GPIO_7_2, (unsigned char *)"GPIO_7_2", (unsigned char *)"client_gpio4"},          /* 第4个GPIO，硬件编码如GPIO_7_2 */
    {5, GPIO_0_1, (unsigned char *)"GPIO_0_1", (unsigned char *)"client_gpio5"},          /* 第5个GPIO，硬件编码如GPIO_0_1 */
    {6, GPIO_5_7, (unsigned char *)"GPIO_5_7", (unsigned char *)"client_gpio6"},          /* 第6个GPIO，硬件编码如GPIO_5_7 */
    {7, GPIO_5_3, (unsigned char *)"GPIO_5_3", (unsigned char *)"client_gpio7"},          /* 第7个GPIO，硬件编码如GPIO_5_3 */
    {8, GPIO_5_5, (unsigned char *)"GPIO_5_5", (unsigned char *)"client_gpio8"},          /* 第8个GPIO，硬件编码如GPIO_5_5 */
    {9, GPIO_11_4, (unsigned char *)"GPIO_11_4", (unsigned char *)"client_gpio9"},        /* 第9个GPIO，硬件编码如GPIO_11_4 */
    {10, GPIO_0_7, (unsigned char *)"GPIO_0_7", (unsigned char *)"client_gpio10"},        /* 第10个GPIO，硬件编码如GPIO_0_7 */
    {11, GPIO_11_3, (unsigned char *)"GPIO_11_3", (unsigned char *)"client_gpio11"},      /* 第11个GPIO，硬件编码如GPIO_11_3 */
};
#else
GPIO_CTRL_STRU g_gpio_ctrl_info_tb[GPIO_CONTROL_NUMBER] = {{0, 0, NULL, NULL}};
#endif
#endif  /*MBB_FACTORY, MBB_AGING_TEST*/
#else
/*每个产品需要适配下面的参数，该at只支持7个gpio，由硬件提供*/
GPIO_CTRL_STRU g_gpio_ctrl_info_tb[GPIO_CONTROL_NUMBER] = {
    {1, 52, (unsigned char*)"GPIO_6_4" ,(unsigned char*)"client_gpio1"},
    {2, 53, (unsigned char*)"GPIO_6_5" ,(unsigned char*)"client_gpio2"},
    {3, 54, (unsigned char*)"GPIO_6_6" ,(unsigned char*)"client_gpio3"},
    {4, 55, (unsigned char*)"GPIO_6_7" ,(unsigned char*)"client_gpio4"},
    {5, 56, (unsigned char*)"GPIO_7_0" ,(unsigned char*)"client_gpio5"},
    {6, 57, (unsigned char*)"GPIO_7_1" ,(unsigned char*)"client_gpio6"},
    {7, 58, (unsigned char*)"GPIO_7_2" ,(unsigned char*)"client_gpio7"},
};
#endif  /*BSP_CONFIG_BOARD_TELEMATIC*/

/*记录gpio的request状态*/
static unsigned char gpio_request_flag[GPIO_CONTROL_NUMBER + 1] = {0};

/*标准内核不支持gpio方向的获取，这里需要记录设置的方向，默认为输入 0*/
static unsigned char gpio_direction_record[GPIO_CONTROL_NUMBER + 1] = {0};

/*注意：这个list是需要测试的GPIO信息配置，具体产品需要适配*/
AT_GPIO_LOOP_STRU g_gpioloop_test_tbl[] = 
{
    {{52, GPIO_6_4, "GPIO_6_4", 0}, {53, GPIO_6_5, "GPIO_6_5", 0}, A_OUT_B_IN, GPIOLOOP_FAIL},
    {{54, GPIO_6_6, "GPIO_6_6", 0}, {55, GPIO_6_7, "GPIO_6_7", 0}, BI_DIRECTIONAL_LOOP, GPIOLOOP_FAIL},
    {{56, GPIO_7_0, "GPIO_7_0", 0}, {57, GPIO_7_1, "GPIO_7_1", 0}, BI_DIRECTIONAL_LOOP, GPIOLOOP_FAIL},
    {{58, GPIO_7_2, "GPIO_7_2", 0}, {59, GPIO_7_3, "GPIO_7_3", 0}, BI_DIRECTIONAL_LOOP, GPIOLOOP_FAIL}
};

#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(BSP_CONFIG_BOARD_TELEMATIC))

void drv_ioctrl_set_function_to_gpio(void)
{
    /*GPIO4_5,设置复用为GPIO*/
    SET_IOS_GPIO4_5_CTRL1_1;

    /*GPIO7_4,设置复用为GPIO*/
    SET_IOS_GPIO7_4_CTRL1_1;

    /*GPIO7_3,设置复用为GPIO*/
    SET_IOS_GPIO7_3_CTRL1_1;

    /*GPIO7_2,设置复用为GPIO*/
    SET_IOS_GPIO7_2_CTRL1_1;

    /*GPIO0_1,设置复用为GPIO*/
    //SET_IOS_GPIO0_1_CTRL1_1;

    /*GPIO2_6,设置复用为GPIO*/
    SET_IOS_GPIO2_6_CTRL1_1;

    /*GPIO5_7,设置复用为GPIO*/
    SET_IOS_GPIO5_7_CTRL1_1;

    /*GPIO5_3,设置复用为GPIO*/
    SET_IOS_GPIO5_3_CTRL1_1;

    /*GPIO5_2,设置复用为GPIO*/
    SET_IOS_GPIO5_2_CTRL1_1;

    /*GPIO5_5,设置复用为GPIO*/
    SET_IOS_GPIO5_5_CTRL1_1;

    /*GPIO5_1,设置复用为GPIO*/
    SET_IOS_GPIO5_1_CTRL1_1;

    /*GPIO7_7,设置复用为GPIO*/
    CLR_IOS_GPIO7_7_CTRL2_1;
    SET_IOS_GPIO7_7_CTRL2_2;
    CLR_IOS_PMU_AUXDAC0_SSI_CTRL1_1;

    /*GPIO11_6,设置复用为GPIO*/
    CLR_IOS_PCIE1_CLKREQ_CTRL4_1;
    CLR_IOS_PCIE1_CLKREQ_CTRL4_2;
    CLR_IOS_PCIE1_CLKREQ_CTRL4_4;
    CLR_IOS_PCIE1_CLKREQ_CTRL4_3;
    SET_IOS_GPIO11_6_CTRL1_1;
    CLR_IOS_CH1_APT_PDM_CTRL1_1;
    CLR_IOS_ANTPA_SEL18_CTRL2_2;

    /*GPIO0_6,设置复用为GPIO*/
    //SET_IOS_GPIO0_6_CTRL1_1;

    /*GPIO11_5,设置复用为GPIO*/
    CLR_IOS_PCIE1_PERST_CTRL4_1;
    CLR_IOS_PCIE1_PERST_CTRL4_2;
    CLR_IOS_PCIE1_PERST_CTRL4_4;
    CLR_IOS_PCIE1_PERST_CTRL4_3;
    SET_IOS_GPIO11_5_CTRL1_1;
    CLR_IOS_RF1_TCVR_ON_CTRL1_1;
    CLR_IOS_ANTPA_SEL17_CTRL2_2;

    /*GPIO6_2,设置复用为GPIO*/
    CLR_IOS_MII_TX_ER_CTRL1_1;
    //OUTSET_IOS_PD_IOM_CTRL85;
    SET_IOS_GPIO6_2_CTRL1_1;
    CLR_IOS_ANTPA_SEL6_CTRL2_1;
    CLR_IOS_AFC_PDM_CTRL1_1;
    CLR_IOS_BBP_OFF_CH0_AFC_PDM_CTRL1_1;
    CLR_IOS_BBP_OFF_CH1_AFC_PDM_CTRL1_1;

    /*GPIO6_3,设置复用为GPIO*/
    CLR_IOS_MII_RMII_RX_ER_CTRL1_1;
    //INSET_IOS_PD_IOM_CTRL86;
    SET_IOS_GPIO6_3_CTRL1_1;
    CLR_IOS_ANTPA_SEL22_CTRL2_1;
    CLR_IOS_SPI0_CS0_CTRL3_2;

    /*GPIO3_7,设置复用为GPIO*/
    CLR_IOS_UART1_RTS_CTRL2_1;
    SET_IOS_GPIO3_7_CTRL1_1;
    CLR_IOS_UART1_RTS_CTRL2_2;
    CLR_IOS_MODEM_UART0_RTS_CTRL1_1;
    CLR_IOS_ANTPA_SEL27_CTRL3_3;

    /*GPIO4_0,设置复用为GPIO*/
    CLR_IOS_UART1_CTS_CTRL2_1;
    SET_IOS_GPIO4_0_CTRL1_1;
    CLR_IOS_UART1_CTS_CTRL2_2;
    CLR_IOS_MODEM_UART0_CTS_CTRL1_1;
    CLR_IOS_ANTPA_SEL30_CTRL3_3;
    CLR_IOS_NF_DATA8_CTRL1_1;

    /*GPIO6_4,设置复用为GPIO*/
    CLR_IOS_UART2_RTS_CTRL2_1;
    SET_IOS_GPIO6_4_CTRL1_1;
    CLR_IOS_UART2_RTS_CTRL2_2;
    CLR_IOS_ANTPA_SEL13_CTRL2_1;
    CLR_IOS_MIPI3_CLK_CTRL2_1;
    CLR_IOS_PCIE0_PERST_CTRL5_4;

    /*GPIO6_5,设置复用为GPIO*/
    CLR_IOS_UART2_CTS_CTRL2_1;
    SET_IOS_GPIO6_5_CTRL1_1;
    CLR_IOS_UART2_CTS_CTRL2_2;
    CLR_IOS_ANTPA_SEL14_CTRL2_1;
    CLR_IOS_MIPI3_DATA_CTRL2_1;
    CLR_IOS_PCIE0_CLKREQ_CTRL5_4;

    /*GPIO11_4,设置复用为GPIO*/
    CLR_IOS_PCIE0_CLKREQ_CTRL5_1;
    CLR_IOS_PCIE0_CLKREQ_CTRL5_2;
    CLR_IOS_PCIE0_CLKREQ_CTRL5_4;
    CLR_IOS_PCIE0_CLKREQ_CTRL5_5;
    CLR_IOS_PCIE0_CLKREQ_CTRL5_3;
    SET_IOS_GPIO11_4_CTRL1_1;
    CLR_IOS_RF1_SSI_CTRL1_1;
    CLR_IOS_ANTPA_SEL16_CTRL2_2;

    /*GPIO11_3,设置复用为GPIO*/
    CLR_IOS_PCIE0_PERST_CTRL5_1;
    CLR_IOS_PCIE0_PERST_CTRL5_2;
    CLR_IOS_PCIE0_PERST_CTRL5_4;
    CLR_IOS_PCIE0_PERST_CTRL5_5;
    CLR_IOS_PCIE0_PERST_CTRL5_3;
    SET_IOS_GPIO11_3_CTRL1_1;
    CLR_IOS_RF1_RESETN_CTRL1_1;
    CLR_IOS_ANTPA_SEL15_CTRL2_2;

}
#endif

static int drv_gpio_get_value(int index,unsigned char *direction, unsigned char *value)
{
    int ret = 0;
    if(NULL == direction || NULL == value)
    {
        gpio_print_err("direction or value is null");
        return -1;
    }

    /* 判断是否有gpio进行设置，如果没有则返回失败 */
    if (0 == g_gpio_ctrl_info_tb[index].gpio_pin)
    {
        gpio_print_err("gpio is NULL!!!\n");
        return -1;
    }

    /*如果该gpio没有被设置过，这里先记录为输入*/
    if(0 == gpio_request_flag[index])
    {
        gpio_direction_record[index] = 0;
    }
    
    /*方向直接从全局变量里获取出来*/
    *direction = gpio_direction_record[index] + '0';

    ret = gpio_get_value(g_gpio_ctrl_info_tb[index].gpio_num);
    if(0 == ret)
    {
        *value = '0';
    }
    else 
    {
        *value = '1';
    }
    return 0;
    

}
static int drv_gpio_set_value(int index, unsigned char direction, unsigned char value)
{
    int ret = 0;

    /* 判断是否有gpio进行设置，如果没有则返回失败 */
    if (0 == g_gpio_ctrl_info_tb[index].gpio_pin)
    {
        gpio_print_err("gpio is NULL!!!\n");
        return -1;
    }

    /*该gpio是否被初始化*/
    if(0 == gpio_request_flag[index])
    {
        ret = gpio_request(g_gpio_ctrl_info_tb[index].gpio_num, g_gpio_ctrl_info_tb[index].gpio_label);
        if(0 != ret)
        {
            gpio_print_err("request %s fail.",g_gpio_ctrl_info_tb[index].gpio_label);
    /*BC 工位测试GPIO,防止其他地方已request , 导致request 失败*/
#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(BSP_CONFIG_BOARD_TELEMATIC))
#else
            return -1;
#endif
        }
        gpio_request_flag[index] = 1;
    }

    /*设置为输出*/
    if(1 == direction) 
    {
        ret = gpio_direction_output(g_gpio_ctrl_info_tb[index].gpio_num, value );
        if(0 != ret)
        {
            gpio_print_err("set %s to output fail.",g_gpio_ctrl_info_tb[index].gpio_label);
            return -1;
        }
    }
    /*设置为输入*/
    else
    {
        ret = gpio_direction_input(g_gpio_ctrl_info_tb[index].gpio_num);
        if(0 != ret)
        {
            gpio_print_err("set %s to input fail.",g_gpio_ctrl_info_tb[index].gpio_label);
            return -1;
        }
    }
    gpio_direction_record[index] = direction;
    return 0;

}
/*****************************************************************************
 函 数 名  : drv_gpio_ioctrl_cmd
 功能描述  : 供AT^IOCTRL调用，设置或者获取提供给客户的gpio方向和电平
 输入参数  : cmd:0--SET,1--GET;
             *gpio:需要设置的gpio，如"0110111",当cmd为get时此参数可忽略；
             *direction:需要设置或者获取到的gpio方向，1为输出，0为输入；
             *value: 需要设置或获取到的gpio电平，1为高电平，0为低电平；
             
 返 回 值  : 0--成功；-1--失败
 说    明  : 调用者需要保证传入的参数有效或者接受的地址范围足够大。
*****************************************************************************/
int drv_gpio_ioctrl_cmd(GPIO_CMD_E cmd, unsigned char *gpio, unsigned char *direction, unsigned char *value)
{
    int  i  = 0, j = 0;
    unsigned char direct = 0;
    unsigned char val    = 0;
    
    if(NULL == direction || NULL == value)
    {
        gpio_print_err("direction or vlaue is null.");
        return -1;
    }

    if(GPIO_CMD_SET == cmd && NULL == gpio)
    {
        gpio_print_err("set cmd but gpio is null.");
        return -1;
    }

    switch(cmd)
    {
        case GPIO_CMD_SET:
        {
#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(BSP_CONFIG_BOARD_TELEMATIC))
            drv_ioctrl_set_function_to_gpio();
#endif
            for(i = 0; i < GPIO_CONTROL_NUMBER; i++)
            {
                /*注意at传入参数是倒序，如0111111，0存储在数组第一个,对应的是第7个gpio*/
                j = GPIO_CONTROL_NUMBER - i - 1;
                
                /*不需要设置*/
                if('0' == gpio[j])
                {
                    continue;
                }
                 /*设置方向和电平*/
                direct = direction[j] - '0';
                val    = value[j] - '0';
                if(0 != drv_gpio_set_value(i, direct, val))
                {
                    return -1;
                }
            }
            break;
         }
        case GPIO_CMD_GET:
        {
            for(i = 0; i < GPIO_CONTROL_NUMBER; i++)
            {
                j = GPIO_CONTROL_NUMBER - i - 1;
                if(0 != drv_gpio_get_value(i, &direction[j], &value[j]))
                {
                    return -1;
                }
            }
            break;
        }
        default:
            gpio_print_err("cmd is not support,pls check.");
            return -1;
    }
    
    return 0;
}


static int drv_gpio_loop_test(unsigned int pin_out, unsigned int pin_in, unsigned int gpio_value)
{
    unsigned int  ret_value = 0;   
    int ret = 0;

    if ( (1 != gpio_value) && (0 != gpio_value) )
    {
        gpio_print_err("drv_gpio_loop_test gpio_value err.\r\n");
        return -1;
    }

    /*设置pin_out为输出*/
    ret = gpio_direction_output(pin_out, gpio_value);
    if (0 != ret)
    {
        gpio_print_err("set GPIO%d to output fail,ret = %d.", pin_out, ret);
        return -1;
    }

    /*设置pin_in为输入*/
    ret = gpio_direction_input(pin_in);
    if (0 != ret)
    {
        gpio_print_err("set GPIO%d to input fail,ret = %d.", pin_in,ret);
        return -1;
    }

    /* 获取pin_in管脚电平值 */
    ret_value = gpio_get_value(pin_in);
    if (ret_value != gpio_value)
    {        
        gpio_print_err("drv_gpio_loop_test loop check fail,ret_value= %d!\r\n",ret_value);
        return -1;
    }

    return 0;
}


int drv_gpioloop_test_process(GPIOLOOP_RST_STRU *pt_ret)
{
    unsigned char i = 0;
    int ret_value = -1;

    if (NULL == pt_ret)
    {
        gpio_print_err("drv_gpioloop_test_process p_ret is null!\r\n");
        return -1;
    }

    /*默认测试结果为fail*/
    memset(pt_ret, 0, sizeof(GPIOLOOP_RST_STRU));
    pt_ret->test_result = GPIOLOOP_FAIL;

    /*遍历被测gpio表进行gpio loop test*/
    for ( i = 0 ; i < sizeof(g_gpioloop_test_tbl) / sizeof(g_gpioloop_test_tbl[0]); i ++ )
    {
        /*如果GPIO未申请，先request*/
        if (0 == g_gpioloop_test_tbl[i].test_a.request_state)
        {
            ret_value = gpio_request(g_gpioloop_test_tbl[i].test_a.gpio_num, g_gpioloop_test_tbl[i].test_a.gpio_req_label);
            if (0 != ret_value)
            {
                gpio_print_err("request %s fail,ret_value = %d.", g_gpioloop_test_tbl[i].test_a.gpio_req_label, ret_value);
                continue;
            }
            /*设置申请标志*/
            g_gpioloop_test_tbl[i].test_a.request_state = 1;
        }

        if (0 == g_gpioloop_test_tbl[i].test_b.request_state)
        {
            ret_value = gpio_request(g_gpioloop_test_tbl[i].test_b.gpio_num, g_gpioloop_test_tbl[i].test_b.gpio_req_label);
            if (0 != ret_value)
            {
                gpio_print_err("request %s fail,ret_value = %d.", g_gpioloop_test_tbl[i].test_b.gpio_req_label, ret_value);
                continue;
            }
            /*设置申请标志*/
            g_gpioloop_test_tbl[i].test_b.request_state = 1;
        }

        /*第一个管脚输出低电平，第二个管脚输入*/
        ret_value = drv_gpio_loop_test(g_gpioloop_test_tbl[i].test_a.gpio_num, g_gpioloop_test_tbl[i].test_b.gpio_num, 0);
        if (0 != ret_value)
        {            
            gpio_print_err("drv_gpioloop_test_process GPIO%d TO GPIO %d 0 test fail,ret_value = %d!\r\n", \
                g_gpioloop_test_tbl[i].test_a.gpio_num, g_gpioloop_test_tbl[i].test_b.gpio_num, ret_value);
            continue;
        }

        /*第一个管脚输出高电平，第二个管脚输入*/
        ret_value = drv_gpio_loop_test(g_gpioloop_test_tbl[i].test_a.gpio_num, g_gpioloop_test_tbl[i].test_b.gpio_num, 1);
        if (0 != ret_value)
        {            
            gpio_print_err("drv_gpioloop_test_process GPIO%d TO GPIO %d 1 test fail,ret_value = %d!\r\n", \
                g_gpioloop_test_tbl[i].test_a.gpio_num, g_gpioloop_test_tbl[i].test_b.gpio_num, ret_value);
            continue;
        }

        /*如果是回环测试类型，则反过来再测试一遍*/
        if (BI_DIRECTIONAL_LOOP == g_gpioloop_test_tbl[i].test_type)
        {
            /*第二个管脚输出低电平，第一个管脚输入*/
            ret_value = drv_gpio_loop_test(g_gpioloop_test_tbl[i].test_b.gpio_num, g_gpioloop_test_tbl[i].test_a.gpio_num, 0);
            if (0 != ret_value)
            {            
                gpio_print_err("drv_gpioloop_test_process GPIO%d TO GPIO %d 0 test fail,ret_value = %d!\r\n", \
                    g_gpioloop_test_tbl[i].test_b.gpio_num, g_gpioloop_test_tbl[i].test_a.gpio_num, ret_value);
                continue;
            }

            /*第二个管脚输出高电平，第一个管脚输入*/
            ret_value = drv_gpio_loop_test(g_gpioloop_test_tbl[i].test_b.gpio_num, g_gpioloop_test_tbl[i].test_a.gpio_num, 1);
            if (0 != ret_value)
            {            
                gpio_print_err("drv_gpioloop_test_process GPIO%d TO GPIO %d 1 test fail,ret_value = %d!\r\n", \
                    g_gpioloop_test_tbl[i].test_b.gpio_num, g_gpioloop_test_tbl[i].test_a.gpio_num, ret_value);
                continue;
            }
        }       
         g_gpioloop_test_tbl[i].result = GPIOLOOP_SUCCESS;
    } 

    for ( i = 0 ; i < sizeof(g_gpioloop_test_tbl) / sizeof(g_gpioloop_test_tbl[0]); i ++ )
    {
        if (GPIOLOOP_SUCCESS != g_gpioloop_test_tbl[i].result)
        {
            pt_ret->test_result = GPIOLOOP_FAIL;
            pt_ret->fail_list[(pt_ret->fail_num)++] = g_gpioloop_test_tbl[i].test_a.gpio_num;
            pt_ret->fail_list[(pt_ret->fail_num)++] = g_gpioloop_test_tbl[i].test_b.gpio_num;
        }
    }

    return 0;
}


int drv_get_support_gpioloop_info(unsigned int *pt_support, unsigned char *pt_num)
{
    unsigned char loop = 0;
    unsigned char support_num = 0;
    if ( (NULL == pt_support) || (NULL == pt_num) )
    {
        gpio_print_err("pt_support OR pt_num is null!\r\n");
        return -1;
    }

    for ( loop = 0 ; loop < sizeof(g_gpioloop_test_tbl) / sizeof(g_gpioloop_test_tbl[0]); loop ++ )
    {
        pt_support[support_num++] = g_gpioloop_test_tbl[loop].test_a.gpio_num;
        pt_support[support_num++] = g_gpioloop_test_tbl[loop].test_b.gpio_num;
    }
    *pt_num = support_num;
    return 0;
}


void drv_gpioloop_set_function_to_gpio(void)
{
    /*GPIO2_6,UART0_RTS/RGMII_MDIO,设置复用为GPIO*/
    /* CLR_IOS_HS_UART1_CTRL1_1; */
    //OUTSET_IOS_PD_IOM_CTRL34;
    /* SET_IOS_GPIO2_6_CTRL1_1; */
    /* CLR_IOS_RGMII_CTRL1_1; */
    /* CLR_IOS_UART1_CTRL3_2; */

    /*GPIO2_7,UART0_CTS/RGMII_MDC,设置复用为GPIO*/
    /* CLR_IOS_HS_UART1_CTRL1_1; */
    //INSET_IOS_PD_IOM_CTRL35;
    /* SET_IOS_GPIO2_7_CTRL1_1; */
    /* CLR_IOS_RGMII_CTRL1_1; */
    /* CLR_IOS_UART1_CTRL3_2; */

    /*GPIO2_8,hs_uart_txd管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_HS_UART2_CTRL1_1; */
    //OUTSET_IOS_PD_IOM_CTRL36;
    /* SET_IOS_GPIO2_8_CTRL1_1; */
    /* CLR_IOS_RGMII_CTRL1_1; */
    /* CLR_IOS_LCD_CTRL3_2; */

    /*GPIO2_9,hs_uart_rxd管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_HS_UART2_CTRL1_1; */
    //INSET_IOS_PD_IOM_CTRL37;
    /* SET_IOS_GPIO2_9_CTRL1_1; */
    /* CLR_IOS_RGMII_CTRL1_1; */
    /* CLR_IOS_LCD_CTRL3_2; */

    /*GPIO2_12,pcm_clk管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_PCM_CTRL2_2; */
    /* CLR_IOS_PCM_CTRL2_1; */
    /* SET_IOS_GPIO2_12_CTRL1_1; */
    /* CLR_IOS_MMC0_CTRL1_1; */

    /*GPIO2_13,pcm_sync管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_PCM_CTRL2_2; */
    /* CLR_IOS_PCM_CTRL2_1; */
    /* SET_IOS_GPIO2_13_CTRL1_1; */
    /* CLR_IOS_MMC0_CTRL1_1; */

    /*GPIO2_14,pcm_do管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_PCM_CTRL2_2; */
    /* CLR_IOS_PCM_CTRL2_1; */
    //OUTSET_IOS_PD_IOM_CTRL42;
    /* SET_IOS_GPIO2_14_CTRL1_1; */
    /* CLR_IOS_MMC0_CTRL1_1; */

    /*GPIO2_15,pcm_di管脚复用配置,设置复用为GPIO*/
    /* CLR_IOS_PCM_CTRL2_2; */
    /* CLR_IOS_PCM_CTRL2_1; */
    //INSET_IOS_PD_IOM_CTRL43;
    /* SET_IOS_GPIO2_15_CTRL1_1; */
    /* CLR_IOS_MMC0_CTRL1_1; */
}


void drv_gpioloop_set_gpio_to_default(void)
{
    /*hs_uart_rts_n管脚复用配置*/
    /*
     * SET_IOS_HS_UART1_CTRL1_1;
     * OUTSET_IOS_PD_IOM_CTRL34;
     * CLR_IOS_GPIO2_6_CTRL1_1;
     * CLR_IOS_RGMII_CTRL1_1;
     * CLR_IOS_UART1_CTRL3_2;
     */

    /*hs_uart_cts_n管脚复用配置*/
    /*
     * SET_IOS_HS_UART1_CTRL1_1;
     * INSET_IOS_PD_IOM_CTRL35;
     * CLR_IOS_GPIO2_7_CTRL1_1;
     * CLR_IOS_RGMII_CTRL1_1;
     * CLR_IOS_UART1_CTRL3_2;
     */

    /*hs_uart_txd管脚复用配置*/
    /*
     * SET_IOS_HS_UART2_CTRL1_1;
     * OUTSET_IOS_PD_IOM_CTRL36;
     * CLR_IOS_GPIO2_8_CTRL1_1;
     * CLR_IOS_RGMII_CTRL1_1;
     * CLR_IOS_LCD_CTRL3_2;
     */

    /*hs_uart_rxd管脚复用配置*/
    /*
     * SET_IOS_HS_UART2_CTRL1_1;
     * INSET_IOS_PD_IOM_CTRL37;
     * CLR_IOS_GPIO2_9_CTRL1_1;
     * CLR_IOS_RGMII_CTRL1_1;
     * CLR_IOS_LCD_CTRL3_2;
     */

    /*pcm_clk管脚复用配置*/
    /*
     * CLR_IOS_PCM_CTRL2_2;
     * SET_IOS_PCM_CTRL2_1;
     * CLR_IOS_GPIO2_12_CTRL1_1;
     * CLR_IOS_MMC0_CTRL1_1;
     */

    /*pcm_sync管脚复用配置*/
    /*
     * CLR_IOS_PCM_CTRL2_2;
     * SET_IOS_PCM_CTRL2_1;
     * CLR_IOS_GPIO2_13_CTRL1_1;
     * CLR_IOS_MMC0_CTRL1_1;
     */

    /*pcm_do管脚复用配置*/
    /*
     * CLR_IOS_PCM_CTRL2_2;
     * SET_IOS_PCM_CTRL2_1;
     * OUTSET_IOS_PD_IOM_CTRL42;
     * CLR_IOS_GPIO2_14_CTRL1_1;
     * CLR_IOS_MMC0_CTRL1_1;
     */

    /*pcm_di管脚复用配置*/
    /*
     * CLR_IOS_PCM_CTRL2_2;
     * SET_IOS_PCM_CTRL2_1;
     * INSET_IOS_PD_IOM_CTRL43;
     * CLR_IOS_GPIO2_15_CTRL1_1;
     * CLR_IOS_MMC0_CTRL1_1;
     */

    /*pin7,GPIO_0_02,设置复用为GPIO*/
    /*
     * SET_IOS_GPIO0_2_CTRL1_1;
     */

    /*pin44,GPIO_0_03,设置复用为GPIO*/
    /*
     * SET_IOS_GPIO0_3_CTRL1_1;
     */
}

#ifdef __cplusplus
}
#endif

