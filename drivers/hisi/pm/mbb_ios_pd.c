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

#include "ios_ao_drv_macro.h"
#include "ios_pd_drv_macro.h"
#include "hi_gpio.h"
#include "mbb_bsp_reg_def.h"
#include <linux/gpio.h>
#include "pm.h"

/* 申请SHARE-MEMORY内存,用来存放fastboot管脚配置的寄存器信息 */
#define SHARED_IOS_ADDR            ((unsigned long)SHM_BASE_ADDR + SHM_OFFSET_IOS)
#define IOS_POOL_SIZE              ((SHM_SIZE_IOS - sizeof(IOS_POOL_CTRL)) / sizeof(IOS_REG_DATA) * sizeof(IOS_REG_DATA))

#if (FEATURE_ON == MBB_IOCFG_HERMES)
#define IOS_NUMBER                 (14)   /* 客户需求的低功耗下配置的管脚数 */
#define GPIO_NUMBER                (2)    /* 客户需求的低功耗下GPIO的个数 */
#else
#define IOS_NUMBER                 (1)   /* 客户需求的低功耗下配置的管脚数 */
#define GPIO_NUMBER                (1)    /* 客户需求的低功耗下GPIO的个数 */
#endif

/* 参考文件ios_pd_drv_macro.h */
#define IOS_PD_BASE_PHYADDR           HI_IOS_PD_REGBASE_ADDR                          /* 外设区IO基地址 */
#define IOS_PD_MF_PHYADDR             (IOS_PD_BASE_PHYADDR + IOS_PD_MF_OFFSET_ADDR)     /* 主功能复用寄存器首地址 */
#define IOS_PD_IOM_PHYADDR            (IOS_PD_BASE_PHYADDR + IOS_PD_IOM_OFFSET_ADDR)    /* 管脚功能控制寄存器首地址 */
#define IOS_PD_MF_CTRL3_PHYADDR       (IOS_PD_MF_PHYADDR + 0x00C)                       /* 主功能复用控制器3 */
#define IOS_PD_MF_CTRL7_PHYADDR       (IOS_PD_MF_PHYADDR + 0x01C)                       /* 主功能复用控制器7 */
#define IOS_PD_IOM_CTRL50_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0C8)                      /* GPIO2_07管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL51_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0CC)                      /* GPIO3_00管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL54_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0D8)                      /* GPIO3_03管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL55_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0DC)                      /* GPIO3_04管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL56_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0E0)                      /* GPIO3_05管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL57_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0E4)                      /* GPIO3_06管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL58_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0E8)                      /* GPIO3_07管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL59_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0EC)                      /* GPIO4_00管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL60_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0F0)                      /* GPIO4_01管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL61_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x0F4)                      /* GPIO4_02管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL69_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x114)                      /* GPIO5_02管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL89_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x164)                      /* GPIO6_06管脚功能控制寄存器 */
#define IOS_PD_IOM_CTRL90_PHYADDR     (IOS_PD_IOM_PHYADDR + 0x168)                      /* GPIO6_07管脚功能控制寄存器 */

#define IOS_OK      (0)    /* 增加成功 */
#define IOS_ERROR   (-1)   /* 更新失败 */
#define IOS_TRUE    (1)    /* 更新成功 */

#define IOS_MULTIPLEX_SIZE     (int)(sizeof(g_lp_ios_multiplex_tb) / sizeof(g_lp_ios_multiplex_tb[0]))        /* 复用关系寄存器表长 */
#define IOS_UPDOWN_SIZE        (int)(sizeof(g_lp_ios_updown_tb) / sizeof(g_lp_ios_updown_tb[0]))              /* 上下拉寄存器表长 */

typedef struct
{
    unsigned int           addr;         /* register address */
    unsigned int           value;        /* register value */
} IOS_REG_DATA;

typedef struct
{
    IOS_REG_DATA           *start;      /* start address of ios data array */
    IOS_REG_DATA           *cur;        /* current postion of ios data array */
    IOS_REG_DATA           *end;        /* last address of ios data array */
} IOS_POOL_CTRL;

