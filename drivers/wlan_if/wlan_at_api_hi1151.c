/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */



#include <linux/module.h>
#include <linux/delay.h>
#include <linux/string.h>

#include <product_config.h>

#include "wlan_if.h"
#include "wlan_at_api.h"
#include "wlan_at.h"
#include "wlan_utils.h"

#define WLAN_AT_SSID_SUPPORT            2                  /*支持的SSID组数*/
#define WLAN_AT_KEY_SUPPORT             5                  /*支持的分组数*/
#define WLAN_AT_MODE_SUPPORT            "1,2,3,4,5"        /*支持的模式(a/b/g/n/ac)*/
#define WLAN_AT_BAND_SUPPORT            "0,1,2"            /*支持的带宽(0-20M/1-40M/2-80M/3-160M)*/
#define WLAN_AT_TSELRF_SUPPORT          "0,1,2,3"          /*支持的天线索引序列*/
#define WLAN_AT_GROUP_MAX               4                  /*支持的最大天线索引*/
#define RF_2G_0   (0)
#define RF_2G_1   (1)
/*5g射频通路*/
#define RF_5G_0   (2)
#define RF_5G_1   (3)
/*MIMO*/
#define RF_2G_MIMO   (4)
#define RF_5G_MIMO   (5)

/*adapter interface*/
#define IF_HIPRIV   "wlan0"

/*WIFI功率的上下限*/
#define WLAN_AT_POWER_MIN               (0)
#define WLAN_AT_POWER_MAX               (30)
#define WLAN_AT_POW_DEFAULT             (600)

#define RX_PACKET_SIZE                  1000                /*装备每次发包数*/

#define WLAN_CHANNEL_2G_MIN             1                   /*2.4G信道最小值*/
#define WLAN_CHANNEL_5G_MIN             36                  /*5G信道最小值*/
#define WLAN_CHANNEL_2G_MAX             14                  /*2.4G信道最大*/
#define WLAN_CHANNEL_2G_MIDDLE          6
#define WLAN_CHANNEL_5G_MAX             165                 /*5G信道最大*/

#define WLAN_CHANNEL_2G_40M_MIN         1
#define WLAN_CHANNEL_2G_40M_MAX         13
#define WLAN_CHANNEL_2G_40M_MID_ABOVE   11
#define WLAN_CHANNEL_2G_40M_MID_BELOW   6

#define WLAN_CHANNEL_5G_W52_START       36
#define WLAN_CHANNEL_5G_W52_END         48
#define WLAN_CHANNEL_5G_W53_START       52
#define WLAN_CHANNEL_5G_W53_END         64
#define WLAN_CHANNEL_5G_W57_START       149
#define WLAN_CHANNEL_5G_W57_END         161

#define WLAN_CHANNEL_5G_INTERVAL        4                     /*5G信道间隔*/
#define WLAN_CHANNEL_5G_40M_INTERVAL    8                     /*5G 40M信道间隔*/
#define WLAN_CHANNEL_5G_80M_INTERVAL    16                     /*5G 80M信道间隔*/
#define WLAN_FREQ_5G_W52_80M_MIN        5180                  /*W52 80M最小频点*/
#define WLAN_FREQ_5G_W53_80M_MAX        5260                  /*W53 80M最大频点*/
#define WLAN_FREQ_5G_W56_80M_MIN        5500                  /*W56 80M最小频点*/
#define WLAN_FREQ_5G_W56_80M_MAX        5660                  /*W56 80M最大频点*/
#define WLAN_FREQ_5G_W57_80M_MIN        5745                  /*W57 80M最小频点*/
#define WLAN_FREQ_5G_W57_80M_MAX        5745                  /*W57 80M最大频点*/
#define WLAN_FREQ_2G_MAX                2484                  /*2.4G最大频点*/
#define WLAN_FREQ_5G_W52_MIN            5180                  /*W52最小频点*/
#define WLAN_FREQ_5G_W53_MAX            5320                  /*W53最大频点*/
#define WLAN_FREQ_5G_W52_40M_MIN        5180                  /*W52 40M最小频点*/
#define WLAN_FREQ_5G_W53_40M_MAX        5300                  /*W53 40M最大频点*/
#define WLAN_FREQ_5G_W56_MIN            5500                  /*W56最小频点*/
#define WLAN_FREQ_5G_W56_MAX            5720                  /*W56最大频点*/
#define WLAN_FREQ_5G_W56_40M_MIN        5500                  /*W56 40M最小频点*/
#define WLAN_FREQ_5G_W56_40M_MAX        5700                  /*W56 40M最大频点*/
#define WLAN_FREQ_5G_W57_MIN            5745                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_MAX            5825                  /*W57最大频点*/
#define WLAN_FREQ_5G_W57_40M_MIN        5745                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_40M_MAX        5785                  /*W57最大频点*/

#define WIFI_CMD_MAX_SIZE               256                   /*cmd字符串256长度*/
#define WIFI_CMD_8_SIZE                 8                     /*cmd字符串8长度*/
#define HUNDRED                         100

#define WLAN_AT_TYPE_MAX                2                  /*支持获取的最大信息类型*/

#define DYNC_ADJUST_DELAY              (300)                  /* 动态校准延时300毫秒 */

/* 海思wifi芯片寄存器地址范围定义 */
#define HISI_SOC_REG_ADDR_MIN   (0x20000000)
#define HISI_SOC_REG_ADDR_MAX   (0x2003bfff)

#define WLAN_RFCH_TRANS(_uc_rfch)       (0 == _uc_rfch)? "0001": "0010"

