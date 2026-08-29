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

/*lint -save -e801*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/spi/spi.h>
#include <osl_types.h>
#include <mdrv_public.h>
#include <bsp_edma.h>
#include <hi_spi.h>
#include "spi-dw.h"

#define DRIVER_NAME  "hisi_spi"
#define XFER_LIMIT   (0x10000U)
#define XFER_PIECE   (0xf000U)
#define XFER_ONETIME (0U) /*若一次需传输的长度小于等于一维传输长度最大值0X10000-1，则进行一次一维传输*/
#define XFER_MULTIME (1U) /*若一次需传输的长度大于一维传输长度最大值，则进行链表传输*/
#define hisi_spi_log(fmt, ...) \
    printk(KERN_ERR"[HISI_SPI]: <%s> line = %d  "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

struct hisi_spi {
    struct clk  *clk;
    int          dma_tx_chan;
    int          dma_rx_chan;
    edma_addr_t  edma_addr;
};

static unsigned g_burstlen = EDMA_BUR_LEN_4;

void hisi_writer(struct dw_spi *dws)
{
#define SPI_STATUS_BUSY      (1 << 0)
#define SPI_STATUS_TXNOTFULL (1 << 1)
#define SPI_STATUS_TXEMPTY   (1 << 2)

    int i;
    unsigned status;

    for (i = 0; i < dws->len; i++) {
        while(!(dw_readl(dws, DW_SPI_SR) & SPI_STATUS_TXNOTFULL));
        dw_writew(dws, DW_SPI_DR, *(u8*)dws->tx);
        ++dws->tx;
    }

    do {
        status = dw_readl(dws, DW_SPI_SR);
    } while((!(status & SPI_STATUS_TXEMPTY)) || (status & SPI_STATUS_BUSY));

    return;
}

void hisi_reader(struct dw_spi *dws)
{
#define SPI_STATUS_RXNOTEMPTY     (1 << 3)

    int i;

    for (i = 0; i < dws->len; i++) {
        while(!(dw_readl(dws, DW_SPI_SR) & SPI_STATUS_RXNOTEMPTY));
        *(u8 *)dws->rx++ = (u8)dw_readw(dws, DW_SPI_DR);
    }

    return;
}

void hisi_poll_transfer(struct dw_spi *dws)
{
    if (NULL != dws->tx) {
        do {
            hisi_writer(dws);
        } while (dws->tx_end > dws->tx);
    }

    if (NULL != dws->rx) {
        do {
            hisi_reader(dws);
        } while (dws->rx_end > dws->rx);
    }

    dw_spi_xfer_done(dws);
}

int hisi_spi_edma_init(struct dw_spi *dws)
{
    int              ret;
    struct hisi_spi *hisi = dws->dma_priv;

    hisi->dma_tx_chan = bsp_edma_channel_init(EDMA_SPI0_TX, NULL, 0, 0);
    if (hisi->dma_tx_chan < 0) {
        hisi_spi_log("bsp_edma_channel_init EDMA_SPI0_TX failed\n");
        ret = hisi->dma_tx_chan;
        goto ret_flag;
    }

    hisi->dma_rx_chan = bsp_edma_channel_init(EDMA_SPI0_RX, NULL, 0, 0);
    if (hisi->dma_rx_chan < 0) {
        hisi_spi_log("bsp_edma_channel_init EDMA_SPI0_RX failed\n");
        ret = hisi->dma_rx_chan;
        goto err_proc;
    }

    dws->dma_inited = 1;

    return 0;

err_proc:
    (void)bsp_edma_channel_free(hisi->dma_tx_chan);

ret_flag:
    return ret;
}

void hisi_spi_edma_exit(struct dw_spi *dws)
{
    int              ret;
    struct hisi_spi *hisi = dws->dma_priv;

    if (!dws->dma_inited)
        return;

    ret = bsp_edma_channel_free(hisi->dma_tx_chan);
    if (ret < 0) {
        hisi_spi_log("bsp_edma_channel_free EDMA_SPI0_TX failed\n");
    }

    ret = bsp_edma_channel_free(hisi->dma_rx_chan);
    if (ret < 0) {
        hisi_spi_log("bsp_edma_channel_free EDMA_SPI0_RX failed\n");
    }

    dws->dma_inited = 0;

    return;
}

int edma_tx_onetime(struct dw_spi *dws, int cs_change)
{
    int              ret;
    struct hisi_spi *hisi = dws->dma_priv;

    ret = bsp_edma_channel_set_config(hisi->dma_tx_chan, EDMA_M2P, dws->dma_width - 1, g_burstlen);
    if (0 != ret) {
        hisi_spi_log("bsp_edma_channel_set_config EDMA_M2P failed\n");
        return ret;
    }

    ret = bsp_edma_channel_async_start(hisi->dma_tx_chan, dws->tx_dma, dws->dma_addr, dws->len);
    if (0 != ret) {
        hisi_spi_log("bsp_edma_channel_async_start EDMA_M2P failed\n");
    }

    return ret;
}

int edma_rx_onetime(struct dw_spi *dws, int cs_change)
{
    int              ret;
    struct hisi_spi *hisi = dws->dma_priv;

    ret = bsp_edma_channel_set_config(hisi->dma_rx_chan, EDMA_P2M, dws->dma_width - 1, g_burstlen);
    if (0 != ret) {
        hisi_spi_log("bsp_edma_channel_set_config EDMA_P2M failed\n");
        return ret;
    }

    ret = bsp_edma_channel_async_start(hisi->dma_rx_chan, dws->dma_addr, dws->rx_dma, dws->len);
    if (0 != ret) {
        hisi_spi_log("bsp_edma_channel_async_start EDMA_P2M failed\n");
    }

    return ret;
}

int edma_xfer_onetime(struct dw_spi *dws, int cs_change)
{
    int              ret  = 0;
    struct hisi_spi *hisi = dws->dma_priv;

    if (dws->tx) {
        ret |= edma_tx_onetime(dws, cs_change);
    }

    if (dws->rx) {
        ret |= edma_rx_onetime(dws, cs_change);
    }

    if (dws->tx && dws->rx) {
        while((EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_tx_chan)) || (EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_rx_chan)));
        goto ret_flag;
    }

    if (dws->tx) {
        while(EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_tx_chan));
        goto ret_flag;
    }

    if (dws->rx) {
        while(EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_rx_chan));
    }

