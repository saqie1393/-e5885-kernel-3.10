/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef _NV_XML_DEC_H_
#define _NV_XML_DEC_H_

/*****************************************************************************
1 其他头文件包含
*****************************************************************************/
#include "v_typdef.h"
#include "CodecNvInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif
#pragma pack(push)
#pragma pack(4) //4字节对齐

/*****************************************************************************
2 宏定义
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
#define AUDIO_XML_FILE_READ_BUFF_SIZE              (4096)  /* 读文件缓冲区大小 */
#define AUDIO_XML_NODE_LABEL_BUFF_LENGTH_ORIGINAL  (128)   /* 节点标签缓冲区大小*/
//#define  AUDIO_XML_NODE_NV_ITEM_LENGTH_ORIGINAL     (128)   /* 节点标签缓冲区大小*/
#define AUDIO_XML_PARAM_VALUE_BUFF_LENGTH_ORIGINAL (2048)  /* 节点值缓冲区大小*/ 

#define AUDIO_CTRL_SAMPLE_RATE_NUM                 (2)     /* 支持的采样率个数 8k和16k */
#define AUDIO_XML_NODE_PROPERTY_NUM                (4)     /*NV属性的个数*/
#define AUDIO_CTRL_MODE_SUB_ID_NUM                 (4)     /* 同一个模式下的ID个数 */
#define AUDIO_DELTA_XML_NV_VALUE_NUM               (150)   /*每个音效NV对应的值个数*/
#define AUDIO_DELTA_XML_NV_VALUE_LENGTH            (10)    /*每个NV值对应的字符串长度*/

#define AUDIO_DELTA_XML_HEAD_TAIL_LABEL_NUM        (2)     /* XML文件首部和尾部标签个数 */

/*****************************************************************************
3 枚举定义
*****************************************************************************/

/*****************************************************************************
枚举名    : XML_RESULT_ENUM_UINT32
协议表格  :
ASN.1描述 :
枚举说明  : XML函数的运行状态返回值
*****************************************************************************/
enum AUDIO_XML_RESULT_ENUM
{
    AUDIO_XML_RESULT_SUCCEED                        = 0 , /* 成功                      */
    AUDIO_XML_RESULT_SUCCEED_IGNORE_CHAR                , /* 可以忽略的字符\r\n\t      */
    AUDIO_XML_RESULT_FALIED_PARA_POINTER                , /* 错误的参数指针            */
    AUDIO_XML_RESULT_FALIED_MALLOC                      , /* 内存申请失败*/
    AUDIO_XML_RESULT_FALIED_BAD_SYNTAX                  , /* 错误的XML语法*/
    AUDIO_XML_RESULT_FALIED_BAD_CHAR                    , /* 错误的字符*/
    AUDIO_XML_RESULT_FALIED_READ_FILE                   , /* 读取文件失败 */
    AUDIO_XML_RESULT_FALIED_OUT_OF_BUFF_LEN             , /* 超出缓冲区长度*/
    AUDIO_XML_RESULT_FALIED_OUT_OF_0_9                  , /* NV ID值不在0-9  */
    AUDIO_XML_RESULT_FALIED_OUT_OF_0_F                  , /* NV Value值不在0-F */
    AUDIO_XML_RESULT_FALIED_OUT_OF_2_CHAR               , /* NV Value值超过1Byte  */
    AUDIO_XML_RESULT_FALIED_NV_ID_IS_NULL               , /* NV ID值为空*/   
    AUDIO_XML_RESULT_FALIED_NV_ID_IS_ERROR              , /* NV ID值为错的*/
    AUDIO_XML_RESULT_FALIED_NV_VALUE_IS_NULL            , /* NV Value值为空*/
    AUDIO_XML_RESULT_FALIED_NODE_MATCH                  , /*不是NV NODE*/
    AUDIO_XML_RESULT_GET_NV_DATA_FAIL                   , /*获取NV值失败*/
    AUDIO_XML_RESULT_WRITE_NV_DATA_FAIL                 , /*写入NV值失败*/
    AUDIO_XML_RESULT_BUTT
};
typedef VOS_UINT32 AUDIO_XML_RESULT_ENUM_UINT32;
 
