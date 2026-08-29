#
# Makefile for the input core drivers.
#

# Each configuration option enables a list of files.
#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_temperature_debug.o := n
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_TEMPERATURE_PROTECT)),YES)
obj-y		+= temperature_balong.o adp_temperature.o temperature_debug.o
obj-$(CONFIG_TEMPERATURE_SYS_BALONG)	+= tempsys_device.o
else
obj-y		+= temperature_stub.o
endif



