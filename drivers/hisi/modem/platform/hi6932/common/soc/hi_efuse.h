#ifndef __HI_EFUSE_H__
#define __HI_EFUSE_H__ 
#define CONFIG_EFUSE_NANDC_GROUP2 
#define EFUSE_MAX_SIZE (64)
#define EFUSE_LAYOUT_MP_FLAG_OFFSET (10)
#define EFUSE_LAYOUT_MP_FLAG_LCS_BIT_OFFSET (16)
#define EFUSE_LAYOUT_MP_FLAG_RMA_BIT_OFFSET (31)
#define EFUSE_LAYOUT_KCE_OFFSET (12)
#define EFUSE_LAYOUT_KCE_LENGTH (4)
#define EFUSE_LAYOUT_DFT_AUTH_KEY_OFFSET (59)
#define EFUSE_LAYOUT_DFT_AUTH_KEY_LENGTH (2)
#define EFUSE_LAYOUT_NS_VERIFY_BIT_OFFSET (1181)
#define EFUSE_LAYOUT_DFT_AUTH_KEY_RD_CTRL_BIT_OFFSET (2024)
#define EFUSE_LAYOUT_HIFI_DBG_CTRL_BIT_OFFSET (1988)
#define EFUSE_LAYOUT_BBE16_DBG_CTRL_BIT_OFFSET (1989)
#define EFUSE_LAYOUT_CS_DEVICE_CTRL_BIT_OFFSET (1990)
#define EFUSE_LAYOUT_JTAGEN_CTRL_BIT_OFFSET (2047)
#define EFUSE_LAYOUT_SEC_DBG_RST_CTRL_BIT_OFFSET (2004)
#define EFUSE_LAYOUT_CORESIGHT_RST_CTRL_BIT_OFFSET (1987)
#define EFUSE_LAYOUT_DFT_DISABLE_SEL_BIT_OFFSET (2018)
#define EFUSE_LAYOUT_ARM_DBG_CTRL_BIT_OFFSET (2025)
#define EFUSE_LAYOUT_ARM_DBG_CTRL_DBGEN_BIT_OFFSET (2027)
#define EFUSE_LAYOUT_ARM_DBG_CTRL_NIDEN_BIT_OFFSET (2028)
#define EFUSE_LAYOUT_ARM_DBG_CTRL_SPIDEN_BIT_OFFSET (2029)
#define EFUSE_LAYOUT_ARM_DBG_CTRL_SPNIDEN_BIT_OFFSET (2030)
#define EFUSE_GRP_DIEID (32)
#define EFUSE_DIEID_SIZE (5)
#define EFUSE_DIEID_BIT (28)
#define EFUSE_DIEID_LEN (EFUSE_DIEID_SIZE * EFUSE_GROUP_SIZE)
#define EFUSE_GRP_HUK (40)
#define EFUSE_HUK_SIZE (4)
#define EFUSE_HUK_LEN (EFUSE_HUK_SIZE * EFUSE_GROUP_SIZE)
#define EFUSE_COUNT_CFG (5)
#define PGM_COUNT_CFG (HI_APB_CLK / 1000000 * 12 - EFUSE_COUNT_CFG)
#define EFUSE_GROUP_ID_FOR_NANDC (63)
#define NAND_INFO_MASK_GROUP0 0x3F010000
#define NAND_INFO_MASK_GROUP1 0x00FE0000
#define HI_EFUSEC_CFG_OFFSET (0x0)
#define HI_EFUSEC_STATUS_OFFSET (0x4)
#define HI_EFUSE_GROUP_OFFSET (0x8)
#define HI_PG_VALUE_OFFSET (0xC)
#define HI_EFUSEC_COUNT_OFFSET (0x10)
#define HI_PGM_COUNT_OFFSET (0x14)
#define HI_EFUSEC_DATA_OFFSET (0x18)
#define HI_HW_CFG_OFFSET (0x1C)
#define HI_EFUSE_PGEN_BIT 0
#define HI_EFUSE_PRE_PG_BIT 1
#define HI_EFUSE_RD_EN_BIT 2
#define HI_EFUSE_AIB_SEL_BIT 3
#define HI_EFUSE_PG_STAT_BIT 0
#define HI_EFUSE_RD_STAT_BIT 1
#define HI_EFUSE_PGENB_STAT_BIT 2
#define HI_EFUSE_PD_STAT_BIT 4
#define HI_EFUSE_PD_EN_BIT 5
#define HI_EFUSE_GROUP_LBIT 0
#define HI_EFUSE_GROUP_HBIT 6
#define HI_EFUSE_PG_VALUE_LBIT 0
#define HI_EFUSE_PG_VALUE_HBIT 31
#define HI_EFUSE_DATA_LBIT 0
#define HI_EFUSE_DATA_HBIT 31
#define HI_EFUSE_COUNT_LBIT 0
#define HI_EFUSE_COUNT_HBIT 7
#define HI_EFUSE_PGM_COUNT_LBIT 0
#define HI_EFUSE_PGM_COUNT_HBIT 15
#define HI_EFUSE_DISFLAG_BIT 0
typedef union
{
    struct
    {
        unsigned int pgen : 1;
        unsigned int pre_pg : 1;
        unsigned int rden : 1;
        unsigned int aib_sel : 1;
        unsigned int rr_en : 1;
        unsigned int pd_en : 1;
        unsigned int mr_en : 1;
        unsigned int undefined : 25;
    } bits;
    unsigned int u32;
}HI_EFUSEC_CFG_T;
typedef union
{
    struct
    {
        unsigned int pg_status : 1;
        unsigned int rd_status : 1;
        unsigned int pgenb_status : 1;
        unsigned int rd_error : 1;
        unsigned int pd_status : 1;
        unsigned int undefined : 27;
    } bits;
    unsigned int u32;
}HI_EFUSEC_STATUS_T;
typedef union
{
    struct
    {
        unsigned int efuse_group : 7;
        unsigned int undefined : 25;
    } bits;
    unsigned int u32;
}HI_EFUSE_GROUP_T;
typedef union
{
    struct
    {
        unsigned int pg_value : 32;
    } bits;
    unsigned int u32;
}HI_PG_VALUE_T;
typedef union
{
    struct
    {
        unsigned int efusec_count : 8;
        unsigned int undefined : 24;
    } bits;
    unsigned int u32;
}HI_EFUSEC_COUNT_T;
typedef union
{
    struct
    {
        unsigned int pgm_count : 16;
        unsigned int undefined : 16;
    } bits;
    unsigned int u32;
}HI_PGM_COUNT_T;
typedef union
{
    struct
    {
        unsigned int efusec_data : 32;
    } bits;
    unsigned int u32;
}HI_EFUSEC_DATA_T;
typedef union
{
    struct
    {
        unsigned int pgm_disable : 1;
        unsigned int pad_disable : 1;
        unsigned int dft_disable_sel : 2;
        unsigned int boot_sel : 1;
        unsigned int secboot_en : 1;
        unsigned int apb_rd_huk_disable : 1;
        unsigned int apb_rd_scp_disable : 1;
        unsigned int apb_rd_dftkey_disable : 1;
        unsigned int arm_dbg_ctrl : 2;
        unsigned int dbgen : 1;
        unsigned int niden : 1;
        unsigned int spiden : 1;
        unsigned int spniden : 1;
        unsigned int reserved : 1;
        unsigned int nf_ctrl_ena0 : 1;
        unsigned int nf_block_size1 : 1;
        unsigned int nf_ecc_type1 : 2;
        unsigned int nf_page_size1 : 2;
        unsigned int nf_addr_num1 : 1;
        unsigned int nf_ctrl_ena1 : 1;
        unsigned int nf_block_size : 1;
        unsigned int nf_ecc_type : 2;
        unsigned int nf_page_size : 2;
        unsigned int nf_addr_num : 1;
        unsigned int nf_ctrl_ena : 1;
        unsigned int jtag_en : 1;
    } bits;
    unsigned int u32;
}HI_HW_CFG_T;
#if (!defined(__KERNEL__)) && (!defined(__OS_RTOSCK__))
#include <osl_bio.h>
#include <soc_memmap.h>
#undef INLINE
#define INLINE inline
#define HI_SET_GET_EFUSE(__full_name__,__reg_name,__reg_type,__reg_base,__reg_offset) \
  static INLINE void set_##__full_name__(unsigned int val) \
  {\
   __reg_type reg_obj; \
   reg_obj.u32 = readl((__reg_base) + __reg_offset); \
   reg_obj.bits.__reg_name = val; \
   writel(reg_obj.u32, (__reg_base) + __reg_offset); \
  } \
  static INLINE unsigned int get_##__full_name__(void) \
  {\
   __reg_type reg_obj; \
   reg_obj.u32 = readl((__reg_base) + __reg_offset); \
   return reg_obj.bits.__reg_name; \
  }
