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
 * *	notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *	notice, this list of conditions and the following disclaimer in the
 * *	documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *	be used to endorse or promote products derived from this software 
 * *	without specific prior written permission.
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



#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/wakelock.h>

#include "product_config.h"
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/netlink.h>
#include <linux/wakelock.h>
#include <mdrv_mbb_channel.h>

#include <osl_module.h>
#include <osl_sem.h>
#include <osl_list.h>
#include <osl_spinlock.h>
#include <bsp_softtimer.h>
#include <bsp_trace.h>
#include <osl_thread.h>
#include "softtimer_balong.h"
#define MOD_NAME "softtimer"
#define softtimer_print(fmt, ...)  (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_SOFTTIMER, "<%s> "fmt" ", MOD_NAME, ##__VA_ARGS__))

struct debug_wakeup_softtimer_s
{
	u32 wakeup_flag;
	const char* wakeup_timer_name;
	u32 wakeup_timer_id;
}debug_wakeup_timer;

#define SPECIAL_TIMER_START             (1)
#define SPECIAL_TIMER_STOP              (0)
#define SPECIAL_TIMER_VOTE_SLEEP_TIME   (1000)
#define SPECIAL_TIMER_TIME_UNIT_SECOND  (1000)
/*用于应用起休眠能唤醒的定时器*/
struct softtimer_list s_special_softtimer;
/*用于special timer超时以后投LCD的反对休眠票，和超时后投允许休眠票*/
static struct wake_lock special_timer_lock;

/*用户态接口锁*/
spinlock_t softtimer_lock;

/* 限制应用最大同时启动定时器数目，如需增加需经过评审 */
#define SPECIAL_TIMER_NUM_MAX  (6)

/* 定时器结构体 */
struct special_timer_str
{
    unsigned int uTime; /* 定时时长 */
    int iId;           /* 定时器编号，由应用指定，定时器到时后将此编号通知应用 */
    struct softtimer_list s_special_softtimer; /* 定时器 */
};

/* 定时器结构体数组，用于存储应用设置的所有定时器 */
struct special_timer_str timer_str[SPECIAL_TIMER_NUM_MAX];


struct softtimer_ctrl
{
	unsigned char timer_id_alloc[SOFTTIMER_MAX_NUM];
	struct list_head timer_list_head;
	u32 softtimer_start_value;
	u32 hard_timer_id;
	spinlock_t  timer_list_lock;
	osl_sem_id soft_timer_sem;
	OSL_TASK_ID softtimer_task;
	u32 clk;	/*hardtimer work freq*/
	u32 support;/*whether support this type softtimer*/
	u32 wake_times;/*softtimer wake system times*/
	u64 start_slice;
	slice_curtime get_curtime;
	slice_value   get_slice_value;
	struct     wake_lock     wake_lock;
};
/*lint --e{64}*/
static struct softtimer_ctrl timer_control[2];	/*timer_control[0] wake, timer_control[1] normal*/
u32 check_softtimer_support_type(enum wakeup type){
	return timer_control[(u32)type].support;
}
static void start_hard_timer(struct softtimer_ctrl *ptimer_control, u32 ulvalue )
{
	ptimer_control->softtimer_start_value = ulvalue;
	(void)ptimer_control->get_curtime(&ptimer_control->start_slice);
	bsp_hardtimer_disable(ptimer_control->hard_timer_id);
	bsp_hardtimer_load_value(ptimer_control->hard_timer_id,ulvalue);
	bsp_hardtimer_enable(ptimer_control->hard_timer_id);
}

static void stop_hard_timer(struct softtimer_ctrl *ptimer_control)
{
	bsp_hardtimer_disable(ptimer_control->hard_timer_id);
	ptimer_control->softtimer_start_value = ELAPESD_TIME_INVAILD;
}
static inline u32 calculate_timer_start_value(u64 expect_cb_slice,u64 cur_slice)
{
	if(expect_cb_slice > cur_slice)
		return (u32)(expect_cb_slice - cur_slice);
	else
		return 0;
}

/*
 bsp_softtimer_add,add the timer to the list;
 */
