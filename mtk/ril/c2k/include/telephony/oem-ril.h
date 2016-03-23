#ifndef OEM_RIL_H
#define OEM_RIL_H

#include "ril.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	char * register_state;
	char *lac;
	char * cid;
	char * radio_technology;
	char * basestation_id;
	char * basestation_latitude;
	char * basestation_longitude;
	char * concurrent_service;
	char * system_id;
	char * network_id;
	char * roaming_indicator;
	char * prl_state;
	char * prl_roaming_indicator;
	char * deny_reason;
}OEM_RIL_Rgistration_state;

typedef struct {
    char * pdp_cid;
    char * name;
    char * ip;
} OEM_RIL_Data_call_resp;

#ifdef __cplusplus
}
#endif

#endif
