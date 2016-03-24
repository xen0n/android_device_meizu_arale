#
# libcamdrv
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += 

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_C_INCLUDES)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
#

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif
ifeq ($(HAVE_AEE_FEATURE),yes)
	LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_isp
#LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_tpipe
LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_twin
#LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_imgsensor
LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_res_mgr
LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_resmanager
LOCAL_WHOLE_STATIC_LIBRARIES += libcamdrv_imem
#LOCAL_WHOLE_STATIC_LIBRARIES += libtpipe

ifeq ($(BUILD_MTK_LDVT),true)
LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#
#LOCAL_STATIC_LIBRARIES += libcamdrv_imgsensor libcamdrv_res_mgr

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libm4u
LOCAL_SHARED_LIBRARIES += libbwc
LOCAL_SHARED_LIBRARIES += libdl
#
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += libcam_mmp
endif
#
ifneq ($(BUILD_MTK_LDVT),true)
    LOCAL_SHARED_LIBRARIES += libcameracustom 
    LOCAL_SHARED_LIBRARIES += libcam.exif
    LOCAL_SHARED_LIBRARIES += libmatv_cust

    ifeq ($(HAVE_AEE_FEATURE),yes)
	LOCAL_SHARED_LIBRARIES += libaed
    endif

endif

ifeq ($(MTK_ION_SUPPORT),yes)
	LOCAL_SHARED_LIBRARIES += libion libion_mtk
endif

#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcamdrv
#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY) 
 
#include $(BUILD_STATIC_LIBRARY) 


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
