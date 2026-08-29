
#ifndef __MDRV_CCORE_WDTIME_H__
#define __MDRV_CCORE_WDTIME_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "mdrv_public.h"

/* APP 投票节点 */
#define SMEM_VOTE_MODEM_WATCHDOG    "SMEM_VOTE_MODEM_WATCHDOG"
#define IPL_WDTIMER_FLAG_MAX_LEN    1

/* 定义GPIO电平高低 */
#define IPL_WDT_GPIO_VAL_LOW        0
#define IPL_WDT_GPIO_VAL_HIGH       1

/* A核睡眠标志位 */
#define ENTER_FREEZE_PROCESSES      0x45465052
#define EXIT_FREEZE_PROCESSES       0x52504645

/* APP投票标志位 */
#define IPL_APPCORE_PROCESS_NOT_ALIVE    0x98761345
#define IPL_APPCORE_PROCESS_ALIVE        0x45137698

#define  WDTIME_OPRT_ERROR    (-1)

/*查询和设置WDTIME。*/
typedef enum WDTIME_OPRT_ENUM
{
    WDTIME_OPRT_SET = 0,
    WDTIME_OPRT_GET,
    WDTIME_OPRT_BUTT
}WDTIME_OPRT_E;

/*****************************************************************************
函 数 名 : stop_wdt_atDload
功能描述 : 一键升级时停止喂狗
输入参数 : 无
返 回 值 : ret
调用函数 : sys_newstat
修改内容 : 新生成函数
*****************************************************************************/
int stop_wdt_atDload(void);

/*****************************************************************************
函 数 名 : kick_wdt_acore_wuc
功能描述 : minidump时继续喂狗
输入参数 : 无
返 回 值 : ret
调用函数 :
修改内容 : 新生成函数
*****************************************************************************/
void kick_wdt_acore_wuc(void);

/*****************************************************************************
 函 数 名  : drv_wdtime_oprt
 功能描述  : 设置和查询WDTIME的电平。
 输入参数  : ulOp : 0:设置/1:查询; wdtime时间
 输出参数  : 无
 返 回 值  : 0  操作成功
            -1  操作失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/

int drv_wdtime_oprt(WDTIME_OPRT_E ulOp, unsigned int *wdtime);

#ifdef __cplusplus
}
#endif

#endif

