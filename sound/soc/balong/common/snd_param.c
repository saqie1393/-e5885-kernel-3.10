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

#include <product_config.h>
#include "snd_param.h"

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#include "bsp_nvim.h"
#include "CodecNvId.h"
#endif /* BSP_CONFIG_BOARD_TELEMATIC */

static unsigned int balong_sample_rate = SND_SAMPLE_RATE_8K;
void snd_sample_rate_set(unsigned int rate)
{
	balong_sample_rate = rate;
}
unsigned int snd_sample_rate_get(void)
{
	return balong_sample_rate;
}

static unsigned int balong_snd_master_slave = SND_SOC_MASTER;
void snd_master_slave_set(unsigned int mode)
{
	balong_snd_master_slave = mode;
}

unsigned int snd_master_slave_get(void)
{
	return balong_snd_master_slave;
}

static unsigned int balong_snd_data_mode = SND_PCM_MODE;
void snd_data_mode_set(unsigned int mode)
{
	balong_snd_data_mode = mode;
}

unsigned int snd_data_mode_get(void)
{
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    unsigned char usI2SPCMMode = 0;
    unsigned int ret = 0;

    /* 查询NV :en_NV_I2sPcmMode */
    /*lint -e64*/
    ret = bsp_nvm_read(en_NV_I2sPcmMode, &usI2SPCMMode, sizeof(unsigned char));
    /*lint +e64*/

    if (0 != ret)
    {
        printk(KERN_ERR "Read NV en_NV_I2sPcmMode fail!\n");
        /* 默认配置为PCM接口 */
        return SND_PCM_MODE;
    }

    if (0 == usI2SPCMMode)
    {
        return SND_I2S_MODE;
    }
    else
    {
        return SND_PCM_MODE;
    }
#else
	return balong_snd_data_mode;
#endif /* BSP_CONFIG_BOARD_TELEMATIC */
}

static unsigned int balong_snd_xrun_mode = SND_XRUN_SILENCE;
void snd_xrun_mode_set(unsigned int mode)
{
	balong_snd_xrun_mode = mode;
}

unsigned int snd_xrun_mode_get(void)
{
	return balong_snd_xrun_mode;
}
