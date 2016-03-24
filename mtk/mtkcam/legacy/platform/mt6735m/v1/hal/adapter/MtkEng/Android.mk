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

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
    LOCAL_CPPFLAGS += -DMTK_TC1_FEATURE
    LOCAL_CFLAGS += -DMTK_A3MAC
    LOCAL_CFLAGS += -DMTK_MAC_EBORTS2
    LOCAL_CPPFLAGS += -DMTK_MAC_EBORTS2
endif

#-----------------------------------------------------------
LOCAL_SRC_FILES += CaptureCmdQueThread.cpp
LOCAL_SRC_FILES += MtkEngCamAdapter.3A.cpp
LOCAL_SRC_FILES += MtkEngCamAdapter.Capture.cpp
LOCAL_SRC_FILES += MtkEngCamAdapter.CaptureCallback.cpp
LOCAL_SRC_FILES += MtkEngCamAdapter.cpp
LOCAL_SRC_FILES += MtkEngCamParameter.cpp

LOCAL_SRC_FILES += Preview/MtkEngCamAdapter.Preview.cpp
LOCAL_SRC_FILES += Preview/PreviewBufMgr.cpp
LOCAL_SRC_FILES += Preview/PreviewCmdQueThread.cpp
LOCAL_SRC_FILES += Preview/RawDumpCmdQueThread.cpp
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/mtkcam/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/mtkcam/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/mtkcam/inc/common/camutils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/v1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common/camutils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/Scenario/Shot/EngShot
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc/ 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/flash_mgr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ae_mgr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/sensor_mgr
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/hal/aaa/flash_mgr\
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/hal/aaa/ae_mgr\
#
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/isp_tuning

#LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/cam
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/featureio/pipe/aaa/ae_mgr
#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/lsc_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ispdrv_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/nvram_mgr \

#-----------------------------------------------------------
#LOCAL_CFLAGS +=

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkeng.state
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcam.camadapter.mtkeng

#-----------------------------------------------------------

#

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..


# End of common part ---------------------------------------
#
include $(BUILD_STATIC_LIBRARY)




################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

