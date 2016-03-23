/*
	Here is an interface of service to hide implementation. 
	The srv_exec() actively sends a request to MTK MDFX and receives the response from it. 
*/

#ifndef __SRV_H__
#define __SRV_H__

#include "sys_info.h"
#include "compiler.h"
#include "conn.h"
#include "io/mailbox.h"
#include "thrd.h"

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// N/A

// Type definitions
typedef struct srv srv_t;
typedef struct srv* srv_ptr_t;
typedef struct srv_init_arg srv_init_arg_t;
typedef struct srv_init_arg* srv_init_arg_ptr_t;
typedef void* (*srv_hdl_t) (srv_ptr_t srv_ptr, void *arg);
typedef bool (*srv_rx_cond_t) (srv_ptr_t srv_ptr, event_ptr_t event_ptr, void *arg);

// Macros
// => Thread service
// N/A

// Functions
#define srv_init(srv_ptr, arg_ptr)	(unlikely((srv_ptr) == NULL || (srv_ptr)->init_fp == NULL) ? NULL : (srv_ptr)->init_fp((srv_ptr), (arg_ptr)))
#define srv_exit(srv_ptr)	(unlikely((srv_ptr) == NULL || (srv_ptr)->exit_fp == NULL) ? NULL : (srv_ptr)->exit_fp((srv_ptr)))
#define srv_exec(srv_ptr, hdl, arg)	(unlikely((srv_ptr) == NULL || (srv_ptr)->exec_fp == NULL) ? SYS_FAIL : (srv_ptr)->exec_fp((srv_ptr), (hdl), (arg)))
#define srv_get_id(srv_ptr)	(unlikely((srv_ptr) == NULL) ? SRV_INVAL_ID : (srv_ptr)->id)
// => Thread service
#define srv_get_mailbox(srv_ptr)	(unlikely((srv_ptr) == NULL) ? NULL : &((srv_ptr)->mailbox))

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the srv_ptr were given as a value, a compile-time error would be given
// => Default service
#define default_srv_employ(srv_ptr)	thrd_srv_employ(srv_ptr)
#define default_srv_dismiss(srv_ptr)	thrd_srv_dismiss(srv_ptr)
// => Thread service
#define thrd_srv_employ(srv_ptr)	(unlikely((srv_ptr) == NULL) ? NULL : \
                                        	(((srv_ptr)->init_fp = thrd_srv_init), \
                                        	((srv_ptr)->exit_fp = thrd_srv_exit), \
                                        	((srv_ptr)->exec_fp = thrd_srv_exec), \
                                        	(srv_ptr)))
#define thrd_srv_dismiss(srv_ptr)	(unlikely((srv_ptr) == NULL) ? NULL : \
                                         	(((srv_ptr)->init_fp = NULL), \
                                         	((srv_ptr)->exit_fp = NULL), \
                                         	((srv_ptr)->exec_fp = NULL), \
                                         	(srv_ptr)))

// Implementation
// => initialization arguments for service
struct srv_init_arg
{
	// General variables
	task_id_t id;
	conn_ptr_t ext_conn_ptr;
	// Individual variables which must be private
	union
	{
		// Thread service
		struct
		{
			// N/A
		}thrd;
	}idv;
};

// => Abstract data type
struct srv
{
	// General variables
	// => Public
	srv_ptr_t (*init_fp) (srv_ptr_t srv_ptr, const srv_init_arg_ptr_t arg_ptr);
	srv_ptr_t (*exit_fp) (srv_ptr_t srv_ptr);
	int (*exec_fp) (srv_ptr_t srv_ptr, srv_hdl_t hdl, void *arg);
	// => Private
	task_id_t id;
	mailbox_t mailbox;
	// Individual variables which must be private
	union
	{
		// Thread service
		struct
		{
			thrd_t thrd;
		}thrd;
	}idv;
};

// => Thread service
extern srv_ptr_t thrd_srv_init (srv_ptr_t srv_ptr, const srv_init_arg_ptr_t arg_ptr);
extern srv_ptr_t thrd_srv_exit (srv_ptr_t srv_ptr);
extern int thrd_srv_exec (srv_ptr_t srv_ptr, srv_hdl_t hdl, void *arg);

#endif
