#!/bin/bash

readonly bin="$(pwd)/bin"
readonly omx_headers="$(pwd)/omx_header"
readonly external_srcs="$(pwd)/../allegro-vcu-ctrl-sw"
readonly external_libs="$external_srcs/bin"
readonly external_includes="$(pwd)/../allegro-vcu-ctrl-sw/include"
readonly external_configs="$(pwd)/../allegro-vcu-ctrl-sw/include/config.h"

BIN=$bin \
OMX_HEADERS=$omx_headers \
EXTERNAL_INCLUDE=$external_includes \
EXTERNAL_SRC=$external_srcs \
EXTERNAL_LIB=$external_libs \
EXTERNAL_CONFIG=$external_configs \
make -j`nproc` "$@"
