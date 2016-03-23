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

#include <ril_callbacks.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#define STK_CHANNEL_CTX getRILChannelCtxFromToken(t)
static const struct timeval TIMEVAL_0 = {2, 0};
static const struct timeval TIMEVAL_SMS = {0, 0};
static sat_at_string_struct g_stk_at;

static char aIs_stk_service_running[4] = {false,false,false,false};
static char aIs_proac_cmd_queued[4] = {false,false,false,false};
static char aIs_pending_open_channel[4] = {false,false,false,false};
static bool aIs_event_notify_queued[4] = {false,false,false,false};
static char* pProactive_cmd[4] = {0};
static char* pOpenChannelTR[4] = {0};
static char* pEvent_notify[4] = {0};
#define SETUP_EVENT_LIST_CMD_DETAIL_CODE "81030105"

void requestStkGetProfile (void *data, size_t datalen, RIL_Token t) {
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    RIL_STK_UNUSED_PARM(t);
}

void requestStkSetProfile (void *data, size_t datalen, RIL_Token t) {
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    RIL_STK_UNUSED_PARM(t);
}

void setStkFlagByTk(RIL_Token t, bool flag, char* source)
{
    if (NULL == source) {
        LOGD("setStkFlagByRid source is null.");
        return;
    }

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 3) = flag;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 2) = flag;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        *(source + 1) = flag;
    } else {
        *(source) = flag;
    }

    LOGD("setStkFlagByTk[%d][%d][%d][%d].",
        *(source), *(source + 1), *(source + 2), *(source + 3));
}

void setStkFlagByRid(RIL_SOCKET_ID rid, bool flag, char* source) {
    if (NULL == source) {
        LOGD("setStkFlagByRid source is null.");
        return;
    }

    if (RIL_SOCKET_4 == rid) {
        *(source + 3) = flag;
    } else if (RIL_SOCKET_3 == rid) {
        *(source + 2) = flag;
    } else if (RIL_SOCKET_2 == rid) {
        *(source + 1) = flag;
    } else {
        *source = flag;
    }

    LOGD("setStkFlagByRid[%d][%d][%d][%d].",
        *(source), *(source + 1), *(source + 2), *(source + 3));
}

char getStkFlagByTk(RIL_Token t, char* source) {

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 3);
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 2);
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 1);
    } else {
        return *(source);
    }

    LOGD("getStkFlagByTk[%d][%d][%d][%d].",
         *(source), *(source + 1), *(source + 2), *(source + 3));
}

char getStkFlagByRid(RIL_SOCKET_ID rid, char* source) {
    if (RIL_SOCKET_4 == rid) {
        return *(source + 3);
    } else if (RIL_SOCKET_3 == rid) {
        return *(source + 2);
    } else if (RIL_SOCKET_2 == rid) {
        return *(source + 1);
    } else {
        return *source;
    }
LOGD("getStkFlagByRid[%d][%d][%d][%d].",
        *(source), *(source + 1), *(source + 2), *(source + 3));
}

char* getStkCachedDataByRid(RIL_SOCKET_ID rid, char** source) {
    if (RIL_SOCKET_4 == rid) {
        return *(source + 3);
    } else if (RIL_SOCKET_3 == rid) {
        return *(source + 2);
    } else if (RIL_SOCKET_2 == rid) {
        return *(source + 1);
    } else {
        return *source;
    }
}
char* getStkCachedData(RIL_Token t, char** source) {

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 3);
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 2);
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return *(source + 1);
    } else {
        return *source;
    }
}

void setStkCachedData(RIL_SOCKET_ID rid, char** source, char* pCmd) {
    if (NULL == source) {
        LOGD("setStkCachedData source is null.");
        return;
    }

    if (RIL_SOCKET_4 == rid) {
        *(source + 3) = pCmd;
    } else if (RIL_SOCKET_3 == rid) {
        *(source + 2) = pCmd;
    } else if (RIL_SOCKET_2 == rid) {
        *(source + 1) = pCmd;
    } else {
        *source = pCmd;
    }
    LOGD("setStkCachedData rid:%d, [%p][%p][%p][%p].",
        rid, *(source), *(source + 1), *(source + 2), *(source + 3));
}
void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag) {
    setStkFlagByRid(rid, flag, aIs_stk_service_running);
}
void setStkEventNotifyQueuedFlagByTk(RIL_Token t, bool flag) {

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
	       aIs_event_notify_queued[3] = flag;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        aIs_event_notify_queued[2] = flag;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        aIs_event_notify_queued[1] = flag;
    } else {
        aIs_event_notify_queued[0] = flag;
    }
    LOGD("setStkEventNotifyQueuedFlagByTk[%d][%d][%d][%d].", aIs_event_notify_queued[0],
            aIs_event_notify_queued[1], aIs_event_notify_queued[2], aIs_event_notify_queued[3]);
}

void setStkEventNotifyQueuedFlag(RIL_SOCKET_ID rid, bool flag) {
    if (RIL_SOCKET_4 == rid) {
        aIs_event_notify_queued[3] = flag;
    } else if (RIL_SOCKET_3 == rid) {
        aIs_event_notify_queued[2] = flag;
    } else if (RIL_SOCKET_2 == rid) {
        aIs_event_notify_queued[1] = flag;
    } else {
        aIs_event_notify_queued[0] = flag;
    }
    LOGD("setStkEventNotifyQueuedFlag[%d][%d][%d][%d].", aIs_event_notify_queued[0],
            aIs_event_notify_queued[1], aIs_event_notify_queued[2], aIs_event_notify_queued[3]);
}

