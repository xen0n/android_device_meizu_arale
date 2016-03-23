/*
    Here is an interface of the map to hide implementation. 
    A map provides the association between the keys and values. 
    Using a pool is a default policy for managing memory allocation. 
    The insert function can only insert a new node without a collision. 
    The update function overwrites the value of an existing node. 
*/

#ifndef __CTNR_MAP_H__
#define __CTNR_MAP_H__

#include "sys_info.h"
#include "compiler.h"
#include "ctnr/pool.h"
#include "ctnr/chain.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
// => Hash map
// N/A

// Type definitions
typedef struct map map_t;
typedef struct map* map_ptr_t;
typedef struct map_node map_node_t;
typedef struct map_node* map_node_ptr_t;
typedef struct map_node** map_node_ptr2_t;
typedef struct map_init_arg map_init_arg_t;
typedef struct map_init_arg* map_init_arg_ptr_t;
typedef void (*map_hook_fp_t) (const void *key, size_t key_len, void *val, size_t val_len, va_list vl);
// => Hash map
typedef int32_t (*hash_map_hash_fp_t) (map_ptr_t map_ptr, const void *key, size_t key_len);    // NEED_TO_BE_NOTICED, should NOT assume 32-bit

// Macros
#define MAP_NODE_KEY_INIT_SIZE    (sizeof(int))    // NEED_TO_BE_NOTICED, we assume that an integer is the most common key to be used
#define MAP_NODE_VAL_INIT_SIZE    (sizeof(int))    // NEED_TO_BE_NOTICED, we assume that an integer is the most common data to be stored
// => Hash map
#define HASH_MAP_INVAL_INDEX    CHAIN_INVAL_IDX
#define HASH_MAP_DEFAULT_TBL_SIZE_PWR    (8)    // NEED_TO_BE_NOTICED, more than 31 is useless due to the 32-bit assumption

// Functions
#define map_init(map_ptr, arg_ptr)    (unlikely((map_ptr) == NULL || (map_ptr)->init_fp == NULL) ? NULL : (map_ptr)->init_fp((map_ptr), (arg_ptr)))
#define map_exit(map_ptr)    (unlikely((map_ptr) == NULL || (map_ptr)->exit_fp == NULL) ? NULL : (map_ptr)->exit_fp((map_ptr)))
#define map_insert(map_ptr, key, key_len, val, val_len)    (unlikely((map_ptr) == NULL || (map_ptr)->insert_fp == NULL) ? NULL : (map_ptr)->insert_fp((map_ptr), (key), (key_len), (val), (val_len)))
#define map_update(map_ptr, key, key_len, val, val_len)    (unlikely((map_ptr) == NULL || (map_ptr)->update_fp == NULL) ? NULL : (map_ptr)->update_fp((map_ptr), (key), (key_len), (val), (val_len)))
#define map_search(map_ptr, key, key_len, val_len_ptr)    (unlikely((map_ptr) == NULL || (map_ptr)->search_fp == NULL) ? NULL : (map_ptr)->search_fp((map_ptr), (key), (key_len), (val_len_ptr)))
#define map_delete(map_ptr, key, key_len)    (unlikely((map_ptr) == NULL || (map_ptr)->delete_fp == NULL) ? SYS_FAIL : (map_ptr)->delete_fp((map_ptr), (key), (key_len)))
#define map_iterate(map_ptr, hook_fp, args ...)    (unlikely((map_ptr) == NULL || (map_ptr)->iterate_fp == NULL) ? SYS_FAIL : (map_ptr)->iterate_fp((map_ptr), (hook_fp), ## args))
#define map_is_empty(map_ptr)    (unlikely((map_ptr) == NULL || (map_ptr)->is_empty_fp == NULL) ? true : (map_ptr)->is_empty_fp((map_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the map_ptr were given as a value, a compile-time error would be given
// => Default map: hash map
#define default_map_employ(map_ptr)    hash_map_employ(map_ptr)
#define default_map_dismiss(map_ptr)    hash_map_dismiss(map_ptr)
// => Hash map
#define hash_map_employ(map_ptr)    (unlikely((map_ptr) == NULL) ? NULL : \
                                    (((map_ptr)->init_fp = hash_map_init),    \
                                    ((map_ptr)->exit_fp = hash_map_exit),    \
                                    ((map_ptr)->insert_fp = hash_map_insert),    \
                                    ((map_ptr)->update_fp = hash_map_update),    \
                                    ((map_ptr)->search_fp = hash_map_search),    \
                                    ((map_ptr)->delete_fp = hash_map_delete),    \
                                    ((map_ptr)->iterate_fp = hash_map_iterate),    \
                                    ((map_ptr)->is_empty_fp = hash_map_is_empty),    \
                                    (map_ptr)))
