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




#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <product_config.h>
#include <mdrv_rfile_common.h>
#include <osl_thread.h>
#include <bsp_nvim.h>
#include <bsp_onoff.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "bsp_blk.h"
#include "nv_xml_dec.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_crc.h"
#include "nv_partition_img.h"
#include "nv_partition_bakup.h"
#include "NVIM_ResumeId.h"
#include "bsp_dump.h"
#include "nv_msg.h"
#include "nv_proc.h"




#include "product_nv_id.h"
#include "bsp_sram.h"

#include <mdrv_chg.h>


#define FACTORY_INFO_LEN              (78)
#define MMI_TEST_FLAG_OFFSET          (24)
#define MMI_TEST_FLAG_LEN             (4)
#define NV_ID_MSP_FACTORY_INFO        (114)

#define SRAM_REBOOT_ADDR  (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_REBOOT_INFO)
#include <power_com.h>  /*fastboot引用的异常信息结构体必备头文件*/
#include "bsp_sram.h"
#include <linux/mtd/flash_huawei_dload.h>

typedef enum
{
    NV_RESTRORE_SUCCESS,
    NV_RESTRORE_FAIL,
}NV_RESTORE_STATUS;


bool nv_isSecListNv(u16 itemid)
{
    /*lint -save -e958*/
    u16 i = 0;

    /*lint -restore*/
    for(i = 0;i < bsp_nvm_getRevertNum(NV_SECURE_ITEM);i++)
    {
        if(itemid == g_ausNvResumeSecureIdList[i])
        {
            return true;
        }
    }
    return false;
}





bool nv_isAutoBakupListNv(u16 itemid)
{
    /*lint -save -e958*/
    u16 i = 0;

    /*lint -restore*/
    for(i = 0;i < bsp_nvm_getRevertNum(NV_MBB_AUTOBACKEUP_ITEM);i++)
    {
        if(itemid == g_ausNvAutoBakeupIdList[i])
        {
            return true;
        }
    }
    return false;
}


/*lint -save -e713 -e830*/

u32 nv_readEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret;
    nv_file_info_s file_info = {0};
    nv_item_info_s item_info = {0};
    nv_rd_req      rreq;
    nv_ctrl_info_s* ctrl_info = (nv_ctrl_info_s*)NV_GLOBAL_CTRL_INFO_ADDR;

    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;
    nv_debug(NV_FUN_READ_EX,0,itemid,modem_id,datalen);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_READ_EX,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    if(NV_ID_DRV_VER_FLAG == itemid)
    {
        printk(KERN_ERR "read itemid = %d\n",itemid);
        memset(pdata,0x00,datalen);
        *pdata = 1;
        return NV_OK;
    }

    /*如果是datalock密码或simlock密码NV,在未授权时禁止读取*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk(KERN_ERR "smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
        if(SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV读取后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk(KERN_ERR  "smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }
    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&item_info,&file_info);
    if(ret)
    {
        nv_printf("can not find 0x%x !\n",itemid);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    if((offset + datalen) > item_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_READ_EX,3,offset,datalen,item_info.nv_len);
        goto nv_readEx_err;
    }

    if((modem_id == 0) || (modem_id > ctrl_info->modem_num))
    {
        ret = BSP_ERR_NV_INVALID_MDMID_ERR;
        nv_debug(NV_FUN_READ_EX,4,ret,itemid,modem_id);
        goto nv_readEx_err;
    }

    if(modem_id > item_info.modem_num)
    {
        ret = BSP_ERR_NV_INVALID_MDMID_ERR;
        nv_debug(NV_FUN_READ_EX,5,ret,itemid,modem_id);
        goto nv_readEx_err;
    }

    rreq.itemid  = itemid;
    rreq.modemid = modem_id;
    rreq.offset  = offset;
    rreq.pdata   = pdata;
    rreq.size    = (datalen < item_info.nv_len) ? datalen : item_info.nv_len;
    (void)nv_read_from_mem(&rreq, &item_info);

    nv_debug_trace(pdata, datalen);

    return NV_OK;

nv_readEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d] 0x%x\n",__FUNCTION__,itemid,modem_id, ret);
    nv_help(NV_FUN_READ_EX);
    return ret;
}

u32 nv_writeEx(u32 modem_id, u32 itemid, u32 offset, u8* pdata, u32 datalen)
{
    u32 ret;
    nv_file_info_s  file_info = {0};
    nv_item_info_s  item_info = {0};
    nv_wr_req       wreq;
    u8  test_byte;
    nv_ctrl_info_s* ctrl_info = (nv_ctrl_info_s*)NV_GLOBAL_CTRL_INFO_ADDR;
    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;

    nv_debug(NV_FUN_WRITE_EX,0,itemid,modem_id,datalen);
    nv_debug_record(NV_DEBUG_WRITEEX_START|itemid<<16);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_WRITE_EX,1,itemid,datalen,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    if(NV_ID_DRV_VER_FLAG == itemid)
    {
        printk(KERN_ERR "write itemid = %d\n",itemid);
        return NV_ERROR;
    }

    /*如果是datalock密码或simlock密码NV,在未授权时禁止操作*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk("smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }

        if (SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV操作后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk("smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }

    /* test pdata is accessable */
    test_byte = *pdata;
    UNUSED(test_byte);

    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&item_info,&file_info);
    if(ret)
    {
        nv_printf("can not find 0x%x !\n",itemid);
        nv_debug(NV_FUN_WRITE_EX,2,itemid,modem_id,offset);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    nv_debug_trace(pdata, datalen);

    if((datalen + offset) >item_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_WRITE_EX,3,itemid,datalen,item_info.nv_len);
        goto nv_writeEx_err;
    }

    if((modem_id == 0) || (modem_id > ctrl_info->modem_num))
    {
        ret = BSP_ERR_NV_INVALID_MDMID_ERR;
        nv_debug(NV_FUN_WRITE_EX,4,itemid,datalen,item_info.nv_len);
        goto nv_writeEx_err;
    }

    if(modem_id > item_info.modem_num)
    {
        ret = BSP_ERR_NV_INVALID_MDMID_ERR;
        nv_debug(NV_FUN_WRITE_EX,5,itemid,datalen,item_info.nv_len);
        goto nv_writeEx_err;
    }

    /* check crc before write */
    if(nv_crc_need_check_inwr(&item_info, datalen))
    {
        ret = nv_crc_check_item(&item_info, modem_id);
        if(ret)
        {
            nv_debug(NV_FUN_WRITE_EX, 6, itemid,datalen,ret);
            ret = nv_resume_item(&item_info, itemid, modem_id);
            if(ret)
            {
                nv_debug(NV_FUN_WRITE_EX,7, itemid, modem_id, ret);
                goto nv_writeEx_err;
            }
        }
    }

    wreq.itemid    = itemid;
    wreq.modemid   = modem_id;
    wreq.offset    = offset;
    wreq.pdata     = pdata;
    wreq.size      = datalen;
    nv_debug_record(NV_DEBUG_WRITEEX_MEM_START);
    ret = nv_write_to_mem(&wreq, &item_info);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,8,itemid,datalen,0);
        goto nv_writeEx_err;
    }

    nv_debug_record(NV_DEBUG_WRITEEX_FILE_START);
    ret = nv_write_to_file(&wreq, &item_info);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,9,itemid,datalen,ret);
        goto nv_writeEx_err;
    }
    nv_debug_record(NV_DEBUG_WRITEEX_END|itemid<<16);

    return NV_OK;