ret_flag:
    return ret;
}

void edma_cb_list_init(struct dw_spi *dws, struct edma_cb *list, unsigned int direction)
{
    struct edma_cb  *node = list;
    struct hisi_spi *hisi = dws->dma_priv;
    unsigned int xfer_res = dws->len % XFER_PIECE;
    unsigned int xfer_num = dws->len / XFER_PIECE + 1;
    unsigned int i        = 0;

    switch(direction) {
    /* RX config */
    case EDMA_P2M:
        for (i = 0; i < xfer_num; i++) {
            node->lli      =
                EDMA_SET_LLI(hisi->edma_addr + (i + 1) * sizeof(struct edma_cb), (i < xfer_num - 1) ? 0 : 1);
            node->config   =
                EDMA_SET_CONFIG(EDMA_SPI0_RX, EDMA_P2M, dws->dma_width - 1, g_burstlen);
            node->src_addr = dws->dma_addr;
            node->des_addr = dws->rx_dma + i * XFER_PIECE;
            node->cnt0     = (i < xfer_num - 1) ? XFER_PIECE : xfer_res;
            node->bindx    = 0;
            node->cindx    = 0;
            node->cnt1     = 0;

            node++;
        }
        break;
    /* TX config */
    case EDMA_M2P:
        for (i = 0; i < xfer_num; i++) {
            node->lli      =
                EDMA_SET_LLI(hisi->edma_addr + (i + 1) * sizeof(struct edma_cb), (i < xfer_num - 1) ? 0 : 1);
            node->config   =
                EDMA_SET_CONFIG(EDMA_SPI0_TX, EDMA_M2P, dws->dma_width - 1, g_burstlen);
            node->src_addr = dws->tx_dma + i * XFER_PIECE;
            node->des_addr = dws->dma_addr;
            node->cnt0     = (i < xfer_num - 1) ? XFER_PIECE : xfer_res;
            node->bindx    = 0;
            node->cindx    = 0;
            node->cnt1     = 0;

            node++;
        }
        break;

    default:
        hisi_spi_log("the parameter direction error\n");
        break;
    }
    return;
}

