/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * liufangyuan <liufangyuan2@huawei.com>
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




#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/bitops.h>
#include "stmmac.h"
#include "stmmac_debug.h"
#include "stmmac_cbs.h"

#define SKB_LEN 			1514
#define ETH_MAC_ADDR_LEN   	6
#define ETH_MAC_HLEN        14
#define IP_DF           	0x4000

#define TX_OK 			0
#define TX_ERROR 		-1
#define GMAC_TEST_CLEAN

/* PTP defines */
#define PTP_TYPE		0x88f7
#define PTP_VERSION		2
#define PTP_HDR_LEN		34
#define	PTP_PAYLOAD_LEN	(SKB_LEN - ETH_MAC_HLEN - PTP_HDR_LEN)

/* IP V4 paragrams */
#define IP_HDR_LEN     20
#define UDP_HDR_LEN    8
#define IP_PAYLOAD     8
#define ETH_HTONS(x)    ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#define SKB_PRINT_LEN   60
#define PAYLOAD_LEN     (SKB_LEN - ETH_MAC_HLEN - IP_HDR_LEN - UDP_HDR_LEN)

/* audio and video packet */
#define AV_HEADER		18
#define AV_SKB_LEN		1486
#define AV_PAYLOAD_LEN	(AV_SKB_LEN - AV_HEADER)
#define GMAC_TEST_PKT_CNT 30
#define CLASS_A_INTERVAL	125
#define CLASS_B_INTERVAL	250
#define GMAC_TEST_TIME_CNT	6

/* phy loop back related */
#define RGMII_COPPER_CTL		0x0
#define RGMII_PHY_LOOPBACK		BIT(14)
#define RGMII_PHY_ADDR			0x1

#define JUDGE_INTERVAL_A(x, n)  (((x) > (1000 * (CLASS_A_INTERVAL - (n)))) && ((x) < (1000 * (CLASS_A_INTERVAL + (n)))))
#define JUDGE_INTERVAL_B(x, n)  (((x) > (1000 * (CLASS_B_INTERVAL - (n)))) && ((x) < (1000 * (CLASS_B_INTERVAL + (n)))))

/* Dst MAC addr */
static unsigned char g_aucDstMac[ETH_MAC_ADDR_LEN] = {0x00,0xE0,0x4C,0x97,0xD7,0xEE};

/* PTP Multicast MAC addr */
static unsigned char ptp_dmac[ETH_MAC_ADDR_LEN] = {0x001,0x80,0xC2,0x00,0x00,0x0E};

/* Src MAC addr */
static unsigned char g_aucSrcMac[ETH_MAC_ADDR_LEN] = {0x00,0x18,0x82,0x0C,0x0D,0x66};

/* Dst ip addr */
static unsigned char g_aucDstIPAddr[4] = {192,168,1,10};

/* Src ip addr */
static unsigned char g_aucSrcIPAddr[4] = {192,168,1,1};
static unsigned short g_usSrcPort = 6001;
static unsigned short g_usDstPort = 6002;

/* av pkt defines */
static unsigned char gmac_hdrs[3][16] = {
	/* for channel 0 : Non tagged header
	 * Dst addr : 0x00:0xE0:0x4C:0x97:0xD7:0xEE
	 * Src addr : 0x00:0x18:0x82:0x0C:0x0D:0x66
	 * Type/Length : 0x800
	 * */
	{0x00, 0xE0, 0x4C, 0x97, 0xD7, 0xEE, 
	 0x00, 0x18, 0x82, 0x0C, 0x0D, 0x66, 
	 0x08, 0x00, 0x00, 0x00},

	/* for channel 1 : VLAN tagged header with priority 4
	 * Dst addr : 0x00:0xE0:0x4C:0x97:0xD7:0xEE
	 * Src addr : 0x00:0x18:0x82:0x0C:0x0D:0x66
	 * Type/Length : 0x8100 
	 * PCP:4
	 * CFI:0
	 * VID:0x64
	 * */
	{0x00, 0xE0, 0x4C, 0x97, 0xD7, 0xEE, 
	 0x00, 0x18, 0x82, 0x0C, 0x0D, 0x66, 
	 0x81, 0x00, 0x80, 0x64},

	/* for channel 2 : VLAN tagged header with priority 5
	 * Dst addr : 0x00:0xE0:0x4C:0x97:0xD7:0xEE
	 * Src addr : 0x00:0x18:0x82:0x0C:0x0D:0x66
	 * Type/Length : 0x8100
	 * PCP:5
	 * CFI:0
	 * VID:0x64
	 * */
	{0x00, 0xE0, 0x4C, 0x97, 0xD7, 0xEE, 
	 0x00, 0x18, 0x82, 0x0C, 0x0D, 0x66, 
	 0x81, 0x00, 0xA0, 0x64},
};

static unsigned char gmac_payload_ch[3] = {
	0x11,
	0x22,
	0x33,
};

static unsigned char avtype[2] = {
	0xf0,
	0x22,
};

static unsigned char av_data[48] = {
	0x01, 0x02, 0x03, 0x04, 0x00, 0x02, 0x00, 0x18, 0x82, 0x0c, 0x0d, 0x66, 0x81, 0x00, 0x80, 0x64,
	0x22, 0xf0, 0x68, 0x81, 0x32, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x02, 0x00, 0x02, 0x0b, 0xeb,
	0xc1, 0xea, 0x30, 0x14, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
};

struct sys_time {
	int seconds;
	int nano_seconds;
};

struct gmac_test_queue {
	struct timer_list timer;
	struct stmmac_priv *priv;
	unsigned int usec;
	unsigned int tx_num;
};

static struct sys_time gmac_last_time;
static struct sys_time gmac_current_time;
static struct sys_time gmac_time_itv[GMAC_TEST_TIME_CNT];	//gmac time interval
static int sys_time_flag = 0;
unsigned int gmac_timer_continue = 0;
int class_a_slot_num = 0;
int class_b_slot_num = 0;
struct timer_list gmac_test_timer;
struct stmmac_time_spec *gmac_test_ts_a = NULL;
struct stmmac_time_spec *gmac_test_ts_b = NULL;
u32 ca_itv[GMAC_TEST_PKT_CNT];	//class A interval
u32 cb_itv[GMAC_TEST_PKT_CNT];	//class B interval
struct gmac_test_queue *gmac_tq = NULL;	//gmac test queue
int usec_limit = 19500;

extern struct stmmac_priv *gmac_priv;
extern void stmmac_tx_clean(struct stmmac_priv *priv);

enum ptp_msg_type {
	PTP_SYNC,
	PTP_DELAY_REQ,
	PTP_PDELAY_REQ,
	PTP_PDELAY_RESP,
	PTP_FOLLOW_UP =  8, 
	PTP_DELAY_RESP,
	PTP_PDELAY_RESP_FOLLOW_UP,
	PTP_ANNOUNCE,
	PTP_SIGNALING,
	PTP_MANAGEMENT
};

enum ptp_msg_ctl_filed {
	CTL_SYNC,
	CTL_DELAY_REQ, 
	CTL_FOLLOW_UP,
	CTL_DELAY_RESP,
	CTL_MANAGEMENT,
	CTL_OTHERS
};

enum ptp_port_state {
	INITIALIZING = 1,
	FAULTY,
	DISABLED, 
	LISTENING,
	PRE_MASTER,
	MASTER,
	PASSIVE,
	UNCALIBRATED,
	SLAVE
};

enum ptp_addr_type {
	UNICAST,
	MULTICAST,
	BROADCAST
};

struct ptp_msg_state {
	enum ptp_msg_type msg_type;
	enum ptp_port_state port_state;
	enum ptp_addr_type addr_type;	
	unsigned int two_step_flag;
};

struct ptp_msg_flag {
	char alt_master_flag:1;
	char two_step_flag:1;
	char unicast_flag:1;
	char ptp_prof_spec1:1;
	char ptp_prof_spec2:1;
	char reserved1:1;
	char leap61:1;
	char leap59:1;
	char cur_utc_offset_valid:1;
	char ptp_time_scale:1;
	char time_traceable:1;
	char freq_traceable:1;
	char reserved2:4;
};

union ptp_msg_flag_field{
	unsigned short flag;
	struct ptp_msg_flag ptp_flag;
};

typedef struct _IPHDR
{
    unsigned char    ucIpHdrLen:4;                 /* version */
    unsigned char    ucIpVer:4;
    unsigned char    ucServiceType;     /* type of service */
    unsigned short   usTotalLen;           /* total length */
    unsigned short   usIdentification;   /* identification */
    unsigned short   usOffset;               /* fragment offset field */
    unsigned char    ucTTL;                    /* time to live*/
    unsigned char    ucProtocol;            /* protocol */
    unsigned short   usCheckSum;        /* checksum */
    unsigned int   	ulSrcAddr;
    unsigned int   	ulDestAddr;          /* source and dest address */
}ETH_TEST_IPHDR_T;

#pragma pack(push)
#pragma pack(1)
typedef struct _IP_PACKET_FORMAT
{
    unsigned char          dstMacAddr[ETH_MAC_ADDR_LEN];
    unsigned char          srcMacAddr[ETH_MAC_ADDR_LEN];
    unsigned short         usType;

    /* IP头 */
    ETH_TEST_IPHDR_T     stSCTPHdr;

    /* UDP头 */
    unsigned short          srcPort;
    unsigned short          dstPort;
    unsigned short          udpLen;
    unsigned short          udpChecksum;

    //unsigned int          ulBody;
}ETH_TEST_IP_PACKET_FORMAT_T;

