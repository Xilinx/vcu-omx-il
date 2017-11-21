THIS.omx_module_common:=$(call get-my-dir)

OMX_MODULE_COMMON_SRCS+=\
	$(THIS.omx_module_common)/omx_convert_module_to_soft.cpp\
	$(THIS.omx_module_common)/omx_convert_soft_to_module.cpp\
