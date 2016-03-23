/*
	Here is an interface of entity to hide implementation. 
	A thread channel deals with the communication between two entities running on different threads of the same process. 
	A socket channel deals with the communication between two entities through the socket interface. 
*/

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "sys_info.h"
#include "task.h"
#include "conn.h"
#include "event.h"
#include "compiler.h"
#include "ctnr/chain.h"
#include "ctnr/map.h"
#include "entity/datamngr/datamngr.h"
#include "entity/mdmngr.h"
#include "entity/nwmngr.h"
#include "entity/imsmngr.h"
#include <sys/types.h>  // NEED_TO_BE_NOTICED, temporarily methods to monitor memory usage

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// => Main entity
// N/A
// => Sample entity
// N/A

// Type definitions
typedef struct entity entity_t;
typedef struct entity* entity_ptr_t;
typedef struct entity_init_arg entity_init_arg_t;
typedef struct entity_init_arg* entity_init_arg_ptr_t;
// => General entity
// N/A
// => Main entity
// => => Default task data
typedef struct reg_data
{
	entity_id_t entity_id;
	task_id_t task_id;
#ifdef __EVENT_USAGE_MONITOR__
    pid_t tid;    
#endif
	mailbox_addr_ptr_t addr_ptr;
}reg_data_t;
typedef struct reg_data* reg_data_ptr_t;
// => => Subscribe event data
typedef struct subscribe_data
{
	event_id_t event_id;
	mailbox_addr_ptr_t addr_ptr;
	struct subscribe_data *link;
}subscribe_data_t;
typedef struct subscribe_data* subscribe_data_ptr_t;
// => => Connection event data
typedef struct conn_data
{
	event_seq_t seq;
	mailbox_addr_ptr_t addr_ptr;
    rx_cond_hdl_t rx_cond;
    void *rx_cond_arg;  // NEED_TO_BE_MOTICED, dangerous! need to be maintain by the user
    conn_hdl_t cb_hdl;
	struct conn_data *link, *rev_link;
}conn_data_t;
typedef struct conn_data* conn_data_ptr_t;

// Macros
// => General entity
// N/A
// => Main entity
// N/A
// => Old sample entity
#define SAMPLE_ENTITY_RX_IDX	0
#define SAMPLE_ENTITY_TX_IDX	1

// Functions
#define entity_init(entity_ptr, arg_ptr)	(unlikely((entity_ptr) == NULL || (entity_ptr)->init_fp == NULL) ? NULL : (entity_ptr)->init_fp((entity_ptr), (arg_ptr)))
#define entity_exit(entity_ptr)	(unlikely((entity_ptr) == NULL || (entity_ptr)->exit_fp == NULL) ? NULL : (entity_ptr)->exit_fp((entity_ptr)))
#define entity_get_id(entity_ptr)	(unlikely((entity_ptr) == NULL) ? ENTITY_INVAL_ID : (entity_ptr)->id)
// => General entity
#define entity_init_arg_set(arg_ptr, data)	(unlikely((arg_ptr) == NULL) ? NULL : (arg_ptr)->idv.gen.arg = (data))
#define entity_init_arg_get(arg_ptr)	(unlikely((arg_ptr) == NULL) ? NULL : (arg_ptr)->idv.gen.arg)
#define entity_set_priv(entity_ptr, priv)	(unlikely((entity_ptr) == NULL) ? NULL : (entity_ptr)->idv.gen.priv = (priv))
#define entity_get_priv(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : (entity_ptr)->idv.gen.priv)

// Interface-implementation binding, NEED_TO_BE_NOTICED, if the entity_ptr were given as a value, a compile-time error would be given
// => Default entity: main entity
#define default_entity_employ(entity_ptr)	main_entity_employ(entity_ptr)
#define default_entity_dismiss(entity_ptr)	main_entity_dismiss(entity_ptr)
// => main entity
#define main_entity_employ(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : \
                                      	(((entity_ptr)->init_fp = main_entity_init), \
                                      	((entity_ptr)->exit_fp = main_entity_exit), \
                                      	(entity_ptr)))
