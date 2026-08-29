/*******************************************************************************
  This is the driver for the ST MAC 10/100/1000 on-chip Ethernet controllers.
  ST Ethernet IPs are built around a Synopsys IP Core.

	Copyright(C) 2007-2011 STMicroelectronics Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet
 * Quality-of-Service (DWC_ether_qos) core, 4.10a.
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

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

  Documentation available at:
	http://www.stlinux.com
  Support available at:
	https://bugzilla.stlinux.com/
*******************************************************************************/


#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/prefetch.h>
#include <linux/bitops.h>
#include <linux/in.h>
#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif /* CONFIG_NEW_STMMAC_DEBUG_FS */
#include <linux/net_tstamp.h>
#include <bsp_slice.h>
#include "stmmac_ptp.h"
#include "stmmac.h"
#include "stmmac_debug.h"
#include "dwmac_dma.h"
#include "stmmac_cbs.h"
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
#include <linux/spe/spe_interface.h>
#include "mdrv_spe_wport.h"
#endif

#if (FEATURE_ON == MBB_MLOG)
#include <linux/mlog_lib.h>
#endif
#if ((FEATURE_ON == MBB_DEVBOOTSTATE) && (FEATURE_ON == MBB_FACTORY))
#define MII_UNREADY 0 /*0表示MII未初始化OK*/
#define MII_READY 1   /*1表示MII已经初始化OK*/
static int g_hs_mii = MII_UNREADY;
struct work_struct mii_flag_work;
#endif
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if ((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST))
#define GMAC_CONTROL        0x00000000 /* Configuration */
//#define GMAC_CONTROL_LM     0x00001000 /* Loop-back mode */
#define GMAC_CONTROL_FULLDUPLEX    0x00000800  /*full duplex*/

#define AR8035_CONTROL_REG              0x0
#define AR8035_DIGITAL_LOOPBACK_100M    0x6100
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY_BCM89811)
#define BCM89811_CONTROL_REG            0x0
#define BCM89811_LOOPBACK               (1 << 14)
#endif

#define LOOPBACK_TEST_OK       0
#define LOOPBACK_TEST_ERROR    (-1)

#define LOOPBACK_RX_OK 0
#define LOOPBACK_RX_LIMITED    (-1)          /**there is not enough receive packets*/
#define LOOPBACK_RX_ERROR      (-2)           /**there is not enough receive packets*/

#define LOOPBACK_TX_OK 0
#define LOOPBACK_TX_ERROR      (-1)
#define LOOPBACK_MATCH_ERROR   (-2)
#define LOOPBACK_TIMEOUT_ERROR (-3)

#define TEST_PACKET_LEN        (76)
#define SKB_LEN_LOOPBACK       (1528)
#define LOOPBACK_WAIT          (1000)

struct sk_buff * g_tx_skb = NULL;
int g_rx_result = -1;
static char packet[TEST_PACKET_LEN]  = {
                        0x00,0x00,0x5e,0x00,0x01,0x69,0x00,0xe0,0x4c,0x97,
                        0xa7,0x6a,0x08,0x00,0x45,0x00,0x00,0x3e,0xdd,0x3a,
                        0x00,0x00,0x40,0x11,0x44,0x6c,0x0a,0x91,0x3d,0xae,
                        0xda,0x19,0x36,0xb0,0x22,0x68,0x1f,0x77,0x00,0x2a,
                        0x38,0xa1,0x00,0x00,0x01,0x03,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x16,0x0f,0xa0,0x00,0x00,0x00,0x10,
                        0x71,0xef,0x7f,0xea,0x3f,0x6c,0x46,0x6a,0x96,0x7c,
                        0xb4,0x6b,0xaf,0xaa,0xa9,0x04,
                        };
#endif
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
#ifdef BSP_CONFIG_BOARD_CPE  
unsigned int entry_list = 0;
unsigned int entry_cnt = 0;

extern void spe_wport_set_push_cb(int port, spe_wport_push_cb_t push_cb);
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY_BCM89811)
extern int bcm89811_phy_hwreset(void* priv);
#endif

static inline void stmmac_rx_refill(struct stmmac_priv *priv);
static int stmmac_rx(struct stmmac_priv *priv, int limit);

static int
stmmac_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags);
static int
stmmac_finish_td(int portno, struct sk_buff *skb, unsigned int flags);
static netdev_tx_t stmmac_spe_xmit(struct sk_buff *skb, struct net_device *dev);
#endif

/* Module parameters */
#define TX_TIMEO            5000
#define JUMBO_LEN           9000
#define GMAC_TIMER_RATIO    (32768)                     /*one second*/
#define GMAC_TMOUT          (2 * GMAC_TIMER_RATIO)      /*must less than 16 second*/
#define GMAC_LEN_RATIO      (1024*1024)

#undef STMMAC_DEBUG
/*#define STMMAC_DEBUG*/
#ifdef STMMAC_DEBUG
#define DBG(nlevel, klevel, fmt, args...) \
		((void)(netif_msg_##nlevel(priv) && \
		printk(KERN_##klevel fmt, ## args)))
#else
#define DBG(nlevel, klevel, fmt, args...) do { } while (0)
#endif

struct stmmac_extra_stats *gmac_status;
struct stmmac_priv *gmac_priv;
EXPORT_SYMBOL(gmac_priv);

#ifdef CONFIG_AVB_NET
struct gmac_tx_queue *tx_queue_class_a;
struct gmac_tx_queue *tx_queue_class_b;
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH)
/* 移植代码，无法直接引用mbb_net.h，需要跨目录，此处暂用extern */
extern int mbb_mac_clone_tx_save(struct sk_buff *skb);
extern int mbb_mac_clone_rx_restore(struct sk_buff *skb);
extern int mbb_check_net_upgrade(struct sk_buff *skb);
extern int mbb_get_eth_macAddr(char *eth_macAddr);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
#define   NET_DEVICE_NAME   "et0"
int g_mmitest_enable = 0; 
#define LAN_WAN_PORT_NAME   "eth0"
#define LAN_WAN_VLAN_TAG    5
#else
#define   NET_DEVICE_NAME   "eth0"
#endif
#define SIOCLINKSTATE        0x89F8
#define SIOCLINKENABLE       0x89FA
#endif

#define STMMAC_ALIGN(x)	L1_CACHE_ALIGN(x)
static int watchdog = TX_TIMEO;

module_param(watchdog, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "Transmit timeout in milliseconds (default 5s)");

static int debug = -1;
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Message Level (-1: default, 0: no output, 16: all)");

#if (FEATURE_ON == MBB_FEATURE_ETH)
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
int phyaddr = 4;
#else
int phyaddr = 0;
#endif
#else
int phyaddr = -1;
#endif
module_param(phyaddr, int, S_IRUGO);
MODULE_PARM_DESC(phyaddr, "Physical device address");

#define DMA_TX_SIZE 512
static int dma_txsize = DMA_TX_SIZE;
module_param(dma_txsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_txsize, "Number of descriptors in the TX list");

#define DMA_RX_SIZE 512
static int dma_rxsize = DMA_RX_SIZE;
module_param(dma_rxsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_rxsize, "Number of descriptors in the RX list");

static int flow_ctrl = FLOW_OFF;
module_param(flow_ctrl, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(flow_ctrl, "Flow control ability [on/off]");

static int pause = PAUSE_TIME;
module_param(pause, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pause, "Flow Control Pause Time");

#define TC_DEFAULT 64
static int tc = TC_DEFAULT;
module_param(tc, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tc, "DMA threshold control value");

#if (FEATURE_ON == MBB_FEATURE_ETH)
#define DMA_BUFFER_SIZE BUF_SIZE_4KiB
#else
#define DMA_BUFFER_SIZE BUF_SIZE_2KiB
#endif

static int buf_sz = DMA_BUFFER_SIZE;
module_param(buf_sz, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(buf_sz, "DMA buffer size");

static const u32 default_msg_level = (NETIF_MSG_DRV | NETIF_MSG_PROBE |
				      NETIF_MSG_LINK | NETIF_MSG_IFUP |
				      NETIF_MSG_IFDOWN | NETIF_MSG_TIMER);

#define STMMAC_DEFAULT_LPI_TIMER	1000
static int eee_timer = STMMAC_DEFAULT_LPI_TIMER;
module_param(eee_timer, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(eee_timer, "LPI tx expiration time in msec");
#define STMMAC_LPI_T(x) (jiffies + msecs_to_jiffies(x))
static irqreturn_t stmmac_interrupt(int irq, void *dev_id);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
static int stmmac_init_fs(struct net_device *dev);
static void stmmac_exit_fs(void);
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
//static
netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev);
#endif
#if ((FEATURE_ON == MBB_DEVBOOTSTATE) && (FEATURE_ON == MBB_FACTORY))
/*************************************************************************
* 函数名     :get_mii_flag
* 功能描述   :读取当前g_hs_mii状态,g_hs_mii标识rgmii/mii是否启动ok
* 输入参数   :
* 输出参数   :
* 返回值     :
**************************************************************************/
int get_mii_flag()
{
    return g_hs_mii;
}
/*************************************************************************
* 函数名     :get_mii_flag
* 功能描述   :设置当前g_hs_mii状态,g_hs_mii标识rgmii/mii是否启动ok
* 输入参数   :
* 输出参数   :
* 返回值     :
**************************************************************************/
void set_mii_flag()
{
    msleep(500); /*MII在上层应用执行open操作后并不能立即开始测试,需要等待一段时间才能进行测试.*/
    g_hs_mii = MII_READY;
}
#endif
#define STMMAC_COAL_TIMER(x) (jiffies + usecs_to_jiffies(x))

unsigned int gmac_msg_level = GMAC_LEVEL_ERR;
EXPORT_SYMBOL(gmac_msg_level);

int riwt_value = MIN_DMA_RIWT;
int rx_irq_flag = 0;

void set_riwt_value(unsigned int cnt)
{
	riwt_value = cnt;
}

void set_rx_irq_flag(unsigned int cnt)
{
	rx_irq_flag = cnt;
}
void set_gmac_msg(unsigned int level)
{
	gmac_msg_level = level;
}
EXPORT_SYMBOL(set_gmac_msg);

#ifdef CONFIG_AVB_NET
void set_gmac_avb(unsigned int status)
{
	gmac_priv->avb_support = !!(status);
}
EXPORT_SYMBOL(set_gmac_avb);

unsigned long get_gmac_avb(void)
{
	return gmac_priv->avb_support;
}
EXPORT_SYMBOL(get_gmac_avb);
#endif
unsigned int get_gmac_msg(void)
{
	printk("level : %x\n",gmac_msg_level);
    return gmac_msg_level;
}
EXPORT_SYMBOL(get_gmac_msg);

void gmac_enable_print_rate(unsigned int enable)
{
	gmac_priv->enable_rate = !!(enable);
}
EXPORT_SYMBOL(gmac_enable_print_rate);

/**
 * stmmac_verify_args - verify the driver parameters.
 * Description: it verifies if some wrong parameter is passed to the driver.
 * Note that wrong parameters are replaced with the default values.
 */
static void stmmac_verify_args(void)
{
	if (unlikely(watchdog < 0))
		watchdog = TX_TIMEO;
	if (unlikely(dma_rxsize < 0))
		dma_rxsize = DMA_RX_SIZE;
	if (unlikely(dma_txsize < 0))
		dma_txsize = DMA_TX_SIZE;
	if (unlikely((buf_sz < DMA_BUFFER_SIZE) || (buf_sz > BUF_SIZE_16KiB)))
		buf_sz = DMA_BUFFER_SIZE;
	if (unlikely(flow_ctrl > 1))
		flow_ctrl = FLOW_AUTO;
	else if (likely(flow_ctrl < 0))
		flow_ctrl = FLOW_OFF;
	if (unlikely((pause < 0) || (pause > 0xffff)))
		pause = PAUSE_TIME;
	if (eee_timer < 0)
		eee_timer = STMMAC_DEFAULT_LPI_TIMER;
}

/**
 * stmmac_clk_csr_set - dynamically set the MDC clock
 * @priv: driver private structure
 * Description: this is to dynamically set the MDC clock according to the csr
 * clock input.
 * Note:
 *	If a specific clk_csr value is passed from the platform
 *	this means that the CSR Clock Range selection cannot be
 *	changed at run-time and it is fixed (as reported in the driver
 *	documentation). Viceversa the driver will try to set the MDC
 *	clock dynamically according to the actual clock input.
 */

/* minimum number of free TX descriptors required to wake up TX process */
#define STMMAC_TX_THRESH(x, n)	(x->tx_queue[n].dma_tx_size/8)

static inline u32 stmmac_tx_avail(struct stmmac_priv *priv, int chn)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);
	return tx_queue->dirty_tx + tx_queue->dma_tx_size - tx_queue->cur_tx - 1;
}

/**
 * stmmac_hw_fix_mac_speed: callback for speed selection
 * @priv: driver private structure
 * Description: on some platforms (e.g. ST), some HW system configuraton
 * registers have to be set according to the link speed negotiated.
 */
static inline void stmmac_hw_fix_mac_speed(struct stmmac_priv *priv)
{
	struct phy_device *phydev = priv->phydev;

	if (likely(priv->plat->fix_mac_speed))
		priv->plat->fix_mac_speed(priv->plat->bsp_priv, phydev->speed);
}

/**
 * stmmac_enable_eee_mode: Check and enter in LPI mode
 * @priv: driver private structure
 * Description: this function is to verify and enter in LPI mode for EEE.
 */
static void stmmac_enable_eee_mode(struct stmmac_priv *priv, int chn)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);

	/* Check and enter in LPI mode */
	if ((tx_queue->dirty_tx == tx_queue->cur_tx) &&
	    (tx_queue->tx_path_in_lpi_mode == false))
		priv->hw->mac->set_eee_mode(priv->ioaddr);
}

/**
 * stmmac_disable_eee_mode: disable/exit from EEE
 * @priv: driver private structure
 * Description: this function is to exit and disable EEE in case of
 * LPI state is true. This is called by the xmit.
 */
void stmmac_disable_eee_mode(struct stmmac_priv *priv)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);

	priv->hw->mac->reset_eee_mode(priv->ioaddr);
	del_timer_sync(&priv->eee_ctrl_timer);
	tx_queue->tx_path_in_lpi_mode = false;
}

/**
 * stmmac_eee_ctrl_timer: EEE TX SW timer.
 * @arg : data hook
 * Description:
 *  if there is no data transfer and if we are not in LPI state,
 *  then MAC Transmitter can be moved to LPI state.
 */
static void stmmac_eee_ctrl_timer(unsigned long arg)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)arg;

	stmmac_enable_eee_mode(priv, 0);
	mod_timer(&priv->eee_ctrl_timer, STMMAC_LPI_T(eee_timer));
}

/**
 * stmmac_eee_init: init EEE
 * @priv: driver private structure
 * Description:
 *  If the EEE support has been enabled while configuring the driver,
 *  if the GMAC actually supports the EEE (from the HW cap reg) and the
 *  phy can also manage EEE, so enable the LPI state and start the timer
 *  to verify if the tx path can enter in LPI state.
 */
bool stmmac_eee_init(struct stmmac_priv *priv)
{
	bool ret = false;

	/* Using PCS we cannot dial with the phy registers at this stage
	 * so we do not support extra feature like EEE.
	 */
	if ((priv->pcs == STMMAC_PCS_RGMII) || (priv->pcs == STMMAC_PCS_TBI) ||
	    (priv->pcs == STMMAC_PCS_RTBI))
		goto out;

	/* MAC core supports the EEE feature. */
	if (priv->dma_cap.eee) {
		/* Check if the PHY supports EEE */
		if (phy_init_eee(priv->phydev, 1))
			goto out;

		if (!priv->eee_active) {
			priv->eee_active = 1;
			init_timer(&priv->eee_ctrl_timer);
			priv->eee_ctrl_timer.function = stmmac_eee_ctrl_timer;
			priv->eee_ctrl_timer.data = (unsigned long)priv;
			priv->eee_ctrl_timer.expires = STMMAC_LPI_T(eee_timer);
			add_timer(&priv->eee_ctrl_timer);

			priv->hw->mac->set_eee_timer(priv->ioaddr,
						     STMMAC_DEFAULT_LIT_LS,
						     priv->tx_lpi_timer);
		} else
			/* Set HW EEE according to the speed */
			priv->hw->mac->set_eee_pls(priv->ioaddr,
						   priv->phydev->link);

		pr_info("stmmac: Energy-Efficient Ethernet initialized\n");

		ret = true;
	}
out:
	return ret;
}

/* stmmac_get_tx_hwtstamp: get HW TX timestamps
 * @priv: driver private structure
 * @entry : descriptor index to be used.
 * @skb : the socket buffer
 * Description :
 * This function will read timestamp from the descriptor & pass it to stack.
 * and also perform some sanity checks.
 */
static void stmmac_get_tx_hwtstamp(struct stmmac_priv *priv,
				   unsigned int entry, struct sk_buff *skb)
{
	struct skb_shared_hwtstamps shhwtstamp;
	u64 ns;
	struct stmmac_time_spec time;
	union dma_desc *desc;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);	//only using channel 0
	unsigned int rollover_mode;

	if (!tx_queue->hwts_tx_en)
		return;

	/* exit if skb doesn't support hw tstamp */
	if (likely(!(skb_shinfo(skb)->tx_flags & SKBTX_IN_PROGRESS)))
		return;

	desc = (tx_queue->dma_tx + entry);

	/* check tx tstamp status */
	if (!priv->hw->desc->get_tx_timestamp_status(desc))
		return;

	if (gmac_msg_level & GMAC_LEVEL_DESC_DBG) {
		printk(KERN_ERR "[%s]entry:%u\n", __func__, entry);
		print_hex_dump(KERN_ERR, "tx_desc:", DUMP_PREFIX_ADDRESS,
		       16, 4, (void *)desc, sizeof(union dma_desc), false);
	}

	/* get the valid tstamp */
	priv->hw->desc->get_timestamp(desc, &time);

	rollover_mode = priv->hw->mac->get_ts_rollover(priv->ioaddr);
	if (rollover_mode) {
		ns = (time.sec * 1000000000ULL) + (u64)time.nsec;//one nanosecond
	} else {
		ns = (time.sec * 1000000000ULL) + div_u64(time.nsec * 61ULL, 131);//0.466
	}

	GMAC_PTP_DBG(("[%s]mode:%d, sec:%u, nsec:%u, ns:%llu\n", __func__,
			rollover_mode, time.sec, time.nsec, ns));

	memset(&shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp.hwtstamp = ns_to_ktime(ns);

	/* pass tstamp to stack */
	skb_tstamp_tx(skb, &shhwtstamp);

	return;
}

static void stmmac_print_rx_tstamp_info(union dma_desc *desc)
{
	u32 pkt_type = 0;
	char *tstamp_dropped = NULL;
	char *tstamp_available = NULL;
	char *ptp_version = NULL;
	char *ptp_pkt_type = NULL;
	char *ptp_msg_type = NULL;
	struct rd_nwb_desc *nwb_desc = &(desc->rx_desc.nwb);

	if (!(gmac_msg_level & GMAC_LEVEL_PTP_DBG)) {
		return;
	}

	GMAC_PTP_DBG(("\n-->stmmac_print_rx_tstamp_info\n"));

	/* status in RDES1 is not valid */
	if (!(nwb_desc->rdes1_stat_valid))
		return;

	tstamp_dropped = (nwb_desc->ts_dropped ? "YES" : "NO");
	tstamp_available = (nwb_desc->ts_avail ? "YES" : "NO");
	ptp_version = (nwb_desc->ptp_ver ? "v2 (1588-2008)" : "v1 (1588-2002)");
	ptp_pkt_type = (nwb_desc->ptp_pkt_type ? "ptp over Eth" : "ptp over IPv4/6");

	pkt_type = nwb_desc->ptp_msg_type;
	switch (pkt_type) {
		case 0:
			ptp_msg_type = "NO PTP msg received";
			break;
		case 1:
			ptp_msg_type = "SYNC";
			break;
		case 2:
			ptp_msg_type = "Follow_Up";
			break;
		case 3:
			ptp_msg_type = "Delay_Req";
			break;
		case 4:
			ptp_msg_type = "Delay_Resp";
			break;
		case 5:
			ptp_msg_type = "Pdelay_Req";
			break;
		case 6:
			ptp_msg_type = "Pdelay_Resp";
			break;
		case 7:
			ptp_msg_type = "Pdelay_Resp_Follow_up";
			break;
		case 8:
			ptp_msg_type = "Announce";
			break;
		case 9:
			ptp_msg_type = "Management";
			break;
		case 10:
			ptp_msg_type = "Signaling";
			break;
		case 11:
		case 12:
		case 13:
		case 14:
			ptp_msg_type = "Reserved";
			break;
		case 15:
			ptp_msg_type = "PTP pkr with Reserved Msg Type";
			break;
	}

	GMAC_PTP_DBG(("Rx timestamp detail for queue 0\n"
			"tstamp dropped    = %s\n"
			"tstamp available  = %s\n"
			"PTP version       = %s\n"
			"PTP Pkt Type      = %s\n"
			"PTP Msg Type      = %s\n",
			tstamp_dropped, tstamp_available,
			ptp_version, ptp_pkt_type, ptp_msg_type));

	GMAC_PTP_DBG(("<--stmmac_print_rx_tstamp_info\n"));
}

void stmmac_print_skb_tstamp(struct sk_buff *skb)
{
	s64 nsec;
	struct skb_shared_hwtstamps *shhwtstamp = NULL;

	if (gmac_msg_level & GMAC_LEVEL_PTP_DBG) {
		shhwtstamp = skb_hwtstamps(skb);
		nsec = shhwtstamp->hwtstamp.tv64;
		printk(KERN_ERR "[%s]nsec:%lld\n", __func__, nsec);
	}
}

/* stmmac_get_rx_hwtstamp: get HW RX timestamps
 * @priv: driver private structure
 * @entry : descriptor index to be used.
 * @skb : the socket buffer
 * Description :
 * This function will read received packet's timestamp from the descriptor
 * and pass it to stack. It also perform some sanity checks.
 */
static void stmmac_get_rx_hwtstamp(struct stmmac_priv *priv,
				  			struct sk_buff *skb)
{
	struct skb_shared_hwtstamps *shhwtstamp = NULL;
	u64 ns;
	union dma_desc *desc;
	struct stmmac_time_spec time;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, 0);	//only using channel 0
	unsigned int entry;
	unsigned int rollover_mode;

	if (!rx_queue->hwts_rx_en)
		return;

	entry = rx_queue->cur_rx % rx_queue->dma_rx_size;
	desc = (rx_queue->dma_rx + entry);
	rx_queue->rx_skbuff[entry] = NULL;

	if (gmac_msg_level & GMAC_LEVEL_DESC_DBG) {
		printk(KERN_ERR "\n[%s]entry:%u, ctx:%d\n", __func__,
			entry, desc->rx_desc.ctx.rx_ctx_desc);
		print_hex_dump(KERN_ERR, "rx_desc:", DUMP_PREFIX_ADDRESS,
		       16, 4, (void *)desc, sizeof(union dma_desc), false);
	}

	if (!(desc->rx_desc.ctx.rx_ctx_desc)) {
		return;
	}

	/* exit if rx tstamp is not valid */
	if (!priv->hw->desc->get_rx_timestamp_status(desc))
		return;

	/* get valid tstamp */
	priv->hw->desc->get_timestamp(desc, &time);
	rollover_mode = priv->hw->mac->get_ts_rollover(priv->ioaddr);
	if (rollover_mode) {
		ns = (time.sec * 1000000000ULL) + (u64)time.nsec;//one nanosecond
	} else {
		ns = (time.sec * 1000000000ULL) + div_u64(time.nsec * 61ULL, 131);//0.466
	}

	GMAC_PTP_DBG(("[%s]mode:%d, sec:%u, nsec:%u, ns:%llu\n", __func__,
			rollover_mode, time.sec, time.nsec, ns));

	shhwtstamp = skb_hwtstamps(skb);
	memset(shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp->hwtstamp = ns_to_ktime(ns);
}

