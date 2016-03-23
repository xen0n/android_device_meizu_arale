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

#include <dirent.h>
#include <stdlib.h>
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
#include "hardware/ccci_intf.h"
#include <cutils/properties.h>

#include <ril_callbacks.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#include <cutils/properties.h>

#include <hardware/ril/librilutils/proto/sap-api.pb.h>

#define PROPERTY_RIL_SIM_READY              "ril.ready.sim"

//[New R8 modem FD]
#define PROPERTY_FD_SCREEN_ON_TIMER     "persist.radio.fd.counter"
#define PROPERTY_FD_SCREEN_ON_R8_TIMER  "persist.radio.fd.r8.counter"
#define PROPERTY_FD_SCREEN_OFF_TIMER    "persist.radio.fd.off.counter"
#define PROPERTY_FD_SCREEN_OFF_R8_TIMER    "persist.radio.fd.off.r8.counter"
#define PROPERTY_FD_ON_ONLY_R8_NETWORK  "persist.radio.fd.on.only.r8"
#define PROPERTY_RIL_FD_MODE    "ril.fd.mode"
/* FD related timer: units: 1 sec */
#define DEFAULT_FD_SCREEN_ON_TIMER "15"
#define DEFAULT_FD_SCREEN_ON_R8_TIMER "15"
#define DEFAULT_FD_SCREEN_OFF_TIMER "5"
#define DEFAULT_FD_SCREEN_OFF_R8_TIMER "5"
#define DEFAULT_FD_ON_ONLY_R8_NETWORK "0"

static void onRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id);
#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id);
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t);
#endif
static RIL_RadioState currentState(RIL_SOCKET_ID rid);
static int onSupports(int requestCode);
static void onCancel(RIL_Token t);
static const char *getVersion();
void determineSSServiceClassFeature(RIL_SOCKET_ID rid);

extern const char *requestToString(int request);
extern void initRILChannels(void);
extern void IMS_RIL_onUnsolicitedResponse(int unsolResponse, void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);
extern void IMS_RIL_onRequestComplete(RIL_Token t, RIL_Errno e,
                           void *response, size_t responselen);
extern bool IMS_isRilRequestFromIms(RIL_Token t);
extern void initialCidTable();
extern int getAttachApnSupport();
extern void IMS_RILA_register(const RIL_RadioFunctionsSocket *callbacks);

/*** Static Variables ***/
static const RIL_RadioFunctionsSocket s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion
};

#ifdef  RIL_SHLIB
const struct RIL_EnvSocket *s_rilenv;
#endif  /* RIL_SHLIB */

const struct RIL_EnvSocket *s_rilsapenv;
static const RIL_RadioFunctionsSocket s_sapcallbacks = {
    RIL_VERSION,
    onSapRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion
};

static RIL_RadioState sState = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState2 = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState3 = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState4 = RADIO_STATE_UNAVAILABLE;

static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;

static int s_port = -1;
static const char *s_device_path = NULL;
static int s_device_socket = 0;

static int s_device_range_begin = -1;
static int s_device_range_end = -1;

/* trigger change to this with s_state_cond */
static int s_closed = 0;

#ifdef  MTK_RIL
static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
#endif  /* MTK_RIL */

//BEGIN mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
extern int inCallNumber;
//END   mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
extern int setPlmnListFormat(RIL_SOCKET_ID rid, int format);

static const struct timeval TIMEVAL_0 = { 0, 0 };

/* Commands pending in AT channel */
static pthread_mutex_t s_pendinglist_mutex = PTHREAD_MUTEX_INITIALIZER;
static int pendinglist[RIL_SUPPORT_CHANNELS] = { 0 };

static int s_isSimSwitched = 0;
static int s_telephonyMode = -1;
extern int bPSBEARERSupport;

extern int s_md_off;
extern int s_main_loop;

extern int bCREGType3Support;
extern int bEopsSupport;
extern int bUbinSupport;  //support Universal BIN(WorldMode)
extern int bWorldModeSwitching;  //[ALPS02277365]

//Add log level for ALPS01270573 reproduce
int mtk_ril_log_level = 0;

/* To check if new SS service class feature is supported or not */
int bNewSSServiceClassFeature = 0;
char s_logicalModemId[SIM_COUNT][MAX_UUID_LENGTH];
/*
 * If found the request in pendinglist, return list index else retrun RIL_SUPPORT_CHANNELS
 */
static int findPendingRequest(int request)
{
    int i;
    for (i = 0; i < getSupportChannels(); i++) {
        if (*(pendinglist + i) == request)
            return i;
    }

    return i;
}

static void setRequest(int request)
{
    pthread_mutex_lock(&s_pendinglist_mutex);
    /* find an empty slot */
    pendinglist[findPendingRequest(0)] = request;
    assert(i < getSupportChannels());
    pthread_mutex_unlock(&s_pendinglist_mutex);
}

static void resetRequest(int request)
{
    pthread_mutex_lock(&s_pendinglist_mutex);
    pendinglist[findPendingRequest(request)] = 0;
    assert(i < getSupportChannels());
    pthread_mutex_unlock(&s_pendinglist_mutex);
}

extern int RILcheckPendingRequest(int request)
{
    return (getSupportChannels()== findPendingRequest(request)) ? 0 : 1;
}

/** do post-AT+CFUN=1 initialization */
static void onRadioPowerOn(RIL_SOCKET_ID rid)
{
    const RIL_SOCKET_ID *p_rilId = &s_pollSimId;

    if (RIL_SOCKET_2 == rid) {
        p_rilId = &s_pollSimId2;
    } else if (RIL_SOCKET_3 == rid) {
        p_rilId = &s_pollSimId3;
    } else if (RIL_SOCKET_4 == rid) {
        p_rilId = &s_pollSimId4;
    }

    pollSIMState((void *)p_rilId);
}

/** do post- SIM ready initialization */
static void onSIMReady(RIL_SOCKET_ID rid)
{
}

/*** Callback methods from the RIL library to us ***/

/**
 * Call from RIL to us to make a RIL_REQUEST
 *
 * Must be completed with a call to RIL_onRequestComplete()
 *
 * RIL_onRequestComplete() may be called from any thread, before or after
 * this function returns.
 *
 * Will always be called from the same thread, so returning here implies
 * that the radio is ready to process another command (whether or not
 * the previous command has completed).
 */
