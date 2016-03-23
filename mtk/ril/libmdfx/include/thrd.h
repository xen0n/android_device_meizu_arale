/*
    Here is an interface of the thread to hide the platform-dependent libraries. 
    The thrd_init function initializes the thread. 
    The thrd_exit function uninitializes the thread. 
    The thrd_exec function assigns a task to a initialized thread. 
    Please notice that the arg is a local variable which would be popped after calling the thrd_exec(). 
    If necessary, you should copy the arg in the handler. 
*/

#ifndef __THRD_H__
#define __THRD_H__

#include "sys_info.h"
#include "compiler.h"
#include <errno.h>
#ifdef __LINUX_PLAT__
#include <pthread.h>
#include <sys/types.h>
#endif

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// => POSIX thread
// N/A

// Type definitions
typedef struct thrd thrd_t;
typedef struct thrd* thrd_ptr_t;
typedef struct thrd_init_arg thrd_init_arg_t;
typedef struct thrd_init_arg* thrd_init_arg_ptr_t;
typedef enum thrd_status {thrd_status_fail, thrd_status_busy, thrd_status_idle} thrd_status_t;
typedef void* (*thrd_hdl_t) (void *arg);

// Macros
#define THRD_BUSY    (EBUSY)
#ifdef __LINUX_PLAT__
// => POSIX thread
//#define __POSIX_THRD_CANCEL_ENABLE__
#endif

// Functions
#define thrd_init(thrd_ptr, arg_ptr)    (unlikely((thrd_ptr) == NULL || (thrd_ptr)->init_fp == NULL) ? NULL : (thrd_ptr)->init_fp((thrd_ptr), (arg_ptr)))
#define thrd_exit(thrd_ptr)    (unlikely((thrd_ptr) == NULL || (thrd_ptr)->exit_fp == NULL) ? NULL : (thrd_ptr)->exit_fp((thrd_ptr)))
#define thrd_exec(thrd_ptr, hdl, arg)    (unlikely((thrd_ptr) == NULL || (thrd_ptr)->exec_fp == NULL) ? SYS_FAIL : (thrd_ptr)->exec_fp((thrd_ptr), (hdl), (arg)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the thrd_ptr were given as a value, a compile-time error would be given
// => Default thread: POSIX thread
#define default_thrd_employ(thrd_ptr)    posix_thrd_employ(thrd_ptr)
#define default_thrd_dismiss(thrd_ptr)    posix_thrd_dismiss(thrd_ptr)
#ifdef __LINUX_PLAT__
// => POSIX thread
#define posix_thrd_employ(thrd_ptr)    (unlikely((thrd_ptr) == NULL) ? NULL : \
                                       (((thrd_ptr)->init_fp = posix_thrd_init), \
                                       ((thrd_ptr)->exit_fp = posix_thrd_exit), \
                                       ((thrd_ptr)->exec_fp = posix_thrd_exec), \
                                       (thrd_ptr)))
#define posix_thrd_dismiss(thrd_ptr)    (unlikely((thrd_ptr) == NULL) ? NULL : \
                                        (((thrd_ptr)->init_fp = NULL), \
                                        ((thrd_ptr)->exit_fp = NULL), \
                                        ((thrd_ptr)->exec_fp = NULL), \
                                        (thrd_ptr)))
#endif

// Implementation
// => initialization the argument for thread
struct thrd_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
#ifdef __LINUX_PLAT__
        // POSIX thread
        struct
        {
            // N/A
        }posix;
#endif
    }idv;
};

// => Abstract data type
struct thrd
{
    // General variables
    // => Public
    thrd_ptr_t (*init_fp) (thrd_ptr_t thrd_ptr, const thrd_init_arg_ptr_t arg_ptr);
    thrd_ptr_t (*exit_fp) (thrd_ptr_t thrd_ptr);
    int (*exec_fp) (thrd_ptr_t thrd_ptr, thrd_hdl_t hdl, void *arg);
    // => Private
    thrd_status_t status;
    thrd_hdl_t hdl;
    void *arg;
    void *ret;
    // Individual variables which must be private
    union
    {
#ifdef __LINUX_PLAT__
        // POSIX thread
        struct
        {
            pthread_t thrd;
            pthread_mutex_t exec_mutex;
            pthread_cond_t exec_cond_var;
            pid_t tid;
        }posix;
#endif
    }idv;
};

#ifdef __LINUX_PLAT__
// => POSIX thread
extern thrd_ptr_t posix_thrd_init (thrd_ptr_t thrd_ptr, const thrd_init_arg_ptr_t arg_ptr);
extern thrd_ptr_t posix_thrd_exit (thrd_ptr_t thrd_ptr);
extern int posix_thrd_exec (thrd_ptr_t thrd_ptr, thrd_hdl_t hdl, void *arg);
#endif

#endif
