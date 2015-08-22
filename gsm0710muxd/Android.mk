# Copyright 2008 Texas Instruments
#
#Author(s) Mikkel Christensen (mlc@ti.com) and Ulrik Bech Hald (ubh@ti.com)

#==========================
# First SIM RIL

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= gsm0710muxd

LOCAL_SRC_FILES:= \
	src/gsm0710muxd.c \
	src/gsm0710muxd_fc.c \

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \
	-DMUX_ANDROID \
	-D__CCMNI_SUPPORT__ \
	-D__MUXD_FLOWCONTROL__ \


LOCAL_CFLAGS += -DMTK_GEMINI

ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

ifneq ($(MTK_INTERNAL),yes)
	LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

LOCAL_CFLAGS += -DMTK_RIL_MD1

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
#LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)

#===========================
# Second SIM RIL

include $(CLEAR_VARS)

LOCAL_MODULE:= gsm0710muxdmd2

LOCAL_SRC_FILES:= \
	src/gsm0710muxd.c \
	src/gsm0710muxd_fc.c \

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \
	-DMUX_ANDROID \
	-D__CCMNI_SUPPORT__ \
	-D__MUXD_FLOWCONTROL__ \

LOCAL_CFLAGS += -DMTK_GEMINI

ifeq ($(MTK_DT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DT_SUPPORT
endif

LOCAL_CFLAGS += -DMTK_RIL_MD2

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
#LOCAL_LDLIBS := -lpthread

include $(BUILD_EXECUTABLE)