bool getStkEventNotifyQueuedFlag(RIL_Token t) {
    LOGD("getStkEventNotifyQueuedFlag[%d][%d][%d][%d].", aIs_event_notify_queued[0],
    	   aIs_event_notify_queued[1], aIs_event_notify_queued[2], aIs_event_notify_queued[3]);
    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[3];
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[2];
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return aIs_event_notify_queued[1];
    } else {
        return aIs_event_notify_queued[0];
    }
}

char* getStkQueuedEventNotify(RIL_Token t) {
    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        return pEvent_notify[3];
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        return pEvent_notify[2];
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        return pEvent_notify[1];
    } else {
        return pEvent_notify[0];
    }
}
char* getStkQueuedEventNotifyWithRid(RIL_SOCKET_ID rid) {
    if (RIL_SOCKET_4 == rid) {
        return pEvent_notify[3];
    } else if (RIL_SOCKET_3 == rid) {
        return pEvent_notify[2];
    } else if (RIL_SOCKET_2 == rid) {
        return pEvent_notify[1];
    } else {
        return pEvent_notify[0];
    }
}

void setStkQueuedEventNotifyWithRid(RIL_SOCKET_ID rid, char* pCmd) {
    LOGD("setStkQueuedEventNotifyWithRid[%p][socketId: %d].", pCmd, rid);
    if (RIL_SOCKET_4 == rid) {
        pEvent_notify[3] = pCmd;
    } else if (RIL_SOCKET_3 == rid) {
        pEvent_notify[2] = pCmd;
    } else if (RIL_SOCKET_2 == rid) {
        pEvent_notify[1] = pCmd;
    } else {
        pEvent_notify[0] = pCmd;
    }
    LOGD("setStkQueuedEventNotifyWithRid[%p][%s].", pCmd, pCmd);
}

void onStkAtSendFromUrc()
{
    int ret;
    ATResponse *p_response = NULL;

    switch(g_stk_at.cmd_type) {
        case CMD_SETUP_CALL:
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKCALL=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }
            break;
        case CMD_DTMF:
            // at_send_command("AT+STKDTMF=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            if(inCallNumber != 0) {
                ret = at_send_command("AT+STKDTMF=0", &p_response, getChannelCtxbyProxy(g_stk_at.rid));
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKDTMF=32,9", NULL, getChannelCtxbyProxy(g_stk_at.rid));
                }
            } else {
                at_send_command("AT+STKDTMF=32,7", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }
            break;
        case CMD_SEND_SMS:
            at_send_command("AT+STKSMS=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            break;
        case CMD_SEND_SS:
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKSS=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            } else if (g_stk_at.cmd_res == 0) {
                ret = at_send_command("AT+STKSS=0", &p_response, getChannelCtxbyProxy(g_stk_at.rid));
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKSS=32,3", NULL, getChannelCtxbyProxy(g_stk_at.rid));
                }
            }
            break;
        case CMD_SEND_USSD:
            if (g_stk_at.cmd_res == 50) {
                at_send_command("AT+STKUSSD=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            } else if (g_stk_at.cmd_res == 0) {
                ret = at_send_command("AT+STKUSSD=0", &p_response, getChannelCtxbyProxy(g_stk_at.rid));
                if (ret < 0 || p_response->success == 0) {
                    at_send_command("AT+STKUSSD=32,8", NULL, getChannelCtxbyProxy(g_stk_at.rid));
                }
            }
            break;
        default:
            break;
    }
}

void StkSendRequestComplete(int err, ATResponse *p_response, RIL_Token t)
{
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

void requestReportStkServiceIsRunning(void *data, size_t datalen, RIL_Token t)
{
    RIL_STK_UNUSED_PARM(data);
    RIL_STK_UNUSED_PARM(datalen);
    setStkFlagByTk(t, true, aIs_stk_service_running);
    LOGD("STK service is running is_proac_cmd_queued[%d].",
            getStkFlagByTk(t, aIs_proac_cmd_queued));
    if(true == getStkFlagByTk(t, aIs_proac_cmd_queued)) {
        char *cmd = (char *)getStkCachedData(t, pProactive_cmd);//getStkQueuedProactivCmd(t);
        LOGD("PC:[%p][%d][%s].", cmd, strlen(cmd), cmd);
        setStkFlagByTk(t, false, aIs_proac_cmd_queued);
        if(NULL != cmd) {
            MTK_UNSOL_STK_PROACTIVE_COMMAND(cmd, STK_CHANNEL_CTX);
       	}
    }
    if(true == getStkEventNotifyQueuedFlag(t)) {
        char *cmd = (char *)getStkQueuedEventNotify(t);
        LOGD("Event Notify:[%d][%s].", strlen(cmd), cmd);
        setStkEventNotifyQueuedFlagByTk(t, false);
        if(NULL != cmd) {
            MTK_UNSOL_STK_EVENT_NOTIFY(cmd, STK_CHANNEL_CTX);
       	}
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestStkSendEnvelopeCommandWithStatus (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;
    char *line = NULL;
    RIL_SIM_IO_Response sr;
    ATLine *p_cur = NULL;
    bool headIntermediate = true;

    RIL_STK_UNUSED_PARM(datalen);

    memset(&sr, 0, sizeof(sr));
    asprintf(&cmd, "AT+CUSATE=\"%s\"", (char *)data);

    err = at_send_command_raw(cmd, &p_response, STK_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        if (NULL == line) {
            LOGE("requestStkSendEnvelopeCommand ok but no intermediates.");
            goto done;
        }

        if (headIntermediate) {
            headIntermediate = false;
            LOGD("CUSATE,[%s]", line);
            //sub_line = strstr(line, "+CUSATE2:");
            err = at_tok_start(&line);
            if (err < 0) {
                LOGE("get +CUSATE: error.");
                goto error;
            }
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) {
                LOGE("response data is null.");
                goto error;
            }
        } else {
            /*skip <busy> field, just deal with sw1 sw2.*/
            LOGD("CUSATE2,[%s]", line);
            err = at_tok_start(&line);
            if (err < 0) {
                LOGE("get +CUSATE2: error.");
                goto error;
            }
            err = at_tok_nextint(&line, &(sr.sw1));
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &(sr.sw2));
            if (err < 0) goto error;
            LOGD("requestStkSendEnvelopeCommand sw: %02x, %02x", sr.sw1, sr.sw2);
        }
    }
    goto done;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    free(cmd);
    at_response_free(p_response);
    return;
done:
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    free(cmd);
    at_response_free(p_response);
    return;
}

void requestStkSendEnvelopeCommand (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;

    RIL_STK_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+STKENV=\"%s\"", (char *)data);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);

}

