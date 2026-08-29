/*******************************************************************************
  Specialised functions for managing Ring mode

  Copyright(C) 2011  STMicroelectronics Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a. 
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

  It defines all the functions used to handle the normal/enhanced
  descriptors in case of the DMA is configured to work in chained or
  in ring mode.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include "stmmac.h"

static unsigned int stmmac_is_jumbo_frm(int len)
{
	unsigned int ret = 0;

	if (len >= BUF_SIZE_4KiB)
		ret = 1;

	return ret;
}

static int stmmac_set_16kib_bfsize(int mtu)
{
	int ret = 0;
	if (unlikely(mtu >= BUF_SIZE_8KiB))
		ret = BUF_SIZE_16KiB;
	return ret;
}

const struct stmmac_ring_mode_ops ring_mode_ops = {
	.is_jumbo_frm = stmmac_is_jumbo_frm,
	.set_16kib_bfsize = stmmac_set_16kib_bfsize,
};
