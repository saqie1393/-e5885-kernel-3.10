/*
 * Driver for AR8035 PHY
 *
 * Author: jiangdihui <jiangdihui@huawei.com>
 *
 */


#include <linux/phy.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "mbb_net.h"
#include <linux/wakelock.h>
#include <linux/netdevice.h>
#include "../ethernet/stmicro/stmmac/stmmac.h"
#include <linux/sched.h>
#include <product_config.h>
#if (FEATURE_ON == MBB_COMMON)
#include <hi_gpio.h>
#endif

#define AR8035_INER_LINK_OK             (1 << 10)
#define AR8035_SPEED1000                (1 << 15)
#define AR8035_SPEED100                 (1 << 14)
#define AR8035_FULLDPLX                 (1 << 13)
#define AR8035_LINKSTAT                 (1 << 10)
#define AR8035_HIB_CTRL                 (1 << 15)

#define AR8035_PHYSR                    0x11
#define AR8035_INER                     0x12
#define AR8035_INSR                     0x13
#define AR8035_DEBUG_ADDR               0x1D
#define AR8035_DEBUG_DATA               0x1E
#define AR8035_DEBUG_HIB                0x0B

#define AR8035_PHY_ID                   0x004dd072   
#define AR8035_PHYID_MASK               0x00ffffff

#define GPIO_PHY_RESET  LAN_PHY_RESET 
#define PHY_RESET_MTIME  100
#define PHY_RESET_DELAY  10
#define LAN_TRAFFIC_OFF_TIMEOUT  (600)  //网口作lan口时无流量检测超时时间
#define LAN_NO_USE_TIMEOUT       (180)  //开机、网口状态变化允许进入待机延时
#define PHY_POWER_ON             (1)
#define PHY_POWER_OFF            (0)
#define ETH_ROUTE_WAN            (0)
#define ETH_ROUTE_LAN            (1)
#define ETH_ROUTE_UNKNOWN        (0xf)
#define ROUTE_STATE_ERR                  (-1)
struct wake_lock wl_eth;   /* eth wakelock */

#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
#define	AR8035_DEBUG_TXCLKDLY_CTRL        0x05
#define	AR8035_TXCLK_DLY                  (1 << 8)
#define POWERON_DELAY             1000
int eth_power_on();
#endif


MODULE_DESCRIPTION("Driver for AR8035 PHY");
MODULE_AUTHOR("jiangdihui <jiangdihui@huawei.com>");
MODULE_LICENSE("GPL");

#define AR8035_MMD_ACCESS_CONTROL       0x0D
#define AR8035_MMD_ACCESS_CONTROL_DATA  0x0E
#define AR8035_MMD_DEVICE_ADDRESS       0x07
#define AR8035_MMD_OFFSET_ADDRESS       0x3C
#define AR8035_MMD_HOLD_ADDRESS_VALUE   0x4007
#define AR8035_MMD_OFFSET_ADDRESS_VALUE   0x00

#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
/*flag wifi power state*/
static bool wifi_power_status = true;
#endif

/* 标识phy上电状态：1有电  0无电 */
int phy_power_state = PHY_POWER_OFF;

static struct phy_device *s_phydev = NULL;

extern struct net init_net;
extern struct net_device *__dev_get_by_name(struct net *net, const char *name);

/*****************************************************************************
函数名：   mbb_get_phy_device
功能描述:  提供给其它模块获取phy设备
输入参数： 无
返回值：   phy设备
*****************************************************************************/
struct phy_device *mbb_get_phy_device(void)
{
    return s_phydev;
}

/*****************************************************************************
函数名：   ar8035_phy_hwreset
功能描述:  phy设备reset引脚复位
输入参数： 无
返回值：   无
*****************************************************************************/
int ar8035_phy_hwreset(void* priv)
{
    if (0 == gpio_request(GPIO_PHY_RESET, "ar8035"))
    {
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
        (void)gpio_direction_output(GPIO_PHY_RESET, 0);
        mdelay(PHY_RESET_DELAY);
        (void)gpio_direction_output(GPIO_PHY_RESET, 1);
        mdelay(PHY_RESET_MTIME);
#else
        (void)gpio_direction_output(GPIO_PHY_RESET, 1);
        mdelay(PHY_RESET_DELAY);
        (void)gpio_direction_output(GPIO_PHY_RESET, 0);
        mdelay(PHY_RESET_MTIME);
#endif
        LAN_DEBUG("AR8035 start phy reset PIN:%d\r\n", GPIO_PHY_RESET);
    }
    else
    {
        LAN_DEBUG("AR8035 phy reset failed!\n");
    }
    
#if (FEATURE_OFF == MBB_FACTORY)
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 车载模块升级版本上客户需要使用此gpio */
    gpio_free(GPIO_PHY_RESET);
#endif /* BSP_CONFIG_BOARD_TELEMATIC */
#endif /* MBB_FACTORY */
    return 0;
}
  
