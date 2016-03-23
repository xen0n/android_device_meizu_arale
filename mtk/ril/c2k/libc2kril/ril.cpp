/* //device/libs/telephony/ril.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "C2K_RILC"

#include <hardware_legacy/power.h>

#include <telephony/ril.h>
#include <telephony/oem-ril.h>
#include <telephony/via-ril.h>
#include <telephony/ril_cdma_sms.h>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <cutils/record_stream.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <binder/Parcel.h>
#include <cutils/jstring.h>

#include <sys/types.h>
#include <sys/limits.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <alloca.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <cutils/properties.h>

#include <libc2kril.h>

namespace android {

#define PHONE_PROCESS "radio"

#define SOCKET_NAME_RIL "rild-via"
#define SOCKET2_NAME_RIL "rild2-via"
#define SOCKET3_NAME_RIL "rild3-via"
#define SOCKET4_NAME_RIL "rild4-via"
#define SOCKET_NAME_RIL_DEBUG "rild-debug"
#define SOCKET_NAME_RPC "rpc"
#define SOCKET_NAME_RIL_CTCLIENT    "rild-ctclient"

#define SOCKET_NAME_ATCI_C2K "rild-atci-c2k"


#define ANDROID_WAKE_LOCK_NAME "radio-interface"


#define PROPERTY_RIL_IMPL "gsm.version.ril-impl"

// match with constant in RIL.java
#define MAX_COMMAND_BYTES (8 * 1024)

//#define PARCEL_COMMAND_BYTES 16
#define PARCEL_COMMAND_BYTES 1024
//c+w
// Basically: memset buffers that the client library
// shouldn't be using anymore in an attempt to find
// memory usage issues sooner.
#define MEMSET_FREED 1

#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1

/* Negative values for private RIL errno's */
#define RIL_ERRNO_INVALID_RESPONSE -1

// request, response, and unsolicited msg print macro
#define PRINTBUF_SIZE 8096

// Enable RILC log
#define RILC_LOG 0

#if RILC_LOG
    #define startRequest           sprintf(printBuf, "(")
    #define closeRequest           sprintf(printBuf, "%s)", printBuf)
    #define printRequest(token, req)           \
            LOGD("[%04d]> %s %s", token, requestToString(req), printBuf)

    #define startResponse           sprintf(printBuf, "%s {", printBuf)
    #define closeResponse           sprintf(printBuf, "%s}", printBuf)
    #define printResponse           LOGD("%s", printBuf)

    #define clearPrintBuf           printBuf[0] = 0
    #define removeLastChar          printBuf[strlen(printBuf)-1] = 0
    #define appendPrintBuf(x...)    sprintf(printBuf, x)
#else
    #define startRequest
    #define closeRequest
    #define printRequest(token, req)
    #define startResponse
    #define closeResponse
    #define printResponse
    #define clearPrintBuf
    #define removeLastChar
    #define appendPrintBuf(x...)
#endif

/*******************************************************************/

RIL_RadioFunctions s_callbacks = {0, NULL, NULL, NULL, NULL, NULL, NULL};
static int s_registerCalled = 0;

static pthread_t s_tid_dispatch;
static pthread_t s_tid_reader;
static int s_started = 0;

static int s_fdListen = -1;
static int s_fdCommand = -1;
static int s_fdDebug = -1;
static int s_fdRpc = -1;
static int s_fdRpcCommand = -1;

static int s_fdCTClientListen = -1;
static int s_fdCTClientCommand = -1;
static struct ril_event s_ctclient_listen_event;
static struct ril_event s_ctclient_command_event;
static pthread_mutex_t s_ctclient_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_rpc_writeMutex = PTHREAD_MUTEX_INITIALIZER;

static int s_fdAtciListen = -1;
static int s_fdAtciCommand = -1;
static struct ril_event s_ATCIlisten_event;
static struct ril_event s_ATCIcommand_event;

static int s_fdWakeupRead;
static int s_fdWakeupWrite;

static struct ril_event s_commands_event;
static struct ril_event s_wakeupfd_event;
static struct ril_event s_listen_event;
static struct ril_event s_wake_timeout_event;
static struct ril_event s_debug_event;
static struct ril_event s_rpc_event;
static struct ril_event s_rpc_command_event;

static const struct timeval TIMEVAL_WAKE_TIMEOUT = {1,0};

static pthread_mutex_t s_pendingRequestsMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_startupMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_startupCond = PTHREAD_COND_INITIALIZER;
static SocketListenParam s_ril_param_socket;

#if (SIM_COUNT >= 2)
static struct ril_event s_commands_event_socket2;
static struct ril_event s_listen_event_socket2;
static SocketListenParam s_ril_param_socket2;

static pthread_mutex_t s_pendingRequestsMutex_socket2  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex_socket2            = PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests_socket2          = NULL;
#endif

#if (SIM_COUNT >= 3)
static struct ril_event s_commands_event_socket3;
static struct ril_event s_listen_event_socket3;
static SocketListenParam s_ril_param_socket3;

static pthread_mutex_t s_pendingRequestsMutex_socket3  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex_socket3            = PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests_socket3          = NULL;
#endif

#if (SIM_COUNT >= 4)
static struct ril_event s_commands_event_socket4;
static struct ril_event s_listen_event_socket4;
static SocketListenParam s_ril_param_socket4;

static pthread_mutex_t s_pendingRequestsMutex_socket4  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex_socket4            = PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests_socket4          = NULL;
#endif

#if defined(ANDROID_MULTI_SIM)
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c), (d))
#define CALL_ONREQUEST(a, b, c, d, e) s_callbacks.onRequest((a), (b), (c), (d), (e))
#define CALL_ONSTATEREQUEST(a) s_callbacks.onStateRequest(a)
#else
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c))
#define CALL_ONREQUEST(a, b, c, d, e) s_callbacks.onRequest((a), (b), (c), (d))
#define CALL_ONSTATEREQUEST(a) s_callbacks.onStateRequest()
#endif

// For multi channel support
pthread_mutex_t s_dispatchMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_dispatchCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t s_queryThreadMutex = PTHREAD_MUTEX_INITIALIZER;
RequestInfoProxy* s_Proxy[RIL_SUPPORT_CHANNELS] = {NULL};
pthread_t s_tid_proxy[RIL_SUPPORT_CHANNELS];
CurExecCmd s_CurExecCmd[RIL_SUPPORT_CHANNELS] = {0};

static RequestInfo *s_pendingRequests = NULL;

static RequestInfo *s_toDispatchHead = NULL;
static RequestInfo *s_toDispatchTail = NULL;

static UserCallbackInfo *s_last_wake_timeout_info = NULL;

static void *s_lastNITZTimeData = NULL;
static size_t s_lastNITZTimeDataSize;

#if RILC_LOG
    static char printBuf[PRINTBUF_SIZE];
#endif

extern "C"
char rild[MAX_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL;
static int sendResponse (Parcel &p, RIL_SOCKET_ID socket_id);

/*******************************************************************/

static void dispatchVoid (Parcel& p, RequestInfo *pRI);
static void dispatchString (Parcel& p, RequestInfo *pRI);
static void dispatchStrings (Parcel& p, RequestInfo *pRI);
static void dispatchInts (Parcel& p, RequestInfo *pRI);
static void dispatchDial (Parcel& p, RequestInfo *pRI);
static void dispatchSIM_IO (Parcel& p, RequestInfo *pRI);
/* Adapt MTK Multi-Application, 2015/01/10 {*/
static void dispatchSIM_IO_EX (Parcel& p, RequestInfo *pRI);
/* Adapt MTK Multi-Application, 2015/01/10 }*/
static void dispatchSIM_APDU (Parcel& p, RequestInfo *pRI);
static void dispatchCallForward(Parcel& p, RequestInfo *pRI);
static void dispatchRaw(Parcel& p, RequestInfo *pRI);
static void dispatchSmsWrite (Parcel &p, RequestInfo *pRI);
static void dispatchDataCall (Parcel& p, RequestInfo *pRI);
static void dispatchVoiceRadioTech (Parcel& p, RequestInfo *pRI);
static void dispatchSetInitialAttachApn (Parcel& p, RequestInfo *pRI);
static void dispatchCdmaSubscriptionSource (Parcel& p, RequestInfo *pRI);

static void dispatchCdmaSms(Parcel &p, RequestInfo *pRI);
static void dispatchImsSms(Parcel &p, RequestInfo *pRI);
static void dispatchImsCdmaSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef);
static void dispatchImsGsmSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef);
static void dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI);
static void dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI);
static void dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI);
static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI);

static void dispatchWritePbToRuim(Parcel &p, RequestInfo *pRI);
static void dispatchMTKWritePbToRuim(Parcel &p, RequestInfo *pRI);

static void dispatchNVReadItem(Parcel &p, RequestInfo *pRI);
static void dispatchNVWriteItem(Parcel &p, RequestInfo *pRI);
static void dispatchUiccSubscripton(Parcel &p, RequestInfo *pRI);
static void dispatchSimAuthentication(Parcel &p, RequestInfo *pRI);
static void dispatchDataProfile(Parcel &p, RequestInfo *pRI);
static void dispatchRadioCapability(Parcel &p, RequestInfo *pRI);
static int responseInts(Parcel &p, void *response, size_t responselen);
static int responseFailCause(Parcel &p, void *response, size_t responselen);
static int responseStrings(Parcel &p, void *response, size_t responselen);
static int responseString(Parcel &p, void *response, size_t responselen);
static int responseVoid(Parcel &p, void *response, size_t responselen);
static int responseCallList(Parcel &p, void *response, size_t responselen);
static int responseSMS(Parcel &p, void *response, size_t responselen);
static int responseSIM_IO(Parcel &p, void *response, size_t responselen);
static int responseCallForwards(Parcel &p, void *response, size_t responselen);
static int responseDataCallList(Parcel &p, void *response, size_t responselen);
static int responseSetupDataCall(Parcel &p, void *response, size_t responselen);
static int responseRaw(Parcel &p, void *response, size_t responselen);
static int responseSsn(Parcel &p, void *response, size_t responselen);
static int responseSimStatus(Parcel &p, void *response, size_t responselen);
static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responseCdmaSms(Parcel &p, void *response, size_t responselen);
static int responseCellList(Parcel &p, void *response, size_t responselen);
static int responseCdmaInformationRecords(Parcel &p,void *response, size_t responselen);
static int responseRilSignalStrength(Parcel &p,void *response, size_t responselen);
static int responseCallRing(Parcel &p, void *response, size_t responselen);
static int responseCdmaSignalInfoRecord(Parcel &p,void *response, size_t responselen);
static int responseCdmaCallWaiting(Parcel &p,void *response, size_t responselen);
static int responseSimRefresh(Parcel &p, void *response, size_t responselen);
static int responseCellInfoList(Parcel &p, void *response, size_t responselen);
static int responseHardwareConfig(Parcel &p, void *response, size_t responselen);
static int responseDcRtInfo(Parcel &p, void *response, size_t responselen);
static int responseRadioCapability(Parcel &p, void *response, size_t responselen);

static int responseReadPbFromRuim(Parcel &p, void *response, size_t responselen);
static int responseMTKReadPbFromRuim(Parcel &p, void *response, size_t responselen);
static int responseGetNitzTime(Parcel &p, void *response, size_t responselen);
static bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType);
static int responseSSData(Parcel &p, void *response, size_t responselen);
static int responseLceStatus(Parcel &p, void *response, size_t responselen);
static int responseLceData(Parcel &p, void *response, size_t responselen);
static int responseActivityData(Parcel &p, void *response, size_t responselen);

static int decodeVoiceRadioTechnology (RIL_RadioState radioState);
static int decodeCdmaSubscriptionSource (RIL_RadioState radioState);
static RIL_RadioState processRadioState(RIL_RadioState newRadioState);

#ifdef ADD_MTK_REQUEST_URC
static void dispatchPlayDTMFTone(Parcel &p, RequestInfo *pRI);
static void dispatchPlayToneSeq(Parcel &p, RequestInfo *pRI);
static int responseGetSmsStatus(Parcel &p, void *response, size_t responselen);
static void onRequestUnsupportComplete(int32_t token,RIL_SOCKET_ID socket_id);

/// M: [C2K] IRAT feature. @{
static int responseIratStateChange(Parcel &p, void *response, size_t responselen);
/// @}

#endif
extern "C" const char * requestToString(int request);
extern "C" const char * failCauseToString(RIL_Errno);
extern "C" const char * callStateToString(RIL_CallState);
extern "C" const char * radioStateToString(RIL_RadioState);

#ifdef RIL_SHLIB
#if defined(ANDROID_MULTI_SIM)
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id);
#else
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
                                size_t datalen);
#endif
#endif

static UserCallbackInfo * internalRequestTimedCallback
    (RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);

static void rilEventAddWakeup(struct ril_event *ev);

/** Index == requestNumber */
static CommandInfo s_commands[] = {
#include "ril_commands.h"
};

static UnsolResponseInfo s_unsolResponses[] = {
#include "ril_unsol_commands.h"
};

/* For older RILs that do not support new commands RIL_REQUEST_VOICE_RADIO_TECH and
   RIL_UNSOL_VOICE_RADIO_TECH_CHANGED messages, decode the voice radio tech from
   radio state message and store it. Every time there is a change in Radio State
   check to see if voice radio tech changes and notify telephony
 */
int voiceRadioTech = -1;

/* For older RILs that do not support new commands RIL_REQUEST_GET_CDMA_SUBSCRIPTION_SOURCE
   and RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED messages, decode the subscription
   source from radio state and store it. Every time there is a change in Radio State
   check to see if subscription source changed and notify telephony
 */
int cdmaSubscriptionSource = -1;

/* For older RILs that do not send RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, decode the
   SIM/RUIM state from radio state and store it. Every time there is a change in Radio State,
   check to see if SIM/RUIM status changed and notify telephony
 */
int simRuimStatus = -1;

static char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

static status_t
readStringFromParcelInplace(Parcel &p, char *str, size_t maxLen) {
    size_t s16Len;
    const char16_t *s16;

    s16 = p.readString16Inplace(&s16Len);
    if (s16 == NULL) {
        return NO_MEMORY;
    }
    size_t strLen = strnlen16to8(s16, s16Len);
    if ((strLen + 1) > maxLen) {
        return NO_MEMORY;
    }
    if (strncpy16to8(str, s16, strLen) == NULL) {
        return NO_MEMORY;
    } else {
        return NO_ERROR;
    }
}

static void
memsetString (char *s) {
    if (s != NULL) {
        memset (s, 0, strlen(s));
    }
}

void   nullParcelReleaseFunction (const uint8_t* data, size_t dataSize,
                                    const size_t* objects, size_t objectsSize,
                                        void* cookie) {
    // do nothing -- the data reference lives longer than the Parcel object
}

const char *rilSocketIdToString(RIL_SOCKET_ID socket_id)
{
    switch(socket_id) {
        case RIL_SOCKET_1:
            return "RIL_SOCKET_1";
#if (SIM_COUNT >= 2)
        case RIL_SOCKET_2:
            return "RIL_SOCKET_2";
#endif
#if (SIM_COUNT >= 3)
        case RIL_SOCKET_3:
            return "RIL_SOCKET_3";
#endif
#if (SIM_COUNT >= 4)
        case RIL_SOCKET_4:
            return "RIL_SOCKET_4";
#endif
        default:
            return "not a valid RIL";
    }
}

/**
 * To be called from dispatch thread
 * Issue a single local request, ensuring that the response
 * is not sent back up to the command process
 */
static void
issueLocalRequest(int request, void *data, int len, RIL_SOCKET_ID socket_id) {
    RequestInfo *pRI;
    int ret;

    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests;

#if (SIM_COUNT == 2)
    if (socket_id == RIL_SOCKET_2) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket2;
        pendingRequestsHook = &s_pendingRequests_socket2;
    }
#endif

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 1;
    pRI->token = 0xffffffff;        // token is not used in this context

    pRI->pCI = &(s_commands[request]);

    pRI->socket_id = socket_id;

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);

    assert (ret == 0);

    LOGD("C[locl]> %s", requestToString(request));

    CALL_ONREQUEST(request, data, len, pRI, pRI->socket_id);
}

static int
processCommandBuffer(void *buffer, size_t buflen, int owner, RIL_SOCKET_ID socket_id) {
    Parcel p;
    status_t status;
    int32_t request;
    int32_t token;
    RequestInfo *pRI;
    int ret;

    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests;

    p.setData((uint8_t *) buffer, buflen);

    // status checked at end
    status = p.readInt32(&request);
    status = p.readInt32 (&token);
    LOGW("token = %d", token);
    LOGW("request = %d", request);
    if (status != NO_ERROR) {
        LOGE("invalid request block");
        return 0;
    }
    RLOGD("SOCKET %s REQUEST: %s length:%d", rilSocketIdToString(socket_id), requestToString(request), buflen);

#if (SIM_COUNT >= 2)
    if (socket_id == RIL_SOCKET_2) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket2;
        pendingRequestsHook = &s_pendingRequests_socket2;
    }
#if (SIM_COUNT >= 3)
    else if (socket_id == RIL_SOCKET_3) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket3;
        pendingRequestsHook = &s_pendingRequests_socket3;
    }
#endif
#if (SIM_COUNT >= 4)
    else if (socket_id == RIL_SOCKET_4) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket4;
        pendingRequestsHook = &s_pendingRequests_socket4;
    }
#endif
#endif

    #ifdef ADD_MTK_REQUEST_URC
    switch(request) {
        case 2084:
            request = RIL_REQUEST_HANGUP_ALL;
            break;
        case 2012:
            request = RIL_REQUEST_QUERY_PHB_STORAGE_INFO;
            break;
        case 2013:
            request = RIL_REQUEST_WRITE_PHB_ENTRY;
            break;
        case 2014:
            request = RIL_REQUEST_READ_PHB_ENTRY;
            break;
        case 2087:
            request = RIL_REQUEST_EMERGENCY_DIAL;
            break;
        case 2026:
            request = RIL_REQUEST_QUERY_ICCID;
            break;
        case 2029:
            request = RIL_REQUEST_GET_SMS_SIM_MEM_STATUS;
            break;
        case 2053:
            request = RIL_REQUEST_SIM_TRANSMIT_BASIC;
            break;
        case 2054:
            request = RIL_REQUEST_SIM_TRANSMIT_CHANNEL;
            break;
        case 2055:
            request = RIL_REQUEST_SIM_GET_ATR;
            break;
        case 2063:
            request = RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW;
            break;
        /* Adapt MTK Multi-Application, 2015/01/10 {*/
        case 2093:
            request = RIL_REQUEST_SIM_IO_EX;
            break;
        /* Adapt MTK Multi-Application, 2015/01/10 }*/
        case 2100:
            request = RIL_REQUEST_RADIO_POWER_CARD_SWITCH;
            break;
        case 2117:
            request = RIL_REQUEST_CONFIG_MODEM_STATUS;
            break;
/*        case 2011:
            request = RIL_REQUEST_RADIO_POWEROFF;
            break;
        case 2033:
            request = RIL_REQUEST_RADIO_POWERON;
            break;*/
        case 2010:
            request = RIL_REQUEST_MODEM_POWEROFF;
            break;
        case 2028:
            request = RIL_REQUEST_MODEM_POWERON;
            break;
        case 4000:
            request = RIL_REQUEST_GET_NITZ_TIME;
            break;
        case 4001:
            request = RIL_REQUEST_QUERY_UIM_INSERTED;
            break;
        case 4002:
            request = RIL_REQUEST_SWITCH_HPF;
            break;
        case 4003:
            request = RIL_REQUEST_SET_AVOID_SYS;
            break;
        case 4004:
            request = RIL_REQUEST_QUERY_AVOID_SYS;
            break;
        case 4005:
            request = RIL_REQUEST_QUERY_CDMA_NETWORK_INFO;
            break;
        case 4006:
            request = RIL_REQUEST_GET_LOCAL_INFO;
            break;
        case 4007:
            request = RIL_REQUEST_UTK_REFRESH;
            break;
        case 4008:
            request = RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS;
            break;
        case 4009:
            request = RIL_REQUEST_QUERY_NETWORK_REGISTRATION;
            break;
        case 4010:
            request = RIL_REQUEST_AGPS_TCP_CONNIND;
            break;
        case 4011:
            request = RIL_REQUEST_AGPS_SET_MPC_IPPORT;
            break;
        case 4012:
            request = RIL_REQUEST_AGPS_GET_MPC_IPPORT;
            break;
        case 4013:
            request = RIL_REQUEST_SET_MEID;
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case 4014:
            request = RIL_REQUEST_RESUME_REGISTRATION_CDMA;
            break;
        case 4015:
            request = RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA;
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        case 4016:
            request = RIL_REQUEST_SET_ETS_DEV;
            break;
        case 4017:
            request = RIL_REQUEST_WRITE_MDN;
            break;
        case 2043:
            request = RIL_REQUEST_SET_TRM;
            break;
        case 4019:
            request = RIL_REQUEST_SET_ARSI_THRESHOLD;
            break;
        case 4021:
            request = RIL_REQUEST_NOTIFY_SIM_HOTPLUG;
            break;
        case 4022:
            request = RIL_REQUEST_CONFIG_EVDO_MODE;
            break;
        //Add for UTK IR case
        case 4023:
            request = RIL_REQUEST_QUERY_UTK_MENU_FROM_MD;
            break;
        case 4024:
            request = RIL_REQUEST_QUERY_STK_MENU_FROM_MD;
            break;
        // [C2K] IRAT confirm rat change request.
        case 2119:
            request = RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE;
            break;
        // [C2K] HD Voice icon information request.
        case 2110:
            request = RIL_REQUEST_SET_SPEECH_CODEC_INFO;
            break;
        /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
        case 2120:
            request = RIL_REQUEST_SET_SVLTE_RAT_MODE;
            break;
        /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}
        case 2131:
            request = RIL_REQUEST_SWITCH_CARD_TYPE;
            break;
        /// M: [C2K][SIM] Support for enable/disable MD3 sleep feature @}
        case 2132:
            request = RIL_REQUEST_ENABLE_MD3_SLEEP;
            break;
        default:
            break;
    }
    #endif
    if (request < 1 || request >= (int32_t)NUM_ELEMS(s_commands)) {
        LOGE("unsupported request code %d token %d socket_id %d", request, token, socket_id);
        onRequestUnsupportComplete(token, socket_id);
        // FIXME this should perhaps return a response
        return 0;
    }


    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->token = token;
    pRI->pCI = &(s_commands[request]);
    pRI->owner = owner;
    pRI->socket_id = socket_id;

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;
    ret = pthread_mutex_unlock(pendingRequestsMutexHook);

    assert (ret == 0);

#ifdef ADD_MTK_REQUEST_URC
    RequestInfoProxy * proxy = NULL;
    int proxyId;
    const char* proxyName = NULL;
    int queueSize = 0;

    proxy = (RequestInfoProxy *) calloc(1, sizeof(RequestInfoProxy));
    proxy->p = new Parcel();

    assert(proxy);

    proxy->p_next = NULL;

    proxy->pRI = pRI;

    proxy->pUCI = NULL;

    proxy->p->setData((uint8_t *) buffer, buflen);

    int temBuffer;
    status = proxy->p->readInt32(&temBuffer);
    // Make sure the request ID not changed after wrap into proxy
    assert(temBuffer == request);
    status = proxy->p->readInt32 (&temBuffer);
    // Make sure the token not changed after wrap into proxy
    assert(temBuffer == token);

    assert(status == NOERROR);

    proxyId = s_commands[request].proxyId;
    /* Save dispatched proxy in RequestInfo */
    proxyName = proxyIdToString(proxyId);
    proxy->pRI->cid = (RILChannelId) proxyId;
    RLOGI("processCommandBuffer enqueueProxyList &s_Proxy[proxyId]=%d, request=%d, proxyId=%d", &s_Proxy[proxyId], request, proxyId);
    queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);
    if (0 != queueSize) {
        RLOGD("%s is busy. %s queued. total:%d requests pending", proxyName,
                requestToString(request), queueSize + 1);
    }
#else
    /*    sLastDispatchedToken = token; */

    /*in case of rild crashed by null dispatch function*/
    if (pRI->pCI->dispatchFunction) {
        pRI->pCI->dispatchFunction(p, pRI);
    } else {
        LOGE("invalid dispatchFunction, need check request list: requestNumber is %d", request);
    }
#endif
    return 0;
}

static void
invalidCommandBlock (RequestInfo *pRI) {
    LOGE("invalid command block for token %d request %s",
                pRI->token, requestToString(pRI->pCI->requestNumber));
}

/** Callee expects NULL */
static void
dispatchVoid (Parcel& p, RequestInfo *pRI) {
    clearPrintBuf;
    printRequest(pRI->token, pRI->pCI->requestNumber);
    CALL_ONREQUEST(pRI->pCI->requestNumber, NULL, 0, pRI, pRI->socket_id);
}

