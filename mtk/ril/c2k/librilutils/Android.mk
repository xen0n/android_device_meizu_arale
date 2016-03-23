# Copyright 2013 The Android Open Source Project
ifeq ($(MTK_C2K_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c
    
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ril/viatelecom/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include

LOCAL_CFLAGS :=

LOCAL_MODULE:= libviarilutils
LOCAL_MULTILIB := 32
#LOCAL_LDLIBS += -lpthread

include $(BUILD_SHARED_LIBRARY)


# Create static library for those that want it
# =========================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    librilutils.c \
    record_stream.c


LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ril/viatelecom/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include

LOCAL_STATIC_LIBRARIES :=

LOCAL_CFLAGS :=

LOCAL_MODULE:= libviarilutils_static
LOCAL_MULTILIB := 32
#LOCAL_LDLIBS += -lpthread

include $(BUILD_STATIC_LIBRARY)

endif
