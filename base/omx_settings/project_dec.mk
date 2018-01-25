THIS.omx_settings_dec:=$(call get-my-dir)

OMX_SETTINGS_DEC_SRCS+=\
	$(THIS.omx_settings_dec)/omx_settings_dec_avc.cpp\
	$(THIS.omx_settings_dec)/omx_settings_dec_hevc.cpp\
	$(THIS.omx_settings_dec)/omx_settings_dec_common.cpp\
	$(THIS.omx_settings_dec)/omx_convert_module_soft_dec.cpp\

UNITTESTS+=$(OMX_SETTINGS_DEC_SRCS)
