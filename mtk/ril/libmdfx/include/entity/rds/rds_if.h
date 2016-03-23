#ifndef __RDS_IF_H__
#define __RDS_IF_H__

#define RDS_VER_MAJOR 0
#define RDS_VER_MINOR 1
#define RDS_VERNO ((RDS_VER_MAJOR << 16) | RDS_VER_MINOR)

#define RDS_STR_LEN 32
#define RDS_FQDN_LEN 128 //[20151012] actual length need to confirm with Alan-YL
#define RDS_MCC_LEN 3
#define RDS_MNC_LEN 3
#define RDS_UNUSED(x) ((void)(x))

#define RDS_MAX_HOCNT 65535

#define RDS_HOKEY_START_IFID0 "ril.volte.hos.interface0"
#define RDS_HOKEY_START_IFID1 "ril.volte.hos.interface1"
#define RDS_HOKEY_START_IFID2 "ril.volte.hos.interface2"
#define RDS_HOKEY_START_IFID3 "ril.volte.hos.interface3"
#define RDS_HOKEY_START_IFID4 "ril.volte.hos.interface4"
#define RDS_HOKEY_START_IFID5 "ril.volte.hos.interface5"
#define RDS_HOKEY_START_IFID6 "ril.volte.hos.interface6"
#define RDS_HOKEY_START_IFID7 "ril.volte.hos.interface7"
#define RDS_HOKEY_START_IFID8 "ril.volte.hos.interface8"
#define RDS_HOKEY_START_IFID9 "ril.volte.hos.interface9"
#define RDS_HOKEY_START_IFID10 "ril.volte.hos.interface10"
#define RDS_HOKEY_START_IFID11 "ril.volte.hos.interface11"
#define RDS_HOKEY_START_IFID12 "ril.volte.hos.interface12"
#define RDS_HOKEY_START_IFID13 "ril.volte.hos.interface13"
#define RDS_HOKEY_START_IFID14 "ril.volte.hos.interface14"
#define RDS_HOKEY_START_IFID15 "ril.volte.hos.interface15"

#define RDS_HOKEY_END_IFID0 "ril.volte.hoe.interface0"
#define RDS_HOKEY_END_IFID1 "ril.volte.hoe.interface1"
#define RDS_HOKEY_END_IFID2 "ril.volte.hoe.interface2"
#define RDS_HOKEY_END_IFID3 "ril.volte.hoe.interface3"
#define RDS_HOKEY_END_IFID4 "ril.volte.hoe.interface4"
#define RDS_HOKEY_END_IFID5 "ril.volte.hoe.interface5"
#define RDS_HOKEY_END_IFID6 "ril.volte.hoe.interface6"
#define RDS_HOKEY_END_IFID7 "ril.volte.hoe.interface7"
#define RDS_HOKEY_END_IFID8 "ril.volte.hoe.interface8"
#define RDS_HOKEY_END_IFID9 "ril.volte.hoe.interface9"
#define RDS_HOKEY_END_IFID10 "ril.volte.hoe.interface10"
#define RDS_HOKEY_END_IFID11 "ril.volte.hoe.interface11"
#define RDS_HOKEY_END_IFID12 "ril.volte.hoe.interface12"
#define RDS_HOKEY_END_IFID13 "ril.volte.hoe.interface13"
#define RDS_HOKEY_END_IFID14 "ril.volte.hoe.interface14"
#define RDS_HOKEY_END_IFID15 "ril.volte.hoe.interface15"


/* =========================================== *
 * General Types							   *
 * =========================================== */
typedef char 					rds_char;
/* portable wide character for unicode character set */
typedef unsigned short			rds_wchar;
/* portable 8-bit unsigned integer */
typedef unsigned char           rds_uint8;
/* portable 8-bit signed integer */
typedef signed char             rds_int8;
/* portable 16-bit unsigned integer */
typedef unsigned short int      rds_uint16;
/* portable 16-bit signed integer */
typedef signed short int        rds_int16;
/* portable 32-bit unsigned integer */
typedef unsigned int            rds_uint32;
/* portable 32-bit signed integer */
typedef signed int              rds_int32;
/* boolean representation */
typedef enum
{
    RDS_ERR_INPUT_LEN = -100,

    /* FALSE value */
    RDS_FALSE = 0,
    /* TRUE value */
    RDS_TRUE  = 1
} rds_bool;

