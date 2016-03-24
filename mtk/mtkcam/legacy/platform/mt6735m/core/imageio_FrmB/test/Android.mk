#
# imageioFrmBtest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#temp mark to avoid build error
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),no_build)
#
LOCAL_SRC_FILES := \
    ts_camio_FrmB.cpp \
    ts_camio_sensor_FrmB.cpp \
    main.cpp
#
#
# camera Hardware
LOCAL_C_INCLUDES +=  \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \

#
# Sensor common include header
LOCAL_C_INCLUDES += $(TOP)/kernel-3.4/drivers/misc/mediatek/imgsensor/inc

LOCAL_C_INCLUDES +=  \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/imageio \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_FrmB/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/campipe/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/imageio_FrmB \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_FrmB/pipe/inc \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc \

#for camera_vendor_tags.h:
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libcamdrv \
    libcamdrv_FrmB \
    libimageio_FrmB \
    libimageio_plat_drv_FrmB \
    libcam.campipe \
#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := imageioFrmBtest

#
LOCAL_MODULE_TAGS := eng

#
LOCAL_PRELINK_MODULE := false

#
#LOCAL_MULTILIB := 32
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
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
# End of common part ---------------------------------------
#
include $(BUILD_EXECUTABLE)


#
#include $(call all-makefiles-under,$(LOCAL_PATH))
endif
