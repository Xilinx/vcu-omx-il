THIS.exe_omx_common:=$(call get-my-dir)

include $(THIS.exe_omx_common)/common/project.mk

EXE_OMX_COMMON_OBJ:=$(EXE_OMX_COMMON_SRCS:%=$(BIN)/%.o)
