#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_edma_balong_test.o := n
endif

obj-$(CONFIG_EDMA_SUPPORT)         += edma_balong.o
obj-$(CONFIG_ENABLE_TEST_CODE)        += edma_balong_test.o
