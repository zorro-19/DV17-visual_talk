objs += \
$(ROOT)/apps/debug.o \
$(ROOT)/apps/flash_write_protect.o \
$(ROOT)/apps/fontinit.o \
$(ROOT)/apps/uart2sd.o \
$(ROOT)/apps/cpp_run_init.o \
$(ROOT)/apps/version.o \

objs_cxx += \
$(ROOT)/apps/cxx_runtime.o \



include $(ROOT)/cpu/$(CPU)/Makefile

include $(ROOT)/camera/Makefile

include $(ROOT)/apps/$(APP_CASE)/Makefile

GEN_LIB=n

-include $(MAKE_RULE)