struct mac_hdr {
	/* MAC header*/
    unsigned char   dst_mac_addr[ETH_MAC_ADDR_LEN];
    unsigned char   src_mac_addr[ETH_MAC_ADDR_LEN];
    unsigned short  type;
};

struct ptp_hdr {
	unsigned char msg_type:4;
	unsigned char transport_specific:4;
	unsigned char ptp_version:4;
	unsigned char reserved0:4;
	unsigned short msg_len;

	unsigned char domain_num;
	unsigned char reserved1;
	unsigned short flags;
	
	unsigned int correction_field[2];
	unsigned int reserved2;
	unsigned int clockIdentity[2];	//part of src_port_identity
	unsigned short port_number;		//part of src_port_identity
	unsigned short sequence_id;
	unsigned char control_field;
	unsigned char log_msg_interval;
} ;
#pragma pack(pop)

extern netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev);
extern unsigned short stmmac_select_queue(struct net_device *dev,  struct sk_buff *skb);

/**************************Build and test IP Packet***************************/
unsigned short gmac_EthTestCRC16(unsigned char *pucData, 
												unsigned short usSize)
{
    unsigned int ulCheckSum  = 0;
    unsigned short *pBuffer = (unsigned short *)pucData;;

    while(usSize > 1)
    {
        ulCheckSum += *pBuffer++;
        usSize     -= sizeof(unsigned short);
    }

    if (usSize)
    {
        ulCheckSum += *(unsigned char*)pBuffer;
    }
    ulCheckSum  = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
    ulCheckSum += (ulCheckSum >> 16);

    return (unsigned short)(~ulCheckSum);
}

void gmac_EthTestBuildIpHdr(ETH_TEST_IP_PACKET_FORMAT_T *pstPkt, 
													unsigned int ulLen)
{
    static unsigned short usIdentification2 = 45557;
    unsigned int ulIpAddr;

    memcpy(pstPkt->dstMacAddr,g_aucDstMac, ETH_MAC_ADDR_LEN);
    memcpy(pstPkt->srcMacAddr,g_aucSrcMac, ETH_MAC_ADDR_LEN);

    /* 填写帧类型 */
    pstPkt->usType = IP_PAYLOAD;

    /* 填写IP头 */
    pstPkt->stSCTPHdr.ucIpVer          = 4;
    pstPkt->stSCTPHdr.ucIpHdrLen       = 5;
    pstPkt->stSCTPHdr.ucServiceType    = 0x10;
    pstPkt->stSCTPHdr.usTotalLen       = ETH_HTONS((unsigned short)ulLen + 
										sizeof(ETH_TEST_IPHDR_T)+ UDP_HDR_LEN);
    pstPkt->stSCTPHdr.usIdentification = ETH_HTONS(usIdentification2);
    usIdentification2++;
    pstPkt->stSCTPHdr.usOffset         = ETH_HTONS(IP_DF);
    pstPkt->stSCTPHdr.ucTTL            = 0xFF;
    pstPkt->stSCTPHdr.ucProtocol       = 17;    /*UDP*/
    pstPkt->stSCTPHdr.usCheckSum       = ETH_HTONS(0);

    memcpy((unsigned char*)(&ulIpAddr),g_aucDstIPAddr,4);
    pstPkt->stSCTPHdr.ulDestAddr = ulIpAddr;
    memcpy((unsigned char*)(&ulIpAddr),g_aucSrcIPAddr,4);
    pstPkt->stSCTPHdr.ulSrcAddr = ulIpAddr;

    pstPkt->stSCTPHdr.usCheckSum  = gmac_EthTestCRC16(
							(unsigned char *)&pstPkt->stSCTPHdr,IP_HDR_LEN);

    pstPkt->srcPort = ETH_HTONS(g_usSrcPort);
    pstPkt->dstPort = ETH_HTONS(g_usDstPort);

    pstPkt->udpLen  = ETH_HTONS((unsigned short)ulLen + UDP_HDR_LEN);
    pstPkt->udpChecksum = ETH_HTONS(0);

    return;
}

void gmac_EthTestBuildPkt(char* data, unsigned int payload, 
											unsigned char value)
{
    ETH_TEST_IP_PACKET_FORMAT_T *pstPkt;
    unsigned int ulHdrLen;

    if(NULL == data)
    {
        return ;
    }

    if (payload > 1472)
    {
        payload = 1472;
    }

    pstPkt = (ETH_TEST_IP_PACKET_FORMAT_T*)(data);
    gmac_EthTestBuildIpHdr(pstPkt, payload);
    ulHdrLen = ETH_MAC_HLEN + IP_HDR_LEN + UDP_HDR_LEN;

    if (value)
    {
        memset((data) + ulHdrLen, value, payload);
    }
}

int gmac_test_single_ip_pkt(int print_flag)
{
    int ret;
    struct sk_buff *skb;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null,return\n",__FUNCTION__));
        return TX_ERROR;
    }

	/* channel 0*/
	skb_set_queue_mapping(skb, TX_CHN_NET);
    gmac_EthTestBuildPkt(skb->data, PAYLOAD_LEN, 1);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    skb_put(skb, SKB_LEN);

	if (print_flag) {
		print_hex_dump(KERN_ERR, "stmmac_test:", DUMP_PREFIX_ADDRESS, \
			16, 1, skb->data, SKB_PRINT_LEN, true);
	}

	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
        GMAC_ERR(("%s:no packet to send,return\n",__FUNCTION__));
        return TX_ERROR;
	}  
	
    return TX_OK;
}

void gmac_test_ip_pkt(int pkt_num)
{
	int i;

	for (i = 0; i < pkt_num; i++) {
		
		/* send ip packet */
		gmac_test_single_ip_pkt(0);
	}
}

/****************************Build and test AV Packet**************************/
void gmac_build_class_pkt(char* data, unsigned int payload, int chn)
{	
	int payload_cnt = 0;
	int i;

	for (i = 0; i < 16; i++) {
		*data++ = gmac_hdrs[chn][i];
	}

	*data++ = avtype[0];
	*data++ = avtype[1];

	/* Add payload */
	while (payload_cnt < payload) {
		*data++ = gmac_payload_ch[chn];
		payload_cnt++;
	}			
}

int gmac_test_single_class_a_pkt(int print_flag)
{
	int ret;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, AV_SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null,return\n",__FUNCTION__));
        return TX_ERROR;
    }

	skb_set_queue_mapping(skb, TX_CHN_CLASS_A);
	skb_shinfo(skb)->tx_flags |= SKBTX_HW_TSTAMP;
	gmac_build_class_pkt(skb->data, AV_PAYLOAD_LEN, TX_CHN_CLASS_A);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    skb_put(skb, AV_SKB_LEN);

	if (print_flag) {
		print_hex_dump(KERN_ERR, "stmmac_test:", DUMP_PREFIX_ADDRESS, \
			16, 1, skb->data, SKB_PRINT_LEN, true);
	}

	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
        GMAC_ERR(("%s:no packet to send,return\n",__FUNCTION__));
        return TX_ERROR;
	}  
	
    return TX_OK;	
}

int gmac_test_single_class_b_pkt(int print_flag)
{
	int ret;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;

    skb = netdev_alloc_skb_ip_align(priv->dev, AV_SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null,return\n",__FUNCTION__));
        return TX_ERROR;
    }

	skb_set_queue_mapping(skb, TX_CHN_CLASS_B);	
	skb_shinfo(skb)->tx_flags |= SKBTX_HW_TSTAMP;
	gmac_build_class_pkt(skb->data, AV_PAYLOAD_LEN, TX_CHN_CLASS_B);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif
    
    skb_put(skb, AV_SKB_LEN);

	if (print_flag) {
		print_hex_dump(KERN_ERR, "stmmac_test:", DUMP_PREFIX_ADDRESS, \
			16, 1, skb->data, SKB_PRINT_LEN, true);
	}

	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
        GMAC_ERR(("%s:no packet to send,return\n",__FUNCTION__));
        return TX_ERROR;
	}  
	
    return TX_OK;	
}

void gmac_test_class_a_pkt(int pkt_num)
{
	int i;

	for (i = 0; i < pkt_num; i++) {
		gmac_test_single_class_a_pkt(0);
	}
}

void gmac_test_class_b_pkt(int pkt_num)
{
	int i;

	for (i = 0; i < pkt_num; i++) {
		gmac_test_single_class_b_pkt(0);
	}
}

/****************************Test Class Interval******************************/
void gmac_sub_test_cfg_tx_slotnum(int chn, union dma_desc *desc)
{
	int slot_num = 0;
	
	if (chn == TX_CHN_CLASS_A) {
		class_a_slot_num++;
		slot_num = class_a_slot_num & 0xf;
	} else if (chn == TX_CHN_CLASS_B) {
		class_b_slot_num += 2;
		slot_num = class_b_slot_num & 0xf;
	} else {
		GMAC_ERR(("[%s]wrong input:%d\n", __func__, chn));
		return;
	}

	desc->tx_desc.nrd.slot_num = slot_num;	
}