/** Callee expects const char * */
static void
dispatchString (Parcel& p, RequestInfo *pRI) {
    status_t status;
    size_t datalen;
    size_t stringlen;
    char *string8 = NULL;

    string8 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%s", printBuf, string8);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, string8,
                       sizeof(char *), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(string8);
#endif

    free(string8);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/** Callee expects const char ** */
static void
dispatchStrings (Parcel &p, RequestInfo *pRI) {
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)alloca(sizeof(char *));
        datalen = 0;
    } else if (((int)countStrings) == -1) {
        pStrings = NULL;
        datalen = 0;
    } else {
        datalen = sizeof(char *) * countStrings;

        pStrings = (char **)alloca(datalen);

        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
            appendPrintBuf("%s%s,", printBuf, pStrings[i]);
        }
    }
    removeLastChar;
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, pStrings, datalen, pRI, pRI->socket_id);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString (pStrings[i]);
#endif
            free(pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
    }

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/** Callee expects const int * */
static void
dispatchInts (Parcel &p, RequestInfo *pRI) {
    int32_t count;
    status_t status;
    size_t datalen;
    int *pInts;

    status = p.readInt32 (&count);
    LOGI("dispatchInts count=%d, status=%d", count, status);

    if (status != NO_ERROR || count == 0) {
        goto invalid;
    }

    datalen = sizeof(int) * count;
    pInts = (int *)alloca(datalen);

    startRequest;
    for (int i = 0 ; i < count ; i++) {
        int32_t t;

        status = p.readInt32(&t);
        pInts[i] = (int)t;
        appendPrintBuf("%s%d,", printBuf, t);
        LOGI("dispatchInts, t=%d, status=%d", t, status);

        if (status != NO_ERROR) {
            goto invalid;
        }
   }
   removeLastChar;
   closeRequest;
   printRequest(pRI->token, pRI->pCI->requestNumber);

   CALL_ONREQUEST(pRI->pCI->requestNumber, const_cast<int *>(pInts),
                       datalen, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(pInts, 0, datalen);
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


/**
 * Callee expects const RIL_SMS_WriteArgs *
 * Payload is:
 *   int32_t status
 *   String pdu
 */
static void
dispatchSmsWrite (Parcel &p, RequestInfo *pRI) {
    RIL_SMS_WriteArgs args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    status = p.readInt32(&t);
    args.status = (int)t;

    args.pdu = strdupReadString(p);

    if (status != NO_ERROR || args.pdu == NULL) {
        goto invalid;
    }

    args.smsc = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%d,%s,smsc=%s", printBuf, args.status,
        (char*)args.pdu,  (char*)args.smsc);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (args.pdu);
#endif

    free (args.pdu);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_Dial *
 * Payload is:
 *   String address
 *   int32_t clir
 */
static void
dispatchDial (Parcel &p, RequestInfo *pRI) {
    RIL_Dial dial;
    RIL_UUS_Info uusInfo;
    int32_t sizeOfDial;
    int32_t t;
    int32_t uusPresent;
    status_t status;

    memset (&dial, 0, sizeof(dial));

    dial.address = strdupReadString(p);

    status = p.readInt32(&t);
    dial.clir = (int)t;

    if (status != NO_ERROR || dial.address == NULL) {
        goto invalid;
    }

    if (s_callbacks.version < 3) { // Remove when partners upgrade to version 3
        uusPresent = 0;
        sizeOfDial = sizeof(dial) - sizeof(RIL_UUS_Info *);
    } else {
        status = p.readInt32(&uusPresent);

        if (status != NO_ERROR) {
            goto invalid;
        }

        if (uusPresent == 0) {
            dial.uusInfo = NULL;
        } else {
            int32_t len;

            memset(&uusInfo, 0, sizeof(RIL_UUS_Info));

            status = p.readInt32(&t);
            uusInfo.uusType = (RIL_UUS_Type) t;

            status = p.readInt32(&t);
            uusInfo.uusDcs = (RIL_UUS_DCS) t;

            status = p.readInt32(&len);
            if (status != NO_ERROR) {
                goto invalid;
            }

            // The java code writes -1 for null arrays
            if (((int) len) == -1) {
                uusInfo.uusData = NULL;
                len = 0;
            } else {
                uusInfo.uusData = (char*) p.readInplace(len);
            }

            uusInfo.uusLength = len;
            dial.uusInfo = &uusInfo;
        }
        sizeOfDial = sizeof(dial);
    }

    startRequest;
    appendPrintBuf("%snum=%s,clir=%d", printBuf, dial.address, dial.clir);
    if (uusPresent) {
        appendPrintBuf("%s,uusType=%d,uusDcs=%d,uusLen=%d", printBuf,
                dial.uusInfo->uusType, dial.uusInfo->uusDcs,
                dial.uusInfo->uusLength);
    }
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &dial, sizeOfDial, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (dial.address);
#endif

    free (dial.address);

#ifdef MEMSET_FREED
    memset(&uusInfo, 0, sizeof(RIL_UUS_Info));
    memset(&dial, 0, sizeof(dial));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_SIM_IO *
 * Payload is:
 *   int32_t command
 *   int32_t fileid
 *   String path
 *   int32_t p1, p2, p3
 *   String data
 *   String pin2
 *   String aidPtr
 */
static void
dispatchSIM_IO (Parcel &p, RequestInfo *pRI) {
    union RIL_SIM_IO {
        RIL_SIM_IO_v6 v6;
        RIL_SIM_IO_v5 v5;
    } simIO;

    int32_t t;
    int size;
    status_t status;

    memset (&simIO, 0, sizeof(simIO));

// note we only check status at the end
//For NFC
    simIO.v6.cla = 0;
    if(pRI->pCI->requestNumber != RIL_REQUEST_SIM_IO) {
        status = p.readInt32(&t);
        simIO.v6.cla = (int)t;
    }
//For NFC

    status = p.readInt32(&t);
    simIO.v6.command = (int)t;

    status = p.readInt32(&t);
    simIO.v6.fileid = (int)t;

    simIO.v6.path = strdupReadString(p);

    status = p.readInt32(&t);
    simIO.v6.p1 = (int)t;

    status = p.readInt32(&t);
    simIO.v6.p2 = (int)t;

    status = p.readInt32(&t);
    simIO.v6.p3 = (int)t;

    simIO.v6.data = strdupReadString(p);
    simIO.v6.pin2 = strdupReadString(p);
    simIO.v6.aidPtr = strdupReadString(p);

    startRequest;
    appendPrintBuf("%scmd=0x%X,efid=0x%X,path=%s,%d,%d,%d,%s,pin2=%s,aid=%s", printBuf,
        simIO.v6.command, simIO.v6.fileid, (char*)simIO.v6.path,
        simIO.v6.p1, simIO.v6.p2, simIO.v6.p3,
        (char*)simIO.v6.data,  (char*)simIO.v6.pin2, simIO.v6.aidPtr);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    size = (s_callbacks.version < 6) ? sizeof(simIO.v5) : sizeof(simIO.v6);
    CALL_ONREQUEST(pRI->pCI->requestNumber, &simIO, size, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (simIO.v6.path);
    memsetString (simIO.v6.data);
    memsetString (simIO.v6.pin2);
    memsetString (simIO.v6.aidPtr);
#endif

    free (simIO.v6.path);
    free (simIO.v6.data);
    free (simIO.v6.pin2);
    free (simIO.v6.aidPtr);

#ifdef MEMSET_FREED
    memset(&simIO, 0, sizeof(simIO));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/* Adapt MTK Multi-Application, 2015/01/10 {*/
/**
 * Callee expects const RIL_SIM_IO *
 * Payload is:
 *   int32_t command
 *   int32_t fileid
 *   String path
 *   int32_t p1, p2, p3
 *   String data
 *   String pin2
 *   String aidPtr
 *   int32_t sessionId
 */
static void
dispatchSIM_IO_EX (Parcel &p, RequestInfo *pRI) {
    RIL_SIM_IO_EX_v6 simIO;

    int32_t t;
    int size;
    status_t status;

    memset (&simIO, 0, sizeof(simIO));

    // note we only check status at the end

    status = p.readInt32(&t);
    simIO.command = (int)t;

    status = p.readInt32(&t);
    simIO.fileid = (int)t;

    simIO.path = strdupReadString(p);

    status = p.readInt32(&t);
    simIO.p1 = (int)t;

    status = p.readInt32(&t);
    simIO.p2 = (int)t;

    status = p.readInt32(&t);
    simIO.p3 = (int)t;

    simIO.data = strdupReadString(p);
    simIO.pin2 = strdupReadString(p);
    simIO.aidPtr = strdupReadString(p);

    // Session Id
    status = p.readInt32(&t);
    simIO.sessionId = (int)t;

    startRequest;
    appendPrintBuf("%scmd=0x%X,efid=0x%X,path=%s,%d,%d,%d,%s,pin2=%s,aid=%s,session=%d", printBuf,
        simIO.command, simIO.fileid, (char*)simIO.path,
        simIO.p1, simIO.p2, simIO.p3,
        (char*)simIO.data,  (char*)simIO.pin2, simIO.aidPtr, simIO.sessionId);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    size = sizeof(simIO);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &simIO, size, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (simIO.path);
    memsetString (simIO.data);
    memsetString (simIO.pin2);
    memsetString (simIO.aidPtr);
#endif

    free (simIO.path);
    free (simIO.data);
    free (simIO.pin2);
    free (simIO.aidPtr);

#ifdef MEMSET_FREED
    memset(&simIO, 0, sizeof(simIO));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}
/* Adapt MTK Multi-Application, 2015/01/10 }*/

/**
 * Callee expects const RIL_SIM_APDU *
 * Payload is:
 *   int32_t sessionid
 *   int32_t cla
 *   int32_t instruction
 *   int32_t p1, p2, p3
 *   String data
 */
static void
dispatchSIM_APDU (Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    RIL_SIM_APDU apdu;

    memset (&apdu, 0, sizeof(RIL_SIM_APDU));

    // Note we only check status at the end. Any single failure leads to
    // subsequent reads filing.
    status = p.readInt32(&t);
    apdu.sessionid = (int)t;

    status = p.readInt32(&t);
    apdu.cla = (int)t;

    status = p.readInt32(&t);
    apdu.instruction = (int)t;

    status = p.readInt32(&t);
    apdu.p1 = (int)t;

    status = p.readInt32(&t);
    apdu.p2 = (int)t;

    status = p.readInt32(&t);
    apdu.p3 = (int)t;

    apdu.data = strdupReadString(p);

    startRequest;
    appendPrintBuf("%ssessionid=%d,cla=%d,ins=%d,p1=%d,p2=%d,p3=%d,data=%s",
        printBuf, apdu.sessionid, apdu.cla, apdu.instruction, apdu.p1, apdu.p2,
        apdu.p3, (char*)apdu.data);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &apdu, sizeof(RIL_SIM_APDU), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(apdu.data);
#endif
    free(apdu.data);

#ifdef MEMSET_FREED
    memset(&apdu, 0, sizeof(RIL_SIM_APDU));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


/**
 * Callee expects const RIL_CallForwardInfo *
 * Payload is:
 *  int32_t status/action
 *  int32_t reason
 *  int32_t serviceCode
 *  int32_t toa
 *  String number  (0 length -> null)
 *  int32_t timeSeconds
 */
static void
dispatchCallForward(Parcel &p, RequestInfo *pRI) {
    RIL_CallForwardInfo cff;
    int32_t t;
    status_t status;

    memset (&cff, 0, sizeof(cff));

    // note we only check status at the end

    status = p.readInt32(&t);
    cff.status = (int)t;

    status = p.readInt32(&t);
    cff.reason = (int)t;

    status = p.readInt32(&t);
    cff.serviceClass = (int)t;

    status = p.readInt32(&t);
    cff.toa = (int)t;

    cff.number = strdupReadString(p);

    status = p.readInt32(&t);
    cff.timeSeconds = (int)t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    // special case: number 0-length fields is null

    if (cff.number != NULL && strlen (cff.number) == 0) {
        cff.number = NULL;
    }

    startRequest;
    appendPrintBuf("%sstat=%d,reason=%d,serv=%d,toa=%d,%s,tout=%d", printBuf,
        cff.status, cff.reason, cff.serviceClass, cff.toa,
        (char*)cff.number, cff.timeSeconds);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &cff, sizeof(cff), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(cff.number);
#endif

    free (cff.number);

#ifdef MEMSET_FREED
    memset(&cff, 0, sizeof(cff));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


static void
dispatchRaw(Parcel &p, RequestInfo *pRI) {
    int32_t len;
    status_t status;
    const void *data;

    status = p.readInt32(&len);

    if (status != NO_ERROR) {
        goto invalid;
    }

    // The java code writes -1 for null arrays
    if (((int)len) == -1) {
        data = NULL;
        len = 0;
    }

    data = p.readInplace(len);

    startRequest;
    appendPrintBuf("%sraw_size=%d", printBuf, len);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, const_cast<void *>(data), len, pRI, pRI->socket_id);

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static status_t
constructCdmaSms(Parcel &p, RequestInfo *pRI, RIL_CDMA_SMS_Message& rcsm) {
    int32_t  t;
    uint8_t ut;
    status_t status;
    int32_t digitCount;
    int digitLimit;

    memset(&rcsm, 0, sizeof(rcsm));

    status = p.readInt32(&t);
    rcsm.uTeleserviceID = (int) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.bIsServicePresent = (uint8_t) ut;

    status = p.readInt32(&t);
    rcsm.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsm.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sAddress.number_of_digits= (uint8_t) ut;

    digitLimit= MIN((rcsm.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.odd = (uint8_t) ut;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.number_of_digits = (uint8_t) ut;

    digitLimit= MIN((rcsm.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sSubAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.uBearerDataLen = (int) t;

    digitLimit= MIN((rcsm.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut, sizeof(ut));
        rcsm.aBearerData[digitCount] = (uint8_t) ut;
    }

    if (status != NO_ERROR) {
        return status;
    }

    startRequest;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.Number_mode=%d, sAddress.number_type=%d, ",
            printBuf, rcsm.uTeleserviceID,rcsm.bIsServicePresent,rcsm.uServicecategory,
            rcsm.sAddress.digit_mode, rcsm.sAddress.number_mode,rcsm.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    return status;
}

static void
dispatchCdmaSms(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Message rcsm;

    ALOGD("dispatchCdmaSms");
    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsm, sizeof(rcsm),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsm, 0, sizeof(rcsm));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchImsCdmaSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    RIL_IMS_SMS_Message rism;
    RIL_CDMA_SMS_Message rcsm;

    ALOGD("dispatchImsCdmaSms: retry=%d, messageRef=%d", retry, messageRef);

    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }
    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP2;
    rism.retry = retry;
    rism.messageRef = messageRef;
    rism.message.cdmaMessage = &rcsm;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism,
            sizeof(RIL_RadioTechnologyFamily)+sizeof(uint8_t)+sizeof(int32_t)
            +sizeof(rcsm),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsm, 0, sizeof(rcsm));
    memset(&rism, 0, sizeof(rism));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchImsGsmSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    RIL_IMS_SMS_Message rism;
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;
    ALOGD("dispatchImsGsmSms: retry=%d, messageRef=%d", retry, messageRef);

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP;
    rism.retry = retry;
    rism.messageRef = messageRef;

    startRequest;
    appendPrintBuf("%sformat=%d,", printBuf, rism.messageRef);
    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)alloca(sizeof(char *));
        datalen = 0;
    } else if (((int)countStrings) == -1) {
        pStrings = NULL;
        datalen = 0;
    } else {
        datalen = sizeof(char *) * countStrings;

        pStrings = (char **)alloca(datalen);

        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
            appendPrintBuf("%s%s,", printBuf, pStrings[i]);
        }
    }
    removeLastChar;
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    rism.message.gsmMessage = pStrings;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism,
            sizeof(RIL_RadioTechnologyFamily)+sizeof(uint8_t)+sizeof(int32_t)
            +datalen, pRI, pRI->socket_id);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString (pStrings[i]);
#endif
            free(pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
    }

#ifdef MEMSET_FREED
    memset(&rism, 0, sizeof(rism));
#endif
    return;
invalid:
    ALOGE("dispatchImsGsmSms invalid block");
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchImsSms(Parcel &p, RequestInfo *pRI) {
    int32_t  t;
    status_t status = p.readInt32(&t);
    RIL_RadioTechnologyFamily format;
    uint8_t retry;
    int32_t messageRef;

    ALOGD("dispatchImsSms");
    if (status != NO_ERROR) {
        goto invalid;
    }
    format = (RIL_RadioTechnologyFamily) t;

    // read retry field
    status = p.read(&retry,sizeof(retry));
    if (status != NO_ERROR) {
        goto invalid;
    }
    // read messageRef field
    status = p.read(&messageRef,sizeof(messageRef));
    if (status != NO_ERROR) {
        goto invalid;
    }

    if (RADIO_TECH_3GPP == format) {
        dispatchImsGsmSms(p, pRI, retry, messageRef);
    } else if (RADIO_TECH_3GPP2 == format) {
        dispatchImsCdmaSms(p, pRI, retry, messageRef);
    } else {
        ALOGE("requestImsSendSMS invalid format value =%d", format);
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Ack rcsa;
    int32_t  t;
    status_t status;
    int32_t digitCount;

    memset(&rcsa, 0, sizeof(rcsa));

    status = p.readInt32(&t);
    rcsa.uErrorClass = (RIL_CDMA_SMS_ErrorClass) t;

    status = p.readInt32(&t);
    rcsa.uSMSCauseCode = (int) t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%suErrorClass=%d, uTLStatus=%d, ",
            printBuf, rcsa.uErrorClass, rcsa.uSMSCauseCode);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsa, 0, sizeof(rcsa));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    {
    RIL_GSM_BroadcastSmsConfigInfo gsmBci[num];
    RIL_GSM_BroadcastSmsConfigInfo *gsmBciPtrs[num];

    startRequest;
    for (int i = 0 ; i < num ; i++ ) {
        gsmBciPtrs[i] = &gsmBci[i];

        status = p.readInt32(&t);
        gsmBci[i].fromServiceId = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].toServiceId = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].fromCodeScheme = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].toCodeScheme = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].selected = (uint8_t) t;

        appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId =%d, \
              fromCodeScheme=%d, toCodeScheme=%d, selected =%d]", printBuf, i,
              gsmBci[i].fromServiceId, gsmBci[i].toServiceId,
              gsmBci[i].fromCodeScheme, gsmBci[i].toCodeScheme,
              gsmBci[i].selected);
    }
    closeRequest;

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber,
                          gsmBciPtrs,
                          num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *),
                          pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(gsmBci, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
    memset(gsmBciPtrs, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    {
    RIL_CDMA_BroadcastSmsConfigInfo cdmaBci[num];
    RIL_CDMA_BroadcastSmsConfigInfo *cdmaBciPtrs[num];

    startRequest;
    for (int i = 0 ; i < num ; i++ ) {
        cdmaBciPtrs[i] = &cdmaBci[i];

        status = p.readInt32(&t);
        cdmaBci[i].service_category = (int) t;

        status = p.readInt32(&t);
        cdmaBci[i].language = (int) t;

        status = p.readInt32(&t);
        cdmaBci[i].selected = (uint8_t) t;

        appendPrintBuf("%s [%d: service_category=%d, language =%d, \
              entries.bSelected =%d]", printBuf, i, cdmaBci[i].service_category,
              cdmaBci[i].language, cdmaBci[i].selected);
    }
    closeRequest;

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber,
                          cdmaBciPtrs,
                          num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *),
                          pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(cdmaBci, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
    memset(cdmaBciPtrs, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}
  /*
static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_WriteArgs rcsw;
    int32_t  t;
    uint32_t ut;
    uint8_t  uct;
    status_t status;
    int32_t  digitCount;

    memset(&rcsw, 0, sizeof(rcsw));

    status = p.readInt32(&t);
    rcsw.status = t;

    status = p.readInt32(&t);
    rcsw.message.uTeleserviceID = (int) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.bIsServicePresent = (uint8_t) uct;

    status = p.readInt32(&t);
    rcsw.message.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sAddress.number_of_digits = (uint8_t) uct;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_ADDRESS_MAX; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.odd = (uint8_t) uct;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.number_of_digits = (uint8_t) uct;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_SUBADDRESS_MAX; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sSubAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.uBearerDataLen = (int) t;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_BEARER_DATA_MAX; digitCount ++) {
        status = p.read(&uct, sizeof(uct));
        rcsw.message.aBearerData[digitCount] = (uint8_t) uct;
    }

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%sstatus=%d, message.uTeleserviceID=%d, message.bIsServicePresent=%d, \
            message.uServicecategory=%d, message.sAddress.digit_mode=%d, \
            message.sAddress.number_mode=%d, \
            message.sAddress.number_type=%d, ",
            printBuf, rcsw.status, rcsw.message.uTeleserviceID, rcsw.message.bIsServicePresent,
            rcsw.message.uServicecategory, rcsw.message.sAddress.digit_mode,
            rcsw.message.sAddress.number_mode,
            rcsw.message.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw),pRI);

#ifdef MEMSET_FREED
    memset(&rcsw, 0, sizeof(rcsw));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;

}
  */ //by ygong

static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI) {

    RIL_CDMA_SMS_WriteArgs rcsw;

    int32_t  t;

    uint32_t ut;

    uint8_t  uct;

    status_t status;

    int32_t  digitCount;

    int digitLimit;



    memset(&rcsw, 0, sizeof(rcsw));



    status = p.readInt32(&t);

    rcsw.status = t;



    status = p.readInt32(&t);

    rcsw.message.uTeleserviceID = (int) t;



    status = p.read(&uct,sizeof(uct));

    rcsw.message.bIsServicePresent = (uint8_t) uct;



    status = p.readInt32(&t);

    rcsw.message.uServicecategory = (int) t;



    status = p.readInt32(&t);

    rcsw.message.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;



    status = p.readInt32(&t);

    rcsw.message.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;



    status = p.readInt32(&t);

    rcsw.message.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;



    status = p.readInt32(&t);

    rcsw.message.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;



    status = p.read(&uct,sizeof(uct));

    rcsw.message.sAddress.number_of_digits = (uint8_t) uct;



    digitLimit= MIN((rcsw.message.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {

        status = p.read(&uct,sizeof(uct));

        rcsw.message.sAddress.digits[digitCount] = (uint8_t) uct;

    }



    status = p.readInt32(&t);

    rcsw.message.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;



    status = p.read(&uct,sizeof(uct));

    rcsw.message.sSubAddress.odd = (uint8_t) uct;



    status = p.read(&uct,sizeof(uct));

    rcsw.message.sSubAddress.number_of_digits = (uint8_t) uct;



    digitLimit= MIN((rcsw.message.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {

        status = p.read(&uct,sizeof(uct));

        rcsw.message.sSubAddress.digits[digitCount] = (uint8_t) uct;

    }



    status = p.readInt32(&t);

    rcsw.message.uBearerDataLen = (int) t;



    digitLimit= MIN((rcsw.message.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {

        status = p.read(&uct, sizeof(uct));

        rcsw.message.aBearerData[digitCount] = (uint8_t) uct;

    }



    if (status != NO_ERROR) {

        goto invalid;

    }



    startRequest;

    appendPrintBuf("%sstatus=%d, message.uTeleserviceID=%d, message.bIsServicePresent=%d, \
            message.uServicecategory=%d, message.sAddress.digit_mode=%d, \
            message.sAddress.number_mode=%d, \
            message.sAddress.number_type=%d, ",
            printBuf, rcsw.status, rcsw.message.uTeleserviceID, rcsw.message.bIsServicePresent,
            rcsw.message.uServicecategory, rcsw.message.sAddress.digit_mode,
            rcsw.message.sAddress.number_mode,
            rcsw.message.sAddress.number_type);
    closeRequest;



    printRequest(pRI->token, pRI->pCI->requestNumber);


    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw),pRI, pRI->socket_id);



#ifdef MEMSET_FREED

    memset(&rcsw, 0, sizeof(rcsw));

#endif



    return;



invalid:

    invalidCommandBlock(pRI);

    return;
}



// For backwards compatibility in RIL_REQUEST_SETUP_DATA_CALL.
// Version 4 of the RIL interface adds a new PDP type parameter to support
// IPv6 and dual-stack PDP contexts. When dealing with a previous version of
// RIL, remove the parameter from the request.
static void dispatchDataCall(Parcel& p, RequestInfo *pRI) {
    // In RIL v3, REQUEST_SETUP_DATA_CALL takes 6 parameters.
    const int numParamsRilV3 = 6;

    // The first bytes of the RIL parcel contain the request number and the
    // serial number - see processCommandBuffer(). Copy them over too.
    int pos = p.dataPosition();

    int numParams = p.readInt32();
    if (s_callbacks.version < 4 && numParams > numParamsRilV3) {
      Parcel p2;
      p2.appendFrom(&p, 0, pos);
      p2.writeInt32(numParamsRilV3);
      for(int i = 0; i < numParamsRilV3; i++) {
        p2.writeString16(p.readString16());
      }
      p2.setDataPosition(pos);
      dispatchStrings(p2, pRI);
    } else {
      p.setDataPosition(pos);
      dispatchStrings(p, pRI);
    }
}

static void dispatchWritePbToRuim(Parcel &p, RequestInfo *pRI) {
    char *write_pb_info[5];
    int32_t t;
    status_t status;

    // we only check status at the end
    status = p.readInt32(&t);
    write_pb_info[0] = (char *)t;
    write_pb_info[1] = strdupReadString(p);
    status |= p.readInt32(&t);
    write_pb_info[2] = (char *)t;
    write_pb_info[3] = strdupReadString(p);
    status |= p.readInt32(&t);
    write_pb_info[4] = (char *)t;

    startRequest;
    appendPrintBuf("%srecnum=%d,number=%s,type=%d,text=%s,coding=%d", printBuf,
        (int)write_pb_info[0], write_pb_info[1], (int)write_pb_info[2], write_pb_info[3], (int)write_pb_info[4]);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &write_pb_info, sizeof(write_pb_info),pRI, pRI->socket_id);

    free (write_pb_info[1]);
    free (write_pb_info[3]);
    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}
// For backwards compatibility with RILs that dont support RIL_REQUEST_VOICE_RADIO_TECH.
// When all RILs handle this request, this function can be removed and
// the request can be sent directly to the RIL using dispatchVoid.
static void dispatchVoiceRadioTech(Parcel& p, RequestInfo *pRI) {

    RIL_RadioState state = CALL_ONSTATEREQUEST((RIL_SOCKET_ID)pRI->socket_id);

    if ((RADIO_STATE_UNAVAILABLE == state) || (RADIO_STATE_OFF == state)) {
        RIL_onRequestComplete(pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
    }

    // RILs that support RADIO_STATE_ON should support this request.
    if (RADIO_STATE_ON == state) {
        dispatchVoid(p, pRI);
        return;
    }

    // For Older RILs, that do not support RADIO_STATE_ON, assume that they
    // will not support this new request either and decode Voice Radio Technology
    // from Radio State
    voiceRadioTech = decodeVoiceRadioTechnology(state);

    if (voiceRadioTech < 0)
        RIL_onRequestComplete(pRI, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(pRI, RIL_E_SUCCESS, &voiceRadioTech, sizeof(int));
}

// For backwards compatibility in RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:.
// When all RILs handle this request, this function can be removed and
// the request can be sent directly to the RIL using dispatchVoid.
static void dispatchCdmaSubscriptionSource(Parcel& p, RequestInfo *pRI) {

    RIL_RadioState state = CALL_ONSTATEREQUEST((RIL_SOCKET_ID)pRI->socket_id);

    if ((RADIO_STATE_UNAVAILABLE == state) || (RADIO_STATE_OFF == state)) {
        RIL_onRequestComplete(pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        ///M: It will invoke RIL_onRequestComplete twice in this situation, so add return.
        return;
    }

    // RILs that support RADIO_STATE_ON should support this request.
    if (RADIO_STATE_ON == state) {
        dispatchVoid(p, pRI);
        return;
    }

    // For Older RILs, that do not support RADIO_STATE_ON, assume that they
    // will not support this new request either and decode CDMA Subscription Source
    // from Radio State
    cdmaSubscriptionSource = decodeCdmaSubscriptionSource(state);

    if (cdmaSubscriptionSource < 0)
        RIL_onRequestComplete(pRI, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(pRI, RIL_E_SUCCESS, &cdmaSubscriptionSource, sizeof(int));
}

static void dispatchSetInitialAttachApn(Parcel &p, RequestInfo *pRI)
{
    RIL_InitialAttachApn pf;
    int32_t  t;
    status_t status;

    memset(&pf, 0, sizeof(pf));

    pf.apn = strdupReadString(p);
    pf.protocol = strdupReadString(p);

    status = p.readInt32(&t);
    pf.authtype = (int) t;

    pf.username = strdupReadString(p);
    pf.password = strdupReadString(p);

    startRequest;
    appendPrintBuf("%sapn=%s, protocol=%s, authype=%d, username=%s, password=%s",
            printBuf, pf.apn, pf.protocol, pf.authtype, pf.username, pf.password);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }
    CALL_ONREQUEST(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(pf.apn);
    memsetString(pf.protocol);
    memsetString(pf.username);
    memsetString(pf.password);
#endif

    free(pf.apn);
    free(pf.protocol);
    free(pf.username);
    free(pf.password);

#ifdef MEMSET_FREED
    memset(&pf, 0, sizeof(pf));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchNVReadItem(Parcel &p, RequestInfo *pRI) {
    RIL_NV_ReadItem nvri;
    int32_t  t;
    status_t status;

    memset(&nvri, 0, sizeof(nvri));

    status = p.readInt32(&t);
    nvri.itemID = (RIL_NV_Item) t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%snvri.itemID=%d, ", printBuf, nvri.itemID);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvri, sizeof(nvri), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&nvri, 0, sizeof(nvri));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchNVWriteItem(Parcel &p, RequestInfo *pRI) {
    RIL_NV_WriteItem nvwi;
    int32_t  t;
    status_t status;

    memset(&nvwi, 0, sizeof(nvwi));

    status = p.readInt32(&t);
    nvwi.itemID = (RIL_NV_Item) t;

    nvwi.value = strdupReadString(p);

    if (status != NO_ERROR || nvwi.value == NULL) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%snvwi.itemID=%d, value=%s, ", printBuf, nvwi.itemID,
            nvwi.value);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvwi, sizeof(nvwi), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(nvwi.value);
#endif

    free(nvwi.value);

#ifdef MEMSET_FREED
    memset(&nvwi, 0, sizeof(nvwi));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}


static void dispatchUiccSubscripton(Parcel &p, RequestInfo *pRI) {
    RIL_SelectUiccSub uicc_sub;
    status_t status;
    int32_t  t;
    memset(&uicc_sub, 0, sizeof(uicc_sub));

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.slot = (int) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.app_index = (int) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.sub_type = (RIL_SubscriptionType) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.act_status = (RIL_UiccSubActStatus) t;

    startRequest;
    appendPrintBuf("slot=%d, app_index=%d, act_status = %d", uicc_sub.slot, uicc_sub.app_index,
            uicc_sub.act_status);
    RLOGD("dispatchUiccSubscription, slot=%d, app_index=%d, act_status = %d", uicc_sub.slot,
            uicc_sub.app_index, uicc_sub.act_status);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &uicc_sub, sizeof(uicc_sub), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&uicc_sub, 0, sizeof(uicc_sub));
#endif
    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchSimAuthentication(Parcel &p, RequestInfo *pRI)
{
    RIL_SimAuthentication pf;
    int32_t  t;
    status_t status;

    memset(&pf, 0, sizeof(pf));

    status = p.readInt32(&t);
    pf.authContext = (int) t;
    pf.authData = strdupReadString(p);
    pf.aid = strdupReadString(p);

    startRequest;
    appendPrintBuf("authContext=%s, authData=%s, aid=%s", pf.authContext, pf.authData, pf.aid);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(pf.authData);
    memsetString(pf.aid);
#endif

    free(pf.authData);
    free(pf.aid);

#ifdef MEMSET_FREED
    memset(&pf, 0, sizeof(pf));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchDataProfile(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    {
        RIL_DataProfileInfo dataProfiles[num];
        RIL_DataProfileInfo *dataProfilePtrs[num];

        startRequest;
        for (int i = 0 ; i < num ; i++ ) {
            dataProfilePtrs[i] = &dataProfiles[i];

            status = p.readInt32(&t);
            dataProfiles[i].profileId = (int) t;

            dataProfiles[i].apn = strdupReadString(p);
            dataProfiles[i].protocol = strdupReadString(p);
            status = p.readInt32(&t);
            dataProfiles[i].authType = (int) t;

            dataProfiles[i].user = strdupReadString(p);
            dataProfiles[i].password = strdupReadString(p);

            status = p.readInt32(&t);
            dataProfiles[i].type = (int) t;

            status = p.readInt32(&t);
            dataProfiles[i].maxConnsTime = (int) t;
            status = p.readInt32(&t);
            dataProfiles[i].maxConns = (int) t;
            status = p.readInt32(&t);
            dataProfiles[i].waitTime = (int) t;

            status = p.readInt32(&t);
            dataProfiles[i].enabled = (int) t;

            appendPrintBuf("%s [%d: profileId=%d, apn =%s, protocol =%s, authType =%d, \
                  user =%s, password =%s, type =%d, maxConnsTime =%d, maxConns =%d, \
                  waitTime =%d, enabled =%d]", printBuf, i, dataProfiles[i].profileId,
                  dataProfiles[i].apn, dataProfiles[i].protocol, dataProfiles[i].authType,
                  dataProfiles[i].user, dataProfiles[i].password, dataProfiles[i].type,
                  dataProfiles[i].maxConnsTime, dataProfiles[i].maxConns,
                  dataProfiles[i].waitTime, dataProfiles[i].enabled);
        }
        closeRequest;
        printRequest(pRI->token, pRI->pCI->requestNumber);

        if (status != NO_ERROR) {
            goto invalid;
        }

        CALL_ONREQUEST(pRI->pCI->requestNumber, dataProfilePtrs,
                num * sizeof(RIL_DataProfileInfo *), pRI, pRI->socket_id);
#ifdef MEMSET_FREED
        memset(dataProfiles, 0, num * sizeof(RIL_DataProfileInfo));
        memset(dataProfilePtrs, 0, num * sizeof(RIL_DataProfileInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchRadioCapability(Parcel &p, RequestInfo *pRI){
    RIL_RadioCapability rc;
    int32_t t;
    status_t status;

    memset (&rc, 0, sizeof(RIL_RadioCapability));

    status = p.readInt32(&t);
    rc.version = (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.session= (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.phase= (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.rat = (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = readStringFromParcelInplace(p, rc.logicalModemUuid, sizeof(rc.logicalModemUuid));
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.status = (int)t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s [version:%d, session:%d, phase:%d, rat:%d, \
            logicalModemUuid:%s, status:%d", printBuf, rc.version, rc.session
            rc.phase, rc.rat, rc.logicalModemUuid, rc.session);

    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rc, sizeof(RIL_RadioCapability), pRI, pRI->socket_id);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static int
blockingWrite(int fd, const void *buffer, size_t len) {
    size_t writeOffset = 0;
    const uint8_t *toWrite;

    toWrite = (const uint8_t *)buffer;

    while (writeOffset < len) {
        ssize_t written;
        do {
            written = write (fd, toWrite + writeOffset,
                                len - writeOffset);
        } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));
        LOGD("writeOffset = %d written = %d", (int)writeOffset, (int)written);
        if (written >= 0) {
            writeOffset += written;
        } else {   // written < 0
            LOGE ("RIL Response: unexpected error on write errno:%d", errno);
            close(fd);
            #if 0
            if(errno == EBADF)
            {
                LOGD("BAD FILENUMBER, kill rild to reset! ");
                killRild();
            }
            #endif
            return -1;
        }
    }

    return 0;
}

static int
sendResponseRaw (const void *data, size_t dataSize, RIL_SOCKET_ID socket_id) {

    int fd = s_ril_param_socket.fdCommand;
    struct ril_event* commands_event = s_ril_param_socket.commands_event;
    struct ril_event* listen_event = s_ril_param_socket.listen_event;
    int ret;
    uint32_t header;

    pthread_mutex_t * writeMutexHook = &s_writeMutex;

    RLOGE("Send Response to %s", rilSocketIdToString(socket_id));

#if (SIM_COUNT >= 2)
    if (socket_id == RIL_SOCKET_2) {
        fd = s_ril_param_socket2.fdCommand;
        commands_event = s_ril_param_socket2.commands_event;
        listen_event = s_ril_param_socket2.listen_event;
        writeMutexHook = &s_writeMutex_socket2;
    }
#if (SIM_COUNT >= 3)
    else if (socket_id == RIL_SOCKET_3) {
        fd = s_ril_param_socket3.fdCommand;
        commands_event = s_ril_param_socket3.commands_event;
        listen_event = s_ril_param_socket3.listen_event;
        writeMutexHook = &s_writeMutex_socket3;
    }
#endif
#if (SIM_COUNT >= 4)
    else if (socket_id == RIL_SOCKET_4) {
        fd = s_ril_param_socket4.fdCommand;
        commands_event = s_ril_param_socket4.commands_event;
        listen_event = s_ril_param_socket4.listen_event;
        writeMutexHook = &s_writeMutex_socket4;
    }
#endif
#endif

    if (fd < 0) {
    	///mtk81234 end
        return -1;
    }

    if (dataSize > MAX_COMMAND_BYTES) {
        LOGE("RIL: packet larger than %u (%u)",
                MAX_COMMAND_BYTES, (unsigned int )dataSize);

        return -1;
    }
    pthread_mutex_lock(writeMutexHook);
    header = htonl(dataSize);

    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);

        fd = -1;
        ril_event_del(commands_event);

        /* start listening for new connections again */
        rilEventAddWakeup(listen_event);
        return ret;
    }

    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);

        fd = -1;
        ril_event_del(commands_event);

        /* start listening for new connections again */
        rilEventAddWakeup(listen_event);
        return ret;
    }

    pthread_mutex_unlock(writeMutexHook);

    return 0;
}

static int
sendResponse (Parcel &p, RIL_SOCKET_ID socket_id) {
    printResponse;
    return sendResponseRaw(p.data(), p.dataSize(), socket_id);
}

/** response is an int* pointing to an array of ints*/

static int
responseInts(Parcel &p, void *response, size_t responselen) {
    int numInts;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;

    numInts = responselen / sizeof(int *);
    p.writeInt32 (numInts);

    /* each int*/
    startResponse;
    for (int i = 0 ; i < numInts ; i++) {
        appendPrintBuf("%s%d,", printBuf, p_int[i]);
        p.writeInt32(p_int[i]);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

// Response is an int or RIL_LastCallFailCauseInfo.
// Currently, only Shamu plans to use RIL_LastCallFailCauseInfo.
// TODO(yjl): Let all implementations use RIL_LastCallFailCauseInfo.
static int responseFailCause(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen == sizeof(int)) {
      startResponse;
      int *p_int = (int *) response;
      appendPrintBuf("%s%d,", printBuf, p_int[0]);
      p.writeInt32(p_int[0]);
      removeLastChar;
      closeResponse;
    } else if (responselen == sizeof(RIL_LastCallFailCauseInfo)) {
      startResponse;
      RIL_LastCallFailCauseInfo *p_fail_cause_info = (RIL_LastCallFailCauseInfo *) response;
      appendPrintBuf("%s[cause_code=%d,vendor_cause=%s]", printBuf, p_fail_cause_info->cause_code,
                     p_fail_cause_info->vendor_cause);
      p.writeInt32(p_fail_cause_info->cause_code);
      writeStringToParcel(p, p_fail_cause_info->vendor_cause);
      removeLastChar;
      closeResponse;
    } else {
      RLOGE("responseFailCause: invalid response length %d expected an int or "
            "RIL_LastCallFailCauseInfo", (int)responselen);
      return RIL_ERRNO_INVALID_RESPONSE;
    }

    return 0;
}

/** response is a char **, pointing to an array of char *'s
    The parcel will begin with the version */
static int responseStringsWithVersion(int version, Parcel &p, void *response, size_t responselen) {
    p.writeInt32(version);
    return responseStrings(p, response, responselen);
}

/** response is a char **, pointing to an array of char *'s */
static int responseStrings(Parcel &p, void *response, size_t responselen) {
    int numStrings;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(char *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (response == NULL) {
        p.writeInt32 (0);
    } else {
        char **p_cur = (char **) response;

        numStrings = responselen / sizeof(char *);
        p.writeInt32 (numStrings);

        /* each string*/
        startResponse;
        for (int i = 0 ; i < numStrings ; i++) {
            appendPrintBuf("%s%s,", printBuf, (char*)p_cur[i]);
            writeStringToParcel (p, p_cur[i]);
        }
        removeLastChar;
        closeResponse;
    }
    return 0;
}


/**
 * NULL strings are accepted
 * FIXME currently ignores responselen
 */
static int responseString(Parcel &p, void *response, size_t responselen) {
    /* one string only */
    startResponse;
    appendPrintBuf("%s%s", printBuf, (char*)response);
    closeResponse;

    writeStringToParcel(p, (const char *)response);

    return 0;
}

static int responseVoid(Parcel &p, void *response, size_t responselen) {
    startResponse;
    removeLastChar;
    return 0;
}

static int responseGetNitzTime(Parcel &p, void *response, size_t responselen){
    int64_t timeReceived;
    startResponse;
    appendPrintBuf("%s%s", printBuf, (char*)response);
    closeResponse;

    writeStringToParcel(p, (const char *)response);
    timeReceived = elapsedRealtime();
    p.writeInt64(timeReceived);
    return 0;
}

static int responseCallList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_Call *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_Call *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of call info's */
    num = responselen / sizeof(RIL_Call *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_Call *p_cur = ((RIL_Call **) response)[i];
        /* each call info */
        p.writeInt32(p_cur->state);
        p.writeInt32(p_cur->index);
        p.writeInt32(p_cur->toa);
        p.writeInt32(p_cur->isMpty);
        p.writeInt32(p_cur->isMT);
        p.writeInt32(p_cur->als);
        p.writeInt32(p_cur->isVoice);
        p.writeInt32(p_cur->isVoicePrivacy);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->numberPresentation);
        writeStringToParcel(p, p_cur->name);
        p.writeInt32(p_cur->namePresentation);
        // Remove when partners upgrade to version 3
        if ((s_callbacks.version < 3) || (p_cur->uusInfo == NULL || p_cur->uusInfo->uusData == NULL)) {
            p.writeInt32(0); /* UUS Information is absent */
        } else {
            RIL_UUS_Info *uusInfo = p_cur->uusInfo;
            p.writeInt32(1); /* UUS Information is present */
            p.writeInt32(uusInfo->uusType);
            p.writeInt32(uusInfo->uusDcs);
            p.writeInt32(uusInfo->uusLength);
            p.write(uusInfo->uusData, uusInfo->uusLength);
        }
        appendPrintBuf("%s[id=%d,%s,toa=%d,",
            printBuf,
            p_cur->index,
            callStateToString(p_cur->state),
            p_cur->toa);
        appendPrintBuf("%s%s,%s,als=%d,%s,%s,",
            printBuf,
            (p_cur->isMpty)?"conf":"norm",
            (p_cur->isMT)?"mt":"mo",
            p_cur->als,
            (p_cur->isVoice)?"voc":"nonvoc",
            (p_cur->isVoicePrivacy)?"evp":"noevp");
        appendPrintBuf("%s%s,cli=%d,name='%s',%d]",
            printBuf,
            p_cur->number,
            p_cur->numberPresentation,
            p_cur->name,
            p_cur->namePresentation);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseSMS(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Response) ) {
        LOGE("invalid response length %d expected %d",
                (int)responselen, (int)sizeof (RIL_SMS_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SMS_Response *p_cur = (RIL_SMS_Response *) response;

    p.writeInt32(p_cur->messageRef);
    writeStringToParcel(p, p_cur->ackPDU);
    p.writeInt32(p_cur->errorCode);

    startResponse;
    appendPrintBuf("%s%d,%s,%d", printBuf, p_cur->messageRef,
        (char*)p_cur->ackPDU, p_cur->errorCode);
    closeResponse;

    return 0;
}

static int responseDataCallListV4(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v4) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v4));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_Data_Call_Response_v4);
    p.writeInt32(num);

    RIL_Data_Call_Response_v4 *p_cur = (RIL_Data_Call_Response_v4 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        // apn is not used, so don't send.
        writeStringToParcel(p, p_cur[i].address);
        appendPrintBuf("%s[cid=%d,%s,%s,%s],", printBuf,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].address);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseDataCallList(Parcel &p, void *response, size_t responselen)
{
    // Write version
    p.writeInt32(s_callbacks.version);

    if (s_callbacks.version < 5) {
        return responseDataCallListV4(p, response, responselen);
    } else {
        if (response == NULL && responselen != 0) {
            LOGE("invalid response: NULL");
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        if (responselen % sizeof(RIL_Data_Call_Response_v11) != 0) {
            LOGE("invalid response length %d expected multiple of %d",
                    (int)responselen, (int)sizeof(RIL_Data_Call_Response_v11));
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        int num = responselen / sizeof(RIL_Data_Call_Response_v11);
        p.writeInt32(num);

        RIL_Data_Call_Response_v11 *p_cur = (RIL_Data_Call_Response_v11 *) response;
        startResponse;
        int i;
        for (i = 0; i < num; i++) {
            p.writeInt32((int)p_cur[i].status);
            p.writeInt32(p_cur[i].suggestedRetryTime);
            p.writeInt32(p_cur[i].cid);
            p.writeInt32(p_cur[i].active);
            writeStringToParcel(p, p_cur[i].type);
            writeStringToParcel(p, p_cur[i].ifname);
            writeStringToParcel(p, p_cur[i].addresses);
            writeStringToParcel(p, p_cur[i].dnses);
            writeStringToParcel(p, p_cur[i].gateways);
            writeStringToParcel(p, "");
            p.writeInt32(p_cur[i].mtu);
            appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s,%d],", printBuf,
                p_cur[i].status,
                p_cur[i].suggestedRetryTime,
                p_cur[i].cid,
                (p_cur[i].active==0)?"down":"up",
                (char*)p_cur[i].type,
                (char*)p_cur[i].ifname,
                (char*)p_cur[i].addresses,
                (char*)p_cur[i].dnses,
                (char*)p_cur[i].gateways,
                (char*)p_cur[i].mtu);
        }
        removeLastChar;
        closeResponse;
    }

    return 0;
}

static int responseSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    if (s_callbacks.version < 5) {
        return responseStringsWithVersion(s_callbacks.version, p, response, responselen);
    } else {
        return responseDataCallList(p, response, responselen);
    }
}

static int responseRaw(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL with responselen != 0");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // The java code reads -1 size as null byte array
    if (response == NULL) {
        p.writeInt32(-1);
    } else {
        p.writeInt32(responselen);
        p.write(response, responselen);
    }

    return 0;
}


static int responseSIM_IO(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SIM_IO_Response) ) {
        LOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SIM_IO_Response *p_cur = (RIL_SIM_IO_Response *) response;
    p.writeInt32(p_cur->sw1);
    p.writeInt32(p_cur->sw2);
    writeStringToParcel(p, p_cur->simResponse);

    startResponse;
    appendPrintBuf("%ssw1=0x%X,sw2=0x%X,%s", printBuf, p_cur->sw1, p_cur->sw2,
        (char*)p_cur->simResponse);
    closeResponse;


    return 0;
}

static int responseCallForwards(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CallForwardInfo *) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CallForwardInfo *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    /* number of call info's */
    num = responselen / sizeof(RIL_CallForwardInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++) {
        RIL_CallForwardInfo *p_cur = ((RIL_CallForwardInfo **) response)[i];

        p.writeInt32(p_cur->status);
        p.writeInt32(p_cur->reason);
        p.writeInt32(p_cur->serviceClass);
        p.writeInt32(p_cur->toa);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->timeSeconds);
        appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d],", printBuf,
            (p_cur->status==1)?"enable":"disable",
            p_cur->reason, p_cur->serviceClass, p_cur->toa,
            (char*)p_cur->number,
            p_cur->timeSeconds);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseSsn(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_SuppSvcNotification)) {
        LOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SuppSvcNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SuppSvcNotification *p_cur = (RIL_SuppSvcNotification *) response;
    p.writeInt32(p_cur->notificationType);
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->index);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);

    startResponse;
    appendPrintBuf("%s%s,code=%d,id=%d,type=%d,%s", printBuf,
        (p_cur->notificationType==0)?"mo":"mt",
         p_cur->code, p_cur->index, p_cur->type,
        (char*)p_cur->number);
    closeResponse;

    return 0;
}

static int responseCellList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_NeighboringCell *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_NeighboringCell *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of records */
    num = responselen / sizeof(RIL_NeighboringCell *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_NeighboringCell *p_cur = ((RIL_NeighboringCell **) response)[i];

        p.writeInt32(p_cur->rssi);
        writeStringToParcel (p, p_cur->cid);

        appendPrintBuf("%s[cid=%s,rssi=%d],", printBuf,
            p_cur->cid, p_cur->rssi);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/**
 * Marshall the signalInfoRecord into the parcel if it exists.
 */
static void marshallSignalInfoRecord(Parcel &p,
            RIL_CDMA_SignalInfoRecord &p_signalInfoRecord) {
    p.writeInt32(p_signalInfoRecord.isPresent);
    p.writeInt32(p_signalInfoRecord.signalType);
    p.writeInt32(p_signalInfoRecord.alertPitch);
    p.writeInt32(p_signalInfoRecord.signal);
}

static int responseCdmaInformationRecords(Parcel &p,
            void *response, size_t responselen) {
    int num;
    char* string8 = NULL;
    int buffer_lenght;
    RIL_CDMA_InformationRecord *infoRec;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_InformationRecords)) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_CDMA_InformationRecords *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_InformationRecords *p_cur =
                             (RIL_CDMA_InformationRecords *) response;
    num = MIN(p_cur->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);

    startResponse;
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        infoRec = &p_cur->infoRec[i];
        p.writeInt32(infoRec->name);
        switch (infoRec->name) {
            case RIL_CDMA_DISPLAY_INFO_REC:
            case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC:
                if (infoRec->rec.display.alpha_len >
                                         CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                    LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.display.alpha_len,
                         CDMA_ALPHA_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) malloc((infoRec->rec.display.alpha_len + 1)
                                                             * sizeof(char) );
                for (int i = 0 ; i < infoRec->rec.display.alpha_len ; i++) {
                    string8[i] = infoRec->rec.display.alpha_buf[i];
                }
                string8[(int)infoRec->rec.display.alpha_len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                break;
            case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
            case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
            case RIL_CDMA_CONNECTED_NUMBER_INFO_REC:
                if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                    LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.number.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) malloc((infoRec->rec.number.len + 1)
                                                             * sizeof(char) );
                for (int i = 0 ; i < infoRec->rec.number.len; i++) {
                    string8[i] = infoRec->rec.number.buf[i];
                }
                string8[(int)infoRec->rec.number.len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                p.writeInt32(infoRec->rec.number.number_type);
                p.writeInt32(infoRec->rec.number.number_plan);
                p.writeInt32(infoRec->rec.number.pi);
                p.writeInt32(infoRec->rec.number.si);
                break;
            case RIL_CDMA_SIGNAL_INFO_REC:
                p.writeInt32(infoRec->rec.signal.isPresent);
                p.writeInt32(infoRec->rec.signal.signalType);
                p.writeInt32(infoRec->rec.signal.alertPitch);
                p.writeInt32(infoRec->rec.signal.signal);

                appendPrintBuf("%sisPresent=%X, signalType=%X, \
                                alertPitch=%X, signal=%X, ",
                   printBuf, (int)infoRec->rec.signal.isPresent,
                   (int)infoRec->rec.signal.signalType,
                   (int)infoRec->rec.signal.alertPitch,
                   (int)infoRec->rec.signal.signal);
                removeLastChar;
                break;
            case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC:
                if (infoRec->rec.redir.redirectingNumber.len >
                                              CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                    LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.redir.redirectingNumber.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) malloc((infoRec->rec.redir.redirectingNumber
                                          .len + 1) * sizeof(char) );
                for (int i = 0;
                         i < infoRec->rec.redir.redirectingNumber.len;
                         i++) {
                    string8[i] = infoRec->rec.redir.redirectingNumber.buf[i];
                }
                string8[(int)infoRec->rec.redir.redirectingNumber.len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                p.writeInt32(infoRec->rec.redir.redirectingNumber.number_type);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.number_plan);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.pi);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.si);
                p.writeInt32(infoRec->rec.redir.redirectingReason);
                break;
            case RIL_CDMA_LINE_CONTROL_INFO_REC:
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPolarityIncluded);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlToggle);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlReverse);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPowerDenial);

                appendPrintBuf("%slineCtrlPolarityIncluded=%d, \
                                lineCtrlToggle=%d, lineCtrlReverse=%d, \
                                lineCtrlPowerDenial=%d, ", printBuf,
                       (int)infoRec->rec.lineCtrl.lineCtrlPolarityIncluded,
                       (int)infoRec->rec.lineCtrl.lineCtrlToggle,
                       (int)infoRec->rec.lineCtrl.lineCtrlReverse,
                       (int)infoRec->rec.lineCtrl.lineCtrlPowerDenial);
                removeLastChar;
                break;
            case RIL_CDMA_T53_CLIR_INFO_REC:
                p.writeInt32((int)(infoRec->rec.clir.cause));

                appendPrintBuf("%scause%d", printBuf, infoRec->rec.clir.cause);
                removeLastChar;
                break;
            case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC:
                p.writeInt32(infoRec->rec.audioCtrl.upLink);
                p.writeInt32(infoRec->rec.audioCtrl.downLink);

                appendPrintBuf("%supLink=%d, downLink=%d, ", printBuf,
                        infoRec->rec.audioCtrl.upLink,
                        infoRec->rec.audioCtrl.downLink);
                removeLastChar;
                break;
            case RIL_CDMA_T53_RELEASE_INFO_REC:
                // TODO(Moto): See David Krause, he has the answer:)
                LOGE("RIL_CDMA_T53_RELEASE_INFO_REC: return INVALID_RESPONSE");
                return RIL_ERRNO_INVALID_RESPONSE;
            default:
                LOGE("Incorrect name value");
                return RIL_ERRNO_INVALID_RESPONSE;
        }
    }
    closeResponse;

    return 0;
}

