THIS.omx_module_enc:=$(call get-my-dir)

OMX_MODULE_ENC_SRCS+=\
	$(THIS.omx_module_enc)/omx_module_enc.cpp\
	$(THIS.omx_module_enc)/omx_device_enc_interface.cpp\
	$(THIS.omx_module_enc)/omx_device_enc_hardware_mcu.cpp\
	$(THIS.omx_module_enc)/ROIMngr.cpp\
	$(THIS.omx_module_enc)/TwoPassMngr.cpp\
	$(THIS.omx_module_enc)/omx_convert_module_soft_roi.cpp\

ifeq ($(ENABLE_VCU),0)
  OMX_MODULE_ENC_SRCS+=\

endif

UNITTESTS+=$(OMX_MODULE_ENC_SRCS)
