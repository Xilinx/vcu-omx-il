#------------------------------------------------------------------------------
# Video Component Definition
#------------------------------------------------------------------------------

BASE_DIR?=.
ENABLE_64BIT?=1
STATIC?=0
CROSS_COMPILE?=

BIN?=bin
EXTERNAL_LIB?=../soft/
EXTERNAL_BIN?=../soft/bin
OMX_HEADER?=../omx_header

INCLUDES+=-I$(OMX_HEADER)
INCLUDES+=-I$(EXTERNAL_LIB)
INCLUDES+=-I$(EXTERNAL_LIB)/include
INCLUDES+=-include $(EXTERNAL_LIB)/include/config.h

-include $(BASE_DIR)/unittests.mk
