#ifndef DATAMNGR_EVENT_H
#define DATAMNGR_EVENT_H

#include "mdfx.h"
#ifdef APN_TABLE_SUPPORT
#include "apn_table.h"
#endif

#define IMSM_NAME "imsm"
#define LANMNGR_NAME "lanmngr"
#define ATI_NAME "ati"
#define ATI1_NAME "ati1"
#define ATI2_NAME "ati2"
#define ATI3_NAME "ati3"
#define ATI4_NAME "ati4"
#define RILDPROXY_NAME "rildproxy"
#define STRING_SIZE 128

#ifndef UNUSED
#define UNUSED(x) (x)   //eliminate "warning: unused parameter"
#endif


#ifndef MAX_IPV4_ADDRESS 
#define MAX_IPV4_ADDRESS 17
#endif
#ifndef MAX_IPV6_ADDRESS 
#define MAX_IPV6_ADDRESS 42
#endif
#ifndef MAX_IPV4_ADDRESS_LENGTH 
#define MAX_IPV4_ADDRESS_LENGTH 17
#endif
//xxx.xxx.xxx.xxx
#ifndef MAX_IPV6_ADDRESS_LENGTH 
#define MAX_IPV6_ADDRESS_LENGTH 65
#endif
//xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
//xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
#ifndef MAX_NUM_DNS_ADDRESS_NUMBER 
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#endif
#ifndef MAX_PCSCF_NUMBER 
#define MAX_PCSCF_NUMBER 6
#endif

#ifndef CONCATENATE_NUM 
#define CONCATENATE_NUM 11
#endif

#ifndef DEFAULT_MTU_SIZE	
#define DEFAULT_MTU_SIZE	1500
#endif



typedef enum {
	MAL_DATA_DETACH = 0,
	MAL_DATA_ATTACH = 1,
	MAL_DATA_DETACH_NOT_RESPONSE = 2,
	MAL_DATA_ATTACH_NOT_RESPONSE = 3,
} dm_data_att_t;

typedef enum {
    MAL_EGACT_CAUSE_DONT_CARE = 0,
    MAL_EGACT_CAUSE_DEACT_NORMAL = 1,
    MAL_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR = 2,
    MAL_EGACT_CAUSE_DEACT_NO_PCSCF = 3,
    MAL_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR = 4,
    MAL_EGACT_CAUSE_DEACT_VIA_RB = 100,
} dm_disconn_cause_t;

