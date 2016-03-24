# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
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

################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, .)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES)
# IP Based  Modify ===============
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/featureio
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libutility
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/iopipe/PostProc
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/algorithm/libvfb


PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
MY_PLATFORM_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(PLATFORM)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include/mmsdk


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/include
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/include/mtkcam/algorithm/libutility
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/include/mtkcam/algorithm/libvfb
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/mtkcam/core/iopipe/PostProc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
# ===============================

#
#LOCAL_C_INCLUDES += $(TOP)/bionic $(TOP)/external/stlport/stlport
#
# IP Based Modify ==================
#LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc/Scenario
#LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/Shot/inc
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/adapter/inc/Scenario
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/adapter/Scenario/Shot/inc
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/mtkcam/featureio

#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include

LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/v1/adapter/inc/Scenario
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/v1/adapter/Scenario/Shot/inc
LOCAL_C_INCLUDES += $(MY_PLATFORM_PATH)/include/mtkcam/featureio
# this is for MtkCamera.h in new file strcture
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

# =============================

#
LOCAL_C_INCLUDES += $(TOP)/mediatek/kernel/include/linux
#
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/base/perfservice/perfservicenative
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/services
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

# IP Based Modify ===================
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc
#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

ifeq ($(PLATFORM),mt6735)
MY_CUSTOM := $(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/D1/inc
else
MY_CUSTOM := $(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc
endif
LOCAL_C_INCLUDES += $(MY_CUSTOM)
#$(warning "!!!MY CUSTOM = "$(MY_CUSTOM)", platform = "$(PLATFORM))

# IP Based Modify ========================
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include/mtkcam/feature/effectHal
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam/feature/effectHal

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += 
#
LOCAL_STATIC_LIBRARIES += 

#-----------------------------------------------------------

LOCAL_SHARED_LIBRARIES += libcutils
#LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libstdc++
#LOCAL_SHARED_LIBRARIES += libcam.campipe
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libm4u
LOCAL_SHARED_LIBRARIES += libcam.camshot
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libcamalgo
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcam.exif.v3
#LOCAL_SHARED_LIBRARIES += libfeatureio.feature
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam_utils

#-----------------------------------------------------------
LOCAL_MODULE := libfeatureio.cfb
#-----------------------------------------------------------
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

