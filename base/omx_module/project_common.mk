THIS.omx_module_common:=$(call get-my-dir)

OMX_MODULE_COMMON_SRCS+=\
	$(THIS.omx_module_common)/omx_convert_module_soft.cpp\

UNITTESTS+=$(shell find $(THIS.omx_module_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_MODULE_COMMON_SRCS)
