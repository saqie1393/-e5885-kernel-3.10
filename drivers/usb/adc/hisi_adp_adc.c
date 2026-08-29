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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/syscalls.h>
#include <sound/asound.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/usb/balong_usb_nv.h>
#include <linux/usb/enum.h>

#include <mdrv.h>
#include <linux/usb/bsp_adc.h>
#include "hisi_adp_adc_lib.h"

#define ADC_WRITE_BUF_NUM       8
#define ADC_READ_BUF_SIZE       (640*6*2)
#define ADC_INFID               0xFF
#define ADP_ADC_FILP_INVALID(filp) (!(filp)  || IS_ERR(filp) || !filp->private_data)


/* adc msg level */
#define ADC_LEVEL_ERR	            BIT(0)
#define ADC_LEVEL_WARNING           BIT(1)
#define ADC_LEVEL_TRACE	            BIT(2)
#define ADC_LEVEL_DBG	            BIT(3)
#define ADC_LEVEL_INFO	            BIT(4)
#define ADC_LEVEL_BUG	            BIT(5)
#define ADC_LEVEL_RX_DBG	        BIT(6)
#define ADC_LEVEL_TX_DBG	        BIT(7)

unsigned int adc_msg_level = ADC_LEVEL_ERR;

#define ADC_ERR(args)               do {if (adc_msg_level & ADC_LEVEL_ERR) printk args;} while (0)
#define ADC_WARNING(args)           do {if (adc_msg_level & ADC_LEVEL_WARNING) printk args;} while (0)
#define ADC_TRACE(args)             do {if (adc_msg_level & ADC_LEVEL_TRACE) printk args;} while (0)
#define ADC_DBG(args)               do {if (adc_msg_level & ADC_LEVEL_DBG) printk args;} while (0)
#define ADC_INFO(args)              do {if (adc_msg_level & ADC_LEVEL_INFO) printk args;} while (0)
#define ADC_BUG(args,condition)          \
    do {\
        if (condition)\
        {\
            printk args;\
        }\
\
        if (gmac_msg_level & ADC_LEVEL_BUG)\
        {\
            BUG_ON(condition);\
        }\
    } while (0)
#define ADC_RX_DBG(args)           do {if (adc_msg_level & ADC_LEVEL_RX_DBG) printk args;} while (0)
#define ADC_TX_DBG(args)           do {if (adc_msg_level & ADC_LEVEL_TX_DBG) printk args;} while (0)


typedef void (*ADC_READ_CB_T)(void);
typedef void (*ADC_WRITE_CB_T)(void *buf);

typedef enum ADC_DEVICE_PORT
{
    ADC_DEVICE_PORT_CAPRURE = 0,
    ADC_DEVICE_PORT_PLAYBACK,
    ADC_DEVICE_PORT_NO
}ADC_DEVICE_PORT_E;

struct adp_adc_stat {
    int stat_init;
    int stat_remove;
    int stat_filp_open_err;
    int stat_wait_dev_insert_fail;
    int stat_wait_filp_open_fail;
    int stat_wait_adc_created;
    int stat_wait_adc_created_cnt;
    int stat_read_buf_alloc_fail;
    int stat_read_thread_created_fail;
    int stat_read_blocked;
    int stat_read_err;
    int stat_read_last_err_result;
    int stat_read_cb_null;
    int stat_read_cb_called;
    int stat_last_read_size;
    int stat_write_thread_created_fail;
    int stat_write_start;
    int stat_write_wait_fail;
    int stat_write_err;
    int stat_write_last_err;
    int stat_write_blocked;
    int stat_write_cb_called;
    int stat_invalid_filp;

    int stat_get_buf;
    int stat_ret_buf;
    int stat_ioctl_prepare_err;
    int stat_close;
    int stat_wait_close;
    int stat_close_err;
};

struct adc_port_context {
    char* dev_name;
    struct file* filp;
    int is_open;
    atomic_t opt_cnt;
};

struct adp_adc_context {
    struct adc_port_context port_ctx[ADC_DEVICE_PORT_NO];

    int start_read_check;
    int start_write_check;
    int bufs_to_write;
    int dev_insert;
    ADC_READ_CB_T adc_read_cb;
    ADC_WRITE_CB_T adc_write_cb;
    struct adp_adc_stat stat;

    struct snd_xferi read_xferi;
    struct snd_xferi write_xferi[ADC_WRITE_BUF_NUM];
    int write_start;
    int write_end;
    //struct workqueue_struct *adc_work_queue;
    wait_queue_head_t write_wait;
    wait_queue_head_t read_wait;
};

