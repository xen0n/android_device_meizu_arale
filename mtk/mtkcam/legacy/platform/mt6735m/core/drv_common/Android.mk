#
# libcamdrv
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.

#
include $(call all-makefiles-under, $(LOCAL_PATH))
