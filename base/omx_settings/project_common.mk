THIS.omx_settings_common:=$(call get-my-dir)

OMX_SETTINGS_COMMON_SRCS+=\
  $(THIS.omx_settings_common)/omx_convert_module_soft.cpp\
  $(THIS.omx_settings_common)/omx_convert_module_soft_avc.cpp\
  $(THIS.omx_settings_common)/omx_convert_module_soft_hevc.cpp\
  $(THIS.omx_settings_common)/omx_settings_checks.cpp\
  $(THIS.omx_settings_common)/omx_settings_mock.cpp\

UNITTESTS+=$(shell find $(THIS.omx_settings_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_SETTINGS_COMMON_SRCS)

