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

#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#define PROPERTY_RIL_SPECIFIC_SM_CAUSE "ril.specific.sm_cause"
#define PROPERTY_SIM_OPERATOR_NUMERIC "gsm.sim.operator.numeric"

#define DATA_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define PPP_PPP_RUNNING_PROPERTY "init.svc.pppd_gprs"
#define PPP_DEF_RUNNING "running"
#define PPP_DEF_STOPPING "stopping"


#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    const char *CCMNI_IFNAME = "usb";
    const char *LOG_TAG_STR = "RILEXTMD";
#else
    #ifdef MTK_RIL_MD2
        #if __MTK_ENABLE_MD5
            const char *CCMNI_IFNAME = "ccemni";
            const char *LOG_TAG_STR = "RIL";
        #else
            const char *CCMNI_IFNAME = "cc2mni";
            const char *LOG_TAG_STR = "RILMD2";
        #endif /* MTK_ENABLE_MD5 */
    #else /* MTK_RIL_MD2 */
        #if __MTK_ENABLE_MD1
            const char *CCMNI_IFNAME = "ccmni";
            const char *LOG_TAG_STR = "RIL";
        #elif __MTK_ENABLE_MD2
            const char *CCMNI_IFNAME = "cc2mni";
            const char *LOG_TAG_STR = "RILMD2";
        #else
            const char *CCMNI_IFNAME = "ccemni";
            const char *LOG_TAG_STR = "RIL";
        #endif
     #endif
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG LOG_TAG_STR

int sock_fd[MAX_CCMNI_NUMBER] = {0};
int sock6_fd[MAX_CCMNI_NUMBER] = {0};

char TEMP_IA_PASSWORD[256] = {0};

extern int max_pdn_support;
extern int pdnInfoSize;
extern int isCid0Support;

/* Refer to system/core/libnetutils/ifc_utils.c */
extern int ifc_disable(const char *ifname);
extern int ifc_remove_default_route(const char *ifname);
extern int ifc_reset_connections(const char *ifname);

extern void initialCidTable();
extern void clearPdnInfo(PdnInfo* info);
extern int get_protocol_type(const char* protocol);
extern int getAvailableCid();
extern int getAvailableWifiCid();
extern void configureNetworkInterface(int interfaceId, int isUp);
extern void requestSetFDMode(void * data, size_t datalen, RIL_Token t);
extern void onNetworkBearerUpdate(void* param);
extern void onPdnDeactResult(void* param);
extern int handleLastPdnDeactivation (const int isEmergency, const int lastPDNCid, RILChannelCtx* rilchnlctx);
extern int deactivateDataCall(int cid, RILChannelCtx* rilchnlctx);
extern int definePdnCtx(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig, const char *requestedApn,
                    int protocol, int availableCid, int authType, const char *username, const char* password, RILChannelCtx *pChannel);
extern int queryMatchedPdnWithSameApn(const char* apn, int* matchedCidList, RILChannelCtx* rilchnlctx);
extern int getAttachApnSupport();

extern int getDefaultDataSim();
extern int queryCurrentConnType(RILChannelCtx* rilchnlctx);
extern int apnMatcher (const char* out, const char* apn);
extern int activatePdn(int availableCid, int *pActivatedPdnNum, int *pConcatenatedBearerNum,
    int *activatedCidList, int isEmergency, int isNoDataActive, RILChannelCtx *pChannel);
extern int detachPs(RILChannelCtx* pChannel);
extern void onMoDataBarring(char *urc, const RIL_SOCKET_ID rid);
extern int isCidActive(int *cidList, int size, RILChannelCtx * rilchnlctx);


void getIaCache(char* cache);
void setIaCache(const char* iccid, const char* protocol, const int authtype, const char* username, const int canHandleIms, const char* apn);
int doPsAttach(RILChannelCtx *pChannel);
extern int doPsAttachAsync(RILChannelCtx *pChannel);
int isAlwaysAttach();

void getTempIa(char* cache);
void setTempIa(const char* iccid, const char* protocol, const int authtype, const char* username,
        const int canHandleIms, const char* apn);
int querryCurrentRat(RILChannelCtx* rilchnlctx);


void shutdownPPP(RIL_Token t);
int startPPPCall(RIL_Data_Call_Response_v11* response, char* port);

void onReAttachForSettingAttachApn(void* param);

extern RILChannelCtx *openPPPDataChannel(int isBlocking);
extern void closePPPDataChannel();
extern int at_send_command_to_ppp_data_channel(const char *command, ATResponse **pp_outResponse,
                                        RILChannelCtx *p_channel);

extern void purge_data_channel(RILChannelCtx *p_channel);
extern int getMtkShareModemCurrent();
extern void freeDataResponse(RIL_Data_Call_Response_v11* response);
extern void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id);

extern void createDataResponse(int interfaceId, int protocol, RIL_Data_Call_Response_v6* response);
extern void dumpDataResponse(RIL_Data_Call_Response_v6* response, const char* reason);

/// [IMS][EPDG] code start. {@
extern int isRequestTokFromMAL(RIL_Token t);
extern void setRequestFromMALProp(int isMAL);
void dumpDefaultBearerConfig(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig);

extern int getImsParamInfo(int intfId,  RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig);
extern int queryEpdgRat(reqSetupConf_t *pConfig, RIL_Default_Bearer_VA_Config_Struct* pDefaultBearerVaConfig);

extern int deactivatePdnThruEpdg(int interfaceId, int reason);
extern void dumpReqSetupConfig(const reqSetupConf_t * pConfig);

extern pSetupDataCallFunc setupDataCallFunc[E_SETUP_DATA_CALL_FUNC_CNT];
/// [IMS][EPDG] code end. @}



#ifdef MTK_RIL
extern const char *proxyIdToString(int id);
#endif

//Fucntion prototype
void ril_data_ioctl_init(int index);
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr);
void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr);

// For op01 NS-IOT test
extern int rildata_isTestSim();

int receivedSCRI_RAU = 0;
int sendSCRI_RAU = 0;

//Global variables/strcuture
static int disableFlag = 1;
int gprs_failure_cause = 0;

extern int gcf_test_mode;
extern int s_md_off;

extern PdnInfo* pdn_info;
extern PdnInfo* pdn_info_wifi;

static const struct timeval TIMEVAL_0 = {0,0};

#define MAX_DUAL_APN_OPERATORS 20
#define MAX_PLMN_LEN 10
static char g_aDualApnPlmnList[MAX_DUAL_APN_OPERATORS][MAX_PLMN_LEN];
static int g_iNumDualApnPlmn;
#define PROPERTY_OPERATOR_NUMERIC "gsm.operator.numeric"
#define PROPERTY_DATA_NETWORK_TYPE "gsm.network.type"
#define DEFAULT_NODATA_APN "NODATA"

#define CT_OPERATOR_NUMERIC_CDMA "46003"
#define CT_OPERATOR_NUMERIC_GSM "46011"
#define CT_MACAO_OPERATOR_NUMERIC_CDMA "45502"
#define CT_MACAO_OPERATOR_NUMERIC_GSM "45507"

void setIaReplaceFlag(int flag);

typedef struct {
    char apn[PROPERTY_VALUE_MAX];
    RILChannelCtx* pDataChannel;
} OnReAttachInfo;

#define TELSTRA_PDP_RETRY_PLMN "50501"
static int isFallbackPdpRetryRunning(RIL_SOCKET_ID rid, const char* apn, int protocol);

//M:  [C2K][IRAT code start. {@
extern PDN_IRAT_ACTION nIratAction;
extern int nReactPdnCount;
extern void onUpdateIratStatus(void* param);
int onPdnSyncFromSourceRat(void* param);
void onIratEvent(void* param);
void onIratPdnReactSucc(int cid);
//M:  [C2K][IRAT] code end. @}

int disableIpv6Interface(char* filepath) {
    int fd = open(filepath, O_WRONLY);
    if (fd < 0) {
        LOGE("failed to open file (%s)", strerror(errno));
        return -1;
    }
    if (write(fd, "1", 1) != 1) {
        LOGE("failed to write property file (%s)",strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

void ril_data_ioctl_init(int index)
{
    if (sock_fd[index] > 0)
        close(sock_fd[index]);
    sock_fd[index] = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd[index] < 0)
        LOGD("Couldn't create IP socket: errno=%d", errno);
    else
        LOGD("Allocate sock_fd=%d, for cid=%d", sock_fd[index], index+1);

#ifdef INET6
    if (sock6_fd[index] > 0)
        close(sock6_fd[index]);
    sock6_fd[index] = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock6_fd[index] < 0) {
        sock6_fd[index] = -errno;    /* save errno for later */
        LOGD("Couldn't create IPv6 socket: errno=%d", errno);
    } else {
        LOGD("Allocate sock6_fd=%d, for cid=%d", sock6_fd[index], index+1);
    }
#endif
}

/* For setting IFF_UP: ril_data_setflags(s, &ifr, IFF_UP, 0) */
/* For setting IFF_DOWN: ril_data_setflags(s, &ifr, 0, IFF_UP) */
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr)
{
    if(ioctl(s, SIOCGIFFLAGS, ifr) < 0)
        goto terminate;
    ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | set;
    if(ioctl(s, SIOCSIFFLAGS, ifr) < 0)
        goto terminate;
    return;
terminate:
    LOGD("Set SIOCSIFFLAGS Error!");
    return;
}

