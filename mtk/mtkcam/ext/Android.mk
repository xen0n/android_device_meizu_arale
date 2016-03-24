LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    MtkCamera.cpp \
    MtkCameraParameters.cpp \
#    MtkCameraProfile.cpp \
#    MtkCameraMMP.cpp \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libbinder \
    libhardware \
    libui \
    libgui \
    libcamera_metadata \
    libcamera_client \

LOCAL_C_INCLUDES += \
    system/media/camera/include \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
#ifeq ($(strip $(MTK_MMPROFILE_SUPPORT)),yes)

#    LOCAL_SHARED_LIBRARIES += libmmprofile
#    LOCAL_CFLAGS += -DMTK_CAMERAMMP_SUPPORT

#    LOCAL_C_INCLUDES += $(TOP)/bionic/libc/kernel/common

#endif
#endif

#//!----------------------------------------------------------------------------

LOCAL_MODULE:= libmtkcamera_client

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