typedef enum {	// fail cause
    MAL_SM_OPERATOR_BARRED = 0b00001000,
    MAL_SM_MBMS_CAPABILITIES_INSUFFICIENT = 0b00011000,
    MAL_SM_LLC_SNDCP_FAILURE = 0b00011001,
    MAL_SM_INSUFFICIENT_RESOURCES = 0b00011010,
    MAL_SM_MISSING_UNKNOWN_APN = 0b00011011,
    MAL_SM_UNKNOWN_PDP_ADDRESS_TYPE = 0b00011100,
    MAL_SM_USER_AUTHENTICATION_FAILED = 0b00011101,
    MAL_SM_ACTIVATION_REJECT_GGSN = 0b00011110 ,
    MAL_SM_ACTIVATION_REJECT_UNSPECIFIED = 0b00011111,
    MAL_SM_SERVICE_OPTION_NOT_SUPPORTED = 0b00100000,
    MAL_SM_SERVICE_OPTION_NOT_SUBSCRIBED = 0b00100001,
    MAL_SM_SERVICE_OPTION_OUT_OF_ORDER = 0b00100010,
    MAL_SM_NSAPI_IN_USE = 0b00100011,
    MAL_SM_REGULAR_DEACTIVATION = 0b00100100,
    MAL_SM_QOS_NOT_ACCEPTED = 0b00100101,
    MAL_SM_NETWORK_FAILURE = 0b00100110,
    MAL_SM_REACTIVATION_REQUESTED = 0b00100111,
    MAL_SM_FEATURE_NOT_SUPPORTED = 0b00101000,
    MAL_SM_SEMANTIC_ERROR_IN_TFT = 0b00101001,
    MAL_SM_SYNTACTICAL_ERROR_IN_TFT = 0b00101010,
    MAL_SM_UNKNOWN_PDP_CONTEXT = 0b00101011,
    MAL_SM_SEMANTIC_ERROR_IN_PACKET_FILTER = 0b00101100,
    MAL_SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER = 0b00101101,
    MAL_SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED = 0b00101110,
    MAL_SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT = 0b00101111,
    MAL_SM_BCM_VIOLATION = 0b00110000,
    MAL_SM_LAST_PDN_DISC_NOT_ALLOWED = 0b00110001,
    MAL_SM_ONLY_IPV4_ALLOWED = 0b00110010,
    MAL_SM_ONLY_IPV6_ALLOWED = 0b00110011,
    MAL_SM_ONLY_SINGLE_BEARER_ALLOWED = 0b00110100,
    MAL_ESM_INFORMATION_NOT_RECEIVED = 0b00110101,
    MAL_SM_PDN_CONNECTION_NOT_EXIST = 0b00110110,
    MAL_SM_MULTIPLE_PDN_APN_NOT_ALLOWED = 0b00110111,
    MAL_SM_COLLISION_WITH_NW_INITIATED_REQUEST = 0b00111000,
    MAL_ESM_UNSUPPORTED_QCI_VALUE = 0b00111011,
    MAL_SM_BEARER_HANDLING_NOT_SUPPORT = 0b00111100,
    MAL_SM_MAX_PDP_NUMBER_REACHED = 0b01000001,
    MAL_SM_APN_NOT_SUPPORT_IN_RAT_PLMN = 0b01000010,
    MAL_SM_INVALID_TRANSACTION_ID_VALUE = 0b01010001,
    MAL_SM_SEMENTICALLY_INCORRECT_MESSAGE = 0b01011111,
    MAL_SM_INVALID_MANDATORY_INFO = 0b01100000,
    MAL_SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED = 0b01100001,
    MAL_SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100010,
    MAL_SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED = 0b01100011,
    MAL_SM_CONDITIONAL_IE_ERROR = 0b01100100,
    MAL_SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100101,
    MAL_SM_PROTOCOL_ERROR = 0b01101111,
    MAL_SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT = 0b01110000
} AT_MAL_SM_Error;


/* See MAL_REQUEST_LAST_CALL_FAIL_CAUSE */
typedef enum {
    MAL_CALL_FAIL_UNOBTAINABLE_NUMBER = 1,
    MAL_CALL_FAIL_NORMAL = 16,
    MAL_CALL_FAIL_BUSY = 17,
    MAL_CALL_FAIL_CONGESTION = 34,
    MAL_CALL_FAIL_ACM_LIMIT_EXCEEDED = 68,
    MAL_CALL_FAIL_MAL_CALL_BARRED = 240,
    MAL_CALL_FAIL_FDN_BLOCKED = 241,
    MAL_CALL_FAIL_IMSI_UNKNOWN_IN_VLR = 242,
    MAL_CALL_FAIL_IMEI_NOT_ACCEPTED = 243,
    MAL_CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE = 1000,
    MAL_CALL_FAIL_CDMA_DROP = 1001,
    MAL_CALL_FAIL_CDMA_INTERCEPT = 1002,
    MAL_CALL_FAIL_CDMA_REORDER = 1003,
    MAL_CALL_FAIL_CDMA_SO_REJECT = 1004,
    MAL_CALL_FAIL_CDMA_RETRY_ORDER = 1005,
    MAL_CALL_FAIL_CDMA_ACCESS_FAILURE = 1006,
    MAL_CALL_FAIL_CDMA_PREEMPTED = 1007,
    MAL_CALL_FAIL_CDMA_NOT_EMERGENCY = 1008, /* For non-emergency number dialed
                                            during emergency callback mode */
    MAL_CALL_FAIL_CDMA_ACCESS_BLOCKED = 1009, /* CDMA network access probes blocked */
    MAL_CALL_FAIL_ERROR_UNSPECIFIED = 0xffff
} MAL_LastCallFailCause;

