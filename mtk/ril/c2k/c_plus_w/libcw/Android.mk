# Copyright 2006 The Android Open Source Project
ifeq ($(MTK_C2K_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../rpc_ril
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include

#ALOG_VERSION := 4.1.1 4.2.1 4.2.2
ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
LOCAL_CFLAGS += -DANDROID_JB
endif

LOCAL_SRC_FILES := cw.c

LOCAL_SHARED_LIBRARIES := libutils libcutils libdl librpcril libbinder


## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libcw
#LOCAL_MODULE:= cw
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)

endif