typedef enum {
    RDS_PREF_WIFI_ONLY = 0,
    RDS_PREF_CELLULAR_PREF = 1,
    RDS_PREF_WIFI_PREF = 2,
    RDS_PREF_CELLULAR_ONLY = 3,
    RDS_PREF_MAX
} rds_pref_e;

#define RDS_TASK_ID(x) ((x)-RDS_TASK_OFFSET)

typedef enum
{
    RDS_TASK_OFFSET = 0xF000,
    RDS_TASK_EPDGS,
    RDS_TASK_RILD,
    RDS_TASK_IMSM,
    RDS_TASK_UT,
    RDS_TASK_ALL
} rds_ext_task_e;

typedef enum{
     IMC_RAT_TYPE_NONE = 0,
     IMC_RAT_TYPE_2G = IMC_RAT_TYPE_NONE,
     IMC_RAT_TYPE_3G_FDD,
     IMC_RAT_TYPE_3G_TDD,
     IMC_RAT_TYPE_4G_FDD,
     IMC_RAT_TYPE_4G_TDD,

     /* WFC */
     IMC_RAT_TYPE_802_11,
     IMC_RAT_TYPE_802_11a,
     IMC_RAT_TYPE_802_11b,
     IMC_RAT_TYPE_802_11g,
     IMC_RAT_TYPE_802_11n,
     IMC_RAT_TYPE_802_11ac,

     IMC_RAT_TYPE_MAX = IMC_RAT_TYPE_802_11ac,
     IMC_RAT_TYPE_UNSPEC = 0x80,
}imc_rat_type_t;

typedef enum {
	RDS_RAN_UNSPEC = 0, //[20150910]
	RDS_RAN_MOBILE_3GPP = 1,
	RDS_RAN_WIFI = 2,
	RDS_RAN_MOBILE_3GPP2,
	RDS_RAN_TYPE_MAX,
	RDS_RAN_NONE = 0xFE, //DENY by PS, but can try CS
	RDS_RAN_DENY = 0xFF  //DENY by RDS
} ran_type_e;

typedef enum {
	RDS_HO_START = 0,
	RDS_HO_STOP = 1,
	RDS_HO_MAX
} ho_notify_t;

typedef enum {
    WOS_IMSCALL_NONE = 0,  //reserve type to skip 0
    WOS_IR92_VOCALL = 1,
    WOS_IR94_VICALL = 2,
    WOS_CALL_CNT,
} wos_call_t;

/*****************************
* [20151016]:TMO IKEV2 Err Retry
* need Po-Chun's help on fill in the actual err code enum*/
typedef enum {
    EA_ATCH_ERR_NONE = 0,
    EA_ATCH_ERR_VZ   = 0xFFF10000,
    EA_ATCH_ERR_WW   = 0xFFF20000,
    EA_ATCH_ERR_TMO  = 0xFFF30000,
    ERROR_NOTIFY_BASE             = 100000,
    UNSUPPORTED_CRITICAL_PAYLOAD  = ERROR_NOTIFY_BASE + 1,
    INVALID_MAJOR_VERSION         = ERROR_NOTIFY_BASE + 5,
    INVALID_MESSAGE_ID            = ERROR_NOTIFY_BASE + 7,
    NO_PROPOSAL_CHOSEN            = ERROR_NOTIFY_BASE + 14,
    INVALID_KE_PAYLOAD            = ERROR_NOTIFY_BASE + 17,
    AUTHENTICATION_FAILED         = ERROR_NOTIFY_BASE + 24,
    SINGLE_PAIR_REQUIRED          = ERROR_NOTIFY_BASE + 34,
    NO_ADDITIONAL_SAS             = ERROR_NOTIFY_BASE + 35,
    INTERNAL_ADDRESS_FAILURE      = ERROR_NOTIFY_BASE + 36,
    FAILED_CP_REQUIRED            = ERROR_NOTIFY_BASE + 37,
    TS_UNACCEPTABLE               = ERROR_NOTIFY_BASE + 38,
    NO_SUBSCRIPTION               = ERROR_NOTIFY_BASE + 9000,
    NETWORK_TOO_BUSY              = ERROR_NOTIFY_BASE + 10000,
    NETWORK_FAILURE               = ERROR_NOTIFY_BASE + 10500,
    ROAMING_NOR_ALLOWED           = ERROR_NOTIFY_BASE + 11000,
    RAT_DISALLOWED                = ERROR_NOTIFY_BASE + 11001,
    EA_TMO_NETWORK_FAILURE,
    EA_TMO_NETWORK_TOO_BUSY,
    EA_TMO_RAT_DISALLOWED,
    EA_TMO_ROAMING_NOR_ALLOWED,
    EA_ATCH_ERR_CNT,
} ea_atch_err;
/*****************************/

