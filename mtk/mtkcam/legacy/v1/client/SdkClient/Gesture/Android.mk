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
-include $(LOCAL_PATH)/../../../../../mtkcam.mk
#-----------------------------------------------------------
sinclude $(MTK_MTKCAM_PLATFORM)/hardware/mtkcam/mtkcam.mk
sinclude $(MTKCAM_PATH)/../include/mtkcam/v1/moduleOption.mk

#-----------------------------------------------------------
MTKSDK_HAVE_GESTURE_CLIENT  ?= '1'  # built-in if '1' ; otherwise not built-in
$(warning "MTKSDK_HAVE_GESTURE_CLIENT:$(MTKSDK_HAVE_GESTURE_CLIENT)")
#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-cpp-files-under, .)
$(warning "LOCAL_SRC_FILES:$(LOCAL_SRC_FILES)")

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../gralloc_extra/include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/ext/include

LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../include/mmsdk
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../mmsdk/include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/legacy/include/mmsdk
#LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
#
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/ext/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

# xen0n
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../kernel-headers

$(warning "SdkClient Gesture LOCAL_C_INCLUDES: $(LOCAL_C_INCLUDES)")
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_MODULE := libcam.sdkclient.gesture
#

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKSDK_HAVE_GESTURE_CLIENT))"
    $(warning "Build Gesture Client")
    include $(BUILD_STATIC_LIBRARY)
else
    $(warning "Not Build Gesture Client")
endif

