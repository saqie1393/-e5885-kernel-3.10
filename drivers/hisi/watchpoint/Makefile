#
# created by y000184236 for k3v3 fpga
#
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-y           += watchpoint.o


subdir-ccflags-y := -Werror
