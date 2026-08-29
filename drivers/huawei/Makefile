-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(MBB_MLOG)),FEATURE_ON)
obj-y	+= mlogdrv/
endif
ifeq ($(strip $(MBB_DLOAD)),FEATURE_ON)
obj-y     += dload_com/
endif

ifeq ($(strip $(MBB_AGING_TEST)),FEATURE_ON)
obj-y	+= aging_test/
endif

ifeq ($(strip $(MBB_FEATURE_GPS_UBLOX)),FEATURE_ON)
obj-y    += ublox_gps/
endif
