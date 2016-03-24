#
# libacdk
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------

LOCAL_CFLAGS += -DACDK_SUPPORT_ENGMODE=1

LOCAL_SRC_FILES := \
    acdk/AcdkBase.cpp \
    acdk/AcdkIF.cpp \
    acdk/AcdkMain.cpp \
    acdk/AcdkMhalBase.cpp \
    acdk/AcdkMhalEng.cpp \
    acdk/AcdkMhalPure.cpp \
    acdk/AcdkUtility.cpp \
    surfaceview/AcdkSurfaceView.cpp \
    surfaceview/surfaceView.cpp

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/acdk \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM) \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/acdk/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/imgsensor \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/res_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/pipe/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/isp_tuning \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa/ae_mgr \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/aaa \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/featureio/pipe/utility \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/common/camexif \
    $(TOP)/mediatek/hardware \
    $(TOP)/mediatek/hardware/include \
    $(TOP)/$(MTKCAM_C_INCLUDES) \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include \
    $(TOP)/$(MTK_PATH_PLATFORM)/custom/hal/inc \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/cgen/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/cgen/cfgfileinc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/imgsensor \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/imgsensor \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/debug_exif/cam \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av
#
LOCAL_WHOLE_STATIC_LIBRARIES := libcct

#
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libimageio \
    libcamdrv \
    libm4u \
    libcam.halsensor \
    #libcam.exif \
    #ACDK_PURE_SMT libcamalgo \

# MM DVFS
LOCAL_SHARED_LIBRARIES += libbwc

LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam_utils
LOCAL_SHARED_LIBRARIES += libcam.metadata

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libdl

LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libmtkcamera_client

ifneq ($(BUILD_MTK_LDVT),true)
LOCAL_SHARED_LIBRARIES += libcamalgo
LOCAL_SHARED_LIBRARIES += libcam.exif
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.camshot
endif

LOCAL_SHARED_LIBRARIES += liblog
#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE := libacdk
#

#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

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
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# MM DVFS
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/bwc/inc
# End of common part ---------------------------------------
#
include $(BUILD_SHARED_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH))