int edma_xfer_multime(struct dw_spi *dws, int cs_change)
{
    unsigned int xfer_num = dws->len / XFER_PIECE + 1;
    struct hisi_spi *hisi = dws->dma_priv;
    int    ret            = 0;
    struct edma_cb  *head = NULL;
    struct edma_cb  *list =
        (struct edma_cb *)dma_alloc_coherent(NULL, xfer_num*sizeof(struct edma_cb), &hisi->edma_addr, GFP_DMA);

    if (NULL == list) {
        hisi_spi_log("dma_alloc_coherent failed\n");
        return -1;
    }

    /* TX operation */
    if (dws->tx) {
        edma_cb_list_init(dws, list, EDMA_M2P);
        head = bsp_edma_channel_get_lli_addr(hisi->dma_tx_chan);
        if (NULL == head) {
            hisi_spi_log("bsp_edma_channel_get_lli_addr hisi->dma_tx_chan failed\n");
            ret = -2;
            goto ret_proc;
        }
        head->lli      = list->lli;
        head->config   = list->config & 0xFFFFFFFE;
        head->src_addr = list->src_addr;  /*物理地址*/
        head->des_addr = list->des_addr;  /*物理地址*/
        head->cnt0     = list->cnt0;
        head->bindx    = 0;
        head->cindx    = 0;
        head->cnt1     = 0;
        /*启动EDMA传输后即返回，通过查询通道是否busy来确定传输是否完成*/
        if (bsp_edma_channel_lli_async_start((u32)hisi->dma_tx_chan)) {
            hisi_spi_log("bsp_edma_channel_lli_async_start EDMA_M2P failed\n");
            ret = -3;
            goto ret_proc;
        }
    }

    /* RX operation */
    if (dws->rx) {
        edma_cb_list_init(dws, list, EDMA_P2M);
        head = bsp_edma_channel_get_lli_addr(hisi->dma_rx_chan);
        if (NULL == head) {
            hisi_spi_log("bsp_edma_channel_get_lli_addr hisi->dma_tx_chan failed\n");
            ret = -4;
            goto ret_proc;
        }
        head->lli      = list->lli;
        head->config   = list->config & 0xFFFFFFFE;
        head->src_addr = list->src_addr;  /*物理地址*/
        head->des_addr = list->des_addr;  /*物理地址*/
        head->cnt0     = list->cnt0;
        head->bindx    = 0;
        head->cindx    = 0;
        head->cnt1     = 0;
        /*启动EDMA传输后即返回，通过查询通道是否busy来确定传输是否完成*/
        if (bsp_edma_channel_lli_async_start((u32)hisi->dma_rx_chan)) {
            hisi_spi_log("bsp_edma_channel_lli_async_start EDMA_P2M failed\n");
            ret = -5;
            goto ret_proc;
        }
    }

    if (dws->tx && dws->rx) {
        while((EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_tx_chan)) || (EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_rx_chan)));
        goto ret_proc;
    }

    if (dws->tx) {
        while(EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_tx_chan));
        goto ret_proc;
    }

    if (dws->rx) {
        while(EDMA_CHN_BUSY == bsp_edma_channel_is_idle(hisi->dma_rx_chan));
    }

ret_proc:
    dma_free_coherent(NULL, xfer_num*sizeof(struct edma_cb), (void*)list, hisi->edma_addr);
    return ret;
}

int hisi_spi_edma_transfer(struct dw_spi *dws, int cs_change)
{
    int ret;
    u16 dma_ctrl = 0;

    if (dws->len <= 0) {
        hisi_spi_log("dws->len <= 0\n");
        return -1;
    }

    spi_enable_chip(dws, 0);
    dw_writew(dws, DW_SPI_DMARDLR, 0x3);
    dw_writew(dws, DW_SPI_DMATDLR, 0x4);
    dw_writew(dws, DW_SPI_RXFLTR,  0x7);
    dw_writew(dws, DW_SPI_TXFLTR,  0x7);
    if (dws->tx_dma)
        dma_ctrl |= 0x2;
    if (dws->rx_dma)
        dma_ctrl |= 0x1;
    dw_writew(dws, DW_SPI_DMACR, dma_ctrl);
    spi_enable_chip(dws, 1);

    if (dws->len < XFER_LIMIT) {
        /*若一次需传输的长度小于等于一维传输长度最大值0X10000-1，则进行一次一维传输*/
        ret = edma_xfer_onetime(dws, cs_change);
    } else {
        /*若一次需传输的长度大于一维传输长度最大值，则进行链表传输*/
        ret = edma_xfer_multime(dws, cs_change);
    }

    dw_spi_xfer_done(dws);

    return ret;
}

