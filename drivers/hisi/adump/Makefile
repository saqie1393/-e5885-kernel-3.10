#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_adump_queue.o := n
endif

obj-y+= adump_area.o adump_field.o adump_queue.o adump_baseinfo.o adump_notifier.o adump_sche_record.o adump_lastkmsg.o adump_core.o adump_exception.o

