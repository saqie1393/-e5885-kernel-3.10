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



#include <linux/module.h>
#include <product_config.h>
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if (FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)
#include <linux/kernel.h> 
#include <linux/fs.h> 
#include <asm/uaccess.h> 
#include <linux/mm.h> 
#include <linux/kmod.h>
#include <linux/types.h>
#include <linux/syscalls.h> /* sys_open, sys_read; sys_close */
#include <linux/init.h>
#include "mbb_process_start.h"
#if (FEATURE_ON == MBB_MLOG)
#include <linux/mlog_lib.h>
#endif

#define EMMC_AT_CMD_MODE 0755

/*****************************************************************************
函 数 名 : SdioATTestEnable
功能描述 : 测试EMMC的文件拷贝是否正常
输入参数 : size 文件的大小
返 回 值 : ret
调用函数 : sys_open；sys_lseek； sys_close； sdio_run_cmd
修改内容 : 新生成函数
*****************************************************************************/

int SdioATTestEnable(unsigned int *size )
{
    int ret = 0; 
    char *FileName = "/system/bin/busybox";
    char *sdioname = "sdiotest";
    unsigned int fd = 0;
    unsigned int len = 0 ;
    if(NULL == size)
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SDIO, mlog_lv_factory, "%s : pointer to size is NULL \n", __func__);
#endif

        return -1;
    }

    fd = sys_open(FileName, O_APPEND | O_RDWR, EMMC_AT_CMD_MODE);
    if(fd < 0)
    {
        printk("open failed while mode is append, ret = %d\n", fd);
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SDIO, mlog_lv_factory, "%s : open failed while mode is append, ret = %d\n", __func__, fd);
#endif
        return -1;
    }

    len = sys_lseek(fd, 0, SEEK_END);
    if(len < 0 )
    {
        printk("seek failed! ret = %d\n", len);
        (void)sys_close(fd);
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SDIO, mlog_lv_factory, "%s : seek failed! ret = %d", __func__, len);
#endif

        return -1;
    }
    printk("%s: seek file size=%u\n", __func__, len);
    *size = len;

    sys_close(fd);

    ret = drv_start_user_process(sdioname, NULL, NEED_RESULT, WAIT_TIME_5S);
    printk("drv_start_user_process ret = %d  \n"  ,ret);
    return ret ;
}
#endif
#endif

MODULE_AUTHOR("HUAWEI DRIVER GROUP");   
MODULE_DESCRIPTION("Driver for huawei product");
MODULE_LICENSE("GPL");                 
