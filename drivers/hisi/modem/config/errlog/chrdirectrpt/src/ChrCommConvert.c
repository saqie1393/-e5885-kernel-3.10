

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include  "ChrCommConvert.h"

/*
#define    THIS_FILE_ID            PS_FILE_ID_CHRCOMMCONVERT_C
*/

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 直通新接口定义，新增事件或XML结构改变时必须更新 */
VOS_UINT8  aucProductName[20] = {"COMMON#101"};
VOS_UINT8  ucLogVersion       = 6;


/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_VOID Modem_ErrLog_FillModemHeader(MODEM_ERR_LOG_HEADER_STRU *pstmodemHeader, VOS_UINT16 usSubEventID,
                                              VOS_UINT8 ucSubEventCause, VOS_BOOL enModemHidsLog)
{

    if (VOS_NULL_PTR == pstmodemHeader)
    {
        return;
    }
    memcpy_s(pstmodemHeader->aucProductName, 20, aucProductName, 20);

    pstmodemHeader->usSubEventID    = usSubEventID;
    pstmodemHeader->enModemHidsLog  = enModemHidsLog;
    pstmodemHeader->ucLogVersion    = ucLogVersion;
    pstmodemHeader->ucSubEventCause = ucSubEventCause;
}



VOS_INT32 Modem_ErrLog_ConvertStruct(VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize,
    const STRUCT_DESC *pStructDesc);


const STRUCT_DESC * GetStructDescByEventID(VOS_UINT16 usEventID)
{
    VOS_UINT32 i = 0;
    const STRUCT_DESC *pStructDesc = VOS_NULL_PTR;
    for (i = 0; i < g_FaultStructDescCount; ++i)
    {
        pStructDesc = (const STRUCT_DESC *) g_FaultStructDesc[i];
        if (VOS_NULL_PTR != pStructDesc && usEventID == pStructDesc->EventID)
        {
            return pStructDesc;
        }
    }

    return VOS_NULL_PTR;
}


const STRUCT_DESC * GetStructDescByStructID(VOS_UINT16 usStructID)
{
    VOS_UINT32 i = 0;
    const STRUCT_DESC *pStructDesc = VOS_NULL_PTR;
    for (i = 0; i < g_FaultStructDescCount; ++i)
    {
        pStructDesc = (const STRUCT_DESC *) g_FaultStructDesc[i];
        if (VOS_NULL_PTR != pStructDesc && usStructID == pStructDesc->StructID)
        {
            return pStructDesc;
        }
    }

    return VOS_NULL_PTR;
}


const STRUCT_DESC * GetStructDescByAbsoluteEventID(VOS_UINT16 usEventID)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    const STRUCT_DESC *pStructDesc = VOS_NULL_PTR;
    const FIELD_DESC *pField = VOS_NULL_PTR;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;
    for (i = 0; i < g_EventStructDescCount; ++i)
    {
        pStructDesc = (const STRUCT_DESC *) g_EventStructDesc[i];
        if (VOS_NULL_PTR != pStructDesc)
        {
            for (j = 0; j < pStructDesc->FieldCount; ++j)
            {
                pField = &pStructDesc->Fields[j];
                if (FIELD_TYPE_CLASS == pField->FieldType)
                {
                    pSubStructDesc = GetStructDescByStructID(pField->FieldLength);
                    if (VOS_NULL_PTR != pSubStructDesc && usEventID == pSubStructDesc->EventID)
                    {
                        return pStructDesc;
                    }
                }
            }
        }
    }

    return VOS_NULL_PTR;
}


void CopyValue(VOS_UINT8 *pDst, const VOS_UINT8 *pSrc, VOS_UINT32 ulLen)
{
    VOS_UINT32 i = 0;

    if (VOS_NULL_PTR != pDst && VOS_NULL_PTR != pSrc)
    {
        for (i = 0; i < ulLen; ++i)
        {
            pDst[i] = pSrc[ulLen - i - 1];
        }
    }
}