/* See MAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE */
typedef enum {
    MAL_PDP_FAIL_NONE = 0, /* No error, connection ok */
    MAL_PDP_FAIL = 1, /* PDN fail to setup */

    /* an integer cause code defined in TS 24.008
       section 6.1.3.1.3 or TS 24.301 Release 8+ Annex B.
       If the implementation does not have access to the exact cause codes,
       then it should return one of the following values,
       as the UI layer needs to distinguish these
       cases for error notification and potential retries. */
    MAL_PDP_FAIL_OPERATOR_BARRED = 0x08,               /* no retry */
    MAL_PDP_FAIL_INSUFFICIENT_RESOURCES = 0x1A,
    MAL_PDP_FAIL_MISSING_UKNOWN_APN = 0x1B,            /* no retry */
    MAL_PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE = 0x1C,      /* no retry */
    MAL_PDP_FAIL_USER_AUTHENTICATION = 0x1D,           /* no retry */
    MAL_PDP_FAIL_ACTIVATION_REJECT_GGSN = 0x1E,        /* no retry */
    MAL_PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED = 0x1F,
    MAL_PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED = 0x20,  /* no retry */
    MAL_PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED = 0x21, /* no retry */
    MAL_PDP_FAIL_SERVICE_OPTION_OUT_OF_ORDER = 0x22,
    MAL_PDP_FAIL_NSAPI_IN_USE = 0x23,                  /* no retry */
    MAL_PDP_FAIL_REGULAR_DEACTIVATION = 0x24,          /* restart radio */
    MAL_PDP_FAIL_ONLY_IPV4_ALLOWED = 0x32,             /* no retry */
    MAL_PDP_FAIL_ONLY_IPV6_ALLOWED = 0x33,             /* no retry */
    MAL_PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED = 0x34,
    MAL_PDP_FAIL_PROTOCOL_ERRORS   = 0x6F,             /* no retry */

    /* Not mentioned in the specification */
    MAL_PDP_FAIL_VOICE_REGISTRATION_FAIL = -1,
    MAL_PDP_FAIL_DATA_REGISTRATION_FAIL = -2,

   /* reasons for data call drop - network/modem disconnect */
    MAL_PDP_FAIL_SIGNAL_LOST = -3,
    MAL_PDP_FAIL_PREF_RADIO_TECH_CHANGED = -4,/* preferred technology has changed, should retry
                                             with parameters appropriate for new technology */
    MAL_PDP_FAIL_RADIO_POWER_OFF = -5,        /* data call was disconnected because radio was resetting,
                                             powered off - no retry */
    MAL_PDP_FAIL_TETHERED_CALL_ACTIVE = -6,   /* data call was disconnected by modem because tethered
                                             mode was up on same APN/data profile - no retry until
                                             tethered call is off */

    MAL_PDP_FAIL_ERROR_UNSPECIFIED = 0xffff,  /* retry silently */
} MAL_DataCallFailCause;


typedef enum {
    DM_MOBILE_3GPP = 1,
    DM_WIFI = 2,
    DM_MOBILE_3GPP2 = 3,
} dm_rat_type_t;

typedef struct {
	char ext_module_name[STRING_SIZE];
} dm_req_notification_t;


typedef enum {
	MAL_AUTHTYPE_NONE = 0,
	MAL_AUTHTYPE_PAP = 1,
	MAL_AUTHTYPE_CHAP = 2,
	MAL_AUTHTYPE_PAP_CHAP = 3,
} auth_type_t;

typedef enum {
	MAL_IPV4 = 0,
	MAL_IPV6 = 1,
	MAL_IPV4V6 = 2,
} ip_type_t; 

