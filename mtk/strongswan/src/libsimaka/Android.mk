LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
libsimaka_la_SOURCES = simaka_message.h simaka_message.c \
  simaka_crypto.h simaka_crypto.c simaka_manager.h simaka_manager.c \
  simaka_card.h simaka_provider.h simaka_hooks.h

LOCAL_SRC_FILES := $(filter %.c,$(libsimaka_la_SOURCES))

# build libsimaka ---------------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(strongswan_PATH)/src/include \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS)

LOCAL_MODULE := libsimaka

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libstrongswan

include $(BUILD_SHARED_LIBRARY)