void bsp_softtimer_add(struct softtimer_list * timer)
{
	struct softtimer_list *p = NULL;
	unsigned long flags = 0;
	u64 now_slice = 0;
	if (NULL == timer)
	{
		softtimer_print("timer to be added is NULL\n");
		return;
	}
	spin_lock_irqsave(&(timer_control[timer->wake_type].timer_list_lock),flags);
	if(!list_empty(&timer->entry))
	{
		spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);
		return;
	}
	(void)timer_control[timer->wake_type].get_curtime(&now_slice);
	timer->expect_cb_slice = timer->count_num + now_slice;
	list_for_each_entry(p,&(timer_control[timer->wake_type].timer_list_head),entry)
	{
		if(p->expect_cb_slice > timer->expect_cb_slice)
		{
			break;
		}
	}
	list_add_tail(&(timer->entry),&(p->entry));
	if (timer_control[timer->wake_type].timer_list_head.next == &(timer->entry))
	{
		if ((timer->entry.next)!=(&(timer_control[timer->wake_type].timer_list_head)))
		{
			p = list_entry(timer->entry.next,struct softtimer_list,entry);
			if(TIMER_TRUE==p->is_running)
			{
				p->is_running = TIMER_FALSE;
			}
		}
		timer->is_running = TIMER_TRUE;
		start_hard_timer(&timer_control[timer->wake_type],calculate_timer_start_value(timer->expect_cb_slice,now_slice));
	}
	spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);
}
s32 bsp_softtimer_delete(struct softtimer_list * timer)
{
	struct softtimer_list * p=NULL;
	unsigned long flags;
	u64 now_slice = 0;
	if (NULL == timer)
	{
		softtimer_print("NULL pointer \n");
		return BSP_ERROR;
	}
	spin_lock_irqsave(&(timer_control[timer->wake_type].timer_list_lock),flags);
	(void)timer_control[timer->wake_type].get_curtime(&now_slice);
	if (list_empty(&timer->entry))
	{
		spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);
		return NOT_ACTIVE;
	}
	else
	{
	 	/*if the timer bo be deleted is the first node */
		if((timer->entry.prev == &(timer_control[timer->wake_type].timer_list_head))
			&&(timer->entry.next != &(timer_control[timer->wake_type].timer_list_head)))
		{
			timer->is_running = TIMER_FALSE;
			list_del_init(&(timer->entry));
			p=list_first_entry(&(timer_control[timer->wake_type].timer_list_head),struct softtimer_list,entry);
			start_hard_timer(&timer_control[p->wake_type], calculate_timer_start_value(p->expect_cb_slice,now_slice));
			p->is_running = TIMER_TRUE;
		}
		else
		{
			timer->is_running = TIMER_FALSE;
			list_del_init(&(timer->entry));
		}
	}
	/*if the list is empty after delete node, then stop hardtimer*/
	if (list_empty(&(timer_control[timer->wake_type].timer_list_head)))
	{
		stop_hard_timer(&timer_control[timer->wake_type]);
	}
	spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);
	return BSP_OK;
}
static inline s32 set_timer_expire_time_s(struct softtimer_list *timer,u32 new_expire_time)
{
	u32 timer_freq = 0;
	timer_freq = timer_control[timer->wake_type].clk;
	if(timer->timeout>0xFFFFFFFF/timer_freq)
	{
		softtimer_print("time too long\n");
		return BSP_ERROR;
	}
	timer->count_num= timer_freq * new_expire_time;
	return BSP_OK;
}

static inline s32 set_timer_expire_time_ms(struct softtimer_list *timer,u32 new_expire_time)
{
	u32 timer_freq = 0;
	timer_freq = timer_control[timer->wake_type].clk;
	if(timer->timeout>0xFFFFFFFF/timer_freq*1000)
	{
		softtimer_print("time too long\n");
		return BSP_ERROR;
	}
	if(timer_freq%1000)
	{
		if((new_expire_time) < (0xFFFFFFFF/timer_freq)) 
		{
			timer->count_num= (timer_freq* new_expire_time)/1000;
		}
		else 
		{
			timer->count_num= timer_freq * (new_expire_time/1000);
		}
	}
	 else 
	{
		timer->count_num= (timer_freq/1000)* new_expire_time;
	}
	return BSP_OK;
}

static inline s32 set_timer_expire_time(struct softtimer_list *timer,u32 expire_time)
{
	if(TYPE_S == timer->unit_type)
	{
		return set_timer_expire_time_s(timer,expire_time);
	}
	else if(TYPE_MS == timer->unit_type)
	{
		return set_timer_expire_time_ms(timer,expire_time);
	}
	else
	{
		return BSP_ERROR;
	}
}
s32 bsp_softtimer_modify(struct softtimer_list *timer,u32 new_expire_time)
{
	
	if((NULL == timer)||(!list_empty(&timer->entry)) )
	{
		return BSP_ERROR;
	}
	timer->timeout = new_expire_time;
	return set_timer_expire_time(timer,new_expire_time);
}