typedef struct {
    //0: QCI is selected by network
    //[1-4]: value range for guaranteed bit rate Traffic Flows
    //[5-9]: value range for non-guarenteed bit rate Traffic Flows
    //[128-254]: value range for Operator-specific QCIs
    int qci; //class of EPS QoS
    int dlGbr; //downlink guaranteed bit rate
    int ulGbr; //uplink guaranteed bit rate
    int dlMbr; //downlink maximum bit rate
    int ulMbr; //uplink maximum bit rate
} MAL_Qos;

typedef struct {

    char radioType[STRING_SIZE];
    char profile[STRING_SIZE];
    char apn[STRING_SIZE];
    char username[STRING_SIZE];
    char passwd[STRING_SIZE];
    int authType;   // auth_type_t
    int protocol;   // ip_type_t
    int interfaceId; // RILD_Req, aka fake
    int retryCount; // SDC retry cnt

    int mIsValid;
    MAL_Qos qos;
    int emergency_ind;
    int pcscf_discovery_flag;
    int signaling_flag;
    int isHandover;
    int ran_type; // for VzW IMS on eHRPD
}MAL_Setup_Data_Call_Struct;

typedef struct {
	int valid;
	MAL_Qos qos;
	int emergency_ind;
	int pcscf_discovery_flag;
	int signaling_flag;
	int is_handover;
	int assigned_rate;  // 0: UNSPEC, 1: LTE, 2: WIFI
	
} dm_ims_para_info_t;


typedef struct {
    int id;
    int precedence;
    int direction;
    int networkPfIdentifier;
    int bitmap;
    char address[MAX_IPV6_ADDRESS_LENGTH];
    char mask[MAX_IPV6_ADDRESS_LENGTH];
    int protocolNextHeader;
    int localPortLow;
    int localPortHigh;
    int remotePortLow;
    int remotePortHigh;
    int spi;
    int tos;
    int tosMask;
    int flowLabel;
} MAL_PktFilter;

typedef struct {
    int authTokenNumber;
    int authTokenList[16];
    int flowIdNumber;
    int flowIdList[4][4];
} MAL_AuthToken;

typedef struct {
    int linkedPfNumber;
    int linkedPfList [16];
    int authtokenFlowIdNumber;
    MAL_AuthToken authtokenFlowIdList[4];
} MAL_TftParameter;

typedef struct {
    int operation;
    int pfNumber;
    MAL_PktFilter pfList [16]; //for response, just send necessary length. check responseSetupDedicateDataCall in ril.cpp
    MAL_TftParameter tftParameter;
} MAL_Tft;


typedef struct {
	unsigned short  pcscfNum;
	unsigned short  pcscfIpv6Mask;
	unsigned char   pcscf[MAX_PCSCF_NUMBER][16];
} dm_pcscf_t;

typedef struct {
	int             ddcId;
	int             interfaceId;
	int             primaryCid;
	int             cid;        /* Context ID, uniquely identifies this call */
	int             active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
	int             signalingFlag;
	int             bearerId;
	int             failCause;
	int             hasQos;
	MAL_Qos         qos;
	int             hasTft;
	MAL_Tft         tft;
	dm_pcscf_t pcscfs;
} MAL_Dedicate_Data_Call_Struct;

#ifdef APN_TABLE_SUPPORT
typedef struct {
	unsigned entry;
	apn_info_t info;
} dm_req_apn_edit_t;


typedef struct {
	unsigned entry;
	apn_para_t para;
} dm_req_apn_edit_para_t;
#endif