#define PTR_NULL        (0L)
int8*   auc_mode_table_2G[6][3] =
{                           /*AT_WIBAND_20M*/   /*AT_WIBAND_40M*/   /*AT_WIBAND_80M */
    /*AT_WIMODE_CW */       {PTR_NULL,          PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211a*/    {PTR_NULL,          PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211b*/    {"11b",             PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211g */   {"11g",             PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211n*/    {"11ng20",          "11ng40plus",       PTR_NULL},
    /*AT_WIMODE_80211ac */  {PTR_NULL,          PTR_NULL,           PTR_NULL},
};
int8*   auc_mode_table_5G[6][3] =
{                           /*AT_WIBAND_20M*/   /*AT_WIBAND_40M*/   /*AT_WIBAND_80M */
    /*AT_WIMODE_CW */       {PTR_NULL,          PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211a*/    {"11a",             PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211b*/    {PTR_NULL,          PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211g */   {PTR_NULL,          PTR_NULL,           PTR_NULL},
    /*AT_WIMODE_80211n*/    {"11na20",          "11na40",           PTR_NULL},
    /*AT_WIMODE_80211ac */  {"11ac20",          "11ac40",           "11ac80"},
};

typedef struct
{
    uint32 brate;
    int8 rate_str[WIFI_CMD_8_SIZE];
}BRATE_ST;

/*全局变量*/
uint8   g_uc_add_user_done = 0;

/*命令类型*/
enum WAL_ATCMDSRV_IOCTL_CMD
{
    WAL_ATCMDSRV_IOCTL_CMD_NORM_SET         = 0,
    WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET      ,
    WAL_ATCMDSRV_IOCTL_CMD_VAP_DOWN_SET     ,
    WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET      ,
    WAL_ATCMDSRV_IOCTL_CMD_VAP_UP_SET       ,
    WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET    ,
    WAL_ATCMDSRV_IOCTL_CMD_REGINFO_SET      ,
    WAL_ATCMDSRV_IOCTL_CMD_CALIINFO_SET     ,
    HWIFI_IOCTL_CMD_TEST_BUTT
};

/*wifi驱动AT命令解析入口，驱动在初始化的时候会将接口注册给该全局变量*/
typedef uint32 *(*hipriv_entry_t)(void *, void*, void *);
static hipriv_entry_t g_at_hipriv_entry = NULL;

/*******************************************************************************
    通用宏
*******************************************************************************/
#define HIPRIV_BEEN_DONE(ret, cmd, ...) do {    \
        WLAN_TRACE_INFO("ret:%d run:"cmd"\n",(ret),##__VA_ARGS__);    \
        if (ret) return (ret);    \
}while(0)

/*向WiFi芯片下发配置命令*/
#define WIFI_CMD_RET(_ret)                     ((int32)((0 == (_ret))?  AT_RETURN_SUCCESS: AT_RETURN_FAILURE))
#define WIFI_TEST_CMD(_cmd_id, _cmd)          WIFI_CMD_RET(g_at_hipriv_entry(IF_HIPRIV, (_cmd_id), (_cmd)))
/* 打印wifi vap信息 */
#define WIFI_VAP_INFO(_vap)     \
        WLAN_TRACE_INFO("%4d_%4d_%4d_%4d_%4d_%4d_%4d_%4d_%4d_%4d_%4d_%4X_%4d\n", \
            (_vap).wifiStatus, \
            (_vap).wifiMode, \
            (_vap).wifiBandwith, \
            (_vap).wifiBand, \
            (_vap).wifiFreq.value, \
            (_vap).wifiChannel, \
            (_vap).wifiRate, \
            (_vap).wifiPower, \
            (_vap).wifiTX, \
            (_vap).wifiRX.onoff, \
            (_vap).wifiPckg.good_result, \
            (_vap).wifiGroup, \
            (_vap).wifiFirstTestItem);

#ifndef STATIC
    #define STATIC static
#endif

/*WiFi全局变量结构体 */
typedef struct tagWlanATGlobal
{
    WLAN_AT_WIENABLE_TYPE   wifiStatus;    /*默认加载测试模式*/
    WLAN_AT_WIMODE_TYPE     wifiMode;      /*wifi协议模式*/
    WLAN_AT_WIBAND_TYPE     wifiBandwith;  /*wifi协议制式*/
    WLAN_AT_WIFREQ_TYPE     wifiBand;      /*wifi当前频段*/
    WLAN_AT_WIFREQ_STRU     wifiFreq;      /*wifi频点信息*/
    uint32                  wifiChannel;   /*wifi信道信息*/
    uint32                  wifiRate;      /*wifi发射速率*/
    int32                   wifiPower;     /*wifi发射功率*/
    WLAN_AT_FEATURE_TYPE    wifiTX;        /*wifi发射机状态*/
    WLAN_AT_WIRX_STRU       wifiRX;        /*wifi接收机状态*/
    WLAN_AT_WIRPCKG_STRU    wifiPckg;      /*wifi误包码*/
    uint32                  wifiGroup;     /* wifi天线模式:(0,1是2G通道0,1)(2,3是5G通道0,1) */
    uint32                  wifiFirstTestItem;  /* 记录当前是否为wifi 2g(420/617)或5g的第一个测试项 */
}WLAN_AT_GLOBAL_ST;

/* 记录当前的WiFi模式，带宽，频率，速率等参数, 0xFF代表天线模式为无效 */
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_OFF, AT_WIMODE_80211n, AT_WIBAND_20M
         , AT_WIFREQ_24G, {2412, 0}, 1, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}
         , {0,0}, 0xFF, 0};/*开启WiFi的默认参数*/

/* 数字2412代表wifi当前频率,6500代表wifi速率,3175代表wifi发射功率,0xFF代表天线模式为无效 */
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data_old = {AT_WIENABLE_OFF, AT_WIMODE_80211n, AT_WIBAND_20M
         , AT_WIFREQ_24G, {2412, 0}, 1, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}
         , {0,0}, 0xFF, 0};/* 备份WiFi参数 */

/*******************************************************************************
    提供给驱动的注册接口
*******************************************************************************/
int reg_at_hipriv_entry(hipriv_entry_t hipriv_entry)
{
    if (NULL == hipriv_entry)
    {
        WLAN_TRACE_ERROR("reg_at_ioctl_entry null point error! ioctl:%p\n", hipriv_entry);
        return -1;
    }
    g_at_hipriv_entry = hipriv_entry;
    return 0;
}
EXPORT_SYMBOL(reg_at_hipriv_entry);
/*****************************************************************************
函数名称  : int unreg_at_hipriv_entry()
功能描述  : wifi驱动的AT接口置空
输入参数  : hipriv_entry: 驱动AT入口指针
输出参数  : NA
返 回 值  : 返回切换结果,0:成功
*****************************************************************************/
int unreg_at_hipriv_entry(hipriv_entry_t hipriv_entry)
{
    g_at_hipriv_entry = NULL;
    return (0);
}

#if (MBB_WIFI_RF_PATH_SWITCH == FEATURE_ON)
/*****************************************************************************
函数名称  : int32 wifi_switch_saw_notify_hisi()
功能描述  : 通知海思wifi驱动更新射频参数
输入参数  : rf_path_index : 0:RF_PATH_NOB40,1:RF_PATH_B40
输出参数  : NA
返 回 值  : 返回切换结果,0:成功,-1:失败
*****************************************************************************/
int32 wifi_switch_saw_notify_hisi(int rf_path_index)
{
    int ret = 0;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "Hisilicon0 set_2g_path %d", rf_path_index);
    if (NULL == g_at_hipriv_entry)
    {
        WLAN_TRACE_INFO("[wifi_switch_saw_notify_hisi] g_at_hipriv_entry is NULL!\n");
        return (-1);
    }
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    WLAN_TRACE_INFO("wifi_switch_saw_notify_hisi rf_path_index:%d,cmd=%s\n", rf_path_index, wl_cmd);
    return ret;
}
#endif
//////////////////////////////////////////////////////////////////////////
/*(1)^WIENABLE 设置WiFi模块使能 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
 功能描述  : 用于wifi 进入测试模式，正常模式，关闭wifi
 输入参数  :  0  关闭
              1  打开正常模式
              2  打开测试模式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
{
    int32 ret = AT_RETURN_SUCCESS;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    if (NULL == g_at_hipriv_entry)
    {
        WLAN_TRACE_INFO("check_wifi_valid failed!\n");
        return AT_RETURN_FAILURE;
    }

    /* wake up vap */
    (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "Hisilicon0 enable 1", IF_HIPRIV);
    (void)WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    (void)memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "Hisilicon0 pm_enable 0", IF_HIPRIV);
    (void)WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET, "1");
    if (AT_RETURN_SUCCESS != ret)
    {
        WLAN_TRACE_INFO("chip_check failed!\n");
        return ret;
    }

    /* 默认关闭动态功率校准,0xFF代表wifi天线模式为无效 */
    g_wlan_at_data.wifiGroup = 0xFF;
    g_wlan_at_data.wifiFirstTestItem = 0;
    (void)memcpy(&g_wlan_at_data_old, &g_wlan_at_data, sizeof(g_wlan_at_data_old));
    WIFI_VAP_INFO(g_wlan_at_data_old);
    WIFI_VAP_INFO(g_wlan_at_data);

    switch (onoff)
    {
        case AT_WIENABLE_OFF:
            {
                WLAN_TRACE_INFO("Set wifi to off mode\n");
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_VAP_DOWN_SET, "1");
                HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 down");
                g_wlan_at_data.wifiStatus = AT_WIENABLE_OFF;
                g_uc_add_user_done = 0;
            }
            break;
        case AT_WIENABLE_ON:
            {
                WLAN_TRACE_INFO("Set wifi to normal mode\n");
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_VAP_UP_SET, "1");
                HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 up");
                g_wlan_at_data.wifiStatus = AT_WIENABLE_ON;
            }
            break;
        case AT_WIENABLE_TEST:
            {
                WLAN_TRACE_INFO("Set wifi to test mode\n");
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_VAP_UP_SET, "1");
                HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 up");
                /* 添加寄存器配置命令，用来关闭wifi的Beacon帧 */
                (void)memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s al_tx_51 1", IF_HIPRIV);
                ret += WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
                HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

                (void)memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s al_tx_51 0", IF_HIPRIV);
                ret += WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
                HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

                g_wlan_at_data.wifiStatus = AT_WIENABLE_TEST;
            }
            break;
        default:
            ret = AT_RETURN_FAILURE;
            break;
    }

    return ret;
}
/*****************************************************************************
 函数名称  : WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable()
 功能描述  : 获取当前的WiFi模块使能状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0  关闭
             1  正常模式(信令模式)
             2  测试模式(非信令模式)
 其他说明  :
*****************************************************************************/
STATIC WLAN_AT_WIENABLE_TYPE ATGetWifiEnable(void)
{
    int32 ret = AT_RETURN_SUCCESS;

    if(NULL == g_at_hipriv_entry)
    {
        WLAN_TRACE_INFO("check_wifi_valid failed!\n");
        return AT_WIENABLE_OFF;
    }

    /* chip check */
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_CHIPCHECK_SET, "1");

    if (AT_RETURN_SUCCESS != ret)
    {
        WLAN_TRACE_INFO("chip_check failed!\n");
        return AT_WIENABLE_OFF;
    }
    else
    {
        return g_wlan_at_data.wifiStatus;
    }
}

