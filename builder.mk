##############################################################
#general rules
##############################################################
.SUFFIXES:

V?=0

ifeq ($(V),0)
	Q=@
else
	Q=
endif

LINK_COMPAT:=

ifneq ($(findstring mingw,$(TARGET)),mingw)
    LINK_COMPAT+=-Wl,--hash-style=both
endif

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(CFLAGS) $(INCLUDES) -std=gnu++11 -o $@ -c $<
	@$(CXX) -MM "$<" -MT "$@" -o "$(BIN)/$*_cpp.deps" $(INCLUDES) $(CFLAGS) -std=gnu++11
	@echo "CXX $<"

$(BIN)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -std=gnu11 -o $@ -c $<
	@$(CC) -MM "$<" -MT "$@" -o "$(BIN)/$*_c.deps" $(INCLUDES) $(CFLAGS) -std=gnu11
	@echo "CC $<"

$(BIN)/%.a:
	@mkdir -p $(dir $@)
	$(Q)$(AR) cr $@ $^
	@echo "AR $@"

$(BIN)/%:
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -o $@ $^ $(LINK_COMPAT) $(LDFLAGS)
	@echo "CXX $@"

$(BIN)/%.so:
	$(Q)$(CXX) $(CFLAGS) -shared -Wl,-soname,$(notdir $@).$(MAJOR) -o "$@.$(VERSION)" $^ $(LDFLAGS)
	@echo "LD $@"
	@ln -fs "$(@:$(BIN)/%=%).$(VERSION)" $@.$(MAJOR)
	@ln -fs "$(@:$(BIN)/%=%).$(VERSION)" $@

# Dependency generation
include $(shell test -d $(BIN) && find $(BIN) -name "*.deps")
