THIS.module_dec:=$(call get-my-dir)

MODULE_DEC_SRCS+=\
                     $(THIS.module_dec)/mediatype_dec_avc.cpp\
                     $(THIS.module_dec)/mediatype_dec_hevc.cpp\
                     $(THIS.module_dec)/mediatype_dec_common.cpp\
                     $(THIS.module_dec)/convert_module_soft_dec.cpp\
                     $(THIS.module_dec)/module_dec.cpp\
                     $(THIS.module_dec)/device_dec_interface.cpp\
                     $(THIS.module_dec)/device_dec_hardware_mcu.cpp\
                     $(THIS.module_dec)/sync_ip_dec.cpp\

ifeq ($(ENABLE_VCU),0)
  MODULE_DEC_SRCS+=\

endif

UNITTESTS+=$(MODULE_DEC_SRCS)
