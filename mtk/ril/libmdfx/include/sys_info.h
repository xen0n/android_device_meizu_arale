/*
    Here is the information of the target system. 
*/

#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

// Platform, NEED_TO_BE_NOTICED, assigned by compilers
#define __LINUX_PLAT__
#define __GCC_COMPILER__
#define __EVENT_USAGE_MONITOR__

// Boolean value
#ifndef __cplusplus
#include <stdbool.h>
/*
typedef int bool;
#define true    (0 == 0)
#define false    (0 != 0)
*/
#endif

#ifdef __EVENT_USAGE_MONITOR__
// NEED_TO_BE_NOTICED, temporarily methods to monitor memory usage
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#define gettid() syscall(__NR_gettid)
extern pid_t mal_pid;
#endif

// Return value
#define SYS_SUCC    (0)
#define SYS_FAIL    (-1)

// Print level
#define ERR_LV    (1)
#define WARN_LV   (2)
#define INFO_LV   (4)
#define DEBUG_LV  (8)
#define PRINT_LV  (ERR_LV | WARN_LV | INFO_LV | DEBUG_LV)    // NEED_TO_BE_NOTICED, assigned by compilers

// MDFX
// => Entity ID
typedef enum entity_id
{
    ENTITY_INVAL_ID,    // for control

    ENTITY_FRAMEWORK_MAIN,
    ENTITY_FRAMEWORK_MFI,
    ENTITY_FRAMEWORK_HOST,
    ENTITY_FRAMEWORK_MFIA,
    ENTITY_ATI,

    ENTITY_MDMNGR,
    ENTITY_RILPROXY,
    ENTITY_SIMMNGR,
    ENTITY_DATAMNGR,
    ENTITY_IMSMNGR,
    ENTITY_RDS,
    ENTITY_EPDGA,
    ENTITY_NWMNGR,
    
    ENTITY_MAX_ID   // for control
}entity_id_t;

// => Task ID
typedef enum task_id
{
    TASK_INVAL_ID,  // for control
    
    TASK_FRAMEWORK_DEFAULT,
    TASK_FRAMEWORK_MFI_ERX,
    TASK_FRAMEWORK_HOST_RX,
    TASK_FRAMEWORK_HOST_TX,
    TASK_FRAMEWORK_MFIA_ERX,
    TASK_ATI_ATC,

    TASK_MDMNGR_START   = 100,
    TASK_MDMNGR1,
    TASK_MDMNGR2,
    TASK_MDMNGR3,
    TASK_MDMNGR4,
    TASK_MDMNGR5,

    TASK_RILPROXY_START = 200,
    TASK_RILPROXY,
    
    TASK_SIMMNGR_START  = 300,
    TASK_SIMMNGR,
    TASK_SIMMNGR_CHECK,
    TASK_SIMMNGR_NOTIFY,

    TASK_DATAMNGR_START = 400,
    TASK_DATAMNGR,
    TASK_DATAMNGR_DATI_HDL,
    TASK_DATAMNGR_NETLINK,
    
    TASK_IMSMNGR_START  = 500,
    TASK_IMSMNGR,

    TASK_RDS_START      = 600,
    TASK_RB,
    TASK_RU,
    TASK_RRA,	
    TASK_RDS_UT,

    TASK_EPDGA_START    = 700,
    TASK_EPDGA_REQ,
    TASK_EPDGA_SIM,
    TASK_EPDGA_QUERY,

    TASK_NWMNGR_START   = 800,
    TASK_NWMNGR,

    TASK_MAX_ID     // for control
}task_id_t;

// => Service ID
typedef enum srv_id
{
    SRV_INVAL_ID = TASK_MAX_ID + 1,         // for control
    
    // => HOST entity
    SRV_FRAMEWORK_HOST_MAIN,

    // ePDGa
    SRV_EPDGA_MAIN,
    SRV_EPDGA_SIM,
    SRV_EPDGA_INIT,

    SRV_MAX_ID        // for control
}srv_id_t;

