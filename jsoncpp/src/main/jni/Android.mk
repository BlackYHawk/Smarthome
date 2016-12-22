LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	src/lib_json/json_writer.cpp \
	src/lib_json/json_reader.cpp \
	src/lib_json/json_value.cpp \

LOCAL_CFLAGS := -fexceptions

LOCAL_MODULE:= json

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES := wchar_static

APP_STL := gnustl_static

include $(BUILD_STATIC_LIBRARY)