/*****************************************************************************
函数名：   ar8035_set_off_EEE
功能描述:  phy设备关闭EEE节能
输入参数： phydev
返回值：   
*****************************************************************************/
static void ar8035_set_off_EEE(struct phy_device *phydev)
{
    int reg = 0;
    int err = 0;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    reg = AR8035_MMD_DEVICE_ADDRESS;               //0x07
    err = phy_write(phydev,AR8035_MMD_ACCESS_CONTROL,reg);
    if (err < 0)
    {
        return err;
    }

    reg = AR8035_MMD_OFFSET_ADDRESS ;              //0x3C
    err = phy_write(phydev,AR8035_MMD_ACCESS_CONTROL_DATA,reg);
    if (err < 0)
    {
        return err;
    }

    reg = AR8035_MMD_HOLD_ADDRESS_VALUE ;          //0x4007
    err = phy_write(phydev,AR8035_MMD_ACCESS_CONTROL,reg);
    if (err < 0)
    {
        return err;
    }

    reg = AR8035_MMD_OFFSET_ADDRESS_VALUE ;        //0x00
    err = phy_write(phydev,AR8035_MMD_ACCESS_CONTROL_DATA,reg);
    if (err < 0)
    {
        return err;
    }
}
/*****************************************************************************
函数名：   ar8035_config_init
功能描述:  phy设备初始化
输入参数： phydev
返回值：   NET_RET_OK(0)为ok
*****************************************************************************/
static int ar8035_config_init(struct phy_device *phydev)
{
    int reg, err;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    LAN_DEBUG("AR8035 phy driver configinit\r\n");
#ifndef BSP_CONFIG_BOARD_E5885Ls_93a
    /*关闭hibernate， 否则GMAC dma初始化会失败*/
    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_HIB);
    if (err < 0)
    {
        return err;
    }

    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg &= (~AR8035_HIB_CTRL);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        return err;
    }
#else
    /*E5885Ls-93a enable hibernate, need to force reset phy first, otherwise dma will init fail*/
    phy_power_state = PHY_POWER_OFF;
    eth_power_on();

    err = phy_write(phydev, AR8035_DEBUG_ADDR, AR8035_DEBUG_TXCLKDLY_CTRL);
    if (err < 0)
    {
        return err;
    }
    reg = phy_read(phydev, AR8035_DEBUG_DATA);
    reg |= (AR8035_TXCLK_DLY);
    err = phy_write(phydev, AR8035_DEBUG_DATA, reg);
    if (err < 0)
    {
        return err;
    }
#endif

    ar8035_set_off_EEE(phydev);
#if 0
    /*配置phy自动协商*/
    reg = phy_read(phydev, MII_BMCR);
    reg |= (BMCR_ANENABLE | BMCR_ANRESTART);
    err = phy_write(phydev, MII_BMCR, reg);
    if (err < 0)
    {
        return err;
    }
    
    reg = phy_read(phydev, AR8035_INER);
    reg = reg | AR8035_INER_LINK_OK;
    err = phy_write(phydev, AR8035_INER, reg);

    if (err < 0)
    {
        return err;
    }    
#endif

    phydev->autoneg = AUTONEG_ENABLE;

    s_phydev = phydev;

    /*网口模块sysfs节点初始化*/
    hw_net_sysfs_init();

    return NET_RET_OK;
}
void eth_wake_lock_timeout();
extern void mbb_eth_state_report(int new_state);
extern void set_eth_lan();
extern int stmmac_restore(struct net_device *dev);
/*****************************************************************************
函数名：   eth_power_down
功能描述:  控制GPIO输出0使phy芯片下电
输入参数：
返回值：1 下电成功    0 此前已下电，无需重复操作
*****************************************************************************/
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
int eth_power_down()
{
    //phy power处于下电状态时，不用重复再下电
    if(PHY_POWER_OFF == phy_power_state)
    {
        return 0;
    }
    phy_power_state = PHY_POWER_OFF;
    set_eth_lan();
    gpio_direction_output(GPIO_PHY_RESET, 0);
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 0);
    LAN_DEBUG("ar8035 power down\n");
    return 1;
}
EXPORT_SYMBOL(eth_power_down);
/*****************************************************************************
函数名：   eth_power_on
功能描述:  控制GPIO输出1使phy芯片上电，上电后进行复位操作
输入参数：
返回值：1上电成功    0此前已上电，无需重复操作
*****************************************************************************/
int eth_power_on()
{
#if ((!defined(BSP_CONFIG_BOARD_TELEMATIC)) && (!defined(BSP_CONFIG_BOARD_E5885Ls_93a)))
    eth_wake_lock_timeout(); //系统从待机状态下唤醒，eth上电后持3分钟超时锁
#endif
    if (PHY_POWER_ON == phy_power_state)
    {
        LAN_DEBUG("phy power is enabled\n");
        return 0;
    }
    else
    {
        phy_power_state = PHY_POWER_ON;
    }
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 1);
    LAN_DEBUG("ar8035 power on\n");
