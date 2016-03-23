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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
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

#include <telephony/ril.h>
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
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "oem-support.h"
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

#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern int inemergency;
extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int invoicecall;
extern int voicetype;

static void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t);
static void sendCallStateChanged(void *param);
static void requestDial(void *data, size_t datalen, RIL_Token t);
static void requestHangup(void *data, size_t datalen, RIL_Token t);
static void requestGetLastCallFailCause(void *data, size_t datalen, RIL_Token t);
static void requestSetMute(void *data, size_t datalen, RIL_Token t);
static void requestQueryPreferredVoicePrivacy(void *data, size_t datalen, RIL_Token t);
static void requestSendFlash(void *data, size_t datalen, RIL_Token t);
static void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t);
static int callFromCLCCLine(char *line, RIL_Call *p_call);
static void requestVtDail(void *data, size_t datalen, RIL_Token t);
static void requestVtEndCall(void *data, size_t datalen, RIL_Token t);
static void requestVtSendData(void *data, size_t datalen, RIL_Token t);
static void requestVtSendString(void *data, size_t datalen, RIL_Token t);
static void requestVtHungup(void *data, size_t datalen, RIL_Token t);
static void requestVtVideoCodec(void *data, size_t datalen, RIL_Token t);

#ifdef ADD_MTK_REQUEST_URC
static void requestEmergencyDial(void *data, size_t datalen, RIL_Token t);
static void requestSwitchHPF(void *data, size_t datalen, RIL_Token t);
#endif

int clccStateToRILState(int state, RIL_CallState *p_state);

/* current playing continuous dtmf character */
static char s_vtc[4] = { 0 };
/*times of loopback call is queried*/
static int loopbackcall_query_times = 0;
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0, 500000};

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_GET_CURRENT_CALLS:
            requestGetCurrentCalls(data, datalen, t);
            break;
        case RIL_REQUEST_DIAL:
            if (oemVcallOutSupportEnable(g_oem_support_flag) || !oemSupportEnable(g_oem_support_flag)) {
                requestDial(data, datalen, t);
            } else {
                RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
            }
            break;
        case RIL_REQUEST_HANGUP:
        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_HANGUP_ALL: /*Add for MTK*/
