#
# libcamdrv_isp
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := isp_drv_FrmB.cpp

# camutils: For CameraProfile APIs.
#
LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/ \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv_common/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/ \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTKCAM_C_INCLUDES)/.. \
    $(MTKCAM_C_INCLUDES)
	
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------

LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_SHARED_LIBRARIES := \
    libcutils \

#
LOCAL_MODULE := libcamdrv_isp_FrmB

#
LOCAL_PRELINK_MODULE := false

#
LOCAL_MODULE_TAGS := optional

#
#LOCAL_MULTILIB := 32
#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

# End of common part ---------------------------------------
#
include $(BUILD_STATIC_LIBRARY)


