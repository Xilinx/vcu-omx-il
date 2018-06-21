THIS.omx_module_dec:=$(call get-my-dir)

OMX_MODULE_DEC_SRCS+=\
	$(THIS.omx_module_dec)/omx_module_dec.cpp\
	$(THIS.omx_module_dec)/omx_device_dec_interface.cpp\
	$(THIS.omx_module_dec)/omx_device_dec_hardware_mcu.cpp\

UNITTESTS+=$(OMX_MODULE_DEC_SRCS)
