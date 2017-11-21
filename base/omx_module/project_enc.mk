THIS.omx_module_enc:=$(call get-my-dir)

OMX_MODULE_ENC_SRCS+=\
	$(THIS.omx_module_enc)/omx_module_enc.cpp\
	$(THIS.omx_module_enc)/omx_device_enc_hardware_mcu.cpp\
	$(THIS.omx_module_enc)/omx_convert_module_to_soft_enc.cpp\
	$(THIS.omx_module_enc)/omx_convert_soft_to_module_enc.cpp\
