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
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <ril_callbacks.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

/* MTK proprietary start */
#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define CRSS_CALL_WAITING             0
#define CRSS_CALLED_LINE_ID_PREST     1
#define CRSS_CALLING_LINE_ID_PREST    2
#define CRSS_CONNECTED_LINE_ID_PREST  3

char *setupCpiData[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int callWaiting = 0;
int isAlertSet = 0;
int hasReceivedRing = 0;
int inCallNumber = 0;
int inCallNumberPerSim[MAX_SIM_COUNT] = {0};
// BEGIN mtk03923 [20120210][ALPS00114093]
int inDTMF = 0;
// END mtk03923 [20120210][ALPS00114093]
bool isRecvECPI0 = false;
int ringCallID = 0;
RIL_SOCKET_ID ridRecvECPI0 = SIM_COUNT;
char cachedCnap[MAX_CNAP_LENGTH] = { 0 };
char cachedClccName[MAX_GSMCALL_CONNECTIONS][MAX_CNAP_LENGTH] = {{0}};

// [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
// mtk04070, 2012.02.24
int bUseLocalCallFailCause = 0;
int dialLastError = 0;
/* MTK proprietary end */

// IMS VoLTE refactoring
dispatch_flg dispatch_flag = GSM;
volte_call_state call_state = none;
extern DIAL_SOURCE dial_source;

/// M: For 3G VT only @{
int isReplaceRequest = 0;
/// @}

extern void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);
extern void IMS_RIL_onUnsolicitedResponse(int unsolResponse, void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);
extern void IMS_RIL_onRequestComplete(RIL_Token t, RIL_Errno e,
                           void *response, size_t responselen);
extern bool IMS_isRilRequestFromIms(RIL_Token t);

static int clccStateToRILState(int state, RIL_CallState *p_state) {
    switch (state) {
        case 0: *p_state = RIL_CALL_ACTIVE;   return 0;
        case 1: *p_state = RIL_CALL_HOLDING;  return 0;
        case 2: *p_state = RIL_CALL_DIALING;  return 0;
        case 3: *p_state = RIL_CALL_ALERTING; return 0;
        case 4: *p_state = RIL_CALL_INCOMING; return 0;
        case 5: *p_state = RIL_CALL_WAITING;  return 0;
        default: return -1;
    }
}

/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
static int callFromCLCCLine(char *line, RIL_Call *p_call) {
    // +CLCC: 1,0,2,0,0,\"+18005551212\",145
    // index,isMT,state,mode,isMpty(,number,TOA)?

    int err;
    int state;
    int mode;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_call->index));
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &(p_call->isMT));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &state);
    if (err < 0) goto error;

    err = clccStateToRILState(state, &(p_call->state));
    if (err < 0) goto error;

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
            memset(cachedClccName[p_call->index-1], 0, MAX_CNAP_LENGTH);
            if (strlen(cachedCnap) > 0) {
                strcpy(cachedClccName[p_call->index-1], cachedCnap);
                memset(cachedCnap, 0, MAX_CNAP_LENGTH);
            }
        }
    }

    // Fill CNAP in individual CLCC parsing and clear in ECPI:133
    if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
        if (strlen(cachedClccName[p_call->index-1]) > 0) {
            strcpy(p_call->name, cachedClccName[p_call->index-1]);
        }
    }

    err = at_tok_nextint(&line, &mode);
    if (err < 0) goto error;

    p_call->isVoice = (mode == 0);

    if (mode >= 20) {
        LOGD("Skip call with mode>=20, i.e. IMS/WFC call\n");
        goto error;
    }

    err = at_tok_nextbool(&line, &(p_call->isMpty));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_call->number));

        /* tolerate null here */
        if (err < 0) return 0;

        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if ((p_call->number != NULL) &&
            (strspn(p_call->number, "+*#0123456789") != strlen(p_call->number))) {
            p_call->number = NULL;
        }

        err = at_tok_nextint(&line, &p_call->toa);
        if (err < 0) goto error;
    }

    p_call->uusInfo = NULL;

    return 0;

error:
    LOGE("invalid CLCC line\n");
    return -1;
}

void clearCnap() {
    int i;
    memset(cachedCnap, 0, MAX_CNAP_LENGTH);
    for (i = 0; i < MAX_GSMCALL_CONNECTIONS; i++) {
        memset(cachedClccName[i], 0, MAX_CNAP_LENGTH);
    }
}

extern void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t) {
    int err;
    ATResponse *p_response;
    ATLine *p_cur;
    int countCalls;
    int countValidCalls;
    RIL_Call *p_calls;
    RIL_Call **pp_calls;
    int i;

    err = at_send_command_multiline("AT+CLCC", "+CLCC:", &p_response, CC_CHANNEL_CTX);

    if (p_response == NULL) {
        clearCnap();
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        LOGD("p_response = NULL");
        return;
    }

    if (err != 0 || p_response->success == 0) {
        clearCnap();
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        at_response_free(p_response);
        return;
    }

    /* count the calls */
    for (countCalls = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
        countCalls++;

    /* yes, there's an array of pointers and then an array of structures */

    pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
    p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
    memset(p_calls, 0, countCalls * sizeof(RIL_Call));

    /* init the pointer array */
    for (i = 0; i < countCalls; i++)
        pp_calls[i] = &(p_calls[i]);

    for (countValidCalls = 0, p_cur = p_response->p_intermediates
         ; p_cur != NULL
         ; p_cur = p_cur->p_next
         ) {
        p_calls[countValidCalls].name = (char *)alloca(MAX_CNAP_LENGTH);
        memset(p_calls[countValidCalls].name, 0, MAX_CNAP_LENGTH);
        err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls);

        if (err != 0)
            continue;

        countValidCalls++;
    }

    if (countValidCalls == 0) {
        clearCnap();
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls, countValidCalls * sizeof(RIL_Call *));

    at_response_free(p_response);
    return;

error:
    clearCnap();
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestDial(void *data, size_t datalen, RIL_Token t, int isEmergency) {
    RIL_Dial *p_dial;
    char *cmd;
    const char *clir;
    int ret;
    ATResponse *p_response = NULL;

    /*mtk00924: ATDxxxI can not used for FDN check, therefore, change to #31# or *31#*/

    p_dial = (RIL_Dial *)data;

    switch (p_dial->clir) {
    case 1: /*invocation*/
        clir = "#31#";
        break;
    case 2: /*suppression*/
        clir = "*31#";
        break;
    case 0:
    default: /*subscription default*/
        clir = "";
        break;
    }

    // IMS VoLTE refactoring
    if (dial_source == IMSPHONE) {
        LOGE("IMS: requestDial from IMS !!");
        ret = at_send_command("AT+EVADSMOD=0", &p_response, CC_CHANNEL_CTX);
        if (ret < 0 || p_response->success == 0) {
            LOGE("IMS: AT+EVADSMOD=0 Fail !!");
        }
        dispatch_flag = IMS;
    } else {
        LOGE("IMS: AT+EVADSMOD=1");
        ret = at_send_command("AT+EVADSMOD=1", &p_response, CC_CHANNEL_CTX);
        if (ret < 0 || p_response->success == 0) {
            LOGE("IMS: AT+EVADSMOD=1 Fail !!");
        }
        dispatch_flag = GSM;
    }
    LOGE("IMS: requestDial, dial_source = %d", dial_source);

    if (isEmergency) {
        /* If an incoming call exists(+CRING is not received yet), hang it up before dial ECC */
       if ((setupCpiData[0] != NULL) && isRecvECPI0) {
            LOGD("To hang up incoming call(+CRING is not received yet) before dialing ECC!");

            asprintf(&cmd, "AT+CHLD=1%s", setupCpiData[0]);

            isRecvECPI0 = false;
            int i;
            for (i = 0; i < 9; i++) {
                free(setupCpiData[i]);
                setupCpiData[i] = NULL;
            }

            at_send_command(cmd, NULL, CC_CHANNEL_CTX);
            free(cmd);

            sleep(1);
        }

        asprintf(&cmd, "ATDE%s%s;", clir, p_dial->address);

    } else {
        // BEGIN mtk03923 [20111004][ALPS00077405]
        // CC operation will fail when dialing number exceed 40 character due to modem capability limitation.
        if (strlen(p_dial->address) > 40) {
            LOGE("strlen(%s)=%d exceeds 40 character\n", p_dial->address, strlen(p_dial->address));

            if (IMS_isRilRequestFromIms(t)) {
                IMS_RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
            }
            at_response_free(p_response);

            // [ALPS00251057][Call]It didn't pop FDN dialog when dial an invalid number
            // But this is not related to FDN issue, it returned to AP since number is too long.
            // mtk04070, 2012.03.12
            bUseLocalCallFailCause = 1;
            dialLastError = 28; /* Refer to CallFailCause.java - INVALID_NUMBER_FORMAT */

            return;
        }
        // END mtk03923 [20111004][ALPS00077405]

        asprintf(&cmd, "ATD%s%s;", clir, p_dial->address);
    }
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

    // [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
    // mtk04070, 2012.02.24
    bUseLocalCallFailCause = 0;
    dialLastError = 0;

    free(cmd);

    if (ret < 0 || p_response->success == 0)
        goto error;
    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
    return;

error:
    if (call_state == pending_mo) {
        LOGE("IMS: requestDial fail !!");
        call_state = none;
        if (inCallNumber == 0) {
            dispatch_flag = GSM;
            dial_source = GSMPHONE;
        }
    }
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
}

void requestHangup(void *data, size_t datalen, RIL_Token t) {
    int *p_line;
    char *cmd;

    p_line = (int *)data;

    // 3GPP 22.030 6.5.5
    // "Releases a specific active call X"
    asprintf(&cmd, "AT+CHLD=1%d", p_line[0]);
    at_send_command(cmd, NULL, CC_CHANNEL_CTX);

    free(cmd);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

void requestHangupWaitingOrBackground(void *data, size_t datalen, RIL_Token t) {
    // 3GPP 22.030 6.5.5
    // "Releases all held calls or sets User Determined User Busy
    //  (UDUB) for a waiting call."
    at_send_command("AT+CHLD=0", NULL, CC_CHANNEL_CTX);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestHangupForegroundResumeBackground(void *data, size_t datalen, RIL_Token t) {
    // 3GPP 22.030 6.5.5
    // "Releases all active calls (if any exist) and accepts
    // the other (held or waiting) call."
    // at_send_command("AT+CHLD=1", NULL, CC_CHANNEL_CTX);
    at_send_command_multiline("AT+CHLD=1", "NO CARRIER", NULL, CC_CHANNEL_CTX);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestSwitchWaitingOrHoldingAndActive(void *data, size_t datalen, RIL_Token t) {
    int ret;
    ATResponse *p_response = NULL;

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
            if (IMS_isRilRequestFromIms(t)) {
                IMS_RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);   // RIL_E_GENERIC_FAILURE
            }
            return;
        }
        // END mtk03923 [20120210][ALPS00114093]


    ret = at_send_command("AT+CHLD=2", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response->success == 0)
        goto error;

    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestAnswer(void *data, size_t datalen, RIL_Token t) {
    at_send_command("ATA", NULL, CC_CHANNEL_CTX);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

void requestConference(void *data, size_t datalen, RIL_Token t) {
    int ret;
    ATResponse *p_response = NULL;

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        // END   mtk03923 [20120210][ALPS00114093]

    ret = at_send_command("AT+CHLD=3", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response->success == 0)
        goto error;

    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
    return;

error:
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
}

void requestUdub(void *data, size_t datalen, RIL_Token t) {
    /* user determined user busy */
    /* sometimes used: ATH */
    at_send_command("ATH", NULL, CC_CHANNEL_CTX);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestSeparateConnection(void *data, size_t datalen, RIL_Token t) {
    char cmd[12];
    int party = ((int *)data)[0];
    int ret;
    ATResponse *p_response = NULL;

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        // END   mtk03923 [20120210][ALPS00114093]


    // Make sure that party is in a valid range.
    // (Note: The Telephony middle layer imposes a range of 1 to 7.
    // It's sufficient for us to just make sure it's single digit.)
    if (party > 0 && party < 10) {
        sprintf(cmd, "AT+CHLD=2%d", party);
        ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

        if (ret < 0 || p_response->success == 0) {
            at_response_free(p_response);
            goto error;
        }

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        at_response_free(p_response);
        return;
    }

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestExplicitCallTransfer(void *data, size_t datalen, RIL_Token t) {
    /* MTK proprietary start */
    int ret;
    ATResponse *p_response = NULL;

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        // END mtk03923 [20120210][ALPS00114093]


    ret = at_send_command("AT+CHLD=4", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response->success == 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    /* MTK proprietary end */
}

void requestLastCallFailCause(void *data, size_t datalen, RIL_Token t) {
    /* MTK proprietary start */
    RIL_LastCallFailCauseInfo callFailCause;
    char *line;
    int ret;
    ATResponse *p_response = NULL;

    memset(&callFailCause, 0, sizeof(RIL_LastCallFailCauseInfo));

    // [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
    // mtk04070, 2012.02.24
    if (bUseLocalCallFailCause == 1) {
       callFailCause.cause_code = dialLastError;
       LOGD("Use local call fail cause = %d", callFailCause.cause_code);
    }
    else {
        ret = at_send_command_singleline("AT+CEER", "+CEER:", &p_response, CC_CHANNEL_CTX);

       if (ret < 0 || p_response->success == 0)
           goto error;

       line = p_response->p_intermediates->line;

       ret = at_tok_start(&line);

       if (ret < 0)
           goto error;

       ret = at_tok_nextint(&line, &(callFailCause.cause_code));
       if (ret < 0)
           goto error;

       ret = at_tok_nextstr(&line, &(callFailCause.vendor_cause));
       if (ret < 0)
           goto error;

       LOGD("MD fail cause_code = %d, vendor_cause = %s",
               callFailCause.cause_code, callFailCause.vendor_cause);
    }

    /*if there are more causes need to be translated in the future,
     * discussing with APP owner to implement this in upper layer.
     * For the hard coded value, please refer to modem code.*/

    if (callFailCause.cause_code == 10)
        callFailCause.cause_code = CALL_FAIL_CALL_BARRED;
    else if (callFailCause.cause_code == 2600)
        callFailCause.cause_code = CALL_FAIL_FDN_BLOCKED;
    else if (callFailCause.cause_code == 2052)
        callFailCause.cause_code = CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
    else if (callFailCause.cause_code == 2053)
        callFailCause.cause_code = CALL_FAIL_IMEI_NOT_ACCEPTED;
    else if ((callFailCause.cause_code > 127 && callFailCause.cause_code != 2165
            && callFailCause.cause_code != 380) || callFailCause.cause_code <= 0)
        callFailCause.cause_code = CALL_FAIL_ERROR_UNSPECIFIED;

    LOGD("RIL fail cause_code = %d, vendor_cause = %s",
            callFailCause.cause_code, callFailCause.vendor_cause);

    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, &callFailCause, sizeof(RIL_LastCallFailCauseInfo));
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &callFailCause, sizeof(RIL_LastCallFailCauseInfo));
    }
    if (NULL != p_response) {
        at_response_free(p_response);
    }
    return;

error:
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    if (NULL != p_response) {
        at_response_free(p_response);
    }
    /* MTK proprietary end */
}

void requestDtmf(void *data, size_t datalen, RIL_Token t) {
    char c = ((char *)data)[0];
    char *cmd;

    asprintf(&cmd, "AT+VTS=%c", (int)c);
    at_send_command(cmd, NULL, CC_CHANNEL_CTX);

    free(cmd);

    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

static void onCnapNotification(char *s, RIL_SOCKET_ID rid) {
    char* p_data[2];
    char* line = s;

    /**
     * CNAP presentaion from the network
     * +CNAP: <name>[,<CNI validity>] to the TE.
     *
     *   <name> : GSM 7bit encode
     *
     *   <CNI validity>: integer type
     *   0 CNI valid
     *   1 CNI has been withheld by the originator.
     *   2 CNI is not available due to interworking problems or limitations of originating network.
     */

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <name> */
    if (at_tok_nextstr(&line, &p_data[0]) < 0) {
       goto error;
    }

    /* Get <CNI validity> */
    if (at_tok_nextstr(&line, &p_data[1]) < 0) {
        goto error;
    }

    snprintf(cachedCnap, MAX_CNAP_LENGTH - 1, "%s", p_data[0]);

    return;

error:
    LOGE("There is something wrong with the +CNAP");
}

/**
 * RIL_UNSOL_CRSS_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_CrssNotification *
 *
 */
static void onCrssNotification(char *s, int code, RIL_SOCKET_ID rid)
{
    RIL_CrssNotification crssNotify;
    char* line = s;
    char* pStrTmp = NULL;
    int  toa = 0;
    /// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
    int type = 0;
#endif
    /// @}

    memset(&crssNotify, 0, sizeof(RIL_CrssNotification));
    crssNotify.code = code;

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <number> */
    if (at_tok_nextstr(&line, &(crssNotify.number)) < 0) {
        LOGE("CRSS: number fail!");
        goto error;
    }

    /// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
    if (code == 0) {
        /* Skip <type> */
        if (at_tok_nextint(&line, &(type)) < 0) {
            LOGE("CRSS: type fail!");
            goto error;
        }
        /* Get <class> */
        if(at_tok_nextint(&line, &(crssNotify.type)) < 0)
        {
            LOGE("CRSS: class fail!");
        }
    } else {
#endif
    /// @}

    /* Get <type> */
    if (at_tok_nextint(&line, &(crssNotify.type)) < 0) {
        LOGE("CRSS: type fail!");
        goto error;
    }

    if (at_tok_hasmore(&line)) {
        /*skip subaddr*/
        if(at_tok_nextstr(&line, &(pStrTmp)) < 0) {
            LOGE("CRSS: sub fail!");
        }

        /*skip satype*/
        if(at_tok_nextint(&line,&(toa)) < 0) {
            LOGE("CRSS: sa type fail!");
        }

        if (at_tok_hasmore(&line)) {
            /* Get alphaid */
            if(at_tok_nextstr(&line, &(crssNotify.alphaid)) < 0) {
                LOGE("CRSS: alphaid fail!");
            }

            /* Get cli_validity */
        if(at_tok_nextint(&line, &(crssNotify.cli_validity)) < 0)
        {
          LOGE("CRSS: cli_validity fail!");
        }
        LOGD("crssNotify.cli_validity = %d", crssNotify.cli_validity);
        }
    }
    /// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
    }
#endif

    /// M: Simulate IMS @{
    if (isSimulateIms(rid)) {
        LOGD("onCrssNotification: Simulate IMS");
        IMS_RIL_onUnsolicitedResponse (
                RIL_UNSOL_CRSS_NOTIFICATION,
                &crssNotify, sizeof(RIL_CrssNotification), rid);
        return;
    }
    /// @}
    RIL_onUnsolicitedResponseSocket (
            RIL_UNSOL_CRSS_NOTIFICATION,
            &crssNotify, sizeof(RIL_CrssNotification),
            rid);

    return;

error:
    LOGE("error on onCrssNotification");
}

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
    case RIL_REQUEST_GET_CURRENT_CALLS:
        requestGetCurrentCalls(data, datalen, t);
        break;
    case RIL_REQUEST_DIAL:
        requestDial(data, datalen, t, 0);
        break;
    case RIL_REQUEST_HANGUP:
        requestHangup(data, datalen, t);
        break;
    case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        requestHangupWaitingOrBackground(data, datalen, t);
        break;
    case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        requestHangupForegroundResumeBackground(data, datalen, t);
        break;
    case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
        requestSwitchWaitingOrHoldingAndActive(data, datalen, t);
        break;
    case RIL_REQUEST_ANSWER:
        requestAnswer(data, datalen, t);
        break;
    case RIL_REQUEST_CONFERENCE:
        requestConference(data, datalen, t);
        break;
    case RIL_REQUEST_UDUB:
        requestUdub(data, datalen, t);
        break;
    case RIL_REQUEST_SEPARATE_CONNECTION:
        requestSeparateConnection(data, datalen, t);
        break;
    case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
        requestExplicitCallTransfer(data, datalen, t);
        break;
    case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
        requestLastCallFailCause(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF:
        requestDtmf(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF_START:
        requestDtmfStart(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF_STOP:
        requestDtmfStop(data, datalen, t);
        break;
    case RIL_REQUEST_SET_TTY_MODE:
        requestSetTTYMode(data, datalen, t);
        break;
    /* MTK proprietary start */
    /* M: call control part start */
    case RIL_REQUEST_HANGUP_ALL:
        requestHangupAll(data, datalen, t);
        break;
    case RIL_REQUEST_FORCE_RELEASE_CALL:
        requestForceReleaseCall(data, datalen, t);
        break;
    case RIL_REQUEST_SET_CALL_INDICATION:
        requestSetCallIndication(data, datalen, t);
        break;
    case RIL_REQUEST_SET_ECC_SERVICE_CATEGORY:
        requestSetEccServiceCategory(data, datalen, t);
        break;
    case RIL_REQUEST_SET_ECC_LIST:
        requestSetEccList(data, datalen, t);
        break;
    case RIL_REQUEST_EMERGENCY_DIAL:
        requestDial(data, datalen, t, 1);
        break;
    case RIL_REQUEST_SET_SPEECH_CODEC_INFO:
        requestSetSpeechCodecInfo(data, datalen, t);
        break;
    /* M: call control part end */
    case RIL_REQUEST_GET_CCM:
        requestGetCcm(data, datalen, t);
        break;
    case RIL_REQUEST_GET_ACM:
        requestGetAcm(data, datalen, t);
        break;
    case RIL_REQUEST_GET_ACMMAX:
        requestGetAcmMax(data, datalen, t);
        break;
    case RIL_REQUEST_GET_PPU_AND_CURRENCY:
        requestGetPpuAndCurrency(data, datalen, t);
        break;
    case RIL_REQUEST_SET_ACMMAX:
        requestSetAcmMax(data, datalen, t);
        break;
    case RIL_REQUEST_RESET_ACM:
        requestResetAcm(data, datalen, t);
        break;
    case RIL_REQUEST_SET_PPU_AND_CURRENCY:
        requestSetPpuAndCurrency(data, datalen, t);
        break;
    case RIL_REQUEST_SET_DATA_CENTRIC:
        requestSetDataCentric(data, datalen, t);
        break;

    case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER:
        requestAddImsConferenceCallMember(data, datalen, t);
        break;

    case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER:
        requestRemoveImsConferenceCallMember(data, datalen, t);
        break;

    case RIL_REQUEST_DIAL_WITH_SIP_URI:
        requestDialWithSipUri(data, datalen, t);
        break;

    case RIL_REQUEST_HOLD_CALL:
        requestHoldCall(data, datalen, t);
        break;

    case RIL_REQUEST_RESUME_CALL:
        requestResumeCall(data, datalen, t);
        break;

    case RIL_REQUEST_SET_IMS_CALL_STATUS:
          requestSetImsCallStatus(data, datalen, t);
          break;

    /// M: For 3G VT only @{
    case RIL_REQUEST_VT_DIAL:
        requestVtDial(data, datalen, t);
        break;

    case RIL_REQUEST_VOICE_ACCEPT:
        requestVoiceAccept(data, datalen, t);
        break;

    case RIL_REQUEST_REPLACE_VT_CALL:
        requestReplaceVtCall(data, datalen, t);
        break;
    /// @}

    case RIL_REQUEST_CONFERENCE_DIAL:
        requestConferenceDial(data, datalen, t);
        break;

    /// M: CC072: Add Customer proprietary-IMS RIL interface. @{
    case RIL_REQUEST_SET_SRVCC_CALL_CONTEXT_TRANSFER:
        requestSetCallContextTransfer(data, datalen, t);
        break;

    case RIL_REQUEST_UPDATE_IMS_REGISTRATION_STATUS:
        requestUpdateImsRegistrationStatus(data, datalen, t);
        break;
    /// @}

    case RIL_REQUEST_SWITCH_ANTENNA:
        requestSwitchAntenna(data, datalen, t);
        break;

    case RIL_REQUEST_VIDEO_CALL_ACCEPT:
        requestVideoAccept(data, datalen, t);
        break;
    /* MTK proprietary end */

    default:
        return 0; /* no matched request */
        break;
    }

    return 1; /* request found and handled */
}

extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx *p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    /* MTK proprietary start */
    if (strStartsWith(s, "RING") || strStartsWith(s, "+CRING")) {
        LOGD("receiving RING!!!!!!");

        if (!isRecvECPI0) {
            LOGD("we havn't receive ECPI0, skip this RING!");
            return 1;
        }
        if (!hasReceivedRing) {
            LOGD("receiving first RING!!!!!!");
            hasReceivedRing = 1;
        }

        if (setupCpiData[0] != NULL) {
            LOGD("sending STATE CHANGE dispatch_flag = %d ", dispatch_flag);

            // IMS VoLTE refactoring
            if (dispatch_flag != IMS) {
                RIL_onUnsolicitedResponseSocket(
                        RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                        NULL, 0, rid);
            } else {
                LOGD("IMS: sending ECPI : 0 When RING");
                IMS_RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_INFO_INDICATION,
                        setupCpiData, 9 * sizeof(char *), rid);
            }

            int i;
            for (i = 0; i < 9; i++) {
                free(setupCpiData[i]);
                setupCpiData[i] = NULL;
            }
            sleep(1);
        }

        if (dispatch_flag == IMS) {
            IMS_RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_RING, NULL, 0, rid);
        } else {
/// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
        if (strStartsWith(s, "+CRING: VIDEO")) {
            if (!isReplaceRequest) {
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_VT_RING_INFO, NULL, 0, rid);
            }
        } else {
#endif
/// @}
            if (!isReplaceRequest) {
                RIL_onUnsolicitedResponseSocket(RIL_UNSOL_CALL_RING, NULL, 0, rid);
            }
/// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
        }
#endif
/// @}
        }
        return 1;
    } else if (strStartsWith(s,"+CCWA:")) {
        callWaiting = 1;
        onCrssNotification((char *) s, CRSS_CALL_WAITING, rid);
        return 1;
    } else if (strStartsWith(s,"+CDIP:")) {
        onCrssNotification((char *) s, CRSS_CALLED_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s,"+CLIP:")) {
        onCrssNotification((char *) s, CRSS_CALLING_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s,"+COLP:")) {
        onCrssNotification((char *) s, CRSS_CONNECTED_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s, "+CNAP:")) {
        onCnapNotification((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECPI")) {
        onCallProgressInfoCallStateChange((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ESPEECH")) {
        onSpeechInfo((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EAIC")) {
        onIncomingCallIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECIPH")) {
        onCipherIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CIREPI")) {
        onImsVopsSupport((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CNEMIU")) {
        onEmbSupport((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CEN1") || strStartsWith(s, "+CEN2")) {
        onEmergencyCallServiceCategoryIndication((char *)s, rid);
    } else if (strStartsWith(s, "+ECONFSRVCC")) {
        onVoLteEconfSrvccIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECONFCHECK")) {
        /* +ECONFCHECK: */
        return 1;
    } else if (strStartsWith(s, "+ECONF")) {
        /* +ECONF:<conf_call_id>,<op>,<num>,<result>,<cause>[,<joined_call_id>] */
        onVoLteEconfResultIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EEMCINFO")) {
        /* IMS (emergency) call related information from the network */
        onEpcNetworkFeatureInfoSupport((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CIREPH")) {
        /* Provides SRVCC and vSRVCC handover information */
        onSrvccStateNotify((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EVOCD")) {
        onSpeechCodecInfo((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ESSAC")) {
        /* Service specific access control parameters from the network */
        onSsacBarringInfo((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ESIPCPI")) {
        onSipCallProgressIndicator((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EVADSREP")) {
        onAdsReport((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CNEMS1")) {
        /// M: CC071: Add Customer proprietary-IMS RIL interface. @{
        /* Emergency bearer services support indicator for S1 mode */
        onEmergencyBearerSupportInfo((char*)s, rid);
        return 1;
        /// @}
    } else if (strStartsWith(s, "+EIMSCMODE")) {
        onCallModeChangeIndicator((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EIMSVCAP")) {
        onVideoCapabilityIndicator((char*)s, rid);
        return 1;
    }
/// M: For 3G VT only @{
#ifdef MTK_VT3G324M_SUPPORT
    else if (strStartsWith(s, "+EVTSTATUS")) {
        onVtStatusInfo((char *)s, rid);
        return 1;
    }
#endif
/// @}
    return 0;
    /* MTK proprietary end */
}
