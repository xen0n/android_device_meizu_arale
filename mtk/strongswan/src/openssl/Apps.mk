#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
# Copyright 2006 The Android Open Source Project

LOCAL_PATH := $(call my-dir)

# include $(CLEAR_VARS)
# LOCAL_MODULE := openssl-ss
# LOCAL_MULTILIB := both
# LOCAL_MODULE_STEM_32 := openssl-ss
# LOCAL_MODULE_STEM_64 := openssl64-ss
# LOCAL_CLANG := true
# LOCAL_MODULE_TAGS := optional
# LOCAL_SHARED_LIBRARIES := libssl-ss libcrypto-ss libcutils
# include $(LOCAL_PATH)/Apps-config-target.mk
# include $(LOCAL_PATH)/android-config.mk
# LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Apps.mk
# include $(BUILD_EXECUTABLE)
# 
# include $(CLEAR_VARS)
# LOCAL_MODULE := openssl-ss
# LOCAL_MODULE_TAGS := optional
# LOCAL_SHARED_LIBRARIES := libssl-host-ss libcrypto-host-ss
# LOCAL_CFLAGS += -DOPENSSL_HOST_LOGD
# include $(LOCAL_PATH)/Apps-config-host.mk
# include $(LOCAL_PATH)/android-config.mk
# LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Apps.mk
# include $(BUILD_HOST_EXECUTABLE)
