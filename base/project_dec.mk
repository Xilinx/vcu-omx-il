THIS.base_dec:=$(call get-my-dir)

include $(THIS.base_dec)/decoder_version.mk
LIB_OMX_DEC=$(BIN)/libOMX.allegro.video_decoder.so

EXTERNAL_DECODE_LIB_NAME=liballegro_decode
LIB_DECODE=$(EXTERNAL_LIB)/$(EXTERNAL_DECODE_LIB_NAME).so

LIBS_DECODE:=$(LIB_DECODE)
-include $(THIS.base_dec)/ref_dec.mk

include $(THIS.base_dec)/omx_codec/project_dec.mk
include $(THIS.base_dec)/omx_mediatype/project_dec.mk
include $(THIS.base_dec)/omx_module/project_dec.mk
include $(THIS.base_dec)/omx_wrapper/project_dec.mk

OMX_DEC_OBJ:=$(OMX_COMMON_OBJ)
OMX_DEC_OBJ+=$(OMX_CODEC_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_MEDIATYPE_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_MODULE_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_WRAPPER_DEC_SRCS:%=$(BIN)/%.o)

$(LIB_DECODE):
	ENABLE_64BIT=$(ENABLE_64BIT) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG=$(EXTERNAL_CONFIG) \
	BIN=$(EXTERNAL_LIB) \
	$(MAKE) -C $(EXTERNAL_CTRLSW) liballegro_decode_dll

$(LIB_OMX_DEC): $(OMX_DEC_OBJ) $(LIBS_DECODE)

$(LIB_OMX_DEC): CFLAGS+=-L$(EXTERNAL_LIB)
$(LIB_OMX_DEC): CFLAGS+=-fPIC
$(LIB_OMX_DEC): LDFLAGS+=-l$(EXTERNAL_DECODE_LIB_NAME:lib%=%)
$(LIB_OMX_DEC): MAJOR:=$(DEC_MAJOR)
$(LIB_OMX_DEC): VERSION:=$(DEC_VERSION)

decode: $(LIB_OMX_DEC)

.PHONY: decode
TARGETS+=decode


