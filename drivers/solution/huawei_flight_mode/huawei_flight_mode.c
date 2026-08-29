/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <bsp_nvim.h>
#include <bsp_icc.h>
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "bsp_version.h"
#include "huawei_flight_mode.h"
#include "mbb_flight_mode.h"

#if (FEATURE_ON == MBB_DLOAD)
#include "bsp_sram.h"
#endif /*MBB_DLOAD*/

/*硬件防抖的定时器*/
static RFSWITCH_T_CTRL_S g_wdis_ctx = {0};
/*切换射频状态时的睡眠锁*/
static struct wake_lock rfswitch_pin_lock;

/*飞行模式状态全局变量，由软硬件开关共同决定*/
static unsigned int g_rfswitch_state = 0;
/*温度保护状态全局变量*/
static unsigned int g_therm_rf_off = FALSE;

/*飞行模式切换触发类型存储队列长度。硬件开关设置飞行模式命令执行时间约为3S，
  硬件开关的防抖动时间为200ms,防止队列被占满*/

static queue_trig queue_trigs[TRIG_QUEUE_LENGTH];

static unsigned int queue_trig_head = 0; /*队头*/
static unsigned int queue_trig_tail = 0; /*队尾*/

/*自旋锁，对队列操作进行加锁操作*/
static spinlock_t trig_lock;



/*****************************************************************************
函 数 名  : rfswitch_trig_queue_in
功能描述  : 飞行模式触发方式入队列
输入参数  : trig_type 触发方式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void rfswitch_trig_queue_in(rfswitch_trig_type trig_type)
{
    unsigned long flags = 0;

    spin_lock_irqsave(&trig_lock, flags);
    if (0 == queue_trigs[queue_trig_tail].is_set)
    {
        /*队列尾部未被设置表明队列未满，执行入队操作*/
        queue_trigs[queue_trig_tail].trig_type = trig_type;
        queue_trigs[queue_trig_tail].is_set = 1;
        queue_trig_tail = (queue_trig_tail + 1) % TRIG_QUEUE_LENGTH;
        spin_unlock_irqrestore(&trig_lock, flags);
        return ;
    }
    else
    {
        /* 队列已满 */
        printk(KERN_ERR "\n%s() The Queue Is Full!",__FUNCTION__);
    }
    spin_unlock_irqrestore(&trig_lock, flags);
}

/*****************************************************************************
函 数 名  : RfSwitch_Trig_Queue_Out
功能描述  : 飞行模式触发方式出队列
输入参数  : void
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void rfswitch_trig_queue_out(void)
{
    unsigned long flags = 0;

    spin_lock_irqsave(&trig_lock, flags);
    if(queue_trig_head != queue_trig_tail)
    {
        /*判断当前队列是否为空，不为空则将队头标识后移。*/
        queue_trigs[queue_trig_head].is_set = 0;
        queue_trig_head = (queue_trig_head + 1) % TRIG_QUEUE_LENGTH;
    }
    else
    {
        /*队列为空*/
        printk(KERN_ERR "\n%s(): The Queue is Empty!", __FUNCTION__);
    }

    spin_unlock_irqrestore(&trig_lock, flags);
}

/*****************************************************************************
函 数 名  : rfswitch_trig_queue_get
功能描述  : 从飞行模式触发方式队列中获取一个触发方式
输入参数  : void
输出参数  : 无
返 回 值  : 触发方式
*****************************************************************************/
rfswitch_trig_type rfswitch_trig_queue_get(void)
{
    unsigned long flags = 0;

    /*加锁*/
    spin_lock_irqsave(&trig_lock, flags);
    if ((0 != queue_trigs[queue_trig_head].is_set))
    {
        /*解锁*/
        spin_unlock_irqrestore(&trig_lock, flags);
        /*当前队头的触发类型是否有效，有效则返回队头触发类型*/
        return queue_trigs[queue_trig_head].trig_type;
    }
    else
    {
        /*解锁*/
        spin_unlock_irqrestore(&trig_lock, flags);
        return RFSWITCH_NONE_TRIG;
    }
}