#endif
            requestHangup(data, datalen, t);
            break;
        case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
            // 3GPP 22.030 6.5.5
            // "Places all active calls (if any exist) on hold and accepts
            //  the other (held or waiting) call."
            at_send_command("AT+CHLD=2", NULL, CC_CHANNEL_CTX);
            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_UDUB:
            /* user determined user busy */
            /* sometimes used: ATH */
            at_send_command("ATH", NULL, CC_CHANNEL_CTX);

            /* set cp rm interface protocol - Relay Layer Rm interface, PPP */
            at_send_command("AT+CRM=1", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
            requestGetLastCallFailCause(data, datalen, t);
            break;
        case RIL_REQUEST_DTMF: {
            char c = ((char *) data)[0];
            char *cmd;
            LOGD("RIL_REQUEST_DTMF, c:%c", c);
            asprintf(&cmd, "AT+VTS=%c", (int) c);
            at_send_command(cmd, NULL, CC_CHANNEL_CTX);
            free(cmd);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        }
        case RIL_REQUEST_ANSWER:
            at_send_command("ATA", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_DTMF_START: {
            char c[4] = { 0 };
            c[0] = ((char *) data)[0];
            char *cmd;
            asprintf(&cmd, "AT+VTC=1,%s", c);
            at_send_command(cmd, NULL, CC_CHANNEL_CTX);
            free(cmd);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            strcpy(s_vtc, c); /* save the char for RIL_REQUEST_DTMF_STOP */
            break;
        }
        case RIL_REQUEST_DTMF_STOP:
            if (s_vtc[0] == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                char *cmd;
                asprintf(&cmd, "AT+VTC=0,%s", s_vtc);
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                free(cmd);
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
                memset(s_vtc, 0, sizeof(s_vtc));
            }
            break;
        case RIL_REQUEST_SET_MUTE:
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_SET_MUTE_FOR_RPC:
#endif
            requestSetMute(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE: {
            char * cmd;
            ATResponse *p_response = NULL;
            int err = 0;
            asprintf(&cmd, "AT+VP=%d", ((int *) data)[0]);
            err = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
            free(cmd);
            if (err < 0 || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }
        case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
            requestQueryPreferredVoicePrivacy(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_FLASH:
            requestSendFlash(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_BURST_DTMF: {
            char *c = ((char **) data)[0];
            char *cmd;
            int i = 0;
            int len = strlen(c);
            for (i = 0; i < len; i++) {
                asprintf(&cmd, "AT+VTS=%c", *(c + i));
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
            }
            free(cmd);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        }
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
            requestExitEmergencyCallbackMode(data, datalen, t);
            break;
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_EMERGENCY_DIAL:
            requestEmergencyDial(data, datalen, t);
            break;
        case RIL_REQUEST_SWITCH_HPF:
            requestSwitchHPF(data, datalen, t);
            break;
#endif
        case RIL_REQUEST_CONFERENCE:
            // 3GPP 22.030 6.5.5
            // "Adds a held call to the conversation"
            at_send_command("AT+CHLD=3", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_SEPARATE_CONNECTION: {
            char cmd[12] = { 0 };
            int party = ((int*) data)[0];

            // Make sure that party is in a valid range.
            // (Note: The Telephony middle layer imposes a range of 1 to 7.
            // It's sufficient for us to just make sure it's single digit.)
            if (party > 0 && party < 10) {
                sprintf(cmd, "AT+CHLD=2%d", party);
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
        }
            break;
        /* M: call control part start */
        case RIL_REQUEST_SET_SPEECH_CODEC_INFO:
            requestSetSpeechCodecInfo(data, datalen, t);
            break;
        /* M: call control part end */
        default:
            return 0; /* no matched request */
    }
    return 1;
}

extern int rilCcUnsolicited(const char *s, const char *sms_pdu) {
    char *line = NULL;
    int err;

    if (strStartsWith(s, "+CCWA:")) {
        char *number;
        int callType = 0;
        RIL_CDMA_CallWaiting_v6 *cdmawait = NULL;
        char *dup = strdup(s);
        cdmawait = (RIL_CDMA_CallWaiting_v6 *) alloca(
                sizeof(RIL_CDMA_CallWaiting_v6));
        memset(cdmawait, 0, sizeof(RIL_CDMA_CallWaiting_v6));
        if (!dup) {
            LOGD("CCWA strdup error");
            return 1;
        }
        line = dup;
        at_tok_start(&line);
        err = at_tok_nextstr(&line, &number);
        cdmawait->number = (char *) alloca(strlen(number) + 1);
        strcpy(cdmawait->number, number);

        at_tok_nextint(&line, &callType);
        if (145 == callType) {
            cdmawait->number_type = 1;
        } else if (129 == callType) {
            cdmawait->number_type = 2;
        }
#ifdef ADD_MTK_REQUEST_URC
        if (!strcmp(cdmawait->number, "Restricted")) {
            cdmawait->numberPresentation = 1;
        }
        /* Actually other unnormal number string may be
         distinguished as well.
         */
        else if (!strcmp(cdmawait->number, "UNKNOWN")
                || !strcmp(cdmawait->number, "Unknown")
                || !strcmp(cdmawait->number, "Unavailable")
                || !strcmp(cdmawait->number, "NotAvailable")
                || !strcmp(cdmawait->number, "LOOPBACK CALL")) {
            cdmawait->numberPresentation = 2;
        }
        if (cdmawait->numberPresentation != 0) {
            cdmawait->number = NULL;
        }
#endif
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CALL_WAITING, cdmawait,
                sizeof(RIL_CDMA_CallWaiting_v6));
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CRING:") || strStartsWith(s, "RING")) {
        if (!oemSupportEnable(g_oem_support_flag)
                || oemVcallInSupportEnable(g_oem_support_flag)) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                    NULL, 0);
            //at_send_command("AT+SPEAKER=4", NULL);    //xfge
        }
        return 1;
    } else if (strStartsWith(s, "^CEND:")) {
        int end_status = 0;
        int skip = 0;

        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &end_status);
        if (err < 0) {
            free(dup); return 1;
        }

        if (end_status == 23) {
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_INTERCEPT;
#if 0
            set_twl5030_path(voicetype);
#endif
        } else if (end_status == 24) {
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_REORDER;
#if 0
            set_twl5030_path(voicetype);
#endif
        } else {
            s_unsol_msg_cache.last_call_fail_cause = 0;
        }

        if (invoicecall != 0) {
            int data = 2;
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_CDMA_SIGNAL_FADE,
                    &data, sizeof(int));

        } else {
            int data = 1;
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_CDMA_SIGNAL_FADE,
                    &data, sizeof(int));
        }

        free(dup);
        //s_wait_repoll_flag = 1;
        //RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL, getDefaultChannelCtx()->id);
        invoicecall = 0;
#ifdef ADD_MTK_REQUEST_URC
        int speechInfo[3] = {0};
        speechInfo[0] = 0;
        RIL_onUnsolicitedResponse(RIL_UNSOL_SPEECH_INFO, speechInfo,
                sizeof(speechInfo));
#endif
        /*Clear loopback call query times*/
        if (loopbackcall_query_times) {
            loopbackcall_query_times = 0;
        }
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "NO CARRIER")) {
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^ORIG")) {
        //"%s:%d,%d", Msg.cmdName, callID, callType
          //LOGD("Call Originated callid = %d, calltype = ", callID, callType);
        LOGD("Call Originating...");
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^CONN:")) {
        LOGD("Call connected voicetype = %d", voicetype);
        invoicecall = 1;
#ifdef ADD_MTK_REQUEST_URC
        int speechInfo[3] = {0};
        speechInfo[0] = 1;
        RIL_onUnsolicitedResponse(RIL_UNSOL_SPEECH_INFO, speechInfo,
                sizeof(speechInfo));
#endif
        /*
         *   for reducing pop noise!
         *   now cp is ready ,finish the audio path routing
         */
#if 0
        set_twl5030_path(voicetype);
#endif
        return 1;
    } else if (strStartsWith(s, "+WSOS: ")) {
        char state = 0;
        int unsol;
        char *dup = NULL;
        line = dup = strdup(s);
        if (!dup) {
            LOGE("+WSOS: Unable to allocate memory");
            return 1;
        }
        if (at_tok_start(&dup) < 0) {
            free(dup);
            return 1;
        }
        if (at_tok_nextbool(&dup, &state) < 0) {
            LOGE("Invalid +WSOS response: %s", line);
            free(dup);
            return 1;
        }
        free(dup);
        unsol = state ?
            RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE : RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE;

        RIL_onUnsolicitedResponse(unsol, NULL, 0);
        return 1;
    /* M: call control part start */
    } else if (strStartsWith(s, "+EVOCD")) {
        onSpeechCodecInfo((char*)s);
        return 1;
    /* M: call control part end */
    } else if (strStartsWith(s, "+CFNM")) {
        onToneSignalsInfo((char*)s);
        return 1;
    }
    return 0;
}