static int responseRilSignalStrength(Parcel &p,
                    void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen >= sizeof (RIL_SignalStrength_v5)) {
        RIL_SignalStrength_v6 *p_cur = ((RIL_SignalStrength_v6 *) response);

        p.writeInt32(p_cur->GW_SignalStrength.signalStrength);
        p.writeInt32(p_cur->GW_SignalStrength.bitErrorRate);
        p.writeInt32(p_cur->CDMA_SignalStrength.dbm);
        p.writeInt32(p_cur->CDMA_SignalStrength.ecio);
        p.writeInt32(p_cur->EVDO_SignalStrength.dbm);
        p.writeInt32(p_cur->EVDO_SignalStrength.ecio);
        p.writeInt32(p_cur->EVDO_SignalStrength.signalNoiseRatio);
        if (responselen >= sizeof (RIL_SignalStrength_v6)) {
            /*
             * Fixup LTE for backwards compatibility
             */
            if (s_callbacks.version <= 6) {
                // signalStrength: -1 -> 99
                if (p_cur->LTE_SignalStrength.signalStrength == -1) {
                    p_cur->LTE_SignalStrength.signalStrength = 99;
                }
                // rsrp: -1 -> INT_MAX all other negative value to positive.
                // So remap here
                if (p_cur->LTE_SignalStrength.rsrp == -1) {
                    p_cur->LTE_SignalStrength.rsrp = INT_MAX;
                } else if (p_cur->LTE_SignalStrength.rsrp < -1) {
                    p_cur->LTE_SignalStrength.rsrp = -p_cur->LTE_SignalStrength.rsrp;
                }
                // rsrq: -1 -> INT_MAX
                if (p_cur->LTE_SignalStrength.rsrq == -1) {
                    p_cur->LTE_SignalStrength.rsrq = INT_MAX;
                }
                // Not remapping rssnr is already using INT_MAX

                // cqi: -1 -> INT_MAX
                if (p_cur->LTE_SignalStrength.cqi == -1) {
                    p_cur->LTE_SignalStrength.cqi = INT_MAX;
                }
            }
            p.writeInt32(p_cur->LTE_SignalStrength.signalStrength);
            p.writeInt32(p_cur->LTE_SignalStrength.rsrp);
            p.writeInt32(p_cur->LTE_SignalStrength.rsrq);
            p.writeInt32(p_cur->LTE_SignalStrength.rssnr);
            p.writeInt32(p_cur->LTE_SignalStrength.cqi);
        } else {
            p.writeInt32(99);
            p.writeInt32(INT_MAX);
            p.writeInt32(INT_MAX);
            p.writeInt32(INT_MAX);
            p.writeInt32(INT_MAX);
        }

        startResponse;
        appendPrintBuf("%s[signalStrength=%d,bitErrorRate=%d,\
                CDMA_SS.dbm=%d,CDMA_SSecio=%d,\
                EVDO_SS.dbm=%d,EVDO_SS.ecio=%d,\
                EVDO_SS.signalNoiseRatio=%d,\
                LTE_SS.signalStrength=%d,LTE_SS.rsrp=%d,LTE_SS.rsrq=%d,\
                LTE_SS.rssnr=%d,LTE_SS.cqi=%d]",
                printBuf,
                p_cur->GW_SignalStrength.signalStrength,
                p_cur->GW_SignalStrength.bitErrorRate,
                p_cur->CDMA_SignalStrength.dbm,
                p_cur->CDMA_SignalStrength.ecio,
                p_cur->EVDO_SignalStrength.dbm,
                p_cur->EVDO_SignalStrength.ecio,
                p_cur->EVDO_SignalStrength.signalNoiseRatio,
                p_cur->LTE_SignalStrength.signalStrength,
                p_cur->LTE_SignalStrength.rsrp,
                p_cur->LTE_SignalStrength.rsrq,
                p_cur->LTE_SignalStrength.rssnr,
                p_cur->LTE_SignalStrength.cqi);
        closeResponse;

    } else {
        LOGE("invalid response length");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    return 0;
}