//////////////////////////////////////////////////////////////////////////
/*(2)^WIMODE 设置WiFi模式参数 目前均为单模式测试*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 功能描述  : 设置WiFi AP支持的制式
 输入参数  : 0,  CW模式
             1,  802.11a制式
             2,  802.11b制式
             3,  802.11g制式
             4,  802.11n制式
             5,  802.11ac制式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
{
    int ret = 0;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    WLAN_TRACE_INFO("%s:enter\n", __FUNCTION__);
    WLAN_TRACE_INFO("WLAN_AT_WIMODE_TYPE:%d\n", mode);

    if (mode > AT_WIMODE_MAX)
    {
        WLAN_TRACE_ERROR("WLAN_AT_WIMODE_TYPE:%d unknow!\n", mode);
        return (AT_RETURN_FAILURE);
    }

    /*双通道天线在模式中配置*/
    if ((AT_WIMODE_80211n == mode)
        || (AT_WIMODE_80211ac == mode))
    {
        /*MIMO 模式*/
        if((RF_2G_MIMO == g_wlan_at_data.wifiGroup) || (RF_5G_MIMO == g_wlan_at_data.wifiGroup))
        {
            /*天线模式*/
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "%s txch 0011", IF_HIPRIV);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "%s rxch 0011", IF_HIPRIV);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
        }
    }

    g_wlan_at_data.wifiMode = mode;
    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取当前WiFi支持的制式
             当前模式，以字符串形式返回eg: 2
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiMode);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取WiFi芯片支持的所有协议模式
             支持的所有模式，以字符串形式返回eg: 2,3,4
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_MODE_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(3)^WIBAND 设置WiFi带宽参数 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width)
 功能描述  : 用于设置wifi带宽
 输入参数  : 0 20M
             1 40M
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 只有在n模式下才可以设置40M带宽
*****************************************************************************/
STATIC int32 ATSetWifiBand(WLAN_AT_WIBAND_TYPE band)
{
    int32 ret = AT_RETURN_SUCCESS;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    WLAN_TRACE_INFO("%s:enter\n", __FUNCTION__);
    WLAN_TRACE_INFO("WLAN_AT_WIBAND_TYPE:%d\n", band);


    switch(band)
    {
        case AT_WIBAND_20M:
        {
            g_wlan_at_data.wifiBandwith = AT_WIBAND_20M;
            break;
        }
        case AT_WIBAND_40M:
        {
            if((AT_WIMODE_80211n == g_wlan_at_data.wifiMode ) || (AT_WIMODE_80211ac == g_wlan_at_data.wifiMode ))
            {
                g_wlan_at_data.wifiBandwith = AT_WIBAND_40M;
            }
            else
            {
                WLAN_TRACE_ERROR("Error wifi mode for bandwidth 40M,must in n or ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        case AT_WIBAND_80M:
        {
            if(AT_WIMODE_80211ac == g_wlan_at_data.wifiMode )
            {
                g_wlan_at_data.wifiBandwith = AT_WIBAND_80M;
            }
            else
            {
                WLAN_TRACE_ERROR("Error wifi mode for bandwidth 80M,must in ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        default:
            ret = AT_RETURN_FAILURE;
            break;
    }


    /*配置模式*/
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    if(AT_WIFREQ_24G == g_wlan_at_data.wifiBand)
    {
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s mode %s", IF_HIPRIV, auc_mode_table_2G[g_wlan_at_data.wifiMode][g_wlan_at_data.wifiBandwith]);
    }
    else
    {
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s mode %s", IF_HIPRIV, auc_mode_table_5G[g_wlan_at_data.wifiMode][g_wlan_at_data.wifiBandwith]);
    }
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取当前带宽配置
            当前带宽，以字符串形式返回eg: 0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiBandwith);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取WiFi支持的带宽配置
            支持带宽，以字符串形式返回eg: 0,1
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_BAND_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    /*2.4G频点集合*/
    const uint16   ausChannels[] = {2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472};/*2.4G*/

/*BCM4354芯片 应装备归一化要求，采用当前带宽下的第一个20M频点表示当前信道的频点*/
    /*5G 20M频点集合*/
    const uint16 aulChannel036[] = {5180,5200,5220,5240,5260,5280,5300,5320};/*w52和w53*/
    const uint16 aulChannel100[] = {5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5720};/*w56*/
    const uint16 aulChannel149[] = {5745,5765,5785,5805,5825};/*w57*/

    /*2.4G 40M频点集合,对应装备信道3至11*/
                                      /* 1+,   2+,   3+,   4+,   5+,   6+,   7+,   8+,   9+ */
    const uint16   ausChannels_40M[] = {2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462};/*2.4G 40M*/
    /*5G 40M频点集合*/
    const uint16 aulChannel036_40M[] = {5180, 5220, 5260, 5300};/*5G 40M*/
    const uint16 aulChannel100_40M[] = {5500, 5540, 5580, 5620, 5660,5700};/*5G 40M*/
    const uint16 aulChannel149_40M[] = {5745, 5785};/*5G 40M*/
    /*5G 80M频点集合*/
    const uint16 aulChannel036_80M[] = {5180, 5260};/*5G 80M*/
    const uint16 aulChannel100_80M[] = {5500, 5580,5660};/*5G 80M*/
    const uint16 aulChannel149_80M[] = {5745};/*5G 80M*/

    char ul_for_40M = '\0';//初始化为0
    uint16 ulWifiFreq = 0;
    uint16 i = 0;
    int32 ret = AT_RETURN_SUCCESS;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    /*20M带宽*/
    if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
    {
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            for (i = 0; i < (sizeof(ausChannels) / sizeof(uint16)); i++)
            {
                if (pFreq->value == ausChannels[i])
                {
                    ulWifiFreq = (i + 1);
                    break;
                }
            }
        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为

        case 36:
            iWIFIchannel = 5180;
            break;
        case 40:
            iWIFIchannel = 5200;
            break;
        case 44:
            iWIFIchannel = 5220;
            break;
        case 48:
            iWIFIchannel = 5240;
            break;
        case 52:
            iWIFIchannel = 5260;
            break;
        case 56:
            iWIFIchannel = 5280;
            break;
        case 60:
            iWIFIchannel = 5300;
            break;
        case 64:
            iWIFIchannel = 5320;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W52_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为

        case 100:
            iWIFIchannel = 5500;
            break;
        case 104:
            iWIFIchannel = 5520;
            break;
        case 108:
            iWIFIchannel = 5540;
            break;
        case 112:
            iWIFIchannel = 5560;
            break;
        case 116:
            iWIFIchannel = 5580;
            break;
        case 120:
            iWIFIchannel = 5600;
            break;
        case 124:
            iWIFIchannel = 5620;
            break;
        case 128:
            iWIFIchannel = 5640;
            break;
        case 132:
            iWIFIchannel = 5660;
            break;
        case 136:
            iWIFIchannel = 5680;
            break;
        case 140:
            iWIFIchannel = 5700;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + HUNDRED);
                    break;
                }
            }

        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为
        case 149:
            iWIFIchannel = 5745;
            break;
        case 153:
            iWIFIchannel = 5765;
            break;
        case 157:
            iWIFIchannel = 5785;
            break;
        case 161:
            iWIFIchannel = 5805;
            break;
        case 165:
            iWIFIchannel = 5825;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {
            WLAN_TRACE_INFO("Error 20M wifiFreq parameters\n");
            return AT_RETURN_FAILURE;
        }

        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);

        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))
            || ((WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))))
        {
            WLAN_TRACE_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }

    }
    else if(AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
    {
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            for (i = 0; i < (sizeof(ausChannels_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == ausChannels_40M[i])
                {
                    ulWifiFreq = (i + 1);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W52_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + HUNDRED);
                    break;
                }
            }

        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {
            WLAN_TRACE_INFO("Error 40M wifiFreq parameters\n");
            return AT_RETURN_FAILURE;
        }

        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))
            || ((WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))))
        {
            WLAN_TRACE_INFO("Target Channel ERROR!\n");
            return AT_RETURN_FAILURE;
        }
    }
    else if(AT_WIBAND_80M == g_wlan_at_data.wifiBandwith)
    {
        if ((pFreq->value >= WLAN_FREQ_5G_W52_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + HUNDRED);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {
            WLAN_TRACE_ERROR("Error 80M wifiFreq parameters %u!\n",pFreq->value);
            return AT_RETURN_FAILURE;
        }
        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);
        if (!(WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))
        {
            WLAN_TRACE_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }

    }

    /*配置信道*/
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s freq %d", IF_HIPRIV, ulWifiFreq);
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    /* 保存全局变量里，以备查询 */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    g_wlan_at_data.wifiChannel = ulWifiFreq;

    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 获取WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    memcpy(pFreq, &(g_wlan_at_data.wifiFreq), sizeof(WLAN_AT_WIFREQ_STRU));
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiDataRate(uint32 rate)
{
    int32 ret = 0;
    uint32  ulNRate = 0;
    uint32  ulacSS = 1; /* 11ac spatial stream,1--SISO, 2--MIMO*/
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    int8 *wifi_bw[] = {"20", "40", "80"};
    /*BG模式下直接传rate参数，因此不需要进行转换*/
    const BRATE_ST wifi_brates_table[] = {{100, "1"}, {200, "2"}, {550, "5.5"}, {1100, "11"}};

    /* WIFI n 和ac 模式 AT^WIDATARATE设置的速率值和WL命令速率值的对应表 */
    const uint32 wifi_20m_nrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, \
                                                                1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000};
    const uint32 wifi_40m_nrates_table[] =  {1350, 2700, 4050, 5400, 8100, 10800, 12150, 13500, \
                                                                 2700, 5400, 8100, 10800, 16200, 21600, 24300, 27000};
    const uint32 wifi_20m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 7800,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,15600};
    const uint32 wifi_40m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 16200, 18000,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,32400,36000};
    const uint32 wifi_80m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 35100, 39000,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,70200,78000};

    //#define WIFI_BRATES_TABLE_SIZE (sizeof(wifi_brates_table) / sizeof(BRATE_ST))
    #define WIFI_20M_NRATES_TABLE_SIZE (sizeof(wifi_20m_nrates_table) / sizeof(uint32))
    #define WIFI_40M_NRATES_TABLE_SIZE (sizeof(wifi_40m_nrates_table) / sizeof(uint32))

   #define WIFI_20M_ACRATES_TABLE_SIZE (sizeof(wifi_20m_acrates_table) / sizeof(uint32))
   #define WIFI_40M_ACRATES_TABLE_SIZE (sizeof(wifi_40m_acrates_table) / sizeof(uint32))
   #define WIFI_80M_ACRATES_TABLE_SIZE (sizeof(wifi_80m_acrates_table) / sizeof(uint32))

   WLAN_TRACE_INFO("WifiRate = %u\n", rate / HUNDRED);

    /*配置带宽*/
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd,  sizeof(wl_cmd), " %s bw %s", IF_HIPRIV, wifi_bw[g_wlan_at_data.wifiBandwith]);
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    /*配置速率*/
    switch (g_wlan_at_data.wifiMode)
    {
        case AT_WIMODE_CW:
            WLAN_TRACE_INFO("AT_WIMODE_CW\n");
            return (AT_RETURN_FAILURE);
        case AT_WIMODE_80211b:
            for (ulNRate = 0; ulNRate < WIFI_20M_NRATES_TABLE_SIZE; ulNRate++)
            {
                if (wifi_brates_table[ulNRate].brate == rate)
                    {
                    WLAN_TRACE_INFO("20M NRate Index = %u\n", ulNRate);
                    break;
                }
            }

            if (WIFI_20M_NRATES_TABLE_SIZE == ulNRate)
            {
                WLAN_TRACE_ERROR("20M NRate Error!\n");
                return (AT_RETURN_FAILURE);
            }

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s rate %s", IF_HIPRIV, wifi_brates_table[ulNRate].rate_str);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
            break;
        case AT_WIMODE_80211a:
        case AT_WIMODE_80211g:
            /*驱动内部对11ag的速率值进行校验*/
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s rate %d", IF_HIPRIV, rate / HUNDRED);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
            break;
        case AT_WIMODE_80211n:
            if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
            {
                /* WIFI 20M n模式WL命令的速率值为0~15，共16个 */
                for (ulNRate = 0; ulNRate < WIFI_20M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_20m_nrates_table[ulNRate] == rate)
                    {
                        if(((RF_2G_MIMO == g_wlan_at_data.wifiGroup) || (RF_5G_MIMO == g_wlan_at_data.wifiGroup))
                            && (ulNRate < (WIFI_20M_NRATES_TABLE_SIZE/2)))
                        {
                            //1300, 2600, 3900, 5200
                        }
                        else
                        {
                            WLAN_TRACE_INFO("20M NRate Index = %u\n", ulNRate);
                            break;
                        }
                    }
                }

                if (WIFI_20M_NRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("20M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_40M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_nrates_table[ulNRate] == rate)
                    {
                        if(((RF_2G_MIMO == g_wlan_at_data.wifiGroup) || (RF_5G_MIMO == g_wlan_at_data.wifiGroup))
                            && (ulNRate < (WIFI_20M_NRATES_TABLE_SIZE/2)))
                        {
                            //2700, 5400, 8100, 10800
                        }
                        else
                        {
                            WLAN_TRACE_INFO("40M NRate Index = %u\n", ulNRate);
                            break;
                        }
                    }
                }

                if (WIFI_40M_NRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("40M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s mcs %d", IF_HIPRIV, ulNRate);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
            break;
        case AT_WIMODE_80211ac:
            if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_20M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_20m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_20M_ACRATES_TABLE_SIZE/2))
                        {
                            if(g_wlan_at_data.wifiGroup < WLAN_AT_GROUP_MAX)
                            {
                                ulacSS = 1;
                                WLAN_TRACE_INFO("20M ACRate Index = %u\n", ulNRate);
                                break;
                            }
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_20M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("20M ACRate Index = %u\n", ulNRate);
                            break;
                        }
                    }
                }
                if (WIFI_20M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("20M ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_40M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_40M_ACRATES_TABLE_SIZE/2))
                        {
                            if(g_wlan_at_data.wifiGroup < WLAN_AT_GROUP_MAX)
                            {
                                ulacSS = 1;
                                WLAN_TRACE_INFO("40M ACRate Index = %u\n", ulNRate);
                                break;
                            }
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_40M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("40M ACRate Index = %u\n", ulNRate);
                            break;
                        }
                    }
                }
                if (WIFI_40M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("40M ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            else if(AT_WIBAND_80M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_80M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_80m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_80M_ACRATES_TABLE_SIZE/2))
                        {
                            if(g_wlan_at_data.wifiGroup < WLAN_AT_GROUP_MAX)
                            {
                                ulacSS = 1;
                                WLAN_TRACE_INFO("80M ACRate Index = %u\n", ulNRate);
                                break;
                            }
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_80M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("80M ACRate Index = %u\n", ulNRate);
                            break;
                        }
                    }
                }
                if (WIFI_80M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("80M ACRate Error! rate=%d\n", rate);
                    return (AT_RETURN_FAILURE);
                }
            }
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s mcsac %d", IF_HIPRIV, ulNRate);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

            //if(ulacSS > 1)
            //{
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s nss %d", IF_HIPRIV, ulacSS);
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
                HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
            //}

            break;
        default:
            return (AT_RETURN_FAILURE);
    }

    /*保存全局变量里，以备查询*/
    g_wlan_at_data.wifiRate = rate;

    return ret;
}
/*****************************************************************************
 函数名称  : uint32 WlanATGetWifiDataRate()
 功能描述  : 查询当前WiFi速率设置
 输入参数  : NA
 输出参数  : NA
 返 回 值  : wifi速率
 其他说明  :
*****************************************************************************/
STATIC uint32 ATGetWifiDataRate(void)
{
    return g_wlan_at_data.wifiRate;
}

