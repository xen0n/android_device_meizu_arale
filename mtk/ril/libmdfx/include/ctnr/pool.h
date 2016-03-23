/*
    Here is an interface of the pool to hide implementation. 
    A pool is used to allocate/free memory blocks with an equal size. 
    The initial size of the pool can be given whereas 0 indicates the default size. 
    A pool can trace all the allocated memory blocks and free them regardless of whether they are used or not. 
    The constructor and the destructor of each memory block is also provided. 
*/

#ifndef __CTNR_POOL_H__
#define __CTNR_POOL_H__

#include "sys_info.h"
#include "compiler.h"
#include <stdarg.h>
#include <stddef.h>

// Compiler flags, NEED_TO_BE NOTICED, set by the compiler
// => Two-tank pool
#define TWO_TANK_POOL_SHRINK

// Type definitions
typedef struct pool pool_t;
typedef struct pool* pool_ptr_t;
typedef struct pool_node pool_node_t;
typedef struct pool_node* pool_node_ptr_t;
typedef struct pool_init_arg pool_init_arg_t;
typedef struct pool_init_arg* pool_init_arg_ptr_t;
typedef void (*async_pool_hook_fp_t) (void *block, size_t block_len, const void *arg);
typedef void (*sync_pool_hook_fp_t) (void *block, size_t block_len, va_list vl);

// Macros, NEED_TO_BE_NOTICED, the maximal size should be constrained
#define DEFAULT_POOL_INIT_SIZE    (64)    // the power of 2

// API
#define pool_init(pool_ptr, arg_ptr)    (unlikely((pool_ptr) == NULL || (pool_ptr)->init_fp == NULL) ? NULL : (pool_ptr)->init_fp((pool_ptr), (arg_ptr)))
#define pool_exit(pool_ptr)    (unlikely((pool_ptr) == NULL || (pool_ptr)->exit_fp == NULL) ? NULL : (pool_ptr)->exit_fp((pool_ptr)))
#define pool_borrow(pool_ptr)    (unlikely((pool_ptr) == NULL || (pool_ptr)->borrow_fp == NULL) ? NULL : (pool_ptr)->borrow_fp((pool_ptr)))
#define pool_return(pool_ptr, block)    (unlikely((pool_ptr) == NULL || (block) == NULL || (pool_ptr)->return_fp == NULL) ? SYS_FAIL : (pool_ptr)->return_fp((pool_ptr), (block)))
#define pool_iterate(pool_ptr, hook_fp, args ...)    (unlikely((pool_ptr) == NULL || (hook_fp) == NULL || (pool_ptr)->iterate_fp == NULL) ? SYS_FAIL : (pool_ptr)->iterate_fp((pool_ptr), (hook_fp), ## args))
#define pool_get_block_len(pool_ptr)    (unlikely((pool_ptr) == NULL) ? 0 : (const size_t)((pool_ptr)->block_len))    // NEED_TO_BE_NOTICED, the block length might be modified

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the pool_ptr were given as a value, a compile-time error would be given
// => Default pool: two-tank pool
#define default_pool_employ(pool_ptr)    two_tank_pool_employ(pool_ptr)
#define default_pool_dismiss(pool_ptr)    two_tank_pool_dismiss(pool_ptr)
// => Two-tank pool
#define two_tank_pool_employ(pool_ptr)    (unlikely((pool_ptr) == NULL) ? NULL : \
                                          (((pool_ptr)->init_fp = two_tank_pool_init), \
                                          ((pool_ptr)->exit_fp = two_tank_pool_exit), \
                                          ((pool_ptr)->borrow_fp = two_tank_pool_borrow), \
                                          ((pool_ptr)->return_fp = two_tank_pool_return), \
                                          ((pool_ptr)->iterate_fp = two_tank_pool_iterate), \
                                          (pool_ptr)))
#define two_tank_pool_dismiss(pool_ptr)    (unlikely((pool_ptr) == NULL) ? NULL : \
                                           (((pool_ptr)->init_fp = NULL), \
                                           ((pool_ptr)->exit_fp = NULL), \
                                           ((pool_ptr)->borrow_fp = NULL), \
                                           ((pool_ptr)->return_fp = NULL), \
                                           ((pool_ptr)->iterate_fp = NULL), \
                                           (pool_ptr)))

// Implementation
// => initialization arguments for threads
struct pool_init_arg
{
    // General variables
    size_t block_len;
    size_t init_size;
    async_pool_hook_fp_t block_init_fp;
    const void *block_init_arg;
    size_t block_init_arg_len;
    async_pool_hook_fp_t block_exit_fp;
    const void *block_exit_arg;
    size_t block_exit_arg_len;
    // Individual variables which must be private
    union
    {
        struct
        {
            // N/A
        }two_tank;
    }idv;
};

// => Internal data type for the pool
struct pool_node
{
    // General variables
    void *block;
    // Individual variables which must be private
    union
    {
        // Two-tank pool
        struct
        {
            pool_node_ptr_t link;
        }two_tank;
    }idv;
};

// => Abstract data type
struct pool
{
    // General variables
    // => Public
    pool_ptr_t (*init_fp) (pool_ptr_t pool_ptr, const pool_init_arg_ptr_t arg_ptr);
    pool_ptr_t (*exit_fp) (pool_ptr_t pool_ptr);
    void* (*borrow_fp) (pool_ptr_t pool_ptr);
    int (*return_fp) (pool_ptr_t pool_ptr, void *block);
    int (*iterate_fp) (pool_ptr_t pool_ptr, sync_pool_hook_fp_t hook_fp, ...);
    // => Private
    size_t block_len;
    size_t capacity;
    size_t init_size;
    async_pool_hook_fp_t block_init_fp;
    void *block_init_arg;
    async_pool_hook_fp_t block_exit_fp;
    void *block_exit_arg;
    pool_node_ptr_t entry;
    // Individual variables which must be private
    union
    {
        // Two-tank pool
        struct
        {
#ifdef TWO_TANK_POOL_SHRINK
            size_t clean_size;
#endif
            pool_node_ptr_t dirty_tank;
        }two_tank;
    }idv;
};

// => Two-tank pool
extern pool_ptr_t two_tank_pool_init (pool_ptr_t pool_ptr, const pool_init_arg_ptr_t arg_ptr);
extern pool_ptr_t two_tank_pool_exit (pool_ptr_t pool_ptr);
extern void* two_tank_pool_borrow (pool_ptr_t pool_ptr);
extern int two_tank_pool_return (pool_ptr_t pool_ptr, void *block);
extern int two_tank_pool_iterate (pool_ptr_t pool_ptr, sync_pool_hook_fp_t hook_fp, ...);

#endif