static int responseCallRing(Parcel &p, void *response, size_t responselen) {
    if ((response == NULL) || (responselen == 0)) {
        return responseVoid(p, response, responselen);
    } else {
        return responseCdmaSignalInfoRecord(p, response, responselen);
    }
}

static int responseCdmaSignalInfoRecord(Parcel &p, void *response, size_t responselen) {
    if (response == NULL || responselen == 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_SignalInfoRecord)) {
        LOGE("invalid response length %d expected sizeof (RIL_CDMA_SignalInfoRecord) of %d\n",
            (int)responselen, (int)sizeof (RIL_CDMA_SignalInfoRecord));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_CDMA_SignalInfoRecord *p_cur = ((RIL_CDMA_SignalInfoRecord *) response);
    marshallSignalInfoRecord(p, *p_cur);

    appendPrintBuf("%s[isPresent=%d,signalType=%d,alertPitch=%d\
              signal=%d]",
              printBuf,
              p_cur->isPresent,
              p_cur->signalType,
              p_cur->alertPitch,
              p_cur->signal);

    closeResponse;
    return 0;
}

static int responseCdmaCallWaiting(Parcel &p, void *response,
            size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen < sizeof(RIL_CDMA_CallWaiting_v6)) {
        LOGW("Upgrade to ril version %d\n", RIL_VERSION);
    }

    RIL_CDMA_CallWaiting_v6 *p_cur = ((RIL_CDMA_CallWaiting_v6 *) response);

    writeStringToParcel(p, p_cur->number);
    p.writeInt32(p_cur->numberPresentation);
    writeStringToParcel(p, p_cur->name);
    marshallSignalInfoRecord(p, p_cur->signalInfoRecord);

    if (responselen >= sizeof(RIL_CDMA_CallWaiting_v6)) {
        p.writeInt32(p_cur->number_type);
        p.writeInt32(p_cur->number_plan);
    } else {
        p.writeInt32(0);
        p.writeInt32(0);
    }

    startResponse;
    appendPrintBuf("%snumber=%s,numberPresentation=%d, name=%s,\
            signalInfoRecord[isPresent=%d,signalType=%d,alertPitch=%d\
            signal=%d,number_type=%d,number_plan=%d]",
            printBuf,
            p_cur->number,
            p_cur->numberPresentation,
            p_cur->name,
            p_cur->signalInfoRecord.isPresent,
            p_cur->signalInfoRecord.signalType,
            p_cur->signalInfoRecord.alertPitch,
            p_cur->signalInfoRecord.signal,
            p_cur->number_type,
            p_cur->number_plan);
    closeResponse;

    return 0;
}

static int responseSimRefresh(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("responseSimRefresh: invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    if (s_callbacks.version == 7) {
        RIL_SimRefreshResponse_v7 *p_cur = ((RIL_SimRefreshResponse_v7 *) response);
        p.writeInt32(p_cur->result);
        p.writeInt32(p_cur->ef_id);
        writeStringToParcel(p, p_cur->aid);

        appendPrintBuf("%sresult=%d, ef_id=%d, aid=%s",
                printBuf,
                p_cur->result,
                p_cur->ef_id,
                p_cur->aid);
    } else {
        int *p_cur = ((int *) response);
        p.writeInt32(p_cur[0]);
        p.writeInt32(p_cur[1]);
        writeStringToParcel(p, NULL);

        appendPrintBuf("%sresult=%d, ef_id=%d",
                printBuf,
                p_cur[0],
                p_cur[1]);
    }
    closeResponse;

    return 0;
}

static int responseCellInfoList(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CellInfo) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CellInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_CellInfo);
    p.writeInt32(num);

    RIL_CellInfo *p_cur = (RIL_CellInfo *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        appendPrintBuf("%s[%d: type=%d,registered=%d,timeStampType=%d,timeStamp=%lld", printBuf, i,
            p_cur->cellInfoType, p_cur->registered, p_cur->timeStampType, p_cur->timeStamp);
        p.writeInt32((int)p_cur->cellInfoType);
        p.writeInt32(p_cur->registered);
        p.writeInt32(p_cur->timeStampType);
        p.writeInt64(p_cur->timeStamp);
        switch(p_cur->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                appendPrintBuf("%s GSM id: mcc=%d,mnc=%d,lac=%d,cid=%d,", printBuf,
                    p_cur->CellInfo.gsm.cellIdentityGsm.mcc,
                    p_cur->CellInfo.gsm.cellIdentityGsm.mnc,
                    p_cur->CellInfo.gsm.cellIdentityGsm.lac,
                    p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                appendPrintBuf("%s gsmSS: ss=%d,ber=%d],", printBuf,
                    p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength,
                    p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);

                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mcc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mnc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.lac);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                appendPrintBuf("%s WCDMA id: mcc=%d,mnc=%d,lac=%d,cid=%d,psc=%d,", printBuf,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.lac,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.cid,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                appendPrintBuf("%s wcdmaSS: ss=%d,ber=%d],", printBuf,
                    p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength,
                    p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);

                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.lac);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.cid);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_CDMA: {
                appendPrintBuf("%s CDMA id: nId=%d,sId=%d,bsId=%d,long=%d,lat=%d", printBuf,
                    p_cur->CellInfo.cdma.cellIdentityCdma.networkId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.systemId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.basestationId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.longitude,
                    p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.networkId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.systemId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.basestationId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.longitude);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                appendPrintBuf("%s cdmaSS: dbm=%d ecio=%d evdoSS: dbm=%d,ecio=%d,snr=%d", printBuf,
                    p_cur->CellInfo.cdma.signalStrengthCdma.dbm,
                    p_cur->CellInfo.cdma.signalStrengthCdma.ecio,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.dbm,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.ecio,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);

                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                break;
            }
            case RIL_CELL_INFO_TYPE_LTE: {
                appendPrintBuf("%s LTE id: mcc=%d,mnc=%d,ci=%d,pci=%d,tac=%d", printBuf,
                    p_cur->CellInfo.lte.cellIdentityLte.mcc,
                    p_cur->CellInfo.lte.cellIdentityLte.mnc,
                    p_cur->CellInfo.lte.cellIdentityLte.ci,
                    p_cur->CellInfo.lte.cellIdentityLte.pci,
                    p_cur->CellInfo.lte.cellIdentityLte.tac);

                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mcc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mnc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.ci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.pci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.tac);

                appendPrintBuf("%s lteSS: ss=%d,rsrp=%d,rsrq=%d,rssnr=%d,cqi=%d,ta=%d", printBuf,
                    p_cur->CellInfo.lte.signalStrengthLte.signalStrength,
                    p_cur->CellInfo.lte.signalStrengthLte.rsrp,
                    p_cur->CellInfo.lte.signalStrengthLte.rsrq,
                    p_cur->CellInfo.lte.signalStrengthLte.rssnr,
                    p_cur->CellInfo.lte.signalStrengthLte.cqi,
                    p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.signalStrength);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrp);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrq);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rssnr);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.cqi);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                break;
            }
        }
        p_cur += 1;
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseHardwareConfig(Parcel &p, void *response, size_t responselen)
{
   if (response == NULL && responselen != 0) {
       RLOGE("invalid response: NULL");
       return RIL_ERRNO_INVALID_RESPONSE;
   }

   if (responselen % sizeof(RIL_HardwareConfig) != 0) {
       RLOGE("responseHardwareConfig: invalid response length %d expected multiple of %d",
          (int)responselen, (int)sizeof(RIL_HardwareConfig));
       return RIL_ERRNO_INVALID_RESPONSE;
   }

   int num = responselen / sizeof(RIL_HardwareConfig);
   int i;
   RIL_HardwareConfig *p_cur = (RIL_HardwareConfig *) response;

   p.writeInt32(num);

   startResponse;
   for (i = 0; i < num; i++) {
      switch (p_cur[i].type) {
         case RIL_HARDWARE_CONFIG_MODEM: {
            writeStringToParcel(p, p_cur[i].uuid);
            p.writeInt32((int)p_cur[i].state);
            p.writeInt32(p_cur[i].cfg.modem.rat);
            p.writeInt32(p_cur[i].cfg.modem.maxVoice);
            p.writeInt32(p_cur[i].cfg.modem.maxData);
            p.writeInt32(p_cur[i].cfg.modem.maxStandby);

            appendPrintBuf("%s modem: uuid=%s,state=%d,rat=%08x,maxV=%d,maxD=%d,maxS=%d", printBuf,
               p_cur[i].uuid, (int)p_cur[i].state, p_cur[i].cfg.modem.rat,
               p_cur[i].cfg.modem.maxVoice, p_cur[i].cfg.modem.maxData, p_cur[i].cfg.modem.maxStandby);
            break;
         }
         case RIL_HARDWARE_CONFIG_SIM: {
            writeStringToParcel(p, p_cur[i].uuid);
            p.writeInt32((int)p_cur[i].state);
            writeStringToParcel(p, p_cur[i].cfg.sim.modemUuid);

            appendPrintBuf("%s sim: uuid=%s,state=%d,modem-uuid=%s", printBuf,
               p_cur[i].uuid, (int)p_cur[i].state, p_cur[i].cfg.sim.modemUuid);
            break;
         }
      }
   }
   removeLastChar;
   closeResponse;
   return 0;
}

static int responseRadioCapability(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_RadioCapability) ) {
        RLOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_RadioCapability *p_cur = (RIL_RadioCapability *) response;
    p.writeInt32(p_cur->version);
    p.writeInt32(p_cur->session);
    p.writeInt32(p_cur->phase);
    p.writeInt32(p_cur->rat);
    writeStringToParcel(p, p_cur->logicalModemUuid);
    p.writeInt32(p_cur->status);

    startResponse;
    appendPrintBuf("%s[version=%d,session=%d,phase=%d,\
            rat=%s,logicalModemUuid=%s,status=%d]",
            printBuf,
            p_cur->version,
            p_cur->session,
            p_cur->phase,
            p_cur->rat,
            p_cur->logicalModemUuid,
            p_cur->status);
    closeResponse;
    return 0;
}

static bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType) {
    if ((reqType == SS_INTERROGATION) &&
        (serType == SS_CFU ||
         serType == SS_CF_BUSY ||
         serType == SS_CF_NO_REPLY ||
         serType == SS_CF_NOT_REACHABLE ||
         serType == SS_CF_ALL ||
         serType == SS_CF_ALL_CONDITIONAL)) {
        return true;
    }
    return false;
}

static int responseSSData(Parcel &p, void *response, size_t responselen) {
    RLOGD("In responseSSData");
    int num;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_StkCcUnsolSsResponse)) {
        RLOGE("invalid response length %d, expected %d",
               (int)responselen, (int)sizeof(RIL_StkCcUnsolSsResponse));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RIL_StkCcUnsolSsResponse *p_cur = (RIL_StkCcUnsolSsResponse *) response;
    p.writeInt32(p_cur->serviceType);
    p.writeInt32(p_cur->requestType);
    p.writeInt32(p_cur->teleserviceType);
    p.writeInt32(p_cur->serviceClass);
    p.writeInt32(p_cur->result);

    if (isServiceTypeCfQuery(p_cur->serviceType, p_cur->requestType)) {
        RLOGD("responseSSData CF type, num of Cf elements %d", p_cur->cfData.numValidIndexes);
        if (p_cur->cfData.numValidIndexes > NUM_SERVICE_CLASSES) {
            RLOGE("numValidIndexes is greater than max value %d, "
                  "truncating it to max value", NUM_SERVICE_CLASSES);
            p_cur->cfData.numValidIndexes = NUM_SERVICE_CLASSES;
        }
        /* number of call info's */
        p.writeInt32(p_cur->cfData.numValidIndexes);

        for (int i = 0; i < p_cur->cfData.numValidIndexes; i++) {
             RIL_CallForwardInfo cf = p_cur->cfData.cfInfo[i];

             p.writeInt32(cf.status);
             p.writeInt32(cf.reason);
             p.writeInt32(cf.serviceClass);
             p.writeInt32(cf.toa);
             writeStringToParcel(p, cf.number);
             p.writeInt32(cf.timeSeconds);
             appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d],", printBuf,
                 (cf.status==1)?"enable":"disable", cf.reason, cf.serviceClass, cf.toa,
                  (char*)cf.number, cf.timeSeconds);
             RLOGD("Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],", cf.status,
                  cf.reason, cf.serviceClass, cf.toa, (char*)cf.number, cf.timeSeconds);
        }
    } else {
        p.writeInt32 (SS_INFO_MAX);

        /* each int*/
        for (int i = 0; i < SS_INFO_MAX; i++) {
             appendPrintBuf("%s%d,", printBuf, p_cur->ssInfo[i]);
             RLOGD("Data: %d",p_cur->ssInfo[i]);
             p.writeInt32(p_cur->ssInfo[i]);
        }
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseLceStatus(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_LceStatusInfo)) {
    if (response == NULL) {
      RLOGE("invalid response: NULL");
    }
    else {
      RLOGE("responseLceStatus: invalid response length %d expecting len: d%",
            sizeof(RIL_LceStatusInfo), responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_LceStatusInfo *p_cur = (RIL_LceStatusInfo *)response;
  p.write((void *)p_cur, 1);  // p_cur->lce_status takes one byte.
  p.writeInt32(p_cur->actual_interval_ms);

  startResponse;
  appendPrintBuf("LCE Status: %d, actual_interval_ms: %d",
                 p_cur->lce_status, p_cur->actual_interval_ms);
  closeResponse;

  return 0;
}

static int responseLceData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_LceDataInfo)) {
    if (response == NULL) {
      RLOGE("invalid response: NULL");
    }
    else {
      RLOGE("responseLceData: invalid response length %d expecting len: d%",
            sizeof(RIL_LceDataInfo), responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_LceDataInfo *p_cur = (RIL_LceDataInfo *)response;
  p.writeInt32(p_cur->last_hop_capacity_kbps);

  /* p_cur->confidence_level and p_cur->lce_suspended take 1 byte each.*/
  p.write((void *)&(p_cur->confidence_level), 1);
  p.write((void *)&(p_cur->lce_suspended), 1);

  startResponse;
  appendPrintBuf("LCE info received: capacity %d confidence level %d
                  and suspended %d",
                  p_cur->last_hop_capacity_kbps, p_cur->confidence_level,
                  p_cur->lce_suspended);
  closeResponse;

  return 0;
}

static int responseActivityData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_ActivityStatsInfo)) {
    if (response == NULL) {
      RLOGE("invalid response: NULL");
    }
    else {
      RLOGE("responseActivityData: invalid response length %d expecting len: d%",
            sizeof(RIL_ActivityStatsInfo), responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_ActivityStatsInfo *p_cur = (RIL_ActivityStatsInfo *)response;
  p.writeInt32(p_cur->sleep_mode_time_ms);
  p.writeInt32(p_cur->idle_mode_time_ms);
  for(int i = 0; i < RIL_NUM_TX_POWER_LEVELS; i++) {
    p.writeInt32(p_cur->tx_mode_time_ms[i]);
  }
  p.writeInt32(p_cur->rx_mode_time_ms);

  startResponse;
  appendPrintBuf("Modem activity info received: sleep_mode_time_ms %d idle_mode_time_ms %d
                  tx_mode_time_ms %d %d %d %d %d and rx_mode_time_ms %d",
                  p_cur->sleep_mode_time_ms, p_cur->idle_mode_time_ms, p_cur->tx_mode_time_ms[0],
                  p_cur->tx_mode_time_ms[1], p_cur->tx_mode_time_ms[2], p_cur->tx_mode_time_ms[3],
                  p_cur->tx_mode_time_ms[4], p_cur->rx_mode_time_ms);
   closeResponse;

  return 0;
}

static void triggerEvLoop() {
    int ret;
    if (!pthread_equal(pthread_self(), s_tid_dispatch)) {
        /* trigger event loop to wakeup. No reason to do this,
         * if we're in the event loop thread */
         do {
            ret = write (s_fdWakeupWrite, " ", 1);
         } while (ret < 0 && errno == EINTR);
    }
}

static void rilEventAddWakeup(struct ril_event *ev) {
    ril_event_add(ev);
    triggerEvLoop();
}

static void sendSimStatusAppInfo(Parcel &p, int num_apps, RIL_AppStatus appStatus[]) {
        p.writeInt32(num_apps);
        startResponse;
        for (int i = 0; i < num_apps; i++) {
            p.writeInt32(appStatus[i].app_type);
            p.writeInt32(appStatus[i].app_state);
            p.writeInt32(appStatus[i].perso_substate);
            writeStringToParcel(p, (const char*)(appStatus[i].aid_ptr));
            writeStringToParcel(p, (const char*)
                                          (appStatus[i].app_label_ptr));
            p.writeInt32(appStatus[i].pin1_replaced);
            p.writeInt32(appStatus[i].pin1);
            p.writeInt32(appStatus[i].pin2);
            appendPrintBuf("%s[app_type=%d,app_state=%d,perso_substate=%d,\
                    aid_ptr=%s,app_label_ptr=%s,pin1_replaced=%d,pin1=%d,pin2=%d],",
                    printBuf,
                    appStatus[i].app_type,
                    appStatus[i].app_state,
                    appStatus[i].perso_substate,
                    appStatus[i].aid_ptr,
                    appStatus[i].app_label_ptr,
                    appStatus[i].pin1_replaced,
                    appStatus[i].pin1,
                    appStatus[i].pin2);
        }
        closeResponse;
}

static int responseSimStatus(Parcel &p, void *response, size_t responselen) {
    int i;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen == sizeof (VIA_RIL_CardStatus)) {
        VIA_RIL_CardStatus *p_cur = ((VIA_RIL_CardStatus *) response);

        p.writeInt32(p_cur->card_state);
        p.writeInt32(p_cur->universal_pin_state);
        p.writeInt32(p_cur->pin1Count);
        p.writeInt32(p_cur->pin2Count);
        p.writeInt32(p_cur->puk1Count);
        p.writeInt32(p_cur->puk2Count);
        p.writeInt32(p_cur->gsm_umts_subscription_app_index);
        p.writeInt32(p_cur->cdma_subscription_app_index);
        p.writeInt32(p_cur->num_applications);

        startResponse;
        for (i = 0; i < p_cur->num_applications; i++) {
            p.writeInt32(p_cur->applications[i].app_type);
            p.writeInt32(p_cur->applications[i].app_state);
            p.writeInt32(p_cur->applications[i].perso_substate);
            writeStringToParcel(p, (const char*)(p_cur->applications[i].aid_ptr));
            writeStringToParcel(p, (const char*)(p_cur->applications[i].app_label_ptr));
            p.writeInt32(p_cur->applications[i].pin1_replaced);
            p.writeInt32(p_cur->applications[i].pin1);
            p.writeInt32(p_cur->applications[i].pin2);
            appendPrintBuf("%s[app_type=%d,app_state=%d,perso_substate=%d,\
                           aid_ptr=%s,app_label_ptr=%s,pin1_replaced=%d,pin1=%d,pin2=%d],",
                           printBuf,
                           p_cur->applications[i].app_type,
                           p_cur->applications[i].app_state,
                           p_cur->applications[i].perso_substate,
                           p_cur->applications[i].aid_ptr,
                           p_cur->applications[i].app_label_ptr,
                           p_cur->applications[i].pin1_replaced,
                           p_cur->applications[i].pin1,
                           p_cur->applications[i].pin2);
        }
        closeResponse;
	} else if (responselen == sizeof (RIL_CardStatus_v6)) {
        RIL_CardStatus_v6 *p_cur = ((RIL_CardStatus_v6 *) response);

        p.writeInt32(p_cur->card_state);
        p.writeInt32(p_cur->universal_pin_state);
        p.writeInt32(p_cur->gsm_umts_subscription_app_index);
        p.writeInt32(p_cur->cdma_subscription_app_index);
        p.writeInt32(p_cur->ims_subscription_app_index);

        sendSimStatusAppInfo(p, p_cur->num_applications, p_cur->applications);
    } else if (responselen == sizeof (RIL_CardStatus_v5)) {
        RIL_CardStatus_v5 *p_cur = ((RIL_CardStatus_v5 *) response);

        p.writeInt32(p_cur->card_state);
        p.writeInt32(p_cur->universal_pin_state);
        p.writeInt32(p_cur->gsm_umts_subscription_app_index);
        p.writeInt32(p_cur->cdma_subscription_app_index);
        p.writeInt32(-1);

        sendSimStatusAppInfo(p, p_cur->num_applications, p_cur->applications);
    } else {
        LOGE("responseSimStatus: A RilCardStatus_v6 or _v5 expected\n");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    return 0;
}

static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    int num = responselen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    RIL_GSM_BroadcastSmsConfigInfo **p_cur =
                (RIL_GSM_BroadcastSmsConfigInfo **) response;
    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->fromServiceId);
        p.writeInt32(p_cur[i]->toServiceId);
        p.writeInt32(p_cur[i]->fromCodeScheme);
        p.writeInt32(p_cur[i]->toCodeScheme);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId=%d, \
                fromCodeScheme=%d, toCodeScheme=%d, selected =%d]",
                printBuf, i, p_cur[i]->fromServiceId, p_cur[i]->toServiceId,
                p_cur[i]->fromCodeScheme, p_cur[i]->toCodeScheme,
                p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    RIL_CDMA_BroadcastSmsConfigInfo **p_cur =
               (RIL_CDMA_BroadcastSmsConfigInfo **) response;

    int num = responselen / sizeof (RIL_CDMA_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++ ) {
        p.writeInt32(p_cur[i]->service_category);
        p.writeInt32(p_cur[i]->language);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: srvice_category=%d, language =%d, \
              selected =%d], ",
              printBuf, i, p_cur[i]->service_category, p_cur[i]->language,
              p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

