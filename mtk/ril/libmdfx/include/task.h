/*
	Here is an interface of task to hide implementation. 
	The task_exec() passively waits for an event from the corresponding mailbox and than passes the event to user-defined handler. 
*/

#ifndef __TASK_H__
#define __TASK_H__

#include "sys_info.h"
#include "event.h"
#include "compiler.h"
#include "ctnr/queue.h"
#include "io/mailbox.h"
#include "thrd.h"
#include "thrd/thrd_lock.h"


// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// N/A

// Type definitions
typedef struct task task_t;
typedef struct task* task_ptr_t;
typedef struct task_init_arg task_init_arg_t;
typedef struct task_init_arg* task_init_arg_ptr_t;
typedef void* (*task_hdl_t) (task_ptr_t task_ptr, event_ptr_t event_ptr, void *arg);
typedef bool (*task_rx_cond_t) (task_ptr_t task_ptr, event_ptr_t event_ptr, void *arg);

// Macros
// => Thread task
// N/A
// => MFI task
#define MFI_TASK_LISTEN_BACKLOG	8
#ifdef __ANDROID__
#define MFI_TASK_PATH	"mal-mfi"
#else
#define MFI_TASK_PATH	"/data/mal/tmp/mfi_task"
#endif
// => MFIA task
#define MFIA_TASK_LISTEN_BACKLOG	8
#define MFIA_PATH_LEN	64
#ifndef __ANDROID__
#define MFIA_TASK_PATH_PREFIX	"/data/mal/tmp/mfia_task_"
#endif
// => ATI task
#define ATI_TASK_LISTEN_BACKLOG	8
#define ATI_TASK_ATC_BUF_SIZE	4096
#define ATI_TASK_UART_PATH_1	"/dev/radio/malcmd1"
#define ATI_TASK_UART_PATH_2	"/dev/radio/malcmd2"
#define ATI_TASK_UART_PATH_3	"/dev/radio/malcmd3"
#define ATI_TASK_UART_PATH_4	"/dev/radio/malcmd4"
#define ATI_TASK_UART_PATH_URC	"/dev/radio/malurc"
#ifndef __ANDROID__
#define ATI_TASK_PATH	"/data/mal/tmp/ati_task"
#endif

// Functions
#define task_init(task_ptr, arg_ptr)	(unlikely((task_ptr) == NULL || (task_ptr)->init_fp == NULL) ? NULL : (task_ptr)->init_fp((task_ptr), (arg_ptr)))
#define task_exit(task_ptr)	(unlikely((task_ptr) == NULL || (task_ptr)->exit_fp == NULL) ? NULL : (task_ptr)->exit_fp((task_ptr)))
#define task_exec(task_ptr, hdl, arg)	(unlikely((task_ptr) == NULL || (task_ptr)->exec_fp == NULL) ? SYS_FAIL : (task_ptr)->exec_fp((task_ptr), (hdl), (arg)))
#define task_get_id(task_ptr)	(unlikely((task_ptr) == NULL) ? TASK_INVAL_ID : (task_ptr)->id)
// => Thread task
#define task_get_mailbox(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : &((task_ptr)->mailbox))
// => MFI task
// N/A
// => MFIA task
// N/A

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the task_ptr were given as a value, a compile-time error would be given
// => Default task
#define default_task_employ(task_ptr)	thrd_task_employ(task_ptr)
#define default_task_dismiss(task_ptr)	thrd_task_dismiss(task_ptr)
// => Thread task
#define thrd_task_employ(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                  	(((task_ptr)->init_fp = thrd_task_init), \
                                  	((task_ptr)->exit_fp = thrd_task_exit), \
                                  	((task_ptr)->exec_fp = thrd_task_exec), \
                                  	(task_ptr)))
#define thrd_task_dismiss(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                   	(((task_ptr)->init_fp = NULL), \
                                   	((task_ptr)->exit_fp = NULL), \
                                   	((task_ptr)->exec_fp = NULL), \
                                   	(task_ptr)))
// => MFI task
#define mfi_task_employ(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                 	(((task_ptr)->init_fp = mfi_task_init), \
                                 	((task_ptr)->exit_fp = mfi_task_exit), \
                                 	((task_ptr)->exec_fp = mfi_task_exec), \
                                 	(task_ptr)))
