#ifndef C2K_RIL_DATA_H
#define C2K_RIL_DATA_H 1

#include <telephony/ril.h>
#include "atchannel.h"

// CDMA PS fail cause defined by MD3.
typedef enum {
	PS_FEATURE_UNSUPPORTED = 140,
	PS_UNKNOWN_PDP_CONTEXT = 143,
	PS_ALREADY_ACTIVATED = 146,
	PS_NETWORK_FAILURE = 158,
	PS_MAX_PDP_CONTEXT_COUNT_REACHED = 178,

	PS_GENERAL_ERROR = 0x8000,
	PS_UNAUTHORIZED_APN = 0x8001,
	PS_PDN_LIMITED_EXCEEDED = 0x8002,
	PS_NO_PGW_AVALAIABLE = 0x8003,
	PS_PGW_UNREACHABLE = 0x8004,
	PS_PGW_REJECT = 0x8005,
	PS_INSUFICIENT_PARAMETERS = 0x8006,
	PS_RESOURCE_UNAVAILABLE = 0x8007,
	PS_PDN_ID_ALREADY_IN_USE = 0x8008,
	PS_SUBSCRIPTION_LIMITATION = 0x8009,
	PS_PDN_CONN_ALREADY_EXIST_FOR_PDN = 0x800a,
	PS_EMERGENCY_NOT_ALLOWED = 0x800b,
	PS_RECONNECT_NOT_ALLOWED = 0x800c,

	PS_EAPAKA_FAILURE = 0x8080,
	PS_RETRY_TIMER_THROTTLING = 0x8081,
	PS_NETWORK_NO_RESPONSE = 0x8082,
	PS_PDN_ATTACH_ABORT = 0x8083,
	PS_PDN_LINIT_EXCEEDED_IN_UE_SIDE = 0x8084,
	PS_PDN_ID_ALREADY_IN_USE_IN_UE_SIDE = 0x8085,
	PS_INVALID_PDN_ATTACH_REQUEST = 0x8086,
	PS_PDN_REC_FAILURE = 0x8087,
	PS_MAIN_CONN_SETUP_FAILURE = 0x8088,
	PS_BEARER_RESOURCE_UNAVAILABLE = 0x8089,
	PS_OP_ABORT_BY_USER = 0x808a,
	PS_RTT_DATA_CONNECTED = 0x808b,
	PS_EAPAKA_REJECT = 0x808c,
	PS__LCP_NEGO3_FAILURE = 0x808d,
	PS_TCH_SETUP_FAILURE = 0x808e,
	PS_NW_NO_RESPONSE_IN_LCP = 0x808f,
	PS_NW_NO_RESPONSE_IN_AUTH = 0x8090,

	PS_ERROR_UNSPECIFIED = 0x80b0,
	PS_AUTH_FAIL = 0x80b1,
	PS_MIP_PPP_LCP_TIMEOUT = 0x80b2,
	PS_MIP_PPP_NCP_TIMEOUT = 0x80b3,
	PS_SIP_PPP_LCP_TIMEOUT = 0x80b4,
	PS_SIP_PPP_NCP_TIMEOUT = 0x80b5,
	PS_MIP_PPP_OPT_MISMATCH = 0x80b6,
	PS_SIP_PPP_OPT_MISMATCH = 0x80b7,
	PS_MIP_SOLICITATION_TIMEOUT = 0x80b8,
	PS_MIP_RRQ_TIMEOUT = 0x80b9,
	PS_MIP_RRP_ERROR = 0x80ba,
	PS_MIP_PPP_LCP_FAIL = 0x80bb,
	PS_SIP_PPP_LCP_FAIL = 0x80bc,
	PS_MIP_PPP_NCP_FAIL = 0x80bd,
	PS_SIP_PPP_NCP_FAIL = 0x80be,
	PS_A12_AUTH_FAIL = 0x80bf,
	PS_EAPAKA_AUTH_FAIL = 0x80c0,
	PS_MIP_ADMIN_PROHIB_FAIL = 0x80c1,
	PS_MULTI_TCH_FAIL = 0x80c2,
	PS_A12_LCP_TIMEOUT = 0x80c3,

	PS_BUSY = 0x80e0,
	PS_INVALID_PARAMETER = 0x80e1,

	PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED = 0x80e2,

} AT_PS_ERROR;

#define MODIFY_CAUSE_PPP_RENEGOTIATION 99

#define ME_PDN_URC " ME PDN ACT "
#define ME_DEDICATE_URC " ME ACT "
#define NW_DEDICATE_URC " NW ACT "
#define ME_PDN_DEACT_URC " ME PDN DEACT "
#define NW_PDN_DEACT_URC " NW PDN DEACT "
#define NW_MODIFY_URC " NW MODIFY "

#define INVALID_CID -1

#define AUTHTYPE_NONE       2
#define AUTHTYPE_PAP        0
#define AUTHTYPE_CHAP       1
#define AUTHTYPE_PAP_CHAP   3
#define AUTHTYPE_NOT_SET    (-1)

#define IPV4        0
#define IPV6        1
#define IPV4V6      2

#define MAX_IPV4_ADDRESS_LENGTH 17
//xxx.xxx.xxx.xxx
#define MAX_IPV6_ADDRESS_LENGTH 65
//xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
//xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx

//VoLTE
#define MAX_PCSCF_NUMBER 6
#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define REQUEST_DATA_DETACH 0
#define REQUEST_DATA_ATTACH 1
#define REQUEST_DATA_DETACH_NOT_RESPONSE 2
#define REQUEST_DATA_ATTACH_NOT_RESPONSE 3

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define MAX_IPV4_ADDRESS 17
#define MAX_IPV6_ADDRESS 42
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#define MAX_CCMNI_NUMBER 8

#define MAX_L2P_VALUE_LENGTH 20
#define L2P_VALUE_CCMNI "M-CCMNI"

#define PROPERTY_IA "persist.radio.ia"
#define PROPERTY_IA_APN "persist.radio.ia-apn"
#define PROPERTY_IA_FROM_NETWORK "ril.ia.network"

#define ATTACH_APN_NOT_SUPPORT 0
#define ATTACH_APN_PARTIAL_SUPPORT 1
#define ATTACH_APN_FULL_SUPPORT 2

#define PROPERTY_RIL_DATA_ICCID "persist.radio.data.iccid"
#define PROPERTY_SVLTE_SIM1_ICCID_LTE "ril.iccid.sim1_lte"
#define PROPERTY_SVLTE_SIM1_ICCID_C2K "ril.iccid.sim1_c2k"
#define PROPERTY_MOBILE_DATA_ENABLE "persist.radio.mobile.data"
#define PROPERTY_4G_SIM "persist.radio.simswitch"

#define SIM_ID_1 1
#define SIM_ID_2 2

#define APN_CTNET "ctnet"

static char PROPERTY_ICCID_SIM[4][25] = {
    "ril.iccid.sim1",
    "ril.iccid.sim2",
    "ril.iccid.sim3",
    "ril.iccid.sim4",
};

#ifdef AT_RSP_FREE
#undef AT_RSP_FREE
#endif

#define AT_RSP_FREE(rsp)    \
if (rsp) {                  \
    at_response_free(rsp);  \
    rsp = NULL;             \
}

#ifdef FREEIF
#undef FREEIF
#endif

#define FREEIF(data)    \
if (data != NULL) {     \
    free(data);         \
    data = NULL;        \
}

#define IPV6_PREFIX "FE80:0000:0000:0000:"
#define NULL_ADDR ""

/* data connect state */
enum {
	DATA_STATE_INACTIVE=0,
    DATA_STATE_LINKDOWN,
    DATA_STATE_ACTIVE,
    DATA_STATE_NEED_DEACT
};

typedef struct {
    int interfaceId;
    int primaryCid;
    int cid;
    int ddcId; //for dedicate bearer
    int isDedicateBearer;
	int isEmergency; //0: normal, 1: emergency PDN
    int active; //0: inactive, 1: link down, 2: active
    int signalingFlag;
    char apn[128];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int mtu;
    int bearerId;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
} PdnInfo;

typedef enum {
	DISABLE_CCMNI = 0,
	ENABLE_CCMNI=1
} CCMNI_STATUS;

typedef enum {
	IRAT_NO_RETRY = 0,
	IRAT_NEED_RETRY = 1
} IRAT_RETRY_STATUS;

typedef enum {
	IRAT_ACTION_UNKNOWN = 0,
	IRAT_ACTION_SOURCE_STARTED = 1,
	IRAT_ACTION_TARGET_STARTED = 2,
	IRAT_ACTION_SOURCE_FINISHED = 3,
	IRAT_ACTION_TARGET_FINISHED = 4
} PDN_IRAT_ACTION;

typedef enum {
	IRAT_TYPE_UNKNOWN = 0,
	IRAT_TYPE_LTE_EHRPD = 1,
	IRAT_TYPE_LTE_HRPD = 2,
	IRAT_TYPE_EHRPD_LTE = 3,
	IRAT_TYPE_HRPD_LTE = 4,
	IRAT_TYPE_FAILED = 5
} PDN_IRAT_TYPE;

typedef enum {
	IRAT_PDN_STATUS_UNKNOWN = -1,
	IRAT_PDN_STATUS_SYNCED = 0,
	IRAT_PDN_STATUS_DEACTED = 1,
	IRAT_PDN_STATUS_REACTED = 2
} IRAT_PDN_STATUS;

typedef enum {
	RAT_UNKNOWN = 0,
	RAT_1XRTT = 1,
	RAT_HRPD = 2,
	RAT_EHRPD = 3,
	RAT_LTE = 4
} PDN_RAT_VALUE;

typedef enum {
	SUSPEND_DATA_TRANSFER = 0,
	RESUME_DATA_TRANSFER = 1,
} IRAT_DATA_TRANSFER_STATE;

// IRAT sync PDN support.
typedef struct {
    int interfaceId;
    int cid;
    char apn[128];
	char addressV4[MAX_IPV4_ADDRESS_LENGTH];
	char addressV6[MAX_IPV6_ADDRESS_LENGTH];
	int pdnStatus;
} SyncPdnInfo;

void requestSetupDataCall(void * data, size_t datalen, RIL_Token t);
void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t);
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t);
void requestDataCallList(void * data, size_t datalen, RIL_Token t);
void confirmIRatChange(void *data, size_t datalen, RIL_Token t);
void onDataCallListChanged();
void allowDataRequest();

void rilDataInitialization(RILChannelCtx *pChannel);
int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
int rilDataUnsolicited(const char *s, const char *sms_pdu);

#endif
