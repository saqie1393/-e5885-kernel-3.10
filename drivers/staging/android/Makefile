-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
ccflags-y += -I$(src)			# needed for trace events

obj-y					+= ion/
obj-$(CONFIG_FIQ_DEBUGGER)		+= fiq_debugger/

obj-$(CONFIG_ASHMEM)			+= ashmem.o
ifeq ($(strip $(MBB_FEATURE_LOGCAT)),FEATURE_ON)
obj-$(CONFIG_ANDROID_LOGGER)	+= logger.o
endif
obj-$(CONFIG_ANDROID_TIMED_OUTPUT)	+= timed_output.o
obj-$(CONFIG_ANDROID_TIMED_GPIO)	+= timed_gpio.o
obj-$(CONFIG_ANDROID_LOW_MEMORY_KILLER)	+= lowmemorykiller.o
obj-$(CONFIG_SYNC)			+= sync.o
obj-$(CONFIG_SW_SYNC)			+= sw_sync.o
