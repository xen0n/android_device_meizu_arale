/*
	Here is an interface of the MTK modem framework to hide implementation. 
*/

#ifndef __MDFX_H__
#define __MDFX_H__

#include "sys_info.h"
#include "compiler.h"
#include "io/std_io.h"
#include "mem/mem_alloc.h"
#include "thrd.h"
#include "thrd/thrd_lock.h"
#include "ctnr/pool.h"
#include "ctnr/queue.h"
#include "ctnr/map.h"
#include "tmr.h"
#include "event.h"

#include "io/mailbox.h"
#include "entity.h"
#include "task.h"
#include "srv.h"
#include "conn.h"

#include <stdarg.h>
#include <stddef.h>

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// N/A

// Type definitions
typedef int context_id_t;

// Macros
#define MDFX_MFI_CHNL	MFI_TASK_PATH
#define CONTEXT_INVAL_ID	TASK_INVAL_ID

// Global variables
// N/A

// Functions
extern void mdfx_mfia_start (void);
extern void mdfx_wait_all_entity_init (void);
context_id_t mdfx_get_id_by_addr (mailbox_addr_ptr_t addr_ptr);
// => Logging
#define mdfx_print(tag, format, args ...)	(lv_print((tag), format, ## args))

// => Memory
#define mdfx_mem_alloc(size)	(mem_alloc((size)))
#define mdfx_mem_realloc(ptr, size)	(mem_realloc((ptr), (size)))
#define mdfx_mem_free(ptr)	(mem_free((ptr)))

// => Thread
extern thrd_ptr_t mdfx_thrd_init (thrd_ptr_t thrd_ptr);
extern thrd_ptr_t mdfx_thrd_exit (thrd_ptr_t thrd_ptr);
#define mdfx_thrd_exec(thrd_ptr, hdl, arg)	(thrd_exec((thrd_ptr), (hdl), (arg)))
// => Thread lock
// => => Mutex
extern thrd_lock_ptr_t mdfx_mutex_init (thrd_lock_ptr_t thrd_lock_ptr);
extern thrd_lock_ptr_t mdfx_mutex_exit (thrd_lock_ptr_t thrd_lock_ptr);
#define mdfx_mutex_lock(thrd_lock_ptr)	(thrd_lock_on((thrd_lock_ptr)))
#define mdfx_mutex_unlock(thrd_lock_ptr)	(thrd_lock_off((thrd_lock_ptr)))
// => => Semaphore, e.g. capacity = 1, avail_rsc = 0 for a binary semaphore
extern thrd_lock_ptr_t mdfx_sem_init (thrd_lock_ptr_t thrd_lock_ptr, size_t capacity, ssize_t avail_rsc);
extern thrd_lock_ptr_t mdfx_sem_exit (thrd_lock_ptr_t thrd_lock_ptr);
#define mdfx_sem_wait(thrd_lock_ptr)	(thrd_lock_sem_wait((thrd_lock_ptr)))
#define mdfx_sem_signal(thrd_lock_ptr)	(thrd_lock_sem_signal((thrd_lock_ptr)))
// => => Producer-consumer model
extern thrd_lock_ptr_t mdfx_pcm_init (thrd_lock_ptr_t thrd_lock_ptr, size_t capacity);
extern thrd_lock_ptr_t mdfx_pcm_exit (thrd_lock_ptr_t thrd_lock_ptr);
#define mdfx_pcm_plock(thrd_lock_ptr)	(thrd_lock_prod_on((thrd_lock_ptr)))
#define mdfx_pcm_punlock(thrd_lock_ptr, num_of_done)	(thrd_lock_prod_off((thrd_lock_ptr), (num_of_done)))
#define mdfx_pcm_clock(thrd_lock_ptr)	(thrd_lock_cons_on((thrd_lock_ptr)))
#define mdfx_pcm_cunlock(thrd_lock_ptr, num_of_done)	(thrd_lock_cons_off((thrd_lock_ptr), (num_of_done)))