#define hash_map_dismiss(map_ptr)    (unlikely((map_ptr) == NULL) ? NULL : \
                                     (((map_ptr)->init_fp = NULL),    \
                                     ((map_ptr)->exit_fp = NULL),    \
                                     ((map_ptr)->insert_fp = NULL),    \
                                     ((map_ptr)->update_fp = NULL),    \
                                     ((map_ptr)->search_fp = NULL),    \
                                     ((map_ptr)->delete_fp = NULL),    \
                                     ((map_ptr)->iterate_fp = NULL),    \
                                     ((map_ptr)->is_empty_fp = NULL),    \
                                     (map_ptr)))

// Implementation
// => initialization arguments for maps
struct map_init_arg
{
    // General variables
    // N/A
    // Individual variables which must be private
    union
    {
        // Hash map
        struct
        {
            hash_map_hash_fp_t hash_fp;
            size_t tbl_size_pwr;
        }hash;
    }idv;
};

// => Internal data type for the map
struct map_node
{
    // General variables
    void *key, *val;
    size_t key_size, val_size;
    size_t key_len, val_len;
    // Individual variables which must be private
    union
    {
        // Hash map
        struct
        {
            struct map_node *link;
        }hash;
    }idv;
};

// => Abstract Data Type
struct map
{
    // General variables
    // => Public
    map_ptr_t (*init_fp) (map_ptr_t map_ptr, const map_init_arg_ptr_t arg_ptr);
    map_ptr_t (*exit_fp) (map_ptr_t map_ptr);
    void* (*insert_fp) (map_ptr_t map_ptr, const void *key, size_t key_len, const void *val, size_t val_len);
    void* (*update_fp) (map_ptr_t map_ptr, const void *key, size_t key_len, const void *val, size_t val_len);
    void* (*search_fp) (map_ptr_t map_ptr, const void *key, size_t key_len, size_t *val_len_ptr);
    int (*delete_fp) (map_ptr_t map_ptr, const void *key, size_t key_len);
    int (*iterate_fp) (map_ptr_t map_ptr, map_hook_fp_t hook_fp, ...);
    bool (*is_empty_fp) (map_ptr_t map_ptr);
    // => Private
    pool_t node_pool;
    // Individual variables which must be private
    union
    {
        struct
        {
            hash_map_hash_fp_t hash_fp;
            chain_t tbl;
            size_t tbl_size_pwr;
        }hash;
    }idv;
};

// => Hash map
extern map_ptr_t hash_map_init (map_ptr_t map_ptr, const map_init_arg_ptr_t arg_ptr);
extern map_ptr_t hash_map_exit (map_ptr_t map_ptr);
extern void* hash_map_insert (map_ptr_t map_ptr, const void* key, size_t key_len, const void* val, size_t val_len);
extern void* hash_map_update (map_ptr_t map_ptr, const void* key, size_t key_len, const void* val, size_t val_len);
extern void* hash_map_search (map_ptr_t map_ptr, const void* key, size_t key_len, size_t *val_len_ptr);
extern int hash_map_delete (map_ptr_t map_ptr, const void* key, size_t key_len);
extern int hash_map_iterate (map_ptr_t map_ptr, map_hook_fp_t hook_fp, ...);
extern bool hash_map_is_empty (map_ptr_t map_ptr);

#endif
