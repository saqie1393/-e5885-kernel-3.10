/*
 *  Driver for AMBA serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
 *  Copyright (C) 2010 ST-Ericsson SA
 *  Copyright (C) Huawei Technologies Co., Ltd. 
 *  2015-02-09 - Ensure interrupts from this UART are masked and cleared 
 *  in probe function  nieluhua <foss@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This is a generic driver for ARM AMBA-type serial ports.  They
 * have a lot of 16550-like features, but are not register compatible.
 * Note that although they do have CTS, DCD and DSR inputs, they do
 * not have an RI input, nor do they have DTR or RTS outputs.  If
 * required, these have to be supplied via some other means (eg, GPIO)
 * and hooked into this driver.
 */

#define UART011_IFLS_RTS_FULL_2	(5 << 6)

static unsigned int get_fifosize_balong(unsigned int periphid)
{
	return 64;
}

static struct vendor_data vendor_balong = {
	.ifls					= UART011_IFLS_RTS_FULL_2 | UART011_IFLS_RX4_8 | UART011_IFLS_TX4_8,
	.lcrh_tx				= UART011_LCRH,
	.lcrh_rx				= UART011_LCRH,
	.oversampling			= false,
	.dma_threshold			= false,
	.cts_event_workaround	= true,
	.get_fifosize		= get_fifosize_balong,
};


#define AMBA_PL011_BALONG_IDS	{ \
		.id		= 0x000410aa,\
		.mask	= 0x000fffff,\
		.data	= &vendor_balong,\
	},
