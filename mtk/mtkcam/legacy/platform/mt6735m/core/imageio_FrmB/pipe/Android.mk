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
LOCAL_SRC_FILES += PipeImp_FrmB.cpp
LOCAL_SRC_FILES += \
    CamIOPipe/CamIOPipe_FrmB.cpp \
    CamIOPipe/ICamIOPipeBridge_FrmB.cpp \
    PostProcPipe/IPostProcPipeBridge_FrmB.cpp \
    PostProcPipe/PostProcPipe_FrmB.cpp

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
#-----------------------------------------------------------
#LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

#LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
#
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include
#
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
#
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)
# systrace
#LOCAL_C_INCLUDES += $(TOP)/system/core/include
#LOCAL_SHARED_LIBRARIES += libutils

#-----------------------------------------------------------


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(call include-path-for, camera)
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/drv_common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include
#
LOCAL_C_INCLUDES += $(TOP)/bionic/libc/kernel/common/linux/mt6582
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_FrmB/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_FrmB/pipe/inc

#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#
#-----------------------------------------------------------




LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libimageio_plat_pipe_FrmB
#LOCAL_MULTILIB := 32
#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

