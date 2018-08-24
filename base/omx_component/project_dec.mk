THIS.omx_component_dec:=$(call get-my-dir)

OMX_COMPONENT_DEC_SRCS+=\
	$(THIS.omx_component_dec)/omx_component_dec.cpp\

UNITTESTS+=$(OMX_COMPONENT_DEC_SRCS)
