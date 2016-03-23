/*
    Here is an interface of channel to hide platform-dependent libraries. 
    A thread channel deals with the communication between two entities running on different threads of the same process. 
    A Unix Domain Socket (UDS) channel deals with the communication between two entities running on the same machine through the socket interface. 
*/

#ifndef __IO_CHNL_H__
#define __IO_CHNL_H__

#include "sys_info.h"
#include "compiler.h"
#include "ctnr/queue.h"
#include "thrd/thrd_lock.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __LINUX_PLAT__
#include <sys/un.h>
#endif

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// => Thread channel
// N/A
#ifdef __LINUX_PLAT__
// => UDS channel
// N/A
#endif

// Type definitions
typedef struct chnl chnl_t;
typedef struct chnl* chnl_ptr_t;
typedef struct chnl** chnl_ptr2_t;
typedef struct chnl_init_arg chnl_init_arg_t;
typedef struct chnl_init_arg* chnl_init_arg_ptr_t;
typedef struct chnl_addr chnl_addr_t;
typedef struct chnl_addr* chnl_addr_ptr_t;
typedef struct chnl_addr** chnl_addr_ptr2_t;
// => Thread channel
// N/A
#ifdef __LINUX_PLAT__
// => Unix domian socket channel
typedef enum uds_chnl_mode {uds_chnl_mode_fail, uds_chnl_mode_cli, uds_chnl_mode_srv, uds_chnl_mode_reuse} uds_chnl_mode_t;
#endif

// Macros
// => Thread channel
// N/A
#ifdef __LINUX_PLAT__
// => Unix domian socket channel
#define UDS_CHNL_LISTEN_BACKLOG    (8)
#define UDS_CHNL_PATH_LEN    (32)
#endif

// Functions
#define chnl_init(chnl_ptr, arg_ptr)    (unlikely((chnl_ptr) == NULL || (chnl_ptr)->init_fp == NULL) ? NULL : (chnl_ptr)->init_fp((chnl_ptr), (arg_ptr)))
#define chnl_exit(chnl_ptr)    (unlikely((chnl_ptr) == NULL || (chnl_ptr)->exit_fp == NULL) ? NULL : (chnl_ptr)->exit_fp((chnl_ptr)))
#define chnl_input(chnl_ptr, data, data_len)    (unlikely((chnl_ptr) == NULL || (chnl_ptr)->input_fp == NULL) ? SYS_FAIL : (chnl_ptr)->input_fp((chnl_ptr), (data), (data_len)))
#define chnl_output(chnl_ptr, data, data_len)    (unlikely((chnl_ptr) == NULL || (chnl_ptr)->output_fp == NULL) ? 0 : (chnl_ptr)->output_fp((chnl_ptr), (data), (data_len)))
#define chnl_is_empty(chnl_ptr)    (unlikely((chnl_ptr) == NULL || (chnl_ptr)->is_empty_fp == NULL) ? true : (chnl_ptr)->is_empty_fp((chnl_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the chnl_ptr were given as a value, a compile-time error would be given
// => Default channel: thread channel
#define default_chnl_employ(chnl_ptr)    thrd_chnl_employ(chnl_ptr)
#define default_chnl_dismiss(chnl_ptr)    thrd_chnl_dismiss(chnl_ptr)
// => Thread channel
#define thrd_chnl_employ(chnl_ptr)    (unlikely((chnl_ptr) == NULL) ? NULL : \
                                      (((chnl_ptr)->init_fp = thrd_chnl_init), \
                                      ((chnl_ptr)->exit_fp = thrd_chnl_exit), \
                                      ((chnl_ptr)->input_fp = thrd_chnl_input), \
                                      ((chnl_ptr)->output_fp = thrd_chnl_output), \
                                      ((chnl_ptr)->is_empty_fp = thrd_chnl_is_empty), \
                                      (chnl_ptr)))
#define thrd_chnl_dismiss(chnl_ptr)    (unlikely((chnl_ptr) == NULL) ? NULL : \
                                       (((chnl_ptr)->init_fp = NULL), \
                                       ((chnl_ptr)->exit_fp = NULL), \
                                       ((chnl_ptr)->input_fp = NULL), \
                                       ((chnl_ptr)->output_fp = NULL), \
                                       ((chnl_ptr)->is_empty_fp = NULL), \
                                       (chnl_ptr)))
