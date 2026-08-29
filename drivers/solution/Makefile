-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-$(CONFIG_FLIGHT_MODE)  += huawei_flight_mode/

ifeq ($(strip $(MBB_MULTI_CARRIER)), FEATURE_ON)
obj-y     += multi_carrier/
endif

ifeq ($(strip $(MBB_FEATURE_GPS)), FEATURE_ON)
obj-y     += gps/
endif

ifeq ($(strip $(MBB_FEATURE_BODYSAR)),FEATURE_ON)
obj-y     += bodysar/  
endif

ifeq ($(strip $(MBB_CLIENT_CODEC)),FEATURE_ON)
obj-y	+= codecdrv/
endif

ifeq ($(strip $(MBB_MODULE_PM)),FEATURE_ON)
obj-y	+=  huawei_pm/
endif

ifeq ($(strip $(CFG_BSP_CONFIG_BOARD_SOLUTION)),YES)
obj-y	+= simlockdrv/  gpio_ctrl/ 
endif

ifeq ($(strip $(MBB_CARRIER_ADAPTER)),FEATURE_ON)
obj-y    += carrier_adapter/
endif

subdir-ccflags-y := -Werror