typedef struct
{
    unsigned int  gpio;          /* gpio */
    const char   *gpio_label;    /* gpio label */
} LP_IOS_INFO;

typedef struct
{
    unsigned int gpio;            /* gpio */
    const char  *gpio_label;      /* gpio label */
    int          dir;             /* gpio direction */
    int          val;             /* gpio value */
} LP_GPIO_INFO;

typedef struct
{
    unsigned int           phyaddr;      /* register physical address */
    void                   *virtaddr;    /* register virtual address */
    unsigned int           value;        /* register value */

} IO_SHARE_DATA;

#if (FEATURE_ON == MBB_IOCFG_HERMES)
/* 哈曼低功耗需要配置的IO表格 */
static LP_IOS_INFO g_lp_ios_info_tb[IOS_NUMBER] =
{
    {GPIO_2_7, "GPIO_2_7"},
    {GPIO_3_0, "GPIO_3_0"},
    {GPIO_3_3, "GPIO_3_3"},
    {GPIO_3_4, "GPIO_3_4"},
    {GPIO_3_5, "GPIO_3_5"},
    {GPIO_3_6, "GPIO_3_6"},
    {GPIO_3_7, "GPIO_3_7"},
    {GPIO_4_0, "GPIO_4_0"},
    {GPIO_4_1, "GPIO_4_1"},
    {GPIO_4_2, "GPIO_4_2"},
    {GPIO_5_2, "GPIO_5_2"},
    {GPIO_5_5, "GPIO_5_5"},
    {GPIO_6_6, "GPIO_6_6"},
    {GPIO_6_7, "GPIO_6_7"},
};
/* 哈曼低功耗需要配置的gpio */
static LP_GPIO_INFO g_lp_gpio_info_tb[GPIO_NUMBER] =
{
    {GPIO_5_2, "GPIO_5_2", 0, 0},
    {GPIO_5_5, "GPIO_5_5", 0, 0},
};

static IO_SHARE_DATA g_lp_ios_multiplex_tb[] =
{
    {IOS_PD_MF_CTRL3_PHYADDR, NULL, 0},  /* 主功能复用寄存器3，GPIO_2_7，GPIO_3_0，GPIO_3_3~GPIO_3_7，GPIO_4_0~GPIO_4_2共10个管脚受此寄存器控制 */
    {IOS_PD_MF_CTRL7_PHYADDR, NULL, 0},  /* 主功能复用寄存器7，GPIO_6_6和GPIO_6_7受此寄存器控制 */
};

static IO_SHARE_DATA g_lp_ios_updown_tb[] =
{
    {IOS_PD_IOM_CTRL50_PHYADDR, NULL, 0},       /* GPIO_2_7管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL51_PHYADDR, NULL, 0},       /* GPIO_3_0管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL54_PHYADDR, NULL, 0},       /* GPIO_3_3管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL55_PHYADDR, NULL, 0},       /* GPIO_3_4管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL56_PHYADDR, NULL, 0},       /* GPIO_3_5管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL57_PHYADDR, NULL, 0},       /* GPIO_3_6管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL58_PHYADDR, NULL, 0},       /* GPIO_3_7管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL59_PHYADDR, NULL, 0},       /* GPIO_4_0管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL60_PHYADDR, NULL, 0},       /* GPIO_4_1管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL61_PHYADDR, NULL, 0},       /* GPIO_4_2管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL69_PHYADDR, NULL, 0},       /* GPIO5_2管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL89_PHYADDR, NULL, 0},       /* GPIO6_6管脚功能控制寄存器，控制管脚上下拉 */
    {IOS_PD_IOM_CTRL90_PHYADDR, NULL, 0},       /* GPIO6_7管脚功能控制寄存器，控制管脚上下拉 */
};
#else
/* 其他产品根据具体情况而定,暂定为空 */
static LP_IOS_INFO g_lp_ios_info_tb[IOS_NUMBER] = {{0, NULL}};

static LP_GPIO_INFO g_lp_gpio_info_tb[GPIO_NUMBER] = {{0, NULL, 0, 0}};

static IO_SHARE_DATA g_lp_ios_multiplex_tb[] = {{0, NULL, 0}};

