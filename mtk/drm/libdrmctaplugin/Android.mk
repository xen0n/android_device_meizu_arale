LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE =
LOCAL_MODULE = libdrmctaplugin
LOCAL_MODULE_CLASS = SHARED_LIBRARIES
LOCAL_MODULE_PATH =
LOCAL_MODULE_RELATIVE_PATH = drm
LOCAL_MODULE_SUFFIX = .so
LOCAL_SHARED_LIBRARIES = libicui18n libicuuc libandroid_runtime libdrmframework libdrmmtkutil libmtk_drvb libbinder libc++
LOCAL_MULTILIB = 32
LOCAL_SRC_FILES_32 = libdrmctaplugin.so
include $(BUILD_PREBUILT)