/**
 *  stmmac_hwtstamp_ioctl - control hardware timestamping.
 *  @dev: device pointer.
 *  @ifr: An IOCTL specefic structure, that can contain a pointer to
 *  a proprietary structure used to pass information to the driver.
 *  Description:
 *  This function configures the MAC to enable/disable both outgoing(TX)
 *  and incoming(RX) packets time stamping based on user input.
 *  Return Value:
 *  0 on success and an appropriate -ve integer on failure.
 */
static int stmmac_hwtstamp_ioctl(struct net_device *dev, struct ifreq *ifr)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);
	struct hwtstamp_config config;
	struct timespec now;
	//u64 temp = 0;
	u32 ptp_v2 = 0;
	u32 tstamp_all = 0;
	u32 ptp_over_ipv4_udp = 0;
	u32 ptp_over_ipv6_udp = 0;
	u32 ptp_over_ethernet = 0;
	u32 snap_type_sel = 0;
	u32 ts_master_en = 0;
	u32 ts_event_en = 0;
	u32 value = 0;

	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, 0);

	memset(&config, 0, sizeof(struct hwtstamp_config));

	if (!(priv->dma_cap.time_stamp)) {
		netdev_alert(priv->dev, "No support for HW time stamping\n");
		tx_queue->hwts_tx_en = 0;
		rx_queue->hwts_rx_en = 0;

		return -EOPNOTSUPP;
	}

	if (copy_from_user(&config, ifr->ifr_data,
			   sizeof(struct hwtstamp_config)))
		return -EFAULT;

	GMAC_DBG(("%s config flags:0x%x, tx_type:0x%x, rx_filter:0x%x\n",
		 __func__, config.flags, config.tx_type, config.rx_filter));

	/* reserved for future extensions */
	if (config.flags)
		return -EINVAL;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
		tx_queue->hwts_tx_en = 0;
		break;
	case HWTSTAMP_TX_ON:
		tx_queue->hwts_tx_en = 1;
		break;
	default:
		return -ERANGE;
	}

	switch (config.rx_filter) {
		case HWTSTAMP_FILTER_NONE:
			/* time stamp no incoming packet at all */
			config.rx_filter = HWTSTAMP_FILTER_NONE;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
			/* PTP v1, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
			/* PTP v1, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_SYNC;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
			/* PTP v1, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
			/* PTP v2, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
			/* PTP v2, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
			/* PTP v2, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_EVENT:
			/* PTP v2/802.AS1 any layer, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_SYNC:
			/* PTP v2/802.AS1, any layer, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
			/* PTP v2/802.AS1, any layer, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_ALL:
			/* time stamp any incoming packet */
			config.rx_filter = HWTSTAMP_FILTER_ALL;
			tstamp_all = PTP_TCR_TSENALL;
			break;

		default:
			return -ERANGE;
	}

	rx_queue->hwts_rx_en = ((config.rx_filter == HWTSTAMP_FILTER_NONE) ? 0 : 1);

	if (!tx_queue->hwts_tx_en && !rx_queue->hwts_rx_en)
		priv->hw->ptp->config_hw_tstamping(priv->ioaddr, 0);
	else {
		value = (GMAC_TCR_TSENA | GMAC_TCR_TSCFUPDT | GMAC_TCR_TSCTRLSSR |
			 tstamp_all | ptp_v2 | ptp_over_ethernet |
			 ptp_over_ipv6_udp | ptp_over_ipv4_udp | ts_event_en |
			 ts_master_en | snap_type_sel);

		GMAC_PTP_DBG(("[%s]value:0x%x\n", __func__, value));
		priv->hw->ptp->config_hw_tstamping(priv->ioaddr, value);

		/* program Sub Second Increment reg */
		priv->hw->ptp->config_sub_second_increment(priv->ioaddr);

		/* calculate default added value:
		 * formula is :
		 * addend = (2^32)/freq_div_ratio;
		 * where, freq_div_ratio = STMMAC_SYSCLOCK/50MHz
		 * hence, addend = ((2^32) * 50MHz)/STMMAC_SYSCLOCK;
		 * NOTE: STMMAC_SYSCLOCK should be >= 50MHz to
		 *       achive 20ns accuracy.
		 *
		 * 2^x * y == (y << x), hence
		 * 2^32 * 50000000 ==> (50000000 << 32)
		 */
		//temp = (u64) (50000000ULL << 32);
		//priv->default_addend = div_u64(temp, STMMAC_SYSCLOCK);

		/* STMMAC_SYSCLOCK == 50M, so we use deafault value */
		priv->default_addend = ADDEND_DEFAULT;
		priv->hw->ptp->config_addend(priv->ioaddr, priv->default_addend);

		/* initialize system time */
		getnstimeofday(&now);
		priv->hw->ptp->init_systime(priv->ioaddr, now.tv_sec,
					    now.tv_nsec);
	}

	return copy_to_user(ifr->ifr_data, &config,
			    sizeof(struct hwtstamp_config)) ? -EFAULT : 0;
}

/**
 * stmmac_init_ptp: init PTP
 * @priv: driver private structure
 * Description: this is to verify if the HW supports the PTPv1 or v2.
 * This is done by looking at the HW cap. register.
 * Also it registers the ptp driver.
 */
static int stmmac_init_ptp(struct stmmac_priv *priv)
{
	int ret;
	int i;

	if (!(priv->dma_cap.time_stamp))
		return -EOPNOTSUPP;

	if (netif_msg_hw(priv)) {
		if (priv->dma_cap.time_stamp) {
			GMAC_DBG(("IEEE 1588-2002 Time Stamp supported\n"));
		}
	}

	priv->hw->ptp = &stmmac_ptp;
	priv->hw->slot = &stmmac_slot;

	for (i = 0; i < priv->tx_queue_cnt; i++) {
		priv->tx_queue[i].hwts_tx_en = 1;
		if (i > TX_CHN_NET) {
			priv->hw->slot->config_slot_compare(priv->ioaddr, i, 1);
			priv->hw->slot->config_slot_compare(priv->ioaddr, i, 1);
		}
	}

	//priv->hw->slot->config_slot_advance_check(priv->ioaddr, TX_CHN_CLASS_B, 1);

	priv->rx_queue[RX_CHN].hwts_rx_en = 1;
	priv->default_addend = ADDEND_DEFAULT;
	ret = priv->hw->ptp->init(priv->ioaddr);
	if (ret) {
		GMAC_ERR(("[%s]Init PTP failed!(error no:%d)\n", __func__, ret));
		return ret;
	}

	return stmmac_ptp_register(priv);
}

static void stmmac_release_ptp(struct stmmac_priv *priv)
{
	stmmac_ptp_unregister(priv);
}

static void stmmac_init_vlan(struct stmmac_priv *priv)
{
	int value;

	/* VLAN Tag Hash Table Match\ Double VLAN Processing Enable */
	//value = readl(priv->ioaddr + GMAC_VLAN_TAG);
	value = VLAN_TAG_VL_VID | VLAN_TAG_VL_CFI | VLAN_TAG_VL_PCP| VLAN_TAG_ETV;
	writel(value, priv->ioaddr + GMAC_VLAN_TAG);
}

int stmmac_link_state_transition(struct stmmac_priv *priv, int flush, unsigned int ctrl)
{
	unsigned int i, val, tx_coal_frames;
	int limit, state_tx, state_rx;
	const struct stmmac_ops *mac = priv->hw->mac;
	const struct stmmac_dma_ops *dma = priv->hw->dma;
	const struct stmmac_mtl_ops *mtl = priv->hw->mtl;
	struct gmac_tx_queue *tx_queue = NULL;

	/* Disable the Transmit DMA*/
	for (i = 0; i < priv->tx_queue_cnt; i++) {
		dma->stop_tx(priv->ioaddr, i);
	}

	/* Disable the MAC receiver */
	mac->dis_rx(priv->ioaddr);

	/* Flush Transmit Queue */
	if (flush) {
		for (i = 0; i < priv->tx_queue_cnt; i++) {
			mtl->flush_txq(priv->ioaddr, i);
		}
	}

	/* Wait for any previous frame transmissions to complete. */
	limit = 10000;
	while (limit--){
		state_tx = 0;
		state_rx = 0;

		/* check tx status*/
		for (i = 0; i < priv->tx_queue_cnt; i++) {
			val = mtl->tx_status(priv->ioaddr, i);
			if (val == MTL_TX_IDLE) {
				state_tx++;
			}
		}

		/* check rx status*/
		for (i = 0; i < priv->rx_queue_cnt; i++) {
			val = mtl->rx_status(priv->ioaddr, i);
			if (val == MTL_RX_IDLE) {
				state_rx++;
			}
		}
		if ((state_rx == priv->rx_queue_cnt) &&
			(state_tx == priv->tx_queue_cnt)) {
			break;
		}
	}
	if (limit < 0) {
		priv->xstats.poll_link_cfailed++;
		return -EBUSY;
	}

	mac->dis_tx(priv->ioaddr);

	/* set mac speed/port/duplex */
	mac->set_mac(priv->ioaddr, ctrl);

	for (i = 0; i < priv->tx_queue_cnt; i++) {
		dma->start_tx(priv->ioaddr, i);
		dma->clear_status(priv->ioaddr, i);
	}

	mac->enable_tx(priv->ioaddr);
	mac->enable_rx(priv->ioaddr);

	priv->old_speed = priv->line_speed;
	stmmac_get_line_speed(priv);

#ifdef CONFIG_AVB_NET
	if (priv->old_speed != priv->line_speed) {
		int bw;
		GMAC_ERR(("Link changed from %uMbps to %uMbps!\n", 
			priv->old_speed, priv->line_speed));
				
		for (i = TX_CHN_CLASS_B; i < priv->tx_queue_cnt; i++) {	
			bw = stmmac_cbs_get_bw(priv->ioaddr, i, priv->old_speed);
			stmmac_cbs_cfg_slope(priv->ioaddr, i, 0, priv->line_speed);		
			
			GMAC_ERR(("Chn%d's bw is changed from %dMbps to zero!\n", i, bw));
		}

		priv->xstats.bw_cleared++;
	}
#endif

	if (priv->line_speed == 1000) {
		tx_coal_frames = STMMAC_TX_FRAMES;
	} else {
		tx_coal_frames = (STMMAC_TX_FRAMES / 2);
	}

	tx_queue = GET_TX_QUEUE(priv, TX_CHN_NET);
	tx_queue->tx_coal_frames = tx_coal_frames;

#ifdef CONFIG_AVB_NET
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_B);
	tx_queue->tx_coal_frames = (tx_coal_frames / 2);

	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_A);
	tx_queue->tx_coal_frames = (tx_coal_frames / 2);
#endif

	return 0;
}

/**
 * stmmac_adjust_link
 * @dev: net device structure
 * Description: it adjusts the link parameters.
 */
static void stmmac_adjust_link(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	unsigned long flags;
	int new_state = 0;
	int chn, ret;
	unsigned int fc = priv->flow_ctrl, pause_time = priv->pause;

	if (phydev == NULL)
		return;

	spin_lock_irqsave(&priv->lock, flags);

	if (phydev->link) {
		u32 ctrl = readl(priv->ioaddr + GMAC_CONTROL);

		/* Now we make sure that we can be in full duplex mode.
		 * If not, we operate in half-duplex mode. */
		if (phydev->duplex != priv->oldduplex) {
			new_state = 1;
			if (!(phydev->duplex))
				ctrl &= ~priv->hw->link.duplex;
			else
				ctrl |= priv->hw->link.duplex;
			priv->oldduplex = phydev->duplex;
		}
		/* Flow Control operation */
		if (phydev->pause) {
			for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
				priv->hw->mac->flow_ctrl(priv->ioaddr, phydev->duplex,
						 fc, pause_time, chn);
			}
		}

		if (phydev->speed != priv->speed) {
			new_state = 1;
			switch (phydev->speed) {
			case 1000:
				if (likely(priv->plat->has_gmac))
					ctrl &= ~priv->hw->link.port;
				stmmac_hw_fix_mac_speed(priv);
				break;
			case 100:
			case 10:
				if (priv->plat->has_gmac) {
					ctrl |= priv->hw->link.port;
					if (phydev->speed == SPEED_100) {
						ctrl |= priv->hw->link.speed;
					} else {
						ctrl &= ~(priv->hw->link.speed);
					}
				} else {
					ctrl &= ~priv->hw->link.port;
				}
				stmmac_hw_fix_mac_speed(priv);
				break;
			default:
				if (netif_msg_link(priv))
					GMAC_WARNING(("%s: Speed (%d) not 10/100\n",
						dev->name, phydev->speed));
				break;
			}

			priv->speed = phydev->speed;
		}


		if (new_state) {
			ret = stmmac_link_state_transition(priv, 1, ctrl);
			if (ret) {
				printk(KERN_ERR"[%s]link state transition failed(%d)!\n",__func__, ret);
				spin_unlock_irqrestore(&priv->lock, flags);
				
				return;
			}

			priv->xstats.poll_link_changed++;
		}

		if (!priv->oldlink) {
			new_state = 1;
			priv->oldlink = 1;
		}
	} else if (priv->oldlink) {
		new_state = 1;
		priv->oldlink = 0;
		priv->speed = 0;
		priv->oldduplex = -1;
	}

	if (new_state && netif_msg_link(priv))
		phy_print_status(phydev);

	/* At this stage, it could be needed to setup the EEE or adjust some
	 * MAC related HW registers.
	 */
	priv->eee_enabled = stmmac_eee_init(priv);

	spin_unlock_irqrestore(&priv->lock, flags);

	//GMAC_DBG(( "stmmac_adjust_link: exiting\n"));
}

/**
 * stmmac_check_pcs_mode: verify if RGMII/SGMII is supported
 * @priv: driver private structure
 * Description: this is to verify if the HW supports the PCS.
 * Physical Coding Sublayer (PCS) interface that can be used when the MAC is
 * configured for the TBI, RTBI, or SGMII PHY interface.
 */
static void stmmac_check_pcs_mode(struct stmmac_priv *priv)
{
	int interface = priv->plat->interface;

	if (priv->dma_cap.pcs) {
		if ((interface & PHY_INTERFACE_MODE_RGMII) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_ID) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_RXID) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_TXID)) {
			GMAC_DBG(("STMMAC: PCS RGMII support enable\n"));
			priv->pcs = STMMAC_PCS_RGMII;
		} else if (interface & PHY_INTERFACE_MODE_SGMII) {
			GMAC_DBG(("STMMAC: PCS SGMII support enable\n"));
			priv->pcs = STMMAC_PCS_SGMII;
		}
	}
}

/**
 * stmmac_init_phy - PHY initialization
 * @dev: net device structure
 * Description: it initializes the driver's PHY state, and attaches the PHY
 * to the mac driver.
 *  Return value:
 *  0 on success
 */
static int stmmac_init_phy(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev;
	char phy_id_fmt[MII_BUS_ID_SIZE + 3];
	char bus_id[MII_BUS_ID_SIZE];
	int interface = priv->plat->interface;
	priv->oldlink = 0;
	priv->speed = 0;
	priv->oldduplex = -1;

	if (priv->plat->phy_bus_name)
		snprintf(bus_id, MII_BUS_ID_SIZE, "%s-%x",
			 priv->plat->phy_bus_name, priv->plat->bus_id);
	else
		snprintf(bus_id, MII_BUS_ID_SIZE, "stmmac-%x", priv->plat->bus_id);

	snprintf(phy_id_fmt, MII_BUS_ID_SIZE + 3, PHY_ID_FMT, bus_id,
		 priv->plat->phy_addr);

	GMAC_DBG(("stmmac_init_phy:  trying to attach to %s\n", phy_id_fmt));

	phydev = phy_connect(dev, phy_id_fmt, &stmmac_adjust_link, interface);
	if (IS_ERR(phydev)) {
		GMAC_ERR(("%s: Could not attach to PHY\n", dev->name));
		return PTR_ERR(phydev);
	}

	/* Stop Advertising 1000BASE Capability if interface is not GMII */
	if ((interface == PHY_INTERFACE_MODE_MII) ||
	    (interface == PHY_INTERFACE_MODE_RMII))
		phydev->advertising &= ~(SUPPORTED_1000baseT_Half |
					 SUPPORTED_1000baseT_Full);

	/*
	 * Broken HW is sometimes missing the pull-up resistor on the
	 * MDIO line, which results in reads to non-existent devices returning
	 * 0 rather than 0xffff. Catch this here and treat 0 as a non-existent
	 * device as well.
	 * Note: phydev->phy_id is the result of reading the UID PHY registers.
	 */
	if (phydev->phy_id == 0) {
		phy_disconnect(phydev);
		return -ENODEV;
	}
	GMAC_DBG(("stmmac_init_phy:  %s: attached to PHY (UID 0x%x)"
		 " Link = %d\n", dev->name, phydev->phy_id, phydev->link));

	priv->phydev = phydev;

	return 0;
}

/**
 * stmmac_display_ring: display ring
 * @head: pointer to the head of the ring passed.
 * @size: size of the ring.
 * Description: display the control/status and buffer descriptors.
 */
static void stmmac_display_ring(void *head, int size, unsigned int control)
{
	union dma_desc *p = (union dma_desc *)head;

	if (gmac_msg_level & control) {
		print_hex_dump(KERN_DEBUG, "gmac desc:", DUMP_PREFIX_OFFSET,
		       16, 4, (void *)p, size, false);
	}
}

static void stmmac_display_rings(struct stmmac_priv *priv)
{
	int chn;
	unsigned int txsize;
	unsigned int rxsize;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		rxsize = rx_queue->dma_rx_size;
		GMAC_RING_DBG(("RX chn[%d] descriptor ring:\n", chn));
		stmmac_display_ring((void *)rx_queue->dma_rx,
			rxsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);
	}

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		txsize = tx_queue->dma_tx_size;
		GMAC_RING_DBG(("TX chn[%d] descriptor ring:\n", chn));
		stmmac_display_ring((void *)(tx_queue->dma_tx),
			txsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);
	}
}

#ifndef CONFIG_BALONG_GMAC_LPM
/**
 * stmmac_clear_descriptors: clear descriptors
 * @priv: driver private structure
 * Description: this function is called to clear the tx and rx descriptors
 */
static void stmmac_clear_descriptors(struct stmmac_priv *priv)
{
	int i;
	int chn;
	struct gmac_tx_queue *tx_queue;
	struct gmac_rx_queue *rx_queue;

	/* Clear the Rx/Tx descriptors */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		for (i = 0; i < tx_queue->dma_tx_size; i++) {
			priv->hw->desc->init_tx_desc(tx_queue->dma_tx + i);	//ndesc_init_tx_desc
		}
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		for (i = 0; i < rx_queue->dma_rx_size; i++) {
			priv->hw->desc->init_rx_desc((rx_queue->dma_rx + i), !(rx_queue->use_riwt))
;	//ndesc_init_rx_desc
		}
	}
}
#endif

static int stmmac_init_rx_buffers(struct stmmac_priv *priv,
	union dma_desc *p, int i, int chn)
{
	struct sk_buff *skb;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);

	skb = __netdev_alloc_skb(priv->dev, priv->dma_buf_sz + NET_IP_ALIGN,
				 GFP_KERNEL);
		if (unlikely(skb == NULL)) {
		pr_err("%s: Rx init fails; skb is NULL\n", __func__);
		return -ENOMEM;
	}

	skb_reserve(skb, NET_IP_ALIGN);
	rx_queue->rx_skbuff[i] = skb;
	rx_queue->rx_skbuff_dma[i] = dma_map_single(priv->device, skb->data,
						priv->dma_buf_sz,
						DMA_FROM_DEVICE);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		spe_hook.set_skb_dma(skb, rx_queue->rx_skbuff_dma[i]);
	}
#endif

	/* Using buffer 1, no buffer 2*/
	p->rx_desc.nrd.buf1_phy_addr = rx_queue->rx_skbuff_dma[i];
	p->rx_desc.nrd.buf1_addr_valid = 1;

	return 0;
}

void stmmac_free_rx_buffers(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);

	for (i = 0; i < rx_queue->dma_rx_size; i++) {
		if (rx_queue->rx_skbuff_dma[i]) {
			dma_unmap_single(priv->device, rx_queue->rx_skbuff_dma[i],
						priv->dma_buf_sz,
						DMA_FROM_DEVICE);
		}

		if (rx_queue->rx_skbuff[i]) {
			dev_kfree_skb_any(rx_queue->rx_skbuff[i]);
			rx_queue->rx_skbuff[i] = NULL;
		}
	}
}


int gmac_alloc_queue_struct(struct stmmac_priv *priv)
{
	int ret = 0;

#ifdef CONFIG_AVB_NET
	priv->tx_queue_cnt = priv->dma_cap.num_tx_queue + 1;
	priv->rx_queue_cnt = priv->dma_cap.num_rx_queue + 1;
#else
	priv->tx_queue_cnt = 1;	//only one queue
	priv->rx_queue_cnt = 1;	//only one queue
#endif

	GMAC_DBG(("[%s]tx_queue_cnt:%d,rx_queue_cnt:%d\n", __func__,
						priv->tx_queue_cnt,	priv->rx_queue_cnt));

	priv->tx_queue =
		kzalloc(sizeof(struct gmac_tx_queue) * (priv->tx_queue_cnt),
		GFP_KERNEL);
	if (priv->tx_queue == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Tx queue structure\n"));
		ret = -ENOMEM;
	}

	priv->rx_queue =
		kzalloc(sizeof(struct gmac_rx_queue) * (priv->rx_queue_cnt),
		GFP_KERNEL);
	if (priv->rx_queue == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Rx queue structure\n"));
		ret = -ENOMEM;
		goto free_res;
	}

	GMAC_DBG(("[%s]\n", __func__));

	return 0;

free_res:
	kfree(priv->tx_queue);
	return ret;
}

void gmac_free_queue_struct(struct stmmac_priv *priv)
{
	kfree(priv->tx_queue);
	kfree(priv->rx_queue);
}

void gmac_free_mac(struct stmmac_priv *priv)
{
	kfree(priv->hw);
}

int gmac_alloc_rx_stat(struct stmmac_priv *priv)
{
	int ret = 0;

	priv->rx_stat =
		kzalloc(sizeof(struct gmac_rx_stat) * (priv->tx_queue_cnt),
		GFP_KERNEL);
	if (priv->rx_stat == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Rx stat\n"));
		ret = -ENOMEM;
	}
	return ret;
}

void gmac_free_rx_stat(struct stmmac_priv *priv)
{
	if (priv->rx_stat) {
		kfree(priv->rx_stat);
	}
}

void init_mtl_para(struct stmmac_priv *priv)
{
    /* Now, rx fifo size is 8K, tx fifo size is 8K, tx_size = 6, rx_size = 6
	 * tx0 fifo size:15(16*256bytes,4K),tx1 fifo size is 7(8*256,2k) tx2 is 2K
	 * rx fifo size:31(32 * 256 bytes, 8k)
	*/
#ifdef CONFIG_AVB_NET
	priv->tx_queue[TX_CHN_NET].mtl_fifo_size = 15;
	priv->tx_queue[TX_CHN_CLASS_B].mtl_fifo_size = 7;
	priv->tx_queue[TX_CHN_CLASS_A].mtl_fifo_size = 7;
#else
	priv->tx_queue[TX_CHN_NET].mtl_fifo_size = 31;
#endif

	priv->rx_queue[RX_CHN].mtl_fifo_size = 31;
}

/**
 * init_dma_desc_rings - init the RX/TX descriptor rings
 * @dev: net device structure
 * Description:  this function initializes the DMA RX/TX descriptors
 * and allocates the socket buffers. It suppors the chained and ring
 * modes.
 */
