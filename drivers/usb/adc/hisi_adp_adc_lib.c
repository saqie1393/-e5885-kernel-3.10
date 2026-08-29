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
#include <linux/slab.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>


/**
 * Some ALSA internal helper functions
 */
static int adc_snd_interval_refine_set(struct snd_interval *i, unsigned int val)
{
    struct snd_interval t;
    t.empty = 0;
    t.min = t.max = val;
    t.openmin = t.openmax = 0;
    t.integer = 1;
    return snd_interval_refine(i, &t);
}

static int _adc_snd_pcm_hw_param_set(struct snd_pcm_hw_params *params,
    snd_pcm_hw_param_t var, unsigned int val, int dir)
{
    int changed;
    if (hw_is_mask(var)){
        struct snd_mask *m = hw_param_mask(params, var);
        if (val == 0 && dir < 0){
            changed = -EINVAL;
            snd_mask_none(m);
        }else{
            if(dir > 0)
                val++;
            else if(dir < 0)
                val--;
            changed = snd_mask_refine_set(hw_param_mask(params, var), val);
        }
    }else if(hw_is_interval(var)){
        struct snd_interval *i = hw_param_interval(params, var);
        if(val == 0 && dir < 0){
            changed = -EINVAL;
            snd_interval_none(i);
        }else if(dir == 0)
            changed = adc_snd_interval_refine_set(i, val);
        else{
            struct snd_interval t;
            t.openmin = 1;
            t.openmax = 1;
            t.empty = 0;
            t.integer = 0;
            if(dir < 0){
                t.min = val - 1;
                t.max = val;
            }else{
                t.min = val;
                t.max = val+1;
            }
            changed = snd_interval_refine(i, &t);
        }
    }else
        return -EINVAL;
    if(changed){
        params->cmask |= 1 << var;
        params->rmask |= 1 << var;
    }
    return changed;
}


int adc_default_hw_params_set(struct snd_pcm_substream *substream)
{
    int ret = 0;
    struct snd_pcm_hw_params *params;
    params = kzalloc(sizeof(*params), GFP_KERNEL);
    if (!params)
        return -ENOMEM;

    _snd_pcm_hw_params_any(params);
    _adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_ACCESS,
        SNDRV_PCM_ACCESS_RW_INTERLEAVED, 0);
    _adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_RATE,
        44100, 0);

#if 0
    snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS,
        &adc_constraints_channels_2);

    ret = _adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_CHANNELS,
            1, 0);
    if(ret < 0){
        printk("[SNDRV_PCM_HW_PARAM_CHANNELS]  failed  ret = %d\n", ret);
        goto set_end;
    }

    adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_FORMAT,
        SNDRV_PCM_FORMAT_S16_LE, 0);
    adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_CHANNELS,
        1, 0);

    ret = _adc_snd_pcm_hw_param_set(params, SNDRV_PCM_HW_PARAM_RATE,
        96000, 0);
    if(ret < 0){
        printk("[SNDRV_PCM_HW_PARAM_RATE]  failed  ret = %d\n", ret);
        goto set_end;
    }


    ret = snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_DROP, NULL);
    if(ret < 0){
        printk("[SNDRV_PCM_IOCTL_DROP]  failed  ret = %d\n", ret);
    }
#endif
    ret = snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_HW_PARAMS, params);
    if(ret < 0){
        printk("[SNDRV_PCM_IOCTL_HW_PARAMS]  failed  ret = %d\n", ret);
        goto set_end;
    }

    ret = snd_pcm_kernel_ioctl(substream, SNDRV_PCM_IOCTL_PREPARE, NULL);
    if (ret < 0) {
        printk("Preparing sound card failed: %d\n", (int)ret);
        goto set_end;
    }

set_end:
    kfree(params);
    return ret;
}