/*ctx 0 is capture; ctx 1 is playback. order can't change*/
static struct adp_adc_context adp_adc_ctx;

void adc_msg_level_set(int val)
{
    adc_msg_level = val;
}

void adc_msg_level_get(void)
{
    printk("adc_msg_level : 0x%x\n", adc_msg_level);
}

int write_xferi_full(void)
{
    return (((adp_adc_ctx.write_end + 1) % ADC_WRITE_BUF_NUM) == adp_adc_ctx.write_start);
}

int write_xferi_empty(void)
{
    return (adp_adc_ctx.write_end == adp_adc_ctx.write_start);
}

void adc_read_thread_check_set(int val)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    ctx->start_read_check = val;
}

void bsp_adc_dev_insert(void)
{
    adp_adc_ctx.dev_insert = 1;
    wake_up_interruptible(&adp_adc_ctx.read_wait);
}

void adc_read_thread(void)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx = &adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_CAPRURE];
    struct snd_xferi *xferi = &ctx->read_xferi;
    struct snd_pcm_file *pcm_file;
    struct snd_pcm_substream *substream;
    struct snd_pcm_runtime *runtime;
    snd_pcm_sframes_t result;
    int status;

start_again:
    ADC_TRACE(("[adc_read_thread]  wait adc dev\n"));

    status = wait_event_interruptible(ctx->read_wait, (adp_adc_ctx.dev_insert != 0));
    if (status) {
        ctx->stat.stat_wait_dev_insert_fail++;
        return;
    }

    // if dev is available
    while (sys_faccessat(AT_FDCWD, port_ctx->dev_name, O_RDWR)) {
        ctx->stat.stat_wait_adc_created++;
        ctx->stat.stat_wait_adc_created_cnt++;
        msleep(100);
    }

    // set_enum_state
    bsp_usb_set_enum_stat(ADC_INFID, 1, 1);
    adp_adc_ctx.start_read_check = 1;

    /*wait dev create*/
    status = wait_event_interruptible(ctx->read_wait, (port_ctx->filp != NULL));
    if (status) {
        ctx->stat.stat_wait_filp_open_fail++;
        return;
    }

    pcm_file = (struct snd_pcm_file *)port_ctx->filp->private_data;
    substream = pcm_file->substream;
    runtime = substream->runtime;

    while(ctx->start_read_check){
        if(ctx->stat.stat_last_read_size){
            ctx->stat.stat_read_blocked++;
        }else{
            xferi->frames = bytes_to_frames(runtime, ADC_READ_BUF_SIZE);
            result = snd_pcm_kernel_ioctl(substream,
                SNDRV_PCM_IOCTL_READI_FRAMES, xferi);

            /*notice application to get the receive buf*/
            if(result == 0){
                if(!ctx->adc_read_cb){
                    ctx->stat.stat_read_cb_null++;
                    continue;
                }else{
                    ctx->stat.stat_last_read_size = (int)frames_to_bytes(runtime,
                        xferi->result);
                    ADC_RX_DBG(("[read_thread] stat_last_read_size : %d  result size : %d\n",
                        ctx->stat.stat_last_read_size, (int)xferi->result));
                    ctx->adc_read_cb();
                    ctx->stat.stat_read_cb_called++;
                }
            }else{
                ctx->stat.stat_read_err++;
                ctx->stat.stat_read_last_err_result = result;
            }
        }
    }

    goto start_again;
}

void adc_write_thread(void)
{
    mm_segment_t old_fs;
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx = &adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_PLAYBACK];
    struct snd_xferi *xferi;
    struct file* filp = port_ctx->filp;
    struct snd_pcm_file *pcm_file;
    struct snd_pcm_substream *substream;
    snd_pcm_uframes_t result;
    int status;

    if (unlikely(ADP_ADC_FILP_INVALID(filp))){
        ctx->stat.stat_invalid_filp++;
        return;
    }

    pcm_file = (struct snd_pcm_file *)filp->private_data;
    substream = pcm_file->substream;

    while(ctx->start_write_check){

        status = wait_event_interruptible(ctx->write_wait, (ctx->bufs_to_write == 1));
        if (status) {
            ctx->stat.stat_write_wait_fail++;
            continue;
        }

        old_fs = get_fs();
        set_fs(KERNEL_DS);

        /*scan the write xferi to write*/
        while(!write_xferi_empty()){
            /*get xferi*/
            xferi = &ctx->write_xferi[ctx->write_start];
            /*write data to pcm*/
            result = snd_pcm_kernel_ioctl(substream,
                SNDRV_PCM_IOCTL_WRITEI_FRAMES, xferi);

            ADC_TX_DBG(("[write_thread]: frame : %d\n", (int)xferi->frames));

            if (result) {
                ADC_ERR(("bsp_adc_write error: %d\n", (int)result));
                ctx->stat.stat_write_err++;
                ctx->stat.stat_write_last_err = result;
                break;
            }else{
                if(xferi->result != xferi->frames){
                    ADC_TX_DBG(("[adc_write_thread] result:%d   frames:%d\n",
                        (int)xferi->result, (int)xferi->frames));
                }
                ADC_TX_DBG(("[write_thread]: result : %d\n", (int)xferi->result));
            }

            if(ctx->adc_write_cb){
                ctx->adc_write_cb(xferi->buf);
                xferi->frames = 0;
                ctx->stat.stat_write_cb_called++;
            }

            ctx->write_start = (ctx->write_start + 1) % ADC_WRITE_BUF_NUM;
        }

        set_fs(old_fs);
        ctx->bufs_to_write = 0;
    }
}