s32 bsp_softtimer_create(struct softtimer_list *timer)
{
	s32 ret = 0,i=0;
	unsigned long flags;
	if (!timer || !timer_control[(u32)timer->wake_type].support)
	{
		softtimer_print("para or wake type is error\n");
		return BSP_ERROR;
	}
	if(timer->init_flags==TIMER_INIT_FLAG)
		return BSP_ERROR;
	spin_lock_irqsave(&(timer_control[timer->wake_type].timer_list_lock),flags);/*lint !e550*/	
	INIT_LIST_HEAD(&(timer->entry));
	timer->is_running = TIMER_FALSE;
	ret = set_timer_expire_time(timer,timer->timeout);
	if(ret)
	{
		spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);	
		return BSP_ERROR;
	}
	for (i=0 ;i < SOFTTIMER_MAX_NUM; i++)
	{
		if (timer_control[timer->wake_type].timer_id_alloc[i] == 0)
		{
			timer->timer_id = i;
			timer_control[timer->wake_type].timer_id_alloc[i] = 1;
			break;
		}
	}
	if (SOFTTIMER_MAX_NUM == i)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_SOFTTIMER,"error,not enough timerid for alloc, already 40 exists\n");
		spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);	
		return BSP_ERROR;
	}
	timer->init_flags=TIMER_INIT_FLAG;
	spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);	
	return BSP_OK;
}

s32 bsp_softtimer_free(struct softtimer_list *timer)
{
	unsigned long flags;
	if ((NULL == timer)||(!list_empty(&timer->entry)))
	{
		return BSP_ERROR;
	}
	spin_lock_irqsave(&(timer_control[timer->wake_type].timer_list_lock),flags);/*lint !e550*/	
	timer->init_flags = 0;
	timer_control[timer->wake_type].timer_id_alloc[timer->timer_id] = 0;
	spin_unlock_irqrestore(&(timer_control[timer->wake_type].timer_list_lock),flags);
	return BSP_OK;   
}


int  softtimer_task_func(void* data)
{
	struct softtimer_ctrl *ptimer_control;
	struct softtimer_list	 *p = NULL,*q = NULL;
	unsigned long flags;
	u64 now_slice = 0;
	u32 temp1 = 0,temp2 = 0;

	ptimer_control = (struct softtimer_ctrl *)data;
	/* coverity[no_escape] */
	for( ; ; )
	{
		/* coverity[sleep] */
		osl_sem_down(&ptimer_control->soft_timer_sem);
		 /* coverity[lock_acquire] */
		spin_lock_irqsave(&ptimer_control->timer_list_lock,flags);
		(void)ptimer_control->get_curtime(&now_slice);
		ptimer_control->softtimer_start_value = ELAPESD_TIME_INVAILD;
		list_for_each_entry_safe(p,q,&(ptimer_control->timer_list_head),entry)
		{
			if(!p->emergency)
			{				
				if(now_slice >= p->expect_cb_slice)
				{
					list_del_init(&p->entry);
					p->is_running = TIMER_FALSE;
					spin_unlock_irqrestore(&ptimer_control->timer_list_lock,flags); 
					temp1 = bsp_get_slice_value();
					if(p->func)
						p->func(p->para);
					temp2 = bsp_get_slice_value();
					p->run_cb_delta = get_timer_slice_delta(temp1,temp2);
					spin_lock_irqsave(&ptimer_control->timer_list_lock,flags);
					(void)ptimer_control->get_curtime(&now_slice);
				}
				else
				{
					break;
				}
			}
			else
				break;
		}
		if (!list_empty(&(ptimer_control->timer_list_head)))/*如果还有未超时定时器*/
		{
			p=list_first_entry(&(ptimer_control->timer_list_head),struct softtimer_list,entry);
			if(p->is_running == TIMER_FALSE)
			{
				p->is_running = TIMER_TRUE;
				start_hard_timer(ptimer_control,calculate_timer_start_value(p->expect_cb_slice , now_slice));
			}
		}
		else 
		{
			stop_hard_timer(ptimer_control);
		}

		if(ACORE_SOFTTIMER_ID==ptimer_control->hard_timer_id)
		{
			wake_unlock(&ptimer_control->wake_lock);
		}

		if(debug_wakeup_timer.wakeup_flag)
		{
			softtimer_print("wakeup timer name:%s,wakeup_timer_id:%d",debug_wakeup_timer.wakeup_timer_name,debug_wakeup_timer.wakeup_timer_id);
			debug_wakeup_timer.wakeup_flag = 0;
		}

		spin_unlock_irqrestore(&ptimer_control->timer_list_lock,flags); 
	} 
	/*lint -save -e527*/ 
	return 0;
	/*lint -restore +e527*/ 
}