static struct dw_spi_dma_ops hisi_edma_ops = {
    .dma_init     = hisi_spi_edma_init,
    .dma_exit     = hisi_spi_edma_exit,
    .dma_transfer = hisi_spi_edma_transfer,
};

int __init hisi_spi_probe(struct platform_device *pdev)
{
    struct hisi_spi    *hisi;
    struct dw_spi      *dws;
    struct device_node *dev_node = NULL;
    int ret;

    if (NULL == pdev) {
        hisi_spi_log("devm_kzalloc dws failed\n");
        return EINVAL;
    }

    dev_node = pdev->dev.of_node;

    dws = devm_kzalloc(&pdev->dev, sizeof(struct dw_spi), GFP_KERNEL);
    if (NULL == dws) {
        hisi_spi_log("devm_kzalloc dws failed\n");
        ret = -ENOMEM;
        goto err_end;
    }

    hisi = devm_kzalloc(&pdev->dev, sizeof(struct hisi_spi), GFP_KERNEL);
    if (NULL == hisi) {
        hisi_spi_log("devm_kzalloc hisi failed\n");
        ret = -ENOMEM;
        goto err_end;
    }

    dws->dma_priv = hisi;

    ret = of_property_read_u32(dev_node, "id", &pdev->id);
    if(0 != ret) {
        hisi_spi_log("of_property_read_u32 id failed\n");
        goto err_end;
    }

    ret = of_property_read_u32_index((const struct device_node *)dev_node, "reg", 0, (u32*)&dws->paddr);
    if (0 != ret) {
        hisi_spi_log("of_property_read_u32_index reg failed\n");
        goto err_end;
    }

    dws->irq = irq_of_parse_and_map(dev_node, 0);
    if (dws->irq < 0) {
        hisi_spi_log("platform_get_irq failed\n");
        ret = dws->irq; /* -ENXIO */
        goto err_end;
    }

    dws->regs = of_iomap(dev_node, 0);
    if (NULL == dws->regs) {
        hisi_spi_log("of_iomap failed\n");
        ret = -ENXIO;
        goto err_end;
    }

    hisi->clk = devm_clk_get(&pdev->dev, pdev->id == 0 ? "dw_ssi0_clk" : "dw_ssi1_clk");
    if (IS_ERR(hisi->clk)) {
        ret = PTR_ERR(hisi->clk);
        hisi_spi_log("devm_clk_get failed\n");
        goto err_map;
    }

    ret = clk_prepare(hisi->clk);
    if (ret) {
        hisi_spi_log("clk_prepare failed\n");
        goto err_map;
    }

    ret = clk_enable(hisi->clk);
    if (ret) {
        hisi_spi_log("clk_enable failed\n");
        goto err_clk_prepare;
    }

    dws->parent_dev = &pdev->dev;
    dws->bus_num = pdev->id;
    dws->num_cs = 2;
    dws->max_freq = clk_get_rate(hisi->clk);
    dws->dma_ops = &hisi_edma_ops;

    ret = dw_spi_add_host(dws);
    if (ret) {
        hisi_spi_log("dw_spi_add_host failed\n");
        goto err_clk_enable;
    }

    platform_set_drvdata(pdev, dws);

    return 0;

err_clk_enable:
    clk_disable(hisi->clk);

err_clk_prepare:
    clk_unprepare(hisi->clk);

err_map:
    iounmap(dws->regs);

err_end:
    return ret;
}

int __exit hisi_spi_remove(struct platform_device *pdev)
{
    struct hisi_spi *hisi = NULL;
    struct dw_spi   *dws  = platform_get_drvdata(pdev);

    if (NULL == dws) {
        hisi_spi_log("platform_get_drvdata return NULL\n");
        return -1;
    }
    hisi = dws->dma_priv;

    platform_set_drvdata(pdev, NULL);
    dw_spi_remove_host(dws);
    clk_disable(hisi->clk);
    clk_unprepare(hisi->clk);
    hisi->clk = NULL;
    iounmap(dws->regs);

    return 0;
}

