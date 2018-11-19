THIS.base_dec:=$(call get-my-dir)

EXTERNAL_DECODE_LIB_NAME:=liballegro_decode.so
LIB_OMX_DEC_NAME:=libOMX.allegro.video_decoder.so
LIB_OMX_DEC:=$(BIN)/$(LIB_OMX_DEC_NAME)
LIBS_DECODE:=

include $(THIS.base_dec)/decoder_version.mk
include $(THIS.base_dec)/omx_component/project_dec.mk
include $(THIS.base_dec)/omx_mediatype/project_dec.mk
include $(THIS.base_dec)/omx_module/project_dec.mk
include $(THIS.base_dec)/omx_wrapper/project_dec.mk

OMX_DEC_OBJ:=$(OMX_COMMON_OBJ)
OMX_DEC_OBJ+=$(OMX_COMPONENT_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_MEDIATYPE_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_MODULE_DEC_SRCS:%=$(BIN)/%.o)
OMX_DEC_OBJ+=$(OMX_WRAPPER_DEC_SRCS:%=$(BIN)/%.o)

OMX_DEC_CFLAGS:=$(DEFAULT_CFLAGS)
OMX_DEC_CFLAGS+=-fPIC
OMX_DEC_CFLAGS+=-pthread
OMX_DEC_LDFLAGS:=$(DEFAULT_LDFLAGS)
OMX_DEC_LDFLAGS+=-lpthread

ifdef EXTERNAL_LIB
LIB_DECODE:=$(EXTERNAL_LIB)/$(EXTERNAL_DECODE_LIB_NAME)
LIBS_DECODE+=$(LIB_DECODE)
-include $(THIS.base_dec)/ref_dec.mk

$(LIB_DECODE):
	ENABLE_64BIT=$(ENABLE_64BIT) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG=$(EXTERNAL_CONFIG) \
	BIN=$(EXTERNAL_LIB) \
	$(MAKE) -C $(EXTERNAL_SRC) liballegro_decode_dll

OMX_DEC_LDFLAGS+=-L$(EXTERNAL_LIB)
endif

OMX_DEC_LDFLAGS+=-l$(EXTERNAL_DECODE_LIB_NAME:lib%.so=%)

$(LIB_OMX_DEC): $(LIBS_DECODE)
$(LIB_OMX_DEC): $(OMX_DEC_OBJ)
$(LIB_OMX_DEC): CFLAGS:=$(OMX_DEC_CFLAGS)
$(LIB_OMX_DEC): LDFLAGS:=$(OMX_DEC_LDFLAGS)
$(LIB_OMX_DEC): MAJOR:=$(DEC_MAJOR)
$(LIB_OMX_DEC): VERSION:=$(DEC_VERSION)

decode: $(LIB_OMX_DEC)

.PHONY: decode
TARGETS+=decode

UNITTESTS+=$(LIBS_DECODE)