nv_writeEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d]\n",__FUNCTION__,itemid,modem_id);
    nv_help(NV_FUN_WRITE_EX);
    return ret;
}



u32 bsp_nvm_get_nv_num(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    return ctrl_info->ref_count;
}

u32 bsp_nvm_get_nvidlist(NV_LIST_INFO_STRU*  nvlist)
{
    u32 i;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)((unsigned long)NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    if(NULL == nvlist)
    {
        return NV_ERROR;
    }

    for(i = 0;i<ctrl_info->ref_count;i++)
    {
        nvlist[i].usNvId       = ref_info[i].itemid;
        nvlist[i].ucNvModemNum = ref_info[i].modem_num;
    }
    return NV_OK;
}

u32 bsp_nvm_get_len(u32 itemid,u32* len)
{
    u32 ret;
    struct nv_ref_data_info_stru ref_info = {0};
    struct nv_file_list_info_stru file_info = {0};

    nv_debug(NV_API_GETLEN,0,itemid,0,0);
    if(NULL == len)
    {
        nv_debug(NV_API_GETLEN,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    /*check init state*/
    if(false == nv_read_right())
    {
        nv_debug(NV_API_GETLEN,3,itemid,0,0);
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }
    ret = nv_search_byid(itemid,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info, &file_info);
    if(NV_OK == ret)
    {
        *len = ref_info.nv_len;
        return NV_OK;
    }
    return ret;
}

u32 bsp_nvm_authgetlen(u32 itemid,u32* len)
{
    return bsp_nvm_get_len(itemid,len);
}



u32 bsp_nvm_dcread_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right())
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcread(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcreadpart(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right())
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    if(false == nv_write_right())
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcwrite(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcwritepart(u32 modem_id,u32 itemid, u32 offset,u8* pdata,u32 datalen)
{
    if(false == nv_write_right())
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}



u32 bsp_nvm_flush(void)
{
    u32 ret;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    ret = nv_flush_wrbuf(ddr_info);
    if (ret) {
        nv_printf("Fail to flush low priority write buffer \n");
    }

    ret = nv_send_msg_sync(NV_TASK_MSG_FLUSH, 0, 0);
    return ret;
}


u32 bsp_nvm_flushSys(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 ulTotalLen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_create_flag_file((s8*)NV_SYS_FLAG_PATH);

    nv_debug(NV_FUN_FLUSH_SYS,0,0,0,0);
    if(nv_file_access((s8*)NV_FILE_SYS_NV_PATH,0))
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        nv_debug(NV_FUN_FLUSH_SYS,1,ret,0,0);
        ret = BSP_ERR_NV_NO_FILE;
        goto nv_flush_err;
    }
    ulTotalLen = ddr_info->file_len;
    /*在nvdload分区文件末尾置标志0xabcd8765*/
    *( unsigned int* )( NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_len )
        = ( unsigned int )NV_FILE_TAIL_MAGIC_NUM;
    ulTotalLen += sizeof(unsigned int);
    /*系统分区数据不做CRC校验，因此回写时不考虑CRC校验码的存放位置*/
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ulTotalLen,fp);
    nv_file_close(fp);
    if(ret != ulTotalLen)
    {
        nv_debug(NV_FUN_FLUSH_SYS,3,ret,ulTotalLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }

    nv_delete_flag_file((s8*)NV_SYS_FLAG_PATH);
    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__func__);
    nv_help(NV_FUN_FLUSH_SYS);
    return ret;
}




u32 bsp_nvm_backup(u32 crc_flag)
{
    u32 ret = NV_ERROR;
    nv_global_info_s* ddr_info = (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;
    FILE* fp = NULL;
    u32 writeLen = 0;

    nv_debug(NV_API_BACKUP,0,0,0,0);

    if( (ddr_info->acore_init_state != NV_INIT_OK)&&
        (ddr_info->acore_init_state != NV_KERNEL_INIT_DOING))
    {
        return NV_ERROR;
    }

    nv_create_flag_file((s8*)NV_BACK_FLAG_PATH);

    if(nv_file_access((s8*)NV_BACK_PATH,0))
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_BACKUP,1,ret,0,0);
        goto nv_backup_fail;
    }

    writeLen = ddr_info->file_len;

    if(NV_FLAG_NEED_CRC == crc_flag)
    {
        ret = nv_crc_check_ddr(NV_RESUME_NO);
        if(ret)
        {
            nv_debug(NV_API_BACKUP,2,ret,0, 0);
            (void)nv_debug_store_ddr_data();
            goto nv_backup_fail;
        }
    }

    /* 如果需要进行CRC校验, 备份数据到备份区，内存中的数据较备份区更新，所以crc check不能带自动恢复,
       要保证写入备份区的数据，crc校验正确，同时备份过程中内存数据不被改写，所以需要锁住内存 */
    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,writeLen,fp);
    nv_ipc_sem_give(IPC_SEM_NV_CRC);
    nv_file_close(fp);
    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_BACKUP,3,ret,writeLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_backup_fail;
    }

    (void)nv_bakup_info_reset();

    if(nv_file_update(NV_BACK_PATH))
    {
        nv_debug(NV_API_BACKUP, 4 , 0, 0, 0);
    }

    nv_delete_flag_file((s8*)NV_BACK_FLAG_PATH);

    return NV_OK;
