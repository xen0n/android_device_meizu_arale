################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += imem_drv.cpp
LOCAL_SRC_FILES += hwutils.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/
#
LOCAL_C_INCLUDES += $(TOP)/$(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(TOPDIR)/system/core/include \
                    $(TOP)/system/core/libion/include \
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_LDVT)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/external/ldvt/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM) \
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(PLATFORM)/D1


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

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += 
#
#ifeq ($(BUILD_MTK_LDVT),true)
#    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
#endif

#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcamdrv_imem

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