static int init_dma_desc_rings(struct net_device *dev)
{
	int i;
	int chn;
	int ret;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;
	unsigned int txsize;
	unsigned int rxsize;
	unsigned int bfsize = 0;
	unsigned int td_width, rd_width;

	/* Set the max buffer size according to the DESC mode
	 * and the MTU. Note that RING mode allows 16KiB bsize.
	 */
	bfsize = priv->hw->ring->set_16kib_bfsize(dev->mtu);
	td_width = sizeof(union dma_desc); //all tx desc have the same size:16 bytes
	rd_width = sizeof(union dma_desc); //all rx desc have the same size:16 bytes

	if (bfsize < BUF_SIZE_16KiB)
        bfsize = BUF_SIZE_1_8kiB;

	/* All of channels using the same buffer size. */
	priv->dma_buf_sz = bfsize;
	buf_sz = bfsize;
	GMAC_TRACE(("stmmac: bfsize %d\n", bfsize));

	/* Create and initialize the TX/RX descriptors chains. */
	priv->dma_buf_sz = STMMAC_ALIGN(buf_sz);

	/* Multi-channel Compatible */
	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {

		rx_queue = GET_RX_QUEUE(priv, chn);
		/* fixed me!:the channel should have its dma_rxsize */
		rx_queue->dma_rx_size = STMMAC_ALIGN(dma_rxsize);
		rxsize = rx_queue->dma_rx_size;

		rx_queue->dma_rx = dma_alloc_coherent(priv->device, rxsize *
					  rd_width, &(rx_queue->dma_rx_phy), GFP_KERNEL);
		if (!(rx_queue->dma_rx)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}

		/* Using one rx buffer in a descriptor. */
		rx_queue->rx_skbuff_dma = kmalloc_array(rxsize, sizeof(dma_addr_t),
					    GFP_KERNEL);
		if (!(rx_queue->rx_skbuff_dma)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}

		rx_queue->rx_skbuff = kmalloc_array(rxsize, sizeof(struct sk_buff *),
						GFP_KERNEL);
		if (!(rx_queue->rx_skbuff)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}

		if (netif_msg_drv(priv)) {
			GMAC_DBG(("(%s) dma_rx_phy[%d]=0x%08x\n", __func__, chn,
			 (unsigned int)(rx_queue->dma_rx_phy)));
		}

		for (i = 0; i < rxsize; i++) {
			union dma_desc *p;
			p = rx_queue->dma_rx + i;
			memset((void *)p, 0, sizeof(union dma_desc));

			ret = stmmac_init_rx_buffers(priv, p, i, chn);
			if (ret) {
				goto rx_queue_free;
			}

			/* Clear the Rx descriptors */
			priv->hw->desc->init_rx_desc((rx_queue->dma_rx + i), !(rx_queue->use_riwt));
		}

		rx_queue->cur_rx = 0;
		rx_queue->dirty_rx = (unsigned int)(i - rxsize);
	}

	/* Multi-channel compatible and using one tx buffer in a descriptor. */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);

		/* the channel should have its dma_txsize */
		tx_queue->dma_tx_size = STMMAC_ALIGN(dma_txsize);
		txsize = tx_queue->dma_tx_size;

		tx_queue->dma_tx = dma_alloc_coherent(priv->device, txsize *
					  td_width, &(tx_queue->dma_tx_phy), GFP_KERNEL);
		if (!(tx_queue->dma_tx)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}

		/* Initialising taril pointer the same as tx_phy */
		tx_queue->dma_tx_tail = tx_queue->dma_tx_phy;
		tx_queue->tx_skbuff_dma = kzalloc((txsize * sizeof(dma_addr_t)), GFP_KERNEL);
		if (!(tx_queue->tx_skbuff_dma)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}

		tx_queue->tx_skbuff = kzalloc((txsize * sizeof(struct sk_buff *)), GFP_KERNEL);
		if (!(tx_queue->tx_skbuff)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}
		memset((void *)tx_queue->dma_tx, 0, (txsize * td_width));

		if (netif_msg_drv(priv)) {
			GMAC_DBG(("(%s) dma_tx_phy[%d]=0x%08x\n", __func__, chn,
			 (unsigned int)(tx_queue->dma_tx_phy)));
		}

		for (i = 0; i < txsize; i++) {
			priv->hw->desc->init_tx_desc(tx_queue->dma_tx + i);
		}

		tx_queue->dirty_tx = 0;
		tx_queue->cur_tx = 0;
	}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		skb_queue_head_init(&priv->free_q);
	}
#endif

	if (netif_msg_hw(priv))
		stmmac_display_rings(priv);

	return 0;

tx_queue_free:
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		if (tx_queue->dma_tx) {
			dma_free_coherent(priv->device, txsize *
					  td_width, tx_queue->dma_tx, tx_queue->dma_tx_phy);
		}

		if (tx_queue->tx_skbuff_dma) {
			kfree(tx_queue->tx_skbuff_dma);
		}

		if (tx_queue->tx_skbuff) {
			kfree(tx_queue->tx_skbuff);
		}
	}

rx_queue_free:
	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		if (rx_queue->dma_rx) {
			dma_free_coherent(priv->device, rx_queue->dma_rx_size *
					  rd_width, rx_queue->dma_rx, rx_queue->dma_rx_phy);
		}

		if (rx_queue->rx_skbuff_dma) {
			kfree(rx_queue->rx_skbuff_dma);
		}

		if (rx_queue->rx_skbuff) {
			kfree(rx_queue->rx_skbuff);
		}

		stmmac_free_rx_buffers(priv, chn);
	}

	return ret;
}

static void dma_free_rx_skbufs(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);

	for (i = 0; i < rx_queue->dma_rx_size; i++) {
		if (rx_queue->rx_skbuff[i]) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
			if (!rx_queue->rx_skbuff[i]->spe_own)
#endif
			{
				dma_unmap_single(priv->device, rx_queue->rx_skbuff_dma[i],
					priv->dma_buf_sz, DMA_FROM_DEVICE);
				dev_kfree_skb_any(rx_queue->rx_skbuff[i]);
			}
		}
		rx_queue->rx_skbuff[i] = NULL;
	}
}

static void dma_free_tx_skbufs(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);

	for (i = 0; i < tx_queue->dma_tx_size; i++) {
		if (tx_queue->tx_skbuff[i] != NULL) {
			union dma_desc *p;
			p = tx_queue->dma_tx + i;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
			if (tx_queue->tx_skbuff[i]->spe_own) {
				if (tx_queue->tx_skbuff_dma[i]) {
					spe_hook.rd_config(tx_queue->tx_skbuff[i]->spe_own,
						tx_queue->tx_skbuff[i], tx_queue->tx_skbuff_dma[i]);
				}
			} else
#endif
			{
				if (tx_queue->tx_skbuff_dma[i])
					dma_unmap_single(priv->device, tx_queue->tx_skbuff_dma[i],
						priv->hw->desc->get_tx_len(p), DMA_TO_DEVICE);
				
				dev_kfree_skb_any(tx_queue->tx_skbuff[i]);
			}
			tx_queue->tx_skbuff[i] = NULL;
			tx_queue->tx_skbuff_dma[i] = 0;
		}
	}
}

static void free_dma_desc_resources(struct stmmac_priv *priv)
{
	int chn;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {

		/* Release the DMA TX/RX socket buffers */
		dma_free_rx_skbufs(priv, chn);

		/* Free DMA regions of consistent memory previously allocated */
		rx_queue = GET_RX_QUEUE(priv, chn);
		dma_free_coherent(priv->device,
			rx_queue->dma_rx_size * sizeof(union dma_desc),
			rx_queue->dma_rx, rx_queue->dma_rx_phy);

		kfree(rx_queue->rx_skbuff_dma);
		kfree(rx_queue->rx_skbuff);
	}

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		dma_free_tx_skbufs(priv, chn);
		tx_queue = GET_TX_QUEUE(priv, chn);
		dma_free_coherent(priv->device,
			tx_queue->dma_tx_size * sizeof(union dma_desc),
			tx_queue->dma_tx, tx_queue->dma_tx_phy);

		kfree(tx_queue->tx_skbuff_dma);
		kfree(tx_queue->tx_skbuff);
	}

}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
static int
stmmac_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags)
{
	struct net_device *dev;

	BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);

	if (!spe_hook.port_is_enable(portno)) {
		dev->stats.tx_dropped++;
		return NETDEV_TX_BUSY;
	}

	return stmmac_spe_xmit(skb, dev);
}

static void stmmac_reset_rx_skb_spe(struct sk_buff *skb)
{
	skb_reset_tail_pointer(skb);
	skb->len = 0;
	skb->cloned = 0;
}

static int
stmmac_finish_td(int portno, struct sk_buff *skb, unsigned int flags)
{
	int ret = 0;
	struct net_device *dev;
	struct stmmac_priv *priv;

	BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);
	priv = netdev_priv(dev);

	stmmac_reset_rx_skb_spe(skb);
	skb_queue_tail(&priv->free_q, skb);
	stmmac_rx_refill(priv);

	return ret;
}
#endif

/**
 * stmmac_tx_clean:
 * @priv: driver private structure
 * Description: it reclaims resources after transmission completes.
 */
void stmmac_tx_clean(struct stmmac_priv *priv)
{
	unsigned int txsize;
	unsigned long flags[TX_CHN_NR];
	int chn;
	struct gmac_tx_queue *tx_queue = NULL;

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		txsize = tx_queue->dma_tx_size;
		spin_lock_irqsave(&tx_queue->tx_lock, flags[chn]);

		priv->xstats.tx_clean[chn]++;

		while (tx_queue->dirty_tx != tx_queue->cur_tx) {
			int last;
			unsigned int entry = tx_queue->dirty_tx % txsize;
			struct sk_buff *skb = tx_queue->tx_skbuff[entry];
			union dma_desc *p;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
			dma_addr_t dma;
#endif

			p = tx_queue->dma_tx + entry;

			/* Check if the descriptor is owned by the DMA. */
			if (priv->hw->desc->get_tx_owner(p)) {
				priv->xstats.tx_clean_own[chn]++;
				break;
			}

			/* Verify tx error by looking at the last segment. */
			last = priv->hw->desc->get_tx_ls(p);
			if (likely(last)) {
				int tx_error =
				    priv->hw->desc->tx_status(&priv->dev->stats,
							      &priv->xstats, p,
							      priv->ioaddr, chn);
				if (likely(tx_error == 0)) {
					priv->dev->stats.tx_packets++;
					priv->xstats.tx_pkt_n[chn]++;
				} else
					priv->dev->stats.tx_errors++;

				if (skb) {
					stmmac_get_tx_hwtstamp(priv, entry, skb);
				}
			}

			GMAC_TX_DBG(("%s: chn[%d], curr %d, dirty %d\n", __func__, chn,
			       tx_queue->cur_tx, tx_queue->dirty_tx));

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
			if (skb && skb->spe_own) {
				tx_queue->tx_skbuff_dma[entry] = 0;
			}else
#endif
			{
				if (likely(tx_queue->tx_skbuff_dma[entry])) {
					dma_unmap_single(priv->device, tx_queue->tx_skbuff_dma[entry],
						priv->hw->desc->get_tx_len(p), DMA_TO_DEVICE);
					tx_queue->tx_skbuff_dma[entry] = 0;
				}
			}

			/*clean the descriptor */
			if (likely(skb != NULL)) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
				if((spe_hook.is_enable && spe_hook.is_enable()) && 
					(skb->spe_own) &&(spe_mode_normal==spe_hook.mode())){
					dma = spe_hook.get_skb_dma(skb);
					spe_hook.rd_config(skb->spe_own, skb, dma);
	            }else
#endif
				{
					dev_kfree_skb_any(skb);
				}
				tx_queue->tx_skbuff[entry] = NULL;
			}

			priv->hw->desc->release_tx_desc(p);
			tx_queue->dirty_tx++;
		}

		if (unlikely(__netif_subqueue_stopped(priv->dev, chn) && (
			stmmac_tx_avail(priv, chn) > STMMAC_TX_THRESH(priv, chn)))) {
				netif_tx_lock(priv->dev);

				GMAC_TX_DBG(("%s: restart transmit\n", __func__));
				netif_wake_subqueue(priv->dev, chn);

				netif_tx_unlock(priv->dev);
			}

		if ((priv->eee_enabled) && (!tx_queue->tx_path_in_lpi_mode)) {
			stmmac_enable_eee_mode(priv, chn);
			mod_timer(&priv->eee_ctrl_timer, STMMAC_LPI_T(eee_timer));
		}
		spin_unlock_irqrestore(&tx_queue->tx_lock, flags[chn]);
	}
}

static inline void stmmac_enable_dma_irq(struct stmmac_priv *priv)
{
	int chn;

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->enable_dma_irq(priv->ioaddr, chn);
	}
}

static inline void stmmac_disable_dma_irq(struct stmmac_priv *priv)
{
	int chn;

	/* disable all of the channels */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->disable_dma_irq(priv->ioaddr, chn);
	}
}

/**
 * stmmac_tx_err: irq tx error mng function
 * @priv: driver private structure
 * Description: it cleans the descriptors and restarts the transmission
 * in case of errors.
 */
static void stmmac_tx_err(struct stmmac_priv *priv, int chn)
{
	priv->dev->stats.tx_errors++;
	netif_wake_subqueue(priv->dev, chn);
}

/**
 * stmmac_dma_interrupt: DMA ISR
 * @priv: driver private structure
 * Description: this is the DMA ISR. It is called by the main ISR.
 * It calls the dwmac dma routine to understand which type of interrupt
 * happened. In case of there is a Normal interrupt and either TX or RX
 * interrupt happened so the NAPI is scheduled.
 */
static void stmmac_dma_interrupt(struct stmmac_priv *priv, int chn)
{
	int status;

	status = priv->hw->dma->dma_interrupt(priv->ioaddr, &priv->xstats, chn);
	if (likely((status & handle_rx)) || (status & handle_tx)) {
		if (likely(napi_schedule_prep(&priv->napi))) {
			stmmac_disable_dma_irq(priv);
			__napi_schedule(&priv->napi);
		}
	}

	if (unlikely(status == tx_hard_error))
		stmmac_tx_err(priv, chn);

}

/**
 * stmmac_mmc_setup: setup the Mac Management Counters (MMC)
 * @priv: driver private structure
 * Description: this masks the MMC irq, in fact, the counters are managed in SW.
 */
static void stmmac_mmc_setup(struct stmmac_priv *priv)
{
	unsigned int mode = /*MMC_CNTRL_RESET_ON_READ | */MMC_CNTRL_COUNTER_RESET |
			    MMC_CNTRL_PRESET | MMC_CNTRL_FULL_HALF_PRESET;

	dwmac_mmc_intr_all_mask(priv->ioaddr);

	if (priv->dma_cap.rmon) {
		dwmac_mmc_ctrl(priv->ioaddr, mode);
		memset(&priv->mmc, 0, sizeof(struct stmmac_counters));
	} else
		GMAC_INFO(("No MAC Management Counters available\n"));
}

static u32 stmmac_get_synopsys_id(struct stmmac_priv *priv)
{
	u32 hwid = priv->hw->synopsys_uid;

	/* Check Synopsys Id (not available on old chips) */
	if (likely(hwid)) {
		u32 uid = ((hwid & 0x0000ff00) >> 8);
		u32 synid = (hwid & 0x000000ff);

		GMAC_INFO(("stmmac - user ID: 0x%x, Synopsys ID: 0x%x\n",
			uid, synid));

		return synid;
	}
	return 0;
}

/**
 * stmmac_get_hw_features: get MAC capabilities from the HW cap. register.
 * @priv: driver private structure
 * Description:
 *  new GMAC chip generations have a new register to indicate the
 *  presence of the optional feature/functions.
 *  This can be also used to override the value passed through the
 *  platform and necessary for old MAC10/100 and GMAC chips.
 */
static int stmmac_get_hw_features(struct stmmac_priv *priv,
							unsigned int *hw_cap1, unsigned int *hw_cap2)
{
	u32 hw_cap0 = 0;

	if (priv->hw->dma->get_hw_feature) {
		hw_cap0 = priv->hw->dma->get_hw_feature(priv->ioaddr, hw_cap1, hw_cap2);

		priv->dma_cap.mbps_10_100 = (hw_cap0 & DMA_HW_FEAT_MIISEL);
		priv->dma_cap.mbps_1000 = (hw_cap0 & DMA_HW_FEAT_GMIISEL) >> 1;
		priv->dma_cap.half_duplex = (hw_cap0 & DMA_HW_FEAT_HDSEL) >> 2;
		priv->dma_cap.pcs = (hw_cap0 & DMA_HW_FEAT_PCSSEL) >> 3;
		priv->dma_cap.vlan_hash_filter = (hw_cap0 & DMA_HW_FEAT_VLHASH) >> 4;
		priv->dma_cap.sma_mdio = (hw_cap0 & DMA_HW_FEAT_SMASEL) >> 5;
		priv->dma_cap.pmt_remote_wake_up = (hw_cap0 & DMA_HW_FEAT_RWKSEL) >> 6;
		priv->dma_cap.pmt_magic_frame = (hw_cap0 & DMA_HW_FEAT_MGKSEL) >> 7;

		/* MMC */
		priv->dma_cap.rmon            = (hw_cap0 & DMA_HW_FEAT_MMCSEL) >> 8;

		/*ARP offload*/
		priv->dma_cap.arp_offload     = (hw_cap0 & DMA_HW_FEAT_ARPOFFSEL) >> 9;

		/* IEEE 1588-2008 */
		priv->dma_cap.time_stamp      = (hw_cap0 & DMA_HW_FEAT_TSSEL) >> 12;

		/* 802.3az - Energy-Efficient Ethernet (EEE) */
		priv->dma_cap.eee             = (hw_cap0 & DMA_HW_FEAT_EEESEL) >> 13;

		/* TX and RX csum */
		priv->dma_cap.tx_coe          = (hw_cap0 & DMA_HW_FEAT_TXCOESEL) >> 14;
		priv->dma_cap.rx_coe          = (hw_cap0 & DMA_HW_FEAT_RXCOESEL) >> 16;
		priv->dma_cap.multi_addr      = (hw_cap0 & DMA_HW_FEAT_ADDMACADRSEL) >> 18;
		priv->dma_cap.multi_addr32    = (hw_cap0 & DMA_HW_FEAT_MACADR32SEL) >> 23;
		priv->dma_cap.multi_addr64    = (hw_cap0 & DMA_HW_FEAT_MACADR64SEL) >> 24;

		/*Timestamp system time source*/
		priv->dma_cap.systime_source  = (hw_cap0 & DMA_HW_FEAT_TSSTSSEL) >> 25;
		priv->dma_cap.sa_vlan_ins     = (hw_cap0 & DMA_HW_FEAT_SAVLANINS) >> 27;
		priv->dma_cap.phy_mode        = (hw_cap0 & DMA_HW_FEAT_ACTPHYSEL) >> 28;


		priv->dma_cap.rx_fifo_size    = ((*hw_cap1) & DMA_HW_FEAT_RXFIFOSIZE);
		priv->dma_cap.tx_fifo_size    = ((*hw_cap1) & DMA_HW_FEAT_TXFIFOSIZE) >> 6;
		priv->dma_cap.one_step_ts     = ((*hw_cap1) & DMA_HW_FEAT_OSTEN) >> 11;
		priv->dma_cap.ptp_offload     = ((*hw_cap1) & DMA_HW_FEAT_PTOEN) >> 12;
		priv->dma_cap.high_word_reg   = ((*hw_cap1) & DMA_HW_FEAT_ADVTHWORD) >> 13;
		priv->dma_cap.addr_width      = ((*hw_cap1) & DMA_HW_FEAT_ADDR64) >> 14;
		priv->dma_cap.dcb_enable      = ((*hw_cap1) & DMA_HW_FEAT_DCBEN) >> 16;
		priv->dma_cap.split_hdr       = ((*hw_cap1) & DMA_HW_FEAT_SPHEN) >> 17;
		priv->dma_cap.tso_en          = ((*hw_cap1) & DMA_HW_FEAT_TSOEN) >> 18;
		priv->dma_cap.debug_mem_if    = ((*hw_cap1) & DMA_HW_FEAT_DBGMEMA) >> 19;
		priv->dma_cap.av_en 		  = ((*hw_cap1) & DMA_HW_FEAT_AVSEL) >> 20;
		priv->dma_cap.hash_table_size = ((*hw_cap1) & DMA_HW_FEAT_HASHTBLSZ) >> 24;
		priv->dma_cap.l3l4_total_num  = ((*hw_cap1) & DMA_HW_FEAT_L3L4FNUM) >> 27;

		priv->dma_cap.num_rx_queue   = ((*hw_cap2) & DMA_HW_FEAT_RXQCNT);
		priv->dma_cap.num_tx_queue   = ((*hw_cap2) & DMA_HW_FEAT_TXQCNT) >> 6;
		priv->dma_cap.num_rx_channel = ((*hw_cap2) & DMA_HW_FEAT_RXCHCNT) >> 12;
		priv->dma_cap.num_tx_channel = ((*hw_cap2) & DMA_HW_FEAT_TXCHCNT) >> 18;
		priv->dma_cap.num_pps_output = ((*hw_cap2) & DMA_HW_FEAT_PPSOUTNUM) >> 24;
		priv->dma_cap.num_aux_snap 	 = ((*hw_cap2) & DMA_HW_FEAT_AUXSNAPNUM) >> 28;
	}

	return hw_cap0;
}

/**
 * stmmac_check_ether_addr: check if the MAC addr is valid
 * @priv: driver private structure
 * Description:
 * it is to verify if the MAC address is valid, in case of failures it
 * generates a random MAC address
 */
static void stmmac_check_ether_addr(struct stmmac_priv *priv)
{
	/* check dev_addr if valid*/
	if (!is_valid_ether_addr(priv->dev->dev_addr)) {

		/* get the 0th mac address form register, and get it to dev_addr */
		priv->hw->mac->get_umac_addr((void __iomem *)priv->dev->base_addr,
			priv->dev->dev_addr, 0);

		/* check the dev_address if valid, or using random mac address */
		if (!is_valid_ether_addr(priv->dev->dev_addr))
			eth_hw_addr_random(priv->dev);
	}
	GMAC_WARNING(("%s: device MAC address %pM\n", priv->dev->name,
		priv->dev->dev_addr));
}

/**
 * stmmac_init_dma_engine: DMA init.
 * @priv: driver private structure
 * Description:
 * It inits the DMA invoking the specific MAC/GMAC callback.
 * Some DMA parameters can be passed from the platform;
 * in case of these are not passed a default is kept for the MAC or GMAC.
 */
static int stmmac_init_dma_engine(struct stmmac_priv *priv)
{
	int fixed_burst = 0, burst_len = 0;
	int mixed_burst = 0;

	if (priv->plat->dma_cfg) {
		fixed_burst = priv->plat->dma_cfg->fixed_burst;
		mixed_burst = priv->plat->dma_cfg->mixed_burst;
		burst_len = priv->plat->dma_cfg->burst_len;
	}

	return priv->hw->dma->init(fixed_burst, mixed_burst, burst_len);
}

/**
 * stmmac_tx_timer: mitigation sw timer for tx.
 * @data: data pointer
 * Description:
 * This is the timer handler to directly invoke the stmmac_tx_clean.
 */
static void stmmac_tx_timer(unsigned long data)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)data;

	stmmac_tx_clean(priv);
}

/**
 * stmmac_init_tx_coalesce: init tx mitigation options.
 * @priv: driver private structure
 * Description:
 * This inits the transmit coalesce parameters: i.e. timer rate,
 * timer handler and default threshold used for enabling the
 * interrupt on completion bit.
 */