int gmac_sub_test_calc_interval(struct stmmac_time_spec *ts_in, u32 *interval, 
							unsigned int size, int mode)
{
	int ts_cnt;
	unsigned int interval_nsec;
	unsigned int interval_sec;
	struct stmmac_time_spec *ts_cur;
	struct stmmac_time_spec *ts_last;

	for (ts_cnt = 0; ts_cnt < size - 1; ts_cnt++) {
		ts_last = ts_in + ts_cnt;
		ts_cur = ts_in + ts_cnt + 1;

		if (ts_cur->sec > ts_last->sec) {
			interval_sec = ts_cur->sec - ts_last->sec;
			if (interval_sec != 1) {
				GMAC_ERR(("[%s]error!cur:%d, interval sec:%d\n", __func__, 
					ts_cur->sec, interval_sec));

				return -EINVAL;
			}

			if (!mode) {	//rollover
				interval_nsec = (0x7fffffff - ts_last->nsec) + ts_cur->nsec;
				interval_nsec = (interval_nsec * 46)/100;
			} else {
				interval_nsec = (1000000000 - ts_last->nsec) + ts_cur->nsec;
			}
			
		} else if (ts_cur->sec == ts_last->sec) {
			if (ts_cur->nsec > ts_last->nsec) {
				interval_nsec = ts_cur->nsec - ts_last->nsec;

				if (!mode) {	//rollover
					interval_nsec = (interval_nsec * 46)/100;
				}
				
			} else {
				GMAC_ERR(("[%s]%dth cur nsec:0x%x, last nsec:0x%x\n", __func__, ts_cnt,
					ts_cur->nsec, ts_last->nsec));

				return -EINVAL;
			}
		} else {
			GMAC_ERR(("[%s]error!cur:%d, last:%d\n", __func__, 
					ts_cur->sec, ts_last->sec));

			return -EINVAL;
		}

		interval[ts_cnt] = interval_nsec;
	}

	return 0;
}

int gmac_test_class_a_interval(void)
{
	int ret;
	int pkt_cnt;
	struct gmac_tx_queue *tx_queue;
	struct stmmac_priv *priv = gmac_priv;
	void (*cfg_slotnum)(int, union dma_desc *);
	spinlock_t tx_test_lock;
	unsigned long flags;
	union dma_desc *desc;
	unsigned int entry;
	unsigned int mode = 0;
	int class_interval;
	struct stmmac_time_spec *ts_cur;
#ifdef GMAC_TEST_CLEAN
	unsigned int widx_tx;
	unsigned int ridx_tx;
#endif

	if (!gmac_test_ts_a) {
		gmac_test_ts_a = (struct stmmac_time_spec *)kzalloc(GMAC_TEST_PKT_CNT * 
			sizeof(struct stmmac_time_spec), GFP_KERNEL);
		if (!gmac_test_ts_a) {
			ret = -ENOMEM; 
			return ret;
		}
	}

	ret = readl(priv->ioaddr + GMAC_TIMESTAMP_CONTROL);
	if (ret & GMAC_TCR_TSCTRLSSR) {
		mode = 1;	//Digital rollover mode
	} 

	stmmac_tx_clean(priv);		//make sure all of the desc are cleared;
	
	spin_lock_init(&tx_test_lock);
	spin_lock_irqsave(&tx_test_lock, flags);

	cfg_slotnum = priv->hw->desc->config_tx_slotnum;//save it
	priv->hw->desc->config_tx_slotnum = gmac_sub_test_cfg_tx_slotnum;
	
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_A);
	tx_queue->tx_coal_frames = 2 * STMMAC_TX_FRAMES; //be sure not clean tx
	mod_timer(&(tx_queue->txtimer), (unsigned long)(-1));//avoid to clear tx	
	
	/* Test Class A interval:125us */
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_A);
	for (pkt_cnt = 0; pkt_cnt < GMAC_TEST_PKT_CNT; pkt_cnt++) {
		ret = gmac_test_single_class_a_pkt(0);
		if (ret) {
			goto restore;
		}
	}

	mdelay(10);	//waiting for complete.
	pkt_cnt = 0;
#ifdef GMAC_TEST_CLEAN
	widx_tx = tx_queue->dirty_tx;
	ridx_tx = tx_queue->cur_tx;
	while(widx_tx != ridx_tx) {
		entry = widx_tx % (tx_queue->dma_tx_size);
		desc = tx_queue->dma_tx + entry;
		ts_cur = gmac_test_ts_a + pkt_cnt;
		priv->hw->desc->get_timestamp(desc, ts_cur);		
		GMAC_DBG(("[%s]class A ptr:%d idx:%dth ts sec:0x%x, ts nsec:0x%x\n", 
			__func__, widx_tx, pkt_cnt, ts_cur->sec, ts_cur->nsec));

		widx_tx++;
		pkt_cnt++;
	}
#else
	while(tx_queue->dirty_tx != tx_queue->cur_tx) {
		entry = tx_queue->dirty_tx % (tx_queue->dma_tx_size);
		desc = tx_queue->dma_tx + entry;
		ts_cur = gmac_test_ts_a + pkt_cnt;
		priv->hw->desc->get_timestamp(desc, ts_cur);		
		GMAC_DBG(("[%s]class A ptr:%d idx:%dth ts sec:0x%x, ts nsec:0x%x\n", 
			__func__, tx_queue->dirty_tx, pkt_cnt, ts_cur->sec, ts_cur->nsec));

		tx_queue->dirty_tx++;
		pkt_cnt++;
	}
#endif

	/* calculate interval */
	ret = gmac_sub_test_calc_interval(gmac_test_ts_a, ca_itv, 
							GMAC_TEST_PKT_CNT, mode);
	if (ret) {
		GMAC_ERR(("[%s]calc class a failed!(%d)\n", __func__, ret));
		goto restore;
	}

	/* printings */
	printk(KERN_ERR "********Class A interval********\n");
	for (pkt_cnt = 1; pkt_cnt < GMAC_TEST_PKT_CNT - 1; pkt_cnt++) { //the first may be out of range, so count with 1
		class_interval = ca_itv[pkt_cnt];
		if (JUDGE_INTERVAL_A(class_interval, 10)) {
			printk(KERN_ERR "%dth interval:%d\n", pkt_cnt, class_interval);
		} else {
			printk(KERN_ERR "[%s]error!Class A %dth interval:%d\n", __func__, 
				pkt_cnt, class_interval);
			ret = class_interval;
			goto restore;
		}
	}

restore:
	/* restore the timer */
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_A);
	tx_queue->tx_coal_frames = STMMAC_TX_FRAMES;	
	mod_timer(&(tx_queue->txtimer), jiffies + 
		usecs_to_jiffies(tx_queue->tx_coal_timer));	

	priv->hw->desc->config_tx_slotnum = cfg_slotnum;//restore the function

	spin_unlock_irqrestore(&tx_test_lock, flags);

	return ret;
}

int gmac_test_class_b_interval(void)
{
	int ret;
	int pkt_cnt;
	struct gmac_tx_queue *tx_queue;
	struct stmmac_priv *priv = gmac_priv;
	void (*cfg_slotnum)(int, union dma_desc *);
	spinlock_t tx_test_lock;
	unsigned long flags;
	union dma_desc *desc;
	unsigned int entry;
	unsigned int mode = 0;
	int class_interval;
	struct stmmac_time_spec *ts_cur;
#ifdef GMAC_TEST_CLEAN
	unsigned int widx_tx;
	unsigned int ridx_tx;
#endif

	if (!gmac_test_ts_b) {
		gmac_test_ts_b = (struct stmmac_time_spec *)kzalloc(GMAC_TEST_PKT_CNT * 
			sizeof(struct stmmac_time_spec), GFP_KERNEL);
		if (!gmac_test_ts_b) {
			ret = -ENOMEM; 
			return ret;
		}
	}

	ret = readl(priv->ioaddr + GMAC_TIMESTAMP_CONTROL);
	if (ret & GMAC_TCR_TSCTRLSSR) {
		mode = 1;	//Digital rollover mode
	} 

	stmmac_tx_clean(priv);		//make sure all of the desc are cleared;
	
	spin_lock_init(&tx_test_lock);
	spin_lock_irqsave(&tx_test_lock, flags);

	cfg_slotnum = priv->hw->desc->config_tx_slotnum;//save it
	priv->hw->desc->config_tx_slotnum = gmac_sub_test_cfg_tx_slotnum;
	
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_B);
	tx_queue->tx_coal_frames = 2 * STMMAC_TX_FRAMES; //be sure not clean tx
	mod_timer(&(tx_queue->txtimer), (unsigned long)(-1));//avoid to clear tx	

	/* Test Class B interval:250us */
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_B);
	for (pkt_cnt = 0; pkt_cnt < GMAC_TEST_PKT_CNT; pkt_cnt++) {
		ret = gmac_test_single_class_b_pkt(0);
		if (ret) {
			goto restore;
		}
	}

	mdelay(10);
	pkt_cnt = 0;
