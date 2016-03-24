#
# camshottest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    main.cpp \
    main_node.cpp \
    main_passxnode.cpp \

#
# 
# camera Hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc

# temp for using iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/inc/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach/

# vector
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \

# node 
LOCAL_SHARED_LIBRARIES += \
    libcam.camnode \

# Imem
LOCAL_SHARED_LIBRARIES += \
    libcamdrv \

LOCAL_SHARED_LIBRARIES += \
	libcam.halsensor

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := camnodetest

#
LOCAL_MODULE_TAGS := eng

#
LOCAL_PRELINK_MODULE := false
#

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

# End of common part ---------------------------------------
#
include $(BUILD_EXECUTABLE)


#
#include $(call all-makefiles-under,$(LOCAL_PATH))
