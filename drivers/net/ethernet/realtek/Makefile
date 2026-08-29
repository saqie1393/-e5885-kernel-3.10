#
# Makefile for the Realtek network device drivers.
#
ifeq ($(MBB_FACTORY),FEATURE_ON)
obj-y += r8111/
endif
obj-$(CONFIG_8139CP) += 8139cp.o
obj-$(CONFIG_8139TOO) += 8139too.o
obj-$(CONFIG_ATP) += atp.o
obj-$(CONFIG_R8169) += r8169.o
