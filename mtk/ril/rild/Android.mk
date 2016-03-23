#
# Copyright (C) 2014 MediaTek Inc.
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(s).
#

# Copyright 2014 The Android Open Source Project


ifneq ($(GOOGLE_RELEASE_RIL), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    rild.c

LOCAL_SHARED_LIBRARIES := \
  liblog \
  libcutils \
  librilmtk \
  libdl

LOCAL_STATIC_LIBRARIES := \
    rild-prop-md1

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

#ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
#endif # arm

LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL

ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

LOCAL_MODULE:= mtkrild

include $(BUILD_EXECUTABLE)


#========================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    rild.c

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    librilmtkmd2 \
    libdl

LOCAL_STATIC_LIBRARIES := \
    rild-prop-md2

ifeq ($(TELEPHONY_DFOSET),yes)
LOCAL_SHARED_LIBRARIES += libdfo
endif
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../external/dfo/featured $(LOCAL_PATH)/../include

#ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
#endif # arm

LOCAL_CFLAGS := -DRIL_SHLIB -DMTK_RIL

ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(MTK_SHARE_MODEM_SUPPORT), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

LOCAL_MODULE:= mtkrildmd2

include $(BUILD_EXECUTABLE)

# For radiooptions binary
# =======================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    radiooptions.c

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \

LOCAL_CFLAGS := \

LOCAL_MODULE:= mtkradiooptions
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

endif # GOOGLE_RELEASE_RIL
