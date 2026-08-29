

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#include  "product_config.h"
#include  "vos.h"
#include  "omerrorlog.h"

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif


/* xml的数据类型 */
enum FIELD_TYPE_ENUM
{
    FIELD_TYPE_INT = 1,     /* 有符号整形，包括char, short, int, long */
    FIELD_TYPE_UINT,    /* 无符号整形，包括unsigned char, unsigned short, unsinged int, unsigned long */
    FIELD_TYPE_VARCHAR, /* 字符串，char[] */
    FIELD_TYPE_ENUM,    /* 枚举 */
    FIELD_TYPE_ARRAY,   /* 字节数组 byte[] */
    FIELD_TYPE_DATETIME,    /* 日期时间 */
    FIELD_TYPE_CLASS,       /* 结构体 */
    FIELD_TYPE_CLASSARRAY,  /* 结构体数组 */
    FIELD_TYPE_ENUMARRAY,   /* 枚举数组 */
    FIELD_TYPE_MASK = 0xFF,
};

/* 字段描述 */
typedef struct
{
    VOS_UINT16          FieldType;
    VOS_UINT16          FieldLength;
} FIELD_DESC;

/* 结构体描述信息结构体 */
typedef struct
{
    VOS_UINT16          EventID;
    VOS_UINT16          StructID;
    VOS_UINT16          StructSize;
    VOS_UINT16          FieldCount;
    FIELD_DESC          Fields[1];
} STRUCT_DESC;

/* 结构体描述信息 */
extern const VOS_VOID * const g_FaultStructDesc[];
/* 结构体个数 */
extern const VOS_UINT32 g_FaultStructDescCount;

extern const VOS_VOID * const g_EventStructDesc[];
extern const VOS_UINT32 g_EventStructDescCount;

/* 直通新接口定义，新增事件或XML结构改变时必须更新 */
extern   VOS_UINT8  aucProductName[20];
extern   VOS_UINT8  ucLogVersion;


extern VOS_VOID Modem_ErrLog_FillModemHeader(MODEM_ERR_LOG_HEADER_STRU *pstmodemHeader, VOS_UINT16 usSubEventID,
                                              VOS_UINT8 ucSubEventCause, VOS_BOOL enModemHidsLog);



extern VOS_INT32 Modem_ErrLog_ConvertEvent(VOS_UINT16 usEventID, VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize);


extern VOS_INT32 Modem_ErrLog_CalcEventSize(VOS_UINT16 usEventID);


extern VOS_INT32 Modem_ErrLog_ConvertAbsoluteEvent(VOS_UINT16 usEventID, VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize);


extern VOS_INT32 Modem_ErrLog_CalcAbsoluteEventSize(VOS_UINT16 usEventID);

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