inline void ril_data_init_sockaddr_in(struct sockaddr_in *sin, const char *addr)
{
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
}

void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr)
{
    LOGD("Configure IPv4 adress :%s", addr);
    ril_data_init_sockaddr_in((struct sockaddr_in *) &ifr->ifr_addr, addr);
    if(ioctl(s, SIOCSIFADDR, ifr) < 0)
        LOGD("Set SIOCSIFADDR Error");
}

void initialDataCallResponse(RIL_Data_Call_Response_v11* responses, int length) {
    int i = 0;
    for (i=0; i<length; i++) {
        memset(&responses[i], 0, sizeof(RIL_Data_Call_Response_v11));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
    }
}

int getAuthType(const char* authTypeStr) {
    int  authType = atoi(authTypeStr);
    //Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
    //Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
    switch (authType) {
        case 0:
            return AUTHTYPE_NONE;
        case 1:
            return AUTHTYPE_PAP;
        case 2:
            return AUTHTYPE_CHAP;
        case 3:
            return AUTHTYPE_CHAP;
        default:
            return AUTHTYPE_NOT_SET;
    }
}

int getLastDataCallFailCause()
{
    if (gprs_failure_cause == SM_OPERATOR_BARRED ||
            gprs_failure_cause == SM_MBMS_CAPABILITIES_INSUFFICIENT ||
            gprs_failure_cause == SM_LLC_SNDCP_FAILURE ||
            gprs_failure_cause == SM_INSUFFICIENT_RESOURCES ||
            gprs_failure_cause == SM_MISSING_UNKNOWN_APN ||
            gprs_failure_cause == SM_UNKNOWN_PDP_ADDRESS_TYPE ||
            gprs_failure_cause == SM_USER_AUTHENTICATION_FAILED ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_GGSN ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_UNSPECIFIED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUBSCRIBED ||
            gprs_failure_cause == SM_SERVICE_OPTION_OUT_OF_ORDER ||
            gprs_failure_cause == SM_NSAPI_IN_USE ||
            gprs_failure_cause == SM_REGULAR_DEACTIVATION ||
            gprs_failure_cause == SM_QOS_NOT_ACCEPTED ||
            gprs_failure_cause == SM_NETWORK_FAILURE ||
            gprs_failure_cause == SM_REACTIVATION_REQUESTED ||
            gprs_failure_cause == SM_FEATURE_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_TFT ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_TFT ||
            gprs_failure_cause == SM_UNKNOWN_PDP_CONTEXT ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED ||
            gprs_failure_cause == SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT ||
            gprs_failure_cause == SM_BCM_VIOLATION ||
            gprs_failure_cause == SM_LAST_PDN_DISC_NOT_ALLOWED ||
            gprs_failure_cause == SM_ONLY_IPV4_ALLOWED ||
            gprs_failure_cause == SM_ONLY_IPV6_ALLOWED ||
            gprs_failure_cause == SM_ONLY_SINGLE_BEARER_ALLOWED ||
            gprs_failure_cause == ESM_INFORMATION_NOT_RECEIVED ||
            gprs_failure_cause == SM_PDN_CONNECTION_NOT_EXIST ||
            gprs_failure_cause == SM_MULTIPLE_PDN_APN_NOT_ALLOWED ||
            gprs_failure_cause == SM_COLLISION_WITH_NW_INITIATED_REQUEST ||
            gprs_failure_cause == ESM_UNSUPPORTED_QCI_VALUE ||
            gprs_failure_cause == SM_BEARER_HANDLING_NOT_SUPPORT ||
            gprs_failure_cause == SM_MAX_PDP_NUMBER_REACHED ||
            gprs_failure_cause == SM_APN_NOT_SUPPORT_IN_RAT_PLMN ||
            gprs_failure_cause == SM_INVALID_TRANSACTION_ID_VALUE ||
            gprs_failure_cause == SM_SEMENTICALLY_INCORRECT_MESSAGE ||
            gprs_failure_cause == SM_INVALID_MANDATORY_INFO ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_CONDITIONAL_IE_ERROR ||
            gprs_failure_cause == SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_PROTOCOL_ERROR ||
            gprs_failure_cause == SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT
#ifdef MTK_FALLBACK_RETRY_SUPPORT
            || gprs_failure_cause == PDP_FAIL_FALLBACK_RETRY
#endif
            ) {
        return gprs_failure_cause;
    } else if (gprs_failure_cause == 0x0E0F){
        //SM_LOCAL_REJECT_ACT_REQ_DUE_TO_GPRS_ATTACH_FAILURE
        //Modem will retry attach
        LOGD("getLastDataCallFailCause(): GMM error %X", gprs_failure_cause);
    } else if (gprs_failure_cause == 0x150A || gprs_failure_cause == 0x1402) {
        LOGD("ESM_TIMER_TIMOUT(0x150A) or REJECT_IMS_PDN_BLOCK(0x1402): %X", gprs_failure_cause);
    } else {
       gprs_failure_cause = 14;  //FailCause.Unknown
    }

    return gprs_failure_cause;
}

/* Change name from requestOrSendPDPContextList to requestOrSendDataCallList */
static void requestOrSendDataCallList(RIL_Token *t, RIL_SOCKET_ID rilid)
{
    /* Because the RIL_Token* t may be NULL passed due to receive URC: Only t is NULL, 2nd parameter rilid is used */
    RILChannelCtx* rilchnlctx = NULL;
    if (t != NULL)
        rilchnlctx = getRILChannelCtxFromToken(*t);
    else
        rilchnlctx = getChannelCtxbyProxy(rilid);

    requestOrSendDataCallListIpv6(rilchnlctx, t, rilid);
}