static void stmmac_init_tx_coalesce(struct stmmac_priv *priv)
{
	int chn;
	struct gmac_tx_queue *tx_queue = NULL;

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		tx_queue->tx_coal_frames = STMMAC_TX_FRAMES;
		tx_queue->tx_coal_timer = STMMAC_COAL_TX_TIMER;
		init_timer(&(tx_queue->txtimer));
		tx_queue->txtimer.expires = STMMAC_COAL_TIMER(tx_queue->tx_coal_timer);
		tx_queue->txtimer.data = (unsigned long)priv;
		tx_queue->txtimer.function = stmmac_tx_timer;
		add_timer(&(tx_queue->txtimer));
	}
}

void stmmac_get_line_speed(struct stmmac_priv *priv)
{
	unsigned int line_status;

	line_status = priv->hw->mac->get_line_spped(priv->ioaddr);

	GMAC_INFO(("%s: line_status:%d\n", __func__, line_status));
	switch (line_status) {
		case STMMAC_LINE_SPPED_1000M:
		case STMMAC_LINE_SPPED_1000M_FES:
			priv->line_speed = 1000;
			break;

		case STMMAC_LINE_SPPED_100M:
			priv->line_speed = 100;
			break;

		case STMMAC_LINE_SPPED_10M:
			priv->line_speed = 10;
			break;

		default:
			priv->line_speed = 0;
			break;
	}
}

#if (FEATURE_ON == MBB_FEATURE_ETH)
static void stmmac_link_init(struct net_device *dev)
{
    struct stmmac_priv *priv = NULL;
    struct phy_device *phydev = NULL;
    unsigned int fc = 0;
    unsigned int pause_time = 0;

    priv = netdev_priv(dev);
    if (NULL == priv || NULL == priv->hw)
    {
        printk(KERN_ERR "stmmac_link_init priv is NULL");
        return;
    }
    
    fc = priv->flow_ctrl;
    pause_time = priv->pause;

    phydev = priv->phydev;
    if (NULL == phydev)
    {
        printk(KERN_ERR "stmmac_link_init phydev is NULL");
        return;
    }

    if (0 != phydev->link)
    {
        u32 ctrl = readl(priv->ioaddr + GMAC_CONTROL);

        if (!(phydev->duplex))
        {
            ctrl &= ~priv->hw->link.duplex;
        }
        else
        {
            ctrl |= priv->hw->link.duplex;
        }
        priv->oldduplex = phydev->duplex;

        /* Flow Control operation */
        if ((0 != phydev->pause)
            && (NULL != priv->hw->mac)
            && (NULL != priv->hw->mac->flow_ctrl))
        {
            priv->hw->mac->flow_ctrl(priv->ioaddr, phydev->duplex,fc, pause_time, 0);
        }

        switch (phydev->speed)
        {
            case SPEED_1000:
            {
                if ((NULL != priv->plat) && (likely(priv->plat->has_gmac)))
                {
                    ctrl &= ~priv->hw->link.port;
                }
                stmmac_hw_fix_mac_speed(priv);
                break;
            }
            case SPEED_100:
            case SPEED_10:
            {
                if (0 != priv->plat->has_gmac)
                {
                    ctrl |= priv->hw->link.port;
                    if (SPEED_100 == phydev->speed)
                    {
                        ctrl |= priv->hw->link.speed;
                    }
                    else
                    {
                        ctrl &= ~(priv->hw->link.speed);
                    }
                }
                else
                {
                    ctrl &= ~priv->hw->link.port;
                }
                stmmac_hw_fix_mac_speed(priv);
                break;
            }
            default:
            {
                break;
            }
        }

        priv->speed = phydev->speed;
        writel(ctrl, priv->ioaddr + GMAC_CONTROL);
        priv->oldlink = 1;
    }
}
#endif


/**
 *  stmmac_open - open entry point of the driver
 *  @dev : pointer to the device structure.
 *  Description:
 *  This function is the open entry point of the driver.
 *  Return value:
 *  0 on success and an appropriate (-)ve integer as defined in errno.h
 *  file on failure.
 */
static int stmmac_open(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret;
	int chn;
	struct gmac_rx_queue *rx_queue;

#if defined(BSP_CONFIG_BOARD_TELEMATIC)
    ret = stmmac_clk_enable(priv->device);
    if (ret) {
        goto close_clk;
    }
#endif

	/* 获取mac地址并检查 */
	stmmac_check_ether_addr(priv);

	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI) {
		ret = stmmac_init_phy(dev);
		if (ret) {
			GMAC_ERR(("%s: Cannot attach to PHY (error: %d)\n",
			       __func__, ret));
			goto open_error;
		}
	}

	ret = init_dma_desc_rings(dev);
	if (ret) {
		goto open_error;
	}

	init_mtl_para(priv);

	/* DMA initialization and SW reset */
	ret = stmmac_init_dma_engine(priv);
	if (ret < 0) {
		GMAC_ERR(("%s: DMA initialization failed\n", __func__));
		goto open_error;
	}

	/* If required, perform hw setup of the bus. */
	if (priv->plat->bus_setup)
		priv->plat->bus_setup(priv->ioaddr);

	/*Initialize the MTL */
	priv->hw->mtl->mtl_init();

	/* Initialize the MAC Core */
	ret = priv->hw->mac->core_init(priv->ioaddr);
	if (ret) {
		goto open_error;
	}

	/* Copy the MAC addr into the HW  */
	priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);

	/* Enable SPE port */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_enable(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
		(void)spe_hook.port_enable(priv->portno_wan);
#endif
	}
#endif

	/* Request the IRQ lines */
	ret = request_irq(dev->irq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, dev->irq, ret));
		goto open_error;
	}

	/* Request the Tx IRQ lines */
	ret = request_irq(priv->ch0_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch0_txirq, ret));
		goto open_error_ch0_irq;
	}

#ifdef CONFIG_AVB_NET
	ret = request_irq(priv->ch1_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch1_txirq, ret));
		goto open_error_ch1_irq;
	}

	ret = request_irq(priv->ch2_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch2_txirq, ret));
		goto open_error_ch2_irq;
	}
#endif

	/* receive interrupt */
	ret = request_irq(priv->rx_irq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->rx_irq, ret));
		goto open_error_rx_irq;
	}

	/* Request the Wake IRQ in case of another line is used for WoL */
	if (priv->wol_irq != dev->irq) {
		ret = request_irq(priv->wol_irq, stmmac_interrupt,
				  IRQF_SHARED, dev->name, dev);
		if (unlikely(ret < 0)) {
			GMAC_ERR(("%s: ERROR: allocating the WoL IRQ %d (%d)\n",
			       __func__, priv->wol_irq, ret));
			goto open_error_wolirq;
		}
	}

	/* Request the IRQ lines */
	if (priv->lpi_irq != -ENXIO) {
		ret = request_irq(priv->lpi_irq, stmmac_interrupt, IRQF_SHARED,
				  dev->name, dev);
		if (unlikely(ret < 0)) {
			GMAC_ERR(("%s: ERROR: allocating the LPI IRQ %d (%d)\n",
			       __func__, priv->lpi_irq, ret));
			goto open_error_lpiirq;
		}
	}

	/* Enable the MAC Rx/Tx */
	stmmac_set_mac(priv->ioaddr, true);

	/* Extra statistics */
	memset(&priv->xstats, 0, sizeof(struct stmmac_extra_stats));
	priv->xstats.threshold = tc;

#ifdef CONFIG_AVB_NET
	/* Support avb feature in default. */
	priv->avb_support = 1;
#endif
	gmac_status = &priv->xstats;

	stmmac_get_line_speed(priv);

	/* mmc_setup*/
	stmmac_mmc_setup(priv);

	ret = stmmac_init_ptp(priv);
	if (ret)
		GMAC_WARNING(("%s: failed PTP initialisation\n", __func__));

	stmmac_init_vlan(priv);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
	ret = stmmac_init_fs(dev);
	if (ret < 0)
		GMAC_WARNING(("%s: failed debugFS registration\n", __func__));
#endif

	GMAC_DBG(( "%s: DMA RX/TX processes started...\n", dev->name));
	for (chn = 0;chn < priv->tx_queue_cnt; chn++)
		priv->hw->dma->start_tx(priv->ioaddr, chn);

	for (chn = 0;chn < priv->tx_queue_cnt; chn++)
		priv->hw->dma->start_rx(priv->ioaddr, chn);

	/* Dump DMA/MAC registers */
	if (netif_msg_hw(priv)) {
		priv->hw->mac->dump_regs(priv->ioaddr);
		priv->hw->dma->dump_regs(priv->ioaddr);
	}
#if (FEATURE_ON == MBB_FEATURE_ETH)
    stmmac_link_init(dev);
#endif

	if (priv->phydev)
		phy_start(priv->phydev);

	priv->tx_lpi_timer = STMMAC_DEFAULT_TWT_LS;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		if ((rx_queue->use_riwt) && (priv->hw->dma->rx_watchdog)) {
			rx_queue->rx_riwt = riwt_value;
			priv->hw->dma->rx_watchdog(priv->ioaddr, riwt_value, chn);
		}
	}

	napi_enable(&priv->napi);
	netif_tx_start_all_queues(dev);
	priv->is_open = 1;
#if ((FEATURE_ON == MBB_DEVBOOTSTATE) && (FEATURE_ON == MBB_FACTORY))
    schedule_work(&mii_flag_work);
#endif
	return 0;

open_error_lpiirq:
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);

open_error_wolirq:
	free_irq(dev->irq, dev);

open_error_rx_irq:
	free_irq(priv->rx_irq, dev);

#ifdef CONFIG_AVB_NET
open_error_ch2_irq:
	free_irq(priv->ch2_txirq, dev);

open_error_ch1_irq:
	free_irq(priv->ch1_txirq, dev);
#endif

open_error_ch0_irq:
	free_irq(priv->ch0_txirq, dev);

open_error:
	if (priv->phydev)
		phy_disconnect(priv->phydev);

#if defined(BSP_CONFIG_BOARD_TELEMATIC)
close_clk:
#endif
	stmmac_clk_disable(priv->device);

	return ret;
}

/**
 *  stmmac_release - close entry point of the driver
 *  @dev : device pointer.
 *  Description:
 *  This is the stop entry point of the driver.
 */
static int stmmac_release(struct net_device *dev)
{
	int chn;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_tx_queue *tx_queue;

	if (priv->eee_enabled)
		del_timer_sync(&priv->eee_ctrl_timer);


#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_disable(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
		(void)spe_hook.port_enable(priv->portno_wan);
#endif
	}
#endif
	/* Stop and disconnect the PHY */
	if (priv->phydev) {
		phy_stop(priv->phydev);
		phy_disconnect(priv->phydev);
		priv->phydev = NULL;
	}

	netif_tx_stop_all_queues(dev);
	priv->xstats.gmac_release++;

	napi_disable(&priv->napi);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		del_timer_sync(&tx_queue->txtimer);
	}

	/* Free the IRQ lines */
	free_irq(dev->irq, dev);
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);
	if (priv->lpi_irq != -ENXIO)
		free_irq(priv->lpi_irq, dev);

	free_irq(priv->ch0_txirq, dev);

#ifdef CONFIG_AVB_NET
	free_irq(priv->ch1_txirq, dev);
	free_irq(priv->ch2_txirq, dev);
#endif

	free_irq(priv->rx_irq, dev);

	/* Stop TX/RX DMA and clear the descriptors */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}

	/* Release and free the Rx/Tx resources */
	free_dma_desc_resources(priv);

	/* Disable the MAC Rx/Tx */
	stmmac_set_mac(priv->ioaddr, false);

	netif_carrier_off(dev);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
	stmmac_exit_fs();
#endif

	stmmac_release_ptp(priv);

	priv->is_open = 0;
	return 0;
}

int gmac_get_skb_chn(struct sk_buff *skb, struct stmmac_priv *priv)
{
	int chn;
	unsigned short type;
	unsigned char pcp = 0;
	struct vlan_ethhdr *mac_header;

	skb_reset_mac_header(skb);
	mac_header = vlan_eth_hdr(skb);
	type = ntohs(mac_header->h_vlan_proto);
	switch (type) {
		case ETH_P_IP:
			chn = TX_CHN_NET;
			break;

		case ETH_P_8021Q:
			pcp = (ntohs(mac_header->h_vlan_TCI) & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT;
			if (pcp == priv->pcp_class_a) {
				chn = TX_CHN_CLASS_A;
			} else if (pcp == priv->pcp_class_b) {
				chn = TX_CHN_CLASS_B;
			} else {
				chn =  -1;
			}
			break;

		default:
			chn =  -1;
	}

	return chn;
}

int gmac_rx_rate(void)
{
	int chn;
	unsigned int rate = 0;
	int time =  0;
	unsigned int ratio = GMAC_LEN_RATIO / (GMAC_TIMER_RATIO * 8);
	unsigned int cal_len;
	unsigned int all_bytes = 0;
	struct gmac_rx_stat *rx_stat;

	if(!gmac_priv->enable_rate) {
		return -EACCES;
	}

	time = get_timer_slice_delta(gmac_priv->time, bsp_get_slice_value());
	if(time < GMAC_TMOUT) {
		return -EAGAIN;
	}

	gmac_priv->time = bsp_get_slice_value();

	for (chn = 0; chn < gmac_priv->tx_queue_cnt; chn++) {
		rx_stat =  GET_RX_STAT(gmac_priv, chn);
		cal_len = rx_stat->rx_bytes - rx_stat->rx_last_bytes;
		rx_stat->rx_last_bytes = rx_stat->rx_bytes;
		all_bytes += cal_len;

		rate = cal_len / (time * ratio);
		printk("[received chn%d] rx rate:%uMbps\n", chn, rate);
	}

	rate= all_bytes / (time * ratio);
	printk("[received all chn] rx rate:%uMbps\n", rate);

	return 0;
}

void stmmac_prepare_ptp_ctx_desc(struct gmac_tx_queue *tx_queue)
{
	union dma_desc *desc;
	unsigned int entry;

	/* build a context desc */
	entry = (tx_queue->cur_tx) % (tx_queue->dma_tx_size);
	desc = tx_queue->dma_tx + entry;
	*(tx_queue->tx_skbuff + entry) = NULL;
	*(tx_queue->tx_skbuff_dma + entry) = 0;

	desc->tx_desc.ctx.ostc = 1;
	desc->tx_desc.ctx.tcmmsv = 1;
	desc->tx_desc.ctx.ctx_type= 1;
	desc->tx_desc.ctx.own = 1;

	if (gmac_msg_level & GMAC_LEVEL_DESC_DBG) {
		printk(KERN_ERR "[%s]entry:%u\n", __func__, entry);
		print_hex_dump(KERN_ERR, "tx_desc:", DUMP_PREFIX_ADDRESS,
		       16, 4, (void *)desc, sizeof(union dma_desc), false);
	}

	tx_queue->cur_tx++;
}

int stmmac_identify_ptp_pkt(struct sk_buff *skb)
{
	struct ethhdr *mac_header = eth_hdr(skb);

	if (!mac_header) {
		GMAC_ERR(("[%s]:mac header is NULL!\n", __func__));
		return -EINVAL;
	}

	if (htons(ETH_P_1588) == mac_header->h_proto) {
		return 1;
	} else {
		return 0;
	}
}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
static netdev_tx_t stmmac_spe_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize;
	unsigned int entry;
	int chn, csum_insertion = 0;
	union dma_desc *desc, *first;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned long flags;
	struct gmac_tx_queue *tx_queue = NULL;

	if(skb->spe_own == priv->portno 
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
		|| skb->spe_own == priv->portno_wan
#endif
		)
	{
		chn = TX_CHN_NET;	//all skb from SPE using chn 0
	} else {
		GMAC_ERR(("[%s]wrong spe_own:%d\n", __func__, skb->spe_own));
		return NETDEV_TX_BUSY;
	}

	priv->xstats.enter_spe_xmit[chn]++;
	tx_queue = GET_TX_QUEUE(priv, chn);

	/* Check if tx avail */
	if (unlikely(stmmac_tx_avail(priv, chn) < 1)) {
		priv->xstats.tx_desc_full[chn]++;
		return NETDEV_TX_BUSY;
	}

	txsize = tx_queue->dma_tx_size;
	spin_lock_irqsave(&tx_queue->tx_lock, flags);

	csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);
	entry = tx_queue->cur_tx % txsize;

	if (gmac_msg_level & GMAC_LEVEL_TX_DBG) {
		if (skb->len > ETH_FRAME_LEN) {
			dma_unmap_single(priv->device, skb->dma, nopaged_len, DMA_TO_DEVICE);
			GMAC_TX_DBG(("[%s]entry:%d, chn:%d, skb_addr:%pK, len:%d"
				" nopagedlen: %d\n\tip_summed: %d - %s gso\n"
				"\ttx_count_frames %d\n", __func__, entry,chn, skb, skb->len,
				nopaged_len, skb->ip_summed, !skb_is_gso(skb) ? "isn't" : "is",
				tx_queue->tx_count_frames));
		}
	}

	/* Get the desc from dma tx queue. */
	desc = tx_queue->dma_tx + entry;
	first = desc;

	tx_queue->tx_skbuff[entry] = skb;
	desc->tx_desc.nrd.buf1_phy_addr = spe_hook.get_skb_dma(skb);
	GMAC_SPE_DBG(("[%s]spe_own:%d, entry:%d, buf_addr:0x%x\n",__func__,
			skb->spe_own, entry, desc->tx_desc.nrd.buf1_phy_addr));

	tx_queue->tx_skbuff_dma[entry] = desc->tx_desc.nrd.buf1_phy_addr;
	priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len, csum_insertion);
	priv->hw->desc->config_tx_slotnum(chn, desc);

	priv->hw->desc->close_tx_desc(desc);

	/* According to the coalesce parameter the IC bit for the latest
	 * segment could be reset and the timer re-started to invoke the
	 * stmmac_tx function. This approach takes care about the fragments.
	 */
	tx_queue->tx_count_frames += 1;
	if (tx_queue->tx_coal_frames > tx_queue->tx_count_frames) {
		priv->hw->desc->clear_tx_ic(desc);
		priv->xstats.tx_reset_ic_bit[chn]++;
		GMAC_TX_DBG(("[%s]entry:%d,tx_count_frames:%d\n", __func__, entry, tx_queue->tx_count_frames));
		mod_timer(&(tx_queue->txtimer),STMMAC_COAL_TIMER(tx_queue->tx_coal_timer));
	} else
		tx_queue->tx_count_frames = 0;

	/* To avoid raise condition */
	priv->hw->desc->set_tx_owner(first);
#ifndef BSP_CONFIG_BOARD_CPE
	wmb();
#endif

	tx_queue->cur_tx++;

	GMAC_TX_DBG(("[%s]chn:%d, curr:%d, dirty:%d entry:%d, first:%pK\n",
			__func__, chn, (tx_queue->cur_tx % txsize),
			(tx_queue->dirty_tx % txsize), entry, first));

	stmmac_display_ring((void *)tx_queue->dma_tx, txsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);
	GMAC_PKT_DBG(("[%s]frame to be transmitted: ", __func__));
	print_pkt(skb->data, 64, GMAC_LEVEL_PKT_DBG);

	dev->stats.tx_bytes += skb->len;
	tx_queue->tx_bytes += skb->len;
	tx_queue->tx_pkt_cnt++;

	if (priv->dma_cap.systime_source && tx_queue->hwts_tx_en) {
		if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP){
			/* declare that device is doing timestamping */
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			priv->hw->desc->enable_tx_timestamp(first);
		}
	}

	if (!tx_queue->hwts_tx_en)
		skb_tx_timestamp(skb);

#ifndef BSP_CONFIG_BOARD_CPE	
	priv->hw->dma->enable_dma_transmission(chn, entry);
#else
    entry_list = entry;
    entry_cnt++;
#endif
	spin_unlock_irqrestore(&tx_queue->tx_lock, flags);		


	return NETDEV_TX_OK;
}

#ifdef BSP_CONFIG_BOARD_CPE
void dwmac_enable_dma_transmission_new(void)
{
    struct stmmac_priv *priv = gmac_priv;
    void __iomem *ioaddr = priv->ioaddr;
    dma_addr_t dma_addr;
    struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);
    unsigned int index;

    if(entry_cnt){
        index = entry_list;
        if ( unlikely((index + 1) >= tx_queue->dma_tx_size)) {
            index = 0;
        } else {
            index++;
        }

        dma_addr = tx_queue->dma_tx_phy + (index * sizeof(union dma_desc));
        writel(dma_addr, ioaddr + DMA_CHN_TD_TAIL_PTR(0));
        entry_cnt = 0;
    }
    return;
}

void push_cb_set(int port)
{
    spe_wport_set_push_cb(port,
        (spe_wport_push_cb_t)dwmac_enable_dma_transmission_new);
}
#endif
#endif

/**
 *  stmmac_xmit: Tx entry point of the driver
 *  @skb : the socket buffer
 *  @dev : device pointer
 *  Description : this is the tx entry point of the driver.
 *  It programs the chain or the ring and supports oversized frames
 *  and SG feature.
 */
netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize;
	unsigned int entry;
	int chn, csum_insertion = 0, is_jumbo = 0;
	int cnt;
	int ptp_flag;
	int nfrags = skb_shinfo(skb)->nr_frags;
	union dma_desc *desc, *first;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned long flags;
	struct gmac_tx_queue *tx_queue = NULL;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if (skb->spe_own) {
		GMAC_ERR(("[%s]wrong spe_own:%d\n", __func__, skb->spe_own));
		return NETDEV_TX_BUSY;
	}
#endif

#ifdef CONFIG_AVB_NET
	chn = skb_get_queue_mapping(skb);
	if ((chn > TX_CHN_CLASS_A) || (chn < TX_CHN_NET)) {
		chn = TX_CHN_NET;
		priv->xstats.tx_chn_err++;
		GMAC_ERR(("[%s]error chn(%d),it will be reset to TX_CHN_NET!\n", __func__, chn));
	}
#else
	chn = TX_CHN_NET;
#endif

	ptp_flag = stmmac_identify_ptp_pkt(skb);
	if (ptp_flag < 0) {
		ptp_flag = 0;		//Reset ptp_flag to zero
		priv->xstats.tx_mac_header_err++;
	}

	priv->xstats.enter_xmit[chn]++;
	tx_queue = GET_TX_QUEUE(priv, chn);

	/* if avb switch is closed, we don't handle AVB channel. */
#ifdef CONFIG_AVB_NET
	if ((!priv->avb_support) && (chn > TX_CHN_NET)) {
		priv->xstats.tx_invalid_pkt++;
		GMAC_ERR(("[%s]error packet, chn:%d\n", __func__,chn));
		return NETDEV_TX_BUSY;
	}
#endif

	/* Check if tx avail */
	if (unlikely(stmmac_tx_avail(priv, chn) < nfrags + 1)) {
		if (!netif_subqueue_stopped(dev, skb)) {
			netif_stop_subqueue(dev, chn);

			/* This is a hard error, log it. */
			GMAC_ERR(("[%s]: chn[%d] Tx Ring full when queue awake\n", __func__, chn));
		}

		priv->xstats.tx_desc_full[chn]++;
		return NETDEV_TX_BUSY;
	}

	txsize = tx_queue->dma_tx_size;
	spin_lock_irqsave(&tx_queue->tx_lock, flags);

#if (FEATURE_ON == MBB_FEATURE_ETH)
    mbb_mac_clone_tx_save(skb);
