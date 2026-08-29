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


/*****************************************************************************
1 头文件包含
*****************************************************************************/
#include "ATCmdProc.h"
#include "AtParse.h"
#include "AtCheckFunc.h"
#include "AtCmdCallProc.h"
#include "AppVcApi.h"
#include "AtSndMsg.h"
#include "AtTafAgentInterface.h"
#include "AtEventReport.h"
#include "AtMsgPrint.h"
#include "MbbAudioNvXmlDec.h"
#include "MbbAtCmdCallProc.h"
#include "TafTypeDef.h"
#include "hi_list.h"
#include "AtMtaInterface.h"
#include "AtTestParaCmd.h"
#include "TafStdlib.h"
#include "CodecNvInterface.h"
#include "CodecNvId.h"
#include <linux/syscalls.h>
#include <linux/string.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#if(FEATURE_ON == MBB_WPG_PCM)
/*****************************************************************************
2 全局变量定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_AUDIO_NV_XML_DEC_C

/*xml decode info*/
static AUDIO_XML_DOCODE_INFO audio_xml_ctrl;

/* XML关键字,不包括0-9,a-z,A-Z */
static VOS_INT8  g_stlaudioxmkeywordtbl[] = { '<', '>', '/', '=', '"', \
                                   ' ', '!', '?', '_', '-', \
                                   ',','{','}','[',']'};

/* NV ID表*/
VOS_UINT16  g_stlaudioxmnvIdtbl[AUDIO_CTRL_MODE_SUB_ID_NUM] \
= {en_Anfu_NB_CarFree1, en_Anfu_NB_CarFree2, en_Anfu_WB_CarFree1, en_Anfu_WB_CarFree2};

/* XML文件解析时的状态                  */
static AUDIO_XML_ANALYSE_STATUS_ENUM_UINT32 g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

/* XML文件首部和尾部标签白名单 */
static const VOS_INT8 *g_paudioxmlheadandtaillabel[AUDIO_DELTA_XML_HEAD_TAIL_LABEL_NUM] \
= {"NV_CATEGORY", "PROJECT_GROUP"};

/* NV节点名称*/
static VOS_INT8 g_aclnodeaudiolabelnv[] = "NV";

/* NV id属性名称 */
static VOS_INT8 g_aclaudiopropertyId[] = "ID";

/*PARAM_VALUE属性名称*/
static VOS_INT8 g_aclpropertyparamvalue[] = "PARAM_VALUE";

/* 节点十六进制值之间的分隔符 */
static VOS_INT8 g_separator = ',';

/*定义存储NVID和NV value值的全局变量*/
static AUDIO_DELAT_XML_NV_INFO_STRU g_audio_delta_nv[AUDIO_CTRL_MODE_SUB_ID_NUM] = {0};

/*NV对应的数组下标*/
static VOS_UINT16 g_nv_number = 0;

/*Global map table used to find the function according the xml analyse status.*/
/*lint -e64*/
AUDIO_XML_FUN g_uslaudioxmlanalysefuntbl[] =
{
    audio_xml_procxmlorginal,             /* 初始状态下的处理 */
    audio_xml_proc_xml_enter_label,       /* 进入Lable后的处理*/
    audio_xml_procxmlignore,              /* 序言或注释状态下直到遇到">"结束*/
    audio_xml_proc_xml_node_label,        /* 标签名字开始 */
    audio_xml_proc_xmlsingle_endlabel,    /* 标准的结尾标签</XXX> */
    audio_xml_procxmlend_mustberight,     /* 形如 <XXX/>的标签,在解析完/的状态 */
    audio_xml_proc_xml_propertystart,     /* 开始解析属性 */
    audio_xml_proc_xml_propertyname,      /* 开始解析属性名字*/
    audio_xml_proc_xml_propertyname_tail, /* 属性名字结束，等待"即属性值开始   */
    audio_xml_proc_xml_valuestart,        /* 属性值开始*/
    audio_xml_proc_xml_valuetail,         /* 属性值结束*/
};

/* 校验XML文件首部和尾部标签的函数声明 */
static AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_checkheadandtaillabel(void);

/*****************************************************************************
3 函数定义
*****************************************************************************/

