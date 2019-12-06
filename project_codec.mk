THIS.base_codec:=$(call get-my-dir)

-include $(THIS.base_codec)/ref_alloc.mk

include $(THIS.base_codec)/module/project_codec.mk
include $(THIS.base_codec)/base/omx_checker/project.mk
include $(THIS.base_codec)/base/omx_component/project_codec.mk

OMX_CODEC_OBJ:=$(UTILITY_SRCS:%=$(BIN)/%.o)

OMX_CODEC_OBJ+=$(OMX_CHECKER_SRCS:%=$(BIN)/%.o)
OMX_CODEC_OBJ+=$(OMX_COMPONENT_CODEC_SRCS:%=$(BIN)/%.o)
OMX_CODEC_OBJ+=$(MODULE_CODEC_SRCS:%=$(BIN)/%.o)
OMX_CODEC_OBJ+=$(OMX_WRAPPER_CODEC_SRCS:%=$(BIN)/%.o)
