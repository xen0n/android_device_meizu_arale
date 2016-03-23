/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef RIL_DATA_H
#define RIL_DATA_H 1

#define SM_CAUSE_BASE 3072 // Defined in AT_DOCUMENT_full.doc (0xC00)
#define SM_CAUSE_END SM_CAUSE_BASE + 0x72
#define ESM_CAUSE_BASE 3328 // ESM cause    (0xD00)
#define ESM_CAUSE_END ESM_CAUSE_BASE + 0xFF

#define ME_PDN_URC "ME PDN ACT"
#define ME_DEDICATE_URC "ME ACT"
#define NW_DEDICATE_URC "NW ACT"

#define AUTHTYPE_NONE       2
#define AUTHTYPE_PAP        0
#define AUTHTYPE_CHAP       1
#define AUTHTYPE_PAP_CHAP   3
#define AUTHTYPE_NOT_SET    (-1)

#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define INVALID_CID -1

#define IPV4        0
#define IPV6        1
#define IPV4V6      2

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define MAX_IPV4_ADDRESS 17
#define MAX_IPV6_ADDRESS 42
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#define MAX_CCMNI_NUMBER 8

#define DATA_STATE_ACTIVE 2
#define DATA_STATE_LINKDOWN 1
#define DATA_STATE_INACTIVE 0


//[C2K][IRAT] Indicate the PDN is deactivated fail before.
#define DATA_STATE_NEED_DEACT -1

#define MAX_L2P_VALUE_LENGTH 20
#define L2P_VALUE_UPS "M-UPS"
#define L2P_VALUE_CCMNI "M-CCMNI"
#define L2P_VALUE_MBIM "M-MBIM"

#define DEFAULT_MAX_PDP_NUM 6

#define WIFI_MAX_PDP_NUM 10
#define WIFI_CID_OFFSET 100

#define DONGLE_MAX_PDP_NUM 2

#define REQUEST_DATA_DETACH 0
#define REQUEST_DATA_ATTACH 1

/* +ESCRI: URC result code */
#define SCRI_RESULT_REQ_SENT 0
#define SCRI_CS_SESSION_ONGOING 1
#define SCRI_PS_SIGNALLING_ONGOING 2
#define SCRI_NO_PS_DATA_SESSION 3
#define SCRI_REQ_NOT_SENT 4
#define SCRI_NOT_ALLOWED 5
#define SCRI_RAU_ENABLED 6

#define CCCI_IOC_MAGIC    'C'
#ifdef CCCI_IOC_FORCE_FD
    #undef CCCI_IOC_FORCE_FD
#endif
#define CCCI_IOC_FORCE_FD    _IOW(CCCI_IOC_MAGIC, 16, unsigned int)

#define REPLACE_APN_FLAG (-1)
#define PROPERTY_RE_IA_FLAG "persist.radio.re.ia.flag"
#define PROPERTY_RE_IA_APN "persist.radio.re.ia-apn"

#define MODEM_RAT_LTE 7

static RIL_SOCKET_ID s_data_ril_cntx[] = {
    RIL_SOCKET_1
    , RIL_SOCKET_2
    , RIL_SOCKET_3
    , RIL_SOCKET_4
};

static RILChannelId sCmdChannel4Id[] = {
    RIL_CMD_4
    , RIL_CMD2_4
    , RIL_CMD3_4
    , RIL_CMD4_4
};

#define disable_test_load

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

#define PROPERTY_IA "persist.radio.ia"
#define PROPERTY_IA_APN "persist.radio.ia-apn"
#define PROPERTY_IA_FROM_NETWORK "ril.ia.network"
#define PROPERTY_IA_APN_SET_ICCID "ril.ia.iccid"
#define INVALID_APN_VALUE "this_is_an_invalid_apn"
#define PROPERTY_IA_PASSWORD_FLAG "persist.radio.ia-pwd-flag"
#define PROPERTY_TEMP_IA "ril.radio.ia"
#define PROPERTY_TEMP_IA_APN "ril.radio.ia-apn"
#define PROPERTY_RIL_DATA_ICCID "persist.radio.data.iccid"
#define PROPERTY_MOBILE_DATA_ENABLE "persist.radio.mobile.data"
#define PROPERTY_DATA_ALLOW_SIM "ril.data.allow"
#define PROPERTY_REQUEST_FROM_MAL "ril.data.mal"
#define PROPERTY_HANDOVER_START_IND "ril.volte.hos.interface"
#define PROPERTY_HANDOVER_STOP_IND "ril.volte.hoe.interface"
#define ATTACH_APN_NOT_SUPPORT 0
#define ATTACH_APN_PARTIAL_SUPPORT 1
#define ATTACH_APN_FULL_SUPPORT 2