typedef enum {
    RDS_WFCA_CMDRSV = 0,  //reserve
    RDS_WFCA_CFGEN,       //configuration
    RDS_WFCA_QUERY,       //query statistics
} rds_wfca_cmd_e;

typedef enum {
    WFCA_STAT_RSV = 0,         //reserve
    WFCA_STAT_UPDATE,          //following contains valid update info
    WFCA_STAT_NOUPDATE,        //No update from LTECSR(under threshould)
    WFCA_STAT_UNINIT = 255,    //WFCA statistics hasn¡¦t been correctly cfg by RDS
} wfca_state_e;

typedef enum {
    RDS_RTPIF_IMS,
    RDS_RTPIF_EMG,
    RDS_RTPIF_CNT,
} wfca_rtpif_e;

typedef enum {
    RDS_RAT_UNKNOWN = 0,
    RDS_RAT_GPRS = 1,
    RDS_RAT_EDGE = 2,
    RDS_RAT_UMTS = 3,
    RDS_RAT_HSDPA = 9,
    RDS_RAT_HSUPA = 10,
    RDS_RAT_HSPA = 11,
    RDS_RAT_LTE = 14
} rds_rat_e;

typedef enum {
    RDS_MD1 = 0,
    RDS_MD2 = 1,
    RDS_MD3 = 2,
    RDS_MD_CNT,
} rds_md_e;

typedef struct _rds_ru_set_mdstat_req_t{
    rds_uint8 md_idx;      // 0:MD1 1:MD2 2:MD3
    rds_uint8 sim_idx;     // for futhur flexibility
    rds_uint8 fgisroaming; // RDS_TRUE, RDS_FALSE
    rds_uint8 md_rat;      // rds_rat_e
    rds_uint8 srv_state;    //service state
} rds_ru_set_mdstat_req_t;

typedef MAL_Data_Call_Response_v11 MAL_Data_Call_RdsRsp_v11;

typedef struct _rds_event_t{
    rds_uint32 u4snd_id;
    void       *pvsnd_data;
    size_t     u4snd_data_len;
    rds_uint32 u4rcv_id;
    void       *pvrcv_data;
    size_t     u4rcv_data_len;
}rds_event_t;

typedef struct _rds_ru_set_uiparam_req_t{
    rds_uint8 fgimsolte_en; // ims over lte enable
    rds_uint8 fgwfc_en;     // wfc enable
    rds_char  omacp_fqdn[RDS_FQDN_LEN];
    rds_uint8 fgwifiui_en;  // ui wifi botton
    rds_uint8 rdspolicy;    // preference mode
    rds_uint8 fgdata_roaming_ui_en; //data roaming ui enable
} rds_ru_set_uiparam_req_t;