#ifdef GMAC_TEST_CLEAN
	widx_tx = tx_queue->dirty_tx;
	ridx_tx = tx_queue->cur_tx;
	while(widx_tx != ridx_tx) {
		entry = widx_tx % (tx_queue->dma_tx_size);
		desc = tx_queue->dma_tx + entry;
		ts_cur = gmac_test_ts_b + pkt_cnt;
		priv->hw->desc->get_timestamp(desc, ts_cur);
		GMAC_DBG(("[%s]class B ptr:%d idx:%dth ts sec:0x%x, ts nsec:0x%x\n", 
			__func__, widx_tx, pkt_cnt, ts_cur->sec, ts_cur->nsec));
		widx_tx++;
		pkt_cnt++;
	}
#else 
	while(tx_queue->dirty_tx != tx_queue->cur_tx) {
		entry = tx_queue->dirty_tx % (tx_queue->dma_tx_size);
		desc = tx_queue->dma_tx + entry;
		ts_cur = gmac_test_ts_b + pkt_cnt;
		priv->hw->desc->get_timestamp(desc, ts_cur);
		GMAC_DBG(("[%s]class B ptr:%d idx:%dth ts sec:0x%x, ts nsec:0x%x\n",
			__func__, tx_queue->dirty_tx, pkt_cnt, ts_cur->sec, ts_cur->nsec));
		tx_queue->dirty_tx++;
		pkt_cnt++;
	}
#endif

	/* calculate interval */
	ret = gmac_sub_test_calc_interval(gmac_test_ts_b, cb_itv, 
							GMAC_TEST_PKT_CNT, mode);
	if (ret) {
		GMAC_ERR(("[%s]calc class b failed!(%d)\n", __func__, ret));
		goto restore;
	}

	/* printings */
	printk(KERN_ERR "********Class B interval********\n");
	for (pkt_cnt = 1; pkt_cnt < GMAC_TEST_PKT_CNT - 1; pkt_cnt++) { //the first may be out of range, so count with 1
		class_interval = cb_itv[pkt_cnt];
		if (JUDGE_INTERVAL_B(class_interval, 20))  {
			printk(KERN_ERR "%dth interval:%d\n", pkt_cnt, class_interval);
		} else {
			printk(KERN_ERR "[%s]error!Class B %dth interval:%d\n", __func__, 
				pkt_cnt, class_interval);
			ret = class_interval;
			goto restore;
		}
	}

restore:
	/* restore the timer */
	tx_queue = GET_TX_QUEUE(priv, TX_CHN_CLASS_B);
	tx_queue->tx_coal_frames = STMMAC_TX_FRAMES;	
	mod_timer(&(tx_queue->txtimer), jiffies + usecs_to_jiffies(tx_queue->tx_coal_timer));	

	priv->hw->desc->config_tx_slotnum = cfg_slotnum;//restore the function

	spin_unlock_irqrestore(&tx_test_lock, flags);

	return ret;
}

/****************************Build and test PTP Packet**************************/

unsigned short gmac_get_ptp_msg_field(void)
{
	union ptp_msg_flag_field msg_flag;	
	
	/* stubed! */
	msg_flag.ptp_flag.alt_master_flag = 0;	//MASTER state
	msg_flag.ptp_flag.two_step_flag = 0;
	msg_flag.ptp_flag.unicast_flag = 1;
	msg_flag.ptp_flag.ptp_prof_spec1 = 0;
	msg_flag.ptp_flag.ptp_prof_spec2 = 0;
	msg_flag.ptp_flag.leap61 = 1;
	msg_flag.ptp_flag.leap59 = 1;
	msg_flag.ptp_flag.cur_utc_offset_valid = 1;
	msg_flag.ptp_flag.ptp_time_scale = 1;
	msg_flag.ptp_flag.time_traceable = 1;
	msg_flag.ptp_flag.freq_traceable = 1;

	return msg_flag.flag;
}

signed char gmac_get_ctl_field(enum ptp_msg_type msg_type)
{
	signed char ctrl_field = -1;
	
	switch (msg_type) {
		case PTP_SYNC:
			ctrl_field = CTL_SYNC;
			break;

		case PTP_DELAY_REQ:
			ctrl_field = CTL_DELAY_REQ;
			break;

		case PTP_FOLLOW_UP:
			ctrl_field = CTL_FOLLOW_UP;
			break;
			
		case PTP_DELAY_RESP:
			ctrl_field = CTL_DELAY_RESP;
			break;
			
		case PTP_MANAGEMENT:
			ctrl_field = CTL_MANAGEMENT;
			break;
			
		default:
			if (msg_type < PTP_MANAGEMENT) {
				ctrl_field = CTL_OTHERS;
			}
			break;
	}

	return ctrl_field;
}
void gmac_buld_ptp_hdr(char* data, int payload, enum ptp_msg_type msg_type)
{
	signed char ctl_field;
	struct mac_hdr *mac_header;
	struct ptp_hdr *ptp_header;

	mac_header = (struct mac_hdr *)data;
	ptp_header = (struct ptp_hdr *)(data + ETH_MAC_HLEN);

	/* dst and src mac addr */
	memcpy(mac_header->dst_mac_addr, ptp_dmac, ETH_MAC_ADDR_LEN);
    memcpy(mac_header->src_mac_addr, g_aucSrcMac, ETH_MAC_ADDR_LEN);
	
    /* type */
    mac_header->type = ETH_HTONS(PTP_TYPE);

    /* ptp header */
	ptp_header->transport_specific = 1 ;		//default:0,avb:1
	ptp_header->msg_type = msg_type;
	ptp_header->ptp_version = PTP_VERSION;		//IEEE 1588 V2
	ptp_header->msg_len = ETH_HTONS(PTP_HDR_LEN + ETH_MAC_HLEN + payload);
	ptp_header->domain_num = 0;					//0 in default
	ptp_header->flags = ETH_HTONS(gmac_get_ptp_msg_field());  //stubbed!
	memset(ptp_header->correction_field, 0, 2 * sizeof(unsigned int));
	memset(ptp_header->clockIdentity, 0, 2 * sizeof(unsigned int));
	ptp_header->port_number = ETH_HTONS(1); 	//supported port number is 1
	ptp_header->sequence_id = ETH_HTONS(msg_type + 1);	//stubbed!

	ctl_field = gmac_get_ctl_field(msg_type);
	if (ctl_field < 0){
		GMAC_ERR(("[%s]error input!(error no:%d)\n", __func__, ctl_field));
		return ;
	}
		
	ptp_header->control_field = (unsigned char)ctl_field;
	ptp_header->log_msg_interval = 0x7f;		//sync packt
}

void gmac_build_ptp_pkt(char* data, unsigned int payload, unsigned char value)
{
    unsigned int hdr_len;

    if(NULL == data)
    {
        return ;
    }

    if (payload > 1472)
    {
        payload = 1472;
    }

    gmac_buld_ptp_hdr(data, payload, PTP_PDELAY_REQ);
    hdr_len = ETH_MAC_HLEN + PTP_HDR_LEN;

    if (value)
    {
        memset((data) + hdr_len, value, payload);
    }
}
int gmac_send_single_ptp_pkt(int print_flag)
{
	int ret;
    struct sk_buff *skb;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null,return\n",__FUNCTION__));
        return TX_ERROR;
    }

	/* using internet channel */
	skb_set_queue_mapping(skb, TX_CHN_NET);
	skb_shinfo(skb)->tx_flags |= SKBTX_HW_TSTAMP;
	gmac_build_ptp_pkt(skb->data, 20, 4);
	skb_reset_mac_header(skb);
	skb_put(skb, SKB_LEN);

	if (print_flag) {
		print_hex_dump(KERN_ERR, "gmac_ptp>>", DUMP_PREFIX_NONE, \
			16, 1, skb->data, SKB_PRINT_LEN, false);
	}

	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
        GMAC_ERR(("%s:no packet to send(%d)\n",__func__, ret));
        return TX_ERROR;
	}  
	
    return TX_OK;
}

void gmac_test_ptp_pkt(int pkt_num)
{
	int i;

	for (i = 0; i < pkt_num; i++) {
		gmac_send_single_ptp_pkt(0);
	}
}
/************************************Test CBS***********************************/
void gmac_test_cbs(int num)
{
	int i;

	for (i = 0; i < num; i++) {
		gmac_test_ip_pkt(1);		//33%
		gmac_test_class_a_pkt(1);	//33%
		gmac_test_class_b_pkt(1);	//33%
	}
}

void gmac_test_cbs_ratio(int num, int ratio_ip, int ratio_audio, int ratio_video)
{
	int i;

	for (i = 0; i < num; i++) {
		gmac_test_ip_pkt(ratio_ip);//ratio_ip / (ratio_ip +ratio_audio+ratio_video)
		gmac_test_class_a_pkt(ratio_audio);	
		gmac_test_class_b_pkt(ratio_video);	
	}
}

int stmmac_cbs_get_class_id(unsigned int chn)

{
	int class_id;
	
	switch (chn) {
		case TX_CHN_CLASS_A:
			class_id = STMMAC_SRP_CLASS_A;
			break;

		case TX_CHN_CLASS_B:
			class_id = STMMAC_SRP_CLASS_B;
			break;

		default:
			class_id = -EINVAL;
			break;
	}

	return class_id;
}