static IO_SHARE_DATA g_lp_ios_updown_tb[] = {{0, NULL, 0}};
#endif
/* 共享内存首地址 */
static IOS_POOL_CTRL *g_p_ios_ctrl = NULL;


static void gpio_lp_suspend(void)
{
    int i = 0;

    /* 获取GPIO的电平和方向 */
    for (i = 0; i < GPIO_NUMBER; i++)
    {
        if (NULL == g_lp_gpio_info_tb[i].gpio_label)
        {
            break;
        }

        g_lp_gpio_info_tb[i].dir = gpio_direction_get(g_lp_gpio_info_tb[i].gpio);
        g_lp_gpio_info_tb[i].val = gpio_get_value(g_lp_gpio_info_tb[i].gpio);
    }
}


static void gpio_lp_resume(void)
{
    int i = 0;
    int ret = 0;

    for (i = 0; i < GPIO_NUMBER; i++)
    {
        if (NULL == g_lp_gpio_info_tb[i].gpio_label)
        {
            break;
        }

        ret = gpio_request(g_lp_gpio_info_tb[i].gpio, g_lp_gpio_info_tb[i].gpio_label);
        if (0 != ret)
        {
            pm_printk(BSP_LOG_LEVEL_INFO, "gpio-%u requested failed!!!\n", g_lp_gpio_info_tb[i].gpio);
            continue;
        }

        /* 恢复gpio的方向和电平 */
        if (0 != g_lp_gpio_info_tb[i].dir)
        {
            ret = gpio_direction_output(g_lp_gpio_info_tb[i].gpio, g_lp_gpio_info_tb[i].val);
            if (0 != ret)
            {
                pm_printk(BSP_LOG_LEVEL_INFO, "set gpio-%u output failed!!!\n", g_lp_gpio_info_tb[i].gpio);
            }
        }
        else
        {
            ret = gpio_direction_input(g_lp_gpio_info_tb[i].gpio);
            if (0 != ret)
            {
                pm_printk(BSP_LOG_LEVEL_INFO, "set gpio-%u input failed!!!\n", g_lp_gpio_info_tb[i].gpio);
            }
        }

        gpio_free(g_lp_gpio_info_tb[i].gpio);
    }
}


static void io_share_save(void)
{
    int i = 0;

    for (i = 0; i < IOS_MULTIPLEX_SIZE; i++)
    {
        if (0 == g_lp_ios_multiplex_tb[i].phyaddr)
        {
            break;
        }

        g_lp_ios_multiplex_tb[i].value = readl(g_lp_ios_multiplex_tb[i].virtaddr);
    }

    for (i = 0; i < IOS_UPDOWN_SIZE; i++)
    {
        if (0 == g_lp_ios_updown_tb[i].phyaddr)
        {
            break;
        }

        g_lp_ios_updown_tb[i].value = readl(g_lp_ios_updown_tb[i].virtaddr);
    }
}


static void telematic_ios_config_resume(void)
{
    int i = 0;

    for (i = 0; i < IOS_MULTIPLEX_SIZE; i++)
    {
        if (0 == g_lp_ios_multiplex_tb[i].phyaddr)
        {
            break;
        }

        writel(g_lp_ios_multiplex_tb[i].value, g_lp_ios_multiplex_tb[i].virtaddr);
    }

    for (i = 0; i < IOS_UPDOWN_SIZE; i++)
    {
        if (0 == g_lp_ios_updown_tb[i].phyaddr)
        {
            break;
        }

        writel(g_lp_ios_updown_tb[i].value, g_lp_ios_updown_tb[i].virtaddr);
    }

}