// => Container
// => => Pool
extern pool_ptr_t mdfx_pool_init (pool_ptr_t pool_ptr, const pool_init_arg_ptr_t arg_ptr);
extern pool_ptr_t mdfx_pool_exit (pool_ptr_t pool_ptr);
#define mdfx_pool_borrow(pool_ptr)	(pool_borrow((pool_ptr)))
#define mdfx_pool_return(pool_ptr, block)	(pool_return((pool_ptr), (block)))
// => => Queue
extern queue_ptr_t mdfx_queue_init (queue_ptr_t queue_ptr, const queue_init_arg_ptr_t arg_ptr);
extern queue_ptr_t mdfx_queue_exit (queue_ptr_t queue_ptr);
#define mdfx_queue_enqueue(queue_ptr, val, val_len)	(queue_enqueue((queue_ptr), (val), (val_len)))
#define mdfx_queue_dequeue(queue_ptr, val, val_size)	(queue_dequeue((queue_ptr), (val), (val_size)))
#define mdfx_queue_iterate(queue_ptr, hook_fp, args ...)	(queue_iterate((queue_ptr), (hook_fp), ## args))
#define mdfx_queue_is_empty(queue_ptr)	(queue_is_empty((queue_ptr)))
// => => Map
extern map_ptr_t mdfx_map_init (map_ptr_t map_ptr, const map_init_arg_ptr_t arg_ptr);
extern map_ptr_t mdfx_map_exit (map_ptr_t map_ptr);
#define mdfx_map_insert(map_ptr, key, key_len, val, val_len)	(map_insert((map_ptr), (key), (key_len), (val), (val_len)))
#define mdfx_map_update(map_ptr, key, key_len, val, val_len)	(map_update((map_ptr), (key), (key_len), (val), (val_len)))
#define mdfx_map_search(map_ptr, key, key_len, val_len_ptr)	(map_search((map_ptr), (key), (key_len), (val_len_ptr)))
#define mdfx_map_delete(map_ptr, key, key_len)	(map_delete((map_ptr), (key), (key_len)))
#define mdfx_map_iterate(map_ptr, hook_fp, args ...)	(map_iterate((map_ptr), (hook_fp), ## args))
#define mdfx_map_is_empty(map_ptr)	(map_is_empty((map_ptr)))

// => Timer
extern tmr_ptr_t mdfx_tmr_init (tmr_ptr_t tmr_ptr, const tmr_init_arg_ptr_t arg_ptr);
extern tmr_ptr_t mdfx_tmr_exit (tmr_ptr_t tmr_ptr);
extern int mdfx_tmr_arm (tmr_ptr_t tmr_ptr, tmr_time_ptr_t time_ptr, event_ptr_t event_ptr, context_id_t context_id);

// => Event
#define mdfx_event_init(event_ptr)	(event_init((event_ptr)))
#define mdfx_event_exit(event_ptr)	(event_exit((event_ptr)))
extern event_ptr_t mdfx_event_alloc (const event_ptr_t rel_event_ptr);
extern int mdfx_event_free (event_ptr_t event_ptr);
extern int mdfx_event_dump (event_ptr_t event_ptr);
// => => Event ID
#define mdfx_event_set_id(event_ptr, id)	(event_set_id((event_ptr), (id)))
#define mdfx_event_get_id(event_ptr)	(event_get_id((event_ptr)))
// => => Event Data
#define mdfx_event_set_data(event_ptr, data, data_len)	(event_set_data((event_ptr), (data), (data_len)))
#define mdfx_event_get_data(event_ptr, data_len_ptr)	(event_get_data((event_ptr), (data_len_ptr)))
#define mdfx_event_get_data_len(event_ptr)	(event_get_data_len((event_ptr)))
// => => Event address
extern context_id_t mdfx_event_get_src_id (event_ptr_t event_ptr);
extern context_id_t mdfx_event_get_ext_id (event_ptr_t event_ptr);
extern context_id_t mdfx_event_get_xsrc_id (event_ptr_t event_ptr);
// ==== NEED_TO_BE_NOTICED, legacy event APIs, higjly NOT recommended to use ====
extern bool mdfx_event_cmp_chnl(event_ptr_t event_ptr_1, event_ptr_t event_ptr_2);  // NEED_TO_BE_NOTICED, requested by Box Wu
// ==== NEED_TO_BE_NOTICED, legacy event APIs, higjly NOT recommended to use ====

