THIS.exe_omx_dec:=$(call get-my-dir)

EXE_NAME_DEC:=omx_decoder.exe

include $(THIS.exe_omx_dec)/decoder/project_dec.mk

EXE_OMX_DECODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_DECODER_OBJ+=$(EXE_OMX_DECODER_SRCS:%=$(BIN)/%.o)

ifneq ($(LINK_SHARED_CTRLSW), 1)
$(BIN)/$(EXE_NAME_DEC): $(EXE_OMX_DECODER_OBJ) $(LIBS_DECODE) $(LIB_OMX_CORE)
else
$(BIN)/$(EXE_NAME_DEC): $(EXE_OMX_DECODER_OBJ) $(LIB_OMX_CORE)
endif
omx_decoder: $(BIN)/$(EXE_NAME_DEC)

.PHONY: omx_decoder
TARGETS+=omx_decoder

