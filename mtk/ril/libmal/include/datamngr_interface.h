#ifndef DATAMNGR_IF_H
#define DATAMNGR_IF_H

#include "mdfx.h"

typedef int (*dm_data_call_list_change_notify_fn)(dm_urc_data_call_list_changed_t *notify, void *priv);
typedef int (*dm_data_call_list_notify_fn)(dm_resp_setup_data_call_t *notify, void *priv);
typedef int (*dm_dedicated_bearer_act_notify_fn)(dm_urc_dedicate_bearer_activated_t *notify, void *priv);
typedef int (*dm_dedicated_bearer_modify_notify_fn)(dm_urc_dedicate_bearer_modified_t *notify, void *priv);
typedef int (*dm_dedicated_bearer_deact_notify_fn)(dm_urc_dedicate_bearer_deactivated_t *notify, void *priv);
typedef int (*dm_dedicated_data_call_notify_fn)(dm_resp_setup_dedicate_data_call_t *notify, void *priv);
typedef int (*dm_handover_notify_fn)(dm_ho_status_t *notify, void *priv);

typedef struct _mal_dm_notify_function{
	dm_data_call_list_change_notify_fn          data_call_list_change;
    dm_data_call_list_notify_fn                 data_call_list;
    dm_dedicated_bearer_act_notify_fn           dedicated_bearer_act;
    dm_dedicated_bearer_modify_notify_fn        dedicated_bearer_modify;
    dm_dedicated_bearer_deact_notify_fn         dedicated_bearer_deact;
    dm_dedicated_data_call_notify_fn            dedicated_data_call;
    dm_handover_notify_fn   handover_notify;
} mal_dm_notify_function_t;

extern int mal_dm_register(mal_dm_notify_function_t *callback_fn);


extern mal_dm_notify_function_t mal_dm_notify;

#endif