VOS_INT32 Modem_ErrLog_ConvertStructByDesc(VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
                                         VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize,
                                         const STRUCT_DESC *pSubStructDesc)
{
    VOS_INT32 ret = VOS_ERR;

    if (VOS_NULL_PTR == pSubStructDesc || 2 + ulStructSize > ulBufferSize)
    {
        return VOS_ERR;
    }

    CopyValue(pBuffer, (VOS_UINT8 *) &pSubStructDesc->StructID, 2);
    pBuffer += 2;
    ulBufferSize -= 2;

    ret = Modem_ErrLog_ConvertStruct(pBuffer, ulBufferSize, pBufferLen, pStruct, ulStructSize, pSubStructDesc);
    if (VOS_OK != ret || ulBufferSize < *pBufferLen)
    {
        return VOS_ERR;
    }

    *pBufferLen = *pBufferLen + 2;

    return VOS_OK;
}


VOS_INT32 Modem_ErrLog_ConvertStructByID(VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize,
    VOS_UINT16 usStructID)
{
    const STRUCT_DESC *pSubStructDesc = GetStructDescByStructID(usStructID);

    return Modem_ErrLog_ConvertStructByDesc(pBuffer, ulBufferSize, pBufferLen, pStruct, ulStructSize, pSubStructDesc);
}


VOS_INT32 ConvertField_Class(VOS_UINT8 *p, VOS_UINT32 ulSize, VOS_UINT32 *pLen, VOS_UINT32 *pStructLen,
                             const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize, const FIELD_DESC *pField)
{
    VOS_INT32 ret = VOS_ERR;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;

    if (VOS_NULL_PTR == p || VOS_NULL_PTR == pLen || VOS_NULL_PTR == pStructLen || VOS_NULL_PTR == pStruct || VOS_NULL_PTR == pField)
    {
        return VOS_ERR;
    }

    *pLen = 0;
    *pStructLen = 0;

    pSubStructDesc = GetStructDescByStructID(pField->FieldLength);

    if (VOS_NULL_PTR == pSubStructDesc)
    {
        return VOS_ERR;
    }

    ret = Modem_ErrLog_ConvertStructByDesc(p, ulSize, pLen, pStruct, ulStructSize, pSubStructDesc);
    if (VOS_OK != ret || ulSize < *pLen)
    {
        return VOS_ERR;
    }

    *pStructLen = pSubStructDesc->StructSize;
    return VOS_OK;
}


VOS_INT32 ConvertField_ClassArray(VOS_UINT8 *p, VOS_UINT32 ulSize, VOS_UINT32 *pLen, VOS_UINT32 *pStructLen,
                             const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize, const FIELD_DESC *pField)
{
    VOS_INT32 ret = VOS_ERR;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;
    VOS_UINT16 usArraySize = 0;
    VOS_UINT32 j = 0;
    VOS_UINT32 ulLen = 0;

    if (VOS_NULL_PTR == p || VOS_NULL_PTR == pLen || VOS_NULL_PTR == pStructLen || VOS_NULL_PTR == pStruct || VOS_NULL_PTR == pField || 2 + ulStructSize > ulSize)
    {
        return VOS_ERR;
    }

    *pLen = 0;
    *pStructLen = 0;

    /* 数组的FieldType高24位为数组元素个数 */
    usArraySize = pField->FieldType >> 8;

    CopyValue(p, (VOS_UINT8 *) &usArraySize, 2);
    p += 2;
    *pLen = *pLen + 2;

    pSubStructDesc = GetStructDescByStructID(pField->FieldLength);

    if (VOS_NULL_PTR == pSubStructDesc)
    {
        return VOS_ERR;
    }

    for (j = 0; j < usArraySize; ++j)
    {
        ret = Modem_ErrLog_ConvertStructByDesc(p, ulSize, &ulLen, pStruct, ulStructSize, pSubStructDesc);
        if (VOS_OK != ret || ulSize < ulLen)
        {
            return VOS_ERR;
        }

        p += ulLen;
        ulSize -= ulLen;
        pStruct += pSubStructDesc->StructSize;
        ulStructSize -= pSubStructDesc->StructSize;
        *pLen += ulLen;
    }

    *pStructLen = usArraySize * pSubStructDesc->StructSize;
    return VOS_OK;
}


