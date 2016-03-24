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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    CamShotImp.cpp \
    CamShotUtils.cpp \
    SingleShot/ISingleShotBridge.cpp \
    SingleShot/SingleShot.cpp \
    SingleShot/SingleShotCtrlNode.cpp \
    CapBuf/CapBufMgr.cpp \
    CapBuf/CapBufNode.cpp \
    MultiShot/MultiShot.cpp \
    MultiShot/MShotCtrlNode.cpp \
    MultiShot/IMultiShotBridge.cpp \
    MfllShot/IMfllShotBridge.cpp \
    MfllShot/MfllShot.cpp \
    MfllShot/BestShotSelection.cpp \
    BurstShot/IBurstShotBridge.cpp \
    BurstShot/BurstShot.cpp \
    BurstShot/BShotCtrlNode.cpp \
    SmartShot/ISmartShotBridge.cpp \
    StereoShot/IStereoShotBridge.cpp \
    StereoShot/StereoShot.cpp \
    StereoShot/StereoShotCtrlNode.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)
#
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#
# camera Hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

#
# custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc
$(warning "[Remove Me] chide.chen shouldn't include this path")
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc
#
# m4u
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)
#
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/dpframework/inc
#LOCAL_C_INCLUDES += $(TOP)/external/jpeg
#
# temp for using iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

# MTK_CAM_MFB_SUPPORT 0: off, 1:  mfll, 2: ais, 3:both, 4: sqc
ifeq ($(strip $(MTK_CAM_MFB_SUPPORT)), 0)
  LOCAL_CFLAGS += -DMTK_CAM_MFB_SUPPORT_LEVEL=0
endif
ifeq ($(strip $(MTK_CAM_MFB_SUPPORT)), 1)
  LOCAL_CFLAGS += -DMTK_CAM_MFB_SUPPORT_LEVEL=1
endif
ifeq ($(strip $(MTK_CAM_MFB_SUPPORT)), 2)
  LOCAL_CFLAGS += -DMTK_CAM_MFB_SUPPORT_LEVEL=2
endif
ifeq ($(strip $(MTK_CAM_MFB_SUPPORT)), 3)
  LOCAL_CFLAGS += -DMTK_CAM_MFB_SUPPORT_LEVEL=3
endif
ifeq ($(strip $(MTK_CAM_MFB_SUPPORT)), 4)
  LOCAL_CFLAGS += -DMTK_CAM_MFB_SUPPORT_LEVEL=4
endif

#-----------------------------------------------------------
# GMO project
#-----------------------------------------------------------
ifeq ($(strip $(MTK_GMO_RAM_OPTIMIZE)), yes)
  LOCAL_CFLAGS += -DMFLL_GMO_RAM_OPTIMIZE=1
else
  LOCAL_CFLAGS += -DMFLL_GMO_RAM_OPTIMIZE=0
endif

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camshot.utils
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam_hwutils
LOCAL_SHARED_LIBRARIES += libcam.exif
#
# camnode
LOCAL_SHARED_LIBRARIES += libcam.camnode
#
# sensor hal
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#
# iopipe
LOCAL_SHARED_LIBRARIES += libcam.iopipe
#
# for 3A
LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libfeatureio
#
# for jpeg enc use
#LOCAL_SHARED_LIBRARIES += libm4u libJpgEncPipe
#
# temp for imem & isp_drv
LOCAL_SHARED_LIBRARIES += libcamdrv

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.camshot

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