static void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int countCalls = 0;
    int countValidCalls = 0;
    RIL_Call *p_calls = NULL;
    RIL_Call **pp_calls = NULL;
    int i = 0;
    int needRepoll = 0;
    int datacall_active = 0;

    err = at_send_command_multiline ("AT+CLCC", "+CLCC:", &p_response, CC_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        return;
    }

    /* count the calls */
    for (countCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        countCalls++;
    }

    /* yes, there's an array of pointers and then an array of structures */

    pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
    p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
    memset (p_calls, 0, countCalls * sizeof(RIL_Call));
    for (countValidCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls);
        if (err != 0) {
            continue;
        }
        p_calls[countValidCalls].index = countValidCalls+1;

        if (p_calls[countValidCalls].state != RIL_CALL_ACTIVE
                && p_calls[countValidCalls].state != RIL_CALL_HOLDING) {
            needRepoll = 1;
        }
        if (!p_calls[countValidCalls].isVoice && (p_calls[countValidCalls].number != NULL)) {
            if (!strcmp(p_calls[countValidCalls].number, "#777")) {
                datacall_active = 1;
            }
        }
        countValidCalls++;
    }

    for (i = 0; i < countValidCalls; i++) {
        pp_calls[i] = &(p_calls[i]);
    }
#if 0
    if (datacall_active) {
        system("echo DATA > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_on");
    } else {
        system("echo DATA > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_off");
    }
    if (!datacall_active && countValidCalls >= 1) {
        system("echo PHONE > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_on");
    } else {
        system("echo PHONE > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_off");
    }