/* 27:RIL_REQUEST_SETUP_DATA_CALL/RIL_REQUEST_SETUP_DEFAULT_PDP */
/* ril_commands.h : {RIL_REQUEST_SETUP_DATA_CALL, dispatchStrings, responseStrings} */
/* ril_commands.h : {RIL_REQUEST_SETUP_DEFAULT_PDP, dispatchStrings, responseStrings, RIL_CMD_PROXY_3} */
/* Change name from requestSetupDefaultPDP to requestSetupDataCall */
void requestSetupDataCall(void * data, size_t datalen, RIL_Token t)
{
    /* In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
     * data[2] is apn, data[3] is username, data[4] is passwd, data[5] is authType (added by Android2.1)
     * data[6] is cid field added by mtk for Multiple PDP contexts setup support 2010-04
     */

/// M: IMS/EPDG @{
    int isReqFromMAL = isRequestTokFromMAL(t);
    setRequestFromMALProp(isReqFromMAL);
/// @}
    int requestParamNumber = (datalen/sizeof(char*));
    LOGD("requestSetupData with datalen=%d and parameter number=%d", datalen, requestParamNumber);

    reqSetupConf_t reqSetupConf = {0};
    reqSetupConf.radioType = ((const char **)data)[0];
    reqSetupConf.profile = ((const char **)data)[1];
    reqSetupConf.requestedApn = ((const char **)data)[2];
    reqSetupConf.username = ((const char **)data)[3];
    reqSetupConf.password = ((const char **)data)[4];
    reqSetupConf.authType = getAuthType(((const char **)data)[5]);
    reqSetupConf.protocol = get_protocol_type(((const char **)data)[6]);

    if (isReqFromMAL) {
        reqSetupConf.interfaceId = requestParamNumber > 7 ? atoi(((const char **)data)[7]) : INVALID_CID;
    } else {
        reqSetupConf.interfaceId = requestParamNumber > 7 ? atoi(((const char **)data)[7]) - 1 : INVALID_CID;
    }

/// M: IMS/EPDG @{
    reqSetupConf.availableCid = INVALID_CID;
    reqSetupConf.retryCount = 1;
/// M: IMS/EPDG @}

    //VoLTE
    RIL_Default_Bearer_VA_Config_Struct defaultBearerVaConfig;
    memset(&defaultBearerVaConfig, 0, sizeof(RIL_Default_Bearer_VA_Config_Struct));

    dumpReqSetupConfig(&reqSetupConf);
    LOGD("defaultBearerVaConfig.isValid=%d, isReqFromMAL: %d", defaultBearerVaConfig.isValid, isReqFromMAL);

    if (reqSetupConf.interfaceId < 0) {
        int i = 0, j = 0;
        for (i = 0; i < 10; i++) {
            if (reqSetupConf.interfaceId == INVALID_CID) {
                reqSetupConf.interfaceId = i;
                for (j = 0; j<max_pdn_support; j++) {
                    if (pdn_info[j].interfaceId == i) {
                        LOGD("requestSetupData interfaceId [%d] is already in use", i);
                        reqSetupConf.interfaceId = INVALID_CID;
                        break;
                    }
                }
            }
        }
        LOGD("requestSetupData selected ineterfaceId is %d", reqSetupConf.interfaceId);
    }

    if (requestParamNumber > 8) {
        defaultBearerVaConfig.isValid = atoi(((const char **)data)[8]);
        if (requestParamNumber == 18) {
            defaultBearerVaConfig.isHandover = atoi(((const char **)data)[17]);
            LOGD("isHandover=%d", defaultBearerVaConfig.isHandover);
        }
    }

    if (defaultBearerVaConfig.isValid) {
        defaultBearerVaConfig.qos.qci = atoi(((const char **) data)[9]);
        defaultBearerVaConfig.qos.dlGbr = atoi(((const char **) data)[10]);
        defaultBearerVaConfig.qos.ulGbr = atoi(((const char **) data)[11]);
        defaultBearerVaConfig.qos.dlMbr = atoi(((const char **) data)[12]);
        defaultBearerVaConfig.qos.ulMbr = atoi(((const char **) data)[13]);
        defaultBearerVaConfig.emergency_ind = atoi(((const char **) data)[14]);
        defaultBearerVaConfig.pcscf_discovery_flag = atoi(((const char **) data)[15]);
        defaultBearerVaConfig.signalingFlag = atoi(((const char **) data)[16]);

        dumpDefaultBearerConfig(&defaultBearerVaConfig);
    }

    if (!isReqFromMAL) {
        /// [IMS][EPDG] code start. {@
        // Request not from MAL, request from MAL means HO, no need to
        // go thru this
        if (getImsParamInfo(reqSetupConf.interfaceId, &defaultBearerVaConfig) != 0) {
            LOGE("get ims param failed!!");
            goto error;
        } else {    // not IMS/EIMS will get the config fill of 0
            dumpDefaultBearerConfig(&defaultBearerVaConfig);
        }

        if (isEpdgSupport()) {
            int nRet = queryEpdgRat(&reqSetupConf, &defaultBearerVaConfig);
            switch (nRet) {
            case PDN_SETUP_THRU_MOBILE:
                break;
            case PDN_SETUP_THRU_WIFI: {
                    reqSetupConf.availableCid = getAvailableWifiCid();
                    if (reqSetupConf.availableCid == INVALID_CID) {
                        LOGE("error not enough CID to use");
                        goto error;
                    } else {
                        LOGD("avaiable cid=%d for wifi", reqSetupConf.availableCid);
                    }
                }
                break;
            case PDN_SETUP_THRU_ERR:
            default:
                LOGE("queryEpdgRat error!!");
                goto error;
            }
        }
        /// [IMS][EPDG] code end. @}
    }

    if (requestParamNumber != 18 &&  requestParamNumber != 8) {
        LOGD("requestSetupData with incorrect parameters, requestParamNumber=%d", requestParamNumber);
        goto error;
    }

#ifdef MTK_FALLBACK_RETRY_SUPPORT
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int flag_T_PdpRetry = isFallbackPdpRetryRunning(rid, apn, protocol);
#endif

#if defined(PURE_AP_USE_EXTERNAL_MODEM) && !defined(MT6280_SUPER_DONGLE)
    if (strcmp(profile, "1002") == 0) {
        // Always request for CID 1 for default connection
        requestSetupDataCallOverIPv6(reqSetupConf.apn, reqSetupConf.username, reqSetupConf.passwd,
                            reqSetupConf.authType, reqSetupConf.protocol, 0, 1, reqSetupConf.profile, t);
    } else {
        requestSetupDataCallOverIPv6(reqSetupConf.apn, reqSetupConf.username, reqSetupConf.passwd,
                            reqSetupConf.authType, reqSetupConf.protocol, 1, 2, reqSetupConf.profile, t);
    }
#else
    int setupFuncIdx = E_SETUP_DATA_CALL_FUNC_ERR;
    if (!isReqFromMAL) {
        if (isEpdgSupport() && reqSetupConf.availableCid != INVALID_CID) {
            setupFuncIdx = E_SETUP_DATA_CALL_OVER_EPDG;
        } else if (defaultBearerVaConfig.emergency_ind == 1) {  // Emergency
            setupFuncIdx = E_SETUP_DATA_CALL_EMERGENCY;
#ifdef MTK_FALLBACK_RETRY_SUPPORT
        } else if (flag_T_PdpRetry) {
            setupFuncIdx = E_SETUP_DATA_CALL_FALLBACK;
#endif
        } else {
            setupFuncIdx = E_SETUP_DATA_CALL_OVER_IPV6;
        }
    } else {  // request from MAL
        if (isEpdgSupport()) {
            setupFuncIdx = E_SETUP_DATA_CALL_HO_EPDG;
            defaultBearerVaConfig.isHandover = 1;
        } else {
            LOGE("%s request from mal (shouldn't), func id: %d!!", __FUNCTION__, setupFuncIdx);
       }
    }

    if (setupFuncIdx != E_SETUP_DATA_CALL_FUNC_ERR) {
        setupDataCallFunc[setupFuncIdx] (&reqSetupConf, (void *) &defaultBearerVaConfig, t);
    } else {
        LOGE("%s can't find mapping setup data call func!!", __FUNCTION__);
        goto error;
    }

#endif

finish:
    FREEIF(reqSetupConf.pQueryRatRsp);
    setRequestFromMALProp(0);
    return;

error:
    LOGE("[%s] error E", __FUNCTION__);
    FREEIF(reqSetupConf.pQueryRatRsp);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    setRequestFromMALProp(0);
}

void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
#ifdef MTK_RIL_MD2
    if (getExternalModemSlot() >= 0) {
        LOGD("[%s] PPP shutdown", __FUNCTION__);
        shutdownPPP(t);
        int cidArray[1] = {1};
        RIL_onRequestComplete(t, RIL_E_SUCCESS, cidArray, sizeof(cidArray));
        return;
    }
#endif

    const char *cid;
    char *cmd;
    int err;
    ATResponse *p_response = NULL;
    int i = 0, lastPdnCid = INVALID_CID;
    int interfaceId = atoi(((const char **)data)[0]);
    int reason = atoi(((const char **)data)[1]);
    LOGD("[%s] interfaceId=%d, reason: %d X", __FUNCTION__, interfaceId, reason);

    int needHandleLastPdn = 0;
    int lastPdnState = DATA_STATE_INACTIVE;
    int isEmergency = 0;
    PdnInfo *pdnInfo = NULL;

    // AT+CGACT=<state>,<cid>;  <state>:0-deactivate;1-activate
    for(i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[i].interfaceId == interfaceId && !pdn_info[i].isDedicateBearer) {
            pdnInfo = &pdn_info[i];

            // reason 100 means that RDS disconnect LTE pdn, no need to ask RDS again
            if (reason != REASON_RDS_DEACT_LTE_PDN && 0 != deactivatePdnThruEpdg(interfaceId, reason)) {
                LOGD("no deactivate thru mobile!!");
                continue;
            }

            int err = deactivateDataCall(pdnInfo->cid, DATA_CHANNEL_CTX);
            switch (err) {
            case CME_SUCCESS:
                break;
            case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
                LOGD("[%s] deactivateDataCall cid%d already deactivated", __FUNCTION__, i);
                break;
            case CME_LAST_PDN_NOT_ALLOW:
                if (needHandleLastPdn == 0) {
                    needHandleLastPdn = 1;
                    lastPdnState = pdnInfo->active;
                    isEmergency = pdnInfo->isEmergency;
                    lastPdnCid = pdnInfo->cid;
                    LOGD("[%s] deactivateDataCall cid%d is the last PDN, state: %d", __FUNCTION__, i, lastPdnState);
                }
                break;
            case CME_LOCAL_REJECTED_DUE_TO_PS_SWITCH:
                // in this case, we make the deactivation fail
                // after switch, framework will check data call list and to deactivation again
                LOGD("[%s] receive LOCAL_REJECTED_DUE_TO_PS_SWITCH", __FUNCTION__);
                goto error;
                /// [C2K][IRAT] new fail cause happened during IRAT.
            case CME_L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED:
                pdnInfo->active = DATA_STATE_NEED_DEACT;
                LOGD("[RILData_GSM_IRAT] receive L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED.");
                break;
            default:
                goto error;
            };
        }
    }

    if (reason != REASON_RDS_DEACT_LTE_PDN) {
        configureNetworkInterface(interfaceId, 0);
    }

    if (pdnInfo == NULL) {
        LOGD("deactivate wifi pdn");
        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            if (pdn_info_wifi[i].interfaceId == interfaceId) {
                // reason 100 means that RDS disconnect LTE pdn, no need to ask RDS again
                pdnInfo = &pdn_info_wifi[i];
                if (reason != REASON_RDS_DEACT_LTE_PDN && 1 == deactivatePdnThruEpdg(interfaceId, reason)) {
                    clearPdnInfo(pdnInfo);
                }
                break;
            }
        }
    }

    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[i].interfaceId == interfaceId && !pdn_info[i].isDedicateBearer
                && pdn_info[i].active != DATA_STATE_NEED_DEACT) {
            // clear only default bearer
            clearPdnInfo(&pdn_info[i]);
        }
    }

    if (needHandleLastPdn) {
        if (lastPdnState != DATA_STATE_LINKDOWN) {
            if (0 != handleLastPdnDeactivation(isEmergency, lastPdnCid, DATA_CHANNEL_CTX)) {
                pdn_info[lastPdnCid].active = DATA_STATE_NEED_DEACT;
                goto error;
            }
        } else {
            LOGD("[%s] last pdn alread linkdown", __FUNCTION__);
        }
        pdn_info[lastPdnCid].active = DATA_STATE_LINKDOWN;
        pdn_info[lastPdnCid].cid = lastPdnCid;
        pdn_info[lastPdnCid].primaryCid = lastPdnCid;
    }

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    updateAvailablePdpNum();
#endif

    //response deactivation result first then do re-attach
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    AT_RSP_FREE(p_response);
    return;
