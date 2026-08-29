


#include <product_config.h>

#include "bsp_dump_mem.h"
#include "bsp_adump.h"
#include "adump_area.h"

#include "adump_debug.h"

struct adump_global_area_ctrl_s g_st_adump_area_ctrl;

EXPORT_SYMBOL(g_st_adump_area_ctrl);
u32 g_kernel_phy_addr = DDR_ACORE_ADDR;

s32 adump_get_area_info(DUMP_AREA_ID areaid,struct dump_area_mntn_addr_info_s* area_info)
{
    if(!g_st_adump_area_ctrl.ulInitFlag){
        adump_err("not initial!\n");
        return ADUMP_ERR;
    }

    if((areaid >= DUMP_AREA_BUTT)||(NULL == area_info))
    {
        adump_err("invalid parameter!\n");
        return ADUMP_ERR;
    }

    area_info->vaddr    = (void*)g_st_adump_area_ctrl.virt_addr+g_st_adump_area_ctrl.virt_addr->area_info[areaid].offset;
    area_info->paddr    = (void*)(g_st_adump_area_ctrl.phy_addr+g_st_adump_area_ctrl.virt_addr->area_info[areaid].offset);
    area_info->len      = g_st_adump_area_ctrl.virt_addr->area_info[areaid].length;

    return ADUMP_OK;
}
EXPORT_SYMBOL(adump_get_area_info);

s32 adump_area_init(void)
{
    dump_load_info_t * dump_load;
    if(g_st_adump_area_ctrl.ulInitFlag)
    {
        return ADUMP_OK;
    }
    g_st_adump_area_ctrl.phy_addr = (void*)MNTN_BASE_ADDR;
    g_st_adump_area_ctrl.length   = MNTN_BASE_SIZE;
    g_st_adump_area_ctrl.virt_addr= (struct dump_global_struct_s*)ioremap_wc((phys_addr_t)g_st_adump_area_ctrl.phy_addr,g_st_adump_area_ctrl.length);
    if(NULL == g_st_adump_area_ctrl.virt_addr)
    {
        adump_err(" ioremap fail !\n");
        return ADUMP_ERR;
    }

    /*判断rdr头部标志是否存在，如果不存在则需要重新初始化*/
    if(g_st_adump_area_ctrl.virt_addr->top_head.magic != DUMP_GLOBALE_TOP_HEAD_MAGIC)
    {
        adump_err("mntn ddr not initial !\n");
        return ADUMP_ERR;
    }
    dump_load = (dump_load_info_t *)((u8*)g_st_adump_area_ctrl.virt_addr+(MNTN_AREA_RESERVE_ADDR-MNTN_BASE_ADDR));
    dump_load->magic_num    = DUMP_LOAD_MAGIC;
    dump_load->ap_ddr  = 0xC0000000;
    dump_load->ap_share= (uintptr_t)SHM_BASE_ADDR;
    dump_load->ap_dump = (u32)g_st_adump_area_ctrl.virt_addr;
    dump_load->ap_sram = (u32)(uintptr_t)HI_SRAM_MEM_ADDR_VIRT;
    dump_load->ap_dts  = HI_IO_ADDRESS(DDR_ACORE_DTS_ADDR);

    g_st_adump_area_ctrl.ulInitFlag = true;
    return 0;
}
EXPORT_SYMBOL(adump_area_init);




