#
# Makefile for the modem drivers.
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifneq ($(strip $(CFG_ATE_VECTOR)),YES)
obj-$(CONFIG_SEC_CHECK) += sec/
obj-$(CONFIG_KDF_HASH) += kdf/
ifneq ($(strip $(CFG_CONFIG_EMMC_BOOT)),YES)
obj-$(CONFIG_BALONG_EARLY_FS) += balong_fs/
endif


ifneq ($(strip $(MBB_MULTI_CARRIER)), FEATURE_ON)
obj-$(CONFIG_MULTI_CARRIER)	+= multicarrier/
endif


obj-$(CONFIG_BALONG_RDR)+= rdr/
obj-y 	+= adump/
obj-y 	+= lpm3_agent/

# nv in modem drv
obj-y 	+= modem/drv/
obj-y 	+= hkadc/


#CMUX
ifeq ($(strip $(MBB_HSUART_CMUX)),FEATURE_ON)
obj-y  += cmux/
endif


obj-y 	+= hifi/
obj-y 	+= ddr/
obj-y 	+= edma/
obj-y	+= emi/
obj-y 	+= adp/

ifneq ($(MBB_USB_SOLUTION),FEATURE_ON)
obj-y   += spe/
else
ifeq ($(MBB_FACTORY),FEATURE_ON)
obj-y   += spe/
endif
endif

obj-y 	+= pss/
obj-y 	+= sdio_slave/
obj-y 	+= sci/
obj-y 	+= pinctrl/
obj-y 	+= pmu_new/
obj-y 	+= tzdriver/
obj-y	+= temperature/

ifeq ($(strip $(CFG_ENABLE_BUILD_AMON_SOC)),YES)
obj-y 	+= aximonitor/
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_VIC)),YES)
obj-y 	+= vic/
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
obj-y		+= busstress/
endif

ifeq ($(strip $(CFG_CONFIG_AT_UART)),YES)
obj-y 	+= uart_at/
endif

ifeq ($(strip $(CFG_CONFIG_CORESIGHT)),YES)
obj-y 	+= coresight/
endif
#HSUART_M2M
obj-y  += dialup_hsuart/

ifeq ($(strip $(CFG_CONFIG_ONCHIPROM_UART)),YES)
obj-y 	+= onchiprom_uart/
endif

obj-y			+= cpuidle/
obj-y			+= pm/

ifneq ($(strip $(OBB_SEPARATE)),true)
obj-$(CONFIG_BALONG_OAM_PS)  += modem/ps/ modem/oam/ modem/taf/
endif

ifeq ($(strip $(CFG_CONFIG_WATCHPOINT)),YES)
obj-$(CONFIG_HISI_WATCHPOINT) 	+= watchpoint/
endif

#subdir-ccflags-y := -Werror

else

obj-y 	+= modem/drv/
obj-y 	+= adp/
obj-y	+= ate/

endif