static int responseCdmaSms(Parcel &p, void *response, size_t responselen) {
    int num;
    int digitCount;
    int digitLimit;
    uint8_t uct;
    void* dest;

    LOGD("Inside responseCdmaSms");

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CDMA_SMS_Message)) {
        LOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof(RIL_CDMA_SMS_Message));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_SMS_Message *p_cur = (RIL_CDMA_SMS_Message *) response;
    p.writeInt32(p_cur->uTeleserviceID);
    p.write(&(p_cur->bIsServicePresent),sizeof(uct));
    p.writeInt32(p_cur->uServicecategory);
    p.writeInt32(p_cur->sAddress.digit_mode);
    p.writeInt32(p_cur->sAddress.number_mode);
    p.writeInt32(p_cur->sAddress.number_type);
    p.writeInt32(p_cur->sAddress.number_plan);
    p.write(&(p_cur->sAddress.number_of_digits), sizeof(uct));
    digitLimit= MIN((p_cur->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sAddress.digits[digitCount]),sizeof(uct));
    }

    p.writeInt32(p_cur->sSubAddress.subaddressType);
    p.write(&(p_cur->sSubAddress.odd),sizeof(uct));
    p.write(&(p_cur->sSubAddress.number_of_digits),sizeof(uct));
    digitLimit= MIN((p_cur->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sSubAddress.digits[digitCount]),sizeof(uct));
    }

    digitLimit= MIN((p_cur->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    p.writeInt32(p_cur->uBearerDataLen);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
       p.write(&(p_cur->aBearerData[digitCount]), sizeof(uct));
    }

    startResponse;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.number_mode=%d, sAddress.number_type=%d, ",
            printBuf, p_cur->uTeleserviceID,p_cur->bIsServicePresent,p_cur->uServicecategory,
            p_cur->sAddress.digit_mode, p_cur->sAddress.number_mode,p_cur->sAddress.number_type);
    closeResponse;

    return 0;
}

static int responseDcRtInfo(Parcel &p, void *response, size_t responselen)
{
    int num = responselen / sizeof(RIL_DcRtInfo);
    if ((responselen % sizeof(RIL_DcRtInfo) != 0) || (num != 1)) {
        RLOGE("responseDcRtInfo: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_DcRtInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RIL_DcRtInfo *pDcRtInfo = (RIL_DcRtInfo *)response;
    p.writeInt64(pDcRtInfo->time);
    p.writeInt32(pDcRtInfo->powerState);
    appendPrintBuf("%s[time=%d,powerState=%d]", printBuf,
        pDcRtInfo->time,
        pDcRtInfo->powerState);
    closeResponse;

    return 0;
}

static int responseReadPbFromRuim(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_Ruim_Phonebook_Record) ) {
        LOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_Ruim_Phonebook_Record));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_Ruim_Phonebook_Record *p_cur = (RIL_Ruim_Phonebook_Record *) response;
    p.writeInt32(p_cur->index);
    writeStringToParcel(p, p_cur->number);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->name);
    p.writeInt32(p_cur->coding);

    startResponse;
    appendPrintBuf("%sindex=%d, number=%s, type=%d, name=%s, coding=%d", printBuf, p_cur->index, (char*)p_cur->number, p_cur->type,
        (char*)p_cur->name, p_cur->coding);
    closeResponse;


    return 0;
}

#ifdef ADD_MTK_REQUEST_URC
static void dispatchMTKWritePbToRuim(Parcel &p, RequestInfo *pRI)
{
    RIL_MTK_Ruim_Phonebook_Record p_Record;
    int32_t t;
    status_t status;

    memset(&p_Record, 0, sizeof(p_Record));

    // we only check status at the end
    status = p.readInt32(&t);
    p_Record.type = t;
    status |= p.readInt32(&t);
    p_Record.index = t;
    p_Record.number = strdupReadString(p);
    status |= p.readInt32(&t);
    p_Record.ton = t;
    p_Record.alphaid= strdupReadString(p);

    startRequest;
    appendPrintBuf("%s type=%d,index=%d,number=%s,ton=%d,alphaid=%s", printBuf,
        p_Record.type, p_Record.index, (char*)p_Record.number, p_Record.ton, (char*)p_Record.alphaid);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber, &p_Record, sizeof(p_Record), pRI);

#ifdef MEMSET_FREED
    memsetString (p_Record.number);
    memsetString (p_Record.alphaid);
#endif

    free (p_Record.number);
    free (p_Record.alphaid);

#ifdef MEMSET_FREED
    memset(&p_Record, 0, sizeof(p_Record));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static int responseMTKReadPbFromRuim(Parcel &p, void *response, size_t responselen) {
    int num = 0;
    LOGD("mtk feature, num1 = %d in ril.cpp, responselen = %d", num, responselen);
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_MTK_Ruim_Phonebook_Record ) != 0) {
        LOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_MTK_Ruim_Phonebook_Record));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of call info's */
    num = responselen / sizeof(RIL_MTK_Ruim_Phonebook_Record);
    LOGD("mtk feature, num = %d in ril.cpp, responselen = %d", num, responselen);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_MTK_Ruim_Phonebook_Record p_cur = ((RIL_MTK_Ruim_Phonebook_Record *) response)[i];
        /* each call info */
        p.writeInt32(p_cur.type);
        p.writeInt32(p_cur.index);
        writeStringToParcel(p, p_cur.number);
        p.writeInt32(p_cur.ton);
        writeStringToParcel(p, p_cur.alphaid);
       appendPrintBuf("%s type=%d, index=%d, number=%s, ton=%d, alphaid=%s", printBuf, p_cur.type, p_cur.index, (char*)p_cur.number, p_cur.ton,
        (char*)p_cur.alphaid);
    }
    closeResponse;

    return 0;
}

static void dispatchPlayDTMFTone(Parcel &p, RequestInfo *pRI)
{
    RIL_Play_DTMF_Tone dtmfTone;
    int32_t t;
    status_t status;

    memset(&dtmfTone, 0, sizeof(dtmfTone));

    status = p.readInt32(&t);
    dtmfTone.mode = (int)t;

    if(status != NO_ERROR)
    {
        goto invalid;
    }

    if(1 == dtmfTone.mode)
    {
        status = p.readInt32(&t);
        dtmfTone.dtmfIndex = (int)t;
        status |= p.readInt32(&t);
        dtmfTone.volume = (int)t;
        status |= p.readInt32(&t);
        dtmfTone.duration = (int)t;
        if (status != NO_ERROR)
        {
            goto invalid;
        }
    }
    startRequest;
    appendPrintBuf("%smode=%d,dtmfIndex=%d, volume=%d, duration=%d", printBuf,
        dtmfTone.mode, dtmfTone.dtmfIndex, dtmfTone.volume, dtmfTone.duration);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &dtmfTone, sizeof(dtmfTone), pRI);

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchPlayToneSeq(Parcel &p, RequestInfo *pRI)
{
    RIL_Play_Tone_Seq toneSeq;
    int32_t t;
    status_t status;
    size_t toneDataLen;
    int i = 0;

    memset(&toneSeq, 0, sizeof(toneSeq));

    startRequest;
    status = p.readInt32(&t);
    toneSeq.num= (int)t;
    appendPrintBuf("%snum=%d,", printBuf, toneSeq.num);
    if(status != NO_ERROR || (toneSeq.num <= 0) || (toneSeq.num > MAX_TONE_DATA_NUM))
    {
        goto invalid;
    }

    status = p.readInt32(&t);
    toneSeq.interation = (int)t;
    status |= p.readInt32(&t);
    toneSeq.volume = (int)t;
    appendPrintBuf("%sinteration=%d,volume=%d", printBuf, toneSeq.interation, toneSeq.volume);
    if (status != NO_ERROR)
    {
        goto invalid;
    }

    for(i = 0 ; i < toneSeq.num ; i++)
    {
        status = p.readInt32(&t);
        toneSeq.toneData[i].duration = (int)t;
        status |= p.readInt32(&t);
        toneSeq.toneData[i].freq[0]= (int)t;
        status |= p.readInt32(&t);
        toneSeq.toneData[i].freq[1]= (int)t;
        appendPrintBuf("%stoneData[%d].duration=%d,toneData[%d].freq[0]=%d,toneData[%d].freq[1]=%d", printBuf,
            i, toneSeq.toneData[i].duration, i, toneSeq.toneData[i].freq[0], i, toneSeq.toneData[i].freq[1]);
        if (status != NO_ERROR)
        {
            goto invalid;
        }
    }
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &toneSeq, sizeof(toneSeq), pRI);

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static int
responseGetSmsStatus(Parcel &p, void *response, size_t responselen) {
    int numInts = 0;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;

    numInts = responselen / sizeof(int *);
    //p.writeInt32 (numInts);

    /* each int*/
    startResponse;
    for (int i = 0 ; i < numInts ; i++) {
        appendPrintBuf("%s%d,", printBuf, p_int[i]);
        p.writeInt32(p_int[i]);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/// [C2K] IRAT feature.
static int responseIratStateChange(Parcel &p, void *response, size_t responselen)
{
	LOGD("responseIRatStateChange: 0 responselen = %d, sizeof(RIL_Pdn_IratInfo) = %d.",
			responselen, sizeof(RIL_Pdn_IratInfo));

	if (response == NULL || responselen == 0) {
		LOGE("invalid response: NULL");
		return RIL_ERRNO_INVALID_RESPONSE;
	}

	if (responselen != sizeof(RIL_Pdn_IratInfo)) {
		LOGE("invalid response length %d expected sizeof (RIL_Pdn_IratInfo) of %d\n",
				(int) responselen, (int) sizeof(RIL_Pdn_IratInfo));
		return RIL_ERRNO_INVALID_RESPONSE;
	}

	startResponse;

	RIL_Pdn_IratInfo *irat_info = (RIL_Pdn_IratInfo*) response;
	p.writeInt32(irat_info->sourceRat);
	p.writeInt32(irat_info->targetRat);
	p.writeInt32(irat_info->action);
	p.writeInt32(irat_info->type);
	appendPrintBuf("%s [sourceRat = %d, targetRat = %d, action = %d, type = %d]", printBuf,
			irat_info->sourceRat, irat_info->targetRat, irat_info->action, irat_info->type);

	closeResponse;
    return 0;
}

static void onRequestUnsupportComplete(int32_t token, RIL_SOCKET_ID socket_id) {
size_t errorOffset;

    Parcel p;

    p.writeInt32 (RESPONSE_SOLICITED);
    p.writeInt32 (token);
    errorOffset = p.dataPosition();

    p.writeInt32 (RIL_E_REQUEST_NOT_SUPPORTED);

    LOGE("onRequestUnsupportComplete: unsupport token = %d", token);
    sendResponse(p, socket_id);
}

#endif

/**
 * A write on the wakeup fd is done just to pop us out of select()
 * We empty the buffer here and then ril_event will reset the timers on the
 * way back down
 */
static void processWakeupCallback(int fd, short flags, void *param) {
    char buff[16];
    int ret;

    LOGV("processWakeupCallback");

    /* empty our wakeup socket out */
    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

static char * RIL_getRilSocketName() {
    return rild;
}

extern "C"
void RIL_setRilSocketName(char * s) {
    strncpy(rild, s, MAX_SOCKET_NAME_LENGTH);
}

static void onCommandsSocketClosed(int owner, RIL_SOCKET_ID socket_id) {
    int ret;
    RequestInfo *p_cur;

    /* mark pending requests as "cancelled" so we dont report responses */
    /* Hook for current context
       pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo **    pendingRequestsHook = &s_pendingRequests;

#if (SIM_COUNT >= 2)
    if (socket_id == RIL_SOCKET_2) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket2;
        pendingRequestsHook = &s_pendingRequests_socket2;
    }
#if (SIM_COUNT >= 3)
    else if (socket_id == RIL_SOCKET_3) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket3;
        pendingRequestsHook = &s_pendingRequests_socket3;
    }
#endif
#if (SIM_COUNT >= 4)
    else if (socket_id == RIL_SOCKET_4) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket4;
        pendingRequestsHook = &s_pendingRequests_socket4;
    }
#endif
#endif

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    p_cur = *pendingRequestsHook;

    for (p_cur = *pendingRequestsHook
            ; p_cur != NULL
            ; p_cur  = p_cur->p_next
    ) {
        if (owner == p_cur->owner) {
            p_cur->cancelled = 1;
        }
    }
    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);
}

#define MAX_DATA_SIZE 2048

RequestInfoProxy* createRpcRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, int* pId) {
    RLOGD("createRpcRequestInfoProxy method start");

    RequestInfoProxy * pRequest = NULL;
    status_t status;
    int proxyId = -1;
    int32_t request;
    int32_t token;
    RLOGD("createRpcRequestInfoProxy method start");
    pRequest = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
    assert(pRequest);
    pRequest->p = new Parcel();
    pRequest->p->setData((uint8_t *) buffer, buflen);
    int temBuffer;
    status = pRequest->p->readInt32(&temBuffer);
    RLOGD("createRpcRequestInfoProxy method starteee temBuffer = %d", temBuffer);

    // Make sure the request ID not changed after wrap into proxy
    assert(temBuffer == request);
    status = pRequest->p->readInt32(&temBuffer);
    RLOGD("createRpcRequestInfoProxy method startfff temBuffer = %d", temBuffer);

    // Make sure the token not changed after wrap into proxy
    assert(temBuffer == token);
    assert(status == NOERROR);
    buflen++;
    RLOGD("createRpcRequestInfoProxy method start111");
    pRequest->p_next = NULL;
    pRequest->pRI = pRI;
    RLOGD("createRpcRequestInfoProxy method startggg = %d", pRI->pCI->requestNumber);

    proxyId = s_commands[pRI->pCI->requestNumber].proxyId;
    RLOGD("createRpcRequestInfoProxy method starthhh = %d", proxyId);

    /* Save dispatched proxy in RequestInfo */
    pRequest->pRI->cid = (RILChannelId) proxyId;
    (*pId) = proxyId;
    return pRequest;
}

void enqueueRpcRequest(RequestInfo* pRI, void *buffer, size_t buflen) {
    RequestInfoProxy * proxy = NULL;
    int proxyId;
    const char* proxyName = NULL;
    int queueSize = 0;
    RLOGD("enqueueRpcRequest method start");
    proxy = createRpcRequestInfoProxy(pRI, buffer, buflen, &proxyId);

    proxyName = ::proxyIdToString(proxyId);

    /* Save dispatched proxy in RequestInfo */
    queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

    if (0 != queueSize) {
        RLOGD("Request timed callback to %s is busy. total:%d requests pending",
                proxyName, queueSize+1);
    }
}

static void processRpcCommandsCallback(int fd, short flags, void *param) {
    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;
    int count;
    Parcel p;
    status_t status;
    int32_t request;
    int32_t token;
    char *readbuf = (char *)malloc(MAX_COMMAND_BYTES);
    if(NULL == readbuf)
    {
        LOGD("processRpcCommandsCallback: malloc failed");
        return ;
    }
    LOGD("processRpcCommandsCallback");
    assert(fd == s_fdRpcCommand);
    count = read(s_fdRpcCommand, readbuf,PARCEL_COMMAND_BYTES);
    LOGD("processRpcCommandsCallback count = %d",count);
    LOGD("PARCEL_COMMAND_BYTES is 1024 now");
//c+w
    if(count == 0){
        LOGD("count = 0, del event");
        close(s_fdRpcCommand);
        s_fdRpcCommand = -1;

        ril_event_del(&s_rpc_command_event);

        /* start listening for new connections again */
        rilEventAddWakeup(&s_rpc_event);

        onCommandsSocketClosed(1, RIL_SOCKET_1);
        free(readbuf);
        readbuf = NULL;
        return;
    }

    // get request and token
    p.setData((uint8_t *) readbuf, count);
    status = p.readInt32(&request);
    status = p.readInt32(&token);

    RequestInfo *pRI;
    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    // set token
    pRI->token = token;        // token is not used in this context

    LOGD("processRpcCommandsCallback request = %d", request);
    LOGD("processRpcCommandsCallback token = %d", token);

    // set CommandInfo
    pRI->pCI = &(s_commands[request]);
    pRI->owner = 0;

    ret = pthread_mutex_lock(&s_pendingRequestsMutex);
    assert(ret == 0);

    pRI->p_next = s_pendingRequests;
    s_pendingRequests = pRI;

    ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
    assert(ret == 0);
    LOGD("C[locl]> %s", requestToString(request));
    LOGD("enqueueRpcRequest");
    enqueueRpcRequest(pRI, readbuf, count);
}

static int
sendResponseForCTClient (const void *data, size_t dataSize) {
    int fd = s_fdCTClientCommand;
    int ret;

    LOGD ("-------CTClient:sendResponseForCTClient fd = %d", fd);
    if (s_fdCTClientCommand < 0) {
        return -1;
    }

    pthread_mutex_lock(&s_ctclient_writeMutex);

    ret = blockingWrite(fd, data, dataSize);
    if (ret < 0) {
        pthread_mutex_unlock(&s_ctclient_writeMutex);
        //close(s_fdCTClientCommand);
        ril_event_del(&s_ctclient_command_event);
        s_fdCTClientCommand = -1;

        rilEventAddWakeup (&s_ctclient_listen_event);

        s_callbacks.reportUsbDisconn();
        return ret;
    }

    pthread_mutex_unlock(&s_ctclient_writeMutex);

    return 0;
}

/**
 * To be called from dispatch thread
 * Issue a single local request, ensuring that the response
 * is not sent back up to the command process
 */
static void
issueCTClientRequest(const char *line) {
    RequestInfo *pRI;
    int ret;
    int request = RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT;

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    //pRI->local = 1;
    pRI->token = 0xffffffff;        // token is not used in this context
    pRI->pCI = &(s_commands[request]);

    ret = pthread_mutex_lock(&s_pendingRequestsMutex);
    assert (ret == 0);

    pRI->p_next = s_pendingRequests;
    s_pendingRequests = pRI;

    ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
    assert (ret == 0);

    s_callbacks.onRequest(request, (void*)line, strlen(line), pRI);
}

static char * findNextEOL(char *cur)
{
    // Find next newline
    while (*cur != '\0' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}

static char s_ATBuffer[MAX_COMMAND_BYTES+1];
static char *s_ATBufferCur = s_ATBuffer;

static const char *readline(int *is_closed)
{
    ssize_t count;

    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;
    *is_closed = 0;

     /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */
    if (*s_ATBufferCur == '\0') {

        /* empty buffer */
        s_ATBufferCur = s_ATBuffer;
        *s_ATBufferCur = '\0';
        p_read = s_ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines

        while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
            s_ATBufferCur++;

        p_eol = findNextEOL(s_ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(s_ATBufferCur);

            memmove(s_ATBuffer, s_ATBufferCur, len + 1);
            p_read = s_ATBuffer + len;
            s_ATBufferCur = s_ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }

    while (p_eol == NULL) {

        if (0 == MAX_COMMAND_BYTES - (p_read - s_ATBuffer)) {
            LOGE("ERROR: Input line exceeded buffer\n");
            /* ditch buffer and start over again */
            s_ATBufferCur = s_ATBuffer;
            *s_ATBufferCur = '\0';
            p_read = s_ATBuffer;
        }

        do {
            count = read(s_fdCTClientCommand, p_read,
                            MAX_COMMAND_BYTES - (p_read - s_ATBuffer));
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            //AT_DUMP( "<< ", p_read, count );
            //s_readCount += count;

            p_read[count] = '\0';

            // skip over leading newlines
            while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
                s_ATBufferCur++;

            p_eol = findNextEOL(s_ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            /* read error encountered or EOF reached */
            if(count == 0) {
                LOGD("CTClient: read EOF");
                *is_closed = 1;
            } else if(errno == EAGAIN){
                LOGD("CTClient: nothing to read now");
            } else {
                LOGD("CTClient: read error %s", strerror(errno));
                *is_closed = 1;
            }
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */

    ret = s_ATBufferCur;
    *p_eol = '\0';
    s_ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
                              /* and there will be a \0 at *p_read */

    LOGD("CTClient> %s\n", ret);
    return ret;
}

extern "C" void handle_bad_fdCTClientCommand()
{
    LOGD("handle_bad_fdCTClientCommand fd = %d\n", s_fdCTClientCommand);
    if (s_fdCTClientCommand != -1) {
        //close(s_fdCTClientCommand);
        ril_event_del(&s_ctclient_command_event);
        s_fdCTClientCommand = -1;

        rilEventAddWakeup (&s_ctclient_listen_event);
    }
}

static void processCTClientCommandsCallback(int fd, short flags, void *param) {
    int count;
    const char *line;

    LOGD(">>>>>>>> processCTClientCommandsCallback begin fd = %d, s_fdCTClientCommand = %d", fd, s_fdCTClientCommand);
    assert(fd == s_fdCTClientCommand);

    int is_closed = 0;

    line = readline(&is_closed);
    while (line != NULL) {
        issueCTClientRequest(line);
        line = readline(&is_closed);
    }
    if (is_closed) {
        LOGD("CTClient: close s_fdCTClientCommand");
        close(s_fdCTClientCommand);
        memset(s_ATBuffer, 0, sizeof(s_ATBuffer));
        s_ATBufferCur = s_ATBuffer;
        ril_event_del(&s_ctclient_command_event);
        s_fdCTClientCommand = -1;

        rilEventAddWakeup (&s_ctclient_listen_event);

        s_callbacks.reportUsbDisconn();
    }
    LOGD("<<<<<<<< processCTClientCommandsCallback end");
}

static void CTClientListenCallback(int fd, short flags, void *param)
{
    int ret;
    int err;
    //RecordStream *p_rs;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    LOGI("CTClientListenCallback s_fdCTClientCommand= %d", s_fdCTClientCommand);
    assert (s_fdCTClientCommand < 0);
    assert (fd == s_fdCTClientListen);
    LOGI("assert success");
    s_fdCTClientCommand = accept(s_fdCTClientListen, (sockaddr *) &peeraddr, &socklen);

    if (s_fdCTClientCommand < 0 ) {
        LOGE("CP_COMM: Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(&s_ctclient_listen_event);
          return;
    }
    LOGI("accept success s_fdCTClientCommand = %d", s_fdCTClientCommand);
    ret = fcntl(s_fdCTClientCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("CP_COMM: Error setting O_NONBLOCK errno:%d", errno);
    }

    LOGI("CTClientListenCallback: new connection");

    //p_rs = record_stream_new(s_fdCommand, MAX_COMMAND_BYTES);

    ril_event_set (&s_ctclient_command_event, s_fdCTClientCommand, 1,
        processCTClientCommandsCallback, NULL);

    rilEventAddWakeup (&s_ctclient_command_event);

    s_ATBufferCur = s_ATBuffer;
    *s_ATBufferCur = '\0';
    //onNewCommandConnect();
}

static void processCommandsCallback(int fd, short flags, void *param) {
    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;
    SocketListenParam *p_info = (SocketListenParam *)param;
    assert(fd == p_info->fdCommand);
    p_rs = p_info->p_rs;

    for (;;) {
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { /* && p_record != NULL */
            processCommandBuffer(p_record, recordlen, 0, p_info->socket_id);
        }
    }

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            LOGE("error on reading command socket errno:%d\n", errno);
        } else {
            LOGW("EOS.  Closing command socket.");
        }

        close(fd);
        p_info->fdCommand = -1;

        //send "0" to viatelecom-withuim-ril.so,tell it socket disconnected
        //the "0" assigned to rildSockconn in viatelecom-withuim-ril.so
        s_callbacks.reportRILDConn(0);

        ril_event_del(p_info->commands_event);

        record_stream_free(p_rs);

        /* start listening for new connections again */
        rilEventAddWakeup(p_info->listen_event);

        onCommandsSocketClosed(0, p_info->socket_id);
    }
}

static void onNewCommandConnect(RIL_SOCKET_ID socket_id) {
    // Inform we are connected and the ril version
    int rilVer = s_callbacks.version;
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RIL_CONNECTED,
                                    &rilVer, sizeof(rilVer), socket_id);

    // implicit radio state changed
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                    NULL, 0, socket_id);

    // Send last NITZ time data, in case it was missed
    if (s_lastNITZTimeData != NULL) {
        sendResponseRaw(s_lastNITZTimeData, s_lastNITZTimeDataSize, socket_id);

        free(s_lastNITZTimeData);
        s_lastNITZTimeData = NULL;
    }

    // Get version string
    if (s_callbacks.getVersion != NULL) {
        const char *version;
        version = s_callbacks.getVersion();
        LOGI("RIL Daemon version: %s\n", version);

        property_set(PROPERTY_RIL_IMPL, version);
    } else {
        LOGI("RIL Daemon version: unavailable\n");
        property_set(PROPERTY_RIL_IMPL, "unavailable");
    }

}

static void listenCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    int fdCommand = -1;
    SocketListenParam *p_info = (SocketListenParam *)param;
    RecordStream *p_rs;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);

    struct passwd *pwd = NULL;

    assert (*p_info->fdCommand < 0);
    assert (fd == *p_info->fdListen);
    /// M: Save s_fdCommand as local variable, mainloop thread may reset s_fdCommand
    // to -1 if write error
    fdCommand = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (fdCommand < 0 ) {
        LOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(p_info->listen_event);
	      return;
    }

    /* check the credential of the other side and only accept socket from
     * phone process
     */
    errno = 0;
    is_phone_socket = 0;

    err = getsockopt(fdCommand, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);

    if (err == 0 && szCreds > 0) {
        errno = 0;
        pwd = getpwuid(creds.uid);
        if (pwd != NULL) {
            if (strcmp(pwd->pw_name, p_info->processName) == 0) {
                is_phone_socket = 1;
            } else {
                LOGE("RILD can't accept socket from process %s", pwd->pw_name);
            }
        } else {
            LOGE("Error on getpwuid() errno: %d", errno);
        }
    } else {
        LOGD("Error on getsockopt() errno: %d", errno);
    }

    if ( !is_phone_socket ) {
      RLOGE("RILD must accept socket from %s", p_info->processName);
      close(fdCommand);
      fdCommand = -1;

      onCommandsSocketClosed(0, p_info->socket_id);


      /* start listening for new connections again */
      rilEventAddWakeup(p_info->listen_event);

      return;
    }

    ret = fcntl(fdCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }
    RLOGI("libril: new connection to  %s", rilSocketIdToString(p_info->socket_id));

    LOGI("libc2kril: new connection");

    p_info->fdCommand = fdCommand;

    p_rs = record_stream_new(p_info->fdCommand, MAX_COMMAND_BYTES);

    p_info->p_rs = p_rs;

    ril_event_set (p_info->commands_event, p_info->fdCommand, 1,
        p_info->processCommandsCallback, p_info);

    rilEventAddWakeup (p_info->commands_event);

    onNewCommandConnect(p_info->socket_id);

}

static void freeDebugCallbackArgs(int number, char **args) {
    for (int i = 0; i < number; i++) {
        if (args[i] != NULL) {
            free(args[i]);
        }
    }
    free(args);
}

static void debugCallback (int fd, short flags, void *param) {
    int acceptFD, option;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int data;
    unsigned int qxdm_data[6];
    const char *deactData[1] = {"1"};
    char *actData[1];
    RIL_Dial dialData;
    int hangupData[1] = {1};
    int number;
    char **args;

    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;
    int sim_id = 0;

    RLOGI("debugCallback for socket %s", rilSocketIdToString(socket_id));

    acceptFD = accept (fd,  (sockaddr *) &peeraddr, &socklen);

    if (acceptFD < 0) {
        LOGE ("error accepting on debug port: %d\n", errno);
        return;
    }

    if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
        LOGE ("error reading on socket: number of Args: \n");
        return;
    }
    args = (char **) malloc(sizeof(char*) * number);

    for (int i = 0; i < number; i++) {
        int len;
        if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
            LOGE ("error reading on socket: Len of Args: \n");
            freeDebugCallbackArgs(i, args);
            return;
        }
        // +1 for null-term
        args[i] = (char *) malloc((sizeof(char) * len) + 1);
        if (recv(acceptFD, args[i], sizeof(char) * len, 0)
            != (int)sizeof(char) * len) {
            LOGE ("error reading on socket: Args[%d] \n", i);
            freeDebugCallbackArgs(i, args);
            return;
        }
        char * buf = args[i];
        buf[len] = 0;
        if ((i+1) == number) {
            /* The last argument should be sim id 0(SIM1)~3(SIM4) */
            sim_id = atoi(args[i]);
            switch (sim_id) {
                case 0:
                    socket_id = RIL_SOCKET_1;
                    break;
            #if (SIM_COUNT >= 2)
                case 1:
                    socket_id = RIL_SOCKET_2;
                    break;
            #endif
            #if (SIM_COUNT >= 3)
                case 2:
                    socket_id = RIL_SOCKET_3;
                    break;
            #endif
            #if (SIM_COUNT >= 4)
                case 3:
                    socket_id = RIL_SOCKET_4;
                    break;
            #endif
                default:
                    socket_id = RIL_SOCKET_1;
                    break;
            }
        }
    }

    switch (atoi(args[0])) {
        case 0:
            LOGI ("Connection on debug port: issuing reset.");
            issueLocalRequest(RIL_REQUEST_RESET_RADIO, NULL, 0, socket_id);
            break;
        case 1:
            LOGI ("Connection on debug port: issuing radio power off.");
            data = 0;
            issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int), socket_id);
            // Close the socket
              if (socket_id == RIL_SOCKET_1 && s_ril_param_socket.fdCommand > 0) {
                  close(s_ril_param_socket.fdCommand);
                  s_ril_param_socket.fdCommand = -1;
              }
          #if (SIM_COUNT == 2)
              else if (socket_id == RIL_SOCKET_2 && s_ril_param_socket2.fdCommand > 0) {
                  close(s_ril_param_socket2.fdCommand);
                  s_ril_param_socket2.fdCommand = -1;
              }
          #endif

            break;
        case 2:
            LOGI ("Debug port: issuing unsolicited voice network change.");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, socket_id);
            break;
        case 3:
            LOGI ("Debug port: QXDM log enable.");
            qxdm_data[0] = 65536;     // head.func_tag
            qxdm_data[1] = 16;        // head.len
            qxdm_data[2] = 1;         // mode: 1 for 'start logging'
            qxdm_data[3] = 32;        // log_file_size: 32megabytes
            qxdm_data[4] = 0;         // log_mask
            qxdm_data[5] = 8;         // log_max_fileindex
            issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, qxdm_data,
                              6 * sizeof(int), socket_id);
            break;
        case 4:
            LOGI ("Debug port: QXDM log disable.");
            qxdm_data[0] = 65536;
            qxdm_data[1] = 16;
            qxdm_data[2] = 0;          // mode: 0 for 'stop logging'
            qxdm_data[3] = 32;
            qxdm_data[4] = 0;
            qxdm_data[5] = 8;
            issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, qxdm_data,
                              6 * sizeof(int), socket_id);
            break;
        case 5:
            LOGI("Debug port: Radio On");
            data = 1;
            issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int), socket_id);
            sleep(2);
            // Set network selection automatic.
            issueLocalRequest(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, NULL, 0, socket_id);
            break;
        case 6:
            LOGI("Debug port: Setup Data Call, Apn :%s\n", args[1]);
            actData[0] = args[1];
            issueLocalRequest(RIL_REQUEST_SETUP_DATA_CALL, &actData,
                              sizeof(actData), socket_id);
            break;
        case 7:
            LOGI("Debug port: Deactivate Data Call");
            issueLocalRequest(RIL_REQUEST_DEACTIVATE_DATA_CALL, &deactData,
                              sizeof(deactData), socket_id);
            break;
        case 8:
            LOGI("Debug port: Dial Call");
            dialData.clir = 0;
            dialData.address = args[1];
            issueLocalRequest(RIL_REQUEST_DIAL, &dialData, sizeof(dialData), socket_id);
            break;
        case 9:
            LOGI("Debug port: Answer Call");
            issueLocalRequest(RIL_REQUEST_ANSWER, NULL, 0, socket_id);
            break;
        case 10:
            LOGI("Debug port: End Call");
            issueLocalRequest(RIL_REQUEST_HANGUP, &hangupData,
                              sizeof(hangupData), socket_id);
            break;
        default:
            LOGE ("Invalid request");
            break;
    }
    freeDebugCallbackArgs(number, args);
    close(acceptFD);
}