nv_backup_fail:
    if(fp){nv_file_close(fp);}
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_BACKUP);
    return ret;

}




/* added by yangzhi for muti-carrier, Begin:*/
/* added by yangzhi for muti-carrier, End! */



/*增加函数入参,判断流程中是否单独写入某个nv*/
u32 bsp_nvm_update_default(bool is_rwnv_updef)
{
    u32 ret;
    FILE* fp = NULL;
    u32 datalen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    NV_SELF_CTRL_STRU self_ctrl = {0};

    nv_debug(NV_FUN_UPDATE_DEFAULT,0,0,0,0);

    if(ddr_info->acore_init_state != NV_INIT_OK)
    {
        return NV_ERROR;
    }


    if(true == is_rwnv_updef)
    {
        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL, (u8*)&self_ctrl, sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            nv_printf("read nv 0x%x fail,ret = 0x%x\n", NV_ID_DRV_SELF_CTRL, ret);
            return NV_ERROR;
        }
        self_ctrl.ulResumeMode = NV_MODE_USER;
        ret = bsp_nvm_write(NV_ID_DRV_SELF_CTRL, (u8*)&self_ctrl, sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            nv_printf("write nv 0x%x fail,ret = 0x%x\n", NV_ID_DRV_SELF_CTRL, ret);
            return NV_ERROR;
        }
    }

    /*在写入文件前进行CRC校验，以防数据不正确*/
    ret = nv_crc_check_ddr(NV_RESUME_NO);
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,3,ret,ddr_info->file_len,0);
        (void)nv_debug_store_ddr_data();
        goto nv_update_default_err;
    }

    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_FUN_UPDATE_DEFAULT,2,ret,0,0);
        goto nv_update_default_err;
    }

    /* 锁住NV内存，在写入文件前进行CRC校验，以防数据不正确，
       同时要保证当前的crc check不做自动恢复，带自动恢复的crc check会在恢复过程中获取ipc semaphore，导致死锁*/
    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT, 3, ret,0,0);
        goto nv_update_default_err;
    }
    datalen = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ddr_info->file_len,fp);
    nv_ipc_sem_give(IPC_SEM_NV_CRC);

    nv_file_close(fp);
    if(datalen != ddr_info->file_len)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,6,ret,ddr_info->file_len,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_update_default_err;
    }

    ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,7,ret,0,0);
        goto nv_update_default_err;
    }

    return NV_OK;
nv_update_default_err:
    /* coverity[deref_arg] */
    if(fp){nv_file_close(fp);}
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_UPDATE_DEFAULT);
    return ret;
}



u32 bsp_nvm_revert_default(void)
{
    u32 ret = NV_ERROR;

    printf("enter to set default nv !\r\n");
    
    ret = nv_revert_data(NV_DEFAULT_PATH,g_ausNvResumeDefualtIdList,\
             bsp_nvm_getRevertNum(NV_MBB_DEFUALT_ITEM), NV_FLAG_NEED_CRC);
    if(ret)
    {
        printf("Set default nv nv_revert_data_with_crc error!\r\n");
        goto err_out;
    }
    
    ret = nv_img_flush_all();
    if(NV_OK != ret)
    {
        nv_error_printf("write back to [img] failed! ret = 0x%x.\n", ret);
        goto err_out;
    }

    ret = bsp_nvm_flushSys();
    if(NV_OK != ret)
    {
        nv_error_printf("write back to [sys] failed! ret = 0x%x.\n", ret);
    }

err_out:
    return ret;
}



/*****************************************************************************
 函 数 名  : nv_atuo_backup
 功能描述  : 处理一些特殊的NV, 这部分NV被修改的时候需要根据需要刷新备份区或者
             恢复出厂区
 输入参数  : 无
 输出参数  : 无
 返 回 值  : TRUE or FALSE
*****************************************************************************/
u32 nv_atuo_backup(u16 itemid)
{
    u32 ret = NV_OK;
    if(true == nv_isAutoBakupListNv(itemid))
    {
        printk(KERN_ERR "%s auto backup list nv, id=%u.\n", __func__, itemid);
        /*设置为false不单独写nv*/
        ret = bsp_nvm_update_default(false);
    }
    else
    {
        if (true == nv_isSecListNv(itemid))
        {
            printk(KERN_ERR "%s sec list nv, id=%u.\n", __func__, itemid);
            ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
        }
    }

    return ret;
}

u32 nv_write2file_handle(nv_cmd_req *msg)
{
    u32 ret;
    nv_item_info_t *nv_info;
    nv_flush_item_s flush_item;

    nv_info = &msg->nv_item_info;
    flush_item.itemid = nv_info->itemid;
    flush_item.modemid = nv_info->modem_id;
    /*写入文件前查看是否需要重启后备份*/
    (void)nv_rebak_check(&flush_item);
    ret = nv_flushItem(&flush_item);
    if (ret)
    {
        return ret;
    }
    /*写入成功后根据白名单写入备份标记*/
    (void)nv_rebak_save_flag();
    /*检查是否立即备份或写入default区,nv可能与上面重复*/
    ret = nv_atuo_backup(nv_info->itemid);
    if(0 != ret)
    {
        printk(KERN_ERR "%s auto backup failed.\n", __func__);
        return ret;
    }

    if (true == nv_isSysNv(nv_info->itemid))
    {
        ret = bsp_nvm_flushSys();
    }

    return ret;
}

