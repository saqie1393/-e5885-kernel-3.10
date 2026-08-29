#
# Makefile for the linux kernel.
#
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

obj-$(CONFIG_HAVE_CPU_IDLE)+=  ladder.o
obj-$(CONFIG_HAVE_CPU_IDLE)+=  cpuidle_balong.o
obj-$(CONFIG_HAVE_CPU_IDLE)+=  sleep_balong.o
