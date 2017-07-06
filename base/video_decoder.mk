#------------------------------------------------------------------------------
# Video Decoder Component
#------------------------------------------------------------------------------
BASE_DIR?=.

include $(BASE_DIR)/video_def.mk
include $(BASE_DIR)/decoder_version.mk

LIB_DECODE_A=$(EXTERNAL_LIB_DIR)/bin/liballegro_decode.a
LIB_OMX_DECODE_SO=$(BIN)/libOMX.allegro.video_decoder.so.$(DEC_VERSION)
LIBS_OMX_DECODE+=$(LIB_DECODE_A)

-include $(BASE_DIR)/ref_dec.mk

$(LIB_OMX_DECODE_SO):
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(LDFLAGS) $(INCLUDES) -shared -Wl,-soname,libOMX.allegro.video_decoder.so.$(DEC_MAJOR) -o "$@" $^ $(LIBS)
	@echo "LD $@"
	rm -f "$(BIN)/libOMX.allegro.video_decoder.so.$(DEC_MAJOR)"
	ln -s "libOMX.allegro.video_decoder.so.$(DEC_VERSION)" "$(BIN)/libOMX.allegro.video_decoder.so.$(DEC_MAJOR)"

TARGETS_OMX_DEC+=$(LIB_OMX_DECODE_SO)

$(LIB_DECODE_A):
	ENABLE_64BIT=$(ENABLE_64BIT) \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	ENABLE_STATIC=$(STATIC) \
	$(MAKE) -C $(EXTERNAL_LIB_DIR) liballegro_decode

LIB_OMX_DECODE_SRC:=\
	$(BASE_DIR)/omx_port/omx_port.cpp\
	$(BASE_DIR)/omx_port/omx_buffer_manager.cpp\
	$(BASE_DIR)/omx_wrapper/omx_wrapper.cpp\
	$(BASE_DIR)/omx_wrapper/omx_dec_wrapper.cpp\
	$(BASE_DIR)/omx_checker/omx_checker.cpp\
	$(BASE_DIR)/omx_videocodec/omx_videocodec.cpp\
	$(BASE_DIR)/omx_processtype/omx_process.cpp\
	$(BASE_DIR)/omx_processtype/omx_dec_process.cpp\
	$(BASE_DIR)/omx_processtype/omx_dec_ipdevice.cpp\
	$(BASE_DIR)/omx_buffer/omx_buffer.cpp\
	$(BASE_DIR)/omx_buffer/omx_buffer_meta.cpp\
	$(BASE_DIR)/omx_codectype/omx_hevc_codec.cpp\
	$(BASE_DIR)/omx_codectype/omx_avc_codec.cpp\

LIB_OMX_DECODE_OBJ:=$(LIB_OMX_DECODE_SRC:%.cpp=$(BIN)/%.cpp.o)

$(LIB_OMX_DECODE_SO): $(LIB_OMX_DECODE_OBJ) $(LIBS_OMX_DECODE)

TARGETS_OMX_DEC+=$(LIB_OMX_DECODE_SO)

lib_omx_decoder: LIBS:= $(LIBS) -lpthread
lib_omx_decoder: $(LIB_DECODE_A) $(TARGETS_OMX_DEC)

TARGETS+=lib_omx_decoder
