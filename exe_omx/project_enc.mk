THIS.exe_omx_enc:=$(call get-my-dir)

EXE_NAME_ENC:=omx_encoder.exe
SH_NAME_ENC:=$(EXE_NAME_ENC:%.exe=%.sh)

include $(THIS.exe_omx_enc)/encoder/project_enc.mk

EXE_OMX_ENCODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_ENCODER_OBJ+=$(EXE_OMX_ENCODER_SRCS:%=$(BIN)/%.o)

EXE_ENC_CFLAGS:=$(DEFAULT_CFLAGS)
EXE_ENC_CFLAGS+=-fPIC

EXE_ENC_LDFLAGS:=$(DEFAULT_LDFLAGS)
EXE_ENC_LDFLAGS+=-L$(BIN)
EXE_ENC_LDFLAGS+=-l$(LIB_OMX_CORE_NAME:lib%.so=%)
EXE_ENC_LDFLAGS+=-l$(LIB_OMX_ENC_NAME:lib%.so=%)
ifdef EXTERNAL_LIB
EXE_ENC_LDFLAGS+=-L$(EXTERNAL_LIB)
endif
EXE_ENC_LDFLAGS+=-l$(EXTERNAL_ENCODE_LIB_NAME:lib%.so=%)

-include $(THIS.exe_omx_enc)/ref_enc.mk

$(BIN)/$(EXE_NAME_ENC): $(LIB_OMX_CORE)
$(BIN)/$(EXE_NAME_ENC): $(LIB_OMX_ENC)
$(BIN)/$(EXE_NAME_ENC): $(EXE_OMX_ENCODER_OBJ)
$(BIN)/$(EXE_NAME_ENC): CFLAGS:=$(EXE_ENC_CFLAGS)
$(BIN)/$(EXE_NAME_ENC): LDFLAGS:=$(EXE_ENC_LDFLAGS)

$(BIN)/$(SH_NAME_ENC): $(BIN)/$(EXE_NAME_ENC)
	@echo "Generate script to launch $^"
	$(shell echo '#!/bin/bash' > $@)
	$(shell echo 'BIN_PATH=$$(dirname $$(realpath "$$0"))' >> $@)
	$(shell echo 'export OMX_ALLEGRO_PATH="$$BIN_PATH"' >> $@)
	$(shell echo 'export LD_LIBRARY_PATH="$$BIN_PATH:$(EXTERNAL_LIB)"' >> $@)
	$(shell echo '"$$BIN_PATH/$(EXE_NAME_ENC)" "$$@"' >> $@)
	$(shell chmod a+x $@)

omx_encoder:  $(BIN)/$(EXE_NAME_ENC) $(BIN)/$(SH_NAME_ENC)

.PHONY: omx_encoder
TARGETS+=omx_encoder


