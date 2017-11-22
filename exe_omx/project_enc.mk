THIS.exe_omx_enc:=$(call get-my-dir)

EXE_NAME_ENC:=omx_encoder.exe

include $(THIS.exe_omx_enc)/encoder/project_enc.mk

EXE_OMX_ENCODER_OBJ:=$(EXE_OMX_COMMON_OBJ)
EXE_OMX_ENCODER_OBJ+=$(EXE_OMX_ENCODER_SRCS:%=$(BIN)/%.o)

ifneq ($(LINK_SHARED_CTRLSW), 1)
$(BIN)/$(EXE_NAME_ENC): $(EXE_OMX_ENCODER_OBJ) $(LIBS_ENCODE) $(LIB_OMX_CORE)
else
LDFLAGS+=-lallegro_encode
$(BIN)/$(EXE_NAME_ENC): $(EXE_OMX_ENCODER_OBJ) $(LIB_OMX_CORE)
endif

omx_encoder: $(BIN)/$(EXE_NAME_ENC)

.PHONY: omx_encoder
TARGETS+=omx_encoder
