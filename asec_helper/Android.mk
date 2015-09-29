LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := asec_helper
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := asec_helper.c
LOCAL_SHARED_LIBRARIES        := \
	libcutils \
	liblog
include $(BUILD_EXECUTABLE)