void requestStkSendTerminalResponse (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;

    RIL_STK_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+STKTR=\"%s\"", (char *)data);
    if (strStartsWith((char*)data, SETUP_EVENT_LIST_CMD_DETAIL_CODE)) {
        LOGD("Ignore TR of set up event list.");
        return;
    }
    if (true == getStkFlagByTk(t, aIs_pending_open_channel)) {
        setStkFlagByTk(t, false, aIs_pending_open_channel);
    }
    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void requestStkHandleCallSetupRequestedFromSim (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0, user_confirm = 0, addtional_info = 0;

    RIL_STK_UNUSED_PARM(datalen);
    char *temp_str = NULL;
    int temp_int = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(STK_CHANNEL_CTX);
    LOGD("requestStkHandleCallSetupRequestedFromSim");
    if (true == getStkFlagByTk(t, aIs_pending_open_channel)) {
        setStkFlagByTk(t, false, aIs_pending_open_channel);
        if (((int *)data)[0] == 1) {
            //redirect OPEN CHANNEL to BipService.
            cmd = getStkCachedData(t, pProactive_cmd);//getStkQueuedProactivCmd(t);
            if (NULL == cmd) {
                LOGD("cmd is null.");
                return;
            }
            LOGD("cmd: %s.", cmd);
            RIL_onUnsolicitedResponseSocket (
                RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                cmd, strlen(cmd),
                rid);
            return;
        } else {
            //response TR of OPEN CAHNNEL to SIM directlly.
            char* cachedTR = getStkCachedData(t, pOpenChannelTR);
            LOGD("cachedTR: %s.", cachedTR);
            requestStkSendTerminalResponse(cachedTR, strlen(cachedTR), t);
            return;
        }
   }

    if(((int *)data)[0] == 1) {
        user_confirm = 0;
    } else if(((int *)data)[0] == 32) { //ME currently unable to process
        user_confirm = 32;
        addtional_info = 2;
    } else if(((int *)data)[0] == 33) { //NW currently unable to process
        user_confirm = 33;
        addtional_info = 0x9d;
    } else if(((int *)data)[0] == 0) {
        user_confirm = 34;
    } else {
        assert(0);
    }

    if( addtional_info == 0) {
        asprintf(&cmd, "AT+STKCALL=%d", user_confirm);
    } else {
        asprintf(&cmd, "AT+STKCALL=%d, %d", user_confirm, addtional_info);
    }

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);

}

void requestStkSetEvdlCallByAP (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;
    int enabled = ((int *)data)[0];
    LOGD("requestStkSetEvdlCallByAP:%d.", enabled);

    RIL_STK_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+EVDLCALL=%d", enabled);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void onStkSessionEnd(char* urc, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_STK_UNUSED_PARM(urc);

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_STK_SESSION_END,
        NULL, 0,
        rid);
    return;
}

#define NUM_EVDL_CALL 6
void onStkEventDownloadCall(char* urc, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    int err, i = 0;
    char *p_cur = (char *)urc;
    int response[NUM_EVDL_CALL]={0};
    //RIL_EVDLCall response = NULL;

    /*
     * +EVDLCALL: <status>,<ti>,<is_mt_call>,<is_far_end>,<cause_len>,<cause>
     */
    for(i = 0; i < NUM_EVDL_CALL-1; i++) {
        response[i] = 0xff;
    }
    err = at_tok_start(&p_cur);
    if (err < 0) goto error;

    for(i = 0; i < NUM_EVDL_CALL-1; i++) {
        err = at_tok_nextint(&p_cur, &(response[i]));
        if(err < 0) { response[i] = 0xFF; LOGE("There is something wrong with item [%d]",i);}//goto error;
    }
    err = at_tok_nexthexint(&p_cur, &(response[i]));
    if(err < 0) {
        LOGE("There is something wrong with item [%d]",i);//goto error;
    }

    RIL_onUnsolicitedResponse(
            RIL_UNSOL_STK_EVDL_CALL,
            response, NUM_EVDL_CALL * sizeof(int),
            rid);

    return;
error:
    LOGE("There is something wrong with the +EVDLCALL");
}

