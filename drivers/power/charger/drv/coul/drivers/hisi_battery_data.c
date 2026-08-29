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
 

#include "hisi_battery_data.h"
#include "auto-generate/hisi_battery_data_array.c"

#define BATTERY_DATA_ARRY_SIZE (g_batt_data_array_size)

/*----------------------------------------------*
 * 全局变量说明                                 *
 *----------------------------------------------*/
 /*存储产品对应电池数据*/
static struct hisi_smartstar_coul_battery_data **battery_data_array = NULL;
/*支持电池格数*/
static unsigned int g_batt_data_array_size = 0;


static void matching_coul_batt_data_by_hw_id(void)
{
    unsigned int i = 0;
    unsigned int hw_id = 0xFFFFFFFF;

    /*获取硬件版本号*/
    hw_id = mbb_version_get_board_type();
    printk(KERN_ERR "[smartstar] Get hardware version ID is 0x%x.\n",hw_id);

    /*通过硬件版本号寻找所支持的电池建模数据*/
    for( i = (ARRAY_SIZE(coul_battery_data_diff_array) - 1); i > 0; i-- )
    {
        if ( hw_id == coul_battery_data_diff_array[i]->hardware_version_id )
        {
            break;
        }
    }
    printk(KERN_ERR "[smartstar] Get current product batt_diff_array position is %u.\n",i);

    /*存储电池建模数据数组大小为支持的电池个数（包含default参数）*/
    g_batt_data_array_size = coul_battery_data_diff_array[i]->support_batt_num;
    /*将匹配到的电池建模数据传递给全局变量*/
    battery_data_array = coul_battery_data_diff_array[i]->coul_batt_data_array;
    return;
}

struct hisi_smartstar_coul_battery_data *get_battery_data(unsigned int id_voltage)
{
    int i;

    /*通过硬件版本号,匹配对应产品所支持的电池建模数据*/
    matching_coul_batt_data_by_hw_id();

    for (i=(BATTERY_DATA_ARRY_SIZE - 1); i>0; i--){
        if ((id_voltage >= battery_data_array[i]->id_voltage_min)
            && (id_voltage <= battery_data_array[i]->id_voltage_max)){
            break;
        }
    }
    printk(KERN_ERR "[smartstar] Get current batt_array position is %d .\n",i);

    return battery_data_array[i];
}


