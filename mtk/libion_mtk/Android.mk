#
# Copyright (C) 2011-2014 MediaTek Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := ion.c
LOCAL_MODULE := libion_mtk
LOCAL_MODULE_TAGS := optional

# xen0n: build in AOSP layout
# ./include is MTK's $(TOP)/vendor/mediatek/proprietary/external/include
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/kernel-headers \
    $(TOP)/system/core/include

LOCAL_SHARED_LIBRARIES := libion liblog

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