typedef struct  {
	int status;     /* A MAL_DataCallFailCause, 0 which is PDP_FAIL_NONE if no error */
	int suggestedRetryTime; /* If status != 0, this fields indicates the suggested retry
					       back-off timer value RIL wants to override the one
					       pre-configured in FW.
					       The unit is miliseconds.
					       The value < 0 means no value is suggested.
					       The value 0 means retry should be done ASAP.
					       The value of INT_MAX(0x7fffffff) means no retry. */
	int cid;        /* interface ID, uniquely identifies this call */
	int active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
	int mtuSize;    /* 0 is default and query fail*/
	int type;       // ip_type_t 
	                            /* One of the PDP_type values in TS 27.007 section 10.1.1.
				       For example, "IP", "IPV6", "IPV4V6", or "PPP". If status is
				       PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED this is the type supported
				       such as "IP" or "IPV6" */
	char ifname[STRING_SIZE];     /* The network interface name */
	unsigned ipv4Addr;
	unsigned char ipv6Addr[16];
	/*char *          addresses;*/  /* A space-delimited list of addresses with optional "/" prefix length,
				       e.g., "192.0.1.3" or "192.0.1.11/16 2001:db8::1/64".
				       May not be empty, typically 1 IPv4 or 1 IPv6 or
				       one of each. If the prefix length is absent the addresses
				       are assumed to be point to point with IPv4 having a prefix
				       length of 32 and IPv6 128. */
	/*unsigned dnsNum;			     
	unsigned dnsIpv6Mask;	
	unsigned char dnses[MAX_NUM_DNS_ADDRESS_NUMBER][16];    */
	unsigned ipv4DnsNum;			     
	unsigned char ipv4Dnses[MAX_NUM_DNS_ADDRESS_NUMBER][4];     
	unsigned ipv6DnsNum;			     
	unsigned char ipv6Dnses[MAX_NUM_DNS_ADDRESS_NUMBER][16];     

	/*char *          dnses;*/      /* A space-delimited list of DNS server addresses,
				       e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
				       May be empty. */

	unsigned ipv4Gateway;
	unsigned char ipv6Gateway[16];

	/*char *          gateways;*/   /* A space-delimited list of default gateway addresses,
				       e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
				       May be empty in which case the addresses represent point
				       to point connections. */
	//VoLTE
	//unsigned char*                           pcscf;
	unsigned network_id;
	int                             concatenateNum;
	MAL_Dedicate_Data_Call_Struct   concatenate[CONCATENATE_NUM];
	int                             defaultBearerValid;
	MAL_Dedicate_Data_Call_Struct   defaultBearer;
      int eran_type;  // 1:RDS_RAN_MOBILE_3GPP, 2:RDS_RAN_WIFI

} MAL_Data_Call_Response_v11;

typedef struct {
	char apn[STRING_SIZE];
	int protocol;   // ip_type_t
	int authType;   // auth_type_t 
	char username[STRING_SIZE];
	char passwd[STRING_SIZE];
	char operatorNumeric[STRING_SIZE];
	int canHandleIms;
} MAL_InitialAttachApn;

typedef struct {
	int id; //InterfaceID (RILD_Req-1, aka real)
	int cause;
} dm_req_t;

typedef dm_req_t dm_ind_t; // InterfaceId RILD Req-1, aka real

typedef struct {
	int ril_status;
} dm_resp_t;

typedef struct {
	int unused;
} dm_dummy_req_t;


typedef struct {
	int on_off;	// 0: cfun=0, > 0: cfun=1
} dm_req_radio_power_t;

typedef struct {
	int setting;    // dm_data_att_t
} dm_req_allow_data_t;

typedef dm_resp_t dm_resp_allow_data_t;

typedef dm_resp_t dm_resp_radio_power_t;


#ifdef APN_TABLE_SUPPORT
typedef struct {
	int mode; // apn_mode_t
} dm_req_apn_set_mode_t;
#endif


typedef MAL_InitialAttachApn dm_req_set_initial_attach_apn_t;
typedef dm_resp_t dm_resp_set_initial_attach_apn_t;
typedef dm_resp_t dm_resp_ho_t;

typedef MAL_Setup_Data_Call_Struct dm_req_setup_data_call_t;

typedef struct { 
	int ril_status;
	MAL_Data_Call_Response_v11 resp;
} dm_resp_setup_data_call_t;

typedef MAL_Dedicate_Data_Call_Struct dm_req_modify_data_call_t; 

typedef struct {
	int ril_status;
	MAL_Dedicate_Data_Call_Struct resp;
} dm_resp_modify_data_call_t; 

typedef MAL_Dedicate_Data_Call_Struct dm_req_setup_dedicate_data_call_t ; 

