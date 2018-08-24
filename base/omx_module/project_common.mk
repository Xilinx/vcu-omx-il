THIS.omx_module_common:=$(call get-my-dir)

OMX_MODULE_COMMON_SRCS+=\
                        $(THIS.omx_module_common)/omx_module_interface.cpp\
                        $(THIS.omx_module_common)/omx_buffer_handle_interface.cpp\
                        $(THIS.omx_module_common)/omx_sync_ip_interface.cpp\
                        $(THIS.omx_module_common)/SyncIp.cpp\
                        $(THIS.omx_module_common)/SyncLog.cpp\
                        $(THIS.omx_module_common)/DummySyncDriver.cpp\
                        $(THIS.omx_module_common)/omx_sync_ip.cpp\

UNITTESTS+=$(shell find $(THIS.omx_module_common)/unittests -name "*.cpp")
UNITTESTS+=$(OMX_MODULE_COMMON_SRCS)