#ifdef __LINUX_PLAT__
// => Unix domian socket channel
#define uds_chnl_employ(chnl_ptr)    (unlikely((chnl_ptr) == NULL) ? NULL : \
                                     (((chnl_ptr)->init_fp = uds_chnl_init), \
                                     ((chnl_ptr)->exit_fp = uds_chnl_exit), \
                                     ((chnl_ptr)->input_fp = uds_chnl_input), \
                                     ((chnl_ptr)->output_fp = uds_chnl_output), \
                                     ((chnl_ptr)->is_empty_fp = uds_chnl_is_empty), \
                                     (chnl_ptr)))
#define uds_chnl_dismiss(chnl_ptr)    (unlikely((chnl_ptr) == NULL) ? NULL : \
                                      (((chnl_ptr)->init_fp = NULL), \
                                      ((chnl_ptr)->exit_fp = NULL), \
                                      ((chnl_ptr)->input_fp = NULL), \
                                      ((chnl_ptr)->output_fp = NULL), \
                                      ((chnl_ptr)->is_empty_fp = NULL), \
                                      (chnl_ptr)))
#endif

// Implementation
// => initialization arguments for channel
struct chnl_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        // Thread channel
        struct
        {
            queue_init_arg_t rx_queue_init_arg;
            queue_ptr_t tx_queue_ptr;
            thrd_lock_ptr_t tx_queue_lock_ptr;
        }thrd;

#ifdef __LINUX_PLAT__
        // Unix domian socket channel
        struct
        {
            uds_chnl_mode_t mode;
            char *path;    // not for uds_chnl_mode_reuse
            chnl_ptr_t chnl_ptr;    // only for uds_chnl_mode_reuse
        }uds;
#endif
    }idv;
};

// => Abstract data type
struct chnl
{
    // General variables
    // => Public
    chnl_ptr_t (*init_fp) (chnl_ptr_t chnl_ptr, const chnl_init_arg_ptr_t arg_ptr);
    chnl_ptr_t (*exit_fp) (chnl_ptr_t chnl_ptr);
    int (*input_fp) (chnl_ptr_t chnl_ptr, const void *data, size_t data_len);
    size_t (*output_fp) (chnl_ptr_t chnl_ptr, void *data, size_t data_size);
    bool (*is_empty_fp) (chnl_ptr_t chnl_ptr);
    // => Private
    // N/A
    // Individual variables which must be private
    union
    {
        // Thread channel
        struct
        {
            queue_t rx_queue;
            thrd_lock_t rx_queue_lock;
            queue_ptr_t tx_queue_ptr;
            thrd_lock_ptr_t tx_queue_lock_ptr;
        }thrd;

#ifdef __LINUX_PLAT__
        // Unix domian socket channel
        struct
        {
            int sock;
            struct sockaddr_un addr;
            uds_chnl_mode_t mode;
            uint32_t data_len;
        }uds;
#endif
    }idv;
};

// => Thread channel
extern chnl_ptr_t thrd_chnl_init (chnl_ptr_t chnl_ptr, const chnl_init_arg_ptr_t arg_ptr);
extern chnl_ptr_t thrd_chnl_exit (chnl_ptr_t chnl_ptr);
extern int thrd_chnl_input (chnl_ptr_t chnl_ptr, const void *data, size_t data_len);
extern size_t thrd_chnl_output (chnl_ptr_t chnl_ptr, void *data, size_t data_size);
extern bool thrd_chnl_is_empty (chnl_ptr_t chnl_ptr);
#ifdef __LINUX_PLAT__
// => Unix domian socket channel
extern chnl_ptr_t uds_chnl_init (chnl_ptr_t chnl_ptr, const chnl_init_arg_ptr_t arg_ptr);
extern chnl_ptr_t uds_chnl_exit (chnl_ptr_t chnl_ptr);
extern int uds_chnl_input (chnl_ptr_t chnl_ptr, const void *data, size_t data_len);
extern size_t uds_chnl_output (chnl_ptr_t chnl_ptr, void *data, size_t data_size);
extern bool uds_chnl_is_empty (chnl_ptr_t chnl_ptr);
#endif

#endif
