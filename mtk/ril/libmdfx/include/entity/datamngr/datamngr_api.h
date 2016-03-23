#ifndef DATAMNGR_API_H
#define DATAMNGR_API_H
#include "mdfx.h"
#include "datamngr_event.h"

extern conn_ptr_t mal_dm_init(conn_init_arg_t *arg);
extern int mal_dm_exit (conn_ptr_t conn_ptr);
extern int mal_dm_notification(dm_req_notification_t *req, conn_ptr_t conn_ptr);
extern int mal_dm_setup_data_call(dm_req_setup_data_call_t *req, dm_resp_setup_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_deactivate_data_call(dm_req_deactivate_data_call_t *req, dm_resp_deactivate_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_setup_dedicate_data_call(dm_req_setup_dedicate_data_call_t *req, dm_resp_setup_dedicate_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_modify_data_call(dm_req_modify_data_call_t *req, dm_resp_modify_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_deactivate_dedicate_data_call(dm_req_deactivate_dedicate_data_call_t *req, dm_resp_deactivate_dedicate_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_abort_setup_data_call(dm_req_abort_setup_data_call_t *req, dm_resp_abort_setup_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_pcscf_discovery_pco_req(dm_req_pcscf_discovery_pco_t *req, dm_resp_pcscf_discovery_pco_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_last_data_call_fail_cause(dm_req_last_data_call_fail_cause_t *req, dm_resp_last_data_call_fail_cause_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_data_call_list(dm_req_data_call_list_t *req, dm_resp_data_call_list_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_if_status(dm_req_if_status_t *req, dm_resp_if_status_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_if_rat_cell_info(dm_if_rat_cell_info_req_t *req, dm_if_rat_cell_info_resp_t *resp, conn_ptr_t conn_ptr);
extern int mal_dm_is_default_bearer(int cid, conn_ptr_t conn_ptr);
extern int mal_dm_pass_ims_pdn_info(dm_req_setup_data_call_t *req, conn_ptr_t conn_ptr);
extern int mal_dm_get_ims_pdn_info(dm_req_if_t *req, dm_resp_setup_data_call_t *resp, conn_ptr_t conn_ptr);
extern int mal_get_ims_para_info(int interface_id, dm_ims_para_info_t *ims_para);
extern int mal_dm_get_bearer(int cid, dm_resp_bearer_t *resp, conn_ptr_t conn_ptr);



#ifdef APN_TABLE_SUPPORT
extern int mal_dm_apn_edit(dm_req_apn_edit_t *req, conn_ptr_t conn_ptr);
extern int mal_dm_apn_edit_para(dm_req_apn_edit_para_t *req, conn_ptr_t conn_ptr);
extern int mal_dm_apn_set_mode(dm_req_apn_set_mode_t *req, conn_ptr_t conn_ptr);
#endif

#endif

