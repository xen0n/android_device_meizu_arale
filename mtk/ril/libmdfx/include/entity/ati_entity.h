#ifndef __ENTITY_ATI_ENTITY_H__
#define __ENTITY_ATI_ENTITY_H__

#include "entity.h"
#include "task.h"
#include "event.h"
#include "ctnr/chain.h"

// Compiler flags, NEED_TO_BE_NOTICED, set by the compiler
// N/A

// Type definitions
typedef struct ati_reg_tbl_entry ati_reg_tbl_entry_t;
typedef struct ati_reg_tbl_entry* ati_reg_tbl_entry_ptr_t;
typedef struct ati_reg_req ati_reg_req_t;
typedef struct ati_reg_req* ati_reg_req_ptr_t;
typedef void* (*ati_atc_hdl_t) (ati_reg_tbl_entry_ptr_t entry_ptr, const char *atc, void *arg);

#define ATI_ATC_PREFIX_MAX_LEN	(32)	

// Variables
extern int mal_mode;   // NEED_TO_BE_NOTICED, where to put?
// => ATI task
extern int ati_sock;
extern int ati_urc_sock;
// => ATI entity
struct ati_reg_tbl_entry
{
    char atc_prefix[ATI_ATC_PREFIX_MAX_LEN];
    event_id_t event_id;
    ati_atc_hdl_t hdl;
    chain_t mailbox_list;
};

struct ati_reg_req
{
    task_id_t task_id;
    event_id_t event_id;
	char atc_prefix[ATI_ATC_PREFIX_MAX_LEN];
};

// Functions
extern entity_ptr_t ati_entity_init (entity_ptr_t entity_ptr, const entity_init_arg_ptr_t arg_ptr);
extern entity_ptr_t ati_entity_exit (entity_ptr_t entity_ptr);

#endif
