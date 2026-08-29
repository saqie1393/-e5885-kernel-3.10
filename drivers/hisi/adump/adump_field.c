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

#include <product_config.h>
#include "bsp_dump_mem.h"
#include "adump_field.h"
#include "adump_area.h"
#include "adump_debug.h"

struct adump_field_ctrl_info_s   g_st_adump_field_ctrl;
EXPORT_SYMBOL(g_st_adump_field_ctrl);

u32 g_arm_ttbr0;
u32 g_arm_ttbr1;

#define TTBR_READ(OP2, VAL) do { \
       asm volatile("mrc p15, 0, %0, c2,c0, " #OP2 : "=r" (VAL));\
} while (0)

/* 验证field_id是否合法，异常id返回BSP_ERROR，正常id返回area id */
static u32 adump_get_areaid_by_fieldid(u32 field_id)
{
    s32 ret;

    if((field_id>=DUMP_MODEMAP_FIELD_BEGIN) && (field_id<DUMP_MODEMAP_FIELD_END))
    {
        ret = DUMP_AREA_MDMAP;
    }
    else if((field_id>=DUMP_CP_FIELD_BEGIN) && (field_id<DUMP_CP_FIELD_END))
    {
        ret = DUMP_AREA_CP;
    }
    else if((field_id>=DUMP_KERNEL_FIELD_BEGIN) && (field_id<DUMP_KERNEL_FIELD_END)) /*ap子系统范围*/
    {
        ret = DUMP_AREA_AP;
    }
    else if((field_id>=DUMP_M3_FIELD_BEGIN) && (field_id<DUMP_M3_FIELD_END))
    {
        ret = DUMP_AREA_LPM3;
    }
    else
    {
        adump_err("invalid field id 0x%x\n", field_id);
        ret = (u32)ADUMP_ERR;
    }

    return ret;
}



/* get field address by mod id, return 0 if failed */
/* get all cpu field address, not just this cpu */
u8 * bsp_adump_get_field_addr(u32 field_id)
{
    u32 i;
    u8 * addr = 0;
    dump_area_t * parea;
    s32    area_id;
    struct dump_area_mntn_addr_info_s area_info;

    /*根据field查找对应的area id*/
    area_id = adump_get_areaid_by_fieldid(field_id);
    if(area_id >= DUMP_AREA_BUTT)
    {
        return NULL;
    }
    /*根据area id查找对应area 基地址*/
    /* coverity[secure_coding] */
    memset(&area_info,0,sizeof(area_info));
    if(adump_get_area_info(area_id,&area_info))
    {
        return NULL;
    }
    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        return NULL;
    }
    parea = (dump_area_t *)area_info.vaddr;
    /* search field addr by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            addr = (u8 *)parea + parea->fields[i].offset_addr;
            return addr;
        }
    }

    return NULL;
}
EXPORT_SYMBOL(bsp_adump_get_field_addr);
u8 * bsp_adump_get_field_phy_addr(u32 field_id)
{
    u8 * addr = 0;

    addr = bsp_adump_get_field_addr(field_id);
    if(NULL == addr)
    {
	    return NULL;
    }
    /* coverity[overflow] */
    return (u8 *)((unsigned long)addr - (unsigned long)g_st_adump_field_ctrl.virt_area_addr+ (unsigned long)g_st_adump_field_ctrl.phy_area_addr);
}
EXPORT_SYMBOL(bsp_adump_get_field_phy_addr);
u8 * bsp_adump_get_field_map(u32 field_id)
{
    u32 i;
    u32 areaid;
    dump_area_t * parea;
    struct dump_area_mntn_addr_info_s area_info;


    areaid = adump_get_areaid_by_fieldid(field_id);
    if(areaid > DUMP_AREA_BUTT)
    {
        return NULL;
    }

    if(adump_get_area_info(areaid,&area_info))
    {
        return NULL;
    }
    parea = (dump_area_t*)area_info.vaddr;

    /* search field map by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            return (u8 * )&parea->fields[i];
        }
    }

    return NULL;
}
EXPORT_SYMBOL(bsp_adump_get_field_map);

/* register field in current core area
 * 1. 不带地址注册，传入参数时virt_addr,phy_addr必须传0，成功返回dump注册地址
 * 2. 自带地址注册，传入参数时phy_addr为自带物理地址，virt_addr为虚拟地址，同时在dump内存中分配相同大小内存，成功返回邋virt_addr
 * PS:
 * 1. 两种注册方式，都将在dump划分内存，对于自带地址的注册方式，在系统异常时，由dump模块做数据拷贝
 * 2. 每个注册区域需要由使用者传入对应的版本号，高8位为主版本号，低8位为次版本号
 */
u8 * bsp_adump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version)
{
    dump_area_t* area_info;
    u8*         ret;
    u32         index;
    int         i;
    unsigned long flags;
    struct dump_field_self_info_s*  self;

    if(!g_st_adump_field_ctrl.ulInitflag)
        return NULL;

    /*注册的不是当前子系统范围的field*/
    if((field_id < CURRENT_FIELD_ID_START)||(field_id > CURRENT_FIELD_ID_END))
    {
        return NULL;
    }

    if(!name)
    {
        return NULL;
    }

    /*剩余空间不足*/
    spin_lock_irqsave(&g_st_adump_field_ctrl.lock, flags);
    area_info = (dump_area_t*)g_st_adump_field_ctrl.virt_area_addr;
    /*注册field个数超出最大范围*/
    index = area_info->area_head.field_num;
    /*检查是否有重复注册*/
    for(i=0;i<index ;i++)
    {
        if(area_info->fields[i].field_id == field_id)
        {
            spin_unlock_irqrestore(&g_st_adump_field_ctrl.lock, flags);
            adump_err("0x%x register twice!\n",field_id);
            return (u8*)area_info + area_info->fields[i].offset_addr;
        }
    }

    if(index >= DUMP_FIELD_MAX_NUM)
    {
        spin_unlock_irqrestore(&g_st_adump_field_ctrl.lock, flags);
        adump_err("filed num is max,do not allow to register!\n");
        return NULL;
    }
    if(g_st_adump_field_ctrl.free_length < length)
    {
        spin_unlock_irqrestore(&g_st_adump_field_ctrl.lock, flags);
        adump_err("space not enough ,0x%x,length:0x%x,free length :0x%x\n",field_id,length,g_st_adump_field_ctrl.free_length);
        return NULL;
    }

    ret = (u8*)area_info + g_st_adump_field_ctrl.free_offset;
    DUMP_FIXED_FIELD((void*)&(area_info->fields[index]),field_id,name,g_st_adump_field_ctrl.free_offset,length);
    area_info->area_head.field_num ++;

    g_st_adump_field_ctrl.free_length -= length;
    g_st_adump_field_ctrl.free_offset += length;
    g_st_adump_field_ctrl.field_num ++;

    area_info->area_head.field_num = g_st_adump_field_ctrl.field_num;

    /*自带地址注册*/
    if( virt_addr || phy_addr )
    {
        self = (struct dump_field_self_info_s*)ret;
        self->magic_num = DUMP_FIELD_SELF_MAGICNUM;
        self->phy_addr  = (u32)phy_addr;
        self->virt_addr  = virt_addr;
        self->reserved  = 0;
    }
    spin_unlock_irqrestore(&g_st_adump_field_ctrl.lock, flags);

    return ret;
}
EXPORT_SYMBOL(bsp_adump_register_field);
void adump_save_self_addr(void)
{
    dump_area_t* area_info;
    u32         index;
    int         i;
    unsigned long flags;
    struct dump_field_self_info_s*  self;
    void* self_virt;


    spin_lock_irqsave(&g_st_adump_field_ctrl.lock, flags);

    area_info = (dump_area_t*)g_st_adump_field_ctrl.virt_area_addr;
    index     = g_st_adump_field_ctrl.field_num;

    for(i=0;i<index;i++)
    {
        self = (struct dump_field_self_info_s*)((u8*)area_info+area_info->fields[i].offset_addr);
        if((DUMP_FIELD_SELF_MAGICNUM == self->magic_num)&&(self->virt_addr))
        {
            self_virt = self->virt_addr;
            memcpy(self,self_virt,area_info->fields[i].length);
        }
    }
    spin_unlock_irqrestore(&g_st_adump_field_ctrl.lock, flags);

    TTBR_READ(0,g_arm_ttbr0);
    TTBR_READ(1,g_arm_ttbr1);
    return ;
}
EXPORT_SYMBOL(adump_save_self_addr);

