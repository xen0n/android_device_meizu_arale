/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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
#include <hardware_legacy/power.h>

#include <telephony/mtk_ril.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <binder/Parcel.h>
#include <pwd.h>

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
#include <sys/socket.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <utils/Log.h>
#include <rilc.h>
#include "ril_ex.h"

namespace android {

/************************************************************/
#define PHONE_PROCESS "radio"
#define SOCKET_NAME_IMS_RIL "rild-ims"
// match with constant in ImsRILAdapter.java
#define MAX_COMMAND_BYTES (8 * 1024)
/* Negative values for private RIL errno's */
#define RIL_ERRNO_INVALID_RESPONSE -1
#define PRINTBUF_SIZE 8096
#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])
#define ANDROID_WAKE_LOCK_NAME "ims-radio-interface"
/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1
#define IMS_COUNT (RIL_SOCKET_NUM_TOTAL)

/************************************************************/
typedef struct ImsResponseList {
    int id;
    char* data;
    size_t datalen;
    RIL_SOCKET_ID socket_id;
    ImsResponseList *pNext;
} ImsResponseList;

static ImsResponseList* queuedUrcList = NULL;
/************************************************************/

static SocketListenParam s_ril_param_socket[IMS_COUNT];
static struct ril_event s_commands_event[IMS_COUNT];
static struct ril_event s_listen_event[IMS_COUNT];

static pthread_mutex_t s_pendingRequestsMutex[IMS_COUNT] = {PTHREAD_MUTEX_INITIALIZER};
static RequestInfo *s_pendingRequests[IMS_COUNT] = {NULL};

static UserCallbackInfo *IMS_s_last_wake_timeout_info = NULL;
static AtResponseList* pendedUrcList1 = NULL;

#if RILC_LOG
    static char printBuf[PRINTBUF_SIZE];

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
/************************************************************/
static pthread_mutex_t s_ims_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_ims_last_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
static int s_ims_fdCommand = -1;
static int s_ims_fdListen = -1;
RIL_RadioFunctions ims_s_callbacks = {0, NULL, NULL, NULL, NULL, NULL};
RIL_RadioFunctionsSocket ims_s_callbacksSocket = {0, NULL, NULL, NULL, NULL, NULL};
static int s_ims_registerCalled = 0;
static const struct timeval TIMEVAL_WAKE_TIMEOUT = {1,0};
static int isSrvccProcedure = 0;
/**********************************************/
#if 1
static void dispatchNotSupport(Parcel& p, RequestInfo *pRI);
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
static void dispatchSmsParams(Parcel &p, RequestInfo *pRI);

static int responseSimStatus(Parcel &p, void *response, size_t responselen);
static int responseCallList(Parcel &p, void *response, size_t responselen);
static int responseSMS(Parcel &p, void *response, size_t responselen);
static int responseSIM_IO(Parcel &p, void *response, size_t responselen);
static int responseCallForwards(Parcel &p, void *response, size_t responselen);
static int responseDataCallList(Parcel &p, void *response, size_t responselen);
static int responseSetupDataCall(Parcel &p, void *response, size_t responselen);
static int responseCellList(Parcel &p, void *response, size_t responselen);
static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responsePhbEntries(Parcel &p,void *response, size_t responselen);
static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen);
static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen);
static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen);
static int responseSmsParams(Parcel &p, void *response, size_t responselen);
static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen);
//MTK-START [mtk80776] WiFi Calling
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI);
static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI);
#endif
static int responseNotSupport(Parcel &p, void *response, size_t responselen);
static int responseRilSignalStrength(Parcel & p,void * response,size_t responselen);
static int responseDataCallList(Parcel & p,void * response,size_t responselen);
static int responseSsn(Parcel & p,void * response,size_t responselen);
static int responseSimRefresh(Parcel & p,void * response,size_t responselen);
static int responseCdmaSms(Parcel & p,void * response,size_t responselen);
static int responseCdmaCallWaiting(Parcel & p,void * response,size_t responselen);
static int responseCdmaInformationRecords(Parcel & p,void * response,size_t responselen);
static int responseCellInfoList(Parcel & p,void * response,size_t responselen);
static int responseHardwareConfig(Parcel & p,void * response,size_t responselen);
static int responseDcRtInfo(Parcel & p,void * response,size_t responselen);
static int responseRadioCapability(Parcel &p, void *response, size_t responselen);
static int responseSSData(Parcel &p, void *response, size_t responselen);
static int responseLceStatus(Parcel &p, void *response, size_t responselen);
static int responseLceData(Parcel &p, void *response, size_t responselen);
static int responseActivityData(Parcel &p, void *response, size_t responselen);
static int responseEtwsNotification(Parcel & p,void * response,size_t responselen);
static int responseSetupDedicateDataCall(Parcel & p,void * response,size_t responselen);
static int responseCrssN(Parcel & p,void * response,size_t responselen);
static int responseEpcNetworkFeatureInfo(Parcel & p,void * response,size_t responselen);
static int responseInts(Parcel &p, void *response, size_t responselen);
static int responseFailCause(Parcel &p, void *response, size_t responselen);
static int responseString(Parcel &p, void *response, size_t responselen);
static int responseStrings(Parcel &p, void *response, size_t responselen);
static int responseVoid(Parcel &p, void *response, size_t responselen);
static int responseCallRing(Parcel &p, void *response, size_t responselen);
static int responseRaw(Parcel &p, void *response, size_t responselen);
static void writeStringToParcel(Parcel &p, const char *s);
static int sendResponse (Parcel &p, int socket_fd);
static int sendResponseRaw (const void *data, size_t dataSize, int sock_fd);
static void rilEventAddWakeup(struct ril_event *ev);
static void IMS_sendPendedUrcs(RIL_SOCKET_ID socket_id, int fdCommand);
static void IMS_queueImsUrc(int unsolResponse, void *data, size_t datalen, RIL_SOCKET_ID socket_id);
static void IMS_dequeueImsUrc();
static int IMS_RIL_get3GSIM();
/// M: [C2K] IRAT feature. @{
static int responseIratStateChange(Parcel &p, void *response, size_t responselen);
/// @}
//External SIM [Start]
static int responseVsimOperationEvent(Parcel &p, void *response, size_t responselen);
//External SIM [End]
static int responseUnsolSetupDataCall(Parcel &p, void *response, size_t responselen);

extern "C" void IMS_RIL_onUnsolicitedResponseSocket(int unsolResponse, void *data, size_t datalen, RIL_SOCKET_ID socket_index);
/**********************************************/
static UserCallbackInfo * IMS_internalRequestTimedCallback
    (RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);
/**********************************************/
/** Index == requestNumber */
CommandInfo ims_s_commands[] = {
#include "ril_commands.h"
};

UnsolResponseInfo ims_s_unsolResponses[] = {
#include "ril_unsol_commands.h"
};
#ifdef MTK_RIL
CommandInfo ims_s_mtk_commands[] = {
#include "mtk_ril_commands.h"
//{0, NULL, NULL, RIL_SUPPORT_PROXYS}
};

UnsolResponseInfo ims_s_mtk_unsolResponses[] = {
//#include "ims_ril_unsol_commands_mtk.h"
#include "mtk_ril_unsol_commands.h"
};
#endif

#if 1
/********************************************************************/
static void callOnRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    if (ims_s_callbacks.onRequest != NULL) {
         /* old vendor ril */
         RLOGD("callOnRequest for old vendor ril");
         ims_s_callbacks.onRequest(request, data, datalen, t);
    } else if (ims_s_callbacksSocket.onRequest != NULL) {
         /* new vendor ril */
         RLOGD("callOnRequest for new vendor ril");
         ims_s_callbacksSocket.onRequest(request, data, datalen, t, socket_id);
    }
}

static char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static void
invalidCommandBlock (RequestInfo *pRI) {
    RLOGE("IMS: invalid command block for token %d request %s",
                pRI->token, requestToString(pRI->pCI->requestNumber));
}

static int callForRilVersion()
{
    int version = 0;
    if (ims_s_callbacks.version > 0) {
        version = ims_s_callbacks.version;
        RLOGD("callForRilVersion old vendor RIL, version: %d", version);
    } else if (ims_s_callbacksSocket.version > 0){
        version = ims_s_callbacksSocket.version;
        RLOGD("callForRilVersion new vendor RIL, version: %d", version);
    }
    return version;
}
/********************************************************************/
static void
dispatchNotSupport(Parcel& p, RequestInfo *pRI) {
    RLOGD("IMS: request not support in IMS !!");
    return;
}

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

static void
dispatchSmsWrite (Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
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

static void
dispatchSIM_IO (Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchSIM_IO_EX (Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchSIM_APDU (Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
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
    dispatchNotSupport(p, pRI);
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
    dispatchNotSupport(p, pRI);
    return 0;
}

static void
dispatchCdmaSms(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchImsCdmaSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchImsGsmSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchImsSms(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void
dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchDataCall(Parcel& p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchPhbEntry(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchWritePhbEntryExt(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchVoiceRadioTech(Parcel& p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchCdmaSubscriptionSource(Parcel& p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchSetInitialAttachApn(Parcel &p, RequestInfo *pRI)
{
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchNVReadItem(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchFD_Mode(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchSimAuth(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchNVWriteItem(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}


static void dispatchUiccSubscripton(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}
/*
static void readQosFromParcel(Parcel* p, Qos* qos) {
    dispatchNotSupport(p, pRI);
    return;
}

static void readTftFromParcel(Parcel* p, Tft* tft) {
    dispatchNotSupport(p, pRI);
    return;
}
*/
static void dispatchSetupDedicateDataCall(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchModifyDedicateDataCall(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchSimAuthentication(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchDataProfile(Parcel &p, RequestInfo *pRI) {
//MTK no use
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchRadioCapability(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchSmsParams(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

//MTK-START [mtk80776] WiFi Calling @{
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}

static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI) {
    dispatchNotSupport(p, pRI);
    return;
}
// @}

/********************************************************************/
static int responseSimStatus(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responsePhbEntries(Parcel &p,void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSIM_IO(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCallForwards(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCallList(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);

}

static int responseSMS(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    return responseNotSupport(p, response, responselen);
}

static int responseCellList(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSmsParams(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}
/********************************************************************/
#endif

static int responseNotSupport(Parcel &p, void *response, size_t responselen) {
    RLOGE("IMS: response URC not support in IMS !!");
    return RIL_ERRNO_INVALID_RESPONSE;
}

static int responseRilSignalStrength(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseDataCallList(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSsn(Parcel & p,void * response,size_t responselen) {
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

static int responseSimRefresh(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCdmaSms(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCdmaCallWaiting(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCdmaInformationRecords(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCellInfoList(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseHardwareConfig(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseRadioCapability(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSSData(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseLceStatus(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseLceData(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseActivityData(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseDcRtInfo(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseEtwsNotification(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseSetupDedicateDataCall(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseCrssN(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseEpcNetworkFeatureInfo(Parcel & p,void * response,size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static int responseInts(Parcel &p, void *response, size_t responselen) {
    int numInts;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        RLOGE("invalid response length %d expected multiple of %d\n",
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

static int responseString(Parcel &p, void *response, size_t responselen) {
    /* one string only */
    startResponse;
    appendPrintBuf("%s%s", printBuf, (char*)response);
    closeResponse;

    writeStringToParcel(p, (const char *)response);

    return 0;
}

static int responseStrings(Parcel &p, void *response, size_t responselen) {
    int numStrings;

    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        RLOGE("invalid response length %d expected multiple of %d\n",
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

static int responseVoid(Parcel &p, void *response, size_t responselen) {
    startResponse;
    removeLastChar;
    return 0;
}

static int responseCallRing(Parcel &p, void *response, size_t responselen) {
    if ((response == NULL) || (responselen == 0)) {
        return responseVoid(p, response, responselen);
    } else {
        RLOGE("responseCallRing : not implement !!");
        return 0;
        //return responseCdmaSignalInfoRecord(p, response, responselen);
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

// [C2K] IRAT feature.
static int responseIratStateChange(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

//External SIM [Start]
static int responseVsimOperationEvent(Parcel &p, void *response, size_t responselen) {
    return 0;
}
//External SIM [End]

static int responseUnsolSetupDataCall(Parcel &p, void *response, size_t responselen) {
    return responseNotSupport(p, response, responselen);
}

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

static void grabPartialWakeLock() {
    acquire_wake_lock(PARTIAL_WAKE_LOCK, ANDROID_WAKE_LOCK_NAME);
}

static void releaseWakeLock() {
    release_wake_lock(ANDROID_WAKE_LOCK_NAME);
}

/**
 * Timer callback to put us back to sleep before the default timeout
 */
static void
wakeTimeoutCallback (void *param) {
    RLOGD("IMS: wakeTimeoutCallback and releaseWakeLock()");
    // We're using "param != NULL" as a cancellation mechanism
    if (param == NULL) {
        //RLOGD("wakeTimeout: releasing wake lock");

        releaseWakeLock();
    } else {
        //RLOGD("wakeTimeout: releasing wake lock CANCELLED");
    }
}

static void onCommandsSocketClosed(int socket_id) {
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
}

const char *
IMS_rilSocketIdToString(RIL_SOCKET_ID socket_id) {
    switch(socket_id) {
        case IMS_RIL_SOCKET_1:
            return "IMS_RIL_SOCKET_1";
        default:
            return "not a valid RIL";
    }
}

static int processCommandBuffer(void *buffer, size_t buflen, RIL_SOCKET_ID socket_id) {
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

    RLOGD("IMS: SOCKET %s REQUEST: %s length:%d", IMS_rilSocketIdToString(socket_id), requestToString(request), buflen);

    if (status != NO_ERROR) {
        RLOGE("invalid request block");
        return 0;
    }


    if (request < 1 || request >= (int32_t)NUM_ELEMS(ims_s_commands)) {
#ifdef MTK_RIL
        if (request > (RIL_REQUEST_VENDOR_BASE + (int32_t)NUM_ELEMS(ims_s_mtk_commands)) ||
            (request >= (int32_t)NUM_ELEMS(ims_s_commands) && request < RIL_REQUEST_VENDOR_BASE))
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
            RLOGE("IMS error on reading command socket errno:%d\n", errno);
        } else {
            RLOGW("IMS EOS.  Closing command socket.");
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

static void listenCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    int fdCommand = -1;
    RecordStream *p_rs;
    SocketListenParam *p_info = (SocketListenParam *)param;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);

    struct passwd *pwd = NULL;

    assert (*p_info->fdCommand < 0);
    assert (fd == *p_info->fdListen);

    fdCommand = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (fdCommand < 0 ) {
        RLOGE("IMS: Error on accept() errno:%d", errno);
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
                RLOGE("IMS: RILD can't accept socket from process %s", pwd->pw_name);
            }
        } else {
            RLOGE("IMS: Error on getpwuid() errno: %d", errno);
        }
    } else {
        RLOGD("IMS: Error on getsockopt() errno: %d", errno);
    }

    if (!is_phone_socket) {
      RLOGE("IMS: RILD must accept socket from %s", p_info->processName);

      close(fdCommand);
      fdCommand = -1;

      onCommandsSocketClosed(p_info->socket_id);

      // start listening for new connections again
      rilEventAddWakeup(p_info->listen_event);

      return;
    }

    ret = fcntl(fdCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }

    RLOGI("IMS: new connection to %s", IMS_rilSocketIdToString(p_info->socket_id));

    p_info->fdCommand = fdCommand;

    p_rs = record_stream_new(p_info->fdCommand, MAX_COMMAND_BYTES);

    p_info->p_rs = p_rs;

    ril_event_set (p_info->commands_event, p_info->fdCommand, 1,
        p_info->processCommandsCallback, p_info);

    rilEventAddWakeup (p_info->commands_event);

    //onNewCommandConnect(p_info->socket_id);
#ifdef MTK_RIL
    IMS_sendPendedUrcs(p_info->socket_id, p_info->fdCommand);
#endif
}

static void rilEventAddWakeup(struct ril_event *ev) {
RLOGI("IMS: Start to rilEventAddWakeup");
    ril_event_add(ev);
    triggerEvLoop();
}

static void startListen(RIL_SOCKET_ID socket_id, SocketListenParam* socket_listen_p) {
    int fdListen = -1;
    int ret;
    char socket_name[10];

    memset(socket_name, 0, sizeof(char)*10);

    switch(socket_id) {
        case IMS_RIL_SOCKET_1:
            strncpy(socket_name, SOCKET_NAME_IMS_RIL, sizeof(SOCKET_NAME_IMS_RIL));
            break;
        default:
            RLOGE("IMS Socket id is wrong!!");
            return;
    }

    RLOGI("IMS: Start to listen %s", IMS_rilSocketIdToString(socket_id));

    fdListen = android_get_control_socket(socket_name);
    if (fdListen < 0) {
        RLOGE("IMS: Failed to get socket %s", socket_name);
        //prevent to exit, just print log for debugging
        //exit(-1);
        return;
    }

    ret = listen(fdListen, 4);

    if (ret < 0) {
        RLOGE("IMS: Failed to listen on control socket '%d': %s",
             fdListen, strerror(errno));
        //prevent to exit, just print log for debugging
        //exit(-1);
        return;
    }
    socket_listen_p->fdListen = fdListen;

    /* note: non-persistent so we can accept only one connection at a time */


    ril_event_set(socket_listen_p->listen_event, fdListen, false,
                listenCallback, socket_listen_p);

    rilEventAddWakeup(socket_listen_p->listen_event);
}

static int blockingWrite(int fd, const void *buffer, size_t len) {
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

    return 0;
}

extern "C"
void IMS_RILA_register (const RIL_RadioFunctionsSocket *callbacks) {
    int ret;
    int flags;
    int i = 0;

    RLOGD("IMS: IMS_RILA_register begin");

    if (s_ims_registerCalled > 0) {
        RLOGE("IMS_RILA_register has been called more than once. "
                "Subsequent call ignored");
        return;
    }

    //memcpy(&ims_s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

    /* Initialize socket1 parameters */
    for (i = IMS_RIL_SOCKET_1; i < RIL_SOCKET_NUM_TOTAL; i++) {
        s_ril_param_socket[i] = {
                            (RIL_SOCKET_ID)(0+i),  /* ims socket_index => 0 */
                            -1,                       /* fdListen */
                            -1,                       /* fdCommand */
                            PHONE_PROCESS,            /* processName */
                            &s_commands_event[i],        /* commands_event */
                            &s_listen_event[i],          /* listen_event */
                            processCommandsCallback,  /* processCommandsCallback */
                            NULL                      /* p_rs */
                            };
    }

    s_ims_registerCalled = 1;
    RLOGI("IMS: s_ims_registerCalled flag set !!");

    // start listen socket1
    for (i = IMS_RIL_SOCKET_1; i < RIL_SOCKET_NUM_TOTAL; i++) {
        startListen((RIL_SOCKET_ID)(RIL_SOCKET_1+i), &s_ril_param_socket[i]);
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

static int sendResponse (Parcel &p, int socket_id) {
    //printResponse;
    return sendResponseRaw(p.data(), p.dataSize(), socket_id);
}

static int sendResponseRaw (const void *data, size_t dataSize, int socket_id) {
    int fd = s_ril_param_socket[socket_id].fdCommand;
    int ret;
    uint32_t header;
    pthread_mutex_t * writeMutexHook = &s_ims_writeMutex;

    RLOGE("Send Response to %s", SOCKET_NAME_IMS_RIL);

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

static void IMS_cacheUrc(int unsolResponse, void *data, size_t datalen, int socket_id){
    //Only the URC list we wanted.
    if (unsolResponse != RIL_UNSOL_IMS_REGISTRATION_INFO
        && unsolResponse != RIL_UNSOL_IMS_ENABLE_DONE
        && unsolResponse != RIL_UNSOL_IMS_DISABLE_DONE
        && unsolResponse != RIL_UNSOL_IMS_ENABLE_START
        && unsolResponse != RIL_UNSOL_IMS_DISABLE_START) {
        RLOGI("Don't need to cache the request = %d ", unsolResponse);
        return;
    }
    AtResponseList* urcCur = NULL;
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;

    switch(socket_id) {
        case 0:
            urcCur = pendedUrcList1;
            break;
        default:
            RLOGE("IMS: Socket id is wrong!!");
            return;
    }
    while (urcCur != NULL) {
        RLOGD("IMS: Pended URC:%d, IMS_RILD:%d, :%s",
            pendedUrcCount,
            0,
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
            case 0:
                pendedUrcList1 = urcCur;
                break;
            default:
                RLOGE("IMS: Socket id is wrong!!");
                return;
        }
    }
    RLOGD("IMS: Current pendedUrcCount = %d", pendedUrcCount + 1);
}

static void IMS_sendUrc(RIL_SOCKET_ID socket_id, AtResponseList* urcCached) {
    AtResponseList* urc = urcCached;
    AtResponseList* urc_temp;
    while (urc != NULL) {
        RLOGD("IMS: sendPendedUrcs IMS_RIL:%d, %s",
        socket_id,
        requestToString(urc->id));
        IMS_RIL_onUnsolicitedResponseSocket (urc->id, urc->data, urc->datalen, socket_id);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

static void IMS_sendPendedUrcs(RIL_SOCKET_ID socket_id, int fdCommand) {
    RLOGD("IMS: Ready to send pended URCs, socket:%d, fdCommand:%d", socket_id, fdCommand);
    if ((IMS_RIL_SOCKET_1 == socket_id) && (fdCommand != -1)) {
        IMS_sendUrc(socket_id, pendedUrcList1);
        pendedUrcList1 = NULL;
    }
}

extern "C"
void IMS_RIL_onUnsolicitedResponse(int unsolResponse, void *data, size_t datalen, RIL_SOCKET_ID socket_id)
{
    if (isSrvccProcedure) {
        IMS_queueImsUrc(unsolResponse, data, datalen, socket_id);
    } else {
        IMS_RIL_onUnsolicitedResponseSocket(unsolResponse, data, datalen, IMS_RIL_SOCKET_1);
    }
}

extern "C"
void IMS_RIL_onUnsolicitedResponseSocket(int unsolResponse, void *data, size_t datalen, RIL_SOCKET_ID socket_index)
{
    int unsolResponseIndex;
    int ret;
    int64_t timeReceived = 0;
    bool shouldScheduleTimeout = false;
    RIL_RadioState newState;
    RIL_SOCKET_ID soc_id = socket_index;
    //int soc_id = socket_index;
#ifdef MTK_RIL
    int fdCommand = -1;
    WakeType wakeType = WAKE_PARTIAL;
#endif

    if (s_ims_registerCalled == 0) {
        // Ignore RIL_onUnsolicitedResponse before RIL_register
        RLOGW("IMS_RIL_onUnsolicitedResponse called before IMS_RILA_register");
        return;
    }

    unsolResponseIndex = unsolResponse - RIL_UNSOL_RESPONSE_BASE;

    if ((unsolResponseIndex < 0)
        || (unsolResponseIndex >= (int32_t)NUM_ELEMS(ims_s_unsolResponses))) {
    #ifdef MTK_RIL
        if (unsolResponse > (RIL_UNSOL_VENDOR_BASE + (int32_t)NUM_ELEMS(ims_s_mtk_unsolResponses)))
    #endif /* MTK_RIL */
        {
            RLOGE("unsupported unsolicited response code %d", unsolResponse);
            return;
        }
    }

#ifdef MTK_RIL
    fdCommand = s_ril_param_socket[soc_id].fdCommand;

    if (fdCommand == -1) {
        RLOGD("IMS: Can't send URC because there is no ims connection yet.");
        IMS_cacheUrc(unsolResponse, data, datalen , 0);
        return;
    }
#endif

   // Grab a wake lock if needed for this reponse,
   // as we exit we'll either release it immediately
   // or set a timer to release it later.
#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
        unsolResponseIndex = unsolResponse - RIL_UNSOL_VENDOR_BASE;
        wakeType = ims_s_mtk_unsolResponses[unsolResponseIndex].wakeType;
    }
    else
#endif /* MTK_RIL */
    {
        wakeType = ims_s_unsolResponses[unsolResponseIndex].wakeType;
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

    //appendPrintBuf("[UNSL]< %s", requestToString(unsolResponse));

    Parcel p;

    p.writeInt32 (RESPONSE_UNSOLICITED);
    p.writeInt32 (unsolResponse);

#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
        ret = ims_s_mtk_unsolResponses[unsolResponseIndex]
              .responseFunction(p, const_cast<void*>(data), datalen);
    } else
#endif /* MTK_RIL */
    {
        ret = ims_s_unsolResponses[unsolResponseIndex]
              .responseFunction(p, const_cast<void*>(data), datalen);
    }

    if (ret != 0) {
        // Problem with the response. Don't continue;
        goto error_exit;
    }

    // some things get more payload
    switch(unsolResponse) {
        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
            RIL_SOCKET_ID tmpId = (RIL_SOCKET_ID) (IMS_RIL_get3GSIM()-1);
            newState = processRadioState(callOnStateRequest(tmpId), tmpId);
            RLOGD("IMS: Radio newState = %d", newState);
            p.writeInt32(newState);
            appendPrintBuf("IMS: RADIO_STATE_CHANGED, %s {%s}", printBuf,
                radioStateToString(callOnStateRequest(tmpId)));
        break;
    }

    //RLOGI("%s UNSOLICITED: %s length:%d", IMS_rilSocketIdToString(soc_id), requestToString(unsolResponse), p.dataSize());
    ret = sendResponse(p, soc_id);

    // For now, we automatically go back to sleep after TIMEVAL_WAKE_TIMEOUT
    // FIXME The java code should handshake here to release wake lock

    if (shouldScheduleTimeout) {
        // Cancel the previous request
        if (IMS_s_last_wake_timeout_info != NULL) {
            IMS_s_last_wake_timeout_info->userParam = (void *)1;
        }
        IMS_s_last_wake_timeout_info = IMS_internalRequestTimedCallback(wakeTimeoutCallback, NULL, &TIMEVAL_WAKE_TIMEOUT);
    }

    // Normal exit
    return;

error_exit:
    if (shouldScheduleTimeout) {
        releaseWakeLock();
    }
}

#ifdef MTK_RIL
void IMS_userTimerCallback (int fd, short flags, void *param) {
#else
static void IMS_userTimerCallback (int fd, short flags, void *param) {
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
    pthread_mutex_lock(&s_ims_last_wake_mutex);
#endif

    // FIXME generalize this...there should be a cancel mechanism
    if (IMS_s_last_wake_timeout_info != NULL && IMS_s_last_wake_timeout_info == p_info) {
#ifdef MTK_RIL
        RLOGD("IMS_s_last_wake_timeout_info: %p reset to NULL", IMS_s_last_wake_timeout_info);
#endif
        IMS_s_last_wake_timeout_info = NULL;
    }
#ifdef MTK_RIL
    else {
        RLOGD("IMS_s_last_wake_timeout_info: %p ", IMS_s_last_wake_timeout_info);
    }
    pthread_mutex_unlock(&s_ims_last_wake_mutex);
#endif


#ifdef MTK_RIL
    if (p_info->cid < 0)
    {
        RLOGD("IMS_userTimerCallback free  p_info: %p", p_info);
        RLOGD("IMS_userTimerCallback free  p_info->event: %p", &(p_info->event));
        free(p_info);
    }
#else
    free(p_info);
#endif

}


/** FIXME generalize this if you track UserCAllbackInfo, clear it
    when the callback occurs
*/
static UserCallbackInfo *
IMS_internalRequestTimedCallback (RIL_TimedCallback callback, void *param,
                                const struct timeval *relativeTime)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) calloc(1, sizeof(UserCallbackInfo));

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

    ril_event_set(&(p_info->event), -1, false, IMS_userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}

static void IMS_queueImsUrc(int unsolResponse, void *data, size_t datalen, RIL_SOCKET_ID socket_id) {
    RLOGD("IMS: IMS_queueImsUrc");
    int pendedUrcCount = 0;
    ImsResponseList* urcCur = NULL;
    ImsResponseList* urcPrev = NULL;

    urcCur = queuedUrcList;
    while (urcCur != NULL) {
        RLOGD("Pended URC:%d, RILD:%s, :%s",
            pendedUrcCount,
            IMS_rilSocketIdToString(socket_id),
            requestToString(urcCur->id));
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }

    urcCur = (ImsResponseList*)calloc(1, sizeof(ImsResponseList));
    if (urcPrev != NULL) {
        urcPrev->pNext = urcCur;
    }
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->socket_id = socket_id;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);

    if (pendedUrcCount == 0) {
        queuedUrcList = urcCur;
    }
    RLOGD("IMS: Current pendedUrcCount = %d", pendedUrcCount + 1);
}

static void IMS_dequeueImsUrc() {
    RLOGD("IMS: IMS_dequeueImsUrc to GSM ");

    ImsResponseList* urc = queuedUrcList;
    ImsResponseList* urc_temp;
    while (urc != NULL) {
        RLOGD("IMS: send queued URCs RIL%s, %s",
        IMS_rilSocketIdToString(urc->socket_id),
        requestToString(urc->id));
        RIL_onUnsolicitedResponseSocket (urc->id, urc->data, urc->datalen, urc->socket_id);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

static int IMS_RIL_get3GSIM() {
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    property_get(PROPERTY_3G_SIM, tmp, "1");
    simId = atoi(tmp);
    RLOGD ("IMS: IMS_RIL_get3GSIM sim id = %d ", simId);
    return simId;
}

extern "C"
void IMS_isSrvccProcedure(int enable) {
    RLOGD("IMS: IMS_isSrvccProcedure, enable = %d ", enable);
    isSrvccProcedure = enable;
}

extern "C" void
IMS_RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI;
    int ret;
    int fd = -1;
    size_t errorOffset;
    RIL_SOCKET_ID socket_id = IMS_RIL_SOCKET_1;

    pRI = (RequestInfo *)t;

    socket_id = pRI->socket_id;
    fd = s_ril_param_socket[socket_id].fdCommand;

    RLOGD("IMS: RequestComplete, %s", IMS_rilSocketIdToString(socket_id));

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
        sendResponse(p, socket_id);
    }

done:
    free(pRI);
}

extern "C" bool
IMS_isRilRequestFromIms(RIL_Token t) {
    RequestInfo *pRI;
    RIL_SOCKET_ID socket_id = IMS_RIL_SOCKET_1;

    if (t == NULL) {
        RLOGD ("IMS: IMS_isRilRequestFromIms, RIL_Token = NULL ");
        return false;
    }
    pRI = (RequestInfo *)t;
    socket_id = pRI->socket_id;

    RLOGD ("IMS: IMS_isRilRequestFromIms, socket_id = %d ", socket_id);

    if (socket_id == IMS_RIL_SOCKET_1) {
        return true;
    }

    return false;
}

}/* namespace android */

