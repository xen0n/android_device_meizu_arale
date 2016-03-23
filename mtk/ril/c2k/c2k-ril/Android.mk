# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
# 

LOCAL_PATH:= $(call my-dir)

#GIT_COMMIT := $(shell cd $(LOCAL_PATH); git rev-parse --verify --short HEAD 2>/dev/null)
#SVN_COMMIT := $(shell svn info 2>/dev/null | grep '^Last Changed Re' | awk '{print $$NF}')
#
#ifneq ($(strip $(GIT_COMMIT)),)
#	REPO_VERSION := \"git_$(GIT_COMMIT)\"
#else
#	REPO_VERSION := \"svn_$(SVN_COMMIT)\"
#endif
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    oem-support.c \
    atchannel.c \
    misc.c \
    at_tok.c \
    sms.c \
    icc.c \
    ril_nw.c \
    ril_cc.c \
    ril_sim.c \
    ril_sms.c \
    ril_ss.c \
    ril_utk.c \
    ril_oem.c \
    ril_callbacks.c \
    c2k_ril_data.c

ifeq ($(MTK_IRAT_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_IRAT_SUPPORT
endif

ifneq ($(MTK_INTERNAL),yes)
LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

#LOCAL_SRC_FILES += via-audio-ril.c

LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libc2kril libnetutils libc2kutils librilutils

LOCAL_STATIC_LIBRARIES := \
    c2k-ril-prop

# for asprinf
LOCAL_CFLAGS += -D_GNU_SOURCE -D_POSIX_SOURCE
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_C_INCLUDES += external/alsa-lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include

LOCAL_C_INCLUDES += $(MTK_ROOT)/external/aee/binary/inc
LOCAL_SHARED_LIBRARIES += libaed

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif

ifeq ($(MTK_SVLTE_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_SVLTE_SUPPORT
endif

ifeq ($(MTK_ECCCI_C2K),yes)
LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(TARGET_DEVICE),apollo)
#  LOCAL_CFLAGS += -DVIATELECOM_APOLLO
endif
ifeq ($(TARGET_DEVICE),marco)
#  LOCAL_CFLAGS += -DVIATELECOM_MARCO
endif
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
      libcutils libutils 

#  LOCAL_SHARED_LIBRARIES += libasound
#  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB

#ALOG_VERSION := 4.1.1 4.2.1 4.2.2
#ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
#LOCAL_CFLAGS += -DANDROID_JB
#endif
#ifeq ($(PLATFORM_VERSION), 4.2.2)
LOCAL_CFLAGS += -DMT6589
LOCAL_CFLAGS += -DUSE_STATUSD
#endif
  LOCAL_PRELINK_MODULE := false
  LOCAL_MODULE:= libviatelecom-withuim-ril
  include $(BUILD_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
      libc2kril
  LOCAL_MODULE:= libviatelecom-withuim-ril
  LOCAL_PRELINK_MODULE := false
  include $(BUILD_EXECUTABLE)
endif
