THIS.omx_mediatype_common:=$(call get-my-dir)

OMX_MEDIATYPE_COMMON_SRCS+=\
			   $(THIS.omx_mediatype_common)/omx_convert_module_soft.cpp\
			   $(THIS.omx_mediatype_common)/omx_convert_module_soft_avc.cpp\
			   $(THIS.omx_mediatype_common)/omx_convert_module_soft_hevc.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_interface.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_checks.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_common.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_common_avc.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_common_hevc.cpp\
                           $(THIS.omx_mediatype_common)/omx_mediatype_dummy.cpp\

UNITTESTS+=$(shell find $(THIS.omx_mediatype_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_MEDIATYPE_COMMON_SRCS)