// => Task API
extern task_ptr_t mdfx_task_init (task_ptr_t task_ptr, entity_id_t entity_id, task_id_t task_id);
extern task_ptr_t mdfx_task_exit (task_ptr_t task_ptr);
#define mdfx_task_exec(task_ptr, hdl, arg)	(task_exec((task_ptr), (hdl), (arg)))
#define mdfx_task_get_id(task_ptr)	(task_get_id((task_ptr)))
extern int mdfx_task_subscribe_event (task_ptr_t task_ptr, event_id_t event_id);
extern int mdfx_task_subscribe_atc (task_ptr_t task_ptr, const char *atc_prefix, event_id_t event_id);
extern int mdfx_task_hijack_atc (task_ptr_t task_ptr, const char *atc_prefix, event_id_t event_id);
extern int mdfx_task_send_event (task_ptr_t task_ptr, event_ptr_t event_ptr, task_id_t task_id);
extern int mdfx_task_reply_event (task_ptr_t task_ptr, event_ptr_t orig_event_ptr, event_ptr_t event_ptr);
extern int mdfx_task_bcast_event (task_ptr_t task_ptr, event_ptr_t event_ptr);
extern int mdfx_task_send_recv_event (task_ptr_t task_ptr, event_ptr_t event_ptr, task_id_t task_id, task_rx_cond_t rx_cond, void *rx_cond_arg);
extern int mdfx_task_send_ext_event (task_ptr_t task_ptr, event_ptr_t event_ptr, ...);
extern int mdfx_task_bcast_ext_event (task_ptr_t task_ptr, event_ptr_t event_ptr, ...);
// ==== NEED_TO_BE_NOTICED, legacy task APIs, higjly NOT recommended to use ====
extern bool rx_cond_event_id (task_ptr_t task_ptr,event_ptr_t event_ptr,void * arg);    // NEED_TO_BE_NOTICED, requested by Box Wu
#define mdfx_task_send_and_wait_for_event(task_ptr, event_ptr, task_id, event_id)	(mdfx_task_send_recv_event((task_ptr), (event_ptr), (task_id), rx_cond_event_id, (void*)(event_id)))    /* NEED_TO_BE_NOTICED, requested by Box Wu */
extern event_ptr_t mdfx_task_recv_event (task_ptr_t task_ptr, task_rx_cond_t rx_cond, void *rx_cond_arg);   // NEED_TO_BE_NOTICED, requested by Ian Cheng
#define mdfx_task_wait_for_event(task_ptr, event_id)	(mdfx_task_recv_event((task_ptr), rx_cond_event_id, (void*)(event_id))) /* NEED_TO_BE_NOTICED, requested by Ian Cheng */
// ==== NEED_TO_BE_NOTICED, legacy task APIs, higjly NOT recommended to use ====

// => Entity API
#define mdfx_entity_set_id(entity_ptr, entity_id)	(unlikely((entity_ptr) == NULL) ? ENTITY_INVAL_ID : ((entity_ptr)->id = (entity_id)))
#define mdfx_entity_get_id(entity_ptr)	(unlikely((entity_ptr) == NULL) ? ENTITY_INVAL_ID : (entity_ptr)->id)
#define mdfx_entity_set_priv(entity_ptr, priv_data)	(unlikely((entity_ptr) == NULL) ? NULL : ((entity_ptr)->idv.gen.priv = (priv_data)))
#define mdfx_entity_get_priv(entity_ptr)	(unlikely((entity_ptr) == NULL) ? NULL : (entity_ptr)->idv.gen.priv)

// => => MFI task
extern task_ptr_t mdfx_mfi_task_init (task_ptr_t task_ptr, entity_id_t entity_id, task_id_t task_id);
extern task_ptr_t mdfx_mfi_task_exit (task_ptr_t task_ptr);
extern task_ptr_t mdfx_mfia_task_init (task_ptr_t task_ptr, entity_id_t entity_id, task_id_t task_id);
extern task_ptr_t mdfx_mfia_task_exit (task_ptr_t task_ptr);
extern task_ptr_t mdfx_ati_task_init (task_ptr_t task_ptr, entity_id_t entity_id, task_id_t task_id);
extern task_ptr_t mdfx_ati_task_exit (task_ptr_t task_ptr);