#endif
    RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls,
            countValidCalls * sizeof (RIL_Call *));

    at_response_free(p_response);

#ifdef POLL_CALL_STATE
    if (countValidCalls) {  // We don't seem to get a "NO CARRIER" message from
                            // smd, so we're forced to poll until the call ends.
#else
    if (needRepoll) {
#endif
        RIL_requestProxyTimedCallback(sendCallStateChanged, NULL,
                &TIMEVAL_CALLSTATEPOLL, getDefaultChannelCtx()->id);
    }
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void sendCallStateChanged(void *param) {
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

static void requestDial(void *data, size_t datalen, RIL_Token t)
{
    RIL_Dial *p_dial;
    char *cmd;
    const char *clir;
    int ret;
    char ecclist[128] = { 0 };
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));

    p_dial = (RIL_Dial *)data;

    switch (p_dial->clir) {
        case 1: clir = "I"; break;  /*invocation*/
        case 2: clir = "i"; break;  /*suppression*/
        default:
        case 0: clir = ""; break;   /*subscription default*/
    }
    LOGD("%s: p_dial->address:%s, clir:%s, num len:%d", __FUNCTION__,
            p_dial->address, clir, strlen(p_dial->address));

    asprintf(&cmd, "AT+CDV=%s", p_dial->address);

#if 0
    /** according to the CT terminal testing specification : BasicReq-02001 & physical truth,
     * except the number "122".
     */
    if (0 == strcmp(p_dial->address, "110")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    } else if (0 == strcmp(p_dial->address, "112")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    } else if (0 == strcmp(p_dial->address, "119")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    } else if (0 == strcmp(p_dial->address, "120")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    //} else if (0 == strcmp(p_dial->address, "122")) {
    //    asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
    //    inemergency = 1;
    } else if (0 == strcmp(p_dial->address, "911")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    } else if (0 == strcmp(p_dial->address, "999")) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
        inemergency = 1;
    } else {
        asprintf(&cmd, "AT+CDV=%s", p_dial->address);
    }
#endif

    ret = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
    free(cmd);

    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

static int isCallEnded(RIL_CALL_STATUS* callStatus)
{
    int err = 0;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int countCalls = 0;
    int countValidCalls = 0;
    RIL_Call *p_calls = NULL;
    RIL_Call **pp_calls = NULL;
    int i = 0;
    LOGD("isCallEnded");

    memset(callStatus, 0, sizeof(RIL_CALL_STATUS));

    err = at_send_command_multiline ("AT+CLCC", "+CLCC:", &p_response, getDefaultChannelCtx());

    if (err != 0 || p_response->success == 0) {
        return -1;
    }

    /* count the calls */
    for (countCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        countCalls++;
    }

    /* yes, there's an array of pointers and then an array of structures */

    pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
    p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
    memset (p_calls, 0, countCalls * sizeof(RIL_Call));
    for (countValidCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls);
        if (err != 0) {
            continue;
        }
        p_calls->index = countValidCalls + 1;
        if (p_calls[countValidCalls].isVoice) {
            callStatus->isVoice = 1;
        }
        countValidCalls++;
    }
    at_response_free(p_response);
    LOGD("%s, isVoice:%d", __FUNCTION__, callStatus->isVoice);
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

