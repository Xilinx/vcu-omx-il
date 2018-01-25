THIS.omx_module_common:=$(call get-my-dir)

OMX_MODULE_COMMON_SRCS+=\

UNITTESTS+=$(shell find $(THIS.omx_module_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_MODULE_COMMON_SRCS)