int bsp_adc_open(void)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx;
    struct file* filp;
    struct snd_pcm_file *pcm_file;
    struct snd_pcm_substream *substream;
    struct task_struct *tsk;
    int id;
    int ret;

    //TODO:ctrl filp need to be open?
    for(id = 0; id < ADC_DEVICE_PORT_NO; id++){
        port_ctx = &adp_adc_ctx.port_ctx[id];

        /* open sound stream*/
        filp = filp_open(port_ctx->dev_name, O_RDWR, 0);
        if (IS_ERR(filp)) {
            ctx->stat.stat_filp_open_err++;
            ret = -ENOENT;
            goto open_err;
        }

        pcm_file = (struct snd_pcm_file *)filp->private_data;
        substream = pcm_file->substream;
        ret = adc_default_hw_params_set(substream);
        if(ret){
            ADC_ERR(("bsp_adc_open: prepare failed! id = %d  result = %d\n", id, ret));
        }

        port_ctx->filp = filp;
        port_ctx->is_open = 1;
    }

    /* alloc read buf to receive sound data*/
    ctx->read_xferi.buf = kmalloc(ADC_READ_BUF_SIZE, GFP_KERNEL);
    if (NULL == ctx->read_xferi.buf)
    {
        ADC_ERR(("adc read buf alloc failed !\n"));
        ctx->stat.stat_read_buf_alloc_fail++;
        ret = -ENOMEM;
        goto open_err;
    }

    wake_up_interruptible(&ctx->read_wait);

    adp_adc_ctx.start_write_check = 1;
    tsk =  kthread_run((void*)adc_write_thread, (void*)NULL, "adc_write");
    if (!tsk)
    {
        ADC_ERR(("cannot start sys_acore thread\n"));
        adp_adc_ctx.stat.stat_write_thread_created_fail++;
        ret = -ENOMEM;
        goto open_err;
    }

    return 0;

open_err:
    for(id = 0; id < ADC_DEVICE_PORT_NO; id++){
        port_ctx = &adp_adc_ctx.port_ctx[id];
        if(port_ctx->filp){
            port_ctx->is_open = 0;
            ret = filp_close(port_ctx->filp, NULL);
                if(ret){
                ctx->stat.stat_close_err++;
            }
            port_ctx->filp = NULL;
        }
    }
    return ret;
}

int bsp_adc_close(void)
{
    struct file* filp;
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx;
    int id;
    int ret = 0;

    for(id = 0; id < ADC_DEVICE_PORT_NO; id++){
        port_ctx = &adp_adc_ctx.port_ctx[id];
        filp = port_ctx->filp;

        if (unlikely(ADP_ADC_FILP_INVALID(filp))) {
            ctx->stat.stat_invalid_filp++;
            return -EINVAL;
        }

        /* wait for file opt complete */

        while(atomic_read(&port_ctx->opt_cnt)) {
            ctx->stat.stat_wait_close++;
            msleep(10);
        }

        filp_close(filp, NULL);
        port_ctx->is_open = 0;
    }

    ctx->stat.stat_close++;
    bsp_adc_remove();

    /*clear read buf*/
    kfree(ctx->read_xferi.buf);
    ctx->read_xferi.buf = NULL;
    ctx->read_xferi.frames = 0;
    ctx->stat.stat_last_read_size = 0;

    return ret;
}

