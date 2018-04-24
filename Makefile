define get-my-dir
$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endef

THIS:=$(call get-my-dir)

all: true_all

PWD:=$(shell pwd)
BIN?=$(PWD)/bin
OMX_HEADERS?=$(PWD)/omx_header

EXTERNAL_CTRLSW?=$(PWD)/../allegro-vcu-ctrl-sw
EXTERNAL_TESTS=$(PWD)/unittests

EXTERNAL_INCLUDE?=$(EXTERNAL_CTRLSW)/include
EXTERNAL_LIB?=$(EXTERNAL_CTRLSW)/bin
EXTERNAL_CONFIG?=$(EXTERNAL_INCLUDE)/config.h

TARGET?=$(shell $(CC) -dumpmachine)

CFLAGS+=-O3
CFLAGS+=-g3
LDFLAGS+=-g3

CFLAGS+=-Wall -Wextra
#CFLAGS+=-Werror

ENABLE_VCU?=1
ENABLE_MCU?=1
ENABLE_64BIT?=1
# Makes the assumption that the external ctrlsw is installed on the machine
# disable this if you want compilation to work with a non installed ctrlsw
LINK_SHARED_CTRLSW?=1

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

-include compiler.mk

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

ifeq ($(ENABLE_64BIT),0)
  # force 32 bit compilation
  ifneq (,$(findstring x86_64,$(TARGET)))
    CFLAGS+=-m32
    LDFLAGS+=-m32
    TARGET:=i686-linux-gnu
  endif
endif

include $(THIS)/builder.mk

INCLUDES+=-I.
INCLUDES+=-I$(OMX_HEADERS)
INCLUDES+=-I$(EXTERNAL_INCLUDE)
INCLUDES+=-include $(EXTERNAL_CONFIG)

-include $(THIS)/get_external.mk

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

true_all: $(TARGETS)

