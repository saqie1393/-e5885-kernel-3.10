
#ifndef _WLAN_SECURITY_H_
#define _WLAN_SECURITY_H_

/*===========================================================================
                       linux系统头文件
===========================================================================*/
#include <linux/skbuff.h>
#include <linux/netdevice.h>

/*****************************************************************************
 函数名称  : wlan_check_arp_spoofing
 功能描述  : 检测wlan的arp欺骗
 输入参数  : port_dev: 桥下挂设备, pskb: 需要检测的数据包
 输出参数  : NA
 返 回 值  : 0:不是ARP欺骗报文；1: 是ARP欺骗报文
*****************************************************************************/
int wlan_check_arp_spoofing(struct net_device *port_dev, struct sk_buff *pskb);

#endif /* _WLAN_SECURITY_H_ */