VOS_INT32 ConvertField_Array(VOS_UINT8 *p, VOS_UINT32 ulSize, VOS_UINT32 *pLen, VOS_UINT32 *pStructLen,
                             const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize, const FIELD_DESC *pField)
{
    if (VOS_NULL_PTR == p || VOS_NULL_PTR == pLen || VOS_NULL_PTR == pStruct || VOS_NULL_PTR == pField)
    {
        return VOS_ERR;
    }

    *pLen = 0;
    *pStructLen = 0;

    /* 根据字段类型进行转换 */
    switch (pField->FieldType & FIELD_TYPE_MASK)
    {
    case FIELD_TYPE_VARCHAR:
    case FIELD_TYPE_ARRAY:
        {
            if (pField->FieldLength > ulSize || pField->FieldLength > ulStructSize)
            {
                return VOS_ERR;
            }

            *pLen = pField->FieldLength;
            memcpy_s(p, ulSize, pStruct, *pLen);

            *pStructLen = pField->FieldLength;
            break;
        }
    default:
        {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}


VOS_INT32 ConvertField_Value(VOS_UINT8 *p, VOS_UINT32 ulSize, VOS_UINT32 *pLen, VOS_UINT32 *pStructLen,
                             const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize, const FIELD_DESC *pField)
{
    VOS_UINT16 usArraySize = 0;
    VOS_UINT32 ulDataLen = 0;
    VOS_UINT32 i = 0;

    if (VOS_NULL_PTR == p || VOS_NULL_PTR == pLen || VOS_NULL_PTR == pStruct || VOS_NULL_PTR == pField)
    {
        return VOS_ERR;
    }

    *pLen = 0;
    *pStructLen = 0;

    /* 根据字段类型进行转换 */
    switch (pField->FieldType & FIELD_TYPE_MASK)
    {
        /* 枚举数组 */
    case FIELD_TYPE_ENUMARRAY:
        {
            /* 数组的FieldType高24位为数组元素个数 */
            usArraySize = pField->FieldType >> 8;

            /* 枚举数组的FieldLength为枚举的字节长度 */
            ulDataLen = (VOS_UINT32) pField->FieldLength * usArraySize;
            if (ulDataLen + 2 > ulSize || ulDataLen > ulStructSize)
            {
                return VOS_ERR;
            }

            CopyValue(p, (VOS_UINT8 *) &usArraySize, 2);
            p += 2;
            *pLen = *pLen + 2;

            for (i = 0; i < usArraySize; ++i)
            {
                CopyValue(p + i * pField->FieldLength, pStruct + i * pField->FieldLength, pField->FieldLength);
            }

            *pLen = *pLen + ulDataLen;

            *pStructLen = ulDataLen;
            break;
        }
        /* 有符号整形、无符号整形、字符串、字节数组、枚举 */
    case FIELD_TYPE_INT:
    case FIELD_TYPE_UINT:
    case FIELD_TYPE_ENUM:
        {
            if (pField->FieldLength > ulSize || pField->FieldLength > ulStructSize)
            {
                return VOS_ERR;
            }

            *pLen = pField->FieldLength;
            CopyValue(p, pStruct, *pLen);

            *pStructLen = pField->FieldLength;
            break;
        }
    default:
        {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}


VOS_INT32 Modem_ErrLog_ConvertStruct(VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize,
    const STRUCT_DESC *pStructDesc)
{
    VOS_UINT32 i = 0;
    const FIELD_DESC *pField = VOS_NULL_PTR;
    VOS_INT32 ret = VOS_OK;
    /* 写入缓冲区的临时指针 */
    VOS_UINT8 *p = pBuffer;
    /* 缓冲区剩余空间 */
    VOS_UINT32 ulSize = ulBufferSize;
    /* 缓冲区长度 */
    VOS_UINT32 ulLen = 0;
    /* 子结构体的大小 */
    VOS_UINT32 ulStructLen = 0;

    if (VOS_NULL_PTR == pBuffer || VOS_NULL_PTR == pStruct || NULL == pBufferLen || VOS_NULL_PTR == pStructDesc || ulBufferSize < ulStructSize)
    {
        return VOS_ERR;
    }

    /* bParamInvalid已经进行了空指针判断，下面的PCLINT可以屏蔽 */
    /*lint -save -e613*/
    /* 初始化返回的字节流长度 */
    *pBufferLen = 0;

    for (i = 0; i < pStructDesc->FieldCount; ++i)
    {
        pField = &pStructDesc->Fields[i];

        /* 根据字段类型进行转换 */
        switch (pField->FieldType & FIELD_TYPE_MASK)
        {
        /* 子结构体 */
        case FIELD_TYPE_CLASS:
            {
                ret = ConvertField_Class(p, ulSize, &ulLen, &ulStructLen, pStruct, ulStructSize, pField);
                break;
            }
        /* 子结构体数组 */
        case FIELD_TYPE_CLASSARRAY:
            {
                ret = ConvertField_ClassArray(p, ulSize, &ulLen, &ulStructLen, pStruct, ulStructSize, pField);
                break;
            }
        /* 枚举数组 */
        case FIELD_TYPE_ENUMARRAY:
        /* 有符号整形、无符号整形、字符串、字节数组、枚举 */
        case FIELD_TYPE_INT:
        case FIELD_TYPE_UINT:
        case FIELD_TYPE_ENUM:
            {
                ret = ConvertField_Value(p, ulSize, &ulLen, &ulStructLen, pStruct, ulStructSize, pField);
                break;
            }
        case FIELD_TYPE_VARCHAR:
        case FIELD_TYPE_ARRAY:
            {
                ret = ConvertField_Array(p, ulSize, &ulLen, &ulStructLen, pStruct, ulStructSize, pField);
                break;
            }
        /* 日期时间，不支持  */
        case FIELD_TYPE_DATETIME:
        default:
            {
                return VOS_ERR;
            }
        }

        if (VOS_OK != ret)
        {
            return ret;
        }

        p += ulLen;
        ulSize -= ulLen;
        pStruct += ulStructLen;
        ulStructSize -= ulStructLen;
    }

    *pBufferLen = (VOS_UINT32) (p - pBuffer);
    /*lint -restore*/
    return VOS_OK;
}


VOS_INT32 Modem_ErrLog_ConvertEvent(VOS_UINT16 usEventID, VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize)
{
    const STRUCT_DESC *pStructDesc = GetStructDescByEventID(usEventID);

    if (VOS_NULL_PTR == pBuffer || VOS_NULL_PTR == pStruct || NULL == pBufferLen || ulBufferSize < ulStructSize + 2 || VOS_NULL_PTR == pStructDesc)
    {
        return VOS_ERR;
    }

    /* 子事件填充子结构体ID, 事件不填ID */
    CopyValue(pBuffer, (VOS_UINT8 *) &pStructDesc->StructID, 2);
    pBuffer += 2;
    ulBufferSize -= 2;

    *pBufferLen = 0;
    if (VOS_OK == Modem_ErrLog_ConvertStruct(pBuffer, ulBufferSize, pBufferLen, pStruct, ulStructSize, pStructDesc))
    {
        *pBufferLen = *pBufferLen + 2;
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_INT32 CalcStructSize(const STRUCT_DESC *pStructDesc)
{
    VOS_UINT32 i = 0;
    const FIELD_DESC *pField = VOS_NULL_PTR;
    VOS_INT32 lRet = 0;
    VOS_INT32 lSize = 0;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;
    VOS_UINT16 usArraySize = 0;

    if (VOS_NULL_PTR == pStructDesc)
    {
        return 0;
    }

    for (i = 0; i < pStructDesc->FieldCount; ++i)
    {
        pField = &pStructDesc->Fields[i];

        /* 根据字段类型进行转换 */
        switch (pField->FieldType & FIELD_TYPE_MASK)
        {
        /* 子结构体 */
        case FIELD_TYPE_CLASS:
            {
                pSubStructDesc = GetStructDescByStructID(pField->FieldLength);
                lRet = CalcStructSize(pSubStructDesc);
                if (0 != lRet)
                {
                    lRet += 2;
                }
                break;
            }
        /* 子结构体数组 */
        case FIELD_TYPE_CLASSARRAY:
            {
                pSubStructDesc = GetStructDescByStructID(pField->FieldLength);
                usArraySize = pField->FieldType >> 8;
                lRet = CalcStructSize(pSubStructDesc);
                if (0 != lRet)
                {
                    lRet = 2 + (lRet + 2) * usArraySize;
                }
                break;
            }
        /* 枚举数组 */
        case FIELD_TYPE_ENUMARRAY:
        /* 有符号整形、无符号整形、字符串、字节数组、枚举 */
            {
                lRet = 2 + pField->FieldLength * (pField->FieldType >> 8);
                break;
            }
        case FIELD_TYPE_INT:
        case FIELD_TYPE_UINT:
        case FIELD_TYPE_ENUM:
        case FIELD_TYPE_VARCHAR:
        case FIELD_TYPE_ARRAY:
            {
                lRet = pField->FieldLength;
                break;
            }
        /* 日期时间，不支持  */
        case FIELD_TYPE_DATETIME:
        default:
            {
                return 0;
            }
        }

        if (0 == lRet)
        {
            return 0;
        }

        lSize += lRet;
    }

    return lSize;
}


VOS_INT32 Modem_ErrLog_CalcEventSize(VOS_UINT16 usEventID)
{
    VOS_INT32 lSize = 2;
    const STRUCT_DESC *pStructDesc = GetStructDescByEventID(usEventID);
    VOS_INT32 lRet = 0;

    if (NULL == pStructDesc)
    {
        return 0;
    }

    lRet = CalcStructSize(pStructDesc);

    if (0 >= lRet)
    {
        return 0;
    }

    lSize += lRet;

    return lSize;
}


VOS_INT32 Modem_ErrLog_ConvertAbsoluteEvent(VOS_UINT16 usEventID, VOS_UINT8 *pBuffer, VOS_UINT32 ulBufferSize,
    VOS_UINT32 *pBufferLen, const VOS_UINT8 *pStruct, VOS_UINT32 ulStructSize)
{
    VOS_UINT32 j = 0;
    const FIELD_DESC *pField = VOS_NULL_PTR;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;
    const STRUCT_DESC *pStructDesc = GetStructDescByAbsoluteEventID(usEventID);
    VOS_UINT32 len = 0;

    if (VOS_NULL_PTR == pBuffer || VOS_NULL_PTR == pStruct || NULL == pBufferLen || ulBufferSize < ulStructSize + 2 || VOS_NULL_PTR == pStructDesc)
    {
        return VOS_ERR;
    }

    *pBufferLen = 0;
    for (j = 0; j < pStructDesc->FieldCount; ++j)
    {
        pField = &pStructDesc->Fields[j];
        if (FIELD_TYPE_CLASS != pField->FieldType)
        {
            return VOS_ERR;
        }

        pSubStructDesc = GetStructDescByStructID(pField->FieldLength);
        if (VOS_NULL_PTR == pSubStructDesc)
        {
            return VOS_ERR;
        }

        if (usEventID == pSubStructDesc->EventID)
        {
            CopyValue(pBuffer, (VOS_UINT8 *) &pSubStructDesc->StructID, 2);
            pBuffer += 2;
            ulBufferSize -= 2;
            len += 2;
            if (VOS_OK != Modem_ErrLog_ConvertStruct(pBuffer, ulBufferSize, pBufferLen, pStruct, ulStructSize, pSubStructDesc))
            {
                return VOS_ERR;
            }
            pBuffer += *pBufferLen;
            ulBufferSize -= *pBufferLen;
        }
        else
        {
            *((VOS_UINT16 *) pBuffer) = 0xFFFFu;
            pBuffer += 2;
            ulBufferSize -= 2;
            len += 2;
        }
    }
    *pBufferLen = *pBufferLen + len;
    return VOS_OK;
}


VOS_INT32 Modem_ErrLog_CalcAbsoluteEventSize(VOS_UINT16 usEventID)
{
    VOS_UINT32 j = 0;
    const FIELD_DESC *pField = VOS_NULL_PTR;
    const STRUCT_DESC *pSubStructDesc = VOS_NULL_PTR;
    const STRUCT_DESC *pStructDesc = GetStructDescByAbsoluteEventID(usEventID);
    VOS_INT32 lSize = 0;
    VOS_INT32 lRet = 0;

    if (VOS_NULL_PTR == pStructDesc)
    {
        return 0;
    }

    for (j = 0; j < pStructDesc->FieldCount; ++j)
    {
        pField = &pStructDesc->Fields[j];
        if (FIELD_TYPE_CLASS != pField->FieldType)
        {
            return 0;
        }

        pSubStructDesc = GetStructDescByStructID(pField->FieldLength);
        if (VOS_NULL_PTR == pSubStructDesc)
        {
            return 0;
        }

        if (usEventID == pSubStructDesc->EventID)
        {
            lRet = CalcStructSize(pSubStructDesc);

            if (0 >= lRet)
            {
                return 0;
            }

            lSize += 2 + lRet;
        }
        else
        {
            lSize += 2;
        }
    }
    return lSize;
}




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


