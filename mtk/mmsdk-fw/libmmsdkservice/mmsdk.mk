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
# HAL Client 
################################################################################
MTKCAM_HAVE_SDK_CLIENT                 ?= '1'


#-----------------------------------------------------------
ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
    MTKCAM_HAVE_SDK_CLIENT                 := '0'
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), mt6797)
    MTKCAM_HAVE_SDK_CLIENT                 := '0'
    MTKCAM_HAVE_IIMAGE_BUFFER              := '0'
endif

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_SDK_CLIENT))"
    LOCAL_CFLAGS += -DMTKCAM_HAVE_SDK_CLIENT="$(MTKCAM_HAVE_SDK_CLIENT)"
    LOCAL_SHARED_LIBRARIES += libcam.sdkclient    
endif

#-----------------------------------------------------------
ifeq ($(strip $(MTK_HEART_RATE_MONITOR_SUPPORT)),yes)
MTKCAM_HR_MONITOR_SUPPORT                 := '1'
else
MTKCAM_HR_MONITOR_SUPPORT                 := '0'
endif
LOCAL_CFLAGS += -DMTKCAM_HR_MONITOR_SUPPORT="$(MTKCAM_HR_MONITOR_SUPPORT)"


################################################################################
# HAL IImage buffer 
################################################################################
MTKCAM_HAVE_IIMAGE_BUFFER       ?= '1'

#-----------------------------------------------------------
ifeq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
    MTKCAM_HAVE_IIMAGE_BUFFER       := '0'
endif 

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_IIMAGE_BUFFER))"
    LOCAL_CFLAGS += -DMTKCAM_HAVE_IIMAGE_BUFFER="$(MTKCAM_HAVE_IIMAGE_BUFFER)"
    LOCAL_SHARED_LIBRARIES +=  libcam_utils
endif 