#define main_entity_dismiss(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : \
                                       	(((entity_ptr)->init_fp = NULL), \
                                       	((entity_ptr)->exit_fp = NULL), \
                                       	(entity_ptr)))

// => Old sample entity
#define old_sample_entity_employ(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : \
                                            	(((entity_ptr)->init_fp = old_sample_entity_init), \
                                            	((entity_ptr)->exit_fp = old_sample_entity_exit), \
                                            	(entity_ptr)))
#define old_sample_entity_dismiss(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : \
                                             	(((entity_ptr)->init_fp = NULL), \
                                             	((entity_ptr)->exit_fp = NULL), \
                                             	(entity_ptr)))

// Implementation
// => initialization arguments for entity
struct entity_init_arg
{
	// General variables
	// N/A
	// Individual variables which must be private
	union
	{
		// General entity
		struct
		{
			void *arg;
		}gen;

		// Main entity
		struct
		{
			// N/A
		}main;

		// ATI entity
		struct
		{
			map_t urc_tbl;
		}ati;

		// Old sample entity
		struct
		{
			// N/A
		}old_sample;
	}idv;
};

// => Abstract data type
struct entity
{
	// General variables
	// => Public
	entity_ptr_t (*init_fp) (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
	entity_ptr_t (*exit_fp) (entity_ptr_t entity_ptr);
	// => Private
	entity_id_t id;
	// Individual variables which must be private
	union
	{
		// General entity, NEED_TO_BE_NOTICED, no specific implementation but made by developers
		struct
		{
			void *priv;
		}gen;

		// Main entity
		struct
		{
			task_t task;
			chain_t entity_tbl, task_tbl, service_tbl, event_tbl, conn_tbl;
			pool_t addr_node_pool;
			thrd_t thrd;
		}main;

		// ATI entity
		struct
		{
			map_t reg_tbl;
			map_t hijack_tbl;
        }ati;

		// Old sample entity
		struct
		{
			task_t task_list[2];
		}old_sample;
	}idv;
};

// => General entity
// N/A
// => Main entity
extern entity_ptr_t main_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t main_entity_exit (entity_ptr_t entity_ptr);

// => Old sample entity
extern entity_ptr_t old_sample_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t old_sample_entity_exit (entity_ptr_t entity_ptr);

// => MFI entity
extern entity_ptr_t mfi_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t mfi_entity_exit (entity_ptr_t entity_ptr);

// => MFIA entity
extern entity_ptr_t mfia_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t mfia_entity_exit (entity_ptr_t entity_ptr);

// => ATI entity
extern entity_ptr_t ati_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t ati_entity_exit (entity_ptr_t entity_ptr);

// => Host entity
extern entity_ptr_t host_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t host_entity_exit (entity_ptr_t entity_ptr);

// => User entity
extern entity_ptr_t usr_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t usr_entity_exit (entity_ptr_t entity_ptr);

extern mailbox_addr_ptr_t default_task_addr_ptr;
extern task_ptr_t default_task_ptr;
extern pool_t default_event_pool;
extern thrd_lock_t default_event_pool_mutex;
extern pool_t default_conn_pool;
extern thrd_lock_t default_conn_pool_mutex;
extern reg_data_t task_tbl[];
extern thrd_lock_t task_tbl_mutex;
extern reg_data_t srv_tbl[];
extern thrd_lock_t srv_tbl_mutex;
extern subscribe_data_ptr_t event_tbl[];
extern thrd_lock_t event_tbl_mutex;
extern conn_ptr_t conn_tbl[];
// => MFI entity
extern int mfi_sock;
// => MFIA entity
extern int mfia_sock;
// => ATI entity
extern int ati_sock;
extern int ati_urc_sock;

#endif