static void requestHangup(void *data, size_t datalen, RIL_Token t)
{
    int ret;
    int i = 0;
    ATResponse *p_response = NULL;
    int waitInterval = 400; //ms
    int maxWaitCount = 25;  //waitInterval*maxWaitCount = 10s;
    RIL_CALL_STATUS callStatus;

    ret = at_send_command("AT+CHV", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response->success == 0) {
        goto error;
    }
    /*wait +CHV to take effect*/
    for (i = 0; i < maxWaitCount; i++) {
        usleep(waitInterval * 1000);
        isCallEnded(&callStatus);
        if (!callStatus.isVoice) {
            break;
        }
    }
    LOGD("%s: isVoice:%d, wait %d ms", __FUNCTION__, callStatus.isVoice, i * waitInterval);

    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestGetLastCallFailCause(void *data, size_t datalen, RIL_Token t)
{
    int responses = 0;
    if (s_unsol_msg_cache.last_call_fail_cause == 0) {
        responses = CALL_FAIL_NORMAL;
    } else {
        responses = s_unsol_msg_cache.last_call_fail_cause;
    }
    LOGD("Call exit code is %d, convert to fail cause %d",
            s_unsol_msg_cache.last_call_fail_cause, responses);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &responses, sizeof(int));
}

static void requestSetMute(void *data, size_t datalen, RIL_Token t)
{
    int err;
    char *cmd;
    assert(datalen >= sizeof(int *));
    LOGD("%s, ((int*)data)[0]:%d", __FUNCTION__, ((int*)data)[0]);
    asprintf(&cmd, "AT+CMUT=%d", ((int*)data)[0]);
    err = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
    free(cmd);
    if (err != 0) { goto error; }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestQueryPreferredVoicePrivacy(
                void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line = NULL;

    err = at_send_command_singleline("AT+VP?", "+VP:", &p_response, CC_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if (err < 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSendFlash(void *data, size_t datalen, RIL_Token t)
{
    char *c;
    char *cmd;
    int ret;
    char a[15] = {0};
    c = (char *)data;

    LOGD("%s, flash string len:%d, string:%s", __FUNCTION__, strlen(c), c);
    if (strlen(c) < 16) {
        asprintf(&cmd, "AT+CFSH=%s", c);
    } else {
        strncpy(a, c, 15);
        LOGD("flash string:%s", a);
        asprintf(&cmd, "AT+CFSH=%s", a);
    }
    ret = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
    free(cmd);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

static void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t)
{
    int ret = 0;
    LOGD("%s", __FUNCTION__);
    ret = at_send_command("AT+VMEMEXIT", NULL, CC_CHANNEL_CTX);
    if (ret != 0) {
        goto error;
    }
    //inemergency = 0;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    LOGD("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

#ifdef ADD_MTK_REQUEST_URC
static void requestEmergencyDial(void *data, size_t datalen, RIL_Token t)
{
    RIL_Dial *p_dial;
    char *cmd;
    const char *clir;
    int ret;
    char ecclist[128] = { 0 };
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));

    p_dial = (RIL_Dial *)data;

    switch (p_dial->clir) {
        case 1: clir = "I"; break;  /*invocation*/
        case 2: clir = "i"; break;  /*suppression*/
        default:
        case 0: clir = ""; break;   /*subscription default*/
    }
    LOGD("%s: p_dial->address:%s, clir:%s, num len:%d", __FUNCTION__,
            p_dial->address, clir, strlen(p_dial->address));

    inemergency = 1;
    psParam.enable = 1;
    turnPSEnable((void *)&psParam);
    asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);

    ret = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
    free(cmd);

    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

/*
Test command:
AT+VGSMST=?
Response:
OK

Write command:
AT+VGSMST =<status>
Response:
1) If successful:
OK
2) If failed
ERROR

Parameters:
<value>   0       GSM in no call state
          1       GSM in CALL state.
          Other   reserved
*/
static void requestSwitchHPF(void *data, size_t datalen, RIL_Token t)
{
    int onOff;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;

    onOff = ((int *)data)[0];

    LOGD("%s, onOff:%d", __FUNCTION__, onOff);
    asprintf(&cmd, "AT+VGSMST=%d", onOff);
    err = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0)) {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
#endif

/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
static int callFromCLCCLine(char *line, RIL_Call *p_call)
{
    int err;
    int state;
    int mode;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &(p_call->index));
    if (err < 0) {
        goto error;
    }
    err = at_tok_nextbool(&line, &(p_call->isMT));
    if (err < 0) {
        goto error;
    }
    err = at_tok_nextint(&line, &state);
    if (err < 0) {
        goto error;
    }
    err = clccStateToRILState(state, &(p_call->state));
    if (err < 0) {
        goto error;
    }
    err = at_tok_nextint(&line, &mode);
    if (err < 0) {
        goto error;
    }

    p_call->isVoice = (mode == 0);
    if (!p_call->isVoice) {
        LOGE("Filter data call line!");
        goto error;
    }

    err = at_tok_nextbool(&line, &(p_call->isMpty));
    if (err < 0) {
        goto error;
    }
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_call->number));
        /* tolerate null here */
        if (err < 0) { return 0; }
        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if (p_call->number != NULL && 0 == strspn(p_call->number, "+*#0123456789")) {
            if (strcmp(p_call->number, "Restricted")
                    && strcmp(p_call->number, "NotAvailable")
                    && strcmp(p_call->number, "UNKNOWN")
                    && strcmp(p_call->number, "LOOPBACK CALL")) {
                LOGD("Set number to null!");
                p_call->number = NULL;
            }
        }
        err = at_tok_nextint(&line, &p_call->toa);
        if (err < 0) { goto error; }
    }
    if (p_call->number != NULL && strcmp(p_call->number, "LOOPBACK CALL") == 0) {
        /*For SO55 test, */
        if (p_call->state == 0 && loopbackcall_query_times == 0) {
            p_call->state = 4;
        }
        loopbackcall_query_times++;
    }

