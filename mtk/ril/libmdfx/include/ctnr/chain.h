/*
    Here is an interface of the chain to hide implementation. 
*/

#ifndef __CTNR_CHAIN_H__
#define __CTNR_CHAIN_H__

#include "sys_info.h"
#include "compiler.h"
#include "ctnr/pool.h"
#include <stdarg.h>
#include <stddef.h>

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
// => Array chain
// N/A

// Type definitions
typedef struct chain chain_t;
typedef struct chain* chain_ptr_t;
typedef struct chain_node chain_node_t;
typedef struct chain_node* chain_node_ptr_t;
typedef struct chain_node** chain_node_ptr2_t;
typedef struct chain_init_arg chain_init_arg_t;
typedef struct chain_init_arg* chain_init_arg_ptr_t;
typedef void (*chain_hook_fp_t) (int idx, void *val, size_t val_len, va_list vl);
// => Array chain
// N/A

// Macros
#define CHAIN_INVAL_IDX    (-1)    // NEED_TO_BE_NOTICED, the valid indexes must be greater than 0
#define CHAIN_MAX_CAPACITY    ((size_t)1 << ((sizeof(size_t) << 3) - 1))
#define DEFAULT_CHAIN_INIT_SIZE    DEFAULT_POOL_INIT_SIZE
#define CHAIN_NODE_VAL_INIT_SIZE    (sizeof(int))    // NEED_TO_BE_NOTICED, we assume that an integer is the most common data to be stored
// => Array chain
// N/A

// Functions
#define chain_init(chain_ptr, arg_ptr)    (unlikely((chain_ptr) == NULL || (chain_ptr)->init_fp == NULL) ? NULL : (chain_ptr)->init_fp((chain_ptr), (arg_ptr)))
#define chain_exit(chain_ptr)    (unlikely((chain_ptr) == NULL || (chain_ptr)->exit_fp == NULL) ? NULL : (chain_ptr)->exit_fp((chain_ptr)))
#define chain_insert(chain_ptr, idx, val, val_len)    (unlikely((chain_ptr) == NULL || (chain_ptr)->insert_fp == NULL) ? NULL : (chain_ptr)->insert_fp((chain_ptr), (idx), (val), (val_len)))
#define chain_search(chain_ptr, idx, val_len_ptr)    (unlikely((chain_ptr) == NULL || (chain_ptr)->search_fp == NULL) ? NULL : (chain_ptr)->search_fp((chain_ptr), (idx), (val_len_ptr)))
#define chain_update(chain_ptr, idx, val, val_len)    (unlikely((chain_ptr) == NULL || (chain_ptr)->update_fp == NULL) ? NULL : (chain_ptr)->update_fp((chain_ptr), (idx), (val), (val_len)))
#define chain_delete(chain_ptr, idx)    (unlikely((chain_ptr) == NULL || (chain_ptr)->delete_fp == NULL) ? 0 : (chain_ptr)->delete_fp((chain_ptr), (idx)))
#define chain_iterate(chain_ptr, hook_fp, args ...)    (unlikely((chain_ptr) == NULL || (chain_ptr)->iterate_fp == NULL) ? SYS_FAIL : (chain_ptr)->iterate_fp((chain_ptr), (hook_fp), ## args))
#define chain_is_empty(chain_ptr)    (unlikely((chain_ptr) == NULL) ? true : ((chain_ptr)->max_idx == CHAIN_INVAL_IDX ? true : false))
#define chain_get_min_idx(chain_ptr)    (unlikely((chain_ptr) == NULL) ? CHAIN_INVAL_IDX : (const int)((chain_ptr)->min_idx))
#define chain_get_max_idx(chain_ptr)    (unlikely((chain_ptr) == NULL) ? CHAIN_INVAL_IDX : (const int)((chain_ptr)->max_idx))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the chain_ptr were given as a value, a compile-time error would be given
// => Default chain: chained chain
#define default_chain_employ(chain_ptr)    array_chain_employ(chain_ptr)
#define default_chain_dismiss(chain_ptr)    array_chain_dismiss(chain_ptr)
// => Threaded chain
#define array_chain_employ(chain_ptr)    (unlikely((chain_ptr) == NULL) ? NULL : \
                                         (((chain_ptr)->init_fp = array_chain_init),    \
                                         ((chain_ptr)->exit_fp = array_chain_exit),    \
                                         ((chain_ptr)->insert_fp = array_chain_insert),    \
                                         ((chain_ptr)->update_fp = array_chain_update),    \
                                         ((chain_ptr)->search_fp = array_chain_search),    \
                                         ((chain_ptr)->delete_fp = array_chain_delete),    \
                                         ((chain_ptr)->iterate_fp = array_chain_iterate),    \
                                         (chain_ptr)))
#define array_chain_dismiss(chain_ptr)    (unlikely((chain_ptr) == NULL) ? NULL : \
                                          (((chain_ptr)->init_fp = NULL),    \
                                          ((chain_ptr)->exit_fp = NULL),    \
                                          ((chain_ptr)->insert_fp = NULL),    \
                                          ((chain_ptr)->update_fp = NULL),    \
                                          ((chain_ptr)->search_fp = NULL),    \
                                          ((chain_ptr)->delete_fp = NULL),    \
                                          ((chain_ptr)->iterate_fp = NULL),    \
                                          (chain_ptr)))

// Implementation
// => initialization arguments for chains
struct chain_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        // Array chain
        struct
        {
            // N/A
        }array;
    }idv;
};

// => Internal data type for the chain
struct chain_node
{
    // General variables
    void *val;
    size_t val_size;
    size_t val_len;
    // Individual variables which must be private
    union
    {
        // Array chain
        struct
        {
            // N/A
        }array;
    }idv;    
};

// => Abstract Data Type
struct chain
{
    // General variables
    // => Public
    chain_ptr_t (*init_fp) (chain_ptr_t chain_ptr, const chain_init_arg_ptr_t arg_ptr);
    chain_ptr_t (*exit_fp) (chain_ptr_t chain_ptr);
    void* (*insert_fp) (chain_ptr_t chain_ptr, int idx, const void* val, size_t val_len);
    void* (*update_fp) (chain_ptr_t chain_ptr, int idx, const void* val, size_t val_len);
    void* (*search_fp) (chain_ptr_t chain_ptr, int idx, size_t *val_len_ptr);
    int (*delete_fp) (chain_ptr_t chain_ptr, int idx);
    int (*iterate_fp) (chain_ptr_t chain_ptr, chain_hook_fp_t hook_fp, ...);
    // => Private
    int min_idx;
    int max_idx;
    pool_t node_pool;
    // Individual variables which must be private
    union
    {
        // Array chain
        struct
        {
            size_t capacity;
            chain_node_ptr2_t node_array;
        }array;
    }idv;
};

// => Array chain
extern chain_ptr_t array_chain_init (chain_ptr_t chain_ptr, const chain_init_arg_ptr_t arg_ptr);
extern chain_ptr_t array_chain_exit (chain_ptr_t chain_ptr);
extern void* array_chain_insert (chain_ptr_t chain_ptr, int idx, const void* val, size_t val_len);
extern void* array_chain_update (chain_ptr_t chain_ptr, int idx, const void* val, size_t val_len);
extern void* array_chain_search (chain_ptr_t chain_ptr, int idx, size_t *val_len_ptr);
extern int array_chain_delete (chain_ptr_t chain_ptr, int idx);
extern int array_chain_iterate (chain_ptr_t chain_ptr, chain_hook_fp_t hook_fp, ...);

#endif