static void onRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    RIL_RadioState radioState = sState;

    RLOGD("onRequest: %s, datalen = %d", requestToString(request), datalen);

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState4;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState3;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState2;
    }

    RLOGD("radioState:%d", radioState);

    if (s_md_off &&
            request != RIL_REQUEST_RADIO_POWER &&
            request != RIL_REQUEST_MODEM_POWERON &&
            request != RIL_REQUEST_GET_RADIO_CAPABILITY &&
            request != RIL_REQUEST_GET_CALIBRATION_DATA &&
            request != RIL_REQUEST_GET_IMEI &&
            request != RIL_REQUEST_GET_IMEISV &&
            request != RIL_REQUEST_BASEBAND_VERSION &&
            request != RIL_REQUEST_DUAL_SIM_MODE_SWITCH &&
            request != RIL_REQUEST_RESET_RADIO &&
            request != RIL_REQUEST_SET_RADIO_CAPABILITY &&
            request != RIL_REQUEST_STORE_MODEM_TYPE &&
            request != RIL_REQUEST_RELOAD_MODEM_TYPE &&
            request != RIL_REQUEST_QUERY_MODEM_TYPE &&
            request != RIL_REQUEST_GENERAL_SIM_AUTH &&
            request != RIL_REQUEST_CONFIG_MODEM_STATUS &&
            request != RIL_REQUEST_SET_FD_MODE &&
            request != RIL_LOCAL_REQUEST_SET_MODEM_THERMAL
       ) {
        RLOGD("MD off and ignore %s", requestToString(request));
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        }
        return;
    }

    /* Ignore all requests except RIL_REQUEST_GET_SIM_STATUS,RIL_REQUEST_DETECT_SIM_MISSING when RADIO_STATE_UNAVAILABLE. */
    // [ALPS00958313] Pass RADIO_POWERON when radio not available that is caused by muxd shutdown device before ccci ioctl.
    if (radioState == RADIO_STATE_UNAVAILABLE &&
            request != RIL_REQUEST_GET_SIM_STATUS &&
            request != RIL_REQUEST_OEM_HOOK_RAW && //This is for ATCI
            request != RIL_REQUEST_OEM_HOOK_STRINGS &&
            request != RIL_REQUEST_GET_RADIO_CAPABILITY &&
            request != RIL_REQUEST_SET_RADIO_CAPABILITY &&
            request != RIL_LOCAL_REQUEST_SIM_AUTHENTICATION &&
            request != RIL_LOCAL_REQUEST_USIM_AUTHENTICATION &&
            request != RIL_REQUEST_SET_SIM_RECOVERY_ON &&
            request != RIL_REQUEST_RESET_RADIO &&
            request != RIL_REQUEST_MODEM_POWERON &&
            request != RIL_REQUEST_MODEM_POWEROFF &&
            request != RIL_REQUEST_STORE_MODEM_TYPE &&
            request != RIL_REQUEST_RELOAD_MODEM_TYPE &&
            request != RIL_REQUEST_QUERY_MODEM_TYPE &&
            request != RIL_REQUEST_ALLOW_DATA &&
            /// M: [C2K][IR] Support SVLTE IR feature. @{
            request != RIL_REQUEST_SET_REG_SUSPEND_ENABLED &&
            request != RIL_REQUEST_RESUME_REGISTRATION &&
            /// M: [C2K][IR] Support SVLTE IR feature. @{
            request != RIL_REQUEST_CONFIG_MODEM_STATUS&&
            request != RIL_REQUEST_SET_FD_MODE &&
            request != RIL_LOCAL_REQUEST_SET_MODEM_THERMAL &&
            request != RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE &&
            request != RIL_REQUEST_SET_IMS_ENABLE
       ) {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        }
        return;
    }

    /* Ignore all non-power requests when RADIO_STATE_OFF
     * (except RIL_REQUEST_GET_SIM_STATUS, RIL_REQUEST_DETECT_SIM_MISSING)
     */
    if (radioState == RADIO_STATE_OFF &&
            !(request == RIL_REQUEST_RADIO_POWER ||
              request == RIL_REQUEST_MODEM_POWERON ||
              request == RIL_REQUEST_DUAL_SIM_MODE_SWITCH ||
              request == RIL_REQUEST_SET_GPRS_CONNECT_TYPE ||
              request == RIL_REQUEST_SET_GPRS_TRANSFER_TYPE ||
              request == RIL_REQUEST_GET_SIM_STATUS ||
              request == RIL_REQUEST_GET_IMEI ||
              request == RIL_REQUEST_GET_IMEISV ||
              request == RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE ||
              request == RIL_REQUEST_BASEBAND_VERSION ||
              request == RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE ||
              request == RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE ||
              request == RIL_REQUEST_SET_LOCATION_UPDATES ||
              // SMS over wifi is able to send SMS even if radio is off
              (isWfcSupport() && request == RIL_REQUEST_SEND_SMS) ||
              (isWfcSupport() && request == RIL_REQUEST_SEND_SMS_EXPECT_MORE) ||
              (isWfcSupport() && request == RIL_REQUEST_SMS_ACKNOWLEDGE) ||
              request == RIL_REQUEST_WRITE_SMS_TO_SIM ||
              request == RIL_REQUEST_DELETE_SMS_ON_SIM ||
              request == RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG ||
              request == RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG ||
              request == RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION ||
              request == RIL_REQUEST_GET_SMSC_ADDRESS ||
              request == RIL_REQUEST_SET_SMSC_ADDRESS ||
              request == RIL_REQUEST_REPORT_SMS_MEMORY_STATUS ||
              request == RIL_REQUEST_SCREEN_STATE ||
              request == RIL_REQUEST_RESET_RADIO ||
              request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE ||
              request == RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND ||
              request == RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM ||
              request == RIL_REQUEST_SET_TTY_MODE ||
              request == RIL_REQUEST_OEM_HOOK_RAW ||
              request == RIL_REQUEST_OEM_HOOK_STRINGS ||
              request == RIL_REQUEST_MOBILEREVISION_AND_IMEI ||
              request == RIL_REQUEST_GET_SMS_SIM_MEM_STATUS ||
              request == RIL_REQUEST_SIM_IO ||
              request == RIL_REQUEST_GET_IMSI ||
              request == RIL_REQUEST_SIM_OPEN_CHANNEL ||
              request == RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC ||
              request == RIL_REQUEST_SIM_CLOSE_CHANNEL ||
              request == RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL ||
              request == RIL_REQUEST_SIM_GET_ATR ||
              request == RIL_REQUEST_GET_RADIO_CAPABILITY ||
              request == RIL_REQUEST_SET_RADIO_CAPABILITY ||
              request == RIL_REQUEST_GET_CALIBRATION_DATA ||
              request == RIL_REQUEST_MODEM_POWEROFF ||
              request == RIL_REQUEST_ENTER_SIM_PIN ||
              request == RIL_REQUEST_ENTER_SIM_PUK ||
              request == RIL_REQUEST_ENTER_SIM_PIN2 ||
              request == RIL_REQUEST_ENTER_SIM_PUK2 ||
              request == RIL_REQUEST_CHANGE_SIM_PIN ||
              request == RIL_REQUEST_CHANGE_SIM_PIN2 ||
              request == RIL_REQUEST_QUERY_FACILITY_LOCK ||
              request == RIL_REQUEST_SET_FACILITY_LOCK ||
              request == RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION ||
              request == RIL_REQUEST_QUERY_SIM_NETWORK_LOCK ||
              request == RIL_REQUEST_SET_SIM_NETWORK_LOCK ||
              request == RIL_REQUEST_SIM_AUTHENTICATION ||
              request == RIL_REQUEST_ISIM_AUTHENTICATION ||
              request == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION ||
              request == RIL_LOCAL_REQUEST_USIM_AUTHENTICATION ||
              // Multiple application support Start
              request == RIL_REQUEST_GENERAL_SIM_AUTH ||
              request == RIL_REQUEST_OPEN_ICC_APPLICATION ||
              request == RIL_REQUEST_GET_ICC_APPLICATION_STATUS ||
              request == RIL_REQUEST_SIM_IO_EX ||
              request == RIL_REQUEST_GENERAL_SIM_AUTH ||
              // Multiple application support End
              // Remote SIM ME Lock Start
              request == RIL_LOCAL_REQUEST_GET_SHARED_KEY ||
              request == RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS ||
              request == RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO ||
              request == RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS ||
              request == RIL_LOCAL_REQUEST_GET_MODEM_STATUS ||
              // Remote SIM ME Lock End
              // External SIM [Start]
              request == RIL_LOCAL_REQUEST_VSIM_NOTIFICATION ||
              request == RIL_LOCAL_REQUEST_VSIM_OPERATION||
              // External SIM [End]
              request == RIL_REQUEST_DETACH_PS ||
              request == RIL_REQUEST_ALLOW_DATA ||
              request == RIL_REQUEST_SET_ECC_LIST ||
              // PHB Start
              request == RIL_REQUEST_QUERY_PHB_STORAGE_INFO ||
              request == RIL_REQUEST_WRITE_PHB_ENTRY ||
              request == RIL_REQUEST_READ_PHB_ENTRY ||
              request == RIL_REQUEST_GET_PHB_STRING_LENGTH ||
              request == RIL_REQUEST_GET_PHB_MEM_STORAGE ||
              request == RIL_REQUEST_SET_PHB_MEM_STORAGE ||
              request == RIL_REQUEST_READ_PHB_ENTRY_EXT ||
              request == RIL_REQUEST_WRITE_PHB_ENTRY_EXT ||
              request == RIL_REQUEST_QUERY_UPB_CAPABILITY ||
              request == RIL_REQUEST_EDIT_UPB_ENTRY ||
              request == RIL_REQUEST_DELETE_UPB_ENTRY ||
              request == RIL_REQUEST_READ_UPB_GAS_LIST ||
              request == RIL_REQUEST_READ_UPB_GRP ||
              request == RIL_REQUEST_WRITE_UPB_GRP ||
              // PHB End
              request == RIL_REQUEST_SET_TRM ||
              request == RIL_REQUEST_STORE_MODEM_TYPE ||
              request == RIL_REQUEST_RELOAD_MODEM_TYPE ||
              request == RIL_REQUEST_QUERY_MODEM_TYPE ||
              // Stk
              request == RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING ||
              // IMS
              request == RIL_REQUEST_SET_IMS_ENABLE ||
              request == RIL_REQUEST_BTSIM_CONNECT ||
              request == RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF ||
              request == RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM ||
              request == RIL_REQUEST_BTSIM_TRANSFERAPDU ||
              request == RIL_REQUEST_SET_DATA_CENTRIC ||
              request == RIL_REQUEST_SET_INITIAL_ATTACH_APN ||
              request == RIL_REQUEST_ALLOW_DATA ||
              // Remote SIM access
              request == RIL_REQUEST_CONFIG_MODEM_STATUS||
              /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
              request == RIL_REQUEST_SET_SVLTE_RAT_MODE ||
              /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}
              /// M: [C2K][IR] Support SVLTE IR feature. @{
              request == RIL_REQUEST_SET_ACTIVE_PS_SLOT ||
              request == RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE ||
              /// M: [C2K][SVLTE] Set the STK UTK mode. @{
              request == RIL_REQUEST_SET_STK_UTK_MODE ||
              /// M: [C2K][SVLTE] Set the STK UTK mode. @}
#ifdef MTK_SVLTE_SUPPORT
              /// M: [C2K][SVLTE] Set Band config. @{
              request == RIL_REQUEST_SET_BAND_MODE ||
              /// M: [C2K][SVLTE] Set Band config. @}
#endif
              request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED ||
              request == RIL_REQUEST_RESUME_REGISTRATION ||
              /// M: [C2K][IR] Support SVLTE IR feature. @}
              /// M: Fast dormancy
              request == RIL_REQUEST_SET_FD_MODE ||
              request == RIL_REQUEST_SWITCH_ANTENNA ||
              request == RIL_LOCAL_REQUEST_SET_MODEM_THERMAL ||
              request == RIL_REQUEST_AT_COMMAND_WITH_PROXY ||
              request == RIL_REQUEST_SWITCH_CARD_TYPE ||
              /// M:  EPDG feature. Update PS state from MAL
              request == RIL_REQUEST_MAL_PS_RGEGISTRATION_STATE
         )) {
             /// M: [WFC]Wifi calling can be done even if the radio is unavailable.
             /// If WFC support then Epdg must be supported.
             if (isWfcSupport() &&
                  (request == RIL_REQUEST_DIAL ||
                   request == RIL_REQUEST_HANGUP ||
                   request == RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND ||
                   request == RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND ||
                   request == RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE ||
                   request == RIL_REQUEST_ANSWER ||
                   request == RIL_REQUEST_CONFERENCE ||
                   request == RIL_REQUEST_SEPARATE_CONNECTION ||
                   request == RIL_REQUEST_LAST_CALL_FAIL_CAUSE ||
                   request == RIL_REQUEST_DTMF ||
                   request == RIL_REQUEST_DTMF_START ||
                   request == RIL_REQUEST_DTMF_STOP ||
                   request == RIL_REQUEST_HANGUP_ALL ||
                   request == RIL_REQUEST_FORCE_RELEASE_CALL ||
                   request == RIL_REQUEST_SET_CALL_INDICATION ||
                   request == RIL_REQUEST_EMERGENCY_DIAL ||
                   request == RIL_REQUEST_SET_CALL_INDICATION ||
                   request == RIL_REQUEST_HOLD_CALL ||
                   request == RIL_REQUEST_RESUME_CALL ||
                   request == RIL_REQUEST_SETUP_DATA_CALL ||
                   request == RIL_REQUEST_DEACTIVATE_DATA_CALL ||
                   request == RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER ||
                   request == RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER ||
                   request == RIL_REQUEST_DIAL_WITH_SIP_URI)) {
                 RLOGD("call command accept in radio off if wfc is support");
             }
             /// M: If Epdg support only case. @{
             else if (isEpdgSupport() &&
                  (request == RIL_REQUEST_OPERATOR ||
                   request == RIL_REQUEST_VOICE_REGISTRATION_STATE ||
                   request == RIL_REQUEST_DATA_REGISTRATION_STATE)) {
                 RLOGD("call command accept in radio off if epdg is support");
             } else {
                 if (IMS_isRilRequestFromIms(t)) {
                     IMS_RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
                 } else {
                     RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
                 }
                 return;
             }
    }

    //BEGIN mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
    if (inCallNumber == 0 &&
        (request == RIL_REQUEST_DTMF ||
         request == RIL_REQUEST_DTMF_START ||
         request == RIL_REQUEST_DTMF_STOP)) {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0); // RIL_E_GENERIC_FAILURE
        } else {
            RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0); // RIL_E_GENERIC_FAILURE
        }
        return;
    }
    //END   mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]

    /* set pending RIL request */
    setRequest(request);

    if (!(rilSimMain(request, data, datalen, t) ||
          rilNwMain(request, data, datalen, t) ||
          rilCcMain(request, data, datalen, t) ||
          rilSsMain(request, data, datalen, t) ||
          rilSmsMain(request, data, datalen, t) ||
          rilStkMain(request, data, datalen, t) ||
          rilOemMain(request, data, datalen, t) ||
          rilDataMain(request, data, datalen, t))) {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        }
    }

    /* Reset pending RIL request */
    resetRequest(request);
}

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t) {
#endif
    RLOGD("onSapRequest: %d", request);
    if (request < MsgId_RIL_SIM_SAP_CONNECT /* MsgId_UNKNOWN_REQ */ ||
            request > MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL) {
        RLOGD("invalid request");
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, Error_RIL_E_REQUEST_NOT_SUPPORTED, MsgId_RIL_SIM_SAP_ERROR_RESP
                , &rsp);
        return;
    }

    if (s_md_off) {
        RLOGD("MD off and reply failure to Sap message");
        RIL_SIM_SAP_CONNECT_RSP rsp;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        rsp.has_max_message_size = false;
        rsp.max_message_size = 0;
        sendSapResponseComplete(t, Error_RIL_E_RADIO_NOT_AVAILABLE, MsgId_RIL_SIM_SAP_CONNECT,
                &rsp);
        return;
    }

    /* set pending RIL request */
    setRequest(request);

    if (!(rilBtSapMain(request, data, datalen, t, socket_id) ||
            rilStkBtSapMain(request, data, datalen, t, socket_id))) {
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, Error_RIL_E_REQUEST_NOT_SUPPORTED, MsgId_RIL_SIM_SAP_ERROR_RESP
                , &rsp);
    }

    /* Reset pending RIL request */
    resetRequest(request);
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState currentState(RIL_SOCKET_ID rid)
{
    return getRadioState(rid);
}

/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */
static int onSupports(int requestCode)
{
    //@@@ todo
    return 1;
}

static void onCancel(RIL_Token t)
{
    //@@@todo
}

static const char *getVersion(void)
{
    switch (SIM_COUNT) {
        case 2:
            return "mtk gemini ril 1.0";
        case 3:
            return "mtk gemini+ 3 SIM ril 1.0";
        case 4:
            return "mtk gemini+ 4 SIM ril 1.0";
        case 1:
        default:
            return "mtk ril w10.20";
    }
}


static int isReadMccMncForBootAnimation()
{
    char prop[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.mtk_rild_read_imsi", prop, "");
    if (!strcmp(prop, "1")) {
        return 1;
    }

    property_get("ro.operator.optr", prop, "");

    return (!strcmp(prop, "OP01") || !strcmp(prop, "OP02")) ? 1 : 0;
}

static int isImsAndVolteSupport()
{
    char volte_prop[PROPERTY_VALUE_MAX] = {0};
    char ims_prop[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.mtk_volte_support", volte_prop, "");
    property_get("ro.mtk_ims_support", ims_prop, "");

    if (!strcmp(volte_prop, "1") && !strcmp(ims_prop, "1")) {
        return 1;
    }

    return 0;
}

// External SIM [Start]
int isExternalSimSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_external_sim_support", property_value, "0");
    return atoi(property_value);
}
// External SIM [End]

static void resetSystemProperties(RIL_SOCKET_ID rid)
{
    RLOGI("[RIL_CALLBACK] resetSystemProperties");
    upadteSystemPropertyByCurrentMode(rid,"ril.ipo.radiooff","ril.ipo.radiooff.2","0");
    // clear SIM switch MCC,MNC
    property_set(PROPERTY_RIL_SIM_MCCMNC[rid], "");
    resetSIMProperties(rid);
}

/**
 * Initialize everything that can be configured while we're still in
 * AT+CFUN=0
 */
#ifdef  MTK_RIL
static void initializeCallback(void *param)
{
    ATResponse *p_response = NULL;
    int err;
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);
    int nRadioState = -1;
    //[New R8 modem FD] Enlarge array size of property_value
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int current_share_modem = 0;
    int mdType = -1;
    int i=0;

    setRadioState(RADIO_STATE_OFF, rid);

    err = at_handshake(getDefaultChannelCtx(rid));

    RLOGI("AT handshake: %d", err);

    resetSystemProperties(rid);

    /* note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed */

    /*  atchannel is tolerant of echo but it must */
    /*  have verbose result codes */
    at_send_command("ATE0Q0V1", NULL, getDefaultChannelCtx(rid));

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL, getDefaultChannelCtx(rid));

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL, getDefaultChannelCtx(rid));

    /*  Disable CFU query */
    err = at_send_command("AT+ESSP=1", &p_response, getDefaultChannelCtx(rid));

    /* check if modem support +CREG=3 */
    err = at_send_command("AT+CREG=3", &p_response, getDefaultChannelCtx(rid));
    if (err < 0 || p_response->success == 0) {
        bCREGType3Support = 0;

        /*  check if modem support +CREG=2 */
        err = at_send_command("AT+CREG=2", &p_response, getDefaultChannelCtx(rid));

        /* some handsets -- in tethered mode -- don't support CREG=2 */
        if (err < 0 || p_response->success == 0)
            at_send_command("AT+CREG=1", NULL, getDefaultChannelCtx(rid));
    }
    at_response_free(p_response);

    if (isImsSupport()) {
        err = at_send_command("AT+CIREG=2", &p_response, getDefaultChannelCtx(rid));
        at_response_free(p_response);
    }

