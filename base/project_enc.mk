THIS.base_enc:=$(call get-my-dir)

EXTERNAL_ENCODE_LIB_NAME:=liballegro_encode.so
LIB_OMX_ENC_NAME:=libOMX.allegro.video_encoder.so
LIB_OMX_ENC:=$(BIN)/$(LIB_OMX_ENC_NAME)
LIBS_ENCODE:=

include $(THIS.base_enc)/encoder_version.mk
include $(THIS.base_enc)/omx_component/project_enc.mk
include $(THIS.base_enc)/omx_module/project_enc.mk
include $(THIS.base_enc)/omx_wrapper/project_enc.mk

OMX_ENC_OBJ:=$(OMX_COMMON_OBJ)
OMX_ENC_OBJ+=$(OMX_COMPONENT_ENC_SRCS:%=$(BIN)/%.o)
OMX_ENC_OBJ+=$(MODULE_ENC_SRCS:%=$(BIN)/%.o)
OMX_ENC_OBJ+=$(OMX_WRAPPER_ENC_SRCS:%=$(BIN)/%.o)

OMX_ENC_CFLAGS:=$(DEFAULT_CFLAGS)
OMX_ENC_CFLAGS+=-fPIC
OMX_ENC_CFLAGS+=-pthread
OMX_ENC_LDFLAGS:=$(DEFAULT_LDFLAGS)
OMX_ENC_LDFLAGS+=-lpthread

-include $(THIS.base_enc)/ref_enc.mk

ifdef EXTERNAL_LIB
LIB_ENCODE:=$(EXTERNAL_LIB)/$(EXTERNAL_ENCODE_LIB_NAME)
LIBS_ENCODE+=$(LIB_ENCODE)

$(LIB_ENCODE):
ifndef EXTERNAL_SRC
	$(error EXTERNAL_SRC must be set to build $(EXTERNAL_ENCODE_LIB_NAME))
endif
	ENABLE_64BIT=$(ENABLE_64BIT) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG=$(EXTERNAL_CONFIG) \
	BIN=$(EXTERNAL_LIB) \
	$(MAKE) -C $(EXTERNAL_SRC) liballegro_encode_dll

OMX_ENC_LDFLAGS+=-L$(EXTERNAL_LIB)
endif

OMX_ENC_LDFLAGS+=-l$(EXTERNAL_ENCODE_LIB_NAME:lib%.so=%)

$(LIB_OMX_ENC): $(LIBS_ENCODE)
$(LIB_OMX_ENC): $(OMX_ENC_OBJ)
$(LIB_OMX_ENC): CFLAGS:=$(OMX_ENC_CFLAGS)
$(LIB_OMX_ENC): LDFLAGS:=$(OMX_ENC_LDFLAGS)
$(LIB_OMX_ENC): MAJOR:=$(ENC_MAJOR)
$(LIB_OMX_ENC): VERSION:=$(ENC_VERSION)

encode: $(LIB_OMX_ENC)

.PHONY: encode
TARGETS+=encode
