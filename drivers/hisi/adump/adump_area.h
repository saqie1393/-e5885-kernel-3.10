

#ifndef __DUMP_AREA_H__
#define __DUMP_AREA_H__

#include <product_config.h>
#include "mntn_interface.h"
#ifndef __ASSEMBLY__
#include "osl_types.h"
#include "osl_list.h"
#include "osl_spinlock.h"
#endif
#include "bsp_memmap.h"
#include "bsp_s_memory.h"
#include "bsp_dump_mem.h"


struct adump_global_area_ctrl_s{
    u32                             ulInitFlag;
    u32                             length;
    struct dump_global_struct_s*    virt_addr;
    void*                           phy_addr;
};

s32 adump_get_area_info(DUMP_AREA_ID areaid,struct dump_area_mntn_addr_info_s* area_info);
s32 adump_area_init(void);


#endif



