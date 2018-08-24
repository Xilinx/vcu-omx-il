THIS.omx_component_enc:=$(call get-my-dir)

OMX_COMPONENT_ENC_SRCS+=\
	$(THIS.omx_component_enc)/omx_component_enc.cpp\

UNITTESTS+=$(OMX_COMPONENT_ENC_SRCS)