typedef struct _rds_ru_set_wifistat_req_t{
    rds_uint8 fgwifi_en;
    rds_char  szwifi_ifname[RDS_STR_LEN];
    rds_char  szwifi_ipv4addr[2*RDS_STR_LEN];
    rds_char  szwifi_ipv6addr[2*RDS_STR_LEN];
    rds_char  szwifi_macaddr[2*RDS_STR_LEN];
} rds_ru_set_wifistat_req_t;

typedef struct _rds_ru_sst_update_req_t{
    // prevent ping-pong between two RAN,
    // rds took high low threshold to determine the H.O. time
    // The rds considers rovein RAN when signal strengh
    // over high threshold and roveout below low threshold
    rds_int32 i4wifirssih;  // wifi rssi threshold high
    rds_int32 i4wifirssil;  // wifi rssi threshold low
    rds_int32 i4ltersrph;   // lte  rsrp threshold high
    rds_int32 i4ltersrpl;   // lte  rsrp threshold low
} rds_ru_sst_update_req_t;

typedef struct _rds_ho_status_t{
    /* please Alan help to provide */
    rds_uint8 ucho_status; // 0: start_ho 1: stop_ho
    rds_uint8 fgho_result; // 0: fail(RDS_FALSE) 1: success(RDS_TRUE)
    rds_uint8 interfaceID; // (RILD_Req -1, aka real) /*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
    rds_uint8 esource_ran_type; //ran_type_e
    rds_uint8 etarget_ran_type; //ran_type_e
} rds_ho_status_t;

typedef struct _rds_reg_req_t{
    rds_uint32  u4rds_task_id; //RDS_TASK_ID()
}rds_reg_req_t;

typedef struct _rds_rb_get_demand_req_t{
    rds_uint32 u4pdn_id;
} rds_rb_get_demand_req_t;

typedef struct _rds_rb_get_ran_type_rsp_t{
    rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
    rds_uint32 u4ran_type; //ran_type_e
} rds_rb_get_ran_type_rsp_t;

typedef struct _rds_rb_get_last_err_rsp_t{
    rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
    rds_int32  i4lasterr;
    rds_int32  i4lastsuberr;
} rds_rb_get_last_err_rsp_t;

typedef struct _rds_err_rsp_t{
   /* rds will reply i4rds_ret if input size is wrong for send and recv */
   rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
} rds_err_rsp_t;

typedef dm_req_setup_data_call_t rds_sdc_req_t;
typedef dm_resp_setup_data_call_t rds_sdc_cnf_t;

typedef struct {
    rds_int32 ril_status;
    MAL_Data_Call_Response_v11 ind;
} rds_sdc_ind_t;

typedef dm_req_t rds_ddc_req_t;

typedef struct {
    rds_int32 status;
    rds_uint8 eran_type;
} rds_ddc_cnf_t;

typedef struct {
    rds_int32 ril_status;
    rds_int32 cid; //InterfaceID (RILD_Req-1, aka real) /*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
} rds_ddc_ind_t;

typedef struct _rds_rvout_alert_t{
    rds_bool  rvalert_en;    //is near rvout threshold 
    rds_int32 i4wifirssi;    //current wifi rssi
} rds_rvout_alert_t;

typedef struct _rds_wifi_pdnact_t{
    rds_int32 ifid;     //currently will always be IMS ifid = 4
    rds_bool  pdn_rdy;  //RDS_TRUE: ready
} rds_wifi_pdnact_t;

typedef int (*rds_ho_status_ind_fn)(rds_ho_status_t *prstatus);
typedef int (*rds_rvout_alert_ind_fn)(rds_rvout_alert_t *pralert);
typedef int (*rds_wifi_pdnact_ind_fn)(rds_wifi_pdnact_t *pralert);
typedef struct _rds_notify_funp_t{
    rds_ho_status_ind_fn      pfrds_ho_status_ind;
    rds_rvout_alert_ind_fn    pfrds_rvout_alert_ind;
    rds_wifi_pdnact_ind_fn    pfrds_wifi_pdnact_ind;
} rds_notify_funp_t;

