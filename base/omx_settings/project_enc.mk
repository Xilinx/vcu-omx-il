THIS.omx_settings_enc:=$(call get-my-dir)

OMX_SETTINGS_ENC_SRCS+=\
	$(THIS.omx_settings_enc)/omx_settings_enc_avc.cpp\
	$(THIS.omx_settings_enc)/omx_settings_enc_hevc.cpp\
	$(THIS.omx_settings_enc)/omx_settings_enc_common.cpp\
	$(THIS.omx_settings_enc)/omx_convert_module_soft_enc.cpp\

UNITTESTS+=$(OMX_SETTINGS_ENC_SRCS)
