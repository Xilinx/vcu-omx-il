THIS.omx_codec_dec:=$(call get-my-dir)

OMX_CODEC_DEC_SRCS+=\
	$(THIS.omx_codec_dec)/omx_codec_dec.cpp\
	$(THIS.omx_codec_dec)/omx_expertise_dec_avc.cpp\
	$(THIS.omx_codec_dec)/omx_expertise_dec_hevc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_omx_to_module_dec.cpp\
	$(THIS.omx_codec_enc)/omx_convert_module_to_omx_dec.cpp\

UNITTESTS+=$(OMX_CODEC_DEC_SRCS)
