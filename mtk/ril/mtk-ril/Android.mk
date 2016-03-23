#
# Copyright (C) 2014 MediaTek Inc.
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(s).
#

# Copyright 2014 The Android Open Source Project

# XXX using libutils for simulator build only...
#
ifneq ($(GOOGLE_RELEASE_RIL), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#Mediatek Configuration
#ifeq ($(TARGET_PRODUCT),generic)
ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
LOCAL_SRC_FILES:= \
    ril_callbacks_emu.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels_emu.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c \
    ril_gsm_util.cpp \
    base64.c
else
LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c \
    ril_gsm_util.cpp \
    base64.c
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libnetutils librilmtk librilutils


    # for asprinf

LOCAL_STATIC_LIBRARIES := \
    mtk-ril-prop-md1 \
    libprotobuf-c-nano-enable_malloc \

LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -D__CCMNI_SUPPORT__

ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
  LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_LTE_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_LTE_SUPPORT
endif

ifeq ($(MTK_DTAG_DUAL_APN_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_DTAG_DUAL_APN_SUPPORT
endif

ifeq ($(MTK_IPV6_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_IPV6_SUPPORT -DINET6
endif

ifeq ($(MTK_GEMINI_3G_SWITCH),yes)
  LOCAL_CFLAGS += -DMTK_GEMINI_3G_SWITCH
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

LOCAL_SHARED_LIBRARIES += libmal libmdfx

LOCAL_C_INCLUDES :=

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(LOCAL_PATH)/../include \
#    $(MTK_ROOT)/external/aee/binary/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADER)/librilutils
LOCAL_C_INCLUDES += external/nanopb-c

ifneq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DFATAL_ERROR_HANDLE
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

# Define local __MTK_ENABLE_MD1 FO
ifeq ($(MTK_ENABLE_MD1), yes)
    LOCAL_CFLAGS += -D__MTK_ENABLE_MD1
endif

# Define local __MTK_ENABLE_MD2 FO
ifeq ($(MTK_ENABLE_MD2), yes)
    LOCAL_CFLAGS += -D__MTK_ENABLE_MD2
endif

# Define local __MTK_ENABLE_MD5 FO
ifeq ($(MTK_ENABLE_MD5), yes)
    LOCAL_CFLAGS += -D__MTK_ENABLE_MD5
endif

ifeq ($(MTK_SVLTE_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SVLTE_SUPPORT
endif

ifeq ($(MTK_IRAT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_IRAT_SUPPORT
endif

ifneq ($(MTK_INTERNAL),yes)
	LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

#build shared library
LOCAL_SHARED_LIBRARIES += \
    libcutils libutils
LOCAL_STATIC_LIBRARIES := \
    mtk-ril-prop-md1 \
    libprotobuf-c-nano-enable_malloc \

LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_MODULE:= mtk-ril
include $(BUILD_SHARED_LIBRARY)

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#====================================

include $(CLEAR_VARS)


#Mediatek Configuration
#ifeq ($(TARGET_PRODUCT),generic)
ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
LOCAL_SRC_FILES:= \
    ril_callbacks_emu.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels_emu.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c \
    ril_gsm_util.cpp \
    base64.c
else
LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_stk.c \
    ril_nw.c \
    ril_data.c \
    ril_sms.c \
    ril_oem.c \
    atchannels.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c \
    ril_gsm_util.cpp \
    base64.c
endif


LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libnetutils librilmtkmd2 librilutils


    # for asprinf

LOCAL_STATIC_LIBRARIES := \
    mtk-ril-prop-md2 \
    libprotobuf-c-nano-enable_malloc \

LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -D__CCMNI_SUPPORT__

ifneq ($(MTK_SHARE_MODEM_SUPPORT),1)
  LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_LTE_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_LTE_SUPPORT
endif

ifeq ($(MTK_IPV6_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_IPV6_SUPPORT -DINET6
endif

ifeq ($(MTK_GEMINI_3G_SWITCH),yes)
  LOCAL_CFLAGS += -DMTK_GEMINI_3G_SWITCH
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

LOCAL_SHARED_LIBRARIES += libmal libmdfx

LOCAL_C_INCLUDES :=

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(LOCAL_PATH)/../include \
#    $(MTK_ROOT)/external/aee/binary/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADER)/librilutils
LOCAL_C_INCLUDES += external/nanopb-c

ifneq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DFATAL_ERROR_HANDLE
endif

ifeq ($(MTK_SVLTE_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SVLTE_SUPPORT
endif

ifeq ($(MTK_IRAT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_IRAT_SUPPORT
endif

ifeq ($(MTK_MD_IRAT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_MD_IRAT_SUPPORT
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

ifneq ($(MTK_INTERNAL),yes)
	LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

#build shared library
LOCAL_SHARED_LIBRARIES += \
    libcutils libutils
LOCAL_STATIC_LIBRARIES := \
    mtk-ril-prop-md2 \
    libprotobuf-c-nano-enable_malloc \

LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_MODULE:= mtk-rilmd2
include $(BUILD_SHARED_LIBRARY)

endif #GOOGLE_RELEASE_RIL
