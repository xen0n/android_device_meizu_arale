LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := libmeizupshelper
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := lib.c
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES              := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES        := \
	liblog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE                  := meizupshelper
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := helper.c
LOCAL_SHARED_LIBRARIES        := \
	libmeizupshelper
include $(BUILD_EXECUTABLE)