//////////////////////////////////////////////////////////////////////////
/*(6)^WIPOW 来设置WiFi发射功率
   WiFi发射功率，单位为0.01dBm，取值范围为 -32768～32767 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiPOW(int32 power_dBm_percent)
 功能描述  : 设置WiFi发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiPOW(int32 power_dBm_percent)
{
    int ret = AT_RETURN_SUCCESS;
    int32 lWifiPower = power_dBm_percent / HUNDRED;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    if ( WLAN_AT_POW_DEFAULT == lWifiPower )
    {
        return (AT_RETURN_SUCCESS);
    }

    if ((lWifiPower >= WLAN_AT_POWER_MIN) && (lWifiPower <= WLAN_AT_POWER_MAX))
    {
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s txpower %d", IF_HIPRIV, lWifiPower);
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
        HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
    }
    else
    {
        WLAN_TRACE_INFO("Invalid argument for WifiPOW\n");
        ret = AT_RETURN_FAILURE;
    }

    if ( AT_RETURN_SUCCESS ==  ret)
    {
         /*保存全局变量里，以备查询*/
        g_wlan_at_data.wifiPower = power_dBm_percent;
    }

    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiPOW()
 功能描述  : 获取WiFi当前发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiPOW(void)
{
    return g_wlan_at_data.wifiPower;
}

