#
# libimageio_plat_pipe_mgr
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    pipe_mgr_drv.cpp \

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/campipe \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/src/imageio/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \

#
LOCAL_MODULE := libcampipe_plat_pipe_mgr

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

# End of common part ---------------------------------------
#
include $(BUILD_STATIC_LIBRARY)



#