#define MTK_REQUEST_SETUP_DATA_CALL(data,datalen,token) \
        requestSetupDataCall(data,datalen,token)
#define MTK_REQUEST_DEACTIVATE_DATA_CALL(data,datalen,token) \
        requestDeactiveDataCall(data,datalen,token)
#define MTK_REQUEST_LAST_DATA_CALL_FAIL_CAUSE(data,datalen,token) \
        requestLastDataCallFailCause(data,datalen,token)
#define MTK_REQUEST_DATA_CALL_LIST(data,datalen,token) \
        requestDataCallList(data,datalen,token)
#define MTK_REQUEST_SET_INITIAL_ATTACH_APN(data,datalen,token) \
        requestSetInitialAttachApn(data,datalen,token)
#define MTK_REQUEST_SET_SCRI(data, datalen, token) \
        requestSetScri(data, datalen, token)
//[New R8 modem FD]
#define MTK_REQUEST_FD_MODE(data, datalen, token) \
        requestSetFDMode(data, datalen, token)
#define MTK_UNSOL_DATA_CALL_LIST_CHANGED(token) \
        onDataCallListChanged(token)
#define MTK_REQUEST_DATA_IDLE(data, datalen, token) \
        requestDataIdle(data, datalen, token)
#define MTK_REQUEST_ALLOW_DATA(data, datalen, token) \
        requestAllowData(data, datalen, token)

#define MTK_REQUEST_SETUP_PPP_CALL(data, datalen, token) \
        requestSetupPPPDataCall(data, datalen, token)
//CC33
#define MTK_REQUEST_SET_DATA_ON_TO_MD(data, datalen, token) \
        requestSetDataOnToMD(data, datalen, token)
#define MTK_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE(data, datalen, token) \
        requestSetRemoveRestrictEutranMode(data, datalen, token)
// M: [LTE][Low Power][UL traffic shaping] Start
#define MTK_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT(data, datalen, token) \
        requestSetLteAccessStratumReport(data, datalen, token)
#define MTK_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER(data, datalen, token) \
        requestSetLteUplinkDataTransfer(data, datalen, token)
// M: [LTE][Low Power][UL traffic shaping] End


/// M: IMS/EPDG @{
#define MTK_RIL_REQUEST_HANDOVER_IND(data, datalen, token) \
        requestHandoverInd(data, datalen, token);

#define UNSPECIFIED_IPV6_GATEWAY    "::"

#include <entity/datamngr/datamngr_api.h>
#include <entity/rds/rds_if.h>

typedef dm_req_t epdgDeactReq_t;
typedef rr_ddc_cnf_t epdgDeactRsp_t;
typedef dm_resp_setup_data_call_t queryRatRsp_t;

#define HO_STATUS_INVALID   (-1)
#define HO_START  (0)
#define HO_STOP (1)
#define REASON_RDS_DEACT_LTE_PDN (100)

typedef enum {
    PDN_SETUP_THRU_MOBILE = 0,
    PDN_SETUP_THRU_WIFI = 1,
    PDN_SETUP_THRU_MAX,
    PDN_SETUP_THRU_ERR = -1
} pdn_setup_thru_type_e;

typedef enum {
    E_HO_TO_LTE = 0,
    E_HO_TO_WIFI = 1,
    E_HO_FROM_LTE = 0,
    E_HO_FROM_WIFI = 1,
    E_HO_UNKNOWN = -1
} pdn_ho_to_e;

#define DACONCPTR(src, dst) const char *dst = src.dst
#define DACONCPTR_P(src, dst) const char *dst = src->dst
#define DACONINT(src, dst) const int dst = src.dst
#define DACONINT_P(src, dst) const int dst = src->dst
#define DAINT_P(src, dst) int dst = src->dst

#define INIT_REQ_SETUP_CNF(CONFIG)      \
    DACONCPTR_P(CONFIG, requestedApn);  \
    DACONCPTR_P(CONFIG, username);      \
    DACONCPTR_P(CONFIG, password);      \
    DACONCPTR_P(CONFIG, profile);       \
    DACONINT_P(CONFIG, authType);       \
    DAINT_P(CONFIG, protocol);       \
    DACONINT_P(CONFIG, interfaceId);    \