/*****************************************************************************
 函数名称  : int trigger_dync_adjust()
 功能描述  : 触发动态功率校准
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
 *****************************************************************************/
STATIC int trigger_dync_adjust(void)
{
    int ret = 0;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "Hisilicon0 dync_txpower 1");
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    msleep(DYNC_ADJUST_DELAY);
    (void)memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    (void)OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), "Hisilicon0 dync_txpower 0");
    ret += WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    return ret;
}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{
    int ret = 0;
    int i = 0;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};

    if (onoff > AT_FEATURE_ENABLE)
    {
        WLAN_TRACE_ERROR("ATSetWifiTX:%d unknow!\n", ATSetWifiTX);
        return (AT_RETURN_FAILURE);
    }

    /* 保存全局变量里，已备查询 */
    g_wlan_at_data.wifiTX = onoff;

    if (AT_WIMODE_CW == g_wlan_at_data.wifiMode)
    {
        if (AT_FEATURE_DISABLE == onoff)
        {
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s cfg_cw_signal %d", IF_HIPRIV, onoff);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
        }
        else
        {
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s cfg_cw_signal %d", IF_HIPRIV, (g_wlan_at_data.wifiGroup % 2 + 1));
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
        }

        return ret;
    }

    if (AT_FEATURE_ENABLE == onoff)
    {
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s add_user 07:06:05:04:03:02 1", IF_HIPRIV);
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
        HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
    }

    OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s al_tx_51 %d", IF_HIPRIV, onoff);
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    if (AT_FEATURE_ENABLE == onoff)
    {
        WIFI_VAP_INFO(g_wlan_at_data_old);
        WIFI_VAP_INFO(g_wlan_at_data);

        /* 使能动态校准 */
        /* 1)第一次切换到射频通道0时，适用于2G 617、420 saw和5G(ANT1\ANT2)的第一个测试项 */
        if ((g_wlan_at_data.wifiGroup != g_wlan_at_data_old.wifiGroup)
            && ((RF_2G_0 == g_wlan_at_data.wifiGroup) || (RF_5G_0 == g_wlan_at_data.wifiGroup) || (RF_5G_1 == g_wlan_at_data.wifiGroup)))
        {
            WLAN_TRACE_INFO("ATSetWifiTX::This is the first item!\n");
            g_wlan_at_data.wifiFirstTestItem = 1;
            ret = trigger_dync_adjust();
            if (0 != ret)
            {
                return ret;
            }
        }

        /* 2)如果是第一个测试项，且两次配置完全相同，认为装备重试，
                    需要重新动态校准；如果配置不同，则测试项已切换 */
        else if (1 == g_wlan_at_data.wifiFirstTestItem)
        {
            ret = memcmp(&g_wlan_at_data_old, &g_wlan_at_data, sizeof(g_wlan_at_data));
            if(0 == ret)
            {
                WLAN_TRACE_INFO("ATSetWifiTX::This is the first item retry again!\n");
                ret = trigger_dync_adjust();
                if (0 != ret)
                {
                    return ret;
                }
            }
            else
            {
                WLAN_TRACE_INFO("ATSetWifiTX::This is to clean wifiFirstTestItem flag!\n");
                g_wlan_at_data.wifiFirstTestItem = 0;
            }
        }
        else
        {
            WLAN_TRACE_INFO("ATSetWifiTX::This is the no use branch!\n");
        }

        (void)memcpy(&g_wlan_at_data_old, &g_wlan_at_data, sizeof(g_wlan_at_data_old));
    }

    WLAN_TRACE_INFO("ATSetWifiTX::g_wlan_at_data.wifiFirstTestItem=%d!\n",
        g_wlan_at_data.wifiFirstTestItem);

    return (AT_RETURN_SUCCESS);
}
/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiTX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  :
*****************************************************************************/
STATIC WLAN_AT_FEATURE_TYPE ATGetWifiTX(void)
{
    return g_wlan_at_data.wifiTX;
}