void bsp_nvm_icc_task(void* parm)
{
    u32 ret = NV_ERROR;
    nv_cmd_req *msg;
    nv_item_info_t *nv_info;
	
    /* coverity[self_assign] */
    parm = parm;

    /* coverity[no_escape] */
    for(;;)
    {
        osl_sem_down(&g_nv_ctrl.task_sem);

        g_nv_ctrl.opState = NV_OPS_STATE;

        /*如果当前处于睡眠状态，则等待唤醒处理*/
        if(g_nv_ctrl.pmState == NV_SLEEP_STATE)
        {
            printk("%s cur state in sleeping,wait for resume end!\n",__func__);
            osl_sem_down(&g_nv_ctrl.suspend_sem);
        }

        msg = nv_get_cmd_req();
        if (msg == NULL) {
            g_nv_ctrl.opState = NV_IDLE_STATE;
            continue;
        }

        nv_debug_printf("msg type:0x%x\n", msg->msg_type);
        nv_info = &msg->nv_item_info;
        switch (msg->msg_type) {
            case NV_TASK_MSG_WRITE2FILE:
                ret = nv_write2file_handle(msg);
                break;

            case NV_TASK_MSG_FLUSH:
                /* there is no actual nv operation, return NV_OK and notify
                           * NV writing process the result actually
                           */
                 ret = NV_OK;
                break;

            case NV_TASK_MSG_RESUEM:
                ret = nv_resume_ddr_from_img();
                break;

            case NV_TASK_MSG_RESUEM_ITEM:
                ret = nv_resume_item(NULL, nv_info->itemid, nv_info->modem_id);
                break;

            default:
                nv_printf("msg type invalid, msg type;0x%x\n", msg->msg_type);
                break;
        }

        nv_debug_printf("deal msg ok\n");
        if (ret) {
            nv_mntn_record("flush nv to file fail, msg type:0x%x errno:0x%x\n", msg->msg_type, ret);
        }

        if (msg->nv_msg_callback) {
            msg->nv_msg_callback(ret, msg->sn);
        }

        nv_put_cmd_req(msg);
        g_nv_ctrl.task_proc_count++;
        g_nv_ctrl.opState = NV_IDLE_STATE;
    }
}


u32 bsp_nvm_xml_decode(void)
{
    u32 ret = NV_ERROR;

    if(!nv_file_access(NV_XNV_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD1_PATH,NV_XNV_CARD1_MAP_PATH,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_XNV_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD2_PATH,NV_XNV_CARD2_MAP_PATH,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_XNV_CARD3_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD3_PATH, NV_XNV_CARD3_MAP_PATH, NV_USIMM_CARD_3);
        if(ret)
        {
            return ret;
        }
    }

    /*CUST XML 无对应MAP文件，传入空值即可*/
    if(!nv_file_access(NV_CUST_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD1_PATH,NULL,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_CUST_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD2_PATH,NULL,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_CUST_CARD3_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD3_PATH, NULL, NV_USIMM_CARD_3);
        if(ret)
        {
            return ret;
        }
    }

    return NV_OK;
}

/* added by yangzhi for muti-carrier, Begin:*/
/* added by yangzhi for muti-carrier, End! */



s32 bsp_nvm_restore_online_handle(NV_RESTORE_STATUS stuType)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        printf("Dload smem_data malloc fail!\n");
        return -1;
    }

    printf("smem_data->smem_online_upgrade_flag :0x%x\n", 
                smem_data->smem_online_upgrade_flag);
    printf("smem_data->smem_multiupg_flag :0x%x\n", 
                smem_data->smem_multiupg_flag);
    if(SMEM_ONNR_FLAG_NUM == smem_data->smem_online_upgrade_flag)
    {
        if(NV_RESTRORE_SUCCESS == stuType)
        {
            /*在线升级NV自动恢复阶段魔术字清零*/
            smem_data->smem_online_upgrade_flag = 0;

            /*组播升级不重启*/
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;
                printf("MULTI UPG success, do not reboot.\n");
            }
            else
            {
                smem_data->smem_switch_pcui_flag = 0;
                printf("MBB:Online Upgrade Sucessful,reboot.\n");
                /*单板重启进入正常模式*/
                bsp_drv_power_reboot();
            }
        }
        else
        {
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;
            }

            printf("MBB:Online Upgrade failed !\n");
        }
    }
    return 0;
}





