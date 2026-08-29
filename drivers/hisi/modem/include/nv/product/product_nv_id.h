

#ifndef __PRODUCT_NV_ID_H__
#define __PRODUCT_NV_ID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <product_config.h>
/*
 *  NV ID 的添加按从小到大排列
 */

typedef enum
{
    NV_ID_CRC_CHECK_RESULT  = 0xc350,
    NV_ID_GUC_CHECK_ITEM    = 0xc351,
    NV_ID_TL_CHECK_ITEM     = 0xc352,
    NV_ID_GUC_M2_CHECK_ITEM = 0xc353,
    
    NV_ID_PRODUCT_START = 0xc360,
    NV_ID_INI_VERSION = 0xc367,        /*50023 ini 配置文件版本号*/
    NV_ID_MTU_Value       = 0xC373,    /*50035*/
    NV_ID_FLIGHT_MODE	  = 0xC37B,    /*50043*/
    NV_HUAWEI_GPS_ENABLE              = 0xC394,    /*50068*/     
    NV_Item_GPS_USER_SETTING          = 0xC399,    /*50073*/     
    NV_ID_GID1_LOCK                   = 0xC3A3,    /*50083*/
    NV_ID_DYNAMIC_NAME                = 0xc3bc,     /*50108*/
    NV_ID_DYNAMIC_INFO_NAME           = 0xc3bd,     /*50109*/
    NV_ID_SPECIAL_PLMN_NOT_ROAM       = 0xC3B7,    /*50103*/
    NV_Item_HUAWEI_PLMN_MODIFY        = 0xC3C3,    /*50115*/
    NV_ID_CSIM_CUSTOMIZATION          = 0xC40D,    /*50189*/
    NV_ID_HUAWEI_SYSCFGEX_MODE_LIST   = 0xC416,    /*50198*/
    en_NV_Item_THERMAL_CONFIG         = 0xC44F,    /*50255*/
    NV_Item_GPS_SETTING               = 0xC468,    /*50280*/
    en_NV_Item_WAKEUP_CFG_FLAG        = 0xC47B,    /* 50299  */
#if (FEATURE_ON == MBB_FEATURE_SKU)
    NV_ITEM_SKU_TYPE                  = 0xC4A0,    /*50336*/
#endif
#if (FEATURE_ON == MBB_FEATURE_ACCOUNT)
    en_NV_Item_ACCOUNT_Type              = 0xC4A1,    /*50337*/
#endif
    /*电池电压校准NV*/
    NV_BATT_VOLT_CALI_I = 0xC4A8,    /*50344*/
    NV_CALL_SPECIAL_NUM_I             = 0xc4b8,       /* 50360  */
    NV_TEST_POWERUP_MODE_CONTROL_FLAG = 0xC4BC,   /*50364*/
    NV_ID_HUAWEI_ANTENNA_TUNER_GSM    = 0xC4C9,    /* 50377 */
    NV_ID_HUAWEI_ANTENNA_TUNER_WCDMA  = 0xC4CA,    /* 50378 */
    NV_ID_HUAWEI_ANTENNA_TUNER_CDMA   = 0xC4CB,    /* 50379 */
    NV_ID_HUAWEI_ANTENNA_TUNER_LTE    = 0xC4CC,    /* 50380 */
    NV_CHG_SHUTOFF_TEMP_PROTECT_I     = 0xC4D1,   /*50385*/
    NV_CHG_SHUTOFF_VOLT_PROTECT_I     = 0xC4D2,   /*50386*/
    NV_HUAWEI_DYNAMIC_VID             = 0xC4DA,   /*50394*/
    NV_HUAWEI_DYNAMIC_BOOT_PID        = 0XC4DB,   /*50395*/
    NV_HUAWEI_DYNAMIC_NORMAL_PID      = 0XC4DC,   /*50396*/
    NV_HUAWEI_DYNAMIC_DEBUG_PID       = 0XC4DD,   /*50397*/
    NV_FOTA_PPP_APN = 0xC4E0,     /* 50400 */
    NV_FOTA_PPP_PASSWORD = 0xC4E1,   /* 50401 */
    NV_FOTA_PPP_USER_ID = 0xC4E2,    /* 50402 */
    NV_FOTA_PPP_AUTHTYPE = 0xC4E3,   /* 50403 */
    NV_FOTA_SERVER_IP = 0xC4E4,      /* 50404 */
    NV_FOTA_SERVER_PORT = 0xC4E5,    /* 50405 */
    NV_FOTA_DETECT_MODE = 0xC4E6,    /* 50406 */
    NV_FOTA_DETECT_COUNT = 0xC4E7,   /* 50407 */
    NV_FOTA_NWTIME = 0xC4E8,      /* 50408 */
    NV_FOTA_TIMER = 0xC4E9,       /* 50409 */
    NV_HUAWEI_MULTI_IMAGE_I  = 0xC4EC,  /*50412*/
    NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I  = 0xC4ED,  /*50413*/
    NV_ID_CUSTOMIZED_BAND_GROUP       = 0xC4FB,   /*50427*/
    NV_ID_SOFT_RELIABLE_CFG  = 0xC50A,
    NV_ID_HPLMN_FIRST_UMTS_TO_LTE     = 0xC515,   /*50453*/
    NV_ID_EE_OPERATOR_NAME_DISPLAY    = 0xC516,   /*50454*/
    NV_ID_TTS_CFG_I                   = 0xC521,   /*50465*/
#if(FEATURE_ON == MBB_WPG_MODULE_AT_HSMF)
    NV_ID_HUAWEI_WEBSDK_SMS_FULL_I    = 0xC522,   /* 50466 */
#endif
    NV_ID_WEB_SITE                    = 0xCB84,
    NV_ID_WPS_PIN                     = 0xCB8D,
    NV_ID_WEB_USER_NAME               = 0xCB9C,
    NV_ID_WEB_ADMIN_PASSWORD_NEW_I = 0xC4F2,   /*50418*/
    NV_ID_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_EX = 0xC4F6,/* 50422 */
    NV_ID_DSFLOW_REPORT_MUTI_WAN = 0xC4EA,/*50410*/
    NV_ID_WINBLUE_PROFILE             = 0xC4F8, /* 50424 */
    NV_ITEM_RPLMN_ACT_DISABLE_CFG = 0xC4F9,  /*50425*/
    NV_ID_PIH_IMSI_TIMER              = 0xC4FA,   /*50426*/
    NV_START_MANUAL_TO_AUTO = 0xC4FC, /*50428 */
    NV_ID_VODAFONE_CPBS               = 0xC4FD,   /*50429*/
#if (FEATURE_ON == MBB_THERMAL_PROTECT)
    NV_LIMIT_SPEED_SECOND_TEMP_PROTECT = 0xC4FE,    /*50430*/
#endif
    en_NV_DEVNUMW_TIME_I             = 0xC4FF ,   /*50431*/
    NV_ID_TATA_DEVICE_LOCK            = 0xC500,   /*50432*/
    NV_Item_HPLMNWithinEPLMNNotRoam = 0XC501,/*50433 */
    NV_Item_APN_LOCK = 0xC508,/*50440*/
    NV_ID_ATCMD_AMPW_STORE = 0xC509, /* 50441*/
    NV_FOTA_DETECT_INTERVAL = 0xC50F, /* 50447 */
    NV_ID_SMS_AUTO_REG                = 0xC511,   /*50449*/
    NV_ID_TME_CPIN                    = 0xC512,   /*50450*/
    NV_ID_LED_PARA                  = 0xC517,/* 50455  */
    NV_ID_USB_CDC_NET_SPEED           = 0xC518, /* 50456 */
    NV_ID_USB_MULTI_CONFIG_PORT           = 0xC519, /* 50457 */
    NV_M2M_LED_CONTROL_EX               = 0xC51D,   /* 50461 */
    NV_HUAWEI_COUL_INFO_I             = 0xC51E,  /*50462*/
    NV_ID_HUAWEI_ANTENNA_TUNER_TDSCDMA = 0xC520, /* 50464 */
    NV_ID_PLATFORM_CATEGORY_SET     = 0xC51A,   /* 50458 */
    en_NV_Item_SLEEP_CFG_FLAG       = 0xC523,   /* 50467 */
    NV_ID_AT_GET_CHANWIINFO = 0xC524,/*50468*/
    NV_ID_GET_PWRWIINFO = 0xC525,/*50469*/
    NV_Item_FOTA_SMS_CONFIG = 0xC526,    /* 50470 */
    NV_Item_FOTA_TIME_STAMP = 0xC527,  /* 50471 */
    NV_Item_FOTA_RSA_PUB_MOD = 0xC528,  /* 50472 */ 
    NV_Item_FOTA_RSA_PUB_EXP = 0xC529,	/* 50473 */ 
    NV_FOTA_SMS_FLAG = 0xC531,            /* 50481 */
#if (FEATURE_ON == MBB_WPG_CELLROAM)
    Nas_Mmc_Nvim_Roam_Mcc_Cmp        = 0xC533, /*50483*/
#endif /*FEATURE_ON == MBB_WPG_CELLROAM*/
    NV_HUAWEI_PWRCFG_SWITCH = 0xC53A,    /* 50490 */
    NV_HUAWEI_PWRCFG_GSM = 0xC53B,    /* 50491 */
    NV_HUAWEI_PWRCFG_WCDMA = 0xC53C,  /* 50492 */
    NV_HUAWEI_PWRCFG_LTE = 0xC53E,  /* 50494 */
    NV_HUAWEI_PWRCFG_TDSCDMA = 0xC53F,  /* 50495 */
    NV_ID_HUAWEI_CUST_NVID_NV = 0xC540, /* 50496 */
    NV_HUAWEI_MLOG_SWITCH    = 0xC541, /*50497*/
    NV_USB_PRIVATE_INFO    = 0xC542, /*50498*/
    NV_TELNET_SWITCH_I             = 0xC545,  /*50501*/
    NV_HUAWEI_OEMLOCK_I   = 0xC546 ,  /*50502*/
    NV_HUAWEI_SIMLOCK_I   = 0xC547 ,  /*50503*/
    NV_ID_ATCMD_AMPW_ADD_IPR_STORE = 0xc549,  /*50505*/
    en_NV_Item_THERMAL_PROTECTION = 0xC54A,  /*50506*/
    NV_HUAWEI_EMERGENCY_CFG_TYPE = 0xC54C,  /*50508*/
    NV_ID_SIM_CUSM_FEATURE = 0xC54E,/*50510*/
    NV_HUAWEI_ROAM_WHITELIST_I         = 0xC550, /*50512*/
    NV_HUAWEI_ROAM_WHITELIST_EXTEND1_I = 0xC551, /*50513*/
    NV_DRV_MAC_NUM                  = 0xC555, /*50517*/
    NV_TEMP_PROTECT_SAR_REDUCTION      = 0xC558 ,  /*50520*/
    NV_HUAWEI_SHELL_CMD             = 0xC55B  ,/*50523*/
    NV_ID_USB_TETHERING = 0xC55C,/*50524*/
    en_NV_Item_AUTHFALLBACK_FEATURE = 50525,  /* 50525 */
    en_NV_Item_CUSTOMAUTH_FEATURE = 50526,    /* 50526 */
    en_NV_HUAWEI_SIM_SWITCH = 50529,  /*50529*/
    NV_HUAWEI_COULCUREENT_INFO     = 0xC568,  /*50536*/
    NV_ID_USB_DEBUG_MODE_FLAG           = 0xC56A, /* 50538 */
    NV_ID_HUAWEI_CUST_NVID_RESTORE = 0xC56C, /* 50540 */
    NV_ID_BODYSAR_CFG                =0xC56D,
    NV_ID_BODYSAR_L_PARA         =0xC56E,
    en_NV_FOTA_SMS_CFG_FLAG         = 0xc56f, /* 50543 */      
    NV_HUAWEI_S3S4_REMOTE_WAKEUP_ENABLE = 0xC571,
    NV_ID_HUAWEI_MBB_FEATURE         = 0xC572, /* 50546 */
    NV_TRUST_CALLER_NUM        = 0xC576, /* 50550 */
    NV_TRUST_BLACK_NUM        = 0xC57C, /* 50556 */
    NV_ID_CE_LED_CONFIG         = 0xC57E, /* 50558 */    
    NV_ID_HUAWEI_FEATURE_CTRL         = 0xC57F, /* 50559 */
    NV_IP_FILTER_CONFIG         = 0xC580, /* 50560 */

    NV_ID_SBM_APN_FLAG                      = 0xC57A, /*50554*/
    NV_ID_HUAWEI_AGING_TEST          = 0xC581, /* 50561 */
    NV_SB_SERIAL_NUM                     = 0xC582,  /*50562*/
    NV_ID_RF_ANTEN_DETECT                = 0xC589,  /*50569*/
    NV_ID_LTE_ANT_INFO                   = 0xC58E,  /*50574*/
    NV_ID_USB_SECURITY                   = 0xC591,/*50577*/

    NV_ID_LTE_ATTACH_PROFILE_0    = 0xC592,   /*50578*/
    NV_ID_LTE_ATTACH_PROFILE_1    = 0xC593,   /*50579*/
    NV_ID_LTE_ATTACH_PROFILE_2    = 0xC594,   /*50580*/
    NV_ID_LTE_ATTACH_PROFILE_3    = 0xC595,   /*50581*/
    NV_ID_LTE_ATTACH_PROFILE_4    = 0xC596,   /*50582*/
    NV_ID_LTE_ATTACH_PROFILE_5    = 0xC597,   /*50583*/
    NV_ID_LTE_ATTACH_PROFILE_CONTROL = 0xC59F,      /*50591*/

    NV_ID_UBX_XO_VOLTAGE          = 0xC5A0,   /*50592*/

    NV_ID_Item_CMIC_Volume        = 0xC5A1,   /*50593*/

    NV_ID_UNDEF_IDENTIFY          = 0xC5A7,   /*50599*/
    NV_ID_ENDEF_IDENTIFY          = 0xC5A8,   /*50600*/
    NV_ID_WIFI_2G_RFCAL           = 0xC5A9,   /*50601*/
    NV_ID_WIFI_5G_RFCAL           = 0xC5AA,   /*50602*/
    NV_ID_SEC_BOOT_SUPPORT_TYPE   = 0xC5B0,   /*50608*/
    NV_MULTIPINS_CONFIG           = 0xc5b2,   /*50610*/
    NV_ID_PLMN_BANDLOCK    = 0xC5B5,       /*50613*/
    NV_CARRIER_ADAPTER_CONFIG     = 0xc5cb,   /*50635*/
    NV_ID_PRODUCT_END      = 0xcb1f

}NV_ID_PRODUCT_ENUM;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


