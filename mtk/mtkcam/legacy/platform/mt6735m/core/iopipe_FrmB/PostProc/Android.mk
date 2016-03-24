#
# libcam.iopipe.postproc_FrmB
#

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    FeatureStream_FrmB.cpp \
    HalPipeWrapper.Thread_FrmB.cpp \
    HalPipeWrapper_FrmB.cpp \
    NormalStream_FrmB.cpp

#-----------------------------------------------------------
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

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
#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#
#-----------------------------------------------------------
LOCAL_CFLAGS +=

ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#HALPOSTPROC_USING_TEST := '1'
#LOCAL_CFLAGS += -DHALPOSTPROC_USING_TEST
#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)

#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES += libcam.temp.camera1.hwscenario
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcam.iopipe.postproc_FrmB
#LOCAL_MULTILIB := 32
#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

