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

#include <linux/device.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/dma-mapping.h>
#include "spi-nor-flash.h"

#define ARRAY_MAX (1<<16)
#define SPI_NOR_PAGE_SIZE (256)

typedef enum SPI_MSG_E {
    SPI_MSG_WE = 0,
    SPI_MSG_CE,
    SPI_MSG_SE,
    SPI_MSG_BE32,
    SPI_MSG_BE64,
    SPI_MSG_PP,
    SPI_MSG_RDDAT,
    SPI_MSG_WD,
    SPI_MSG_MAX,
    SPI_MSG_SIZE = ARRAY_MAX + 4
} SPI_MSG_E;

static u8 g_spi_msg[SPI_MSG_MAX][SPI_MSG_SIZE];

struct spi_nor_flash {
    struct spi_device *spi;
};

static struct spi_nor_flash *g_flash = NULL;
static int g_spi_dma_xfer = 1;

void spi_nor_flash_msgrecv(u8 *tx_msg, u8 *rx_msg, unsigned len)
{
    struct spi_device  *spi = g_flash->spi;
    struct spi_transfer t;
    struct spi_message  m;
    int                 ret;

    spi_message_init(&m);
    memset(&t, 0, sizeof(t));

    t.len    = len;
    t.tx_buf = tx_msg;
    t.rx_buf = rx_msg;

    if (g_spi_dma_xfer) {
        t.tx_dma = dma_map_single(&spi->dev, (void*)t.tx_buf, t.len, DMA_TO_DEVICE);
        if (dma_mapping_error(&spi->dev, t.tx_dma)) {
            hisi_spi_log("dma_map_single DMA_TO_DEVICE failed, ret = 0x%x\n", dma_mapping_error(&spi->dev, t.tx_dma));
            return;
        }
        t.rx_dma = dma_map_single(&spi->dev, t.rx_buf, t.len, DMA_FROM_DEVICE);
        if (dma_mapping_error(&spi->dev, t.rx_dma)) {
            dma_unmap_single(&spi->dev, t.tx_dma, t.len, DMA_TO_DEVICE);
            hisi_spi_log("dma_map_single DMA_TO_DEVICE failed, ret = 0x%x\n", dma_mapping_error(&spi->dev, t.rx_dma));
            return;
        }
        m.is_dma_mapped = 1;
    }

    spi_message_add_tail(&t, &m);

    ret = spi_sync(spi, &m);
    if (0 != ret) {
        hisi_spi_log("spi_sync failed\n");
    }

    return;
}

void spi_nor_flash_cmdsend(u8 *cmd, unsigned len)
{
    struct spi_device  *spi = g_flash->spi;
    struct spi_transfer t;
    struct spi_message  m;
    int                 ret;

    spi_message_init(&m);
    memset(&t, 0, sizeof(t));

    t.len    = len;
    t.tx_buf = cmd;

    if (g_spi_dma_xfer) {
        t.tx_dma = dma_map_single(&spi->dev, (void*)t.tx_buf, t.len, DMA_TO_DEVICE);
        if (dma_mapping_error(&spi->dev, t.tx_dma)) {
            hisi_spi_log("dma_map_single DMA_TO_DEVICE failed, ret = 0x%x\n", dma_mapping_error(&spi->dev, t.tx_dma));
            return;
        }
        m.is_dma_mapped = 1;
    }

    spi_message_add_tail(&t, &m);
    ret = spi_sync(spi, &m);

    if (0 != ret) {
        hisi_spi_log("spi_sync failed\n");
    }

    return;
}

int spi_nor_flash_reset(void)
{
    u8 *tx_msg = kmalloc(2, GFP_KERNEL | GFP_DMA);
    if (!tx_msg) {
        hisi_spi_log("kmalloc failed\n");
        return -ENOMEM;
    }

    tx_msg[0] = SPI_DEV_CMD_ER;
    tx_msg[1] = SPI_DEV_CMD_RSTDEV;

    spi_nor_flash_cmdsend(tx_msg, 2);

    kfree(tx_msg);

    return 0;
}

