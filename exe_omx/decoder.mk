#------------------------------------------------------------------------------
# Video Decoder Test Application
#------------------------------------------------------------------------------
APP_DIR?=exe_omx
DEC_APP_NAME?=omx_decoder.exe

EXE_TEST_DECODE_OMX_SRC:=\
	$(APP_DIR)/decoder/main.cpp\
	$(APP_DIR)/common/helpers.cpp\
	$(APP_DIR)/common/setters.cpp\

EXE_TEST_DECODE_OMX_OBJ:=$(EXE_TEST_DECODE_OMX_SRC:%.cpp=$(BIN)/%.cpp.o)

test_omx_decode: $(BIN)/$(DEC_APP_NAME) lib_omx_decoder

ifneq ($(LINK_SHARED_CTRLSW), 1)
$(BIN)/$(DEC_APP_NAME): $(EXE_TEST_DECODE_OMX_OBJ) $(LIB_DECODE_A) $(LIB_OMX_CORE_OBJ)
else
LDFLAGS+=-lallegro_decode
$(BIN)/$(DEC_APP_NAME): $(EXE_TEST_DECODE_OMX_OBJ) $(LIB_OMX_CORE_OBJ)
endif

TARGETS+=test_omx_decode
