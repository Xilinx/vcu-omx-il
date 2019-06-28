THIS.module_common:=$(call get-my-dir)

MODULE_COMMON_SRCS+=\
                    $(THIS.module_common)/convert_module_soft.cpp\
                    $(THIS.module_common)/convert_module_soft_avc.cpp\
                    $(THIS.module_common)/convert_module_soft_hevc.cpp\
                    $(THIS.module_common)/mediatype_interface.cpp\
                    $(THIS.module_common)/mediatype_checks.cpp\
                    $(THIS.module_common)/mediatype_common.cpp\
                    $(THIS.module_common)/mediatype_common_avc.cpp\
                    $(THIS.module_common)/mediatype_common_hevc.cpp\
                    $(THIS.module_common)/mediatype_dummy.cpp\
                    $(THIS.module_common)/module_interface.cpp\
                    $(THIS.module_common)/module_dummy.cpp\
                    $(THIS.module_common)/buffer_handle_interface.cpp\
                    $(THIS.module_common)/sync_ip_interface.cpp\
                    $(THIS.module_common)/SyncIp.cpp\
                    $(THIS.module_common)/SyncLog.cpp\
                    $(THIS.module_common)/DummySyncDriver.cpp\


UNITTESTS+=$(shell find $(THIS.module_common)/unittests -name "*.cpp")
UNITTESTS+=$(MODULE_COMMON_SRCS)
