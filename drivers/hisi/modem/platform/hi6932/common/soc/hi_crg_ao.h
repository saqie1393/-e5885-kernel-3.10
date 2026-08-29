#ifndef __HI_AO_CRG_REG_H__
#define __HI_AO_CRG_REG_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_AO_CRG_CLKEN1_OFFSET (0x0)
#define HI_AO_CRG_CLKDIS1_OFFSET (0x4)
#define HI_AO_CRG_CLKSTAT1_OFFSET (0x8)
#define HI_AO_CRG_CLKCG_STAT1_OFFSET (0xC)
#define HI_AO_CRG_SRSTEN1_OFFSET (0x60)
#define HI_AO_CRG_SRSTDIS1_OFFSET (0x64)
#define HI_AO_CRG_SRSTSTAT1_OFFSET (0x68)
#define HI_AO_CRG_CLKDIV1_OFFSET (0x100)
#define HI_AO_CRG_CLK_SEL_OFFSET (0x140)
#define HI_AO_CRG_CLKSW_STAT1_OFFSET (0x260)
#define HI_AO_CRG_SCLKEN1_OFFSET (0x300)
#define HI_AO_CRG_SCLKDIS1_OFFSET (0x304)
#define HI_AO_CRG_SCLKSTAT1_OFFSET (0x308)
#define HI_AO_CRG_SCLKCG_STAT1_OFFSET (0x30C)
#define HI_AO_CRG_SSRSTEN1_OFFSET (0x360)
#define HI_AO_CRG_SSRSTDIS1_OFFSET (0x364)
#define HI_AO_CRG_SSRSTSTAT1_OFFSET (0x368)
#ifndef __ASSEMBLY__
typedef union
{
    struct
    {
        unsigned int gpio0_clk_en : 1;
        unsigned int tsensor_clk_en : 1;
        unsigned int sci0_clk_en : 1;
        unsigned int sci1_clk_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int efuse_clk_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int timer_clk_en : 8;
        unsigned int reserved_2 : 9;
        unsigned int rtc_clk_en : 1;
        unsigned int ios_ao_clk_en : 1;
        unsigned int bootrom_clk_en : 1;
        unsigned int ao_dbg_pclk_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int bbpon_clk_en : 1;
        unsigned int bc_clk_en : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKEN1_T;
typedef union
{
    struct
    {
        unsigned int gpio0_clk_dis : 1;
        unsigned int tsensor_clk_dis : 1;
        unsigned int sci0_clk_dis : 1;
        unsigned int sci1_clk_dis : 1;
        unsigned int reserved_4 : 1;
        unsigned int efuse_clk_dis : 1;
        unsigned int reserved_3 : 1;
        unsigned int timer_clk_dis : 8;
        unsigned int reserved_2 : 9;
        unsigned int rtc_clk_dis : 1;
        unsigned int ios_ao_clk_dis : 1;
        unsigned int bootrom_clk_dis : 1;
        unsigned int ao_dbg_pclk_dis : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int bbpon_clk_dis : 1;
        unsigned int bc_clk_dis : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKDIS1_T;
typedef union
{
    struct
    {
        unsigned int gpio0_clk_status : 1;
        unsigned int tsensor_clk_status : 1;
        unsigned int sci0_clk_status : 1;
        unsigned int sci1_clk_status : 1;
        unsigned int reserved_4 : 1;
        unsigned int efuse_clk_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int timer_clk_status : 8;
        unsigned int reserved_2 : 9;
        unsigned int rtc_clk_status : 1;
        unsigned int ios_ao_clk_status : 1;
        unsigned int bootrom_clk_status : 1;
        unsigned int ao_dbg_pclk_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int bbpon_clk_status : 1;
        unsigned int bc_clk_status : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKSTAT1_T;
typedef union
{
    struct
    {
        unsigned int gpio0_clkcg_status : 1;
        unsigned int tsensor_clkcg_status : 1;
        unsigned int sci0_clkcg_status : 1;
        unsigned int sci1_clkcg_status : 1;
        unsigned int reserved_4 : 1;
        unsigned int efuse_clkcg_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int timer_clkcg_status : 8;
        unsigned int reserved_2 : 9;
        unsigned int rtc_clkcg_status : 1;
        unsigned int ios_ao_clkcg_status : 1;
        unsigned int bootrom_clkcg_status : 1;
        unsigned int ao_dbg_pclkcg_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int bbpon_clkcg_status : 1;
        unsigned int bc_clkcg_status : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKCG_STAT1_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 2;
        unsigned int tsensor_srst_en : 1;
        unsigned int sci0_srst_en : 1;
        unsigned int ao_timer0_srst_en : 1;
        unsigned int ao_timer1_srst_en : 1;
        unsigned int ao_timer2_srst_en : 1;
        unsigned int ao_timer3_srst_en : 1;
        unsigned int ao_timer4_srst_en : 1;
        unsigned int ao_timer5_srst_en : 1;
        unsigned int ao_timer6_srst_en : 1;
        unsigned int ao_timer7_srst_en : 1;
        unsigned int reserved_2 : 3;
        unsigned int sci1_srst_en : 1;
        unsigned int gpio0_srst_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ios_ao_srst_en : 1;
        unsigned int ao_dbg_srst_en : 1;
        unsigned int bcctrl_srst_en : 1;
        unsigned int usbphy_srst_en : 1;
        unsigned int usbotg_ctrl_srst_en : 1;
        unsigned int usbctrl_vaux_srs_en : 1;
        unsigned int usbctrl_vcc_srst_en : 1;
        unsigned int bbp_apb_srst_en : 1;
        unsigned int bbp_19m_srst_en : 1;
        unsigned int bbp_32k_srst_en : 1;
        unsigned int gbbp1_104m_srst_en : 1;
        unsigned int gbbp2_104m_srst_en : 1;
        unsigned int bbp_timer_srst_en : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SRSTEN1_T;
typedef union
{
    struct
    {
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tsensor_srst_dis : 1;
        unsigned int sci0_srst_dis : 1;
        unsigned int ao_timer0_srst_dis : 1;
        unsigned int ao_timer1_srst_dis : 1;
        unsigned int ao_timer2_srst_dis : 1;
        unsigned int ao_timer3_srst_dis : 1;
        unsigned int ao_timer4_srst_dis : 1;
        unsigned int ao_timer5_srst_dis : 1;
        unsigned int ao_timer6_srst_dis : 1;
        unsigned int ao_timer7_srst_dis : 1;
        unsigned int reserved_2 : 3;
        unsigned int sci1_srst_dis : 1;
        unsigned int gpio0_srst_dis : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ios_ao_srst_dis : 1;
        unsigned int ao_dbg_srst_dis : 1;
        unsigned int bcctrl_srst_dis : 1;
        unsigned int usbphy_srst_dis : 1;
        unsigned int usbotg_ctrl_srst_dis : 1;
        unsigned int usbctrl_vaux_srs_dis : 1;
        unsigned int usbctrl_vcc_srst_dis : 1;
        unsigned int bbp_apb_srst_dis : 1;
        unsigned int bbp_19m_srst_dis : 1;
        unsigned int bbp_32k_srst_dis : 1;
        unsigned int gbbp1_104m_srst_dis : 1;
        unsigned int gbbp2_104m_srst_dis : 1;
        unsigned int bbp_timer_srst_dis : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SRSTDIS1_T;
typedef union
{
    struct
    {
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int tsensor_srst_status : 1;
        unsigned int sci0_srst_status : 1;
        unsigned int ao_timer0_srst_status : 1;
        unsigned int ao_timer1_srst_status : 1;
        unsigned int ao_timer2_srst_status : 1;
        unsigned int ao_timer3_srst_status : 1;
        unsigned int ao_timer4_srst_status : 1;
        unsigned int ao_timer5_srst_status : 1;
        unsigned int ao_timer6_srst_status : 1;
        unsigned int ao_timer7_srst_status : 1;
        unsigned int reserved_2 : 3;
        unsigned int sci1_srst_status : 1;
        unsigned int gpio0_srst_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ios_ao_srst_status : 1;
        unsigned int ao_dbg_srst_status : 1;
        unsigned int bcctrl_srst_status : 1;
        unsigned int usbphy_srst_status : 1;
        unsigned int usbotg_ctrl_srst_status : 1;
        unsigned int usbctrl_vaux_srs_status : 1;
        unsigned int usbctrl_vcc_srst_status : 1;
        unsigned int bbp_apb_srst_status : 1;
        unsigned int bbp_19m_srst_status : 1;
        unsigned int bbp_32k_srst_status : 1;
        unsigned int gbbp1_104m_srst_status : 1;
        unsigned int gbbp2_104m_srst_status : 1;
        unsigned int bbp_timer_srst_status : 1;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SRSTSTAT1_T;
typedef union
{
    struct
    {
        unsigned int cm3_clk_div_num : 4;
        unsigned int ao_apb_freqmode : 2;
        unsigned int ao_apb_freqmode_sw_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int sci_clk_div_num : 4;
        unsigned int reserved_1 : 4;
        unsigned int reserved_0 : 16;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKDIV1_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 24;
        unsigned int reserved_2 : 1;
        unsigned int sleep_clk_sel : 1;
        unsigned int reserved_1 : 4;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLK_SEL_T;
typedef union
{
    struct
    {
        unsigned int reserved_1 : 14;
        unsigned int cm3_clksw_stat : 3;
        unsigned int reserved_0 : 15;
    } bits;
    unsigned int u32;
}HI_AO_CRG_CLKSW_STAT1_T;
typedef union
{
    struct
    {
        unsigned int timer1_clk_en : 2;
        unsigned int reserved_1 : 2;
        unsigned int wdog_clk_en : 1;
        unsigned int rtc1_clk_en : 1;
        unsigned int clkmon_clk_en : 1;
        unsigned int jtagauth_clk_en : 1;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SCLKEN1_T;
typedef union
{
    struct
    {
        unsigned int timer1_clk_dis : 2;
        unsigned int reserved_1 : 2;
        unsigned int wdog_clk_dis : 1;
        unsigned int rtc1_clk_dis : 1;
        unsigned int clkmon_clk_dis : 1;
        unsigned int jtagauth_clk_dis : 1;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SCLKDIS1_T;
typedef union
{
    struct
    {
        unsigned int timer1_clk_status : 2;
        unsigned int reserved_1 : 2;
        unsigned int wdog_clk_status : 1;
        unsigned int rtc1_clk_status : 1;
        unsigned int clkmon_clk_status : 1;
        unsigned int jtagauth_clk_status : 1;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SCLKSTAT1_T;
typedef union
{
    struct
    {
        unsigned int timer1_clkcg_status : 2;
        unsigned int reserved_1 : 2;
        unsigned int wdog_clkcg_status : 1;
        unsigned int rtc1_clkcg_status : 1;
        unsigned int clkmon_clkcg_status : 1;
        unsigned int jtagauth_clkcg_status : 1;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SCLKCG_STAT1_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int cm3_core_srst_en : 1;
        unsigned int reserved_2 : 2;
        unsigned int ao_timer8_srst_en : 1;
        unsigned int ao_timer9_srst_en : 1;
        unsigned int reserved_1 : 2;
        unsigned int cc_ao_srst_en : 1;
        unsigned int reserved_0 : 23;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SSRSTEN1_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int cm3_core_srst_dis : 1;
        unsigned int reserved_2 : 2;
        unsigned int ao_timer8_srst_dis : 1;
        unsigned int ao_timer9_srst_dis : 1;
        unsigned int reserved_1 : 2;
        unsigned int cc_ao_srst_dis : 1;
        unsigned int reserved_0 : 23;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SSRSTDIS1_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int cm3_core_srst_status : 1;
        unsigned int reserved_2 : 2;
        unsigned int ao_timer8_srst_status : 1;
        unsigned int ao_timer9_srst_status : 1;
        unsigned int reserved_1 : 2;
        unsigned int cc_ao_srst_status : 1;
        unsigned int reserved_0 : 23;
    } bits;
    unsigned int u32;
}HI_AO_CRG_SSRSTSTAT1_T;
HI_SET_GET(hi_ao_crg_clken1_gpio0_clk_en,gpio0_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_tsensor_clk_en,tsensor_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_sci0_clk_en,sci0_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_sci1_clk_en,sci1_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_reserved_4,reserved_4,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_efuse_clk_en,efuse_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_reserved_3,reserved_3,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_timer_clk_en,timer_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_reserved_2,reserved_2,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_rtc_clk_en,rtc_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_ios_ao_clk_en,ios_ao_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_bootrom_clk_en,bootrom_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_ao_dbg_pclk_en,ao_dbg_pclk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_reserved_1,reserved_1,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_reserved_0,reserved_0,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_bbpon_clk_en,bbpon_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clken1_bc_clk_en,bc_clk_en,HI_AO_CRG_CLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_gpio0_clk_dis,gpio0_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_tsensor_clk_dis,tsensor_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_sci0_clk_dis,sci0_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_sci1_clk_dis,sci1_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_reserved_4,reserved_4,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_efuse_clk_dis,efuse_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_reserved_3,reserved_3,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_timer_clk_dis,timer_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_reserved_2,reserved_2,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_rtc_clk_dis,rtc_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_ios_ao_clk_dis,ios_ao_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_bootrom_clk_dis,bootrom_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_ao_dbg_pclk_dis,ao_dbg_pclk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_reserved_1,reserved_1,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_reserved_0,reserved_0,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_bbpon_clk_dis,bbpon_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdis1_bc_clk_dis,bc_clk_dis,HI_AO_CRG_CLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_gpio0_clk_status,gpio0_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_tsensor_clk_status,tsensor_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_sci0_clk_status,sci0_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_sci1_clk_status,sci1_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_reserved_4,reserved_4,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_efuse_clk_status,efuse_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_reserved_3,reserved_3,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_timer_clk_status,timer_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_reserved_2,reserved_2,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_rtc_clk_status,rtc_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_ios_ao_clk_status,ios_ao_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_bootrom_clk_status,bootrom_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_ao_dbg_pclk_status,ao_dbg_pclk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_reserved_1,reserved_1,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_reserved_0,reserved_0,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_bbpon_clk_status,bbpon_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkstat1_bc_clk_status,bc_clk_status,HI_AO_CRG_CLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_gpio0_clkcg_status,gpio0_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_tsensor_clkcg_status,tsensor_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_sci0_clkcg_status,sci0_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_sci1_clkcg_status,sci1_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_reserved_4,reserved_4,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_efuse_clkcg_status,efuse_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_reserved_3,reserved_3,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_timer_clkcg_status,timer_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_reserved_2,reserved_2,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_rtc_clkcg_status,rtc_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_ios_ao_clkcg_status,ios_ao_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_bootrom_clkcg_status,bootrom_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_ao_dbg_pclkcg_status,ao_dbg_pclkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_reserved_1,reserved_1,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_reserved_0,reserved_0,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_bbpon_clkcg_status,bbpon_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkcg_stat1_bc_clkcg_status,bc_clkcg_status,HI_AO_CRG_CLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_reserved_3,reserved_3,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_tsensor_srst_en,tsensor_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_sci0_srst_en,sci0_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer0_srst_en,ao_timer0_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer1_srst_en,ao_timer1_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer2_srst_en,ao_timer2_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer3_srst_en,ao_timer3_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer4_srst_en,ao_timer4_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer5_srst_en,ao_timer5_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer6_srst_en,ao_timer6_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_timer7_srst_en,ao_timer7_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_reserved_2,reserved_2,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_sci1_srst_en,sci1_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_gpio0_srst_en,gpio0_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_reserved_1,reserved_1,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_reserved_0,reserved_0,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ios_ao_srst_en,ios_ao_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_ao_dbg_srst_en,ao_dbg_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_bcctrl_srst_en,bcctrl_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_usbphy_srst_en,usbphy_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_usbotg_ctrl_srst_en,usbotg_ctrl_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_usbctrl_vaux_srs_en,usbctrl_vaux_srs_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_usbctrl_vcc_srst_en,usbctrl_vcc_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_bbp_apb_srst_en,bbp_apb_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_bbp_19m_srst_en,bbp_19m_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_bbp_32k_srst_en,bbp_32k_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_gbbp1_104m_srst_en,gbbp1_104m_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_gbbp2_104m_srst_en,gbbp2_104m_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srsten1_bbp_timer_srst_en,bbp_timer_srst_en,HI_AO_CRG_SRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_reserved_4,reserved_4,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_reserved_3,reserved_3,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_tsensor_srst_dis,tsensor_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_sci0_srst_dis,sci0_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer0_srst_dis,ao_timer0_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer1_srst_dis,ao_timer1_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer2_srst_dis,ao_timer2_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer3_srst_dis,ao_timer3_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer4_srst_dis,ao_timer4_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer5_srst_dis,ao_timer5_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer6_srst_dis,ao_timer6_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_timer7_srst_dis,ao_timer7_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_reserved_2,reserved_2,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_sci1_srst_dis,sci1_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_gpio0_srst_dis,gpio0_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_reserved_1,reserved_1,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_reserved_0,reserved_0,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ios_ao_srst_dis,ios_ao_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_ao_dbg_srst_dis,ao_dbg_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_bcctrl_srst_dis,bcctrl_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_usbphy_srst_dis,usbphy_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_usbotg_ctrl_srst_dis,usbotg_ctrl_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_usbctrl_vaux_srs_dis,usbctrl_vaux_srs_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_usbctrl_vcc_srst_dis,usbctrl_vcc_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_bbp_apb_srst_dis,bbp_apb_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_bbp_19m_srst_dis,bbp_19m_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_bbp_32k_srst_dis,bbp_32k_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_gbbp1_104m_srst_dis,gbbp1_104m_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_gbbp2_104m_srst_dis,gbbp2_104m_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srstdis1_bbp_timer_srst_dis,bbp_timer_srst_dis,HI_AO_CRG_SRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_reserved_4,reserved_4,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_reserved_3,reserved_3,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_tsensor_srst_status,tsensor_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_sci0_srst_status,sci0_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer0_srst_status,ao_timer0_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer1_srst_status,ao_timer1_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer2_srst_status,ao_timer2_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer3_srst_status,ao_timer3_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer4_srst_status,ao_timer4_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer5_srst_status,ao_timer5_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer6_srst_status,ao_timer6_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_timer7_srst_status,ao_timer7_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_reserved_2,reserved_2,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_sci1_srst_status,sci1_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_gpio0_srst_status,gpio0_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_reserved_1,reserved_1,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_reserved_0,reserved_0,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ios_ao_srst_status,ios_ao_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_ao_dbg_srst_status,ao_dbg_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_bcctrl_srst_status,bcctrl_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_usbphy_srst_status,usbphy_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_usbotg_ctrl_srst_status,usbotg_ctrl_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_usbctrl_vaux_srs_status,usbctrl_vaux_srs_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_usbctrl_vcc_srst_status,usbctrl_vcc_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_bbp_apb_srst_status,bbp_apb_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_bbp_19m_srst_status,bbp_19m_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_bbp_32k_srst_status,bbp_32k_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_gbbp1_104m_srst_status,gbbp1_104m_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_gbbp2_104m_srst_status,gbbp2_104m_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_srststat1_bbp_timer_srst_status,bbp_timer_srst_status,HI_AO_CRG_SRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_cm3_clk_div_num,cm3_clk_div_num,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_ao_apb_freqmode,ao_apb_freqmode,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_ao_apb_freqmode_sw_en,ao_apb_freqmode_sw_en,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_reserved_2,reserved_2,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_sci_clk_div_num,sci_clk_div_num,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_reserved_1,reserved_1,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clkdiv1_reserved_0,reserved_0,HI_AO_CRG_CLKDIV1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_ao_crg_clk_sel_reserved_3,reserved_3,HI_AO_CRG_CLK_SEL_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLK_SEL_OFFSET)
HI_SET_GET(hi_ao_crg_clk_sel_reserved_2,reserved_2,HI_AO_CRG_CLK_SEL_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLK_SEL_OFFSET)
HI_SET_GET(hi_ao_crg_clk_sel_sleep_clk_sel,sleep_clk_sel,HI_AO_CRG_CLK_SEL_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLK_SEL_OFFSET)
HI_SET_GET(hi_ao_crg_clk_sel_reserved_1,reserved_1,HI_AO_CRG_CLK_SEL_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLK_SEL_OFFSET)
HI_SET_GET(hi_ao_crg_clk_sel_reserved_0,reserved_0,HI_AO_CRG_CLK_SEL_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLK_SEL_OFFSET)
HI_SET_GET(hi_ao_crg_clksw_stat1_reserved_1,reserved_1,HI_AO_CRG_CLKSW_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clksw_stat1_cm3_clksw_stat,cm3_clksw_stat,HI_AO_CRG_CLKSW_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_clksw_stat1_reserved_0,reserved_0,HI_AO_CRG_CLKSW_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_timer1_clk_en,timer1_clk_en,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_reserved_1,reserved_1,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_wdog_clk_en,wdog_clk_en,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_rtc1_clk_en,rtc1_clk_en,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_clkmon_clk_en,clkmon_clk_en,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_jtagauth_clk_en,jtagauth_clk_en,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclken1_reserved_0,reserved_0,HI_AO_CRG_SCLKEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKEN1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_timer1_clk_dis,timer1_clk_dis,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_reserved_1,reserved_1,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_wdog_clk_dis,wdog_clk_dis,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_rtc1_clk_dis,rtc1_clk_dis,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_clkmon_clk_dis,clkmon_clk_dis,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_jtagauth_clk_dis,jtagauth_clk_dis,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkdis1_reserved_0,reserved_0,HI_AO_CRG_SCLKDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_timer1_clk_status,timer1_clk_status,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_reserved_1,reserved_1,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_wdog_clk_status,wdog_clk_status,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_rtc1_clk_status,rtc1_clk_status,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_clkmon_clk_status,clkmon_clk_status,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_jtagauth_clk_status,jtagauth_clk_status,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkstat1_reserved_0,reserved_0,HI_AO_CRG_SCLKSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_timer1_clkcg_status,timer1_clkcg_status,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_reserved_1,reserved_1,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_wdog_clkcg_status,wdog_clkcg_status,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_rtc1_clkcg_status,rtc1_clkcg_status,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_clkmon_clkcg_status,clkmon_clkcg_status,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_jtagauth_clkcg_status,jtagauth_clkcg_status,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_sclkcg_stat1_reserved_0,reserved_0,HI_AO_CRG_SCLKCG_STAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SCLKCG_STAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_reserved_3,reserved_3,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_cm3_core_srst_en,cm3_core_srst_en,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_reserved_2,reserved_2,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_ao_timer8_srst_en,ao_timer8_srst_en,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_ao_timer9_srst_en,ao_timer9_srst_en,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_reserved_1,reserved_1,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_cc_ao_srst_en,cc_ao_srst_en,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrsten1_reserved_0,reserved_0,HI_AO_CRG_SSRSTEN1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTEN1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_reserved_3,reserved_3,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_cm3_core_srst_dis,cm3_core_srst_dis,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_reserved_2,reserved_2,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_ao_timer8_srst_dis,ao_timer8_srst_dis,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_ao_timer9_srst_dis,ao_timer9_srst_dis,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_reserved_1,reserved_1,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_cc_ao_srst_dis,cc_ao_srst_dis,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrstdis1_reserved_0,reserved_0,HI_AO_CRG_SSRSTDIS1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTDIS1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_reserved_3,reserved_3,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_cm3_core_srst_status,cm3_core_srst_status,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_reserved_2,reserved_2,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_ao_timer8_srst_status,ao_timer8_srst_status,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_ao_timer9_srst_status,ao_timer9_srst_status,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_reserved_1,reserved_1,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_cc_ao_srst_status,cc_ao_srst_status,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
HI_SET_GET(hi_ao_crg_ssrststat1_reserved_0,reserved_0,HI_AO_CRG_SSRSTSTAT1_T,HI_AO_CRG_REG_BASE_ADDR, HI_AO_CRG_SSRSTSTAT1_OFFSET)
#endif
#endif
