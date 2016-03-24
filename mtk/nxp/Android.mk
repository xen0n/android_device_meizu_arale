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

############################## libtfa9890_interface
include $(CLEAR_VARS)
LOCAL_CFLAGS += -DNXP_TFA9890_SUPPORT
LOCAL_C_INCLUDES:=  $(LOCAL_PATH)/tfa9890 \
                    $(LOCAL_PATH)/tfa9890/hal/inc \
                    $(LOCAL_PATH)/tfa9890/tfa/inc \
                    $(LOCAL_PATH)/tfa9890/srv/inc
                    
LOCAL_SRC_FILES := 	tfa9890/tfa9890_interface.c \
                    mtk_tfa98xx_interface.c

LOCAL_MODULE := libtfa9890_interface
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfasrv libtfa98xx libtfahal
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

###############################################

include $(call all-makefiles-under,$(LOCAL_PATH))

endif

ifeq ($(NXP_SMARTPA_SUPPORT),yes)

LOCAL_PATH:= $(call my-dir)

############################## libtfa9887_interface
include $(CLEAR_VARS)
LOCAL_CFLAGS += -DNXP_TFA9887_SUPPORT
LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/tfa9887/interface \
    $(LOCAL_PATH)/tfa9887/climax/inc \
    $(LOCAL_PATH)/tfa9887/climax/src/lxScribo \
    $(LOCAL_PATH)/tfa9887/climax/src/lxScribo/scribosrv \
    $(LOCAL_PATH)/tfa9887/Tfa98xxhal/inc \
    $(LOCAL_PATH)/tfa9887/Tfa98xxhal/src/lxScribo \
    $(LOCAL_PATH)/tfa9887/Tfa98xxhal/inc \
    $(LOCAL_PATH)/tfa9887/Tfa98xx/inc \
    $(LOCAL_PATH)/tfa9887/Tfa98xx/src
                    
LOCAL_SRC_FILES := 	tfa9887/interface/tfa9887_interface.c \
                    mtk_tfa98xx_interface.c

LOCAL_MODULE := libtfa9887_interface
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_STATIC_LIBRARIES:= libtfa9887 liblxScribo9887
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

###############################################

include $(call all-makefiles-under,$(LOCAL_PATH))

endif

