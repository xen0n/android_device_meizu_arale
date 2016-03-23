/*
    Here is an interface of the queue to hide implementation. 
    The return value of dequeue indicates (1) the retrieved length when it is smaller than or equal to the val_len; (2) the required size when it is larger than the val_len. 
*/

#ifndef __CTNR_QUEUE_H__
#define __CTNR_QUEUE_H__

#include "sys_info.h"
#include "compiler.h"
#include "ctnr/pool.h"
#include <stddef.h>
#include <stdarg.h>

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
// => Threaded queue
// N/A

// Type definitions
typedef struct queue queue_t;
typedef struct queue* queue_ptr_t;
typedef struct queue_node queue_node_t;
typedef struct queue_node* queue_node_ptr_t;
typedef struct queue_node** queue_node_ptr2_t;
typedef struct queue_init_arg queue_init_arg_t;
typedef struct queue_init_arg* queue_init_arg_ptr_t;
typedef void (*queue_hook_fp_t) (void *val, size_t val_len, va_list vl);
// => Threaded queue
typedef int queue_id_t;

// Macros
#define QUEUE_NODE_VAL_INIT_SIZE    (sizeof(int))    // NEED_TO_BE_NOTICED, we assume that an integer is the most common data to be stored
// => Threaded queue
#define THRD_QUEUE_INVAL_ID    (-1)
#define THRD_QUEUE_TRUNK_ID    (0)
#define THRD_QUEUE_LINK_TBL_INIT_SIZE    (32)    // NEED_TO_BE_NOTICED, set by compiler

// Functions
#define queue_init(queue_ptr, arg_ptr)    (unlikely((queue_ptr) == NULL || (queue_ptr)->init_fp == NULL) ? NULL : (queue_ptr)->init_fp((queue_ptr), (arg_ptr)))
#define queue_exit(queue_ptr)    (unlikely((queue_ptr) == NULL || (queue_ptr)->exit_fp == NULL) ? NULL : (queue_ptr)->exit_fp((queue_ptr)))
#define queue_enqueue(queue_ptr, val, val_len)    (unlikely((queue_ptr) == NULL || (queue_ptr)->enqueue_fp == NULL) ? SYS_FAIL : (queue_ptr)->enqueue_fp((queue_ptr), (val), (val_len)))
#define queue_dequeue(queue_ptr, val, val_size)    (unlikely((queue_ptr) == NULL || (queue_ptr)->dequeue_fp == NULL) ? 0 : (queue_ptr)->dequeue_fp((queue_ptr), (val), (val_size)))
#define queue_iterate(queue_ptr, hook_fp, args ...)    (unlikely((queue_ptr) == NULL || (queue_ptr)->iterate_fp == NULL) ? SYS_FAIL : (queue_ptr)->iterate_fp((queue_ptr), (hook_fp), ## args))
#define queue_is_empty(queue_ptr)    (unlikely((queue_ptr) == NULL || (queue_ptr)->is_empty_fp == NULL) ? true : (queue_ptr)->is_empty_fp((queue_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the queue_ptr were given as a value, a compile-time error would be given
// => Default queue: chained queue
#define default_queue_employ(queue_ptr)    thrd_queue_employ(queue_ptr)
#define default_queue_dismiss(queue_ptr)    thrd_queue_dismiss(queue_ptr)
// => Threaded queue
#define thrd_queue_employ(queue_ptr)    (unlikely((queue_ptr) == NULL) ? NULL : \
                                        (((queue_ptr)->init_fp = thrd_queue_init),    \
                                        ((queue_ptr)->exit_fp = thrd_queue_exit),    \
                                        ((queue_ptr)->enqueue_fp = thrd_queue_enqueue),    \
                                        ((queue_ptr)->dequeue_fp = thrd_queue_dequeue),    \
                                        ((queue_ptr)->iterate_fp = thrd_queue_iterate),    \
                                        ((queue_ptr)->is_empty_fp = thrd_queue_is_empty),    \
                                        (queue_ptr)))
#define thrd_queue_dismiss(queue_ptr)    (unlikely((queue_ptr) == NULL) ? NULL : \
                                         (((queue_ptr)->init_fp = NULL),    \
                                         ((queue_ptr)->exit_fp = NULL),    \
                                         ((queue_ptr)->enqueue_fp = NULL),    \
                                         ((queue_ptr)->dequeue_fp = NULL),    \
                                         ((queue_ptr)->iterate_fp = NULL),    \
                                         ((queue_ptr)->is_empty_fp = NULL),    \
                                         (queue_ptr)))

// Implementation
// => initialization arguments for queues
struct queue_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        struct
        {
            queue_ptr_t trunk;
        }thrd;
    }idv;
};

// => Internal data type for the queue
struct queue_node
{
    // General variables
    void *val;
    size_t val_size;
    size_t val_len;
    // Individual variables which must be private
    union
    {
        // Threaded queue
        struct
        {
            queue_node_ptr2_t link_tbl;
            size_t link_tbl_size;
            size_t link_tbl_cnt;
            // for trunk linked list only
            queue_node_ptr_t rev_link;
        }thrd;
    }idv;    
};

// => Abstract Data Type
struct queue
{
    // General variables
    // => Public
    queue_ptr_t (*init_fp) (queue_ptr_t queue_ptr, const queue_init_arg_ptr_t arg_ptr);
    queue_ptr_t (*exit_fp) (queue_ptr_t queue_ptr);
    int (*enqueue_fp) (queue_ptr_t queue_ptr, const void *val, size_t val_len);
    size_t (*dequeue_fp) (queue_ptr_t queue_ptr, void *val, size_t val_size);
    int (*iterate_fp) (queue_ptr_t queue_ptr, queue_hook_fp_t hook_fp, ...);
    bool (*is_empty_fp) (queue_ptr_t queue_ptr);
    // => Private
    pool_t node_pool;
    // Individual variables which must be private
    union
    {
        // Threaded queue
        struct
        {
            queue_id_t id;
            queue_node_ptr_t head, tail;
            queue_ptr_t trunk;
            // for trunk linked list only
            queue_id_t next_id;    // NEED_TO_BE_NOTICED, how to recycle?
        }thrd;
    }idv;
};

// => Threaded queue
extern queue_ptr_t thrd_queue_init (queue_ptr_t queue_ptr, const queue_init_arg_ptr_t arg_ptr);
extern queue_ptr_t thrd_queue_exit (queue_ptr_t queue_ptr);
extern int thrd_queue_enqueue (queue_ptr_t queue_ptr, const void *val, size_t val_len);
extern size_t thrd_queue_dequeue (queue_ptr_t queue_ptr, void *val, size_t val_size);
extern int thrd_queue_iterate (queue_ptr_t queue_ptr, queue_hook_fp_t hook_fp, ...);
extern bool thrd_queue_is_empty (queue_ptr_t queue_ptr);

#endif
