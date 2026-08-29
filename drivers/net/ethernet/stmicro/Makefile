#
# Makefile for the STMicroelectronics device drivers.
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-$(CONFIG_STMMAC_ETH) += stmmac/
ifeq ($(MBB_FEATURE_ETH),FEATURE_ON)
obj-$(CONFIG_NEW_STMMAC_ETH) += stmmac_avb/
endif