#ifdef MTK_FEMTO_CELL_SUPPORT
    err = at_send_command("AT+ECSG=4,1", NULL, getDefaultChannelCtx(rid));
#endif
    /*  GPRS registration events */
    at_send_command("AT+CGREG=1", NULL, getDefaultChannelCtx(rid));
    at_send_command("AT+CEREG=1", NULL, getDefaultChannelCtx(rid));
    err = at_send_command("AT+PSBEARER=1", NULL, getDefaultChannelCtx(rid));
    if (err < 0)
        bPSBEARERSupport = 0;

    /*  Call Waiting notifications */
    at_send_command("AT+CCWA=1", NULL, getDefaultChannelCtx(rid));

    /*  mtk00924: enable Call Progress notifications */
    at_send_command("AT+ECPI=4294967295", NULL, getDefaultChannelCtx(rid));

    /*  Alternating voice/data off */
    /*
     * at_send_command("AT+CMOD=0", NULL, getDefaultChannelCtx(rid));
     */

    /*  Not muted */
    /*
     * at_send_command("AT+CMUT=0", NULL, getDefaultChannelCtx(rid));
     */

    /*  +CSSU unsolicited supp service notifications */
    at_send_command("AT+CSSN=1,1", NULL, getDefaultChannelCtx(rid));

    /*  connected line identification on */
    at_send_command("AT+COLP=1", NULL, getDefaultChannelCtx(rid));

    /*  HEX character set */
    at_send_command("AT+CSCS=\"UCS2\"", NULL, getDefaultChannelCtx(rid));

    /*  USSD unsolicited */
    at_send_command("AT+CUSD=1", NULL, getDefaultChannelCtx(rid));

    /*  Enable +CGEV GPRS event notifications, but don't buffer */
    at_send_command("AT+CGEREP=1,0", NULL, getDefaultChannelCtx(rid));

    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL, getDefaultChannelCtx(rid));

    /* Enable getting NITZ, include TZ and Operator Name*/
    /* To Receive +CIEV: 9 and +CIEV: 10*/
    at_send_command("AT+CTZR=1", NULL, getDefaultChannelCtx(rid));

    /*  Enable getting CFU info +ECFU and speech info +ESPEECH*/
    int einfo_value;
#ifdef MTK_UMTS_TDD128_MODE //Add for TDD rb release[bit 9]
    einfo_value = 306;
#else
    einfo_value = 50;
#endif
    /*  Enable getting CFU info +ECFU and speech info +ESPEECH and modem warning +EWARNING(0x100) */
    char modemWarningProperty[PROPERTY_VALUE_MAX];
    char einfoStr[32];
    property_get("persist.radio.modem.warning", modemWarningProperty, 0);
    if (strcmp(modemWarningProperty, "1") == 0) {
        /* Enable "+EWARNING" */
       einfo_value |= 512;
    }
#ifdef MTK_RIL_MD2
    if (isEVDODTSupport() == 0) {
        /* This is for ROME_DSDA's tempariory solution */
        einfo_value |= 128;
        RLOGD("DSDA, so add 128 into EINFO value for STK");
    }
#endif
    /* Enable response message of call ctrl by sim. */
    einfo_value |= 1024;
    sprintf(einfoStr, "AT+EINFO=%d", einfo_value);
    at_send_command(einfoStr, NULL, getDefaultChannelCtx(rid));

/* M: Start - abnormal event logging for logger */
    einfo_value |= 8;
    sprintf(einfoStr, "AT+EINFO=%d,401,0", einfo_value);
    /* Enable smart logging no service notification +ENWINFO */
    at_send_command(einfoStr, NULL, getDefaultChannelCtx(rid));
/* M: End - abnormal event logging for logger */

    /*  Enable get ECSQ URC */
    /* ALPS00465815, fix power consumption issue, START */
    if (isDualTalkMode() && getExternalModemSlot() == (int)rid) {
        at_send_command("AT+ECSQ=0", NULL, getDefaultChannelCtx(rid));
    } else {
        at_send_command("AT+ECSQ=1", NULL, getDefaultChannelCtx(rid));
    }
    /* ALPS00465815, fix power consumption issue, END */

    /*  Enable get +CIEV:7 URC to receive SMS SIM Storage Status*/
    at_send_command("AT+CMER=1,0,0,2,0", NULL, getDefaultChannelCtx(rid));

    /// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
    at_send_command("AT+CRC=1", NULL, getDefaultChannelCtx(rid));
    at_send_command("AT+CBST=134,1,0", NULL, getDefaultChannelCtx(rid));
#else
    at_send_command("AT+ECCP=1", NULL, getDefaultChannelCtx(rid));
#endif
    /// @}

    /* Set 4GVT capability */
#ifdef MTK_VILTE_SUPPORT
    at_send_command("AT+EIMSCCP=1", NULL, getDefaultChannelCtx(rid));
    RLOGD("ViLTE is enable and AT+EIMSCCP=1 is sent");
#else
    at_send_command("AT+EIMSCCP=0", NULL, getDefaultChannelCtx(rid));
    RLOGD("ViLTE is disable and AT+EIMSCCP=0 is sent");
#endif

    if (isWfcSupport()) {
        at_send_command("AT+EIMSWFC=1", NULL, getDefaultChannelCtx(rid));
    }

#ifndef MTK_FD_SUPPORT
    RLOGD("Not Support Fast Dormancy");
#else
    RLOGD("Check RIL FD Mode on rid=%d", rid);
#endif

    /* check if modem support +ECSRA? */
    if (getMainProtocolRid() == rid) {
        err = at_send_command_singleline("AT+ECSRA?", "+ECSRA:", &p_response, getDefaultChannelCtx(rid));
        if (err < 0 || p_response->success == 0) {
            bUbinSupport = 0;
        }
        at_response_free(p_response);
    }

#ifdef MTK_WORLD_PHONE
    if (!bUbinSupport){ //WorldMode (+ECSRA? is not support)
        at_send_command("AT+ECOPS=1", NULL, getDefaultChannelCtx(rid));
        RLOGD("AT+ECOPS=1 sent");
        at_send_command("AT+EMSR=0,1", NULL, getDefaultChannelCtx(rid));
        RLOGD("AT+EMSR=0,1 sent");

        //world phone optimization
        if (getMainProtocolRid() == rid) {
            //cause type
            property_get(PROPERTY_SWITCH_MODEM_CAUSE_TYPE,
                property_value,
                PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE);
            //delay info
            char property_value2[PROPERTY_VALUE_MAX] = { 0 };
            property_get(PROPERTY_SWITCH_MODEM_DELAY_INFO,
                property_value2,
                PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE);
            char *smciCmd;
            asprintf(&smciCmd, "AT+ESMCI=%s,%s", property_value, property_value2);
            at_send_command(smciCmd, NULL, getDefaultChannelCtx(rid));
            RLOGD("%s sent", smciCmd);
            //set default value
            property_set(PROPERTY_SWITCH_MODEM_CAUSE_TYPE,
                PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE);
            property_set(PROPERTY_SWITCH_MODEM_DELAY_INFO,
                PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE);
        }
    } else {
        at_send_command("AT+ECSRA=1", NULL, getDefaultChannelCtx(rid));
        RLOGD("AT+ECSRA=1 sent");

        switch(getWorldPhonePolicy()) {
            case 1:
                at_send_command("AT+EWMPOLICY=1", NULL, getDefaultChannelCtx(rid));
                break;
            case 2:
                at_send_command("AT+EWMPOLICY=2", NULL, getDefaultChannelCtx(rid));
                break;
            case 3:
                at_send_command("AT+EWMPOLICY=3", NULL, getDefaultChannelCtx(rid));
                break;
            case 4:
                at_send_command("AT+EWMPOLICY=4", NULL, getDefaultChannelCtx(rid));
                break;
            default:
                RLOGD("WorldPhone Policy unknown!");
        }
    }
#endif
    triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &mdType);
    setActiveModemType(mdType);
    LOGD("[WPO][initCallback] Set ril.active.md = %d", mdType);

    //get active mode
    if ((bUbinSupport) && (getMainProtocolRid() == rid)){
        p_response = NULL;
        char* line;
        char* activeMode = NULL;
        int csraaResponse[3] = {0};
        err = at_send_command_singleline("AT+CSRA?", "+CSRAA:", &p_response, getDefaultChannelCtx(rid));
        if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL){
            line = p_response->p_intermediates->line;
            asprintf(&activeMode, "%d", 0);
            /* +CSRAA: <GERAN-TDMA>,<UTRANFDD>,<UTRAN-TDD-LCR>,<UTRAN-TDD-HCR>,
                                            <UTRANTDD-VHCR>,<E-UTRAN-FDD>,<E-UTRAN-TDD>*/
            err = at_tok_start(&line);
            if (err >= 0){
                err = at_tok_nextint(&line, &csraaResponse[0]);//<GERAN-TDMA>
                if (err >= 0){
                    err = at_tok_nextint(&line, &csraaResponse[1]);//<UTRANFDD>
                    if (err >= 0){
                        LOGD("+CSRAA:<UTRANFDD> = %d", csraaResponse[1]);
                        err = at_tok_nextint(&line, &csraaResponse[2]);//<UTRAN-TDD-LCR>
                        if (err >= 0){
                            LOGD("+CSRAA:<UTRAN-TDD-LCR> = %d", csraaResponse[2]);
                            if ((csraaResponse[1] == 1) && (csraaResponse[2] == 0)){
                                //FDD mode
                                asprintf(&activeMode, "%d", 1);
                            }
                            if ((csraaResponse[1] == 0) && (csraaResponse[2] == 1)){
                                //TDD mode
                                asprintf(&activeMode, "%d", 2);
                            }
                            LOGD("update property ril.nw.worldmode.activemode to %s", activeMode);
                            property_set("ril.nw.worldmode.activemode", activeMode);
                            if (NULL != activeMode){
                                free(activeMode);
                                activeMode = NULL;
                            }
                        }
                    }
                }
            }
        }
        at_response_free(p_response);
    }

    if (isInternationalRoamingEnabled()) {
        /* ALPS00501602, fix power issue by disable ECOPS URC START */
        if (!isDualTalkMode() && rid == RIL_SOCKET_1) {
            at_send_command("AT+ECOPS=1", NULL, getDefaultChannelCtx(rid));
            at_send_command("AT+EMSR=0,1", NULL, getDefaultChannelCtx(rid));
        }
        /* ALPS00501602, fix power issue by disable ECOPS URC END */

        /* ALPS00523054 START */
        if (isDualTalkMode() && getExternalModemSlot() == (int)rid) {
            RLOGI("Turn off URC of the unused modem SIM protocol, rid=%d", rid);
            at_send_command("AT+ECSQ=0", NULL, getDefaultChannelCtx(rid));
            at_send_command("AT+CREG=1", NULL, getDefaultChannelCtx(rid));
            at_send_command("AT+CGREG=1", NULL, getDefaultChannelCtx(rid));
            at_send_command("AT+PSBEARER=0", NULL, getDefaultChannelCtx(rid));
        }
        /* ALPS00523054 END */
    }

    //[New R8 modem FD]
