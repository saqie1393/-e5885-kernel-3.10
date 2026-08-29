/*
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
 *
 */

/*lint --e{537} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <hi_hifi.h>
#include <hi_onoff.h>
#include <ptable_com.h>

#include <bsp_trace.h>
#include <bsp_dsp.h>
#include <bsp_ipc.h>
#include <bsp_icc.h>
#include <bsp_sec.h>
#include <bsp_sram.h>
#include <bsp_nandc.h>
#include <bsp_blk.h>
#include <bsp_shared_ddr.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include <bsp_sysctrl.h>
#include <drv_mailbox_cfg.h>
#include <product_config.h>

#ifdef __cplusplus
extern "C" {
#endif

int bsp_dsp_is_hifi_exist(void)
{
    int ret = 0;
    DRV_MODULE_SUPPORT_STRU   stSupportNv = {0};

    ret = (int)bsp_nvm_read(NV_ID_DRV_MODULE_SUPPORT, (u8*)&stSupportNv, sizeof(DRV_MODULE_SUPPORT_STRU));
    if (ret)
        ret = 0;
    else
        ret = (int)stSupportNv.hifi;

    return ret;
}

int bsp_hifi_probe(struct platform_device *pdev)
{
    int ret = 0;
    if(0 == bsp_dsp_is_hifi_exist())
    {
        printk(KERN_INFO "load hifi not enable.\n");
        return 0;
    }

    ret = bsp_ipc_int_send(IPC_CORE_MCORE, IPC_MCU_INT_SRC_HIFI_PU);
    if (ret)
    {
        printk(KERN_ERR "send hifi ipc error %d\r\n", ret);
        return -1;
    
    }
    printk(KERN_ERR "load hifi ok.\n");

    return 0;
}

static struct platform_device bsp_hifi_device = {
    .name = "bsp_hifi",
    .id = 0,
    .dev = {
    .init_name = "bsp_hifi",
    },
};

static struct platform_driver bsp_hifi_drv = {
    .probe      = bsp_hifi_probe,
    .driver     = {
        .name     = "bsp_hifi",
        .owner    = THIS_MODULE,
    },
};

static int __init bsp_hifi_acore_init(void)
{
    int ret = 0;

    ret = platform_device_register(&bsp_hifi_device);
    if(ret)
    {
        printk("register his_modem device failed\r\n");
        return ret;
    }

    ret = platform_driver_register(&bsp_hifi_drv);
    if(ret)
    {
        printk("register his_modem driver failed\r\n");
        platform_device_unregister(&bsp_hifi_device);
    }

    return ret;
}

static void __exit bsp_hifi_acore_exit(void)
{
    platform_driver_unregister(&bsp_hifi_drv);
    platform_device_unregister(&bsp_hifi_device);
}

module_init(bsp_hifi_acore_init);
module_exit(bsp_hifi_acore_exit);

MODULE_AUTHOR("z00227143@huawei.com");
MODULE_DESCRIPTION("HIS Balong HIFI load");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
}
#endif