#define NUM_STK_CALL_CTRL 3
void onStkCallControl(char* urc, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    int err, i = 0;
    char *p_cur = (char *)urc;
    char *responseStr[NUM_STK_CALL_CTRL] = {0};
    LOGD("onStkCallControl URC = %s", urc);

    /**
     * +STKCTRL: <mmi_info_type>,"<alphs_id>","<address>or<ss string>or<ussd string>"
     **/
    err = at_tok_start(&p_cur);
    if (err < 0) goto error;

    for (i = 0; i < NUM_STK_CALL_CTRL; i++) {
        err = at_tok_nextstr(&p_cur, &(responseStr[i]));
        if (err < 0) {
            //responseStr[i] = 0xFF;
            LOGE("There is something wrong with item [%d]",i);
        }//goto error;
    }
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_STK_CC_ALPHA_NOTIFY,
            responseStr, sizeof(responseStr),
            rid);
    return;
error:
    LOGE("There is something wrong with the +STKCTRL");
}

int checkStkCmdDisplay(char *cmd_str)
{
    int is_alpha_id_existed = 0;
    int is_icon_existed_not_self_explanatory = 0;
    int index = 0;
    int cmd_length = 0;
    char temp_str[3] = {0};
    char *end;

    cmd_length = strlen(cmd_str);

    while (cmd_length > index) {
        if (cmd_str[index + 1] == '5' && (cmd_str[index] == '0' || cmd_str[index] == '8') ) {

            index += 2;
            if (cmd_str[index] != '0' || cmd_str[index + 1] != '0' ) {
                is_alpha_id_existed = 1;
            }
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        } else if((cmd_str[index + 1] == 'E' || cmd_str[index + 1] == 'e') && (cmd_str[index] == '1' || cmd_str[index] == '9')) {
            int icon_qualifier = 0;

            index += 4;
            memset(temp_str, 0, 3);
            memcpy(temp_str, &(cmd_str[index + 1]), 1);
            icon_qualifier = strtoul(temp_str, &end, 16);

            if((icon_qualifier & 0x01) == 0x01) {
                if(is_alpha_id_existed == 0) {
                    return 1;
                }
            }
            index += 4;
        } else {
            index += 2;
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        }
    }
    return 0;
}

int checkStkCommandType(char *cmd_str)
{
    char temp_str[3] = {0};
    char *end;
    int cmd_type = 0;

    memcpy(temp_str, cmd_str, 2);

    cmd_type = strtoul(temp_str, &end, 16);
    cmd_type = 0x7F & cmd_type;

    return cmd_type;
}

char* decodeStkRefreshFileChange(char *str, int **cmd, int *cmd_length)
{
    int str_length = 0, file_num = 0, offset = 20, cmdoffset = 0;
    /*offset 20 including cmd_detail tlv: 10, device id tlv:8, file list tag:2*/
    char temp_str[5] = {0};
    char *end;
    str_length = strlen(str);
    char *efId_str;
    int file_idx = 0;

    if(str[offset] <= '7') { //file list length: if length < 7F it will use 2 bytes else it will use 4 bytes
        offset += 2;
    } else {
        offset += 4;
    }
    memcpy(temp_str, str + offset, 2); //copy number of files in file list to temp_str
    offset += 2;

    file_num = strtoul(temp_str, &end, 16);
    efId_str = (char*)calloc(1, (file_num * (sizeof(char) * 4)) + 1);

#ifndef MTK_WIFI_CALLING_RIL_SUPPORT
    *cmd_length = (file_num + 1) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
#else
    *cmd_length = (file_num + 2) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
    cmdoffset++;
#endif

    *(*cmd + cmdoffset) = SIM_FILE_UPDATE;

    cmdoffset++;
    LOGD("decodeStkRefreshFileChange file_num = %d", file_num);

    while( offset < str_length && file_num > file_idx) {
        if(((str[offset] == '6') || (str[offset] == '2') || (str[offset] == '4'))
           && ((str[offset + 1] == 'F')/*||(str[offset+1] == 'f')*/)) {
            memcpy(temp_str, str + offset, 4); //copy EFID to temo_str
            strncat(efId_str, temp_str, 4);
            *(*cmd + cmdoffset) = strtoul(temp_str, &end, 16);

            cmdoffset++;
            file_idx++;
        }
        offset += 4;
    }
    return efId_str;
}

extern int rild_sms_hexCharToDecInt(char *hex, int length);

int decodeStkRefreshAid(char *urc, char **paid)
{
    int offset = 18; //cmd_details & device identifies
    int refresh_length = strlen(urc) / 2;
    int files_length;
    int files_offset;
    int temp;

    if (offset >= refresh_length) {
        *paid = NULL;
        return 0;
    }

    temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
    offset += 2;
    if (temp == 0x12 || temp == 0x92) { //file list tag
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
        if (temp < 0x7F) {
            offset += (2 + temp * 2);
        } else {
            offset += 2;
            temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
            offset += (2 + temp * 2);
        }
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
    } else {
        offset -=2; // No flie list tag
    }

    LOGD("decodeStkRefreshAid temp = %02x, offset = %d", temp, offset);

    if (temp == 0x2F || temp == 0xAF) { // aid tag
        offset += 2;
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
        offset += 2;
        *paid = &urc[offset];
        return temp * 2;
    }

    return -1;
}