HI_SET_GET_EFUSE(hi_efusec_cfg_pgen,pgen,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_pre_pg,pre_pg,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_rden,rden,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_aib_sel,aib_sel,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_rr_en,rr_en,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_pd_en,pd_en,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_mr_en,mr_en,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_cfg_undefined,undefined,HI_EFUSEC_CFG_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_pg_status,pg_status,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_rd_status,rd_status,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_pgenb_status,pgenb_status,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_rd_error,rd_error,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_pd_status,pd_status,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_status_undefined,undefined,HI_EFUSEC_STATUS_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_STATUS_OFFSET)
HI_SET_GET_EFUSE(hi_efuse_group_efuse_group,efuse_group,HI_EFUSE_GROUP_T,HI_EFUSE_BASE_ADDR, HI_EFUSE_GROUP_OFFSET)
HI_SET_GET_EFUSE(hi_efuse_group_undefined,undefined,HI_EFUSE_GROUP_T,HI_EFUSE_BASE_ADDR, HI_EFUSE_GROUP_OFFSET)
HI_SET_GET_EFUSE(hi_pg_value_pg_value,pg_value,HI_PG_VALUE_T,HI_EFUSE_BASE_ADDR, HI_PG_VALUE_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_count_efusec_count,efusec_count,HI_EFUSEC_COUNT_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_COUNT_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_count_undefined,undefined,HI_EFUSEC_COUNT_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_COUNT_OFFSET)
HI_SET_GET_EFUSE(hi_pgm_count_pgm_count,pgm_count,HI_PGM_COUNT_T,HI_EFUSE_BASE_ADDR, HI_PGM_COUNT_OFFSET)
HI_SET_GET_EFUSE(hi_pgm_count_undefined,undefined,HI_PGM_COUNT_T,HI_EFUSE_BASE_ADDR, HI_PGM_COUNT_OFFSET)
HI_SET_GET_EFUSE(hi_efusec_data_efusec_data,efusec_data,HI_EFUSEC_DATA_T,HI_EFUSE_BASE_ADDR, HI_EFUSEC_DATA_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_pgm_disable,pgm_disable,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_pad_disable,pad_disable,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_dft_disable_sel,dft_disable_sel,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_boot_sel,boot_sel,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_secboot_en,secboot_en,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_apb_rd_huk_disable,apb_rd_huk_disable,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_apb_rd_scp_disable,apb_rd_scp_disable,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_apb_rd_dftkey_disable,apb_rd_dftkey_disable,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_arm_dbg_ctrl,arm_dbg_ctrl,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_dbgen,dbgen,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_niden,niden,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_spiden,spiden,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_spniden,spniden,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_reserved,reserved,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_ctrl_ena0,nf_ctrl_ena0,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_block_size1,nf_block_size1,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_ecc_type1,nf_ecc_type1,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_page_size1,nf_page_size1,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_addr_num1,nf_addr_num1,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_ctrl_ena1,nf_ctrl_ena1,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_block_size,nf_block_size,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_ecc_type,nf_ecc_type,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_page_size,nf_page_size,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_addr_num,nf_addr_num,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_nf_ctrl_ena,nf_ctrl_ena,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
HI_SET_GET_EFUSE(hi_hw_cfg_jtag_en,jtag_en,HI_HW_CFG_T,HI_EFUSE_BASE_ADDR, HI_HW_CFG_OFFSET)
static __inline__ unsigned int hi_efuse_get_pgm_disable_flag(void)
{
    return get_hi_hw_cfg_pgm_disable();
}
#endif
#endif
