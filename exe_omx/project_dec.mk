THIS.exe_omx_dec:=$(call get-my-dir)

EXE_NAME_DEC:=omx_decoder.exe

include $(THIS.exe_omx_dec)/decoder/project_dec.mk

EXE_OMX_DECODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_DECODER_OBJ+=$(EXE_OMX_DECODER_SRCS:%=$(BIN)/%.o)

$(BIN)/$(EXE_NAME_DEC): $(EXE_OMX_DECODER_OBJ) $(LIBS_DECODE) $(LIB_OMX_CORE)

omx_decoder: $(BIN)/$(EXE_NAME_DEC)

.PHONY: omx_decoder
TARGETS+=omx_decoder