error:
    LOGE("[%s] error E", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    AT_RSP_FREE(p_response);
}

void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int lastPdpFailCause = 14;
    lastPdpFailCause = getLastDataCallFailCause();
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &lastPdpFailCause, sizeof(lastPdpFailCause));
}

void requestDataCallList(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    requestOrSendDataCallList(&t, getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
}

void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    RIL_InitialAttachApn* param = data;
    int isApnDiff = 1;
    int isDualApnSim = 0;
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};

    LOGD("IA: requestSetInitialAttachApn [apn=%s, protocol=%s, auth_type=%d, username=%s, \
password=%s, operatorNumeric=%s, canHandleIms=%d], attachApnSupport=%d, isCid0Support=%d",
            param->apn, param->protocol, param->authtype, param->username,
            "XXXXX"/*param->password*/, param->operatorNumeric, param->canHandleIms,
            getAttachApnSupport(), isCid0Support);

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    if (rid + 1 != (unsigned int) RIL_get3GSIM()) {
        LOGD("IA: requestSetInitialAttachApn but not main socket");
        goto error;
    } else if (getAttachApnSupport() == ATTACH_APN_NOT_SUPPORT) {
        LOGD("IA: requestSetInitialAttachApn but attach apn is not support");
        goto error;
    } else if (isDualTalkMode()) {
        #ifdef MTK_RIL_MD2
            LOGD("IA: requestSetInitialAttachApn but dual talk MD2 is not support LTE");
            goto error;
        #endif
    }

    getMSimProperty(rid, PROPERTY_SIM_OPERATOR_NUMERIC, operatorNumeric);

    if (isCdmaLteDcSupport()) {
       if (strcmp(operatorNumeric, CT_OPERATOR_NUMERIC_CDMA) == 0) {
           strcpy(operatorNumeric, CT_OPERATOR_NUMERIC_GSM);
       } else if (strcmp(operatorNumeric, CT_MACAO_OPERATOR_NUMERIC_CDMA) == 0) {
           strcpy(operatorNumeric, CT_MACAO_OPERATOR_NUMERIC_GSM);
       }
    }

#ifdef MTK_DTAG_DUAL_APN_SUPPORT
    int i = 0;
    char** dualApnPlmnList = param->dualApnPlmnList;
    int stringLength = datalen - sizeof(RIL_InitialAttachApn) + sizeof(param->dualApnPlmnList);
    stringLength = stringLength / sizeof(char *);

    if (stringLength > MAX_DUAL_APN_OPERATORS) {
        LOGE("IA: requestSetInitialAttachApn Need to extend the dual APN PLMN size %d !! ", stringLength);
        goto error;
    }

    g_iNumDualApnPlmn = stringLength;

    for (i = 0; i < stringLength; i++) {
        //Check if the HPLMN and registered PLMN is in dual APN list
        if (strcmp(operatorNumeric, dualApnPlmnList[i]) == 0) {
            isDualApnSim = 1;
            LOGD("IA: dualApnSim=%s", dualApnPlmnList[i]);
        }
        strncpy(g_aDualApnPlmnList[i], dualApnPlmnList[i], strlen(dualApnPlmnList[i]));
        LOGD("IA: dualApnPlmnList=%s", g_aDualApnPlmnList[i]);
    }
