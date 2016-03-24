#
# libimageio_plat_drv
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    cam/cam_path.cpp \
    cam/cam_path_pass1.cpp \
    cam/cam_path_pass2.cpp \
    cam/isp_function.cpp \
    cdp/cdp_drv.cpp \
    mdp/mdp_mgr.cpp

# camutils: For CameraProfile APIs.
#
LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/ \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio_common/inc \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common/camutils \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/drv \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/imageio \

LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include/mtkcam/drv
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \


LOCAL_SHARED_LIBRARIES := \
    libdpframework \
    libcutils \
    libcamdrv \
    libm4u \

#new from Jonas requests 130510
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_CFLAGS += -DUSING_D2

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#
LOCAL_MODULE := libimageio_plat_drv
LOCAL_MODULE_TAGS := optional
#LOCAL_MULTILIB := 32
#
#include $(BUILD_STATIC_LIBRARY)

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
include $(BUILD_SHARED_LIBRARY)


#
#include $(call all-makefiles-under, $(LOCAL_PATH))