#if defined(BSP_CONFIG_BOARD_E5885Ls_93a)
    (void)gpio_direction_output(GPIO_PHY_RESET, 0);
    mdelay(PHY_RESET_DELAY);
    (void)gpio_direction_output(GPIO_PHY_RESET, 1);
    mdelay(PHY_RESET_MTIME);
#else
    (void)gpio_direction_output(GPIO_PHY_RESET, 1);
    mdelay(PHY_RESET_DELAY);
    (void)gpio_direction_output(GPIO_PHY_RESET, 0);
    mdelay(PHY_RESET_MTIME);
#endif

    return 1;
}
EXPORT_SYMBOL(eth_power_on);
#endif
/*****************************************************************************
函数名：   eth_check_wan
功能描述:  检测网口识别为lan口或wan口
输入参数：
返回值：   如果识别为wan口返回1，识别为lan口返回0
*****************************************************************************/
extern int eth_check_wan();

/*****************************************************************************
函数名：   check_lan_stream
功能描述:  检测lan口流量
输入参数：
返回值：   如果有流量返回1，无流量返回0
*****************************************************************************/
int check_lan_stream()
{
    static int eth_rx_packets = 0;
    static int eth_tx_packets = 0;
    struct net_device *dev = __dev_get_by_name(&init_net, "eth0");
    if( (NULL != dev) && (dev->stats.rx_packets != eth_rx_packets)
            && (dev->stats.tx_packets != eth_tx_packets) )
    {
        eth_rx_packets = dev->stats.rx_packets;
        eth_tx_packets = dev->stats.tx_packets;
        return PHY_HAVE_FLOW;
    }
    else
    {
        return PHY_NO_FLOW;
    }
}
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
/*****************************************************************************
name：   eth_traffic_check
function:  Ethernet low power check
para：
return： 1：in LAN mode ,has some stream
         0：no stream
        -1：unknow state
*****************************************************************************/
int eth_traffic_check(void)
{
    int route_state,ret=0;
    route_state = eth_check_wan();

    if ( ETH_ROUTE_WAN == route_state )
    {
        wake_unlock(&wl_eth);
        return PHY_NO_FLOW;
    }
    else if (ETH_ROUTE_LAN == route_state)
    {
        /*check the eth is lan and wifi is not power down,firbit sleep(wait the timer of wifi time out release the lock)*/
        if(wifi_power_status)
        {
            wake_lock(&wl_eth);
        }
        ret= check_lan_stream();
        return ret;
    }
    else
    {
        return ROUTE_STATE_ERR;
    }
}
/*****************************************************************************
name：   wifi_unlock_eth
function:  Release the lock WiFi hold, the WiFi module is used to achieve the ETH
 and WIFI together power down
para：
return：
*****************************************************************************/
void wifi_unlock_eth(void)
{
    /*when the ETH is only LAN,wifi release the wakelock*/
    if ( ETH_ROUTE_LAN == eth_check_wan() )
    {
        /*record power state*/
        wifi_power_status = false;
        LAN_DEBUG("wifi power off timeout, eth as lan release eth_lowpower lock\n");
        wake_unlock(&wl_eth);
    }
}
#endif
void eth_wake_lock_timeout()
{
    LAN_DEBUG("set eth wakelock timeout 180\n");
    wake_lock_timeout(&wl_eth, LAN_NO_USE_TIMEOUT * HZ);
}
void eth_wake_unlock()
{
    wake_unlock(&wl_eth);
}

