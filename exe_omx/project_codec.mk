THIS.exe_omx_codec:=$(call get-my-dir)

include $(THIS.exe_omx_codec)/common/project.mk

EXE_OMX_CODEC_OBJ:=$(EXE_OMX_CODEC_SRCS:%=$(BIN)/%.o)
