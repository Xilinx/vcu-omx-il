THIS.omx_component_codec:=$(call get-my-dir)

OMX_COMPONENT_CODEC_SRCS+=\
	$(THIS.omx_component_codec)/omx_component_interface.cpp\
	$(THIS.omx_component_codec)/omx_component.cpp\
	$(THIS.omx_component_codec)/omx_convert_omx_media.cpp\
	$(THIS.omx_component_codec)/omx_buffer_handle.cpp\
	$(THIS.omx_component_codec)/omx_component_getset.cpp\
	$(THIS.omx_component_codec)/omx_expertise_interface.cpp\
	$(THIS.omx_component_codec)/omx_expertise_avc.cpp\
	$(THIS.omx_component_codec)/omx_expertise_hevc.cpp\

UNITTESTS+=$(shell find $(THIS.omx_component_codec)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_COMPONENT_CODEC_SRCS)
