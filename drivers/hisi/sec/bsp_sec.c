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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <linux/crypto.h>
#include <linux/scatterlist.h>

#include <osl_types.h>

#include <bsp_sram.h>
#include <bsp_nandc.h>
#include <bsp_blk.h>
#include <bsp_efuse.h>
#include <bsp_onoff.h>
#include <bsp_nvim.h>
#include <drv_nv_id.h>
#include <bsp_sec.h>

#include <soc_onchiprom.h>


#ifdef __cplusplus
extern "C" {
#endif

#define CIPHER_BSWAP32(val) \
        (((val) >> 24) | (((val) >> 8) & 0x0000ff00) | \
        (((val) << 8) & 0x00ff0000) | ((val) << 24))

extern int kdf_hash_setkey(void * key_addr);
extern int kdf_result_hash_get(void * out_hash);
extern int kdf_hash_make(void * sha_s_addr, u32 sha_s_len);
extern int kdf_hash_init(void);

int bsp_sec_calc_sha256(void* dataAddr, u32 dataLen, u32* hash)
{
    int i, ret;
    u32 length;
    u32 key[8];

    memset((void*)hash, 0, sizeof(key));
    while (dataLen)
    {
        length = dataLen > 512 ? 512 : dataLen;
        memcpy((void*)key, (void*)hash, sizeof(key));

        ret = kdf_hash_setkey((void*)key);
        if (ret) {
            printk(KERN_ERR"fail to set cipher key\r\n");
            return ret;
        }

        ret = kdf_hash_make(dataAddr, length);
        if (ret) {
            printk(KERN_ERR"fail to make hash\r\n");
            return ret;
        }

        ret = kdf_result_hash_get((void*)hash);
        if (ret) {
            printk(KERN_ERR"fail to get hash\r\n");
            return ret;
        }

        dataLen -= length;
        dataAddr += length;
    }

    for (i = 0; i < 8; i++) {
        hash[i] = CIPHER_BSWAP32(hash[i]);
    }

    return 0;
}

void rsa_mod_expex(unsigned int *a, unsigned int *b, unsigned int *c, unsigned int cDigits, unsigned int *d, unsigned int dDigits, unsigned int *key_c);

int bsp_sec_calc_rsa(KEY_STRUCT *pubKey, u32* pIndata, u32* pOutdata)
{
    if ((NULL == pubKey) || (NULL == pIndata) || (NULL == pOutdata))
        return -1;

    rsa_mod_expex((unsigned int *)pOutdata, (unsigned int *)pIndata, (unsigned int *)pubKey->e,
                   pubKey->eLen, (unsigned int *)pubKey->n, pubKey->nLen, (unsigned int *)pubKey->c);

    return 0;
}

int bsp_sec_idio_identify(void* dataAddr, u32 dataLen, KEY_STRUCT *pubKey, u32* pIdio)
{
    int i, ret;
    u32 hash[8];
    u32 rsa[IDIO_LEN/4];

    memset(hash, 0, 8*4);
    memset(rsa, 0, IDIO_LEN);

    ret = bsp_sec_calc_sha256(dataAddr, dataLen, hash);
    if (ret) {
        return ret;
    }

    ret = bsp_sec_calc_rsa(pubKey, pIdio, rsa);
    if (ret) {
        return ret;
    }

    for(i = 0; i < 8; i++) {
        if(hash[i] != rsa[i]) {
            printk(KERN_ERR"\nthe hash is:\n");
            for (i = 0; i < 8; i++)
                printk(KERN_ERR"%08X ", hash[i]);

            printk(KERN_ERR"\nthe corect result is:\n");
            for (i = 0; i < 8; i++)
                printk(KERN_ERR"%08X ", rsa[i]);

            printk(KERN_ERR"\n");

            return -1;
        }
    }

    return 0;
}

/* we use ssdma(sha-160) before, and we'll use cipher(sha-256) form now on.
 * we should use sha-256 to make idio for all images checked in kernel.
 */
int bsp_sec_check(void* image_addr, u32 image_length)
{
    void *oem_ca_addr, *oem_ca_idio_addr, *image_idio_addr;

    tOcrShareData *share_date = (tOcrShareData *)SRAM_SEC_SHARE_ADDR;

    /* if the sec check is disabled, return */
    if ((1 != share_date->bSecEn) || (1 != share_date->bRootCaInited))
    {
        printk(KERN_INFO"sec disable\r\n");
        return 0;
    }

    /* check oem ca */
    oem_ca_addr = image_addr + image_length + IDIO_LEN;
    oem_ca_idio_addr = oem_ca_addr + OEM_CA_LEN;
    if (bsp_sec_idio_identify(oem_ca_addr, OEM_CA_LEN, (KEY_STRUCT *)SRAM_SEC_CHECK_ROOTCA_ADDR, (u32*)oem_ca_idio_addr))
    {
        printk(KERN_ERR"oem ca check error, reboot...\r\n");
        bsp_drv_power_reboot();
        return -1;
    }

    /* check image */
    image_idio_addr = image_addr + image_length;
    if (bsp_sec_idio_identify(image_addr, image_length, (KEY_STRUCT *)oem_ca_addr, (u32*)image_idio_addr))
    {
        printk(KERN_ERR"image check error, reboot...\r\n");
        bsp_drv_power_reboot();
        return -1;
    }

    return 0;
}

static int __init bsp_sec_init(void)
{
    int ret = 0;
    struct clk *bootrom_clk;

    ret = kdf_hash_init();
    if (ret)
    {
        printk(KERN_ERR"fail to init kdf, error code %d\r\n", ret);
        return ret;
    }

    bootrom_clk = clk_get(NULL, "bootrom_clk");
    if (IS_ERR(bootrom_clk))
        return ret;

    (void)clk_prepare(bootrom_clk);
    (void)clk_enable(bootrom_clk);
    clk_disable(bootrom_clk);
    clk_unprepare(bootrom_clk);
    clk_put(bootrom_clk);

    return ret;
}

static void __exit bsp_sec_exit(void)
{
#if 0
    kdfdev_exit();
#endif
}

/*****************************************************************************
* 函 数 名  : bsp_sec_started
*
* 功能描述  : 查询当前版本是否已经启用安全启动
*
* 输入参数  : unsigned char *pu8Data 是否已经启用安全启动标志[1:启用   0:未启用]
* 输出参数  :
*
* 返 回 值  : 0，成功；其他，失败；
*
* 其它说明  :
*
*****************************************************************************/
int bsp_sec_started(unsigned char *value)
{
    tOcrShareData *share_date = (tOcrShareData *)SRAM_SEC_SHARE_ADDR;

    if (!value)
    {
        printk(KERN_ERR"error para.\n");
        return -1;
    }

    *value = share_date->bSecEn ? 1 : 0;

    return 0;
}

/*****************************************************************************
* 函 数 名  : bsp_sec_secboot_supported
*
* 功能描述  : 当前版本是否支持安全启动
*
* 输入参数  : unsigned char *pu8Data 是否支持安全启动标志[1:支持   0:不支持]
* 输出参数  :
*
* 返 回 值  : 0，成功；其他，失败；
*
* 其它说明  :
*
*****************************************************************************/
int bsp_sec_secboot_supported(unsigned char *value)
{
    int ret = 0;
    u16 nv_value = 0;

    if (!value)
    {
        printk(KERN_ERR"error para.\n");
        return -1;
    }

    *value = 0;

    ret = (int)bsp_nvm_read(NV_ID_DRV_SEC_BOOT_ENABLE_FLAG, (u8*)&nv_value, sizeof(nv_value));
    if (ret)
    {
        printk(KERN_ERR"fail to read nv, id = %d\n", (int)NV_ID_DRV_SEC_BOOT_ENABLE_FLAG);
        return -1;
    }

    if (nv_value)
        *value = 1;

    return 0;
}

/*****************************************************************************
* 函 数 名  : bsp_sec_check_image
*
* 功能描述  : 当前镜像是否支持安全启动
*
* 输入参数  : unsigned char *pu8Data 是否支持安全启动标志[1:支持   0:不支持]
* 输出参数  :
*
* 返 回 值  : 0，成功；其他，失败；
*
* 其它说明  :
*
*****************************************************************************/
int bsp_sec_check_image(unsigned char *value, unsigned int *ca_result)
{
    int ret = 0;
    u32 i, skip_len = 0;
    struct ST_PART_TBL *part_xloader = NULL;

    /* coverity[var_decl] */
    unsigned int root_ca[ROOT_CA_LEN / 4];

    if (!value)
    {
        printk(KERN_ERR"error para.\n");
        return -1;
    }

    *value = 0;

    part_xloader = find_partition_by_name(PTABLE_M3BOOT_NM);
    if(NULL == part_xloader)
    {
        printk(KERN_ERR"find xloader ptable fail.\n");
        return -1;
    }

    ret = bsp_blk_read(part_xloader->name, ROOT_CA_INDEX, (void*)&root_ca, sizeof(root_ca) , &skip_len);
    if (ret)
    {
        printk(KERN_ERR"read xloader error.\n");
        return -1;
    }

    for (i = 0; i < sizeof(root_ca)/sizeof(root_ca[0]); i++)
    {
        /* coverity[uninit_use] */
        if (root_ca[i])
        {
            *value = 1;
            break;
        }
    }

    if (ca_result)
        memcpy((void*)ca_result, (void*)root_ca, sizeof(root_ca));

    return ret;
}

/*****************************************************************************
* 函 数 名  : bsp_sec_start_secure
*
* 功能描述  : 启用安全启动
*
* 输入参数  : void
* 输出参数  :
*
* 返 回 值  : 0，成功；其他，失败；
*
* 其它说明  :
*
*****************************************************************************/
int bsp_sec_start_secure(void)
{
    int ret = 0;
    unsigned int value = 0;

    unsigned int root_ca[ROOT_CA_LEN / 4];

    struct hash_desc desc;

    struct scatterlist sg;
    unsigned int key[SHA256_HASH_SIZE];
    unsigned int digest[SHA256_HASH_SIZE];
#ifndef ROOT_CA_WITHOUT_MD5
    unsigned int md5_digest[MD5_HASH_SIZE];
#endif

    ret = bsp_sec_started((unsigned char *)&value);
    if (ret || value)
    {
        printk(KERN_ERR"secure already started.\n");
        return -1;
    }

    ret = bsp_sec_check_image((unsigned char *)&value, &root_ca[0]);
    if (ret || !value)
    {
        printk(KERN_ERR"secure not supported.\n");
        return -1;
    }

    desc.tfm = crypto_alloc_hash("hmac(sha256)", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(desc.tfm))
    {
        printk(KERN_ERR"crypto_alloc_hash error.\n");
        return -1;
    }

    (void)memset((void*)key, 0, sizeof(key));
    ret = crypto_hash_setkey(desc.tfm, (u8*)key, sizeof(key));
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_setkey error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    ret = crypto_hash_init(&desc);
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_init error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    sg_init_table(&sg, 1);
    sg_set_buf(&sg, (void*)root_ca, sizeof(root_ca));

    ret = crypto_hash_update(&desc, &sg, sizeof(root_ca));
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_update error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    ret = crypto_hash_final(&desc, (u8*)digest);
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_final error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

#ifndef ROOT_CA_WITHOUT_MD5
    crypto_free_hash(desc.tfm);

    desc.tfm = crypto_alloc_hash("md5", 0, CRYPTO_ALG_ASYNC);
    if (IS_ERR(desc.tfm))
    {
        printk(KERN_ERR"crypto_alloc_hash error.\n");
        return -1;
    }

    ret = crypto_hash_init(&desc);
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_init error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    sg_init_table(&sg, 1);
    sg_set_buf(&sg, (void*)digest, sizeof(digest));

    ret = crypto_hash_update(&desc, &sg, sizeof(digest));
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_update error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    ret = crypto_hash_final(&desc, (u8*)md5_digest);
    if (ret)
    {
        printk(KERN_ERR"crypto_hash_final error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    ret = bsp_efuse_write(md5_digest, 0, sizeof(md5_digest)/sizeof(md5_digest[0]));
    if (ret)
    {
        printk(KERN_ERR"write efuse error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }
#else
    ret = bsp_efuse_write(digest, 0, sizeof(digest)/sizeof(digest[0]));
    if (ret)
    {
        printk(KERN_ERR"write efuse error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }
#endif

    ret = bsp_efuse_read(&value, EFUSE_GROUP_INDEX_SECURE, 1);
    if (ret)
    {
        printk(KERN_ERR"read efuse error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

    if (value & EFUSE_BIT_FLAG_SEC_EN)
    {
        ret = -1;
        printk(KERN_ERR"secure already started.\n");
        goto err_set_key;
    }

    value |= EFUSE_BIT_FLAG_SEC_EN;
    ret = bsp_efuse_write(&value, EFUSE_GROUP_INDEX_SECURE, 1);
    if (ret)
    {
        printk(KERN_ERR"write efuse error. ret = 0x%08X\n", ret);
        goto err_set_key;
    }

err_set_key:
    crypto_free_hash(desc.tfm);

    return ret;
}

/*arch_initcall*/
subsys_initcall(bsp_sec_init);
module_exit(bsp_sec_exit);

MODULE_AUTHOR("z00227143@huawei.com");
MODULE_DESCRIPTION("HIS Balong V7R2 Image Check");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
}
#endif