int spi_nor_flash_rdid(void)
{
    u8 *tx_msg = NULL;
    u8 *rx_msg = NULL;

    tx_msg = kmalloc(4, GFP_KERNEL | GFP_DMA);
    if (!tx_msg) {
        hisi_spi_log("kmalloc failed\n");
        return -ENOMEM;
    }

    rx_msg = kmalloc(4, GFP_KERNEL | GFP_DMA);
    if (!rx_msg) {
        kfree(tx_msg);
        hisi_spi_log("kmalloc failed\n");
        return -ENOMEM;
    }

    tx_msg[0] = SPI_DEV_CMD_JDCID;
    tx_msg[1] = 0x00;
    tx_msg[2] = 0x00;
    tx_msg[3] = 0x00;
    rx_msg[0] = 0xff;
    rx_msg[1] = 0xff;
    rx_msg[2] = 0xff;
    rx_msg[3] = 0xff;

    spi_nor_flash_msgrecv(tx_msg, rx_msg, 4);

    printk(KERN_ERR"0x%x,0x%x,0x%x,0x%x\n", rx_msg[0], rx_msg[1], rx_msg[2], rx_msg[3]);

    kfree(tx_msg);
    kfree(rx_msg);

    return 0;
}

int spi_nor_flash_probe(struct spi_device *spi)
{
    int ret;

    g_flash = devm_kzalloc(&spi->dev, sizeof(*g_flash), GFP_KERNEL);
    if (NULL == g_flash) {
        hisi_spi_log("devm_kzalloc return NULL\n");
        return -ENOMEM;
    }

    g_flash->spi = spi;

    spi_set_drvdata(spi, g_flash);

    spi->mode          = SPI_MODE_3;
    spi->bits_per_word = 8;
    spi->chip_select   = 1;
    spi->max_speed_hz  = 2000000;

    ret = spi_setup(spi);
    if (ret < 0) {
        hisi_spi_log("spi_setup failed\n");
        return ret;
    }

    spi_nor_flash_reset();

    hisi_spi_log("ok\n");

    return 0;
}

static struct spi_driver spi_nor_flash_driver = {
    .driver = {
        .name           = "spi-nor-flash",
        .owner          = THIS_MODULE,
        .of_match_table = NULL,
    },
    .probe  = spi_nor_flash_probe,
};

