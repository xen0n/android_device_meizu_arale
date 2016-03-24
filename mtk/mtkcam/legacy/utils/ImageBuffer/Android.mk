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
-include $(LOCAL_PATH)/../../../mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += BaseImageBuffer.cpp
LOCAL_SRC_FILES += BaseImageBufferHeap.cpp
LOCAL_SRC_FILES += ImageBufferAllocator.cpp
LOCAL_SRC_FILES += GrallocImageBufferHeap.cpp
LOCAL_SRC_FILES += GraphicImageBufferHeap.cpp
LOCAL_SRC_FILES += IonImageBufferHeap.cpp
LOCAL_SRC_FILES += ImageBufferHeap.cpp
LOCAL_SRC_FILES += DummyImageBufferHeap.cpp

#-----------------------------------------------------------
ifeq ($(BUILD_MTK_LDVT),true)
MTKCAM_DISABLE_GRALLOC     := '1'
else
MTKCAM_DISABLE_GRALLOC     := '0'
endif

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../gralloc_extra/include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/ext/include

LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../mmutils/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../gralloc_extra/include
#
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../libion_mtk/include
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../include
#

# xen0n
LOCAL_C_INCLUDES += $(MTKCAM_PATH)/../kernel-headers

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DMTKCAM_DISABLE_GRALLOC="$(MTKCAM_DISABLE_GRALLOC)"
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcam_utils.common.imagebuffer

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