typedef struct {
    int isHandOver;
    int eran_type;
} epdgConfig_t;

typedef struct {
    int ucho_status;  // 0: start_ho 1: stop_ho
    int fgho_result;  // 0: fail(RDS_FALSE) 1: success(RDS_TRUE)
    int interfaceId;
    int esource_ran_type;  // ran_type_e
    int etarget_ran_type;  // ran_type_e
} epdgHandoverStatus_t;

typedef struct {
    const char *radioType;
    const char* requestedApn;
    const char *username;
    const char *password;
    int authType;
    int protocol;
    int interfaceId;
    int availableCid;   //for epdg
    int retryCount;
    const char *profile;
    void *pQueryRatRsp;
} reqSetupConf_t;


typedef void ( *pSetupDataCallFunc) (const reqSetupConf_t *, const void *, RIL_Token);

typedef enum {
    E_SETUP_DATA_CALL_OVER_IPV6 = 0,
    E_SETUP_DATA_CALL_EMERGENCY,
    E_SETUP_DATA_CALL_FALLBACK,
    E_SETUP_DATA_CALL_OVER_EPDG,
    E_SETUP_DATA_CALL_HO_EPDG,      // 5

    E_SETUP_DATA_CALL_FUNC_CNT,
    E_SETUP_DATA_CALL_FUNC_ERR = -1
} setup_data_call_type_e;
/// M: IMS/EPDG @}


#define UNUSED(x) (x)   //eliminate "warning: unused parameter"

extern void requestSetupDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestOrSendDataCallListIpv6(RILChannelCtx* rilchnlctx, RIL_Token *t, RIL_SOCKET_ID rilid);
extern void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t);
extern void requestDataCallList(void * data, size_t datalen, RIL_Token t);
extern void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t);
extern void requestDataIdle(void* data, size_t datalen, RIL_Token t);
extern void requestAllowData(void* data, size_t datalen, RIL_Token t);
extern void requestSetScri(void* data, size_t datalen, RIL_Token t);
extern void requestSetDataOnToMD(void* data, size_t datalen, RIL_Token t);
extern void requestSetRemoveRestrictEutranMode(void* data, size_t datalen, RIL_Token t);
extern void onScriResult(char *s, RIL_SOCKET_ID rid);
extern void onDataCallListChanged(RIL_Token t);
extern void onGPRSDeatch(char* urc, RIL_SOCKET_ID rid);
// M: [LTE][Low Power][UL traffic shaping] Start
extern void requestSetLteAccessStratumReport(void* data, size_t datalen, RIL_Token t);
extern void requestSetLteUplinkDataTransfer(void* data, size_t datalen, RIL_Token t);
// M: [LTE][Low Power][UL traffic shaping] End

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

extern int getAuthTypeInt(int authTypeInt);

//VoLTE
extern void requestSetupDedicateDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestDeactivateDedicateDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestModifyDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestAbortSetupDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestPcscfPco(void *data, size_t datalen, RIL_Token t);
extern void requestClearDataBearer(void *data, size_t datalen, RIL_Token t);

extern int isPsDualTalkMode();

/// M: IMS/EPDG @{
extern void requestHandoverInd(void *data, size_t datalen, RIL_Token t);
/// M: IMS/EPDG @}

/// M: WiFi disconnect indication @{
extern void requestWifiDisconnectInd(void *data, size_t datalen, RIL_Token t);
/// @}

//FALLBACK PDP retry
#define PDP_FAIL_FALLBACK_RETRY (-1000)

