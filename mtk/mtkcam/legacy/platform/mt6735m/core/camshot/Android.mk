################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    CamShotImp.cpp \
    SingleShot/ISingleShotBridge.cpp \
    SingleShot/SingleShot.cpp \
    BurstShot/BurstShot.cpp \
    BurstShot/IBurstShotBridge.cpp

ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_SRC_FILES += \
    MultiShot/IMultiShotBridge.cpp \
    MultiShot/ImageCreateThread.cpp \
    MultiShot/MultiShot.cpp \
    MultiShot/MultiShotCc.cpp \
    MultiShot/MultiShotNcc.cpp
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

#
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
#
# camera Hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/v1/hal/adapter/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/mtkjpeg/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/mtkjpeg/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

# m4u
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camshot.simager
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camshot.utils
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libcam.campipe
LOCAL_SHARED_LIBRARIES += libcamdrv
#
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libmtkjpeg
#
# for jpeg enc use
LOCAL_SHARED_LIBRARIES += libm4u libJpgEncPipe
#
# for 3A
LOCAL_SHARED_LIBRARIES += libfeatureio
#
# camUtils
LOCAL_SHARED_LIBRARIES += libcam_mmp
LOCAL_SHARED_LIBRARIES += libcam.utils
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.camshot

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

