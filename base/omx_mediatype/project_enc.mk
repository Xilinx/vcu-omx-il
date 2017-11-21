THIS.omx_mediatype_enc:=$(call get-my-dir)

OMX_MEDIATYPE_ENC_SRCS+=\
	$(THIS.omx_mediatype_enc)/omx_mediatype_enc_avc.cpp\
	$(THIS.omx_mediatype_enc)/omx_mediatype_enc_hevc.cpp\
