# Copyright 2006 The Android Open Source Project
ifeq ($(MTK_C2K_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rild.c


LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libc2kril \
    libc2kutils

ifeq ($(TARGET_ARCH),arm64)
LOCAL_SHARED_LIBRARIES += libdl
else 
ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif #arm
endif # arm64

LOCAL_CFLAGS := -DRIL_SHLIB

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif
## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ril/viatelecom/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include

#ALOG_VERSION := 4.1.1 4.2.1 4.2.2
#ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
#LOCAL_CFLAGS += -DANDROID_JB
#endif

LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_MODULE:= viarild
LOCAL_PRELINK_MODULE := false

ifneq ($(MTK_INTERNAL),yes)
LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

include $(BUILD_EXECUTABLE)

# For radiooptions binary
# =======================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	radiooptions.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif

LOCAL_MODULE:= viaradiooptions
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := debug
#ifeq ($(PLATFORM_VERSION), 4.1.1)
#LOCAL_CFLAGS += -DANDROID_JB
#endif
LOCAL_PRELINK_MODULE := false
include $(BUILD_EXECUTABLE)

ifeq ($(C2K_GEMINI),yes)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(C2K_MODEM_SUPPORT), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(C2K_MODEM_SUPPORT), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(C2K_MODEM_SUPPORT), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

endif
