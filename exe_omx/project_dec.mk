THIS.exe_omx_dec:=$(call get-my-dir)

EXE_NAME_DEC:=omx_decoder.exe
SH_NAME_DEC:=$(EXE_NAME_DEC:%.exe=%.sh)

include $(THIS.exe_omx_dec)/decoder/project_dec.mk

EXE_OMX_DECODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_DECODER_OBJ+=$(EXE_OMX_DECODER_SRCS:%=$(BIN)/%.o)

$(BIN)/$(EXE_NAME_DEC): $(EXE_OMX_DECODER_OBJ) $(LIB_OMX_DEC) $(LIB_OMX_CORE)
$(BIN)/$(EXE_NAME_DEC): CFLAGS+=-fPIC
$(BIN)/$(EXE_NAME_DEC): CFLAGS+=-pthread
$(BIN)/$(EXE_NAME_DEC): LDFLAGS+=-lpthread
$(BIN)/$(EXE_NAME_DEC): LDFLAGS+=-L$(EXTERNAL_LIB)
$(BIN)/$(EXE_NAME_DEC): LDFLAGS+=-l$(EXTERNAL_DECODE_LIB_NAME:lib%=%)


$(BIN)/$(SH_NAME_DEC): $(BIN)/$(EXE_NAME_DEC)
	@echo "Generate script to launch $^"
	$(shell echo '#!/bin/bash' > $@)
	$(shell echo 'BIN_PATH=$$(dirname $$(realpath "$$0"))' >> $@)
	$(shell echo 'export OMX_ALLEGRO_PATH="$$BIN_PATH"' >> $@)
	$(shell echo 'export LD_LIBRARY_PATH="$$BIN_PATH:$(EXTERNAL_LIB)"' >> $@)
	$(shell echo '"$$BIN_PATH/$(EXE_NAME_DEC)" "$$@"' >> $@)
	$(shell chmod a+x $@)

omx_decoder: $(BIN)/$(EXE_NAME_DEC) $(BIN)/$(SH_NAME_DEC)

.PHONY: omx_decoder
TARGETS+=omx_decoder

