



#include <linux/phy.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "mbb_net.h"
#include <linux/wakelock.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <product_config.h>
#if (FEATURE_ON == MBB_COMMON)
#include <hi_gpio.h>
#endif
#include "bsp_version.h"
#include "mbb_bsp_reg_def.h"
#include "ios_ao_drv_macro.h"
#include "ios_pd_drv_macro.h"
#include "product_nv_def.h"
#include "bsp_nvim.h"  
#include "product_nv_id.h"
#include "bsp_sram.h" 

#define LRE_Control                       0x0       //寄存器0x0
#define LRE_Status                        0x1       //寄存器0x1
#define LRE_PHY_Identifier1               0x2       //寄存器0x2
#define LRE_PHY_Identifier2               0x3       //寄存器0x3
#define BCM89811_PHYSR                    0x11      //寄存器0x11

#define RDB_ADDR                          (0x1e)    //RDB寄存器地址
#define RDB_VAL                           (0x1f)    //RDB寄存器地址值

#define BCM89811_LINKSTAT                 (1 << 8)  //寄存器0x11 bit 8
#define BCM89811_LOCAL_RECEIVER_STATUS    (1 << 10) //寄存器0x11 bit 10
#define BCM89811_REMOTE_RECEIVER_STATUS   (1 << 11) //寄存器0x11 bit 11

#define MASTER_MODE                       0x208     //主模式
#define SLAVE                             0x200     //从模式

#define BCM89811_PHY_ID                   0xAE025020  /* 对应于Register 02h and 03h: LRE PHY Identifier */
#define BCM89811_PHYID_MASK               0xfffffff0

/* RGMII的部分管脚，可参考原理图 */
#define PHY_BCM89811_PIN_RXDV             LAN_PHY_ADDR
#define PHY_BCM89811_PIN_RXD1             LAN_PHY_MODE1
#define PHY_BCM89811_PIN_RXD2             LAN_PHY_MODE2
#define GPIO_PHY_RESET                    LAN_PHY_RESET
#define PHY_RESET_DELAY                   20
#define PHY_RESET_WAIT                    1

#define PHY_BCM89811_FEATURES   (SUPPORTED_TP | \
                                 SUPPORTED_MII | \
                                 SUPPORTED_100baseT_Full)

#define MULTIPINS_RGMII   (2)
#define MULTIPINS_MII    (3)

MODULE_DESCRIPTION("Driver for BCM89811 PHY");
MODULE_AUTHOR("HUAWEI DRIVER GROUP");
MODULE_LICENSE("GPL");

static struct phy_device* s_phydev = NULL;


/*****************************************************************************
函数名：   mbb_get_phy_device
功能描述:  提供给其它模块获取phy设备
输入参数： 无
返回值：   phy设备
*****************************************************************************/
struct phy_device* mbb_get_phy_device(void)
{
    return s_phydev;
}

/*lint -e666*/
/*****************************************************************************
函数名：   drv_mii_rgmii_set_function_to_gpio
功能描述:  将RX_DV,RXD1,RXD2 复用为GPIO
输入参数： 无
返回值：   无
*****************************************************************************/
void drv_mii_rgmii_set_function_to_gpio(void)
{
    /*rgmii_rxdv管脚复用为GPIO_2_0*/
    SET_IOS_GPIO2_0_CTRL1_1;
    CLR_IOS_RGMII_RXDV_CTRL1_1;

    /*rgmii_rxd[1]管脚复用为GPIO_2_2*/
    SET_IOS_GPIO2_2_CTRL1_1;
    CLR_IOS_RGMII_RXD1_CTRL1_1;

    /*rgmii_rxd[2]管脚复用为GPIO_2_3*/
    SET_IOS_GPIO2_3_CTRL1_1;
    CLR_IOS_RGMII_RXD2_CTRL1_1;
}

/*****************************************************************************
函数名：   drv_mii_rgmii_set_gpio_to_function
功能描述:   将RX_DV,RXD1,RXD2 对应的GPIO 复用为其功能脚
输入参数： 无
返回值：   无
*****************************************************************************/
void drv_mii_rgmii_set_gpio_to_function(void)
{
    /*rgmii_rxdv管脚复用配置*/
    SET_IOS_RGMII_RXDV_CTRL1_1;
    INSET_IOS_PD_IOM_CTRL43;
    CLR_IOS_GPIO2_0_CTRL1_1;

    /*rgmii_rxd[1]管脚复用配置*/
    SET_IOS_RGMII_RXD1_CTRL1_1;
    INSET_IOS_PD_IOM_CTRL45;
    CLR_IOS_GPIO2_2_CTRL1_1;

    /*rgmii_rxd[2]管脚复用配置*/
    SET_IOS_RGMII_RXD2_CTRL1_1;
    INSET_IOS_PD_IOM_CTRL46;
    CLR_IOS_GPIO2_3_CTRL1_1;
}