/*****************************************************************************
 

函数名：   ar8035_read_status
功能描述:  phy设备获取状态
输入参数： phydev
返回值：   NET_RET_OK(0)为获取状态OK
*****************************************************************************/
int ar8035_read_status(struct phy_device *phydev)
{
    int reg;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    reg = phy_read(phydev, AR8035_PHYSR);    
    if (reg < 0)
    {
        return reg;
    }
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    //phy芯片下电后，读取的寄存器值为0xffff，需要return防止误报link up
    if (reg == 0xffff)
    {
        return reg;
    }
#endif
    
    /*phy是否link*/
    if (reg & AR8035_LINKSTAT)
    {
        phydev->link = 1;
    }    
    else
    {
        phydev->link = 0;
    }    

    /*phy速率配置*/
    if ((reg & AR8035_SPEED100) && !(reg & AR8035_SPEED1000))
    {
        phydev->speed = SPEED_100;
    }
    else if (!(reg & AR8035_SPEED100) && (reg & AR8035_SPEED1000))
    {
        phydev->speed = SPEED_1000;
    }
    else
    {
        phydev->speed = SPEED_10;
    }
    if (reg & AR8035_FULLDPLX)
    {
        phydev->duplex = DUPLEX_FULL;
    }
    else
    {
        phydev->duplex = DUPLEX_HALF;
    }
    
    phydev->pause = 0;
    phydev->asym_pause = 0;

    //LAN_DEBUG("AR8035 phy read status link:%d, speed:%d, duplex:%d\r\n", phydev->link, phydev->speed, phydev->duplex);

    return NET_RET_OK;
}

#define REG_NUM_MAX   32
void SetPhyReg(unsigned int  regnum, unsigned short val)
{
    int err = 0;
    if (NULL == s_phydev)
    {
        return;
    }

    if (regnum >= REG_NUM_MAX || val > 0xFFFF)
    {       
        return;
    }

    LAN_DEBUG("set phy register %d value: 0x%x\r\n", regnum, val);

    err = phy_write(s_phydev, regnum, val);
    if(err < 0)
    {
        LAN_DEBUG("write phy register fail");
    }
}

int GetPhyReg(unsigned int regnum)
{
    int value; 
    if (NULL == s_phydev)
    {
        return -1;
    }

    if (regnum >= REG_NUM_MAX)
    {       
        return -1;
    }
    
    value = phy_read(s_phydev, regnum);
   
    LAN_DEBUG("get phy register %d value: 0x%x\r\n", regnum, value);

    return value;
}

EXPORT_SYMBOL(SetPhyReg);
EXPORT_SYMBOL(GetPhyReg);

static struct phy_driver ar8035_driver = {
    .phy_id         = AR8035_PHY_ID,
    .phy_id_mask    = AR8035_PHYID_MASK,      
    .name           = "AR8035 Gigabit Phy",
    .features       = PHY_GBIT_FEATURES,
    .flags          = PHY_HAS_INTERRUPT,
    .config_aneg    = genphy_config_aneg,
    .read_status    = ar8035_read_status,
    .config_init    = ar8035_config_init,
    .driver         = { .owner = THIS_MODULE,},
};

/*****************************************************************************
函数名：   ar8035_init
功能描述: ar8035 phy设备注册
输入参数： 无
返回值：   
*****************************************************************************/
static int __init ar8035_init(void)
{
    int ret;
    wake_lock_init(&wl_eth, WAKE_LOCK_SUSPEND, "eth_lowpower");
#ifndef BSP_CONFIG_BOARD_TELEMATIC
    eth_wake_lock_timeout();
#endif
    phy_power_state = PHY_POWER_ON;

#ifdef BSP_CONFIG_BOARD_E5885Ls_93a
    ret = gpio_request(GPIO_PHY_LOWPOWEREN, "lowpower_en");
    if (ret)
    {
        LAN_DEBUG("Failed to get GPIO_PHY_LOWPOWEREN. Code: %d.", ret);
        return ret;
    }
    gpio_direction_output(GPIO_PHY_LOWPOWEREN, 1);
    msleep(POWERON_DELAY);
#endif

    ar8035_phy_hwreset(NULL);

    LAN_DEBUG("AR8035 phy driver register\r\n");

    ret = phy_driver_register(&ar8035_driver);

    return ret;
}

/*****************************************************************************
函数名：   ar8035_exit
功能描述: ar8035 phy设备退出
输入参数： 无
返回值：   
*****************************************************************************/
static void __exit ar8035_exit(void)
{
    phy_driver_unregister(&ar8035_driver);

    hw_net_sysfs_uninit();
}

module_init(ar8035_init);
module_exit(ar8035_exit);

static struct mdio_device_id __maybe_unused ar8035_tbl[] = {
    { AR8035_PHY_ID, AR8035_PHYID_MASK },
    { }
};

MODULE_DEVICE_TABLE(mdio, ar8035_tbl);

