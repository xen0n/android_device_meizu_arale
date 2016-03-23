/*
	Here is an interface of connection to hide implementation. 
*/

#ifndef __CONN_H__
#define __CONN_H__

#include "sys_info.h"
#include "compiler.h"
#include "task.h"
#include "event.h"
#include "io/mailbox.h"
#include "io/chnl.h"
#include <stdlib.h>

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
// N/A

// Type definitions
typedef struct conn conn_t;
typedef struct conn* conn_ptr_t;
typedef struct conn** conn_ptr2_t;
typedef struct conn_init_arg conn_init_arg_t;
typedef struct conn_init_arg* conn_init_arg_ptr_t;
typedef int conn_id_t;
typedef void* (*conn_hdl_t) (conn_ptr_t conn_ptr, event_ptr_t event_ptr, void *arg);
typedef bool (*rx_cond_hdl_t) (event_ptr_t event_ptr, void *arg);


// Macros
#define CONN_TBL_SIZE	(1024)
#define CONN_PATH_LEN	(64)
#define CONN_PATH_PREFIX	"tmp/conn_"

// Functions, NEED_TO_BE_NOTICED, if the conn_ptr were given as a value, a compile-time error would rise
// N/A

// Implementation
// => initialization arguments for channel
struct conn_init_arg
{
	// General variables
	conn_hdl_t hdl;
	void *hdl_arg;
	// Individual variables which must be private
	union
	{
		// N/A
	}idv;
};

// => Abstract Data Type
struct conn
{
	// General variables
	conn_id_t id;
	mailbox_t mailbox;
	chnl_t chnl;
	event_seq_t seq;
	task_t task;
	struct conn_data *data_ptr;
	thrd_lock_t mutex;
	thrd_lock_t sem;
	// Individual variables which must be private
	union
	{
		// N/A
	}idv;
};

// => Connection
extern bool mdfx_conn_rx_cond_event_id (conn_ptr_t conn_ptr,event_ptr_t event_ptr, void *arg);

/*
extern conn_ptr_t conn_init (conn_ptr_t conn_ptr);
extern conn_ptr_t conn_exit (conn_ptr_t conn_ptr);
extern int conn_set_id (conn_ptr_t conn_ptr, conn_id_t id);
extern conn_id_t conn_get_id (conn_ptr_t conn_ptr);
extern void* event_set_data (event_ptr_t event_ptr, const void *data, size_t data_len);
extern size_t event_set_data_len (event_ptr_t event_ptr, size_t data_len);	// NEED_TO_BE_NOTICED, not a good design
extern size_t event_set_data_size (event_ptr_t event_ptr, size_t data_size);	// NEED_TO_BE_NOTICED, not a good design
extern void* event_get_data (event_ptr_t event_ptr, size_t *data_len_ptr);
extern size_t event_get_data_len (event_ptr_t event_ptr);	// NEED_TO_BE_NOTICED, not a good design
extern size_t event_get_data_size (event_ptr_t event_ptr);	// NEED_TO_BE_NOTICED, not a good design
extern int event_set_src_addr (event_ptr_t event_ptr, mailbox_addr_ptr_t src_addr_ptr);
extern mailbox_addr_ptr_t event_get_src_addr (event_ptr_t event_ptr);
extern int event_set_ext_chnl (event_ptr_t event_ptr, chnl_ptr_t ext_chnl_ptr);
extern chnl_ptr_t event_get_ext_chnl (event_ptr_t event_ptr);
*/

#endif
