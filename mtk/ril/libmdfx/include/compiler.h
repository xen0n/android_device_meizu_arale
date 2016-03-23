/*
    Here is the information of the compiler to hide the platform-dependent libraries. 
*/

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "sys_info.h"

// API
#ifdef __LINUX_PLAT__
#define likely(cond)    (__builtin_expect(!!(cond), 1))
#define unlikely(cond)    (__builtin_expect(!!(cond), 0))
#endif

#endif
