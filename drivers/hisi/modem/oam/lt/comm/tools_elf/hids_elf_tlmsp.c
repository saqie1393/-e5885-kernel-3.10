


#include "msp_diag_comm.h"
#include "lte_sleepflow.h"
#include "diag_msgbbp.h"
#include "diag_msgbsp.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_debug.h"
#include "diag_msgapplog.h"

typedef enum _PRINTLOG_SWITCH_CFG
{
    PRINT_SWITCHOFF,
    PRINT_SWITCHON
}DRV_PRINT_SWITCH_CFG;

typedef enum _PRINTLOG_LEVEL_CFG
{
    PRINT_LEVEL_DEBUG,
    PRINT_LEVEL_INFO,
    PRINT_LEVEL_NOTICE,
    PRINT_LEVEL_WARNING,
    PRINT_LEVEL_ERROR,
    PRINT_LEVEL_CRIT,
    PRINT_LEVEL_ALERT,
    PRINT_LEVEL_FATAL
}DRV_PRINT_LEVEL_CFG;

typedef struct
{
    DRV_PRINT_SWITCH_CFG    enable;
    DRV_PRINT_LEVEL_CFG     level;
    unsigned char           resv[8];
}DRV_PRINT_CFG_REQ;

typedef struct
{
    unsigned int temperater;
    unsigned int time;
}trans_report_stru;

void msp_main(void)
{
// 消息结构声明开始
    MSP_SLEEP_MNTN_MSG_STRU stSleepMsg;
    
    DIAG_MSG_MSP_CONN_STRU  stMspConn;
    
    DIAG_MSG_A_TRANS_C_STRU stMsgATransC;
    
    DIAG_BSP_MSG_A_TRANS_C_STRU  stBspMsgTrans;
    
    DIAG_BBP_MSG_A_TRANS_C_STRU  stBbpMsgTrans;
    
    DIAG_PHY_MSG_A_TRANS_C_STRU  stPhyMsgTrans;
    
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
    DIAG_MSG_SOCP_VOTE_REQ_STRU  stMsgSocpVote;
    
    DIAG_MSG_SOCP_VOTE_WAKE_CNF_STRU stMsgSocpVoteCnf;
#endif
    
    DIAG_A_DEBUG_C_REQ_STRU      stADebugCReq;
    
    DIAG_BSP_NV_AUTH_STRU        stBspNvAuth;

    DIAG_MNTN_INFO_STRU          stMntnInfo;
    
    DIAG_LOST_INFO_STRU          stLostInfo;
	DIAG_APPLOG_CFG_REQ          stAppLog;
	DRV_PRINT_CFG_REQ            stPrintLog;
	trans_report_stru            stTransStu;
// 消息结构声明结束
}