int gmac_init_test_queue(void)
{
	int chn;
	struct gmac_test_queue *tq;
	
	if (!gmac_tq) {
		gmac_tq = kzalloc(TX_CHN_NR * sizeof(struct gmac_test_queue), GFP_KERNEL);
		if (!gmac_tq) {
			return -ENOMEM;
		}

		for (chn = 0; chn < TX_CHN_NR; chn++) {
			tq = gmac_tq + chn;
			tq->priv = gmac_priv;
		}
	}

	return 0;
}

int send_class_a(void)
{
	int ret;
	static unsigned long prev =  0;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, AV_SKB_LEN);
    if(NULL == skb)
    {
    	if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
	        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
    	}
		return -ENOMEM;
    }

	skb_set_queue_mapping(skb, TX_CHN_CLASS_A);
	//skb_shinfo(skb)->tx_flags |= SKBTX_HW_TSTAMP;
	gmac_build_class_pkt(skb->data, AV_PAYLOAD_LEN, TX_CHN_CLASS_A);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    skb_put(skb, AV_SKB_LEN);
	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
		if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
			GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
		}
		return ret;
	} 

	return 0;
}

int send_class_b(void)
{
	int ret;
	static unsigned long prev =  0;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, AV_SKB_LEN);
    if(NULL == skb)
    {
       	if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
	        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
       	}
		return -ENOMEM;
    }

	skb_set_queue_mapping(skb, TX_CHN_CLASS_B);
	//skb_shinfo(skb)->tx_flags |= SKBTX_HW_TSTAMP;
	gmac_build_class_pkt(skb->data, AV_PAYLOAD_LEN, TX_CHN_CLASS_B);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    skb_put(skb, AV_SKB_LEN);
	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
		if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
			GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
		}
		return ret;
	}  

	return 0;
}

int send_ip(void)
{
    int ret;
	static unsigned long prev =  0;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;
        
    skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb)
    {
      	if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
	        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
      	}
		return -ENOMEM;
    }

	/* channel 0*/
	skb_set_queue_mapping(skb, TX_CHN_NET);
    gmac_EthTestBuildPkt(skb->data, PAYLOAD_LEN, 1);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = NET_IP_ALIGN;
#else
	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

    skb_put(skb, SKB_LEN);
	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
		if (time_after(jiffies, prev + 2 * HZ)) {
			prev = jiffies;
			GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
		}
		return ret;
	}  

	return 0;
}
void send_class_a_pkt(unsigned long data)
{
	int i;
	int ret;
	struct gmac_test_queue *tq = (struct gmac_test_queue *)data;

	for (i = 0; i < tq->tx_num; i++) {
		ret = send_class_a();
		if (ret) {
			break;
		}
	}

	mod_timer(&(tq->timer), jiffies + usecs_to_jiffies(tq->usec));
}

void send_class_b_pkt(unsigned long data)
{
	int i;
	int ret;
	struct gmac_test_queue *tq = (struct gmac_test_queue *)data;

	for (i = 0; i < tq->tx_num; i++) {
		ret = send_class_b();
		if (ret) {
			break;
		}
	}

	mod_timer(&(tq->timer), jiffies + usecs_to_jiffies(tq->usec));
}

void send_ip_pkt(unsigned long data)
{
	int i;
	int ret;
	struct gmac_test_queue *tq = (struct gmac_test_queue *)data;

	for (i = 0; i < tq->tx_num; i++) {
		ret = send_ip();
		if (ret) {
			break;
		}
	}

	mod_timer(&(tq->timer), jiffies + usecs_to_jiffies(tq->usec));
}

static int gmac_send_class_a_pkt(struct gmac_test_queue *tq)
{
	init_timer(&(tq->timer));
	tq->usec = 19500;
	tq->tx_num = 1;
	tq->timer.data = (unsigned long)tq;
	tq->timer.function = send_class_a_pkt;
	tq->timer.expires = jiffies + usecs_to_jiffies(tq->usec);

	add_timer(&(tq->timer));	

	return 0;
}

static int gmac_send_class_b_pkt(struct gmac_test_queue *tq)
{
	init_timer(&(tq->timer));
	tq->usec = 19500;
	tq->tx_num = 1;
	tq->timer.data = (unsigned long)tq;
	tq->timer.function = send_class_b_pkt;
	tq->timer.expires = jiffies + usecs_to_jiffies(tq->usec);

	add_timer(&(tq->timer));	

	return 0;
}

static int gmac_send_ip_pkt(struct gmac_test_queue *tq)
{
	init_timer(&(tq->timer));
	tq->usec = 19500;
	tq->tx_num = 1;
	tq->timer.data = (unsigned long)tq;
	tq->timer.function = send_ip_pkt;
	tq->timer.expires = jiffies + usecs_to_jiffies(tq->usec);

	add_timer(&(tq->timer));	

	return 0;
}
int gmac_set_usec_limit(int limit)
{
	usec_limit = limit;

	return usec_limit;
}

/* rate >> format:Mbps */
int gmac_set_class_flow(int chn, unsigned int rate)
{
	int pkt_size;
	int work_done = 1;
	struct gmac_test_queue *tq;
	
	if (!gmac_tq) {
		return -ENOMEM;
	}

	if ((chn < TX_CHN_NET) || (chn > TX_CHN_CLASS_A)) {
		return -EINVAL;
	}
	
	if (chn == TX_CHN_NET) {
		pkt_size = SKB_LEN;
	} else {
		pkt_size = AV_SKB_LEN;
	}

	tq = gmac_tq + chn;
	if (!rate) {
		tq->tx_num = 0;
		goto print;
	}
	
	do {
		//8* 10^6 >> 17 -> 15625 >> 11 -> 15616 >> 11 -> 61 >> 3
		tq->usec = (((pkt_size * tq->tx_num) * 61)/rate) >> 3; 
		if (tq->usec < usec_limit) {
			tq->tx_num++;

		/* delta(usec) = const * delta(tx_num) 
		* delta(tx_num) = 1, so limit is usec + delta(usec),
		* approximate const = pktsize * 61/ (rate *8) ~~ pktsize * 8/rate
		*/
		} else if (tq->usec > (usec_limit + ((pkt_size * 8)/rate))) {
			tq->tx_num--;
		} else {
			work_done = 0;
		}
	} while(work_done);

	print:	
	printk(KERN_ERR "[chn%d], usec:%u, num:%u\n", chn, tq->usec, tq->tx_num);

	return 0;
}

int gmac_send_class_pkt(int chn)
{
	int ret;
	struct gmac_test_queue *tq;
	int (*send_pkt)(struct gmac_test_queue *tq);

	ret = gmac_init_test_queue();
	if (ret) {
		return ret;
	}

	if (chn == TX_CHN_CLASS_A) {
		send_pkt = gmac_send_class_a_pkt;
	} else if (chn == TX_CHN_CLASS_B) {
		send_pkt = gmac_send_class_b_pkt;
	} else if (chn == TX_CHN_NET) {
		send_pkt = gmac_send_ip_pkt;
	} else {
		printk(KERN_ERR "[%s]error input!(%d)\n", __func__, chn);
		return -EPERM;
	}

	tq = gmac_tq + chn;
	ret = send_pkt(tq);
	if (ret) {
		printk(KERN_ERR "[%s]create timer failed(%d, chn%d)!\n", __func__, ret, chn);
		return -EPERM;
	}

	return 0;
}



int gmac_close_send_chn(unsigned int chn)
{
	int ret;
	struct gmac_test_queue *tq;

	if (!gmac_tq) {
		return -EINVAL;
	}

	/* delete timer */
	tq = gmac_tq + chn;
	ret = del_timer(&(tq->timer));
	if (ret == 1) {
		printk(KERN_ERR "[%s]chn:%d,killed an active timer!\n", __func__, chn);
	} else if (!ret) {
		printk(KERN_ERR "[%s]chn:%d,killed an inactive timer!\n", __func__, chn);
	} else {
		return -1;
	}

	/* enable slot number compare */
	if ((chn == TX_CHN_CLASS_A) || (chn == TX_CHN_CLASS_B)) {
		tq->priv->hw->slot->config_slot_compare(tq->priv->ioaddr, chn, 1);
	}
	
	return 0;
}

int gmac_close_send(void)
{
	int ret;

	ret = gmac_init_test_queue();
	if (ret) {
		return ret;
	}

	/* close Class A */
	ret = gmac_close_send_chn(TX_CHN_CLASS_A);
	if (ret) {
		return ret;
	}
	
	/* close Class B */
	ret = gmac_close_send_chn(TX_CHN_CLASS_B);
	if (ret) {
		return ret;
	}

	/* close IP */
	ret = gmac_close_send_chn(TX_CHN_NET);
	if (ret) {
		return ret;
	}

	return 0;	
}

void gmac_get_credit(void) 
{
	int class_id;
	int i;
	int hicredit;
	int locredit;
	struct stmmac_priv * priv = gmac_priv;
	
	for (i = TX_CHN_CLASS_B; i < priv->tx_queue_cnt; i++) {
		class_id = stmmac_cbs_get_class_id(i);
		priv->hw->cbs->get_locredit(priv->dev, class_id, &locredit);
		priv->hw->cbs->get_hicredit(priv->dev, class_id, &hicredit);

		printk(KERN_ERR "[%s]chn%d, hicredit:0x%x, locredit:0x%x\n", __func__, 
			i, hicredit, locredit);
	}
}


