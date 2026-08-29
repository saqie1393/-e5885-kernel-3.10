/******************************************************************************
  Copyright, 1988-2014, Huawei Tech. Co., Ltd.
  File name:      ublox_gps.h
  Author:         lishuai
  Version:        V1.0
  Date:           20160317
  Description:    ublox_gps头文件
  Others:

*****************************************************************************/

#ifndef __UBLOX_GPS_H__
#define __UBLOX_GPS_H__
/******************************************************************************
头文件声明
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
变量声明
******************************************************************************/
#define GPS_PRINT(level, fmt, ...) (printk(level"[*UBLOX-GPS*] %s:%d: "fmt"\n", \
        __func__, __LINE__, ##__VA_ARGS__))

#if ( FEATURE_ON == MBB_FACTORY )
#define GPIO_10_00              (80)
#define GPS_UPDATE              GPIO_10_00
#endif /* FEATURE_ON == MBB_FACTORY */
#define GPS_UBX_CHIP_POWER      (13) //VLD09_3V3

/*GPS IOCTRL事件*/
#define GPS_CMD_BASE                    (0X66)
/*ublox gps芯片上电*/
#define GPS_POWER_ON_CMD                _IOWR(GPS_CMD_BASE, 0, unsigned long)
/*ublox gps芯片下电*/
#define GPS_POWER_OFF_CMD               _IOWR(GPS_CMD_BASE, 1, unsigned long)
#if ( FEATURE_ON == MBB_FACTORY )
/*ublox gps芯片safeboot模式控制*/
#define GPS_UPDATE_GPIO_SET_CMD         _IOWR(GPS_CMD_BASE, 2, unsigned long)
#endif /* FEATURE_ON == MBB_FACTORY */

/******************************************************************************
函数声明
******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif


