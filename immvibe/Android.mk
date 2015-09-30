LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := libimmvibeclient
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := client.c api.c
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES              := $(LOCAL_PATH)/include
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE                  := libimmvibeconnector
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := connector.c
LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES              := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES        := \
	liblog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE                  := openimmvibed
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := daemon.c threadqueue.c
LOCAL_C_INCLUDES              := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES        := \
	libimmvibeclient \
	liblog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE                  := immvibetst
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := tst.c
LOCAL_SHARED_LIBRARIES        := libimmvibeclient
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE                  := immvibetst-connector
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := connector-tst.c
LOCAL_SHARED_LIBRARIES        := libimmvibeconnector
include $(BUILD_EXECUTABLE)
