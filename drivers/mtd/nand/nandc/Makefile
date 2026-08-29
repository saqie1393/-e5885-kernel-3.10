ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk


obj-$(CONFIG_MTD_NAND_BALONG_NANDC)    += nandc_mtd.o  \
	nandc_ctrl.o nandc_v600.o  flash_huawei_dload.o


