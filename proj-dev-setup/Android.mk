LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                  := proj-dev-setup
LOCAL_MODULE_TAGS             := optional
LOCAL_SRC_FILES               := main.c
LOCAL_SHARED_LIBRARIES        := \
	libcutils \
	liblog \
	libselinux \

include $(BUILD_EXECUTABLE)