static void rpcCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    RecordStream *p_rs;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    LOGD("$$$$$$$$$$$$rpcCallback");
    s_fdRpcCommand = accept(s_fdRpc, (sockaddr *) &peeraddr, &socklen);
    if (s_fdRpcCommand < 0 ) {
        LOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(&s_rpc_event);
	      return;
    }
    ret = fcntl(s_fdRpcCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }

    LOGI("libril: new connection");


    ril_event_set (&s_rpc_command_event, s_fdRpcCommand, 1,
        processRpcCommandsCallback, NULL);

    rilEventAddWakeup (&s_rpc_command_event);

    //onNewCommandConnect();
}

static void userTimerCallback (int fd, short flags, void *param) {
    UserCallbackInfo *p_info;

    pthread_mutex_lock(&s_wakeTimeoutMutex);
    p_info = (UserCallbackInfo *) param;

#ifdef ADD_MTK_REQUEST_URC
    if (p_info->cid > -1)
    {
        RequestInfoProxy * proxy = NULL;
        int proxyId;
        const char* proxyName = NULL;
        int queueSize = 0;

        proxy = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));

        assert(proxy);

        proxy->p_next = NULL;

        proxy->pRI = NULL;

        proxy->pUCI = p_info;

        proxyId = p_info->cid;

        proxyName = proxyIdToString(proxyId);

        /* Save dispatched proxy in RequestInfo */
        RLOGI("userTimerCallback enqueueProxyList &s_Proxy[proxyId]=%d, proxyId=%d", &s_Proxy[proxyId], proxyId);
        queueSize = enqueueProxyList(&s_Proxy[proxyId], proxy);

        if (0 != queueSize)
        {
            RLOGD("Request timed callback to %s is busy. total:%d requests pending",
                    proxyName,queueSize+1);
        }
    }
    else
    {
        p_info->p_callback(p_info->userParam);
    }
#else
    p_info->p_callback(p_info->userParam);
#endif /* ADD_MTK_REQUEST_URC */

    // FIXME generalize this...there should be a cancel mechanism
    if (s_last_wake_timeout_info != NULL && s_last_wake_timeout_info == p_info) {
        s_last_wake_timeout_info = NULL;
    }

    if (p_info->cid < 0)
    {
        free(p_info);
    }
    pthread_mutex_unlock(&s_wakeTimeoutMutex);
}


/* atci start */

RequestInfoProxy* createAtciRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, int* pId) {
    RequestInfoProxy * pRequest = NULL;
    status_t status;
    int request = RIL_REQUEST_OEM_HOOK_RAW;
    int32_t token;
    int proxyId = -1;

    pRequest = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
    assert(pRequest);

    pRequest->p = new Parcel();

    buflen++;
    pRequest->p_next = NULL;

    pRequest->pRI = pRI;
    pRequest->p->writeInt32(buflen);
    pRequest->p->write((void*) buffer, (size_t) buflen);
    pRequest->p->setDataPosition(0);

    proxyId = s_commands[request].proxyId;

    /* Save dispatched proxy in RequestInfo */
    pRequest->pRI->cid = (RILChannelId) proxyId;

    (*pId) = proxyId;

    return pRequest;
}


void enqueueAtciRequest(RequestInfo* pRI, void *buffer, size_t buflen) {
    RequestInfoProxy * proxy = NULL;
    int proxyId;
    const char* proxyName = NULL;
    int queueSize = 0;

    proxy = createAtciRequestInfoProxy(pRI, buffer, buflen, &proxyId);

    proxyName = ::proxyIdToString(proxyId);

    /* Save dispatched proxy in RequestInfo */
    queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

    if (0 != queueSize)
    {
        RLOGD("Request timed callback to %s is busy. total:%d requests pending",
                proxyName,queueSize+1);
    }
}


static void processAtciCommandsCallback(int fd, short flags, void *param) {
    LOGD("[ATCI]processAtciCommandsCallback");

    int ret;
    int request = RIL_REQUEST_OEM_HOOK_RAW;
    int recvLen = -1;
    char buffer[MAX_DATA_SIZE] = {0};

    while(recvLen == -1) {
        recvLen = recv(s_fdAtciCommand, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            RLOGE("[ATCI] fail to receive data from ril-atci socket. errno = %d", errno);
            if(errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
    }

    LOGD("[ATCI] data receive from atci is %s, data length is %d", buffer, recvLen);

    if (strcmp(buffer, "DISC") == 0 || recvLen <= 0) { //recLen == 0 means the client is disconnected.
        int sendLen = 0;

        if(recvLen != -1) {
            sendLen = send(s_fdAtciCommand, buffer, recvLen, 0);
            if (sendLen != recvLen) {
                RLOGE("[ATCI] lose data when send disc cfm to atci. errno = %d", errno);
            }
        } else {
            RLOGE("[ATCI] close ATCI due to error = %d", errno);
        }

        ril_event_del(&s_ATCIcommand_event);
        //rilEventAddWakeup (&s_ATCIlisten_event);
        close(s_fdAtciCommand);
        s_fdAtciCommand = -1;
    } else {
        RequestInfo *pRI;
        pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));
        pRI->token = 0xffffffff;        // token is not used in this context

        pRI->pCI = &(s_commands[request]);
		pRI->owner = 0;

	    ret = pthread_mutex_lock(&s_pendingRequestsMutex);
	    assert (ret == 0);

	    pRI->p_next = s_pendingRequests;
	    s_pendingRequests = pRI;

	    ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
	    assert (ret == 0);

        LOGD("C[locl]> %s", requestToString(request));

        enqueueAtciRequest(pRI, buffer, recvLen);
    }
}


static void atci_Callback (int fd, short flags, void *param)
{
    LOGD("[ATCI] enter atci_Callback");
    int ret;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    assert (s_fdAtciCommand< 0);
    assert (fd == s_fdAtciListen);

    s_fdAtciCommand = accept(s_fdAtciListen, (sockaddr *) &peeraddr, &socklen);

    if (s_fdAtciCommand < 0 ) {
        RLOGE("[ATCI] Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(&s_ATCIlisten_event);
        s_fdAtciCommand = -1;
        return;
    }

    LOGD("[ATCI] accept");

    ret = fcntl(s_fdAtciCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("[ATCI] Error setting O_NONBLOCK errno:%d", errno);
    }

    LOGD("[ATCI] librilmtk: new rild-atci connection");


    ril_event_set (&s_ATCIcommand_event, s_fdAtciCommand, true,
                   processAtciCommandsCallback, NULL);

    rilEventAddWakeup (&s_ATCIcommand_event);
}
/* atci end */


static void *
eventLoop(void *param) {
    int ret;
    int filedes[2];

    ril_event_init();

    pthread_mutex_lock(&s_startupMutex);

    s_started = 1;
    pthread_cond_broadcast(&s_startupCond);

    pthread_mutex_unlock(&s_startupMutex);

    ret = pipe(filedes);

    if (ret < 0) {
        LOGE("Error in pipe() errno:%d", errno);
        return NULL;
    }

    s_fdWakeupRead = filedes[0];
    s_fdWakeupWrite = filedes[1];

    fcntl(s_fdWakeupRead, F_SETFL, O_NONBLOCK);

    ril_event_set (&s_wakeupfd_event, s_fdWakeupRead, true,
                processWakeupCallback, NULL);

    rilEventAddWakeup (&s_wakeupfd_event);

    // Only returns on error
    ril_event_loop();
    LOGE ("error in event_loop_base errno:%d", errno);

    /*
    // kill self to restart on error
    kill(0, SIGKILL);
    */
    /*
    In current project, rild is a child process created by statusd. So we must
    not use kill(0, SIGKILL) cause it can kill parent process statusd and other
    children process as flashlessd etc as well.
    */
    killRild();

    return NULL;
}

extern "C" void
RIL_startEventLoop(void) {
    int ret;
    pthread_attr_t attr;

    // M: For multi channel support
    RIL_startRILProxys();
    /* spin up eventLoop thread and wait for it to get started */
    s_started = 0;
    pthread_mutex_lock(&s_startupMutex);

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_dispatch, &attr, eventLoop, NULL);

    while (s_started == 0) {
        pthread_cond_wait(&s_startupCond, &s_startupMutex);
    }

    pthread_mutex_unlock(&s_startupMutex);

    if (ret < 0) {
        LOGE("Failed to create dispatch thread errno:%d", errno);
        return;
    }
}

// Used for testing purpose only.
extern "C" void RIL_setcallbacks (const RIL_RadioFunctions *callbacks) {
    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));
}

static void startListen(RIL_SOCKET_ID socket_id, SocketListenParam* socket_listen_p) {
    int fdListen = -1;
    int ret;
    char socket_name[10];

    memset(socket_name, 0, sizeof(char)*10);

    switch(socket_id) {
        case RIL_SOCKET_1:
            strncpy(socket_name, RIL_getRilSocketName(), 9);
            break;
    #if (SIM_COUNT >= 2)
        case RIL_SOCKET_2:
            strncpy(socket_name, SOCKET2_NAME_RIL, 9);
            break;
    #endif
    #if (SIM_COUNT >= 3)
        case RIL_SOCKET_3:
            strncpy(socket_name, SOCKET3_NAME_RIL, 9);
            break;
    #endif
    #if (SIM_COUNT >= 4)
        case RIL_SOCKET_4:
            strncpy(socket_name, SOCKET4_NAME_RIL, 9);
            break;
    #endif
        default:
            RLOGE("Socket id is wrong!!");
            return;
    }

    RLOGI("Start to listen %s", rilSocketIdToString(socket_id));

    fdListen = android_get_control_socket(socket_name);
    if (fdListen < 0) {
        RLOGE("Failed to get socket %s", socket_name);
        exit(-1);
    }

    ret = listen(fdListen, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on control socket '%d': %s",
             fdListen, strerror(errno));
        exit(-1);
    }
    socket_listen_p->fdListen = fdListen;

    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (socket_listen_p->listen_event, fdListen, false,
                listenCallback, socket_listen_p);

    rilEventAddWakeup (socket_listen_p->listen_event);
}



extern "C" void
RIL_register (const RIL_RadioFunctions *callbacks) {
    int ret;
    int flags;
    RLOGI("SIM_COUNT: %d", SIM_COUNT);
    LOGD("vialibril suffix Version: %s", VIA_SUFFIX_VERSION);
    if (callbacks == NULL) {
        LOGE("RIL_register: RIL_RadioFunctions * null");
        return;
    }
    if (callbacks->version < RIL_VERSION_MIN) {
        LOGE("RIL_register: version %d is to old, min version is %d",
             callbacks->version, RIL_VERSION_MIN);
        return;
    }
    if (callbacks->version > RIL_VERSION) {
        LOGE("RIL_register: version %d is too new, max version is %d",
             callbacks->version, RIL_VERSION);
        return;
    }
    LOGE("RIL_register: RIL version %d", callbacks->version);

    if (s_registerCalled > 0) {
        LOGE("RIL_register has been called more than once. "
                "Subsequent call ignored");
        return;
    }

    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

    /* Initialize socket1 parameters */
    s_ril_param_socket = {
                        RIL_SOCKET_1,             /* socket_id */
                        -1,                       /* fdListen */
                        -1,                       /* fdCommand */
                        PHONE_PROCESS,            /* processName */
                        &s_commands_event,        /* commands_event */
                        &s_listen_event,          /* listen_event */
                        processCommandsCallback,  /* processCommandsCallback */
                        NULL                      /* p_rs */
                        };

#if (SIM_COUNT >= 2)
    s_ril_param_socket2 = {
                        RIL_SOCKET_2,               /* socket_id */
                        -1,                         /* fdListen */
                        -1,                         /* fdCommand */
                        PHONE_PROCESS,              /* processName */
                        &s_commands_event_socket2,  /* commands_event */
                        &s_listen_event_socket2,    /* listen_event */
                        processCommandsCallback,    /* processCommandsCallback */
                        NULL                        /* p_rs */
                        };
#endif

#if (SIM_COUNT >= 3)
    s_ril_param_socket3 = {
                        RIL_SOCKET_3,               /* socket_id */
                        -1,                         /* fdListen */
                        -1,                         /* fdCommand */
                        PHONE_PROCESS,              /* processName */
                        &s_commands_event_socket3,  /* commands_event */
                        &s_listen_event_socket3,    /* listen_event */
                        processCommandsCallback,    /* processCommandsCallback */
                        NULL                        /* p_rs */
                        };
#endif

#if (SIM_COUNT >= 4)
    s_ril_param_socket4 = {
                        RIL_SOCKET_4,               /* socket_id */
                        -1,                         /* fdListen */
                        -1,                         /* fdCommand */
                        PHONE_PROCESS,              /* processName */
                        &s_commands_event_socket4,  /* commands_event */
                        &s_listen_event_socket4,    /* listen_event */
                        processCommandsCallback,    /* processCommandsCallback */
                        NULL                        /* p_rs */
                        };
#endif


    s_registerCalled = 1;

    // Little self-check

    for (int i = 0; i < (int)NUM_ELEMS(s_commands); i++) {
    LOGD("RIL_register: i=%d, requestNumber is %d, proxyId is %d", i, s_commands[i].requestNumber, s_commands[i].proxyId);
        assert(i == s_commands[i].requestNumber);
    }

    for (int i = 0; i < (int)NUM_ELEMS(s_unsolResponses); i++) {
        assert(i + RIL_UNSOL_RESPONSE_BASE
                == s_unsolResponses[i].requestNumber);
    }

    // New rild impl calls RIL_startEventLoop() first
    // old standalone impl wants it here.

    if (s_started == 0) {
        RIL_startEventLoop();
    }

    // start listen socket
    startListen(RIL_SOCKET_1, &s_ril_param_socket);

#if (SIM_COUNT >= 2)
    // start listen socket2
    startListen(RIL_SOCKET_2, &s_ril_param_socket2);
#endif /* (SIM_COUNT == 2) */

#if (SIM_COUNT >= 3)
    // start listen socket3
    startListen(RIL_SOCKET_3, &s_ril_param_socket3);
#endif /* (SIM_COUNT == 3) */

#if (SIM_COUNT >= 4)
    // start listen socket4
    startListen(RIL_SOCKET_4, &s_ril_param_socket4);
#endif /* (SIM_COUNT == 4) */


#if 1
    // start debug interface socket

    char *inst = NULL;
    if (strlen(RIL_getRilSocketName()) >= strlen(SOCKET_NAME_RIL)) {
        inst = RIL_getRilSocketName() + strlen(SOCKET_NAME_RIL);
    }

    char rildebug[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_DEBUG;
    if (inst != NULL) {
        strncat(rildebug, inst, MAX_DEBUG_SOCKET_NAME_LENGTH);
    }

    s_fdDebug = android_get_control_socket(SOCKET_NAME_RIL_DEBUG);
    if (s_fdDebug < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
        exit(-1);
    }

    ret = listen(s_fdDebug, 4);

    if (ret < 0) {
        LOGE("Failed to listen on ril debug socket '%d': %s",
             s_fdDebug, strerror(errno));
        exit(-1);
    }

    ril_event_set (&s_debug_event, s_fdDebug, true,
                debugCallback, NULL);

    rilEventAddWakeup (&s_debug_event);
#endif
    LOGD("RIL_register: before get control socket RPC");
    s_fdRpc = android_get_control_socket(SOCKET_NAME_RPC);
    if (s_fdRpc < 0) {
        LOGE("Failed to get socket '" SOCKET_NAME_RPC "'");
        exit(-1);
    }
    ret = listen(s_fdRpc, 4);
    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s",
             s_fdRpc, strerror(errno));
        exit(-1);
    }



    LOGD("persistent connect! ril_event_set true");
    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_rpc_event, s_fdRpc, false,
                rpcCallback, NULL);

    rilEventAddWakeup (&s_rpc_event);

	// start ctclient socket
    s_fdCTClientListen = android_get_control_socket(SOCKET_NAME_RIL_CTCLIENT);
    if (s_fdCTClientListen < 0)
    {
        LOGE("CP_COMM: Failed to get socket %s", SOCKET_NAME_RIL_CTCLIENT);
        exit(-1);
    }

    ret = listen(s_fdCTClientListen, 4);

    if (ret < 0) {
        LOGE("CP_COMM: Failed to listen on control socket '%d': %s",
             s_fdCTClientListen, strerror(errno));
        exit(-1);
    }


    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_ctclient_listen_event, s_fdCTClientListen, false,
                CTClientListenCallback, NULL);

    rilEventAddWakeup (&s_ctclient_listen_event);


    /* atci start */
    s_fdAtciListen = android_get_control_socket(SOCKET_NAME_ATCI_C2K);
    if (s_fdAtciListen < 0) {
        LOGE("Failed to get atci socket");
        exit(-1);
    }
    ret = listen(s_fdAtciListen, 4);

    if (ret < 0) {
        LOGE("Failed to listen on ATCI socket '%d': %s",
             s_fdAtciListen, strerror(errno));
        exit(-1);
    }
    ril_event_set (&s_ATCIlisten_event, s_fdAtciListen, true,
                atci_Callback, NULL);

    rilEventAddWakeup (&s_ATCIlisten_event);
    /* atci end */

}