s32 adump_field_init(void)
{
    struct dump_area_mntn_addr_info_s area_info;

    spin_lock_init(&g_st_adump_field_ctrl.lock);
    /* coverity[secure_coding] */
    memset(&area_info,0,sizeof(struct dump_area_mntn_addr_info_s));

    if(adump_get_area_info(CURRENT_AREA,&area_info))
    {
        return -1;
    }

    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        return -1;
    }

    g_st_adump_field_ctrl.virt_area_addr = area_info.vaddr;
    g_st_adump_field_ctrl.phy_area_addr  = area_info.paddr;
    g_st_adump_field_ctrl.total_length   = area_info.len;
    /* coverity[secure_coding] */
    memset(g_st_adump_field_ctrl.virt_area_addr,0,g_st_adump_field_ctrl.total_length);
    g_st_adump_field_ctrl.free_offset    = sizeof(dump_area_t);
    g_st_adump_field_ctrl.free_length    = g_st_adump_field_ctrl.total_length - g_st_adump_field_ctrl.free_offset;

    g_st_adump_field_ctrl.virt_area_addr->area_head.magic_num = DUMP_AREA_MAGICNUM;
    memcpy(g_st_adump_field_ctrl.virt_area_addr->area_head.name,CURRENT_AREA_NAME,strlen(CURRENT_AREA_NAME));
    g_st_adump_field_ctrl.virt_area_addr->area_head.field_num = 0;

    TTBR_READ(0,g_arm_ttbr0);
    TTBR_READ(1,g_arm_ttbr1);

    g_st_adump_field_ctrl.ulInitflag = true;

    return 0;
}

s32 __init adump_mem_init(void)
{
    (void)adump_area_init();

    (void)adump_field_init();

    adump_err("ok!\n");
    return ADUMP_OK;
}

core_initcall_sync(adump_mem_init);



