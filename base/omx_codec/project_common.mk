THIS.omx_codec_common:=$(call get-my-dir)

OMX_CODEC_COMMON_SRCS+=\
	$(THIS.omx_codec_common)/omx_codec.cpp\
	$(THIS.omx_codec_common)/omx_convert_omx_module.cpp\
	$(THIS.omx_codec_common)/omx_buffer_handle.cpp\

UNITTESTS+=$(shell find $(THIS.omx_codec_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_CODEC_COMMON_SRCS)