#endif

	if (ptp_flag){
		GMAC_PTP_DBG(("[%s]ptp packet cnt:%d\n", __func__, priv->ptp_cnt));
		stmmac_prepare_ptp_ctx_desc(tx_queue);
		priv->ptp_cnt++;
		priv->ptp_bytes += skb->len;
	}

	csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);
	entry = tx_queue->cur_tx % txsize;

	if ((skb->len > ETH_FRAME_LEN) || nfrags)
		GMAC_TX_DBG(("[%s]: [entry %d], chn[%d]: skb addr %pK len: %d"
			" nopagedlen: %d\n\tn_frags: %d - ip_summed: %d - %s gso\n"
			"\ttx_count_frames %d\n", __func__, entry,chn, skb, skb->len,
			nopaged_len, nfrags, skb->ip_summed, !skb_is_gso(skb) ? "isn't" : "is",
			tx_queue->tx_count_frames));

	/* Get the desc from dma tx queue. */
	desc = tx_queue->dma_tx + entry;
	first = desc;
	if ((nfrags > 0) || (skb->len > ETH_FRAME_LEN))
		GMAC_TX_DBG(("[%s]skb len: %d, nopaged_len: %d,\n"
			 "\t\tn_frags: %d, ip_summed: %d\n", __func__,
			 skb->len, nopaged_len, nfrags, skb->ip_summed));

	tx_queue->tx_skbuff[entry] = skb;

	/* To program the descriptors according to the size of the frame */
	is_jumbo = priv->hw->ring->is_jumbo_frm(skb->len);

	if (likely(!is_jumbo)) {
		desc->tx_desc.nrd.buf1_phy_addr = dma_map_single(priv->device,
			skb->data, nopaged_len, DMA_TO_DEVICE);

		tx_queue->tx_skbuff_dma[entry] = desc->tx_desc.nrd.buf1_phy_addr;
		priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len, csum_insertion);
		priv->hw->desc->config_tx_slotnum(chn, desc);
	} else {
		desc = first;
	}

	for (cnt = 0; cnt < nfrags; cnt++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[cnt];
		int len = skb_frag_size(frag);

		entry = (++tx_queue->cur_tx) % txsize;

		desc = tx_queue->dma_tx + entry;
		GMAC_TX_DBG(("[%s]chn[%d],[entry %d] segment len: %d\n", __func__, chn, entry, len));
		desc->tx_desc.nrd.buf1_phy_addr = skb_frag_dma_map(priv->device, frag,
			0, len, DMA_TO_DEVICE);

		tx_queue->tx_skbuff_dma[entry] = desc->tx_desc.nrd.buf1_phy_addr;
		tx_queue->tx_skbuff[entry] = NULL;
		priv->hw->desc->prepare_tx_desc(desc, 0, len, csum_insertion);
		priv->hw->desc->config_tx_slotnum(chn, desc);
		wmb();
		priv->hw->desc->set_tx_owner(desc);
		wmb();
	}

	priv->hw->desc->close_tx_desc(desc);

	/* According to the coalesce parameter the IC bit for the latest
	 * segment could be reset and the timer re-started to invoke the
	 * stmmac_tx function. This approach takes care about the fragments.
	 */
	tx_queue->tx_count_frames += nfrags + 1;
	if (tx_queue->tx_coal_frames > tx_queue->tx_count_frames) {
		priv->hw->desc->clear_tx_ic(desc);
		priv->xstats.tx_reset_ic_bit[chn]++;
		GMAC_TX_DBG(("[%s]entry:%d, tx_count_frames:%d\n", __func__, entry, tx_queue->tx_count_frames));
		mod_timer(&(tx_queue->txtimer),STMMAC_COAL_TIMER(tx_queue->tx_coal_timer));
	} else
		tx_queue->tx_count_frames = 0;

	/* To avoid raise condition */
	priv->hw->desc->set_tx_owner(first);
	wmb();

	tx_queue->cur_tx++;

	GMAC_TX_DBG(("[%s]chn:%d, curr:%d dirty:%d entry:%d, first:%pK, nfrags:%d\n",
			__func__, chn, (tx_queue->cur_tx % txsize),
			(tx_queue->dirty_tx % txsize), entry, first, nfrags));

	stmmac_display_ring((void *)tx_queue->dma_tx, txsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);
	GMAC_PKT_DBG(("[%s]frame to be transmitted: ", __func__));
	print_pkt(skb->data, 64, GMAC_LEVEL_PKT_DBG);

	if (unlikely(stmmac_tx_avail(priv, chn) <= (MAX_SKB_FRAGS + 1))) {
		GMAC_TX_DBG(("[%s]stop transmitted packets\n", __func__));
		netif_stop_subqueue(dev, chn);
		priv->xstats.xmit_td_full[chn]++;
		priv->xstats.tx_avail[chn] = stmmac_tx_avail(priv, chn);
	}

	dev->stats.tx_bytes += skb->len;
	tx_queue->tx_bytes += skb->len;
	tx_queue->tx_pkt_cnt++;

	if (priv->dma_cap.systime_source && tx_queue->hwts_tx_en) {
		if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP){
			/* declare that device is doing timestamping */
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			priv->hw->desc->enable_tx_timestamp(first);
			GMAC_PTP_DBG(("[%s]chn:%d, flags:0x%x\n", __func__, chn,
				skb_shinfo(skb)->tx_flags));
		}
	}

	if (!tx_queue->hwts_tx_en)
		skb_tx_timestamp(skb);

	priv->hw->dma->enable_dma_transmission(chn, entry);

	spin_unlock_irqrestore(&tx_queue->tx_lock, flags);

	return NETDEV_TX_OK;
}

/**
 * stmmac_rx_refill: refill used skb preallocated buffers
 * @priv: driver private structure
 * Description : this is to reallocate the skb for the reception process
 * that is based on zero-copy.
 */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	unsigned int rxsize = rx_queue->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
	unsigned int index = 0;
	dma_addr_t dma;

	for (; rx_queue->cur_rx - rx_queue->dirty_rx > 0; rx_queue->dirty_rx++) {
		unsigned int entry = rx_queue->dirty_rx % rxsize;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (likely(rx_queue->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;
			if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
				skb = skb_dequeue(&priv->free_q);
				if (unlikely(skb == NULL))
					break;
				
				rx_queue->rx_skbuff[entry] = skb;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
				dma = dma_map_single(priv->device, skb->data, 32, DMA_FROM_DEVICE);
#endif
				dma = spe_hook.get_skb_dma(skb);
				p->rx_desc.nrd.buf1_phy_addr = dma;
			}else {
				skb = netdev_alloc_skb_ip_align(priv->dev, bfsize);
				if (unlikely(skb == NULL))
					break;

				rx_queue->rx_skbuff[entry] = skb;
				rx_queue->rx_skbuff_dma[entry] =
					dma_map_single(priv->device, skb->data, bfsize, DMA_FROM_DEVICE);
				p->rx_desc.nrd.buf1_phy_addr= rx_queue->rx_skbuff_dma[entry];
			}
		}
		wmb();
		p->rx_desc.nrd.buf1_addr_valid = 1;
		priv->hw->desc->set_rx_owner(p);
		priv->hw->desc->set_rx_ioc(p,rx_irq_flag);
		wmb();

		index = (entry + rxsize - 1) % rxsize;
		priv->hw->dma->set_rx_tail_ptr(index, RX_CHN);
	}
}

/**
 * @priv: driver private structure
 * @limit: napi bugget.
 * Description :  this the function called by the napi poll method.
 * It gets all the frames inside the ring.
 */
static int stmmac_rx(struct stmmac_priv *priv, int limit)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	struct gmac_rx_stat *rx_stat;
	unsigned int rxsize = rx_queue->dma_rx_size;
	unsigned int entry = rx_queue->cur_rx % rxsize;
	unsigned int next_entry;
	unsigned int count = 0;
	int chn;
	int ret;
	int coe = priv->plat->rx_coe;
	struct sk_buff *last_skb = NULL;
	unsigned long flags;
	int spe_enable = 0;
	struct sk_buff *skb;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
	u16 vlan_tag = 0;
	struct net_device *lan_wan_dev = NULL;

	lan_wan_dev = dev_get_by_name(&init_net, LAN_WAN_PORT_NAME);
#endif
	GMAC_RING_DBG((">>> stmmac_rx: descriptor ring:\n"));
	stmmac_display_ring((void *)rx_queue->dma_rx, rxsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);

	spe_enable = spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode());
	if(spe_enable){
		spin_lock_irqsave(&priv->rx_lock,flags);
	}

	while (count < limit) {
		int status;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (priv->hw->desc->get_rx_owner(p))
			break;

		if (gmac_msg_level & GMAC_LEVEL_DESC_DBG) {
			printk(KERN_ERR "\n[%s]entry:%u\n", __func__, entry);
			print_hex_dump(KERN_ERR, "rx_desc:", DUMP_PREFIX_ADDRESS,
			       16, 4, (void *)p, sizeof(union dma_desc), false);
		}

		count++;
		next_entry = (++rx_queue->cur_rx) % rxsize;
		prefetch(rx_queue->dma_rx + next_entry);

		/* read the status of the incoming frame */
		status = priv->hw->desc->rx_status(&priv->dev->stats,
						   &priv->xstats, p);

		if (unlikely(status == discard_frame)) {
			priv->dev->stats.rx_errors++;
			if (spe_enable) {
				skb = rx_queue->rx_skbuff[entry];
				rx_queue->rx_skbuff[entry] = NULL;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
				ret = vlan_get_tag(skb, &vlan_tag);
				if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
					&& (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
					) {
					stmmac_finish_td(priv->portno_wan, skb, 0);
				}else{
					stmmac_finish_td(priv->portno, skb, 0);
				}
#else
				stmmac_finish_td(priv->portno, skb, 0);
#endif
			}
		} else {
			int frame_len;
			dma_addr_t dma;
			int ptp_pkt = 0;
			struct sk_buff *skb_next;

			frame_len = priv->hw->desc->get_rx_frame_len(p);
			if (unlikely(status != llc_snap))
				frame_len -= ETH_FCS_LEN;

			if (frame_len > ETH_FRAME_LEN)
				GMAC_DBG(("\tRX frame size %d, COE status: %d\n",
					 frame_len, status));

			skb = rx_queue->rx_skbuff[entry];
			if (unlikely(!skb)) {
				GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n", priv->dev->name));
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			rx_queue->rx_skbuff[entry] = NULL;

			if (rx_queue->hwts_rx_en) {
				if (priv->hw->desc->rx_tstamp_available(p)) {
					unsigned int entry_new;

					entry_new = rx_queue->cur_rx % rxsize;
					skb_next = rx_queue->rx_skbuff[entry_new];
					if (!spe_enable) {
						dev_kfree_skb_any(skb_next);	//release the skb
					}

					stmmac_print_rx_tstamp_info(p);
					stmmac_get_rx_hwtstamp(priv, skb);
					ptp_pkt = 1;

					next_entry = (++rx_queue->cur_rx) % rxsize;
					prefetch(rx_queue->dma_rx + next_entry);
				}
			}
#if (FEATURE_ON == MBB_FEATURE_ETH)
            if (DMA_BUFFER_SIZE >= frame_len)
            {
                skb_put(skb, frame_len);
            }
            else
            {
                skb_put(skb, DMA_BUFFER_SIZE);
            }
#else
            skb_put(skb, frame_len);
#endif

#ifdef CONFIG_AVB_NET
			chn = gmac_get_skb_chn(skb, priv);
#else
			chn = TX_CHN_NET;
#endif
			if (chn >= TX_CHN_NET) {
				rx_stat = GET_RX_STAT(priv, chn);
				rx_stat->rx_bytes += skb->len;
				rx_stat->rx_cnt++;
			}
			gmac_rx_rate();

			/*all of packets*/
			rx_queue->rx_bytes += skb->len;
			rx_queue->rx_pkt_cnt++;

			GMAC_RX_DBG(("[%s]: frame received (%dbytes)\n",__func__, frame_len));
			print_pkt(skb->data, 64, GMAC_LEVEL_PKT_DBG);

#ifdef CONFIG_NEW_GMAC_TEST
			if (priv->phy_loopback) {
				priv->recv_pkt++;
				priv->recv_skb = skb_copy(skb, GFP_ATOMIC);
			}
#endif

#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
    && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
            local_receive_skb(frame_len, skb);
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH)
            mbb_mac_clone_rx_restore(skb);
            mbb_check_net_upgrade(skb);
#endif

			if(spe_enable){
				if (ptp_pkt) {	//if it's ptp packet, it will be send to network stack.
					struct skb_shared_hwtstamps *tstamp_new, *tstamp_old;
					struct sk_buff *ts_skb = skb_copy(skb, GFP_ATOMIC);
					if(!ts_skb) {
						GMAC_ERR(("[%s]alloc ts_skb failed!\n", __func__));
						break;
					}

					tstamp_new = skb_hwtstamps(ts_skb);
					tstamp_old = skb_hwtstamps(skb);
					memcpy((void *)tstamp_new, (void *)tstamp_old,
							sizeof(struct skb_shared_hwtstamps));

					stmmac_print_skb_tstamp(ts_skb);
					if (unlikely(!coe)) {
			            skb_checksum_none_assert(ts_skb);
			        } else {
			            ts_skb->ip_summed = CHECKSUM_UNNECESSARY;
			        }

					napi_gro_receive(&priv->napi, ts_skb);
					stmmac_finish_td(priv->portno, skb, 0);//recycle the first skb
					stmmac_finish_td(priv->portno, skb_next, 0);//recycle the next skb
				} else {
					dma = spe_hook.get_skb_dma(skb);

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
					dma_unmap_single(priv->device,rx_queue->rx_skbuff_dma[entry],32, DMA_FROM_DEVICE);
					ret = vlan_get_tag(skb, &vlan_tag);
					if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
						&& (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
						){
						ret = spe_hook.td_config(priv->portno_wan, skb, dma, spe_l3_bottom, 0);
					}else{
						ret = spe_hook.td_config(priv->portno, skb, dma, spe_l3_bottom, 0);
					}

#else
					ret = spe_hook.td_config(priv->portno, skb, dma, spe_l3_bottom, 0);
#endif

					if (SPE_ERR_TDFULL == ret || SPE_ERR_PORT_DISABLED == ret) {
						GMAC_DBG(("%s: td full\n",__func__));
						priv->xstats.rx_collision++;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
					ret = vlan_get_tag(skb, &vlan_tag);
					if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
						&& (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
						)
					{
						stmmac_finish_td(priv->portno_wan, skb, 0);
					}else{
						stmmac_finish_td(priv->portno, skb, 0);
					}
#else
						stmmac_finish_td(priv->portno, skb, 0);
#endif
					}
				}
			}else {
				dma_unmap_single(priv->device,
						 rx_queue->rx_skbuff_dma[entry],
						 priv->dma_buf_sz, DMA_FROM_DEVICE);

				skb->protocol = eth_type_trans(skb, priv->dev);

				if(last_skb){
					if (unlikely(!coe)) {
						skb_checksum_none_assert(last_skb);
					} else {
						last_skb->ip_summed = CHECKSUM_UNNECESSARY;
					}

#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
                         && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
                 //local_receive_skb(count-1, last_skb);
#else
                 napi_gro_receive(&priv->napi, last_skb);
#endif
                }
#ifdef CONFIG_BALONG_SKB_MEMBER
				skb->psh = 0;
#endif
				last_skb = skb;
			}

			gmac_status->rx_skb_count++;
			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
		}
		entry = next_entry;
	}

	if(spe_enable){
		spin_unlock_irqrestore(&priv->rx_lock,flags);
	}

	if(last_skb){
#ifdef CONFIG_BALONG_SKB_MEMBER
		last_skb->psh = 1;	//Make NCM send packet immediately.
#endif
		gmac_status->rx_psh_count++;

        if (unlikely(!coe)) {
            skb_checksum_none_assert(last_skb);
        } else {
            last_skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
             && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
        //local_receive_skb(count-1, last_skb);
#else
        napi_gro_receive(&priv->napi, last_skb);
#endif

    } 


	if(!spe_enable){
		stmmac_rx_refill(priv);
	}

	priv->xstats.rx_pkt_n += count;

	return count;
}

#else
static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	unsigned int rxsize = rx_queue->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
	unsigned int index = 0;

	for (; rx_queue->cur_rx - rx_queue->dirty_rx > 0; rx_queue->dirty_rx++) {
		unsigned int entry = rx_queue->dirty_rx % rxsize;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (likely(rx_queue->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;
			skb = netdev_alloc_skb_ip_align(priv->dev, bfsize);

			if (unlikely(skb == NULL))
				break;

			rx_queue->rx_skbuff[entry] = skb;
			rx_queue->rx_skbuff_dma[entry] =
			    dma_map_single(priv->device, skb->data, bfsize, DMA_FROM_DEVICE);

			p->rx_desc.nrd.buf1_phy_addr= rx_queue->rx_skbuff_dma[entry];
		}
		wmb();
		p->rx_desc.nrd.buf1_addr_valid = 1;
		priv->hw->desc->set_rx_owner(p);
		priv->hw->desc->set_rx_ioc(p,rx_irq_flag);
		wmb();

		index = (entry + rxsize - 1) % rxsize;
		priv->hw->dma->set_rx_tail_ptr(index, RX_CHN);
	}
}

static int stmmac_rx(struct stmmac_priv *priv, int limit)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	struct gmac_rx_stat *rx_stat;
	unsigned int rxsize = rx_queue->dma_rx_size;
	unsigned int entry = rx_queue->cur_rx % rxsize;
	unsigned int next_entry;
	unsigned int count = 0;
	int chn;
	int coe = priv->plat->rx_coe;
	struct sk_buff *last_skb = NULL;

	GMAC_RING_DBG(("[%s]: descriptor ring:\n", __func__));
	stmmac_display_ring((void *)rx_queue->dma_rx, rxsize * sizeof(union dma_desc), GMAC_LEVEL_RING_DBG);

	while (count < limit) {
		int status;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (priv->hw->desc->get_rx_owner(p))
			break;

		if (gmac_msg_level & GMAC_LEVEL_DESC_DBG) {
			printk(KERN_ERR "[%s]entry:%u\n", __func__, entry);
			print_hex_dump(KERN_ERR, "rx_desc:", DUMP_PREFIX_ADDRESS,
			       16, 4, (void *)p, sizeof(union dma_desc), false);
		}

		count++;
		next_entry = (++rx_queue->cur_rx) % rxsize;
		prefetch(rx_queue->dma_rx + next_entry);

		/* read the status of the incoming frame */
		status = priv->hw->desc->rx_status(&priv->dev->stats, &priv->xstats, p);
		if (unlikely(status == discard_frame)) {
			priv->dev->stats.rx_errors++;
		} else {
			struct sk_buff *skb;
			int frame_len;
			frame_len = priv->hw->desc->get_rx_frame_len(p);

			/* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
			 * Type frames (LLC/LLC-SNAP)
			 */
			if (unlikely(status != llc_snap))
				frame_len -= ETH_FCS_LEN;

			if (frame_len > ETH_FRAME_LEN)
				GMAC_RX_DBG(("\tRX frame size %d, COE status: %d\n",
					 frame_len, status));

			skb = rx_queue->rx_skbuff[entry];
			if (unlikely(!skb)) {
				GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
				       priv->dev->name));
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			rx_queue->rx_skbuff[entry] = NULL;

			if (rx_queue->hwts_rx_en) {
				if (priv->hw->desc->rx_tstamp_available(p)) {
					struct sk_buff *skb_next;
					unsigned int entry_new;

					entry_new = rx_queue->cur_rx % rxsize;
					skb_next = rx_queue->rx_skbuff[entry_new];
					dev_kfree_skb_any(skb_next);	//release the skb

					stmmac_print_rx_tstamp_info(p);
					stmmac_get_rx_hwtstamp(priv, skb);

					next_entry = (++rx_queue->cur_rx) % rxsize;
					prefetch(rx_queue->dma_rx + next_entry);
				}
			}
#if (FEATURE_ON == MBB_FEATURE_ETH)
            if (DMA_BUFFER_SIZE >= frame_len)
            {
                skb_put(skb, frame_len);
            }
            else
            {
                skb_put(skb, DMA_BUFFER_SIZE);
            }
#else
            skb_put(skb, frame_len);
#endif

#ifdef CONFIG_AVB_NET
			chn = gmac_get_skb_chn(skb, priv);
#else
			chn = TX_CHN_NET;
#endif
			if (chn >= TX_CHN_NET) {
				/* for the specific channel*/
				rx_stat = GET_RX_STAT(priv, chn);
				rx_stat->rx_bytes += skb->len;
				rx_stat->rx_cnt++;
			}
			gmac_rx_rate();

			/*all of packets*/
			rx_queue->rx_bytes += skb->len;
			rx_queue->rx_pkt_cnt++;

			dma_unmap_single(priv->device,
					 rx_queue->rx_skbuff_dma[entry],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);

			GMAC_RX_DBG(("[%s]frame received (%dbytes)\n",__func__, frame_len));
			print_pkt(skb->data, 64, GMAC_LEVEL_RX_DBG | GMAC_LEVEL_PTP_DBG);

#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
            local_receive_skb(frame_len, skb);
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH)
            mbb_mac_clone_rx_restore(skb);
            mbb_check_net_upgrade(skb);
#endif

			skb->protocol = eth_type_trans(skb, priv->dev);

			if(last_skb){
            	if (unlikely(!coe)) {
                    skb_checksum_none_assert(last_skb);
                } else {
                    last_skb->ip_summed = CHECKSUM_UNNECESSARY;
                }

#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
					//local_receive_skb(count-1, last_skb);
#else
					napi_gro_receive(&priv->napi, last_skb);
#endif

            }
#ifdef CONFIG_BALONG_SKB_MEMBER
			skb->psh = 0;
#endif
			last_skb = skb;
			gmac_status->rx_skb_count++;
			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
		}
		entry = next_entry;
	}

	if(last_skb){
#ifdef CONFIG_BALONG_SKB_MEMBER
		last_skb->psh = 1;	//Make NCM send packet immediately.
#endif
		gmac_status->rx_psh_count++;

        if (unlikely(!coe)) {
            skb_checksum_none_assert(last_skb);
        } else {
            last_skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
#if (((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST)) \
     && defined(CONFIG_NEW_GMAC_TEST) && defined(BSP_CONFIG_BOARD_TELEMATIC))
        //local_receive_skb(count-1, last_skb);
#else
       napi_gro_receive(&priv->napi, last_skb);
#endif
    }

	stmmac_rx_refill(priv);
	priv->xstats.rx_pkt_n += count;

	return count;
}

#endif

/**
 *  stmmac_poll - stmmac poll method (NAPI)
 *  @napi : pointer to the napi structure.
 *  @budget : maximum number of packets that the current CPU can receive from
 *	      all interfaces.
 *  Description :
 *  To look at the incoming frames and clear the tx resources.
 */
static int stmmac_poll(struct napi_struct *napi, int budget)
{
	struct stmmac_priv *priv = container_of(napi, struct stmmac_priv, napi);
	int work_done = 0;

	priv->xstats.napi_poll++;
	stmmac_tx_clean(priv);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    if(0 != g_mmitest_enable)
    {
        return work_done;
    }
#endif

	work_done = stmmac_rx(priv, budget);
	if (work_done < budget) {
		napi_complete(napi);
		stmmac_enable_dma_irq(priv);
	}
	return work_done;
}

/**
 *  stmmac_tx_timeout
 *  @dev : Pointer to net device structure
 *  Description: this function is called when a packet transmission fails to
 *   complete within a reasonable time. The driver will mark the error in the
 *   netdev structure and arrange for the device to be reset to a sane state
 *   in order to transmit a new packet.
 */
static void stmmac_tx_timeout(struct net_device *dev)
{
	int i;
	struct stmmac_priv *priv = netdev_priv(dev);

	/* Clear Tx resources and restart transmitting again */
	for (i = TX_CHN_NET; i < priv->tx_queue_cnt; i++) {
		stmmac_tx_err(priv, i);
	}
}

/* Configuration changes (passed on by ifconfig) */
static int stmmac_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP)	/* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		GMAC_WARNING(("%s: can't change I/O address\n", dev->name));
		return -EOPNOTSUPP;
	}

	/* Don't allow changing the IRQ */
	if (map->irq != dev->irq) {
		GMAC_WARNING(("%s: not change IRQ number %d\n", dev->name, dev->irq));
		return -EOPNOTSUPP;
	}

	return 0;
}