#ifdef MTK_FD_SUPPORT
    /* MTK_FD_SUPPORT is turned on single sim 3G Project, WG/G Gemini Project => For EVDO_DT_SUPPORT: Turn off MTK_FD_DORMANCY */
    if (rid == RIL_SOCKET_1) {
        RLOGD("Support Fast Dormancy Feature");
        /* Fast Dormancy is only available on 3G Protocol Set */
        /* [Step#01] Query if the new FD mechanism is supported by modem or not */
        err = at_send_command_singleline("AT+EFD=?", "+EFD:", &p_response, getDefaultChannelCtx(rid));
        RLOGD("Result of AT+EFD => %s", p_response->finalResponse);

        memset(property_value, 0, sizeof(property_value));
        property_get(PROPERTY_FD_SCREEN_OFF_R8_TIMER, property_value, DEFAULT_FD_SCREEN_OFF_R8_TIMER);
        RLOGD("Test FD value=%s,%d", property_value, (int)(atof(property_value) * 10));

        //[New R8 modem FD for test purpose]
        if ((err == 0 && p_response->success == 1) && (strncmp(p_response->finalResponse, "OK", 2) == 0)) {
            int errcode = -1;
            /* TEL FW can query this variable to know if AP side is necessary to execute FD or not */
            errcode = property_set(PROPERTY_RIL_FD_MODE, "1");
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_RIL_FD_MODE, property_value, "0");
            RLOGD("ril.fd.mode=%s, errcode=%d", property_value, errcode);

            RLOGD("Try to get FD related timers from system");
#if 1
            /* [Step#02] Set default FD related timers for mode: format => AT+EFD=2, timer_id, timer_value (unit:0.1sec)  */
            char *timer_value;
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_FD_SCREEN_OFF_TIMER, property_value, DEFAULT_FD_SCREEN_OFF_TIMER);
            RLOGD("Screen Off FD Timer=%s", property_value);
            /* timerId=0: Screen Off + Legancy FD */
            asprintf(&timer_value, "AT+EFD=2,0,%d", (int)(atof(property_value) * 10));
            at_send_command(timer_value, NULL, getDefaultChannelCtx(rid));
            free(timer_value);

            /* timerId=2: Screen Off + R8 FD */
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_FD_SCREEN_OFF_R8_TIMER, property_value, DEFAULT_FD_SCREEN_OFF_R8_TIMER);
            asprintf(&timer_value, "AT+EFD=2,2,%d", (int)(atof(property_value) * 10));
            at_send_command(timer_value, NULL, getDefaultChannelCtx(rid));
            free(timer_value);
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_FD_SCREEN_ON_TIMER, property_value, DEFAULT_FD_SCREEN_ON_TIMER);
            RLOGD("Screen On FD Timer=%s", property_value);
            /* timerId=1: Screen On + Legancy FD */
            asprintf(&timer_value, "AT+EFD=2,1,%d", (int)(atof(property_value) * 10));
            at_send_command(timer_value, NULL, getDefaultChannelCtx(rid));
            free(timer_value);

            /* timerId=3: Screen On + R8 FD */
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_FD_SCREEN_ON_R8_TIMER, property_value, DEFAULT_FD_SCREEN_ON_R8_TIMER);
            asprintf(&timer_value, "AT+EFD=2,3,%d", (int)(atof(property_value) * 10));
            at_send_command(timer_value, NULL, getDefaultChannelCtx(rid));
            free(timer_value);
            memset(property_value, 0, sizeof(property_value));
            property_get(PROPERTY_FD_ON_ONLY_R8_NETWORK, property_value, DEFAULT_FD_ON_ONLY_R8_NETWORK);
            LOGD("%s = %s", PROPERTY_FD_ON_ONLY_R8_NETWORK, property_value);
            if (atoi(property_value) == 1) {
                at_send_command("AT+EPCT=0,4194304", NULL, getDefaultChannelCtx(rid));
            }

            /* [Step#03] Enable FD Mechanism MD: after finishing to set FD related default timer to modem */
            at_send_command("AT+EFD=1", NULL, getDefaultChannelCtx(rid));
#endif
        }
    }
#endif

    at_send_command("AT+EAIC=2", NULL, getDefaultChannelCtx(rid));

    at_send_command("AT+CLIP=1", NULL, getDefaultChannelCtx(rid));

    at_send_command("AT+CNAP=1", NULL, getDefaultChannelCtx(rid));

    // set data/call prefer
    // 0 : call prefer
    // 1 : data prefer
    char gprsPrefer[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.radio.gprs.prefer", gprsPrefer, "0");
    if ((atoi(gprsPrefer) == 0)) {
        // call prefer
        at_send_command("AT+EGTP=1", NULL, getDefaultChannelCtx(rid));
        at_send_command("AT+EMPPCH=1", NULL, getDefaultChannelCtx(rid));
    } else {
        // data prefer
        at_send_command("AT+EGTP=0", NULL, getDefaultChannelCtx(rid));
        at_send_command("AT+EMPPCH=0", NULL, getDefaultChannelCtx(rid));
    }

    /* ALPS00574862 Remove redundant +COPS=3,2;+COPS? multiple cmd in REQUEST_OPERATOR */
    at_send_command("AT+COPS=3,2", NULL, getDefaultChannelCtx(rid));

    // ALPS00353868 START
    err = at_send_command("AT+COPS=3,3",  &p_response, getDefaultChannelCtx(rid));
    RLOGI("AT+COPS=3,3 got err= %d,success=%d", err, p_response->success);

    if (err >= 0 && p_response->success != 0) {
        setPlmnListFormat(rid, 1);
    }
    at_response_free(p_response);
    // ALPS00353868 END

    /* To support get PLMN when not registered to network via AT+EOPS?  START */
    err = at_send_command("AT+EOPS=3,2",  &p_response, getDefaultChannelCtx(rid));
    RLOGI("AT+EOPS=3,2 got err= %d,success=%d", err, p_response->success);

    if (err >= 0 && p_response->success != 0){
        bEopsSupport = 1;
    }
    at_response_free(p_response);
    /* To support get PLMN when not registered to network via AT+EOPS?  END */

    //BEGIN mtk03923 [20110713][Enable +ESIMS URC]
    at_send_command("AT+ESIMS=1", NULL, getDefaultChannelCtx(rid));
    resetAidInfo(rid);
    //END   mtk03923 [20110713][Enable +ESIMS URC]

    //ALPS01228632
#ifdef MTK_SIM_RECOVERY
    at_send_command("AT+ESIMREC=1", NULL, getDefaultChannelCtx(rid));
#else
    at_send_command("AT+ESIMREC=0", NULL, getDefaultChannelCtx(rid));
#endif

    mtk_initializeCallback(param);

    if (isDualTalkMode()) {
        current_share_modem = 1;
    } else {
        current_share_modem = getMtkShareModemCurrent();
    }

#ifdef MTK_TC1_FEATURE
    if (isImsSupport()) {
        at_send_command("AT+EEMCINFO=1", NULL, getDefaultChannelCtx(rid));
        at_send_command("AT+EMOBD=1", NULL, getDefaultChannelCtx(rid));
        at_send_command("AT+ESSAC=1", NULL, getDefaultChannelCtx(rid));
    }
#endif /* MTK_TC1_FEATURE */

    nRadioState = queryRadioState(rid);
#if 1

    // External SIM [Start]
    if (isExternalSimSupport()) {
        requestSwitchExternalSim(rid);
    }
    // External SIM [End]

    if (SIM_COUNT >= 2) {
        if (0 == nRadioState)
            requestSimReset(rid);
        else    /* MTK_GEMINI */
            requestSimInsertStatus(rid);

    RLOGD("start rild bootup flow [%d, %d, %d, %d, %d]", isDualTalkMode(), rid, RIL_is3GSwitched(), current_share_modem, isEVDODTSupport());
    if (isInternationalRoamingEnabled()
        || getTelephonyMode() == 100
        || getTelephonyMode() == 101
        || isEVDODTSupport()) {
        if (isDualTalkMode()) {
            /* 104 and 105 should be add handler here */
            if (rid == RIL_SOCKET_2) {
                flightModeBoot();
                imsInit(rid);

                // Regional Phone: boot animation START
                RLOGD("Regional Phone: boot animation START (DT) [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
                if(isReadMccMncForBootAnimation()) {
#ifdef MTK_RIL_MD2
                    requestMccMncForBootAnimation(RIL_SOCKET_2);
#else
                    requestMccMncForBootAnimation(RIL_SOCKET_1);
#endif
                }
                // Regional Phone: boot animation END
                if (isCdmaLteDcSupport()) {
                    RLOGD("SVLTE ril 1 init");
                    bootupGetIccid(RIL_SOCKET_1);
                    bootupGetImei(RIL_SOCKET_1);
                    bootupGetImeisv(RIL_SOCKET_1);
                    bootupGetBasebandVersion(RIL_SOCKET_1);
                    bootupGetCalData(RIL_SOCKET_1);
                }

                bootupGetIccid(rid);
                bootupGetImei(rid);
                bootupGetImeisv(rid);
                bootupGetBasebandVersion(rid);

                if (isCdmaLteDcSupport() == 0) {
                    bootupGetCalData(rid);
                }
                RLOGD("case 1 get SIM inserted status [%d]", sim_inserted_status);
            }
        } else {
            if (rid == RIL_SOCKET_2) {
                flightModeBoot();
                imsInit(rid);

                // Regional Phone: boot animation START
                RLOGD("Regional Phone: boot animation START (Single) [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
                if(isReadMccMncForBootAnimation()) {
                    requestMccMncForBootAnimation(RIL_SOCKET_1);
                    requestMccMncForBootAnimation(RIL_SOCKET_2);
                }
                // Regional Phone: boot animation END

                bootupGetIccid(RIL_SOCKET_1);
                bootupGetIccid(RIL_SOCKET_2);
                bootupGetImei(RIL_SOCKET_1);
                bootupGetImei(RIL_SOCKET_2);
                bootupGetImeisv(RIL_SOCKET_1);
                bootupGetImeisv(RIL_SOCKET_2);
                bootupGetBasebandVersion(RIL_SOCKET_1);
                bootupGetBasebandVersion(RIL_SOCKET_2);
                bootupGetCalData(RIL_SOCKET_1);
                RLOGD("case 2 get SIM inserted status [%d]", sim_inserted_status);
            }
        }
    } else {
        if ((!isDualTalkMode() && SIM_COUNT == 2 && rid == RIL_SOCKET_2) ||
            (isDualTalkMode() && (rid == RIL_SOCKET_1))
           ) {
            flightModeBoot();
            imsInit(rid);
            if (isDualTalkMode()) {

                // Regional Phone: boot animation START
                RLOGD("Regional Phone: boot animation START (DT) [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
                if(isReadMccMncForBootAnimation()) {
#ifdef MTK_RIL_MD2
                    requestMccMncForBootAnimation(RIL_SOCKET_2);
#else
                    requestMccMncForBootAnimation(RIL_SOCKET_1);
#endif
                }
                // Regional Phone: boot animation END

                bootupGetIccid(rid); //query ICCID after AT+ESIMS
                bootupGetImei(rid);
                bootupGetImeisv(rid);
                bootupGetBasebandVersion(rid);
                if (rid == RIL_SOCKET_1) {
                    bootupGetCalData(rid);
                    RLOGD("case 3 bootupGetCalData");
                }
            } else {
                // Regional Phone: boot animation START
                RLOGD("Regional Phone: boot animation START (Single) [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
                if(isReadMccMncForBootAnimation()) {
                    requestMccMncForBootAnimation(RIL_SOCKET_1);
                    requestMccMncForBootAnimation(RIL_SOCKET_2);
                }
                // Regional Phone: boot animation END

                // get iccid of main protocol first for data requirement
                // needs to decide EGTYP arguments for auto attached
                bootupGetIccid(getMainProtocolRid());
                for (i=0; i<SIM_COUNT; i++) {
                    if (i == (int)(getMainProtocolRid())) continue;
                    bootupGetIccid(i);
                }
                bootupGetImei(RIL_SOCKET_1);
                bootupGetImei(RIL_SOCKET_2);
                bootupGetImeisv(RIL_SOCKET_1);
                bootupGetImeisv(RIL_SOCKET_2);
                bootupGetBasebandVersion(RIL_SOCKET_1);
                bootupGetBasebandVersion(RIL_SOCKET_2);
                bootupGetCalData(RIL_SOCKET_2);
            }
            RLOGD("case 3 get SIM inserted status [%d]", sim_inserted_status);
        } else if (current_share_modem == 1 && rid == RIL_SOCKET_1) {
            flightModeBoot();
            imsInit(rid);

            // Regional Phone: boot animation START
            RLOGD("Regional Phone: boot animation START (Single) [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
            if(isReadMccMncForBootAnimation()) {
                requestMccMncForBootAnimation(rid);
            }
            // Regional Phone: boot animation END

            bootupGetIccid(rid);
            bootupGetImei(rid);
            bootupGetImeisv(rid);
            bootupGetBasebandVersion(rid);
            bootupGetCalData(rid);
            RLOGD("get SIM inserted status (Single) [%d]", sim_inserted_status);
        }  else if (rid == RIL_SOCKET_2 + (SIM_COUNT - 2)) {
            int i;
            flightModeBoot();
            imsInit(rid);

            // get iccid of main protocol first for data requirement
            // needs to decide EGTYP arguments for auto attached
            bootupGetIccid(getMainProtocolRid());

            for (i = 0; i < SIM_COUNT; i++) {
                // Regional Phone: boot animation START
                RLOGD("Regional Phone: boot animation START [%d,%d,%d]", current_share_modem, rid, isReadMccMncForBootAnimation());
                if(isReadMccMncForBootAnimation()) {
                    // assume: current_share_modem == 2
                    requestMccMncForBootAnimation(RIL_SOCKET_1 + i);
                }
                // Regional Phone: boot animation END


                if (i != (int)(getMainProtocolRid())) {
                    bootupGetIccid(RIL_SOCKET_1 + i);
                }
                bootupGetImei(RIL_SOCKET_1 + i);
                bootupGetImeisv(RIL_SOCKET_1 + i);
                bootupGetBasebandVersion(RIL_SOCKET_1 + i);
            }
            bootupGetCalData(RIL_SOCKET_1);

                        RLOGD("rid=%d,report sim_inserted_status [%d]", rid, sim_inserted_status);
                    }
                }
    }else {
        flightModeBoot();

        imsInit(rid);

        requestSimInsertStatus(rid);

        if(isReadMccMncForBootAnimation()) {
            requestMccMncForBootAnimation(RIL_SOCKET_1);
        }

        bootupGetIccid(rid);
        bootupGetImei(rid);
        bootupGetImeisv(rid);
        bootupGetBasebandVersion(rid);
        bootupGetCalData(rid);
    }
#else
    if (0 == nRadioState)
        requestSimReset(rid);
    else    /* MTK_GEMINI */
        requestSimInsertStatus(rid);

#endif
    requestGetPacketSwitchBearer(rid);

    requestPhbStatus(rid);

    updateNitzOperInfo(rid);

    // GCG switcher feature
    requestGetGcfMode(rid);
    // GCG switcher feature

    // This is used for wifi-onlg version load
    // Since RIL is not connected to RILD in wifi-only version
    // we query it and stored into a system property
    // note: since this patch has no impact to nomal load, do this in normal initial procedure
    requestSN(rid);

    initCFUQueryType();

    /* To check if new SS service class feature is supported or not */
    determineSSServiceClassFeature(rid);

    /* assume radio is off on error */
    if (isRadioOn(rid) > 0) {
        setRadioState(RADIO_STATE_ON, rid);
    }
    bootupSetRadio(rid);
}
#else   /* MTK_RIL */
static void initializeCallback(void *param)
{
    ATResponse *p_response = NULL;
    int err;

    setRadioState(RADIO_STATE_OFF);

    at_handshake();

    /* note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed */

    /*  atchannel is tolerant of echo but it must */
    /*  have verbose result codes */
    at_send_command("ATE0Q0V1", NULL);

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL);

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL);

    /*  Network registration events */
    err = at_send_command("AT+CREG=2", &p_response);

    /* some handsets -- in tethered mode -- don't support CREG=2 */
    if (err < 0 || p_response->success == 0)
        at_send_command("AT+CREG=1", NULL);

    at_response_free(p_response);

    /*  GPRS registration events */
    at_send_command("AT+CGREG=1", NULL);

    /*  Call Waiting notifications */
    at_send_command("AT+CCWA=1", NULL);

    /*  Alternating voice/data off */
    at_send_command("AT+CMOD=0", NULL);

    /*  Not muted */
    at_send_command("AT+CMUT=0", NULL);

    /*  +CSSU unsolicited supp service notifications */
    at_send_command("AT+CSSN=0,1", NULL);

    /*  no connected line identification */
    at_send_command("AT+COLP=0", NULL);

    /*  HEX character set */
    at_send_command("AT+CSCS=\"HEX\"", NULL);

    /*  USSD unsolicited */
    at_send_command("AT+CUSD=1", NULL);

    /*  Enable +CGEV GPRS event notifications, but don't buffer */
    at_send_command("AT+CGEREP=1,0", NULL);

    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL);

#ifdef  USE_TI_COMMANDS
    at_send_command("AT%CPI=3", NULL);

    /*  TI specific -- notifications when SMS is ready (currently ignored) */
    at_send_command("AT%CSTAT=1", NULL);
#endif  /* USE_TI_COMMANDS */

    /* assume radio is off on error */
    if (isRadioOn() > 0) {
        setRadioState (RADIO_STATE_ON);
    }
}
#endif  /* MTK_RIL */

static void waitForClose()
{
    pthread_mutex_lock(&s_state_mutex);

    while (s_closed == 0)
        pthread_cond_wait(&s_state_cond, &s_state_mutex);

    pthread_mutex_unlock(&s_state_mutex);
}


/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
static void onUnsolicited(const char *s, const char *sms_pdu, void *pChannel)
{
    char *line = NULL;
    int err;
    RIL_RadioState radioState = sState;
    RILChannelCtx *p_channel = (RILChannelCtx *)pChannel;

    if (RIL_SOCKET_2 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState2;
    } else if (RIL_SOCKET_3 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState3;
    } else if (RIL_SOCKET_4 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState4;
    }

    /* Ignore unsolicited responses until we're initialized.
     * This is OK because the RIL library will poll for initial state
     */
    //[ALPS02277365] some URC need to report under world mode switching
    //if (radioState == RADIO_STATE_UNAVAILABLE)
    if (radioState == RADIO_STATE_UNAVAILABLE && isWorldModeSwitching(s))
        return;

    //MTK-START [mtk04070][111213][ALPS00093395] ATCI for unsolicited response
    char atci_urc_enable[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.service.atci_urc.enable", atci_urc_enable, "0");
    if ((NULL != s) && (atoi(atci_urc_enable) == 1))
        RIL_onUnsolicitedResponse(RIL_UNSOL_ATCI_RESPONSE, s, strlen(s), getRILIdByChannelCtx(p_channel));
    //MTK-END [mtk04070][111213][ALPS00093395] ATCI for unsolicited response

    //MTK-START: MAL AT: relay the at urc to MAL socket
    if ((NULL != s))
    {
        RIL_onUnsolicitedResponse(RIL_UNSOL_MAL_AT_INFO, s, strlen(s)+1, getRILIdByChannelCtx(p_channel));
        if (isSMSUnsolicited(s))
        {
            RIL_onUnsolicitedResponse(RIL_UNSOL_MAL_AT_INFO, sms_pdu, strlen(sms_pdu)+1, getRILIdByChannelCtx(p_channel));
        }
    }
    //MTK-END

    if (!(rilNwUnsolicited(s, sms_pdu, p_channel) ||
          rilCcUnsolicited(s, sms_pdu, p_channel) ||
          rilSsUnsolicited(s, sms_pdu, p_channel) ||
          rilSmsUnsolicited(s, sms_pdu, p_channel) ||
          rilStkUnsolicited(s, sms_pdu, p_channel) ||
          rilOemUnsolicited(s, sms_pdu, p_channel) ||
          rilDataUnsolicited(s, sms_pdu, p_channel) ||
          rilSimUnsolicited(s, sms_pdu, p_channel)))
        RLOGE("Unhandled unsolicited result code: %s\n", s);
}

int isWorldModeSwitching(const char *s)
{
    int needIgnore = 1;
    if (bWorldModeSwitching) {
        if (strStartsWith(s, "+EUSIM:")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC+EUSIM");
            needIgnore = 0;
        } else if (strStartsWith(s, "+STKPCI:")){
            RLOGD("WorldModeSwitching=true, don't ignore URC+STKPCI");
            needIgnore = 0;
        }
    } else {
        needIgnore = 1;
    }
    return needIgnore;
}

#ifdef  MTK_RIL
/* Called on command or reader thread */
static void onATReaderClosed(RILChannelCtx *p_channel)
{
    RLOGI("AT channel closed\n");
    at_close(p_channel);
    assert(0);
    s_closed = 1;

    setRadioState(RADIO_STATE_UNAVAILABLE, getRILIdByChannelCtx(p_channel));
}

/* Called on command thread */
static void onATTimeout(RILChannelCtx *p_channel)
{
    RLOGI("AT channel timeout; closing\n");
    at_close(p_channel);
    assert(0);
    s_closed = 1;

    /* FIXME cause a radio reset here */

    setRadioState(RADIO_STATE_UNAVAILABLE, getRILIdByChannelCtx(p_channel));
}
#else   /* MTK_RIL */
/* Called on command or reader thread */
static void onATReaderClosed()
{
    RLOGI("AT channel closed\n");
    at_close();
    s_closed = 1;

    setRadioState(RADIO_STATE_UNAVAILABLE);
}

/* Called on command thread */
static void onATTimeout()
{
    RLOGI("AT channel timeout; closing\n");
    at_close();

    s_closed = 1;

    /* FIXME cause a radio reset here */

    setRadioState(RADIO_STATE_UNAVAILABLE);
}
#endif  /* MTK_RIL */

static void usage(char *s)
{
#ifdef  RIL_SHLIB
    fprintf(stderr, "reference-ril requires: -p <tcp port> or -d /dev/tty_device\n");
#else   /* RIL_SHLIB */
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif  /* RIL_SHLIB */
}


#ifdef  MTK_RIL
/* These nodes are created by gsm0710muxd */
#define RIL_SUPPORT_CHANNELS_MAX_NAME_LEN 32
char s_mux_path[RIL_SUPPORT_CHANNELS][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {
#ifdef MTK_RIL_MD2
    "/dev/radio/pttynoti-md2",
    "/dev/radio/pttycmd1-md2",
    "/dev/radio/pttycmd2-md2",
    "/dev/radio/pttycmd3-md2",
    "/dev/radio/pttycmd4-md2",
    "/dev/radio/atci1-md2",

    "/dev/radio/ptty2noti-md2",
    "/dev/radio/ptty2cmd1-md2",
    "/dev/radio/ptty2cmd2-md2",
    "/dev/radio/ptty2cmd3-md2",
    "/dev/radio/ptty2cmd4-md2",
    "/dev/radio/atci2-md2",

#else
    "/dev/radio/pttynoti",
    "/dev/radio/pttycmd1",
    "/dev/radio/pttycmd2",
    "/dev/radio/pttycmd3",
    "/dev/radio/pttycmd4",
    "/dev/radio/atci1",

    "/dev/radio/ptty2noti",
    "/dev/radio/ptty2cmd1",
    "/dev/radio/ptty2cmd2",
    "/dev/radio/ptty2cmd3",
    "/dev/radio/ptty2cmd4",
    "/dev/radio/atci2",

    "/dev/radio/ptty3noti",
    "/dev/radio/ptty3cmd1",
    "/dev/radio/ptty3cmd2",
    "/dev/radio/ptty3cmd3",
    "/dev/radio/ptty3cmd4",
    "/dev/radio/atci3",

    "/dev/radio/ptty4noti",
    "/dev/radio/ptty4cmd1",
    "/dev/radio/ptty4cmd2",
    "/dev/radio/ptty4cmd3",
    "/dev/radio/ptty4cmd4",
    "/dev/radio/atci4",
#endif /* #ifdef MTK_RIL_MD2 */
};

static void switchMuxPath() {
    RLOGD("switchMuxPath start");
    if (SIM_COUNT >= 2) {
#ifndef MTK_RIL_MD2
        char prop_value[PROPERTY_VALUE_MAX] = { 0 };
        char s_mux_path_tmp[RIL_CHANNEL_OFFSET][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {0};
        int targetSim = 0;
        int i;

        property_get(PROPERTY_3G_SIM, prop_value, "1");

        targetSim = atoi(prop_value);
        RLOGD("targetSim : %d", targetSim);
        if(targetSim*RIL_CHANNEL_OFFSET > getSupportChannels()) {
            RLOGD("!!!! targetSim*RIL_CHANNEL_OFFSET > RIL_SUPPORT_CHANNELS");
        } else if(targetSim != 1){
            // exchange mux channel for SIM switch

            for(i=0;i<RIL_CHANNEL_OFFSET; i++) {
                strcpy(s_mux_path_tmp[i], s_mux_path[(targetSim-1)*RIL_CHANNEL_OFFSET+i]);
                strcpy(s_mux_path[(targetSim-1)*RIL_CHANNEL_OFFSET+i], s_mux_path[i]);
                strcpy(s_mux_path[i], s_mux_path_tmp[i]);
            }

        }
        for(i=0;i<getSupportChannels(); i++) {
            RLOGD("s_mux_path[%d] = %s", i, s_mux_path[i]);
        }

#endif // MTK_RIL_MD2
    }
    RLOGD("switchMuxPath end");
}

static void doSWSimSwitch(int target3GSim)
{
    set3GCapability(NULL, target3GSim);
    //requestResetRadio(NULL, 0, NULL);
    property_set("ril.mux.report.case", "6");
    //property_set("ctl.start", "muxreport-daemon");
    property_set("ril.muxreport", "1");
    s_main_loop = 0;
}

static int performSwSimSwitchForCL(int sim3G){
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int old3GSim = queryMainProtocol();

    RLOGI("performSwSimSwitchForCL : %d, %d", sim3G, old3GSim);
    if (old3GSim == sim3G) {
        RLOGI("Alrady do sim switch for CL");
		// continue init because sim switch for CL is finished
        return 1;
    }
    /* Gemini+ , +ES3G response 1: SIM1 , 2: SIM2 , 4:SIM3 ,8: SIM4.  */
    int modem_setting_value = CAPABILITY_3G_SIM1 << (sim3G-1);
    if(isLteSupport()) {
        asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_GSM_UMTS_LTE);
    } else {
        asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_WCDMA_PREF);
    }

    err = at_send_command(cmd, &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
    free(cmd);
    if (err < 0 || p_response->success == 0) {
		LOGI("do sim switch fail, error:%d", err);
        at_response_free(p_response);
        return 0;
    }
    RLOGI("reset modem");
    property_set("gsm.ril.eboot", "1");
    resetRadio();
    at_response_free(p_response);
	return 0;
}

static int performSWSimSwitchIfNecessary(int sim3G)
{
    int continueInit = 1;
    int telephonyMode = getTelephonyMode();
    int firstModem = getFirstModem();
    RLOGI("current telephony mode and 3G SIM: [mode:%d, 3G SIM:%d, FirstMD:%d]", telephonyMode, sim3G, firstModem);

    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.telephony.cl.config", prop_value, NULL);
    if (prop_value[0] == '1') {
        continueInit = performSwSimSwitchForCL(CAPABILITY_3G_SIM2);
		RLOGI("MTK_CL_SUPPORT performSWSimSwitchIfNecessary, continueInit:%d", continueInit);
    }

    switch (telephonyMode) {
        case TELEPHONY_MODE_5_WGNTG_DUALTALK:
        case TELEPHONY_MODE_6_TGNG_DUALTALK:
        case TELEPHONY_MODE_7_WGNG_DUALTALK:
        case TELEPHONY_MODE_8_GNG_DUALTALK:
#ifdef MTK_RIL_MD2
            if (sim3G == CAPABILITY_3G_SIM1) {
                RLOGI("MD2 need to do default SW switch [%d, %d]", firstModem, telephonyMode);
                doSWSimSwitch(CAPABILITY_3G_SIM2);
                continueInit = 0;
            } else {
                s_isSimSwitched = 1;
            }
#else
            s_isSimSwitched = 0;
#endif
            break;
        case TELEPHONY_MODE_100_TDNC_DUALTALK:
#ifdef MTK_RIL_MD2
            if (!isInternationalRoamingEnabled()) {
                s_isSimSwitched = 1;
            } else if (sim3G == CAPABILITY_3G_SIM1 && !isInternationalRoamingEnabled()) {
                RLOGI("MD2 need to do default SW switch [%d, %d]", firstModem, telephonyMode);
                doSWSimSwitch(CAPABILITY_3G_SIM2);
                continueInit = 0;
            } else {
                s_isSimSwitched = 1;
            }
#else
            s_isSimSwitched = 0;
#endif
            break;
        case TELEPHONY_MODE_101_FDNC_DUALTALK:
            s_isSimSwitched = 0;
            break;
        default:
            if (sim3G >= CAPABILITY_3G_SIM2)
                s_isSimSwitched = 1;
            else
                s_isSimSwitched = 0;
            break;
    }
    return continueInit;
}

static void sendRadioCapabilityDoneIfNeeded () {
    int sim3G = queryMainProtocol();
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int sessionId = 0;

    property_get(PROPERTY_SET_RC_SESSION_ID[0], property_value, "-1");
    sessionId = atoi(property_value);
    RLOGI("sendRadioCapabilityDoneIfNeeded sessionId:%d", sessionId);
    //if (sessionId != 0) {
        RIL_RadioCapability* rc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
        memset(rc, 0, sizeof(RIL_RadioCapability));
        rc->version = RIL_RADIO_CAPABILITY_VERSION;
        rc->session = sessionId;
        rc->phase = RC_PHASE_UNSOL_RSP;
        rc->rat = RAF_GSM;
        rc->status = RC_STATUS_SUCCESS;

        RLOGI("sim switch done, send URC, sim3G %d", sim3G);

        switch(sim3G) {
            case CAPABILITY_3G_SIM2:
                strcpy(rc->logicalModemUuid, s_logicalModemId[1]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_1);
                if (SIM_COUNT >= 3) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[2]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_3);
                }
                if (SIM_COUNT >= 4) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[3]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_4);
                }

                if (isLteSupport()) {
                    rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
                } else {
                    rc->rat = RAF_GSM | RAF_UMTS;
                }
                strcpy(rc->logicalModemUuid, s_logicalModemId[0]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_2);
                break;
            case CAPABILITY_3G_SIM3:
                strcpy(rc->logicalModemUuid, s_logicalModemId[2]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_1);
                strcpy(rc->logicalModemUuid, s_logicalModemId[1]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_2);
                if (SIM_COUNT >= 4) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[3]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_4);
                }

                if (isLteSupport()) {
                    rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
                } else {
                    rc->rat = RAF_GSM | RAF_UMTS;
                }
                strcpy(rc->logicalModemUuid, s_logicalModemId[0]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_3);
                break;
            case CAPABILITY_3G_SIM4:
                strcpy(rc->logicalModemUuid, s_logicalModemId[3]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_1);
                strcpy(rc->logicalModemUuid, s_logicalModemId[1]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_2);
                strcpy(rc->logicalModemUuid, s_logicalModemId[2]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_3);
                if (isLteSupport()) {
                    rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
                } else {
                    rc->rat = RAF_GSM | RAF_UMTS;
                }
                strcpy(rc->logicalModemUuid, s_logicalModemId[0]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_4);
                break;
            default:
            case CAPABILITY_3G_SIM1:
                if (SIM_COUNT >= 2) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[1]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_2);
                }
                if (SIM_COUNT >= 3) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[2]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_3);
                }
                if (SIM_COUNT >= 4) {
                    strcpy(rc->logicalModemUuid, s_logicalModemId[3]);
                    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_4);
                }

                if (isLteSupport()) {
                    rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
                } else {
                    rc->rat = RAF_GSM | RAF_UMTS;
                }
                strcpy(rc->logicalModemUuid, s_logicalModemId[0]);
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), RIL_SOCKET_1);
                break;
        }
    //}
}

