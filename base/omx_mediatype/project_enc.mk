THIS.omx_mediatype_enc:=$(call get-my-dir)

OMX_MEDIATYPE_ENC_SRCS+=\
	$(THIS.omx_mediatype_enc)/omx_mediatype_enc_avc.cpp\
	$(THIS.omx_mediatype_enc)/omx_mediatype_enc_hevc.cpp\
	$(THIS.omx_mediatype_enc)/omx_mediatype_enc_common.cpp\
	$(THIS.omx_mediatype_enc)/omx_convert_module_soft_enc.cpp\

UNITTESTS+=$(OMX_MEDIATYPE_ENC_SRCS)
