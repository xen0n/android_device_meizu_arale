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
#===============================================================================


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_SRC_FILES += \
    flicker_hal.cpp flicker_util.cpp flicker_hal_base.cpp

LOCAL_C_INCLUDES:= \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/algorithm/libflicker \
    $(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/sensor_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_mgr    \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ispdrv_mgr    \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/featureio \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/drv/imgsensor \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/custom/hal/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/custom/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/custom/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/isp_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/sensor_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/ae_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/af_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/featureio/pipe/aaa/ispdrv_mgr \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/imgsensor \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/aaa \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/mtkcam \
    $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/isp_tuning \
    $(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
    $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/cam \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/cam_cal \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/awb_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/nvram_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/buf_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ispdrv_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/inc \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/pca_mgr/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning/paramctrl/ccm_mgr/ \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/lsc_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ae_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/flash_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/af_mgr \
    $(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/sensor_mgr \
    $(MTK_PATH_PLATFORM)/hardware/hardware/include/D1/mtkcam/algorithm/lib3a \
    $(MTK_PATH_PLATFORM)/hardware/hardware/include/D1/mtkcam/algorithm/liblsctrans \
    $(MTK_PATH_PLATFORM)/hardware/hardware/include/D1/mtkcam/algorithm/libtsf\
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware \
    $(MTK_MTKCAM_PLATFORM)/core/drv/imgsensor \
    $(MTK_MTKCAM_PLATFORM)/core/imageio/inc \


LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
    
#.MT6595.SB.DEV\alps\mediatek\platform\mt6595\hardware\include\mtkcam\drv
#e\mtkcam\core\featureio\pipe\aaa\ispdrv_mgr
#EV\KK0.MT6595.SB.DEV\alps\mediatek\platform\mt6595\hardware\mtkcam\core\featureio\pipe\aaa\isp_mgr
#595.SB.DEV\alps\mediatek\hardware\include\mtkcam\hal

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/featureio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/camera/inc/drv

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \





LOCAL_MODULE := libfeatureiopipe_flk


include $(BUILD_STATIC_LIBRARY)