typedef rds_sdc_req_t rr_sdc_req_t;
typedef rds_sdc_cnf_t rr_sdc_cnf_t;
typedef rds_ddc_cnf_t rr_ddc_cnf_t;
typedef rds_ddc_req_t rr_ddc_req_t;

typedef struct _wfca_callinfo_t {
    wfca_state_e  state;
    int           id;
    int           loss_rate; 
    int           thrpt;     //throughput*
} wfca_callinfo_t;

typedef struct _rds_wfca_cfg_t{
    int    id;
    int    threshold;
    int    duration;
} rds_wfca_cfg_t;

typedef struct _rds_wfca_req_t{
    rds_wfca_cmd_e    cmd;
    rds_wfca_cfg_t     cfg; 
} rds_wfca_req_t;

/* test mode purpose */
typedef enum {
	SDC_IND_SUCC = 0,
    SDC_IND_FAIL,
    PDN_NW_DDC,
    PDN_INFO_DUMP,
    UT_CFG_WONLY,
    UT_CFG_WPREF,
    UT_CFG_MONLY,
    UT_CFG_MPREF,
    UT_SKIP_1stIMS = 254,
    ENABLE_ENTITY_UT = 255,
} ut_cmd_t;

typedef struct _rds_ut_cmd_t{
    rds_uint32 evnt_id;
    rds_uint32 cmd;
    rds_uint32 ril_ifid;
    rds_char iipv6[16]; //for SDC IND UT
} rds_ut_cmd_t;

typedef struct _rds_ru_set_callstat_req_t{
    rds_bool      in_call;
    wos_call_t    call_type;
} rds_ru_set_callstat_req_t;

typedef struct _ru_nm_cgreg_mode_req_t{
    rds_uint8 uccgreg_mode; 
} ru_nm_cgreg_mode_req_t;

typedef struct _ru_nm_cgreg_mode_cnf_t{
	unsigned int        result;
	char                data[4096];
} ru_nm_cgreg_mode_cnf_t;

/* =========================================== *
 * Public APIs  							   *
 * =========================================== */
extern rds_int32 rds_get_verno(void);
extern rds_int32 rds_set_ui_param(rds_ru_set_uiparam_req_t *req, conn_ptr_t conn_ptr);
extern rds_int32 rds_set_wifistat(rds_ru_set_wifistat_req_t *req, conn_ptr_t conn_ptr);
extern rds_int32 rds_set_callstat(rds_ru_set_callstat_req_t *req, conn_ptr_t conn_ptr);
extern rds_int32 rds_set_mdstat(rds_ru_set_mdstat_req_t *req, conn_ptr_t conn_ptr);
extern conn_ptr_t epdgs_rds_conn_init(rds_notify_funp_t *callback_fn);
extern rds_int32 epdgs_rds_conn_exit(conn_ptr_t conn_ptr);
extern conn_ptr_t rild_rds_conn_init(rds_notify_funp_t *callback_fn);
extern rds_int32 rild_rds_conn_exit(conn_ptr_t conn_ptr);
extern rds_int32 rds_get_ran_type(rds_rb_get_demand_req_t *req, rds_rb_get_ran_type_rsp_t *rsp, conn_ptr_t conn_ptr);
extern rds_int32 rds_get_last_err(rds_rb_get_demand_req_t *req, rds_rb_get_last_err_rsp_t *rsp, conn_ptr_t conn_ptr);
extern rds_int32 rds_urc_ut_req(rds_rb_get_demand_req_t *req, conn_ptr_t conn_ptr); // UT ONLY
extern rds_int32 epdgs_rds_error_test(conn_ptr_t conn_ptr); //UT ONLY
extern rds_int32 rild_rds_sdc_req(dm_req_setup_data_call_t *req, dm_resp_setup_data_call_t *cnf);
extern rds_int32 rild_rds_ddc_req(dm_req_t *req, rr_ddc_cnf_t *cnf);
extern rds_int32 rild_rds_utcmd_req(rds_ut_cmd_t *req);
#endif
