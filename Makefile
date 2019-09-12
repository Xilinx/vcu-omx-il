define get-my-dir
$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endef

THIS:=$(call get-my-dir)

all: true_all

PWD:=$(shell pwd)

BIN?=$(PWD)/bin
OMX_HEADERS?=$(PWD)/omx_header
EXTERNAL_INCLUDE?=$(PWD)/../allegro-vcu-ctrl-sw/include
EXTERNAL_CONFIG?=$(EXTERNAL_INCLUDE)/config.h

ifndef BIN
$(error BIN must be set)
endif

ifndef OMX_HEADERS
$(error OMX_HEADERS must be set)
endif

ifndef EXTERNAL_INCLUDE
$(error EXTERNAL_INCLUDE must be set)
endif

ifndef EXTERNAL_CONFIG
$(error EXTERNAL_CONFIG must be set)
endif

EXTERNAL_SRC?=
EXTERNAL_LIB?=


ENABLE_VCU?=1
ENABLE_MCU?=1
ENABLE_64BIT?=1
ENABLE_DMA_COPY_ENC?=1

-include quirks.mk
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

ifeq ($(ENABLE_DMA_COPY_ENC), 1)
  CFLAGS+=-DAL_ENABLE_DMA_COPY_ENC
endif

TARGET?=$(shell $(CC) -dumpmachine)

ifeq ($(ENABLE_64BIT),0)
  # force 32 bit compilation
  ifneq (,$(findstring x86_64,$(TARGET)))
    CFLAGS+=-m32
    LDFLAGS+=-m32
    TARGET:=i686-linux-gnu
  endif
endif

include $(THIS)/builder.mk

DEFAULT_CFLAGS:=$(CFLAGS)
DEFAULT_CFLAGS+=-O3
DEFAULT_CFLAGS+=-pedantic
DEFAULT_CFLAGS+=-g0
DEFAULT_CFLAGS+=-Wall
DEFAULT_CFLAGS+=-Wextra

DEFAULT_CFLAGS+=-Wno-missing-field-initializers

DEFAULT_LDFLAGS:=$(LDFLAGS)

INCLUDES+=-I$(THIS)
INCLUDES+=-I$(OMX_HEADERS)
INCLUDES+=-I$(EXTERNAL_INCLUDE)
INCLUDES+=-include $(EXTERNAL_CONFIG)

-include $(THIS)/get_external.mk

include $(THIS)/utility/project.mk

include $(THIS)/core/project.mk

include $(THIS)/base/project_common.mk
-include $(THIS)/base/project_enc.mk
-include $(THIS)/base/project_dec.mk

include $(THIS)/exe_omx/project_common.mk
-include $(THIS)/exe_omx/project_enc.mk
-include $(THIS)/exe_omx/project_dec.mk

-include $(THIS)/conformance/project.mk
-include $(THIS)/unittests.mk

.PHONY: clean
clean:
	$(Q)rm -rf $(BIN)
	@echo "CLEAN $(BIN)"

.PHONY: distclean
distclean: clean
	@echo "CLEAN $(EXTERNAL_LIB)"
	$(Q)rm -rf $(EXTERNAL_LIB)

TARGETS: $(externals)
true_all: $(TARGETS)