void onSimRefresh(char* urc, RILChannelCtx * p_channel)
{
    int *cmd = NULL;
    int cmd_length = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    RIL_SimRefreshResponse_v7 simRefreshRspV7;

#ifdef MTK_WIFI_CALLING_RIL_SUPPORT
    int sessionId = 0;
#endif
    int aid_len = 0;
    char *aid = NULL;
    char *efId_str;

    memset(&simRefreshRspV7, 0, sizeof(RIL_SimRefreshResponse_v7));
    LOGD("[onSimRefresh]type: %c, %s", urc[9], urc);
    switch(urc[9]) { // t point to cmd_deatil tag t[9] mean refresh type
        case '0':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT_FULL_FILE_CHANGE;
            break;
        case '1':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            if (NULL != efId_str && 0 < strlen(efId_str)) {
                simRefreshRspV7.ef_id = efId_str;
                LOGD("[onSimRefresh]efId = %s", simRefreshRspV7.ef_id);
            }
            simRefreshRspV7.result = SIM_FILE_UPDATE;
            break;
        case '2':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            if (NULL != efId_str && 0 < strlen(efId_str)) {
                simRefreshRspV7.ef_id = efId_str;
                LOGD("[onSimRefresh]efId = %s", simRefreshRspV7.ef_id);
            }
            simRefreshRspV7.result = SIM_INIT_FILE_CHANGE;
            break;
        case '3':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT;
            break;
        case '4':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_RESET;
            break;
        case '5': // ISIM app_reset
#ifdef MTK_WIFI_CALLING_RIL_SUPPORT
            aid_len = decodeStkRefreshAid(urc, &aid);
            sessionId = getActiveLogicalChannelId(aid);
            LOGD("[WiFi_Calling]decodeStkRefreshAid sessionId = %d", sessionId);
            //simRefreshRspV7.sessiodId = sessionId;
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = APP_INIT;
#else
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = APP_INIT;
#endif /* MTK_WIFI_CALLING_RIL_SUPPORT */
            break;
        case '6':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            if (NULL != efId_str && 0 < strlen(efId_str)) {
                simRefreshRspV7.ef_id = efId_str;
                LOGD("[onSimRefresh]efId = %s", simRefreshRspV7.ef_id);
            }
            simRefreshRspV7.result = SESSION_RESET;
            break;
        default:
            LOGD("Refresh type does not support.");
            return;
    }
    LOGD("aid = %s, %d.", simRefreshRspV7.aid, aid_len);
    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_SIM_REFRESH,
/*        cmd, cmd_length,*/
        &simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7),
        rid);
    free(cmd);
}