int bsp_adc_write(void *buf, u32 size)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx = &adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_PLAYBACK];
    struct file* filp = port_ctx->filp;
    struct snd_pcm_file *pcm_file;
    struct snd_pcm_substream *substream;
    struct snd_pcm_runtime *runtime;
    int result = 0;

    if (unlikely(ADP_ADC_FILP_INVALID(filp))){
        ctx->stat.stat_invalid_filp++;
        return -EINVAL;
    }

    pcm_file = (struct snd_pcm_file *)filp->private_data;
    substream = pcm_file->substream;
    runtime = substream->runtime;

    atomic_inc(&port_ctx->opt_cnt);
    if (unlikely(!port_ctx->is_open || !(filp->f_path.dentry))){
        result = -ENXIO;
        goto write_ret;
    }

    /*if has xferi to use*/
    if(write_xferi_full()){
        ctx->stat.stat_write_blocked++;
        result = -EBUSY;
        goto write_ret;
    }else{
        /*add buf to the chain*/
        ctx->write_xferi[ctx->write_end].buf = buf;
        ctx->write_xferi[ctx->write_end].frames = bytes_to_frames(runtime, size);

        ADC_TX_DBG(("[write] frame : %d  size : %d\n",
            (int)ctx->write_xferi[ctx->write_end].frames, size));

        if (runtime->status->state == SNDRV_PCM_STATE_XRUN ||
                runtime->status->state == SNDRV_PCM_STATE_SUSPENDED){
            result = snd_pcm_kernel_ioctl(substream,
                SNDRV_PCM_IOCTL_PREPARE, NULL);
            if (result < 0) {
                ADC_ERR(("Preparing sound card failed: %d\n", (int)result));
                ctx->stat.stat_ioctl_prepare_err++;
                goto write_ret;
            }
        }

        ctx->bufs_to_write = 1;
        ctx->write_end = (ctx->write_end + 1) % ADC_WRITE_BUF_NUM;
        wake_up_interruptible(&ctx->write_wait);
        ctx->stat.stat_write_start++;
    }

write_ret:
    atomic_dec(&port_ctx->opt_cnt);
    return result;
}

/*get read buf info*/
static void adc_get_buf(ADC_BUF_INFO_STRUCT *info)
{
    info->pVirAddr = adp_adc_ctx.read_xferi.buf;
    info->u32Size = adp_adc_ctx.stat.stat_last_read_size;
}

/*return read buf to thread to get new data*/
static void adc_ret_buf(void)
{
    adp_adc_ctx.stat.stat_last_read_size = 0;
}

int adc_capture_ioctl(u32 cmd, void *para)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    int ret = 0;

    switch (cmd) {
    case UDI_ADC_IOCTL_GET_READ_BUFFER_CB:
    {
        adc_get_buf((ADC_BUF_INFO_STRUCT *)para);
        ctx->stat.stat_get_buf++;
        break;
    }
    case UDI_ADC_IOCTL_RETUR_BUFFER_CB:
    {
        adc_ret_buf();
        ctx->stat.stat_ret_buf++;
        break;
    }
    default:
        break;
    }
    return ret;
}

int bsp_adc_ioctl(u32 cmd, void *para)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;

    switch (cmd) {
    case UDI_ADC_IOCTL_SET_WRITE_CB:
    {
        ctx->adc_write_cb = (ADC_WRITE_CB_T)para;
        break;
    }
    case UDI_ADC_IOCTL_SET_READ_CB:
    {
        ctx->adc_read_cb = (ADC_READ_CB_T)para;
        break;
    }
    case UDI_ADC_IOCTL_GET_READ_BUFFER_CB:
    case UDI_ADC_IOCTL_RETUR_BUFFER_CB:
    {
        adc_capture_ioctl(cmd, para);
        break;
    }
    default:
        break;
    }
    return 0;
}