typedef struct {
	int ril_status;
	short total_event;
	short event_seq;
	MAL_Dedicate_Data_Call_Struct resp;
} dm_resp_setup_dedicate_data_call_t ; 

typedef dm_req_t dm_req_deactivate_data_call_t;	// interfaceId for RILD //RILD_Req-1 , aka real
typedef dm_resp_t dm_resp_deactivate_data_call_t;

typedef dm_req_t dm_req_deactivate_dedicate_data_call_t;	// cid
typedef dm_resp_t dm_resp_deactivate_dedicate_data_call_t;

typedef dm_req_t dm_req_abort_setup_data_call_t; // ddcId
typedef dm_resp_t dm_resp_abort_setup_data_call_t; 

typedef dm_req_t dm_req_pcscf_discovery_pco_t; // cid

typedef struct {
	int ril_status;
	dm_pcscf_t resp;
} dm_resp_pcscf_discovery_pco_t; 

typedef dm_dummy_req_t dm_req_last_data_call_fail_cause_t; 

typedef struct {
	int ril_status;
	int fail_cause;
} dm_resp_last_data_call_fail_cause_t; 

typedef dm_dummy_req_t dm_req_data_call_list_t;

typedef struct {
	int ril_status;	
	short total_event;
	short event_seq;
	MAL_Data_Call_Response_v11 resp;
} dm_resp_data_call_list_t;

typedef struct {
	int ril_status;
	short total_event;
	short event_seq;
	MAL_Data_Call_Response_v11 resp;
} dm_urc_data_call_list_changed_t;

typedef struct {
	int ril_status;
	MAL_Dedicate_Data_Call_Struct resp;
} dm_urc_dedicate_bearer_activated_t;

typedef struct{
	int ril_status;
	MAL_Dedicate_Data_Call_Struct resp;
} dm_urc_dedicate_bearer_modified_t ;

typedef struct{
	int ril_status;
	int cid;
} dm_urc_dedicate_bearer_deactivated_t;     // should be equal to rds_ddc_ind_t

typedef struct{
	int cid;
} dm_urc_default_bearer_modified_t ;

typedef struct{
	int cid;
} dm_req_if_status_t;



typedef enum {
	IF_FAIL = -1,
	IF_SUCC = 0,
} dm_if_rval_t;

typedef struct{
	int ril_status;
	int rval;   // dm_if_rval_t
	
	char ifname[STRING_SIZE];
	int ip_type;    // ip_type_t
	
	unsigned ipv4_addr;
	unsigned ipv4_dns_num;
	unsigned ipv4_dns[2];

	unsigned char ipv6_addr[16];
	unsigned ipv6_dns_num;
	unsigned char ipv6_dns[2][16];
	
	unsigned prefix_len;
	unsigned prefer_lifetime;
	unsigned valid_lifetime;
	
} dm_resp_if_status_t;


typedef struct{
	int type;
} dm_req_netlink_t;


typedef struct {
	int cid;
} dm_if_rat_cell_info_req_t;


typedef struct {
	int cid;
} dm_req_bearer_t;


typedef struct {
	char ifname[STRING_SIZE];
	unsigned network_id;
} dm_req_if_t;

typedef struct{
	int ril_status;
	MAL_Dedicate_Data_Call_Struct resp;
} dm_resp_bearer_t;

typedef struct {
    int interface_id;
} dm_req_ims_para_info_t;

typedef struct {
	/*imc_rat_cell_info_struct*/
	unsigned rat_type;
	char plmn[8];
	char lac[8];
	char ci[12];
	char cell_id[64];
	int is_ems_support;
	int ril_status; 
} dm_if_rat_cell_info_resp_t;



typedef struct {
    unsigned char ucho_status; // 0: start_ho 1: stop_ho
    unsigned char fgho_result; // 0: fail(RDS_FALSE) 1: success(RDS_TRUE)
    unsigned char uccid; // cid, for RILD
    unsigned char esource_ran_type; // ran_type_e
    unsigned char etarget_ran_type; // ran_type_e
} dm_ho_status_t;

#endif