bool checkAlphaIdExist(char *cmd) {
    int cmdTag = 0;
    int curIdx = 0;

    if (NULL == cmd) {
        return false;
    }
    cmdTag = rild_sms_hexCharToDecInt(cmd, 2);
    LOGD("checkAlphaIdExist cmd %s", cmd);
    //search alpha id tag.
    if (0x05 == cmdTag || 0x85 == cmdTag) {
        if (4 < strlen(cmd)) {
            curIdx += 2;
            cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2); //length tag
            if (0x00 < cmdTag) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return false;
}

char* buildOpenChannelTRStr(char *cmd) {
    int curIdx = 0;
    int cmdLen = 0;
    int cmdTag = 0;
    int openChannelFirstPartLen = 0;
    int bearerDescLen = 0;
    int bufferSizeLen = 0;
    int openChannelTRLen = 0;
    char *pOpenChannelFirstPart = NULL;
    char *pBearerDesc = NULL;
    char *pBufferSize = NULL;
    char* pOpenChannelTR = NULL;

    LOGD("buildOpenChannelTRStr cmd %s", cmd);

    if (NULL != cmd) {
        cmdLen = strlen(cmd);
    } else {
        return NULL;
    }
    // 10: 2( command detail tag) + 2(len) + 2(number of command) +
    //      2(command type) + 2(command qualifier).
    // 8: device tag
    // 6: result tag
    openChannelFirstPartLen = (sizeof(char) * 24) + 1; // 10 + 8 + 6
    pOpenChannelFirstPart = (char*)calloc(1, openChannelFirstPartLen);
    memset(pOpenChannelFirstPart, 0, openChannelFirstPartLen);
    //7 = 6 (result tag) + 1 ('\0')
    memcpy(pOpenChannelFirstPart, &(cmd[curIdx]), openChannelFirstPartLen - 7);
    LOGD("pOpenChannelFirstPart cmd %s", pOpenChannelFirstPart);
    *(pOpenChannelFirstPart + 15) = '2'; //8 '2'
    *(pOpenChannelFirstPart + 17) = '1'; //8 '1'
    // append result code: 830122 (6 bytes)
    *(pOpenChannelFirstPart + 18) = '8';
    *(pOpenChannelFirstPart + 19) = '3';
    *(pOpenChannelFirstPart + 20) = '0';
    *(pOpenChannelFirstPart + 21) = '1';
    *(pOpenChannelFirstPart + 22) = '2';
    *(pOpenChannelFirstPart + 23) = '2';
    LOGD("pOpenChannelFirstPart %s", pOpenChannelFirstPart);
    curIdx += 18; // 10+8
    cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
    do {
        //search alpha id tag.
        if (0x05 == cmdTag || 0x85 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx + (cmdLen * 2) /*alpha id*/;
            }
        }
        cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
        //search bearer description tag.
        if (0x35 == cmdTag || 0xB5 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;//Back to command tag index.
                bearerDescLen = sizeof(char) * ((cmdLen * 2) + 4);
                pBearerDesc = (char*)calloc(1, bearerDescLen);
                memset(pBearerDesc, 0, bearerDescLen);
                memcpy(pBearerDesc, &(cmd[curIdx]), bearerDescLen);
                curIdx = curIdx + bearerDescLen;
                openChannelTRLen += bearerDescLen;
            }
        }
        LOGD("pOpenChannelFirstPart cmd %s", &(cmd[curIdx]));
        cmdTag = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
        //search buffer size tag.
        if (0x39 == cmdTag || 0xB9 == cmdTag) {
            if (4 < strlen(&(cmd[curIdx]))) {
                curIdx += 2;
            } else {
                break;
            }
            cmdLen = rild_sms_hexCharToDecInt(&cmd[curIdx], 2);
            curIdx += 2;
            if (0 < cmdLen) {
                curIdx = curIdx - 4;//Back to command tag index.
                bufferSizeLen = sizeof(char) * ((cmdLen * 2) + 4);
                pBufferSize = (char*)calloc(1, bufferSizeLen);
                memset(pBufferSize, 0, bufferSizeLen);
                memcpy(pBufferSize, &(cmd[curIdx]), bufferSizeLen);
                openChannelTRLen += bufferSizeLen;
            }
        }
    } while(false);
    curIdx = 0;
    openChannelTRLen += openChannelFirstPartLen; // 24: for the open channel first part +  1 '\0'
    pOpenChannelTR = (char*)calloc(1, openChannelTRLen);
    memset(pOpenChannelTR, 0, openChannelTRLen);
    strncat(pOpenChannelTR, pOpenChannelFirstPart, openChannelFirstPartLen - 1);
    LOGD("pOpenChannelTR %s", pOpenChannelTR);
    curIdx += (openChannelFirstPartLen - 1);
    if (NULL != pBearerDesc) {
        memcpy(&(pOpenChannelTR[curIdx]), pBearerDesc, bearerDescLen);
        curIdx += bearerDescLen;
        free(pBearerDesc);
    }
    if (NULL != pBufferSize) {
        memcpy(&(pOpenChannelTR[curIdx]), pBufferSize, bufferSizeLen);
        free(pBufferSize);
    }
    free(pOpenChannelFirstPart);
    LOGD("pOpenChannelTR end %s", pOpenChannelTR);
    return pOpenChannelTR;
}

void onStkProactiveCommand(char* urc, RILChannelCtx* p_channel)
{
    int err = 0, temp_int = 0, type_pos = 0, cmdDetail_pos = 0, alphaId_pos = 0;
    ATResponse *p_response = NULL;
    char *temp_str;
    char *cmd;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int urc_len = 0;
    bool isStkServiceRunning = false;
    char *pProCmd = NULL;
    char *pTempFullCmd = NULL;
    int cur_cmd = 0;
    char* cachedPtr = NULL;

    if(urc != NULL) urc_len = strlen(urc);

    isStkServiceRunning = getStkFlagByRid(rid, aIs_stk_service_running);
    LOGD("onStkProactiveCommand check %d.",isStkServiceRunning);

    if(false == isStkServiceRunning) {
        setStkFlagByRid(rid, true, aIs_proac_cmd_queued);
        pProCmd = (char*)calloc(1, urc_len + 1);
        memset(pProCmd, 0x0, urc_len + 1);
        memcpy(pProCmd, urc, urc_len);
        pTempFullCmd = getStkCachedDataByRid(rid, pProactive_cmd);
        if (NULL != pTempFullCmd) {
            free(pTempFullCmd);
        }
        setStkCachedData(rid, pProactive_cmd, pProCmd);
        LOGD("STK service is not running yet.[%p]",pProCmd);

        return;
    }

    err = at_tok_start(&urc);

    err = at_tok_nextint(&urc, &temp_int);

    err = at_tok_nextstr(&urc, &temp_str);

    if(temp_str[2] <= '7' ) { /*add comment*/
        type_pos = 10;
        cmdDetail_pos = 4;
        alphaId_pos = 22;
    } else {
        type_pos = 12;
        cmdDetail_pos = 6;
        alphaId_pos = 24;
    }
    switch(checkStkCommandType(&(temp_str[type_pos]))) {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos - 6]), p_channel);
            // return;
            break;
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            RIL_onUnsolicitedResponseSocket (
                RIL_UNSOL_STK_EVENT_NOTIFY,
                temp_str, strlen(temp_str),
                rid);
            return;
            break;
        case CMD_OPEN_CHAN:
            if (checkAlphaIdExist(&(temp_str[alphaId_pos]))) {
                //Cache TR of Open channel command.
                setStkFlagByRid(rid, true, aIs_pending_open_channel);
                cachedPtr = buildOpenChannelTRStr(&(temp_str[cmdDetail_pos]));
                pTempFullCmd = getStkCachedDataByRid(rid, pOpenChannelTR);
                if (NULL != pTempFullCmd) {
                    free(pTempFullCmd);
                }
                setStkCachedData(rid, pOpenChannelTR, cachedPtr);

                //Cache Open channel command for redirecting to BipService.
                pProCmd = (char*)calloc(1, strlen(temp_str) + 1);
                memset(pProCmd, 0x0, strlen(temp_str) + 1);
                memcpy(pProCmd, temp_str, strlen(temp_str));
                pTempFullCmd = getStkCachedDataByRid(rid, pProactive_cmd);
                if (NULL != pTempFullCmd) {
                    free(pTempFullCmd);
                }
                setStkCachedData(rid, pProactive_cmd, pProCmd);
            } else {
                LOGD("Redirect PC to BipService.");
                RIL_onUnsolicitedResponseSocket (
                    RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                    temp_str, strlen(temp_str),
                    rid);
                return;
            }
            break;
        case CMD_CLOSE_CHAN:
        case CMD_RECEIVE_DATA:
        case CMD_SEND_DATA:
        case CMD_GET_CHAN_STATUS:
            RIL_onUnsolicitedResponseSocket (
                RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND,
                temp_str, strlen(temp_str),
                rid);
            return;
        default:
            break;
    }
    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_STK_PROACTIVE_COMMAND,
        temp_str, strlen(temp_str),
        rid);

    return;
}