void gmac_set_credit(int chn, u32 hi_size, u32 lo_size) 
{
	int hicredit;
	int locredit;
	struct stmmac_priv * priv = gmac_priv;

	if ((chn != TX_CHN_CLASS_A) && (chn != TX_CHN_CLASS_B)) {
		printk(KERN_ERR "[%s]wrong chn(%d)!\n", __func__, chn);
		return;
	}
		
	hicredit = (hi_size * 8) << 10;
	locredit = (lo_size * 8) << 10;
	locredit = - locredit;		//negative value
	priv->hw->cbs->config_high_credit(chn, priv->ioaddr, hicredit);
	priv->hw->cbs->config_low_credit(chn, priv->ioaddr, locredit);
}

int gmac_set_bw_chn(int chn, unsigned int req_bw)
{
	struct stmmac_priv *priv = gmac_priv;

	if ((chn != TX_CHN_CLASS_A) && (chn != TX_CHN_CLASS_B)) {
		GMAC_ERR(("[%s]input error chn(%d)\n!", __func__, chn));
		return -EINVAL;
	}

	if (req_bw % STMMAC_CBS_ROUND) {
		req_bw = STMMAC_CBS_ALIGN_DEC(req_bw, STMMAC_CBS_ROUND);
		printk(KERN_ERR "[%s]BW aligned to %dMbps!\n", __func__, req_bw);
	}
	
	stmmac_cbs_cfg_slope(priv->ioaddr, chn, req_bw, priv->line_speed);

	printk(KERN_ERR "[%s]Adjusted to %dMbps!\n", __func__, req_bw);

	return 0;
}

int gmac_get_bw_chn(int chn)
{
	int bw;
	struct stmmac_priv *priv = gmac_priv;
	
	bw = stmmac_cbs_get_bw(priv->ioaddr, chn, priv->line_speed);
	if (bw < 0) {
		return bw;
	}
	
	printk(KERN_ERR "[%s]chn%d's bandwidth:%dMbps!\n", __func__, chn, bw);

	return 0;
}

int gmac_test_cbs_algo(void)
{
	struct stmmac_priv *priv;
	int ret;
	int chn;
	u32 max_frame_size;
	struct gmac_test_queue *tq;

	ret = gmac_init_test_queue();
	if (ret) {
		return ret;
	}
	
	tq = gmac_tq;
	priv = tq->priv;

	max_frame_size = VLAN_ETH_FRAME_LEN;
	for (chn = TX_CHN_CLASS_B; chn < priv->tx_queue_cnt; chn++) {
		gmac_set_credit(chn, max_frame_size, max_frame_size);

	}

	gmac_get_credit();
	
	ret = gmac_send_class_a_pkt(tq + TX_CHN_CLASS_A);
	if (ret) {
		printk(KERN_ERR "[%s]create Class A timer failed!\n", __func__);
		return -EPERM;
	}

	ret = gmac_send_class_b_pkt(tq + TX_CHN_CLASS_B);
	if (ret) {
		printk(KERN_ERR "[%s]create Class B timer failed!\n", __func__);
		return -EPERM;
	}

	ret = gmac_send_ip_pkt(tq + TX_CHN_NET);
	if (ret) {
		printk(KERN_ERR "[%s]create IP timer failed!\n", __func__);
		return -EPERM;
	}

	/* disable slot number compare */
	priv->hw->slot->config_slot_compare(priv->ioaddr, TX_CHN_CLASS_A, 0);
	priv->hw->slot->config_slot_compare(priv->ioaddr, TX_CHN_CLASS_B, 0);

	return 0;
}

/********************************Test system time*******************************/
void gmac_delete_test_timer(void)
{
	del_timer(&gmac_test_timer);
}

void gmac_enable_test_timer_continue(int enable)
{
	gmac_timer_continue = enable;
}

void gmac_calc_sys_time(unsigned long data)
{
	static int time_cnt = 0;	
	
	gmac_enable_test_timer_continue(1);
	gmac_current_time.seconds = readl(gmac_priv->ioaddr + 
									GMAC_SYSTEM_TIME_SECONDS);
	gmac_current_time.nano_seconds = readl(gmac_priv->ioaddr + 
									GMAC_SYSTEM_TIME_NANOSECONDS);

	gmac_time_itv[time_cnt].seconds =  gmac_current_time.seconds - gmac_last_time.seconds;
	gmac_time_itv[time_cnt].nano_seconds = gmac_current_time.nano_seconds - gmac_last_time.nano_seconds;
	
	gmac_last_time.seconds = gmac_current_time.seconds;
	gmac_last_time.nano_seconds = gmac_current_time.nano_seconds;

	if (time_cnt >= GMAC_TEST_TIME_CNT - 1) {
		int cnt;
		
		gmac_enable_test_timer_continue(0);
		gmac_delete_test_timer();
		time_cnt = 0;
		
		for (cnt = 0; cnt < GMAC_TEST_TIME_CNT; cnt++) {
			
			printk("[time itv]%dth seconds:%d, nano seconds:%d\n", cnt, 
					gmac_time_itv[cnt].seconds, gmac_time_itv[cnt].nano_seconds);			
			if (gmac_time_itv[cnt].seconds != 1) {
				sys_time_flag = -1;
				return;
			}
		}

		sys_time_flag = 1;
		return;
	}
	
	time_cnt++;
	if (gmac_timer_continue) {
		mod_timer(&gmac_test_timer, jiffies + HZ);
	}
}

void gmac_sub_test_sys_timer(void)
{	
	init_timer(&gmac_test_timer);
	gmac_test_timer.expires = jiffies + HZ;
	gmac_test_timer.data = (unsigned long)gmac_priv;
	gmac_test_timer.function = gmac_calc_sys_time;

	gmac_last_time.seconds = readl(gmac_priv->ioaddr + GMAC_SYSTEM_TIME_SECONDS);
	gmac_last_time.nano_seconds = readl(gmac_priv->ioaddr + 
										GMAC_SYSTEM_TIME_NANOSECONDS);	
	add_timer(&gmac_test_timer);		
}

int gmac_sub_test_get_sys_time(struct sys_time *time)
{
	time->seconds = readl(gmac_priv->ioaddr + GMAC_SYSTEM_TIME_SECONDS);
	time->nano_seconds = readl(gmac_priv->ioaddr + GMAC_SYSTEM_TIME_NANOSECONDS);

	printk("[system time]seconds:%d, nano_seconds:%d\n", time->seconds, time->nano_seconds);

	return 0;
}

int gmac_test_systime(void)
{
	int ret = 0;
	struct sys_time cur_time;

	gmac_sub_test_get_sys_time(&cur_time);
	if (!cur_time.seconds) {
		return -EIO;
	}

	gmac_sub_test_sys_timer();

	while (!sys_time_flag) {
		msleep(10);
	}

	if (sys_time_flag <= 0) {
		ret = -1;
	} else {
		ret = 0;
	}
	
	sys_time_flag = 0;
	
	return ret;
}
/*******************************Test PHY interface******************************/

int gmac_test_phy_write(void)
{
	int ret = -1;
	u16 value;

	/* PHY ID fixed to 1 */
	value = (u16)stmmac_phy_read(1, 0);
	ret = stmmac_phy_write(1, 0, value);

	return ret;
}

int gmac_test_phy_read(void)
{
	int ret = -1;
	int value;

	/* PHY ID fixed to 1 */
	value = stmmac_phy_read(1, 0);
	if (value >= 0) {
		ret = 0;
	}

	return ret;
}

/*******************************Test Xmit interface******************************/

int gmac_sub_test_xmit(int mac_err, int chn_err, int spe_own) 
{
    int ret;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;
	
	/* error test:error chn test */
	skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
		return -ENOMEM;
    }
	
    gmac_EthTestBuildPkt(skb->data, PAYLOAD_LEN, 1);
	
	if (!mac_err) {
		skb_reset_mac_header(skb);
	}

	if (chn_err) {
		skb_set_queue_mapping(skb, 5);		//error chn:5
	}else {
		skb_set_queue_mapping(skb, TX_CHN_NET);	
	}

#ifdef CONFIG_BALONG_SPE
	skb->spe_own = spe_own;