void adc_ctx_stat_dump(struct adp_adc_stat *stat)
{
    //TODO:
    printk("stat_init                       %d\n", stat->stat_init);
    printk("stat_filp_open_err              %d\n", stat->stat_filp_open_err);
    printk("stat_wait_dev_insert_fail       %d\n", stat->stat_wait_dev_insert_fail);
    printk("stat_wait_filp_open_fail        %d\n", stat->stat_wait_filp_open_fail);
    printk("stat_wait_adc_created           %d\n", stat->stat_wait_adc_created);
    printk("stat_wait_adc_created_cnt       %d\n", stat->stat_wait_adc_created_cnt);
    printk("stat_read_buf_alloc_fail        %d\n", stat->stat_read_buf_alloc_fail);
    printk("stat_read_thread_created_fail   %d\n", stat->stat_read_thread_created_fail);
    printk("stat_read_blocked               %d\n", stat->stat_read_blocked);
    printk("stat_read_err                   %d\n", stat->stat_read_err);
    printk("stat_read_cb_null               %d\n", stat->stat_read_cb_null);
    printk("stat_read_cb_called             %d\n", stat->stat_read_cb_called);
    printk("stat_last_read_size             %d\n", stat->stat_last_read_size);
    printk("stat_write_thread_created_fail  %d\n", stat->stat_write_thread_created_fail);
    printk("stat_write_start                %d\n", stat->stat_write_start);
    printk("stat_write_wait_fail            %d\n", stat->stat_write_wait_fail);
    printk("stat_write_err                  %d\n", stat->stat_write_err);
    printk("stat_write_last_err             %d\n", stat->stat_write_last_err);
    printk("stat_write_blocked              %d\n", stat->stat_write_blocked);
    printk("stat_write_cb_called            %d\n", stat->stat_write_cb_called);
    printk("stat_get_buf                    %d\n", stat->stat_get_buf);
    printk("stat_ret_buf                    %d\n", stat->stat_ret_buf);
    printk("stat_ioctl_prepare_err          %d\n", stat->stat_ioctl_prepare_err);
    printk("stat_close                      %d\n", stat->stat_close);
    printk("stat_wait_close                 %d\n", stat->stat_wait_close);
    printk("stat_close_err                  %d\n", stat->stat_close_err);

    return;
}

void bsp_adc_dump(void)
{
    struct adp_adc_context *ctx = &adp_adc_ctx;
    struct adc_port_context *port_ctx;
    int id;

    printk("start_read_check        %d\n", ctx->start_read_check);
    printk("start_write_check       %d\n", ctx->start_write_check);
    printk("bufs_to_write           %d\n", ctx->bufs_to_write);
    printk("dev_insert              %d\n", ctx->dev_insert);
    printk("adc_read_cb             %pS\n", ctx->adc_read_cb);
    printk("adc_write_cb            %pS\n", ctx->adc_write_cb);
    printk("write_start             %d\n", ctx->write_start);
    printk("write_end               %d\n", ctx->write_end);

    for(id = 0; id < ADC_DEVICE_PORT_NO; id++){
        port_ctx = &ctx->port_ctx[id];
        printk("\ndev name            %s\n", port_ctx->dev_name);
        printk("is_open               %d\n", port_ctx->is_open);
        printk("opt_cnt               %d\n", atomic_read(&port_ctx->opt_cnt));
    }

    adc_ctx_stat_dump(&ctx->stat);
}

static void adc_port_ctx_clean(struct adc_port_context *port_ctx)
{
    port_ctx->filp = NULL;
    port_ctx->is_open = 0;
    atomic_set(&port_ctx->opt_cnt, 0);
}
void bsp_adc_remove(void)
{
    adp_adc_ctx.start_read_check = 0;
    adp_adc_ctx.start_write_check = 0;
    adp_adc_ctx.bufs_to_write = 0;
    adp_adc_ctx.adc_read_cb = NULL;
    adp_adc_ctx.adc_write_cb = NULL;
    adp_adc_ctx.write_start = 0;
    adc_port_ctx_clean(&adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_CAPRURE]);
    adc_port_ctx_clean(&adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_PLAYBACK]);

    // set_enum_state
    bsp_usb_set_enum_stat(ADC_INFID, 0, 1);

    adp_adc_ctx.stat.stat_remove++;
}

/*static*/ int adc_init(void)
{
    struct task_struct *tsk;
    int ret = 0;

    bsp_usb_host_init_enum_stat();

    memset(&adp_adc_ctx, 0, sizeof(adp_adc_ctx));
    adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_CAPRURE].dev_name =
        "/dev/snd/pcmC0D0c";
    adp_adc_ctx.port_ctx[ADC_DEVICE_PORT_PLAYBACK].dev_name =
        "/dev/snd/pcmC0D0p";

    init_waitqueue_head(&adp_adc_ctx.read_wait);
    init_waitqueue_head(&adp_adc_ctx.write_wait);

    tsk =  kthread_run((void*)adc_read_thread, (void*)NULL, "adc_read");
    if (!tsk)
    {
        ADC_ERR(("cannot start sys_acore thread\n"));
        adp_adc_ctx.stat.stat_read_thread_created_fail++;
        ret = -ENOMEM;
        goto init_fail;
    }

    bsp_usb_add_setup_dev_fdname(ADC_INFID, __FILE__, 1);
    adp_adc_ctx.stat.stat_init++;
    return ret;

init_fail:
    adp_adc_ctx.start_read_check = 0;
    return ret;

}
module_init(adc_init);

static void adc_cleanup(void)
{
    adp_adc_ctx.start_read_check = 0;
    adp_adc_ctx.start_write_check = 0;
}
module_exit(adc_cleanup);