#endif //MTK_DTAG_DUAL_APN_SUPPORT

    if ((strcmp(operatorNumeric, param->operatorNumeric) != 0) && (isDualApnSim == 0)) {
        LOGE("IA: requestSetInitialAttachApn current SIM operator numeric is different from the \
parameter, skip setting attach APN [%s, %s]", operatorNumeric, param->operatorNumeric);
        goto error;
    } else if (isCid0Support) {
        int triggerPsAttach = 1;
        if (getMtkShareModemCurrent() > 1) {
            //This is for gemini load that attach apn should be set on data SIM
            int dataSim = getDefaultDataSim();
            int simId3G = RIL_get3GSIM();

            if (dataSim == 0) {
                LOGD("IA: requestSetInitialAttachApn data disabled, trigger attach on 4G SIM");
            } else if (simId3G != dataSim) {
                LOGD("IA: requestSetInitialAttachApn data not enabled on 4G SIM, defind CID0 and \
not to trigger PS re-attach");
                triggerPsAttach = 0;
            }
        }

        char iccid[PROPERTY_VALUE_MAX] = {0};
        char iaProperty[PROPERTY_VALUE_MAX*2] = {0};

        property_get(PROPERTY_ICCID_SIM[rid], iccid, "");
        getIaCache(iaProperty);

        char iaPwdFlag[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_IA_PASSWORD_FLAG, iaPwdFlag, "0");

        if (strlen(iaProperty) != 0) {
            //here we need to check if current IA property is different than what we want to set
            //if different, need to clear cache and do PS detach
            //this scenario happens when attach APN is changed
            char iaParameter[PROPERTY_VALUE_MAX*2] = {0};
            if (strlen(param->password) == 0) {
                sprintf(iaParameter, "%s,%s,%d,%s,%d,%s", iccid, param->protocol,
                        param->authtype, param->username, param->canHandleIms, param->apn);
            } else {
                //when password is set, iccid is not recorded
                //so we do not need to compare iccid
                sprintf(iaParameter, "%s,%s,%d,%s,%d,%s", "", param->protocol, param->authtype,
                        param->username, param->canHandleIms, param->apn);
            }

            if (strcmp(iaParameter, iaProperty) != 0) {
                LOGD("IA: requestSetInitialAttachApn the parameter is different than cache \
[%s][%s]", iaParameter, iaProperty);
                if (strlen(iccid) > 0) {
                    LOGD("IA: requestSetInitialAttachApn clear IA property");
                    memset(iaProperty, 0, sizeof(iaProperty));
                } else {
                    LOGD("IA: requestSetInitialAttachApn no SIM detected");
                }
            }
        } else if (atoi(iaPwdFlag)) {
            //the IA Cache is empty due to password
            char iaProperty[PROPERTY_VALUE_MAX*2] = {0};
            char iaParameter[PROPERTY_VALUE_MAX*2] = {0};
            getTempIa(iaProperty);
            sprintf(iaParameter, "%s,%s,%d,%s,%d,%s,%s", iccid, param->protocol,
                param->authtype, param->username, param->canHandleIms, param->apn, param->password);
            if (strcmp(iaParameter, iaProperty) != 0) {
                LOGD("IA: requestSetInitialAttachApn the parameter(pwd) is different than cache \
[%s][%s]", iaParameter, iaProperty);
            } else {
                LOGD("IA: requestSetInitialAttachApn the parameter(pwd) is the same as cache");
                isApnDiff = 0;
            }
        }

        if (strlen(iaProperty) == 0 && isApnDiff) {
            int isOp01 = 0;
            int isOp01Volte = 0;
            int check = 0;
            char optr[PROPERTY_VALUE_MAX] = {0};
            char* apn = "";
            RIL_Default_Bearer_VA_Config_Struct* defaultBearer = NULL;

            property_get("ro.operator.optr", optr, "");
            isOp01 = (strcmp("OP01", optr) == 0);

            property_get("ro.mtk_md_sbp_custom_value", optr, "");
            isOp01Volte = (strcmp("1", optr) == 0);

            if (param->canHandleIms || isOp01Volte) {
                defaultBearer = calloc(1, sizeof(RIL_Default_Bearer_VA_Config_Struct));
                defaultBearer->signalingFlag = isOp01Volte ? 0 : 1;
                defaultBearer->pcscf_discovery_flag  =1;
                defaultBearer->emergency_ind = 0;
            }

            if (strlen(param->password) != 0) {
                strncpy(TEMP_IA_PASSWORD, param->password, sizeof(TEMP_IA_PASSWORD)-1);
                property_set(PROPERTY_IA_PASSWORD_FLAG, "1");
            } else {
                memset(TEMP_IA_PASSWORD, 0, sizeof(TEMP_IA_PASSWORD));
                property_set(PROPERTY_IA_PASSWORD_FLAG, "0");
            }

            // for op01 test sim, force apn to empty.
            if (rildata_isTestSim() && (isOp01 || isOp01Volte)) {
                apn = "";
            } else {
                apn = param->apn;
            }

            setTempIa(iccid, param->protocol, param->authtype, param->username,
                        param->canHandleIms, param->apn);

            check = definePdnCtx(defaultBearer, apn, get_protocol_type(param->protocol), 0,
                    getAuthTypeInt(param->authtype), param->username, param->password,
                    DATA_CHANNEL_CTX);

            if (defaultBearer != NULL) {
                free(defaultBearer);
                defaultBearer = NULL;
            }

            if (0 == check) {
                LOGE("IA: requestSetInitialAttachApn define CID0 failed");
                at_send_command("AT+EGREA=0", NULL, DATA_CHANNEL_CTX);
                goto error;
            }

            //update IA APN ICCID
            LOGD("IA: requestSetInitialAttachApn update IA ICCID [%s]", iccid);
            property_set(PROPERTY_IA_APN_SET_ICCID, iccid);

            if (strlen(param->password) == 0) {
                setIaCache(iccid, param->protocol, param->authtype, param->username,
                        param->canHandleIms, param->apn);
            } else {
                LOGD("IA: requestSetInitialAttachApn initial attach APN contain password, \
set patch to invalid value");
                setIaCache("", "", 0, "", 0, "");
            }

            if (triggerPsAttach && queryCurrentConnType(DATA_CHANNEL_CTX) &&
                    MODEM_RAT_LTE == querryCurrentRat(DATA_CHANNEL_CTX)) {
                LOGD("IA: requestSetInitialAttachApn relay re-attach to data channel");
                OnReAttachInfo* pInfo = malloc(sizeof(OnReAttachInfo));
                pInfo->pDataChannel = getRILChannelCtx(RIL_DATA, rid);
                strncpy(pInfo->apn, param->apn, PROPERTY_VALUE_MAX-1);
                RIL_requestProxyTimedCallback (onReAttachForSettingAttachApn, (void*)pInfo, &TIMEVAL_0,
                    pInfo->pDataChannel->id, "onReAttachForSettingAttachApn");
            } else {
                LOGD("IA: requestSetInitialAttachApn not to do PS attach");
            }
        } else {
            LOGD("IA: requestSetInitialAttachApn valid IA property is set, \
not to do attach again");
        }
    } else {
        LOGD("IA: requestSetInitialAttachApn but CID0 is not support");
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestSetupPPPDataCall(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    const char *apn;
    //const char *username, *passwd, *authType, *protocol;
    char *req_dns1="", *req_dns2="";
    char *cmd;
    int err;
    int isPPPSucceed = 0;
    ATResponse *p_response = NULL;
    RIL_Data_Call_Response_v11 *responses = alloca(sizeof(RIL_Data_Call_Response_v11));
    initialDataCallResponse(responses, 1);

    LOGD("Shutdown PPP before setup data call");
    shutdownPPP(t);

    RILChannelCtx* p_channel = openPPPDataChannel(1);
    if (p_channel == NULL) {
        goto error;
    }

    // response[0]:cid, response[1]:interface name, response[2]:ipaddr, response[3]:dns1,
    // response[4]:dns2 [5]: remote-ip

    //In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
    //data[2] is apn, data[3] is username, data[4] is passwd: modified 2009-0908 by mtk01411
    apn = ((const char **)data)[2];

    LOGD("requesting data connection to APN '%s'", apn);

    asprintf(&cmd, "AT+CGDCONT=1,\"IP\",\"%s\",,0,0", apn);
    //FIXME check for error here
    err = at_send_command_to_ppp_data_channel(cmd, NULL, p_channel);
    free(cmd);

    // packet-domain event reporting
    err = at_send_command_to_ppp_data_channel("AT+CGEREP=1,0", NULL, p_channel);

    // Hangup anything that's happening there now
    // 20120508: we shutdown ppp at the begining of this function, so remove this
    //err = at_send_command_to_ppp_data_channel("AT+CGACT=0,1", NULL, p_channel);

    // Start data on PDP context 1
    err = at_send_command_to_ppp_data_channel("ATD*99***1#", &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    closePPPDataChannel();

    p_channel = openPPPDataChannel(0);
    purge_data_channel(p_channel);
    closePPPDataChannel();

    //TODO: Check
    isPPPSucceed = startPPPCall(responses, "pppd_gprs");


    if (!isPPPSucceed)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responses, sizeof(RIL_Data_Call_Response_v11));
    at_response_free(p_response);
    if (responses != NULL)
        freeDataResponse(responses);
    return;
error:
    closePPPDataChannel();
    LOGD("Shutdown PPP before return fail");
    shutdownPPP(t);

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (responses != NULL)
        freeDataResponse(responses);
}

int isPPPRunning(char* runningProperty) {
    char runningCode[PROPERTY_VALUE_MAX] = {0};
    LOGD("isPPPRunning:%s", runningProperty);
    property_get(runningProperty, runningCode, "");
    if (strcmp(runningCode, PPP_DEF_RUNNING) == 0 || strcmp(runningCode, PPP_DEF_STOPPING) == 0)
        return 1;
    else
        return 0;
}

int startPPPCall(RIL_Data_Call_Response_v11* response, char* port) {

    property_set("net.gprs.ppp-exit", "");
    // Start pppd
    property_set("ctl.start", port);

    char *cmd;
    asprintf(&cmd, "init.svc.%s", port);

    int count = 0;
    while (count < 10) {
        if (isPPPRunning(cmd)) {
            LOGD("pppd started [%d]", count);
            break;
        } else {
            LOGD("Wait pppd started [%d]", count);
            sleep(1);
        }
        ++count;
    }
    free(cmd);

    int isPolling = 1;
    char* exitCode = calloc(1, PROPERTY_VALUE_MAX);
    char* runningCode = calloc(1, PROPERTY_VALUE_MAX);
    count = 0;
    int isPPPSucceed = 0;
    while (isPolling && count < 300) {
        memset(exitCode, 0, PROPERTY_VALUE_MAX);
        memset(runningCode, 0, PROPERTY_VALUE_MAX);
        property_get("net.gprs.ppp-running", runningCode, "");
        property_get("net.gprs.ppp-exit", exitCode, "");

        if (strcmp(exitCode, "") != 0) {
            LOGD("pppd exited [%s] during starting", exitCode);
            isPolling = 0;
        } else if (strcmp(runningCode, "yes") == 0) {
            //PPP Parameter
            char local[PROPERTY_VALUE_MAX] = {0};
            char remote[PROPERTY_VALUE_MAX] = {0};
            char dns1[PROPERTY_VALUE_MAX] = {0};
            char dns2[PROPERTY_VALUE_MAX] = {0};

            LOGD("polling pppd done");

            property_get("net.ppp0.local-ip", local, "0.0.0.0");
            property_get("net.ppp0.remote-ip", remote, "0.0.0.0");
            property_get("net.ppp0.dns1", dns1, "0.0.0.0");
            property_get("net.ppp0.dns2", dns2, "0.0.0.0");

            response->status = PDP_FAIL_NONE;
            response->suggestedRetryTime = 0;
            response->cid = 0;
            response->active = 2;
            asprintf(&response->type, "%s", "PPP");
            asprintf(&response->ifname, "%s", "ppp0");
            //address, local-ip
            asprintf(&response->addresses, "%s", local);
            //gateway, remote-ip
            asprintf(&response->gateways, "%s", remote);
            //dns
            asprintf(&response->dnses, "%s %s", dns1, dns2);

            LOGD("PPP Data call response: status=%d, suggestedRetryTime=%d, cid=%d, active=%d, \
                type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s",
                response->status, response->suggestedRetryTime,
                response->cid, response->active, response->type,
                response->ifname, response->addresses,
                response->dnses, response->gateways);

            isPPPSucceed = 1;
            isPolling = 0;
        } else {
            LOGD("polling pppd not finished [%d]", count);
            sleep(1);
        }
        ++count;
    }
    free(exitCode);
    free(runningCode);
    return isPPPSucceed;

}

void shutdownPPP(RIL_Token t) {
    int isPPPOriginRunning = isPPPRunning(PPP_PPP_RUNNING_PROPERTY);
    int count = 0;

    if (isPPPOriginRunning) {
        do {
            LOGD("Shutdown ppp is retring [%d]", count);
            //property_set("ctl.stop", "pppd_gprs");
            at_send_command("AT+CGACT=0,1", NULL, DATA_CHANNEL_CTX);
            ++count;
            sleep(1);
        } while (count < 60 && isPPPRunning(PPP_PPP_RUNNING_PROPERTY));

        LOGD("Shutdown ppp is done [%d]", count);

        RILChannelCtx* p_channel = openPPPDataChannel(0);
        //waitForTargetPPPStopped(p_channel);
        closePPPDataChannel();
    }

    RILChannelCtx* p_channel = openPPPDataChannel(0);
    purge_data_channel(p_channel);
    closePPPDataChannel();
}

void onGPRSDeatch(char* urc, RIL_SOCKET_ID rid)
{
    LOGD("onGPRSDeatch:%s", urc);
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_GPRS_DETACH, NULL, 0, rid);
}

