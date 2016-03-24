
LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#After Jonas' review in KK
#-----------------------------------------------------------
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, .)

#-----------------------------------------------------------
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/lomo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/lomo_jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo_jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/featureio
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/imageio/inc
# for log.h
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)

D_1 = mt6735 mt6753
D_2 = mt6735m
D_12 = mt6735 mt6753 mt6735m
# temp for using iopipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/iopipe/PostProc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/lomo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/lomo_jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/featureio
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include//mtkcam/iopipe/PostProc

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils

#-----------------------------------------------------------
# iopipe 
LOCAL_SHARED_LIBRARIES += \
	libcam.iopipe

#-----------------------------------------------------------

# camUtils 
LOCAL_SHARED_LIBRARIES +=\
    libcam_utils \

# for feature stream
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libdpframework

#LOCAL_STATIC_LIBRARIES := \


     
#

#-----------------------------------------------------------
LOCAL_MODULE := libcam.jni.lomohaljni

LOCAL_MODULE_TAGS := optional

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

