THIS.module_enc:=$(call get-my-dir)

MODULE_ENC_SRCS+=\
                 $(THIS.module_enc)/mediatype_enc_avc.cpp\
                 $(THIS.module_enc)/mediatype_enc_hevc.cpp\
                 $(THIS.module_enc)/mediatype_enc_common.cpp\
                 $(THIS.module_enc)/convert_module_soft_enc.cpp\
                 $(THIS.module_enc)/module_enc.cpp\
                 $(THIS.module_enc)/memory_interface.cpp\
                 $(THIS.module_enc)/dma_memory.cpp\
                 $(THIS.module_enc)/cpp_memory.cpp\
                 $(THIS.module_enc)/device_enc_interface.cpp\
                 $(THIS.module_enc)/device_enc_hardware_mcu.cpp\
                 $(THIS.module_enc)/ROIMngr.cpp\
                 $(THIS.module_enc)/TwoPassMngr.cpp\
                 $(THIS.module_enc)/convert_module_soft_roi.cpp\
                 $(THIS.module_enc)/sync_ip_enc.cpp\

ifeq ($(ENABLE_VCU),0)
  MODULE_ENC_SRCS+=\

endif

UNITTESTS+=$(MODULE_ENC_SRCS)
