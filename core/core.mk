#------------------------------------------------------------------------------
# Core Component
#------------------------------------------------------------------------------

CORE_DIR?=.

-include $(CORE_DIR)/core_version.mk
-include $(CORE_DIR)/unittests.mk

OMX_HEADER?=../omx_header
INCLUDES+=-I$(OMX_HEADER)

LIB_OMX_CORE_SO=$(BIN)/libOMX.allegro.core.so.$(CORE_VERSION)

LIB_OMX_CORE_SRC:=\
	$(CORE_DIR)/omx_core/omx_core.cpp\

LIB_OMX_CORE_OBJ:=$(LIB_OMX_CORE_SRC:%.cpp=$(BIN)/%.cpp.o)
$(LIB_OMX_CORE_SO): $(LIB_OMX_CORE_OBJ)

TARGETS_OMX_CORE+=$(LIB_OMX_CORE_SO)

$(LIB_OMX_CORE_SO):
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(LDFLAGS) -shared -Wl,-soname,libOMX.allegro.core.so.$(CORE_MAJOR) -o "$@" $^ -ldl
	@echo "LD $@"
	ln -fs "libOMX.allegro.core.so.$(CORE_VERSION)" "$(BIN)/libOMX.allegro.core.so"

lib_omx_core: $(TARGETS_OMX_CORE)

TARGETS+=lib_omx_core