/*****************************************************************************
 函数名称  : static uint8 charToData()
 功能描述  : 字符转整数
 输入参数  : NA
 输出参数  : NA
 返 回 值  :
 其他说明  :
*****************************************************************************/
static uint8 charToData(const char ch)
{
    switch(ch)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    }
    return 0;
}

/***************************************************************************
 函数名称  : wl_ether_atoe()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : const char *a  ,struct ether_addr *n
 输出参数  : NA
 返 回 值  : NA
 其他说明  :

****************************************************************************/
static void wl_ether_atoe(const char *a, uint8* addr)
{
    char *c = NULL;
    int i = 0;
    int j = 0;
    if (NULL == a || NULL == addr || '\0' == *a)
    {
        WLAN_TRACE_ERROR("params error\n");
        return;
    }
    for (i = 0; i < MAC_ADDRESS_LEN; i++)
    {
        addr[i] = charToData(*a++) * 16;    //十六进制转十进制
        addr[i] += charToData(*a++);
        if ('\0' == *a)
        {
            //WLAN_TRACE_INFO("The End of MAC address!!\n");
            break;
        }
        a++; //跳过冒号
    }
    return ;
}


//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
STATIC int32 ATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    int ret = AT_RETURN_SUCCESS;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    uint8 addr[MAC_ADDRESS_LEN] = {0};

    if ((NULL == params) || (params->onoff > AT_FEATURE_ENABLE))
    {
        return (AT_RETURN_FAILURE);
    }

    /*配置mac地址和用户*/
    if (0 != strncmp(params->src_mac,"",MAC_ADDRESS_LEN))
    {
        WLAN_TRACE_INFO("params->src_mac is %s\n",params->src_mac);
        wl_ether_atoe(params->src_mac, &addr);

        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_VAP_DOWN_SET, "1");
        HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 down");
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_HW_ADDR_SET, addr);
        HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 hw ether");
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_VAP_UP_SET, "1");
        HIPRIV_BEEN_DONE(ret, "%s", "ifconfig vap0 up");

        if(0 == g_uc_add_user_done)
        {
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s add_user 07:06:05:04:03:02 1", IF_HIPRIV);
            ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
            HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
            g_uc_add_user_done++;
        }
    }
    else
    {
        WLAN_TRACE_ERROR("src mac is NULL\n");
    }

    /*常收*/
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s al_rx_51 %d", IF_HIPRIV, params->onoff);
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
    HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    memcpy(&g_wlan_at_data.wifiRX, params, sizeof(WLAN_AT_WIRX_STRU));

    return (ret);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 功能描述  : 获取wifi接收机的状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    if (NULL == params)
    {
        return (AT_RETURN_FAILURE);
    }

    memcpy(params, &g_wlan_at_data.wifiRX, sizeof(WLAN_AT_WIRX_STRU));

    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(9)^WIRPCKG 查询WiFi接收机误包码，上报接收到的包的数量*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 清除Wifi接收统计包为零
 输入参数  : 0 清除wifi统计包
             非0 无效参数
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetWifiRPCKG(int32 flag)
{
    int ret = AT_RETURN_SUCCESS;
    int32   good_result = 0;

    if (0 != flag)
    {
        WLAN_TRACE_INFO("Exit on flag = %d\n", flag);
        return (AT_RETURN_FAILURE);
    }
    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET, &good_result);

    return (ret);
}


/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 查询WiFi接收机误包码，上报接收到的包的数量
 输入参数  : WLAN_AT_WIRPCKG_STRU *params
 输出参数  : uint16 good_result; //单板接收到的好包数，取值范围为0~65535
             uint16 bad_result;  //单板接收到的坏包数，取值范围为0~65535
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiRPCKG(WLAN_AT_WIRPCKG_STRU *params)
{
    int32 ret = AT_RETURN_SUCCESS;
    uint8 auc_result[2] = {0};

    if (NULL == params)
    {
        WLAN_TRACE_ERROR("%s:POINTER_NULL!\n", __FUNCTION__);
        ret = AT_RETURN_FAILURE;
        return ret;
    }

    /* 判断接收机是否打开 */
    if(AT_FEATURE_DISABLE == g_wlan_at_data.wifiRX.onoff)
    {
        WLAN_TRACE_ERROR("%s:Not Rx Mode.\n", __FUNCTION__);
        ret = AT_RETURN_FAILURE;
        return ret;
    }

    ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_RX_PCKG_GET, auc_result);
    params->good_result = (auc_result[0]<<8) + auc_result[1];
    WLAN_TRACE_INFO("auc_result=%X_%X\n", auc_result[0], auc_result[1]);

    params->bad_result = 0;     /* 不关注坏包数 */
    WLAN_TRACE_INFO("Exit [good = %d, bad = %d]\n", params->good_result, params->bad_result);

    return (ret);
}

