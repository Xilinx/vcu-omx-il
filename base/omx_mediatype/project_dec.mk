THIS.omx_mediatype_dec:=$(call get-my-dir)

OMX_MEDIATYPE_DEC_SRCS+=\
	$(THIS.omx_mediatype_dec)/omx_mediatype_dec_avc.cpp\
	$(THIS.omx_mediatype_dec)/omx_mediatype_dec_hevc.cpp\
	$(THIS.omx_mediatype_dec)/omx_mediatype_dec_common.cpp\
	$(THIS.omx_mediatype_dec)/omx_convert_module_soft_dec.cpp\

UNITTESTS+=$(OMX_MEDIATYPE_DEC_SRCS)
