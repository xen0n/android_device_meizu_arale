/*
    Here is an interface of mailbox to hide platform-dependent libraries. 
    A thread mailbox deals with the communication between two entities running on different threads of the same process. 
*/

#ifndef __IO_MAILBOX_H__
#define __IO_MAILBOX_H__

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
// => Thread mailbox
// N/A
#ifdef __LINUX_PLAT__
// => UDS mailbox
// N/A
#endif

// Type definitions
typedef struct mailbox mailbox_t;
typedef struct mailbox* mailbox_ptr_t;
typedef struct mailbox** mailbox_ptr2_t;
typedef struct mailbox_init_arg mailbox_init_arg_t;
typedef struct mailbox_init_arg* mailbox_init_arg_ptr_t;
typedef struct mailbox_addr mailbox_addr_t;
typedef struct mailbox_addr* mailbox_addr_ptr_t;
typedef struct mailbox_addr** mailbox_addr_ptr2_t;
// => Thread mailbox
// N/A
#ifdef __LINUX_PLAT__
// => Unix domian socket mailbox
typedef enum uds_mailbox_mode {uds_mailbox_mode_fail, uds_mailbox_mode_normal, uds_mailbox_mode_reuse} uds_mailbox_mode_t;
#endif

// Macros
// => Thread mailbox
// N/A
#ifdef __LINUX_PLAT__
// => UDS mailbox
// N/A
#endif

// Functions
#define mailbox_init(mailbox_ptr, arg_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->init_fp == NULL) ? NULL : (mailbox_ptr)->init_fp((mailbox_ptr), (arg_ptr)))
#define mailbox_exit(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->exit_fp == NULL) ? NULL : (mailbox_ptr)->exit_fp((mailbox_ptr)))
#define mailbox_send(mailbox_ptr, data, data_len, dst_addr_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->send_fp == NULL) ? SYS_FAIL : (mailbox_ptr)->send_fp((mailbox_ptr), (data), (data_len), (dst_addr_ptr)))
#define mailbox_recv(mailbox_ptr, data, data_len, src_addr_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->recv_fp == NULL) ? 0 : (mailbox_ptr)->recv_fp((mailbox_ptr), (data), (data_len), (src_addr_ptr)))
#define mailbox_get_addr(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->get_addr_fp == NULL) ? NULL : (mailbox_ptr)->get_addr_fp((mailbox_ptr)))
#define mailbox_is_empty(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL || (mailbox_ptr)->is_empty_fp == NULL) ? true : (mailbox_ptr)->is_empty_fp((mailbox_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the mailbox_ptr were given as a value, a compile-time error would be given
// => Default mailbox_ptr: thread mailbox
#define default_mailbox_employ(mailbox_ptr)    thrd_mailbox_employ(mailbox_ptr)
#define default_mailbox_dismiss(mailbox_ptr)    thrd_mailbox_dismiss(mailbox_ptr)
// => Thread mailbox
#define thrd_mailbox_employ(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL) ? NULL : \
                                            (((mailbox_ptr)->init_fp = thrd_mailbox_init), \
                                            ((mailbox_ptr)->exit_fp = thrd_mailbox_exit), \
                                            ((mailbox_ptr)->send_fp = thrd_mailbox_send), \
                                            ((mailbox_ptr)->recv_fp = thrd_mailbox_recv), \
                                            ((mailbox_ptr)->get_addr_fp = thrd_mailbox_get_addr), \
                                            ((mailbox_ptr)->is_empty_fp = thrd_mailbox_is_empty), \
                                            (mailbox_ptr)))
#define thrd_mailbox_dismiss(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL) ? NULL : \
                                             (((mailbox_ptr)->init_fp = NULL), \
                                             ((mailbox_ptr)->exit_fp = NULL), \
                                             ((mailbox_ptr)->send_fp = NULL), \
                                             ((mailbox_ptr)->recv_fp = NULL), \
                                             ((mailbox_ptr)->get_addr_fp = NULL), \
                                             ((mailbox_ptr)->is_empty_fp = NULL), \
                                             (mailbox_ptr)))
#ifdef __LINUX_PLAT__
// => UDS mailbox
#define uds_mailbox_employ(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL) ? NULL : \
                                           (((mailbox_ptr)->init_fp = uds_mailbox_init), \
                                           ((mailbox_ptr)->exit_fp = uds_mailbox_exit), \
                                           ((mailbox_ptr)->send_fp = uds_mailbox_send), \
                                           ((mailbox_ptr)->recv_fp = uds_mailbox_recv), \
                                           ((mailbox_ptr)->get_addr_fp = uds_mailbox_get_addr), \
                                           ((mailbox_ptr)->is_empty_fp = uds_mailbox_is_empty), \
                                           (mailbox_ptr)))
