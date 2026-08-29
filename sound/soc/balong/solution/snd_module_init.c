/*
 *  sound/soc/balong/snd_module_init.c
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
 

#include <linux/module.h>
#include <sound/soc.h>
#include <product_config.h>
#include "snd_init.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "snd_param.h"

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#include "CodecNvId.h"
#include "bsp_nvim.h"
#endif

#include <linux/proc_fs.h>

#if (FEATURE_ON == MBB_MLOG)
#include <linux/mlog_lib.h>
#endif

#define SND_DEV_NUM 5

#ifdef BSP_CONFIG_BOARD_TELEMATIC
unsigned int g_bclk_rate = FS_CLK_4M;
#define NV_SIO_CFG_LOW_4_BIT 0xf
#endif

void snd_dev_release(struct device *dev)
{
    return;
}

static const struct platform_driver snd_balong_drv[SND_DEV_NUM]=
{
    [0]={
        .probe  = snd_pcm_dev_probe,
        .remove = snd_pcm_dev_remove,
        .driver = {
            .name  = "pcm_dai",
            .owner = THIS_MODULE,
        }
    },
    [1]={
        .probe  = snd_platform_probe,
        .remove = snd_platform_remove,
        .driver = {
            .name  = "snd_plat",
            .owner = THIS_MODULE,
        }
    },
   [2]={
        .probe  = snd_module_platform_probe,
        .remove = snd_module_platform_remove,
        .driver = {
            .name  = "snd_module_plat",
            .owner = THIS_MODULE,
        }
    },
    [3]={
        .probe  = snd_codec_probe,
        .remove = snd_codec_remove,
        .driver = {
            .name  = "dummy_codec",
            .owner = THIS_MODULE,
        }
    },
    [4]={
        .probe  = snd_module_machine_probe,
        .remove = snd_module_machine_remove,
        .driver = {
            .name  = "module_machine",
            .owner = THIS_MODULE,
        }
    },

};

static const struct platform_device snd_balong_dev[SND_DEV_NUM]=
{
    [0]={
        .name = "pcm_dai",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [1]={
        .name = "snd_plat",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [2]={
        .name = "snd_module_plat",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [3]={
        .name = "dummy_codec",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [4]={
        .name = "module_machine",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
};


static struct platform_device snd_dev[SND_DEV_NUM];
static struct platform_driver snd_drv[SND_DEV_NUM];

#define HS_PROC_FILE "audio_fs_flag"


static ssize_t audio_fs_proc_read(struct file* filp,
                                  char* buffer, size_t length, loff_t* offset)
{
    int ret = -1;
    int call_status;
    call_status = AT_IsCallOn(0);
    //获取通话状态传递给应用层
    ret = copy_to_user((char*)buffer, &call_status, sizeof(int));

    if (0 != ret)
    {
        printk(KERN_ERR"HS_FLAG copy_from_user fail!\r\n");
        return ret;
    }

    return 0;
}

//音频内核与应用层进行通信的文件操作方法集
static const struct file_operations fs_flag_ops =
{
    .owner    = THIS_MODULE,
    .open     = NULL,
    .read     = audio_fs_proc_read,
    .write    = NULL,
};

#ifdef BSP_CONFIG_BOARD_TELEMATIC

unsigned int snd_sio_bclk_get(void)
{
    unsigned short sio_config = 0;
    unsigned int clk_freq_idx = 0;
    unsigned int ret = 0;

    /* 根据NV30005配置时钟频率,bit9 (0 : 2m  1:4m) */
    ret = bsp_nvm_read(en_NV_Item_Sio_Config, &sio_config, sizeof(unsigned short));

    if (0 != ret)
    {
        printk(KERN_ERR"Read NV en_NV_Item_Sio_Config fail!\n");

        /* 默认配置为4M */
        return FS_CLK_4M;
    }

    /* NV30005中8-11bit表示车载的时钟：0代表给CODEC提供2M时钟，1代表给CODEC提供4M时钟 */
    clk_freq_idx = (sio_config >> NV_SIO_CFG_BIT_CODECFREQ) & (NV_SIO_CFG_LOW_4_BIT);

    if (0 == clk_freq_idx)
    {
        return FS_CLK_2M;
    }
    else
    {
        return FS_CLK_4M;
    }
}
#endif

int snd_balong_init(void)
{
    int ret = 0;
    int i   = 0;
    SOLUTION_PRODUCT_TYPE type = PRODUCT_TYPE_INVALID;

    /*如果是升级模式，就不初始化，防止无法正常升级*/
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        printk(KERN_ERR "Dload smem_data malloc fail!\n");
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_PCM, mlog_lv_factory, "%s: Dload smem_data malloc fail!\n", __func__);
#endif

        return -1;
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_PCM, mlog_lv_factory, "%s: download mode\n", __func__);
#endif

        return ret;
    }

    type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE == type)
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_PCM, mlog_lv_factory, "%s: ce product mode\n", __func__);
#endif

        return 0;
    }
    memcpy(snd_dev, snd_balong_dev, sizeof(snd_balong_dev));
    memcpy(snd_drv, snd_balong_drv, sizeof(snd_balong_drv));

    for(i = 0;i < SND_DEV_NUM;i++)
    {
        ret = platform_device_register(&snd_dev[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm failed to register platform device!\n");
#if ( FEATURE_ON == MBB_MLOG )
            mlog_print(MLOG_PCM, mlog_lv_factory, "%s: pcm failed to register platform device!\n", __func__);
#endif

            goto dev_fail;
        }

        ret = platform_driver_register(&snd_drv[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm balong failed to register platform driver!\n");
#if ( FEATURE_ON == MBB_MLOG )
            mlog_print(MLOG_PCM, mlog_lv_factory, "%s: pcm balong failed to register platform driver!\n", __func__);
#endif

            goto drv_fail;
        }
    }

    //创建/proc/audio_fs_flag文件，与应用层进行通信
    struct proc_dir_entry* audio_proc_file = proc_create(HS_PROC_FILE, S_IFREG \
            | S_IRUGO , NULL, &fs_flag_ops);

    if (NULL == audio_proc_file)
    {
        printk(KERN_ERR"%s: create proc file for hs_flag failed\n", __FUNCTION__);
    }

#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 上电初始化时将读取到的时钟频率赋值给全局变量 */
    g_bclk_rate = snd_sio_bclk_get();
#endif

    printk(KERN_ERR "sound card init ok!\n");

    return 0;

drv_fail:
    platform_device_unregister(&snd_dev[i]);

dev_fail:
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };
    printk(KERN_ERR "sound machine init fail!\n");
#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_PCM, mlog_lv_factory, "%s: sound machine init fail! \n", __func__);
#endif

    return ret;
}

void snd_balong_exit(void)
{
    int i = SND_DEV_NUM;
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };

    remove_proc_entry(HS_PROC_FILE, NULL);

    printk(KERN_ERR "sound balong exit ok!\n");
}

module_init(snd_balong_init);
module_exit(snd_balong_exit);


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC Driver");
MODULE_LICENSE("GPL");