/*****************************************************************************
枚举名    : XML_RESULT_ENUM_UINT32
协议表格  :
ASN.1描述 :
枚举说明  : XML函数的状态
*****************************************************************************/
enum AUDIO_XML_ANALYSE_STATUS_ENUM
{
    AUDIO_XML_ANASTT_ORIGINAL                       = 0 , /* 初始                 */
    AUDIO_XML_ANASTT_ENTER_LABLE                        , /* 进入标签             */
    AUDIO_XML_ANASTT_IGNORE                             , /* 序言或注释           */
    AUDIO_XML_ANASTT_NODE_LABLE                         , /* 解析标签名           */
    AUDIO_XML_ANASTT_SINGLE_ENDS_LABLE                  , /* 独立结尾标签         */
    AUDIO_XML_ANASTT_LABLE_END_MUST_RIGHT               , /* 标签开始即收尾       */
    AUDIO_XML_ANASTT_PROPERTY_START                     , /* 开始解析属性         */
    AUDIO_XML_ANASTT_PROPERTY_NAME_START                , /* 开始解析属性名称     */
    AUDIO_XML_ANASTT_PROPERTY_NAME_END                  , /* 属性名称结束，等待"  */
    AUDIO_XML_ANASTT_PROPERTY_VALUE_START               , /* "结束，等待="        */
    AUDIO_XML_ANASTT_PROPERTY_VALUE_END                 , /* 属性值结束，等待>    */
    AUDIO_XML_ANASTT_BUTT
};
typedef VOS_UINT32 AUDIO_XML_ANALYSE_STATUS_ENUM_UINT32;
typedef VOS_UINT32 (*AUDIO_XML_FUN)(s8 cnowchar);

/*****************************************************************************
4 UNION定义
*****************************************************************************/

/*****************************************************************************
5 STRUCT定义
*****************************************************************************/

/*****************************************************************************
结构名    : XML_NODE_PROPERTY
协议表格  :
ASN.1描述 :
结构说明  : 节点属性链表的单元
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulnamelength;                  /* pcPropertyName 缓冲区长度  */
    VOS_UINT32 ulvaluelength;                 /* pcPropertyValue 缓冲区长度 */
    VOS_INT8* pcpropertyname;                 /* 属性名称                   */
    VOS_INT8* pcpropertyvalue;                /* 属性值                     */
}AUDIO_XML_NODE_PROPERTY_STRU;

/*****************************************************************************
结构名    : XML_NODE_STRU
协议表格  :
ASN.1描述 :
结构说明  : XML树的节点
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ullabellength;                 /* pcNodeLabel 缓冲区长度*/
    VOS_INT8*  pcnodelabel;                   /* 节点标签*/
    VOS_UINT32 ulPropertyIndex;               /*属性节点索引,记录stproperty的下标值*/
    AUDIO_XML_NODE_PROPERTY_STRU stproperty[AUDIO_XML_NODE_PROPERTY_NUM]; /* 属性 */
}AUDIO_XML_NODE_STRU;

/*****************************************************************************
结构名    : XML_ERROR_INFO_STRU
协议表格  :
ASN.1描述 :
结构说明  : 用于记录XML错误信息
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulxmlline;      /* XML对应的行号      */
    VOS_UINT32 ulstatus;       /* XML对应的解析状态  */
    VOS_UINT32 ulcodeline;     /* 出错代码对应的行号 */
    VOS_UINT16 usnvid;         /* NV ID的值          */
    VOS_UINT16 usreserve;      /* NV ID的值          */
    VOS_UINT16 ulresult;       /* 返回的结果  */
} AUDIO_XML_ERROR_INFO_STRU;

typedef struct
{
    VOS_UINT16            arraylocation;    /*NV ID在NV数组中的位置*/
    VOS_UINT16            nv_id;            /*nv id*/
    ANFU_ECNR_NV_STRU     AudioNvValue;     /*nv value*/
}AUDIO_DELAT_XML_NV_INFO_STRU;

typedef struct
{
    AUDIO_XML_NODE_STRU g_stlxmlcurrentnode;            /*node结构体*/
    VOS_INT8*  g_pclfilereadbuff;                       /*读取文件时缓存buffer*/
    VOS_UINT32 g_stlxml_lineno;                         /*记录xml文件行号*/
    AUDIO_XML_ERROR_INFO_STRU    g_stlxmlerrorinfo;     /*错误类型结构体*/
}AUDIO_XML_DOCODE_INFO;
/*****************************************************************************
6 消息头定义
*****************************************************************************/


/*****************************************************************************
7 消息定义
*****************************************************************************/


/*****************************************************************************
8 全局变量声明
*****************************************************************************/


/*****************************************************************************
9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
10 函数声明
*****************************************************************************/
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_decode_main(VOS_UCHAR* ulFiledir);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_decode_xml_file(VOS_UCHAR* xmldir);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procinit();
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_analyse(VOS_INT8 cnowchar);

AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlorginal(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_enter_label(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlignore(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_node_label(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xmlsingle_endlabel(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlend_mustberight(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertystart(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertyname(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertyname_tail(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_valuestart(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_valuetail(VOS_INT8 cnowchar);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_nv_search_byid(VOS_UINT16 itemid);
AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_write_nv(void);
void audio_xml_write_error_log(VOS_UINT32 ulerrorline, VOS_UINT16 ulnvid, VOS_UINT32 ret);
void audio_xml_help(void);
#endif /* FEATURE_ON == MBB_WPG_PCM */

#pragma pack(pop)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _NV_XML_DEC_H_ */
