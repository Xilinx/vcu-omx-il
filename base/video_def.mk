#------------------------------------------------------------------------------
# Video Component Definition
#------------------------------------------------------------------------------

BIN?=bin
BASE_DIR?=.

ENABLE_64BIT?=1
STATIC?=0
CROSS_COMPILE?=

EXTERNAL_LIB?=soft
EXTERNAL_LIB_PATH?=..
EXTERNAL_LIB_DIR?=$(EXTERNAL_LIB_PATH)/$(EXTERNAL_LIB)

OMX_HEADER?=../omx_header
INCLUDES+=-I$(OMX_HEADER)
INCLUDES+=-I$(EXTERNAL_LIB_DIR)
INCLUDES+=-I$(EXTERNAL_LIB_DIR)/include
INCLUDES+=-include $(EXTERNAL_LIB_DIR)/config.h

-include $(BASE_DIR)/unittests.mk