#endif

    skb_put(skb, SKB_LEN);
	priv->xstats.tx_chn_err = 0;
	priv->xstats.tx_mac_header_err = 0;
	
	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
		GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
		return ret;
	}

	return 0;
}
int gmac_test_xmit(void)
{
	int ret;
	struct stmmac_priv *priv = gmac_priv;

	/* Normal test without spe 
	 * mac_err = 0;
	 * chn_err = 0;
	 * spe_own = 0;
	*/
	ret = gmac_sub_test_xmit(0, 0, 0);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* mac err test without spe 
	 * mac_err = 1;
	 * chn_err = 0;
	 * spe_own = 0;
	*/
	ret = gmac_sub_test_xmit(1, 0, 0);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* It should be error,if not, failed.*/
	if (!(priv->xstats.tx_mac_header_err)) {
		GMAC_ERR(("[%s]mac err test without spe failed!", __func__));
		return -EPERM;
	}

	/* chn err test without spe 
	 * mac_err = 0;
	 * chn_err = 1;
	 * spe_own = 0;
	*/
	ret = gmac_sub_test_xmit(0, 1, 0);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* It should be error,if not, failed.*/
	if (!(priv->xstats.tx_chn_err)) {
		GMAC_ERR(("[%s]chn err test without spe failed!", __func__));
		return -ENOENT;
	}

	/* normal test with spe 
	 * mac_err = 0;
	 * chn_err = 0;
	 * spe_own = 1;
	*/
	ret = gmac_sub_test_xmit(0, 0, 1);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* mac err test with spe 
	 * mac_err = 1;
	 * chn_err = 0;
	 * spe_own = 1;
	*/
	ret = gmac_sub_test_xmit(1, 0, 1);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* if enter other branch, failed.*/
	if (priv->xstats.tx_chn_err) {
		GMAC_ERR(("[%s]mac err test with spe failed!", __func__));
		return -ENOENT;
	}

	/* chn err test with spe 
	 * mac_err = 0;
	 * chn_err = 1;
	 * spe_own = 1;
	*/
	ret = gmac_sub_test_xmit(0, 1, 1);
	if (ret) {
		GMAC_ERR(("[%s]line:%d, test failed!(%d)\n", __func__, __LINE__, ret));
		return ret;	
	}

	/* if enter other branch, failed.*/
	if (priv->xstats.tx_chn_err) {
		GMAC_ERR(("[%s]chn err test with spe failed!", __func__));
		return -ENOENT;
	}

	return 0;	
}

#ifdef BSP_CONFIG_BOARD_TELEMATIC
#else
void gmac_test_help(void)

{
    int ret;
    struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;

    /* error test:error chn test */
    skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb)
    {
        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
        return -ENOMEM;
    }

    gmac_EthTestBuildPkt(skb->data, PAYLOAD_LEN, 1);

    if (!mac_err) {
        skb_reset_mac_header(skb);
    }

    if (chn_err) {
        skb_set_queue_mapping(skb, 5);        //error chn:5
    }else {
        skb_set_queue_mapping(skb, TX_CHN_NET);
    }

#ifdef CONFIG_BALONG_SPE
    skb->spe_own = spe_own;
#endif

    skb_put(skb, SKB_LEN);
    priv->xstats.tx_chn_err = 0;
    priv->xstats.tx_mac_header_err = 0;

    ret = stmmac_xmit(skb, priv->dev);
    if (ret){
        dev_kfree_skb_any(skb);
        GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
        return ret;
    }

    return 0;
}
#endif

void modify_av_pkt(unsigned int offset, unsigned char value)
{
	unsigned char *ptr;

	ptr = av_data + offset;
	*ptr = value;
}

void print_av_pkt_hdr(void)
{
	print_hex_dump(KERN_ERR, "av_pkt_hdr:", DUMP_PREFIX_OFFSET,
       16, 1, (void *)av_data, 48, false);
}

void build_av_pkt(unsigned char *data, unsigned int len, char value)
{
	memcpy((void *)data, av_data, 48);
	memset((void *)(data + 48), value, len - 48);	
}

int gmac_test_select_queue(void)
{
    int ret;
	int chn;
	struct sk_buff *skb = NULL;
    struct stmmac_priv *priv =  gmac_priv;

	skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
    if(NULL == skb) {
        GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
		return -ENOMEM;
    }
	
    build_av_pkt(skb->data, PAYLOAD_LEN, 1);
	
	skb_reset_mac_header(skb);
	chn = stmmac_select_queue(priv->dev, skb);
	skb_set_queue_mapping(skb, chn);
	GMAC_ERR(("[%s]:chn(%d)\n", __func__, chn));

    skb_put(skb, SKB_LEN);
	
	ret = stmmac_xmit(skb, priv->dev);
    if (ret){
		dev_kfree_skb_any(skb);
		GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
	}

	return ret;
}

int gmac_test_cbs_get_preamble_ipg(void)
{
	union cbs_mac_para igb_pre;
	int ret;

	/* normal test */
	ret = stmmac_get_preamble_ipg(gmac_priv->dev, &(igb_pre.value));
	if (ret) {
		printk("[%s]ret:%d\n", __func__, ret);
		return ret;
	}

	printk("[%s]inter packt gap:%u, preamble:%u\n", __func__, igb_pre.ipg_preamble.ipg, 
								igb_pre.ipg_preamble.preamble);

	/* error test */
	ret = stmmac_get_preamble_ipg(NULL, &(igb_pre.value));
	if (!ret) {
		printk("[%s]error test1 failed!\n", __func__);
		return -EACCES;
	}

	ret = stmmac_get_preamble_ipg(gmac_priv->dev, NULL);
	if (!ret) {
		printk("[%s]error test2 failed!\n", __func__);
		return -EACCES;
	}

	ret = stmmac_get_preamble_ipg(NULL, NULL);
	if (!ret) {
		printk("[%s]error test3 failed!\n", __func__);
		return -EACCES;
	}

	return 0;	
}

int gmac_test_cbs_config_bw(void)
{
	int ret;

	/* normal test */
	ret = stmmac_cbs_config_bw(gmac_priv->dev, CBS_REQUEST_BW, 10, STMMAC_SRP_CLASS_A);
	if (ret < 0) {
		printk("[%s]ret:%d\n", __func__, ret);
		return ret;
	}

	/* error test, dev is NULL */
	ret = stmmac_cbs_config_bw(NULL, CBS_REQUEST_BW, 10, STMMAC_SRP_CLASS_A);
	if (ret >= 0) {
		printk("[%s]error test1 failed!\n", __func__);
		return -EACCES;
	}

	/* error test, action is 10 */
	ret = stmmac_cbs_config_bw(gmac_priv->dev, 10, 10, STMMAC_SRP_CLASS_A);
	if (ret >= 0) {
		printk("[%s]error test2 failed!\n", __func__);
		return -EACCES;
	}

	/* error test, bw is exceed the limit */
	ret = stmmac_cbs_config_bw(gmac_priv->dev, CBS_REQUEST_BW, 
							(gmac_priv->line_speed) * 2, STMMAC_SRP_CLASS_A);
	if (ret >= 0) {
		printk("[%s]error test3 failed!\n", __func__);
		return -EACCES;
	}

	/* error test, bw is exceed the limit */
	ret = stmmac_cbs_config_bw(gmac_priv->dev, CBS_RELEASE_BW, 
							(gmac_priv->line_speed) * 2, STMMAC_SRP_CLASS_A);
	if (ret < 0) {//return value isn't a negative value
		printk("[%s]error test4 failed!\n", __func__);
		return -EACCES;
	}

	/* error test, class_id is error */
	ret = stmmac_cbs_config_bw(gmac_priv->dev, CBS_REQUEST_BW, 10, STMMAC_SRP_CLASS_A * 2);
	if (ret >= 0) {
		printk("[%s]error test5 failed!\n", __func__);
		return -EACCES;
	}

	return 0;	
}

int gmac_test_cbs_get_class_bw(void)
{
	int ret;

	/* normal test */
	ret = stmmac_cbs_get_class_bw(gmac_priv->dev, STMMAC_SRP_CLASS_A);
	if (ret < 0) {
		printk("[%s]class A,ret:%d\n", __func__, ret);
		return ret;
	} else {
		printk("[%s]class:%d, bandwidth:%d\n", __func__, STMMAC_SRP_CLASS_A, ret);
	}

	ret = stmmac_cbs_get_class_bw(gmac_priv->dev, STMMAC_SRP_CLASS_B);
	if (ret < 0) {
		printk("[%s]class B,ret:%d\n", __func__, ret);
		return ret;
	} else {
		printk("[%s]class:%d, bandwidth:%d\n", __func__, STMMAC_SRP_CLASS_B, ret);
	}

	/* error test */
	ret = stmmac_cbs_get_class_bw(NULL, STMMAC_SRP_CLASS_B);
	if (ret >= 0) {
		printk("[%s]error test1 failed!\n", __func__);
		return -EACCES;
	}

	ret = stmmac_cbs_get_class_bw(gmac_priv->dev, STMMAC_SRP_CLASS_B * 2);
	if (ret >= 0) {
		printk("[%s]error test2 failed!\n", __func__);
		return -EACCES;
	}

	return 0;	
}

int gmac_test_ptp_adjfreq(int neg_flag, int ppb)
{
	int ret;
	struct ptp_clock_info *gmac_ptp;
	int (*adj_freq)(struct ptp_clock_info *ptp, s32 delta);
	struct sys_time old_time, new_time;
	int sec_delta, nsec_delta;
	
	adj_freq = gmac_priv->ptp_clock_ops.adjfreq;
	gmac_ptp = &(gmac_priv->ptp_clock_ops);

	if (neg_flag) {
		ppb = -ppb;
	}
	
	ret = adj_freq(gmac_ptp, ppb);
	if (ret) {
		printk("[%s]test failed:%d\n", __func__, ret);
		return ret;
	}

	gmac_sub_test_get_sys_time(&old_time);
	ssleep(1);
	gmac_sub_test_get_sys_time(&new_time);

	sec_delta = new_time.seconds - old_time.seconds;
	nsec_delta = new_time.nano_seconds - old_time.nano_seconds;

	printk("[%s]delta sec:%d, delta nsec:%d, addend:0x%x\n", __func__, 
		sec_delta, nsec_delta, gmac_priv->default_addend);

	return 0;
}

