/*
    Here is an interface for events to hide implementation. 
    An event is identified by unique source address and a per-source sequence number. 
    The hop-by-hop forwarding path sbould be traced. 
    On the other hand, an event carries an ID and arbitrary user data. 
    The TLV data format may be used to various number of arguments in a single event. 

    MAL events are required to use mailboxes inside MAL and channels outside MAL. 
    Therefore, the addressing information would be recorded for both of them, respectively. 
    For data and event_addr, a setting operation duplicates a copy while a getting operation retrieve a pointer to the one in the event. 
    Moreover, MAL limits the header files which can be modified by users, so that the exact event-ID definitions are specified in "sys_info.h". 
*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include "sys_info.h"
#include "compiler.h"
#include "io/mailbox.h"
#include "io/chnl.h"
#include "ctnr/chain.h"
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>  // NEED_TO_BE_NOTICED, temporarily methods to monitor memory usage

// Compiler flags, NEED_TO_BE NOTICED, set by compiler
#define MAL_SUPPORT

// Type definitions
typedef struct event event_t;
typedef struct event* event_ptr_t;
typedef struct event** event_ptr2_t;
typedef struct event_addr event_addr_t;
typedef struct event_addr* event_addr_ptr_t;
typedef struct event_addr** event_addr_ptr2_t;
typedef int event_seq_t;
typedef enum event_id event_id_t;
typedef void (*event_hook_fp_t) (event_addr_ptr_t addr_ptr, va_list vl);
typedef enum event_addr_ext_type {EVENT_ADDR_EXT_TYPE_NONE, EVENT_ADDR_EXT_TYPE_CHNL, EVENT_ADDR_EXT_TYPE_MAILBOX} event_addr_ext_type_t;

// Macros
#define EVENT_DATA_INIT_SIZE    (sizeof(int))    // NEED_TO_BE_NOTICED, we assume that an integer is the most common data to be stored
#define EVENT_INVALID_SEQ    (0)
#ifdef MAL_SUPPORT
// => MAL
#define EVENT_UNSOLICITED    (-1)
#endif
#define EVENT_USAGE_THRESHOLD   (1 << 20)  // NEED_TO_BE_NOTICED, temporarily methods to monitor memory usage

// Implementation
// => Internal data type for the event
struct event_addr
{
    // General variables
    event_seq_t seq;
    // Individual variables which must be private
#ifdef MAL_SUPPORT
    // MAL
    // => Inside-MAL address
    mailbox_addr_t src_addr;
    mailbox_addr_t dst_addr;
    // => Outside-MAL address
    event_addr_ext_type_t ext_type;
    chnl_t ext_chnl;
    mailbox_t ext_mailbox;
    event_seq_t ext_seq;
#endif
};

// => Abstract Data Type
struct event
{
    // General variables
    // => Public
    event_id_t id;
    void *data;
    size_t data_len;
    size_t data_size;
    // => Private
    chain_t addr_track;
//    chain_t scatter_list;    // NEED_TO_BE_NOTICED, to do
    // Individual variables which must be private
#ifdef __EVENT_USAGE_MONITOR__
    pid_t tid;  // NEED_TO_BE_NOTICED, temporarily methods to monitor memory usage
#endif

#ifdef MAL_SUPPORT
    // N/A
#endif
};

// => Functions
extern event_ptr_t event_init (event_ptr_t event_ptr);
extern event_ptr_t event_exit (event_ptr_t event_ptr);
// => => Event ID
extern event_id_t event_set_id (event_ptr_t event_ptr, event_id_t id);
extern event_id_t event_get_id (event_ptr_t event_ptr);
// => => Event data
extern void* event_set_data (event_ptr_t event_ptr, const void *data, size_t data_len);
extern void* event_get_data (event_ptr_t event_ptr, size_t *data_len_ptr);
extern size_t event_set_data_len (event_ptr_t event_ptr, size_t data_len);    // NEED_TO_BE_NOTICED, a dangerous operation, be careful
extern size_t event_get_data_len (event_ptr_t event_ptr);    // NEED_TO_BE_NOTICED, a dangerous operation, be careful
extern size_t event_set_data_size (event_ptr_t event_ptr, size_t data_size);    // NEED_TO_BE_NOTICED, a dangerous operation, be careful
extern size_t event_get_data_size (event_ptr_t event_ptr);    // NEED_TO_BE_NOTICED, a dangerous operation, be careful
// => => Event address
extern event_addr_ptr_t event_set_addr (event_ptr_t event_ptr, event_addr_ptr_t addr_ptr);
extern int event_reset_addr (event_ptr_t event_ptr);
extern event_addr_ptr_t event_get_first_addr (event_ptr_t event_ptr);
extern event_addr_ptr_t event_get_last_addr (event_ptr_t event_ptr);
extern int event_reset_last_addr (event_ptr_t event_ptr);
extern int event_iterate_addr_track (event_ptr_t event_ptr, event_hook_fp_t hook_fp, ...);
// => => => Address operations
extern event_seq_t event_addr_set_seq (event_addr_ptr_t addr_ptr, event_seq_t seq);
extern event_seq_t event_addr_get_seq (event_addr_ptr_t addr_ptr);

#ifdef MAL_SUPPORT
// => => MAL-specific
// => => => Address operations, NEEDTO_BE_NOTICED, certain functions will be implemented on demand
extern mailbox_addr_ptr_t event_addr_set_src_addr (event_addr_ptr_t addr_ptr, const mailbox_addr_ptr_t src_addr_ptr);
extern mailbox_addr_ptr_t event_addr_get_src_addr (event_addr_ptr_t addr_ptr);
extern mailbox_addr_ptr_t event_addr_set_dst_addr (event_addr_ptr_t addr_ptr, const mailbox_addr_ptr_t dst_addr_ptr);
extern mailbox_addr_ptr_t event_addr_get_dst_addr (event_addr_ptr_t addr_ptr);
extern event_addr_ext_type_t event_addr_get_ext_type (event_addr_ptr_t addr_ptr);
extern chnl_ptr_t event_addr_set_ext_chnl (event_addr_ptr_t addr_ptr, const chnl_ptr_t ext_chnl_ptr);
extern chnl_ptr_t event_addr_get_ext_chnl (event_addr_ptr_t addr_ptr);
extern mailbox_ptr_t event_addr_set_ext_mailbox (event_addr_ptr_t addr_ptr, const mailbox_ptr_t ext_mailbox_ptr);
extern mailbox_ptr_t event_addr_get_ext_mailbox (event_addr_ptr_t addr_ptr);
extern event_seq_t event_addr_set_ext_seq (event_addr_ptr_t addr_ptr, event_seq_t seq);
extern event_seq_t event_addr_get_ext_seq (event_addr_ptr_t addr_ptr);
extern event_addr_ptr_t event_addr_cpy (event_addr_ptr_t dst_addr_ptr, const event_addr_ptr_t src_addr_ptr);
#endif

#endif
