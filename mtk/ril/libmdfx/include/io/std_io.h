/*
    Here is an interface of the standard I/O to hide the platform-dependent libraries. 
*/

#ifndef __IO_STD_IO_H__
#define __IO_STD_IO_H__

#include "sys_info.h"
#include <stdio.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

// Additional information
#ifdef __ANDROID__
#define DEFAULT_TAG_NAME    "MAL-MAIN"
extern int android_log_prio[];
#endif

// API
#ifdef __ANDROID__
#ifdef __GCC_COMPILER__
#define lv_print(lv, format, args ...)    (((lv) & PRINT_LV) == 0 ? SYS_SUCC : \
                                          ((format) == NULL ? SYS_FAIL : \
                                          (__android_log_print(android_log_prio[(lv)], DEFAULT_TAG_NAME, format "%s%s%s%d%s", ## args, "[", __func__, ", ", __LINE__, "]: ") < 0 ? \
                                          SYS_FAIL : (fflush(stdout), SYS_SUCC))))
#else
#define lv_print(lv, format, args ...)    (((lv) & PRINT_LV) == 0 ? SYS_SUCC : \
                                          ((format) == NULL ? SYS_FAIL : \
                                          (__android_log_print(android_log_prio[(lv)], DEFAULT_TAG_NAME, (format),  ## args) < 0 ? \
                                          SYS_FAIL : (fflush(stdout), SYS_SUCC))))
#endif
#else
#ifdef __GCC_COMPILER__
#define lv_print(lv, format, args ...)    (((lv) & PRINT_LV) == 0 ? SYS_SUCC : \
                                          ((format) == NULL ? SYS_FAIL : \
                                          (fprintf(stdout, "%s%s%s%d%s",  "[", __func__, ", ", __LINE__, "]: "), \
                                          (fprintf(stdout, (format),  ## args) < 0 ? SYS_FAIL : (fflush(stdout), SYS_SUCC)))))
#else
#define lv_print(lv, format, args ...)    (((lv) & PRINT_LV) == 0 ? SYS_SUCC : \
                                          ((format) == NULL ? SYS_FAIL : \
                                          (fprintf(stdout, (format),  ## args) < 0 ? SYS_FAIL : (fflush(stdout), SYS_SUCC))))
#endif
#endif

#endif