OSL_IRQ_FUNC(static irqreturn_t,softtimer_interrupt_call_back,irq,dev)
{	
	struct softtimer_ctrl *ptimer_control;
	u32 readValue = 0,temp1 = 0,temp2 = 0;
	u64 now_slice = 0;
	struct softtimer_list	 *p = NULL,*q = NULL;
	unsigned long flags=0;

	ptimer_control = dev;
	(void)ptimer_control->get_curtime(&now_slice);
	readValue = bsp_hardtimer_int_status(ptimer_control->hard_timer_id);
	if (0 != readValue)
	{
		bsp_hardtimer_int_clear(ptimer_control->hard_timer_id);
		spin_lock_irqsave(&ptimer_control->timer_list_lock,flags);/*lint !e550*/
		list_for_each_entry_safe(p,q,&ptimer_control->timer_list_head,entry)
		{
			if(p->emergency)
			{
				if(now_slice >= p->expect_cb_slice)
				{
					list_del_init(&p->entry);
					p->is_running = TIMER_FALSE;
					spin_unlock_irqrestore(&ptimer_control->timer_list_lock,flags); /*lint !e550*/
					temp1 = bsp_get_slice_value();
					if(p->func)
						p->func(p->para);
					temp2 = bsp_get_slice_value();
					p->run_cb_delta = get_timer_slice_delta(temp1,temp2);
					spin_lock_irqsave(&ptimer_control->timer_list_lock,flags);/*lint !e550*/
				}
				else
				{
					break;
				}
			}
			else
				break;
		}
		spin_unlock_irqrestore(&ptimer_control->timer_list_lock,flags); /*lint !e550*/
		if(ACORE_SOFTTIMER_ID==ptimer_control->hard_timer_id)
		{
			wake_lock(&ptimer_control->wake_lock);
		}

		osl_sem_up(&ptimer_control->soft_timer_sem);
	}
	return IRQ_HANDLED;
}
static int get_softtimer_int_stat(int arg)
{
	struct softtimer_list	 *p = NULL;
	timer_control[SOFTTIMER_WAKE].wake_times++;
	if(!list_empty(&(timer_control[SOFTTIMER_WAKE].timer_list_head)))
	{
		p=list_first_entry(&(timer_control[SOFTTIMER_WAKE].timer_list_head),struct softtimer_list,entry);/*lint !e826*/
		debug_wakeup_timer.wakeup_timer_name = p->name;
		debug_wakeup_timer.wakeup_timer_id = p->timer_id;
		debug_wakeup_timer.wakeup_flag = 1;
	}
	return 0;
}

int  bsp_softtimer_init(void)
{
	s32 ret = 0;
	struct device_node *node = NULL;
	/* coverity[var_decl] */
	struct bsp_hardtimer_control timer_ctrl ;
	INIT_LIST_HEAD(&(timer_control[SOFTTIMER_WAKE].timer_list_head));
	INIT_LIST_HEAD(&(timer_control[SOFTTIMER_NOWAKE].timer_list_head));
	timer_control[SOFTTIMER_NOWAKE].hard_timer_id	  = ACORE_SOFTTIMER_NOWAKE_ID;
	timer_control[SOFTTIMER_WAKE].hard_timer_id		  = ACORE_SOFTTIMER_ID;
	timer_control[SOFTTIMER_WAKE].softtimer_start_value  = ELAPESD_TIME_INVAILD;
	timer_control[SOFTTIMER_NOWAKE].softtimer_start_value = ELAPESD_TIME_INVAILD;
	osl_sem_init(SEM_EMPTY,&(timer_control[SOFTTIMER_NOWAKE].soft_timer_sem));
	osl_sem_init(SEM_EMPTY,&(timer_control[SOFTTIMER_WAKE].soft_timer_sem));
	spin_lock_init(&(timer_control[SOFTTIMER_WAKE].timer_list_lock));
	spin_lock_init(&(timer_control[SOFTTIMER_NOWAKE].timer_list_lock));
	timer_ctrl.func = (irq_handler_t)softtimer_interrupt_call_back;
	timer_ctrl.mode=TIMER_ONCE_COUNT;
	timer_ctrl.timeout=0xffffffff;/*default value set 0xFFFFFFFF*/
	timer_ctrl.unit=TIMER_UNIT_NONE;
	node = of_find_compatible_node(NULL, NULL, "hisilicon,softtimer_support_type");
	if(!node)
	{
		softtimer_print("softtimer_support_type get failed.\n");
		return BSP_ERROR;
	}
	if (!of_device_is_available(node)){
		softtimer_print("softtimer_support_type status not ok.\n");
		return BSP_ERROR;
	}
	debug_wakeup_timer.wakeup_flag = 0;
	debug_wakeup_timer.wakeup_timer_id = 0xffffffff;
	ret = of_property_read_u32(node, "support_wake", &timer_control[SOFTTIMER_WAKE].support);
	ret |= of_property_read_u32(node, "wake-frequency", &timer_control[SOFTTIMER_WAKE].clk);
	ret |= of_property_read_u32(node, "support_unwake", &timer_control[SOFTTIMER_NOWAKE].support);
	ret |= of_property_read_u32(node, "unwake-frequency", &timer_control[SOFTTIMER_NOWAKE].clk);
	if(ret)
	{
		softtimer_print(" softtimer property  get failed.\n");
		return BSP_ERROR;
	}
	if (timer_control[SOFTTIMER_WAKE].support)
	{
		if(ERROR == osl_task_init("softtimer_wake", TIMER_TASK_WAKE_PRI, TIMER_TASK_STK_SIZE ,(void *)softtimer_task_func, (void*)&timer_control[SOFTTIMER_WAKE],
			&timer_control[SOFTTIMER_WAKE].softtimer_task))
		{
			softtimer_print("softtimer_wake task create failed\n");
			return BSP_ERROR;
		}
		timer_ctrl.para = (void*)&timer_control[SOFTTIMER_WAKE];
		timer_ctrl.timerId =ACORE_SOFTTIMER_ID;
		timer_ctrl.irq_flags = IRQF_NO_SUSPEND;
		ret =bsp_hardtimer_config_init(&timer_ctrl);
		if (ret)
		{
			softtimer_print("bsp_hardtimer_alloc error,softtimer init failed 2\n");
			return BSP_ERROR;
		}
		wake_lock_init(&timer_control[SOFTTIMER_WAKE].wake_lock, WAKE_LOCK_SUSPEND, "softtimer_wake");
		timer_control[SOFTTIMER_WAKE].get_curtime = bsp_slice_getcurtime;
		timer_control[SOFTTIMER_WAKE].get_slice_value = bsp_get_slice_value;
		mdrv_timer_debug_register(timer_control[SOFTTIMER_WAKE].hard_timer_id,(FUNCPTR_1)get_softtimer_int_stat,0);
	}
	 if (timer_control[SOFTTIMER_NOWAKE].support)
	 {
		 if(ERROR == osl_task_init("softtimer_nowake", TIMER_TASK_NOWAKE_PRI, TIMER_TASK_STK_SIZE ,(void *)softtimer_task_func, (void*)&timer_control[SOFTTIMER_NOWAKE],
			&timer_control[SOFTTIMER_NOWAKE].softtimer_task))
			{
				softtimer_print("softtimer_normal task create failed\n");
				return BSP_ERROR;
			}
		timer_ctrl.para = (void*)&timer_control[SOFTTIMER_NOWAKE];
		timer_ctrl.timerId =ACORE_SOFTTIMER_NOWAKE_ID;
		timer_ctrl.irq_flags = 0;
		/* coverity[uninit_use_in_call] */
		 ret =bsp_hardtimer_config_init(&timer_ctrl);
		if (ret)
		{
			softtimer_print("bsp_hardtimer_alloc error,softtimer init failed 2\n");
			return BSP_ERROR;
		}
		if(timer_control[SOFTTIMER_NOWAKE].clk%1000)
		{
			timer_control[SOFTTIMER_NOWAKE].get_curtime = bsp_slice_getcurtime;
			timer_control[SOFTTIMER_NOWAKE].get_slice_value = bsp_get_slice_value;
		}
		else
		{
			timer_control[SOFTTIMER_NOWAKE].get_curtime = bsp_slice_getcurtime_hrt;
			timer_control[SOFTTIMER_NOWAKE].get_slice_value = bsp_get_slice_value_hrt;
		}
		wake_lock_init(&timer_control[SOFTTIMER_NOWAKE].wake_lock, WAKE_LOCK_SUSPEND, "softtimer_nowake");
	 }
	 bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_SOFTTIMER,"softtimer init success\n");
	return BSP_OK;
}