unsigned short stmmac_select_queue(struct net_device *dev,  struct sk_buff *skb)
{
	unsigned int vlan_qos;
	unsigned short queue_index = 0;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct vlan_ethhdr *mac_header = NULL;

	mac_header = vlan_eth_hdr(skb);
	if (!mac_header) {
		GMAC_ERR(("[%s]:mac header is NULL!\n", __func__));
		return -EINVAL;
	}

	if (htons(ETH_P_8021Q) == mac_header->h_vlan_proto) { //Only upport single VLAN tag.
		vlan_qos = (ntohs(mac_header->h_vlan_TCI) & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT;
		if (vlan_qos == priv->pcp_class_a) {
			queue_index = TX_CHN_CLASS_A;
		} else if (vlan_qos == priv->pcp_class_b) {
			queue_index = TX_CHN_CLASS_B;
		} else {
			GMAC_ERR(("%s: wrong vlan_qos(%u), it will be reset to 0!\n", __func__, vlan_qos));
			queue_index =  TX_CHN_NET;
		}
	} else {
		queue_index =  TX_CHN_NET;
	}

	return queue_index;
}

/**
 *  stmmac_set_rx_mode - entry point for multicast addressing
 *  @dev : pointer to the device structure
 *  Description:
 *  This function is a driver entry point which gets called by the kernel
 *  whenever multicast addresses must be enabled/disabled.
 *  Return value:
 *  void.
 */
static void stmmac_set_rx_mode(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	spin_lock(&priv->lock);
	priv->hw->mac->set_filter(dev, priv->synopsys_id);
	spin_unlock(&priv->lock);
}

/**
 *  stmmac_change_mtu - entry point to change MTU size for the device.
 *  @dev : device pointer.
 *  @new_mtu : the new MTU size for the device.
 *  Description: the Maximum Transfer Unit (MTU) is used by the network layer
 *  to drive packet transmission. Ethernet has an MTU of 1500 octets
 *  (ETH_DATA_LEN). This value can be changed with ifconfig.
 *  Return value:
 *  0 on success and an appropriate (-)ve integer as defined in errno.h
 *  file on failure.
 */
static int stmmac_change_mtu(struct net_device *dev, int new_mtu)
{
	int max_mtu;

	if (netif_running(dev)) {
		GMAC_ERR(("%s: must be stopped to change its MTU\n", dev->name));
		return -EBUSY;
	}

	max_mtu = SKB_MAX_HEAD(NET_SKB_PAD + NET_IP_ALIGN);

	if ((new_mtu < 46) || (new_mtu > max_mtu)) {
		GMAC_ERR(("%s: invalid MTU, max MTU is: %d\n", dev->name, max_mtu));
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	netdev_update_features(dev);

	return 0;
}

static netdev_features_t stmmac_fix_features(struct net_device *dev,
					     netdev_features_t features)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	if (priv->plat->rx_coe == STMMAC_RX_COE_NONE)
		features &= ~NETIF_F_RXCSUM;
	else if (priv->plat->rx_coe == STMMAC_RX_COE_TYPE1)
		features &= ~NETIF_F_IPV6_CSUM;
	if (!priv->plat->tx_coe)
		features &= ~NETIF_F_ALL_CSUM;

	/* Some GMAC devices have a bugged Jumbo frame support that
	 * needs to have the Tx COE disabled for oversized frames
	 * (due to limited buffer sizes). In this case we disable
	 * the TX csum insertionin the TDES and not use SF.
	 */
	if (priv->plat->bugged_jumbo && (dev->mtu > ETH_DATA_LEN))
		features &= ~NETIF_F_ALL_CSUM;

	return features;
}

/**
 *  stmmac_interrupt - main ISR
 *  @irq: interrupt number.
 *  @dev_id: to pass the net device pointer.
 *  Description: this is the main driver interrupt service routine.
 *  It calls the DMA ISR and also the core ISR to manage PMT, MMC, LPI
 *  interrupts.
 */
static irqreturn_t stmmac_interrupt(int irq, void *dev_id)
{
	int ret;
	int i;
	struct net_device *dev = (struct net_device *)dev_id;
	struct stmmac_priv *priv = netdev_priv(dev);
	void __iomem * ioaddr = (void __iomem *)dev->base_addr;

	if (unlikely(!dev)) {
		GMAC_ERR(("%s: invalid dev pointer\n", __func__));
		return IRQ_NONE;
	}

	if (priv->plat->has_gmac) {
		ret = priv->hw->mac->handle_irq(ioaddr, &priv->xstats);
		if (ret) {
			GMAC_ERR(("%s: Interrupt unclear! ret:0x%08x\n", __func__, ret));
			return IRQ_NONE;
		}
	}

	/* To handle DMA interrupts */
	for (i = 0; i < priv->tx_queue_cnt; i++) {
		stmmac_dma_interrupt(priv, i);
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/* Polling receive - used by NETCONSOLE and other diagnostic tools
 * to allow network I/O with interrupts disabled.
 */
static void stmmac_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	stmmac_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

int stmmac_set_mac_address(struct net_device *dev, void *addr)
{
	int ret=0;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	struct stmmac_priv *priv = netdev_priv(dev);
#endif

	printk("in %s \n",__func__);

	ret = eth_mac_addr(dev, addr);
	if(ret)
	{
		printk("eth_mac_addr return %d \n",ret);
		return ret;
	}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		spe_hook.port_ioctl(priv->portno,spe_port_ioctl_set_mac,(int)dev->dev_addr);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
		spe_hook.port_ioctl(priv->portno_wan,spe_port_ioctl_set_mac,(int)dev->dev_addr);
#endif
	}
#endif
	return 0;
}

/**
 *  stmmac_ioctl - Entry point for the Ioctl
 *  @dev: Device pointer.
 *  @rq: An IOCTL specefic structure, that can contain a pointer to
 *  a proprietary structure used to pass information to the driver.
 *  @cmd: IOCTL command
 *  Description:
 *  Currently it supports the phy_mii_ioctl(...) and HW time stamping.
 */
static int stmmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret = -EOPNOTSUPP;

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
#if (FEATURE_ON == MBB_FEATURE_ETH)
    case SIOCLINKSTATE:
    case SIOCLINKENABLE:
#endif
		if (!priv->phydev)
			return -EINVAL;
		ret = phy_mii_ioctl(priv->phydev, rq, cmd);
		break;

	case SIOCSHWTSTAMP:
		ret = stmmac_hwtstamp_ioctl(dev, rq);
		break;

	case STMMAC_CBS_REQUEST_BANDWIDTH:
 	case STMMAC_CBS_RELEASE_BANDWIDTH:
 	case STMMAC_CBS_QUERY_CHN:
 	case STMMAC_CBS_CONFIG_ALG:
		ret = stmmac_cbs_ioctl(dev, rq, cmd);
		break;

	default:
		break;
	}

	return ret;
}

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
static struct dentry *stmmac_fs_dir;
static struct dentry *stmmac_rings_status;
static struct dentry *stmmac_dma_cap;

static void sysfs_display_ring(void *head, int size, struct seq_file *seq)
{
	int i;
	union dma_desc *p = (union dma_desc *)head;

	for (i = 0; i < size; i++) {
		u32 *x;
		x = (u32 *)p;
		seq_printf(seq, "%d [0x%x]: 0x%x 0x%x 0x%x 0x%x\n",
			   i, (unsigned int)virt_to_phys(p),
			   *x, *(x + 1), *(x + 2), *(x + 3));
		p++;
		seq_printf(seq, "\n");
	}
}

static int stmmac_sysfs_ring_read(struct seq_file *seq, void *v)
{
	unsigned int chn;
	struct net_device *dev = seq->private;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_rx_queue *rx_queue = NULL;
	struct gmac_tx_queue *tx_queue = NULL;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		seq_printf(seq, "Chn%d RX descriptor ring:\n", chn);
		rx_queue = GET_RX_QUEUE(priv, chn);
		sysfs_display_ring((void *)rx_queue->dma_rx, rx_queue->dma_rx_size, seq);
	}

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		seq_printf(seq, "Chn%d TX descriptor ring:\n", chn);
		tx_queue = GET_TX_QUEUE(priv, chn);
		sysfs_display_ring((void *)tx_queue->dma_tx, tx_queue->dma_tx_size, seq);
	}

	return 0;
}

static int stmmac_sysfs_ring_open(struct inode *inode, struct file *file)
{
	return single_open(file, stmmac_sysfs_ring_read, inode->i_private);
}

static const struct file_operations stmmac_rings_status_fops = {
	.owner = THIS_MODULE,
	.open = stmmac_sysfs_ring_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int stmmac_sysfs_dma_cap_read(struct seq_file *seq, void *v)
{
	char *option;
	unsigned int number;
	struct net_device *dev = seq->private;
	struct stmmac_priv *priv = netdev_priv(dev);

	if (!priv->hw_cap_support) {
		seq_printf(seq, "DMA HW features not supported\n");
		return 0;
	}

	seq_printf(seq, "==============================\n");
	seq_printf(seq, "\tDMA HW features\n");
	seq_printf(seq, "==============================\n");

	seq_printf(seq, "\t10/100 Mbps %s\n",
		   (priv->dma_cap.mbps_10_100) ? "Y" : "N");
	seq_printf(seq, "\t1000 Mbps %s\n",
		   (priv->dma_cap.mbps_1000) ? "Y" : "N");
	seq_printf(seq, "\tHalf duple %s\n",
		   (priv->dma_cap.half_duplex) ? "Y" : "N");
	seq_printf(seq, "\tPCS (TBI/SGMII/RTBI PHY interfatces): %s\n",
		   (priv->dma_cap.pcs) ? "Y" : "N");
	seq_printf(seq, "\tVlan Hash Filter: %s\n",
		   (priv->dma_cap.vlan_hash_filter) ? "Y" : "N");
	seq_printf(seq, "\tSMA (MDIO) Interface: %s\n",
		   (priv->dma_cap.sma_mdio) ? "Y" : "N");
	seq_printf(seq, "\tPMT Remote wake up: %s\n",
		   (priv->dma_cap.pmt_remote_wake_up) ? "Y" : "N");
	seq_printf(seq, "\tPMT Magic Frame: %s\n",
		   (priv->dma_cap.pmt_magic_frame) ? "Y" : "N");
	seq_printf(seq, "\tRMON module: %s\n",
		   (priv->dma_cap.rmon) ? "Y" : "N");
	seq_printf(seq, "\tARP Offload: %s\n",
		   (priv->dma_cap.arp_offload) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588-2002 Time Stamp: %s\n",
		   (priv->dma_cap.time_stamp) ? "Y" : "N");
	seq_printf(seq, "\t802.3az - Energy-Efficient Ethernet (EEE) %s\n",
		   (priv->dma_cap.eee) ? "Y" : "N");
	seq_printf(seq, "\tChecksum Offload in TX: %s\n",
		   (priv->dma_cap.tx_coe) ? "Y" : "N");
	seq_printf(seq, "\tChecksum Offload in RX: %s\n",
		   (priv->dma_cap.rx_coe) ? "Y" : "N");
	seq_printf(seq, "\tNumber of Additional MAC addresses: %d\n",
		   priv->dma_cap.multi_addr);
	seq_printf(seq, "\tMAC address 32: %s\n",
		   (priv->dma_cap.multi_addr32) ? "Y" : "N");
	seq_printf(seq, "\tMAC address 64: %s\n",
		   (priv->dma_cap.multi_addr64) ? "Y" : "N");

	switch (priv->dma_cap.systime_source) {
		case 1:
			option = "Internal";
			break;

		case 2:
			option = "External";
			break;

		case 3:
			option = "both";
			break;

		default:
			option = "reserved";
			break;
	}

	seq_printf(seq, "\tSystime Source address: %s\n", option);

	seq_printf(seq, "\tSource Address or VLAN Insertion: %s\n",
		   (priv->dma_cap.sa_vlan_ins) ? "Y" : "N");

	switch (priv->dma_cap.phy_mode) {
		case 0:
			option = "GMII or MII";
			break;

		case 1:
			option = "RGMII";
			break;

		case 2:
			option = "SGMII";
			break;

		case 3:
			option = "TBII";
			break;

		case 4:
			option = "RMII";
			break;

		case 5:
			option = "RTBII";
			break;

		case 6:
			option = "SMII";
			break;

		case 7:
			option = "RevMII";
			break;

		default:
			option = "Resevered";
			break;
	}
	seq_printf(seq, "\tPHY Mode: %s\n", option);

	if (priv->dma_cap.rx_fifo_size > 11) {
		seq_printf(seq, "\tMTL Rx Fifo Size: reserved\n");
	} else {
		number = (1 << (priv->dma_cap.rx_fifo_size + 7));
		seq_printf(seq, "\tMTL Rx Fifo Size: %dbytes\n", number);
	}

	if (priv->dma_cap.tx_fifo_size > 11) {
		seq_printf(seq, "\tMTL Rx Fifo Size: reserved\n");
	} else {
		number = (1 << (priv->dma_cap.tx_fifo_size + 7));
		seq_printf(seq, "\tMTL Rx Fifo Size: %dbytes\n", number);
	}

	seq_printf(seq, "\tOne step Time Stamp: %s\n",
		   (priv->dma_cap.one_step_ts) ? "Y" : "N");
	seq_printf(seq, "\tPTP Offload: %s\n",
			(priv->dma_cap.ptp_offload) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588 High Word Reister: %s\n",
			(priv->dma_cap.high_word_reg) ? "Y" : "N");

	if (priv->dma_cap.addr_width > 2) {
		seq_printf(seq, "\tAddress Width: reserved\n");
	} else {
		number = (priv->dma_cap.addr_width * 8) + 32;
		seq_printf(seq, "\tAddress Width: %d\n", number);
	}

	seq_printf(seq, "\tDCB Feature: %s\n",
			(priv->dma_cap.dcb_enable) ? "Y" : "N");
	seq_printf(seq, "\tSplit Header Feature: %s\n",
			(priv->dma_cap.split_hdr) ? "Y" : "N");
	seq_printf(seq, "\tTCP Segmentation offload: %s\n",
			(priv->dma_cap.tso_en) ? "Y" : "N");
	seq_printf(seq, "\tDebug memory interface: %s\n",
			(priv->dma_cap.debug_mem_if) ? "Y" : "N");
	seq_printf(seq, "\tAV feature: %s\n",
			(priv->dma_cap.av_en) ? "Y" : "N");

	if (priv->dma_cap.hash_table_size) {
		number = (1 << (priv->dma_cap.hash_table_size + 5));
		seq_printf(seq, "\tHash Table Size: %d\n", number);
	}

	if (!(priv->dma_cap.l3l4_total_num)) {
		seq_printf(seq, "\tNo L3 or L4 filter!\n");
	} else {
		number = priv->dma_cap.l3l4_total_num;
		seq_printf(seq, "\tTotal Number L3 or L4 Filters: %d\n", number);
	}

	number = priv->dma_cap.num_rx_queue + 1;
	seq_printf(seq, "\tNumber of MTL Rx queue:%d\n", number);

	number = priv->dma_cap.num_tx_queue + 1;
	seq_printf(seq, "\tNumber of MTL Tx queue:%d\n", number);

	number = priv->dma_cap.num_rx_channel + 1;
	seq_printf(seq, "\tNumber of DMA Rx Channel:%d\n", number);

	number = priv->dma_cap.num_tx_channel + 1;
	seq_printf(seq, "\tNumber of DMA Tx Channel:%d\n", number);

	if (!(priv->dma_cap.num_pps_output)) {
		seq_printf(seq, "\tNo PPS output!\n");
	} else if (4 < priv->dma_cap.num_pps_output) {
		seq_printf(seq, "\tPPS output: reserved!\n");
	} else {
		seq_printf(seq, "\tNumber of PPS output:%d\n",
			priv->dma_cap.num_pps_output);
	}

	if (!(priv->dma_cap.num_aux_snap)) {
		seq_printf(seq, "\tNo Auxiliary Snapshot input!\n");
	} else if (4 < priv->dma_cap.num_aux_snap) {
		seq_printf(seq, "\tAuxiliary Snapshot input: reserved!\n");
	} else {
		seq_printf(seq, "\tNumber of Auxiliary Snapshot input:%d\n",
			priv->dma_cap.num_aux_snap);
	}

	return 0;
}

static int stmmac_sysfs_dma_cap_open(struct inode *inode, struct file *file)
{
	return single_open(file, stmmac_sysfs_dma_cap_read, inode->i_private);
}

static const struct file_operations stmmac_dma_cap_fops = {
	.owner = THIS_MODULE,
	.open = stmmac_sysfs_dma_cap_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int stmmac_init_fs(struct net_device *dev)
{
	/* Create debugfs entries */
	stmmac_fs_dir = debugfs_create_dir(STMMAC_RESOURCE_NAME, NULL);

	if (!stmmac_fs_dir || IS_ERR(stmmac_fs_dir)) {
		GMAC_ERR(("ERROR %s, debugfs create directory failed\n",
		       STMMAC_RESOURCE_NAME));

		return -ENOMEM;
	}

	/* Entry to report DMA RX/TX rings */
	stmmac_rings_status = debugfs_create_file("descriptors_status",
						  S_IRUGO, stmmac_fs_dir, dev,
						  &stmmac_rings_status_fops);

	if (!stmmac_rings_status || IS_ERR(stmmac_rings_status)) {
		pr_info("ERROR creating stmmac ring debugfs file\n");
		debugfs_remove(stmmac_fs_dir);

		return -ENOMEM;
	}

	/* Entry to report the DMA HW features */
	stmmac_dma_cap = debugfs_create_file("dma_cap", S_IRUGO, stmmac_fs_dir,
					     dev, &stmmac_dma_cap_fops);

	if (!stmmac_dma_cap || IS_ERR(stmmac_dma_cap)) {
		pr_info("ERROR creating stmmac MMC debugfs file\n");
		debugfs_remove(stmmac_rings_status);
		debugfs_remove(stmmac_fs_dir);

		return -ENOMEM;
	}

	return 0;
}

static void stmmac_exit_fs(void)
{
	debugfs_remove(stmmac_rings_status);
	debugfs_remove(stmmac_dma_cap);
	debugfs_remove(stmmac_fs_dir);
}
#endif /* CONFIG_NEW_STMMAC_DEBUG_FS */

static const struct net_device_ops stmmac_netdev_ops = {
	.ndo_open = stmmac_open,
	.ndo_start_xmit = stmmac_xmit,
	.ndo_stop = stmmac_release,
	.ndo_change_mtu = stmmac_change_mtu,
	.ndo_fix_features = stmmac_fix_features,
	.ndo_set_rx_mode = stmmac_set_rx_mode,
	.ndo_tx_timeout = stmmac_tx_timeout,
	.ndo_do_ioctl = stmmac_ioctl,
	.ndo_set_config = stmmac_config,
	.ndo_select_queue = stmmac_select_queue,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = stmmac_poll_controller,
#endif
	.ndo_set_mac_address = stmmac_set_mac_address,
};

#ifdef CONFIG_AVB_NET
static const struct net_device_cbs_ops stmmac_netdev_cbs_ops = {
	.ndo_cbs_config_bw = stmmac_cbs_config_bw,
	.ndo_cbs_get_bw = stmmac_cbs_get_class_bw,
	.ndo_cbs_get_locredit = stmmac_get_locredit,
	.ndo_cbs_get_hicredit = stmmac_get_hicredit,
	.ndo_cbs_get_avb_algorithm= stmmac_get_avb_algorithm,
	.ndo_cbs_get_preamble_ipg = stmmac_get_preamble_ipg,
	.ndo_cbs_set_class_pcp = stmmac_set_class_pcp,
	.ndo_cbs_get_class_pcp = stmmac_get_class_pcp,
};
#endif

/**
 *  stmmac_hw_init - Init the MAC device
 *  @priv: driver private structure
 *  Description: this function detects which MAC device
 *  (GMAC/MAC10-100) has to attached, checks the HW capability
 *  (if supported) and sets the driver's features (for example
 *  to use the ring or chaine mode or support the normal/enh
 *  descriptor structure).
 */
static int stmmac_hw_init(struct stmmac_priv *priv)
{
	int ret;
	struct mac_device_info *mac = NULL;
	int hw_cap[DMA_CAP_NUM];

	/* Identify the MAC HW device */
	if (priv->plat->has_gmac) {
		priv->dev->priv_flags |= IFF_UNICAST_FLT;
		mac = dwmac1000_setup(priv->ioaddr);
	}

	if (!mac)
		return -ENOMEM;

	priv->hw = mac;

	/* Get and dump the chip ID */
	priv->synopsys_id = stmmac_get_synopsys_id(priv);

	/* Get the HW capability (new GMAC newer than 3.50a) */
	hw_cap[0] = stmmac_get_hw_features(priv, &hw_cap[1], &hw_cap[2]);
	if (hw_cap[0] && hw_cap[1] && hw_cap[2]) {
		GMAC_INFO((" DMA HW capability register supported"));

		/* We can override some gmac/dma configuration fields: e.g.
		 *  tx_coe (e.g. that are passed through the
		 * platform) with the values from the HW capability
		 * register (if supported).
		 */
		priv->hw_cap_support = 1;
		priv->plat->pmt = priv->dma_cap.pmt_remote_wake_up;
		priv->plat->tx_coe = priv->dma_cap.tx_coe;
		if ((priv->dma_cap.rx_coe)) {
			priv->plat->rx_coe = STMMAC_RX_COE;
		}

	} else {
		GMAC_INFO((" No HW DMA feature register supported"));
	}

	ret = priv->hw->mac->rx_ipc(priv->ioaddr);
	if (!ret) {
		GMAC_WARNING((" RX IPC Checksum Offload not configured.\n"));
		priv->plat->rx_coe = STMMAC_RX_COE_NONE;
	}

	if (priv->plat->rx_coe)
		GMAC_INFO((" RX Checksum Offload Engine supported (type %d)\n",
			priv->plat->rx_coe));
	if (priv->plat->tx_coe)
		GMAC_INFO((" TX Checksum insertion supported\n"));

	if (priv->plat->pmt) {
		GMAC_INFO((" Wake-Up On Lan supported\n"));
		device_set_wakeup_capable(priv->device, 1);
	}
	return 0;
}
static int get_gmac_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	random_ether_addr(dev_addr);
	return 1;
}

static char *stmmac_mac = {"00:18:82:0C:0D:66"};
module_param(stmmac_mac, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(stmmac_mac, "stmmac ethernet address ");

/**
 * stmmac_dvr_probe
 * @device: device pointer
 * @plat_dat: platform data pointer
 * @addr: iobase memory address
 * Description: this is the main probe function used to
 * call the alloc_etherdev, allocate the priv structure.
 */
struct stmmac_priv *stmmac_dvr_probe(struct device *device,
				     struct plat_stmmacenet_data *plat_dat,
				     void __iomem *addr)
{
	int ret = 0;
	int chn;
	unsigned int txqs;
	struct net_device *ndev = NULL;
	struct stmmac_priv *priv;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    int portno;
    struct spe_port_attr stmmac_attr = {0};
#endif

#ifdef CONFIG_AVB_NET
	txqs = TX_CHN_NR;	//three tx queues
#else
	txqs = 1;
#endif

	ndev = alloc_etherdev_mqs(sizeof(struct stmmac_priv),txqs, RX_CHN_NR);
	if (!ndev)
		return NULL;

	SET_NETDEV_DEV(ndev, device);

	priv = netdev_priv(ndev);
	priv->device = device;
	priv->dev = ndev;
	priv->pcp_class_a = 5;		//default set
	priv->pcp_class_b = 4;		//default set

	/*using for debug*/
	gmac_priv = priv;

	ether_setup(ndev);
	stmmac_set_ethtool_ops(ndev);
	priv->pause = pause;
	priv->plat = plat_dat;
	priv->ioaddr = addr;
	priv->dev->base_addr = (unsigned long)addr;
	ndev->priv_flags |= IFF_LIVE_ADDR_CHANGE;

	/* Verify driver arguments */
	stmmac_verify_args();

	/* Override with kernel parameters if supplied XXX CRS XXX
	 * this needs to have multiple instances
	 */
	if ((phyaddr >= 0) && (phyaddr <= 31))
		priv->plat->phy_addr = phyaddr;

	/* Init MAC and get the capabilities */
	ret = stmmac_hw_init(priv);
	if (ret)
		goto error_free_netdev;
#if (FEATURE_ON == MBB_FEATURE_ETH)
    /* Network inteface name--add by wangweichao */
    snprintf(ndev->name, sizeof(ndev->name), "%s", NET_DEVICE_NAME);
#else
    snprintf(ndev->name, sizeof(ndev->name), "%s%%d", "gmac");// output is gmac%d
#endif

	ret = gmac_alloc_queue_struct(priv);
	if (ret) {
		GMAC_ERR(("%s: Alloc queue failed (error: %d)\n", __func__, ret));
		goto error_free_mac;
	}

	ret = gmac_alloc_rx_stat(priv);
	if (ret) {
		GMAC_ERR (("%s: Alloc rx stat failed (error: %d)\n", __func__, ret));
		goto error_free_queue_struct;
	}

	/* using for debug */
#ifdef CONFIG_AVB_NET
	tx_queue_class_a = GET_TX_QUEUE(priv,TX_CHN_CLASS_A);
	tx_queue_class_b = GET_TX_QUEUE(priv,TX_CHN_CLASS_B);
#endif	

    if (get_gmac_addr(stmmac_mac, priv->dev->dev_addr)) {
            GMAC_INFO(("%s:using random ethernet address\n", __func__));
    }
	
#if (FEATURE_ON == MBB_FEATURE_ETH)
    if (0 != mbb_get_eth_macAddr(priv->dev->dev_addr))
    {
        if (0 != get_gmac_addr(stmmac_mac, priv->dev->dev_addr)) 
        {
            GMAC_INFO(("%s:using random ethernet address\n", __func__));
        }
    }
#else
    if (get_gmac_addr(stmmac_mac, priv->dev->dev_addr)) {
            GMAC_INFO(("%s:using random ethernet address\n", __func__));
    }
#endif

	ndev->netdev_ops = &stmmac_netdev_ops;
#ifdef CONFIG_AVB_NET
	ndev->netdev_cbs_ops = &stmmac_netdev_cbs_ops;
#endif

	ndev->flags |= IFF_ALLMULTI;

	ndev->features |= ndev->hw_features | NETIF_F_HIGHDMA;
	ndev->watchdog_timeo = msecs_to_jiffies(watchdog);
#ifdef STMMAC_VLAN_TAG_USED
	/* Both mac100 and gmac support receive VLAN tag detection */
	ndev->features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif
	priv->msg_enable = netif_msg_init(debug, default_msg_level);

	if (flow_ctrl)
		priv->flow_ctrl = FLOW_AUTO;	/* RX/TX pause on */

	/* Rx Watchdog is available in the COREs newer than the 3.40.
	 * In some case, for example on bugged HW this feature
	 * has to be disable and this can be done by passing the
	 * riwt_off field from the platform.
	 */
	if ((priv->synopsys_id >= DWMAC_CORE_3_50) && (!priv->plat->riwt_off)) {
		for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
			rx_queue = GET_RX_QUEUE(priv, chn);
			rx_queue->use_riwt = 1;
		}
		pr_info(" Enable RX Mitigation via HW Watchdog Timer\n");
	}

	netif_napi_add(ndev, &priv->napi, stmmac_poll, 64);
	spin_lock_init(&priv->lock);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue =  GET_TX_QUEUE(priv, chn);
		spin_lock_init(&(tx_queue->tx_lock));
	}
	ret = register_netdev(ndev);
	if (ret) {
		GMAC_ERR(("%s: ERROR %i registering the device\n", __func__, ret));
		goto error_netdev_register;
	}

	/* If a specific clk_csr value is passed from the platform
	 * this means that the CSR Clock Range selection cannot be
	 * changed at run-time and it is fixed. Viceversa the driver'll try to
	 * set the MDC clock dynamically according to the csr actual
	 * clock input.
	 */
	if (priv->plat->clk_csr)
		priv->clk_csr = priv->plat->clk_csr;

	stmmac_check_pcs_mode(priv);

	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI) {
		/* MDIO bus Registration */
		ret = stmmac_mdio_register(ndev);
		if (ret < 0) {
			GMAC_DBG(("%s: MDIO bus (id: %d) registration failed",
				 __func__, priv->plat->bus_id));
			goto error_mdio_register;
		}
	}

	stmmac_init_tx_coalesce(priv);

	/* Alloc SPE port, configure parameters */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()){
		stmmac_attr.enc_type = spe_enc_none;
        stmmac_attr.attach_brg = spe_attach_brg_normal;
        stmmac_attr.rd_depth = 256;
        stmmac_attr.td_depth = 256;/*same as rx */
        stmmac_attr.rd_skb_size = BUF_SIZE_1_8kiB;//to save memory
        stmmac_attr.rd_skb_num = 256;
        stmmac_attr.desc_ops.finish_rd = stmmac_finish_rd;
        stmmac_attr.desc_ops.finish_td = stmmac_finish_td;
        stmmac_attr.net = ndev;
        portno = spe_hook.port_alloc(&stmmac_attr);
        priv->portno = portno;
#ifdef BSP_CONFIG_BOARD_CPE
        push_cb_set(portno);
#endif
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        priv->portno_wan = spe_hook.port_alloc(&stmmac_attr);
#ifdef BSP_CONFIG_BOARD_CPE
        push_cb_set(priv->portno_wan);
#endif
#endif
    }
