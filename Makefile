.PHONY: clean

define get-my-dir
$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endef

BIN?=$(shell pwd)/bin
EXTERNAL_LIB?=$(shell pwd)/../CtrlSW
EXTERNAL_BIN?=$(shell pwd)/../CtrlSW/bin
EXTERNAL_CONFIG?=$(EXTERNAL_LIB)/include/config.h
OMX_HEADER?=$(shell pwd)/omx_header

##############################################################
# Enabling FLAGS
##############################################################
ENABLE_64BIT?=1
ENABLE_VCU?=1
ENABLE_MCU?=1
STATIC?=0
LINK_SHARED_CTRLSW?=0

ifeq ($(ENABLE_VCU), 1)
	CFLAGS+=-DAL_USE_VCU
else
	CFLAGS+=-DAL_USE_VCU=0
endif

ifeq ($(ENABLE_MCU), 1)
	CFLAGS+=-DAL_USE_MCU
else
	CFLAGS+=-DAL_USE_MCU=0
endif

CFLAGS+=-DAL_USE_MODULE=0

##############################################################
# Master rule
##############################################################
all: true_all

##############################################################
# Cross build
##############################################################
CROSS_COMPILE?=

CXX:=$(CROSS_COMPILE)g++
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
NM:=$(CROSS_COMPILE)nm
LD:=$(CROSS_COMPILE)ld
OBJDUMP:=$(CROSS_COMPILE)objdump
OBJCOPY:=$(CROSS_COMPILE)objcopy
RANLIB:=$(CROSS_COMPILE)ranlib
STRIP:=$(CROSS_COMPILE)strip
SIZE:=$(CROSS_COMPILE)size

TARGET:=$(shell $(CC) -dumpmachine)
ifeq ($(ENABLE_64BIT),0)
  # force 32 bit compilation
  ifneq (,$(findstring x86_64,$(TARGET)))
    CFLAGS+=-m32
    LDFLAGS+=-m32
    TARGET:=i686-linux-gnu
  endif
endif

##############################################################
# Debug flags
##############################################################
CFLAGS+=-O3
CFLAGS+=-g3
CFLAGS+=-Wno-missing-field-initializers
LDFLAGS+=-g3

##############################################################
# Basic flags
##############################################################
CFLAGS+=-Wall -Wextra -Werror -fPIC
LDFLAGS+=-fPIC
LDFLAGS+=-ldl
LDFLAGS+=-lpthread
INCLUDES+=-I.

##############################################################
# Miscellaneous
##############################################################


##############################################################
# Base component
##############################################################
BASE_DIR:=base
include $(BASE_DIR)/base.mk

##############################################################
# Core component
##############################################################
CORE_DIR:=core
include $(CORE_DIR)/core.mk

##############################################################
# Tests Applications component
##############################################################
APP_DIR:=exe_omx
include $(APP_DIR)/encoder.mk
include $(APP_DIR)/decoder.mk
-include $(APP_DIR)/unittests.mk

##############################################################
# Regression tests
##############################################################
TEST_SRC_DIR:=unittests
-include ./unittests.mk


##############################################################
# Conformance tests
##############################################################
CONF_DIR:=conformance
-include ./conformance/conformance.mk

##############################################################
# Build
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
	$(Q)$(CXX) $(CFLAGS) $(INCLUDES) -std=c++11 -o $@ -c $<
	@$(CXX) -MM "$<" -MT "$@" -o "$(BIN)/$*_cpp.deps" $(INCLUDES) $(CFLAGS) -std=c++11
	@echo "CXX $<"

$(BIN)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -std=gnu99 -o $@ -c $<
	@$(CC) -MM "$<" -MT "$@" -o "$(BIN)/$*_c.deps" $(INCLUDES) $(CFLAGS) -std=gnu99
	@echo "CC $<"

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -o $@ $^ $(LINK_COMPAT) $(LDFLAGS)
	@echo "CXX $@"

##############################################################
# Clean
##############################################################
clean:
	rm -rf $(BIN)
	rm -rf $(EXTERNAL_BIN)

true_all: $(TARGETS)