/*****************************************************************************
函 数 名  : rfswitch_therm_state_set
功能描述  : 飞行模式温度保护状态设置
输入参数  : state:需要设置的状态
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static void rfswitch_therm_state_set(unsigned int state)
{
    g_therm_rf_off = state;
}

/*****************************************************************************
函 数 名  : rfswitch_therm_state_get
功能描述  : 飞行模式温度保护状态设置
输入参数  : 无
输出参数  : 无
返 回 值  : 当前温度保护状态
*****************************************************************************/
unsigned int rfswitch_therm_state_get(void)
{
    return g_therm_rf_off;
}
/*****************************************************************************
函 数 名  : rfswitch_sw_set
功能描述  : 飞行模式软件开关设置
输入参数  : swstate软件开关状态
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void rfswitch_sw_set(unsigned int swstate)
{
    int ret = 0;
    FLIGHT_MODE_STRU  swswitchPara;     
    swswitchPara.huawei_flight_mode = swstate;

    ret = bsp_nvm_write(NV_ID_FLIGHT_MODE,
                       (unsigned char *)&(swswitchPara),
                        sizeof(swswitchPara));
    if(0 != ret)
    {
        printk(KERN_ERR "write sw state to nv failed\n");
    }
    return;
}
/*****************************************************************************
函 数 名  : rfswitch_sw_get
功能描述  : 飞行模式软件开关查询
输入参数  : 无
输出参数  : 无
返 回 值  : 0:软件开关打开，关闭射频
            1:软件开关关闭，打开射频
*****************************************************************************/
unsigned int rfswitch_sw_get(void)
{
    unsigned int ret = 0;
    FLIGHT_MODE_STRU  swswitchPara;

    (void)memset(&swswitchPara, 0x1, sizeof(swswitchPara));
    ret = bsp_nvm_read(NV_ID_FLIGHT_MODE,
                       (unsigned char *)&(swswitchPara),
                       sizeof(swswitchPara));
    if(0 != ret)
    {
        printk(KERN_ERR "read sw state from nv failed\n");
    }
    return swswitchPara.huawei_flight_mode;
}

/*****************************************************************************
函 数 名  : rfswitch_hw_get
功能描述  : 获取飞行模式开关GPIO状态a核调用
输入参数  : 无
输出参数  : 无
返 回 值  : 0:硬件开关打开飞行模式，关闭射频
            1:硬件开关关闭飞行模式，打开射频
*****************************************************************************/
unsigned int rfswitch_hw_get(void)
{
    int value = 1;
    value = gpio_get_value(W_DISABLE_PIN);
    if ( 0 == value )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*****************************************************************************
函 数 名  : rfswitch_State_Set
功能描述  : 设置飞行模式状态
输入参数  : rf_state 射频模式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void rfswitch_state_set(unsigned int rf_state)
{
    if(RF_ON != rf_state && RF_OFF != rf_state)
    {
        printk(KERN_ERR "<rfswitch_state_set>para is error:rf_state=%d\r\n",rf_state);
        rf_state = RF_ON;
    }
    g_rfswitch_state = rf_state;
}

/*****************************************************************************
函 数 名  : rfswitch_state_get
功能描述  : 查询飞行模式状态
输入参数  : 无
输出参数  : 无
返 回 值  : g_rfswitch_state
*****************************************************************************/
unsigned int rfswitch_state_get(void)
{
    return g_rfswitch_state;
}


/*****************************************************************************
函 数 名  : rfswitch_Change_Mode_To_State
功能描述  : 根据设置RF的mode获得RFSWITCH的state
输入参数  : current_mode 设置的模式
输出参数  : 无
返 回 值  : 0:软件进入飞行模式，关闭射频
            1:软件退出飞行模式，打开射频
*****************************************************************************/
static int rfswitch_change_mode_to_state(unsigned int current_mode)
{
    unsigned int temp_state = RF_ON;
    if (1 ==current_mode)
    {
        temp_state = RF_ON;
    }
    else
    {
        temp_state = RF_OFF;
    }
    return temp_state;
}

/*****************************************************************************
函 数 名  : RfSwitch_Swswtich_op_end
功能描述  : 触发飞行模式结束处理函数
输入参数  : trig_type:触发方式，op_mode:软件操作模式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void rfswitch_switch_end_operation(rfswitch_trig_type trig_type, unsigned int sw_op_mode)
{
    unsigned int sw_state = RF_ON; 
    unsigned int hw_state = RF_ON; 
    unsigned int rf_state = RF_ON; 
    
    /*获取真实的硬件开关状态，由GPIO管脚获取*/
    hw_state = rfswitch_hw_get();

    if(RFSWITCH_INIT_TRIG == trig_type)
    {
        /*获取软件开关状态，由nv获取*/
        sw_state = rfswitch_sw_get();
        rf_state = (sw_state & hw_state) & RF_ON;
    }  
    else if(RFSWITCH_SW_TRIG == trig_type)
    {
        /*获取软件开关状态，由入参决定*/
        sw_state = rfswitch_change_mode_to_state(sw_op_mode);
        rf_state = (sw_state & hw_state) & RF_ON;
        
        /*设置软件飞行模式开关NV*/
        rfswitch_sw_set(sw_state); 
        
        /*主动上报*/
        rfswitch_state_report(sw_state,hw_state);
#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
        /*通知温度保护模块，当前飞行模式的状态*/
        rfswitch_state_info_therm(rf_state);
#endif
    }
    else if(RFSWITCH_HW_TRIG == trig_type)
    {
        /*获取软件开关状态，由nv获取*/
        sw_state = rfswitch_sw_get();
        rf_state = (sw_state & hw_state) & RF_ON;
        
        /*主动上报*/
        rfswitch_state_report(sw_state,hw_state); 
    }

     /*设置飞行模式标记*/
    rfswitch_state_set(rf_state);
    rfswitch_trig_queue_out();
    return;
}
/*****************************************************************************
函 数 名  : rfswitch_Hwswitch_handle
功能描述  : 硬件飞行模式处理函数
输入参数  :current_mode 当前模式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static irqreturn_t rfswitch_hwswitch_handle(int irq, void *dev_id)
{
    unsigned long flags;

    spin_lock_irqsave(&g_wdis_ctx.lock, flags);

    /*先获取一次gpio的值*/
    g_wdis_ctx.hw_temp = rfswitch_hw_get();
    
    /*起定时器,如果存在未到超时定时器，先删除*/
    (void)bsp_softtimer_delete(&g_wdis_ctx.irq_timer);
    
    /*起timer前延迟系统 休眠，防止模块休眠timer失效，时间要大于timer定时时间*/
    wake_lock_timeout(&rfswitch_pin_lock, (long)msecs_to_jiffies(WDIS_N_TIMER_LENGTH+2000)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_wdis_ctx.irq_timer);

    spin_unlock_irqrestore(&g_wdis_ctx.lock, flags);
    return IRQ_HANDLED;
}

