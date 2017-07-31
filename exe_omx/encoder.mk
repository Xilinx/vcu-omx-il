#------------------------------------------------------------------------------
# Video Encoder Test Application
#------------------------------------------------------------------------------
APP_DIR?=exe_omx
ENC_APP_NAME=omx_encoder.exe

EXE_TEST_ENCODE_OMX_SRC:=\
	$(APP_DIR)/encoder/main.cpp\
	$(APP_DIR)/common/helpers.cpp\
	$(APP_DIR)/common/setters.cpp\
	$(APP_DIR)/common/getters.cpp\

EXE_TEST_ENCODE_OMX_OBJ:=$(EXE_TEST_ENCODE_OMX_SRC:%.cpp=$(BIN)/%.cpp.o)

test_omx_encode: $(BIN)/$(ENC_APP_NAME) lib_omx_encoder

ifneq ($(LINK_SHARED_CTRLSW), 1)
$(BIN)/$(ENC_APP_NAME): $(EXE_TEST_ENCODE_OMX_OBJ) $(LIB_ENCODE_A) $(LIB_OMX_CORE_OBJ)
else
LDFLAGS+=-lallegro_encode
$(BIN)/$(ENC_APP_NAME): $(EXE_TEST_ENCODE_OMX_OBJ) $(LIB_OMX_CORE_OBJ)
endif

TARGETS+=test_omx_encode
