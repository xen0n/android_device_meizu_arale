#
# libimageio_plat_pipe
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    PipeImp.cpp \
    CamIOPipe/CamIOPipe.cpp \
    CamIOPipe/ICamIOPipeBridge.cpp \
    CdpPipe/CdpPipe.cpp \
    CdpPipe/ICdpPipeBridge.cpp \
    PostProcPipe/IPostProcPipeBridge.cpp \
    PostProcPipe/PostProcPipe.cpp

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/imageio \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/pipe/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/inc/drv \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc


ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
#
LOCAL_SHARED_LIBRARIES := \

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := libimageio_plat_pipe

#
#LOCAL_MULTILIB := 32
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

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
# End of common part ---------------------------------------
#
include $(BUILD_STATIC_LIBRARY)



#
#include $(call all-makefiles-under,$(LOCAL_PATH))