/*****************************************************************************
函 数 名  : rfswitch_wdis_timer_handler
功能描述  :飞行模式硬件开关定时器回调函数，
           用来发消息给rfswtich_task切换射频
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static void rfswitch_wdis_timer_handler(unsigned int param)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON;
    unsigned int rf_state = RF_ON;

    sw_state = rfswitch_sw_get();
    hw_state = rfswitch_hw_get();
    
    printk(KERN_ERR "rfswitch hwstate %d, swstate is %d\r\n",hw_state,sw_state);
    
    /*此时读取的值与上次的值不一样则不处理，防抖*/
    if (hw_state != g_wdis_ctx.hw_temp)
    {
        printk(KERN_ERR "two read gpio value is different.\n");
        return;
    }

    /*如果需要切换的状态与目前状态一致，则不处理*/
    if (hw_state == g_wdis_ctx.hw_state)
    {
        printk(KERN_ERR "the state to set is the same as now.\n");
        return;
    }

    g_wdis_ctx.hw_state = hw_state;
    /*设置RF状态*/
    rf_state = (sw_state & hw_state) & RF_ON;
    /*硬件触发类型入队*/
    rfswitch_trig_queue_in(RFSWITCH_HW_TRIG);
    /*切换状态*/
    rfswitch_change(rf_state);
}
/*****************************************************************************
函 数 名  : rfswitch_timer_init
功能描述  : 飞行模式定时器初始化
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static int rfswitch_timer_init(void)
{
    int ret = -1;
    struct softtimer_list *irq_timer = &g_wdis_ctx.irq_timer;
    
    /*irq_timer用来硬件开关的处理*/
    irq_timer->func = rfswitch_wdis_timer_handler;
    irq_timer->para = (u32)&g_wdis_ctx;
    irq_timer->timeout = WDIS_N_TIMER_LENGTH;
    irq_timer->wake_type = SOFTTIMER_WAKE;
    ret = bsp_softtimer_create(irq_timer);

    if(0 != ret)
    {
        printk(KERN_ERR "rfswitch_timer_init create fail\n");
        return -1;
    }
    return 0;
}

