THIS.omx_module_dec:=$(call get-my-dir)

OMX_MODULE_DEC_SRCS+=\
	$(THIS.omx_module_dec)/omx_module_dec.cpp\
	$(THIS.omx_module_dec)/omx_device_dec_hardware_mcu.cpp\
	$(THIS.omx_module_dec)/omx_convert_module_to_soft_dec.cpp\
	$(THIS.omx_module_dec)/omx_convert_soft_to_module_dec.cpp\
