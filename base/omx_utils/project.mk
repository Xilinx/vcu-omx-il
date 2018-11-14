THIS.omx_utils:=$(call get-my-dir)

OMX_UTILS_SRCS+=\
	$(THIS.omx_utils)/logger.cpp\

UNITTESTS+=$(OMX_UTILS_SRCS)
UNITTESTS+=$(shell find $(THIS.omx_utils)/unittests -name "*.cpp")
