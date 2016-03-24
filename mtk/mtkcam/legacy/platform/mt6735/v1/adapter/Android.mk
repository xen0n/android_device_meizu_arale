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
-include $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/adapter.mk

#-----------------------------------------------------------
MTKCAM_HAVE_MTKATV              := '0'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKPHOTO            := '0'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKDEFAULT          := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKENG              := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKSTEREO           := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKVT               := '0'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_MTKZSD              := '0'      # built-in if '1' ; otherwise not built-in
#
MTKCAM_HAVE_SENSOR_HAL          := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_3A_HAL              := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_CAMPIPE             := '0'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_CAMSHOT             := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_CAMPROFILE          := '0'      # built-in if '1' ; otherwise not built-in

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKATV))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKATV="$(MTKCAM_HAVE_MTKATV)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKPHOTO))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKPHOTO="$(MTKCAM_HAVE_MTKPHOTO)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKDEFAULT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKDEFAULT="$(MTKCAM_HAVE_MTKDEFAULT)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKENG))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKENG="$(MTKCAM_HAVE_MTKENG)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKSTEREO="$(MTKCAM_HAVE_MTKSTEREO)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKVT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKVT="$(MTKCAM_HAVE_MTKVT)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKZSD))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKZSD="$(MTKCAM_HAVE_MTKZSD)"
endif
ifeq "'1'" "$(strip $(MTKCAM_HAVE_SENSOR_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_3A_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMPIPE))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMPIPE="$(MTKCAM_HAVE_CAMPIPE)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMSHOT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMSHOT="$(MTKCAM_HAVE_CAMSHOT)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMPROFILE))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMPROFILE="$(MTKCAM_HAVE_CAMPROFILE)"
endif
#

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    BaseCamAdapter.Instance.cpp \
    BaseCamAdapter.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
#
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKATV))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkatv
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKPHOTO))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkphoto
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKDEFAULT))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkdefault
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKENG))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkeng
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkstereo
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKVT))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkvt
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKZSD))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.mtkzsd
endif
#
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.scenario
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.camnode
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.callback
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.thermalmonitor

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libdl
#LOCAL_SHARED_LIBRARIES += libjpeg
LOCAL_SHARED_LIBRARIES += libaed 
#
LOCAL_SHARED_LIBRARIES += libcamera_client libmtkcamera_client
LOCAL_SHARED_LIBRARIES += libskia
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam1_utils
LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libcam.exif.v3
LOCAL_SHARED_LIBRARIES += libcam_mmp
LOCAL_SHARED_LIBRARIES += libcam_hwutils
#
LOCAL_SHARED_LIBRARIES += libcamdrv
LOCAL_SHARED_LIBRARIES += libimageio
LOCAL_SHARED_LIBRARIES += libfeatureio
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMPIPE))"
LOCAL_SHARED_LIBRARIES += libcam.campipe
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMSHOT))"
LOCAL_SHARED_LIBRARIES += libcam.camshot
endif
#
LOCAL_SHARED_LIBRARIES += libcam.camnode
#
LOCAL_SHARED_LIBRARIES += libJpgEncPipe 
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#
LOCAL_SHARED_LIBRARIES += libcam.iopipe

LOCAL_SHARED_LIBRARIES += libcam.utils.cpuctrl
#
LOCAL_SHARED_LIBRARIES += libmmsdkservice.feature

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.camadapter

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

