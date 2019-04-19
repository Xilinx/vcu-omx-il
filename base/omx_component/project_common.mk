THIS.omx_component_common:=$(call get-my-dir)

OMX_COMPONENT_COMMON_SRCS+=\
	$(THIS.omx_component_common)/omx_component_interface.cpp\
	$(THIS.omx_component_common)/omx_component.cpp\
	$(THIS.omx_component_common)/omx_convert_omx_media.cpp\
	$(THIS.omx_component_common)/omx_buffer_handle.cpp\
	$(THIS.omx_component_common)/omx_component_getset.cpp\
	$(THIS.omx_component_common)/omx_expertise_interface.cpp\
	$(THIS.omx_component_common)/omx_expertise_avc.cpp\
	$(THIS.omx_component_common)/omx_expertise_hevc.cpp\

UNITTESTS+=$(shell find $(THIS.omx_component_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_COMPONENT_COMMON_SRCS)

