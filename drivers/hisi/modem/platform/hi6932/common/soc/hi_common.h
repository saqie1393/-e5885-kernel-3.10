#ifndef __HI_COMMON_H__
#define __HI_COMMON_H__ 
#include "product_config.h"
#include "bsp_memmap.h"
#include "hi_crg_pd.h"
#include "hi_crg_ao.h"
#include "hi_sc_pd.h"
static __inline__ void prepare_ccpu_a9(void)
{
#ifndef CONFIG_LOAD_SEC_IMAGE
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x20))) = (0x8040);
#endif
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x00))) = (0x28);
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x418))) = (MCORE_TEXT_START_ADDR);
}
static __inline__ void start_ccpu_a9(unsigned int addr)
{
    (void)addr;
#ifndef CONFIG_LOAD_SEC_IMAGE
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x24))) = (0x8040);
#endif
}
static __inline__ void hi_mcore_reset(void)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_AO_CRG_REG_BASE_ADDR + HI_AO_CRG_SSRSTEN1_OFFSET))) = (0x2);
}
static __inline__ void hi_mcore_active(void)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_AO_CRG_REG_BASE_ADDR + HI_AO_CRG_SSRSTDIS1_OFFSET))) = (0x2);
}
static __inline__ void hi_acore_set_entry_addr(int a)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_SYSCTRL_AO_REG_BASE_ADDR + HI_SC_TOP_CTRL3_OFFSET))) = (a);
}
#endif