#endif
#if ((FEATURE_ON == MBB_DEVBOOTSTATE) && (FEATURE_ON == MBB_FACTORY))
    INIT_WORK(&mii_flag_work, set_mii_flag);
#endif
	return priv;

error_mdio_register:
	unregister_netdev(ndev);
error_netdev_register:
	netif_napi_del(&priv->napi);
	gmac_free_rx_stat(priv);
error_free_queue_struct:
	gmac_free_queue_struct(priv);
error_free_mac:
	gmac_free_mac(priv);
error_free_netdev:
	free_netdev(ndev);

	return NULL;
}

/**
 * stmmac_dvr_remove
 * @ndev: net device pointer
 * Description: this function resets the TX/RX processes, disables the MAC RX/TX
 * changes the link status, releases the DMA descriptor rings.
 */
int stmmac_dvr_remove(struct net_device *ndev)
{
	int chn;
	struct stmmac_priv *priv = netdev_priv(ndev);

	pr_info("%s:\n\tremoving driver", __func__);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_free(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
		(void)spe_hook.port_free(priv->portno_wan);
#endif
	}
#endif

	stmmac_set_mac(priv->ioaddr, false);
	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI)
		stmmac_mdio_unregister(ndev);
	netif_carrier_off(ndev);
	unregister_netdev(ndev);
	gmac_free_rx_stat(priv);
	gmac_free_queue_struct(priv);
	gmac_free_mac(priv);
	free_netdev(ndev);

	return 0;
}

void gmac_show_common_info(void)
{
	struct stmmac_priv *priv;
	struct stmmac_extra_stats *status;
#ifdef CONFIG_AVB_NET
	u64 sys_freq;
#endif

	priv = gmac_priv;
	status = gmac_status;
	printk("relese:%lu\n", status->gmac_release);
	printk("rx psh count:%lu\n",status->rx_psh_count);
	printk("rx skb count:%lu\n",status->rx_skb_count);
	printk("tx_losscarrier:%lu\n",status->tx_losscarrier);
	printk("tx_invalid_pkt:%lu\n",status->tx_invalid_pkt);
	printk("tx_spe_invalid_pkt:%lu\n",status->tx_spe_invalid_pkt);
	printk("poll_link_changed:%lu\n",status->poll_link_changed);
	printk("poll_link_cfailed:%lu\n",status->poll_link_cfailed);
	printk("tx_queue_cnt:%u\n", priv->tx_queue_cnt);
	printk("rx_queue_cnt:%u\n", priv->rx_queue_cnt);
	printk("is_open:%u\n", priv->is_open);
	printk("suspend:%lu\n", status->gmac_suspend);
	printk("resume:%lu\n", status->gmac_resume);
	printk("freeze:%lu\n", status->gmac_freeze);
	printk("restore:%lu\n", status->gmac_restore);
	printk("rx_busy_n:%lu\n", status->rx_busy_n);
	printk("tx_busy_n:%lu\n", status->tx_busy_n);
	printk("gmac_suspend_save:%lu\n", status->gmac_suspend_save);
	printk("gmac_suspend_save_suc:%lu\n", status->gmac_suspend_save_suc);
	printk("gmac_suspend_restore:%lu\n", status->gmac_suspend_restore);
	printk("gmac_suspend_restore_suc:%lu\n", status->gmac_suspend_restore_suc);
	printk("gmac_suspend_restore_fail:%lu\n", status->gmac_suspend_restore_fail);
	printk("ptp_cnt:%u\n", priv->ptp_cnt);
	printk("line_speed:%u\n", priv->line_speed);
	printk("duplex mode:%d\n", priv->oldduplex);
#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : relese=%lu\n", __func__, status->gmac_release);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_psh_count=%lu\n", __func__, status->rx_psh_count);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->rx_skb_count=%lu\n", __func__, status->rx_skb_count);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->tx_losscarrier=%lu\n", __func__, status->tx_losscarrier);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->tx_invalid_pkt=%lu\n", __func__, status->tx_invalid_pkt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->tx_spe_invalid_pkt=%lu\n", __func__, status->tx_spe_invalid_pkt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->poll_link_changed=%lu\n", __func__, status->poll_link_changed);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->poll_link_cfailed=%lu\n", __func__, status->poll_link_cfailed);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->tx_queue_cnt=%lu\n", __func__, priv->tx_queue_cnt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->rx_queue_cnt=%u\n", __func__, priv->rx_queue_cnt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->is_open=%u\n", __func__, priv->is_open);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend=%lu\n", __func__, status->gmac_suspend);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_resume=%lu\n", __func__, status->gmac_resume);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_freeze=%lu\n", __func__, status->gmac_freeze);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_restore=%lu\n", __func__, status->gmac_restore);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->rx_busy_n=%lu\n", __func__, status->rx_busy_n);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->tx_busy_n=%lu\n", __func__, status->tx_busy_n);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend_save=%lu\n", __func__, status->gmac_suspend_save);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend_save_suc=%lu\n", __func__, status->gmac_suspend_save_suc);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend_restore=%lu\n", __func__, status->gmac_suspend_restore);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend_restore_suc=%lu\n", __func__, status->gmac_suspend_restore_suc);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : status->gmac_suspend_restore_fail=%lu\n", __func__, status->gmac_suspend_restore_fail);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->ptp_cnt=%u\n", __func__, priv->ptp_cnt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->line_speed=%u\n", __func__, priv->line_speed);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : priv->oldduplex=%d\n", __func__, priv->oldduplex);
#endif

#ifdef CONFIG_AVB_NET
	printk("bw_cleared:%lu\n",status->bw_cleared);
	sys_freq = div_u64((u64)priv->default_addend * STMMAC_SYSCLOCK, ADDEND_DEFAULT);
	printk("sys_freq:%lluHz\n", sys_freq);
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	printk("portno:%d\n",priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
	printk("portno_wan:%d\n",gmac_priv->portno_wan);
#endif
#endif
}

void gmac_show_tx_channel(int chn)
{
	struct gmac_tx_queue *tx_queue =  NULL;
	struct stmmac_priv *priv;
	struct stmmac_extra_stats *status;

	priv = gmac_priv;
	status = gmac_status;
	if (chn >= priv->tx_queue_cnt) {
		printk("[%s]error channel:%d\n", __func__, chn);
#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : error channel:%d\n", __func__, chn);
#endif
		return;
	}

	tx_queue = GET_TX_QUEUE(priv, chn);

	printk("[tx queue %d]cur:%d, dirty:%d\n", chn,
		(tx_queue->cur_tx % tx_queue->dma_tx_size),
		(tx_queue->dirty_tx % tx_queue->dma_tx_size));

	printk("[tx queue %d]TX bytes:%u, TX count:%u\n", chn,
		tx_queue->tx_bytes, tx_queue->tx_pkt_cnt);
	
	printk("[tx queue %d]tx_coal_frames:%u\n", chn, tx_queue->tx_coal_frames);

#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [tx queue %d]cur:%d, dirty:%d\n", __func__, chn,
                (tx_queue->cur_tx % tx_queue->dma_tx_size),
                (tx_queue->dirty_tx % tx_queue->dma_tx_size));
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [tx queue %d]TX bytes:%u, TX count:%u\n", __func__, chn,
                tx_queue->tx_bytes,
                tx_queue->tx_pkt_cnt);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [tx queue %d]tx_coal_frames:%u\n", __func__, chn,
                tx_queue->tx_coal_frames);
#endif

    
#ifdef CONFIG_AVB_NET
	if (chn > TX_CHN_NET) {
		int bw;
		bw = stmmac_cbs_get_bw(priv->ioaddr, chn, priv->line_speed);
		printk("[tx queue %d]bw:%u\n", chn, bw);
	}
#endif

	printk("tx_desc_full:%lu\n", status->tx_desc_full[chn]);
	printk("enter_xmit:%lu\n", status->enter_xmit[chn]);
	printk("enter_spe_xmit:%lu\n", status->enter_spe_xmit[chn]);
	printk("xmit_td_full:%lu\n", status->xmit_td_full[chn]);
	printk("tx avail:%lu\n", status->tx_avail[chn]);
	printk("tx_pkt_cnt:%lu\n", status->tx_pkt_cnt[chn]);
	printk("tx_stopped:%lu\n", status->tx_stopped[chn]);
	printk("tx_early_irq:%lu\n", status->tx_early_irq[chn]);
	printk("fatal_bus_error_irq:%lu\n", status->fatal_bus_error_irq[chn]);
	printk("tx_ctx_desc_err:%lu\n", status->tx_ctx_desc_err[chn]);
	printk("tx_dma_err:%lu\n", status->tx_dma_err[chn]);
	printk("tx_buf_unavail:%lu\n", status->tx_buf_unavail[chn]);
	printk("tx_clean_own:%lu\n", status->tx_clean_own[chn]);
	printk("tx_pkt_n:%lu\n", status->tx_pkt_n[chn]);
	printk("tx_normal_irq_n:%lu\n", status->tx_normal_irq_n[chn]);
	printk("tx_clean:%lu\n", status->tx_clean[chn]);
	printk("tx_reset_ic_bit:%lu\n", status->tx_reset_ic_bit[chn]);
	printk("mtl_int:%lu\n", status->mtl_int[chn]);
	printk("dma_chn_int:%lu\n", status->dma_chn_int[chn]);
	printk("normal_irq_n:%lu\n", status->normal_irq_n[chn]);

#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_desc_full=%lu\n", __func__, status->tx_desc_full[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : enter_xmit=%lu\n", __func__, status->enter_xmit[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : enter_spe_xmit=%lu\n", __func__, status->enter_spe_xmit[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : xmit_td_full=%lu\n", __func__, status->xmit_td_full[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_avail=%lu\n", __func__, status->tx_avail[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_pkt_cnt=%lu\n", __func__, status->tx_pkt_cnt[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_stopped=%lu\n", __func__, status->tx_stopped[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_early_irq=%lu\n", __func__, status->tx_early_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : fatal_bus_error_irq=%lu\n", __func__, status->fatal_bus_error_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_ctx_desc_err=%lu\n", __func__, status->tx_ctx_desc_err[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_dma_err=%lu\n", __func__, status->tx_dma_err[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_buf_unavail=%lu\n", __func__, status->tx_buf_unavail[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_clean_own:%lu\n", __func__, status->tx_clean_own[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_pkt_n=%lu\n", __func__, status->tx_pkt_n[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_normal_irq_n=%lu\n", __func__, status->tx_normal_irq_n[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_clean=%lu\n", __func__, status->tx_clean[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : tx_reset_ic_bit=%lu\n", __func__, status->tx_reset_ic_bit[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : mtl_int=%lu\n", __func__, status->mtl_int[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : dma_chn_int=%lu\n", __func__, status->dma_chn_int[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : normal_irq_n=%lu\n", __func__, status->normal_irq_n[chn]);
#endif
}

int gmac_show_rx_channel(unsigned int chn)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(gmac_priv, chn);
	struct gmac_rx_stat *rx_stat = GET_RX_STAT(gmac_priv, chn);

	if (chn >= gmac_priv->rx_queue_cnt) {
		printk("Error chn(%d)!\n", chn);
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : Error chn(%d)!\n", __func__, chn);
#endif
		return chn;
	}

	printk("[rx queue %d]cur:%d, dirty:%d\n", 0,
		(rx_queue->cur_rx % rx_queue->dma_rx_size),
		(rx_queue->dirty_rx % rx_queue->dma_rx_size));

	printk("[rx queue %d]RX bytes:%u, RX count:%d\n", 0,
		rx_queue->rx_bytes, rx_queue->rx_pkt_cnt);

	printk("[received chn%d]bytes:%u, count:%d\n", chn, 
		rx_stat->rx_bytes, rx_stat->rx_cnt);

	printk("rx_buf_unav_irq:%lu\n", gmac_status->rx_buf_unav_irq[chn]);
	printk("rx_process_stopped_irq:%lu\n", gmac_status->rx_process_stopped_irq[chn]);
	printk("rx_watchdog_irq:%lu\n", gmac_status->rx_watchdog_irq[chn]);
	printk("rx_dma_err:%lu\n", gmac_status->rx_dma_err[chn]);
	printk("rx_early_irq:%lu\n", gmac_status->rx_early_irq[chn]);
	printk("dma_chn_int:%lu\n", gmac_status->rx_normal_irq_n[chn]);

#if ( FEATURE_ON == MBB_MLOG )
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [rx queue %d]cur:%d, dirty:%d\n",__func__, 0,
                (rx_queue->cur_rx % rx_queue->dma_rx_size),
                (rx_queue->dirty_rx % rx_queue->dma_rx_size));

    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [rx queue %d]RX bytes:%u, RX count:%d\n", __func__, 0,
                rx_queue->rx_bytes, rx_queue->rx_pkt_cnt);

    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : [received chn%d]bytes:%u, count:%d\n", __func__, chn,
                rx_stat->rx_bytes, rx_stat->rx_cnt);

    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_buf_unav_irq:%lu\n", __func__, gmac_status->rx_buf_unav_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_process_stopped_irq:%lu\n", __func__,
                gmac_status->rx_process_stopped_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_watchdog_irq:%lu\n", __func__, gmac_status->rx_watchdog_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_dma_err:%lu\n", __func__, gmac_status->rx_dma_err[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx_early_irq:%lu\n", __func__, gmac_status->rx_early_irq[chn]);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : dma_chn_int:%lu\n", __func__, gmac_status->rx_normal_irq_n[chn]);
#endif

	return 0;
}

void gmac_help(void)
{
	printk("show tx channel n:      gmac_show_tx_channel n\n");
	printk("show rx channel n:      gmac_show_rx_channel n\n");
	printk("show all channels:      gmac_show_all_channel\n");
	printk("show_common_info:       gmac_show_common_info\n");
	printk("show_tx_ring_buffer:    gmac_show_tx_ring_buffer n\n");
	printk("show_rx_ring_buffer:    gmac_show_rx_ring_buffer n\n");
	printk("mac_readl:              mac_readl\n");
	printk("mac_writel:             mac_readl\n");
	printk("mac_reg_dump:           mac_reg_dump\n");
}

int gmac_show_tx_ring_buffer(unsigned int chn)
{
	unsigned int txsize;
	struct gmac_tx_queue *tx_queue;
	struct stmmac_priv *priv = gmac_priv;

	if (chn >= priv->tx_queue_cnt) {
		printk("Error chn(%d)!\n", chn);
		return chn;
	}

	tx_queue = GET_TX_QUEUE(priv, chn);
	txsize = tx_queue->dma_tx_size;

	printk("TX chn[%d] descriptor ring:\n", chn);
	print_hex_dump(KERN_ERR, "TX desc:", DUMP_PREFIX_OFFSET, 16, 4,
		(void *)(tx_queue->dma_tx), txsize * sizeof(union dma_desc), false);

	return 0;
}

int gmac_show_rx_ring_buffer(unsigned int chn)
{
	unsigned int rxsize;
	struct gmac_rx_queue *rx_queue;
	struct stmmac_priv *priv = gmac_priv;

	if (chn >= priv->rx_queue_cnt) {
		printk("Error chn(%d)!\n", chn);
		return chn;
	}

	rx_queue = GET_RX_QUEUE(priv, chn);
	rxsize = rx_queue->dma_rx_size;
	printk("RX chn[%d] descriptor ring:\n", chn);
	print_hex_dump(KERN_ERR, "RX desc:", DUMP_PREFIX_OFFSET, 16, 4,
		(void *)(rx_queue->dma_rx), rxsize * sizeof(union dma_desc), false);

	return 0;
}

/* get register */
int mac_readl(int offset)
{
	int ret;

	if (offset > GMAC_REG_OFFSET_END) {
		printk(KERN_ERR "offset should <= 0x%x!\n", GMAC_REG_OFFSET_END);
		return offset;
	}

	ret = readl(gmac_priv->ioaddr + offset);
	printk(KERN_ERR "[offset 0x%x]:value 0x%x\n", offset, ret);

	return ret;
}

void mac_writel(unsigned int offset, int value)
{
	if (offset > GMAC_REG_OFFSET_END) {
		printk(KERN_ERR "offset should <= 0x%x!\n", GMAC_REG_OFFSET_END);
		return;
	}

	writel(value, (gmac_priv->ioaddr + offset));
	mac_readl(offset);
}

void mac_reg_dump(unsigned int size)
{
	if (size > GMAC_REG_OFFSET_END) {
		printk(KERN_ERR "size:0x%x, please input size <= 0x%x!\n", size, GMAC_REG_OFFSET_END);
		return;
	}

	print_hex_dump(KERN_ERR, "gmac_reg:", DUMP_PREFIX_OFFSET,
		       16, 4, (void *)gmac_priv->ioaddr, size, false);
}
/*
*	chn_ctrl:0,print tx chn0 and rx chn0
*			1,print tx chn1 and rx chn0
*			2 print tx chn2 and rx chn0
*			3,print all tx chn and rx chn0
*/
void gmac_show_all(void)
{
	int chn;

	gmac_show_common_info();

	for (chn = 0; chn < gmac_priv->tx_queue_cnt; chn++) {
		printk("======show chn%d tx stat info======\n", chn);
		gmac_show_tx_channel(chn);
		printk("======show chn%d tx ring buffer======\n", chn);
		gmac_show_tx_ring_buffer(chn);
	}

	for (chn = 0; chn < gmac_priv->rx_queue_cnt; chn++) {
		printk("======show chn%d rx stat info======\n", chn);
		gmac_show_rx_channel(chn);
		printk("======show chn%d rx ring buffer======\n", chn);
		gmac_show_rx_ring_buffer(chn);
	}

	printk("======dump all regs======\n");
	mac_reg_dump(GMAC_REG_OFFSET_END);
}

#ifdef CONFIG_PM
#ifndef CONFIG_BALONG_GMAC_LPM
int stmmac_suspend(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;
	int chn;

#ifndef BSP_CONFIG_BOARD_TELEMATIC
	if (priv->init_suc) {
		return -1;
	}
#endif

	if (!ndev || !netif_running(ndev)) {
		printk("[%s]suspend finished(no up)!\n", __func__);
		return 0;
	}

	if (priv->phydev)
		phy_stop(priv->phydev);

	spin_lock_irqsave(&priv->lock, flags);

	netif_device_detach(ndev);
	netif_tx_stop_all_queues(ndev);
	priv->xstats.gmac_suspend++;

	napi_disable(&priv->napi);

	/* Stop TX/RX DMA */
	for (chn = 0;chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0;chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}

	stmmac_clear_descriptors(priv);

	/* Enable Power down mode by programming the PMT regs */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, priv->wolopts);
	else {
		stmmac_set_mac(priv->ioaddr, false);
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	/* disable clock */
	stmmac_clk_disable(priv->device);
	printk("[%s]suspend finished(after up)!\n", __func__);

	return 0;
}

