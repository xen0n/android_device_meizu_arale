#
# libacdk
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    acdk/AcdkBase.cpp \
    acdk/AcdkIF.cpp \
    acdk/AcdkMain.cpp \
    acdk/AcdkMhalBase.cpp \
    acdk/AcdkMhalEng.cpp \
    acdk/AcdkMhalPure.cpp \
    acdk/AcdkUtility.cpp \
    surfaceview/AcdkSurfaceView.cpp \
    surfaceview/surfaceView.cpp \
    main.cpp \
    test_preview.cpp
#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/hal/sensor/test/inc/acdk \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio_common/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/hal/adapter/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/featureio/pipe/aaa/isp_tuning \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/featureio/pipe/aaa/ae_mgr \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/featureio/pipe/aaa \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    #    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/cam \
    $(TOP)/$(MTK_PATH_CUSTOM)/custom/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/imgsensor \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam \
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libcct \

ifeq ($(BUILD_MTK_LDVT),true)
   LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif
#

#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libimageio \
    libcamdrv \
    libm4u \
    libcam.exif \
    libcamalgo \
    libcam.halsensor


ifneq ($(BUILD_MTK_LDVT),true)
LOCAL_SHARED_LIBRARIES += libfeatureio
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.camshot
endif
LOCAL_SHARED_LIBRARIES += liblog
#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE := test_preview

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

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include

# End of common part ---------------------------------------
#
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

#
include $(call all-makefiles-under, $(LOCAL_PATH))
