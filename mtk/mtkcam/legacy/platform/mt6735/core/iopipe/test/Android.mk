#
# iotest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#
LOCAL_SRC_FILES := \
    test_ef_bond.cpp \
    test_normalstream.cpp \
    main.cpp \
    main_iopipe.cpp \
    main_camio.cpp \
    main_IT.cpp
#
# camera_vendor_tags.h
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# camera Hardware 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc
# temp for using iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach/

# vector
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \

# iopipe
LOCAL_SHARED_LIBRARIES += \
    libcam.iopipe \
    libimageio \
    libimageio_plat_drv \

# Imem
LOCAL_SHARED_LIBRARIES += \
    libcamdrv \

LOCAL_SHARED_LIBRARIES += \
	libcam.halsensor

LOCAL_SHARED_LIBRARIES += \
	libcam_utils

LOCAL_SHARED_LIBRARIES += libfeatureiodrv


#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := iopipetest

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


# End of common part ---------------------------------------
#
include $(BUILD_EXECUTABLE)


#
#include $(call all-makefiles-under,$(LOCAL_PATH))
