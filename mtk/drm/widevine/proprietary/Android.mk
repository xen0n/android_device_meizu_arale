# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2013. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


LOCAL_PATH:= $(call my-dir)

include $(call all-subdir-makefiles)

ifeq ($(strip $(MTK_WVDRM_SUPPORT)),yes)
#  $(warning "Widevine DRM is enabled")

ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
    BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 1
else
    BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3
endif

#$(warning "BOARD_WIDEVINE_OEMCRYPTO_LEVEL = $(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)")

include $(CLEAR_VARS)

#########################################################################
# libwvm.so

include $(CLEAR_VARS)

$(warning "Build libwvm")

LOCAL_MODULE := libwvm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_STRIP_MODULE := true
LOCAL_STATIC_LIBRARIES += liboemcrypto_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libwvmcommon_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_SHARED_LIBRARIES := \
    libstagefright \
    libWVStreamControlAPI_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libdrmframework \
    libcutils \
    liblog \
    libutils \
    libz \
    libdl \
    libdrmmtkutil

ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
    ifeq ($(MTK_IN_HOUSE_TEE_SUPPORT), yes)
        #LOCAL_SHARED_LIBRARIES += lib_uree_mtk_crypto
    else ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
        #LOCAL_SHARED_LIBRARIES += libtlcWidevine
        #LOCAL_SHARED_LIBRARIES += libMcClient libsec_mem
        #LOCAL_SHARED_LIBRARIES += libcutils libnetutils libc libft
        ifeq ($(MTK_DRM_KEY_MNG_NOT_READY), yes)
            #LOCAL_STATIC_LIBRARIES += liburee_meta_drmkeyinstall
        else
            #LOCAL_STATIC_LIBRARIES += liburee_meta_drmkeyinstall
        endif
    else
        # nothing
    endif
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := widevine
LOCAL_MULTILIB := 32
include $(BUILD_SHARED_LIBRARY)

#########################################################################
# libdrmwvmplugin.so

include $(CLEAR_VARS)

$(warning "Build libdrmwvmplugin")

LOCAL_MODULE := libdrmwvmplugin
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/drm
LOCAL_MODULE_RELATIVE_PATH := drm
LOCAL_PRELINK_MODULE := false
LOCAL_STRIP_MODULE := true
LOCAL_WHOLE_STATIC_LIBRARIES += liboemcrypto_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_WHOLE_STATIC_LIBRARIES += \
    libdrmwvmcommon

LOCAL_STATIC_LIBRARIES += \
    libdrmframeworkcommon \
    libdrmwvmcommon \
    libwvocs_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL)

LOCAL_SHARED_LIBRARIES += \
    libbinder \
    libutils \
    libcutils \
    libz \
    libwvdrm_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libWVStreamControlAPI_L$(BOARD_WIDEVINE_OEMCRYPTO_LEVEL) \
    libdl \
    libdrmmtkutil

ifeq ($(strip $(MTK_WVDRM_L1_SUPPORT)),yes)
    ifeq ($(MTK_IN_HOUSE_TEE_SUPPORT), yes)
        #LOCAL_SHARED_LIBRARIES += lib_uree_mtk_crypto
    else ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
        #LOCAL_SHARED_LIBRARIES += libtlcWidevine
        #LOCAL_SHARED_LIBRARIES += libMcClient libsec_mem
        #LOCAL_SHARED_LIBRARIES += libcutils libnetutils libc libft
        ifeq ($(MTK_DRM_KEY_MNG_NOT_READY), yes)
            #LOCAL_STATIC_LIBRARIES += liburee_meta_drmkeyinstall
        else
            #LOCAL_STATIC_LIBRARIES += liburee_meta_drmkeyinstall
        endif
    else
        # nothing
    endif
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := widevine
LOCAL_MULTILIB := 32
include $(BUILD_SHARED_LIBRARY)

else
  $(warning "Widevine DRM is disabled")
endif
