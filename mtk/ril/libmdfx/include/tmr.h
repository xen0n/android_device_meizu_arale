/*
    Here is an interface of the timer to hide the platform-dependent libraries. 
    The tmr_init function initializes the timer with a spcified signalling method. 
    The tmr_exit function uninitializes the timer. 
    The tmr_set_time function one-shot arms or disarms the timer with a used-defined callback function. 
    The tmr_get_time function (NEED_TO_BE_NOTICED, NOT yet to implement) the remaining time to the next expiration. 
    Please notice that the arg is a local variable which would be popped after calling the used-defined callback function(). 
    If necessary, you should copy the arg in the handler. 
*/

#ifndef __TMR_H__
#define __TMR_H__

#include "sys_info.h"
#include "compiler.h"
#include "thrd/thrd_lock.h"
#include <time.h>
#ifdef __LINUX_PLAT__
#include <signal.h>
#include <sys/types.h>
#include <stddef.h>
#include <pthread.h>
#endif

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// => POSIX timer
// N/A

// Type definitions
typedef struct tmr tmr_t;
typedef struct tmr* tmr_ptr_t;
typedef struct tmr** tmr_ptr2_t;
typedef struct tmr_init_arg tmr_init_arg_t;
typedef struct tmr_init_arg* tmr_init_arg_ptr_t;
typedef struct tmr_time tmr_time_t;
typedef struct tmr_time* tmr_time_ptr_t;
typedef enum tmr_sig {tmr_sig_none, tmr_sig_block, tmr_sig_nonblock, tmr_sig_agent} tmr_sig_t;
typedef void (*tmr_sig_hdl_t) (tmr_ptr_t tmr_ptr, void *arg);    // SHOULD be an async-signal-safe function for POSIX

// Macros
#ifdef __LINUX_PLAT__
// => POSIX timer
#define TMR_INVAL_TID    (0)
#define TMR_SIG_BLOCK    ((SIGRTMIN + SIGRTMAX) >> 1)
#define TMR_SIG_NONBLOCK    ((SIGRTMIN + SIGRTMAX) >> 2)
#endif

// Functions
#define tmr_init(tmr_ptr, arg_ptr)    (unlikely((tmr_ptr) == NULL || (tmr_ptr)->init_fp == NULL) ? NULL : (tmr_ptr)->init_fp((tmr_ptr), (arg_ptr)))
#define tmr_exit(tmr_ptr)    (unlikely((tmr_ptr) == NULL || (tmr_ptr)->exit_fp == NULL) ? NULL : (tmr_ptr)->exit_fp((tmr_ptr)))
#define tmr_set_time(tmr_ptr, time, hdl, arg)    (unlikely((tmr_ptr) == NULL || (tmr_ptr)->set_time_fp == NULL) ? SYS_FAIL : (tmr_ptr)->set_time_fp((tmr_ptr), (time), (hdl), (arg)))
#define tmr_get_time(tmr_ptr)    (unlikely((tmr_ptr) == NULL || (tmr_ptr)->get_time_fp == NULL) ? 0 : (tmr_ptr)->get_time_fp((tmr_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the tmr_ptr were given as a value, a compile-time error would be given
// => Default timer: POSIX timer
#define default_tmr_employ(tmr_ptr)    posix_tmr_employ(tmr_ptr)
#define default_tmr_dismiss(tmr_ptr)    posix_tmr_dismiss(tmr_ptr)
#ifdef __LINUX_PLAT__
// => POSIX timer
#define posix_tmr_employ(tmr_ptr)    (unlikely((tmr_ptr) == NULL) ? NULL : \
                                     (((tmr_ptr)->init_fp = posix_tmr_init), \
                                     ((tmr_ptr)->exit_fp = posix_tmr_exit), \
                                     ((tmr_ptr)->set_time_fp = posix_tmr_set_time), \
                                     ((tmr_ptr)->get_time_fp = posix_tmr_get_time), \
                                     (tmr_ptr)))
#define posix_tmr_dismiss(tmr_ptr)    (unlikely((tmr_ptr) == NULL) ? NULL : \
                                      (((tmr_ptr)->init_fp = NULL), \
                                      ((tmr_ptr)->exit_fp = NULL), \
                                      ((tmr_ptr)->set_time_fp = NULL), \
                                      ((tmr_ptr)->get_time_fp = NULL), \
                                      (tmr_ptr)))
#endif

// Implementation
// => initialization the argument for timer
struct tmr_init_arg
{
    // General variables
    tmr_sig_t sig;
    // Individual variables which must be private
    union
    {
#ifdef __LINUX_PLAT__
        // POSIX timer
        struct
        {
            pid_t tid;
        }posix;
#endif
    }idv;
};

// => tmr time
struct tmr_time
{
    // General variables
    size_t sec;
    size_t nsec;    // for 32-bit variables, a scalable unit is come out between every 1 * 10^9
    // Individual variables which must be private
    union
    {
        // POSIX timer
        struct
        {
            // N/A
        }posix;
    }idv;
};

// => Abstract data type
struct tmr
{
    // General variables
    // => Public
    tmr_ptr_t (*init_fp) (tmr_ptr_t tmr_ptr, const tmr_init_arg_ptr_t arg_ptr);
    tmr_ptr_t (*exit_fp) (tmr_ptr_t tmr_ptr);
    int (*set_time_fp) (tmr_ptr_t tmr_ptr, const tmr_time_ptr_t time_ptr, tmr_sig_hdl_t hdl, void *arg);
    time_t (*get_time_fp) (tmr_ptr_t tmr_ptr);
    // => Private
    tmr_sig_t sig;
    tmr_sig_hdl_t hdl;    // SHOULD be an async-signal-safe function for POSIX
    void *arg;
    // Individual variables which must be private
    union
    {
#ifdef __LINUX_PLAT__
        // POSIX timer
        struct
        {
            thrd_lock_t mutex;

            // for tmr_sig_block
            pthread_t thrd;

            // for tmr_sig_nonblock
            timer_t id;    // also for tmr_sig_agent
            size_t num_of_req;
            size_t count;
            pid_t tid;
        }posix;
#endif
    }idv;
};

#ifdef __LINUX_PLAT__
// => POSIX timer
extern tmr_ptr_t posix_tmr_init (tmr_ptr_t tmr_ptr, const tmr_init_arg_ptr_t arg_ptr);
extern tmr_ptr_t posix_tmr_exit (tmr_ptr_t tmr_ptr);
extern int posix_tmr_set_time (tmr_ptr_t tmr_ptr, const tmr_time_ptr_t time_ptr, tmr_sig_hdl_t hdl, void *arg);
extern time_t posix_tmr_get_time (tmr_ptr_t tmr_ptr);
#endif

#endif