s32 show_list(u32 wake_type)
{
	struct softtimer_list * timer = NULL;
	unsigned long flags = 0;
	u64 now_slice = 0;
	softtimer_print("softttimer wakeup %d times\n",timer_control[wake_type].wake_times);
	(void)timer_control[wake_type].get_curtime(&now_slice);
	softtimer_print("id name  expect_cb  now_slice  cb_cost  emerg\n");
	softtimer_print("----------------------------------------------------------------------------------\n");
	spin_lock_irqsave(&(timer_control[wake_type].timer_list_lock),flags); 
	list_for_each_entry(timer,&(timer_control[wake_type].timer_list_head),entry)
	{
		softtimer_print("%d %s  0x%x  0x%x  %d  %d\n",timer->timer_id,timer->name,(u32)timer->expect_cb_slice,(u32)now_slice,timer->run_cb_delta,timer->emergency);
	}
	 spin_unlock_irqrestore(&(timer_control[wake_type].timer_list_lock),flags); 
	return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : special_timer_id_search
* 功能描述  :  查询已有定时器的下标，如果没有找到则返回可用下标，如果都没有则返回-1
* 输入参数  : int id：需要查询的定时器ID，该ID由应用通过ioctl设置
* 输出参数  : 无
* 返 回 值  : 有可用下标则返回下标，无可用位置则返回-1
* 其它说明  : 无
*****************************************************************************/
int special_timer_id_search(int id)
{
    int i = 0;
    /* 首先找id相同的，如果找到了则说明已经有这个定时器了，返回该定时器在数组中的位置 */
    for(i = 0; i < SPECIAL_TIMER_NUM_MAX; i++)
    {
        if(id == timer_str[i].iId )
        {
            return i;
        }
    }

    /* ID没有相同的说明需要新建，找到空闲位置并返回下标 */
    for(i = 0; i < SPECIAL_TIMER_NUM_MAX; i++)
    {
        if(TIMER_INIT_FLAG != timer_str[i].s_special_softtimer.init_flags)
        {
            return i;
        }
    }

    /* 如果ID即没有在已有定时器中找到，也没有空闲的位置，则返回-1 */
    return -1;
}

/*****************************************************************************
* 函 数 名  : special_timer_cb
* 功能描述  : Special_timer 超时处理函数
*                               借用LCD投反对票，起1秒定时器，超时后投LCD允许休眠票
*                               通过netlink上报上层定时器超时事件
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : 无
* 其它说明  : 无
*****************************************************************************/
/*lint -e10 -e64 */
int special_timer_cb(void * temp)
{
    int ret = -1;
    int size = -1;
    BSP_S32 timer_num = 0;
    DEVICE_EVENT *event = NULL;
    int id = (int)temp;
    char buff[sizeof(DEVICE_EVENT) + sizeof(int)];

    /* 超时wakelock锁 */
    wake_lock_timeout(&special_timer_lock, (long)msecs_to_jiffies(SPECIAL_TIMER_VOTE_SLEEP_TIME));

    /* restart the timer */
    timer_num = special_timer_id_search(id);
    if( 0 > timer_num)
    {
        printk(KERN_ERR "[special_timer] Special timer is not exist, id by app is %d !\n",id);
        return -1;
    }

    if( TIMER_INIT_FLAG == timer_str[timer_num].s_special_softtimer.init_flags)
    {
        bsp_softtimer_add(&(timer_str[timer_num].s_special_softtimer));
    }

    event = (DEVICE_EVENT *)buff;
    size =  sizeof(buff);

    event->device_id = DEVICE_ID_TIMER;      /* 设备ID */
    event->event_code = DEVICE_TIMEROU_F;    /* 事件代码 */
    event->len = sizeof(int);
    memcpy(event->data, &id, sizeof(int));

    /* 超时事件上报 */
    ret = device_event_report(event, size);
    printk( KERN_EMERG "softtimer event: kernel_id=%d, code=%d, len=%d, user_id=%d\n",
        event->device_id, event->event_code, event->len, *((int *)(event->data)));

    if (-1 == ret) 
    {
        printk(KERN_ERR "special_timer_cb device_event_report fail!\n");
    }

    return ret;
}

/*****************************************************************************
* 函 数 名  : special_timer_start_func
* 功能描述  : 起special timer定时器
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : BSP_OK--创建成功；
                           BSP_ERROR--创建失败；
* 其它说明  : 无
*****************************************************************************/
BSP_S32 special_timer_start_func(struct special_timer_str special_timer_par)
{
    unsigned int uTimeTemp = 0;
    BSP_S32 timer_num = 0;
    s32 ret = 0;

    uTimeTemp = special_timer_par.uTime * SPECIAL_TIMER_TIME_UNIT_SECOND;

    printk(KERN_ERR "[special_timer] Timer create start, user ID  is  %d \n", special_timer_par.iId);

    timer_num = special_timer_id_search(special_timer_par.iId);
    if( 0 > timer_num)
    {
        printk(KERN_EMERG "[special_timer] Special timer is full, creat not allowed!\n");
        return -1;
    }

    if(TIMER_INIT_FLAG == timer_str[timer_num].s_special_softtimer.init_flags)
    {
        printk(KERN_EMERG "[special_timer] Special timer is already created, creat again not allowed!\n");
        return -1;
    }

    /* 填充定时器信息 */
    timer_str[timer_num].s_special_softtimer.func = (softtimer_func)special_timer_cb;     /* 超时回调 */
    timer_str[timer_num].s_special_softtimer.para = (void *)(special_timer_par.iId);/* 定时器编号 */
    timer_str[timer_num].s_special_softtimer.timeout = uTimeTemp;         /* 超时时间 */
    timer_str[timer_num].s_special_softtimer.wake_type = SOFTTIMER_WAKE;                 /* 可唤醒定时器 */
    timer_str[timer_num].s_special_softtimer.emergency = 1;

    /* 创建定时器 */
    ret = bsp_softtimer_create(&(timer_str[timer_num].s_special_softtimer));
    if (ret)
    {
        printk(KERN_ERR "create vote_sleep softtimer failed \n");
        return -1;
    }
    /* 添加到定时器列表中，并启动该定时器 */
    timer_str[timer_num].iId = special_timer_par.iId;
    timer_str[timer_num].uTime = special_timer_par.uTime;
    bsp_softtimer_add(&(timer_str[timer_num].s_special_softtimer));
    printk(KERN_INFO "[special_timer] Creat add start special timer sucess, kernel timerID is %d \n",
        timer_str[timer_num].s_special_softtimer.timer_id);
    
    return 0;
}

/*****************************************************************************
* 函 数 名  : special_timer_stop_func
* 功能描述  : 停止special timer定时器
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : BSP_OK--创建成功；
                           BSP_ERROR--创建失败；
* 其它说明  : 无
*****************************************************************************/
BSP_S32 special_timer_stop_func(struct special_timer_str special_timer_par)
{
    BSP_S32 timer_num = 0;
    struct softtimer_list *  softtimer;

    timer_num = special_timer_id_search(special_timer_par.iId);
    if( 0 > timer_num )
    {
        printk(KERN_ERR "[special_timer] Timer delete end, No such ID in the list!\n");
        return -1;
    }

    softtimer = &(timer_str[timer_num].s_special_softtimer);

    printk(KERN_ERR "[special_timer] Timer delete start, user ID  is  %d \n", special_timer_par.iId);

    if ( TIMER_INIT_FLAG == softtimer->init_flags )
    {
        printk(KERN_INFO "Before delete, timerID  is  %d \n", softtimer->timer_id);
        /* 从超时列表中删除该定时器 */
        if(0 > bsp_softtimer_delete(softtimer))
        {
            printk(KERN_EMERG "Delete special_timer fail,kernel timerID is %d !\n", softtimer->timer_id);
            return -1;
        }
        /* 释放定时器 */
        if(0 != bsp_softtimer_free(softtimer))
        {
            printk(KERN_EMERG "Free special_timer fail!\n");
            return -1;
        }
        timer_str[timer_num].iId = -1;
        timer_str[timer_num].uTime = -1;
    }
    else
    {
        printk(KERN_ERR "[special_timer] Timer delete end, already deleted!\n");
    }

    return 0;
}

/*****************************************************************************
* 函 数 名  : balong_special_timer_open
* 功能描述  : Special_timer open处理函数
                               预留，目前没使用
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : 
* 其它说明  : 无
*****************************************************************************/
BSP_S32 balong_special_timer_open(struct inode *inode, struct file *file)
{
    if (NULL != inode && NULL != file)
    {
        ;//for pclint
    }
    return 0;
}

/*****************************************************************************
* 函 数 名  : balong_special_timer_release
* 功能描述  : Special_timer release处理函数
                               预留，目前没使用
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : 
* 其它说明  : 无
*****************************************************************************/
BSP_S32 balong_special_timer_release(struct inode *inode, struct file *file)
{
    if (NULL != inode && NULL != file)
    {
        ;//for pclint
    }
    return 0;
}

/*****************************************************************************
* 函 数 名  : balong_special_timer_read
* 功能描述  : Special_timer read处理函数
                               预留，目前没使用
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : 
* 其它说明  : 无
*****************************************************************************/
BSP_S32 balong_special_timer_read(struct file *file, char __user * buffer, 
        size_t count, loff_t *ppos)
{
    if (NULL != file)
    {
        ; //for pclint
    }

    return 0;
}

/*****************************************************************************
* 函 数 名  : balong_special_timer_write
* 功能描述  :字符设备写函数，预留
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : 
* 其它说明  : 无
*****************************************************************************/
BSP_S32 balong_special_timer_write(struct file *file, const char __user *buf, 
        size_t count,loff_t *ppos)
{
    if (NULL == file && NULL == ppos)
    {
        ;//for pclint
    }

    return 0;
}

/*****************************************************************************
* 函 数 名  : balong_special_timer_ioctl
* 功能描述  : Special_timer ioctrl处理函数
* 输入参数  : 
* 输出参数  : 无
* 返 回 值  : BSP_OK:    操作成功
*             BSP_ERROR: 操作失败
* 其它说明  : 无
*****************************************************************************/
long balong_special_timer_ioctl(struct file *file, unsigned int bStart, unsigned long arg)
{
    BSP_S32 err_code = 0;
    unsigned long flag = 0;
    struct special_timer_str special_timer_par; /* 用户存储用户态传递的参数信息 */

    if (NULL == file)
    {
        ; //for pclint
    }
    spin_lock_irqsave(&softtimer_lock, flag);

    /* 从用户态读取参数信息 */
    if (copy_from_user(&special_timer_par, (void *)arg, sizeof(struct special_timer_str)))
    {
        printk(KERN_ERR "[special_timer] copy_from_user failed!\n");
        err_code = -1;
        goto end;
    }

    /* 启动定时器 */
    if(SPECIAL_TIMER_START == bStart)
    {
        err_code = special_timer_start_func(special_timer_par);
        if(0 != err_code)
        {
            printk(KERN_EMERG "balong_special_timer_ioctll() start timer error---!!!\n");
            goto end;
        }
    }
    /* 停止定时器 */
    else if(SPECIAL_TIMER_STOP == bStart)
    {
        err_code = special_timer_stop_func(special_timer_par);
        if(0 != err_code)
        {
            printk(KERN_EMERG "balong_special_timer_ioctll() stop timer error---!!!\n");
            goto end;
        }
    }
    else
    {
        printk(KERN_EMERG "param bStart invalid, bStart=%d!\n",bStart);
        err_code = -1;
        goto end;
    }
    /*printk(KERN_EMERG "balong_special_timer_ioctll() leave-------!!!\n");*/
end:
    spin_unlock_irqrestore(&softtimer_lock, flag);
    return (long)err_code;
}

/*special timer函数数据结构*/
/*lint -e527 */
struct file_operations balong_special_timer_fops = {
    .owner = THIS_MODULE,
    .read = balong_special_timer_read,
    .write = balong_special_timer_write,
    .unlocked_ioctl = balong_special_timer_ioctl,
    .open = balong_special_timer_open,
    .release = balong_special_timer_release,
};
/*lint +e527 */

/*balong_special_timer_miscdev作为调用misc_register函数的参数，
  用于向linux内核注册special timer(硬timer)misc设备。
*/
static struct miscdevice balong_special_timer_miscdev = {
    .name = "special_timer",
    .minor = MISC_DYNAMIC_MINOR,/*动态分配子设备号（minor）*/
    .fops = &balong_special_timer_fops,
};

/*****************************************************************************
* 函 数 名  : balong_special_timer_init
*
* 功能描述  : Special_timer A核模块初始化
*
* 输入参数  : 
* 输出参数  : 无
*
* 返 回 值  : BSP_OK:    操作成功
*             BSP_ERROR: 操作失败
*
* 其它说明  : 无
*
*****************************************************************************/
BSP_S32 balong_special_timer_init(void)
{
    int ret = 0;
    int i = 0;

    /* 初始化wakelock锁 */
    wake_lock_init(&special_timer_lock , WAKE_LOCK_SUSPEND, "special_timer_lock");
    printk("------special_timer wakelock init ok ----------!\n");
    
    /* 注册misc设备，/dev/special_timer */
    ret = misc_register(&balong_special_timer_miscdev);
    if (0 != ret)
    {
        printk("------special_timer misc register fail!\n");
        return ret;
    }

    /* 初始化定时器数组, 存储应用设置的定时器，支持同时启动多个定时器 */
    for(i = 0; i < SPECIAL_TIMER_NUM_MAX; i++)
    {
        timer_str[i].iId = -1;
        timer_str[i].uTime = -1;
    }

    spin_lock_init(&softtimer_lock);
    printk("------special_timer misc register leave!\n");
    return BSP_OK;
}
/*lint -esym(529,balong_special_timer_init,__initcall_balong_special_timer_init6)*/
module_init(balong_special_timer_init);
/*lint +e10 +e64 */


EXPORT_SYMBOL(bsp_softtimer_create);
EXPORT_SYMBOL(bsp_softtimer_delete);
EXPORT_SYMBOL(bsp_softtimer_modify);
EXPORT_SYMBOL(bsp_softtimer_add);
EXPORT_SYMBOL(bsp_softtimer_free);
EXPORT_SYMBOL(check_softtimer_support_type);
EXPORT_SYMBOL(show_list);
arch_initcall(bsp_softtimer_init);