/*****************************************************************************
函数名：   drv_set_rgmii_txclk
功能描述:   配置mii txclk
输入参数： 无
返回值：   无
*****************************************************************************/
void drv_set_rgmii_txclk(void)
{
    /*rgmii_rmii_tx_clk管脚复用配置*/
    SET_IOS_RGMII_TX_CLK_CTRL1_1;
    OUTSET_IOS_PD_IOM_CTRL39;
    CLR_IOS_GPIO1_4_CTRL1_1;
    /*rgmii_rmii_tx_clk管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL39;
}

/*****************************************************************************
函数名：   drv_set_mii_txclk
功能描述:   配置mii txclk
输入参数： 无
返回值：   无
*****************************************************************************/
void drv_set_mii_txclk(void)
{
    /*mii_tx_clk管脚复用配置*/
    SET_IOS_MII_TX_CLK_CTRL1_1;
    INSET_IOS_PD_IOM_CTRL39;
    CLR_IOS_GPIO1_4_CTRL1_1;
    CLR_IOS_RGMII_TX_CLK_CTRL1_1;
    CLR_IOS_ANTPA_SEL29_CTRL3_1;
    /*mii_tx_clk管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL39;
}
/*lint +e666*/

/*****************************************************************************
函数名：   bcm89811_set_phyaddr
功能描述:  phy设备reset引脚复位
输入参数： 无
返回值：   无
*****************************************************************************/
int bcm89811_set_phyaddr(void)
{
    int err = -1;
    /*
     *PHY address of the BCM89811 is determined at reset by the value of RX_DV. The address will be 00000 + RX_DV.
     *此处将PHY的地址配置0，与stmmac_main_balong.c中的phyaddr对应。
     */
    err = gpio_request(PHY_BCM89811_PIN_RXDV, "rgmiirxdv");
    if (0 != err)
    {
        LAN_DEBUG("PHY_BCM89811_PIN_RXDV request fail.\r\n");
        return err;
    }

    err = gpio_direction_output(PHY_BCM89811_PIN_RXDV, 0);
    if (0 != err)
    {
        LAN_DEBUG("PHY_BCM89811_PIN_RXDV pull down fail.\r\n");
    }
    gpio_free(PHY_BCM89811_PIN_RXDV);
    return err;

}

/*****************************************************************************
函数名：   bcm89811_setphy_to_rgmii
功能描述:  将phy 设置为RGMII 模式
输入参数： 无
返回值：   无
*****************************************************************************/
int bcm89811_setphy_to_rgmii(void)
{
    int err = -1;

    /*将txclk 配置为rgmii_tx_clk*/
    drv_set_rgmii_txclk();

    /*根据博通datesheet，配置RXD2和RXD1，将PHY设置为RGMII模式*/
    err = gpio_request(PHY_BCM89811_PIN_RXD1, "rgmiirxd1");
    if (0 != err)
    {
        LAN_DEBUG("%s:rgmiirxd1 request fail.\r\n",__func__);
        return err;
    }

    err = gpio_direction_output(PHY_BCM89811_PIN_RXD1, 1);
    gpio_free(PHY_BCM89811_PIN_RXD1);
    if (0 != err)
    {
        LAN_DEBUG("%s:set PHY_BCM89811_PIN_RXD1 fail.\r\n",__func__);
        return err;
    }

    err = gpio_request(PHY_BCM89811_PIN_RXD2, "rgmiirxd2");
    if (0 != err)
    {
        LAN_DEBUG("%s:rgmiirxd2 request fail.\r\n",__func__);
        return err;
    }

    err = gpio_direction_output(PHY_BCM89811_PIN_RXD2, 1);
    gpio_free(PHY_BCM89811_PIN_RXD2);
    if (0 != err)
    {
        LAN_DEBUG("%s:set PHY_BCM89811_PIN_RXD2 fail.\r\n",__func__);
        return err;
    }

    return err;

}

/*****************************************************************************
函数名：   bcm89811_setphy_to_mii
功能描述:  将phy 设置为MII 模式
输入参数： 无
返回值：   无
*****************************************************************************/
int bcm89811_setphy_to_mii(void)
{
    int err = -1;

    /*将txclk 配置为mii_tx_clk*/
    drv_set_mii_txclk();

    /*根据博通datesheet，配置RXD2和RXD1，将PHY设置为MII模式*/
    err = gpio_request(PHY_BCM89811_PIN_RXD1, "rgmiirxd1");
    if (0 != err)
    {
        LAN_DEBUG("%s:rgmiirxd1 request fail.\r\n",__func__);
        return err;
    }

    err = gpio_direction_output(PHY_BCM89811_PIN_RXD1, 0);
    gpio_free(PHY_BCM89811_PIN_RXD1);
    if (0 != err)
    {
        LAN_DEBUG("%s:set PHY_BCM89811_PIN_RXD1 fail.\r\n",__func__);
        return err;
    }

    err = gpio_request(PHY_BCM89811_PIN_RXD2, "rgmiirxd2");
    if (0 != err)
    {
        LAN_DEBUG("%s:rgmiirxd2 request fail.\r\n",__func__);
        return err;
    }

    err = gpio_direction_output(PHY_BCM89811_PIN_RXD2, 0);
    gpio_free(PHY_BCM89811_PIN_RXD2);
    if (0 != err)
    {
        LAN_DEBUG("%s:set PHY_BCM89811_PIN_RXD2 fail.\r\n",__func__);
        return err;
    }

    return err;

}

/*****************************************************************************
函数名：   bcm89811_phy_hwreset
功能描述:  phy设备reset引脚复位
输入参数： 无
返回值：   无
*****************************************************************************/
int bcm89811_phy_hwreset(void* priv)
{
#if (FEATURE_ON == MBB_FACTORY)  // 升级版本采用MII-MII的方式和客户的MCU相连，不读写PHY寄存器
    int err = -1;
    NV_MULTIPINS_TYPE    MultiPinsPara;

    (void)memset((void*)(&MultiPinsPara),0,sizeof(NV_MULTIPINS_TYPE));

    /*在这个位置判定一下不同的版本和读取位置,烧片版本定义一个魔术字，如果读取的是一致的，说明是设置过，
    就把共享内存中的数读入，不一致的话，就设置为MII格式。升级版本读取NV值，如果读取失败也设置为MII。**/
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
    if (NV_OK != NV_ReadEx(MODEM_ID_0,
        NV_MULTIPINS_CONFIG,
        &MultiPinsPara,
        sizeof(NV_MULTIPINS_TYPE)))
    {
        MultiPinsPara.RGMII_MII = MULTIPINS_MII;
        printk("read  wrong nv num, but set to MII\n"); 
    }
#endif
    /*将rxdv,rxd1,rxd2复用为GPIO*/
    drv_mii_rgmii_set_function_to_gpio();

    /*根据博通datesheet,hardware reset之前要设置phyaddr*/
    err = bcm89811_set_phyaddr();
    if (0 != err)
    {
        LAN_DEBUG("Set phy address fail.\r\n");
        return err;
    }

    /*根据博通datesheet,将phy设置为mii模式*/
    if(MULTIPINS_MII == MultiPinsPara.RGMII_MII)
    {
        err = bcm89811_setphy_to_mii();
        if (0 != err)
        {
            LAN_DEBUG("Set phy to mii fail.\r\n");
            return err;
        }
        LAN_DEBUG("Set phy to mii successful.\r\n");
    }
    else if(MULTIPINS_RGMII == MultiPinsPara.RGMII_MII)
    {
        err = bcm89811_setphy_to_rgmii();
        if (0 != err)
        {
            LAN_DEBUG("Set phy to rgmii fail.\r\n");
            return err;
        }
        LAN_DEBUG("Set phy to rgmii successful.\r\n");
    }
    else
    {
        err = bcm89811_setphy_to_mii();
        if (0 != err)
        {
            LAN_DEBUG("Set phy to mii fail.\r\n");
            return err;
        }
        LAN_DEBUG("get wrong num,but Set phy to mii.\r\n");
    }

    /*phy hardware reset,reset脚为反向脚*/
    err = gpio_request(GPIO_PHY_RESET, "bcm89811");
    if (0 != err)
    {
        LAN_DEBUG("GPIO_PHY_RESET request fail.\r\n");
        return err;
    }

    err = gpio_direction_output(GPIO_PHY_RESET, 1);
    if (0 != err)
    {
        LAN_DEBUG("GPIO_PHY_RESET pull up fail.\r\n");
        return err;
    }

    mdelay(PHY_RESET_DELAY); 

    err = gpio_direction_output(GPIO_PHY_RESET, 0);
    gpio_free(GPIO_PHY_RESET);
    if (0 != err)
    {
        LAN_DEBUG("GPIO_PHY_RESET pull down fail.\r\n");
        return err;
    }

    mdelay(PHY_RESET_WAIT);

    /*reset之后要将rxdv,rxd1,rxd2 对应的GPIO 复用为其功能脚*/
    drv_mii_rgmii_set_gpio_to_function();

    LAN_DEBUG("BCM89811 start phy reset PIN:%d\r\n", GPIO_PHY_RESET);

#endif  /* FEATURE_ON == MBB_FACTORY */
    return 0;
}

/*****************************************************************************
函数名：   bcm89811_config_init
功能描述:  phy设备初始化
输入参数： phydev
返回值：   NET_RET_OK(0)为ok
*****************************************************************************/
static int bcm89811_config_init(struct phy_device* phydev)
{
    int err = 0;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    LAN_DEBUG("BCM89811 phy driver configinit\r\n");

    phydev->autoneg = AUTONEG_DISABLE;

    /*phy速率配置，仅支持100MHz*/
    phydev->speed = SPEED_100;

    /*phy双工配置，仅支持全双工*/
    phydev->duplex = DUPLEX_FULL;

    /*不支持流控*/
    phydev->pause = 0;
    phydev->asym_pause = 0;
#if (FEATURE_ON == MBB_FACTORY)  // 升级版本采用MII-MII的方式和客户的MCU相连，不读写PHY寄存器
    /*配置成master mode*/
    err = phy_write(phydev, LRE_Control, MASTER_MODE);
    if (err < 0)
    {
        return err;
    }
#endif /* FEATURE_ON == MBB_FACTORY */
    s_phydev = phydev;
#if (FEATURE_ON == MBB_FACTORY)  // 升级版本采用MII-MII的方式和客户的MCU相连，不读写PHY寄存器
    LAN_DEBUG("%s: read Reg 0x00=%x\n", __func__, phy_read(phydev, LRE_Control));
    LAN_DEBUG("%s: read Reg 0x01=%x\n", __func__, phy_read(phydev, LRE_Status));
    LAN_DEBUG("%s: read Reg 0x02=%x\n", __func__, phy_read(phydev, LRE_PHY_Identifier1));
    LAN_DEBUG("%s: read Reg 0x03=%x\n", __func__, phy_read(phydev, LRE_PHY_Identifier2));
    LAN_DEBUG("%s: read Reg 0x11=%x\n", __func__, phy_read(phydev, BCM89811_PHYSR));
#endif /* FEATURE_ON == MBB_FACTORY */
    /*网口模块sysfs节点初始化*/
    if (NET_RET_OK != hw_net_sysfs_init())
    {
        return NET_RET_FAIL;
    }

    return NET_RET_OK;
}

/*****************************************************************************
函数名：   bcm89811_read_status
功能描述:  phy设备获取link 状态
输入参数:  phydev
返回值：   NET_RET_OK(0)为获取状态OK
*****************************************************************************/
int bcm89811_read_status(struct phy_device* phydev)
{
    int reg = 0;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

#if (FEATURE_ON == MBB_FACTORY)  // 升级版本采用MII-MII的方式和客户的MCU相连，不读写PHY寄存器
    reg = phy_read(phydev, BCM89811_PHYSR);
    if (reg < 0)
    {
        return reg;
    }

    /*phy是否link*/
    if (reg & BCM89811_LINKSTAT)
    {
        phydev->link = 1;
    }
    else
    {
        phydev->link = 0;
    }
#else
    phydev->link = 1;  // 升级版本采用MII-MII的方式和客户的MCU相连，PHY的连接状态需要返回已连接
#endif /* FEATURE_ON == MBB_FACTORY */

    /*phy速率配置，仅支持100MHz*/
    phydev->speed = SPEED_100;

    /*phy双工配置，仅支持全双工*/
    phydev->duplex = DUPLEX_FULL;

    phydev->pause = 0;
    phydev->asym_pause = 0;

    return NET_RET_OK;
}

#define REG_NUM_MAX   0x1c
#define REG_VAL_MAX   0xFFFF
/*****************************************************************************
函数名：  SetPhyReg
功能描述: 写寄存器
输入参数: 寄存器ID以及待写入的值
返回值：  无
*****************************************************************************/
void SetPhyReg(unsigned int regnum, unsigned int val)
{
    if (NULL == s_phydev)
    {
        return;
    }

    if ((regnum >= REG_NUM_MAX) || (val > REG_VAL_MAX))
    {
        return;
    }

    if (0 != phy_write(s_phydev, regnum, val))
    {
        return;
    }

    LAN_DEBUG("Set phy register 0x%x value to 0x%x.\r\n", regnum, val);
}

/*****************************************************************************
函数名：  GetPhyReg
功能描述: 读寄存器
输入参数: 寄存器ID
返回值：  寄存器的值
*****************************************************************************/
int GetPhyReg(unsigned int regnum)
{
    int value = 0;
    if (NULL == s_phydev)
    {
        return -1;
    }

    if (regnum >= REG_NUM_MAX)
    {
        return -1;
    }

    value = phy_read(s_phydev, regnum);

    LAN_DEBUG("Get phy register 0x%x value is: 0x%x\r\n", regnum, value);

    return value;
}

EXPORT_SYMBOL(SetPhyReg);
EXPORT_SYMBOL(GetPhyReg);

/*****************************************************************************
函数名：  Set_RDBReg
功能描述: 写RDB 寄存器
输入参数: 寄存器ID以及待写入的值
返回值：  0:写入成功，其他:写入失败
*****************************************************************************/
int Set_RDBReg(unsigned int regnum, unsigned short val)
{
    int err = -1;
    if (NULL == s_phydev)
    {
        return err;
    }

    err = phy_write(s_phydev, RDB_ADDR, regnum);
    if(0 != err)
    {
        LAN_DEBUG("Set RDB register fail.\r\n");
        return err;
    }
    err = phy_write(s_phydev, RDB_VAL, val);
    LAN_DEBUG("Set RDB register %d value to 0x%x, result is %d\r\n", regnum, val, err);

    return err;
}

/*****************************************************************************
函数名：  Get_RDBReg
功能描述: 读RDB 寄存器
输入参数: 寄存器ID
返回值：  寄存器的值
*****************************************************************************/
int Get_RDBReg(unsigned int regnum)
{
    int err = -1;
    int value = 0;
    if (NULL == s_phydev)
    {
        return err;
    }

    err = phy_write(s_phydev, RDB_ADDR, regnum);
    if(0 != err)
    {
        LAN_DEBUG("Get RDB register fail.\r\n");
        return err;
    }
    value = phy_read(s_phydev, RDB_VAL);
    LAN_DEBUG("Get RDB register %d value is: 0x%x\r\n", regnum, value);

    return value;
}

EXPORT_SYMBOL(Set_RDBReg);
EXPORT_SYMBOL(Get_RDBReg);

int bcm89811_config_aneg(struct phy_device* phydev)
{
    /*不支持自协商*/
    return 0;
}

static struct phy_driver bcm89811_driver =
{
    .phy_id         = BCM89811_PHY_ID,
    .phy_id_mask    = BCM89811_PHYID_MASK,
    .name           = "BCM89811 Phy",
    .features       = PHY_BCM89811_FEATURES,
    .flags          = PHY_HAS_INTERRUPT,
    .config_aneg    = bcm89811_config_aneg,
    .read_status    = bcm89811_read_status,
    .config_init    = bcm89811_config_init,
    .driver         = {
        .owner      = THIS_MODULE,
    },
};

/*****************************************************************************
函数名：  bcm89811_init
功能描述: bcm89811 phy设备注册
输入参数： 无
返回值：
*****************************************************************************/
static int __init bcm89811_init(void)
{
    int ret = 0;

    ret = bcm89811_phy_hwreset(NULL);
	if (0 != ret)
    {
        return ret;
    }

    LAN_DEBUG("BCM89811 phy driver register\r\n");
    ret = phy_driver_register(&bcm89811_driver);

    return ret;
}

/*****************************************************************************
函数名：  bcm89811_exit
功能描述: bcm89811 phy设备退出
输入参数： 无
返回值：
*****************************************************************************/
static void __exit bcm89811_exit(void)
{
    LAN_DEBUG("%s\r\n", __FUNCTION__);
    phy_driver_unregister(&bcm89811_driver);

    hw_net_sysfs_uninit();
}

module_init(bcm89811_init);
module_exit(bcm89811_exit);

static struct mdio_device_id __maybe_unused bcm89811_tbl[] =
{
    { BCM89811_PHY_ID, BCM89811_PHYID_MASK },
    { }
};

MODULE_DEVICE_TABLE(mdio, bcm89811_tbl);