static int
checkAndDequeueRequestInfo(struct RequestInfo *pRI) {
    int ret = 0;
    /* Hook for current context
       pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo ** pendingRequestsHook = &s_pendingRequests;
    if (pRI == NULL) {
        return 0;
    }

#if (SIM_COUNT >= 2)
    if (pRI->socket_id == RIL_SOCKET_2) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket2;
        pendingRequestsHook = &s_pendingRequests_socket2;
    }
#if (SIM_COUNT >= 3)
        if (pRI->socket_id == RIL_SOCKET_3) {
            pendingRequestsMutexHook = &s_pendingRequestsMutex_socket3;
            pendingRequestsHook = &s_pendingRequests_socket3;
        }
#endif
#if (SIM_COUNT >= 4)
    if (pRI->socket_id == RIL_SOCKET_4) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex_socket4;
        pendingRequestsHook = &s_pendingRequests_socket4;
    }
#endif
#endif
    pthread_mutex_lock(pendingRequestsMutexHook);

    for(RequestInfo **ppCur = pendingRequestsHook
        ; *ppCur != NULL
        ; ppCur = &((*ppCur)->p_next)
    ) {
        if (pRI == *ppCur) {
            ret = 1;

            *ppCur = (*ppCur)->p_next;
            break;
        }
    }

    pthread_mutex_unlock(pendingRequestsMutexHook);

    return ret;
}


static int
sendRpcResponseRaw (const void *data, size_t dataSize) {
    int fd = s_fdRpcCommand;
    int ret;
    LOGD("sendRpcResponseRaw");
    if (s_fdRpcCommand < 0) {
        return -1;
    }
    LOGW("dataSize = %d", dataSize);
    if (dataSize > MAX_COMMAND_BYTES) {
        LOGE("RIL: packet larger than %u (%u)",
                MAX_COMMAND_BYTES, (unsigned int )dataSize);

        return -1;
    }

    pthread_mutex_lock(&s_rpc_writeMutex);


    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(&s_rpc_writeMutex);
        return ret;
    }

    pthread_mutex_unlock(&s_rpc_writeMutex);

    return 0;
}
extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI;
    int ret;
    int fd = s_ril_param_socket.fdCommand;
    size_t errorOffset;
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;
    int send_num;
    pRI = (RequestInfo *)t;

    socket_id = pRI->socket_id;
#if (SIM_COUNT >= 2)
    if (socket_id == RIL_SOCKET_2) {
        fd = s_ril_param_socket2.fdCommand;
    }
#if (SIM_COUNT >= 3)
        if (socket_id == RIL_SOCKET_3) {
            fd = s_ril_param_socket3.fdCommand;
        }
#endif
#if (SIM_COUNT >= 4)
    if (socket_id == RIL_SOCKET_4) {
        fd = s_ril_param_socket4.fdCommand;
    }
#endif
#endif
    RLOGD("RequestComplete, %s", rilSocketIdToString(socket_id));


    if (!checkAndDequeueRequestInfo(pRI)) {
        LOGE ("RIL_onRequestComplete: invalid RIL_Token");
        return;
    }

	if (pRI->pCI->requestNumber == RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT) {
		send_num = sendResponseForCTClient(response, responselen);
		return;
	}

    if (pRI->local > 0) {
        // Locally issued command...void only!
        // response does not go back up the command socket
        LOGD("C[locl]< %s", requestToString(pRI->pCI->requestNumber));

        goto done;
    }

    appendPrintBuf("[%04d]< %s",
        pRI->token, requestToString(pRI->pCI->requestNumber));


    /*RIL_REQUEST_OEM_HOOK_RAW for ATCI start*/
    if (s_fdAtciCommand > 0 && pRI->pCI->requestNumber == RIL_REQUEST_OEM_HOOK_RAW) {
        char* data = NULL;
        if(NULL == response)
        {
            data = "ERROR";
        }
        else
        {
            data = (char*)response;
        }
        int write_size = (int)strlen(data);
        int write_consumed = 0;
        LOGD("[ATCI] data sent to atci length(%d);data:<%s>", write_size, data);
        write_consumed = send(s_fdAtciCommand, data, write_size, 0);
        if (write_consumed != write_size) {
            LOGD("[ATCI]lose data when send to atci. errno = %s", strerror(errno));
        }
        goto done;
    }
    /*RIL_REQUEST_OEM_HOOK_RAW for ATCI end*/


    if (pRI->cancelled == 0) {
        Parcel p;

        p.writeInt32 (RESPONSE_SOLICITED);
        p.writeInt32 (pRI->token);
        errorOffset = p.dataPosition();

        p.writeInt32 (e);

        if (response != NULL) {
            // there is a response payload, no matter success or not.
            ret = pRI->pCI->responseFunction(p, response, responselen);

            /* if an error occurred, rewind and mark it */
            if (ret != 0) {
                p.setDataPosition(errorOffset);
                p.writeInt32 (ret);
            }
        }

        if (e != RIL_E_SUCCESS) {
            appendPrintBuf("%s fails by %s", printBuf, failCauseToString(e));
        }

        if (fd < 0) {
            LOGD ("RIL onRequestComplete: Command channel closed");
        }

        if (((pRI->pCI->requestNumber >= RIL_REQUEST_SET_AUDIO_PATH)
             && (pRI->pCI->requestNumber <= RIL_REQUEST_SET_MUTE_FOR_RPC)) ||
                pRI->pCI->requestNumber == RIL_REQUEST_SET_MODEM_THERMAL) {
            LOGD("RIL_onRequestComplete requestNumber = %d, Not normal, it's RPC!!!", pRI->pCI->requestNumber);
            send_num = send(s_fdRpcCommand,p.data(),p.dataSize(),MSG_DONTWAIT);
            LOGD("RIL_onRequestComplete send_num = %d", send_num);
            if (send_num < 0) {
                LOGE("RIL_onRequestComplete error = %s", strerror(errno) );
            }

            close(s_fdRpcCommand);
            ril_event_del(&s_rpc_command_event);

      /* start listening for new connections again */
            rilEventAddWakeup(&s_rpc_event);
        } else {
            sendResponse(p, socket_id);
        }
    }

done:
    free(pRI);
}


static void
grabPartialWakeLock() {
    acquire_wake_lock(PARTIAL_WAKE_LOCK, ANDROID_WAKE_LOCK_NAME);
}

static void
releaseWakeLock() {
    release_wake_lock(ANDROID_WAKE_LOCK_NAME);
}

/**
 * Timer callback to put us back to sleep before the default timeout
 */
static void
wakeTimeoutCallback (void *param) {
    // We're using "param != NULL" as a cancellation mechanism
    if (param == NULL) {
        //LOGD("wakeTimeout: releasing wake lock");

        releaseWakeLock();
    } else {
        //LOGD("wakeTimeout: releasing wake lock CANCELLED");
    }
}

static int
decodeVoiceRadioTechnology (RIL_RadioState radioState) {
    switch (radioState) {
        case RADIO_STATE_SIM_NOT_READY:
        case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
        case RADIO_STATE_SIM_READY:
            return RADIO_TECH_UMTS;

        case RADIO_STATE_RUIM_NOT_READY:
        case RADIO_STATE_RUIM_READY:
        case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
        case RADIO_STATE_NV_NOT_READY:
        case RADIO_STATE_NV_READY:
            return RADIO_TECH_1xRTT;

        default:
            ALOGD("decodeVoiceRadioTechnology: Invoked with incorrect RadioState");
            return -1;
    }
}

static int
decodeCdmaSubscriptionSource (RIL_RadioState radioState) {
    switch (radioState) {
        case RADIO_STATE_SIM_NOT_READY:
        case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
        case RADIO_STATE_SIM_READY:
        case RADIO_STATE_RUIM_NOT_READY:
        case RADIO_STATE_RUIM_READY:
        case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
            return CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM;

        case RADIO_STATE_NV_NOT_READY:
        case RADIO_STATE_NV_READY:
            return CDMA_SUBSCRIPTION_SOURCE_NV;

        default:
            LOGD("decodeCdmaSubscriptionSource: Invoked with incorrect RadioState");
            return -1;
    }
}

static int
decodeSimStatus (RIL_RadioState radioState) {
   switch (radioState) {
       case RADIO_STATE_SIM_NOT_READY:
       case RADIO_STATE_RUIM_NOT_READY:
       case RADIO_STATE_NV_NOT_READY:
       case RADIO_STATE_NV_READY:
           return -1;
       case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
       case RADIO_STATE_SIM_READY:
       case RADIO_STATE_RUIM_READY:
       case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
           return radioState;
       default:
           LOGD("decodeSimStatus: Invoked with incorrect RadioState");
           return -1;
   }
}

static bool is3gpp2(int radioTech) {
    switch (radioTech) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            return true;
        default:
            return false;
    }
}

/* If RIL sends SIM states or RUIM states, store the voice radio
 * technology and subscription source information so that they can be
 * returned when telephony framework requests them
 */
static RIL_RadioState
processRadioState(RIL_RadioState newRadioState, RIL_SOCKET_ID socket_id) {
    if((newRadioState > RADIO_STATE_UNAVAILABLE) && (newRadioState < RADIO_STATE_ON)) {
        int newVoiceRadioTech;
        int newCdmaSubscriptionSource;
        int newSimStatus;

        /* This is old RIL. Decode Subscription source and Voice Radio Technology
           from Radio State and send change notifications if there has been a change */
        newVoiceRadioTech = decodeVoiceRadioTechnology(newRadioState);
        if(newVoiceRadioTech != voiceRadioTech) {
            voiceRadioTech = newVoiceRadioTech;
            RIL_UNSOL_RESPONSE(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,
                        &voiceRadioTech, sizeof(voiceRadioTech), socket_id);
        }
        if(is3gpp2(newVoiceRadioTech)) {
            newCdmaSubscriptionSource = decodeCdmaSubscriptionSource(newRadioState);
            if(newCdmaSubscriptionSource != cdmaSubscriptionSource) {
                cdmaSubscriptionSource = newCdmaSubscriptionSource;
                RIL_UNSOL_RESPONSE(RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED,
                                       &cdmaSubscriptionSource, sizeof(cdmaSubscriptionSource), socket_id);
            }
        }
        newSimStatus = decodeSimStatus(newRadioState);
        if(newSimStatus != simRuimStatus) {
            simRuimStatus = newSimStatus;
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, socket_id);
        }

        /* Send RADIO_ON to telephony */
        newRadioState = RADIO_STATE_ON;
    }

    return newRadioState;
}

#if defined(ANDROID_MULTI_SIM)
extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id)
#else
extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
                                size_t datalen)
#endif
{
    int unsolResponseIndex;
    int ret;
    int64_t timeReceived = 0;
    bool shouldScheduleTimeout = false;
    RIL_RadioState newState;
    RIL_SOCKET_ID soc_id = RIL_SOCKET_1;

#if defined(ANDROID_MULTI_SIM)
    soc_id = socket_id;
#endif

    if (s_registerCalled == 0) {
        // Ignore RIL_onUnsolicitedResponse before RIL_register
        LOGW("RIL_onUnsolicitedResponse called before RIL_register");
        return;
    }

    unsolResponseIndex = unsolResponse - RIL_UNSOL_RESPONSE_BASE;

    if ((unsolResponseIndex < 0)
        || (unsolResponseIndex >= (int32_t)NUM_ELEMS(s_unsolResponses))) {
        LOGE("unsupported unsolicited response code %d", unsolResponse);
        return;
    }

	if (unsolResponse == RIL_UNSOL_FOR_CTCLIENT) {
		LOGD("sendResponseForCTClient for RIL_UNSOL_FOR_CTCLIENT");
		sendResponseForCTClient(data, datalen);
		return;
	}

    // Grab a wake lock if needed for this reponse,
    // as we exit we'll either release it immediately
    // or set a timer to release it later.
    switch (s_unsolResponses[unsolResponseIndex].wakeType) {
        case WAKE_PARTIAL:
            LOGD("RIL_onUnsolicitedResponse call grabPartialWakeLock");
            grabPartialWakeLock();
            shouldScheduleTimeout = true;
        break;

        case DONT_WAKE:
        default:
            // No wake lock is grabed so don't set timeout
            shouldScheduleTimeout = false;
            break;
    }

    // Mark the time this was received, doing this
    // after grabing the wakelock incase getting
    // the elapsedRealTime might cause us to goto
    // sleep.
    if (unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {
        timeReceived = elapsedRealtime();
    }

    appendPrintBuf("[UNSL]< %s", requestToString(unsolResponse));

    Parcel p;

    p.writeInt32 (RESPONSE_UNSOLICITED);
    #ifdef ADD_MTK_REQUEST_URC
    switch(unsolResponse) {
        case RIL_UNSOL_PHB_READY_NOTIFICATION:
            unsolResponse = 3002;
            break;
/*        case RIL_UNSOL_SPEECH_INFO:
            unsolResponse = 3006;
            break;*/
        case RIL_UNSOL_SIM_INSERTED_STATUS:
            unsolResponse = 3003;
            break;
        case RIL_UNSOL_SMS_READY_NOTIFICATION:
            unsolResponse = 3006;
            break;
        case RIL_UNSOL_CDMA_CALL_ACCEPTED:
            unsolResponse = 5000;
            break;
        case RIL_UNSOL_UTK_SESSION_END:
            unsolResponse = 5001;
            break;
        case RIL_UNSOL_UTK_PROACTIVE_COMMAND:
            unsolResponse = 5002;
            break;
        case RIL_UNSOL_UTK_EVENT_NOTIFY:
            unsolResponse = 5003;
            break;
        case RIL_UNSOL_VIA_GPS_EVENT:
            unsolResponse = 5004;
            break;
        case RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE:
            unsolResponse = 5005;
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED:
            unsolResponse = 5006;
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        case RIL_UNSOL_VIA_INVALID_SIM_DETECTED:
            unsolResponse = 5007;
            break;
        case RIL_UNSOL_ENG_MODE_NETWORK_INFO:
            unsolResponse = 5008;
            break;
        /// M: [C2K] for ps type changed. @{
        case RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED:
            unsolResponse = 5009;
            break;
        case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED:
            unsolResponse = 3015;
            break;
        /// @}
		case RIL_UNSOL_CDMA_IMSI_READY:
			unsolResponse = 5011;
			break;
        /// M: [C2K] DSDA support. @{
        case RIL_UNSOL_DATA_ALLOWED:
            unsolResponse = 3046;
            break;
        /// @}

        case RIL_UNSOL_CDMA_CARD_TYPE:
            unsolResponse = 3063;
            break;

        /// M: [C2K] IRAT feature. @{
        case RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE:
        	unsolResponse = 3060;
        	break;
        /// @}
    
        case RIL_UNSOL_MD_STATE_CHANGE:
            unsolResponse = 3053;
            break;

        /// M: [C2K] HD Voice icon feature. @{
        case RIL_UNSOL_SPEECH_CODEC_INFO:
            unsolResponse = 3052;
            break;
        /// @}

        /// M: Notify RILJ that the AT+EUSIM was received. @{
        case RIL_UNSOL_CDMA_SIGNAL_FADE:
            unsolResponse = 5012;
            break;
        /// @}

        /// M: Notify RILJ that call fade happened.  @{
        case RIL_UNSOL_CDMA_TONE_SIGNALS:
            unsolResponse = 5013;
            break;
        /// @}

        /// M: Notify RILJ that network information.  @{
        case RIL_UNSOL_NETWORK_INFO:
          unsolResponse = 3001;
          break;
        /// @}

/*        case RIL_UNSOL_CDMA_SIM_PLUG_IN:
            unsolResponse = 3028;
            break;
        case RIL_UNSOL_CDMA_SIM_PLUG_OUT:
            unsolResponse = 3027;
            break;
        case RIL_UNSOL_GSM_SIM_PLUG_IN:
            unsolResponse = 5010;
            break;*/

        case RIL_UNSOL_NETWORK_EXIST:
          unsolResponse = 5014;
          break;

        default:
            break;
    }
    #endif
    p.writeInt32 (unsolResponse);

    ret = s_unsolResponses[unsolResponseIndex]
                .responseFunction(p, data, datalen);
    if (ret != 0) {
        LOGE("ResponseIndex %d: error\n", unsolResponseIndex);
        // Problem with the response. Don't continue;
        goto error_exit;
    }

    // some things get more payload
    switch(unsolResponse) {
        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
            newState = processRadioState(CALL_ONSTATEREQUEST(soc_id), soc_id);
            p.writeInt32(newState);
            appendPrintBuf("%s {%s}", printBuf,
                    radioStateToString(CALL_ONSTATEREQUEST(soc_id)));
        break;


        case RIL_UNSOL_NITZ_TIME_RECEIVED:
            // Store the time that this was received so the
            // handler of this message can account for
            // the time it takes to arrive and process. In
            // particular the system has been known to sleep
            // before this message can be processed.
            p.writeInt64(timeReceived);
        break;
    }

    RLOGI("%s UNSOLICITED: %s length:%d", rilSocketIdToString(soc_id), requestToString(unsolResponse), p.dataSize());
    ret = sendResponse(p, soc_id);
    if (ret != 0 && unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {

        // Unfortunately, NITZ time is not poll/update like everything
        // else in the system. So, if the upstream client isn't connected,
        // keep a copy of the last NITZ response (with receive time noted
        // above) around so we can deliver it when it is connected

        if (s_lastNITZTimeData != NULL) {
            free (s_lastNITZTimeData);
            s_lastNITZTimeData = NULL;
        }

        s_lastNITZTimeData = malloc(p.dataSize());
        s_lastNITZTimeDataSize = p.dataSize();
        memcpy(s_lastNITZTimeData, p.data(), p.dataSize());
    }

    // For now, we automatically go back to sleep after TIMEVAL_WAKE_TIMEOUT
    // FIXME The java code should handshake here to release wake lock

    if (shouldScheduleTimeout) {
        // Cancel the previous request
        if (s_last_wake_timeout_info != NULL) {
            s_last_wake_timeout_info->userParam = (void *)1;
        }

        s_last_wake_timeout_info
            = internalRequestTimedCallback(wakeTimeoutCallback, NULL,
                                            &TIMEVAL_WAKE_TIMEOUT);
    }

    // Normal exit
    return;

error_exit:
    if (shouldScheduleTimeout) {
        LOGD("RIL_onUnsolicitedResponse call releaseWakeLock");
        releaseWakeLock();
    }
}

/** FIXME generalize this if you track UserCAllbackInfo, clear it
    when the callback occurs
*/
static UserCallbackInfo *
internalRequestTimedCallback (RIL_TimedCallback callback, void *param,
                                const struct timeval *relativeTime)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) malloc (sizeof(UserCallbackInfo));

    p_info->p_callback = callback;
    p_info->userParam = param;
    // For multi channel support
#ifdef ADD_MTK_REQUEST_URC
    p_info->cid = (RILChannelId)-1;
#endif /* ADD_MTK_REQUEST_URC */

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}


extern "C" void
RIL_requestTimedCallback (RIL_TimedCallback callback, void *param,
                                const struct timeval *relativeTime) {
    internalRequestTimedCallback (callback, param, relativeTime);
}

const char *
failCauseToString(RIL_Errno e) {
    switch(e) {
        case RIL_E_SUCCESS: return "E_SUCCESS";
        case RIL_E_RADIO_NOT_AVAILABLE: return "E_RADIO_NOT_AVAILABLE";
        case RIL_E_GENERIC_FAILURE: return "E_GENERIC_FAILURE";
        case RIL_E_PASSWORD_INCORRECT: return "E_PASSWORD_INCORRECT";
        case RIL_E_SIM_PIN2: return "E_SIM_PIN2";
        case RIL_E_SIM_PUK2: return "E_SIM_PUK2";
        case RIL_E_REQUEST_NOT_SUPPORTED: return "E_REQUEST_NOT_SUPPORTED";
        case RIL_E_CANCELLED: return "E_CANCELLED";
        case RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL: return "E_OP_NOT_ALLOWED_DURING_VOICE_CALL";
        case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW: return "E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
        case RIL_E_SMS_SEND_FAIL_RETRY: return "E_SMS_SEND_FAIL_RETRY";
        case RIL_E_SIM_ABSENT:return "E_SIM_ABSENT";
        case RIL_E_ILLEGAL_SIM_OR_ME:return "E_ILLEGAL_SIM_OR_ME";
#ifdef FEATURE_MULTIMODE_ANDROID
        case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:return "E_SUBSCRIPTION_NOT_AVAILABLE";
        case RIL_E_MODE_NOT_SUPPORTED:return "E_MODE_NOT_SUPPORTED";
#endif
        default: return "<unknown error>";
    }
}

const char *
radioStateToString(RIL_RadioState s) {
    switch(s) {
        case RADIO_STATE_OFF: return "RADIO_OFF";
        case RADIO_STATE_UNAVAILABLE: return "RADIO_UNAVAILABLE";
        case RADIO_STATE_SIM_NOT_READY: return "RADIO_SIM_NOT_READY";
        case RADIO_STATE_SIM_LOCKED_OR_ABSENT: return "RADIO_SIM_LOCKED_OR_ABSENT";
        case RADIO_STATE_SIM_READY: return "RADIO_SIM_READY";
        case RADIO_STATE_RUIM_NOT_READY:return"RADIO_RUIM_NOT_READY";
        case RADIO_STATE_RUIM_READY:return"RADIO_RUIM_READY";
        case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:return"RADIO_RUIM_LOCKED_OR_ABSENT";
        case RADIO_STATE_NV_NOT_READY:return"RADIO_NV_NOT_READY";
        case RADIO_STATE_NV_READY:return"RADIO_NV_READY";
        case RADIO_STATE_ON: return "RADIO_ON";
        #ifdef ADD_MTK_REQUEST_URC
        case RADIO_STATE_OFF_CARD_SWITCH:return"RADIO_STATE_OFF_CARD_SWITCH";
        #endif
        default: return "<unknown state>";
    }
}

const char *
callStateToString(RIL_CallState s) {
    switch(s) {
        case RIL_CALL_ACTIVE : return "ACTIVE";
        case RIL_CALL_HOLDING: return "HOLDING";
        case RIL_CALL_DIALING: return "DIALING";
        case RIL_CALL_ALERTING: return "ALERTING";
        case RIL_CALL_INCOMING: return "INCOMING";
        case RIL_CALL_WAITING: return "WAITING";
        default: return "<unknown state>";
    }
}

const char *
requestToString(int request) {
/*
 cat libs/telephony/ril_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),[^,]+,([^}]+).+/case RIL_\1: return "\1";/'


 cat libs/telephony/ril_unsol_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),([^}]+).+/case RIL_\1: return "\1";/'

*/
    switch(request) {
        case RIL_REQUEST_GET_SIM_STATUS: return "GET_SIM_STATUS";
        case RIL_REQUEST_ENTER_SIM_PIN: return "ENTER_SIM_PIN";
        case RIL_REQUEST_ENTER_SIM_PUK: return "ENTER_SIM_PUK";
        case RIL_REQUEST_ENTER_SIM_PIN2: return "ENTER_SIM_PIN2";
        case RIL_REQUEST_ENTER_SIM_PUK2: return "ENTER_SIM_PUK2";
        case RIL_REQUEST_CHANGE_SIM_PIN: return "CHANGE_SIM_PIN";
        case RIL_REQUEST_CHANGE_SIM_PIN2: return "CHANGE_SIM_PIN2";
        case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION: return "ENTER_NETWORK_DEPERSONALIZATION";
        case RIL_REQUEST_GET_CURRENT_CALLS: return "GET_CURRENT_CALLS";
        case RIL_REQUEST_DIAL: return "DIAL";
        case RIL_REQUEST_GET_IMSI: return "GET_IMSI";
        case RIL_REQUEST_HANGUP: return "HANGUP";
        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND: return "HANGUP_WAITING_OR_BACKGROUND";
        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
        case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE: return "SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
        case RIL_REQUEST_CONFERENCE: return "CONFERENCE";
        case RIL_REQUEST_UDUB: return "UDUB";
        case RIL_REQUEST_LAST_CALL_FAIL_CAUSE: return "LAST_CALL_FAIL_CAUSE";
        case RIL_REQUEST_SIGNAL_STRENGTH: return "SIGNAL_STRENGTH";
        case RIL_REQUEST_VOICE_REGISTRATION_STATE: return "VOICE_REGISTRATION_STATE";
        case RIL_REQUEST_DATA_REGISTRATION_STATE: return "DATA_REGISTRATION_STATE";
        case RIL_REQUEST_OPERATOR: return "OPERATOR";
        case RIL_REQUEST_RADIO_POWER: return "RADIO_POWER";
        case RIL_REQUEST_DTMF: return "DTMF";
        case RIL_REQUEST_SEND_SMS: return "SEND_SMS";
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE: return "SEND_SMS_EXPECT_MORE";
        case RIL_REQUEST_SETUP_DATA_CALL: return "SETUP_DATA_CALL";
        case RIL_REQUEST_SIM_IO: return "SIM_IO";
        case RIL_REQUEST_SEND_USSD: return "SEND_USSD";
        case RIL_REQUEST_CANCEL_USSD: return "CANCEL_USSD";
        case RIL_REQUEST_GET_CLIR: return "GET_CLIR";
        case RIL_REQUEST_SET_CLIR: return "SET_CLIR";
        case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS: return "QUERY_CALL_FORWARD_STATUS";
        case RIL_REQUEST_SET_CALL_FORWARD: return "SET_CALL_FORWARD";
        case RIL_REQUEST_QUERY_CALL_WAITING: return "QUERY_CALL_WAITING";
        case RIL_REQUEST_SET_CALL_WAITING: return "SET_CALL_WAITING";
        case RIL_REQUEST_SMS_ACKNOWLEDGE: return "SMS_ACKNOWLEDGE";
        case RIL_REQUEST_GET_IMEI: return "GET_IMEI";
        case RIL_REQUEST_GET_IMEISV: return "GET_IMEISV";
        case RIL_REQUEST_ANSWER: return "ANSWER";
        case RIL_REQUEST_DEACTIVATE_DATA_CALL: return "DEACTIVATE_DATA_CALL";
        case RIL_REQUEST_QUERY_FACILITY_LOCK: return "QUERY_FACILITY_LOCK";
        case RIL_REQUEST_SET_FACILITY_LOCK: return "SET_FACILITY_LOCK";
        case RIL_REQUEST_CHANGE_BARRING_PASSWORD: return "CHANGE_BARRING_PASSWORD";
        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: return "QUERY_NETWORK_SELECTION_MODE";
        case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: return "SET_NETWORK_SELECTION_AUTOMATIC";
        case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL: return "SET_NETWORK_SELECTION_MANUAL";
        case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS : return "QUERY_AVAILABLE_NETWORKS ";
        case RIL_REQUEST_DTMF_START: return "DTMF_START";
        case RIL_REQUEST_DTMF_STOP: return "DTMF_STOP";
        case RIL_REQUEST_BASEBAND_VERSION: return "BASEBAND_VERSION";
        case RIL_REQUEST_SEPARATE_CONNECTION: return "SEPARATE_CONNECTION";
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE: return "SET_PREFERRED_NETWORK_TYPE";
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE: return "GET_PREFERRED_NETWORK_TYPE";
        case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS: return "GET_NEIGHBORING_CELL_IDS";
        case RIL_REQUEST_SET_MUTE: return "SET_MUTE";
        case RIL_REQUEST_GET_MUTE: return "GET_MUTE";
        case RIL_REQUEST_QUERY_CLIP: return "QUERY_CLIP";
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE: return "LAST_DATA_CALL_FAIL_CAUSE";
        case RIL_REQUEST_DATA_CALL_LIST: return "DATA_CALL_LIST";
        case RIL_REQUEST_RESET_RADIO: return "RESET_RADIO";
        case RIL_REQUEST_OEM_HOOK_RAW: return "OEM_HOOK_RAW";
        case RIL_REQUEST_OEM_HOOK_STRINGS: return "OEM_HOOK_STRINGS";
        case RIL_REQUEST_SET_BAND_MODE: return "SET_BAND_MODE";
        case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE: return "QUERY_AVAILABLE_BAND_MODE";
        case RIL_REQUEST_STK_GET_PROFILE: return "STK_GET_PROFILE";
        case RIL_REQUEST_STK_SET_PROFILE: return "STK_SET_PROFILE";
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND: return "STK_SEND_ENVELOPE_COMMAND";
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE: return "STK_SEND_TERMINAL_RESPONSE";
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM: return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM";
        case RIL_REQUEST_SCREEN_STATE: return "SCREEN_STATE";
        case RIL_REQUEST_EXPLICIT_CALL_TRANSFER: return "EXPLICIT_CALL_TRANSFER";
        case RIL_REQUEST_SET_LOCATION_UPDATES: return "SET_LOCATION_UPDATES";
        case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:return"CDMA_SET_SUBSCRIPTION_SOURCE";
        case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:return"CDMA_SET_ROAMING_PREFERENCE";
        case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:return"CDMA_QUERY_ROAMING_PREFERENCE";
        case RIL_REQUEST_SET_TTY_MODE:return"SET_TTY_MODE";
        case RIL_REQUEST_QUERY_TTY_MODE:return"QUERY_TTY_MODE";
        case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:return"CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE";
        case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:return"CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE";
        case RIL_REQUEST_CDMA_FLASH:return"CDMA_FLASH";
        case RIL_REQUEST_CDMA_BURST_DTMF:return"CDMA_BURST_DTMF";
        case RIL_REQUEST_CDMA_SEND_SMS:return"CDMA_SEND_SMS";
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:return"CDMA_SMS_ACKNOWLEDGE";
        case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:return"GSM_GET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:return"GSM_SET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:return "CDMA_GET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:return "CDMA_SET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:return "CDMA_SMS_BROADCAST_ACTIVATION";
        case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY: return"CDMA_VALIDATE_AND_WRITE_AKEY";
        case RIL_REQUEST_CDMA_SUBSCRIPTION: return"CDMA_SUBSCRIPTION";
        case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM: return "CDMA_WRITE_SMS_TO_RUIM";
        case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: return "CDMA_DELETE_SMS_ON_RUIM";
        case RIL_REQUEST_DEVICE_IDENTITY: return "DEVICE_IDENTITY";
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: return "EXIT_EMERGENCY_CALLBACK_MODE";
        case RIL_REQUEST_GET_SMSC_ADDRESS: return "GET_SMSC_ADDRESS";
        case RIL_REQUEST_SET_SMSC_ADDRESS: return "SET_SMSC_ADDRESS";
        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS: return "REPORT_SMS_MEMORY_STATUS";
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING: return "REPORT_STK_SERVICE_IS_RUNNING";
        case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE: return "CDMA_GET_SUBSCRIPTION_SOURCE";
        case RIL_REQUEST_ISIM_AUTHENTICATION: return "ISIM_AUTHENTICATION";
        case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU: return "RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU";
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS: return "RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS";
        case RIL_REQUEST_VOICE_RADIO_TECH: return "VOICE_RADIO_TECH";
        case RIL_REQUEST_GET_CELL_INFO_LIST: return"GET_CELL_INFO_LIST";
        case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE: return"SET_UNSOL_CELL_INFO_LIST_RATE";
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN: return "RIL_REQUEST_SET_INITIAL_ATTACH_APN";
        case RIL_REQUEST_IMS_REGISTRATION_STATE: return "IMS_REGISTRATION_STATE";
        case RIL_REQUEST_IMS_SEND_SMS: return "IMS_SEND_SMS";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC: return "SIM_TRANSMIT_APDU_BASIC";
        case RIL_REQUEST_SIM_OPEN_CHANNEL: return "SIM_OPEN_CHANNEL";
        case RIL_REQUEST_SIM_CLOSE_CHANNEL: return "SIM_CLOSE_CHANNEL";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: return "SIM_TRANSMIT_APDU_CHANNEL";
        case RIL_REQUEST_SET_UICC_SUBSCRIPTION: return "SET_UICC_SUBSCRIPTION";
        case RIL_REQUEST_ALLOW_DATA: return "ALLOW_DATA";
        case RIL_REQUEST_GET_HARDWARE_CONFIG: return "GET_HARDWARE_CONFIG";
        case RIL_REQUEST_SIM_AUTHENTICATION: return "SIM_AUTHENTICATION";
        case RIL_REQUEST_GET_DC_RT_INFO: return "GET_DC_RT_INFO";
        case RIL_REQUEST_SET_DC_RT_INFO_RATE: return "SET_DC_RT_INFO_RATE";
        case RIL_REQUEST_SET_DATA_PROFILE: return "SET_DATA_PROFILE";
        case RIL_REQUEST_SHUTDOWN: return "RIL_REQUEST_SHUTDOWN";
        case RIL_REQUEST_GET_RADIO_CAPABILITY: return "RIL_REQUEST_GET_RADIO_CAPABILITY";
        case RIL_REQUEST_SET_RADIO_CAPABILITY: return "RIL_REQUEST_SET_RADIO_CAPABILITY";
#ifdef VIA_ADDED_RIL_REQUEST
        case RIL_REQUEST_GET_NITZ_TIME: return "GET_NITZ_TIME";
        case RIL_REQUEST_SET_VOICERECORD: return "SET_VOICERECORD";
        case RIL_REQUEST_SET_AUDIO_PATH: return "SET_AUDIO_PATH";
        case RIL_REQUEST_SET_GPS: return "SET_GPS";
        case RIL_REQUEST_GPS_START: return "COOL_RESET_GPS";
        case RIL_REQUEST_READ_SMS_STATUS_REPORT: return "READ_SMS_STATUS_REPORT";
        case RIL_REQUEST_GET_LOCAL_INFO: return "GET_LOCAL_INFO";
        case RIL_REQUEST_SET_ETS_DEV: return "SET_ETS_DEV";
        case RIL_REQUEST_WRITE_PB_TO_RUIM: return "WRITE_PB_TO_RUIM";
        case RIL_REQUEST_READ_PB_FROM_RUIM: return "READ_PB_FROM_RUIM";
        case RIL_REQUEST_GET_PB_SIZE_FROM_RUIM: return "GET_PB_SIZE_FROM_RUIM";
        case RIL_REQUEST_AGPS_TCP_CONNIND: return "RIL_REQUEST_AGPS_TCP_CONNIND";
        case RIL_REQUEST_AGPS_SET_MPC_IPPORT: return "RIL_REQUEST_AGPS_SET_MPC_IPPORT";
        case RIL_REQUEST_AGPS_GET_MPC_IPPORT: return "RIL_REQUEST_AGPS_GET_MPC_IPPORT";
        case RIL_REQUEST_PHONE_TEST_INFO: return "RIL_REQUEST_PHONE_TEST_INFO";
        case RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT: return "SEND_COMMAND_FOR_CTCLIENT";
        case RIL_REQUEST_UTK_REFRESH: return "UTK_REFRESH";
        case RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS: return "QUERY_SMS_AND_PHONEBOOK_STATUS";
        case RIL_REQUEST_CDMA_SEND_REAL_PDU:return "RIL_REQUEST_CDMA_SEND_REAL_PDU";
        case RIL_REQUEST_CDMA_CONVERT_REAL_PDU:return "RIL_REQUEST_CDMA_CONVERT_REAL_PDU";

#endif
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_RADIO_POWER_CARD_SWITCH: return "RIL_REQUEST_RADIO_POWER_CARD_SWITCH";
        case RIL_REQUEST_RADIO_POWEROFF: return "RIL_REQUEST_RADIO_POWEROFF";
        case RIL_REQUEST_RADIO_POWERON: return "RIL_REQUEST_RADIO_POWERON";
        case RIL_REQUEST_MODEM_POWEROFF: return "RIL_REQUEST_MODEM_POWEROFF";
        case RIL_REQUEST_MODEM_POWERON: return "RIL_REQUEST_MODEM_POWERON";
        case RIL_REQUEST_HANGUP_ALL: return "RIL_REQUEST_HANGUP_ALL";
        case RIL_REQUEST_EMERGENCY_DIAL: return "RIL_REQUEST_EMERGENCY_DIAL";
        case RIL_REQUEST_QUERY_ICCID: return "RIL_REQUEST_QUERY_ICCID";
        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS: return "RIL_REQUEST_GET_SMS_SIM_MEM_STATUS";
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO: return "RIL_REQUEST_QUERY_PHB_STORAGE_INFO";
        case RIL_REQUEST_WRITE_PHB_ENTRY: return "RIL_REQUEST_WRITE_PHB_ENTRY";
        case RIL_REQUEST_READ_PHB_ENTRY: return "RIL_REQUEST_READ_PHB_ENTRY";
        case RIL_REQUEST_QUERY_UIM_INSERTED: return "RIL_REQUEST_QUERY_UIM_INSERTED";
        case RIL_REQUEST_SWITCH_HPF: return "RIL_REQUEST_SWITCH_HPF";
        case RIL_REQUEST_SET_AVOID_SYS: return "RIL_REQUEST_SET_AVOID_SYS";
        case RIL_REQUEST_QUERY_AVOID_SYS: return "RIL_REQUEST_QUERY_AVOID_SYS";
        case RIL_REQUEST_QUERY_CDMA_NETWORK_INFO: return "RIL_REQUEST_QUERY_CDMA_NETWORK_INFO";
        case RIL_REQUEST_SET_VOICE_VOLUME: return "RIL_REQUEST_SET_VOICE_VOLUME";
        case RIL_REQUEST_PLAY_DTMF_TONE: return "RIL_REQUEST_PLAY_DTMF_TONE";
        case RIL_REQUEST_PLAY_TONE_SEQ: return "RIL_REQUEST_PLAY_TONE_SEQ";
        case RIL_REQUEST_SET_VOICE_RECORD: return "RIL_REQUEST_SET_VOICE_RECORD";
        case RIL_REQUEST_SET_MUTE_FOR_RPC: return "RIL_REQUEST_SET_MUTE_FOR_RPC";
        case RIL_REQUEST_QUERY_NETWORK_REGISTRATION: return "RIL_REQUEST_QUERY_NETWORK_REGISTRATION";
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case RIL_REQUEST_RESUME_REGISTRATION_CDMA: return "RIL_REQUEST_RESUME_REGISTRATION_CDMA";
        case RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA: return "RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA";
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        case RIL_REQUEST_WRITE_MDN: return "RIL_REQUEST_WRITE_MDN";
        case RIL_REQUEST_SET_TRM: return "RIL_REQUEST_SET_TRM";
        case RIL_REQUEST_SET_ARSI_THRESHOLD:return "RIL_REQUEST_SET_ARSI_THRESHOLD";
        case RIL_REQUEST_SET_UIM_SMS_READ: return "RIL_REQUEST_SET_UIM_SMS_READ";
        case RIL_REQUEST_NOTIFY_SIM_HOTPLUG: return "RIL_REQUEST_NOTIFY_SIM_HOTPLUG";
        case RIL_REQUEST_SIM_TRANSMIT_BASIC: return "RIL_REQUEST_SIM_TRANSMIT_BASIC";
        case RIL_REQUEST_SIM_TRANSMIT_CHANNEL:return "RIL_REQUEST_SIM_TRANSMIT_CHANNEL";
        case RIL_REQUEST_SIM_GET_ATR: return "RIL_REQUEST_SIM_GET_ATR";
        case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW: return "RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW";
        case RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE: return "RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE";
        case RIL_REQUEST_CONFIG_EVDO_MODE: return "RIL_REQUEST_CONFIG_EVDO_MODE";
        case RIL_REQUEST_QUERY_UTK_MENU_FROM_MD: return "RIL_REQUEST_QUERY_UTK_MENU_FROM_MD";
        case RIL_REQUEST_QUERY_STK_MENU_FROM_MD: return "RIL_REQUEST_QUERY_STK_MENU_FROM_MD";
        case RIL_REQUEST_SET_SPEECH_CODEC_INFO: return "SET_SPEECH_CODEC_INFO";
        /* Power on/off MD notified by thermal service, 2015/07/08 {*/
        case RIL_REQUEST_SET_MODEM_THERMAL: return "SET_MODEM_THERMAL";
        /* Power on/off MD notified by thermal service, 2015/07/08 }*/
#endif

        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED: return "UNSOL_RESPONSE_RADIO_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_NEW_SMS: return "UNSOL_RESPONSE_NEW_SMS";
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT: return "UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT";
        case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM: return "UNSOL_RESPONSE_NEW_SMS_ON_SIM";
        case RIL_UNSOL_ON_USSD: return "UNSOL_ON_USSD";
        case RIL_UNSOL_ON_USSD_REQUEST: return "UNSOL_ON_USSD_REQUEST(obsolete)";
        case RIL_UNSOL_NITZ_TIME_RECEIVED: return "UNSOL_NITZ_TIME_RECEIVED";
        case RIL_UNSOL_SIGNAL_STRENGTH: return "UNSOL_SIGNAL_STRENGTH";
        case RIL_UNSOL_STK_SESSION_END: return "UNSOL_STK_SESSION_END";
        case RIL_UNSOL_STK_PROACTIVE_COMMAND: return "UNSOL_STK_PROACTIVE_COMMAND";
        case RIL_UNSOL_STK_EVENT_NOTIFY: return "UNSOL_STK_EVENT_NOTIFY";
        case RIL_UNSOL_STK_CALL_SETUP: return "UNSOL_STK_CALL_SETUP";
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL: return "UNSOL_SIM_SMS_STORAGE_FUL";
        case RIL_UNSOL_SIM_REFRESH: return "UNSOL_SIM_REFRESH";
        case RIL_UNSOL_DATA_CALL_LIST_CHANGED: return "UNSOL_DATA_CALL_LIST_CHANGED";
        case RIL_UNSOL_CALL_RING: return "UNSOL_CALL_RING";
        case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED: return "UNSOL_RESPONSE_SIM_STATUS_CHANGED";
        case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS: return "UNSOL_NEW_CDMA_SMS";
        case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS: return "UNSOL_NEW_BROADCAST_SMS";
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: return "UNSOL_CDMA_RUIM_SMS_STORAGE_FULL";
        case RIL_UNSOL_RESTRICTED_STATE_CHANGED: return "UNSOL_RESTRICTED_STATE_CHANGED";
        case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE: return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
        case RIL_UNSOL_CDMA_CALL_WAITING: return "UNSOL_CDMA_CALL_WAITING";
        case RIL_UNSOL_CDMA_OTA_PROVISION_STATUS: return "UNSOL_CDMA_OTA_PROVISION_STATUS";
        case RIL_UNSOL_CDMA_INFO_REC: return "UNSOL_CDMA_INFO_REC";
        case RIL_UNSOL_OEM_HOOK_RAW: return "UNSOL_OEM_HOOK_RAW";
        case RIL_UNSOL_RINGBACK_TONE: return "UNSOL_RINGBACK_TONE";
        case RIL_UNSOL_RESEND_INCALL_MUTE: return "UNSOL_RESEND_INCALL_MUTE";
        case RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED: return "UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED";
        case RIL_UNSOL_CDMA_PRL_CHANGED: return "UNSOL_CDMA_PRL_CHANGED";
        case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE: return "UNSOL_EXIT_EMERGENCY_CALLBACK_MODE";
        case RIL_UNSOL_RIL_CONNECTED: return "UNSOL_RIL_CONNECTED";
        case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED: return "UNSOL_VOICE_RADIO_TECH_CHANGED";
        case RIL_UNSOL_CELL_INFO_LIST: return "UNSOL_CELL_INFO_LIST";
        case RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED: return "RESPONSE_IMS_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED: return "UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
        case RIL_UNSOL_SRVCC_STATE_NOTIFY: return "UNSOL_SRVCC_STATE_NOTIFY";
        case RIL_UNSOL_HARDWARE_CONFIG_CHANGED: return "HARDWARE_CONFIG_CHANGED";
        case RIL_UNSOL_DC_RT_INFO_CHANGED: return "UNSOL_DC_RT_INFO_CHANGED";
        case RIL_UNSOL_RADIO_CAPABILITY: return "UNSOL_RADIO_CAPABILITY";
#ifdef VIA_ADDED_RIL_REQUEST
        case RIL_UNSOL_FOR_CTCLIENT: return "UNSOL_FOR_CTCLIENT";
        case RIL_UNSOL_CDMA_CALL_ACCEPTED: return "RIL_UNSOL_CDMA_CALL_ACCEPTED";
        case RIL_UNSOL_VIA_GPS_EVENT: return "RIL_UNSOL_VIA_GPS_EVENT";
        case RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED: return "RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED";
        case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: return "RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_READ_NEW_SMS_STATUS_REPORT: return "RIL_UNSOL_RESPONSE_READ_NEW_SMS_STATUS_REPORT";
        case RIL_UNSOL_UTK_SESSION_END: return "UNSOL_UTK_SESSION_END";
        case RIL_UNSOL_UTK_PROACTIVE_COMMAND: return "UNSOL_UTK_PROACTIVE_COMMAND";
        case RIL_UNSOL_UTK_EVENT_NOTIFY: return "UNSOL_UTK_EVENT_NOTIFY";
#endif
#ifdef ADD_MTK_REQUEST_URC
        case RIL_UNSOL_SPEECH_INFO: return "RIL_UNSOL_SPEECH_INFO";
        case RIL_UNSOL_SIM_INSERTED_STATUS: return "RIL_UNSOL_SIM_INSERTED_STATUS";
        case RIL_UNSOL_SMS_READY_NOTIFICATION: return "RIL_UNSOL_SMS_READY_NOTIFICATION";
        case RIL_UNSOL_PHB_READY_NOTIFICATION: return "RIL_UNSOL_PHB_READY_NOTIFICATION";
        case RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE: return "RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE";
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED: return "RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED";
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        case RIL_UNSOL_VIA_INVALID_SIM_DETECTED: return "RIL_UNSOL_VIA_INVALID_SIM_DETECTED";
        case RIL_UNSOL_CDMA_SIM_PLUG_IN: return "RIL_UNSOL_CDMA_SIM_PLUG_IN";
        case RIL_UNSOL_CDMA_SIM_PLUG_OUT: return "RIL_UNSOL_CDMA_SIM_PLUG_OUT";
        case RIL_UNSOL_GSM_SIM_PLUG_IN: return "RIL_UNSOL_GSIM_PLUG_IN";
        case RIL_UNSOL_CDMA_CARD_TYPE: return "RIL_UNSOL_CDMA_CARD_TYPE";
        case RIL_UNSOL_DATA_ALLOWED: return "RIL_UNSOL_DATA_ALLOWED";
        case RIL_UNSOL_ENG_MODE_NETWORK_INFO: return "RIL_UNSOL_ENG_MODE_NETWORK_INFO";
        case RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE: return "RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        case RIL_UNSOL_CDMA_IMSI_READY: return "RIL_UNSOL_CDMA_IMSI_READY";
        case RIL_UNSOL_SPEECH_CODEC_INFO: return "UNSOL_SPEECH_CODEC_INFO";
        case RIL_UNSOL_CDMA_SIGNAL_FADE: return "UNSOL_CDMA_SIGNAL_FADE";
        case RIL_UNSOL_CDMA_TONE_SIGNALS: return "UNSOL_CDMA_TONE_SIGNALS";
        case RIL_UNSOL_NETWORK_EXIST: return "RIL_UNSOL_NETWORK_EXIST";
#endif
        case RIL_REQUEST_SET_MEID: return "SET_MEID";
        case RIL_REQUEST_CONFIG_MODEM_STATUS: return "RIL_REQUEST_CONFIG_MODEM_STATUS";
        //MD state change
        case RIL_UNSOL_MD_STATE_CHANGE: return "RIL_UNSOL_MD_STATE_CHANGE";
        case RIL_REQUEST_SWITCH_CARD_TYPE: return "RIL_REQUEST_SWITCH_CARD_TYPE";
        case RIL_REQUEST_ENABLE_MD3_SLEEP: return "RIL_REQUEST_ENABLE_MD3_SLEEP";
        default: return "<unknown request>";
    }
}

// For multi channel support
extern "C" RILChannelId
RIL_queryMyChannelId(RIL_Token t)
{
    LOGE("proxy RIL_queryMyChannelId channelid = %d", ((RequestInfo *) t)->cid);
    return ((RequestInfo *) t)->cid;
}

extern "C" int
RIL_queryMyProxyIdByThread()
{
    pthread_t selfThread = pthread_self();
    pthread_mutex_lock(&s_queryThreadMutex);
    int i;
    for (i=0; i< RIL_SUPPORT_CHANNELS; i++)
    {
        if (0 != pthread_equal(s_tid_proxy[i], selfThread))
        {
            pthread_mutex_unlock(&s_queryThreadMutex);
            return i;
        }
    }
    pthread_mutex_unlock(&s_queryThreadMutex);
    return -1;
}

static UserCallbackInfo *
internalRequestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                              const struct timeval *relativeTime,
                              int proxyId)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) malloc (sizeof(UserCallbackInfo));

    p_info->p_callback = callback;
    p_info->userParam = param;
    p_info->cid = (RILChannelId)proxyId;

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}

extern "C" void
RIL_requestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                          const struct timeval *relativeTime,
                          RILChannelId proxyId) {
    internalRequestProxyTimedCallback (callback, param,
                relativeTime, proxyId);
}

extern "C"
void RIL_MDStateChange(RIL_MDState state)
{
    int i = 0;
    int state_tmp = (int) state;
    LOGD("modem state: %d", state);
    RIL_onUnsolicitedResponse(RIL_UNSOL_MD_STATE_CHANGE, &state_tmp, sizeof(int));
}

} /* namespace android */