int stmmac_resume(struct net_device *ndev)
{
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    //stmmac_resume改为在phy上电后调用，见ar8035.c中调用stmmac_restore()
    return 0;
#endif
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;
	int chn;

#ifndef BSP_CONFIG_BOARD_TELEMATIC
	if (priv->init_suc) {
		return 0;
	}
#endif

	if (!netif_running(ndev)) {
		printk("[%s]resume finished(no up)!\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&priv->lock, flags);

	/* Power Down bit, into the PM register, is cleared
	 * automatically as soon as a magic packet or a Wake-up frame
	 * is received. Anyway, it's better to manually clear
	 * this bit because it can generate problems while resuming
	 * from another devices (e.g. serial console).
	 */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, 0);	//dwmac1000_pmt

	netif_device_attach(ndev);

	/* open clock */
	stmmac_clk_enable(priv->device);

	/* Enable the MAC and DMA */
	stmmac_set_mac(priv->ioaddr, true);

	for (chn = 0;chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->start_tx(priv->ioaddr, chn);
	}

	for (chn = 0;chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->start_rx(priv->ioaddr, chn);
	}

	napi_enable(&priv->napi);

	netif_tx_start_all_queues(ndev);

	spin_unlock_irqrestore(&priv->lock, flags);

	if (priv->phydev)
		phy_start(priv->phydev);

	printk("[%s]resume finished(after up)!\n", __func__);

	return 0;
}
#else
int stmmac_suspend_balong(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_tx_queue *tx_queue;
	struct gmac_rx_queue *rx_queue;
	int chn;
	unsigned int entry;
	union dma_desc *p;

	priv->xstats.gmac_suspend++;
	priv->need_open = 0;
	priv->old_speed = priv->line_speed;

#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    (void)eth_power_down();
#endif

	if(priv->is_open){
		/*if gmac has data to transfer, you can't suspend*/
		for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
			tx_queue = GET_TX_QUEUE(priv, chn);
			if(tx_queue->dirty_tx !=tx_queue->cur_tx){
				priv->xstats.tx_busy_n++;
				return -EBUSY;
			}
		}

		/*if gmac has received data to deal, you can't suspend*/
		for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
			rx_queue = GET_RX_QUEUE(priv, chn);
			entry = rx_queue->cur_rx % rx_queue->dma_rx_size;
			p = rx_queue->dma_rx + entry;
			if (!priv->hw->desc->get_rx_owner(p)){
				priv->xstats.rx_busy_n++;
				return -EBUSY;
			}
		}

		/*mark the gmac open state, we need to open it in resume process*/
		priv->need_open = 1;
		priv->xstats.gmac_suspend_save++;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
		if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
			(void)spe_hook.port_disable(priv->portno);
		}
#endif
		/* Stop and disconnect the PHY */
		if (priv->phydev) {
			phy_stop(priv->phydev);
			phy_disconnect(priv->phydev);
			priv->phydev = NULL;
		}

		netif_tx_stop_all_queues(dev);
		priv->xstats.gmac_release++;
		napi_disable(&priv->napi);

		for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
			tx_queue = GET_TX_QUEUE(priv, chn);
			del_timer_sync(&tx_queue->txtimer);
		}

		/* Stop TX/RX DMA and clear the descriptors and
		* save tx queue bandwidth and algorithm.
		*/
		for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
			priv->hw->dma->stop_tx(priv->ioaddr, chn);
#ifdef CONFIG_AVB_NET
			tx_queue = GET_TX_QUEUE(priv, chn);
			tx_queue->bandwidth = stmmac_cbs_get_bw(priv->ioaddr, chn,
			    priv->line_speed);
#endif
        }

		for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
			priv->hw->dma->stop_rx(priv->ioaddr, chn);
		}

		/* Disable the MAC Rx/Tx */
		stmmac_set_mac(priv->ioaddr, false);
		netif_carrier_off(dev);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
		stmmac_exit_fs();
#endif

		stmmac_release_ptp(priv);
		priv->is_open = 0;
		priv->xstats.gmac_suspend_save_suc++;
	}

	stmmac_clk_disable(priv->device);
	return 0;
}

int stmmac_resume_balong(struct net_device *dev)
{
    struct stmmac_priv *priv = netdev_priv(dev);
    int ret;
    int chn;
    struct gmac_rx_queue *rx_queue;
    struct gmac_tx_queue *tx_queue;

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY_BCM89811)
    (void)bcm89811_phy_hwreset(priv);
#endif
#endif
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    (void)eth_power_on();
#endif
    priv->xstats.gmac_resume++;

    stmmac_clk_enable(priv->device);

    if(priv->need_open){
        priv->xstats.gmac_suspend_restore++;

        /* get and check mac addr */
        stmmac_check_ether_addr(priv);

        if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
            priv->pcs != STMMAC_PCS_RTBI) {
            ret = stmmac_init_phy(dev);
            if (ret) {
                GMAC_ERR(("%s: Cannot attach to PHY (error: %d)\n",
                     __func__, ret));
                goto open_error;
            }
        }

		/*don't release dma,just reset the point*/
		for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
			rx_queue = GET_RX_QUEUE(priv, chn);
			rx_queue->cur_rx = 0;
			rx_queue->dirty_rx = 0;
		}
		
		for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
			tx_queue = GET_TX_QUEUE(priv, chn);
			tx_queue->dirty_tx = 0;
			tx_queue->cur_tx = 0;
		}

		/* DMA initialization and SW reset */
		ret = stmmac_init_dma_engine(priv);
		if (ret < 0) {
			GMAC_ERR(("%s: DMA initialization failed\n", __func__));
			goto open_error;
		}

		/* If required, perform hw setup of the bus. */
		if (priv->plat->bus_setup)
			priv->plat->bus_setup(priv->ioaddr);

		/*Initialize the MTL */
		priv->hw->mtl->mtl_init();

		/* Initialize the MAC Core */
		ret = priv->hw->mac->core_init(priv->ioaddr);
		if (ret) {
			GMAC_ERR(("%s: core_init initialization failed\n", __func__));
			goto open_error;
		}

		/* Copy the MAC addr into the HW  */
		priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);

		/* Enable SPE port */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
		if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
			(void)spe_hook.port_enable(priv->portno);
		}
#endif

		/* Enable the MAC Rx/Tx */
		stmmac_set_mac(priv->ioaddr, true);

		/* mmc_setup*/
		stmmac_mmc_setup(priv);

		ret = stmmac_init_ptp(priv);
		if (ret)
			GMAC_WARNING(("%s: failed PTP initialisation\n", __func__));

#ifdef CONFIG_AVB_NET
		stmmac_get_line_speed(priv);
		if (priv->line_speed == priv->old_speed) {
			for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
				tx_queue = GET_TX_QUEUE(priv, chn);
				stmmac_cbs_cfg_slope(priv->ioaddr, chn, tx_queue->bandwidth,
					priv->line_speed);
			}
		} else {
			GMAC_ERR(("[%s]Line speed changed from %dMbps to %dMbps!\n",
			__func__, priv->old_speed, priv->line_speed));
		}
#endif

		stmmac_init_vlan(priv);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
		ret = stmmac_init_fs(dev);
		if (ret < 0)
			GMAC_WARNING(("%s: failed debugFS registration\n", __func__));
#endif

		GMAC_DBG(( "%s: DMA RX/TX processes started...\n", dev->name));
		for (chn = 0;chn < priv->tx_queue_cnt; chn++)
			priv->hw->dma->start_tx(priv->ioaddr, chn);

		for (chn = 0;chn < priv->tx_queue_cnt; chn++)
			priv->hw->dma->start_rx(priv->ioaddr, chn);

		/* Dump DMA/MAC registers */
		if (netif_msg_hw(priv)) {
			priv->hw->mac->dump_regs(priv->ioaddr);
			priv->hw->dma->dump_regs(priv->ioaddr);
		}

		if (priv->phydev)
			phy_start(priv->phydev);

        for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
            rx_queue = GET_RX_QUEUE(priv, chn);
            if ((rx_queue->use_riwt) && (priv->hw->dma->rx_watchdog)) {
                rx_queue->rx_riwt = riwt_value;
                priv->hw->dma->rx_watchdog(priv->ioaddr, riwt_value, chn);
            }
        }
#ifdef BSP_CONFIG_BOARD_TELEMATIC
        stmmac_set_rx_mode(dev);
#endif
        napi_enable(&priv->napi);
        netif_tx_start_all_queues(dev);
        priv->is_open = 1;
        priv->xstats.gmac_suspend_restore_suc++;
        return 0;

    open_error:
        if (priv->phydev)
            phy_disconnect(priv->phydev);
        priv->xstats.gmac_suspend_restore_fail++;
        return ret;
    }

	return 0;
}
#endif

int stmmac_freeze(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	priv->xstats.gmac_freeze++;
	if (!ndev || !netif_running(ndev))
		return 0;

	return stmmac_release(ndev);
}

int stmmac_restore(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	priv->xstats.gmac_restore++;
	if (!ndev || !netif_running(ndev))
		return 0;

	return stmmac_open(ndev);
}
#endif /* CONFIG_PM */

#ifndef MODULE
static int __init stmmac_cmdline_opt(char *str)
{
	char *opt;

	if (!str || !*str)
		return -EINVAL;
	while ((opt = strsep(&str, ",")) != NULL) {
		if (!strncmp(opt, "debug:", 6)) {
			if (kstrtoint(opt + 6, 0, &debug))
				goto err;
		} else if (!strncmp(opt, "phyaddr:", 8)) {
			if (kstrtoint(opt + 8, 0, &phyaddr))
				goto err;
		} else if (!strncmp(opt, "dma_txsize:", 11)) {
			if (kstrtoint(opt + 11, 0, &dma_txsize))
				goto err;
		} else if (!strncmp(opt, "dma_rxsize:", 11)) {
			if (kstrtoint(opt + 11, 0, &dma_rxsize))
				goto err;
		} else if (!strncmp(opt, "buf_sz:", 7)) {
			if (kstrtoint(opt + 7, 0, &buf_sz))
				goto err;
		} else if (!strncmp(opt, "tc:", 3)) {
			if (kstrtoint(opt + 3, 0, &tc))
				goto err;
		} else if (!strncmp(opt, "watchdog:", 9)) {
			if (kstrtoint(opt + 9, 0, &watchdog))
				goto err;
		} else if (!strncmp(opt, "flow_ctrl:", 10)) {
			if (kstrtoint(opt + 10, 0, &flow_ctrl))
				goto err;
		} else if (!strncmp(opt, "pause:", 6)) {
			if (kstrtoint(opt + 6, 0, &pause))
				goto err;
		} else if (!strncmp(opt, "eee_timer:", 10)) {
			if (kstrtoint(opt + 10, 0, &eee_timer))
				goto err;
		}
	}
	return 0;

err:
	GMAC_ERR(("%s: ERROR broken module parameter conversion", __func__));
	return -EINVAL;
}

__setup("stmmaceth=", stmmac_cmdline_opt);
#endif /* MODULE */

MODULE_DESCRIPTION("STMMAC 10/100/1000 Ethernet device driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");

#if (FEATURE_ON == MBB_FEATURE_ETH)
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
typedef int (*check_recv_packet)(struct sk_buff *skb, int frame_len);

void mbb_mmitest_check_recv(struct net_device *dev, check_recv_packet check_fun)
{
    struct stmmac_priv *priv = netdev_priv(dev);
    struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, 0);
    unsigned int rxsize = rx_queue->dma_rx_size;
    unsigned int entry = rx_queue->cur_rx % rxsize;
    unsigned int next_entry;
    union dma_desc *p = rx_queue->dma_rx + entry;
    union dma_desc *p_next;    
    int bfsize = priv->dma_buf_sz;
    //int coe = priv->plat->rx_coe;

    stmmac_tx_clean(priv);
       
    while (!priv->hw->desc->get_rx_owner(p)) 
    {
        int status;

        next_entry = (++rx_queue->cur_rx) % rxsize;
        p_next = rx_queue->dma_rx + next_entry;
        prefetch(p_next);

        /* read the status of the incoming frame */
        status = priv->hw->desc->rx_status(&priv->dev->stats, &priv->xstats, p);
        
        if (unlikely(status == discard_frame))
        {
            priv->dev->stats.rx_errors++;
        }    
        else
        {
            struct sk_buff *skb;
            int frame_len;            

            frame_len = priv->hw->desc->get_rx_frame_len(p);
            /* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
             * Type frames (LLC/LLC-SNAP) */
            if (unlikely(status != llc_snap))
            {   
                frame_len -= ETH_FCS_LEN;
            }

            skb = rx_queue->rx_skbuff[entry];
            if (unlikely(!skb)) 
            {
                GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
                    priv->dev->name));
                priv->dev->stats.rx_dropped++;
                break;
            }
            prefetch(skb->data - NET_IP_ALIGN);
            rx_queue->rx_skbuff[entry] = NULL;

            skb_put(skb, frame_len);
            dma_unmap_single(priv->device,
                rx_queue->rx_skbuff_dma[entry],
                priv->dma_buf_sz, DMA_FROM_DEVICE);

            check_fun(skb, frame_len);

            dev_kfree_skb(skb);
        } 

        entry = next_entry;
        p = p_next; /*use prefetched values*/
    }

    stmmac_rx_refill(priv);
}

void mbb_mmitest_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int chn =0;
    struct stmmac_priv *priv = netdev_priv(dev);
    
    struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);
	
    unsigned int txsize = tx_queue->dma_tx_size;
    unsigned int entry;
    static unsigned int pkt_cnt = 0;
    int i, csum_insertion = 0;
    union dma_desc *desc, *first;
    unsigned int nopaged_len = skb_headlen(skb);

    spin_lock(&tx_queue->tx_lock);

    entry = tx_queue->cur_tx % txsize;

    csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);

    desc = tx_queue->dma_tx + entry;
    first = desc;

    tx_queue->tx_skbuff[entry] = skb;

    if (priv->hw->ring->is_jumbo_frm(skb->len))
    {
        entry = priv->hw->ring->jumbo_frm(priv, skb, csum_insertion);
        desc = tx_queue->dma_tx + entry;
    }
    else 
    {
        desc->tx_desc.nrd.buf1_phy_addr = dma_map_single(priv->device, skb->data,
            nopaged_len, DMA_TO_DEVICE);
	priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len,
						csum_insertion);
    }

    /* Interrupt on completition only for the latest segment */
    pkt_cnt++;
    if(pkt_cnt < 32)
    {
        priv->hw->desc->close_tx_desc(desc);
    }
    else
    {
        pkt_cnt = 0;
        priv->hw->desc->close_tx_desc(desc);
    }

    wmb();

    /* To avoid raise condition */
    priv->hw->desc->set_tx_owner(first);

    tx_queue->cur_tx++;

    priv->hw->dma->enable_dma_transmission(chn,priv->ioaddr);

    spin_unlock(&tx_queue->tx_lock);
}

int mbb_get_spe_wan_portno(void)
{
    struct stmmac_priv *priv = NULL;
    struct net_device *ndev = NULL;
    ndev = dev_get_by_name(&init_net, NET_DEVICE_NAME);
    if (NULL != ndev)
    {
        priv = netdev_priv(ndev);
    }

    if (NULL != priv)
    {
        return priv->portno_wan;
    }
    else
    {
        return -1;
    }
}
#endif
#endif

/*******************************Test phy loopback******************************/
#if ( FEATURE_ON == MBB_MLOG )
void mbb_gmac_readl()
{
    int reg_value = 0;

    /*MAC Configuration Register */
    reg_value = mac_readl(0x0);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x0", reg_value);

    /* txoctetcount_gb */
    reg_value = mac_readl(0x714);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x714", reg_value);

    /* txframecount_gb */
    reg_value = mac_readl(0x718);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x718", reg_value);

    /* txbroadcastframes_g */
    reg_value = mac_readl(0x71c);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x71c", reg_value);

    /* txmulticastframes_g */
    reg_value = mac_readl(0x720);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x720", reg_value);

    /* txunderflowerror */
    reg_value = mac_readl(0x748);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x748", reg_value);

    /* txsinglecol_g */
    reg_value = mac_readl(0x74c);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x74c", reg_value);

    /* txmulticol_g */
    reg_value = mac_readl(0x750);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x750", reg_value);

    /* txdeferred */
    reg_value = mac_readl(0x754);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x754",reg_value);

    /* txlatecol */
    reg_value = mac_readl(0x758);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x758", reg_value);

    /* txexesscol */
    reg_value = mac_readl(0x75c);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x75c", reg_value);

    /* txcarriererror */
    reg_value = mac_readl(0x760);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x760", reg_value);

    /* txexcessdef */
    reg_value = mac_readl(0x76c);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x76c", reg_value);

    /* txpauseframes */
    reg_value = mac_readl(0x770);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x770", reg_value);

    /* rxframecount_gb */
    reg_value = mac_readl(0x780);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x780", reg_value);

    /* rxoctetcount_gb */
    reg_value = mac_readl(0x784);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x784", reg_value);

    /* rxcrcerror */
    reg_value = mac_readl(0x794);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x794", reg_value);

    /*rxalignmenterror */
    reg_value = mac_readl(0x798);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x798", reg_value);

    /* rxrunterror */
    reg_value = mac_readl(0x79c);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x79c", reg_value);

    /* rxjabbererror */
    reg_value = mac_readl(0x7a0);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7a0", reg_value);

    /* rxlengtherror */
    reg_value = mac_readl(0x7c8);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7c8", reg_value);

    /* rxoutofrangetype */
    reg_value = mac_readl(0x7cc);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7cc", reg_value);

    /* rxpauseframes */
    reg_value = mac_readl(0x7d0);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7d0", reg_value);

    /* rxfifooverflow */
    reg_value = mac_readl(0x7d4);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7d4", reg_value);

    /* rxvlanframes_gb */
    reg_value = mac_readl(0x7d8);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7d8", reg_value);

    /* rxwatchdogerror */
    reg_value = mac_readl(0x7dc);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x720", reg_value);

    /* rxrcverror */
    reg_value = mac_readl(0x7e0);
    mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : reg[%s]=0x%x\n", __func__, "0x7e0", reg_value);

}
#endif

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#if ((FEATURE_ON == MBB_FACTORY) || (FEATURE_ON == MBB_AGING_TEST))
void local_receive_skb(int frame_len,struct sk_buff *skb)
{

    char rx_pkt[100] = {0};
    int ret = 0;
    int i = 0;

    ret = LOOPBACK_RX_LIMITED;

    if (0 == frame_len || NULL == skb)
    {
        g_rx_result = LOOPBACK_RX_ERROR;
        return;
    }

    if (TEST_PACKET_LEN == frame_len)
    {
        memcpy(rx_pkt, (uint8_t *)skb->data, frame_len);

        for (i = 0; i < TEST_PACKET_LEN; i++)
        {
            if (rx_pkt[i] != packet[i])
            {
                break;
            }
        }
        if (TEST_PACKET_LEN != i)
        {
            GMAC_ERR(("%s:match failed.\n",__FUNCTION__));
            ret = LOOPBACK_RX_ERROR;
        }
        else
        {
            GMAC_DBG(("%s:match success.\n",__FUNCTION__));
            ret = LOOPBACK_RX_OK;
        }
    }

    g_rx_result = ret;
}
EXPORT_SYMBOL(local_receive_skb);

int stmmac_send_packet()
{
    int ret = 0;

    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv = gmac_priv;

    skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN_LOOPBACK);
    if (NULL == skb)
    {
        GMAC_ERR(("%s:no packet to send,return\n",__FUNCTION__));
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : no packet to send,return\n", __func__);
#endif

        return LOOPBACK_TX_ERROR;
    }

#ifdef NET_SKBUFF_DATA_USES_OFFSET
    skb->mac_header = NET_IP_ALIGN;
#else
    skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    memcpy(skb->data, packet, TEST_PACKET_LEN);
    skb->len = TEST_PACKET_LEN;
    skb_set_queue_mapping(skb, TX_CHN_NET);

    /*send the packet*/
    ret = dev_queue_xmit(skb);
    if (NETDEV_TX_OK != ret)
    {
        GMAC_ERR(("%s:xmit error\n", __FUNCTION__));
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : xmit error\n", __func__);
#endif

        return LOOPBACK_TX_ERROR;
    }

    g_tx_skb = skb;

    return ret;

}

int start_test()
{

    int ret = 0;
    g_rx_result = LOOPBACK_RX_LIMITED;
    ret = stmmac_send_packet();
    if (ret)
    {
        GMAC_ERR(("%s:send packet error:ret=%d\n",__FUNCTION__,ret));
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : send packet error:ret=%d\n", __func__, ret);
#endif

        return LOOPBACK_TX_ERROR;
    }

    msleep(100);

    if (LOOPBACK_RX_ERROR == g_rx_result)
    {

        GMAC_ERR(("%s:tx and rx do not match!\n",__FUNCTION__));
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : send packet error:ret=%d\n", __func__, ret);
#endif

        return LOOPBACK_MATCH_ERROR;
    } 
    else if (LOOPBACK_RX_LIMITED == g_rx_result)
    {
        GMAC_ERR(("%s:rx timeout!\n",__FUNCTION__));
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : rx timeout!\n", __func__);
#endif

        return LOOPBACK_TIMEOUT_ERROR;
    }

    return LOOPBACK_TX_OK;
}

void set_gmac_fullduplex(void __iomem *ioaddr)
{
    u32 value = readl(ioaddr + GMAC_CONTROL);
    value |= GMAC_CONTROL_FULLDUPLEX;
    writel(value, ioaddr + GMAC_CONTROL);
}

int start_loopback_test()
{
    struct stmmac_priv *priv = NULL;
    int ret = 0;
    int regnum = 0;
    struct phy_device *phydev = mbb_get_phy_device();

    printk("%s:loopback test begin! \n",__FUNCTION__);
    priv = gmac_priv;
    if (NULL == phydev)
    {
        printk("%s:Ethrtnet driver failed! \n",__FUNCTION__);
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : Ethrtnet driver failed! \n", __func__);
#endif
        return LOOPBACK_TEST_ERROR;
    }

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY_BCM89811)
    regnum = GetPhyReg(BCM89811_CONTROL_REG);
    if (0 == (regnum & BCM89811_LOOPBACK))
    {
        SetPhyReg(BCM89811_CONTROL_REG, (regnum | BCM89811_LOOPBACK));
        msleep(LOOPBACK_WAIT);
    }
#else
    if (AR8035_DIGITAL_LOOPBACK_100M != GetPhyReg(AR8035_CONTROL_REG))
    {
        SetPhyReg(AR8035_CONTROL_REG, AR8035_DIGITAL_LOOPBACK_100M);
        msleep(LOOPBACK_WAIT);
    }
#endif
    ret = start_test();
    if (ret) {
        gmac_show_common_info();
        gmac_show_tx_channel(0);
        ret = gmac_show_rx_channel(0);
        if (0 != ret)
        {
            GMAC_ERR(("%s:Error chn(%d)!\n",__FUNCTION__, ret));
        }

        GMAC_ERR(("%s:loopback test failed! ret = %d\n",__FUNCTION__,ret));
#if ( FEATURE_ON == MBB_MLOG )
        mbb_gmac_readl();
        mlog_print(MLOG_RGMII, mlog_lv_factory, "%s : loopback test failed! ret = %d\n", __func__, ret);
#endif

        return LOOPBACK_TEST_ERROR;
    }
    printk("%s:loopback test sucess! \n",__FUNCTION__);
    return LOOPBACK_TEST_OK;
}
#endif
#endif

