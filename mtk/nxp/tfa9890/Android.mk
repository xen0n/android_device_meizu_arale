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

ifeq ($(strip $(NXP_SMARTPA_SUPPORT)),tfa9890)

LOCAL_PATH:= $(call my-dir)

#NOTE: for now we use static libs
############################## libtfahal linux
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	$(LOCAL_PATH)/hal/inc \
                    $(LOCAL_PATH)/tfa/inc \
                    $(LOCAL_PATH)/hal/src/lxScribo

LOCAL_SRC_FILES:= 	hal/src/NXP_I2C_linux.c  \
                    hal/src/lxScribo/lxScribo.c \
                    hal/src/lxScribo/lxDummy.c  \
                    hal/src/lxScribo/lxScriboSerial.c  \
                    hal/src/lxScribo/lxScriboSocket.c\
                    hal/src/lxScribo/lxI2c.c \
                    hal/src/lxScribo/scribosrv/i2cserver.c \
                    hal/src/lxScribo/scribosrv/cmd.c
LOCAL_MODULE := libtfahal
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)


############################### libtfa98xx 
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	$(LOCAL_PATH)/tfa/inc \
                    $(LOCAL_PATH)/hal/inc \
                    $(LOCAL_PATH)/hal/src \
                    $(LOCAL_PATH)/hal/src/lxScribo \
                    $(LOCAL_PATH)/hal/src/lxScribo/scribosrv

LOCAL_SRC_FILES := 	\
			              tfa/src/initTfa9890.c\
			              tfa/src/initTfa9890N1B12.c\
			              tfa/src/Tfa98xx.c\
			              tfa/src/Tfa98xx_TextSupport.c

LOCAL_MODULE := libtfa98xx
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################### libtfasrv 
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	$(LOCAL_PATH)/srv/inc \
                    $(LOCAL_PATH)/tfa/inc \
                    $(LOCAL_PATH)/hal/inc \
                    $(LOCAL_PATH)/hal/src \
                    $(LOCAL_PATH)/hal/src/lxScribo

LOCAL_SRC_FILES:=  \
                    srv/src/nxpTfa98xx.c\
                    srv/src/tfa98xxRuntime.c \
                    srv/src/tfa98xxCalibration.c \
                    srv/src/tfa98xxDiagnostics.c \
                    srv/src/tfa98xxLiveData.c
LOCAL_MODULE := libtfasrv
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfahal libtfa98xx 
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

############################## cli app
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= 	$(LOCAL_PATH)/app/climax/inc \
                    $(LOCAL_PATH)/srv/inc \
                    $(LOCAL_PATH)/tfa/inc \
                    $(LOCAL_PATH)/hal/inc \
                    $(LOCAL_PATH)/hal/src

LOCAL_SRC_FILES:=  \
                    app/climax/src/climax.c \
                    app/climax/src/cliCommands.c \
                    app/climax/src/cli/cmdline.c
LOCAL_MODULE := climax
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa98xx libtfahal  
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)

###############################################

endif