module_spi_driver(spi_nor_flash_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("gengjianguo@hisilicon.com");
MODULE_DESCRIPTION("spi nor flash w25q128fw driver");

int set_spi_dma_xfer(int val)
{
    int ret;

    switch(val) {
    case 0:
    case 1:
        g_spi_dma_xfer = val;
    default:
        ret = g_spi_dma_xfer;
    }

    return ret;
}

#define DATA_256 \
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,\
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,\
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,\
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,\
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,\
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,\
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,\
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,\
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,\
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,\
    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,\
    0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,\
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,\
    0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,\
    0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,\
    0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff

#define DATA_1K \
    DATA_256,\
    DATA_256,\
    DATA_256,\
    DATA_256

#define DATA_4K \
    DATA_1K,\
    DATA_1K,\
    DATA_1K,\
    DATA_1K

#define DATA_16K \
    DATA_4K,\
    DATA_4K,\
    DATA_4K,\
    DATA_4K

#define DATA_64K \
    DATA_16K,\
    DATA_16K,\
    DATA_16K,\
    DATA_16K

static u8 g_data[ARRAY_MAX] = {DATA_64K};
static u8 r_data[ARRAY_MAX + 4] = {0};

int spi_nor_flash_get_status(void)
{
    int status = 0xff;
    u8 *tx_msg = NULL;
    u8 *rx_msg = NULL;

    tx_msg = kmalloc(2, GFP_KERNEL | GFP_DMA);
    if (!tx_msg) {
        hisi_spi_log("kmalloc failed\n");
        return -ENOMEM;
    }

    rx_msg = kmalloc(2, GFP_KERNEL | GFP_DMA);
    if (!rx_msg) {
        kfree(tx_msg);
        hisi_spi_log("kmalloc failed\n");
        return -ENOMEM;
    }

    tx_msg[0] = SPI_DEV_CMD_RSR1;
    tx_msg[1] = 0x00;
    rx_msg[0] = 0xff;
    rx_msg[1] = 0xff;

    spi_nor_flash_msgrecv(tx_msg, rx_msg, 2);
    status = rx_msg[1];

    kfree(tx_msg);
    kfree(rx_msg);

    return status;
}

int spi_nor_flash_read(u32 flash_addr, u8 *mem_buf, u32 len)
{
    if (0 > flash_addr) {
        hisi_spi_log("0 > flash_addr, set flash_addr = 0\n");
        flash_addr = 0;
    }

    if (NULL == mem_buf) {
        hisi_spi_log("NULL == mem_buf\n");
        return -1;
    }

    if (0 >= len || len > SPI_MSG_SIZE - 4) {
        hisi_spi_log("0 >= len, return 0\n");
        return -2;
    }

    g_spi_msg[SPI_MSG_RDDAT][0] = SPI_DEV_CMD_RDDAT;
    g_spi_msg[SPI_MSG_RDDAT][1] = flash_addr >> 16;
    g_spi_msg[SPI_MSG_RDDAT][2] = flash_addr >> 8;
    g_spi_msg[SPI_MSG_RDDAT][3] = flash_addr;

    while (spi_nor_flash_get_status() & SPI_DEV_STATUS_BUSY);
    spi_nor_flash_msgrecv(g_spi_msg[SPI_MSG_RDDAT], mem_buf, len + 4);

    return 0;
}

int spi_nor_flash_erase(u32 flash_addr, u32 emod)
{
    u32 cmd_len = 0;
    u32 loop    = SPI_MAX_DELAY_TIMES;

    if (0 > flash_addr) {
        hisi_spi_log("0 > flash_addr, set flash_addr = 0\n");
        flash_addr = 0;
    }

    switch(emod) {
    case SPI_MSG_CE:
        cmd_len = 1;
        g_spi_msg[emod][0] = SPI_DEV_CMD_CE;
        break;
    case SPI_MSG_SE:
        cmd_len = 4;
        g_spi_msg[emod][0] = SPI_DEV_CMD_SE;
        if (0 != (flash_addr % 0x1000)) {
            hisi_spi_log("modify flash_addr 0x%x to  0x%x\n", flash_addr, flash_addr & ~0xfff);
            flash_addr &= ~0xfff;
        }
        g_spi_msg[emod][1] = flash_addr >> 16;
        g_spi_msg[emod][2] = flash_addr >> 8;
        g_spi_msg[emod][3] = flash_addr;
        break;
    case SPI_MSG_BE32:
        cmd_len = 4;
        g_spi_msg[emod][0] = SPI_DEV_CMD_BE32;
        if (0 != (flash_addr % 0x8000)) {
            hisi_spi_log("modify flash_addr 0x%x to  0x%x\n", flash_addr, flash_addr & ~0x7fff);
            flash_addr &= ~0x7fff;
        }
        g_spi_msg[emod][1] = flash_addr >> 16;
        g_spi_msg[emod][2] = flash_addr >> 8;
        g_spi_msg[emod][3] = flash_addr;
        break;
    case SPI_MSG_BE64:
        cmd_len = 4;
        g_spi_msg[emod][0] = SPI_DEV_CMD_BE64;
        if (0 != (flash_addr % 0x10000)) {
            hisi_spi_log("modify flash_addr 0x%x to  0x%x\n", flash_addr, flash_addr & ~0xffff);
            flash_addr &= ~0xffff;
        }
        g_spi_msg[emod][1] = flash_addr >> 16;
        g_spi_msg[emod][2] = flash_addr >> 8;
        g_spi_msg[emod][3] = flash_addr;
        break;
    default:
        hisi_spi_log("Erase cmd: 0x%x not support\n", emod);
        break;
    }

    g_spi_msg[SPI_MSG_WE][0] = SPI_DEV_CMD_WE;
    g_spi_msg[SPI_MSG_WD][0] = SPI_DEV_CMD_WD;

    while (spi_nor_flash_get_status() & SPI_DEV_STATUS_BUSY);
    spi_nor_flash_cmdsend(g_spi_msg[SPI_MSG_WE], 1);
    while (!(spi_nor_flash_get_status() & SPI_DEV_STATUS_WEL));
    spi_nor_flash_cmdsend(g_spi_msg[emod], cmd_len);
    while (spi_nor_flash_get_status() & (SPI_DEV_STATUS_BUSY|SPI_DEV_STATUS_WEL)) {
        loop--;
        if (0 == loop) {
            spi_nor_flash_cmdsend(g_spi_msg[SPI_MSG_WD], 1);
            hisi_spi_log("timeout\n");
            return -3;
        }
    }

    hisi_spi_log("spi erase success\n");

    return 0;
}

int spi_nor_flash_write(u32 flash_addr, u8 *mem_buf, u32 len)
{
    u32 loop    = SPI_MAX_DELAY_TIMES;

    if (0 > flash_addr) {
        hisi_spi_log("0 > flash_addr, set flash_addr = 0\n");
        flash_addr = 0;
    }

    if (NULL == mem_buf) {
        hisi_spi_log("NULL == mem_buf\n");
        return -1;
    }

    if (0 >= len || len > SPI_MSG_SIZE - 4) {
        hisi_spi_log("0 >= len, return 0\n");
        return -2;
    }

    g_spi_msg[SPI_MSG_WE][0] = SPI_DEV_CMD_WE;
    g_spi_msg[SPI_MSG_WD][0] = SPI_DEV_CMD_WD;

    g_spi_msg[SPI_MSG_PP][0] = SPI_DEV_CMD_PP;
    g_spi_msg[SPI_MSG_PP][1] = flash_addr >> 16;
    g_spi_msg[SPI_MSG_PP][2] = flash_addr >> 8;
    g_spi_msg[SPI_MSG_PP][3] = flash_addr;
    memcpy(g_spi_msg[SPI_MSG_PP] + 4, mem_buf, len);

    while (spi_nor_flash_get_status() & SPI_DEV_STATUS_BUSY);
    spi_nor_flash_cmdsend(g_spi_msg[SPI_MSG_WE], 1);
    while (!(spi_nor_flash_get_status() & SPI_DEV_STATUS_WEL));
    spi_nor_flash_cmdsend(g_spi_msg[SPI_MSG_PP], 4 + len);
    while (spi_nor_flash_get_status() & (SPI_DEV_STATUS_BUSY|SPI_DEV_STATUS_WEL)) {
        loop--;
        if (0 == loop) {
            spi_nor_flash_cmdsend(g_spi_msg[SPI_MSG_WD], 1);
            hisi_spi_log("timeout\n");
            return -3;
        }
    }

    return 0;
}

int spi_nor_flash_testcase_write(unsigned int addr, unsigned len)
{
    int ret;
    unsigned num = len >> 8;
    unsigned res = len % SPI_NOR_PAGE_SIZE;
    unsigned i   = 0;

    for (i = 0; i < num; i++) {
        ret = spi_nor_flash_write(addr + i * SPI_NOR_PAGE_SIZE, g_data, SPI_NOR_PAGE_SIZE);
        if (ret) {
            hisi_spi_log("i = %d, FAILED\n", i);
            return ret;
        }
    }

    if (res) {
        ret = spi_nor_flash_write(addr + i * SPI_NOR_PAGE_SIZE, g_data, res);
        if (ret) {
            hisi_spi_log("i = %d, FAILED\n", i);
            return ret;
        }
    }

    hisi_spi_log("PASS\n");
    return ret;
}

int spi_nor_flash_testcase_read(unsigned int addr, unsigned len)
{
    int          ret;
    unsigned int i;

    for (i = 0; i < len; i++) {
        r_data[i] = 0;
    }

    ret = spi_nor_flash_read(addr, r_data, len);
    if (ret) {
        hisi_spi_log("FAILED\n");
        return ret;
    }

    for (i = 0; i < len; i++) {
        if (g_data[i] != r_data[i + 4]) {
            hisi_spi_log("ERROR: g_data[0x%x] = 0x%x, r_data[0x%x] = 0x%x\n", i, g_data[i], i+4, r_data[i + 4]);
            return -1;
        }
    }

    hisi_spi_log("PASS\n");

    return ret;
}

