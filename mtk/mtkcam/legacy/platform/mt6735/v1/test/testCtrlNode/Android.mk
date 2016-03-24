
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/v1/adapter.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, .)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include
#
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc
#

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.camnode
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.callback

#
LOCAL_STATIC_LIBRARIES += 

#-----------------------------------------------------------
# vector
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils \

# node 
LOCAL_SHARED_LIBRARIES += \
    libcam.camnode \

LOCAL_SHARED_LIBRARIES += \
	libcam.halsensor \
	libcam.utils \
	libcam_utils \
	libfeatureio \
	libcamera_client libmtkcamera_client \
	libcam.paramsmgr \
	libcam.camshot \
    libcam_hwutils \

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := testCtrlNode

#-----------------------------------------------------------
include $(BUILD_EXECUTABLE)

