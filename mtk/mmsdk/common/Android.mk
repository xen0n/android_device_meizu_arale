LOCAL_PATH:= $(call my-dir)
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

ALLOWED_PLATFORM := mt6795
ALLOWED_PLATFORM += mt6752
ALLOWED_PLATFORM += mt6735
ALLOWED_PLATFORM += mt6755
ALLOWED_PLATFORM += mt6797

ifneq ($(BUILD_MTK_LDVT),yes)

ifeq ($(PLATFORM),$(filter $(PLATFORM),$(ALLOWED_PLATFORM)))
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

endif