/*****************************************************************************
函 数 名  : rfswitch_gpio_init
功能描述  : 飞行模式硬件开关初始化
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static int rfswitch_gpio_init(void)
{
    int value = 0; /*注册共享中断时dev_id不能为空*/
    
    if(0 != gpio_request(W_DISABLE_PIN, WDIS_DRIVER_NAME))
    {
        printk(KERN_ERR "gpio_request W_DISABLE_PIN  fail \n");
        return -1;
    }
    
    (void)gpio_direction_input(W_DISABLE_PIN);

    if(0 != request_irq(gpio_to_irq(W_DISABLE_PIN), rfswitch_hwswitch_handle, \
        IRQF_NO_SUSPEND | IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, \
        (char *)WDIS_DRIVER_NAME, &value))
    {
        printk(KERN_ERR "rfswitch_gpio_init request_irq  fail \n");
        gpio_free(W_DISABLE_PIN);
        return -1;
    }
    return 0;
}

/*****************************************************************************
 函 数 名  : rfswitch_icc_init_cb
 功能描述  : C核初始化后向A核发icc消息的回调函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : 0:成功  -1:失败
*****************************************************************************/
static int rfswitch_icc_cb(void)
{
    int read_len = 0;
    unsigned int rf_init = (unsigned int)RFSWITCH_RF_NONE;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_RF_INIT;
    read_len = bsp_icc_read(channel_id, (unsigned char*)&rf_init, sizeof(rf_init));
    if ( sizeof(rf_init) != read_len )
    {
        printk("rfswitch_icc_cb bsp_icc_read fail\r\n");
        return -1;
    }
    
    /*温度保护要开关射频，这里只需要修改标记即可*/
    if(RFSWITCH_RF_OFF == (rfswitch_ccore_op_type)rf_init)
    {
        rfswitch_therm_state_set(TRUE);
        return 0;
    }
    else if(RFSWITCH_RF_ON == (rfswitch_ccore_op_type)rf_init)
    {
        rfswitch_therm_state_set(FALSE);
        return 0;
    }
    else if(RFSWITCH_RF_INIT == (rfswitch_ccore_op_type)rf_init)
    {
        /*获得硬件飞行模式状态*/
        g_wdis_ctx.hw_state = rfswitch_hw_get();
        /*获得软件飞行模式状态*/
        g_wdis_ctx.sw_state = rfswitch_sw_get();

        /*如果上电初始化是软件进入飞行模式，则直接切换状态*/  
        if ((RF_OFF == g_wdis_ctx.sw_state) || (RF_OFF == g_wdis_ctx.hw_state))
        {  
            /*上电初始化触发方式入队*/
            rfswitch_trig_queue_in(RFSWITCH_INIT_TRIG);
            /*如果上电初始化即进入飞行模式，则直接切换状态*/    
            rfswitch_change(RF_OFF);  
            return 0;
        }
        rfswitch_state_set(RF_ON);
        return 0;
    }  
    else
    {
        printk("RfSwitch_icc_cb rf_init is rfswitch_RF_NONE\r\n");
        return -1;
    }

}

/*****************************************************************************
函 数 名  : rfswitch_Init
功能描述  : 飞行模式初始化函数
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static int __init  rfswitch_init(void)
{
    int ret = -1;
    SOLUTION_PRODUCT_TYPE module_type = PRODUCT_TYPE_INVALID;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_RF_INIT;
    
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        return ret;
    }
    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return ret;
    }
#endif /*MBB_DLOAD*/

    /* 仅CE模块才初始化该模块 */
    module_type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE != module_type)
    {
        printk(KERN_ERR "rfswitch module will not init!");
        return ret;
    }

    /*trig type 队列初始化*/
    (void)memset(queue_trigs, 0x0, sizeof(queue_trigs));
    
    if (rfswitch_timer_init())
    {
        printk(KERN_ERR "%s(): timer init fail. \n",__FUNCTION__);
        return ret;
    }

    (void)memset(queue_trigs, 0x0, sizeof(queue_trigs));
    
    /*初始化系统 锁*/
    wake_lock_init(&rfswitch_pin_lock, WAKE_LOCK_SUSPEND, WDIS_DRIVER_NAME);
    spin_lock_init(&trig_lock);
    
    /*设定飞行模式管脚中断状态*/ 
    ret = rfswitch_gpio_init();
    if(0 != ret)
    {
        printk(KERN_ERR "rfswitch_gpio_init fail\n");
    }
    
    /*注册icc事件，为C核初始化准备*/
    ret = bsp_icc_event_register(channel_id, (read_cb_func)rfswitch_icc_cb, NULL, NULL, NULL);
    if(0 != ret)
    {
        printk(KERN_ERR "rfswitch_Init bsp_icc_event_register fail\n");
    }
    return 0 ;
}
module_init(rfswitch_init);


