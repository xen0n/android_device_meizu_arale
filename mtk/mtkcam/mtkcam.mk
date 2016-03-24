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

# xen0n: directory layout
MTKCAM_PATH := $(call my-dir)

################################################################################
#
################################################################################
-include $(TOP)/$(MTK_PATH_CUSTOM)/hal/mtkcam/mtkcam.mk
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)

#-----------------------------------------------------------
# Camera ip base design version control
#-----------------------------------------------------------
ifeq ($(TARGET_BOARD_PLATFORM), mt6797)
IS_LEGACY = 0
MTKCAM_HAL_VERSION := v1.4
MTKCAM_3A_VERSION := IPv1.1
MTKCAM_DRV_VERSION := $(PLATFORM)
MTKCAM_ALGO_VERSION := $(PLATFORM)
else
IS_LEGACY = 1
endif

#-----------------------------------------------------------
# Camera ip base design include path
#-----------------------------------------------------------
ifeq ($(IS_LEGACY), 0)
#Middleware
MTKCAM_HAL_INCLUDE := $(MTKCAM_PATH)/middleware/$(MTKCAM_HAL_VERSION)/include
#3A
MTKCAM_3A_INCLUDE := $(MTKCAM_PATH)/aaa/include/$(MTKCAM_3A_VERSION)
#Drv
MTKCAM_DRV_INCLUDE := $(MTKCAM_PATH)/drv/include/$(MTKCAM_DRV_VERSION)
#Algo
MTKCAM_ALGO_INCLUDE := $(MTKCAM_PATH)/include/algorithm/$(MTKCAM_ALGO_VERSION)

MTKCAM_C_INCLUDES := $(MTKCAM_PATH)/common/include

# this define is used by cpp & h to separate legacy path & MTKCAM ip base path
LOCAL_CFLAGS += -DMTK_CAMERA_IP_BASE

else
MTKCAM_C_INCLUDES := $(MTKCAM_PATH)/legacy/include/mtkcam

endif

# path for legacy platform
MTK_MTKCAM_PLATFORM    := $(MTKCAM_PATH)/legacy/platform/$(PLATFORM)
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mt6735m))
MTK_MTKCAM_PLATFORM    := $(MTKCAM_PATH)/legacy/platform/mt6735m
endif


#-----------------------------------------------------------
# MTKCAM_CFLAGS define
# In Android.mk, add followed words to use it:
# LOCAL_CFLAGS + = MTKCAM_CFLAGS
#-----------------------------------------------------------
# MTKCAM_HAVE_AEE_FEATURE
ifeq "yes" "$(strip $(HAVE_AEE_FEATURE))"
    MTKCAM_HAVE_AEE_FEATURE ?= 1
else
    MTKCAM_HAVE_AEE_FEATURE := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_AEE_FEATURE=$(MTKCAM_HAVE_AEE_FEATURE)
#-----------------------------------------------------------
# MTK_BASIC_PACKAGE
ifneq ($(MTK_BASIC_PACKAGE), yes)
    MTKCAM_BASIC_PACKAGE := 0
else
    MTKCAM_BASIC_PACKAGE := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_BASIC_PACKAGE=$(MTKCAM_BASIC_PACKAGE)
#-----------------------------------------------------------
# ZSD+MFLL
ifeq "yes" "$(strip $(MTK_CAM_ZSDMFB_SUPPORT))"
    MTK_CAM_HAVE_ZSDMFB_SUPPORT := 1
else
    MTK_CAM_HAVE_ZSDMFB_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_HAVE_ZSDMFB_SUPPORT=$(MTK_CAM_HAVE_ZSDMFB_SUPPORT)
#-----------------------------------------------------------
# ZSD+HDR
ifeq "yes" "$(strip $(MTK_CAM_ZSDHDR_SUPPORT))"
    MTK_CAM_HAVE_ZSDHDR_SUPPORT := 1
else
    MTK_CAM_HAVE_ZSDHDR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_HAVE_ZSDHDR_SUPPORT=$(MTK_CAM_HAVE_ZSDHDR_SUPPORT)
#-----------------------------------------------------------
# MTKCAM_HAVE_RR_PRIORITY
MTKCAM_HAVE_RR_PRIORITY      ?= 0  # built-in if '1' ; otherwise not built-in
MTKCAM_CFLAGS += -DMTKCAM_HAVE_RR_PRIORITY=$(MTKCAM_HAVE_RR_PRIORITY)
#-----------------------------------------------------------
# L1_CACHE_BYTES for 32-byte cache line
MTKCAM_CFLAGS += -DL1_CACHE_BYTES=32

# xen0n: MTK defines are not automatically propagated to compiler in non-MTK
# build systems, so we have to do that ourselves.
# MTK_ION_SUPPORT
ifeq "yes" "$(strip $(MTK_ION_SUPPORT))"
    MTKCAM_CFLAGS += -DMTK_ION_SUPPORT=1
endif