#define mfi_task_dismiss(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                  	(((task_ptr)->init_fp = NULL), \
                                  	((task_ptr)->exit_fp = NULL), \
                                  	((task_ptr)->exec_fp = NULL), \
                                  	(task_ptr)))

// => ATI task
#define ati_task_employ(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                 	(((task_ptr)->init_fp = ati_task_init), \
                                 	((task_ptr)->exit_fp = ati_task_exit), \
                                 	((task_ptr)->exec_fp = ati_task_exec), \
                                 	(task_ptr)))
#define ati_task_dismiss(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                  	(((task_ptr)->init_fp = NULL), \
                                  	((task_ptr)->exit_fp = NULL), \
                                  	((task_ptr)->exec_fp = NULL), \
                                  	(task_ptr)))

// => MFIA task
#define mfia_task_employ(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                  	(((task_ptr)->init_fp = mfia_task_init), \
                                  	((task_ptr)->exit_fp = mfia_task_exit), \
                                  	((task_ptr)->exec_fp = mfia_task_exec), \
                                  	(task_ptr)))
#define mfia_task_dismiss(task_ptr)	(unlikely((task_ptr) == NULL) ? NULL : \
                                  	(((task_ptr)->init_fp = NULL), \
                                  	((task_ptr)->exit_fp = NULL), \
                                  	((task_ptr)->exec_fp = NULL), \
                                  	(task_ptr)))

// Implementation
// => initialization arguments for tasks
struct task_init_arg
{
	// General variables
	task_id_t id;
	// Individual variables which must be private
	union
	{
		// Thread task
		struct
		{
			// N/A
		}thrd;

		// MFI task
		struct
		{
			// N/A
		}mfi;

		// ATI task
		struct
		{
			// N/A
		}ati;

		// MFIA task
		struct
		{
			char *chnl_name;
		}mfia;
	}idv;
};

// => Abstract data type
struct task
{
	// General variables
	// => Public
	task_ptr_t (*init_fp) (task_ptr_t task_ptr, const task_init_arg_ptr_t arg_ptr);
	task_ptr_t (*exit_fp) (task_ptr_t task_ptr);
	int (*exec_fp) (task_ptr_t task_ptr, task_hdl_t hdl, void *arg);
	// => Private
	task_id_t id;
	task_hdl_t hdl;
	mailbox_t mailbox;
	// Individual variables which must be private
	union
	{
		// Thread task
		struct
		{
			thrd_t thrd;
			queue_t event_buf;
			task_rx_cond_t rx_cond;
			void *rx_cond_arg;	// NEED_TO_BE_MOTICED, dangerous! need to be maintain by the user
			thrd_lock_t rx_cond_mutex;
		}thrd;

		// MFI task
		struct
		{
			thrd_t thrd;
		}mfi;

		// ATI task
		struct
		{
			thrd_t thrd;
		}ati;

		// MFIA task
		struct
		{
			char path[MFIA_PATH_LEN];
			thrd_t thrd;
		}mfia;
	}idv;
};

// => Thread task
extern task_ptr_t thrd_task_init (task_ptr_t task_ptr, const task_init_arg_ptr_t arg_ptr);
extern task_ptr_t thrd_task_exit (task_ptr_t task_ptr);
extern int thrd_task_exec (task_ptr_t task_ptr, task_hdl_t hdl, void *arg);
// => MFI task
extern task_ptr_t mfi_task_init (task_ptr_t task_ptr, const task_init_arg_ptr_t arg_ptr);
extern task_ptr_t mfi_task_exit (task_ptr_t task_ptr);
extern int mfi_task_exec (task_ptr_t task_ptr, task_hdl_t hdl, void *arg);
// => ATI task
extern task_ptr_t ati_task_init (task_ptr_t task_ptr, const task_init_arg_ptr_t arg_ptr);
extern task_ptr_t ati_task_exit (task_ptr_t task_ptr);
extern int ati_task_exec (task_ptr_t task_ptr, task_hdl_t hdl, void *arg);
// => MFIA task
extern task_ptr_t mfia_task_init (task_ptr_t task_ptr, const task_init_arg_ptr_t arg_ptr);
extern task_ptr_t mfia_task_exit (task_ptr_t task_ptr);
extern int mfia_task_exec (task_ptr_t task_ptr, task_hdl_t hdl, void *arg);

#endif
