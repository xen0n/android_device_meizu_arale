# NOTE: This is based on CM's QCOM power HAL, although basically only the
# skeleton remains. ;-)

LOCAL_PATH := $(call my-dir)

# Hey Mr. Make Author, DIAF PLX
ifeq ($(TARGET_POWERHAL_VARIANT),mtk-xen0n)
USE_ME := true
endif

ifneq (,$(filter true,$(USE_ME)))

# HAL module implemenation stored in
# hw/<POWERS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)

LOCAL_MODULE_RELATIVE_PATH := hw
# LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl
LOCAL_SRC_FILES := power.c metadata-parser.c hint-data.c

ifneq ($(BOARD_POWER_CUSTOM_BOARD_LIB),)
  LOCAL_WHOLE_STATIC_LIBRARIES += $(BOARD_POWER_CUSTOM_BOARD_LIB)
else

# Include target-specific files.
#ifeq ($(call is-board-platform-in-list, msm8974), true)
#LOCAL_SRC_FILES += power-8974.c
#endif

endif  #  End of board specific list

ifneq ($(TARGET_POWERHAL_SET_INTERACTIVE_EXT),)
LOCAL_CFLAGS += -DSET_INTERACTIVE_EXT
LOCAL_SRC_FILES += ../../../../$(TARGET_POWERHAL_SET_INTERACTIVE_EXT)
endif

ifneq ($(TARGET_TAP_TO_WAKE_NODE),)
  LOCAL_CFLAGS += -DTAP_TO_WAKE_NODE=\"$(TARGET_TAP_TO_WAKE_NODE)\"
endif

ifeq ($(TARGET_POWER_SET_FEATURE_LIB),)
  LOCAL_SRC_FILES += power-feature-default.c
else
  LOCAL_STATIC_LIBRARIES += $(TARGET_POWER_SET_FEATURE_LIB)
endif

ifneq ($(CM_POWERHAL_EXTENSION),)
LOCAL_MODULE := power.$(CM_POWERHAL_EXTENSION)
else
LOCAL_MODULE := power.$(TARGET_BOARD_PLATFORM)
endif
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

endif # TARGET_POWERHAL_VARIANT == mtk-xen0n
