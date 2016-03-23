# Copyright 2006 The Android Open Source Project
ifeq ($(MTK_C2K_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= \
    rpc_ril.cpp 

LOCAL_SHARED_LIBRARIES := \
    libutils \
	libbinder \
    libcutils \
    libc2kutils

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
#ALOG_VERSION := 4.1.1 4.2.1 4.2.2 
ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
LOCAL_CFLAGS += -DANDROID_JB
endif

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= librpcril
LOCAL_MULTILIB := 32
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)

#LOCAL_LDLIBS += -lpthread
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

endif
