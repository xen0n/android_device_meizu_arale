#ifndef __ENTITY_DATI_H_
#define __ENTITY_DATI_H_

#include "entity/datamngr/datamngr.h"

#define DATI_DUMMY_ATC_ERR   (0)
#define DATI_DUMMY_ATC_OK   (1)
#define DATI_DUMMY_NORMAL_URC   (0)
#define DATI_DUMMY_BYPASS_URC   (1)

#define INVAL_ATC	"INVAL_AT_VAL"
#define ATC_TBL_SIZE	4	// NEED_TO_BE_NOTICED, temporarily used, a map imple. would replace this one later
typedef struct dati_atc_tbl_entry dati_atc_tbl_entry_t;
typedef struct dati_atc_tbl_entry*	dati_atc_tbl_entry_ptr_t;
typedef struct dati_atc_hdl_arg dati_atc_hdl_arg_t;
typedef struct dati_atc_hdl_arg* dati_atc_hdl_arg_ptr_t;
typedef void* (*dati_atc_hdl_t) (dati_atc_tbl_entry_ptr_t entry_ptr, char* atc, void* arg);

typedef struct dati_atc_tbl_entry
{
	const char *cmd;
	int num_of_param;
	const char * const *param;
	dati_atc_hdl_t assign_hdl;
	dati_atc_hdl_t query_hdl;
} dati_atc_tbl_entry_t;

typedef struct dati_pdn_tbl_entry
{
	int state;
    int cid;
	dm_req_setup_data_call_t dm_req_setup_data_call;
	dm_req_deactivate_data_call_t dm_req_deactivate_data_call;
}dati_pdn_tbl_entry_t;

typedef struct dati_atc_hdl_arg
{
	event_ptr_t event_ptr;
	// AT+CGDCONT; AT+CGACT; AT+CGDATA
	dati_pdn_tbl_entry_t pdn_tbl[DM_MAX_PDN_NUM];
    thrd_lock_t dati_pdn_tbl_mutex;
} dati_atc_hdl_arg_t;

extern struct dati_atc_tbl_entry atc_tbl[];
extern void* datamngr_dati_task_hdl (task_ptr_t task_ptr, event_ptr_t event_ptr, void *arg);

// Data manager handler
extern int dati_dummy_resp (void *resp, event_t *event, int res_type);
extern int dati_dummy_urc (void *resp, char* urc, int res_type);
extern void* dati_cfun_hdl (dati_atc_tbl_entry_ptr_t entry_ptr, char* atc, void *arg);
extern void* dati_cgdcont_hdl (dati_atc_tbl_entry_ptr_t entry_ptr, char* atc, void *arg);
extern void* dati_cgact_hdl (dati_atc_tbl_entry_ptr_t entry_ptr, char* atc, void *arg);
extern void* dati_cgdata_hdl (dati_atc_tbl_entry_ptr_t entry_ptr, char* atc, void *arg);
extern int dati_setup_data_call_rep (dm_resp_setup_data_call_t *resp, event_t *event);
extern int dati_setup_data_call_rep (dm_resp_setup_data_call_t *resp, event_t *event);
extern int dati_setup_data_call_resp (dm_resp_setup_data_call_t *resp, event_t *event);
extern int dati_deactivate_data_call_resp (dm_resp_deactivate_data_call_t *resp, event_t *event);
extern int dati_deactivate_data_call_if_resp (dm_resp_deactivate_data_call_t *resp, event_t *event);
extern int dati_setup_dedicate_data_call_resp (dm_resp_setup_dedicate_data_call_t *resp, event_t *event);
extern int dati_modify_data_call_resp (dm_resp_modify_data_call_t *resp, event_t *event);
extern int dati_deactivate_dedicate_data_call_resp (dm_resp_deactivate_dedicate_data_call_t *resp, event_t *event);
extern int dati_abort_setup_data_call_resp (dm_resp_abort_setup_data_call_t *resp, event_t *event);
extern int dati_pcscf_discovery_pco_resp (dm_resp_pcscf_discovery_pco_t *resp, event_t *event);
extern int dati_last_data_call_fail_cause_resp (dm_resp_last_data_call_fail_cause_t *resp, event_t *event);
extern int dati_data_call_list_resp (dm_resp_data_call_list_t *resp, event_t *event);
extern int dati_data_call_list_changed_urc (dm_urc_data_call_list_changed_t *urc, event_t *event);
extern int dati_dedicate_bearer_activated_urc (dm_urc_dedicate_bearer_activated_t *urc, event_t *event);
extern int dati_dedicate_bearer_modified_urc (dm_urc_dedicate_bearer_modified_t *urc, event_t *event);
extern int dati_dedicate_bearer_deactivated_urc (dm_urc_dedicate_bearer_deactivated_t *urc, event_t *event);
extern int dati_fail_resp (dm_resp_t *resp, event_t *event);
extern int dati_radio_power_resp(dm_resp_radio_power_t *resp, event_t *event);
extern int dati_allow_data_resp(dm_resp_allow_data_t *resp, event_t *event);

#endif
