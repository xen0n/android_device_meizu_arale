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
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += Hal3AAdapter3.cpp
LOCAL_SRC_FILES += IHal3A.cpp
LOCAL_SRC_FILES += EventIrq/DefaultEventIrq.cpp
LOCAL_SRC_FILES += EventIrq/HwEventIrq.cpp
LOCAL_SRC_FILES += aaa_hal_if.cpp
LOCAL_SRC_FILES += aaa_hal_flowCtrl.cpp
LOCAL_SRC_FILES += aaa_hal_flowCtrl.thread.cpp
LOCAL_SRC_FILES += aaa_hal_raw.cpp
LOCAL_SRC_FILES += aaa_hal_raw.thread.cpp
LOCAL_SRC_FILES += ResultBufMgr/ResultBufMgr.cpp
LOCAL_SRC_FILES += state_mgr/aaa_state.cpp
LOCAL_SRC_FILES += state_mgr/aaa_state_camera_preview.cpp
#LOCAL_SRC_FILES += state_mgr/aaa_state_camcorder_preview.cpp
#LOCAL_SRC_FILES += state_mgr/aaa_state_recording.cpp
LOCAL_SRC_FILES += state_mgr/aaa_state_af.cpp
LOCAL_SRC_FILES += state_mgr/aaa_state_precapture.cpp
#LOCAL_SRC_FILES += state_mgr/aaa_state_capture.cpp
LOCAL_SRC_FILES += state_mgr/aaa_state_mgr.cpp
LOCAL_SRC_FILES += sensor_mgr/aaa_sensor_mgr.cpp
LOCAL_SRC_FILES += sensor_mgr/aaa_sensor_buf_mgr.cpp
#LOCAL_SRC_FILES += nvram_mgr/nvram_drv_mgr.cpp  //ME14@@
LOCAL_SRC_FILES += awb_mgr/awb_mgr.cpp
LOCAL_SRC_FILES += awb_mgr/awb_mgr_if.cpp
LOCAL_SRC_FILES += awb_mgr/awb_cct_feature.cpp
LOCAL_SRC_FILES += awb_mgr/awb_state.cpp
LOCAL_SRC_FILES += ae_mgr/ae_mgr.cpp
LOCAL_SRC_FILES += ae_mgr/ae_mgr_ctrl.cpp
LOCAL_SRC_FILES += ae_mgr/ae_cct_feature.cpp
LOCAL_SRC_FILES += ae_mgr/ae_mgr_if.cpp
LOCAL_SRC_FILES += af_mgr/af_mgr.cpp
LOCAL_SRC_FILES += isp_mgr/isp_mgr.cpp
LOCAL_SRC_FILES += isp_mgr/isp_mgr_helper.cpp
LOCAL_SRC_FILES += isp_mgr/isp_debug.cpp
LOCAL_SRC_FILES += buf_mgr/aao_buf_mgr.cpp
LOCAL_SRC_FILES += buf_mgr/afo_buf_mgr.cpp
LOCAL_SRC_FILES += ispdrv_mgr/ispdrv_mgr.cpp
LOCAL_SRC_FILES += isp_tuning/isp_tuning_mgr.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_lifetime.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_user.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_attributes.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_validate.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_per_frame.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_frameless.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/paramctrl_exif.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/pca_mgr/pca_mgr.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/ccm_mgr/ccm_mgr.cpp
LOCAL_SRC_FILES += isp_tuning/paramctrl/ggm_mgr/ggm_mgr.cpp
LOCAL_SRC_FILES += lsc_mgr/lsc_mgr2.cpp
LOCAL_SRC_FILES += lsc_mgr/lsc_mgr2.thread.cpp
LOCAL_SRC_FILES += lsc_mgr/lsc_mgr2.misc.cpp
LOCAL_SRC_FILES += flash_mgr/flash_mgr.cpp
LOCAL_SRC_FILES += flash_mgr/flash_util.cpp
#LOCAL_SRC_FILES += flash_mgr/flash_cct_test.cpp
#LOCAL_SRC_FILES += flash_mgr/flash_cct.cpp
LOCAL_SRC_FILES += flash_mgr/flash_mgr_m.cpp
LOCAL_SRC_FILES += flash_mgr/flash_pline_tool.cpp
LOCAL_SRC_FILES += flash_mgr/flash_hal.cpp
LOCAL_SRC_FILES += flicker/flicker_hal.cpp
LOCAL_SRC_FILES += flicker/flicker_hal_base.cpp
LOCAL_SRC_FILES += flicker/flicker_util.cpp
LOCAL_SRC_FILES += Thread/ThreadSensorGainImp.cpp
LOCAL_SRC_FILES += Thread/ThreadSensorI2CImp.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/camera
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/mtkcam
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/core/featureio/drv/inc
#LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/hardware
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/awb_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/af_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/ae_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/flash_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/nvram_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/buf_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/ispdrv_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_tuning
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_tuning/paramctrl/inc
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_tuning/paramctrl/pca_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_tuning/paramctrl/ccm_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/isp_tuning/paramctrl/ggm_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/lsc_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/hal/aaa/sensor_mgr
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/cgen/cfgfileinc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/algorithm/libflicker
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/algorithm/lib3a

LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera_3a/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc

LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/core/drv/nvram
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include//mtkcam/featureio
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include/








#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_CFLAGS += -DUSE_AE_THD=1
LOCAL_CFLAGS += -DCAM3_3ATESTLVL=1

LOCAL_CFLAGS += -DCAM3_3AUT=0
LOCAL_CFLAGS += -DCAM3_3AIT=1
LOCAL_CFLAGS += -DCAM3_3AON=2

ifeq ($(strip $(MTK_NATIVE_3D_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_NATIVE_3D_SUPPORT
endif
#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES += libfeatureio.aaa.custom
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libcamdrv
#
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libcamdrv_FrmB
#LOCAL_SHARED_LIBRARIES += libcamnvutil
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcamalgo
#LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam.metadata
LOCAL_SHARED_LIBRARIES += libcam.metadataprovider
LOCAL_SHARED_LIBRARIES += libcam.utils.sensorlistener
LOCAL_SHARED_LIBRARIES += libcam.iopipe_FrmB
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.dng
#
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
endif
#
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += lib3a
#    LOCAL_SHARED_LIBRARIES += lib3a_sample
    LOCAL_SHARED_LIBRARIES += libnvram
    LOCAL_SHARED_LIBRARIES += libcam.exif
    LOCAL_SHARED_LIBRARIES += libcameracustom
    LOCAL_SHARED_LIBRARIES += libcam_mmp
    LOCAL_SHARED_LIBRARIES += libdpframework
endif
#-----------------------------------------------------------
#LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libcam.hal3a.v3

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


