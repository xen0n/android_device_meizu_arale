# Copyright 2006 The Android Open Source Project
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril.cpp \
    ril_event.cpp \
    libc2kril.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libhardware_legacy \
    librilutils \
    libc2kutils
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include

#for ril command
HAS_VIA_ADDED_RIL_REQUEST := true
HAS_ADD_MTK_REQUEST_URC := true

ifeq ($(MTK_IRAT_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_IRAT_SUPPORT
endif

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif

ifeq ($(HAS_ADD_MTK_REQUEST_URC), true)
LOCAL_CFLAGS += -DADD_MTK_REQUEST_URC
endif

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libc2kril
LOCAL_MULTILIB := 32
#LOCAL_LDLIBS += -lpthread
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)


# For RdoServD which needs a static library
# =========================================
ifneq ($(ANDROID_BIONIC_TRANSITION),)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril.cpp

LOCAL_STATIC_LIBRARIES := \
    libutils_static \
    libcutils \
    librilutils_static

LOCAL_CFLAGS :=

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libc2kril_static
LOCAL_MULTILIB := 32
#LOCAL_LDLIBS += -lpthread
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)
endif # ANDROID_BIONIC_TRANSITION

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