static void ios_lp_config(void)
{
    int i = 0;
    int ret = 0;

    /* gpio设置为输入 */
    for (i = 0; i < IOS_NUMBER; i++)
    {
        if (NULL == g_lp_ios_info_tb[i].gpio_label)
        {
            break;
        }

        ret  = gpio_request(g_lp_ios_info_tb[i].gpio, g_lp_ios_info_tb[i].gpio_label);
        if (0 != ret)
        {
            pm_printk(BSP_LOG_LEVEL_INFO, "gpio-%u requested failed!!!\n", g_lp_ios_info_tb[i].gpio);
            continue;
        }

        ret = gpio_direction_input(g_lp_ios_info_tb[i].gpio);
        if (0 != ret)
        {
            pm_printk(BSP_LOG_LEVEL_INFO, "set gpio-%u input failed!!!\n", g_lp_ios_info_tb[i].gpio);
        }

        gpio_free(g_lp_ios_info_tb[i].gpio);
    }

#if (FEATURE_ON == MBB_IOCFG_HERMES)
    /* gpio2[7]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO2_7_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL50;

    /* gpio3[0]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_0_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL51;

    /* gpio3[3]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_3_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL54;

    /* gpio3[4]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_4_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL55;

    /* gpio3[5]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_5_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL56;

    /* gpio3[6]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_6_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL57;

    /* gpio3[7]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO3_7_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL58;

    /* gpio4[0]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO4_0_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL59;

    /* gpio4[1]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO4_1_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL60;

    /* gpio4[2]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO4_2_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL61;

    /* gpio6[6]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO6_6_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL89;

    /* gpio6[7]管脚低功耗复用和管脚上下拉配置 */
    SET_IOS_GPIO6_7_CTRL1_1;
    PDSET_IOS_PD_IOM_CTRL90;

    /* gpio5[2]管脚低功耗管脚上下拉配置 */
    PDSET_IOS_PD_IOM_CTRL69;
#endif
}


static void phy_virt_ctrl_convert(IOS_POOL_CTRL *pool_ctrl, IOS_POOL_CTRL *p_ctrl)
{
    if ((NULL == pool_ctrl) || (NULL == p_ctrl))
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "pool_ctrl is NULL!!!\n");
        return;
    }

    pool_ctrl->start  = (IOS_REG_DATA *)ioremap_wc((phys_addr_t)p_ctrl->start, IOS_POOL_SIZE);
    if (NULL == pool_ctrl->start)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "get start adrr failed!!!\n");
        return;
    }

    pool_ctrl->cur    = (pool_ctrl->start + (p_ctrl->cur - p_ctrl->start));
    pool_ctrl->end    = (pool_ctrl->start + (p_ctrl->end - p_ctrl->start));
}


static void virt_ctrl_unmap(IOS_POOL_CTRL *pool_ctrl)
{
    if (NULL == pool_ctrl)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "pool_ctrl is NULL!!!\n");
        return;
    }

    iounmap((void *)(pool_ctrl->start));
}


static int telematic_update_list(IOS_POOL_CTRL *p_ctrl, unsigned int phyaddr, void *virtaddr)
{
    unsigned int value = 0;

    IOS_POOL_CTRL pool_ctrl = {NULL, NULL, NULL};
    IOS_REG_DATA *p_data = NULL;

    if (NULL == p_ctrl)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "p_ctrl is NULL!!!\n");
        return IOS_ERROR;
    }

    if (NULL == virtaddr)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "virt addr is NULL!!!\n");
        return IOS_ERROR;
    }

    phy_virt_ctrl_convert(&pool_ctrl, p_ctrl);

    value = readl(virtaddr);
    p_data = pool_ctrl.start;
    if (NULL == p_data)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "get start addr failed!!!\n");
        return IOS_ERROR;
    }

    while (p_data != pool_ctrl.cur)
    {
        if (phyaddr == p_data->addr)
        {
            p_data->value = value;
            virt_ctrl_unmap(&pool_ctrl);
            return IOS_TRUE;
        }

        ++p_data;
    }

    if (pool_ctrl.cur == pool_ctrl.end)
    {
        pm_printk(BSP_LOG_LEVEL_INFO, "the ios list is full.\n");
        virt_ctrl_unmap(&pool_ctrl);
        return IOS_ERROR;
    }

    pool_ctrl.cur->addr = phyaddr;
    pool_ctrl.cur->value = value;
    (p_ctrl->cur)++;

    virt_ctrl_unmap(&pool_ctrl);

    return IOS_OK;

}


