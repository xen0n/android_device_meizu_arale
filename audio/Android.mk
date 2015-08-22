LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= audio.cpp

LOCAL_MODULE := audio.primary.mt6595
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_C_INCLUDES += frameworks/av/media/mtp/ system/core/include/ frameworks/rs/server/ frameworks/av/include/ hardware/libhardware/include/
#LOCAL_LDFLAGS += -L$(LOCAL_PATH)/../../../../vendor/mediatek/mt6592/lib/ -laudio.primary.default
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libutils libbinder

include $(BUILD_SHARED_LIBRARY)

LIBORIG := $(LOCAL_INSTALLED_MODULE)
LIBLINK1 := $(subst audio.primary.mt6595,audio.primary.default,$(LIBORIG))
$(LIBLINK1): $(LIBORIG)
	@echo "Symlink: $@ -> $(notdir $<)"
	@mkdir -p $(dir $@)
	$(hide) ln -sf $(notdir $<) $@
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
	$(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(LIBLINK1)
LIBORIG :=
LIBLINK1 :=