//////////////////////////////////////////////////////////////////////////
/*(10)^WIINFO 查询WiFi的相关信息*/
//////////////////////////////////////////////////////////////////////////
#define SIZE_OF_INFOGROUP(group) (sizeof(group) / sizeof(WLAN_AT_WIINFO_MEMBER_STRU))
/*****************************************************************************
 函数名称  : uint32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
 功能描述  : 查询WiFi的相关信息(内部接口)
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
{
    static char sChannels24G[] = "1,2,3,4,5,6,7,8,9,10,11,12,13";
    static char sChannels24G_n4[] = "3,4,5,6,7,8,9,10,11";
    static char sChannels5G[] = "36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,149,153,157,161,165";
    static char sChannels5G_ac4[] = "36,44,52,60,100,108,116,124,132,140,149,157";
    static char sChannels5G_ac8[] = "36,52,100,116,132,149";

    /*信道信息*/
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup0[] =
    {
        {"b", sChannels24G},
        {"g", sChannels24G},
        {"n", sChannels24G},
        {"n4", sChannels24G_n4}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup1[] =
    {
        {"b", sChannels24G},
        {"g", sChannels24G},
        {"n", sChannels24G},
        {"n4", sChannels24G_n4}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup2[] =
    {
        {"a", sChannels5G},
        {"n", sChannels5G},
        {"n4", sChannels5G_ac4},
        {"ac", sChannels5G},
        {"ac4", sChannels5G_ac4},
        {"ac8", sChannels5G_ac8}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup3[] =
    {
        {"a", sChannels5G},
        {"n", sChannels5G},
        {"n4", sChannels5G_ac4},
        {"ac", sChannels5G},
        {"ac4", sChannels5G_ac4},
        {"ac8", sChannels5G_ac8}
    };
    /*功率信息*/
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup0[] =
    {
        {"b", "120"},
        {"g", "120"},
        {"n", "120"},
        {"n4", "120"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup1[] =
    {
        {"b", "120"},
        {"g", "120"},
        {"n", "120"},
        {"n4", "120"}
    };
    static  WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup2[] =
    {
        {"a",   "100"},
        {"n",   "100"},
        {"n4",  "100"},
        {"ac",  "100"},
        {"ac4", "100"},
        {"ac8", "100"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup3[] =
    {
        {"a",   "100"},
        {"n",   "100"},
        {"n4",  "100"},
        {"ac",  "100"},
        {"ac4", "100"},
        {"ac8", "100"}
    };
    /*频段信息:0表示2.4G,1表示5G*/
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup0[] =
    {
        {"b", "0"},
        {"g", "0"},
        {"n", "0"},
        {"n4", "0"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup1[] =
    {
        {"b", "0"},
        {"g", "0"},
        {"n", "0"},
        {"n4", "0"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup2[] =
    {
        {"a",   "1"},
        {"n",   "1"},
        {"n4",  "1"},
        {"ac",  "1"},
        {"ac4", "1"},
        {"ac8", "1"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup3[] =
    {
        {"a",   "1"},
        {"n",   "1"},
        {"n4",  "1"},
        {"ac",  "1"},
        {"ac4", "1"},
        {"ac8", "1"}
    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup0[] =
    {
        {sChannelGroup0, SIZE_OF_INFOGROUP(sChannelGroup0)},
        {sPowerGroup0, SIZE_OF_INFOGROUP(sPowerGroup0)},
        {sFreqGroup0, SIZE_OF_INFOGROUP(sFreqGroup0)}
    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup1[] =
    {
        {sChannelGroup1, SIZE_OF_INFOGROUP(sChannelGroup1)},
        {sPowerGroup1, SIZE_OF_INFOGROUP(sPowerGroup1)},
        {sFreqGroup1, SIZE_OF_INFOGROUP(sFreqGroup1)}

    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup2[] =
    {
        {sChannelGroup2, SIZE_OF_INFOGROUP(sChannelGroup2)},
        {sPowerGroup2, SIZE_OF_INFOGROUP(sPowerGroup2)},
        {sFreqGroup2, SIZE_OF_INFOGROUP(sFreqGroup2)}

    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup3[] =
    {
        {sChannelGroup3, SIZE_OF_INFOGROUP(sChannelGroup3)},
        {sPowerGroup3, SIZE_OF_INFOGROUP(sPowerGroup3)},
        {sFreqGroup3, SIZE_OF_INFOGROUP(sFreqGroup3)}
    };

    static WLAN_AT_WIINFO_GROUP_STRU *sTotalInfoGroups[] =
    {
        sInfoGroup0,
        sInfoGroup1,
        sInfoGroup2,
        sInfoGroup3
    };

    char *strBuf = NULL;
    int32 idx = 0, iLen = 0, igroup = 0,itype = 0, iTmp = 0;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoGrup = NULL;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoType = NULL;

    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
    PLAT_WLAN_INFO("Enter ATGetWifiInfo [group=%d,type=%u]\n", params->member.group, params->type);

    igroup = (int32)params->member.group;
    if (WLAN_AT_GROUP_MAX < igroup)
    {
        return (AT_RETURN_FAILURE);
    }

    itype = (int32)params->type;
    if (WLAN_AT_TYPE_MAX < itype)
    {
        return (AT_RETURN_FAILURE);
    }

    strBuf = (char *)params->member.content;
    iLen = (int32)(sizeof(params->member.content) - 1);

    pstuInfoGrup = sTotalInfoGroups[igroup];

    pstuInfoType = &pstuInfoGrup[itype];
    for(idx = 0; idx < pstuInfoType->size; idx++)
    {
        if (NULL == pstuInfoType->member[idx].name
          || NULL == pstuInfoType->member[idx].value)
        {
            continue;
        }

        OSA_SNPRINTF(strBuf, iLen, "%s,%s"
                    , pstuInfoType->member[idx].name
                    , pstuInfoType->member[idx].value);

        iTmp = (int32)(strlen(strBuf) + 1);
        iLen -= iTmp;
        strBuf += iTmp;
        if (iLen <= 0)
        {
            return (AT_RETURN_FAILURE);
        }
    }

    *strBuf = '\0';
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(11)^WIPLATFORM 查询WiFi方案平台供应商信息 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 功能描述  : 查询WiFi方案平台供应商信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC WLAN_AT_WIPLATFORM_TYPE ATGetWifiPlatform(void)
{
    return (AT_WIPLATFORM_HISI);
}

//////////////////////////////////////////////////////////////////////////
/*(12)^TSELRF 查询设置单板的WiFi射频通路*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATSetTSELRF(uint32 group)
{
    int ret = AT_RETURN_SUCCESS;
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8* auc_rfch[3]= {"0000", "0001", "0010"};

    if(RF_5G_MIMO < group)
    {
        WLAN_TRACE_ERROR("WLAN_AT_GROUP_TYPE:%d unknow!\n", group);
        return AT_RETURN_FAILURE;
    }
    g_wlan_at_data.wifiGroup = group;
    WLAN_TRACE_INFO("[%s]:Enter,group = %u\n", __FUNCTION__, group);

    if ( AT_WIMODE_CW == g_wlan_at_data.wifiMode )
    {
        return (AT_RETURN_SUCCESS);
    }

    if(group < WLAN_AT_GROUP_MAX)
    {
        /*天线模式*/
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s txch %s", IF_HIPRIV, auc_rfch[(group % 2 + 1)]);
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
        HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);
        /*天线模式*/
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), " %s rxch %s", IF_HIPRIV, auc_rfch[(group % 2 + 1)]);
        ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, wl_cmd);
        HIPRIV_BEEN_DONE(ret, "%s", wl_cmd);

    }
    else
    {
        ;/*goup=4时在ATSetWifiMode中设置天线模式，
           其他值在 函数开始已经校验，这里不再处理*/
    }

    if (g_wlan_at_data.wifiGroup == RF_2G_0 || g_wlan_at_data.wifiGroup == RF_2G_1
     || g_wlan_at_data.wifiGroup == RF_2G_MIMO)
    {
        g_wlan_at_data.wifiBand = AT_WIFREQ_24G;
    }
    else if (g_wlan_at_data.wifiGroup == RF_5G_0 || g_wlan_at_data.wifiGroup == RF_5G_1
     || g_wlan_at_data.wifiGroup == RF_5G_MIMO)
    {
        g_wlan_at_data.wifiBand = AT_WIFREQ_50G;
    }

    return (ret);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetTSELRF(void)
 功能描述  : 获取天线
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 天线
 其他说明  :
*****************************************************************************/
int32 ATGetTSELRF(void) /* 获取天线 */
{
    return g_wlan_at_data.wifiGroup;
}
/*****************************************************************************
 函数名称  : int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的天线索引序列，以字符串形式返回eg: 0,1,2,3
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  :
*****************************************************************************/
STATIC int32 ATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
     OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%s", WLAN_AT_TSELRF_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 函数名称  : int32 ATWifiDebug(WLAN_AT_DEBUG_TYPE type, uint32 value1, uint32 *value2)
 功能描述  :^WIFIDEBUG 读写WIFI寄存器的相关信息
 输入参数  :
     <type> 查询类型1~5，可根据需求扩展功能
         1 单个或者连续寄存器读操作
         2 单个寄存器写操作
         3 读取所有寄存器值并写入文件(写入文件为/online/hal_all_reg_data*.txt)
         4 温补开关
         5 打印上电校准和动态校准时的相关参数 
     <value1> 输入配置，当type=1和2表示寄存器地址，type=3为空，其余为自定义配置；
     <value2> 输入/输出配置，当type=1和2表示寄存器值，type=3为空，其余为自定义配置；

 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
 其他说明  : NA
*****************************************************************************/
STATIC int32 ATWifiDebug(WLAN_AT_DEBUG_TYPE type, uint32 value1, uint32 *value2)
{

    int8 cmd[WIFI_CMD_MAX_SIZE] = {0};
    int32 ret = AT_RETURN_SUCCESS;

    if (NULL == g_at_hipriv_entry)
    {
        WLAN_TRACE_INFO("check_wifi_valid failed!\n");
        return AT_RETURN_FAILURE;
    }

    switch (type)
    {
        case AT_WIDEBUG_REG_READ: /* sample: AT^WIFIDEBUG=1,0x20038c00,2 */
            {
                if (NULL == value2)
                {
                    WLAN_TRACE_INFO("input param error!\n");
                    return AT_RETURN_FAILURE;
                }

                /* check reg address is valid or not */
                if ((HISI_SOC_REG_ADDR_MIN > value1)
                    || (HISI_SOC_REG_ADDR_MAX < value1))
                {
                    WLAN_TRACE_INFO("input soc register address invalid, must between \"0x%08X~0x%08X\"!\n",
                        HISI_SOC_REG_ADDR_MIN, HISI_SOC_REG_ADDR_MAX);
                    return AT_RETURN_FAILURE;
                }

                (void)OSA_SNPRINTF(cmd, sizeof(cmd), "soc 0x%x 0x%x", value1, value1);
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_REGINFO_SET, cmd);
                *value2 = *(uint32 *)cmd;
            }
            break;
        case AT_WIDEBUG_REG_WRITE: /* sample: AT^WIFIDEBUG=2,0x20038c00,0x10 */
            {
                if (NULL == value2)
                {
                    WLAN_TRACE_INFO("input param error!\n");
                    return AT_RETURN_FAILURE;
                }

                /* check reg address is valid or not */
                if ((HISI_SOC_REG_ADDR_MIN > value1)
                    || (HISI_SOC_REG_ADDR_MAX < value1))
                {
                    WLAN_TRACE_INFO("input soc register address invalid, must between \"0x%08X~0x%08X\"!\n",
                        HISI_SOC_REG_ADDR_MIN, HISI_SOC_REG_ADDR_MAX);
                    return AT_RETURN_FAILURE;
                }

                (void)OSA_SNPRINTF(cmd, sizeof(cmd), "Hisilicon0 regwrite soc 0x%x 0x%x", value1, *value2);
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, cmd);
            }
            break;
        case AT_WIDEBUG_REG_FILE: /* sample: AT^WIFIDEBUG=3 */
            {
                (void)OSA_SNPRINTF(cmd, sizeof(cmd), "Hisilicon0 get_all_regs");
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, cmd);
            }
            break;
        case AT_WIDEBUG_TEM_COM_SWTICH: /* sample: AT^WIFIDEBUG=4,1 */
            {
                /* check temperature compesation switch is valid or not */
                if ((0 != value1) && (1 != value1))
                {
                    WLAN_TRACE_INFO("input temperature compesation switch value \"%d\" invalid!\n", value1);
                    return AT_RETURN_FAILURE;
                }

                (void)OSA_SNPRINTF(cmd, sizeof(cmd), "Hisilicon0 dync_txpower %d", value1);
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_NORM_SET, cmd);
            }
            break;
        case AT_WIDEBUG_CALI_INTO_READ: /* sample: AT^WIFIDEBUG=5 */
            {
                WLAN_TRACE_INFO("AT^WIFIDEBUG=5 input point addr=%p\n", value2);
                ret = WIFI_TEST_CMD(WAL_ATCMDSRV_IOCTL_CMD_CALIINFO_SET, value2);
                /* 该指令用于装备测试结束时返回校准维测信息，装备要求该指令一定返回OK，避免单板测试fail */
                return AT_RETURN_SUCCESS;
            }
            break;
        default:
            {
                WLAN_TRACE_INFO("type %d not support\n", type);
                ret = AT_RETURN_FAILURE;
            }
            break;
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////

WLAN_CHIP_OPS hi1151_ops =
{
    .WlanATSetWifiEnable = ATSetWifiEnable,
    .WlanATGetWifiEnable = ATGetWifiEnable,

    .WlanATSetWifiMode   = ATSetWifiMode,
    .WlanATGetWifiMode   = ATGetWifiMode,
    .WlanATGetWifiBandSupport = ATGetWifiModeSupport,

    .WlanATSetWifiBand = ATSetWifiBand,
    .WlanATGetWifiBand = ATGetWifiBand,
    .WlanATGetWifiBandSupport = ATGetWifiBandSupport,

    .WlanATSetWifiFreq = ATSetWifiFreq,
    .WlanATGetWifiFreq = ATGetWifiFreq,

    .WlanATSetWifiDataRate = ATSetWifiDataRate,
    .WlanATGetWifiDataRate = ATGetWifiDataRate,

    .WlanATSetWifiPOW = ATSetWifiPOW,
    .WlanATGetWifiPOW = ATGetWifiPOW,

    .WlanATSetWifiTX = ATSetWifiTX,
    .WlanATGetWifiTX = ATGetWifiTX,

    .WlanATSetWifiRX = ATSetWifiRX,
    .WlanATGetWifiRX = ATGetWifiRX,

    .WlanATSetWifiRPCKG = ATSetWifiRPCKG,
    .WlanATGetWifiRPCKG = ATGetWifiRPCKG,
    .WlanATGetWifiPlatform = ATGetWifiPlatform,
    .WlanATGetWifiInfo = ATGetWifiInfo,

    .WlanATSetTSELRF = ATSetTSELRF,
    .WlanATGetTSELRF = NULL,
    .WlanATGetTSELRFSupport = ATGetTSELRFSupport,

    .WlanATSetWifiParange = NULL,
    .WlanATGetWifiParange = NULL,

    .WlanATGetWifiParangeSupport = NULL,

    .WlanATGetWifiCalTemp = NULL,
    .WlanATSetWifiCalTemp = NULL,
    .WlanATSetWifiCalData = NULL,
    .WlanATGetWifiCalData = NULL,
    .WlanATSetWifiCal = NULL,
    .WlanATGetWifiCal = NULL,
    .WlanATGetWifiCalSupport = NULL,
    .WlanATSetWifiCalFreq = NULL,
    .WlanATGetWifiCalFreq = NULL,
    .WlanATSetWifiCalPOW = NULL,
    .WlanATGetWifiCalPOW = NULL,
    .WlanATSetWifi2GPavars = NULL,
    .WlanATGetWifi2GPavars = NULL,
    .WlanATSetWifi5GPavars = NULL,
    .WlanATGetWifi5GPavars = NULL,

    .WlanATSetWifiDebug = ATWifiDebug,
};

/*===========================================================================
                       导出符号列表
===========================================================================*/
EXPORT_SYMBOL(unreg_at_hipriv_entry);