void audio_xml_write_error_log(VOS_UINT32 ulerrorline, VOS_UINT16 ulnvid,
                                VOS_UINT32 ret)
{
    audio_xml_ctrl.g_stlxmlerrorinfo.ulxmlline = audio_xml_ctrl.g_stlxml_lineno;
    audio_xml_ctrl.g_stlxmlerrorinfo.ulstatus = g_stlaudioxmlstatus;
    audio_xml_ctrl.g_stlxmlerrorinfo.ulcodeline = ulerrorline;
    audio_xml_ctrl.g_stlxmlerrorinfo.usnvid = ulnvid;
    audio_xml_ctrl.g_stlxmlerrorinfo.ulresult = (VOS_UINT32)ret;

    return ;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_checkxmlkeyword(VOS_INT8 currentchar)
{
    VOS_UINT32 lcount = 0;

    if ((('0' <= currentchar) && ('9' >= currentchar))   /* 有效字符：0-9  */
        || (('a' <= currentchar) && ('z' >= currentchar)) /* 有效字符：a-z  */
        || (('A' <= currentchar) && ('Z' >= currentchar)))/* 有效字符：A-Z  */
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 除 0-9,a-z,A-Z 之外的 XML关键字 */
    for (lcount = 0; lcount < sizeof(g_stlaudioxmkeywordtbl); lcount++)
    {
        if (currentchar == g_stlaudioxmkeywordtbl[lcount])
        {
            return AUDIO_XML_RESULT_SUCCEED;
        }
    }
 
    return AUDIO_XML_RESULT_FALIED_BAD_CHAR;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_checkcharvalidity(VOS_INT8 currentchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    if (('\r' == currentchar)       /* 忽略回车   */
        || ('\t' == currentchar))   /* 忽略制表符 */
    {
        return AUDIO_XML_RESULT_SUCCEED_IGNORE_CHAR;
    }

    if ('\n' == currentchar)    /* 忽略换行   */
    {
        audio_xml_ctrl.g_stlxml_lineno++;
        return AUDIO_XML_RESULT_SUCCEED_IGNORE_CHAR;
    }

    /* 在注释中的字符不做检查 */
    if ( AUDIO_XML_ANASTT_IGNORE == g_stlaudioxmlstatus)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 检查XML的关键字 */
    returnval = audio_xml_checkxmlkeyword(currentchar);
 
    if (AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        audio_xml_write_error_log(__LINE__, 0, returnval);

        return returnval;
    }

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_stringtodec(VOS_INT8  *pcbuff,
                                            VOS_INT32 *pusretval)
{
    VOS_UINT32 ultemp = 0;  /* 字符串转成整型时的中间变量 */
    VOS_INT8   currentchar;
    VOS_INT8   *pcsrc;
    VOS_BOOL   bMinus = VOS_FALSE;

    pcsrc = pcbuff;

    /* 如果NV ID是空的，则返回错误 */
    if (0 == *pcsrc)
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_NV_ID_IS_NULL);

        return AUDIO_XML_RESULT_FALIED_NV_ID_IS_NULL;
    }

    if (('0' > *pcsrc || '9' < *pcsrc) && ('+' == *pcsrc || '-' == *pcsrc))
    {
        if ('-' == *pcsrc)
        {
            bMinus = VOS_TRUE;
        }
        pcsrc++;
    }

    /* 把字符串转成十进制的格式 */
    while ('\0' != *pcsrc)
    {
        currentchar = *pcsrc;

        /* 对不在0－9之间的字符，按错误处理 */
        if ((currentchar < '0') || (currentchar > '9'))
        {
            audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_OUT_OF_0_9);
            return AUDIO_XML_RESULT_FALIED_OUT_OF_0_9;
        }

        
        currentchar -= '0';
        /* 转成十进制格式 */
        ultemp = (ultemp * 10) + (VOS_UINT8)currentchar;

        pcsrc++;
    }

    /* 输出 转换后的值 */
    *pusretval = (bMinus ? (0 - (VOS_UINT16)ultemp) : (VOS_UINT16)ultemp);

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_stringtovalue(VOS_UINT8  *pucbuff)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;
    VOS_UINT8 *pcsrc;
    VOS_UINT8 pcdest[AUDIO_DELTA_XML_NV_VALUE_NUM][AUDIO_DELTA_XML_NV_VALUE_LENGTH] = {0};
    VOS_INT32 usnvitemvalue = 0;
    VOS_INT8   currentchar;
    VOS_UINT16 i = 0;
    VOS_UINT16 j = 0;
    VOS_UINT16 nv_num = 0;
    
    pcsrc  = pucbuff;

     /* 如果NV VALUE是空的，则返回错误 */
    if (0 == *pcsrc)
    {
        printk(KERN_ERR " the string is null!\n");

        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_NV_VALUE_IS_NULL);
        return AUDIO_XML_RESULT_FALIED_NV_VALUE_IS_NULL;
    }

    /* 先把原字符串中的分隔符去掉 */
    while ('\0' != *pcsrc)
    {
        currentchar = *pcsrc;

        if(('{' == currentchar) || ('[' == currentchar) || \
           ('}' == currentchar) || (']' == currentchar))
        {
            pcsrc++;
            continue;
        }
        
        /* 如果当前字符是分隔符 */
        if (g_separator == currentchar)
        {
            pcdest[nv_num][j] = '\0';

            nv_num += 1;
            j = 0;

            pcsrc++;
            continue;
        }

        pcdest[nv_num][j++] = currentchar;

        pcsrc++;

    }

    /* 最后一个字符串加上字符串结束府'\0' */
    pcdest[nv_num][j] = '\0';

    /*检查NV的个数是否正确*/
    if ((AUDIO_DELTA_XML_NV_VALUE_NUM - 1) != nv_num)
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_MALLOC);

        return AUDIO_XML_RESULT_GET_NV_DATA_FAIL;
    }

    for (i = 0; i < AUDIO_DELTA_XML_NV_VALUE_NUM; i++)
    {
        returnval = audio_xml_stringtodec(pcdest[i],&usnvitemvalue);

        if (AUDIO_XML_RESULT_SUCCEED != returnval)
        {
            return returnval;
        }
        else
        {
            g_audio_delta_nv[g_nv_number].AudioNvValue.ashwNv[i] = (VOS_INT16)usnvitemvalue;

            usnvitemvalue = 0;
        }
    }
    
    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_get_nv_data(void)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_BUTT;
    VOS_INT32  usnvitemid   = 0;
    VOS_UINT32 ulPropertyIndex = 0;

    /* 如果属性值为空,则不做任何处理*/
    audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[0].pcpropertyvalue[
                        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[0].ulvaluelength] = '\0';

    if ((0 == *audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[0].pcpropertyvalue))
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }
    for(ulPropertyIndex = 0; ulPropertyIndex <= audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex; ulPropertyIndex++)
    {
        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname[
                    audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulnamelength] = '\0';

        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyvalue[
                            audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulvaluelength] = '\0';

        if(!strcmp(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname, g_aclaudiopropertyId))
        {
            /* 把id 属性值转成NV ID */
            returnval = audio_xml_stringtodec(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyvalue,
                                        &usnvitemid);

            if (AUDIO_XML_RESULT_SUCCEED != returnval)
            {
                goto out;
            }

            /*查看当前NV ID是否为音频NV ID*/
            returnval = audio_xml_nv_search_byid((VOS_UINT16)usnvitemid);

            if(AUDIO_XML_RESULT_SUCCEED == returnval)
            {
                g_audio_delta_nv[g_nv_number].nv_id = usnvitemid;
                continue;
            }
            else
            {
                return AUDIO_XML_RESULT_FALIED_NV_ID_IS_ERROR;
            }
        }
        else if(!strcmp(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname, g_aclpropertyparamvalue))
        {
            /*防止没有NVID，只有param value的情况*/
            if( 0 == g_audio_delta_nv[g_nv_number].nv_id)
            {
                break;
            }

            /* 把param value字符串转换为数值*/
            returnval = audio_xml_stringtovalue(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyvalue);

            if (AUDIO_XML_RESULT_SUCCEED != returnval)
            {
                goto out;
            }
            else
            {
                g_nv_number += 1;
                break;
            }
        }
        else
        {
            continue;
        }
    }

    return AUDIO_XML_RESULT_SUCCEED;
