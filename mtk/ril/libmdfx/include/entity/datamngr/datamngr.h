#ifndef DATAMNGR_H
#define DATAMNGR_H
#include "mdfx.h"
#include "pdn_info.h"
#include "apn_table.h"
#include "datamngr_def.h"
#include "datamngr_debug.h"
#include "datamngr_util.h"
#include "datamngr_event.h"
#include "datamngr_op_handle.h"
#include "dati_hdl.h"


extern int dm_forge_urc_hdl (char *cmd, event_t *event);
extern int dm_notification(dm_req_notification_t *req, event_t *event);
extern int dm_if_status_req(dm_req_if_status_t *req, event_t *event);
extern int dm_set_initial_attach_apn_req(dm_req_set_initial_attach_apn_t *req, event_t *event);
extern int dm_set_initial_attach_apn_resp(dm_resp_set_initial_attach_apn_t *resp, event_t *event);
extern int dm_setup_data_call_req(dm_req_setup_data_call_t *req, event_t *event);
extern int dm_setup_data_call_resp(dm_resp_setup_data_call_t *resp, event_t *event);
extern int dm_setup_data_call_fail_ind(dm_resp_t *resp, event_t *event);
extern int dm_deactivate_data_call_req(dm_req_deactivate_data_call_t *req, event_t *event);
extern int dm_deactivate_data_call_resp(dm_resp_deactivate_data_call_t *resp, event_t *event);
extern int dm_deactivate_data_call_if_req(dm_req_deactivate_data_call_t *req, event_t *event);
extern int dm_deactivate_data_call_if_with_cause_req(dm_req_deactivate_data_call_t *req, event_t *event, int cause);
extern int dm_deactivate_data_call_if_resp(dm_resp_deactivate_data_call_t *resp, event_t *event);
extern int dm_setup_dedicate_data_call_req(dm_req_setup_dedicate_data_call_t *req, event_t *event);
extern int dm_setup_dedicate_data_call_resp(dm_resp_setup_dedicate_data_call_t *resp, event_t *event);
extern int dm_modify_data_call_req(dm_req_modify_data_call_t *req, event_t *event);
extern int dm_modify_data_call_resp(dm_resp_modify_data_call_t *resp, event_t *event);
extern int dm_deactivate_dedicate_data_call_req(dm_req_deactivate_dedicate_data_call_t *req, event_t *event);
extern int dm_deactivate_dedicate_data_call_resp(dm_resp_deactivate_dedicate_data_call_t *resp, event_t *event);
extern int dm_abort_setup_data_call_req(dm_req_abort_setup_data_call_t *req, event_t *event);
extern int dm_abort_setup_data_call_resp(dm_resp_abort_setup_data_call_t *resp, event_t *event);
extern int dm_pcscf_discovery_pco_req(dm_req_pcscf_discovery_pco_t *req, event_t *event);
extern int dm_pcscf_discovery_pco_resp(dm_resp_pcscf_discovery_pco_t *resp, event_t *event);
extern int dm_last_data_call_fail_cause_req(dm_req_last_data_call_fail_cause_t *req, event_t *event);
extern int dm_last_data_call_fail_cause_resp(dm_resp_last_data_call_fail_cause_t *resp, event_t *event);
extern int dm_data_call_list_req(dm_req_data_call_list_t *req, event_t *event);
extern int dm_data_call_list_resp(dm_resp_data_call_list_t *resp, event_t *event);
extern int dm_data_call_list_changed_urc(dm_urc_data_call_list_changed_t *urc, event_t *event);
extern int dm_default_bearer_modified_urc(dm_urc_default_bearer_modified_t *urc, event_t *event);
extern int dm_dedicate_bearer_activated_urc(dm_urc_dedicate_bearer_activated_t *urc, event_t *event);
extern int dm_dedicate_bearer_modified_urc(dm_urc_dedicate_bearer_modified_t *urc, event_t *event);
extern int dm_dedicate_bearer_deactivated_urc(dm_urc_dedicate_bearer_deactivated_t *urc, event_t *event);
extern int dm_fail_resp(dm_resp_t *resp, event_t *event);
#ifdef APN_TABLE_SUPPORT
extern int dm_apn_edit_req(dm_req_apn_edit_t *req, event_t *event);
extern int dm_apn_edit_para_req(dm_req_apn_edit_para_t *req, event_t *event);
extern int dm_apn_set_mode_req(dm_req_apn_set_mode_t *req, event_t *event);
extern int dm_apn_reset_req(int reset, event_t *event);
#endif
extern int dm_radio_power_req(dm_req_radio_power_t *req, event_t *event);
extern int dm_radio_power_resp(dm_resp_radio_power_t *resp, event_t *event);
extern int dm_radio_power_do_req(dm_req_radio_power_t *req, event_t *event);
extern int dm_allow_data_req(dm_req_allow_data_t *req, event_t *event);
extern int dm_allow_data_resp(dm_resp_allow_data_t *resp, event_t *event);
extern int dm_netlink_listen_req(dm_req_netlink_t *req, event_t *event);
extern int dm_bear_req(dm_req_bearer_t *req, event_t *event);               
extern int dm_pass_ims_pdn_req(dm_req_setup_data_call_t *req, event_t *event);              
extern int dm_get_ims_pdn_req(dm_req_if_t *req, event_t *event);
extern int dm_get_ims_para_req(dm_req_ims_para_info_t *req, event_t *event);
extern int dm_if_rat_cell_info_req(dm_if_rat_cell_info_req_t *req, event_t *event);
extern int dm_handover_req(dm_ho_status_t *req, event_t *event);
extern int dm_handover_resp(dm_resp_ho_t *resp, event_t *event);
extern int dm_notify_epdg_deactivate(dm_req_deactivate_data_call_t *req, event_t *event);







#endif

