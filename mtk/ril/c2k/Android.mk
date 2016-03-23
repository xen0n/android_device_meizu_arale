include $(all-subdir-makefiles)

ifeq ($(C2K_GEMINI),yes)
  LOCAL_CFLAGS := -DANDROID_MULTI_SIM
endif

ifeq ($(C2K_MODEM_SUPPORT), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(C2K_MODEM_SUPPORT), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(C2K_MODEM_SUPPORT), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif
