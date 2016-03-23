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

#ifdef MTK_RIL_MD1
#define LOG_TAG "RILC"
#else
#define LOG_TAG "RIL2C"
#endif

#include <hardware_legacy/power.h>

#include <telephony/mtk_ril.h>
#include <telephony/ril_cdma_sms.h>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <telephony/record_stream.h>
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
#include <RilSapSocket.h>

#include <rilc.h>

extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);
namespace android {


static RIL_SOCKET_ID s_ril_cntx[] = {
    RIL_SOCKET_1
    , RIL_SOCKET_2
    , RIL_SOCKET_3
    , RIL_SOCKET_4
};

#define PHONE_PROCESS "radio"
#define BLUETOOTH_PROCESS "bluetooth"

#define SOCKET_NAME_RIL "rild"
#define SOCKET2_NAME_RIL "rild2"
#define SOCKET3_NAME_RIL "rild3"
#define SOCKET4_NAME_RIL "rild4"

#define SOCKET_NAME_RIL_DEBUG "rild-debug"
#define SOCKET_NAME_ATCI "rild-atci"
#define PROPERTY_ATCI_CHANNEL "lackof.atci.channel"
#define SOCKET_NAME_RIL_OEM "rild-oem"
#define SOCKET_NAME_RIL_VSIM "rild-vsim"

#define SOCKET_NAME_RIL_MD2 "rild-md2"
#define SOCKET_NAME_RIL2_MD2 "rild2-md2"
#define SOCKET_NAME_RIL_DEBUG_MD2 "rild-debug-md2"
#define SOCKET_NAME_RIL_OEM_MD2 "rild-oem-md2"
#define SOCKET_NAME_ATCI_MD2 "rild-atci-md2"
#define SOCKET_NAME_RIL_VSIM_MD2 "rild-vsim-md2"
#define SOCKET_NAME_RIL_MAL "rild-mal"
#define SOCKET_NAME_RIL_MAL_AT "rild-mal-at"
#define SOCKET_NAME_RIL_MAL_MD2 "rild-mal-md2"
#define SOCKET_NAME_RIL_MAL_AT_MD2 "rild-mal-at-md2"
#define ANDROID_WAKE_LOCK_NAME "radio-interface"


#define PROPERTY_RIL_IMPL "gsm.version.ril-impl"

#define PROPERTY_RIL_CURRENT_SHARE_MODEM    "ril.current.share_modem"

// match with constant in RIL.java
#define MAX_COMMAND_BYTES (20 * 1024)

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

// Enable verbose logging
#define VDBG 0

// Enable RILC log
#define RILC_LOG 0

//External SIM [Start]
#define VSIM_CLIENT_ID 1000
//External SIM [End]

#if RILC_LOG
    #define startRequest           sprintf(printBuf, "(")
    #define closeRequest           sprintf(printBuf, "%s)", printBuf)
    #define printRequest(token, req)           \
            RLOGD("[%04d]> %s %s", token, requestToString(req), printBuf)

    #define startResponse           sprintf(printBuf, "%s {", printBuf)
    #define closeResponse           sprintf(printBuf, "%s}", printBuf)
    #define printResponse           RLOGD("%s", printBuf)

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

extern "C"
#ifdef MTK_RIL_MD2
char rild[MAX_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_MD2;
#else
char rild[MAX_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL;
#endif
/*******************************************************************/

RIL_RadioFunctions s_callbacks = {0, NULL, NULL, NULL, NULL, NULL};
RIL_RadioFunctionsSocket s_callbacksSocket = {0, NULL, NULL, NULL, NULL, NULL};
static int s_registerCalled = 0;

static pthread_t s_tid_dispatch;
static pthread_t s_tid_reader;
static int s_started = 0;

static int s_fdDebug = -1;
static int s_fdDebug_socket2 = -1;

static int s_fdOem = -1;
static int s_fdOem_command = -1;
static int s_EAPSIMAKA_fd = -1;
static int s_SIMLOCK_fd = -1;
extern "C" int s_THERMAL_fd = -1;
/// M: For 3G VT only @{
static int s_VT_fd = -1;
/// @}

/* atci start */
static int s_fdATCI_listen = -1;
static int s_fdATCI_command = -1;
static int s_fdReserved_command = -1;
static int sentByPS = 0;
static int sentBySIM = 0;
/* atci end */

//External SIM [Start]
static pthread_t s_vsim_dispatch;
static int s_fdVsim = -1;
static int s_fdVsim_command = -1;
//External SIM [End]

static int s_fdWakeupRead;
static int s_fdWakeupWrite;

static struct ril_event s_wakeupfd_event;

static struct ril_event s_commands_event[SIM_COUNT];
static struct ril_event s_listen_event[SIM_COUNT];
static SocketListenParam s_ril_param_socket[SIM_COUNT];

static pthread_mutex_t s_last_wake_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t s_pendingRequestsMutex[SIM_COUNT] = {PTHREAD_MUTEX_INITIALIZER};
static pthread_mutex_t s_writeMutex[SIM_COUNT] = {PTHREAD_MUTEX_INITIALIZER};
static RequestInfo *s_pendingRequests[SIM_COUNT] = {NULL};

static struct ril_event s_wake_timeout_event;
static struct ril_event s_debug_event;
static struct ril_event s_oem_event;

static int s_current_3g_sim = -1; //which sim is for main PS

static struct ril_event s_mal_listen_event;
static struct ril_event s_mal_command_event;
static pthread_mutex_t s_mal_writeMutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef MTK_RIL_MD1
static RIL_Client_Type s_mal_client = {RIL_CLIENT_ID_MAL,
                            SOCKET_NAME_RIL_MAL,
                            RIL_SOCKET_1,
                            -1, -1,
                            &s_mal_listen_event, &s_mal_command_event,
                            &s_mal_writeMutex,
                            NULL};

#else
static RIL_Client_Type s_mal_client = { RIL_CLIENT_ID_MAL,
                            SOCKET_NAME_RIL_MAL_MD2,
                            RIL_SOCKET_1,
                            -1, -1,
                            &s_mal_listen_event, &s_mal_command_event,
                            &s_mal_writeMutex,
                            NULL};
#endif

static struct ril_event s_mal_at_listen_event;
static struct ril_event s_mal_at_command_event;
static pthread_mutex_t s_mal_at_writeMutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef MTK_RIL_MD1
static RIL_Client_Type s_mal_at_client = { RIL_CLIENT_ID_MAL_AT,
                                 SOCKET_NAME_RIL_MAL_AT,
                                 RIL_SOCKET_1,
                                 -1, -1,
                                 &s_mal_at_listen_event,
                                 &s_mal_at_command_event,
                                 &s_mal_at_writeMutex,
                                 NULL};
#else
static RIL_Client_Type s_mal_at_client = { RIL_CLIENT_ID_MAL_AT,
                                 SOCKET_NAME_RIL_MAL_AT_MD2,
                                 RIL_SOCKET_1,
                                 -1, -1,
                                 &s_mal_at_listen_event,
                                 &s_mal_at_command_event,
                                 &s_mal_at_writeMutex,
                                 NULL};
#endif
/* atci start */
static struct ril_event s_ATCIlisten_event;
static struct ril_event s_ATCIcommand_event;
static char simNo[PROPERTY_VALUE_MAX] = {0};
static char psNo[PROPERTY_VALUE_MAX] = {0};
/* atci end */

//External SIM [Start]
static struct ril_event s_vsim_listen_event;
static RIL_CLIENT s_vsim_client = {VSIM_CLIENT_ID, -1};
//External SIM [End]

static const struct timeval TIMEVAL_WAKE_TIMEOUT = {1,0};


static pthread_mutex_t s_startupMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_startupCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t s_dispatchMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_dispatchCond = PTHREAD_COND_INITIALIZER;

static RequestInfo *s_toDispatchHead = NULL;
static RequestInfo *s_toDispatchTail = NULL;

static UserCallbackInfo *s_last_wake_timeout_info = NULL;

static void *s_lastNITZTimeData = NULL;
static size_t s_lastNITZTimeDataSize;

static void *s_prevEmsrData[SIM_COUNT] = {NULL};
static size_t s_prevEmsrDataSize[SIM_COUNT];
static void *s_prevEcopsData[SIM_COUNT] = {NULL};
static size_t s_prevEcopsDataSize[SIM_COUNT];

#if RILC_LOG
    static char printBuf[PRINTBUF_SIZE];
#endif

/*******************************************************************/
static int sendResponse (Parcel &p, RIL_SOCKET_ID socket_id);
static int sendClientResponse(Parcel &p, RIL_Client_Type *client);

static void dispatchVoid (Parcel& p, RequestInfo *pRI);
static void dispatchString (Parcel& p, RequestInfo *pRI);
static void dispatchStrings (Parcel& p, RequestInfo *pRI);
static void dispatchInts (Parcel& p, RequestInfo *pRI);
static void dispatchDial (Parcel& p, RequestInfo *pRI);
static void dispatchSIM_IO (Parcel& p, RequestInfo *pRI);
static void dispatchSIM_IO_EX (Parcel& p, RequestInfo *pRI);
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
static void dispatchPhbEntry(Parcel &p, RequestInfo *pRI);
static void dispatchWritePhbEntryExt(Parcel &p, RequestInfo *pRI);
//[New R8 modem FD]
static void dispatchFD_Mode(Parcel &p, RequestInfo *pRI);
static int responseUssdStrings(Parcel &p, void *response, size_t responselen);
//New SIM Authentication
static void dispatchSimAuth(Parcel &p, RequestInfo *pRI);
static void dispatchNVReadItem(Parcel &p, RequestInfo *pRI);
static void dispatchNVWriteItem(Parcel &p, RequestInfo *pRI);
static void dispatchUiccSubscripton(Parcel &p, RequestInfo *pRI);
//VoLTE
static void dispatchSetupDedicateDataCall(Parcel &p, RequestInfo *pRI);
static void dispatchModifyDedicateDataCall(Parcel &p, RequestInfo *pRI);

static void dispatchSimAuthentication(Parcel &p, RequestInfo *pRI);
static void dispatchDataProfile(Parcel &p, RequestInfo *pRI);
static void dispatchRadioCapability(Parcel &p, RequestInfo *pRI);
//External SIM [Start]
static void dispatchVsimEvent(Parcel &p, RequestInfo *pRI);
static void dispatchVsimOperationEvent(Parcel &p, RequestInfo *pRI);
static int responseVsimOperationEvent(Parcel &p, void *response, size_t responselen);
//External SIM [End]
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
static int responseUnsolSetupDataCall(Parcel &p, void *response, size_t responselen);
static int responseSetupDataCall(Parcel &p, void *response, size_t responselen);
static int responseRaw(Parcel &p, void *response, size_t responselen);
static int responseSsn(Parcel &p, void *response, size_t responselen);
static int responseCrssN(Parcel &p, void *response, size_t responselen);
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
static int responsePhbEntries(Parcel &p,void *response, size_t responselen);
static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen);
static int handleSpecialRequestWithArgs(int argCount, char** args);
static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen);
static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen);
static void dispatchSmsParams(Parcel &p, RequestInfo *pRI);
static int responseSmsParams(Parcel &p, void *response, size_t responselen);
static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen);
static int responseEtwsNotification(Parcel &p, void *response, size_t responselen);

//VoLTE
static int responseSetupDedicateDataCall(Parcel &p, void *response, size_t responselen);
static int responseEpcNetworkFeatureInfo(Parcel &p, void *response, size_t responselen);

/// M: [C2K] IRAT feature. @{
static int responseIratStateChange(Parcel &p, void *response, size_t responselen);
/// @}
//MTK-START [mtk80776] WiFi Calling
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI);
static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI);
//MTK-END [mtk80776] WiFi Calling
static int responseHardwareConfig(Parcel &p, void *response, size_t responselen);
static int responseDcRtInfo(Parcel &p, void *response, size_t responselen);
static int responseRadioCapability(Parcel &p, void *response, size_t responselen);
static int responseSSData(Parcel &p, void *response, size_t responselen);
static int responseLceStatus(Parcel &p, void *response, size_t responselen);
static int responseLceData(Parcel &p, void *response, size_t responselen);
static int responseActivityData(Parcel &p, void *response, size_t responselen);

static int decodeVoiceRadioTechnology (RIL_RadioState radioState);
static int decodeCdmaSubscriptionSource (RIL_RadioState radioState);
static RIL_RadioState processRadioState(RIL_RadioState newRadioState);

static bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType);

#ifdef RIL_SHLIB
extern "C" void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id);

extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen);
#endif

static int sendResponse(Parcel &p, RIL_SOCKET_ID socket_id);

static UserCallbackInfo * internalRequestTimedCallback
    (RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);

/** Index == requestNumber */
CommandInfo s_commands[] = {
#include "ril_commands.h"
};

UnsolResponseInfo s_unsolResponses[] = {
#include "ril_unsol_commands.h"
};

#ifdef MTK_RIL
CommandInfo s_mtk_commands[] = {
#include "mtk_ril_commands.h"
//{0, NULL, NULL, RIL_SUPPORT_PROXYS}
};

static UnsolResponseInfo s_mtk_unsolResponses[] = {
#include "mtk_ril_unsol_commands.h"
};

static CommandInfo s_mtk_local_commands[] = {
#include "mtk_ril_local_commands.h"
};

static UserCallbackInfo * internalRequestProxyTimedCallback
(RIL_TimedCallback callback, void *param,
const struct timeval *relativeTime, int proxyId);

const int s_commands_size = (int32_t)NUM_ELEMS(s_commands);

const int s_mtk_commands_size = (int32_t)NUM_ELEMS(s_mtk_commands);

#endif

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
static int isMalSupported()
{
    static int is_mal_supported = -1;
    if (is_mal_supported == -1) {
        char volte_prop[PROPERTY_VALUE_MAX] = {0};
        //MAL is depended on VOLTE feature
        property_get("ro.mtk_volte_support", volte_prop, "0");
        if (!strcmp(volte_prop, "1")) {
            is_mal_supported = 1;
        } else {
            is_mal_supported = 0;
        }
    }
    return is_mal_supported;
}

#define MT_DONGLE_BOOT 98
static int isMalDongleMode()
{
  static int s_is_dongle_mode = -1;
  if (s_is_dongle_mode== -1) {
      int fd;
      size_t s;
      char boot_mode[4] = {'0'};
      fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
      if (fd < 0) {
        RLOGD("MAL-fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
        return 0;
      }
      s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
      close(fd);
      if(s <= 0) {
        RLOGD("MAL-could not read boot mode sys file");
        return 0;
      }
      boot_mode[s] = '\0';
      RLOGD("MAL-boot_mode: %s", boot_mode);
      s_is_dongle_mode = (MT_DONGLE_BOOT == atoi(boot_mode))?1:0;
   }
   return  s_is_dongle_mode;
}

extern "C"
int RIL_get3GSimInfo()
{
    if (s_current_3g_sim == -1) {
        char prop_value[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_3G_SIM, prop_value, "1");
        s_current_3g_sim = atoi(prop_value) - 1;
    }
    return s_current_3g_sim;
}

static void update3GSimInfo(int simID)
{
    RLOGD("current 3G sim set to: SIM_%d", simID+1);
    s_current_3g_sim = simID;
}

static int callForRilVersion()
{
    int version = 0;
    if (s_callbacks.version > 0) {
        version = s_callbacks.version;
        RLOGD("callForRilVersion old vendor RIL, version: %d", version);
    } else if (s_callbacksSocket.version > 0){
        version = s_callbacksSocket.version;
        RLOGD("callForRilVersion new vendor RIL, version: %d", version);
    }
    return version;
}

static const char* callForGetVersion() {
    const char* version = NULL;
    if (s_callbacks.getVersion != NULL) {
        version = s_callbacks.getVersion();
        RLOGD("callForRilVersion old vendor version, version: %s", version);
    } else if (s_callbacksSocket.getVersion != NULL){
        RLOGD("callForRilVersion new vendor version, version: %s", version);
        version = s_callbacksSocket.getVersion();
    }
    return version;
}

static void callOnRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    if (s_callbacks.onRequest != NULL) {
         /* old vendor ril */
         RLOGD("callOnRequest for old vendor ril");
         s_callbacks.onRequest(request, data, datalen, t);
    } else if (s_callbacksSocket.onRequest != NULL) {
         /* new vendor ril */
         RLOGD("callOnRequest for new vendor ril");
         s_callbacksSocket.onRequest(request, data, datalen, t, socket_id);
    }
}

RIL_RadioState callOnStateRequest(RIL_SOCKET_ID rid)
{
    RIL_RadioState radioState = RADIO_STATE_UNAVAILABLE;
    if (s_callbacks.onStateRequest != NULL) {
         /* old vendor ril */
         RLOGD("callOnStateRequest for old vendor ril");
         radioState = s_callbacks.onStateRequest();
    } else if (s_callbacksSocket.onStateRequest != NULL) {
         /* new vendor ril */
         RLOGD("callOnStateRequest for new vendor ril");
         radioState = s_callbacksSocket.onStateRequest(rid);
    }
    return radioState;
}

static char * RIL_getRilSocketName() {
    return rild;
}

extern "C"
void RIL_setRilSocketName(char * s) {
    strncpy(rild, s, MAX_SOCKET_NAME_LENGTH);
}

static char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
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

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
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
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests[socket_id];

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 1;
    pRI->token = 0xffffffff;        // token is not used in this context
    if (request < 1 || request > (int32_t)NUM_ELEMS(s_commands)) {
#ifdef MTK_RIL
        if (request >= (RIL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)))
#endif /* MTK_RIL */
        {

            LOGE("issueLocalRequest: unsupported request code %d", request);
            // FIXME this should perhaps return a response
            free(pRI);
            return;
        }
    }

#ifdef MTK_RIL
    if (request < RIL_REQUEST_VENDOR_BASE) {
        pRI->pCI = &(s_commands[request]);
    } else {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);
    }
#else
    pRI->pCI = &(s_commands[request]);
#endif /* MTK_RIL */
    pRI->socket_id = socket_id;

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    RLOGD("C[locl]> %s", requestToString(request));

    callOnRequest(request, data, len, pRI, pRI->socket_id);
}

/**
 * To be called from dispatch thread
 * Issue a single local request for specified SIM,
 * and sent back up to the command process
 */
extern "C"
void issueLocalRequestForResponse(int request, void *data, int len, RIL_SOCKET_ID socket_id) {
    RequestInfo *pRI;
    int ret;
    status_t status;

    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests[socket_id];


    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 0;
    pRI->socket_id = socket_id;

    if (request < 1 || request > (int32_t)NUM_ELEMS(s_commands)) {
#ifdef MTK_RIL
        if (request < RIL_REQUEST_VENDOR_BASE || request >= (RIL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)))
#endif /* MTK_RIL */
        {
            if(request < RIL_LOCAL_REQUEST_VENDOR_BASE || request >= (RIL_LOCAL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_local_commands)))
            {
                LOGE("issueLocalRequestForResponse: unsupported request code %d", request);
                // FIXME this should perhaps return a response
                free(pRI);
                return;
            }
        }
    }

#ifdef MTK_RIL
    if (request >= RIL_LOCAL_REQUEST_VENDOR_BASE){
        pRI->pCI = &(s_mtk_local_commands[request - RIL_LOCAL_REQUEST_VENDOR_BASE]);
    }else if(request < RIL_REQUEST_VENDOR_BASE) {
        pRI->pCI = &(s_commands[request]);
    } else {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);
    }
#else
    pRI->pCI = &(s_commands[request]);
#endif /* MTK_RIL */


    pRI->cid = RIL_CMD_PROXY_1;
    if (socket_id == RIL_SOCKET_2){
        pRI->cid = RIL_CMD2_PROXY_1;
    } else if (socket_id == RIL_SOCKET_3) {
        pRI->cid = RIL_CMD3_PROXY_1;
    } else if (socket_id == RIL_SOCKET_4) {
        pRI->cid = RIL_CMD4_PROXY_1;
    }

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    LOGD("C[locl]> %s", requestToString(request));

#ifdef MTK_RIL
    {
        enqueueLocalRequestResponse(pRI, data, len, NULL, socket_id);
    }
#else
    s_callbacks.onRequest(request, data, len, pRI);
#endif
}

static int
processCommandBuffer(void *buffer, size_t buflen, RIL_SOCKET_ID socket_id) {
    Parcel p;
    status_t status;
    int32_t request;
    int32_t token;
    RequestInfo *pRI;
    int ret;
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests[socket_id];

    p.setData((uint8_t *) buffer, buflen);

    // status checked at end
    status = p.readInt32(&request);
    status = p.readInt32 (&token);

    RLOGD("SOCKET %s REQUEST: %s length:%d", rilSocketIdToString(socket_id), requestToString(request), buflen);

    if (status != NO_ERROR) {
        RLOGE("invalid request block");
        return 0;
    }

    if (request < 1 || request >= (int32_t)NUM_ELEMS(s_commands)) {
    #ifdef MTK_RIL
        if (request > (RIL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)) ||
            (request >= (int32_t)NUM_ELEMS(s_commands) && request < RIL_REQUEST_VENDOR_BASE))
    #endif /* MTK_RIL */
        {
            Parcel pErr;
            RLOGE("unsupported request code %d token %d", request, token);
            // FIXME this should perhaps return a response
            pErr.writeInt32 (RESPONSE_SOLICITED);
            pErr.writeInt32 (token);
            pErr.writeInt32 (RIL_E_GENERIC_FAILURE);

            sendResponse(pErr, socket_id);
            return 0;
        }
    }


    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->token = token;
#ifdef MTK_RIL
    if (request >= RIL_REQUEST_VENDOR_BASE) {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);
    } else
#endif /* MTK_RIL */
    {
        pRI->pCI = &(s_commands[request]);
    }

    pRI->socket_id = socket_id;

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

/*    sLastDispatchedToken = token; */

#ifdef MTK_RIL
    {
        enqueue(pRI, buffer, buflen, NULL, socket_id);
    }
#else
    pRI->pCI->dispatchFunction(p, pRI);
#endif

    return 0;
}



static int
processClientCommandBuffer(void *buffer, size_t buflen, RIL_Client_Type *client) {
    Parcel p;
    status_t status;
    int32_t request;
    int32_t token;
    RequestInfo *pRI;
    int ret;
    RIL_SOCKET_ID socket_id;
    socket_id = client->simID;
    if (client->clientID == RIL_CLIENT_ID_MAL) {
        //always schedule to 3G SIM
        socket_id = (RIL_SOCKET_ID) (RIL_SOCKET_1+RIL_get3GSimInfo());
    }
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = &s_pendingRequests[socket_id];

    p.setData((uint8_t *) buffer, buflen);

    // status checked at end
    status = p.readInt32(&request);
    status = p.readInt32 (&token);

    RLOGD("Client %s REQUEST: %s length:%d", client->name, requestToString(request), buflen);

    if (status != NO_ERROR) {
        RLOGE("invalid request block");
        return 0;
    }

    if (request < 1 || request >= (int32_t)NUM_ELEMS(s_commands)) {
    #ifdef MTK_RIL
        if (request > (RIL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)) ||
            (request >= (int32_t)NUM_ELEMS(s_commands) && request < RIL_REQUEST_VENDOR_BASE))
    #endif /* MTK_RIL */
        {
            Parcel pErr;
            RLOGE("unsupported request code %d token %d", request, token);
            // FIXME this should perhaps return a response
            pErr.writeInt32 (RESPONSE_SOLICITED);
            pErr.writeInt32 (token);
            pErr.writeInt32 (RIL_E_GENERIC_FAILURE);
            sendClientResponse(pErr, client);
            return 0;
        }
    }

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->token = token;
#ifdef MTK_RIL
    if (request >= RIL_REQUEST_VENDOR_BASE) {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);
    } else
#endif /* MTK_RIL */
    {
        pRI->pCI = &(s_commands[request]);
    }

    pRI->socket_id = socket_id;
    pRI->client = client;

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

#ifdef MTK_RIL
    {
        enqueue(pRI, buffer, buflen, NULL, socket_id);
    }
#else
    pRI->pCI->dispatchFunction(p, pRI);
#endif

    return 0;
}

static void
invalidCommandBlock (RequestInfo *pRI) {
    RLOGE("invalid command block for token %d request %s",
                pRI->token, requestToString(pRI->pCI->requestNumber));
}

/** Callee expects NULL */
static void
dispatchVoid (Parcel& p, RequestInfo *pRI) {
    clearPrintBuf;
    printRequest(pRI->token, pRI->pCI->requestNumber);
    callOnRequest(pRI->pCI->requestNumber, NULL, 0, pRI, pRI->socket_id);
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

    callOnRequest(pRI->pCI->requestNumber, string8,
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

    callOnRequest(pRI->pCI->requestNumber, pStrings, datalen, pRI, pRI->socket_id);

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

        if (status != NO_ERROR) {
            goto invalid;
        }
   }
   removeLastChar;
   closeRequest;
   printRequest(pRI->token, pRI->pCI->requestNumber);

   callOnRequest(pRI->pCI->requestNumber, const_cast<int *>(pInts),
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

    RLOGD("dispatchSmsWrite");
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

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

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

    RLOGD("dispatchDial");
    memset (&dial, 0, sizeof(dial));

    dial.address = strdupReadString(p);

    status = p.readInt32(&t);
    dial.clir = (int)t;

    if (status != NO_ERROR || dial.address == NULL) {
        goto invalid;
    }

    if (callForRilVersion() < 3) { // Remove when partners upgrade to version 3
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

    callOnRequest(pRI->pCI->requestNumber, &dial, sizeOfDial, pRI, pRI->socket_id);

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

#if VDBG
    RLOGD("dispatchSIM_IO");
#endif
    memset (&simIO, 0, sizeof(simIO));

    // note we only check status at the end

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

    size = (callForRilVersion() < 6) ? sizeof(simIO.v5) : sizeof(simIO.v6);
    callOnRequest(pRI->pCI->requestNumber, &simIO, size, pRI, pRI->socket_id);

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
    callOnRequest(pRI->pCI->requestNumber, &simIO, size, pRI, pRI->socket_id);

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

#if VDBG
    RLOGD("dispatchSIM_APDU");
#endif
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

    callOnRequest(pRI->pCI->requestNumber, &apdu, sizeof(RIL_SIM_APDU), pRI, pRI->socket_id);

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

    RLOGD("dispatchCallForward");
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

    callOnRequest(pRI->pCI->requestNumber, &cff, sizeof(cff), pRI, pRI->socket_id);

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

    callOnRequest(pRI->pCI->requestNumber, const_cast<void *>(data), len, pRI, pRI->socket_id);

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

    RLOGD("dispatchCdmaSms");
    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &rcsm, sizeof(rcsm),pRI, pRI->socket_id);

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

    RLOGD("dispatchImsCdmaSms: retry=%d, messageRef=%d", retry, messageRef);

    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }
    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP2;
    rism.retry = retry;
    rism.messageRef = messageRef;
    rism.message.cdmaMessage = &rcsm;

    callOnRequest(pRI->pCI->requestNumber, &rism,
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
    RLOGD("dispatchImsGsmSms: retry=%d, messageRef=%d", retry, messageRef);

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP;
    rism.retry = retry;
    rism.messageRef = messageRef;

    startRequest;
    appendPrintBuf("%stech=%d, retry=%d, messageRef=%d, ", printBuf,
                    (int)rism.tech, (int)rism.retry, rism.messageRef);
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
    callOnRequest(pRI->pCI->requestNumber, &rism,
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

    RLOGD("dispatchImsSms");
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

    RLOGD("dispatchCdmaSmsAck");
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

    callOnRequest(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa),pRI, pRI->socket_id);

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

        callOnRequest(pRI->pCI->requestNumber,
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

        callOnRequest(pRI->pCI->requestNumber,
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

    callOnRequest(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw),pRI, pRI->socket_id);

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
    if (callForRilVersion() < 4 && numParams > numParamsRilV3) {
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

static void dispatchPhbEntry(Parcel &p, RequestInfo *pRI) {

    RIL_PhbEntryStrucutre args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // storage type
    status = p.readInt32(&t);
    args.type = (int) t;

    // index of the entry
    status = p.readInt32(&t);
    args.index = (int) t;

    // phone number
    args.number = strdupReadString(p);

    // Type of the number
    status = p.readInt32(&t);
    args.ton = (int) t;

    // alpha Id
    args.alphaId = strdupReadString(p);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s%d,index=%d,num=%s,ton=%d,alphaId=%s", printBuf, args.type,
                   args.index, (char*)args.number, args.ton,  (char*)args.alphaId);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (s_callbacks.onRequest != NULL) {
         /* old vendor ril */
         RLOGD("callOnRequest for old vendor ril");
         s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);
    } else if (s_callbacksSocket.onRequest != NULL) {
         /* new vendor ril */
         RLOGD("callOnRequest for new vendor ril");
         s_callbacksSocket.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);
    }

#ifdef MEMSET_FREED
    memsetString (args.number);
    memsetString (args.alphaId);
#endif

    free (args.number);
    free (args.alphaId);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;

}

static void dispatchWritePhbEntryExt(Parcel &p, RequestInfo *pRI) {

    RIL_PHB_ENTRY args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // index of the entry
    status = p.readInt32(&t);
    args.index = (int) t;
    // phone number
    args.number = strdupReadString(p);
    // Type of the number
    status = p.readInt32(&t);
    args.type = (int) t;
    //text
    args.text = strdupReadString(p);
    //hidden
    status = p.readInt32(&t);
    args.hidden = (int) t;

    //group
    args.group = strdupReadString(p);
    //anr
    args.adnumber = strdupReadString(p);
    // Type of the adnumber
    status = p.readInt32(&t);
    args.adtype = (int) t;
    //SNE
    args.secondtext = strdupReadString(p);
    // email
    args.email = strdupReadString(p);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s,index=%d,num=%s,type=%d,text=%s,hidden=%d,group=%s,adnumber=%s,adtype=%d,secondtext=%s,email=%s", printBuf,
                   args.index, (char*)args.number, args.type, (char*)args.text,
                   args.hidden, (char*)args.group,(char*)args.adnumber, args.adtype,(char*)args.secondtext,(char*)args.email);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (s_callbacks.onRequest != NULL) {
         /* old vendor ril */
         RLOGD("callOnRequest for old vendor ril");
         s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);
    } else if (s_callbacksSocket.onRequest != NULL) {
         /* new vendor ril */
         RLOGD("callOnRequest for new vendor ril");
         s_callbacksSocket.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);
    }

#ifdef MEMSET_FREED
    memsetString (args.number);
    memsetString (args.text);
    memsetString (args.group);
    memsetString (args.adnumber);
    memsetString (args.secondtext);
    memsetString (args.email);
#endif
    free (args.number);
    free (args.text);
    free (args.group);
    free (args.adnumber);
    free (args.secondtext);
    free (args.email);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;

}
// For backwards compatibility with RILs that dont support RIL_REQUEST_VOICE_RADIO_TECH.
// When all RILs handle this request, this function can be removed and
// the request can be sent directly to the RIL using dispatchVoid.
static void dispatchVoiceRadioTech(Parcel& p, RequestInfo *pRI) {
    RIL_RadioState state = callOnStateRequest((RIL_SOCKET_ID)pRI->socket_id);

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
    RIL_RadioState state = callOnStateRequest((RIL_SOCKET_ID)pRI->socket_id);


    if ((RADIO_STATE_UNAVAILABLE == state) || (RADIO_STATE_OFF == state)) {
        RIL_onRequestComplete(pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
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

    int32_t countStrings;
    size_t datalen;
    char **pStrings;
    size_t totalLength;

    memset(&pf, 0, sizeof(pf));

    pf.apn = strdupReadString(p);
    pf.protocol = strdupReadString(p);

    status = p.readInt32(&t);
    pf.authtype = (int) t;

    pf.username = strdupReadString(p);
    pf.password = strdupReadString(p);
    pf.operatorNumeric = strdupReadString(p);

    status = p.readInt32(&t);
    pf.canHandleIms = (int) t;

    startRequest;
    appendPrintBuf("%sapn=%s, protocol=%s, auth_type=%d, username=%s, password=%s, operatorNumeric=%s, canHandleIms=%d",
            printBuf, pf.apn, pf.protocol, pf.auth_type, pf.username, pf.password, pf.operatorNumeric, pf.canHandleIms);
    closeRequest;

    status = p.readInt32 (&countStrings);
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

    pf.dualApnPlmnList = pStrings;
    totalLength = sizeof(pf)-sizeof(pf.dualApnPlmnList)+datalen;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }
    callOnRequest(pRI->pCI->requestNumber, &pf, totalLength, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(pf.apn);
    memsetString(pf.protocol);
    memsetString(pf.username);
    memsetString(pf.password);
    memsetString(pf.operatorNumeric);
#endif

    free(pf.apn);
    free(pf.protocol);
    free(pf.username);
    free(pf.password);
    free(pf.operatorNumeric);

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

    callOnRequest(pRI->pCI->requestNumber, &nvri, sizeof(nvri), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&nvri, 0, sizeof(nvri));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}
//[New R8 modem FD]
static void dispatchFD_Mode(Parcel &p, RequestInfo *pRI) {
    RIL_FDModeStructure args;
    status_t status;
    int t_value = 0;
    memset(&args, 0, sizeof(args));
    status = p.readInt32(&t_value);
    args.args_num = t_value;

    /* AT+EFD=<mode>[,<param1>[,<param2>]] */
    /* For all modes: but mode 0 & 1 only has one argument */
    if (args.args_num >= 1) {
        status = p.readInt32(&t_value);
        args.mode = t_value;
    }
    /* For mode 2 & 3 */
    if (args.args_num >= 2) {
        status = p.readInt32(&t_value);
        args.parameter1 = t_value;
    }
    /* Only mode 2 */
    if (args.args_num >=3) {
        status = p.readInt32(&t_value);
        args.parameter2 = t_value;
    }
    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

}

static void dispatchSimAuth(Parcel &p, RequestInfo *pRI) {

    RIL_SimAuthStructure args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // Session ID
    status = p.readInt32(&t);
    args.sessionId = (int) t;

    // mode
    status = p.readInt32(&t);
    args.mode = (int) t;

    args.param1 = strdupReadString(p);
    args.param2 = strdupReadString(p);

    if(args.mode == 1) {
        //GBA
        status = p.readInt32(&t);
        args.tag = (int) t;
    }

    if (status != NO_ERROR) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

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

    callOnRequest(pRI->pCI->requestNumber, &nvwi, sizeof(nvwi), pRI, pRI->socket_id);

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

    callOnRequest(pRI->pCI->requestNumber, &uicc_sub, sizeof(uicc_sub), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&uicc_sub, 0, sizeof(uicc_sub));
#endif
    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void readQosFromParcel(Parcel* p, Qos* qos)
{
    p->readInt32(&qos->qci);
    p->readInt32(&qos->dlGbr);
    p->readInt32(&qos->ulGbr);
    p->readInt32(&qos->dlMbr);
    p->readInt32(&qos->ulMbr);

    LOGD ("readQosFromParcel [%d %d %d %d %d]", qos->qci, qos->dlGbr, qos->ulGbr, qos->dlMbr, qos->ulMbr);
}

static void readTftFromParcel(Parcel* p, Tft* tft)
{
    int i, j, k;

    p->readInt32(&tft->operation);
    p->readInt32(&tft->pfNumber);
    LOGD ("readTftFromParcel [%d %d]", tft->operation, tft->pfNumber);

    for (i=0; i<tft->pfNumber; i++) {
        //packet filter information
        p->readInt32(&tft->pfList[i].id);
        p->readInt32(&tft->pfList[i].precedence);
        p->readInt32(&tft->pfList[i].direction);
        p->readInt32(&tft->pfList[i].networkPfIdentifier);
        p->readInt32(&tft->pfList[i].bitmap);
        char* address = strdupReadString(*p);
        char* mask = strdupReadString(*p);
        if (address != NULL) {
            int addrStrLen = strlen(address);
            int addrBufSize = sizeof(tft->pfList[i].address) - 1;
            strncpy(tft->pfList[i].address, address, (addrStrLen > addrBufSize) ? addrBufSize: addrStrLen);
        }
        if (mask != NULL) {
            int maskStrLen = strlen(mask);
            int maskBufSize = sizeof(tft->pfList[i].mask) - 1;
            strncpy(tft->pfList[i].mask, mask, (maskStrLen > maskBufSize) ? maskBufSize: maskStrLen);
        }
        p->readInt32(&tft->pfList[i].protocolNextHeader);
        p->readInt32(&tft->pfList[i].localPortLow);
        p->readInt32(&tft->pfList[i].localPortHigh);
        p->readInt32(&tft->pfList[i].remotePortLow);
        p->readInt32(&tft->pfList[i].remotePortHigh);
        p->readInt32(&tft->pfList[i].spi);
        p->readInt32(&tft->pfList[i].tos);
        p->readInt32(&tft->pfList[i].tosMask);
        p->readInt32(&tft->pfList[i].flowLabel);

        if (address != NULL) {
            memsetString (address);
            free (address);
        }
        if (mask != NULL) {
            memsetString (mask);
            free (mask);
        }
    }

    //TftParameter
    tft->tftParameter.linkedPfNumber = p->readInt32();
    for (i=0; i<tft->tftParameter.linkedPfNumber; i++)
        tft->tftParameter.linkedPfList[i] = p->readInt32();

    tft->tftParameter.authtokenFlowIdNumber = p->readInt32();
    for (i=0; i<tft->tftParameter.authtokenFlowIdNumber; i++) {
        tft->tftParameter.authtokenFlowIdList[i].authTokenNumber = p->readInt32();
        for (j=0; j<tft->tftParameter.authtokenFlowIdList[i].authTokenNumber; j++)
            tft->tftParameter.authtokenFlowIdList[i].authTokenList[j] = p->readInt32();

        tft->tftParameter.authtokenFlowIdList[i].flowIdNumber = p->readInt32();
        for (j=0; j<tft->tftParameter.authtokenFlowIdList[i].flowIdNumber; j++) {
            for (k=0; k<4; k++)
                tft->tftParameter.authtokenFlowIdList[i].flowIdList[j][k] = p->readInt32();
        }
    }
}

static void dispatchSetupDedicateDataCall(Parcel &p, RequestInfo *pRI)
{
    RIL_Dedicate_Data_Call_Struct args;
    memset(&args, 0, sizeof(args));

    int version = 0;
    p.readInt32(&version);
    p.readInt32(&args.ddcId);
    p.readInt32(&args.interfaceId);
    p.readInt32(&args.signalingFlag);

    LOGD ("dispatchSetupDedicateDataCall [version=%d, ddcId=%d, interfaceId=%d, signalingFlag=%d]", version, args.ddcId, args.interfaceId, args.signalingFlag);

    p.readInt32(&args.hasQos);
    if (args.hasQos) { //qos
        readQosFromParcel(&p, &args.qos);
    } else {
        LOGW("dispatchSetupDedicateDataCall no QOS specified");
    }

    p.readInt32(&args.hasTft);
    if (args.hasTft) { //tft
        readTftFromParcel(&p, &args.tft);
    } else {
        LOGW("dispatchSetupDedicateDataCall no TFT specified");
    }

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif
}

static void dispatchModifyDedicateDataCall(Parcel &p, RequestInfo *pRI)
{
    RIL_Dedicate_Data_Call_Struct args;
    memset(&args, 0, sizeof(args));

    int version = 0;
    p.readInt32(&version);
    p.readInt32(&args.cid);

    LOGD ("dispatchModifyDedicateDataCall [%d %d %d]", version, args.cid, args.signalingFlag);

    p.readInt32(&args.hasQos);
    if (args.hasQos) { //qos
        readQosFromParcel(&p, &args.qos);
    } else {
        LOGW("dispatchModifyDedicateDataCall no QOS specified");
    }

    p.readInt32(&args.hasTft);
    if (args.hasTft) { //tft
        readTftFromParcel(&p, &args.tft);
    } else {
        LOGW("dispatchModifyDedicateDataCall no TFT specified");
    }

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif
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
    callOnRequest(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, pRI->socket_id);

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
//MTK no use
#if 0
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
        CALL_ONREQUEST(pRI->pCI->requestNumber,
                              dataProfilePtrs,
                              num * sizeof(RIL_DataProfileInfo *),
                              pRI, pRI->socket_id);

#ifdef MEMSET_FREED
        memset(dataProfiles, 0, num * sizeof(RIL_DataProfileInfo));
        memset(dataProfilePtrs, 0, num * sizeof(RIL_DataProfileInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
#endif
}

static void dispatchRadioCapability(Parcel &p, RequestInfo *pRI){
    RIL_RadioCapability rc;
    int32_t t;
    status_t status;
    char *string8 = NULL;

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

    callOnRequest(pRI->pCI->requestNumber, &rc, sizeof(RIL_RadioCapability), pRI, pRI->socket_id);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

// External SIM [Start]
static void dispatchVsimEvent(Parcel &p, RequestInfo *pRI) {
    RIL_VsimEvent args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // Transcation id
    status = p.readInt32(&t);
    args.transaction_id = (int) t;

    // Event id
    status = p.readInt32(&t);
    args.eventId = (int) t;

    // Sim type
    status = p.readInt32(&t);
    args.sim_type = (int) t;

    startRequest;
    appendPrintBuf("%scmd=0x%X,transaction_id=%d,eventId=%d,sim_type=%d", printBuf,
        args.transaction_id, args.eventId, args.sim_type);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchVsimOperationEvent(Parcel &p, RequestInfo *pRI) {
    RIL_VsimOperationEvent args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // Transcation id
    status = p.readInt32(&t);
    args.transaction_id = (int) t;

    // Event id
    status = p.readInt32(&t);
    args.eventId = (int) t;

    // Result
    status = p.readInt32(&t);
    args.result = (int) t;

    // Data length
    status = p.readInt32(&t);
    args.data_length = (int) t;

    // Data array
    args.data = (char *)strdupReadString(p);

    LOGI ("[dispatchVsimOperationEvent]%d, %d, %d, %d, data:%s, response_addr:%p",
            args.transaction_id, args.eventId, args.result, args.data_length, args.data, &args);

    startRequest;
    appendPrintBuf("%scmd=0x%X,transaction_id=%d,eventId=%d,result=%d, data_len=%d, data=%s", printBuf,
        args.transaction_id, args.eventId, args.result, args.data_length, args.data);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static int responseVsimOperationEvent(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_VsimOperationEvent) ) {
        RLOGE("invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_VsimOperationEvent));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_VsimOperationEvent *p_cur = (RIL_VsimOperationEvent *) response;
    p.writeInt32(p_cur->transaction_id);
    p.writeInt32(p_cur->eventId);
    p.writeInt32(p_cur->result);
    p.writeInt32(p_cur->data_length);
    writeStringToParcel(p, p_cur->data);

    startResponse;
    appendPrintBuf("%stransaction_id=%d,eventId=%d,datalen=%d", printBuf, p_cur->transaction_id,
            p_cur->eventId, p_cur->data_length);
    closeResponse;

    return 0;
}
// External SIM [Start]


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

        if (written >= 0) {
            writeOffset += written;
        } else {   // written < 0
            RLOGE ("RIL Response: unexpected error on write errno:%d", errno);
            close(fd);
            return -1;
        }
    }
#if VDBG
    RLOGE("RIL Response bytes written:%d", writeOffset);
#endif
    return 0;
}

static int
sendResponseRaw (const void *data, size_t dataSize, RIL_SOCKET_ID socket_id) {
    int fd = s_ril_param_socket[socket_id].fdCommand;
    int ret;
    uint32_t header;
    pthread_mutex_t * writeMutexHook = &s_writeMutex[socket_id];

    RLOGE("Send Response to %s", rilSocketIdToString(socket_id));

    if (fd < 0) {
        RLOGE("Send Response, but fd is incorrect");
        return -1;
    }

    if (dataSize > MAX_COMMAND_BYTES) {
        RLOGE("RIL: packet larger than %u (%u)",
                MAX_COMMAND_BYTES, (unsigned int )dataSize);

        return -1;
    }

    pthread_mutex_lock(writeMutexHook);

    header = htonl(dataSize);

    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
        return ret;
    }

    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
        return ret;
    }

    pthread_mutex_unlock(writeMutexHook);

    return 0;
}

static int
sendClientResponseRaw (const void *data, size_t dataSize, RIL_Client_Type *client) {
    RIL_SOCKET_ID socket_id = client->simID;
    int fd = client->fdCommand;
    int ret;
    uint32_t header;
    pthread_mutex_t * writeMutexHook = client->writeMutex;

    RLOGE("Send Response to %s", client->name);

    if (fd < 0) {
        RLOGE("Send Response, but fd is incorrect");
        return -1;
    }

    if (dataSize > MAX_COMMAND_BYTES) {
        RLOGE("RIL: packet larger than %u (%u)",
                MAX_COMMAND_BYTES, (unsigned int )dataSize);

        return -1;
    }

    pthread_mutex_lock(writeMutexHook);

    header = htonl(dataSize);

    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
        return ret;
    }

    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
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

static int
sendClientUnsolResponse (Parcel &p, int unsolResponse, const void *data,
                                size_t datalen, RIL_Client_Type *client) {
    int clientID = client->clientID;
    printResponse;
    switch (clientID)
    {
        case RIL_CLIENT_ID_MAL:
                //TODO: send Unsol to client? is needed
                if (unsolResponse == RIL_UNSOL_ATCI_RESPONSE) {
                    return -1;
                } else {
                    return sendClientResponseRaw(p.data(), p.dataSize(), client);
                }
                break;
        default:
                RLOGD("RIL client %s: lmited the unsol response",client->name);
                break;
    }
    return -1;
}

static int
sendClientResponse(Parcel &p, RIL_Client_Type *client) {
    int clientID = client->clientID;
    printResponse;
    switch (clientID)
    {
        case RIL_CLIENT_ID_MAL:
                return sendClientResponseRaw(p.data(), p.dataSize(), client);
        default:
                RLOGD("RIL client %s: lmited the response",client->name);
                break;
    }
    return -1;
}
/** response is an int* pointing to an array of ints */

static int
responseInts(Parcel &p, void *response, size_t responselen) {
    int numInts;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        RLOGE("responseInts: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;

    numInts = responselen / sizeof(int);
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
      return responseInts(p, response, responselen);
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        RLOGE("responseStrings: invalid response length %d expected multiple of %d\n",
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

static int responseCallList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_Call *) != 0) {
        RLOGE("responseCallList: invalid response length %d expected multiple of %d\n",
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
        if ((callForRilVersion() < 3) || (p_cur->uusInfo == NULL || p_cur->uusInfo->uusData == NULL)) {
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Response) ) {
        RLOGE("invalid response length %d expected %d",
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

static void dispatchSmsParams(Parcel &p, RequestInfo *pRI) {
    RIL_SmsParams smsParams;
    int32_t t;
    status_t status;

    LOGD("dispatchSmsParams Enter.");

    memset(&smsParams, 0, sizeof(smsParams));

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    smsParams.format = t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    smsParams.vp = t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    smsParams.pid = t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    smsParams.dcs = t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    LOGD("dispatchSmsParams format: %d", smsParams.format);
    LOGD("dispatchSmsParams vp: %d", smsParams.vp);
    LOGD("dispatchSmsParams pid: %d", smsParams.pid);
    LOGD("dispatchSmsParams dcs: %d", smsParams.dcs);

    LOGD("dispatchSmsParams Send Request..");

    startRequest;
    appendPrintBuf("%sformat=%d,vp=%d,pid=%d,dcs=%d", printBuf,
            smsParams.format, smsParams.vp, smsParams.pid, smsParams.dcs);
    closeRequest;

    callOnRequest(pRI->pCI->requestNumber, &smsParams, sizeof(smsParams), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&smsParams, 0, sizeof(smsParams));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static int responseSmsParams(Parcel &p, void *response, size_t responselen) {
    if(response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if(responselen != (int)sizeof(RIL_SmsParams)) {
        LOGE("invalid response length %d expected %d",
             (int)responselen, (int)sizeof(RIL_SmsParams));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SmsParams *p_cur = (RIL_SmsParams *)response;
    p.writeInt32(p_cur->format);
    p.writeInt32(p_cur->vp);
    p.writeInt32(p_cur->pid);
    p.writeInt32(p_cur->dcs);

    startResponse;
    appendPrintBuf("%s%d,%d,%d,%d", printBuf, p_cur->format, p_cur->vp,
                   p_cur->pid, p_cur->dcs);
    closeResponse;

    return 0;
}

static int responseDataCallListV4(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v4) != 0) {
        RLOGE("responseDataCallListV4: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v4));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(4);

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

static int responseDataCallListV6(Parcel &p, void *response, size_t responselen)
{
   if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v6) != 0) {
        RLOGE("responseDataCallListV6: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v6));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(6);

    int num = responselen / sizeof(RIL_Data_Call_Response_v6);
    p.writeInt32(num);

    RIL_Data_Call_Response_v6 *p_cur = (RIL_Data_Call_Response_v6 *) response;
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
        appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s],", printBuf,
            p_cur[i].status,
            p_cur[i].suggestedRetryTime,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].ifname,
            (char*)p_cur[i].addresses,
            (char*)p_cur[i].dnses,
            (char*)p_cur[i].gateways);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseDataCallListV9(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v9) != 0) {
        RLOGE("responseDataCallListV9: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v9));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(10);

    int num = responselen / sizeof(RIL_Data_Call_Response_v9);
    p.writeInt32(num);

    RIL_Data_Call_Response_v9 *p_cur = (RIL_Data_Call_Response_v9 *) response;
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
        writeStringToParcel(p, p_cur[i].pcscf);
        appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s,%s],", printBuf,
            p_cur[i].status,
            p_cur[i].suggestedRetryTime,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].ifname,
            (char*)p_cur[i].addresses,
            (char*)p_cur[i].dnses,
            (char*)p_cur[i].gateways,
            (char*)p_cur[i].pcscf);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseDataCallList(Parcel &p, void *response, size_t responselen)
{
    // Write version
    if (isImsSupport()) {
        // For dedicate bearer support, add the magic number 1000
        // When framework parsing the version, it can handle if or not to get concatenated bearer
        p.writeInt32(callForRilVersion() + 1000);
    } else {
        p.writeInt32(callForRilVersion());
    }

    if (callForRilVersion() < 5) {
        return responseDataCallListV4(p, response, responselen);
    } else {
        if (response == NULL && responselen != 0) {
            RLOGE("invalid response: NULL");
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        if (responselen % sizeof(RIL_Data_Call_Response_v11) != 0) {
            RLOGE("invalid response length %d expected multiple of %d",
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
            if (isImsSupport()) {
                writeStringToParcel(p, p_cur[i].pcscf);
            } else {
                writeStringToParcel(p, "");
            }
            p.writeInt32(p_cur[i].mtu);

            // No need to include default and dedicate bearer info for data call list response.
            if (isImsSupport() && isReqFromMAL()) {
                if (p_cur[i].concatenateNum > 0) {
                    // the total number of concatenated bearer will be written here
                    responseSetupDedicateDataCall(p, p_cur[i].concatenate,
                            p_cur[i].concatenateNum*sizeof(RIL_Dedicate_Data_Call_Struct));
                } else {
                    // the number of concatenated bearer should be always written so we add it here
                    p.writeInt32(p_cur[i].concatenateNum);
                }

                // VoLTE response for default bearer
                responseSetupDedicateDataCall(p, &p_cur[i].defaultBearer, sizeof(RIL_Dedicate_Data_Call_Struct));

                p.writeInt32(p_cur[i].eran_type);
                appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%d,%s,%s,%s,%d %d],", printBuf,
                    p_cur[i].status,
                    p_cur[i].suggestedRetryTime,
                    p_cur[i].cid,
                    (p_cur[i].active == 0)?"down":"up",
                    (char*)p_cur[i].ifname,
                    (char*)p_cur[i].addresses,
                    (char*)p_cur[i].dnses,
                    (char*)p_cur[i].gateways,
                    p_cur[i].concatenateNum,
                    p_cur[i].eran_type);
            } else {
                appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%d,%s,%s,%s],", printBuf,
                    p_cur[i].status,
                    p_cur[i].suggestedRetryTime,
                    p_cur[i].cid,
                    (p_cur[i].active == 0)?"down":"up",
                    (char*)p_cur[i].ifname,
                    (char*)p_cur[i].addresses,
                    (char*)p_cur[i].dnses,
                    (char*)p_cur[i].gateways);
            }
        }
        removeLastChar;
        closeResponse;
    }

    return 0;
}

static int responseUnsolSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    // Write version
    if (isImsSupport()) {
        // For dedicate bearer support, add the magic number 1000
        // When framework parsing the version, it can handle if or not to get concatenated bearer
        p.writeInt32(callForRilVersion() + 1000);
    } else {
        p.writeInt32(callForRilVersion());
    }

    if (callForRilVersion() < 5) {
        return responseDataCallListV4(p, response, responselen);
    } else {
        if (response == NULL && responselen != 0) {
            RLOGE("invalid response: NULL");
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        if (responselen % sizeof(RIL_Data_Call_Response_v11) != 0) {
            RLOGE("invalid response length %d expected multiple of %d",
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
            if (isImsSupport()) {
                writeStringToParcel(p, p_cur[i].pcscf);
            } else {
                writeStringToParcel(p, "");
            }
            p.writeInt32(p_cur[i].mtu);

            if (isImsSupport()) {
                if (p_cur[i].concatenateNum > 0) {
                    // the total number of concatenated bearer will be written here
                    responseSetupDedicateDataCall(p, p_cur[i].concatenate,
                            p_cur[i].concatenateNum*sizeof(RIL_Dedicate_Data_Call_Struct));
                } else {
                    // the number of concatenated bearer should be always written so we add it here
                    p.writeInt32(p_cur[i].concatenateNum);
                }

                // VoLTE response for default bearer
                responseSetupDedicateDataCall(p, &p_cur[i].defaultBearer, sizeof(RIL_Dedicate_Data_Call_Struct));

                p.writeInt32(p_cur[i].eran_type);
                appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%d,%s,%s,%s,%d %d],", printBuf,
                    p_cur[i].status,
                    p_cur[i].suggestedRetryTime,
                    p_cur[i].cid,
                    (p_cur[i].active == 0)?"down":"up",
                    (char*)p_cur[i].ifname,
                    (char*)p_cur[i].addresses,
                    (char*)p_cur[i].dnses,
                    (char*)p_cur[i].gateways,
                    p_cur[i].concatenateNum,
                    p_cur[i].eran_type);
            } else {
                appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%d,%s,%s,%s],", printBuf,
                    p_cur[i].status,
                    p_cur[i].suggestedRetryTime,
                    p_cur[i].cid,
                    (p_cur[i].active == 0)?"down":"up",
                    (char*)p_cur[i].ifname,
                    (char*)p_cur[i].addresses,
                    (char*)p_cur[i].dnses,
                    (char*)p_cur[i].gateways);
            }
        }
        removeLastChar;
        closeResponse;
    }

    return 0;
}

static int responseSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    RLOGD("s_callbacks.version %d", s_callbacks.version);
    if (callForRilVersion() < 5) {
        return responseStringsWithVersion(callForRilVersion(), p, response, responselen);
    } else {
        return responseDataCallList(p, response, responselen);
    }
}

static int responseRaw(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL with responselen != 0");
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SIM_IO_Response) ) {
        RLOGE("invalid response length was %d expected %d",
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CallForwardInfo *) != 0) {
        RLOGE("responseCallForwards: invalid response length %d expected multiple of %d",
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_SuppSvcNotification)) {
        RLOGE("invalid response length was %d expected %d",
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

static int responseCrssN(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CrssNotification)) {
        LOGE("invalid response length was %d expected %d",
             (int)responselen, (int)sizeof (RIL_CrssNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CrssNotification *p_cur = (RIL_CrssNotification *) response;
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);
    writeStringToParcel(p, p_cur->alphaid);
    p.writeInt32(p_cur->cli_validity);
    return 0;
}

static int responseCellList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_NeighboringCell *) != 0) {
        RLOGE("responseCellList: invalid response length %d expected multiple of %d\n",
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_InformationRecords)) {
        RLOGE("responseCdmaInformationRecords: invalid response length %d expected multiple of %d\n",
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
                    RLOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.display.alpha_len,
                         CDMA_ALPHA_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) calloc(1, (infoRec->rec.display.alpha_len + 1)
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
                    RLOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.number.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) calloc(1, (infoRec->rec.number.len + 1)
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
                    RLOGE("invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.redir.redirectingNumber.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*)calloc(1, (infoRec->rec.redir.redirectingNumber
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
                RLOGE("RIL_CDMA_T53_RELEASE_INFO_REC: return INVALID_RESPONSE");
                return RIL_ERRNO_INVALID_RESPONSE;
            default:
                RLOGE("Incorrect name value");
                return RIL_ERRNO_INVALID_RESPONSE;
        }
    }
    closeResponse;

    return 0;
}

static int responseRilSignalStrength(Parcel &p,
                    void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
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
            if (callForRilVersion() <= 6) {
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
            p.writeInt32(p_cur->WCDMA_SignalStrength.isGsm);
            p.writeInt32(p_cur->WCDMA_SignalStrength.rssiQdbm);
            p.writeInt32(p_cur->WCDMA_SignalStrength.rscpQdbm);
            p.writeInt32(p_cur->WCDMA_SignalStrength.Ecn0Qdbm);
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
        RLOGE("invalid response length");
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_SignalInfoRecord)) {
        RLOGE("invalid response length %d expected sizeof (RIL_CDMA_SignalInfoRecord) of %d\n",
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen < sizeof(RIL_CDMA_CallWaiting_v6)) {
        RLOGW("Upgrade to ril version %d\n", RIL_VERSION);
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
        RLOGE("responseSimRefresh: invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RLOGD("responseSimRefresh: %d.", callForRilVersion());
//    if (callForRilVersion() > 7) {
        RIL_SimRefreshResponse_v7 *p_cur = ((RIL_SimRefreshResponse_v7 *) response);
        p.writeInt32(p_cur->result);
//        p.writeInt32(p_cur->ef_id);
        writeStringToParcel(p, p_cur->ef_id);
        writeStringToParcel(p, p_cur->aid);
        RLOGD("responseSimRefresh: %s, %s.", p_cur->ef_id, p_cur->aid);

        appendPrintBuf("%sresult=%d, ef_id=%s, aid=%s",
                printBuf,
                p_cur->result,
                p_cur->ef_id,
                p_cur->aid);
//    } else {
//        int *p_cur = ((int *) response);
//        p.writeInt32(p_cur[0]);
//        p.writeInt32(p_cur[1]);
//        writeStringToParcel(p, NULL);

//        appendPrintBuf("%sresult=%d, ef_id=%d",
//                printBuf,
//                p_cur[0],
//                p_cur[1]);
//    }
    closeResponse;

    return 0;
}

static int responseCellInfoList(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CellInfo) != 0) {
        RLOGE("responseCellInfoList: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CellInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_CellInfo);
    p.writeInt32(num);

    RIL_CellInfo** res = (RIL_CellInfo**)response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        RIL_CellInfo *p_cur = (RIL_CellInfo*)res[i];
        p_cur->timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
        p_cur->timeStamp = elapsedRealtime() * 1000000; // Time milliseconds since bootup, convert to nano seconds

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
            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                appendPrintBuf("%s TDSCDMA id: mcc=%d,mnc=%d,lac=%d,cid=%d,cpid=%d,", printBuf,
                    p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                    p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                    p_cur->CellInfo.tdscdma.cellIdentityTdscdma.lac,
                    p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cid,
                    p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                appendPrintBuf("%s tdscdmaSS: rscp=%d],", printBuf,
                    p_cur->CellInfo.tdscdma.signalStrengthTdscdma.rscp);

                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.lac);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cid);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                p.writeInt32(p_cur->CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                break;
            }
        }
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

static int responseEpcNetworkFeatureInfo(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_EpcNetworkFeatureInfo) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_EpcNetworkFeatureInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_EpcNetworkFeatureInfo *p_cur = (RIL_EpcNetworkFeatureInfo *) response;
    startResponse;
    p.writeInt32(p_cur->emc_attached_support);
    p.writeInt32(p_cur->emc_call_barred);
    writeStringToParcel(p, p_cur->tac);
    writeStringToParcel(p, p_cur->ci);
    closeResponse;

    return 0;
}

static int responseSetupDedicateDataCall(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Dedicate_Data_Call_Struct) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Dedicate_Data_Call_Struct));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_Dedicate_Data_Call_Struct);
    p.writeInt32(num);

    RIL_Dedicate_Data_Call_Struct *p_cur = (RIL_Dedicate_Data_Call_Struct *) response;
    startResponse;
    int i=0, j=0, k=0, l=0;
    for (i = 0; i < num; i++) {
        p.writeInt32(p_cur[i].interfaceId);
        p.writeInt32(p_cur[i].primaryCid);
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        p.writeInt32(p_cur[i].signalingFlag);
        p.writeInt32(p_cur[i].bearerId);
        p.writeInt32(p_cur[i].failCause);
        //QOS
        p.writeInt32(p_cur[i].hasQos);
        if (p_cur[i].hasQos) {
            p.writeInt32(p_cur[i].qos.qci);
            p.writeInt32(p_cur[i].qos.dlGbr);
            p.writeInt32(p_cur[i].qos.ulGbr);
            p.writeInt32(p_cur[i].qos.dlMbr);
            p.writeInt32(p_cur[i].qos.ulMbr);
        }
        //tft
        p.writeInt32(p_cur[i].hasTft);
        if (p_cur[i].hasTft) {
            p.writeInt32(p_cur[i].tft.operation);
            p.writeInt32(p_cur[i].tft.pfNumber);
            for (j=0; j<p_cur[i].tft.pfNumber; j++) { //only response necessary length
                p.writeInt32(p_cur[i].tft.pfList[j].id);
                p.writeInt32(p_cur[i].tft.pfList[j].precedence);
                p.writeInt32(p_cur[i].tft.pfList[j].direction);
                p.writeInt32(p_cur[i].tft.pfList[j].networkPfIdentifier);
                p.writeInt32(p_cur[i].tft.pfList[j].bitmap);
                writeStringToParcel(p, p_cur[i].tft.pfList[j].address);
                writeStringToParcel(p, p_cur[i].tft.pfList[j].mask);
                p.writeInt32(p_cur[i].tft.pfList[j].protocolNextHeader);
                p.writeInt32(p_cur[i].tft.pfList[j].localPortLow);
                p.writeInt32(p_cur[i].tft.pfList[j].localPortHigh);
                p.writeInt32(p_cur[i].tft.pfList[j].remotePortLow);
                p.writeInt32(p_cur[i].tft.pfList[j].remotePortHigh);
                p.writeInt32(p_cur[i].tft.pfList[j].spi);
                p.writeInt32(p_cur[i].tft.pfList[j].tos);
                p.writeInt32(p_cur[i].tft.pfList[j].tosMask);
                p.writeInt32(p_cur[i].tft.pfList[j].flowLabel);
            }
            //TftParameter
            p.writeInt32(p_cur[i].tft.tftParameter.linkedPfNumber);
            for (j=0; j<p_cur[i].tft.tftParameter.linkedPfNumber; j++) //only response necessary length
                p.writeInt32(p_cur[i].tft.tftParameter.linkedPfList[j]);

            p.writeInt32(p_cur[i].tft.tftParameter.authtokenFlowIdNumber);
            for (j=0; j<p_cur[i].tft.tftParameter.authtokenFlowIdNumber; j++) {
                p.writeInt32(p_cur[i].tft.tftParameter.authtokenFlowIdList[k].authTokenNumber);
                for (k=0; k<p_cur[i].tft.tftParameter.authtokenFlowIdList[k].authTokenNumber; k++)
                    p.writeInt32(p_cur[i].tft.tftParameter.authtokenFlowIdList[j].authTokenList[k]);

                p.writeInt32(p_cur[i].tft.tftParameter.authtokenFlowIdList[k].flowIdNumber);
                for (k=0; k<p_cur[i].tft.tftParameter.authtokenFlowIdList[k].flowIdNumber; k++) {
                    for (l=0; l<4; l++)
                        p.writeInt32(p_cur[i].tft.tftParameter.authtokenFlowIdList[j].flowIdList[k][l]);
                }
            }
        }

        //PCSCF
        p.writeInt32(p_cur[i].hasPcscf);
        if (p_cur[i].hasPcscf)
            writeStringToParcel(p, p_cur[i].pcscf);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/// [C2K] IRAT feature.
static int responseIratStateChange(Parcel &p, void *response, size_t responselen) {
	LOGD("responseIratStateChange: 0 responselen = %d, sizeof(RIL_Pdn_IratInfo) = %d.",
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

#ifdef MTK_RIL
void triggerEvLoop() {
#else
static void triggerEvLoop() {
#endif
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
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen == sizeof (RIL_CardStatus_v6)) {
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
        RLOGE("responseSimStatus: A RilCardStatus_v6 or _v5 expected\n");
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

    RLOGD("Inside responseCdmaSms");

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CDMA_SMS_Message)) {
        RLOGE("invalid response length was %d expected %d",
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

static int responsePhbEntries(Parcel &p,void *response, size_t responselen) {


    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_PhbEntryStrucutre *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_PhbEntryStrucutre *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_PhbEntryStrucutre *);
    p.writeInt32(num);

    startResponse;
    RIL_PhbEntryStrucutre **p_cur =
        (RIL_PhbEntryStrucutre **) response;
    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->type);
        p.writeInt32(p_cur[i]->index);
        writeStringToParcel(p, p_cur[i]->number);
        p.writeInt32(p_cur[i]->ton);
        writeStringToParcel(p, p_cur[i]->alphaId);

        appendPrintBuf("%s [%d: type=%d, index=%d, \
                number=%s, ton=%d, alphaId =%s]",
                       printBuf, i, p_cur[i]->type, p_cur[i]->index,
                       p_cur[i]->number, p_cur[i]->ton,
                       p_cur[i]->alphaId);
    }
    closeResponse;

    return 0;

}

static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof (RIL_PHB_ENTRY *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_PHB_ENTRY *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    int num = responselen / sizeof(RIL_PHB_ENTRY *);
    p.writeInt32(num);
    startResponse;
    RIL_PHB_ENTRY **p_cur =
        (RIL_PHB_ENTRY **) response;

    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->index);
        writeStringToParcel(p, p_cur[i]->number);
        p.writeInt32(p_cur[i]->type);
        writeStringToParcel(p, p_cur[i]->text);
        p.writeInt32(p_cur[i]->hidden);
        writeStringToParcel(p, p_cur[i]->group);
        writeStringToParcel(p, p_cur[i]->adnumber);
        p.writeInt32(p_cur[i]->adtype);
        writeStringToParcel(p, p_cur[i]->secondtext);
        writeStringToParcel(p, p_cur[i]->email);

        appendPrintBuf("%s [%d: index=%d, \
                number=%s, type=%d, text =%s, hidden=%d,group=%s,adnumber=%s,adtype=%d,sectext=%s,email=%s]",
                       printBuf, i,  p_cur[i]->index,
                       p_cur[i]->number, p_cur[i]->type,
                       p_cur[i]->text, p_cur[i]->hidden,p_cur[i]->group,p_cur[i]->adnumber,
                       p_cur[i]->adtype,p_cur[i]->secondtext,p_cur[i]->email);
    }

    closeResponse;

    return 0;

}

static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL || responselen == 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Memory_Status)) {
        LOGE("invalid response length %d expected sizeof (RIL_SMS_Memory_Status) of %d\n",
             (int)responselen, (int)sizeof(RIL_SMS_Memory_Status));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_SMS_Memory_Status *mem_status = (RIL_SMS_Memory_Status*)response;

    p.writeInt32(mem_status->used);
    p.writeInt32(mem_status->total);

    appendPrintBuf("%s [used = %d, total = %d]", printBuf, mem_status->used, mem_status->total);

    closeResponse;

    return 0;
}

static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL || responselen == 0) {
        LOGE("responseGetPhbMemStorage invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen != sizeof (RIL_PHB_MEM_STORAGE_RESPONSE)) {
        LOGE("invalid response length %d expected sizeof (RIL_PHB_MEM_STORAGE_RESPONSE) of %d\n",
             (int)responselen, (int)sizeof(RIL_PHB_MEM_STORAGE_RESPONSE));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_PHB_MEM_STORAGE_RESPONSE *mem_status = (RIL_PHB_MEM_STORAGE_RESPONSE*)response;
    writeStringToParcel (p, mem_status->storage);
    p.writeInt32(mem_status->used);
    p.writeInt32(mem_status->total);

    appendPrintBuf("%s [storage = %s, used = %d, total = %d]", printBuf, mem_status->storage, mem_status->used, mem_status->total);

    closeResponse;
    return 0;
}

static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen) {
    if(NULL == response) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if(responselen != sizeof(RIL_CBConfigInfo)) {
        LOGE("invalid response length %d expected %d",
            responselen, sizeof(RIL_CBConfigInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CBConfigInfo *p_cur = (RIL_CBConfigInfo *)response;
    p.writeInt32(p_cur->mode);
    writeStringToParcel(p, p_cur->channelConfigInfo);
    writeStringToParcel(p, p_cur->languageConfigInfo);
    p.writeInt32(p_cur->isAllLanguageOn);

    startResponse;
    appendPrintBuf("%s%d,%s,%s,%d", printBuf, p_cur->mode, p_cur->channelConfigInfo,
                   p_cur->languageConfigInfo, p_cur->isAllLanguageOn);
    closeResponse;

    return 0;
}

static int responseEtwsNotification(Parcel &p, void *response, size_t responselen) {
    if(NULL == response) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if(responselen != sizeof(RIL_CBEtwsNotification)) {
        LOGE("invalid response length %d expected %d",
            responselen, sizeof(RIL_CBEtwsNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CBEtwsNotification *p_cur = (RIL_CBEtwsNotification *)response;
    p.writeInt32(p_cur->warningType);
    p.writeInt32(p_cur->messageId);
    p.writeInt32(p_cur->serialNumber);
    writeStringToParcel(p, p_cur->plmnId);
    writeStringToParcel(p, p_cur->securityInfo);

    startResponse;
    appendPrintBuf("%s%d,%d,%d,%s,%s", printBuf, p_cur->waringType, p_cur->messageId,
                   p_cur->serialNumber, p_cur->plmnId, p_cur->securityInfo);
    closeResponse;

    return 0;
}

static int responseDcRtInfo(Parcel &p, void *response, size_t responselen)
{
    int num = responselen / sizeof(RIL_DcRtInfo);
    if ((responselen % sizeof(RIL_DcRtInfo) != 0) || (num != 1)) {
        RLOGE("invalid response length %d expected multiple of %d",
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

//MTK-START [mtk80776] WiFi Calling
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI)
{
    RIL_UICC_IO_v6 uiccIo;
    int32_t t;
    status_t status;

    memset(&uiccIo, 0, sizeof(uiccIo));

    status = p.readInt32(&t);
    uiccIo.sessionId = (int)t;

    status = p.readInt32(&t);
    uiccIo.command = (int)t;

    status = p.readInt32(&t);
    uiccIo.fileId = (int)t;

    uiccIo.path = strdupReadString(p);

    status = p.readInt32(&t);
    uiccIo.p1 = (int)t;

    status = p.readInt32(&t);
    uiccIo.p2 = (int)t;

    status = p.readInt32(&t);
    uiccIo.p3 = (int)t;

    uiccIo.data = strdupReadString(p);
    uiccIo.pin2 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s sessionId=0x%X, cmd=0x%x, efid=0x%x path=%s,%d,%d,%d,%s,pin2=%s", printBuf,
            uiccIo.sessionId, uiccIo.command, uiccIo.fileId, (char *)uiccIo.path,
            uiccIo.p1, uiccIo.p2, uiccIo.p3, (char *)uiccIo.data, (char *)uiccIo.pin2);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->reqeustNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &uiccIo, sizeof(RIL_UICC_IO_v6), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(uiccIo.path);
    memsetString(uiccIo.data);
    memsetString(uiccIo.pin2);
#endif

    free(uiccIo.path);
    free(uiccIo.data);
    free(uiccIo.pin2);

#ifdef MEMSET_FREED
    memset(&uiccIo, 0, sizeof(RIL_UICC_IO_v6));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI)
{
    RIL_UICC_Authentication uiccAuth;
    int32_t t;
    int32_t len;
    status_t status;

    LOGD("dispatchUiccAuthentication Enter...");

    memset(&uiccAuth, 0, sizeof(uiccAuth));

    status = p.readInt32(&t);
    uiccAuth.session_id = (int)t;

    uiccAuth.context1 = strdupReadString(p);
    uiccAuth.context2 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%ssessionId=%d, context1=%s, context2=%s", printBuf,
            uiccAuth.session_id, uiccAuth.context1, uiccAuth.context2);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    callOnRequest(pRI->pCI->requestNumber, &uiccAuth, sizeof(uiccAuth), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(uiccAuth.context1);
    memsetString(uiccAuth.context2);
#endif

    free(uiccAuth.context1);
    free(uiccAuth.context2);

#ifdef MEMSET_FREED
    memset(&uiccAuth, 0, sizeof(uiccAuth));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}
//MTK-END [mtk80776] WiFi Calling

/**
 * A write on the wakeup fd is done just to pop us out of select()
 * We empty the buffer here and then ril_event will reset the timers on the
 * way back down
 */
static void processWakeupCallback(int fd, short flags, void *param) {
    char buff[16];
    int ret;

    RLOGV("processWakeupCallback");

    /* empty our wakeup socket out */
    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}
static void onCommandsClinetClosed(RIL_Client_Type *client)
{
     int ret;
     RequestInfo *p_cur;
     /* Hook for current context
     pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
     int socket_id;

     if (client == NULL) return;
     RLOGD("RIL client: %s disconnect", client->name);
     for (socket_id = RIL_SOCKET_1; socket_id < SIM_COUNT; socket_id++)
     {
             pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
             /* pendingRequestsHook refer to &s_pendingRequests */
             RequestInfo ** pendingRequestsHook = &s_pendingRequests[socket_id];

             /* mark pending requests as "cancelled" so we dont report responses */
             ret = pthread_mutex_lock(pendingRequestsMutexHook);
             assert (ret == 0);

             p_cur = *pendingRequestsHook;

             for (p_cur = *pendingRequestsHook
                          ; p_cur != NULL
                          ; p_cur  = p_cur->p_next
             )
             {
                   if ( p_cur->client && (p_cur->client == client) )
                   {
                      p_cur->cancelled = 1;
                  }
             }

             ret = pthread_mutex_unlock(pendingRequestsMutexHook);
             assert (ret == 0);
     }
}
static void onCommandsSocketClosed(RIL_SOCKET_ID socket_id) {
    int ret;
    RequestInfo *p_cur;
    /* Hook for current context
       pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex[socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo **    pendingRequestsHook = &s_pendingRequests[socket_id];

    /* mark pending requests as "cancelled" so we dont report responses */
    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    p_cur = *pendingRequestsHook;

    for (p_cur = *pendingRequestsHook
            ; p_cur != NULL
            ; p_cur  = p_cur->p_next
    ) {
        p_cur->cancelled = 1;
    }

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    ///M: CC075: Hangup all calls to sync with rild in phone process crash case @{
#if defined(MTK_RIL) && defined(MTK_MD_SHUT_DOWN_NT)
    RequestInfo *pRI;
    int request = RIL_REQUEST_HANGUP_ALL;

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 1;
    pRI->token = 0xffffffff;
    pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);

    //Add check for MSIM architecture - must set correct cid according to socket_id
    //Becuase in onRequest():It will set radioState according RIL_queryMyChannelId(t) return t->cid
    //If uses wrong cid:it will get wrong radioState!
    //If the radioState is not in RADIO_ON (e.g., SIM2:wo SIM card, then get RADIO_OFF state)
    //Then RIL_REQUEST_HANGUP_ALL will be skipped due to RADIO_OFF state
     if (socket_id == RIL_SOCKET_1) {
        pRI->cid = RIL_CMD_2;
    } else if (socket_id == RIL_SOCKET_2) {
            pRI->cid = RIL_CMD2_2;
    } else if (socket_id == RIL_SOCKET_3) {
        pRI->cid = RIL_CMD3_2;
    } else if (socket_id == RIL_SOCKET_4) {
        pRI->cid = RIL_CMD4_2;
    }

    pRI->socket_id = socket_id;
    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    //For Andriod L version: it should use s_callbacksSocket instead of s_callbacks
    //Use old ril vendor lib s_callbacks will cause SIGSEGV
    //Do not porting KK solution to L directly
    RLOGD("onCommandsSocketClosed():C[locl]> %s", requestToString(request));
    callOnRequest(RIL_REQUEST_HANGUP_ALL, NULL, 0, pRI, pRI->socket_id);

    //Use new API:issueLocalRequest() with pRI->socket_id for this local hangup action
    //But in this API, it will not fill pRI->cid (using default value 0 - RIL_URC) then can't get correct radio state
    //1.Invalid radio state (E.g.,If call is in SIM2: but only SIM2 with valid sim card) will skip this request
    //2.Even get radio state (E.g., Both SIM1 and SIM2 with valid sim cards)
    //  but it will send this request to the URC channel instead of CC channel
    //issueLocalRequest(RIL_REQUEST_HANGUP_ALL, NULL, 0, socket_id);
#endif
    /// @}


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
            processCommandBuffer(p_record, recordlen, p_info->socket_id);
        }
    }

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            RLOGE("error on reading command socket errno:%d\n", errno);
        } else {
            RLOGW("EOS.  Closing command socket.");
        }

        close(fd);
        p_info->fdCommand = -1;

        ril_event_del(p_info->commands_event);

        record_stream_free(p_rs);

        /* start listening for new connections again */
        rilEventAddWakeup(&s_listen_event[p_info->socket_id]);

        onCommandsSocketClosed(p_info->socket_id);
    }
}


static void processClientCommandsCallback(int fd, short flags, void *param) {
    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;
    RIL_Client_Type *client = (RIL_Client_Type *)param;

    assert(fd == client->fdCommand);

    p_rs = client->p_rs;

    for (;;) {
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { /* && p_record != NULL */
            processClientCommandBuffer(p_record, recordlen, client);
        }
    }

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            RLOGE("error on reading command socket %s errno:%d\n", client->name, errno);
        } else {
            RLOGW("EOS.  Closing command socket %s.", client->name);
        }

        close(fd);
        client->fdCommand = -1;

        ril_event_del(client->commands_event);

        record_stream_free(p_rs);

        /* start listening for new connections again */
        rilEventAddWakeup(client->listen_event);

        //remove all request related to the client fdCommand
        onCommandsClinetClosed(client);
    }
}
static void onNewCommandConnect(RIL_SOCKET_ID socket_id) {
    // Inform we are connected and the ril version
    int rilVer = callForRilVersion();
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RIL_CONNECTED,
                                    &rilVer, sizeof(rilVer), socket_id);

    // implicit radio state changed
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                    NULL, 0, socket_id);

    // Send last NITZ time data, in case it was missed
    if (s_lastNITZTimeData != NULL) {
        sendResponseRaw(s_lastNITZTimeData, s_lastNITZTimeDataSize, socket_id);

        free(s_lastNITZTimeData);
        s_lastNITZTimeData = NULL;
    }

    // Send previous  ECOPS data in case EOS happened before worldphone send resume camping
    if (s_prevEcopsData[socket_id] != NULL) {
        sendResponseRaw(s_prevEcopsData[socket_id], s_prevEcopsDataSize[socket_id], socket_id);
    }

    // Send previous EMSR data in case EOS happened before worldphone send resume camping
    if (s_prevEmsrData[socket_id] != NULL) {
        sendResponseRaw(s_prevEmsrData[socket_id], s_prevEmsrDataSize[socket_id], socket_id);
    }

    // Get version string
    if (callForGetVersion() != NULL) {
        const char *version;
        version = callForGetVersion();
        RLOGI("RIL Daemon version: %s\n", version);

        property_set(PROPERTY_RIL_IMPL, version);
    } else {
        RLOGI("RIL Daemon version: unavailable\n");
        property_set(PROPERTY_RIL_IMPL, "unavailable");
    }

}

static void listenCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    int fdCommand = -1;
    char* processName;
    RecordStream *p_rs;
    MySocketListenParam* listenParam;
    RilSocket *sapSocket = NULL;
    socketClient *sClient = NULL;

    SocketListenParam *p_info = (SocketListenParam *)param;

    if(RIL_SAP_SOCKET == p_info->type) {
        listenParam = (MySocketListenParam *)param;
        sapSocket = listenParam->socket;
    }

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);

    struct passwd *pwd = NULL;

    if(NULL == sapSocket) {
    assert (*p_info->fdCommand < 0);
    assert (fd == *p_info->fdListen);
        processName = PHONE_PROCESS;
    } else {
        assert (sapSocket->commandFd < 0);
        assert (fd == sapSocket->listenFd);
        processName = BLUETOOTH_PROCESS;
    }


    fdCommand = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (fdCommand < 0 ) {
        RLOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        if(NULL == sapSocket) {
        rilEventAddWakeup(p_info->listen_event);
        } else {
            rilEventAddWakeup(sapSocket->getListenEvent());
        }
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
            if (strcmp(pwd->pw_name, processName) == 0) {
                is_phone_socket = 1;
            } else {
                RLOGE("RILD can't accept socket from process %s", pwd->pw_name);
            }
        } else {
            RLOGE("Error on getpwuid() errno: %d", errno);
        }
    } else {
        RLOGD("Error on getsockopt() errno: %d", errno);
    }

    if (!is_phone_socket) {
        RLOGE("RILD must accept socket from %s", processName);

      close(fdCommand);
      fdCommand = -1;

        if(NULL == sapSocket) {
      onCommandsSocketClosed(p_info->socket_id);

      /* start listening for new connections again */
      rilEventAddWakeup(p_info->listen_event);
        } else {
            sapSocket->onCommandsSocketClosed();

            /* start listening for new connections again */
            rilEventAddWakeup(sapSocket->getListenEvent());
        }

      return;
    }

    ret = fcntl(fdCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }

    if(NULL == sapSocket) {
        RLOGI("libril: new connection to %s", rilSocketIdToString(p_info->socket_id));

        p_info->fdCommand = fdCommand;
        p_rs = record_stream_new(p_info->fdCommand, MAX_COMMAND_BYTES);
        p_info->p_rs = p_rs;

        ril_event_set (p_info->commands_event, p_info->fdCommand, 1,
                p_info->processCommandsCallback, p_info);
        rilEventAddWakeup (p_info->commands_event);

       onNewCommandConnect(p_info->socket_id);
    } else {
        RLOGI("libril: new connection");

        sapSocket->setCommandFd(fdCommand);
        p_rs = record_stream_new(sapSocket->getCommandFd(), MAX_COMMAND_BYTES);
        sClient = new socketClient(sapSocket,p_rs);
        ril_event_set (sapSocket->getCallbackEvent(), sapSocket->getCommandFd(), 1,
        sapSocket->getCommandCb(), sClient);

        rilEventAddWakeup(sapSocket->getCallbackEvent());
        sapSocket->onNewCommandConnect();
    }
#ifdef MTK_RIL
    sendPendedUrcs(p_info->socket_id, p_info->fdCommand);
#endif
}
static void clientListenCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    int fdCommand = -1;
    RecordStream *p_rs;
    RIL_Client_Type *client  = (RIL_Client_Type*) param;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    assert (client->fdCommand < 0);
    assert (fd == client->fdListen);

    client->fdCommand = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (client->fdCommand < 0 ) {
        RLOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(client->listen_event);
        return;
    }

    ret = fcntl(client->fdCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }

    RLOGI("libril: new connection to %s",client->name);

    p_rs = record_stream_new(client->fdCommand, MAX_COMMAND_BYTES);

    client->p_rs = p_rs;

    ril_event_set (client->commands_event, client->fdCommand, 1,
        processClientCommandsCallback, client);

    rilEventAddWakeup (client->commands_event);

     //TODO: notify the ril connected?
    //onNewCommandConnect(p_info->socket_id);

    //TODO: send the pending urc?
#ifdef MTK_RIL
    //sendPendedUrcs(p_info->socket_id, p_info->fdCommand);
#endif
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
    char *stk_str = NULL;
    RLOGI("debugCallback for socket %s", rilSocketIdToString(socket_id));

    acceptFD = accept (fd,  (sockaddr *) &peeraddr, &socklen);

    if (acceptFD < 0) {
        RLOGE ("error accepting on debug port: %d\n", errno);
        return;
    }

    if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
        RLOGE ("error reading on socket: number of Args: \n");
        return;
    }
    args = (char **) calloc(1, sizeof(char*) * number);


    RLOGI ("NUMBER:%d", number);

    for (int i = 0; i < number; i++) {
        int len;
        if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
            RLOGE ("error reading on socket: Len of Args: \n");
            freeDebugCallbackArgs(i, args);
            return;
        }
        // +1 for null-term

        RLOGI ("arg len:%d", len);

        args[i] = (char *) calloc(1, (sizeof(char) * len) + 1);
        if (recv(acceptFD, args[i], sizeof(char) * len, 0)
            != (int)sizeof(char) * len) {
            RLOGE ("error reading on socket: Args[%d] \n", i);
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
                case 1:
                    socket_id = RIL_SOCKET_2;
                    break;
                case 2:
                    socket_id = RIL_SOCKET_3;
                    break;
                case 3:
                    socket_id = RIL_SOCKET_4;
                    break;
                default:
                    socket_id = RIL_SOCKET_1;
                    break;
            }
        }
    }

    switch (atoi(args[0])) {
        case 0:
            RLOGI ("Connection on debug port: issuing reset.");
            issueLocalRequest(RIL_REQUEST_RESET_RADIO, NULL, 0, socket_id);
            break;
        case 1:
            RLOGI ("Connection on debug port: issuing radio power off.");
            data = 0;
            issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int), socket_id);
            // Close the socket
            if (s_ril_param_socket[socket_id].fdCommand > 0) {
                close(s_ril_param_socket[socket_id].fdCommand);
                s_ril_param_socket[socket_id].fdCommand = -1;
            }
            break;
        case 2:
            RLOGI ("Debug port: issuing unsolicited voice network change.");
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, socket_id);
            break;
        case 3:
            RLOGI ("Debug port: QXDM log enable.");
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
            RLOGI ("Debug port: QXDM log disable.");
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
            RLOGI("Debug port: Radio On");
            data = 1;
            issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int), socket_id);
            sleep(2);
            // Set network selection automatic.
            issueLocalRequest(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, NULL, 0, socket_id);
            break;
        case 6:
            RLOGI("Debug port: Setup Data Call, Apn :%s\n", args[1]);
            actData[0] = args[1];
            issueLocalRequest(RIL_REQUEST_SETUP_DATA_CALL, &actData,
                              sizeof(actData), socket_id);
            break;
        case 7:
            RLOGI("Debug port: Deactivate Data Call");
            issueLocalRequest(RIL_REQUEST_DEACTIVATE_DATA_CALL, &deactData,
                              sizeof(deactData), socket_id);
            break;
        case 8:
            RLOGI("Debug port: Dial Call");
            dialData.clir = 0;
            dialData.address = args[1];
            issueLocalRequest(RIL_REQUEST_DIAL, &dialData, sizeof(dialData), socket_id);
            break;
        case 9:
            RLOGI("Debug port: Answer Call");
            issueLocalRequest(RIL_REQUEST_ANSWER, NULL, 0, socket_id);
            break;
        case 10:
            RLOGI("Debug port: End Call");
            issueLocalRequest(RIL_REQUEST_HANGUP, &hangupData,
                              sizeof(hangupData), socket_id);
            break;
        default:
            RLOGE ("Invalid request");
            break;
    }
    freeDebugCallbackArgs(number, args);
    close(acceptFD);
}

static void oemCallback (int fd, short flags, void *param) {
    int acceptFD;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int number;
    char **args;

    acceptFD = accept (fd,  (sockaddr *) &peeraddr, &socklen);

    if (acceptFD < 0) {
        LOGE ("error accepting on oem port: %d\n", errno);
        return;
    }

    s_fdOem_command = acceptFD;

    if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
        LOGE ("error reading on socket: number of Args: \n");
        return;
    }
    args = (char **) calloc(1, sizeof(char*) * number);

    LOGI ("NUMBER:%d", number);

    for (int i = 0; i < number; i++) {
        unsigned int len;
        if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
            LOGE ("error reading on socket: Len of Args: \n");
            freeDebugCallbackArgs(i, args);
            return;
        }
        // +1 for null-term

        LOGI ("arg len:%d", len);

        args[i] = (char *) calloc(1, (sizeof(char) * len) + 1);
        if (recv(acceptFD, args[i], sizeof(char) * len, 0)
                != (int)(sizeof(char) * len)) {
            LOGE ("error reading on socket: Args[%d] \n", i);
            freeDebugCallbackArgs(i, args);
            return;
        }
        char * buf = args[i];
        buf[len] = 0;

        LOGI ("ARGS[%d]:%s",i, buf);
    }

    if(0 < handleSpecialRequestWithArgs(number, args)){
        freeDebugCallbackArgs(number, args);
    } else {
        LOGI("Oem port: SpecialRequest not support");
        freeDebugCallbackArgs(number, args);
        close(s_fdOem_command);
        s_fdOem_command = -1;
    }
}

void cacheEcopsUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID soc_id) {
    if (unsolResponse == RIL_UNSOL_RESPONSE_PLMN_CHANGED) {
        RLOGI("Can't send URCABC RIL_UNSOL_RESPONSE_PLMN_CHANGED cache it in s_prevEcopsData");
        int unsolResponseIndex;
        Parcel ptemp;
        ptemp.writeInt32 (RESPONSE_UNSOLICITED);
        ptemp.writeInt32 (unsolResponse);
        unsolResponseIndex = unsolResponse - RIL_UNSOL_VENDOR_BASE;
        s_mtk_unsolResponses[unsolResponseIndex]
              .responseFunction(ptemp, const_cast<void*>(data), datalen);
        if (s_prevEcopsData[soc_id] != NULL) {
            free(s_prevEcopsData[soc_id]);
            s_prevEcopsData[soc_id] = NULL;
        }

        s_prevEcopsData[soc_id] = calloc(1, ptemp.dataSize());
        s_prevEcopsDataSize[soc_id] = ptemp.dataSize();
        memcpy(s_prevEcopsData[soc_id], ptemp.data(), ptemp.dataSize());
        return;
    }
}

// External SIM [Start]
static AtResponseList* pendedVsimUrcList1 = NULL;
static AtResponseList* pendedVsimUrcList2 = NULL;
static AtResponseList* pendedVsimUrcList3 = NULL;
static AtResponseList* pendedVsimUrcList4 = NULL;

void cacheVsimUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID socket_id){
    //Only the URC list we wanted.
    if (unsolResponse != RIL_UNSOL_VSIM_OPERATION_INDICATION) {
        RLOGI("Don't need to cache the request");
        return;
    }
    AtResponseList* urcCur = NULL;
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;

    switch(socket_id) {
        case RIL_SOCKET_1:
            urcCur = pendedVsimUrcList1;
            break;
        case RIL_SOCKET_2:
            urcCur = pendedVsimUrcList2;
            break;
        case RIL_SOCKET_3:
            urcCur = pendedVsimUrcList3;
            break;
        case RIL_SOCKET_4:
            urcCur = pendedVsimUrcList4;
            break;
        default:
            RLOGE("Socket id is wrong!!");
            return;
    }
    while (urcCur != NULL) {
        RLOGD("Pended Vsim URC:%d, RILD:%s, :%s",
            pendedUrcCount,
            rilSocketIdToString(socket_id),
            requestToString(urcCur->id));
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (AtResponseList*)calloc(1, sizeof(AtResponseList));
    if (urcPrev != NULL)
        urcPrev->pNext = urcCur;
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);
    if (pendedUrcCount == 0) {
        switch(socket_id) {
            case RIL_SOCKET_1:
                pendedVsimUrcList1 = urcCur;
                break;
            case RIL_SOCKET_2:
                pendedVsimUrcList2 = urcCur;
                break;
            case RIL_SOCKET_3:
                pendedVsimUrcList3 = urcCur;
                break;
            case RIL_SOCKET_4:
                pendedVsimUrcList4 = urcCur;
                break;
            default:
                RLOGE("Socket id is wrong!!");
                return;
        }
    }
    RLOGD("Current pendedVsimUrcCount = %d", pendedUrcCount + 1);
}

static void sendVsimUrc(RIL_SOCKET_ID socket_id, AtResponseList* urcCached) {
    AtResponseList* urc = urcCached;
    AtResponseList* urc_temp;
    while (urc != NULL) {
        RLOGD("sendVsimPendedUrcs RIL%s, %s",
        rilSocketIdToString(socket_id),
        requestToString(urc->id));
        RIL_onUnsolicitedResponseSocket (urc->id, urc->data, urc->datalen, socket_id);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

void sendPendedVsimUrcs(int fdCommand) {
    RLOGD("Ready to send pended Vsim URCs, fdCommand:%d", fdCommand);
    if (fdCommand != -1) {
        sendVsimUrc(RIL_SOCKET_1, pendedVsimUrcList1);
        pendedVsimUrcList1 = NULL;
        if (SIM_COUNT >= 2) {
            sendVsimUrc(RIL_SOCKET_2, pendedVsimUrcList2);
            pendedVsimUrcList2 = NULL;
        }
        if (SIM_COUNT >= 3) {
            sendVsimUrc(RIL_SOCKET_3, pendedVsimUrcList3);
            pendedVsimUrcList3 = NULL;
        }
        if (SIM_COUNT >= 4) {
            sendVsimUrc(RIL_SOCKET_4, pendedVsimUrcList4);
            pendedVsimUrcList4 = NULL;
        }
    }
}

static int
blockingRead(int fd, void *buffer, size_t len) {
    size_t readOffset = 0;
    uint8_t *toRead;

    toRead = (uint8_t *)buffer;

    while (readOffset < len) {
        ssize_t reader;
        do {
            reader = recv(fd, toRead + readOffset, len - readOffset, 0);
        } while (reader < 0 && ((errno == EINTR) || (errno == EAGAIN)));

        if (reader >= 0) {
            readOffset += reader;
        } else {   // written < 0
            RLOGE ("blockingRead on read errno:%d", errno);
            return -1;
        }
    }

    return 0;
}

static void processVsimCommandsCallback(int fd, short flags, void *param) {
    VsimMessage msg;

    while (true) {
    if (blockingRead(fd, (void*)&msg, sizeof(msg)) < 0) {
        LOGE ("[processVsimCommandsCallback] error reading on socket: msg_id, errno:%d\n", errno);
        return;
    }

    LOGI ("[processVsimCommandsCallback] id: %d, msg_id:%d, slot_id:%d, data_len:%d",
            msg.transaction_id, msg.message_id, (msg.slotId >> 1), msg.length);

    /*
       public static final int MSG_ID_EVENT_REQUEST = 3;
       public static final int MSG_ID_UICC_RESET_RESPONSE = 4;
       public static final int MSG_ID_UICC_APDU_RESPONSE = 5;
    */

    if (msg.message_id == MSG_ID_EVENT_REQUEST) {
        RIL_VsimEvent event;

        event.transaction_id = msg.transaction_id;

        if (blockingRead(fd, (void*)&event.eventId, sizeof(event.eventId)) < 0) {
            LOGE ("[processVsimCommandsCallback] error reading on socket: eventId, errno:%d.\n", errno);
            return;
        }
        if (blockingRead(fd, (void*)&event.sim_type, sizeof(event.sim_type)) < 0) {
            LOGE ("[processVsimCommandsCallback] error reading on socket: sim_type, errno:%d.\n", errno);
            return;
        }

        LOGI ("[processVsimCommandsCallback] MSG_ID_EVENT_REQUEST, eventId:%d, sim_type:%d", event.eventId, event.sim_type);

        issueLocalRequestForResponse(RIL_LOCAL_REQUEST_VSIM_NOTIFICATION,
                &event, 1, s_ril_cntx[(msg.slotId >> 1)]);
    } else if (msg.message_id == MSG_ID_UICC_RESET_RESPONSE || msg.message_id == MSG_ID_UICC_APDU_RESPONSE) {
        RIL_VsimOperationEvent *response = (RIL_VsimOperationEvent *)calloc(1, sizeof(RIL_VsimOperationEvent));
        memset(response, 0, sizeof(RIL_VsimOperationEvent));

        response->transaction_id = msg.transaction_id;
        response->eventId = (msg.message_id == MSG_ID_UICC_RESET_RESPONSE)
                ? REQUEST_TYPE_ATR_EVENT : REQUEST_TYPE_APDU_EVENT;

        if (blockingRead(fd, (void*)&response->result, sizeof(response->result)) < 0) {
            LOGE ("[processVsimCommandsCallback] error reading on socket: response.result, errno:%d.\n", errno);
            free(response);
            return;
        }


        if (blockingRead(fd, (void*)&response->data_length, sizeof(response->data_length)) < 0) {
            LOGE ("[processVsimCommandsCallback] error reading on socket: response.data_length, errno:%d.\n", errno);
            free(response);
            return;
        }

        LOGI ("[processVsimCommandsCallback] UICC_RESPONSE, result:%d, data_length:%d", response->result, response->data_length);

        char *temp_buffer = (char  *) calloc(1, (sizeof(char) * response->data_length) + 1);
        memset(temp_buffer, 0, ((sizeof(char) * response->data_length) + 1));

        LOGI ("[processVsimCommandsCallback] UICC_RESPONSE, data:%s, (before)", (char *)response->data);

        if (blockingRead(fd, (void*)temp_buffer, response->data_length) < 0) {
            LOGE ("[processVsimCommandsCallback] error reading on socket: response.data_length, errno:%d.\n", errno);
            free(temp_buffer);
            free(response);
            return;
        }

        response->data = (char  *) calloc(1, (sizeof(char) * response->data_length * 2) + 1);
        memset(response->data, 0, ((sizeof(char) * response->data_length * 2) + 1));

        for (int i = 0; i < response->data_length; i++) {
            sprintf((response->data + (i*2)), "%02X", temp_buffer[i]);
        }

        free(temp_buffer);

        LOGI ("[processVsimCommandsCallback] UICC_RESPONSE, data:%s, response_addr:%p", (char *)response->data, response);

        issueLocalRequestForResponse(RIL_LOCAL_REQUEST_VSIM_OPERATION,
                response, sizeof(RIL_VsimOperationEvent), s_ril_cntx[(msg.slotId >> 1)]);

        free(response->data);
        free(response);
    }
    }
}

static void* vsimCallback (/*int fd, short flags,*/void *param)
{
    int acceptFD;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;

    char *data;

    LOGI ("[vsimCallback] enter, fd = %d", s_fdVsim);

    acceptFD = accept(s_fdVsim,  (sockaddr *) &peeraddr, &socklen);
    s_vsim_client.client_command_fd = acceptFD;

    LOGI ("[vsimCallback] acceptFD %d", s_vsim_client.client_command_fd);

    if (acceptFD < 0) {
        LOGE ("[vsimCallback] error accepting on oem port: %d\n", errno);
        return NULL;
    }

    s_fdVsim_command = acceptFD;

    //ril_event_set(&s_vsim_listen_event, s_fdVsim_command, true,
    //               processVsimCommandsCallback, NULL);

    //rilEventAddWakeup(&s_vsim_listen_event);

    sendPendedVsimUrcs(s_fdVsim_command);

    processVsimCommandsCallback(s_fdVsim_command, 0, NULL);

    return NULL;
}
// External SIM [End]

static void skipWhiteSpace(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && isspace(**p_cur)) {
        (*p_cur)++;
    }
}

static void skipNextComma(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && **p_cur != ',') {
        (*p_cur)++;
    }

    if (**p_cur == ',') {
        (*p_cur)++;
    }
}

static char * nextTok(char **p_cur)
{
    char *ret = NULL;

    skipWhiteSpace(p_cur);

    if (*p_cur == NULL) {
        ret = NULL;
    } else if (**p_cur == '"') {
        (*p_cur)++;
        ret = strsep(p_cur, "\"");
        skipNextComma(p_cur);
    } else {
        ret = strsep(p_cur, ",");
    }

    return ret;
}

int at_tok_nextstr(char **p_cur, char **p_out)
{
    if (*p_cur == NULL) {
        return -1;
    }

    *p_out = nextTok(p_cur);

    return 0;
}

/** returns 1 on "has more tokens" and 0 if no */
int at_tok_hasmore(char **p_cur)
{
    return ! (*p_cur == NULL || **p_cur == '\0');
}

/**
 * Parses the next integer in the AT response line and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 * "base" is the same as the base param in strtol
 */

static int at_tok_nextint_base(char **p_cur, int *p_out, int base, int  uns)
{
    char *ret;

    if (*p_cur == NULL) {
        return -1;
    }

    ret = nextTok(p_cur);

    if (ret == NULL) {
        return -1;
    } else {
        long l;
        char *end;

        if (uns)
            l = strtoul(ret, &end, base);
        else
            l = strtol(ret, &end, base);

        *p_out = (int)l;

        if (end == ret) {
            return -1;
        }
    }

    return 0;
}

/**
 * Parses the next base 10 integer in the AT response line
 * and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 */
int at_tok_nextint(char **p_cur, int *p_out)
{
    return at_tok_nextint_base(p_cur, p_out, 10, 0);
}

static int handleSpecialRequestWithArgs(int argCount, char** args){
    char *line, *cmd;
    int err;
    int slotId = 0;
    char *param[2];
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;
    char sim[PROPERTY_VALUE_MAX] ={0};
    int sim_status = 0;
    int simId3G = 0;
    char org_args[768] = {0};
    int modemOn = 0;

    if (1 == argCount)
    {
        line = args[0];
        strcpy(org_args, args[0]);
        err = at_tok_nextstr(&line,&cmd);
        if (err < 0) {
            LOGD("invalid command");
            goto error;
        }
        //LOGD("handleSpecialRequestWithArgs cmd = %s", cmd);

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line,&slotId);
            if (err < 0 || slotId >= SIM_COUNT) {
                LOGD("invalid slotId, %d", slotId);
                goto error;
            }
            //LOGD("handleSpecialRequestWithArgs slotId = %d", slotId);
        }

        if(at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line,&param[0]);
            if (err < 0) {
                LOGD("invalid param");
                goto error;
            }
            //LOGD("handleSpecialRequestWithArgs param[0] = %s", param[0]);
        }

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line, &param[1]);
            if (err < 0) {
                goto error;
            }
            //LOGD("handleSpecialRequestWithArgs param[1] = %s", param[1]);
        }

        if(strcmp(cmd, "EAP_SIM") == 0){
            s_EAPSIMAKA_fd = s_fdOem_command;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_SIM_AUTHENTICATION,&param,1,s_ril_cntx[slotId]);
            return 1;
        }else if(strcmp(cmd, "EAP_AKA") == 0){
            s_EAPSIMAKA_fd = s_fdOem_command;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_USIM_AUTHENTICATION,&param, 2,s_ril_cntx[slotId]);
            return 1;
        } else if (strcmp(cmd, "THERMAL") == 0) {
            if(s_THERMAL_fd > 0) {
                close(s_THERMAL_fd); // close previous fd, avoid fd leak
            }
            s_THERMAL_fd = s_fdOem_command;
            strcpy(args[0], org_args);
            line = args[0];
            //LOGD("Thermal line = %s", line);
            err = at_tok_nextstr(&line,&cmd);
            err = at_tok_nextint(&line,&slotId);
            char prop_value[PROPERTY_VALUE_MAX] = { 0 };
            int targetSim = 0;
            property_get(PROPERTY_3G_SIM, prop_value, "1");
            targetSim = atoi(prop_value) - 1;
            //LOGD("Thermal line = %s, cmd:%s, slotId:%d, targetSim: %d", line, cmd, slotId, targetSim);
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL, &line,1, (RIL_SOCKET_ID)targetSim);
            return 1;
        } else if (strcmp(cmd, "CHARGING_STOP") == 0) {

            //strcpy(org_args, "AT+ERAT=3");
            //issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, org_args, strlen(org_args));
            memset(org_args, 0, sizeof(org_args));
            if (slotId == 0) {
              char def_value[PROPERTY_VALUE_MAX] = {0};

              strcpy(org_args, "AT+EFUN=1");
              issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, org_args, strlen(org_args),socket_id);
              memset(org_args, 0, sizeof(org_args));
              property_get("persist.radio.charging_stop", def_value, "40");
              sprintf(org_args, "AT+ERFTX=1,0,0,%s", def_value);
              property_set("ril.charging_stop_enable", "1");
              issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, org_args, strlen(org_args),socket_id);
            } else {
              char charg_para[10] = {0};
              //sprintf(org_args, "AT+ERFTX=1,0,0,%d", slotId);
              sprintf(charg_para, "%d", slotId);
              property_set("persist.radio.charging_stop", charg_para);
            }

            close(s_fdOem_command);
            s_fdOem_command = -1;
            return 1;
        } else if (strcmp(cmd, "AT+ERFTX=1") == 0){
            issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, org_args, strlen(org_args),socket_id);

            char* result = (char*)calloc(10, sizeof(char));
            strcpy(result, "OK");
            int len = (int)strlen(result);
            int ret = send(s_fdOem_command, &len, sizeof(int), 0);
            LOGD("AT+ERFTX=1, fdOem:%d", s_fdOem_command);
            if (ret != sizeof(int)) {
                LOGD("Socket write Error: when sending arg length");
            } else {
                ret = send(s_fdOem_command, result, len, 0);
                if (ret != len) {
                    LOGD("lose data when send response. ");
                }
            }
            free(result);
            close(s_fdOem_command);
            s_fdOem_command = -1;

            return 1;
        }else if(strcmp(cmd, "SIMMELOCK_GETKEY") == 0) {
            s_SIMLOCK_fd = s_fdOem_command;
            s_fdOem_command = -1;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_GET_SHARED_KEY,NULL, 0,s_ril_cntx[slotId]);
            return 1;
        } else if(strcmp(cmd, "SIMMELOCK_SET") == 0) {
            s_SIMLOCK_fd = s_fdOem_command;
            s_fdOem_command = -1;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS,&param, 1,s_ril_cntx[slotId]);
            return 1;
        } else if(strcmp(cmd, "SIMMELOCK_GET_VERSION") == 0) {
            s_SIMLOCK_fd = s_fdOem_command;
            s_fdOem_command = -1;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO,NULL, 0,s_ril_cntx[slotId]);
            return 1;
        } else if(strcmp(cmd, "SIMMELOCK_RESET") == 0) {
            s_SIMLOCK_fd = s_fdOem_command;
            s_fdOem_command = -1;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS,NULL, 0,s_ril_cntx[slotId]);
            return 1;
        } else if(strcmp(cmd, "SIMMELOCK_GET_STATUS") == 0) {
            s_SIMLOCK_fd = s_fdOem_command;
            s_fdOem_command = -1;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_GET_MODEM_STATUS,NULL, 0,s_ril_cntx[slotId]);
            return 1;
        } else if (strcmp(cmd, "MDTM_TOG") == 0) {
            strcpy(args[0], org_args);
            line = args[0];
            LOGD("Thermal line = %s", line);
            err = at_tok_nextstr(&line, &cmd);
            err = at_tok_nextint(&line, &modemOn);
            // LOGD("modemOn = %d", modemOn);
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_SET_MODEM_THERMAL, &modemOn, 1, socket_id);
            return 1;
        /// M: For 3G VT only @{
        } else if (strcmp(cmd, "VT") == 0) {
            if(s_VT_fd > 0) {
                close(s_VT_fd); // close previous fd, avoid fd leak
            }
            s_VT_fd = s_fdOem_command;
            return 1;
        /// @}
        } else {
            // invalid request
            LOGD("invalid request");
            goto error;
        }
    }
    else
    {
        goto error;
    }
error:
    LOGE ("Invalid request");

    return 0;
}


#ifdef MTK_RIL
void userTimerCallback (int fd, short flags, void *param) {
#else
static void userTimerCallback (int fd, short flags, void *param) {
#endif
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *)param;

#ifdef MTK_RIL
    if (p_info->cid > -1)
    {
        enqueue(NULL, NULL, 0, p_info, RIL_SOCKET_1);
        return;
    }
    else
    {
        p_info->p_callback(p_info->userParam);
    }
#else
    p_info->p_callback(p_info->userParam);
#endif

#ifdef MTK_RIL
    pthread_mutex_lock(&s_last_wake_mutex);
#endif
    // FIXME generalize this...there should be a cancel mechanism
    if (s_last_wake_timeout_info != NULL && s_last_wake_timeout_info == p_info) {
#ifdef MTK_RIL
        RLOGD("s_last_wake_timeout_info: %p reset to NULL", s_last_wake_timeout_info);
#endif
        s_last_wake_timeout_info = NULL;

    }
#ifdef MTK_RIL
    else
    {
         RLOGD("s_last_wake_timeout_info: %p ", s_last_wake_timeout_info);
    }
    pthread_mutex_unlock(&s_last_wake_mutex);
#endif

#ifdef MTK_RIL
    if (p_info->cid < 0)
    {
        RLOGD("userTimerCallback free  p_info: %p", p_info);
        RLOGD("userTimerCallback free  p_info->event: %p", &(p_info->event));
        free(p_info);
    }
#else
    free(p_info);
#endif

}

/* atci start */
#define MAX_DATA_SIZE 2048

static void processAtciCommandsCallback(int fd, short flags, void *param) {
    RLOGD("[ATCI]processAtciCommandsCallback");

    int ret;
    int request = RIL_REQUEST_OEM_HOOK_RAW;
    int recvLen = -1;
    char buffer[MAX_DATA_SIZE] = {0};
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_1];
    RequestInfo **    pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_1];
    int forceHandle = 0;

    while(recvLen == -1) {
        recvLen = recv(s_fdATCI_command, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            RLOGE("[ATCI] fail to receive data from ril-atci socket. errno = %d", errno);
            if(errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
    }

    RLOGD("[ATCI] data receive from atci is %s, data length is %d", buffer, recvLen);

    if (strcmp(buffer, "DISC") == 0 || recvLen <= 0) { //recLen == 0 means the client is disconnected.
        int sendLen = 0;

        if(0 < recvLen) {
            if (!isDualTalkMode()) {
                sendLen = send(s_fdATCI_command, buffer, recvLen, 0);
            }
            if (sendLen != recvLen) {
                RLOGE("[ATCI] lose data when send disc cfm to atci. errno = %d", errno);
            }
        } else {
            RLOGE("[ATCI] close ATCI due to error = %d", errno);
        }

        ril_event_del(&s_ATCIcommand_event);
        close(s_fdATCI_command);
        s_fdATCI_command = -1;
    } else {
        RequestInfo *pRI;
        pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));
        pRI->token = 0xffffffff;        // token is not used in this context

        if((strcmp(buffer, "AT+CFUN=0") == 0) || (strcmp(buffer, "at+cfun=0") == 0)){
            /* ALPS00306731 */
            RLOGE("Special Handling for AT+CFUN=0 from ATCI");
            forceHandle = 1;
        }

        if((strcmp(buffer, "AT+CFUN=1") == 0) || (strcmp(buffer, "at+cfun=1") == 0)){
            /* ALPS00314520 */
            RLOGE("Special Handling for AT+CFUN=1 from ATCI");
            forceHandle = 1;
        }

        if((strcmp(buffer, "AT+CFUN=4") == 0) || (strcmp(buffer, "at+cfun=4") == 0)){
            RLOGE("Special Handling for AT+CFUN=4 from ATCI");
            forceHandle = 1;
        }

        if((strcmp(buffer, "AT+ESPO=1") == 0)) {
            sentByPS = 1;
            sentBySIM = 0;
            property_set("persist.service.atci.ps", "0");
        } else if ((strcmp(buffer, "AT+ESPO=2") == 0)) {
            sentByPS = 2;
            sentBySIM = 0;
            property_set("persist.service.atci.ps", "1");
        } else if ((strcmp(buffer, "AT+ESPO=3") == 0)) {
            sentByPS = 3;
            sentBySIM = 0;
            property_set("persist.service.atci.ps", "2");
        } else if ((strcmp(buffer, "AT+ESUO=4") == 0)) {
            sentBySIM = 1;
            sentByPS = 0;
            property_set("persist.service.atci.sim", "0");
        } else if ((strcmp(buffer, "AT+ESUO=5") == 0)) {
            sentBySIM = 2;
            sentByPS = 0;
            property_set("persist.service.atci.sim", "1");
        } else if ((strcmp(buffer, "AT+ESUO=6") == 0)) {
            sentBySIM = 3;
            sentByPS = 0;
            property_set("persist.service.atci.sim", "2");
        } else if ((strcmp(buffer, "AT+ESUO=7") == 0)) {
            sentBySIM = 4;
            sentByPS = 0;
            property_set("persist.service.atci.sim", "3");
        }

    #ifdef MTK_RIL
        if (request < RIL_REQUEST_VENDOR_BASE){
            int channelDisable = 0;
            char channel_disable[PROPERTY_VALUE_MAX] ={0};
            property_get(PROPERTY_ATCI_CHANNEL, channel_disable, "0");
            channelDisable = atoi(channel_disable);

            pRI->pCI = &(s_commands[request]);
            if(1 == channelDisable) {
                pRI->pCI->proxyId = RIL_CMD_PROXY_3;
            }
        } else {
            pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_VENDOR_BASE]);
        }
    #else
        pRI->pCI = &(s_commands[request]);
    #endif /* MTK_RIL */

        pRI->cid = RIL_CMD_1;
        pRI->socket_id = RIL_SOCKET_1;
        property_get("persist.service.atci.ps", psNo, "");
        property_get("persist.service.atci.sim", simNo, "");
        LOGD("psNo = %s, simNo = %s", psNo, simNo);
        LOGD("sentByPS > %d, sentBySIM > %d", sentByPS, sentBySIM);
        if (sentByPS > 0 || (strlen(psNo) > 0 && sentBySIM == 0)) {
            LOGD("persist.service.atci.ps > %s", psNo);
            if (psNo[0] == '1' || sentByPS == 2) {
                LOGD("CID set to PS2");
                pRI->cid = RIL_CMD2_1;
                pRI->socket_id = RIL_SOCKET_2;
                pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_2];
                pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_2];
            } else if (psNo[0] == '2' || sentByPS == 3) {
                LOGD("CID set to PS3");
                pRI->cid = RIL_CMD3_1;
                pRI->socket_id = RIL_SOCKET_3;
                pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_3];
                pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_3];
            }
        } else if (sentBySIM > 0 || (strlen(simNo) > 0 && sentByPS == 0)) {
            LOGD("persist.service.atci.sim > %s", simNo);
            if (simNo[0] == '3' || sentBySIM == 4) {
                LOGD("CID set to SIM4");
                pRI->cid = RIL_CMD4_1;
                pRI->socket_id = RIL_SOCKET_4;
                pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_4];
                pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_4];
            } else if (simNo[0] == '2' || sentBySIM == 3) {
                LOGD("CID set to SIM3");
                pRI->cid = RIL_CMD3_1;
                pRI->socket_id = RIL_SOCKET_3;
                pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_3];
                pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_3];
            } else if (simNo[0] == '1' || sentBySIM == 2) {
                LOGD("CID set to SIM2");
                pRI->cid = RIL_CMD2_1;
                pRI->socket_id = RIL_SOCKET_2;
                pendingRequestsMutexHook = &s_pendingRequestsMutex[RIL_SOCKET_2];
                pendingRequestsHook = &s_pendingRequests[RIL_SOCKET_2];
            }
        } else {
            LOGD("Default CID SIM1");
        }
        ret = pthread_mutex_lock(pendingRequestsMutexHook);
        assert (ret == 0);
        pRI->p_next = *pendingRequestsHook;
        *pendingRequestsHook = pRI;

        ret = pthread_mutex_unlock(pendingRequestsMutexHook);
        assert (ret == 0);


        RLOGD("C[locl]> %s", requestToString(request));

#ifdef MTK_RIL
        {
            enqueueAtciRequest(pRI, buffer, recvLen, NULL, forceHandle);
        }
#else
        /*    sLastDispatchedToken = token; */
        pRI->pCI->dispatchFunction(p, pRI);
#endif

    }

}

static void atci_Callback (int fd, short flags, void *param)
{
    RLOGD("[ATCI] enter atci_Callback");
    int ret;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    assert (s_fdATCI_command< 0);
    assert (fd == s_fdATCI_listen);

    s_fdATCI_command = accept(s_fdATCI_listen, (sockaddr *) &peeraddr, &socklen);

    if (s_fdATCI_command < 0 ) {
        RLOGE("[ATCI] Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(&s_ATCIlisten_event);
        s_fdATCI_command = -1;
        return;
    }

    RLOGD("[ATCI] accept");

    ret = fcntl(s_fdATCI_command, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("[ATCI] Error setting O_NONBLOCK errno:%d", errno);
    }

    RLOGD("[ATCI] librilmtk: new rild-atci connection");


    ril_event_set (&s_ATCIcommand_event, s_fdATCI_command, true,
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
        RLOGE("Error in pipe() errno:%d", errno);
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
    RLOGE ("error in event_loop_base errno:%d", errno);
    // kill self to restart on error
    kill(0, SIGKILL);

    return NULL;
}

extern "C" void
RIL_startEventLoop(void) {
#ifdef MTK_RIL
    RIL_startRILProxys();
#endif /* MTK_RIL */

    /* spin up eventLoop thread and wait for it to get started */
    s_started = 0;
    pthread_mutex_lock(&s_startupMutex);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int result = pthread_create(&s_tid_dispatch, &attr, eventLoop, NULL);
    if (result != 0) {
        RLOGE("Failed to create dispatch thread: %s", strerror(result));
        goto done;
    }

    while (s_started == 0) {
        pthread_cond_wait(&s_startupCond, &s_startupMutex);
    }

done:
    pthread_mutex_unlock(&s_startupMutex);
}

// Used for testing purpose only.
extern "C" void RIL_setcallbacks (const RIL_RadioFunctions *callbacks) {
    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));
}
static void registerRILClient(RIL_Client_Type *client)
{
    int ret;

    client->fdListen= android_get_control_socket(client->name);
    if (client->fdListen < 0) {
        RLOGE("Failed to get socket : %s errno:%d", client->name, errno);
        exit(-1);
    }
    ret = listen(client->fdListen, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on ril client socket '%d': %s",
             client->fdListen  , strerror(errno));
        exit(-1);
    }
    ril_event_set (client->listen_event, client->fdListen , false,
                clientListenCallback, client);

    rilEventAddWakeup (client->listen_event);
    RLOGD("RIL client [%s]: fdListen is listening", client->name);
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
        case RIL_SOCKET_2:
            strncpy(socket_name, SOCKET2_NAME_RIL, 9);
            break;
        case RIL_SOCKET_3:
            strncpy(socket_name, SOCKET3_NAME_RIL, 9);
            break;
        case RIL_SOCKET_4:
            strncpy(socket_name, SOCKET4_NAME_RIL, 9);
            break;
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
    int i = 0;

    RLOGI("SIM_COUNT: %d", SIM_COUNT);

    if (callbacks == NULL) {
        RLOGE("RIL_register: RIL_RadioFunctions * null");
        return;
    }
    if (callbacks->version < RIL_VERSION_MIN) {
        RLOGE("RIL_register: version %d is to old, min version is %d",
             callbacks->version, RIL_VERSION_MIN);
        return;
    }
    if (callbacks->version > RIL_VERSION) {
        RLOGE("RIL_register: version %d is too new, max version is %d",
             callbacks->version, RIL_VERSION);
        return;
    }
    RLOGE("RIL_register: RIL version %d", callbacks->version);

    if (s_registerCalled > 0) {
        RLOGE("RIL_register has been called more than once. "
                "Subsequent call ignored");
        return;
    }

    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

    /* Initialize socket1 parameters */
    for (i = 0; i < SIM_COUNT; i++) {
        s_ril_param_socket[i] = {
                            (RIL_SOCKET_ID)(RIL_SOCKET_1+i),             /* socket_id */
                            -1,                       /* fdListen */
                            -1,                       /* fdCommand */
                            PHONE_PROCESS,            /* processName */
                            &s_commands_event[i],        /* commands_event */
                            &s_listen_event[i],          /* listen_event */
                            processCommandsCallback,  /* processCommandsCallback */
                            NULL                      /* p_rs */
                            };
    }

    s_registerCalled = 1;

    RLOGI("s_registerCalled flag set, %d", s_started);
    // Little self-check

    for (int i = 0; i < (int)NUM_ELEMS(s_commands); i++) {
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

    // start listen socket1
    for (i = 0; i < SIM_COUNT; i++) {
        startListen((RIL_SOCKET_ID)(RIL_SOCKET_1+i), &s_ril_param_socket[i]);
    }

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

    s_fdDebug = android_get_control_socket(rildebug);
    if (s_fdDebug < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
        exit(-1);
    }

    ret = listen(s_fdDebug, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on ril debug socket '%d': %s",
             s_fdDebug, strerror(errno));
        exit(-1);
    }
    ril_event_set (&s_debug_event, s_fdDebug, true,
                debugCallback, NULL);

    rilEventAddWakeup (&s_debug_event);
#endif
}

extern "C" void
RIL_registerSocket (const RIL_RadioFunctionsSocket *callbacks) {
    int ret;
    int flags;
    int i = 0;

    RLOGI("SIM_COUNT: %d", SIM_COUNT);

    if (callbacks == NULL) {
        RLOGE("RIL_registerSocket: RIL_RadioFunctions * null");
        return;
    }
    if (callbacks->version < RIL_VERSION_MIN) {
        RLOGE("RIL_registerSocket: version %d is to old, min version is %d",
             callbacks->version, RIL_VERSION_MIN);
        return;
    }
    if (callbacks->version > RIL_VERSION) {
        RLOGE("RIL_registerSocket: version %d is too new, max version is %d",
             callbacks->version, RIL_VERSION);
        return;
    }
    RLOGE("RIL_registerSocket: RIL version %d", callbacks->version);

    if (s_registerCalled > 0) {
        RLOGE("RIL_registerSocket has been called more than once. "
                "Subsequent call ignored");
        return;
    }

    memcpy(&s_callbacksSocket, callbacks, sizeof (RIL_RadioFunctionsSocket));

    /* Initialize socket1 parameters */
    for (i = 0; i < SIM_COUNT; i++) {
        s_ril_param_socket[i] = {
                            (RIL_SOCKET_ID)(RIL_SOCKET_1+i),             /* socket_id */
                            -1,                       /* fdListen */
                            -1,                       /* fdCommand */
                            PHONE_PROCESS,            /* processName */
                            &s_commands_event[i],        /* commands_event */
                            &s_listen_event[i],          /* listen_event */
                            processCommandsCallback,  /* processCommandsCallback */
                            NULL                      /* p_rs */
                            };
    }

    s_registerCalled = 1;

    RLOGI("s_registerCalled flag set, %d", s_started);
    // Little self-check

    for (int i = 0; i < (int)NUM_ELEMS(s_commands); i++) {
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

    // start listen socket1
    for (i = 0; i < SIM_COUNT; i++) {
        startListen((RIL_SOCKET_ID)(RIL_SOCKET_1+i), &s_ril_param_socket[i]);
    }

#if 1
    // start debug interface socket

    char *inst = NULL;
    if (strlen(RIL_getRilSocketName()) >= strlen(SOCKET_NAME_RIL)) {
        inst = RIL_getRilSocketName() + strlen(SOCKET_NAME_RIL);
    }

#ifdef MTK_RIL_MD1
    char rildebug[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_DEBUG;
#else
    char rildebug[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_DEBUG_MD2;
#endif

    s_fdDebug = android_get_control_socket(rildebug);
    if (s_fdDebug < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
        exit(-1);
    }

    ret = listen(s_fdDebug, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on ril debug socket '%d': %s",
             s_fdDebug, strerror(errno));
        exit(-1);
    }
    ril_event_set (&s_debug_event, s_fdDebug, true,
                debugCallback, NULL);

    rilEventAddWakeup (&s_debug_event);
#endif

#ifdef MTK_RIL_MD1
    char rildatci[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_ATCI;
#else
    char rildatci[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_ATCI_MD2;
#endif

    /* atci start */
    s_fdATCI_listen= android_get_control_socket(rildatci);
    if (s_fdATCI_listen < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
    }
    ret = listen(s_fdATCI_listen, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on ril ATCI socket '%d': %s",
             s_fdATCI_listen, strerror(errno));
        exit(-1);
    }
    ril_event_set (&s_ATCIlisten_event, s_fdATCI_listen, true,
                atci_Callback, NULL);

    rilEventAddWakeup (&s_ATCIlisten_event);
    /* atci end */

#ifdef MTK_RIL_MD1
    char rildoem[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_OEM;
#else
    char rildoem[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_OEM_MD2;
#endif

    /* oem start */
    s_fdOem= android_get_control_socket(rildoem);
    if (s_fdOem < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildebug, errno);
        exit(-1);
    }
    ret = listen(s_fdOem, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on ril OEM socket '%d': %s",
             s_fdOem, strerror(errno));
        exit(-1);
    }
    ril_event_set (&s_oem_event, s_fdOem, true,
                oemCallback, NULL);

    rilEventAddWakeup (&s_oem_event);
    /* atci end */

// External SIM [Start]
#ifdef MTK_RIL_MD1
    char rildVsim[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_VSIM;
#else
    char rildVsim[MAX_DEBUG_SOCKET_NAME_LENGTH] = SOCKET_NAME_RIL_VSIM_MD2;
#endif
// External SIM [End]

    // External SIM [START]
    s_fdVsim = android_get_control_socket(rildVsim);
    RLOGD("create vsim socket, fd = %d", s_fdVsim);

    if (s_fdVsim < 0) {
        RLOGE("Failed to get socket : %s errno:%d", rildVsim, errno);
        //exit(-1);
    } else {
        ret = listen(s_fdVsim, 4);

        if (ret < 0) {
            RLOGE("Failed to listen on ril VSIM socket '%d': %s",
                 s_fdVsim, strerror(errno));
            exit(-1);
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        int result = pthread_create(&s_vsim_dispatch, &attr, vsimCallback, NULL);
        if (result != 0) {
            RLOGE("Failed to create dispatch thread: %s", strerror(result));
        }
    }
    // External SIM [End]
    if (isMalSupported()) {
        /* add mal client */
        registerRILClient(&s_mal_client);
        /* add mal at client */
        registerRILClient(&s_mal_at_client);
    }
    RLOGE("finish RIL_registerSocket");
}

extern "C" void
RIL_register_socket (RIL_RadioFunctionsSocket *(*Init)(const struct RIL_EnvSocket *, int, char **),
        RIL_SOCKET_TYPE socketType, int argc, char **argv) {

    RIL_RadioFunctionsSocket* UimFuncs = NULL;

    if(Init) {
        UimFuncs = Init(&RilSapSocket::uimRilEnv, argc, argv);

        switch(socketType) {
            case RIL_SAP_SOCKET:
                RilSapSocket::initSapSocket("sap_uim_socket1", UimFuncs);
                if (SIM_COUNT >= 2) {
                    RilSapSocket::initSapSocket("sap_uim_socket2", UimFuncs);
                }
                if (SIM_COUNT >= 3) {
                    RilSapSocket::initSapSocket("sap_uim_socket3", UimFuncs);
                }
                if (SIM_COUNT >= 4) {
                    RilSapSocket::initSapSocket("sap_uim_socket4", UimFuncs);
                }
        }
    }
}

static int
checkAndDequeueRequestInfo(struct RequestInfo *pRI) {
    int ret = 0;
    /* Hook for current context
       pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = &s_pendingRequestsMutex[pRI->socket_id];
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo ** pendingRequestsHook = &s_pendingRequests[pRI->socket_id];

    if (pRI == NULL) {
        return 0;
    }

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


extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI;
    int ret;
    int fd = -1;
    size_t errorOffset;
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;

    pRI = (RequestInfo *)t;

    socket_id = pRI->socket_id;
    fd = s_ril_param_socket[socket_id].fdCommand;

    RLOGD("RequestComplete, %s", rilSocketIdToString(socket_id));

    if (!checkAndDequeueRequestInfo(pRI)) {
        RLOGE ("RIL_onRequestComplete: invalid RIL_Token");
        return;
    }

    if (pRI->local > 0) {
        // Locally issued command...void only!
        // response does not go back up the command socket
        RLOGD("C[locl]< %s", requestToString(pRI->pCI->requestNumber));

        goto done;
    }

    appendPrintBuf("[%04d]< %s",
        pRI->token, requestToString(pRI->pCI->requestNumber));

    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_RILD_READ_IMSI){
        RLOGD("C[locl]< %s done.", requestToString(pRI->pCI->requestNumber));

        goto done;
    }


    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL){

        LOGD("[THERMAL] local request for THERMAL returned ");
        char* strResult = NULL;
        if(RIL_E_SUCCESS == e){
            asprintf(&strResult, "%s",(char*)response);
        } else {
            asprintf(&strResult, "ERROR");

        }

        if(s_THERMAL_fd > 0){
            LOGD("[THERMAL] s_THERMAL_fd is valid strResult is %s", strResult);

            int len = (int)strlen(strResult);
            ret = send(s_THERMAL_fd, strResult, len, MSG_NOSIGNAL);
            if (ret != len) {
                LOGD("[THERMAL] lose data when send response. ");
            }
            free(strResult);
            goto done;
        } else {
            LOGD("[EAP] s_THERMAL_fd is < 0");
            free(strResult);
            goto done;
        }

    }

    if (pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_SET_MODEM_THERMAL) {
            LOGD("[MDTM_TOG] Not need to send response");
            goto done;
    }

#ifdef MTK_EAP_SIM_AKA
    /*handle response for local request for response*/
    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION
        || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_USIM_AUTHENTICATION){

        LOGD("[EAP] local request for EAP returned ");
        char* strResult = NULL;
        char* res = (char*)response;
        if(RIL_E_SUCCESS == e){
            if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION
                 && strlen(res) > 24){
                strResult = new char[25];
                strResult[0] = '\0';
                if(res[0] == '0' && res[1] == '4'){
                    strncpy(strResult, res+2, 8);
                    if(res[10] == '0' && res[11] == '8'){
                        strncpy(strResult+8, res+12, 16);
                        strResult[24] = '\0';
                    }else{
                        LOGE("The length of KC is not valid.");
                    }
                }else{
                   LOGE("The length of SRES is not valid.");
                }
            }else{
                asprintf(&strResult, "%s",res);
            }
        }else{
            asprintf(&strResult, "ERROR:%s", failCauseToString(e));
        }
        if(s_EAPSIMAKA_fd > 0){
            LOGD("[EAP] s_EAPSIMAKA_fd is valid strResult is %s", strResult);
            int len = (int)strlen(strResult);
            ret = send(s_EAPSIMAKA_fd, &len, sizeof(int), 0);
            if (ret != sizeof(int)) {
                LOGD("Socket write Error: when sending arg length");
            }else{
                ret = send(s_EAPSIMAKA_fd, strResult, len, 0);
                if (ret != len) {
                    LOGD("[EAP]lose data when send response. ");
                }
            }
            close(s_EAPSIMAKA_fd);
            s_EAPSIMAKA_fd = -1;
            free(strResult);
            goto done;
        }

        if (strResult != NULL) {
            free(strResult);
        }
    }
#endif

   if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_GET_SHARED_KEY
      || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS
      || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO
      || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS
      || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_GET_MODEM_STATUS){

        LOGD("[SIM_LOCK] local request for SIM_LOCK returned ");

        char* strResult = NULL;
        if(RIL_E_SUCCESS == e){
            asprintf(&strResult, "%s",(char*)response);
        } else {
            asprintf(&strResult, "ERROR:%d", e);
        }

        if(s_SIMLOCK_fd > 0){
            LOGD("[SIM_LOCK] s_SIMLOCK_fd is valid strResult is %s", strResult);

            int len = (int)strlen(strResult);

            ret = send(s_SIMLOCK_fd, &len, sizeof(int), 0);
            ret = send(s_SIMLOCK_fd, strResult, len, 0);
            if (ret != len) {
                LOGD("[SIM_LOCK] lose data when send response. ");
            }
            close(s_SIMLOCK_fd);
            s_SIMLOCK_fd = -1;
            free(strResult);
            goto done;
        } else {
            LOGD("[SIM_LOCK] s_SIMLOCK_fd is < 0");
            free(strResult);
            goto done;
        }
    }

    // External SIM API [Start]
    if (pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_VSIM_NOTIFICATION) {
        LOGD("[VSIM] local request RIL_LOCAL_REQUEST_VSIM_NOTIFICATION returned, fd: %d", s_fdVsim_command);

        VsimMessage msg;
        msg.transaction_id = 0;
        msg.message_id = MSG_ID_EVENT_RESPONSE;
        msg.slotId = 1 << socket_id;
        msg.length = sizeof(int);

        // Write event response event
        if (blockingWrite(s_fdVsim_command, &msg, sizeof(msg)) < 0) {
            LOGD("[VSIM] local request fail to write msg back. ");
            goto done;
        }

        /** [error cause]
         *  RESPONSE_RESULT_OK = 0,
         *  RESPONSE_RESULT_GENERIC_ERROR = -1,
         *  RESPONSE_RESULT_PLATFORM_NOT_READY = -2,
         */
        int result = (RIL_E_SUCCESS == e) ? 0 : -1;

        // Write event response event result data
        if (blockingWrite(s_fdVsim_command, &result, sizeof(result)) < 0) {
            LOGD("[VSIM] local request fail to write data back. ");
            goto done;
        }

        goto done;
    }

    if (pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_VSIM_OPERATION) {
        LOGD("[VSIM] local request fRIL_LOCAL_REQUEST_VSIM_OPERATION returned, fd: %d", s_fdVsim_command);
        goto done;
    }
    // External SIM API [End]

    /*RIL_REQUEST_OEM_HOOK_RAW for ATCI start*/
    if (s_fdATCI_command > 0 && pRI->pCI->requestNumber == RIL_REQUEST_OEM_HOOK_RAW) {
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
        write_consumed = send(s_fdATCI_command, data, write_size, 0);
        if (write_consumed != write_size) {
            LOGD("[ATCI]lose data when send to atci. errno = %s", strerror(errno));
        }
        goto done;
    }
    /*RIL_REQUEST_OEM_HOOK_RAW for ATCI end*/

    /* RIL_REQUEST_RESUME_REGISTRATION clear backup data start */
    if (pRI->pCI->requestNumber == RIL_REQUEST_RESUME_REGISTRATION) {
        //LOGD("[WPO] ECOPS backup dataSize:<%d>, data:<%s>", s_prevEcopsDataSize[socket_id], s_prevEcopsData[socket_id]);
        if (s_prevEcopsData[socket_id] != NULL) {
            LOGD("[WPO] ECOPS data is not NULL, so clear it.");
            free(s_prevEcopsData[socket_id]);
            s_prevEcopsData[socket_id] = NULL;
            s_prevEcopsDataSize[socket_id] = 0;
        }
        //LOGD("[WPO] EMSR backup dataSize:<%d>, data:<%s>", s_prevEmsrDataSize[socket_id], s_prevEmsrData[socket_id]);
        if (s_prevEmsrData[socket_id] != NULL) {
            LOGD("[WPO] EMSR data is not NULL, so clear it.");
            free(s_prevEmsrData[socket_id]);
            s_prevEmsrData[socket_id] = NULL;
            s_prevEmsrDataSize[socket_id] = 0;
        }
    }
    /* RIL_REQUEST_RESUME_REGISTRATION clear backup data end */

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
                RLOGE ("responseFunction error, ret %d", ret);
                p.setDataPosition(errorOffset);
                p.writeInt32 (ret);
            }
        }

        if (e != RIL_E_SUCCESS) {
            appendPrintBuf("%s fails by %s", printBuf, failCauseToString(e));
        }

        if (fd < 0) {
            RLOGD ("RIL onRequestComplete: Command channel closed");
        }
        if (pRI->client)
        {
            sendClientResponse(p, pRI->client);
        }
        else
        {
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
        //RLOGD("wakeTimeout: releasing wake lock");

        releaseWakeLock();
    } else {
        //RLOGD("wakeTimeout: releasing wake lock CANCELLED");
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
            RLOGD("decodeVoiceRadioTechnology: Invoked with incorrect RadioState");
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
            RLOGD("decodeCdmaSubscriptionSource: Invoked with incorrect RadioState");
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
           RLOGD("decodeSimStatus: Invoked with incorrect RadioState");
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
RIL_RadioState
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
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,
                        &voiceRadioTech, sizeof(voiceRadioTech), socket_id);
        }
        if(is3gpp2(newVoiceRadioTech)) {
            newCdmaSubscriptionSource = decodeCdmaSubscriptionSource(newRadioState);
            if(newCdmaSubscriptionSource != cdmaSubscriptionSource) {
                cdmaSubscriptionSource = newCdmaSubscriptionSource;
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED,
                        &cdmaSubscriptionSource, sizeof(cdmaSubscriptionSource), socket_id);
            }
        }
        newSimStatus = decodeSimStatus(newRadioState);
        if(newSimStatus != simRuimStatus) {
            simRuimStatus = newSimStatus;
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, socket_id);
        }

        /* Send RADIO_ON to telephony */
        newRadioState = RADIO_STATE_ON;
    }

    return newRadioState;
}

extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen)
{
    RIL_onUnsolicitedResponseSocket(unsolResponse, data, datalen, RIL_SOCKET_1);
}

extern "C"
void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id)
{
    int unsolResponseIndex;
    int ret;
    int64_t timeReceived = 0;
    bool shouldScheduleTimeout = false;
    RIL_RadioState newState;
    RIL_SOCKET_ID soc_id = socket_id;
#ifdef MTK_RIL
    int fdCommand = -1;
    WakeType wakeType = WAKE_PARTIAL;
#endif

    if (s_registerCalled == 0) {
        // Ignore RIL_onUnsolicitedResponse before RIL_register
        RLOGW("RIL_onUnsolicitedResponse called before RIL_register");
        return;
    }
    //report the AT URC to mal at channel
    if (RIL_UNSOL_MAL_AT_INFO == unsolResponse)
    {
         //RIL_UNSOL_MAL_AT_INFO will be sent to
         //s_mal_at_client.fdCommand only
         if ((s_mal_at_client.fdCommand < 0) )
         {
            return;
         }
    }
    else if (RIL_UNSOL_MAIN_SIM_INFO == unsolResponse)
    {
        int simID = *(int *) data;
        update3GSimInfo(simID);
        //do not update AFW currently
        return;
    }

    unsolResponseIndex = unsolResponse - RIL_UNSOL_RESPONSE_BASE;

    if ((unsolResponseIndex < 0)
        || (unsolResponseIndex >= (int32_t)NUM_ELEMS(s_unsolResponses))) {
    #ifdef MTK_RIL
        if (unsolResponse > (RIL_UNSOL_VENDOR_BASE + (int32_t)NUM_ELEMS(s_mtk_unsolResponses)))
    #endif /* MTK_RIL */
        {
            RLOGE("unsupported unsolicited response code %d", unsolResponse);
            return;
        }
    }

#ifdef MTK_RIL
    fdCommand = s_ril_param_socket[soc_id].fdCommand;

    if (fdCommand == -1
        // External SIM [Start]
        && unsolResponse != RIL_UNSOL_VSIM_OPERATION_INDICATION) {
        // External SIM [End] { //cache for rilj client
        if (isMalSupported()) {
            if(isMalDongleMode()) {
                if (s_mal_client.fdCommand <0 ||
                    s_mal_at_client.fdCommand <0 ||
                    socket_id != RIL_get3GSimInfo()) {
                    return;
                }
            } else {
                RLOGD("Try to cache request:%s in RIL%s",
                    requestToString(unsolResponse),
                    rilSocketIdToString(soc_id));
                if (unsolResponse == RIL_UNSOL_RESPONSE_PLMN_CHANGED) {
                    cacheEcopsUrc(unsolResponse, data, datalen, soc_id);
                    return;
                }
                cacheUrc(unsolResponse, data, datalen , soc_id);
                return;
            }
        } else {
                RLOGD("Can't send URC because there is no connection yet."\
                    "Try to cache request:%s in RIL%s",
                    requestToString(unsolResponse), rilSocketIdToString(soc_id));
                if (unsolResponse == RIL_UNSOL_RESPONSE_PLMN_CHANGED) {
                    cacheEcopsUrc(unsolResponse, data, datalen, soc_id);
                    return;
                }
                cacheUrc(unsolResponse, data, datalen , soc_id);
                return; //cahche then return
        }
    }

    // External SIM [Start]
    if (s_fdVsim_command == -1 && unsolResponse == RIL_UNSOL_VSIM_OPERATION_INDICATION) {
        RLOGD("Can't send URC because there is no vsim connection yet. Try to cache request:%s in RIL%s", requestToString(unsolResponse), rilSocketIdToString(soc_id));
        cacheVsimUrc(unsolResponse, data, datalen , soc_id);
        return;
    }
    // External SIM [End]
#endif

    //MTK-START [mtk04070][111213][ALPS00093395] ATCI for unsolicited response
    char atci_urc_enable[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.service.atci_urc.enable", atci_urc_enable, "0");
    if ((s_fdATCI_command > 0) &&
        (RIL_UNSOL_ATCI_RESPONSE == unsolResponse) &&
        (atoi(atci_urc_enable) == 1))
    {
        char* responseData = (char*)data;
        RLOGD("[ATCI] data sent to atci is '%s'", responseData);
        unsigned int sendLen = send(s_fdATCI_command, responseData, strlen(responseData), 0);
        if (sendLen != (int) strlen(responseData)) {
            RLOGD("[ATCI]lose data when send to atci.");
        }
    }

    // External SIM [Start]
    if (s_fdVsim_command > 0 && unsolResponse == RIL_UNSOL_VSIM_OPERATION_INDICATION) {

        RIL_VsimOperationEvent *event = (RIL_VsimOperationEvent *) data;

        LOGD("[VSIM] URC RIL_UNSOL_VSIM_OPERATION_INDICATION, eventId = %d", event->eventId);

        VsimMessage msg;
        msg.transaction_id = event->transaction_id;
        msg.slotId = 1 << socket_id;
        if (event->eventId == REQUEST_TYPE_ATR_EVENT) {
            msg.message_id = MSG_ID_UICC_RESET_REQUEST;
            msg.length = 0;
            // Write event response event
            if (blockingWrite(s_fdVsim_command, &msg, sizeof(msg)) < 0) {
                LOGD("[VSIM] local request fail to write msg back. ");
            }
        } else if (event->eventId == REQUEST_TYPE_APDU_EVENT) {
            msg.message_id = MSG_ID_UICC_APDU_REQUEST;
            msg.length = sizeof(event->data_length) + event->data_length;

            // Write event response event
            if (blockingWrite(s_fdVsim_command, &msg, sizeof(msg)) < 0) {
                LOGD("[VSIM] local request fail to write msg back. ");
            }

            // Write APDU length
            if (blockingWrite(s_fdVsim_command, &event->data_length, sizeof(event->data_length)) < 0) {
                LOGD("[VSIM] local request fail to write data_length back. ");
            }

            // Write APDU data
            if (blockingWrite(s_fdVsim_command, event->data, event->data_length) < 0) {
                LOGD("[VSIM] local request fail to write data back. ");
            }
        } else if (event->eventId == REQUEST_TYPE_CARD_POWER_DOWN) {
            msg.message_id = MSG_ID_UICC_POWER_DOWN_REQUEST;
            msg.length = sizeof(int);

            // Write event response event
            if (blockingWrite(s_fdVsim_command, &msg, sizeof(msg)) < 0) {
                LOGD("[VSIM] local request fail to write msg back. ");
            }

            // Use result filed to represnt power down mode
            if (blockingWrite(s_fdVsim_command, &event->result, sizeof(event->result)) < 0) {
                LOGD("[VSIM] local request fail to write data back. ");
            }
        }

        LOGD("[VSIM] URC RIL_UNSOL_VSIM_OPERATION_INDICATION send");
        return;
    }
    // External SIM [End]
    if (fdCommand <0 &&
        s_mal_client.fdCommand <0 &&
        s_mal_at_client.fdCommand <0 &&
        s_fdATCI_command <0)
    {
        return;
    }

   // Grab a wake lock if needed for this reponse,
   // as we exit we'll either release it immediately
   // or set a timer to release it later.
#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
        unsolResponseIndex = unsolResponse - RIL_UNSOL_VENDOR_BASE;
        wakeType = s_mtk_unsolResponses[unsolResponseIndex].wakeType;
    }
    else
#endif /* MTK_RIL */
    {
        wakeType = s_unsolResponses[unsolResponseIndex].wakeType;
    }

    // Grab a wake lock if needed for this reponse,
    // as we exit we'll either release it immediately
    // or set a timer to release it later.
    switch (wakeType) {
        case WAKE_PARTIAL:
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
    p.writeInt32 (unsolResponse);

#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
        ret = s_mtk_unsolResponses[unsolResponseIndex]
              .responseFunction(p, const_cast<void*>(data), datalen);
    } else
#endif /* MTK_RIL */
    {
        ret = s_unsolResponses[unsolResponseIndex]
              .responseFunction(p, const_cast<void*>(data), datalen);
    }

    if (ret != 0) {
        // Problem with the response. Don't continue;
        goto error_exit;
    }

    // some things get more payload
    switch(unsolResponse) {
        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
            newState = processRadioState(callOnStateRequest(soc_id), soc_id);
            p.writeInt32(newState);
            appendPrintBuf("%s {%s}", printBuf,
                radioStateToString(callOnStateRequest(soc_id)));
        break;


        case RIL_UNSOL_NITZ_TIME_RECEIVED:
            // Store the time that this was received so the
            // handler of this message can account for
            // the time it takes to arrive and process. In
            // particular the system has been known to sleep
            // before this message can be processed.
            p.writeInt64(timeReceived);
        break;


        case RIL_UNSOL_RESPONSE_PLMN_CHANGED:
            // Backup ECOPS data in case EOS happened before worldphone can handle it.

            if (s_prevEcopsData[soc_id] != NULL) {
                free(s_prevEcopsData[soc_id]);
                s_prevEcopsData[soc_id] = NULL;
            }

            s_prevEcopsData[soc_id] = calloc(1, p.dataSize());
            s_prevEcopsDataSize[soc_id] = p.dataSize();
            memcpy(s_prevEcopsData[soc_id], p.data(), p.dataSize());
        break;


        case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED:
            // Backup EMSR data in case EOS happened before worldphone can handle it.

            if (s_prevEmsrData[soc_id] != NULL) {
                free(s_prevEmsrData[soc_id]);
                s_prevEmsrData[soc_id] = NULL;
            }

            s_prevEmsrData[soc_id] = calloc(1, p.dataSize());
            s_prevEmsrDataSize[soc_id] = p.dataSize();
            memcpy(s_prevEmsrData[soc_id], p.data(), p.dataSize());
        break;
    }

    if ((RIL_UNSOL_MAL_AT_INFO == unsolResponse) )
    {
        if(s_mal_at_client.fdCommand >= 0 && socket_id == RIL_get3GSimInfo()) {
             //RIL_UNSOL_MAL_AT_INFO is special will be sent to
             //s_mal_at_client.fdCommand only
            char* responseData = (char*) data;
            RLOGD("[MAL_AT] data sent to %s is '%s'", s_mal_at_client.name, responseData);
            unsigned int sendLen = send(s_mal_at_client.fdCommand , responseData, strlen(responseData), 0);
            if (sendLen != (int) strlen(responseData)) {
                RLOGD("[MAL_AT] lose data when send to mal.");
            }
            sendLen = send(s_mal_at_client.fdCommand , "\r\n", 2, 0);
            if (sendLen != 2) {
                RLOGD("[MAL_AT] lose data when send to mal..");
            }
        }
    }
    else
    {
        RLOGI("%s UNSOLICITED: %s length:%d", rilSocketIdToString(soc_id), requestToString(unsolResponse), p.dataSize());
        //check if other client
        if (s_mal_client.fdCommand >=0 && socket_id == RIL_get3GSimInfo()) {
            sendClientUnsolResponse (p, unsolResponse,  data,  datalen, &s_mal_client);
        }
        if (fdCommand >=0) {
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

                s_lastNITZTimeData = calloc(1, p.dataSize());
                s_lastNITZTimeDataSize = p.dataSize();
                memcpy(s_lastNITZTimeData, p.data(), p.dataSize());
            }
        }
    }

    // For now, we automatically go back to sleep after TIMEVAL_WAKE_TIMEOUT
    // FIXME The java code should handshake here to release wake lock

    if (shouldScheduleTimeout) {
#ifdef MTK_RIL
        pthread_mutex_lock(&s_last_wake_mutex);
#endif
        // Cancel the previous request
        if (s_last_wake_timeout_info != NULL) {
            s_last_wake_timeout_info->userParam = (void *)1;
#ifdef MTK_RIL
             RLOGD("s_last_wake_timeout_info: %p, cancel",s_last_wake_timeout_info);
#endif
        }
        s_last_wake_timeout_info
            = internalRequestTimedCallback(wakeTimeoutCallback, NULL,
                                            &TIMEVAL_WAKE_TIMEOUT);
#ifdef MTK_RIL
        pthread_mutex_unlock(&s_last_wake_mutex);
#endif
    }

    // Normal exit
    return;

error_exit:
    if (shouldScheduleTimeout) {
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

    p_info = (UserCallbackInfo *) calloc(1, sizeof(UserCallbackInfo));
    RLOGD("internalRequestTimedCallback malloc p_info: %p", p_info);

    p_info->p_callback = callback;
    p_info->userParam = param;
#ifdef MTK_RIL
    p_info->cid = (RILChannelId)-1;
#endif

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    RLOGD("internalRequestTimedCallback malloc p_info->event: %p", &(p_info->event));

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
        case RADIO_STATE_ON:return"RADIO_ON";
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
        ///M: For query CNAP
        case RIL_REQUEST_SEND_CNAP: return "SEND_CNAP";
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
        case RIL_REQUEST_GET_RADIO_CAPABILITY: return "RIL_REQUEST_GET_RADIO_CAPABILITY";
        case RIL_REQUEST_SET_RADIO_CAPABILITY: return "RIL_REQUEST_SET_RADIO_CAPABILITY";
        case RIL_REQUEST_START_LCE: return "RIL_REQUEST_START_LCE";
        case RIL_REQUEST_STOP_LCE: return "RIL_REQUEST_STOP_LCE";
        case RIL_REQUEST_PULL_LCEDATA: return "RIL_REQUEST_PULL_LCEDATA";
        case RIL_REQUEST_GET_ACTIVITY_INFO: return "RIL_REQUEST_GET_ACTIVITY_INFO";
        case RIL_REQUEST_SET_UICC_SUBSCRIPTION: return "SET_UICC_SUBSCRIPTION";
        case RIL_REQUEST_ALLOW_DATA: return "ALLOW_DATA";
        case RIL_REQUEST_GET_HARDWARE_CONFIG: return "GET_HARDWARE_CONFIG";
        case RIL_REQUEST_SIM_AUTHENTICATION: return "SIM_AUTHENTICATION";
        case RIL_REQUEST_GET_DC_RT_INFO: return "GET_DC_RT_INFO";
        case RIL_REQUEST_SET_DC_RT_INFO_RATE: return "SET_DC_RT_INFO_RATE";
        case RIL_REQUEST_SET_DATA_PROFILE: return "SET_DATA_PROFILE";
        case RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL: return "QUERY_MODEM_THERMAL";
        // Added by M begin
        case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK: return "RIL_REQUEST_QUERY_SIM_NETWORK_LOCK";
        case RIL_REQUEST_SET_SIM_NETWORK_LOCK: return "RIL_REQUEST_SET_SIM_NETWORK_LOCK";
        case RIL_LOCAL_REQUEST_SIM_AUTHENTICATION: return "RIL_LOCAL_REQUEST_SIM_AUTHENTICATION";
        case RIL_LOCAL_REQUEST_USIM_AUTHENTICATION: return "RIL_LOCAL_REQUEST_USIM_AUTHENTICATION";
        case RIL_REQUEST_GENERAL_SIM_AUTH: return "RIL_REQUEST_GENERAL_SIM_AUTH";
        case RIL_REQUEST_SIM_GET_ATR: return "SIM_GET_ATR";
        case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW: return "SIM_OPEN_CHANNEL_WITH_SW";
        case RIL_REQUEST_OPEN_ICC_APPLICATION: return "RIL_REQUEST_OPEN_ICC_APPLICATION";
        case RIL_REQUEST_GET_ICC_APPLICATION_STATUS: return "RIL_REQUEST_GET_ICC_APPLICATION_STATUS";
        case RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED: return "RIL_UNSOL_APPLICATION_SESSION_ID_CHANGED";
        case RIL_REQUEST_SIM_IO_EX: return "SIM_IO_EX";
        case RIL_REQUEST_STK_EVDL_CALL_BY_AP: return "RIL_REQUEST_STK_EVDL_CALL_BY_AP";
        case RIL_REQUEST_BTSIM_CONNECT: return "RIL_REQUEST_BTSIM_CONNECT";
        case RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF: return "RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF";
        case RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM: return "RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM";
        case RIL_REQUEST_BTSIM_TRANSFERAPDU: return "RIL_REQUEST_SEND_BTSIM_TRANSFERAPDU";
        // Added by M end
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
        case RIL_UNSOL_IMS_REGISTRATION_INFO: return "RIL_UNSOL_IMS_REGISTRATION_INFO";
        case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED: return "UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
        case RIL_UNSOL_SRVCC_STATE_NOTIFY: return "UNSOL_SRVCC_STATE_NOTIFY";
        case RIL_UNSOL_HARDWARE_CONFIG_CHANGED: return "HARDWARE_CONFIG_CHANGED";
        case RIL_UNSOL_DC_RT_INFO_CHANGED: return "UNSOL_DC_RT_INFO_CHANGED";
        case RIL_REQUEST_SHUTDOWN: return "SHUTDOWN";
        case RIL_UNSOL_RADIO_CAPABILITY: return "RIL_UNSOL_RADIO_CAPABILITY";
        // Added by M begin
        case RIL_UNSOL_SIM_MISSING: return "UNSOL_SIM_MISSING";
        case RIL_UNSOL_SIM_RECOVERY: return "RIL_UNSOL_SIM_RECOVERY";
        case RIL_UNSOL_VIRTUAL_SIM_ON: return "RIL_UNSOL_VIRTUAL_SIM_ON";
        case RIL_UNSOL_VIRTUAL_SIM_OFF: return "RIL_UNSOL_VIRTUAL_SIM_OFF";
        case RIL_UNSOL_SIM_PLUG_OUT: return "RIL_UNSOL_SIM_PLUG_OUT";
        case RIL_UNSOL_SIM_PLUG_IN: return "RIL_UNSOL_SIM_PLUG_IN";
        case RIL_UNSOL_TRAY_PLUG_IN: return "RIL_UNSOL_TRAY_PLUG_IN";
        case RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED: return "RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED";
        case RIL_UNSOL_EUSIM_READY: return "RIL_UNSOL_EUSIM_READY";
        case RIL_UNSOL_DATA_ALLOWED: return "RIL_UNSOL_DATA_ALLOWED";
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO: return "QUERY_PHB_STORAGE_INFO";
        case RIL_REQUEST_WRITE_PHB_ENTRY: return "WRITE_PHB_ENTRY";
        case RIL_REQUEST_READ_PHB_ENTRY: return "READ_PHB_ENTRY";
        case RIL_REQUEST_QUERY_UPB_CAPABILITY: return "RIL_REQUEST_QUERY_UPB_CAPABILITY";
        case RIL_REQUEST_EDIT_UPB_ENTRY: return "RIL_REQUEST_EDIT_UPB_ENTRY";
        case RIL_REQUEST_DELETE_UPB_ENTRY: return "RIL_REQUEST_DELETE_UPB_ENTRY";
        case RIL_REQUEST_READ_UPB_GAS_LIST: return "RIL_REQUEST_READ_UPB_GAS_LIST";
        case RIL_REQUEST_READ_UPB_GRP: return "RIL_REQUEST_READ_UPB_GRP";
        case RIL_REQUEST_WRITE_UPB_GRP: return "RIL_REQUEST_WRITE_UPB_GRP";
        case RIL_REQUEST_GET_PHB_STRING_LENGTH: return "RIL_REQUEST_GET_PHB_STRING_LENGTH";
        case RIL_REQUEST_GET_PHB_MEM_STORAGE: return "RIL_REQUEST_GET_PHB_MEM_STORAGE";
        case RIL_REQUEST_SET_PHB_MEM_STORAGE: return "RIL_REQUEST_SET_PHB_MEM_STORAGE";
        case RIL_REQUEST_READ_PHB_ENTRY_EXT: return "RIL_REQUEST_READ_PHB_ENTRY_EXT";
        case RIL_REQUEST_WRITE_PHB_ENTRY_EXT: return "RIL_REQUEST_WRITE_PHB_ENTRY_EXT";
        case RIL_REQUEST_RESUME_REGISTRATION: return "RIL_REQUEST_RESUME_REGISTRATION";
        case RIL_UNSOL_RESPONSE_PLMN_CHANGED: return "RIL_UNSOL_RESPONSE_PLMN_CHANGED";
        case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED: return "RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED";
        case RIL_REQUEST_QUERY_MODEM_TYPE: return "RIL_REQUEST_QUERY_MODEM_TYPE";
        case RIL_REQUEST_STORE_MODEM_TYPE: return "RIL_REQUEST_STORE_MODEM_TYPE";
        case RIL_REQUEST_SET_IMS_ENABLE: return "RIL_REQUEST_SET_IMS_ENABLE";
        case RIL_REQUEST_SET_DATA_CENTRIC: return "RIL_REQUEST_SET_DATA_CENTRIC";
         // Remote SIM ME Lock Start
        case RIL_LOCAL_REQUEST_GET_SHARED_KEY: return "RIL_LOCAL_REQUEST_GET_SHARED_KEY";
        case RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS: return "RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS";
        case RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO: return "RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO";
        case RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS: return "RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS";
        case RIL_LOCAL_REQUEST_GET_MODEM_STATUS: return "RIL_LOCAL_REQUEST_GET_MODEM_STATUS";
        case RIL_UNSOL_MELOCK_NOTIFICATION: return "RIL_UNSOL_MELOCK_NOTIFICATION";
        // Remote SIM ME Lock End
        // External SIM [Start]
        case RIL_LOCAL_REQUEST_VSIM_NOTIFICATION: return "RIL_LOCAL_REQUEST_VSIM_NOTIFICATION";
        case RIL_LOCAL_REQUEST_VSIM_OPERATION: return "RIL_LOCAL_REQUEST_VSIM_OPERATION";
        case RIL_UNSOL_VSIM_OPERATION_INDICATION: return "RIL_UNSOL_VSIM_OPERATION_INDICATION";
        // External SIM [End]
        case RIL_UNSOL_STK_EVDL_CALL: return "RIL_UNSOL_STK_EVDL_CALL";
        case RIL_UNSOL_STK_SETUP_MENU_RESET: return "RIL_UNSOL_STK_SETUP_MENU_RESET";
        case RIL_UNSOL_STK_CALL_CTRL: return "RIL_UNSOL_STK_CALL_CTRL";
        // Added by M end
        // IMS
        case RIL_UNSOL_IMS_ENABLE_DONE: return "RIL_UNSOL_IMS_ENABLE_DONE";
        case RIL_UNSOL_IMS_DISABLE_DONE: return "RIL_UNSOL_IMS_DISABLE_DONE";

        //VOLTE data
        case RIL_REQUEST_SETUP_DEDICATE_DATA_CALL: return "RIL_REQUEST_SETUP_DEDICATE_DATA_CALL";
        case RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL: return "RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL";
        case RIL_REQUEST_MODIFY_DATA_CALL: return "RIL_REQUEST_MODIFY_DATA_CALL";
        case RIL_REQUEST_ABORT_SETUP_DATA_CALL: return "RIL_REQUEST_ABORT_SETUP_DATA_CALL";
        case RIL_REQUEST_PCSCF_DISCOVERY_PCO: return "RIL_REQUEST_PCSCF_DISCOVERY_PCO";
        case RIL_UNSOL_DEDICATE_BEARER_ACTIVATED: return "RIL_UNSOL_DEDICATE_BEARER_ACTIVATED";
        case RIL_UNSOL_DEDICATE_BEARER_MODIFIED: return "RIL_UNSOL_DEDICATE_BEARER_MODIFIED";
        case RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED: return "RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED";
        case RIL_REQUEST_CLEAR_DATA_BEARER: return "RIL_REQUEST_CLEAR_DATA_BEARER";
		// M: Fast Dormancy
        case RIL_UNSOL_SCRI_RESULT: return "RIL_UNSOL_SCRI_RESULT";
        case RIL_REQUEST_SET_SCRI: return "RIL_REQUEST_SET_SCRI";
        case RIL_REQUEST_SET_FD_MODE: return "RIL_REQUEST_SET_FD_MODE";
        // SMS part, porting start
        case RIL_REQUEST_WRITE_SMS_TO_SIM: return "RIL_REQUEST_WRITE_SMS_TO_SIM";
        case RIL_REQUEST_DELETE_SMS_ON_SIM: return "RIL_REQUEST_DELETE_SMS_ON_SIM";
        case RIL_REQUEST_GET_SMS_PARAMS: return "RIL_REQUEST_GET_SMS_PARAMS";
        case RIL_REQUEST_SET_SMS_PARAMS: return "RIL_REQUEST_SET_SMS_PARAMS";
        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS: return "RIL_REQUEST_GET_SMS_SIM_MEM_STATUS";
        case RIL_REQUEST_SET_ETWS: return "RIL_REQUEST_SET_ETWS";
        case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO: return "RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO";
        case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO: return "RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO";
        case RIL_REQUEST_GET_CB_CONFIG_INFO: return "RIL_REQUEST_GET_CB_CONFIG_INFO";
        case RIL_REQUEST_REMOVE_CB_MESSAGE: return "RIL_REQUEST_REMOVE_CB_MESSAGE";
        // SMS part, porting end
        case RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_SUPPORT: return "RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_SUPPORT";

        ///M: MTK added Network part start
        case RIL_REQUEST_SET_TRM: return "RIL_REQUEST_SET_TRM";
        case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT : return "QUERY_AVAILABLE_NETWORKS_WITH_ACT";
        case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT: return "SET_NETWORK_SELECTION_MANUAL_WIT_ACT";
        case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS : return "ABORT_QUERY_AVAILABLE_NETWORKS";
        case RIL_REQUEST_GET_FEMTOCELL_LIST: return "RIL_REQUEST_GET_FEMTOCELL_LIST";
        case RIL_REQUEST_ABORT_FEMTOCELL_LIST: return "RIL_REQUEST_ABORT_FEMTOCELL_LIST";
        case RIL_REQUEST_SELECT_FEMTOCELL: return "RIL_REQUEST_SELECT_FEMTOCELL";
        case RIL_REQUEST_GET_POL_CAPABILITY: return "RIL_REQUEST_GET_POL_CAPABILITY";
        case RIL_REQUEST_GET_POL_LIST: return "RIL_REQUEST_GET_POL_LIST";
        case RIL_REQUEST_SET_POL_ENTRY: return "RIL_REQUEST_SET_POL_ENTRY";
        case RIL_UNSOL_NEIGHBORING_CELL_INFO: return "UNSOL_NEIGHBORING_CELL_INFO";
        case RIL_UNSOL_NETWORK_INFO: return "UNSOL_NETWORK_INFO";
        case RIL_UNSOL_FEMTOCELL_INFO: return "UNSOL_FEMTOCELL_INFO";
        case RIL_UNSOL_RESPONSE_ACMT: return "UNSOL_RESPONSE_ACMT";
        case RIL_UNSOL_INVALID_SIM: return "UNSOL_INVALID_SIM";
        case RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED: return "UNSOL_RESPONSE_MMRR_STATUS_CHANGED";
        case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED";
        ///M: MTK added Network part end


        /* M: call control part start */
        case RIL_REQUEST_HANGUP_ALL: return "HANGUP_ALL";
        case RIL_REQUEST_FORCE_RELEASE_CALL: return "FORCE_RELEASE_CALL";
        case RIL_REQUEST_SET_CALL_INDICATION: return "SET_CALL_INDICATION";
        case RIL_REQUEST_EMERGENCY_DIAL: return "EMERGENCY_DIAL";
        case RIL_REQUEST_SET_ECC_SERVICE_CATEGORY: return "SET_ECC_SERVICE_CATEGORY";
        case RIL_REQUEST_SET_ECC_LIST: return "SET_ECC_LIST";
        case RIL_REQUEST_SET_SPEECH_CODEC_INFO: return "SET_SPEECH_CODEC_INFO";

        case RIL_UNSOL_CALL_FORWARDING: return "UNSOL_CALL_FORWARDING";
        case RIL_UNSOL_CRSS_NOTIFICATION: return "UNSOL_CRSS_NOTIFICATION";
        case RIL_UNSOL_INCOMING_CALL_INDICATION: return "UNSOL_INCOMING_CALL_INDICATION";
        case RIL_UNSOL_CIPHER_INDICATION: return "UNSOL_CIPHER_INDICATION";
        case RIL_UNSOL_CNAP: return "UNSOL_CNAP"; //obsolete
        case RIL_UNSOL_SPEECH_CODEC_INFO: return "UNSOL_SPEECH_CODEC_INFO";
        /* M: call control part end */

        /// M: IMS feature. @{
        case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER";
        case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER: return "RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER";
        case RIL_REQUEST_DIAL_WITH_SIP_URI: return "RIL_REQUEST_DIAL_WITH_SIP_URI";
        case RIL_REQUEST_HOLD_CALL: return "RIL_REQUEST_HOLD_CALL";
        case RIL_REQUEST_RESUME_CALL: return "RIL_REQUEST_RESUME_CALL";
        case RIL_UNSOL_ECONF_SRVCC_INDICATION: return "RIL_UNSOL_ECONF_SRVCC_INDICATION";
        case RIL_UNSOL_ECONF_RESULT_INDICATION: return "RIL_UNSOL_ECONF_RESULT_INDICATION";
        /// @}

        case RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO: return "RIL_UNSOL_VOLTE_EPS_NETWORK_FEATURE_INFO";
        case RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION: return "RIL_UNSOL_SRVCC_HANDOVER_INFO_INDICATION";

        case RIL_REQUEST_MODEM_POWEROFF: return "MODEM_POWEROFF";
        case RIL_REQUEST_MODEM_POWERON: return "MODEM_POWERON";
        // M : CC33
        case RIL_REQUEST_SET_DATA_ON_TO_MD: return "RIL_REQUEST_SET_DATA_ON_TO_MD";
        case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE: return "RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE";
        case RIL_UNSOL_REMOVE_RESTRICT_EUTRAN: return "RIL_UNSOL_REMOVE_RESTRICT_EUTRAN";

        case RIL_UNSOL_MO_DATA_BARRING_INFO: return "RIL_UNSOL_MO_DATA_BARRING_INFO";
        case RIL_UNSOL_SSAC_BARRING_INFO: return "RIL_UNSOL_SSAC_BARRING_INFO";
        case RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR: return "RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR";
        case RIL_UNSOL_CALLMOD_CHANGE_INDICATOR: return "RIL_UNSOL_CALLMOD_CHANGE_INDICATOR";
        case RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR: return "RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR";

        case RIL_UNSOL_ABNORMAL_EVENT: return "RIL_UNSOL_ABNORMAL_EVENT";

        /// M: CC071: Add Customer proprietary-IMS RIL interface. @{
        case RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY: return "RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY";
        /// @}

        //MD state change
        case RIL_UNSOL_MD_STATE_CHANGE: return "RIL_UNSOL_MD_STATE_CHANGE";

        case RIL_REQUEST_SET_IMS_CALL_STATUS: return "RIL_REQUEST_SET_IMS_CALL_STATUS";

        /// M: CC072: Add Customer proprietary-IMS RIL interface. @{
        case RIL_REQUEST_SET_SRVCC_CALL_CONTEXT_TRANSFER: return "RIL_REQUEST_SET_SRVCC_CALL_CONTEXT_TRANSFER";
        case RIL_REQUEST_UPDATE_IMS_REGISTRATION_STATUS: return "RIL_REQUEST_UPDATE_IMS_REGISTRATION_STATUS";
        /// @}

        // SMS part
        // SMS ready event
        case RIL_UNSOL_SMS_READY_NOTIFICATION: return "RIL_UNSOL_SMS_READY_NOTIFICATION";
        // Memory storage full
        case RIL_UNSOL_ME_SMS_STORAGE_FULL: return "RIL_UNSOL_ME_SMS_STORAGE_FULL";
        // ETWS primary notification
        case RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION: return "RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION";
        // SMS part

        /// M: IMS VoLTE conference dial feature. @{
        case RIL_REQUEST_CONFERENCE_DIAL: return "RIL_REQUEST_CONFERENCE_DIAL";
        /// @}
        case RIL_REQUEST_RELOAD_MODEM_TYPE: return "RIL_REQUEST_RELOAD_MODEM_TYPE";
        /// [C2K] IRAT feature code start.
        case RIL_REQUEST_SET_ACTIVE_PS_SLOT: return "RIL_REQUEST_SET_ACTIVE_PS_SLOT";
        case RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE: return "RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE";
        case RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE: return "RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        case RIL_REQUEST_DEACTIVATE_LINK_DOWN_PDN: return "RIL_REQUEST_DEACTIVATE_LINK_DOWN_PDN";
        /// [C2K] IRAT feature code finish.
        // M: C2K SVLTE
        case RIL_REQUEST_SET_SVLTE_RAT_MODE: return "RIL_REQUEST_SET_SVLTE_RAT_MODE";
        case RIL_REQUEST_SET_STK_UTK_MODE: return "RIL_REQUEST_SET_STK_UTK_MODE";
        case RIL_UNSOL_STK_CC_ALPHA_NOTIFY: return "UNSOL_STK_CC_ALPHA_NOTIFY";
        case RIL_UNSOL_LCEDATA_RECV: return "RIL_UNSOL_LCEDATA_RECV";
        /// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @{
        case RIL_UNSOL_GMSS_RAT_CHANGED: return "RIL_UNSOL_GMSS_RAT_CHANGED";
        /// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @{
        // IMS
        case RIL_UNSOL_IMS_ENABLE_START: return "RIL_UNSOL_IMS_ENABLE_START";
        case RIL_UNSOL_IMS_DISABLE_START: return "RIL_UNSOL_IMS_DISABLE_START";
        case RIL_UNSOL_IMSI_REFRESH_DONE: return "RIL_UNSOL_IMSI_REFRESH_DONE";
        /// M: For 3G VT only @{
        case RIL_REQUEST_VT_DIAL: return "VT_DIAL";
        case RIL_REQUEST_VOICE_ACCEPT: return "VOICE_ACCEPT";
        case RIL_REQUEST_REPLACE_VT_CALL: return "REPLACE_VT_CALL";
        case RIL_UNSOL_VT_STATUS_INFO: return "UNSOL_VT_STATUS_INFO";
        case RIL_UNSOL_VT_RING_INFO: return "UNSOL_VT_RING_INFO";
        /// @}
        case RIL_REQUEST_SWITCH_ANTENNA: return "RIL_REQUEST_SWITCH_ANTENNA";
        // MD on/off from thermal
        case RIL_LOCAL_REQUEST_SET_MODEM_THERMAL: return "SET_MODEM_THERMAL";
        // M: BIP {
        case RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND: return "UNSOL_STK_BIP_PROACTIVE_COMMAND";
        // M: BIP }
        //WorldMode
        case RIL_UNSOL_WORLD_MODE_CHANGED: return "RIL_UNSOL_WORLD_MODE_CHANGED";
        //Reset Attach APN
        case RIL_UNSOL_SET_ATTACH_APN: return "RIL_UNSOL_SET_ATTACH_APN";
        case RIL_REQUEST_AT_COMMAND_WITH_PROXY: return "RIL_REQUEST_AT_COMMAND_WITH_PROXY";
        case RIL_REQUEST_SWITCH_CARD_TYPE: return "RIL_REQUEST_SWITCH_CARD_TYPE";
        /// M: IMS ViLTE feature. @{
        case RIL_REQUEST_VIDEO_CALL_ACCEPT: return "RIL_REQUEST_VIDEO_CALL_ACCEPT";
        /// @}
        // M: [LTE][Low Power][UL traffic shaping] Start
        case RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT: return "RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT";
        case RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER: return "RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER";
        case RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE: return "RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE";
        // M: [LTE][Low Power][UL traffic shaping] End
        case RIL_REQUEST_HANDOVER_IND: return "RIL_REQUEST_HANDOVER_IND";
        case RIL_REQUEST_WIFI_DISCONNECT_IND: return "RIL_REQUEST_WIFI_DISCONNECT_IND";
        /// M: EPDG feature. Update PS state from MAL @{
        case RIL_REQUEST_MAL_PS_RGEGISTRATION_STATE: return "RIL_REQUEST_MAL_PS_RGEGISTRATION_STATE";
        /// @}
        case RIL_UNSOL_SETUP_DATA_CALL_RESPONSE: return "RIL_UNSOL_SETUP_DATA_CALL_RESPONSE";
        default: return "<unknown request>";
    }
}

const char *
rilSocketIdToString(RIL_SOCKET_ID socket_id)
{
    switch(socket_id) {
        case RIL_SOCKET_1:
            return "RIL_SOCKET_1";
        case RIL_SOCKET_2:
            return "RIL_SOCKET_2";
        case RIL_SOCKET_3:
            return "RIL_SOCKET_3";
        case RIL_SOCKET_4:
            return "RIL_SOCKET_4";
        default:
            return "not a valid RIL";
    }
}

int isDualTalkMode() {
    if (isEVDODTSupport()) {
        return (getExternalModemSlotTelephonyMode() == 0);
    }

    int telephonyMode = getTelephonyMode();
    if (telephonyMode == 0) {
        char property_value1[PROPERTY_VALUE_MAX] = { 0 };
        char property_value2[PROPERTY_VALUE_MAX] = { 0 };
        property_get("rild3.libpath", property_value1, "");
        property_get("rild3.libargs", property_value2, "");
        return (strlen(property_value1) > 0) && (strlen(property_value2) > 0);
    } else if (telephonyMode >= 5) {
        return 1;
    } else {
        return 0;
    }
}

/*********************************
    [Telephony Mode Definition]
        0: default, not MT6589
        1: W+G Gemini
        2: T+G Gemini
        3: FDD Single SIM
        4: TDD Single SIM
        5: W+T DualTalk
        6: T+G DualTalk
        7: W+G DualTalk
        8: G+G DualTalk
*********************************/
int getTelephonyMode() {
    int telephonyMode = -1;
    char mode[PROPERTY_VALUE_MAX] = {0};
    property_get("ril.telephony.mode", mode, "0");

    if (strlen(mode) > 0)
        telephonyMode = atoi(mode);
    else
        telephonyMode = 0;
    return telephonyMode;
}

int isGeminiMode()
{
    int telephonyMode = getTelephonyMode();
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int current_share_modem = 0;

    if (telephonyMode == 0)
    {
        property_get(PROPERTY_RIL_CURRENT_SHARE_MODEM, property_value, "1");
        current_share_modem = atoi(property_value);
        switch (current_share_modem) {
            case 1:
                return 0;
            case 2:
                return 1;
        }
    } else if (telephonyMode < 3) {
        LOGD("isGeminiMode 1");
        return 1;
    } else if (telephonyMode >= 100) {
        LOGD("isGeminiMode 1 for EVDO case");
        return 1;
    }
    return 0;
}

/// M: SVLTE solution2 modification. @{
/**
 * Get slot id which connect to c2k rild socket.
 * @return slot id which connect to c2k rild socket
 *         1 : slot1 connect to c2k rild socket
 *         2 : slot2 connect to c2k rild socket
 */
int getCdmaSocketSlotId() {
    int ret = 0;
    char tempstr[PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.radio.cdma_slot", tempstr, "1");

    ret = atoi(tempstr);
    RLOGI("getCdmaSocketSlotId : %d", ret);
    return ret;
}

int isSvlteSupport() {
    int isSvlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_support", property_value, "0");
    isSvlteSupport = atoi(property_value);
    RLOGI("isSvlteSupport: %d", isSvlteSupport);
    return isSvlteSupport;
}

int getExternalModemSlot() {
    if (isSvlteSupport() == 1) {
        return getCdmaSocketSlotId() - 1;
    }
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.external.md", property_value, "0");
    return atoi(property_value)-1;
}

int isInternationalRoamingEnabled() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.evdo.irsupport", property_value, "0");
    return atoi(property_value);
}

int isSingleMode()
{
    int telephonyMode = getTelephonyMode();
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int current_share_modem = 0;

    if (telephonyMode == 0) {
        property_get(PROPERTY_RIL_CURRENT_SHARE_MODEM, property_value, "1");
        current_share_modem = atoi(property_value);
        switch (current_share_modem) {
            case 1:
                return 1;
            case 2:
                return 0;
        }
    } else if (telephonyMode == 3 || telephonyMode == 4) {
        return 1;
    }
    return 0;
}

int getExternalModemSlotTelephonyMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    if (getExternalModemSlot() == 0) {
        property_get("mtk_telephony_mode_slot1", property_value, "0");
    } else {
        property_get("mtk_telephony_mode_slot2", property_value, "0");
    }
    return atoi(property_value);
}

int isEVDODTSupport()
{
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_c2k_support", property_value, "0");
    return atoi(property_value);
}

int isImsSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_ims_support", property_value, "0");
    return atoi(property_value);
}

int isEpdgSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_epdg_support", property_value, "0");
    return atoi(property_value);
}

int isReqFromMAL() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.data.mal", property_value, "0");
    return atoi(property_value);
}

int isBootupWith3GCapability() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    if (isEVDODTSupport()) {
        if (getExternalModemSlot() == 0) {
            property_get("mtk_telephony_mode_slot2", property_value, "1");
        } else {
            property_get("mtk_telephony_mode_slot1", property_value, "1");
        }
    }
    return atoi(property_value);
}

int getRilProxysNum()
{
    //return RIL_SUPPORT_PROXYS;
    int supportProxy = 0;
    switch(SIM_COUNT) {
        case 2:
            supportProxy = RIL_PROXY_SET3_OFFSET;
            break;
        case 3:
            supportProxy = RIL_PROXY_SET4_OFFSET;
            break;
        case 4:
            supportProxy = RIL_SUPPORT_PROXYS;
            break;
        case 1:
        default:
            supportProxy = RIL_PROXY_OFFSET;
            break;
    }
    return supportProxy;
}

int getSimCount() {
    return SIM_COUNT;
}

extern "C"
void RIL_MDStateChange(RIL_MDState state)
{
    int i = 0;
    int state_tmp = (int) state;

    /* for each connected socket, rild should send onsolicited notification to RILJ(phone)*/
    for (i=0; i < SIM_COUNT; i++)
    {
        /*if the socket is not connected (-1), the unsol message will be ignore*/
        RIL_onUnsolicitedResponseSocket(RIL_UNSOL_MD_STATE_CHANGE, &state_tmp, sizeof(int),  (RIL_SOCKET_ID) (RIL_SOCKET_1+i));
    }
}

/// M: For 3G VT only @{
extern "C"
void RIL_UpdateToVT(RIL_VT_MsgType type, RIL_VT_MsgParams param)
{
    int ret;
    int paramLen =0;

    if(s_VT_fd > 0){
        LOGD("[VT] s_VT_fd is valid");

        // msgType is defined as int in VT Service, so just passing the value.
        int msgType = (int) type;
        ret = send(s_VT_fd, (const void *) &msgType, sizeof(int), 0);
        LOGD("[VT] send msgType ret = %d", ret);
        if (sizeof(int) != ret) {
            goto failed;
        }

        paramLen = sizeof(RIL_VT_MsgParams);
        ret = send(s_VT_fd, (const void *)&paramLen, sizeof(paramLen), 0);
        LOGD("[VT] send paramLen ret = %d", ret);
        if (sizeof(paramLen) != ret) {
            goto failed;
        }

        ret = send(s_VT_fd, (const void *) &param, paramLen, 0);
        LOGD("[VT] send msgParam ret = %d", ret);
        if (paramLen != ret) {
            goto failed;
        }
    } else {
        LOGD("[VT] s_VT_fd is < 0");
    }
    return;
failed:
    LOGD("[VT] s_VT_fd send fail");
}
/// @}

} /* namespace android */

void rilEventAddWakeup_helper(struct ril_event *ev) {
    android::rilEventAddWakeup(ev);
}

void listenCallback_helper(int fd, short flags, void *param) {
    android::listenCallback(fd, flags, param);
}

int blockingWrite_helper(int fd, void *buffer, size_t len) {
    return android::blockingWrite(fd, buffer, len);
}
