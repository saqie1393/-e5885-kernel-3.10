/*
 * register rdr buffer for TEEOS. (RDR: kernel run data recorder.)
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/hisi/rdr_pub.h>
#include "tee_rdr_register.h"
#include "tc_ns_client.h"
#include "tee_client_constants.h"
#include "teek_ns_client.h"
#include "tc_ns_log.h"
#include "smc.h"
#include "securec.h"

#include "bsp_dump_mem.h"
#include "bsp_om_enum.h"

//#define TC_DEBUG
#define TEEOS_MODID         HISI_MNTN_EXC_TEEOS_START
#define TEEOS_MODID_END     HISI_MNTN_EXC_TEEOS_END
struct rdr_register_module_result current_rdr_info;
static const u64 current_core_id = RDR_TEEOS;

void tee_fn_dump(u32 modid,
		 u32 etype,
		 u64 coreid,
		 char *pathname,
		 pfn_cb_dump_done pfn_cb)
{
	u32 l_modid = 0;

	l_modid = modid;
	pfn_cb(l_modid, current_core_id);
}

void tee_fn_reset(u32 modid, u32 etype, u64 coreid)
{
	return;
}

/*
* add header of rdr.bin for balong v722
* dont delete. pls.
*/
#define TEEOS_LOG_MAGIC  0x70604102
void tee_rdr_buffer_init(void)
{
	struct _dump_area_s *s_area_info;
	errno_t ret_s;

	tloge("tee_rdr_buffer enter.\n");
	s_area_info = (void *)ioremap_wc(current_rdr_info.log_addr,
						sizeof(struct _dump_area_s));

	/*tloge("tee_rdr_buffer log start 0x%x.\n", s_area_info);*/
	tlogi("tee_rdr_buffer struct size 0x%x.\n",
		sizeof(struct _dump_area_s));

	current_rdr_info.log_addr += sizeof(struct _dump_area_s);
	current_rdr_info.log_len -= sizeof(struct _dump_area_s);
	/*tloge("tee_rdr_buffer log offset,0x%x.\n", current_rdr_info.log_addr);*/
	tlogi("tee_rdr_buffer log len,0x%x.\n", current_rdr_info.log_len);

	s_area_info->area_head.magic_num = TEEOS_LOG_MAGIC;
	s_area_info->area_head.field_num = 1;
	ret_s = memcpy_s((char *)s_area_info->area_head.name,
			8, "TrustOS", strlen("TrustOS"));  // len <= 8
	if (EOK != ret_s) {
		tloge("memcpy s_area_info->area_head.name failed.\n");
	}
	ret_s = memcpy_s((char *)s_area_info->area_head.version,
			16, "TrustOS 3.3.0", strlen("TrustOS 3.3.0")); //len <= 16
	if (EOK != ret_s) {
		tloge("memcpy s_area_info->area_head.version failed.\n");
	}
	s_area_info->fields[0].field_id = DUMP_TEE_FIELD_LOG;
	s_area_info->fields[0].offset_addr = sizeof(struct _dump_area_s);
	s_area_info->fields[0].length = current_rdr_info.log_len;
	s_area_info->fields[0].version = 312;
	s_area_info->fields[0].status = 1;
	ret_s = memcpy_s((char *)s_area_info->fields[0].field_name,
			16, "TrustOS Log", strlen("TrustOS Log"));//len <= 16
	if (EOK != ret_s) {
	    tloge("memcpy einfo.e_desc failed.\n");
	}
	iounmap(s_area_info);
	return;
}


int tee_rdr_register_core(void)
{
	struct rdr_module_ops_pub s_module_ops = {0};
	int ret = -1;

	s_module_ops.ops_dump = tee_fn_dump;
	s_module_ops.ops_reset = tee_fn_reset;

	ret = rdr_register_module_ops(current_core_id,
				      &s_module_ops, &current_rdr_info);
	tee_rdr_buffer_init();
	if (ret) {
	    tloge("register rdr mem failed.\n");
	}

	return ret;
}

int teeos_register_exception(void)
{
	struct rdr_exception_info_s einfo;
	int ret = -1;
	errno_t ret_s;
	const char tee_module_name[] = "RDR_TEEOS";
	const char tee_module_desc[] = "RDR_TEEOS crash";

	ret_s = memset_s(&einfo, sizeof(struct rdr_exception_info_s),
			0, sizeof(struct rdr_exception_info_s));
	if (ret_s) {
	    tloge("memset einfo failed.\n");
	    return ret_s;
	}

	einfo.e_modid = TEEOS_MODID;
	einfo.e_modid_end = TEEOS_MODID_END;
	einfo.e_process_priority = RDR_ERR;
	einfo.e_reboot_priority = RDR_REBOOT_WAIT;
	einfo.e_notify_core_mask = RDR_TEEOS | RDR_AP;
	einfo.e_reset_core_mask = RDR_TEEOS | RDR_AP;
	einfo.e_reentrant = RDR_REENTRANT_ALLOW;
	einfo.e_exce_type = TEE_S_EXCEPTION;
	einfo.e_from_core = RDR_TEEOS;
	einfo.e_upload_flag = RDR_UPLOAD_YES;

	ret_s = memcpy_s(einfo.e_from_module, sizeof(einfo.e_from_module),
			tee_module_name, sizeof(tee_module_name));
	if (ret_s) {
	    tloge("memcpy einfo.e_from_module failed.\n");
	    return ret_s;
	}

	ret_s = memcpy_s(einfo.e_desc, sizeof(einfo.e_desc),
			tee_module_desc, sizeof(tee_module_desc));
	if (ret_s) {
	    tloge("memcpy einfo.e_desc failed.\n");
	    return ret_s;
	}

	ret = rdr_register_exception(&einfo);
	if (ret) {
	    tloge("register exception mem failed.");
	}

	return ret;
}

/*Register rdr memory*/
int TC_NS_register_rdr_mem(void)
{
	TC_NS_SMC_CMD smc_cmd = {0};
	int ret = 0;
	unsigned char uuid[17] = {0};
	TC_NS_Operation operation = {0};
	u64 rdr_mem_addr;
	unsigned int rdr_mem_len;

	ret = tee_rdr_register_core();
	if (ret) {
		current_rdr_info.log_addr = 0x0;
		current_rdr_info.log_len = 0;
		return ret;
	}

	rdr_mem_addr = current_rdr_info.log_addr;
	rdr_mem_len = current_rdr_info.log_len;

	uuid[0] = 1;
	smc_cmd.uuid_phys = virt_to_phys(uuid);
	smc_cmd.uuid_h_phys = 0;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_RDR_MEM;

	operation.paramTypes = TEE_PARAM_TYPE_VALUE_INPUT | TEE_PARAM_TYPE_VALUE_INPUT << 4;
	operation.params[0].value.a = rdr_mem_addr;
	operation.params[0].value.b = rdr_mem_addr >> 32;
	operation.params[1].value.a = rdr_mem_len;

	smc_cmd.operation_phys = virt_to_phys(&operation);
	smc_cmd.operation_h_phys = 0;

        ret = TC_NS_SMC(&smc_cmd, 0, TEE_NO_CLIENT_TIMEOUT);
	if (ret) {
	    tloge("Send rdr mem info failed.\n");
	}

	return ret;
}

unsigned long TC_NS_get_rdr_mem_addr(void)
{

	return current_rdr_info.log_addr;
}

unsigned int TC_NS_get_rdr_mem_len(void)
{
	return current_rdr_info.log_len;
}