#ifdef ADD_MTK_REQUEST_URC
    //for fix CRTS#19922
    if (p_call->number != NULL && !strcmp(p_call->number, "Restricted")) {
        p_call->numberPresentation = 1;
    }
    /* Actually other unnormal number string may be
        distinguished as well.
    */
    else if (p_call->number != NULL && (!strcmp(p_call->number, "UNKNOWN")
        || !strcmp(p_call->number, "Unknown")
        || !strcmp(p_call->number, "Unavailable")
        || !strcmp(p_call->number, "NotAvailable")
        || !strcmp(p_call->number, "LOOPBACK CALL"))
    ) {
        p_call->numberPresentation = 2;
    }
    if (p_call->numberPresentation != 0) {
        p_call->number = NULL;
    }
#endif
    return 0;

error:
    LOGE("Invalid CLCC line");
    return -1;
}

int clccStateToRILState(int state, RIL_CallState *p_state)
{
    switch(state) {
        case 0: *p_state = RIL_CALL_ACTIVE;   return 0;
        case 1: *p_state = RIL_CALL_HOLDING;  return 0;
        case 2: *p_state = RIL_CALL_DIALING;  return 0;
        case 3: *p_state = RIL_CALL_ALERTING; return 0;
        case 4: *p_state = RIL_CALL_INCOMING; return 0;
        case 5: *p_state = RIL_CALL_WAITING;  return 0;
        default: return -1;
    }
}

/* M: call control part start */
void requestSetSpeechCodecInfo(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int support = ((int *)data)[0];
    LOGD("%s, isSupport:%d", __FUNCTION__, support);
    asprintf(&cmd, "AT+EVOCD=%d", support);
    err = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void onSpeechCodecInfo(char *urc)
{
    int ret;
    int info = 0;
    LOGD("%s, urc:%s", __FUNCTION__, urc);
    if (strStartsWith(urc, "+EVOCD")) {
        ret = at_tok_start(&urc);
        if (ret < 0) { goto error; }

        ret = at_tok_nextint(&urc, &info);
        if (ret < 0) { goto error; }
    } else {
        goto error;
    }

    LOGD("%s, value:%d", __FUNCTION__, info);
    RIL_onUnsolicitedResponse(RIL_UNSOL_SPEECH_CODEC_INFO, &info, sizeof(int));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onToneSignalsInfo(char *urc)
{
    int ret;
    int info = 0;

    LOGD("%s, urc:%s", __FUNCTION__, urc);

    if (strStartsWith(urc, "+CFNM")) {
        ret = at_tok_start(&urc);
        if (ret < 0) { goto error; }

        ret = at_tok_nextint(&urc, &info);
        if (ret < 0) { goto error; }

    } else {
        goto error;
    }

    LOGD("%s, tone:%d", __FUNCTION__, info);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_TONE_SIGNALS, &info, sizeof(int));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}
/* M: call control part end */
