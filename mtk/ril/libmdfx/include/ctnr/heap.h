/*
	Here is an interface of the heap to hide implementation. 
	A heap provides the association between the keys and values. 
	Using a pool is a default policy for managing memory allocation. 
	The delete function returns the maximum values in the heap with a user-defined comparation function. 
*/

#ifndef __CTNR_HEAP_H__
#define __CTNR_HEAP_H__

#include "sys_info.h"
#include "compiler.h"
#include "ctnr/pool.h"
#include "ctnr/chain.h"
#include <stddef.h>
#include <stdarg.h>

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
// => Chain heap
// N/A

// Type definitions
typedef struct heap heap_t;
typedef struct heap* heap_ptr_t;
typedef struct heap_node heap_node_t;
typedef struct heap_node* heap_node_ptr_t;
typedef struct heap_node** heap_node_ptr2_t;
typedef struct heap_init_arg heap_init_arg_t;
typedef struct heap_init_arg* heap_init_arg_ptr_t;
typedef enum heap_cmp_res {heap_cmp_lesser = -1, heap_cmp_equal, heap_cmp_greater} heap_cmp_res_t;
typedef heap_cmp_res_t (*heap_cmp_fp_t) (const void *key_1, size_t key_len_1, const void *key_2, size_t key_len_2);
typedef void (*heap_hook_fp_t) (const void *key, size_t key_len, void *val, size_t val_len, va_list vl);
// => Chain heap
// N/A

// Macros
#define HEAP_NODE_KEY_INIT_SIZE	(sizeof(int))	// NEED_TO_BE_NOTICED, we assume that an integer is the most common key to be used
#define HEAP_NODE_VAL_INIT_SIZE	(sizeof(int))	// NEED_TO_BE_NOTICED, we assume that an integer is the most common data to be stored
// => Chain heap
#define CHAIN_HEAP_INVAL_INDEX	CHAIN_INVAL_IDX
#define CHAIN_HEAP_INIT_TBL_SIZE_PWR	(6)

// Functions
#define heap_init(heap_ptr, arg_ptr)	(unlikely((heap_ptr) == NULL || (heap_ptr)->init_fp == NULL) ? NULL : (heap_ptr)->init_fp((heap_ptr), (arg_ptr)))
#define heap_exit(heap_ptr)	(unlikely((heap_ptr) == NULL || (heap_ptr)->exit_fp == NULL) ? NULL : (heap_ptr)->exit_fp((heap_ptr)))
#define heap_insert(heap_ptr, key, key_len, val, val_len)	(unlikely((heap_ptr) == NULL || (heap_ptr)->insert_fp == NULL) ? SYS_FAIL : (heap_ptr)->insert_fp((heap_ptr), (key), (key_len), (val), (val_len)))
#define heap_delete(heap_ptr, val, val_size)	(unlikely((heap_ptr) == NULL || (heap_ptr)->delete_fp == NULL) ? 0 : (heap_ptr)->delete_fp((heap_ptr), (val), (val_size)))
#define heap_iterate(heap_ptr, hook_fp, args ...)	(unlikely((heap_ptr) == NULL || (heap_ptr)->iterate_fp == NULL) ? SYS_FAIL : (heap_ptr)->iterate_fp((heap_ptr), (hook_fp), ## args))
#define heap_is_empty(heap_ptr)	(unlikely((heap_ptr) == NULL || (heap_ptr)->is_empty_fp == NULL) ? true : (heap_ptr)->is_empty_fp((heap_ptr)))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the heap_ptr were given as a value, a compile-time error would be given
// => Default heap: chain heap
#define default_heap_employ(heap_ptr)	chain_heap_employ(heap_ptr)
#define default_heap_dismiss(heap_ptr)	chain_heap_dismiss(heap_ptr)
// => Chain heap
#define chain_heap_employ(heap_ptr)	(unlikely((heap_ptr) == NULL) ? NULL : \
                                   	(((heap_ptr)->init_fp = chain_heap_init),	\
                                   	((heap_ptr)->exit_fp = chain_heap_exit),	\
                                   	((heap_ptr)->insert_fp = chain_heap_insert),	\
                                   	((heap_ptr)->delete_fp = chain_heap_delete),	\
                                   	((heap_ptr)->iterate_fp = chain_heap_iterate),	\
                                   	((heap_ptr)->is_empty_fp = chain_heap_is_empty),	\
                                   	(heap_ptr)))
#define chain_heap_dismiss(heap_ptr)	(unlikely((heap_ptr) == NULL) ? NULL : \
                                    	(((heap_ptr)->init_fp = NULL),	\
                                    	((heap_ptr)->exit_fp = NULL),	\
                                    	((heap_ptr)->insert_fp = NULL),	\
                                    	((heap_ptr)->delete_fp = NULL),	\
                                    	((heap_ptr)->iterate_fp = NULL),	\
                                    	((heap_ptr)->is_empty_fp = NULL),	\
                                    	(heap_ptr)))

// Implementation
// => initialization arguments for heaps
struct heap_init_arg
{
	// General variables
	heap_cmp_fp_t cmp_fp;
	// Individual variables which must be private
	union
	{
		// Chain heap
		struct
		{
			// N/A
		}chain;
	}idv;
};

// => Internal data type for the heap
struct heap_node
{
	// General variables
	void *key, *val;
	size_t key_size, val_size;
	size_t key_len, val_len;
	// Individual variables which must be private
	union
	{
		// Chain heap
		struct
		{
			// N/A
		}chain;
	}idv;
};

// => Abstract Data Type
struct heap
{
	// General variables
	// => Public
	heap_ptr_t (*init_fp) (heap_ptr_t heap_ptr, const heap_init_arg_ptr_t arg_ptr);
	heap_ptr_t (*exit_fp) (heap_ptr_t heap_ptr);
	int (*insert_fp) (heap_ptr_t heap_ptr, const void *key, size_t key_len, const void *val, size_t val_len);
	size_t (*delete_fp) (heap_ptr_t heap_ptr, void *val, size_t val_size);
	int (*iterate_fp) (heap_ptr_t heap_ptr, heap_hook_fp_t hook_fp, ...);
	bool (*is_empty_fp) (heap_ptr_t heap_ptr);
	// => Private
	pool_t node_pool;
	heap_cmp_fp_t cmp_fp;
	// Individual variables which must be private
	union
	{
		struct
		{
			chain_t tbl;
			size_t tbl_size_pwr;
		}chain;
	}idv;
};

// => Chain heap
extern heap_ptr_t chain_heap_init (heap_ptr_t heap_ptr, const heap_init_arg_ptr_t arg_ptr);
extern heap_ptr_t chain_heap_exit (heap_ptr_t heap_ptr);
extern int chain_heap_insert (heap_ptr_t heap_ptr, const void* key, size_t key_len, const void* val, size_t val_len);
extern size_t chain_heap_delete (heap_ptr_t heap_ptr, void *val, size_t val_size);
extern int chain_heap_iterate (heap_ptr_t heap_ptr, heap_hook_fp_t hook_fp, ...);
extern bool chain_heap_is_empty (heap_ptr_t heap_ptr);

#endif