u32 bsp_nvm_upgrade(void)
{
    u32 ret;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;


    nv_debug(NV_FUN_UPGRADE_PROC,0,0,0,0);
    nv_mntn_record("Balong nv upgrade start!\n");

    /*判断fastboot阶段xml 解析是否异常，若出现异常，则需要重新解析xml*/
    if(ddr_info->xml_dec_state != NV_XML_DEC_SUCC_STATE)
    {
        nv_mntn_record("fastboot xml decode fail ,need kernel decode again!\n");
        ret = bsp_nvm_xml_decode();
        if(ret)
        {
            nv_mntn_record("kernel xml decode failed 0x%x!\n", ret);
            nv_debug(NV_FUN_UPGRADE_PROC,1,ret,0,0);
            goto upgrade_fail_out;
        }
    }

    /* added by yangzhi for muti-carrier, Begin:*/


    /*恢复处理前先将某些nv刷新到default(定制需求)*/
    ret = nv_writepart_to_default();
    if (NV_OK != ret)
    {
        /* 保留log打印,但不返回错误 */
        printk(KERN_ERR "[nv upgrade]: write part to default error.\n");
    }

    /*升级恢复处理，烧片版本直接返回ok*/
    ret = nv_upgrade_revert_proc();
    if(ret)
    {
        nv_mntn_record("upgrade revert failed 0x%x!\n", ret);
        nv_debug(NV_FUN_UPGRADE_PROC,4,ret,0,0);
        goto upgrade_fail_out;
    }
    else
    {
        nv_mntn_record("upgrade revert success!\n");
    }
    /* added by yangzhi for muti-carrier, Begin:*/
    /* added by yangzhi for muti-carrier, End! */

    (void)nv_crc_make_ddr();
    nv_mntn_record("upgrade mkddr crc success!\n");

    /*将最新数据写入各个分区*/
    ret = nv_data_writeback();
    if(ret)
    {
        nv_mntn_record("upgrade writeback failed 0x%x!\n", ret);
        nv_debug(NV_FUN_UPGRADE_PROC,7,ret,0,0);
        goto upgrade_fail_out;
    }
    else
    {
        nv_mntn_record("upgrade writeback success!\n");
    }

    /*置升级包无效*/
    ret = (u32)nv_modify_upgrade_flag((bool)false);
    if(ret)
    {
        nv_mntn_record("upgrade set dload packet invalid failed 0x%x!\n", ret);
        nv_debug(NV_FUN_UPGRADE_PROC,8,ret,0,0);
        goto upgrade_fail_out;
    }
    ret = nv_file_update(NV_DLOAD_PATH);
    if(ret)
    {
        nv_mntn_record("upgrade nv_file_update failed 0x%x!\n", ret);
        nv_debug(NV_FUN_UPGRADE_PROC, 9,ret,0,0);
        goto upgrade_fail_out;
    }


    return NV_OK;
upgrade_fail_out:
    nv_mntn_record("\n%s\n",__func__);
    nv_help(NV_FUN_UPGRADE_PROC);
    return NV_ERROR;
}


u32 bsp_nvm_resume_bakup(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;

    if(true == nv_check_file_validity((s8 *)NV_BACK_PATH, (s8 *)NV_BACK_FLAG_PATH))
    {
        nv_mntn_record("load from %s\n",NV_BACK_PATH);
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_READ);
        if(!fp)
        {
            nv_debug(NV_FUN_MEM_INIT,5,0,0,0);
            goto load_err_proc;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen);
        nv_file_close(fp);
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,6,0,0,0);
            goto load_err_proc;
        }

        ret = nv_crc_check_ddr(NV_RESUME_NO);
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,8,ret,0,0);
            goto load_err_proc;
        }

        /*从备份区加载需要首先写入工作区*/
        ret = nv_img_flush_all();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,9,0,0,0);
            goto load_err_proc;
        }

        return NV_OK;
    }

load_err_proc:
    ret = nv_load_err_proc();
    if(ret)
    {
        nv_mntn_record("%s %d ,err revert proc ,ret :0x%x\n",__func__,__LINE__,ret);
        nv_help(NV_FUN_MEM_INIT);
    }

    return ret;
}



u32 bsp_nvm_reload(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;
    NV_SELF_CTRL_STRU self_ctrl = {};
    u32 try_times = STARTUP_TRY_TIMES;
    /*获取fastboot中异常信息共享内存地址*/
    power_info_s *power_info = (power_info_s *)SRAM_REBOOT_ADDR;

    /*工作分区数据存在，且无未写入完成的标志文件*/
    if( true == nv_check_file_validity((s8 *)NV_IMG_PATH, (s8 *)NV_IMG_FLAG_PATH))
    {
        nv_mntn_record("load from %s current slice:0x%x\n",NV_IMG_PATH, bsp_get_slice_value());

        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL, (u8*)&self_ctrl, sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            self_ctrl.ulResumeMode = NV_MODE_USER; /*如果NV读取失败，默认值设为NV_MODE_USER，满足如下的条件，重新load NV*/
            nv_mntn_record("%s %s :read nv NV_ID_DRV_SELF_CTRL fail\n",__DATE__,__TIME__);
        }
        /*如果发现反复重启的次数等于STARTUP_TRY_TIMES次，且是用户模式*/
        /*产线模式下会出现异常重启导致工作区的NV被nvbak重新load的场景*/
        if((NV_MODE_USER == self_ctrl.ulResumeMode) && (try_times == power_info->wdg_rst_cnt))
        {
            /*如果备份区没有数据不做任何操作*/
            if(!nv_file_access(NV_BACK_PATH,0))
            {
                /*记录发生异常的log信息到nvlog中*/
                nv_mntn_record("%s %s :The restart time has reached MAX:%d!\n",__DATE__,__TIME__,try_times);
                /*删除工作区域nv文件*/
                nv_file_remove((s8*)NV_IMG_PATH);
                /*启用备份区nv文件*/
                goto load_bak;
            }
        }

        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_READ);
        if(!fp)
        {
            nv_mntn_record("[%s]open %s fail\n", __FUNCTION__, NV_IMG_PATH);
            goto load_bak;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen);
        nv_file_close(fp);
        if(ret)
        {
            nv_mntn_record("[%s] read %s fail, ret = 0x%x\n", __FUNCTION__, NV_IMG_PATH, ret);
            goto load_bak;
        }

        /*reload在初始化过程中，需要从流程上保证此时不会有写NV的操作，从而做crc check的时候内存不会被改写
          带自动恢复的crc check，会在恢复过程中获取crc ipc semaphore，所以不能锁住内存做crc check*/
        ret = nv_crc_check_ddr(NV_RESUME_BAKUP);
        if(BSP_ERR_NV_CRC_RESUME_SUCC == ret)
        {
            ret = nv_img_flush_all();
            if(ret)
            {
                nv_mntn_record("nv resume write back failed 0x%x\n", ret);
                return ret;
            }
            nv_printf("img check crc error, but resume success, write back to img!\n");
        }
        else if(ret)
        {
            nv_mntn_record("nv image check crc failed %d...current slice:0x%x\n", ret, bsp_get_slice_value());

            /* 保存错误镜像，然后从bakup分区恢复 */
            (void)nv_debug_store_file(NV_IMG_PATH);
            if(nv_debug_is_resume_bakup())
            {
                ret = bsp_nvm_resume_bakup();
                if(ret)
                {
                    nv_mntn_record("nv resume bakup failed %d...current slice:0x%s \n", ret, bsp_get_slice_value());
                }
            }
            else
            {
                nv_mntn_record("config don't resume bakup...slice:0x%x \n",bsp_get_slice_value());
            }

            /* 复位系统 */
            if(nv_debug_is_reset())
            {
                system_error(DRV_ERRNO_NV_CRC_ERR, NV_FUN_MEM_INIT, 3, NULL, 0);
            }
        }

        /*nvimg分区加载成功,检查是否需要备份
                无论是否成功单板都正常运行*/
        (void)nv_rebak_save_data();

        return ret;
    }

