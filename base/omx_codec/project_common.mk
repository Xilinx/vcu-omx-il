THIS.omx_codec_common:=$(call get-my-dir)

OMX_CODEC_COMMON_SRCS+=\
	$(THIS.omx_codec_common)/omx_codec.cpp\
	$(THIS.omx_codec_common)/omx_convert_module_to_omx.cpp\
	$(THIS.omx_codec_common)/omx_convert_module_to_omx_avc.cpp\
	$(THIS.omx_codec_common)/omx_convert_module_to_omx_hevc.cpp\
	$(THIS.omx_codec_common)/omx_convert_omx_to_module.cpp\
	$(THIS.omx_codec_common)/omx_convert_omx_to_module_avc.cpp\
	$(THIS.omx_codec_common)/omx_convert_omx_to_module_hevc.cpp\

UNITTESTS+=$(shell find $(THIS.omx_codec_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_CODEC_COMMON_SRCS)

