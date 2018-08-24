THIS.base_enc:=$(call get-my-dir)

include $(THIS.base_enc)/encoder_version.mk
LIB_OMX_ENC=$(BIN)/libOMX.allegro.video_encoder.so

EXTERNAL_ENCODE_LIB_NAME=liballegro_encode
LIB_ENCODE=$(EXTERNAL_LIB)/$(EXTERNAL_ENCODE_LIB_NAME).so

LIBS_ENCODE:=$(LIB_ENCODE)
-include $(THIS.base_enc)/ref_enc.mk

include $(THIS.base_enc)/omx_component/project_enc.mk
include $(THIS.base_enc)/omx_mediatype/project_enc.mk
include $(THIS.base_enc)/omx_module/project_enc.mk
include $(THIS.base_enc)/omx_wrapper/project_enc.mk

OMX_ENC_OBJ:=$(OMX_COMMON_OBJ)
OMX_ENC_OBJ+=$(OMX_COMPONENT_ENC_SRCS:%=$(BIN)/%.o)
OMX_ENC_OBJ+=$(OMX_MEDIATYPE_ENC_SRCS:%=$(BIN)/%.o)
OMX_ENC_OBJ+=$(OMX_MODULE_ENC_SRCS:%=$(BIN)/%.o)
OMX_ENC_OBJ+=$(OMX_WRAPPER_ENC_SRCS:%=$(BIN)/%.o)

ifneq ($(LINK_SHARED_CTRLSW), 1)
$(LIB_ENCODE):
	ENABLE_64BIT=$(ENABLE_64BIT) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG=$(EXTERNAL_CONFIG) \
	BIN=$(EXTERNAL_LIB) \
	$(MAKE) -C $(EXTERNAL_CTRLSW) liballegro_encode_dll

$(LIB_OMX_ENC): $(OMX_ENC_OBJ) $(LIBS_ENCODE)
$(LIB_OMX_ENC): CFLAGS+=-L$(EXTERNAL_LIB)
else
$(LIB_OMX_ENC): $(OMX_ENC_OBJ)
endif

UNITTESTS+=$(LIBS_ENCODE)

$(LIB_OMX_ENC): CFLAGS+=-fPIC
$(LIB_OMX_ENC): CFLAGS+=-pthread
$(LIB_OMX_ENC): LDFLAGS+=-l$(EXTERNAL_ENCODE_LIB_NAME:lib%=%)
$(LIB_OMX_ENC): LDFLAGS+=-lpthread
$(LIB_OMX_ENC): MAJOR:=$(ENC_MAJOR)
$(LIB_OMX_ENC): VERSION:=$(ENC_VERSION)

encode: $(LIB_OMX_ENC)

.PHONY: encode
TARGETS+=encode