out:
    /* 记录出错的NV ID */
    return AUDIO_XML_RESULT_GET_NV_DATA_FAIL;
}


void audio_xml_nodereset(void)
{
    VOS_UINT32 ulPropertyIndex = 0;

    /* 节点标签复位,已使用的长度为0  */
    audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength = 0;

    /*xml_ctrl.g_stlxmlcurrentnode.stproperty的下标索引归0*/
    audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex = 0;
  
    for(ulPropertyIndex = 0; ulPropertyIndex < AUDIO_XML_NODE_PROPERTY_NUM; ulPropertyIndex++)
    {
        /* 节点属性名复位,已使用的长度为0 */
        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulnamelength = 0;

        /* 节点属性值复位,已使用的长度为0 */
        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulvaluelength = 0;
    }

    return;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_write_char_to_buff(VOS_INT8   cnowchar,
                                                 VOS_INT8   *pcstrbuff,
                                                 VOS_UINT32 *plbufflength,
                                                 VOS_BOOL    ulisparamvalue)
{
    /* 忽略空格 */
    if (' ' == cnowchar)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 如果达到了Node Lable或者param name的最大长度 */
    if ((VOS_FALSE == ulisparamvalue) \
        && (*plbufflength >= AUDIO_XML_NODE_LABEL_BUFF_LENGTH_ORIGINAL))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_OUT_OF_BUFF_LEN);

        return AUDIO_XML_RESULT_FALIED_OUT_OF_BUFF_LEN;
    }

    /* 如果达到了param value的最大长度 */
    if ((VOS_TRUE == ulisparamvalue) \
        && (*plbufflength >= AUDIO_XML_PARAM_VALUE_BUFF_LENGTH_ORIGINAL))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_OUT_OF_BUFF_LEN);

        return AUDIO_XML_RESULT_FALIED_OUT_OF_BUFF_LEN;
    }

    /* 把新字符加进缓冲区 */
    *(pcstrbuff + *plbufflength) = cnowchar;

    /* 缓冲区长度加1 */
    (*plbufflength)++;

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_createaproperty( void )
{
    VOS_UINT32 propertyIndex = 0;

    /* 属性节点的下标索引归0 */
    audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex = 0;

    for(propertyIndex = 0;propertyIndex < AUDIO_XML_NODE_PROPERTY_NUM; propertyIndex++)
    {
        /* 分配属性名内存,+1为保留字符串结束符用 */
        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].ulnamelength = 0; /* 已使用的长度 */

        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].pcpropertyname = \
                        (VOS_INT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, AUDIO_XML_NODE_LABEL_BUFF_LENGTH_ORIGINAL + 1);

        if (NULL == audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].pcpropertyname)
        {
             audio_xml_write_error_log(__LINE__, propertyIndex, AUDIO_XML_RESULT_FALIED_MALLOC);

            return AUDIO_XML_RESULT_FALIED_MALLOC;
        }

        /* 分配属性值内存,+1为保留字符串结束符用 */
        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].ulvaluelength = 0; /* 已使用的长度 */


        audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].pcpropertyvalue = \
                   (VOS_INT8*)PS_MEM_ALLOC(WUEPS_PID_AT,AUDIO_XML_PARAM_VALUE_BUFF_LENGTH_ORIGINAL + 1);
        
        if (NULL == audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[propertyIndex].pcpropertyvalue)
        {
            audio_xml_write_error_log(__LINE__, propertyIndex, AUDIO_XML_RESULT_FALIED_MALLOC);

            return AUDIO_XML_RESULT_FALIED_MALLOC;
        }
    }
    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_createanode(void)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;

    /* 创建一个新属性 */
    returnval = audio_xml_createaproperty();

    if(AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        return returnval;
    }

    /* 分配节点标签内存,+1为保留字符串结束符用*/
    audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength = 0; /* 已使用的长度 */

    audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel = (VOS_INT8*)PS_MEM_ALLOC( \
                               WUEPS_PID_AT,AUDIO_XML_NODE_LABEL_BUFF_LENGTH_ORIGINAL + 1);

    if (NULL ==  audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel)
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_MALLOC);

        return AUDIO_XML_RESULT_FALIED_MALLOC;
    }
    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlorginal(VOS_INT8 cnowchar)
{
    /* 遇到<则更改状态 */
    if ('<' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ENTER_LABLE;
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到空格则继续 */
    if (' ' == cnowchar)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到>,/,",=则表示XML语法错误 */
    if (('>' == cnowchar)
         || ('/' == cnowchar)
         || ('"' == cnowchar)
         || ('=' == cnowchar)
         || ('[' == cnowchar)
         || (']' == cnowchar)
         || ('}' == cnowchar)
         || ('{' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);
        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlignore(VOS_INT8 cnowchar)
{

    /* 直到遇到标签结尾，否则一直忽略 */
    if ('>' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;
    }
    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xmlsingle_endlabel(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 遇到<则更改状态 */
    if ('>' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

        /* 检查当前结尾标签的有效性 */
        audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel[audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength] = '\0';
        returnval = audio_xml_checkheadandtaillabel();

        return returnval;
    }

    /* 遇到<,/,",=则表示XML语法错误 */
    if (('<' == cnowchar)
         || ('"' == cnowchar)
         || ('/' == cnowchar)
         || ('=' == cnowchar)
         || (']' == cnowchar)
         || ('[' == cnowchar)
         || ('}' == cnowchar)
         || ('{' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    /* 把这个字节放进当前节点值的缓冲区内 */
    returnval = audio_xml_write_char_to_buff(cnowchar,
                                     audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel,
                                     &(audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength),
                                     VOS_FALSE);

    return returnval;
}


static AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_checkheadandtaillabel(void)
{
    VOS_INT8 labelindex = 0;
    int ret = -1;

    for (labelindex = 0; labelindex < AUDIO_DELTA_XML_HEAD_TAIL_LABEL_NUM; labelindex++)
    {
        ret = strcmp(audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel,
                         g_paudioxmlheadandtaillabel[labelindex]);

        /* 如果当前标签在XML文件首部和尾部标签白名单中，则也认为是合法标签 */
        if (0 == ret)
        {
            /* <xx/>标签结束时，清空节点信息 */
            audio_xml_nodereset();

            return AUDIO_XML_RESULT_SUCCEED;
        }
    }

    audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_CHAR);

    return AUDIO_XML_RESULT_FALIED_BAD_CHAR;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_checknode_rightlabel(void)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 判断该标签是否有效 */
    audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel[audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength] = '\0';

    /* 如果不是NV标签，则需进一步检查是否属于XML首部标签 */
    if (strcmp(audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel,
                             g_aclnodeaudiolabelnv))
    {
        returnval = audio_xml_checkheadandtaillabel();

        return returnval;
    }
    else
    {
        /* 写节点信息到NV中 */
        returnval = audio_xml_get_nv_data();

        if (AUDIO_XML_RESULT_SUCCEED != returnval)
        {
            return returnval;
        }

        /* <xx/>标签结束时，清空节点信息 */
        audio_xml_nodereset();
    }

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procxmlend_mustberight(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 忽略空格 */
    if (' ' == cnowchar)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到不是>,则表示XML语法错误 */
    if ('>' != cnowchar)
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    /* 变更状态 */
    g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

    /*must be right need to check label*/
    returnval = audio_xml_checknode_rightlabel();

    return returnval;

}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_node_label(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 遇到/或者>或者空格说明Node的名字结束了 */
    if ('/' == cnowchar)
    {
        /* 结束并收尾整个节点,下个字节一定是> */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_LABLE_END_MUST_RIGHT;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 标签结束 */
    if ('>' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

        /*检查当前节点的有效性*/
        returnval = audio_xml_checknode_rightlabel();

        return returnval;
    }

    /* 标签名字结束,进入属性解析状态 */
    if (' ' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_START;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到<,",=则表示XML语法错误 */
    if (('<' == cnowchar)
         || ('"' == cnowchar)
         || ('=' == cnowchar)
         || ('[' == cnowchar)
         || (']' == cnowchar)
         || ('{' == cnowchar)
         || ('}' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0,AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    /* 把这个字节放进当前节点值的缓冲区内 */
    returnval = audio_xml_write_char_to_buff(cnowchar,
                                     audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel,
                                     &(audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength),
                                     VOS_FALSE);
    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_enter_label(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;

    /* 遇到首行版本信息 */
    if ('?' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_IGNORE;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到序言 */
    if ('!' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_IGNORE;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到尾节点标签 */
    if ('/' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_SINGLE_ENDS_LABLE;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到标签结束 */
    if ('>' == cnowchar)
    {
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

        /*检查当前节点的有效性*/
        returnval = audio_xml_checknode_rightlabel();

        return returnval;
    }

    /* 遇到<,",=则表示XML语法错误 */
    if (('<' == cnowchar)
         || ('"' == cnowchar)
         || ('=' == cnowchar)
         || ('[' == cnowchar)
         || (']' == cnowchar)
         || ('{' == cnowchar)
         || ('}' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);
        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    /* 跳过空格 */
    if (' ' != cnowchar)
    {
        /* 变更状态，表示进入一个新节点 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_NODE_LABLE;

        /* 把这个字节放进当前节点值的缓冲区内 */
        returnval = audio_xml_write_char_to_buff(cnowchar,
                                         audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel,
                                         &(audio_xml_ctrl.g_stlxmlcurrentnode.ullabellength),
                                         VOS_FALSE);
    }

    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertystart(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;
    VOS_UINT32 ulPropertyIndex = 0;


    /* 遇到尾节点标签 */
    if ('/' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_LABLE_END_MUST_RIGHT;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 标签结束 */
    if ('>' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

        /*检查当前节点的有效性*/
        returnval = audio_xml_checknode_rightlabel();

        return returnval;
    }

    /* 遇到<,",=则表示XML语法错误 */
    if (('<' == cnowchar)
         || ('"' == cnowchar)
         || ('=' == cnowchar)
         || ('[' == cnowchar)
         || (']' == cnowchar)
         || ('{' == cnowchar)
         || ('}' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    /* 更改状态 */
    g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_NAME_START;

    ulPropertyIndex = audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex;

    returnval = audio_xml_write_char_to_buff(cnowchar,
                  audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname,
                  &(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulnamelength),
                  VOS_FALSE);

    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertyname(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;
    VOS_UINT32 ulPropertyIndex = 0;

    /* 等待=进入属性值解析 */
    if ('=' == cnowchar)
    {
        /* 翻状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_NAME_END;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到<,>,/,"则表示XML语法错误 */
    if (('<' == cnowchar)
        || ('>' == cnowchar)
        || ('/' == cnowchar)
        || ('"' == cnowchar)
        || (']' == cnowchar)
        || ('[' == cnowchar)
        || ('}' == cnowchar)
        || ('{' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    ulPropertyIndex = audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex;

    /* 容许属性名中的空格错误, 如 <nv i d="123"> */
    returnval = audio_xml_write_char_to_buff(cnowchar,
                  audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname,
                  &(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulnamelength),
                  VOS_FALSE);

    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_propertyname_tail(VOS_INT8 cnowchar)
{

    /* 跳过空格 */
    if ( ' ' == cnowchar)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 等待" */
    if ('"' == cnowchar)
    {
        /* 更改状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_VALUE_START;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到不是"，则表示XML语法错误 */
    audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

    return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_valuestart(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;
    VOS_UINT32 ulPropertyIndex = 0;

    /* 遇到" */
    if ('"' == cnowchar)
    {
        /* 翻状态,返回开始解析属性的状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_VALUE_END;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到<,>,/,=则表示XML语法错误 */
    if (('<' == cnowchar)
         || ('>' == cnowchar)
         || ('/' == cnowchar)
         || ('=' == cnowchar))
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

        return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
    }

    ulPropertyIndex = audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex;

    /* 把当前字符加到属性值中 */
    returnval = audio_xml_write_char_to_buff(cnowchar,
                  audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyvalue,
                  &(audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].ulvaluelength),
                  VOS_TRUE);

    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_proc_xml_valuetail(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 空格下一个属性解析开始 */
    if (' ' == cnowchar)
    {
        /* 变更状态,返回开始解析属性的状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_PROPERTY_START;
        audio_xml_ctrl.g_stlxmlcurrentnode.ulPropertyIndex++;
        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到'/' */
    if ('/' == cnowchar)
    {
        /* 变更状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_LABLE_END_MUST_RIGHT;

        return AUDIO_XML_RESULT_SUCCEED;
    }

    /* 遇到'>' */
    if ('>' == cnowchar)
    {
        /* 变更状态,返回开始解析属性的状态 */
        g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

        /* 检查当前节点有效 */
        returnval = audio_xml_checknode_rightlabel();

        return returnval;
    }

    /* 遇到不是>,/则表示XML语法错误 */
    audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_BAD_SYNTAX);

    return AUDIO_XML_RESULT_FALIED_BAD_SYNTAX;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_analyse(VOS_INT8 cnowchar)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = 0;

    /* 检查当前字符的有效性 */
    returnval = audio_xml_checkcharvalidity(cnowchar);

    if (AUDIO_XML_RESULT_SUCCEED_IGNORE_CHAR == returnval)
    {
        /* 如果遇到序言，则跳过该字符 */
        return AUDIO_XML_RESULT_SUCCEED;
    }

    if (AUDIO_XML_RESULT_FALIED_BAD_CHAR == returnval)
    {
        /* 如果遇到非法字符，则停止解析 */
        return AUDIO_XML_RESULT_FALIED_BAD_CHAR;
    }
    
    /* 调用XML解析时，相应状态的对应函数 */
    returnval = g_uslaudioxmlanalysefuntbl[g_stlaudioxmlstatus](cnowchar);

    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_decode_xml_file(VOS_UCHAR* xmldir)
{
    VOS_INT32                     lreaded   = 0;       /* 读出的字节数 */
    VOS_INT32                     lcount    = 0;       /* 遍历缓冲区用 */
    VOS_INT16                     fd        = 0;
    AUDIO_XML_RESULT_ENUM_UINT32  returnval = 0;

    printk(KERN_ERR " xml_decode_xml_file: xmldir = %s!\n",xmldir);

    /*打开文件路径*/
    fd = (unsigned int)sys_open(xmldir, O_RDONLY, 0);

    if(fd < 0)
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_READ_FILE);
        return AUDIO_XML_RESULT_FALIED_READ_FILE;
    }

    /*读取文件*/
    lreaded = (int)sys_read(fd,(VOS_UINT8*)(audio_xml_ctrl.g_pclfilereadbuff),AUDIO_XML_FILE_READ_BUFF_SIZE);

    /* 音频参数delta文件为空的异常处理 */
    if (0 == lreaded)
    {
        return AUDIO_XML_RESULT_SUCCEED;
    }

    if ( (lreaded > AUDIO_XML_FILE_READ_BUFF_SIZE) || (lreaded < 0) )
    {
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_READ_FILE);
        return AUDIO_XML_RESULT_FALIED_READ_FILE;
    }

    for(lcount = 0;lcount < lreaded; lcount++)
    {
        returnval = audio_xml_analyse(*(audio_xml_ctrl.g_pclfilereadbuff + lcount));

        if(AUDIO_XML_RESULT_SUCCEED != returnval)
        {
            /* 遇到解析错误，则停止解析 */
            return returnval;
        }
    }

    return AUDIO_XML_RESULT_SUCCEED;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_procinit(void)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;

    /* 创建节点信息 */
    returnval = audio_xml_createanode();

    if (AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        return returnval;
    }

    /* 初始化当前状态 */
    g_stlaudioxmlstatus = AUDIO_XML_ANASTT_ORIGINAL;

    /* 用于记录读取XML文件的行数 */
    audio_xml_ctrl.g_stlxml_lineno    = 1;

    /* 申请读取文件数据的缓冲区,+1为保留字符串结束符用 */ 
    audio_xml_ctrl.g_pclfilereadbuff = (VOS_INT8*)PS_MEM_ALLOC( \
                               WUEPS_PID_AT,AUDIO_XML_FILE_READ_BUFF_SIZE + 1);
    
    if (NULL == audio_xml_ctrl.g_pclfilereadbuff)
    {      
        audio_xml_write_error_log(__LINE__, 0, AUDIO_XML_RESULT_FALIED_MALLOC);
        return AUDIO_XML_RESULT_FALIED_MALLOC;
    }
    
    return AUDIO_XML_RESULT_SUCCEED;
}


void audio_xml_freemem(void)
{
    VOS_UINT32 ulPropertyIndex = 0;

    for(ulPropertyIndex = 0; ulPropertyIndex < AUDIO_XML_NODE_PROPERTY_NUM; ulPropertyIndex++)
    {
        PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname);

        PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyvalue);
    }
    PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel);

    PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_pclfilereadbuff);
}


void audio_xml_decodefail_freemem(void)
{
    VOS_UINT32 ulPropertyIndex = 0;

    for(ulPropertyIndex = 0; ulPropertyIndex < AUDIO_XML_NODE_PROPERTY_NUM; ulPropertyIndex++)
    {
        if(NULL != audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname)
        {
            PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname);
            audio_xml_ctrl.g_stlxmlcurrentnode.stproperty[ulPropertyIndex].pcpropertyname = NULL;
        }
    }

    if(NULL != audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel)
    {
        PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel);
        audio_xml_ctrl.g_stlxmlcurrentnode.pcnodelabel = NULL;
    }

    if(NULL != audio_xml_ctrl.g_pclfilereadbuff)
    {
        PS_MEM_FREE(WUEPS_PID_AT,audio_xml_ctrl.g_pclfilereadbuff);
        audio_xml_ctrl.g_pclfilereadbuff = NULL;
    }

}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_decode_main(VOS_UCHAR* ulFiledir)
{
    AUDIO_XML_RESULT_ENUM_UINT32 returnval = AUDIO_XML_RESULT_SUCCEED;

    /*初始化全局变量*/
    returnval = audio_xml_procinit();

    if (AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        goto out;
    }

    /* 解析xnv.xml文件  */
    returnval = audio_xml_decode_xml_file(ulFiledir);

    if (AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        goto out;
    }

    /*写音效相关的NV*/
    returnval = audio_xml_write_nv();

    if (AUDIO_XML_RESULT_SUCCEED != returnval)
    {
        goto out;
    }

    /*将记录NV个数的全局变量置为0*/
    g_nv_number = 0;
    
    /* 释放已分配的内存 */
    audio_xml_freemem();
 
    return AUDIO_XML_RESULT_SUCCEED;
out:
    audio_xml_help();
    audio_xml_decodefail_freemem();
    return returnval;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_nv_search_byid(VOS_UINT16 itemid)
{
    VOS_UINT16 lcount = 0;

    for (lcount = 0; lcount < ( sizeof(g_stlaudioxmnvIdtbl) / sizeof(g_stlaudioxmnvIdtbl[0]) ); lcount++)
    {
        if (itemid == g_stlaudioxmnvIdtbl[lcount])
        {
            g_audio_delta_nv[g_nv_number].arraylocation = lcount;
            return AUDIO_XML_RESULT_SUCCEED;
        }
    }
    
    printk(KERN_ERR "audio_xml_nv_search_byid: err char :%d\n",itemid);
    return AUDIO_XML_RESULT_FALIED_NV_ID_IS_NULL;
}


AUDIO_XML_RESULT_ENUM_UINT32 audio_xml_write_nv(void)
{
    VOS_UINT16            lcount = 0;

    for (lcount = 0; lcount < g_nv_number; lcount++)
    {
        if (NV_OK != NV_WriteEx(MODEM_ID_0,g_stlaudioxmnvIdtbl[g_audio_delta_nv[lcount].arraylocation], \
                   g_audio_delta_nv[lcount].AudioNvValue.ashwNv,AUDIO_DELTA_XML_NV_VALUE_NUM*sizeof(VOS_INT16)))
        {
            printk(KERN_ERR "audio_xml_write_nv():ERROR: the nv id is %d\n",g_audio_delta_nv[lcount].nv_id);
            return AUDIO_XML_RESULT_WRITE_NV_DATA_FAIL;
        }
    }

    return AUDIO_XML_RESULT_SUCCEED;
}


void audio_xml_help(void)
{
    printk(KERN_ERR "audio_xml_ctrl.g_stlxmlerrorinfo.ulxmlline  =  %d\n",audio_xml_ctrl.g_stlxmlerrorinfo.ulxmlline);
    printk(KERN_ERR "audio_xml_ctrl.g_stlxmlerrorinfo.ulstatus   =  %d\n",audio_xml_ctrl.g_stlxmlerrorinfo.ulstatus);
    printk(KERN_ERR "audio_xml_ctrl.g_stlxmlerrorinfo.ulcodeline =  %d\n",audio_xml_ctrl.g_stlxmlerrorinfo.ulcodeline);
    printk(KERN_ERR "audio_xml_ctrl.g_stlxmlerrorinfo.usnvid     =  %d\n",audio_xml_ctrl.g_stlxmlerrorinfo.usnvid);
    printk(KERN_ERR "audio_xml_ctrl.g_stlxmlerrorinfo.ulresult   =  %d\n",audio_xml_ctrl.g_stlxmlerrorinfo.ulresult);
    printk(KERN_ERR "\n");
    printk(KERN_ERR "g_stlaudioxmlstatus                         = %d\n",g_stlaudioxmlstatus);
    printk(KERN_ERR "audio_xml_ctrl.g_stlxml_lineno              = %d\n",audio_xml_ctrl.g_stlxml_lineno);
}

EXPORT_SYMBOL(audio_xml_analyse);
EXPORT_SYMBOL(audio_xml_help);
EXPORT_SYMBOL(audio_xml_write_error_log);
EXPORT_SYMBOL(audio_xml_procinit);
#endif /* FEATURE_ON == MBB_WPG_PCM */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

