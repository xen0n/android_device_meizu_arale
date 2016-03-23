#include <telephony/ril_cdma_sms.h>
#include <cutils/properties.h>
#include <dirent.h>
#include <ctype.h>
#include "atchannel.h"
#include "at_tok.h"
#include "oem-support.h"

#define LOG_TAG "C2K_RIL"
#include <utils/Log.h>

extern int  s_wait_repoll_flag;
extern const struct timeval TIMEVAL_NETWORK_REPOLL;


int get_oem_support_flag()
{
    char    string[PROPERTY_VALUE_MAX];
    char  * ptr = string;
    char ** p_cur = &ptr;
    int     oem_spt_flag = DEFAULT_OEM_SUPPORT_FLAG;

    property_get(OEM_SUPPORT_FLAG_PROP_NAME, string, "");
    if (strcmp(string, "") != 0)
    {
        at_tok_nexthexint(p_cur, &oem_spt_flag);
    }
    LOGD("[CDMA] get_oem_support_flag, 0x%X", oem_spt_flag);

    return oem_spt_flag;
}

int get_oem_ril_version(void)
{
    char    string[PROPERTY_VALUE_MAX];
    char  * ptr = string;
    char ** p_cur = &ptr;
    int     oem_ril_version = DEFAULT_OEM_RIL_VERSION;

    property_get(OEM_RIL_VERSION_PROP_NAME, string, "");
    if (strcmp(string, "") != 0)
    {
        at_tok_nexthexint(p_cur, &oem_ril_version);
    }
    LOGD("[CDMA] get_oem_ril_version, %d", oem_ril_version);

    return oem_ril_version;
}

int oem_support_bit_verify(int oem_support_flag, int oem_support_mask)
{
    return (oem_support_flag & oem_support_mask);
}

int dispatch_oem_rgistration_state(RIL_Rgistration_state *p_input, OEM_RIL_Rgistration_state *p_output)
{
    if (p_input == NULL || p_output == NULL)
    {
        return -1;
    }

    p_output->register_state = p_input->register_state;
    p_output->lac = p_input->lac;
    p_output->cid = p_input->cid;
    p_output->radio_technology = p_input->radio_technology;
    p_output->basestation_id = p_input->basestation_id;
    p_output->basestation_latitude = p_input->basestation_latitude;
    p_output->basestation_longitude = p_input->basestation_longitude;
    p_output->concurrent_service = p_input->concurrent_service;
    p_output->system_id = p_input->system_id; 
    p_output->network_id = p_input->network_id;
    p_output->roaming_indicator = p_input->roaming_indicator;
    p_output->prl_state = p_input->prl_state;
    p_output->prl_roaming_indicator = p_input->prl_roaming_indicator;
    p_output->deny_reason = p_input->deny_reason;

    return 0;
}

int dispatch_oem_card_status(VIA_RIL_CardStatus * p_card_status, RIL_CardStatus_v6 ** pp_oem_card_status)
{
    int i;

    if (p_card_status == NULL || pp_oem_card_status == NULL) {
        return -1;
    }

    RIL_CardStatus_v6 *p_oem_card_status = malloc(sizeof(RIL_CardStatus_v6));
    p_oem_card_status->card_state = p_card_status->card_state;
    p_oem_card_status->universal_pin_state = p_card_status->universal_pin_state;
    p_oem_card_status->gsm_umts_subscription_app_index = p_card_status->gsm_umts_subscription_app_index;
    p_oem_card_status->cdma_subscription_app_index = p_card_status->cdma_subscription_app_index;
    p_oem_card_status->ims_subscription_app_index = p_card_status->cdma_subscription_app_index;
    p_oem_card_status->num_applications = p_card_status->num_applications;

    for (i = 0; i < RIL_CARD_MAX_APPS; i++)
    {
        p_oem_card_status->applications[i].app_type = p_card_status->applications[i].app_type;
        p_oem_card_status->applications[i].app_state = p_card_status->applications[i].app_state;
        p_oem_card_status->applications[i].perso_substate = p_card_status->applications[i].perso_substate;
        p_oem_card_status->applications[i].aid_ptr = p_card_status->applications[i].aid_ptr;
        p_oem_card_status->applications[i].app_label_ptr  = p_card_status->applications[i].app_label_ptr;
        p_oem_card_status->applications[i].pin1_replaced = p_card_status->applications[i].pin1_replaced;
        p_oem_card_status->applications[i].pin1 = p_card_status->applications[i].pin1;
        p_oem_card_status->applications[i].pin2 = p_card_status->applications[i].pin2;
        LOGD("QFU: applications[%d].app_type = %d, applications[%d].app_state = %d"
            , i, p_oem_card_status->applications[i].app_type, i, p_oem_card_status->applications[i].app_state);
    }
    *pp_oem_card_status = p_oem_card_status;

    return 0;
}
