# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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

#-----------------------------------------------------------
include $(CLEAR_VARS)

#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += tuning_mgr/tuning_mgr.cpp
LOCAL_SRC_FILES += nvram/nvram_drv.cpp
LOCAL_SRC_FILES += nvram/nvram_buf_mgr.cpp
LOCAL_SRC_FILES += nvram/nvbuf_util.cpp
LOCAL_SRC_FILES += strobe/strobe_drv.cpp
LOCAL_SRC_FILES += strobe/strobe_global_driver.cpp
LOCAL_SRC_FILES += strobe/flashlight_drv.cpp
LOCAL_SRC_FILES += cam_cal/cam_cal_drv.cpp
LOCAL_SRC_FILES += cam_cal/camera_common_calibration.cpp
LOCAL_SRC_FILES += eeprom/eeprom_drv.cpp
LOCAL_SRC_FILES += eeprom/camera_common_calibration.cpp

#
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SRC_FILES += eis/eis_drv.cpp
    LOCAL_SRC_FILES += eis/eis_drv_r.cpp
    LOCAL_SRC_FILES += lens/mcu_drv.cpp
    LOCAL_SRC_FILES += lens/lens_drv.cpp
    LOCAL_SRC_FILES += lens/gaflens_drv.cpp
    LOCAL_SRC_FILES += lens/lens_sensor_drv.cpp
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

#
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..


#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/kernel/lens/inc/D1
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/kernel/flashlight/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/camera
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/camera_3a
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/nvram/libnvram
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/drv/imem
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ispdrv_mgr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/nvram/nvramagentclient



#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES +=

#
LOCAL_STATIC_LIBRARIES += libfeatureiopipe_aaa
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
#
#LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libnvramagentclient
LOCAL_SHARED_LIBRARIES += libcam_hwutils

#
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += libnvram
    LOCAL_SHARED_LIBRARIES += libcameracustom
endif
#-----------------------------------------------------------
LOCAL_MODULE := libfeatureiodrv
#-----------------------------------------------------------
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
#-----------------------------------------------------------
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