// => Event ID
typedef enum event_id
{
    EVENT_INVAL_ID,     // for control
 
    // => Main entity
    // N/A
 
    // => MFI entity
    // N/A
    
    // => MFIA entity
    EVENT_MFIA_CHNL_INIT,
    EVENT_MFIA_CHNL_EXIT,
	// N/A

    // => ATI entity
	EVENT_ATI_REG, 
	EVENT_ATI_ATC_REQ, 
    EVENT_ATI_BYPASS_ATC_REQ, 
    EVENT_ATI_ATC_RESP, 
	EVENT_ATI_URC, 
    EVENT_ATI_BYPASS_URC, 
    EVENT_ATI_HIJACK, 

/*
    EVENT_ATI_ATC_CFUN, 
    EVENT_ATI_ATC_CGDCONT, 
    EVENT_ATI_ATC_CGSDCONT, 
    EVENT_ATI_ATC_CGQREQ,	
    EVENT_ATI_ATC_CGQMIN,	
    EVENT_ATI_ATC_CGEQREQ, 
    EVENT_ATI_ATC_CGEQMIN, 
    EVENT_ATI_ATC_CGATT, 
    EVENT_ATI_ATC_CGACT, 
    EVENT_ATI_ATC_CGCMOD, 
    EVENT_ATI_ATC_CGDATA, 
    EVENT_ATI_ATC_CGPADDR, 
    EVENT_ATI_ATC_CGCONTRDP, 
    EVENT_ATI_ATC_CGSCONTRDPT, 
    EVENT_ATI_ATC_CGREG, 
    EVENT_ATI_ATC_CGPRCO, 
    EVENT_ATI_ATC_EREP, 
    EVENT_ATI_ATC_CGANS, 
    EVENT_ATI_ATC_CGSDATA, 
    EVENT_ATI_ATC_ACTTEST, 
    EVENT_ATI_ATC_EGCH, 
    EVENT_ATI_ATC_EGTYPE, 
    EVENT_ATI_ATC_MBPC, 
    EVENT_ATI_ATC_VZWAPNE, 
    EVENT_ATI_ATC_EFUN, 
	EVENT_ATI_URC_CME, 
*/

    // => Modem Manager event
    EVENT_MDMNGR_START = 100,
    EVENT_MDMNGR_ATCMD_REQ,             /* mdmngr_atcmd_req_t */
    EVENT_MDMNGR_ATCMD_RESP,            /* mdmngr_atcmd_resp_t */
    EVENT_MDMNGR_ATURC_REGISTER,
    EVENT_MDMNGR_ATURC_NOTIFY,          /* mdmngr_aturc_notify_t */
    EVENT_MDMNGR_ATCMD_REQ_EX,          /* mdmngr_atcmd_req_t */
    EVENT_MDMNGR_STATE_NOTIFY,          /* mdmngr_state_t */
    
    // => SIM Manager event
    EVENT_SIMMNGR_START = 200,
    EVENT_SIMMNGR_QUERY_CSQ,
    EVENT_SIMMNGR_CSQ_NOTIFY,
    EVENT_SIMMNGR_NOTIFY_CHECKIN,
    EVENT_SIMMNGR_NOTIFY_CHECKOUT,
    EVENT_SIMMNGR_CHECK_SIM_TYPE_REQ,
    EVENT_SIMMNGR_QUERY_SIM_INFO_REQ,
    EVENT_SIMMNGR_AKA_REQ,
    EVENT_SIMMNGR_SIM_INSERT_STATE_NOTIFY,
    EVENT_SIMMNGR_SIM_USIM_DETECT_NOTIFY,
    EVENT_SIMMNGR_SIM_ISIM_CH_NOTIFY,
    EVENT_SIMMNGR_SIM_STK_NOTIFY,
    EVENT_SIMMNGR_INSERT_CHECK_NOTIFY,
    EVENT_SIMMNGR_SIM_STATE_CHECK,
    EVENT_SIMMNGR_SIM_INSERT_CHECK,
    EVENT_SIMMNGR_IMS_ENABLE_REQ,
    EVENT_SIMMNGR_IMS_DISABLE_REQ,
    EVENT_SIMMNGR_MNC_LEN_REQ,

    // => Data Manager event 
    EVENT_DATAMNGR_START = 300,
    EVENT_DATAMNGR_NOTIFICATION,
    EVENT_SETUP_DATA_CALL_REQ,
    EVENT_SETUP_DATA_CALL_RESP,
    EVENT_DEACTIVATE_DATA_CALL_REQ,
    EVENT_DEACTIVATE_DATA_CALL_RESP,
    EVENT_SETUP_DEDICATE_DATA_CALL_REQ,
    EVENT_SETUP_DEDICATE_DATA_CALL_RESP,
    EVENT_MODIFY_DATA_CALL_REQ,
    EVENT_MODIFY_DATA_CALL_RESP,
    EVENT_DEACTIVATE_DEDICATE_DATA_CALL_REQ,
    EVENT_DEACTIVATE_DEDICATE_DATA_CALL_RESP,
    EVENT_ABORT_SETUP_DATA_CALL_REQ,
    EVENT_ABORT_SETUP_DATA_CALL_RESP,
    EVENT_PCSCF_DISCOVERY_PCO_REQ,
    EVENT_PCSCF_DISCOVERY_PCO_RESP,
    EVENT_LAST_DATA_CALL_FAIL_CAUSE_REQ,
    EVENT_LAST_DATA_CALL_FAIL_CAUSE_RESP,
    EVENT_DATA_CALL_LIST_REQ,
    EVENT_DATA_CALL_LIST_RESP,
    EVENT_DATA_CALL_LIST_CHANGED_URC,
    EVENT_DEFAULT_BEARER_MODIFIED_URC,
    EVENT_DEDICATE_BEARER_ACTIVATED_URC,
    EVENT_DEDICATE_BEARER_MODIFIED_URC,
    EVENT_DEDICATE_BEARER_DEACTIVATED_URC,
    EVENT_APN_EDIT_REQ,
    EVENT_APN_EDIT_PARA_REQ,
    EVENT_APN_SET_MODE_REQ,
    EVENT_SET_INITIAL_ATTACH_APN_REQ,
    EVENT_SET_INITIAL_ATTACH_APN_RESP,
    EVENT_RADIO_POWER_REQ,
    EVENT_RADIO_POWER_DO_REQ,
    EVENT_RADIO_POWER_RESP,
    EVENT_IF_STATUS_REQ,
    EVENT_IF_STATUS_RESP,
    EVENT_ALLOW_DATA_REQ,
    EVENT_ALLOW_DATA_RESP,
    EVENT_IF_RAT_CELL_INFO_REQ,
    EVENT_IF_RAT_CELL_INFO_RESP,
    EVENT_BEARER_REQ,
    EVENT_BEARER_RESP,
    EVENT_PASS_IMS_PDN_REQ,
    EVENT_PASS_IMS_PDN_RESP,
    EVENT_GET_IMS_PDN_REQ,
    EVENT_GET_IMS_PDN_RESP,
    EVENT_GET_IMS_PARA_REQ,
    EVENT_GET_IMS_PARA_RESP,
    EVENT_SETUP_DATA_CALL_IND,
    EVENT_SETUP_DATA_CALL_IND_FAIL,
    EVENT_DM_HO_RESP,


    // => => DATI
    EVENT_DATAMNGR_ATCMD_REQ, 
    EVENT_DATAMNGR_ATCMD_RESP, 
    EVENT_DATAMNGR_ATCMD_REG, 

    // => Netlink
    EVENT_NETLINK_LISTEN_REQ,
    
    // => RIL Proxy event
    EVENT_RILPROXY_START = 400,
    EVENT_RILPROXY_ATCMD_REQ,           /* rilproxy_atcmd_req_t */
    EVENT_RILPROXY_ATCMD_RESP,          /* rilproxy_atcmd_resp_t */
    EVENT_RILPROXY_CONNECT,             /* rilproxy_ril_connect_req_t */
    EVENT_RILPROXY_DISCONNECT,
    
    // => IMS Manager event
    EVENT_IMSMNGR_START = 500,
    EVENT_IMSMNGR_REQ,                  /* imsmngr_req_t / imsmngr_resp_t */
    EVENT_IMSMNGR_NOTIFY,               /* imsmngr_notify_t */

    // => RDS event 
    EVENT_RDS_START = 600,
    EVENT_RDS_RU = EVENT_RDS_START,
    /* RU IF */
    EVENT_MFI_RU_SET_UIPARAM_REQ,       /* 1: UI setting request */
    EVENT_MFI_RU_SET_WIFISTATUS_REQ,    /* 2: wifi status update request */
    EVENT_MFI_RU_SET_SST_REQ,           /* 3: UI(?) RDS threshold request */    
    EVENT_RU_WS_WIFI_STATUS_IND,        /* 4: WPA Supplicant WiFi status indication */    
    EVENT_RU_DM_3GPP_STATUS_IND,        /* 5: Data Manager(?) 3GPP status indication */
    EVENT_RU_RB_WSL_CFG_IND,            /* 6: WPA Supplicant Lisener config */
    EVENT_MFI_RU_SET_CALLSTATUS_REQ,    /* 7: IMS in call status update request */
    EVENT_RU_RB_WIFI_RVOUT_IND,         /* 8: Wifi roveout indication */
    EVENT_MFI_RU_SET_MDSTATUS_REQ,      /* 9: FWK set MD status */
    EVENT_RU_NM_CGREG_REQ,              /* 10: CGREG request to NM */
    EVENT_RU_NM_CGREG_CNF,              /* 11: CGREG confirm from NM */

    EVENT_RDS_RB = EVENT_RDS_START+30,   
    /* RB IF */
    EVENT_MFI_RB_SETUP_DATA_CALL_REQ,   /* 31: MFI RB setup data call request */
    EVENT_MFI_RB_SETUP_DATA_CALL_CNF,   /* 32: MFI RB setup data call confirm */
    EVENT_RB_DM_SETUP_DATA_CALL_IND,    /* 33: RB DM setup data call indication */ 
    EVENT_MFI_RB_DEACT_DATA_CALL_REQ,   /* 34: RB deactive data call request */
    EVENT_MFI_RB_DEACT_DATA_CALL_CNF,   /* 35: RB deactive data call confirm */
    EVENT_RB_DM_DDC_REQ,                /* 36: RB data call disconnect request */ 
    EVENT_RU_RB_SET_RPL_REQ,            /* 37: RPL Update*/
    EVENT_MFI_RB_GET_RANTYPE_REQ,       /* 38: */
    EVENT_MFI_RB_GET_RANTYPE_RSP,       /* 39: */
    EVENT_MFI_RB_GET_LASTERR_REQ,       /* 40: */
    EVENT_MFI_RB_GET_LASTERR_RSP,       /* 41: */
    EVENT_RRA_RB_RETRY_HO_REQ,          /* 42: */
    EVENT_RB_DM_SETUP_DATA_CALL_REQ,    /* 43: Mobile HO PDN setup request (rr_sdc_req_t)*/
    EVENT_RB_DM_SETUP_DATA_CALL_CNF,    /* 44: Mobile HO PDN setup confirm (dm_resp_setup_data_call_t)*/
    EVENT_RB_DM_DEACT_DATA_CALL_REQ,    /* 45: Interface ID (RILD_Req-1, aka real)*//*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
    EVENT_RB_DM_DEACT_DATA_CALL_IND,    /* 46: Interface ID (RILD_Req-1, aka real)*//*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
    EVENT_RB_DM_HO_REQ,                 /* 47: H.O request rds_ho_status_t*/ 
    EVENT_RB_HO2RAN_REQ,                /* 48: RB H.O 2 RAN request */
    EVENT_RB_EA_HOLIST_IND,             /* 49: New HO apn info update IND from EA */
    EVENT_RB_EA_HO_REQ,                 /* 50: */
    EVENT_RB_DM_EPDG_PDN_DETACT_REQ,    /* 51: */
    EVENT_RB_SYSHO_TO_IND,              /* 52: H.O. timeout indication */
    EVENT_MFI_RB_WIFI_RVOUT_IND,        /* 53: wifi rove out indication */
    EVENT_MFI_RB_WIFIPDN_ACT_IND,       /* 54: wifi PDN State indication */

    EVENT_RDS_RRA = EVENT_RDS_START+60,   
    /* RRA IF */
    EVENT_RRA_RB_HO_RETRY_IND,          /* 61: */
    EVENT_RRA_SYS_RETRY_TO_IND,         /* 62: */
    EVENT_RRA_SYSTATE_UPADTE_IND,       /* 63: */

    EVENT_RDS_EA = EVENT_RDS_START+70,
    /* for RB & EA */   
    EVENT_RB_EA_ATTACH_REQ,             /* 71: attach request to EA*/
    EVENT_RB_EA_ATTACH_CNF,             /* 72: */
    EVENT_RB_EA_DETACH_REQ,             /* 73: dettach request to EA*/
    EVENT_RB_EA_DETACH_CNF,             /* 74: */
    EVENT_RB_EA_HOL2W_REQ,              /* 75: H.O. request to EA*/
    EVENT_RB_EA_HOL2W_CNF,              /* 76: */
    EVENT_RB_EA_DISCONN_IND,            /* 77: disconnect indication from EA2RB*/ 

    EVENT_RDS_MISC = EVENT_RDS_START+80,   
    /* To UT IF */
    EVENT_RU_RB_UT_RPL_UPDATE_REQ,      /* 81: */
    /* To outside MAL */
    EVENT_MFI_RB_HO_STATUS_IND,         /* 82: handover status indication */
    /* To RDS Common */
    EVENT_MAL_RDS_REG_REQ,              /* 83: register task */
    EVENT_MAL_RDS_DEREG_REQ,            /* 84: deregister task */
    EVENT_MAL_RDS_DEREG_RSP,            /* 85: */
    EVENT_MFI_RB_URC_TEST_REQ,          /* 86: ut only removed in OF*/
    EVENT_RDS_INTERNAL_ERR_RSP,         /* 87: for error response */
    /* Timer to RDS */
    EVENT_RU_TM_WSL_TIMER_IND,          /* 88: WPA Supplicant timer indication */
    EVENT_UT_RU_CONIF_REQ,              /* 89: UT purpose */
    EVENT_UT_RB_DUMPDNI4,               /* 90: */
    EVENT_UT_RB_NWDDCIND,               /* 91: */

    EVENT_RDS_RB_SKIP1IMS_UT,
    EVENT_RDS_RB_MOBILE_DDC_UT,
    EVENT_RDS_RB_WIFI_DDC_UT,
    EVENT_MFI_RDS_UT_CMD,
    
    /* epdga event */
    EVENT_EPDGA_START = 700,
    EVENT_EPDGA_QUERY_WO_SUP_REQ,
    EVENT_EPDGA_QUERY_WO_SUP_RSP,

    EVENT_EPDGA_QUERY_WIFI_PCSCFS_REQ,
    EVENT_EPDGA_QUERY_WIFI_PCSCFS_RSP,

    EVENT_EPDGA_QUERY_AP_MAC_REQ,
    EVENT_EPDGA_QUERY_AP_MAC_RSP,


    // => Network Manager event
    EVENT_NWMNGR_START = 800,
    
    EVENT_NWMNGR_SET_PS_REG_STATE_MODE,
    EVENT_NWMNGR_GET_PS_REG_STATE,
    EVENT_NWMNGR_NOTIFY_PS_REG_STATE,
    
    EVENT_MAX_ID        // for control
}event_id_t;




/* ------------------------------------ */
/*  SIM Manager                         */
/* ------------------------------------ */

typedef struct _simmngr_csq_result {
    int                 result;
    int                 rssi;
    int                 ber;
} simmngr_csq_result_t;


#endif
