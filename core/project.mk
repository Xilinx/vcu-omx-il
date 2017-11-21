THIS.core:=$(call get-my-dir)

include $(THIS.core)/core_version.mk
LIB_OMX_CORE=$(BIN)/libOMX.allegro.core.so

include $(THIS.core)/omx_core/project.mk

OMX_CORE_OBJ:=$(OMX_CORE_SRCS:%=$(BIN)/%.o)

$(LIB_OMX_CORE): $(OMX_CORE_OBJ)
$(LIB_OMX_CORE): CFLAGS+=-fPIC
$(LIB_OMX_CORE): CLFAGS+=-pthread
$(LIB_OMX_CORE): LDFLAGS+=-ldl
$(LIB_OMX_CORE): LDFLAGS+=-lpthread
$(LIB_OMX_CORE): MAJOR:=$(CORE_MAJOR)
$(LIB_OMX_CORE): VERSION:=$(CORE_VERSION)

core: $(LIB_OMX_CORE)

.PHONY:core
TARGETS+=core




