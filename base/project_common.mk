THIS.base_common:=$(call get-my-dir)

-include $(THIS.base_common)/ref_alloc.mk

include $(THIS.base_common)/omx_checker/project.mk
include $(THIS.base_common)/omx_component/project_common.mk
include $(THIS.base_common)/omx_module/project_common.mk

OMX_COMMON_OBJ:=$(UTILITY_SRCS:%=$(BIN)/%.o)

OMX_COMMON_OBJ+=$(OMX_CHECKER_SRCS:%=$(BIN)/%.o)
OMX_COMMON_OBJ+=$(OMX_COMPONENT_COMMON_SRCS:%=$(BIN)/%.o)
OMX_COMMON_OBJ+=$(MODULE_COMMON_SRCS:%=$(BIN)/%.o)
OMX_COMMON_OBJ+=$(OMX_WRAPPER_COMMON_SRCS:%=$(BIN)/%.o)