/* Change name from onPdpContextListChanged to onDataCallListChanged */
/* It can be called in onUnsolicited() mtk-ril\ril_callbacks.c */
void onDataCallListChanged(void* param)
{
    RIL_SOCKET_ID rilid = *((RIL_SOCKET_ID *) param);
    requestOrSendDataCallList(NULL, rilid);
}

void onDualApnCheckNetworkChange(void* param)
{
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* rilchnlctx = (RILChannelCtx*) param;
    ATLine *p_cur = NULL;
    char *out = NULL;
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    char networkType[PROPERTY_VALUE_MAX] = {0};
    int isNoDataApnExist = 0;
    int isDualApnPlmnNw = 0;
    int i;
    char *token;
    int activeSimFor4G = 0;

    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    static int isNoDataActive = 1;

    property_get(PROPERTY_DATA_NETWORK_TYPE, networkType, "");
    LOGD("onDualApnCheckNetworkChange: Network type [%s]", networkType);

    i = 0;
    token = strtok(networkType, ",");
    while (token != NULL) {
        if (0 == strcmp(token, "LTE")) {
            LOGD("onDualApnCheckNetworkChange: Find out LTE in SIM#%d\n", i+1);
            break;
        }
        token = strtok(NULL,",");
        i++;
    }
    activeSimFor4G = i; //keep the SIM ID for LTE service 0:SIM#1

    property_get(PROPERTY_OPERATOR_NUMERIC, operatorNumeric, "");
    LOGD("onDualApnCheckNetworkChange: Registered PLMN [%s]", operatorNumeric);

    token = strtok(operatorNumeric, ",");
    while(i > 0) {
        if (token != NULL) {
            token = strtok(NULL,",");
        }
        i--;
    }

    if (token == NULL) {
        LOGD("onDualApnCheckNetworkChange: Not register yet\n");
        return;
    } else {
        LOGD("onDualApnCheckNetworkChange: Find out LTE PLMN [%s]\n", token);
    }

    char simOperatorNumeric[PROPERTY_VALUE_MAX] = {0};
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    //If the logic of getMSimProperty() is changed, then here should be also considered
    if ((rid - RIL_SOCKET_1) != activeSimFor4G) {
        LOGD("onDualApnCheckNetworkChange: Shouldn't be here since only 4G URC[id:%d]\n", activeSimFor4G);
        return;
    }
    getMSimProperty(rid, PROPERTY_SIM_OPERATOR_NUMERIC, simOperatorNumeric);

    for (i = 0; i < g_iNumDualApnPlmn; i++) {
        //Check the HPLMN
        if (0 == strncmp(simOperatorNumeric, g_aDualApnPlmnList[i], MAX_PLMN_LEN - 1)) {
            isDualApnPlmnNw = 1;
            break;
        }
    }

    if (0 == isDualApnPlmnNw) {
        LOGD("onDualApnCheckNetworkChange: Not a Dual APN SIM [%s]", simOperatorNumeric);
        return;
    }

    isDualApnPlmnNw = 0;
    for (i = 0; i < g_iNumDualApnPlmn; i++) {
        if (0 == strncmp(token, g_aDualApnPlmnList[i], MAX_PLMN_LEN - 1)) {
            isDualApnPlmnNw = 1;
            break;
        }
    }

    if (0 == isDualApnPlmnNw) {
        LOGD("onDualApnCheckNetworkChange: Not a Dual APN PLMN now [%s]", token);
        return;
    }

    int *activatedCidList = (int *) calloc(1, pdnInfoSize * sizeof(int));

    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    err = at_send_command_multiline ("AT+CGCONTRDP", "+CGCONTRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGD("onDualApnCheckNetworkChange: AT+CGCONTRDP response error");
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;
            int cid;
            int type;

            err = at_tok_start(&line);
            if (err < 0)
                break;

            err = at_tok_nextint(&line, &cid); //cid
            if (err < 0)
                break;

            err = at_tok_nextint(&line, &type); //type
            if (err < 0)
                break;

            err = at_tok_nextstr(&line, &out); //apn
            if (err < 0)
                break;

            if (out != NULL && strlen(out) > 0 && apnMatcher(out, DEFAULT_NODATA_APN) == 0)
            {
                //NODATA APN exists, do nothing
                LOGD("onDualApnCheckNetworkChange: APN exist [%s]", out);
                isNoDataApnExist = 1;
                break;
            }
        }
    }

    AT_RSP_FREE(p_response);

    if (0 == isNoDataApnExist) {
        //activate NODATA APN
        //set follow on flag for modem to re-use the connection
        //this helps to speed up PDN activation when we need to do PS attach
        err = at_send_command("AT+EGTYPE=3", &p_response, rilchnlctx);
        AT_RSP_FREE(p_response);

        //set to auto attach mode so that modem will do PS re-attach when network detach happened
        err = at_send_command("AT+EGTYPE=1", &p_response, rilchnlctx);
        if (isATCmdRspErr(err, p_response)) {
            LOGD("onDualApnCheckNetworkChange: fail to set connetion type");
            gprs_failure_cause = 14;
            goto error;
        }
        AT_RSP_FREE(p_response);

        //Use this command to make sure PS is attached before activate PDP
        err = at_send_command("AT+CGATT=1", &p_response, rilchnlctx);
        if (isATCmdRspErr(err, p_response)) {
            LOGD("onDualApnCheckNetworkChange: fail to do PS attach");
            gprs_failure_cause = 14;
            goto error;
        }
        AT_RSP_FREE(p_response);

        //after AT+CGATT=1, the CID0 would be activated, enable fallback again
        //not to check error since it is not critical
        //AT+EGFB=<fallback_enable>,<ipv4_first>
        err = at_send_command("AT+EGFB=1,1", &p_response, rilchnlctx);
        AT_RSP_FREE(p_response);

        int availableCid = getAvailableCid();
        if (availableCid == INVALID_CID) {
            LOGE("onDualApnCheckNetworkChange: no available CID to use");
            goto error;
        } else {
            LOGD("onDualApnCheckNetworkChange: available CID is [%d]", availableCid);
        }
        // define Pdn ctx
        if (0 == definePdnCtx(NULL, DEFAULT_NODATA_APN, IPV4, availableCid, AUTHTYPE_NONE, "", "", rilchnlctx)){
            goto error;
        }

        // packet-domain event reporting: +CGEREP=<mode>,<bfr>
        err = at_send_command("AT+CGEREP=1,0", NULL, rilchnlctx);

        // activate No Data PDN
        if (0 == activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                             activatedCidList, 0, isNoDataActive, rilchnlctx)) {
            goto error;
        }

        if (DATA_STATE_INACTIVE == isCidActive(activatedCidList, activatedPdnNum, rilchnlctx)) {
            goto error;
        }
    }

    if(activatedCidList != NULL) {
        free(activatedCidList);
    }

    LOGD("onDualApnCheckNetworkChange finish");
    return;

error:
    LOGE("onDualApnCheckNetworkChange ERROR, clear %d", activatedPdnNum);
    for (i = 0; i < activatedPdnNum; i++)
        clearPdnInfo(&pdn_info[activatedCidList[i]]);

    if(activatedCidList != NULL) {
        free(activatedCidList);
    }

    AT_RSP_FREE(p_response);
}

