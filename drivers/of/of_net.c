/*
 * OF helpers for network devices.
 *
 * This file is released under the GPLv2
 *
 * Initially copied out of arch/powerpc/kernel/prom_parse.c
 */


#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/export.h>
#ifdef BSP_CONFIG_BOARD_TELEMATIC
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "bsp_nvim.h"
#include "bsp_sram.h"

#define MULTIPINS_RGMII   (2)
#define MULTIPINS_MII    (3)
#endif /* BSP_CONFIG_BOARD_TELEMATIC */

/**
 * It maps 'enum phy_interface_t' found in include/linux/phy.h
 * into the device tree binding of 'phy-mode', so that Ethernet
 * device driver can get phy interface from device tree.
 */
static const char *phy_modes[] = {
	[PHY_INTERFACE_MODE_NA]		= "",
	[PHY_INTERFACE_MODE_MII]	= "mii",
	[PHY_INTERFACE_MODE_GMII]	= "gmii",
	[PHY_INTERFACE_MODE_SGMII]	= "sgmii",
	[PHY_INTERFACE_MODE_TBI]	= "tbi",
	[PHY_INTERFACE_MODE_RMII]	= "rmii",
	[PHY_INTERFACE_MODE_RGMII]	= "rgmii",
	[PHY_INTERFACE_MODE_RGMII_ID]	= "rgmii-id",
	[PHY_INTERFACE_MODE_RGMII_RXID]	= "rgmii-rxid",
	[PHY_INTERFACE_MODE_RGMII_TXID] = "rgmii-txid",
	[PHY_INTERFACE_MODE_RTBI]	= "rtbi",
	[PHY_INTERFACE_MODE_SMII]	= "smii",
};

/**
 * of_get_phy_mode - Get phy mode for given device_node
 * @np:	Pointer to the given device_node
 *
 * The function gets phy interface string from property 'phy-mode',
 * and return its index in phy_modes table, or errno in error case.
 */
const int of_get_phy_mode(struct device_node *np)
{
	const char *pm;
	int err, i;

/*车载模块phy_mode 不通过dts 获取, 其中烧片版本通过共享内存
*获取，升级版本通过nv 获取
*/
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    int phy_interface = PHY_INTERFACE_MODE_NA;
    NV_MULTIPINS_TYPE MultiPinsPara = {0};

#if (FEATURE_ON == MBB_FACTORY)
    multipins_share *multipins_data = NULL;
    multipins_data = (multipins_share *)SARM_MULTIPINS_ADDR;
    if (MULTIPINS_STATUS == multipins_data->MULTIPINS_FLAG)
    {
        MultiPinsPara.RGMII_MII = multipins_data->RGMII_MII;
    }
    else
    {
        MultiPinsPara.RGMII_MII = MULTIPINS_MII;
    }
#else
    err = NV_ReadEx(MODEM_ID_0,NV_MULTIPINS_CONFIG,&MultiPinsPara,sizeof(NV_MULTIPINS_TYPE));
    if (NV_OK != err)
    {
        MultiPinsPara.RGMII_MII = MULTIPINS_MII;
        printk("read  wrong nv num, but set to MII\n");
    }
#endif
    if (MULTIPINS_RGMII == MultiPinsPara.RGMII_MII)
    {
        phy_interface = PHY_INTERFACE_MODE_RGMII;
    }
    else if (MULTIPINS_MII == MultiPinsPara.RGMII_MII)
    {
        phy_interface = PHY_INTERFACE_MODE_MII;
    }
    else
    {
        phy_interface = PHY_INTERFACE_MODE_MII;
        printk("get wrong num,but set MII\n");
    }

    return phy_interface;
#else /* BSP_CONFIG_BOARD_TELEMATIC */
	err = of_property_read_string(np, "phy-mode", &pm);
	if (err < 0)
		return err;

	for (i = 0; i < ARRAY_SIZE(phy_modes); i++)
		if (!strcasecmp(pm, phy_modes[i]))
			return i;
#endif
	return -ENODEV;
}
EXPORT_SYMBOL_GPL(of_get_phy_mode);

/**
 * Search the device tree for the best MAC address to use.  'mac-address' is
 * checked first, because that is supposed to contain to "most recent" MAC
 * address. If that isn't set, then 'local-mac-address' is checked next,
 * because that is the default address.  If that isn't set, then the obsolete
 * 'address' is checked, just in case we're using an old device tree.
 *
 * Note that the 'address' property is supposed to contain a virtual address of
 * the register set, but some DTS files have redefined that property to be the
 * MAC address.
 *
 * All-zero MAC addresses are rejected, because those could be properties that
 * exist in the device tree, but were not set by U-Boot.  For example, the
 * DTS could define 'mac-address' and 'local-mac-address', with zero MAC
 * addresses.  Some older U-Boots only initialized 'local-mac-address'.  In
 * this case, the real MAC is in 'local-mac-address', and 'mac-address' exists
 * but is all zeros.
*/
const void *of_get_mac_address(struct device_node *np)
{
	struct property *pp;

	pp = of_find_property(np, "mac-address", NULL);
	if (pp && (pp->length == 6) && is_valid_ether_addr(pp->value))
		return pp->value;

	pp = of_find_property(np, "local-mac-address", NULL);
	if (pp && (pp->length == 6) && is_valid_ether_addr(pp->value))
		return pp->value;

	pp = of_find_property(np, "address", NULL);
	if (pp && (pp->length == 6) && is_valid_ether_addr(pp->value))
		return pp->value;

	return NULL;
}
EXPORT_SYMBOL(of_get_mac_address);