load_bak:

    return bsp_nvm_resume_bakup();
}

u32 clean_mmi_nv_flag(void)
{

    u32 ret = NV_OK;
    /*0xFF,表示无效值*/
    u32 real_factory_mode = 0xFF;
    u8  factory_info[FACTORY_INFO_LEN] = {0};

    memset(factory_info, 0x00, FACTORY_INFO_LEN);
    /*读取当前单板所升级的软件为烧片软件还是升级软件*/
    ret = bsp_nvm_read(NV_ID_MSP_SW_VER_FLAG,(u8*)(&real_factory_mode),sizeof(u32));
    if(ret)
    {
        nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_SW_VER_FLAG,ret);
        return ret;
    }
    /*烧片软件一键升级返工，清空MMI标记*/
    if(0 == real_factory_mode)
    {
        /*读取NV114中的值*/
        ret = bsp_nvm_read(NV_ID_MSP_FACTORY_INFO,(u8*)(&factory_info),(sizeof(u8) * FACTORY_INFO_LEN));
        if(ret)
        {
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FACTORY_INFO,ret);
        }
        else
        {
            /*清空NV114中MMI比较bit位*/
            memcpy(&factory_info[MMI_TEST_FLAG_OFFSET], "0000", MMI_TEST_FLAG_LEN);
            /*写入114清空MMI结果后的值*/
            ret = bsp_nvm_write(NV_ID_MSP_FACTORY_INFO,(u8*)(&factory_info),(sizeof(u8) * FACTORY_INFO_LEN));
            if(ret)
            {
                nv_printf("write 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FACTORY_INFO,ret);
            }
            else
            {
                ret = NV_OK;
            }
        }
    }
    /*出错的时候直接返回非0值，正确的时候返回NV_OK*/
    return ret;
}

/*****************************************************************************
 函 数 名  : bsp_nvm_write_buf_init
 功能描述  : 初始化写入NV时使用的buf和信号量
 输入参数  :
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
u32 bsp_nvm_buf_init(void)
{
    /*create sem*/
    osl_sem_init(1,&g_nv_ctrl.nv_list_sem);
    INIT_LIST_HEAD(&g_nv_ctrl.nv_list);

    return NV_OK;
}

s32 bsp_nvm_kernel_init(void)
{
    u32 ret = NV_ERROR;
    u32 clean_mmi_flag = 0;
    huawei_smem_info *smem_data = NULL;
    u32 nvsys_boot_flag = 0;
    u32 nvback_boot_flag = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    nv_debug(NV_FUN_KERNEL_INIT,0,0,0,0);

    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printf("nv_file_init: smem_data is NULL \n");
        return -1;  

    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽nv模块的启动*/
        printf("entry update not init nvim !\n");
        return -1;  
    }


    /*sem & lock init*/
    spin_lock_init(&g_nv_ctrl.spinlock);
    osl_sem_init(0,&g_nv_ctrl.task_sem);
    osl_sem_init(0,&g_nv_ctrl.suspend_sem);
    osl_sem_init(1,&g_nv_ctrl.rw_sem);
    osl_sem_init(0,&g_nv_ctrl.cc_sem);
    wake_lock_init(&g_nv_ctrl.wake_lock,WAKE_LOCK_SUSPEND,"nv_wakelock");
    g_nv_ctrl.shared_addr = (struct nv_global_ddr_info_stru *)NV_GLOBAL_INFO_ADDR;

    nv_mntn_record("Balong nv init  start! %s %s\n",__DATE__,__TIME__);

    (void)nv_debug_init();

    /* check nv file */
    ret = (u32)nv_img_boot_check("/mnvm2:0");
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,1,ret,0,0);
        goto out;
    }
    
    /*file info init*/
    ret = nv_file_init();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,2,ret,0,0);
        goto out;
    }

    /* 标识清除前先保存下 */
    nvsys_boot_flag = ddr_info->nvsys_boot_state;
    nvback_boot_flag = ddr_info->nvback_boot_state;
    if(ddr_info->acore_init_state != NV_BOOT_INIT_OK)
    {
        nv_mntn_record("fast boot nv init fail !\n");
        nv_show_fastboot_err();
        /* coverity[secure_coding] */
        memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
    }

    ddr_info->acore_init_state = NV_KERNEL_INIT_DOING;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    ret = (u32)bsp_ipc_sem_create((u32)IPC_SEM_NV_CRC);
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT, 3, ret, 0, 0);
        goto out;
    }

    if((ddr_info->mem_file_type == NV_MEM_DLOAD) &&
       (!nv_file_access((s8*)NV_DLOAD_PATH,0)) &&/*升级分区存在数据*/
       (true == nv_get_upgrade_flag())/*升级文件有效*/
       )
    {
        clean_mmi_flag = 1;
        /*判断升级方式，如果是SD卡或在线升级则不进行库仑计固件的升级*/
        if ((SMEM_SDNR_FLAG_NUM != smem_data->smem_online_upgrade_flag) &&
            (SMEM_ONNR_FLAG_NUM != smem_data->smem_online_upgrade_flag))
        {
            /*update coul firmware*/
            bq27510_coul_firmware_update_init();
        }
        ret = bsp_nvm_upgrade();
        if(ret)
        {
            nv_mntn_record("upgrade faided! 0x%x\n", ret);
            nv_debug(NV_FUN_KERNEL_INIT,4,ret,0,0);
            goto out;
        }
        else
        {
            nv_mntn_record("upgrade success!\n");
        }

        /*读取NV自管理配置*/
        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL,(u8*)(&(g_nv_ctrl.nv_self_ctrl)),(u32)sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            g_nv_ctrl.nv_self_ctrl.ulResumeMode = NV_MODE_USER;
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_DRV_SELF_CTRL,ret);
        }
    }
    else
    {
        /*读取NV自管理配置*/
        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL,(u8*)(&(g_nv_ctrl.nv_self_ctrl)), (u32)sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            g_nv_ctrl.nv_self_ctrl.ulResumeMode = NV_MODE_USER;
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_DRV_SELF_CTRL,ret);
        }

        /*重新加载最新数据*/
        ret = bsp_nvm_reload();
        if(ret)
        {
            nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
            goto out;
        }
        else
        {
            nv_printf("reload success!\n");
            /* reload success查看是否需要修复nvsys和nvback
                共享内存值可能已修复,因此两者都要判断 */
            if ((NV_INIT_OK != nvsys_boot_flag)
                && (NV_INIT_OK != ddr_info->nvsys_boot_state))
            {
                /* nvsys加载失败修复,无论结果均执行且清空标识 */
                (void)bsp_nvm_flushSys();
                nvsys_boot_flag = NV_INIT_OK;
                ddr_info->nvsys_boot_state = NV_INIT_OK;
                /* 在nv log文件中记录*/
                nv_mntn_record("[nv init]:repair nvsys data ok.\n");
            }
            if ((NV_INIT_OK != nvback_boot_flag)
                && (NV_INIT_OK != ddr_info->nvback_boot_state))
            {
                /* nvsys加载失败修复,无论结果均执行且清空标识 */
                (void)bsp_nvm_backup(NV_FLAG_NO_CRC);
                nvback_boot_flag = NV_INIT_OK;
                ddr_info->nvback_boot_state = NV_INIT_OK;
                /* 在nv log文件中记录*/
                nv_mntn_record("[nv init]:repair nvback data ok.\n");
            }
        }
    }

    /*初始化双核使用的链表*/
    nv_flushListInit();

    ret = bsp_nvm_buf_init();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,10,ret,0,0);
        goto out;
    }

    /*置初始化状态为OK*/
    ddr_info->acore_init_state = NV_INIT_OK;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    /*保证各分区数据正常写入*/
    nv_file_flag_check();

    INIT_LIST_HEAD(&g_nv_ctrl.stList);