unsigned int findStkCallDuration(char* str)
{
    int length = 0, offset = 0, temp = 0;
    unsigned int duration = 0;
    char temp_str[3] = {0};
    char *end;

    length = strlen(str);

    while(length > 0) {

        if(str[offset] == '8' && str[offset + 1] == '4') {
            memcpy(temp_str, &(str[offset + 6]), 2);
            if(str[offset + 5] == '0') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 60000;
            } else if(str[offset + 5] == '1') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 1000;
            } else if(str[offset + 5] == '2') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 100;
            }
            break;
        } else {
            length -= 2;
            offset += 2;
            memcpy(temp_str, &(str[offset]), 2);
            temp = strtoul(temp_str, &end, 16);
            length -= (2 * temp + 2);
            offset += (2 * temp + 2);

        }
    }
    return duration;
}

void onStkEventNotify(char* urc, RILChannelCtx* p_channel)
{
    int err = 0;
    int temp_int = 0;
    int cmd_type = 0;
    int type_pos = 0;
    int cmd_not_understood = 0; /* mtk02374 20100502*/
    unsigned int duration = 0;
    char *temp_str;
    char *cmd;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int urc_len = 0;
    bool isStkServiceRunning = false;
    char *pEventNotify = NULL;

    ATResponse *p_response = NULL;

    if(urc != NULL) urc_len = strlen(urc);

    isStkServiceRunning = getStkFlagByRid(rid, aIs_stk_service_running);
    LOGD("onStkEventNotify check %d.urc_len %d.", isStkServiceRunning, urc_len);

    if(false == isStkServiceRunning) {
        setStkEventNotifyQueuedFlag(rid, true);
        pEventNotify = (char*)calloc(1, urc_len + 1);
        memset(pEventNotify, 0x0, urc_len + 1);
        memcpy(pEventNotify, urc, urc_len);
        setStkQueuedEventNotifyWithRid(rid, pEventNotify);
        LOGD("STK service is not running yet.[%p]", pEventNotify);

        return;
    }

    err = at_tok_start(&urc);
    err = at_tok_nextint(&urc, &temp_int);
    err = at_tok_nextstr(&urc, &temp_str);
    if(temp_str[2] <= '7' ) { /*add comment*/
        type_pos = 10;
    } else if(temp_str[2] > '7' ) {
        type_pos = 12;
    }
    cmd_not_understood = checkStkCmdDisplay(&(temp_str[type_pos - 6])); /*temp_str[type_pos -6] points to cmd_detail tag*/
    switch(checkStkCommandType(&(temp_str[type_pos]))) {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos - 6]), p_channel);
            break;
        case CMD_SETUP_CALL:
            cmd_type = CMD_SETUP_CALL;
            if(cmd_not_understood == 0) {
                duration = findStkCallDuration(&temp_str[type_pos - 6]); /*temp_str[type_pos -6] points to cmd_detail tag*/
            } else {
                g_stk_at.cmd_type = CMD_SETUP_CALL;
                g_stk_at.cmd_res = 50;
                g_stk_at.rid = rid;
                RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                              getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            }
            break;
        case CMD_SEND_SMS:
            g_stk_at.cmd_type = CMD_SEND_SMS;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_SMS,
                                          getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        case CMD_SEND_SS:
            g_stk_at.cmd_type = CMD_SEND_SS;
            if(cmd_not_understood == 0) {
                g_stk_at.cmd_res = 0;
            } else {
                g_stk_at.cmd_res = 50;
            }

            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        case CMD_SEND_USSD:
            g_stk_at.cmd_type = CMD_SEND_USSD;
            if(cmd_not_understood == 0) {
                g_stk_at.cmd_res = 0;
            } else {
                g_stk_at.cmd_res = 50;
            }
            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                                          getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        default:
            break;
    }

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_STK_EVENT_NOTIFY,
        temp_str, strlen(temp_str),
        rid);
    if(CMD_SETUP_CALL == cmd_type) {
        RIL_onUnsolicitedResponseSocket (
            RIL_UNSOL_STK_CALL_SETUP,
            &duration, sizeof(duration),
            rid);
    }
    return;
}