static void *mainLoop(void *param)
{
    int ret;
    int i;
    RILChannelCtx *p_channel;
    int curr_share_modem = 0;
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };

    AT_DUMP("== ", "entering mainLoop()", -1);
    property_get("mux.debuglog.enable", prop_value, NULL);
    if (prop_value[0] == '1') {
        RLOGI("enable full log of mtk-ril.so");
            mtk_ril_log_level = 1;
    }

    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);
    initRILChannels();
    getLogicModemId();
    switchMuxPath();
    while (s_main_loop) {
        for (i = 0; i < getSupportChannels(); i++) {
            p_channel = getChannelCtxbyId(i);

            while (p_channel->fd < 0) {
                do
                    p_channel->fd = open(s_mux_path[i], O_RDWR);
                while (p_channel->fd < 0 && errno == EINTR);

                if (p_channel->fd < 0) {
                    perror("opening AT interface. retrying...");
                    RLOGE("could not connect to %s: %s",
                          s_mux_path[i], strerror(errno));
                    sleep(10);
                    /* never returns */
                } else {
                    struct termios ios;
                    tcgetattr(p_channel->fd, &ios);
                    ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                    ios.c_iflag = 0;
                    tcsetattr(p_channel->fd, TCSANOW, &ios);
                }
            }


            s_closed = 0;
            ret = at_open(p_channel->fd, onUnsolicited, p_channel);


            if (ret < 0) {
                RLOGE("AT error %d on at_open\n", ret);
                return 0;
            }
        }

        //#ifdef MTK_RIL_MD1
        // rome dsda 4g sim switch should avoid effecting EVDO project.
        if (isDualTalkMode() && !isEVDODTSupport())
        {
            int simSwitchMode = getSimSwitchMode(0);
            LOGI("current simSwitchMode: %d", simSwitchMode);
        }
        //#endif
        int sim3G = queryMainProtocol();
        //reset the sim switching property
        property_set("ril.cdma.switching", "0");
        if (sim3G != RIL_get3GSIM()) {
            RLOGE("sim switch property is different with modem ES3G?, sync it with TRM");
            setSimSwitchProp(sim3G);
            property_set("ril.mux.report.case", "2");
            property_set("ril.muxreport", "1");
            return 0;
        }
        if (isDisableCapabilitySwitch() == 1) {
            if (sim3G != 1) {
                setSimSwitchProp(1);
                property_set("ril.mux.report.case", "2");
                property_set("ril.muxreport", "1");
                return 0;
            }
        }
        sendRadioCapabilityDoneIfNeeded();

        // update sim switch status when booting
        setSimSwitchProp(sim3G);

        if (performSWSimSwitchIfNecessary(sim3G)) {
            initialCidTable();

            //the first time use AT command to get the capability
            getAttachApnSupport();

            if (isDualTalkMode()) {
                RLOGI("DualTalk mode and default switch status: %d", s_isSimSwitched);
            } else {
                RLOGI("Is Sim Switch now: [%d], Sim SIM mapped for framework: [SIM%d]", s_isSimSwitched, sim3G);
                char* value = NULL;
                asprintf(&value, "%d", sim3G);
                property_set(PROPERTY_3G_SIM, value);
                free(value);
            }

            //Set ril init flag
            property_set("gsm.ril.init", "1");
            RLOGI("set gsm.ril.init to 1");

            RLOGI("Start initialized callback");
            RIL_requestTimedCallback(initializeCallback, (void *)&s_pollSimId, &TIMEVAL_0);

            //BEGIN mtk03923 [20110711][Ingore SIM2 when SHARED_MODEM=single]
            if (SIM_COUNT >= 2) {
                if (isInternationalRoamingEnabled()
                    || getTelephonyMode() == 100
                    || getTelephonyMode() == 101
                    || isEVDODTSupport()) {
                    curr_share_modem = 2;
                } else {
                    if (isDualTalkMode()) {
                        curr_share_modem = 1;
                    } else {
                        curr_share_modem = getMtkShareModemCurrent();
                    }
                }

                switch (curr_share_modem) {
                    case 2:
                    case 3:
                    case 4:
                        RLOGI("initializeCallback ril2");
                        RIL_requestTimedCallback(initializeCallback, (void *)&s_pollSimId2, &TIMEVAL_0);
                        break;
                    default:
                        break;
                }
            }
            if (SIM_COUNT >= 3) {
                RLOGI("initializeCallback ril3");
                RIL_requestTimedCallback(initializeCallback, (void *)&s_pollSimId3, &TIMEVAL_0);
            }
            if (SIM_COUNT >= 4) {
                RLOGI("initializeCallback ril4");
                RIL_requestTimedCallback(initializeCallback, (void *)&s_pollSimId4, &TIMEVAL_0);
            }

            // Give initializeCallback a chance to dispatched, since
            // we don't presently have a cancellation mechanism
            sleep(1);
            //END   mtk03923 [20110711][Ingore SIM2 when SHARED_MODEM=single]
        }

        waitForClose();
        RLOGI("Re-opening after close");
    }
    RLOGI("Main loop exit");
    return 0;
}
#else   /* MTK_RIL */
static void *mainLoop(void *param)
{
    int fd;
    int ret;
    char path[50];
    int ttys_index;

    AT_DUMP("== ", "entering mainLoop()", -1);
    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);

    for (;; ) {
        fd = -1;
        while (fd < 0) {
            if (s_port > 0) {
                fd = socket_loopback_client(s_port, SOCK_STREAM);
            } else if (s_device_socket) {
                if (!strcmp(s_device_path, "/dev/socket/qemud")) {
                    /* Qemu-specific control socket */
                    fd = socket_local_client("qemud", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
                    if (fd >= 0) {
                        char answer[2];

                        if (write(fd, "gsm", 3) != 3 ||
                            read(fd, answer, 2) != 2 ||
                            memcmp(answer, "OK", 2) != 0) {
                            close(fd);
                            fd = -1;
                        }
                    }
                } else {
                    fd = socket_local_client(s_device_path, ANDROID_SOCKET_NAMESPACE_FILESYSTEM, SOCK_STREAM);
                }
            } else if (s_device_path != NULL) {
                fd = open(s_device_path, O_RDWR);
                if (fd >= 0 && !memcmp(s_device_path, "/dev/ttyS", 9)) {
                    /* disable echo on serial ports */
                    struct termios ios;
                    tcgetattr(fd, &ios);
                    ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                    ios.c_iflag = 0;
                    tcsetattr(fd, TCSANOW, &ios);
                }
            }
#if 0
            else if (s_device_range_begin >= 0 && s_device_range_end >= 0) {
                RLOGD("Open ttyS....");
                ttys_index = s_device_range_begin;
                while (ttys_index <= s_device_range_end) {
                    sprintf(path, "/dev/ttyS%d", ttys_index);
                    fd = open(path, O_RDWR);
                    if (fd >= 0) {
                        /* disable echo on serial ports */
                        struct termios ios;
                        tcgetattr(fd, &ios);
                        ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                        ios.c_iflag = 0;
                        tcsetattr(fd, TCSANOW, &ios);
                    } else {
                        RLOGE("Can't open the device /dev/ttyS%d: %s", ttys_index, strerror(errno));
                    }
                    ttys_index++;
                }
            }
#endif

            if (fd < 0) {
                perror("opening AT interface. retrying...");
                sleep(10);
                /* never returns */
            }
        }

        RLOGD("FD: %d", fd);

        s_closed = 0;
        ret = at_open(fd, onUnsolicited);

        if (ret < 0) {
            RLOGE("AT error %d on at_open\n", ret);
            return 0;
        }

        RIL_requestTimedCallback(initializeCallback, NULL, &TIMEVAL_0);

        // Give initializeCallback a chance to dispatched, since we don't presently have a cancellation mechanism
        sleep(1);

        waitForClose();
        RLOGI("Re-opening after close");
    }
}
#endif  /* MTK_RIL */

