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

#include "of.h"
#include <linux/slab.h>
#include "amon_balong.h"

AMON_PORT_CFG g_amon_port_cfg = {0,NULL};


s32 amon_parse_port_cfg(void)
{
    char* name = "hisilicon,amon_soc2_port_cfg";
    struct device_node *dev_node = NULL;
    struct device_node *child = NULL;
    char* port_name[] = {"port0","port1","port2","port3","port4","port5","port6","port7","port8"};
    u32 port_num = 0;
    u32 i = 0;
    u32 port_id = 0;
    u32 j = 0;
    char* temp = NULL;
    u32 len = 0;

    dev_node = of_find_compatible_node(NULL,NULL,name);
    if(!dev_node)
    {
        amon_error("get amon port cfg node failed!\n");
        return BSP_ERROR;
    }
    if(of_property_read_u32(dev_node, "port_num", &port_num))
    {
        amon_error("port_num failed!\n");
        return BSP_ERROR;
    }
    g_amon_port_cfg.port_num = port_num;
    
    len = port_num * sizeof(AMON_PORT_INFO);
    g_amon_port_cfg.port_info = kmalloc(len,GFP_KERNEL);
    if(g_amon_port_cfg.port_info == NULL)
    {
        amon_error("g_amon_port_cfg.port_info failed!\n");
        return BSP_ERROR;
    }
    memset(g_amon_port_cfg.port_info,0,len);

    for_each_child_of_node(dev_node,child)
    {

         if(of_property_read_u32(child, "capt_flag", &port_num))
         {
             amon_error("port_num failed!\n");
             return BSP_ERROR;
         }
         g_amon_port_cfg.port_info[i].capt_flag= port_num;

         if(of_property_read_u32(child, "port_id", &port_id))
         {
            amon_error("port_id fail\n");
            return BSP_ERROR;
         }
         g_amon_port_cfg.port_info[i].port_id = port_id;
         memcpy(g_amon_port_cfg.port_info[i].port_name,port_name[i],strlen(port_name[i]));
         
         if(of_property_read_u32(child, "master_sum", &port_id))
         {
            amon_error("master_sum fail\n");

            return BSP_ERROR;
         }
         g_amon_port_cfg.port_info[i].master_num = port_id;
         len = g_amon_port_cfg.port_info[i].master_num * sizeof(AMON_MASTER_INFO);
         g_amon_port_cfg.port_info[i].master_info = kmalloc(len,GFP_KERNEL);
         if(g_amon_port_cfg.port_info[i].master_info == NULL)
         {
             amon_error("g_amon_port_cfg.port_info[i].master_info failed!\n");
             kfree(g_amon_port_cfg.port_info);
             return BSP_ERROR;
         }
         memset(g_amon_port_cfg.port_info[i].master_info,0,len);
 
         for(j = 0;j < g_amon_port_cfg.port_info[i].master_num;j++)
         {
 
             if(of_property_read_string_index(child, (const char*)"master_name", j,(const char**)&temp))
             {
                 amon_error("get master_name fail\n");
                 kfree(g_amon_port_cfg.port_info);
                 kfree(g_amon_port_cfg.port_info[i].master_info);
                 return BSP_ERROR;
             }
             memset(g_amon_port_cfg.port_info[i].master_info[j].master_name,0,16);
             memcpy(g_amon_port_cfg.port_info[i].master_info[j].master_name,temp,strlen(temp)<16 ? strlen(temp) : 15);
 
        
             if(of_property_read_u32_index(child, "master_id" ,j, &port_id))
             {
                 amon_error("get master_id fail\n");
                 kfree(g_amon_port_cfg.port_info);
                 kfree(g_amon_port_cfg.port_info[i].master_info);
                 return BSP_ERROR;
             }
             g_amon_port_cfg.port_info[i].master_info[j].master_id = port_id;
 
             if(of_property_read_u32_index(child, "cnt_reg_cfg" ,j, &port_id))
             {
                 amon_error("get cnt_reg_cfg fail\n");
                 kfree(g_amon_port_cfg.port_info);
                 kfree(g_amon_port_cfg.port_info[i].master_info);
                 return BSP_ERROR;
             }
             g_amon_port_cfg.port_info[i].master_info[j].cnt_reg_cfg = port_id;

             if(of_property_read_u32_index(child, "capt_reg_cfg" ,j, &port_id))
             {
                 amon_error("get capt_reg_cfg fail\n");
                 kfree(g_amon_port_cfg.port_info);
                 kfree(g_amon_port_cfg.port_info[i].master_info);
                 return BSP_ERROR;
             }
             g_amon_port_cfg.port_info[i].master_info[j].capt_reg_cfg = port_id;
            
         }
         i++;
     }
 
    return BSP_OK;


}

void amon_show_port_info(void)
{
    u32 i =0;
    u32 j = 0;
    char* capt_info[2]={"cnt only","cnt & capt"};
    if(g_amon_port_cfg.port_num == 0)
    {
        amon_error("no port info\n");
        return;
    }
    
    for(i = 0;i < g_amon_port_cfg.port_num;i++)
    {
        amon_error("**************************%s info****************************\n",g_amon_port_cfg.port_info[i].port_name);
        amon_error("port id  : %x\n",g_amon_port_cfg.port_info[i].port_id);
        amon_error("port name: %x\n",g_amon_port_cfg.port_info[i].port_name);
        amon_error("master num: %x\n",g_amon_port_cfg.port_info[i].master_num);
        amon_error("fuction support: %s\n",capt_info[g_amon_port_cfg.port_info[i].capt_flag]);

        for(j = 0; j < g_amon_port_cfg.port_info[i].master_num;j++)
        {
             amon_error("master id: %x,maser name : %s\n",g_amon_port_cfg.port_info[i].master_info[j].master_id,g_amon_port_cfg.port_info[i].master_info[j].master_name);
        }
        amon_error("*************************************************************\n\n");
    }
}


s32 amon_get_cnt_cfg(u32 port ,u32 master_id,u32* cfg_id)
{
    u32 i =0;
    u32 j = 0;

    if(g_amon_port_cfg.port_num == 0 || cfg_id == NULL)
    {
        amon_error("no port info\n");
        return BSP_ERROR;
    }
    
    for(i = 0;i < g_amon_port_cfg.port_num;i++)
    {
        if(i == port)
        {
            for(j = 0;j < g_amon_port_cfg.port_info[i].master_num;j++)
            {
                if(g_amon_port_cfg.port_info[i].master_info[j].master_id == master_id )
                {
                    *cfg_id = g_amon_port_cfg.port_info[i].master_info[j].cnt_reg_cfg;
                    return BSP_OK;
                }
            }
        }
    }
    return BSP_ERROR;
}

s32 amon_get_capt_cfg(u32 port ,u32 master_id,u32* cfg_id)
{
    u32 i =0;
    u32 j = 0;
    if(g_amon_port_cfg.port_num == 0 || cfg_id == NULL)
    {
        amon_error("no port info\n");
        return BSP_ERROR;
    }
    for(i = 0;i < g_amon_port_cfg.port_num;i++)
    {
        if(i == port)
        {
            for(j = 0;j < g_amon_port_cfg.port_info[i].master_num;j++)
            {
                if(g_amon_port_cfg.port_info[i].master_info[j].master_id == master_id )
                {
                    *cfg_id = g_amon_port_cfg.port_info[i].master_info[j].capt_reg_cfg;
                    return BSP_OK;
                }
            }
        }
    }
    return BSP_ERROR;
}

