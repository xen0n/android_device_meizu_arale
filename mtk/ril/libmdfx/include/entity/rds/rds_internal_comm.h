#ifndef __RDS_INTERNAL_COMM_H__
#define __RDS_INTERNAL_COMM_H__

#include "rds_if.h"
#include "mdfx.h"

#define RDS_EA_IMSIF     (4) //REAL
#define RDS_EA_IMSEMGIF  (5) //REAL
#define RDS_EA_CNFSUCC   (0) 
#define RDS_EA_CNFSUCC   (0)
#define RDS_HO_WLSTCNT   (8)

typedef struct _ho_apn_t{
    rds_char  ho_apn[RDS_STR_LEN];
    rds_int8  ea_state;
} ho_apn_t;

typedef struct _rds_ho_wlist_t{
    rds_int8 list_cnt;
    ho_apn_t i4ho_apn[RDS_HO_WLSTCNT];
} rds_ho_wlist_t;

typedef enum _rds_wo_action_t
{
    RDS_WO_ACTION_ATTACH,
    RDS_WO_ACTION_HOL2W,
    RDS_WO_ACTION_DETACH,
    RDS_WO_ACTION_CNT
} rds_wo_action_t;

typedef enum {
    RDS_FQDN_OMACP,
    RDS_FQDN_NW,
    RDS_FQDN_SIM,
    RDS_FQDN_CNT,
} rds_fqdn_e;

typedef struct _rds_fqdni4_t{
    rds_fqdn_e  fqdn_type;
    rds_char    fqdn_name[RDS_FQDN_LEN];
} rds_fqdni4_t;

typedef MAL_Setup_Data_Call_Struct rds_setup_data_call_req_t;
typedef MAL_Data_Call_RdsRsp_v11  ea_attach_cnf_t;

typedef struct _rds_comm_req_t{
	rds_int32 id;
	rds_int32 cause;
} rds_comm_req_t;

typedef struct _rds_comm_rsp_t{
	rds_int32 ril_status;
} rds_comm_rsp_t;

typedef struct _rds_setup_data_call_rsp_t{ 
	rds_int32  ril_status;
	MAL_Data_Call_Response_v11 resp;
} rds_setup_data_call_rsp_t;

typedef rds_comm_req_t rds_deact_data_call_req_t;
typedef rds_comm_rsp_t rds_deact_data_call_rsp_t;

typedef struct _ea_detach_req_t{
    rds_uint8 ewod_action; //WO_ACTION_DETACH
    rds_char  szapn[STRING_SIZE];
}ea_detach_req_t;

typedef struct _ea_detach_cnf_t {
    rds_int32 i4ret; 
    rds_char  szapn[RDS_STR_LEN];
}ea_detach_cnf_t;

typedef ea_attach_cnf_t ea_hol2w_cnf_t;

typedef struct _ea_disconn_ind_t{
    rds_char  szapn[RDS_STR_LEN];
}ea_disconn_ind_t;

typedef struct _ea_fqdn_t{
    rds_uint8    i4fqdn_cnt;
    rds_fqdni4_t i4fqdn_list[RDS_FQDN_CNT];
} ea_fqdn_t;

typedef struct _ea_attach_req_t{
    rds_wo_action_t ewod_action;     
    ip_type_t eprotocol; 
    rds_int32 u4mIsValid;
    rds_int32 u4interfaceId;
    rds_int32 u4emergency_ind;
    rds_char  ipv6iid[16];
    rds_char  szapn[STRING_SIZE];
    rds_char  szwifi_ifname[RDS_STR_LEN]; 
    rds_char  szwifi_ipv4addr[2*RDS_STR_LEN];
    rds_char  szwifi_ipv6addr[2*RDS_STR_LEN];
    rds_char  szwifi_macaddr[2*RDS_STR_LEN];
    rds_uint32 attach_to;  //Attach timeout(sec)
    ea_fqdn_t eafqdn_info;
}ea_attach_req_t;

typedef ea_attach_req_t ea_hol2w_req_t;

#endif 