#ifdef  MTK_RIL
RIL_RadioState getRadioState(RIL_SOCKET_ID rid)
{
    RIL_RadioState radioState = sState;

    if (RIL_SOCKET_2 == rid) {
        radioState = sState2;
    } else if (RIL_SOCKET_3 == rid) {
        radioState = sState3;
    } else if (RIL_SOCKET_4 == rid) {
        radioState = sState4;
    }

    RLOGI("getRadioState(): radioState=%d\n", radioState);

    return radioState;
}
#else   /* MTK_RIL */
RIL_RadioState getRadioState(void)
{
    return sState;
}
#endif  /* MTK_RIL */


#ifdef  MTK_RIL
void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid)
#else
void setRadioState(RIL_RadioState newState)
#endif  /* MTK_RIL */
{
    RIL_RadioState oldState;
    RIL_RadioState *pState = NULL;

    pthread_mutex_lock(&s_state_mutex);

    oldState = sState;
    pState = &sState;
    if (RIL_SOCKET_2 == rid) {
        oldState = sState2;
        pState = &sState2;
    } else if (RIL_SOCKET_3 == rid) {
        oldState = sState3;
        pState = &sState3;
    } else if (RIL_SOCKET_4 == rid) {
        oldState = sState4;
        pState = &sState4;
    }

    if (s_closed > 0) {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        assert(0);
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (*pState != newState || s_closed > 0) {
        *pState = newState;
        assert(0);
        pthread_cond_broadcast(&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);

    RLOGI("setRadioState(%d): newState=%d, oldState=%d, *pState=%d\n", rid, newState, oldState, *pState);

    /* do these outside of the mutex */
    if (*pState != oldState) {
        RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0, rid);
        // notify IMS
        IMS_RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0, rid);

        // Sim state can change as result of radio state change
        RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                                    NULL, 0, rid);

        if (*pState == RADIO_STATE_ON) {
            onRadioPowerOn(rid);
        }
    }
}


#ifdef  RIL_SHLIB
pthread_t s_tid_mainloop;

/* 0: vendor RIL API wihtout socket id
   1: vendor RIL API with socket id */
const int RIL_MultiSocketInRild(void)
{
    return 1;
}

const RIL_RadioFunctionsSocket *RIL_InitSocket(const struct RIL_EnvSocket *env, int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;
    int index;
    char *tmp;
    pthread_attr_t attr;
    char path[50];
    int ttys_index, i;

    s_rilenv = env;

    while (-1 != (opt = getopt(argc, argv, "p:d:s:m:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                    return NULL;
                }
                RLOGI("Opening loopback port %d\n", s_port);
                break;

            case 'd':
                s_device_path = optarg;
                RLOGI("Opening tty device %s\n", s_device_path);
                break;

            case 's':
                s_device_path = optarg;
                s_device_socket = 1;
                RLOGI("Opening socket %s\n", s_device_path);
                RLOGD("Got device!");
                break;
            case 'c':
                RLOGD("skip the argument");
                break;

#ifdef  MTK_RIL
            case 'm':
                RLOGD("Input range: %s %s %s", optarg, argv[1], argv[2]);

                tmp = strtok(optarg, " ");
                s_device_range_begin = atoi(tmp);
                tmp = strtok(NULL, " ");
                s_device_range_end = atoi(tmp);

                if ((s_device_range_end - s_device_range_begin + 1) != getSupportChannels()) {
                    RLOGE("We can't accept the input configuration for muliple channel since we need %d COM ports", getSupportChannels());
                    return NULL;
                }

                RLOGD("Open the ttyS%d to ttyS%d", s_device_range_begin, s_device_range_end);

                RLOGD("Link ttyS....");
                ttys_index = s_device_range_begin;
                i = 0;
                switchMuxPath();
                while (ttys_index <= s_device_range_end) {
                    sprintf(path, "/dev/ttyS%d", ttys_index);
                    RLOGD("Unlock %s on Link %s", path, s_mux_path[i]);
                    /*if(chmod(path, 0666) < 0 )
                     * {
                     *  RLOGD("chomod: system-error: '%s' (code: %d)", strerror(errno), errno);
                     *  return NULL;
                     * }*/
                    if (symlink(path, s_mux_path[i]) < 0) {
                        RLOGD("symlink: system-error: '%s' (code: %d)", strerror(errno), errno);
                        return NULL;
                    }
                    ttys_index++;
                    i++;
                }
                break;
#endif  /* MTK_RIL */

            default:
                usage(argv[0]);
                RLOGD("1- return null, %c", opt);
                //return NULL;
        }
    }

    if (s_port < 0 && s_device_path == NULL &&
        (s_device_range_begin < 0 || s_device_range_end < 0)) {
        usage(argv[0]);
        return NULL;
    }

    /**
     * Call this once at startup to register ims socket
     */
    RLOGD("IMS: IMS_RILA_register");
    if (isImsAndVolteSupport()) {
        IMS_RILA_register(NULL);
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);

    return &s_callbacks;
}

const RIL_RadioFunctionsSocket *RIL_SAP_Init(const struct RIL_EnvSocket *env, int argc,
        char **argv) {
    s_rilsapenv = env;
    return &s_sapcallbacks;
}
#else   /* RIL_SHLIB */
int main(int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while (-1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0)
                    usage(argv[0]);
                RLOGI("Opening loopback port %d\n", s_port);
                break;

            case 'd':
                s_device_path = optarg;
                RLOGI("Opening tty device %s\n", s_device_path);
                break;

            case 's':
                s_device_path = optarg;
                s_device_socket = 1;
                RLOGI("Opening socket %s\n", s_device_path);
                break;

            default:
                usage(argv[0]);
        }
    }

    if (s_port < 0 && s_device_path == NULL)
        usage(argv[0]);

    RIL_register(&s_callbacks);

    mainLoop(NULL);

    return 0;
}
#endif  /* RIL_SHLIB */

int RIL_is3GSwitched()
{
#if 0
    int is3GSwitched = 0;

    char *sim = calloc(1, sizeof(char) * 2);

    property_get(PROPERTY_3G_SIM, sim, "1");

    if (strcmp(sim, CAPABILITY_3G_SIM2) == 0)
        is3GSwitched = 1;

    free(sim);

    return is3GSwitched;
#else
    return s_isSimSwitched;
#endif
}

/*********************************
    [Telephony Mode Definition]
        0: default
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
    //s_telephonyMode = MTK_TELEPHONY_MODE;
    s_telephonyMode = 0;
    if (s_telephonyMode < 0) {
        s_telephonyMode = 0;
    }
    return s_telephonyMode;
}

int RIL_get3GSIM()
{
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    property_get(PROPERTY_3G_SIM, tmp, "1");
    simId = atoi(tmp);
    return simId;
}

void setSimSwitchProp(int SimId)
{
    char* simSwitchValue = NULL;
    int info = SimId-1;
    char tmp[PROPERTY_VALUE_MAX] = { 0 };

    asprintf(&simSwitchValue, "%d", SimId);
    property_set(PROPERTY_3G_SIM, simSwitchValue);
    free(simSwitchValue);
    property_get(PROPERTY_ICCID_SIM[SimId-1], tmp, "");
    if (strlen(tmp) > 0) {
        property_set("persist.radio.simswitch.iccid", tmp);
        RLOGI("write simswitch iccid : %s", tmp);
    }
     //notify the MAIN SIM status change by RIL_SOCKET_1
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_MAIN_SIM_INFO,
                                    &info, sizeof(int), RIL_SOCKET_1);
}

int isEmulatorRunning()
{
    int isEmulatorRunning = 0;
    char *qemu = calloc(1, sizeof(char) * PROPERTY_VALUE_MAX);
    property_get("ro.kernel.qemu", qemu, "");
    if (strcmp(qemu, "1") == 0)
        isEmulatorRunning = 1;

    free(qemu);
    return isEmulatorRunning;
}

int isCCCIPowerOffModem()
{
    /* Return 1 if CCCI support power-off modem completely and deeply.
       If not support, we switch on/off modem by AT+EPON and AT+EPOF */
#ifdef MTK_MD_SHUT_DOWN_NT
    return 1;
#else
    return 0;
#endif
}

int getFirstModem()
{
    //here we could merge with external modem mechanism
    int result = FIRST_MODEM_NOT_SPECIFIED;
    int telephonyMode = getTelephonyMode();
    switch (telephonyMode) {
        case TELEPHONY_MODE_0_NONE:
        case TELEPHONY_MODE_100_TDNC_DUALTALK:
        case TELEPHONY_MODE_101_FDNC_DUALTALK:
        case TELEPHONY_MODE_102_WNC_DUALTALK:
        case TELEPHONY_MODE_103_TNC_DUALTALK:
            //do nothing since there is no first MD concept in these cases
            break;
        default: {
            //result = MTK_FIRST_MD;
            result = 1;
        }
    }
    return result;
}

int isDualTalkMode()
{
    int telephonyMode = getTelephonyMode();
    int isDtSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    /// M: when EVDODT support, check if external phone is CDMA.
    if (isEVDODTSupport()) {
        return (getExternalModemSlotTelephonyMode() == 0);
    }

    if (telephonyMode == 0) {
        property_get("ro.mtk_dt_support", property_value, "0");
        isDtSupport = atoi(property_value);
    } else if (telephonyMode >= 5) {
        isDtSupport = 1;
    }
    RLOGI("isDualTalkSupport:%d",isDtSupport);
    return isDtSupport;
}

static char *gettaskName(char *buf, int size, FILE *file)
{
    int cnt = 0;
    int eof = 0;
    int eol = 0;
    int c;

    if (size < 1) {
        return NULL;
    }

    while (cnt < (size - 1)) {
        c = getc(file);
        if (c == EOF) {
            eof = 1;
            break;
        }

        *(buf + cnt) = c;
        cnt++;

        if (c == '\n') {
            eol = 1;
            break;
        }
    }

    /* Null terminate what we've read */
    *(buf + cnt) = '\0';

    if (eof) {
        if (cnt) {
            return buf;
        } else {
            return NULL;
        }
    } else if (eol) {
        return buf;
    } else {
        /* The line is too long.  Read till a newline or EOF.
         * If EOF, return null, if newline, return an empty buffer.
         */
        while(1) {
            c = getc(file);
            if (c == EOF) {
                return NULL;
            } else if (c == '\n') {
                *buf = '\0';
                return buf;
            }
        }
    }
    return NULL;
}


int findPid(char* name)
{
    int pid = -1;

    if (name != NULL) {
        RLOGI("Find PID of %s <process name>", name);
    } else {
        return pid;
    }

    const char* directory = "/proc";
    size_t      taskNameSize = 256;
    char*       taskName = calloc(taskNameSize, sizeof(char));

    DIR* dir = opendir(directory);

    if (dir) {
        struct dirent* de = 0;

        while ((de = readdir(dir)) != 0) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;

            int res = sscanf(de->d_name, "%d", &pid);

            if (res == 1) {
                // we have a valid pid

                // open the cmdline file to determine what's the name of the process running
                char cmdline_file[256] = {0};
                sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

                FILE* cmdline = fopen(cmdline_file, "r");

                if (gettaskName(taskName, taskNameSize, cmdline) != NULL) {
                    // is it the process we care about?
                    if (strstr(taskName, name) != 0) {
                        RLOGI("%s process, with PID %d, has been detected.", name, pid);
                        // just let the OS free this process' memory!
                        fclose(cmdline);
                        free(taskName);
                        closedir(dir);
                        return pid;
                    }
                }

                fclose(cmdline);
            }
        }

        closedir(dir);
    }

    // just let the OS free this process' memory!
    free(taskName);

    return -1;
}

// ALPS00447663, EVDO power on
int getExternalModemSlot()
{
    if (isCdmaLteDcSupport()) {
        return getCdmaSocketSlotId() - 1;
    }
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.external.md", property_value, "0");
    return atoi(property_value) - 1;
}

int isInternationalRoamingEnabled()
{
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.evdo.irsupport", property_value, "0");
    return atoi(property_value);
}

int isEVDODTSupport()
{
    // TODO: Temp solution, just replace evdo.dt.support with ro.mtk_c2k_support.
    // Should review later
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_c2k_support", property_value, "0");
    return atoi(property_value);
}

int getExternalModemSlotTelephonyMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
	int externalModemSlot = getExternalModemSlot();
	RLOGI("getExternalModemSlot: %d", externalModemSlot);
    if (externalModemSlot == 0) {
        property_get("mtk_telephony_mode_slot1", property_value, "0");
    } else {
        property_get("mtk_telephony_mode_slot2", property_value, "0");
    }
	RLOGI("getExternalModemSlotTelephonyMode: %d", atoi(property_value));
    return atoi(property_value);
}

int isSupportSimHotSwapC2k()
{
    return isSvlteSupport();
}

int isSupportCommonSlot()
{
    /* Return 1 if support SIM Hot Swap with Common Slot Feature */
#ifdef MTK_SIM_HOT_SWAP_COMMON_SLOT
    return 1;
#else
    return 0;
#endif
}

/// M: workaround for world phone lib test @{
int isWorldPhoneSupport() {
#ifdef MTK_WORLD_PHONE
    return 1;
#else
    return 0;
#endif
}

int isGeminiSupport() {
    if (SIM_COUNT >= 2) {
        return 1;
    } else {
        return 0;
    }
}