void onMePdnActive(void* param)
{
    MePdnActiveInfo* pInfo = (MePdnActiveInfo*)param;
    RILChannelCtx* rilchnlctx = pInfo->pDataChannel;
    int activatedCid = pInfo->activeCid;
    int err = 0;
    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;
    char *out = NULL;
    int i;
    int isFindNoDataPdn = 0;

    LOGD("onMePdnActive: CID%d is activated and current state is %d", activatedCid, pdn_info[activatedCid].active);
	// [C2K][IRAT] Check react PDN count for cid 0 case.
	if (nIratAction == IRAT_ACTION_TARGET_STARTED && nReactPdnCount > 0) {
		onIratPdnReactSucc(activatedCid);
		free(param);
		return;
	}

    if (pdn_info[activatedCid].active == DATA_STATE_INACTIVE) {
        pdn_info[activatedCid].active = DATA_STATE_LINKDOWN; // Update with link down state.
        pdn_info[activatedCid].cid = activatedCid;
        pdn_info[activatedCid].primaryCid = activatedCid;
    }

    //Check no data PDN
    for (i = 0; i < pdnInfoSize; i++) {
        if (1 == pdn_info[i].isNoDataPdn) {
            LOGD("onMePdnActive: CID%d NoData PDN %s exist", i, pdn_info[i].apn);
            isFindNoDataPdn = 1;
            break;
        }
    }

    if (0 == isFindNoDataPdn) {
        err = at_send_command_multiline ("AT+CGCONTRDP", "+CGCONTRDP:", &p_response, rilchnlctx);
        if (isATCmdRspErr(err, p_response)) {
            LOGD("onMePdnActive: AT+CGCONTRDP response error");
        } else {
            for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
                char *line = p_cur->line;
                int cid;
                int type;

                err = at_tok_start(&line);
                if (err < 0)
                    break;

                err = at_tok_nextint(&line, &cid); //cid
                if (err < 0)
                    break;

                err = at_tok_nextint(&line, &type); //type
                if (err < 0)
                    break;

                err = at_tok_nextstr(&line, &out); //apn
                if (err < 0)
                    break;

                if (out != NULL && strlen(out) > 0 && apnMatcher(out, DEFAULT_NODATA_APN) == 0)
                {
                    //NODATA APN is found
                    LOGD("onMePdnActive: NoData APN is found [%s] Cid:%d", out, cid);
                    pdn_info[cid].isNoDataPdn = 1;
                    isFindNoDataPdn = 1;
                    break;
                }
            }
        }

        if (0 == isFindNoDataPdn)
            LOGD("onMePdnActive: NoData APN is not found");
    }

    free(param);
    AT_RSP_FREE(p_response);
}

void onReAttachForSettingAttachApn(void* param)
{
    LOGD("IA: onReAttachForSettingAttachApn start detach");
    int *matchCount = (int *) calloc(1, pdnInfoSize * sizeof(int));
    OnReAttachInfo* pInfo = (OnReAttachInfo*)param;
    RILChannelCtx* pChannel = pInfo->pDataChannel;
    char* apn = pInfo->apn;
    int isReplaceApnFlag = 0;
    int i;

    for (i = 0; i < pdnInfoSize; i++) {
        matchCount[i] = 0;
    }

    //when AT+EGREA=1 is set, EUTRAN would not be disabled after PS detached
    at_send_command("AT+EGREA=1", NULL, pChannel);
    detachPs(pChannel);

    LOGD("IA: onReAttachForSettingAttachApn detach done and start attach");
    //when AT+EGREA=0 is set, EUTRAN would be disabled after PS detached
    at_send_command("AT+EGREA=0", NULL, pChannel);

    // Use non-blocking AT command to do PS attach, to avoid
    // modem AT command conflict
    if (!doPsAttachAsync(pChannel)) {
        LOGE("IA: onReAttachForSettingAttachApn trigger attach failed");
        //we sill continue to update cache even if attach is failed
    }
    //Trigger query apn to update network assigned apn
    queryMatchedPdnWithSameApn((const char *)apn, matchCount, pChannel);
    LOGD("IA: onReAttachForSettingAttachApn re-attach complete");

    if(matchCount != NULL) {
        free(matchCount);
    }

    free(pInfo);
}

void onAttachApnReset(void* param)
{
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    int rilid = timeCallbackParam->rid;

    setIaCache("", "", 0, "", 0, "");
    LOGD("onAttachApnReset: clean IA Cache and send RIL_UNSOL_SET_ATTACH_APN");
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_SET_ATTACH_APN, NULL, 0, rilid);

    free(timeCallbackParam);
}

int isAlwaysAttach()
{
    return 1;
}