char* StkbtSapMsgIdToString(MsgId msgId) {
    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            return "BT_SAP_CARD_READER_STATUS";
        default:
            return "BT_SAP_UNKNOWN_MSG_ID";
    }
}

/* Response value for Card Reader status
 * bit 8: Card in reader is powered on or not (powered on, this bit=1)
 * bit 7: Card inserted or not (Card inserted, this bit=1)
 * bit 6: Card reader is ID-1 size or not (our device is not ID-1 size, so this bit =0)
 * bit 5: Card reader is present or not (for our device, this bit=1)
 * bit 4: Card reader is removable or not (for our device, it is not removable, so this bit=0)
 * bit 3-1: Identifier of the Card reader (for our device: ID=0)
 * normal case, card reader status of our device = 0x11010000 = 0xD0
 * default case, card reader status of our device = 0x00010000 = 0x10
 */

void requestBtSapGetCardStatus(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ *req = NULL;
        (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)data;
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP rsp;
    BtSapStatus status = -1;

    LOGD("[BTSAP] requestBtSapGetCardStatus start, (%d)", rid);

    req = (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)
        malloc(sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));
    memset(req, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));

    status = queryBtSapStatus(rid);
    LOGD("[BTSAP] requestBtSapGetCardStatus status : %d", status);

    //decodeStkBtSapPayload(MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS, data, datalen, req);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
    rsp.CardReaderStatus = BT_SAP_CARDREADER_RESPONSE_DEFAULT;
    rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS;
    rsp.has_CardReaderStatus = true;   //always true

    if (isSimInserted(rid)) {
        LOGD("[BTSAP] requestBtSapGetCardStatus, Sim inserted");
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_SIM_INSERT;
    }

    if (status == BT_SAP_CONNECTION_SETUP || status == BT_SAP_ONGOING_CONNECTION
        || status == BT_SAP_POWER_ON) {
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_READER_POWER;
    }

    LOGD("[BTSAP] requestBtSapGetCardStatus, CardReaderStatus result : %x", rsp.CardReaderStatus);
    sendStkBtSapResponseComplete(t, RIL_E_SUCCESS, MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS,
        &rsp);
    free(req);

    RLOGD("[BTSAP] requestBtSapGetCardStatus end");
}

void decodeStkBtSapPayload(MsgId msgId, void *src, size_t srclen, void *dst) {
    pb_istream_t stream;
    const pb_field_t *fields = NULL;

    RLOGD("[BTSAP] decodeStkBtSapPayload start (%s)", StkbtSapMsgIdToString(msgId));
    if (dst == NULL || src == NULL) {
        RLOGE("[BTSAP] decodeStkBtSapPayload, dst or src is NULL!!");
        return;
    }

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields;
            break;
        default:
            RLOGE("[BTSAP] decodeStkBtSapPayload, MsgId is mistake!");
            return;
    }

    stream = pb_istream_from_buffer((uint8_t *)src, srclen);
    if (!pb_decode(&stream, fields, dst) ) {
        RLOGE("[BTSAP] decodeStkBtSapPayload, Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    } else {
        RLOGD("[BTSAP] decodeStkBtSapPayload, Success!");
    }
}

void sendStkBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("[BTSAP] sendStkBtSapResponseComplete, start (%s)", StkbtSapMsgIdToString(msgId));

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields;
            break;
        default:
            RLOGE("[BTSAP] sendStkBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("[BTSAP] sendStkBtSapResponseComplete, Size: %d (0x%x) Size as written: 0x%x",
                encoded_size, encoded_size, written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("[BTSAP] sendStkBtSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %u. encoded size result: %d",
        msgId, encoded_size, success);
    }
}

extern int rilStkMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND_WITH_STATUS(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            MTK_REQUEST_STK_SEND_TERMINAL_RESPONSE(data, datalen, t);
            break;
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
            MTK_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM(data, datalen, t);
            break;
        case RIL_REQUEST_STK_EVDL_CALL_BY_AP:
            MTK_REQUEST_STK_SET_EVDL_CALL_BY_AP(data, datalen, t);
            break;
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            MTK_REQUEST_REPORT_STK_SERVICE_IS_RUNNING(data, datalen, t);
            break;
        default:
            return 0; /* no matched requests */
            break;
    }

    return 1; /* request found and handled */
}

extern int rilStkUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_STK_UNUSED_PARM(sms_pdu);

    if(strStartsWith(s, "+STKPCI: 0")) {
        MTK_UNSOL_STK_PROACTIVE_COMMAND((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKPCI: 1")) {
        MTK_UNSOL_STK_EVENT_NOTIFY((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKPCI: 2")) {
        MTK_UNSOL_STK_SESSION_END((char *)s, p_channel);
    } else if(strStartsWith(s, "+EVDLCALL:")) {
        MTK_UNSOL_STK_EVDL_CALL((char *)s, p_channel);
    } else if(strStartsWith(s, "+STKCTRL:")) {
        MTK_UNSOL_STK_CALL_CTRL((char *)s, p_channel);
    } else {
        return 0;
    }
    return 1;
}

extern int rilStkBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    switch (request) {
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            requestBtSapGetCardStatus(data, datalen, t, rid);
            break;
        default:
            return 0;
            break;
    }

    return 1;
}

