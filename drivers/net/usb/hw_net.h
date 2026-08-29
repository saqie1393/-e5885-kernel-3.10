
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include "mdrv_mbb_channel.h"
#include "mbb_common.h"

void mbb_usbnet_rx_set_mac_clone(struct sk_buff* skb);
void mbb_usbnet_tx_set_mac_clone(struct sk_buff* skb);
void mbb_usbnet_set_net_state(CRADLE_EVENT state);
void mbb_usbnet_set_last_net_state(CRADLE_EVENT state);
void mbb_usbnet_set_usb_state(GPIO_USB_EVENT state);
void mbb_usbnet_net_state_notify(CRADLE_EVENT status);
void mbb_usbnet_usb_state_notify(GPIO_USB_EVENT eventcode);
void mbb_usbnet_set_speed(CRADLE_SPEED speed);
void mbb_usbnet_set_cradle_type(CRADLE_TYPE type);
void mbb_usbnet_set_fty_cradle_state(GPIO_USB_EVENT state);