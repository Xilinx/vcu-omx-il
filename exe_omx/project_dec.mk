THIS.exe_omx_dec:=$(call get-my-dir)

EXE_NAME_DEC:=omx_decoder.exe
SH_NAME_DEC:=$(EXE_NAME_DEC:%.exe=%.sh)

include $(THIS.exe_omx_dec)/decoder/project_dec.mk

EXE_OMX_DECODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_DECODER_OBJ+=$(EXE_OMX_DECODER_SRCS:%=$(BIN)/%.o)

EXE_DEC_CFLAGS:=$(DEFAULT_CFLAGS)
EXE_DEC_CFLAGS+=-fPIC
EXE_DEC_CFLAGS+=-pthread

EXE_DEC_LDFLAGS:=$(DEFAULT_LDFLAGS)
EXE_DEC_LDFLAGS+=-lpthread
EXE_DEC_LDFLAGS+=-L$(BIN)
EXE_DEC_LDFLAGS+=-l$(LIB_OMX_DEC_NAME:lib%.so=%)
EXE_DEC_LDFLAGS+=-l$(LIB_OMX_CORE_NAME:lib%.so=%)
ifdef EXTERNAL_LIB
EXE_DEC_LDFLAGS+=-L$(EXTERNAL_LIB)
endif
EXE_DEC_LDFLAGS+=-l$(EXTERNAL_DECODE_LIB_NAME:lib%.so=%)

-include $(THIS.exe_omx_dec)/ref_dec.mk

$(BIN)/$(EXE_NAME_DEC): $(LIB_OMX_CORE)
$(BIN)/$(EXE_NAME_DEC): $(LIB_OMX_DEC)
$(BIN)/$(EXE_NAME_DEC): $(EXE_OMX_DECODER_OBJ)
$(BIN)/$(EXE_NAME_DEC): CFLAGS:=$(EXE_DEC_CFLAGS)
$(BIN)/$(EXE_NAME_DEC): LDFLAGS:=$(EXE_DEC_LDFLAGS)

$(BIN)/$(SH_NAME_DEC): $(BIN)/$(EXE_NAME_DEC)
	@echo "Generate script to launch $^"
	$(shell echo 'BIN_PATH=$$(dirname $$(realpath "$$0"))' >> $@)
	$(shell echo 'export OMX_ALLEGRO_PATH="$$BIN_PATH"' >> $@)
	$(shell echo 'export LD_LIBRARY_PATH="$$BIN_PATH:$(EXTERNAL_LIB)"' >> $@)
	$(shell echo '"$$BIN_PATH/$(EXE_NAME_DEC)" "$$@"' >> $@)
	$(shell chmod a+x $@)

omx_decoder: $(BIN)/$(EXE_NAME_DEC) $(BIN)/$(SH_NAME_DEC)

.PHONY: omx_decoder
TARGETS+=omx_decoder

