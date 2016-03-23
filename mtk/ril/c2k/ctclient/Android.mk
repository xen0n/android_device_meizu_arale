# Copyright 2010 VIA Telecom
#
#Author(s) Karfield Chen <kfchen@via-telecom.com.cn>

#
ifeq ($(MTK_C2K_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= ctclient
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES:= ctclient.c

LOCAL_SHARED_LIBRARIES := libcutils libc2kutils

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
#ALOG_VERSION := 4.1.1 4.2.1 4.2.2
ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
LOCAL_CFLAGS += -DANDROID_JB
endif

LOCAL_MODULE_TAGS := optional

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif
LOCAL_CFLAGS += -DMUX_ANDROID

#LOCAL_LDLIBS := -lpthread
#
#CAL_LDLIBS += -lpthread

include $(BUILD_EXECUTABLE)

endif