static void telematic_update_ios_list(void)
{
    int i = 0;
    int ret = IOS_ERROR;
    IOS_POOL_CTRL *p_ctrl = g_p_ios_ctrl;

    for (i = 0; i < IOS_MULTIPLEX_SIZE; i++)
    {
        if (0 == g_lp_ios_multiplex_tb[i].phyaddr)
        {
            break;
        }

        ret = telematic_update_list(p_ctrl, g_lp_ios_multiplex_tb[i].phyaddr, g_lp_ios_multiplex_tb[i].virtaddr);
        if (IOS_ERROR == ret)
        {
            pm_printk(BSP_LOG_LEVEL_INFO, "change ios list fail: g_lp_ios_multiplex_tb[%d].\n", i);
        }
    }

    for (i = 0; i < IOS_UPDOWN_SIZE; i++)
    {
        if (0 == g_lp_ios_updown_tb[i].phyaddr)
        {
            break;
        }

        ret = telematic_update_list(p_ctrl, g_lp_ios_updown_tb[i].phyaddr, g_lp_ios_updown_tb[i].virtaddr);
        if (IOS_ERROR == ret)
        {
            pm_printk(BSP_LOG_LEVEL_INFO, "change ios list fail: g_lp_ios_updown_tb[%d].\n", i);
        }
    }
}


void ios_suspend(void)
{
    gpio_lp_suspend();              /* 睡眠之前需要保存gpio的方向和电平 */
    io_share_save();                /* 睡眠之前保存复用和管脚上下拉配置 */
    ios_lp_config();                /* 睡眠之前将所有管脚配置为gpio输入下拉 */
    telematic_update_ios_list();    /* 更新共享内存中的管脚配置寄存器，m核唤醒需要通过共享内存来恢复功能 */
}


void ios_resume(void)
{
    telematic_ios_config_resume();    /* 唤醒后恢复客户指定的管脚的功能 */
    /* 由于IO管脚复用关系寄存器链表只在M核恢复中用到，此处不再更新链表内容 */
    gpio_lp_resume();    /* 唤醒后恢复客户指定管脚中gpio的方向和电平 */
}


void ios_init(void)
{
    g_p_ios_ctrl = (IOS_POOL_CTRL *)SHARED_IOS_ADDR;

#if (FEATURE_ON == MBB_IOCFG_HERMES)
    g_lp_ios_multiplex_tb[0].virtaddr = IOS_PD_MF_CTRL3;    /* 主功能复用寄存器3，GPIO_2_7，GPIO_3_0，GPIO_3_3~GPIO_3_7，GPIO_4_0~GPIO_4_2共10个管脚受此寄存器控制 */
    g_lp_ios_multiplex_tb[1].virtaddr = IOS_PD_MF_CTRL7;    /* 主功能复用寄存器7，GPIO_6_6和GPIO_6_7受此寄存器控制 */

    g_lp_ios_updown_tb[0].virtaddr = IOS_PD_IOM_CTRL50;     /* GPIO2_7管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[1].virtaddr = IOS_PD_IOM_CTRL51;     /* GPIO_3_0管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[2].virtaddr = IOS_PD_IOM_CTRL54;     /* GPIO_3_3管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[3].virtaddr = IOS_PD_IOM_CTRL55;     /* GPIO_3_4管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[4].virtaddr = IOS_PD_IOM_CTRL56;     /* GPIO_3_5管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[5].virtaddr = IOS_PD_IOM_CTRL57;     /* GPIO_3_6管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[6].virtaddr = IOS_PD_IOM_CTRL58;     /* GPIO_3_7管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[7].virtaddr = IOS_PD_IOM_CTRL59;     /* GPIO_4_0管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[8].virtaddr = IOS_PD_IOM_CTRL60;     /* GPIO_4_1管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[9].virtaddr = IOS_PD_IOM_CTRL61;     /* GPIO_4_2管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[10].virtaddr = IOS_PD_IOM_CTRL69;    /* GPIO_5_2管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[11].virtaddr = IOS_PD_IOM_CTRL89;    /* GPIO_6_6管脚功能控制寄存器，控制管脚上下拉 */
    g_lp_ios_updown_tb[12].virtaddr = IOS_PD_IOM_CTRL90;    /* GPIO_6_7管脚功能控制寄存器，控制管脚上下拉 */
#endif
}

