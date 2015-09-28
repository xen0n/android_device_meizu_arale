LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := immvibetst
LOCAL_MODULE_TAGS             := optional
# LOCAL_C_INCLUDES              := $(LOCAL_PATH)/include
LOCAL_SRC_FILES               := immvibetst.c
#LOCAL_FORCE_STATIC_EXECUTABLE := true
#LOCAL_STATIC_LIBRARIES        := libc
include $(BUILD_EXECUTABLE)