int isLteSupport() {
#ifdef MTK_LTE_SUPPORT
    return 1;
#else
    return 0;
#endif
}

int isImsSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_ims_support", property_value, "0");
    return atoi(property_value);
}

int isC2KWorldPhoneP2Support() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk.c2kwp.p2.support", property_value, "0");
    return atoi(property_value);
}

int isC2KWorldPhoneSimSwitchSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk.c2kwp.simswitch.support", property_value, "0");
    return atoi(property_value);
}

/* To check if new SS service class feature is supported or not */
void determineSSServiceClassFeature(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;

    bNewSSServiceClassFeature = 0;

    /* Temporary solution */
    /*err = at_send_command("AT+CESQ=?",  &p_response, getDefaultChannelCtx(rid));
    LOGI("AT+CESQ=?, got err= %d, success=%d, finalResponse = %s", err, p_response->success, p_response->finalResponse);

    if ((err == 0 && p_response->success == 1) && (strncmp(p_response->finalResponse,"OK", 2) == 0)) {
       bNewSSServiceClassFeature = 1;
    }
    at_response_free(p_response);
    */


    err = at_send_command("AT+ECUSD?",  &p_response, getDefaultChannelCtx(rid));
    RLOGI("AT+ECUSD?, got err= %d, success=%d, finalResponse = %s", err, p_response->success, p_response->finalResponse);

    if ((err == 0 && p_response->success == 1) && (strncmp(p_response->finalResponse,"OK", 2) == 0)) {
       bNewSSServiceClassFeature = 1;
    }
    at_response_free(p_response);

    RLOGI("bNewSSServiceClassFeature = %d", bNewSSServiceClassFeature);
}
/// @}

int getMtkShareModemCurrent() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int current_share_modem = 0;
    property_get("ril.current.share_modem", property_value, NULL);
    current_share_modem = atoi(property_value);
    return current_share_modem;
}

int isBootupWith3GCapability() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    if (isEVDODTSupport()) {
        if (getExternalModemSlot() == 0) {
            property_get("mtk_telephony_mode_slot2", property_value, "2");
        } else {
            property_get("mtk_telephony_mode_slot1", property_value, "2");
        }
    }
    return atoi(property_value);
}

void setMSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue) {
    #define MAX_PHONE_NUM 10
    #define MIN(a,b) ((a)<(b) ? (a) : (b))

    char oldItemValue[PROPERTY_VALUE_MAX] = {0};
    char newPropertyValue[PROPERTY_VALUE_MAX] = {0};
    int i = 0;
    int strLen = 0;

    for (i = 0; i < MAX_PHONE_NUM; i++) {
        if (i == phoneId) {
            // use new value
            strcat(newPropertyValue, pUpdateValue);
        } else {
            getMSimProperty(i, pPropertyName, oldItemValue);
            strcat(newPropertyValue, oldItemValue);
        }
        if (i != MAX_PHONE_NUM-1) {
            strcat(newPropertyValue, ",");
        }
        memset(oldItemValue, 0, PROPERTY_VALUE_MAX);
    }
    RLOGI("setMSimProperty phoneId=%d, newPropertyValue=%s", phoneId, newPropertyValue);
    // remove no use ','
    strLen = strlen(newPropertyValue);
    for (i = (strLen-1); i >= 0; i--) {
        if (newPropertyValue[i] == ',') {
            // remove
            newPropertyValue[i] = '\0';
        } else {
            break;
        }
    }
    RLOGI("newPropertyValue %s\n", newPropertyValue);
    property_set(pPropertyName, newPropertyValue);
}

void getMSimProperty(int phoneId, char *pPropertyName,char *pPropertyValue) {
    char prop[PROPERTY_VALUE_MAX] = {0};
    char value[PROPERTY_VALUE_MAX] = {0};
    int count= 0;
    int propLen = 0;
    int i = 0;
    int j = 0;

	property_get(pPropertyName, prop, "");
    RLOGI("getMSimProperty pPropertyName=%s, prop=%s", pPropertyName, prop);
    propLen = strlen(prop);
    for (i = 0; i < propLen; i++) {
        if(prop[i] == ',') {
            count++;
            if((count-1) == phoneId) {
                // return current buffer
                RLOGI("getMSimProperty found! phoneId=%d, value =%s", phoneId, value);
                strcpy(pPropertyValue, value);
                return;
            } else {
                // clear current buffer
                j = 0;
                memset(value, 0, sizeof(char) * PROPERTY_VALUE_MAX);
            }
        } else {
            value[j] = prop[i];
            j++;
        }
    }
    if (count == phoneId) {
        strcpy(pPropertyValue, value);
        RLOGI("getMSimProperty found at end! phoneId=%d, value =%s", phoneId, value);
    }
}

int handleAee(const char *modem_warning, const char *modem_version) {
#ifdef HAVE_AEE_FEATURE
    return aee_modem_warning("Modem Warning", NULL, DB_OPT_DUMMY_DUMP, modem_warning, modem_version);
#else
    LOGD("[handleOemUnsolicited]HAVE_AEE_FEATURE is not defined");
    return 1;
#endif
}

int isNeedCombineAttach()
{
    if(isCdmaLteDcSupport()) {
        return 0;
    }
    return 1;
}

int isCdmaLteDcSupport()
{
    int isSvlteSupport = 0;
    int isSrlteSupport = 0;

    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_support", property_value, "0");
    isSvlteSupport = atoi(property_value);
    RLOGI("isSvlteSupport: %d", isSvlteSupport);

    property_get("ro.mtk_srlte_support", property_value, "0");
    isSrlteSupport = atoi(property_value);
    RLOGI("isSrlteSupport: %d", isSrlteSupport);

    return (isSvlteSupport || isSrlteSupport) ? 1 : 0;
}

int isSrlteSupport()
{
    int isSrlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_srlte_support", property_value, "0");
    isSrlteSupport = atoi(property_value);
    RLOGI("isSrlteSupport: %d", isSrlteSupport);
    return isSrlteSupport;
}

int isSvlteSupport()
{
    int isSvlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_support", property_value, "0");
    isSvlteSupport = atoi(property_value);
    RLOGI("isSvlteSupport: %d", isSvlteSupport);
    return isSvlteSupport;
}

int isCdmaSupport()
{
    int isCdmaSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_c2k_support", property_value, "0");
    isCdmaSupport = atoi(property_value);
    RLOGI("isCdmaSupport: %d", isCdmaSupport);
    return isCdmaSupport;
}

int isCdmaIratSupport()
{
    int isIratSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.c2k.irat.support", property_value, "0");
    isIratSupport = atoi(property_value);
    RLOGI("isCdmaIratSupport: %d", isIratSupport);
    return isIratSupport;
}

int isSvlteLcgSupport()
{
    int isSvlteLcgSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_lcg_support", property_value, "0");
    isSvlteLcgSupport = atoi(property_value);
    RLOGI("isSvlteLcgSupport: %d", isSvlteLcgSupport);
    return isSvlteLcgSupport;
}

int isDisableCapabilitySwitch()
{
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_disable_cap_switch", property_value, "0");
    return atoi(property_value);
}

/// M: SVLTE solution2 modification. @{
/**
 * Get the SVLTE slot id.
 * @return SVLTE slot id.
 *         1 : svlte in slot1
 *         2 : svlte in slot2
 */
int getActiveSvlteModeSlotId() {
    int i = 1;
    char tempstr[PROPERTY_VALUE_MAX];
    char *tok;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    property_get("persist.radio.svlte_slot", tempstr, "3,2");
    tok = strtok(tempstr, ",");
    while(tok != NULL)
    {
        if (3 == atoi(tok)) {
            RLOGI("getActiveSvlteModeSlotId : %d", i);
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    RLOGI("getActiveSvlteModeSlotId : -1");
    return -1;
}

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

/**
 * Get telephony mode for SVLTE.
 * @return current telephony mode
 */
int getSvlteTelephonyMode() {
    int svlteSlotId = getActiveSvlteModeSlotId();
    char tempstr[PROPERTY_VALUE_MAX];
    int currMajorSim = 1;
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.radio.simswitch", tempstr, "1");
    currMajorSim = atoi(tempstr);
    if (SIM_COUNT == 1) {
        RLOGI("[getSvlteTelephonyMode] >>> SINGLE SIM case.");
        if(svlteSlotId == 1){
            return TELEPHONY_MODE_LC_SINGLE;
        } else {
            return TELEPHONY_MODE_LWTG_SINGLE;
        }
    } else if (SIM_COUNT == 2) {
        RLOGI("[getSvlteTelephonyMode] svlteSlotId : %d, currMajorSim = %d", svlteSlotId, currMajorSim);
        if (currMajorSim == 1) {
            if (svlteSlotId == 1) {
                return TELEPHONY_MODE_LC_G;
            } else if (svlteSlotId == 2) {
                return TELEPHONY_MODE_LWTG_C;
            } else {
                return TELEPHONY_MODE_LWTG_G;
            }
        } else if (currMajorSim == 2) {
            if (svlteSlotId == 1) {
                return TELEPHONY_MODE_C_LWTG;
            } else if (svlteSlotId == 2) {
                return TELEPHONY_MODE_G_LC;
            } else {
                return TELEPHONY_MODE_G_LWTG;
            }
        }
    }
    return TELEPHONY_MODE_UNKNOWN;
}

/**
 * Get the actual SVLTE slot id custom.
 * @return SVLTE slot id.
 *         1 : svlte in slot1
 *         2 : svlte in slot2
 */
int getActualSvlteModeSlotId() {
    int i = 1;
    char tempstr[PROPERTY_VALUE_MAX];
    char *tok;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    property_get("persist.radio.actual.svlte_slot", tempstr, "3,2");
    tok = strtok(tempstr, ",");
    while(tok != NULL)
    {
        if (3 == atoi(tok)) {
            RLOGI("getActualSvlteModeSlotId : %d", i);
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    RLOGI("getActualSvlteModeSlotId : -1");
    return -1;
}
/// @}

int getSimCount() {
    return SIM_COUNT;
}

/// M: Wfc @{
/**
 * Checks if Wfc is supported.
 * @return true if wfc is supported
 */
bool isWfcSupport()
{
    int isWfcSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_wfc_support", property_value, "0");
    isWfcSupport = atoi(property_value);
    RLOGI("isWfcSupport: %d", isWfcSupport);
    return (isWfcSupport == 1) ? true : false;
}
/// @}

/// M: Ims init flow @{
/**
 * ims initilization.
 * @param rid ril id.
 * 
 */
void imsInit(RIL_SOCKET_ID rid) {
    if (isImsAndVolteSupport() || isWfcSupport()) {
        char volteEnable[PROPERTY_VALUE_MAX] = { 0 };
        property_get("ro.mtk.volte.enable", volteEnable, "0");
        if ((atoi(volteEnable) == 0)) {
            at_send_command("AT+EIMS=0", NULL, getDefaultChannelCtx(rid));
        } else {
            at_send_command("AT+EIMS=1", NULL, getDefaultChannelCtx(rid));
        }
    }
}
/// @}

/// M: SAP start
void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("sendSapResponseComplete, start (%d)", msgId);

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            fields = RIL_SIM_SAP_ERROR_RSP_fields;
            break;
        default:
            RLOGE("sendSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("sendSapResponseComplete, Size: %d (0x%x) Size as written: 0x%x",
                encoded_size, encoded_size, written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("sendSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %u. encoded size result: %d",
        msgId, encoded_size, success);
    }
}
/// SAP end

// M: India 3M/5M @{
int getSvlteProjectType() {
    int type = -1;
    char property_value_mode[PROPERTY_VALUE_MAX] = { 0 };
    char property_value_nw_sel[PROPERTY_VALUE_MAX] = { 0 };

    property_get("ro.mtk.c2k.om.mode", property_value_mode, "cllwtg");
    property_get("ro.mtk_c2k_om_nw_sel_type", property_value_nw_sel, "0");
    if (strcmp(property_value_nw_sel, "1") == 0) {
        if (strcmp(property_value_mode, "cllwtg") == 0) {
            type = SVLTE_PROJ_SC_6M;
        } else if (strcmp(property_value_mode, "cllwg") == 0) {
            type = SVLTE_PROJ_SC_5M;
        } else if (strcmp(property_value_mode, "cllg") == 0) {
            type = SVLTE_PROJ_SC_4M;
        } else if (strcmp(property_value_mode, "cwg") == 0) {
            type = SVLTE_PROJ_SC_3M;
        }
    } else {
        if (strcmp(property_value_mode, "cllwtg") == 0) {
            type = SVLTE_PROJ_DC_6M;
        } else if (strcmp(property_value_mode, "cllwg") == 0) {
            type = SVLTE_PROJ_DC_5M;
        } else if (strcmp(property_value_mode, "cllg") == 0) {
            type = SVLTE_PROJ_DC_4M;
        } else if (strcmp(property_value_mode, "cwg") == 0) {
            type = SVLTE_PROJ_DC_3M;
        }
    }
    RLOGI("getSvlteWwopType(): %s %s %d", property_value_mode, property_value_nw_sel, type);
    return type;
}
// M: India 3M/5M }@


