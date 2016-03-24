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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES := ./mmsdkHAL.cpp \
                   ./imageTransformDevice.cpp \
                   ./gestureDevice.cpp \
                   ./effectDevice.cpp \


#-----------------------------------------------------------
LOCAL_C_INCLUDES += system/core/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libsync/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mmsdk/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

ifeq ($(TARGET_BOARD_PLATFORM), mt6797)
#swo
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/iopipe
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/iopipe
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(TARGET_BOARD_PLATFORM)/libfdft
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common/Gesture/GS1.0
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/sdk
else
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include/mtkcam/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include/mtkcam/iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/core/iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/include/mtkcam/sdk/hal
endif

# for mt6595 HDR
ifeq ($(MTK_PERFSERVICE_SUPPORT), yes)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/perfservice
LOCAL_CFLAGS += -DHDRSHOT_USE_PERFSERVICE
LOCAL_SHARED_LIBRARIES += libperfservicenative
endif


#-----------------------------------------------------------
# for fb
LOCAL_WHOLE_STATIC_LIBRARIES += libsched

LOCAL_SHARED_LIBRARIES += libmtkcamera_client
LOCAL_SHARED_LIBRARIES += libcam_mmp
#LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam_hwutils
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libcam.exif.v3
LOCAL_SHARED_LIBRARIES += libaed
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcamera_client


#end for fb

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
#-----------------------------------------------------------
#SImager
LOCAL_SHARED_LIBRARIES += libcam.iopipe

# For Effect Factory
ALLOWED_EFFECT_FACT_PLATFROM := mt6735
ALLOWED_EFFECT_FACT_PLATFROM += mt6755
ALLOWED_EFFECT_FACT_PLATFROM += mt6797
ALLOWED_EFFECT_FACT_PLATFROM += mt6753
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),$(ALLOWED_EFFECT_FACT_PLATFROM)))
LOCAL_SHARED_LIBRARIES += libfeatureio.featurefactory
endif

# For Gesture
$(info "IS_LEGACY=$(IS_LEGACY)")
ifeq ($(IS_LEGACY), 0)
LOCAL_WHOLE_STATIC_LIBRARIES += libfeature.gesture
# Gesture Algo
LOCAL_WHOLE_STATIC_LIBRARIES += libgd
# Gesture Algo use drvb_xxx()
LOCAL_WHOLE_STATIC_LIBRARIES += libdrvb
LOCAL_SHARED_LIBRARIES += libmtk_drvb
else

LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.scenario.shot
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libcam.camshot
LOCAL_SHARED_LIBRARIES += libcam1_utils
LOCAL_SHARED_LIBRARIES += libcam.camnode
LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libfeatureio
endif


#-----------------------------------------------------------
LOCAL_MODULE := mmsdk.$(TARGET_BOARD_PLATFORM)
#LOCAL_MODULE := mmsdk.commonhal
ifeq "21" "$(word 1, $(sort 21 $(PLATFORM_SDK_VERSION)))"
LOCAL_MODULE_RELATIVE_PATH := hw
else
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

LOCAL_CFLAGS += -Wall -Wextra -fvisibility=hidden

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
