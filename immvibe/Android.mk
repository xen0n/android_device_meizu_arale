LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := libimmvibeclient
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := immvibeclient.c
LOCAL_EXPORT_C_INCLUDES       := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES              := $(LOCAL_EXPORT_C_INCLUDES)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE                  := immvibetst
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := immvibetst.c

# hack to suppress build errors when building for the first time (?)
LOCAL_C_INCLUDES              := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES        := libimmvibeclient
include $(BUILD_EXECUTABLE)
