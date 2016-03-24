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
ifneq ($(TARGET_BUILD_PDK),true)
LOCAL_PATH := $(call my-dir)

################################################################################
# MMSDK test 
################################################################################
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
LOCAL_SRC_FILES := ./Cmd_test_service.cpp \
                   ./Cmd_exit.cpp \
                   ./CmdMap.cpp \
                   ./Main.cpp \
                   ./Utils.cpp \
                   ./Cmd_test_client_scaler.cpp \
                   ./Cmd_test_client_effect.cpp \
                   ./Cmd_test_client_gesture.cpp \
                   ./Cmd_test_client_heartrate.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include/ \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/include/ \
    $(TOP)/$(MTKCAM_C_INCLUDES) \
    $(TOP)/system/media/camera/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/hardware/include #remove later

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), mt6797)
#swo
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
else
endif

#-----------------------------------------------------------
LOCAL_MODULE := mmsdktest
#LOCAL_MODULE_TAGS := test
LOCAL_MODULE_TAGS := eng
LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libui


LOCAL_SHARED_LIBRARIES += libmmsdkservice
LOCAL_32_BIT_ONLY := true

include $(BUILD_EXECUTABLE)
#-------------------------------------------------------------------------------
endif