#ifdef CONFIG_PM
static int hisi_spi_suspend(struct device *dev)
{
    /*store reg data*/
    struct platform_device *pdev = to_platform_device(dev);
    struct dw_spi          *dws  = platform_get_drvdata(pdev);
    struct hisi_spi        *hisi = NULL;
    int                     ret  = 0;

    if(NULL == dws) {
        hisi_spi_log("platform_get_drvdata return NULL\n");
        return -1;
    }

    hisi = dws->dma_priv;

    ret = dw_spi_suspend_host(dws);
    if (ret) {
        hisi_spi_log("dw_spi_suspend_host failed\n");
        return ret;
    }

    clk_disable(hisi->clk);

    hisi_spi_log("hisi hisi suspend ok\n");

    return 0;
}

static int hisi_spi_resume(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct dw_spi          *dws  = platform_get_drvdata(pdev);
    struct hisi_spi        *hisi = NULL;
    int                     ret  = 0;

    if(NULL == dws) {
        hisi_spi_log("platform_get_drvdata return NULL\n");
        return -1;
    }

    hisi = dws->dma_priv;

    ret = clk_enable(hisi->clk);
    if (ret) {
        hisi_spi_log("clk_enable failed\n");
        return ret;
    }

    ret = dw_spi_resume_host(dws);
    if (ret) {
        clk_disable(hisi->clk);
        hisi_spi_log("dw_spi_resume_host failed\n");
        return ret;
    }

    hisi_spi_log("hisi spi resume ok\n");

    return 0;
}

static const struct dev_pm_ops hisi_spi_dev_pm_ops = {
    .suspend = hisi_spi_suspend,
    .resume  = hisi_spi_resume,
};

#define HISI_SPI_DEV_PM_OPS (&hisi_spi_dev_pm_ops)
#else
#define HISI_SPI_DEV_PM_OPS NULL
#endif

static const struct of_device_id hisi_spi_of_match[] = {
    { .compatible = "hisilicon,spi_app", },
    {},
};
MODULE_DEVICE_TABLE(of, hisi_spi_of_match);

/*SPI驱动注册*/
static struct platform_driver hisi_driver_spi = {
    .probe     = hisi_spi_probe,
    .remove    = __exit_p(hisi_spi_remove),
    .driver    = {
        .name  = DRIVER_NAME,
        .owner = THIS_MODULE,
        .pm    = HISI_SPI_DEV_PM_OPS,
        .of_match_table = of_match_ptr(hisi_spi_of_match),
    },
};

void hisi_spi_cs_control(u32 command)
{
    return;
}

/*spi控制器私有数据*/
struct dw_spi_chip sflash_chip_info = {
    .poll_mode  = 0,
    .type       = SSI_MOTO_SPI,
    .enable_dma = 1,
    .cs_control = hisi_spi_cs_control,
};

struct dw_spi_chip lcd_chip_info = {
    .poll_mode  = 0,
    .type       = SSI_MOTO_SPI,
    .enable_dma = 1,
    .cs_control = hisi_spi_cs_control,
};

/*spi从设备私有数据*/
struct spi_board_info spidev_board_info[] = {
    {
        .modalias        = "spi-nor-flash",
        .max_speed_hz    = 2000000,
        .bus_num         = SPI_SFLASH_NUM,
        .chip_select     = SPI_SFLASH_CS,
        .mode            = SPI_MODE_3,
        .controller_data = &sflash_chip_info,
    },
    {
        .modalias        = "balong_lcd_spi",
        .max_speed_hz    = 8000000,
        .bus_num         = SPI_LCD_NUM,
        .chip_select     = SPI_LCD_CS,
#if (FEATURE_ON == MBB_OLED)
        .mode            = SPI_MODE_3,
#else
        .mode            = SPI_MODE_0,
#endif

        .controller_data = &lcd_chip_info,
    },
};

int __init hisi_spi_init(void)
{
    int ret = 0;

    hisi_spi_log("start\n");

    ret = platform_driver_register(&hisi_driver_spi);
    if (ret) {
        hisi_spi_log("platform_driver_register failed\n");
        return -1;
    }

    ret = spi_register_board_info(spidev_board_info, ARRAY_SIZE(spidev_board_info));
    if (ret) {
        platform_driver_unregister(&hisi_driver_spi);
        hisi_spi_log("spi_register_board_info failed\n");
        return -1;
    }

    hisi_spi_log("end\n");
    return ret;
}
module_init(hisi_spi_init);

void __exit hisi_spi_exit(void)
{
    platform_driver_unregister(&hisi_driver_spi);
}
module_exit(hisi_spi_exit);

unsigned ch_burstlen_debug(unsigned burstlen)
{
    g_burstlen = burstlen;
    return g_burstlen;
}
/*lint -restore*/