static int isFallbackPdpRetryRunning(RIL_SOCKET_ID rid, const char* apn, int protocol)
{
    char SimOperatorNumeric[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(rid, PROPERTY_SIM_OPERATOR_NUMERIC, SimOperatorNumeric);
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    char networkType[PROPERTY_VALUE_MAX] = {0};
    char *nwType;
    int sim_index = rid;
    int ret = 0;

    //Check HPLMN
    if (0 == strncmp(SimOperatorNumeric, TELSTRA_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("requestSetupData SIM in Telstra 505-01 [%s]", SimOperatorNumeric);
        property_get(PROPERTY_OPERATOR_NUMERIC, operatorNumeric, "");
        //Check registered PLMN
        if (0 == strncmp(operatorNumeric, TELSTRA_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
            LOGD("requestSetupData registered in Telstra 505-01 [%s]", operatorNumeric);
            property_get(PROPERTY_DATA_NETWORK_TYPE, networkType, "");
            //Check if 3G network
            nwType = strtok(networkType, ",");
            while(sim_index > 0) {
                if (nwType != NULL) {
                    nwType = strtok(NULL,",");
                }
                sim_index--;
            }

            if (nwType == NULL) {
                LOGD("requestSetupData: Not register yet\n");
            } else {
                LOGD("requestSetupData: registered in [%s] NW\n", nwType);
                if ((0 == strcmp(nwType, "UMTS")) ||
                    (0 == strcmp(nwType, "HSDPA")) ||
                    (0 == strcmp(nwType, "HSUPA")) ||
                    (0 == strcmp(nwType, "HSPA")) ||
                    (0 == strcmp(nwType, "HSPAP"))) {
                    //Check if IPv4v6 and default APN
                    if ((NULL != apn) &&
                       (NULL == strstr(apn, "mms")) &&
                       (IPV4V6 == protocol)) {
                        LOGD("requestSetupData: match FALLBACK PDP retry support\n");
                        ret = 1;
                        return ret;
                    }
                }
            }
        }
    }
    LOGD("requestSetupData: Not FALLBACK PDP retry \n");

    return ret;
}

int isPsDualTalkMode()
{
    if (isCdmaSupport()) {
        // TODO: need check for DSDA
        return 0;
    } else {
        return isDualTalkMode();
    }
}

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_SETUP_DATA_CALL:
            if (getExternalModemSlot() >= 0) {
#ifdef MTK_RIL_MD2
                MTK_REQUEST_SETUP_PPP_CALL(data, datalen, t);
#else
                MTK_REQUEST_SETUP_DATA_CALL(data, datalen, t);
#endif
            } else {
                MTK_REQUEST_SETUP_DATA_CALL(data, datalen, t);
            }
            break;
        case RIL_REQUEST_DATA_CALL_LIST:
            MTK_REQUEST_DATA_CALL_LIST(data, datalen, t);
            break;
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
            MTK_REQUEST_DEACTIVATE_DATA_CALL(data, datalen, t);
            break;
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
            MTK_REQUEST_SET_INITIAL_ATTACH_APN(data, datalen, t);
            break;
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
            MTK_REQUEST_LAST_DATA_CALL_FAIL_CAUSE(data, datalen, t);
            break;
        case RIL_REQUEST_SET_SCRI:
            MTK_REQUEST_SET_SCRI(data, datalen, t);
            break;
        //[New R8 modem FD]
        case RIL_REQUEST_SET_FD_MODE:
            MTK_REQUEST_FD_MODE(data, datalen, t);
            break;
        case RIL_REQUEST_DETACH_PS:
            MTK_REQUEST_DATA_IDLE(data, datalen, t);
            break;
        case RIL_REQUEST_ALLOW_DATA:
            MTK_REQUEST_ALLOW_DATA(data, datalen, t);
            break;
        //VoLTE
        case RIL_REQUEST_SETUP_DEDICATE_DATA_CALL:
            requestSetupDedicateDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL:
            requestDeactivateDedicateDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_MODIFY_DATA_CALL:
            requestModifyDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_ABORT_SETUP_DATA_CALL:
            requestAbortSetupDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_PCSCF_DISCOVERY_PCO:
            requestPcscfPco(data, datalen, t);
            break;
        case RIL_REQUEST_CLEAR_DATA_BEARER:
            requestClearDataBearer(data,datalen,t);
            break;
        case RIL_REQUEST_SET_DATA_ON_TO_MD:
            MTK_REQUEST_SET_DATA_ON_TO_MD(data, datalen, t);
            break;
        case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE:
            MTK_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE(data, datalen, t);
            break;
        // M: [C2K][IRAT] request start. {@
		case RIL_REQUEST_SET_ACTIVE_PS_SLOT:
			requestSetActivePsSimSlot(data, datalen, t);
			break;
		case RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE:
			confirmIratChange(data, datalen, t);
			break;
                case RIL_REQUEST_DEACTIVATE_LINK_DOWN_PDN:
                        requestDeactivateLinkdownPdn(data, datalen, t);
                        break;
		// M: [C2K][IRAT] request end. @}
        // M: [LTE][Low Power][UL traffic shaping] Start
        case RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT:
            MTK_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT(data, datalen, t);
            break;
        case RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER:
            MTK_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER(data, datalen, t);
            break;
        // M: [LTE][Low Power][UL traffic shaping] End
        // M: [IMS][EPDG] requst start. {@
        case RIL_REQUEST_HANDOVER_IND:
            MTK_RIL_REQUEST_HANDOVER_IND(data, datalen, t);
            break;
        // M: [IMS][EPDG] request end. @}
        // M: [IMS][EPDG] WiFi disconnect ind start {@
        case RIL_REQUEST_WIFI_DISCONNECT_IND:
            requestWifiDisconnectInd(data, datalen, t);
            break;
        // @}
        default:
            return 0; /* no matched request */
    }

    return 1; /* request found and handled */

}

extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    UNUSED(sms_pdu);
    int rilid = getRILIdByChannelCtx(p_channel);
    RILChannelCtx* pDataChannel = getChannelCtxbyId(sCmdChannel4Id[rilid]);

    if (strStartsWith(s, "+CGEV: NW DEACT") || strStartsWith(s, "+CGEV: ME DEACT") ||
        strStartsWith(s, "+CGEV: NW REACT") ||
        strStartsWith(s, "+CGEV: NW PDN DEACT") || strStartsWith(s, "+CGEV: ME PDN DEACT"))
    {
		// Ignore PDN deact message report from source RAT during Irat.
		if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
			LOGD("[RILData_GSM_URC] Ignore source PDN deact during IRAT: status = %d, urc = %s.",
					nIratAction, s);
            TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                    sizeof(TimeCallbackParam));
            param->rid = rilid;
            RIL_requestProxyTimedCallback(onUpdateIratStatus, param, &TIMEVAL_0,
                    pDataChannel->id, "onUpdateIratStatus");
		} else {
			char* urc = NULL;
			asprintf(&urc, "%s", s);
			TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
					sizeof(TimeCallbackParam));
			param->rid = rilid;
			param->urc = urc;

			RIL_requestProxyTimedCallback(onPdnDeactResult, param, &TIMEVAL_0,
					pDataChannel->id, "onPdnDeactResult");
		}
        return 1;
    } else if (strStartsWith(s, "+CGEV: NW ACT") || strStartsWith(s, "+CGEV: NW MODIFY")) {
        //+CGEV: NW ACT <p_cid>, <cid>, <event_type>
        //+CGEV: NW MODIFY <cid>, <change_reason>, <event_type>
        LOGD("rilDataUnsolicited receive bearer activation/modification by network [%s]", s);
        char* urc = NULL;
        asprintf(&urc, "%s", s);

        TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
        param->rid = rilid;
        param->urc = urc;
        if (strStartsWith(s, "+CGEV: NW MODIFY"))
            param->isModification = 1;
        else
            param->isModification = 0;

        //be aware that the param->urc & param should be free in onNetworkBearerUpdate
        RIL_requestProxyTimedCallback (onNetworkBearerUpdate,
               param,
               &TIMEVAL_0,
               pDataChannel->id,
               "onNetworkBearerUpdate");
        return 1;
    } else if (strStartsWith(s, "+CGEV: ME")) {
        if (strStartsWith(s, "+CGEV: ME PDN ACT")) {
            char *p = strstr(s, ME_PDN_URC);
            if( p == NULL) {
                return 1;
            }
            p = p + strlen(ME_PDN_URC) + 1;

            MePdnActiveInfo* pInfo = calloc(1, sizeof(MePdnActiveInfo));

            if (NULL == pInfo) {
                LOGD("rilDataUnsolicited allocate pInfo fail!!");
                return 1;
            }

            pInfo->activeCid = atoi(p);
            pInfo->pDataChannel = pDataChannel;

            LOGD("rilDataUnsolicited CID%d is activated and current state is %d", pInfo->activeCid, pdn_info[pInfo->activeCid].active);

            RIL_requestProxyTimedCallback (onMePdnActive,
                (void *) pInfo,
                &TIMEVAL_0,
                pDataChannel->id,
                "onMePdnActive");
        } else {
            //+CGEV: ME related cases should be handled in setup data call request handler
            LOGD("rilDataUnsolicited ignore +CGEV: ME cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV:")) {
		if (strStartsWith(s, "+CGEV: NW DETACH")) {
			// TODO: the action is not update to source finished since the time callback proxy is not executed yet, think a better way here.
			if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
				LOGD("[RILData_GSM_URC] Ignore source NW DETACH during IRAT: status = %d, urc = %s.",
						nIratAction, s);
				return 1;
			}
		}

        if (s_md_off) {
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rilid);
         } else {
            LOGD("rilDataUnsolicited before data call list: urc = %s.", s);
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rilid],
                &TIMEVAL_0,
                pDataChannel->id,
                "onDataCallListChanged");
         }

        return 1;

#ifdef WORKAROUND_FAKE_CGEV
    } else if (strStartsWith(s, "+CME ERROR: 150")) {

        if (s_md_off)    {
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rilid);
         } else {
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rilid],
                &TIMEVAL_0,
                pDataChannel->id,
                "onDataCallListChanged");
         }

        return 1;

#endif /* WORKAROUND_FAKE_CGEV */

    } else if(strStartsWith(s, "+ESCRI:")) {
        onScriResult((char*) s,(RIL_SOCKET_ID) rilid);
        return 1;
    } else if (strStartsWith(s, "+ECODE33")) {
        //M: CC33.
        RIL_onUnsolicitedResponse(RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, NULL, 0, (RIL_SOCKET_ID) rilid);
        return 1;
    } else if (strStartsWith(s, "+EMOBD:")) {
        onMoDataBarring((char*) s,(RIL_SOCKET_ID) rilid);
        return 1;
	//M: [C2K][IRAT] URC start. {@
	} else if (strStartsWith(s, "+EI3GPPIRAT:")) {
		LOGD("[RILData_GSM_URC] EI3GPPIRAT = %s.", s);
		char* urc = NULL;
		asprintf(&urc, "%s", s);
		TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
				sizeof(TimeCallbackParam));
		param->rid = rilid;
		param->urc = urc;
		RIL_requestProxyTimedCallback(onIratEvent, param, &TIMEVAL_0,
				pDataChannel->id, "onIratEvent");
		return 1;
	} else if (strStartsWith(s, "+EGCONTRDP:")) {
		LOGD("[RILData_GSM_URC] EGCONTRDP nIratAction = %d.", nIratAction);

		char* urc = NULL;
		asprintf(&urc, "%s", s);
		TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
				sizeof(TimeCallbackParam));
		param->rid = rilid;
		param->urc = urc;
		RIL_requestProxyTimedCallback(onPdnSyncFromSourceRat, param, &TIMEVAL_0,
				pDataChannel->id, "onPdnSyncFromSourceRat");
		return 1;
    //M: [C2K][IRAT] URC end. @}
    } else if (strStartsWith(s, "+EAPN:")) {
        if (strStartsWith(s, "+EAPN: \"this_is_an_invalid_apn\"")) {
            TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
            param->rid = rilid;

            RIL_requestProxyTimedCallback (onAttachApnReset,
                   param,
                   &TIMEVAL_0,
                   pDataChannel->id,
                   "onAttachApnReset");
        } else {
            LOGD("rilDataUnsolicited(): not handle message of +EAPN");
        }
        return 1;
    }

    return 0;
}

/// [IMS][EPDG] code start. {@
void dumpDefaultBearerConfig(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig) {
    if (pDefaultBearerVaConfig != NULL) {
        LOGD("dump defaultBearerVaConfig QOS[qci=%d, dlGbr=%d, ulGbr=%d, dlMbr=%d, \
ulMbr=%d] flag[emergency_ind=%d, pcscf_discovery_flag=%d, signalingFlag=%d, assigned_rat=%d]",
        pDefaultBearerVaConfig->qos.qci, pDefaultBearerVaConfig->qos.dlGbr,
        pDefaultBearerVaConfig->qos.ulGbr, pDefaultBearerVaConfig->qos.dlMbr,
        pDefaultBearerVaConfig->qos.ulMbr, pDefaultBearerVaConfig->emergency_ind,
        pDefaultBearerVaConfig->pcscf_discovery_flag, pDefaultBearerVaConfig->signalingFlag,
        pDefaultBearerVaConfig->assigned_rate);
    }
}
/// [IMS][EPDG] code end. @}