typedef enum {
    SM_OPERATOR_BARRED = 0b00001000,
    SM_MBMS_CAPABILITIES_INSUFFICIENT = 0b00011000,
    SM_LLC_SNDCP_FAILURE = 0b00011001,
    SM_INSUFFICIENT_RESOURCES = 0b00011010,
    SM_MISSING_UNKNOWN_APN = 0b00011011,
    SM_UNKNOWN_PDP_ADDRESS_TYPE = 0b00011100,
    SM_USER_AUTHENTICATION_FAILED = 0b00011101,
    SM_ACTIVATION_REJECT_GGSN = 0b00011110 ,
    SM_ACTIVATION_REJECT_UNSPECIFIED = 0b00011111,
    SM_SERVICE_OPTION_NOT_SUPPORTED = 0b00100000,
    SM_SERVICE_OPTION_NOT_SUBSCRIBED = 0b00100001,
    SM_SERVICE_OPTION_OUT_OF_ORDER = 0b00100010,
    SM_NSAPI_IN_USE = 0b00100011,
    SM_REGULAR_DEACTIVATION = 0b00100100,
    SM_QOS_NOT_ACCEPTED = 0b00100101,
    SM_NETWORK_FAILURE = 0b00100110,
    SM_REACTIVATION_REQUESTED = 0b00100111,
    SM_FEATURE_NOT_SUPPORTED = 0b00101000,
    SM_SEMANTIC_ERROR_IN_TFT = 0b00101001,
    SM_SYNTACTICAL_ERROR_IN_TFT = 0b00101010,
    SM_UNKNOWN_PDP_CONTEXT = 0b00101011,
    SM_SEMANTIC_ERROR_IN_PACKET_FILTER = 0b00101100,
    SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER = 0b00101101,
    SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED = 0b00101110,
    SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT = 0b00101111,
    SM_BCM_VIOLATION = 0b00110000,
    SM_LAST_PDN_DISC_NOT_ALLOWED = 0b00110001,
    SM_ONLY_IPV4_ALLOWED = 0b00110010,
    SM_ONLY_IPV6_ALLOWED = 0b00110011,
    SM_ONLY_SINGLE_BEARER_ALLOWED = 0b00110100,
    ESM_INFORMATION_NOT_RECEIVED = 0b00110101,
    SM_PDN_CONNECTION_NOT_EXIST = 0b00110110,
    SM_MULTIPLE_PDN_APN_NOT_ALLOWED = 0b00110111,
    SM_COLLISION_WITH_NW_INITIATED_REQUEST = 0b00111000,
    ESM_UNSUPPORTED_QCI_VALUE = 0b00111011,
    SM_BEARER_HANDLING_NOT_SUPPORT = 0b00111100,
    SM_MAX_PDP_NUMBER_REACHED = 0b01000001,
    SM_APN_NOT_SUPPORT_IN_RAT_PLMN = 0b01000010,
    SM_INVALID_TRANSACTION_ID_VALUE = 0b01010001,
    SM_SEMENTICALLY_INCORRECT_MESSAGE = 0b01011111,
    SM_INVALID_MANDATORY_INFO = 0b01100000,
    SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED = 0b01100001,
    SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100010,
    SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED = 0b01100011,
    SM_CONDITIONAL_IE_ERROR = 0b01100100,
    SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100101,
    SM_PROTOCOL_ERROR = 0b01101111,
    SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT = 0b01110000
} AT_SM_Error;

#define ERROR_CAUSE_GENERIC_FAIL 14

typedef struct {
    int interfaceId;
    int primaryCid;
    int cid;
    int ddcId;  // for dedicate bearer
    int isDedicateBearer;
    int isEmergency;  // 0: normal, 1: emergency PDN
    int active;  // 0: inactive, 1: link down, 2: active
    int signalingFlag;
    char apn[128];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    Qos qos;
    Tft tft;
    int bearerId;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int isNoDataPdn;  // 0: false, 1: true; for No Data PDN check
    int mtu;
    int eran_type;  /* 0: UNSPEC, 1: MOBILE_3GPP, 2: WIFI, 3: MOBILE_3GPP2
                             4: TYPE_MAX, 0xFE: RDS_RAN_NONE, 0xFF: RDS_RAN_DENY */
    epdgHandoverStatus_t hoStatus;
    int deactbyNw;  // 0: not deact, 1: deact by nw for HO stop used
    int ho_start_cnt;
    int ho_stop_cnt;
} PdnInfo;

typedef struct {
    int rid;
    char* urc;
    int isModification;
} TimeCallbackParam;

typedef struct {
    int activeCid;
    RILChannelCtx* pDataChannel;
} MePdnActiveInfo;

// [C2K][IRAT] start {@
void requestSetActivePsSimSlot(void *data, size_t datalen, RIL_Token t);
void confirmIratChange(void *data, size_t datalen, RIL_Token t);
void requestDeactivateLinkdownPdn(void *data, size_t datalen, RIL_Token t);

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
	DISABLE_CCMNI = 0,
	ENABLE_CCMNI=1
} CCMNI_STATUS;
// [C2K][IRAT] end @}

#endif /* RIL_DATA_H */