/*  处理单板的CAT等级，如果处理失败，则不再做处理  */
    ret = (u32)osl_task_init("drv_nv",15,1024,(OSL_TASK_FUNC)bsp_nvm_icc_task,NULL,(OSL_TASK_ID*)&g_nv_ctrl.task_id);
    if(ret)
    {
        nv_mntn_record("[%s]:nv task init err! ret :0x%x\n",__func__,ret);
        goto out;
    }
    ret = nv_icc_chan_init(NV_RECV_FUNC_AC);
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
        goto out;
    }

    ret = nv_msg_init();
    if (ret) {
        nv_debug(NV_FUN_KERNEL_INIT,6,ret,0,0);
        goto out;
    }

    /*to do:nv id use macro define*/
    ret = bsp_nvm_read(NV_ID_MSP_FLASH_LESS_MID_THRED,(u8*)(&(g_nv_ctrl.mid_prio)),(u32)sizeof(u32));
    if(ret)
    {
        g_nv_ctrl.mid_prio = 20;
        nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FLASH_LESS_MID_THRED,ret);
    }

    nvchar_init();
    /*在一键升级备份恢复NV的时候对烧片版本的MMI标记进行清0操作*/
    if(1 == clean_mmi_flag)
    {
        ret = clean_mmi_nv_flag();
        if(ret)
        {
            /*5为debug级别*/
            nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
            goto out;
        }
    }

    nv_mntn_record("Balong nv init ok!\n");

//    nv_show_ddr_info();

    /*nv初始化成功后将软件版版本号写入oeminfo 分区
    先读出判断是否一致,不一致写入
    此处不判断返回值,是否写入成功均向下执行*/
    (void)huawei_dload_set_swver_to_oeminfo();


    ret = bsp_nvm_restore_online_handle(NV_RESTRORE_SUCCESS);
    if(ret)
    {
        return ret;
    }
    return NV_OK;

out:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    ddr_info->acore_init_state = NV_INIT_FAIL;
    nv_help(NV_FUN_KERNEL_INIT);
    nv_show_ddr_info();
    bsp_nvm_restore_online_handle(NV_RESTRORE_FAIL);
    return -1;
}

static void bsp_nvm_exit(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    /* coverity[self_assign] */
    ddr_info = ddr_info;

    /*关机写数据*/
    (void)bsp_nvm_flush();
    /*清除标志*/
    /* coverity[secure_coding] */
    memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
    
}

void modem_nv_delay(void)
{
    u32  blk_size;
    char *blk_label;
    int i, ret = -1;

    /*最长等待时长10s*/
    for(i=0;i<10;i++)
    {
        nv_printf("modem nv wait for nv block device %d s\n",i);

        blk_label = (char*)NV_BACK_SEC_NAME;
        ret = bsp_blk_size(blk_label, &blk_size);
        if (ret) {
            nv_taskdelay(1000);
            nv_printf("get block device %s fail\n", blk_label);
            continue;
        }

        blk_label = (char*)NV_DLOAD_SEC_NAME;
        ret = bsp_blk_size(blk_label, &blk_size);
        if (ret) {
            nv_taskdelay(1000);
            nv_printf("get block device %s fail\n", blk_label);
            continue;
        }

        blk_label = (char*)NV_SYS_SEC_NAME;
        ret = bsp_blk_size(blk_label, &blk_size);
        if (ret) {
            nv_taskdelay(1000);
            nv_printf("get block device %s fail\n", blk_label);
            continue;
        }

        blk_label = (char*)NV_DEF_SEC_NAME;
        ret = bsp_blk_size(blk_label, &blk_size);
        if (ret) {
            nv_taskdelay(1000);
            nv_printf("get block device %s fail\n", blk_label);
            continue;
        }
        return;
    }
}