extern int mdfx_mfi_task_send_ext_event (task_ptr_t task_ptr, event_ptr_t event_ptr);
extern int mdfx_mfia_task_send_ext_event (task_ptr_t task_ptr, event_ptr_t event_ptr);
// => Service API
extern srv_ptr_t mdfx_srv_init (srv_ptr_t srv_ptr, entity_id_t entity_id, task_id_t task_id);
extern srv_ptr_t mdfx_srv_exit (srv_ptr_t srv_ptr);
#define mdfx_srv_exec(srv_ptr, hdl, arg)	(srv_exec((srv_ptr), (hdl), (arg)))
#define mdfx_srv_get_id(srv_ptr)	(unlikely((srv_ptr) == NULL) ? SRV_INVAL_ID : (srv_ptr)->id)
extern int mdfx_srv_subscribe_event (srv_ptr_t srv_ptr, event_id_t event_id);
int mdfx_srv_send_event (srv_ptr_t srv_ptr, event_ptr_t event_ptr, task_id_t srv_id);
event_ptr_t mdfx_srv_recv_event (srv_ptr_t srv_ptr);
int mdfx_srv_reply_event (srv_ptr_t srv_ptr, event_ptr_t orig_event_ptr, event_ptr_t event_ptr);
extern event_ptr_t mdfx_srv_send_recv_event (srv_ptr_t srv_ptr, event_ptr_t event_ptr, task_id_t task_id, srv_rx_cond_t rx_cond, void *rx_cond_arg);
extern int mdfx_srv_send_ext_event (srv_ptr_t srv_ptr, event_ptr_t event_ptr);
#if 0
// => Event API
extern event_ptr_t mdfx_event_alloc(event_ptr_t rel_event_ptr);
extern int mdfx_event_free (event_ptr_t event_ptr);
#define mdfx_event_set_id(event_ptr, id)	(event_set_id((event_ptr), (id)))
#define mdfx_event_get_id(event_ptr)	(event_get_id((event_ptr)))
#define mdfx_event_set_data(event_ptr, data, data_len)	(event_set_data((event_ptr), (data), (data_len)))
#define mdfx_event_get_data(event_ptr, data_len_ptr)	(event_get_data((event_ptr), (data_len_ptr)))
#define mdfx_event_get_data_len(event_ptr)	(event_get_data_len((event_ptr)))
#define mdfx_event_set_unsolicited(event_ptr)	(event_set_unsolicited((event_ptr)))
#define mdfx_event_set_chnl(event_ptr, chnl_ptr)	(event_set_chnl((event_ptr), (chnl_ptr)))
#define mdfx_event_get_chnl(event_ptr, chnl_ptr)	(event_get_chnl((event_ptr), (chnl_ptr)))
extern bool mdfx_event_cmp_chnl(event_ptr_t event_ptr_1, event_ptr_t event_ptr_2);
#endif
// => Connection API
extern int mdfx_chnl_send (chnl_ptr_t chnl_ptr, event_ptr_t event_ptr);
extern conn_ptr_t mdfx_conn_alloc (void);
extern int mdfx_conn_free (conn_ptr_t conn_ptr);
extern conn_ptr_t mdfx_conn_init (conn_ptr_t conn_ptr, const conn_init_arg_ptr_t arg_ptr);
extern conn_ptr_t mdfx_conn_exit (conn_ptr_t conn_ptr);
extern int mdfx_conn_send (conn_ptr_t conn_ptr, event_ptr_t event_ptr);
extern int mdfx_conn_recv (conn_ptr_t conn_ptr, event_ptr_t event_ptr);
extern event_ptr_t mdfx_conn_send_recv (conn_ptr_t conn_ptr, event_ptr_t event_ptr);
extern event_ptr_t mdfx_conn_send_cb (conn_ptr_t conn_ptr, event_ptr_t event_ptr, rx_cond_hdl_t rx_cond, void *rx_cond_arg, conn_hdl_t cb_hdl);
#define mdfx_conn_get_id(conn_ptr)   (unlikely((conn_ptr) == NULL) ? CONTEXT_INVAL_ID : (conn_ptr)->->chnl.idv.uds.sock + SRV_MAX_ID)
#endif
