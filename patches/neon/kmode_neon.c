// SPDX-License-Identifier: GPL-2.0
/*
 * Private kernel-mode NEON for kernels built without CONFIG_KERNEL_MODE_NEON.
 * Same logic as kernel_neon_begin()/kernel_neon_end() of arch/arm/vfp/vfpmodule.c
 * in this very kernel tree, using vfp_current_hw_state and vfp_save_state found
 * through kallsyms_lookup_name(). Uniprocessor only.
 *
 * The in_interrupt() BUG_ON of the original is not kept: may_use_simd() below
 * refuses hard and soft irq context, and nothing else in this kernel touches
 * NEON from BH-disabled sections.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <asm/thread_info.h>
#include <asm/vfp.h>

#ifdef CONFIG_SMP
#error "kmode_neon.c supports uniprocessor kernels only"
#endif

#define awg_fmrx(_vfp_) ({ u32 __v; \
	asm("mrc p10, 7, %0, " #_vfp_ ", cr0, 0" : "=r" (__v) : : "cc"); __v; })
#define awg_fmxr(_vfp_, _var_) \
	asm("mcr p10, 7, %0, " #_vfp_ ", cr0, 0" : : "r" (_var_) : "cc")

static bool neon = true;
module_param(neon, bool, 0444);
MODULE_PARM_DESC(neon, "Use kernel-mode NEON for crypto (default: on)");

bool awg_kmode_neon_enabled __read_mostly;

static union vfp_state **awg_vfp_hw_state __read_mostly;
static void (*awg_vfp_save_state)(void *location, u32 fpexc) __read_mostly;

void kernel_neon_begin(void)
{
	struct thread_info *thread = current_thread_info();
	union vfp_state **hw;
	unsigned int cpu;
	u32 fpexc;

	cpu = get_cpu();
	hw = &awg_vfp_hw_state[cpu];

	fpexc = awg_fmrx(cr8) | FPEXC_EN;
	awg_fmxr(cr8, fpexc);

	/* Save the userland NEON/VFP state; under UP the owner may be another task. */
	if (*hw == &thread->vfpstate)
		awg_vfp_save_state(&thread->vfpstate, fpexc);
	else if (*hw != NULL)
		awg_vfp_save_state(*hw, fpexc);
	*hw = NULL;
}

void kernel_neon_end(void)
{
	awg_fmxr(cr8, awg_fmrx(cr8) & ~FPEXC_EN);
	put_cpu();
}

void __init awg_kmode_neon_init(void)
{
	unsigned long hw, save;

	if (!neon) {
		pr_info("kernel-mode NEON: off (neon=0)\n");
		return;
	}
	if (!(elf_hwcap & HWCAP_NEON)) {
		pr_info("kernel-mode NEON: off (no NEON in hwcap)\n");
		return;
	}
	hw = kallsyms_lookup_name("vfp_current_hw_state");
	save = kallsyms_lookup_name("vfp_save_state");
	if (!hw || !save) {
		pr_warn("kernel-mode NEON: off (vfp symbols not found)\n");
		return;
	}
	awg_vfp_hw_state = (union vfp_state **)hw;
	awg_vfp_save_state = (void (*)(void *, u32))save;
	awg_kmode_neon_enabled = true;
	pr_info("kernel-mode NEON: on\n");
}
