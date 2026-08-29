#
# Makefile for the linux kernel.
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_pm_debug.o := n
endif

## PM ##
obj-$(CONFIG_BALONG_PM_SUSPEND) += pm.o pm_debug.o
obj-$(CONFIG_BALONG_PM_SUSPEND) += pm_asm_deepsleep.o

ifeq ($(strip $(CFG_BSP_CONFIG_BOARD_TELEMATIC)),YES)
obj-$(CONFIG_BALONG_PM_SUSPEND) += mbb_ios_pd.o
endif

