THIS.omx_codec_enc:=$(call get-my-dir)

OMX_CODEC_ENC_SRCS+=\
	$(THIS.omx_codec_enc)/omx_codec_enc.cpp\
	$(THIS.omx_codec_enc)/omx_expertise_enc_avc.cpp\
	$(THIS.omx_codec_enc)/omx_expertise_enc_hevc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_module_to_omx_enc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_module_to_omx_enc_avc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_module_to_omx_enc_hevc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_omx_to_module_enc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_omx_to_module_enc_avc.cpp\
	$(THIS.omx_codec_enc)/omx_convert_omx_to_module_enc_hevc.cpp\


