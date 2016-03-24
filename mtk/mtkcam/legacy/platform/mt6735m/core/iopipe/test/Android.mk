#
# camiopipetest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#-----------------------------------------------------------
LOCAL_SRC_FILES := \
    test_normalstream.cpp \
    ts_iopipe.cpp \
    ts_iopipe_sensor.cpp \
    main.cpp


#utility
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/

#linux kernel
LOCAL_C_INCLUDES += $(TOP)/kernel-3.10/arch/arc

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc

#for camera_vendor_tags.h:
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libcam_utils \
    libcam.iopipe \
    libcamdrv \
    libcam.halsensor
#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := camiopipetest

#
LOCAL_MODULE_TAGS := eng

#
LOCAL_PRELINK_MODULE := false

#

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

MTKCAM_CFLAGS += -DL1_CACHE_BYTES=32
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

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/

# End of common part ---------------------------------------
#
#include $(BUILD_EXECUTABLE)


#
#include $(call all-makefiles-under,$(LOCAL_PATH))