int gmac_test_ptp_adjtime(int neg_flag,int delta_sec, int delta_nsec)
{
	int ret;
	struct ptp_clock_info *gmac_ptp;
	int (*adj_time)(struct ptp_clock_info *ptp, s64 delta);

	struct sys_time old_time, new_time;
	int sec_delta, nsec_delta;
	s64 delta;

	adj_time = gmac_priv->ptp_clock_ops.adjtime;
	gmac_ptp = &(gmac_priv->ptp_clock_ops);

	delta = ((s64)delta_sec) * 1000000000ULL + (s64)delta_nsec;
	if (neg_flag) {
		delta = -delta;
	}

	printk("[%s]delta:%lld, sec:%d, nsec:%d\n", __func__, delta, delta_sec, 
		delta_nsec);
	
	gmac_sub_test_get_sys_time(&old_time);
	ret = adj_time(gmac_ptp, delta);

	if (ret) {
		printk("[%s]test failed:%d\n", __func__, ret);
		return ret;
	}
	
	gmac_sub_test_get_sys_time(&new_time);

	sec_delta = new_time.seconds - old_time.seconds;
	nsec_delta = new_time.nano_seconds - old_time.nano_seconds;

	printk("[%s]delta sec:%d, delta nsec:%d\n", __func__, sec_delta, nsec_delta);

	return 0;
	
}

int gmac_test_ptp_gettime(void)
{
	int ret;
	struct ptp_clock_info *gmac_ptp;
	int (*get_time)(struct ptp_clock_info *ptp, struct timespec *ts);
	struct timespec time_spec;
	
	get_time = gmac_priv->ptp_clock_ops.gettime;
	gmac_ptp = &(gmac_priv->ptp_clock_ops);
	
	ret = get_time(gmac_ptp, &time_spec);
	if (ret) {
		printk("[%s]test failed:%d\n", __func__, ret);
		return ret;
	}

	printk("[%s]sec:%ld, nsec:%ld\n", __func__, time_spec.tv_sec, time_spec.tv_nsec);

	return 0;
}

int gmac_test_ptp_settime(unsigned int sec, unsigned int nsec)
{
	int ret;
	struct ptp_clock_info *gmac_ptp;
	int (*set_time)(struct ptp_clock_info *ptp, const struct timespec *ts);
	struct timespec time_spec;
	struct sys_time current_time;
	
	set_time = gmac_priv->ptp_clock_ops.settime;
	gmac_ptp = &(gmac_priv->ptp_clock_ops); 

	time_spec.tv_sec = (__kernel_time_t)sec;
	time_spec.tv_nsec = (long)nsec;

	gmac_sub_test_get_sys_time(&current_time);
	ret = set_time(gmac_ptp, &time_spec);
	if (ret) {
		printk("[%s]test failed:%d\n", __func__, ret);
		return ret;
	}

	gmac_sub_test_get_sys_time(&current_time);

	return 0;	

}

/***************************phy loopback test**********************************/
int loopback_compare_pkt(struct sk_buff *send_skb, struct sk_buff *recv_skb)
{
	int ret;
	
	if (send_skb->len != recv_skb->len) {
		GMAC_ERR(("[%s]length error!\n", __func__));
		return -EINVAL;
	}
	
	ret = memcmp((void *)(send_skb->data), (void *)(recv_skb->data), send_skb->len);
	if (ret) {
		GMAC_ERR(("[%s]compared failed!\n", __func__));
		return ret;
	}
	
	return 0;
}

int loopback_send_pkt(struct sk_buff **send_skb)
{
 	int ret;
 	static unsigned long prev =  0;
 	struct sk_buff *skb = NULL;
 	struct stmmac_priv *priv =  gmac_priv;
	netdev_tx_t (*send_pkt)(struct sk_buff *skb, struct net_device *ndev);

	send_pkt = priv->dev->netdev_ops->ndo_start_xmit;
 	skb = netdev_alloc_skb_ip_align(priv->dev, SKB_LEN);
 	if(NULL == skb){
	 	if (time_after(jiffies, prev + 2 * HZ)) {
		 	prev = jiffies;
		 	GMAC_ERR(("%s:skb is null\n",__FUNCTION__));
	 	}
	 	return -ENOMEM;
 	}

 	/* channel 0*/
 	skb_set_queue_mapping(skb, TX_CHN_NET);
 	gmac_EthTestBuildPkt(skb->data, PAYLOAD_LEN, 1);

#ifdef NET_SKBUFF_DATA_USES_OFFSET
 	skb->mac_header = NET_IP_ALIGN;
#else
 	skb->mac_header = skb->head + NET_IP_ALIGN;
#endif

 	skb_put(skb, SKB_LEN);
 	*send_skb = skb_copy(skb, GFP_ATOMIC);
 	if (!(*send_skb)) {
	 	dev_kfree_skb_any(skb);
	 	GMAC_ERR(("Copy send skb failed!\n"));
	 	return -ENOMEM;
 	}

 	ret = send_pkt(skb, priv->dev);
 	if (ret){
	 	dev_kfree_skb_any(skb);
	 	if (time_after(jiffies, prev + 2 * HZ)) {
		 	prev = jiffies;
		 	GMAC_ERR(("[%s]:no packet to send(%d)\n",__FUNCTION__, ret));
	 	}
	 	return ret;
 	}	

 	return 0;
}

int gmac_phy_loop_test(void)
{
	int ret, reg_val, limit, phy_ret;
	unsigned int reg_backup_mac, reg_backup_phy;
	unsigned int pkt_cnt_before;
	struct sk_buff *src_skb = NULL;
	struct stmmac_priv *priv = gmac_priv;

	if (!(priv->is_open)) {
		GMAC_ERR(("Gmac has not opened!\n"));
		return -EPERM;
	}
 
	mdelay(100);	 //delay to be stable.

	/* change gmac filter configuration */
	reg_backup_mac = readl(priv->ioaddr + GMAC_PKT_FILTER);
	writel(0x1, priv->ioaddr + GMAC_PKT_FILTER);	 //promisc mode  

	/* Set phy loopback mode */
	reg_backup_phy = stmmac_phy_read(RGMII_PHY_ADDR,RGMII_COPPER_CTL);
	reg_val = reg_backup_phy | RGMII_PHY_LOOPBACK;
	ret = stmmac_phy_write(RGMII_PHY_ADDR, RGMII_COPPER_CTL, reg_val);
	if (ret) {
		return ret;
	}

	priv->phy_loopback = 1;

	/* test phy loop */
	pkt_cnt_before = priv->recv_pkt;

	/* send packet */
	ret = loopback_send_pkt(&src_skb);
	if (ret) {
		GMAC_ERR(("loopback_send_pkt failed!\n"));
		goto clear_and_end;
	}

	/* Waiting for completion. */
	limit = 100;
	while ((limit--) && (priv->recv_pkt <= pkt_cnt_before)) {
		mdelay(10);
	}
	if (limit < 0) {
		GMAC_ERR(("timeout!\n"));
		ret = limit;
		goto clear_and_end;
	}
 
 	/* Check if it received one packet */
 	if (priv->recv_pkt != (pkt_cnt_before + 1)) {
	 	GMAC_ERR(("Sending packet in chaos!\n"));
	 	ret = -EBUSY;
	 	goto clear_and_end;
 	}

 	printk("before:%d, after:%d\n", pkt_cnt_before, priv->recv_pkt);
 	ret = loopback_compare_pkt(src_skb, priv->recv_skb);
 	if (priv->recv_skb) {
	 	dev_kfree_skb_any(priv->recv_skb);
	 	priv->recv_skb = NULL;
 	}
 
 	if (ret) {
	 	GMAC_ERR(("The wrong packet!\n"));
	 	priv->phy_loopback = 0;
 	} else {
		printk("compare succeed!\n");
	}

clear_and_end:
 	/*free skb*/
 	if (src_skb) {
	 	dev_kfree_skb_any(src_skb); 
	 	src_skb = NULL; 
 	}

 	if (priv->recv_skb) {
	 	dev_kfree_skb_any(priv->recv_skb);
	 	priv->recv_skb = NULL;
 	}
 
 	priv->phy_loopback = 0;

 	/* restore mac mode */
 	writel(reg_backup_mac, priv->ioaddr + GMAC_PKT_FILTER);

	/* restore phy mode */
 	phy_ret = stmmac_phy_write(RGMII_PHY_ADDR, RGMII_COPPER_CTL, reg_backup_phy);
 	if (phy_ret) {
	 	GMAC_ERR(("restore phy mode failed!\n"));
	 	return phy_ret;
 	}
 
 	return ret;
}


int gmac_change_tx_coal_frames(unsigned int chn, unsigned int value)
{
	struct gmac_tx_queue *tx_queue = NULL;
	struct stmmac_priv *priv = gmac_priv;

	tx_queue = GET_TX_QUEUE(priv, chn);
	printk("before:chn%d, tx_coal_frames:%d\n", chn, tx_queue->tx_coal_frames);
	tx_queue->tx_coal_frames = value;
	printk("after:chn%d, tx_coal_frames:%d\n", chn, tx_queue->tx_coal_frames);

	return 0;
}