#define uds_mailbox_dismiss(mailbox_ptr)    (unlikely((mailbox_ptr) == NULL) ? NULL : \
                                            (((mailbox_ptr)->init_fp = NULL), \
                                            ((mailbox_ptr)->exit_fp = NULL), \
                                            ((mailbox_ptr)->send_fp = NULL), \
                                            ((mailbox_ptr)->recv_fp = NULL), \
                                            ((mailbox_ptr)->get_addr_fp = NULL), \
                                            ((mailbox_ptr)->is_empty_fp = NULL), \
                                            (mailbox_ptr)))
#endif

// Implementation
// => initialization arguments for mailbox
struct mailbox_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        // Thread mailbox
        struct
        {
            queue_init_arg_t rx_queue_init_arg;
        }thrd;

#ifdef __LINUX_PLAT__
        // UDS mailbox
        struct
        {
            uds_mailbox_mode_t mode;
            char *path;    // only for uds_mailbox_mode_normal
            mailbox_ptr_t mailbox_ptr;    // only for uds_mailbox_mode_reuse
        }uds;
#endif
    }idv;
};

// => mailbox address
struct mailbox_addr
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        // Thread mailbox
        struct
        {
            queue_ptr_t tx_queue_ptr;
            thrd_lock_ptr_t tx_queue_lock_ptr;
        }thrd;

#ifdef __LINUX_PLAT__
        // UDS mailbox
        struct
        {
            struct sockaddr_un addr;
        }uds;
#endif
    }idv;
};

// => Abstract data type
struct mailbox
{
    // General variables
    // => Public
    mailbox_ptr_t (*init_fp) (mailbox_ptr_t mailbox_ptr, const mailbox_init_arg_ptr_t arg_ptr);
    mailbox_ptr_t (*exit_fp) (mailbox_ptr_t mailbox_ptr);
    int (*send_fp) (mailbox_ptr_t mailbox_ptr, const void *data, size_t data_len, const mailbox_addr_ptr_t dst_addr_ptr);
    size_t (*recv_fp) (mailbox_ptr_t mailbox_ptr, void *data, size_t data_size, mailbox_addr_ptr_t src_addr_ptr);
    const mailbox_addr_ptr_t (*get_addr_fp) (mailbox_ptr_t mailbox_ptr);
    bool (*is_empty_fp) (mailbox_ptr_t mailbox_ptr);
    // => Private
    mailbox_addr_t addr;
    // Individual variables which must be private
    union
    {
        // Thread mailbox
        struct
        {
            queue_t rx_queue;
            thrd_lock_t rx_queue_lock;
        }thrd;

#ifdef __LINUX_PLAT__
        // Unix domian socket channel
        struct
        {
            int sock;
            uds_mailbox_mode_t mode;
            uint32_t data_len;
        }uds;
#endif
    }idv;
};

// => Thread mailbox
extern mailbox_ptr_t thrd_mailbox_init (mailbox_ptr_t mailbox_ptr, const mailbox_init_arg_ptr_t arg_ptr);
extern mailbox_ptr_t thrd_mailbox_exit (mailbox_ptr_t mailbox_ptr);
extern int thrd_mailbox_send (mailbox_ptr_t mailbox_ptr, const void *data, size_t data_len, const mailbox_addr_ptr_t dst_addr_ptr);
extern size_t thrd_mailbox_recv (mailbox_ptr_t mailbox_ptr, void *data, size_t data_size, mailbox_addr_ptr_t src_addr_ptr);
extern const mailbox_addr_ptr_t thrd_mailbox_get_addr (mailbox_ptr_t mailbox_ptr);
extern bool thrd_mailbox_is_empty (mailbox_ptr_t mailbox_ptr);
#ifdef __LINUX_PLAT__
// => UDS mailbox
extern mailbox_ptr_t uds_mailbox_init (mailbox_ptr_t mailbox_ptr, const mailbox_init_arg_ptr_t arg_ptr);
extern mailbox_ptr_t uds_mailbox_exit (mailbox_ptr_t mailbox_ptr);
extern int uds_mailbox_send (mailbox_ptr_t mailbox_ptr, const void *data, size_t data_len, const mailbox_addr_ptr_t dst_addr_ptr);
extern size_t uds_mailbox_recv (mailbox_ptr_t mailbox_ptr, void *data, size_t data_size, mailbox_addr_ptr_t src_addr_ptr);
extern const mailbox_addr_ptr_t uds_mailbox_get_addr (mailbox_ptr_t mailbox_ptr);
extern bool uds_mailbox_is_empty (mailbox_ptr_t mailbox_ptr);
#endif

#endif
