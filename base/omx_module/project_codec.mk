THIS.module_codec:=$(call get-my-dir)

MODULE_CODEC_SRCS+=\
                    $(THIS.module_codec)/convert_module_soft.cpp\
                    $(THIS.module_codec)/convert_module_soft_avc.cpp\
                    $(THIS.module_codec)/convert_module_soft_hevc.cpp\
                    $(THIS.module_codec)/mediatype_interface.cpp\
                    $(THIS.module_codec)/mediatype_checks.cpp\
                    $(THIS.module_codec)/mediatype_codec_itu.cpp\
                    $(THIS.module_codec)/mediatype_codec_avc.cpp\
                    $(THIS.module_codec)/mediatype_codec_hevc.cpp\
                    $(THIS.module_codec)/mediatype_dummy.cpp\
                    $(THIS.module_codec)/module_interface.cpp\
                    $(THIS.module_codec)/module_dummy.cpp\
                    $(THIS.module_codec)/buffer_handle_interface.cpp\
                    $(THIS.module_codec)/sync_ip_interface.cpp\
                    $(THIS.module_codec)/SyncIp.cpp\
                    $(THIS.module_codec)/SyncLog.cpp\
                    $(THIS.module_codec)/DummySyncDriver.cpp\


UNITTESTS+=$(shell find $(THIS.module_codec)/unittests -name "*.cpp")
UNITTESTS+=$(MODULE_CODEC_SRCS)