/*lint -save -e715*//*715表示入参dev未使用*/
static int  modem_nv_probe(struct platform_device *dev)
{
    int ret;

    g_nv_ctrl.pmState = NV_WAKEUP_STATE;
    g_nv_ctrl.opState = NV_IDLE_STATE;

    modem_nv_delay();

    /* coverity[Event check_return] *//* coverity[Event unchecked_value] */
    if(mdrv_file_access("/modem_log/drv/nv",0))
        (void)mdrv_file_mkdir("/modem_log/drv/nv");


    ret = bsp_nvm_kernel_init();

    ret |= modemNv_ProcInit();

    return ret;
}

#define NV_SHUTDOWN_STATE   NV_BOOT_INIT_OK
static void modem_nv_shutdown(struct platform_device *dev)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    printk("%s shutdown start %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n",__func__);

    /*read only*/
    ddr_info->acore_init_state = NV_SHUTDOWN_STATE;
    ddr_info->ccore_init_state = NV_SHUTDOWN_STATE;
    ddr_info->mcore_init_state = NV_SHUTDOWN_STATE;
}
/*lint -restore*/

/*lint -save -e785*//*785表示对结构体初始化的不完全modem_nv_pm_ops和modem_nv_drv modem_nv_device*/
/*lint -save -e715*//*715表示入参dev未使用*/
static s32 modem_nv_suspend(struct device *dev)
{
    static int count = 0;
    if(g_nv_ctrl.opState == NV_OPS_STATE)
    {
        printk(KERN_ERR"%s Modem nv in doing !\n",__func__);
        return -1;
    }
    g_nv_ctrl.pmState = NV_SLEEP_STATE;
    printk(KERN_ERR"Modem nv enter suspend! %d times\n",++count);
    return 0;
}
static s32 modem_nv_resume(struct device *dev)
{
    static int count = 0;
    
    g_nv_ctrl.pmState = NV_WAKEUP_STATE;
    if(NV_OPS_STATE== g_nv_ctrl.opState)
    {
        printk(KERN_ERR"%s need to enter task proc!\n",__func__);
        osl_sem_up(&g_nv_ctrl.suspend_sem);
    }
    printk(KERN_ERR"Modem nv enter resume! %d times\n",++count);
    return 0;
}
/*lint -restore*/
static const struct dev_pm_ops modem_nv_pm_ops ={
    .suspend = modem_nv_suspend,
    .resume  = modem_nv_resume,
};

#define MODEM_NV_PM_OPS (&modem_nv_pm_ops)

static struct platform_driver modem_nv_drv = {
    .shutdown   = modem_nv_shutdown,
    .driver     = {
        .name     = "modem_nv",
        .owner    = (struct module *)(unsigned long)THIS_MODULE,
        .pm       = MODEM_NV_PM_OPS,
    },
};


static struct platform_device modem_nv_device = {
    .name = "modem_nv",
    .id = 0,
    .dev = {
    .init_name = "modem_nv",
    },
};
/*lint -restore*/

int  modem_nv_init(void)
{
    struct platform_device *dev = NULL;
    int ret;
    if(0 == g_nv_ctrl.initStatus)
    {
        g_nv_ctrl.initStatus = 1;
    }
    else
    {
        show_stack(current, NULL);
    }

    ret = modem_nv_probe(dev);

    return ret;
}
/*仅用于初始化nv设备*/
int nv_init_dev(void)
{
    u32 ret;
    ret = (u32)platform_device_register(&modem_nv_device);
    if(ret)
    {
        printk(KERN_ERR"platform_device_register modem_nv_device fail !\n");
        return -1;
    }

    ret = (u32)platform_driver_register(&modem_nv_drv);
    if(ret)
    {
        printk(KERN_ERR"platform_device_register modem_nv_drv fail !\n");
        platform_device_unregister(&modem_nv_device);
        return -1;
    }
    nv_printf("init modem nv dev ok\n");
    return NV_OK;
}
void  modem_nv_exit(void)
{
    bsp_nvm_exit();
    platform_device_unregister(&modem_nv_device);
    platform_driver_unregister(&modem_nv_drv);
}

device_initcall(nv_init_dev);
module_init(modem_nv_init);
module_exit(modem_nv_exit);

void bsp_nvm_make_pclint_happy(void)
{
    (void)__initcall_nv_init_dev6();
}
EXPORT_SYMBOL(bsp_nvm_backup);
EXPORT_SYMBOL(bsp_nvm_dcread);
EXPORT_SYMBOL(bsp_nvm_kernel_init);
EXPORT_SYMBOL(bsp_nvm_update_default);
EXPORT_SYMBOL(bsp_nvm_revert_default);
EXPORT_SYMBOL(bsp_nvm_dcreadpart);
EXPORT_SYMBOL(bsp_nvm_get_len);
EXPORT_SYMBOL(bsp_nvm_dcwrite);
EXPORT_SYMBOL(bsp_nvm_flush);
EXPORT_SYMBOL(bsp_nvm_reload);
EXPORT_SYMBOL(nvm_read_rand);
EXPORT_SYMBOL(nvm_read_randex);
EXPORT_SYMBOL(bsp_nvm_dcread_direct);
EXPORT_SYMBOL(bsp_nvm_dcwrite_direct);
EXPORT_SYMBOL(bsp_nvm_auth_dcread);
EXPORT_SYMBOL(bsp_nvm_auth_dcwrite);
EXPORT_SYMBOL(bsp_nvm_dcwritepart);
EXPORT_SYMBOL(bsp_nvm_get_nvidlist);
EXPORT_SYMBOL(bsp_nvm_authgetlen);
EXPORT_SYMBOL(bsp_nvm_xml_decode);

/*lint -restore*/




