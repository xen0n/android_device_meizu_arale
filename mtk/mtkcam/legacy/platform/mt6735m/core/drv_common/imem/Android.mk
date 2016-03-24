#
# libcamdrv_imem
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
#
LOCAL_SRC_FILES := \
    imem_drv.cpp
LOCAL_SRC_FILES += \
    hwutils.cpp \

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_MTKCAM_PLATFORM) \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv_common/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/bionic/libc/kernel/common \
    $(TOP)/$(MTK_PATH_LDVT)/include \
    $(TOPDIR)/system/core/include \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/system/core/libion/include \
    $(TOPDIR)/system/core/include \
    $(TOP)/$(MTK_PATH_SOURCE)/kernel/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)/D2 \
    $(TOP)/$(MTK_PATH_SOURCE)/external \


# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
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

ifeq ($(MTK_ION_SUPPORT), yes)
  LOCAL_CFLAGS += -DUSING_MTK_ION
endif
#-----------------------------------------------------------

#
LOCAL_STATIC_LIBRARIES := \

LOCAL_SHARED_LIBRARIES := libion

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#ifeq ($(BUILD_MTK_LDVT),true)
#    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
#endif

#
LOCAL_MODULE := libcamdrv_imem

#
#LOCAL_MULTILIB := 32
#
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
#include $(call all-makefiles-under, $(LOCAL_PATH))
