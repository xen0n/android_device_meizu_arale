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

#define SMS_CHANNEL_CTX getRILChannelCtxFromToken(t)

typedef struct{
    int sms_index;
    char sms_storeType[5];
} NewMsgStorageStruct;

extern const struct timeval TIMEVAL_NETWORK_REPOLL;
extern int s_wait_repoll_flag;

static int sms_index = -1;

static void requestSMSAcknowledge(void *data, size_t datalen, RIL_Token t);
static void requestSendSMS(void *data, size_t datalen, RIL_Token t);
static void requestCdmaWriteSmsToRuim(void *data, size_t datalen, RIL_Token t);
static void requestWriteSmsToSim(void *data, size_t datalen, RIL_Token t);
static void oemDispatchNewCmdaSMS(const char *sms_pdu);
static void oemRequestSendCdmaSMS(void *data, size_t datalen, RIL_Token t);
static void requesCdmatSMSAcknowledge(void *data, size_t datalen, RIL_Token t);
static void requestGetSmsSimMemStatus(void *data, size_t datalen, RIL_Token t);
static void requestSendCNMA(void* param);
static void requestSetUimSmsRead(void *data, size_t datalen, RIL_Token t);
static void requestSendCdmaSMS(void *data, size_t datalen, RIL_Token t);
static void repollNetworkState(void *param);
static int isSimSmsFull(void);
static int getSmsSimMemStatus(int *totalMem, int *usedMem);
static void requestConvertRealPdu(void *data, size_t datalen, RIL_Token t);
static void encodeUimPduFromRilSmsStruc(RIL_CDMA_SMS_Message* SmsMsg, char** s_pdu);
static void requestSetPreferStorage(void *data, size_t datalen, RIL_Token t);
static void requestQuerySmsAndPB(void *data, size_t datalen, RIL_Token t);
static void requestReadDelNewSms(void* param);


int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response;
    int err = 0;
    switch (request) {
        case RIL_REQUEST_SEND_SMS:
            requestSendSMS(data, datalen, t);
            break;
        case RIL_REQUEST_SMS_ACKNOWLEDGE:
            requestSMSAcknowledge(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_SMS_TO_SIM:
            requestWriteSmsToSim(data, datalen, t);
            break;
        case RIL_REQUEST_DELETE_SMS_ON_SIM: {
            char * cmd;
            p_response = NULL;
            asprintf(&cmd, "AT+CMGD=%d", ((int *) data)[0]);
            err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
            free(cmd);
            if (err < 0 || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }
        case RIL_REQUEST_CDMA_SEND_SMS:
            #if 0
            if (oemSmsOutSupportEnable(g_oem_support_flag)) {
                oemRequestSendCdmaSMS(data, datalen, t);
            } else if (oemSupportEnable(g_oem_support_flag)) {
                RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
            } else {
                requestSendCdmaSMS(data, datalen, t);
            }
            #else
                oemRequestSendCdmaSMS(data, datalen, t);
            #endif
            break;
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
            requesCdmatSMSAcknowledge(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
            requestCdmaWriteSmsToRuim(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: {
            char * cmd;
            int index;
            p_response = NULL;
            err = -1;

            index = ((int *)data)[0] - 1;
            if ( index >= 0)
            {
              at_send_command("AT+CPMS=\"SM\"", NULL, SMS_CHANNEL_CTX);
              asprintf(&cmd, "AT+CMGD=%d", ((int *)data)[0] - 1);
              err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
              at_send_command("AT+CPMS=\"ME\"", NULL, SMS_CHANNEL_CTX);
              free(cmd);
            }
            #ifdef ADD_MTK_REQUEST_URC
            else if (((int *)data)[0] == -1) {
              at_send_command("AT+CPMS=\"SM\"", NULL, SMS_CHANNEL_CTX);
              err = at_send_command("AT+CMGD=,4", &p_response, SMS_CHANNEL_CTX);
              at_send_command("AT+CPMS=\"ME\"", NULL, SMS_CHANNEL_CTX);
            }
            #endif

            if (err < 0 || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS:
            requestGetSmsSimMemStatus(data, datalen, t);
            break;
        case RIL_REQUEST_SET_UIM_SMS_READ:
            requestSetUimSmsRead(data, datalen, t);
            break;
#endif
        case RIL_REQUEST_READ_SMS_STATUS_REPORT: {
            int index = ((int*) data)[0];
            char *cmd_read = NULL;

            asprintf(&cmd_read, "AT+CMGR=%d", index);
            int err_read = at_send_command(cmd_read, NULL, SMS_CHANNEL_CTX);
            free(cmd_read);

            if (err == 0)
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            else
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

            break;
        }
        case RIL_REQUEST_CDMA_SEND_REAL_PDU:
            LOGD("RIL_REQUEST_CDMA_SEND_REAL_PDU got in rilc");
            requestSendCdmaSMS(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS:
            requestQuerySmsAndPB(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_CONVERT_REAL_PDU:
            LOGD("RIL_REQUEST_CDMA_CONVERT_REAL_PDU got in rilc");
            requestConvertRealPdu(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }
    return 1;
}

int rilSmsUnsolicited(const char *s, const char *sms_pdu)
{
    char *line = NULL;
    int err;
    if (strStartsWith(s, "+CDSI:")) {
        int index = 0;
        char* memType;

        char* dup = strdup(s);
        if (!dup)
            return 1;

        line = dup;
        at_tok_start(&line);

        /** We can ignore storeType because SM is default storage type*/
        err = at_tok_nextstr(&line, &memType);
        if (err < 0) {
            free(dup);
            return 1; //error at command format
        }
        NewMsgStorageStruct *sms_newMsgStore;
        sms_newMsgStore = (NewMsgStorageStruct *) malloc(
                sizeof(NewMsgStorageStruct));
        memset(sms_newMsgStore, 0, sizeof(NewMsgStorageStruct));
        strcpy(sms_newMsgStore->sms_storeType, memType);
        err = at_tok_nextint(&line, &(sms_newMsgStore->sms_index));
        if (err < 0) {
            free(dup);
            return 1; //error at command format
        }
        RIL_requestProxyTimedCallback(requestReadDelNewSms, (void*) sms_newMsgStore,
                NULL, MISC_CHANNEL);

        free(dup);
        return 1;
    } else if (strStartsWith(s, "^SMMEMFULL:")) {
        char *memType = NULL;
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &memType);
        if (err < 0) {
            free(dup);
            return 1;
        }
        if (!strncmp(memType, "SM", 2)) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);
        }
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CMT:")) {
        /**
         *
         * !using CMT unsolicited new sms, should send cnma as an ack to modem
         * !this shoulde be immediately after CMT unsolicited
         * !requestSendCNMA is callback using to send at command in unsolicited thread
         *
         */
        RIL_requestProxyTimedCallback(requestSendCNMA, NULL, NULL, MISC_CHANNEL);

        /**
         * +CMT format:
         * +CMT:<mem>,<index>,<length>,<pdu>
         */
        RIL_CDMA_New_Sms NewSms;
        memset(&NewSms, 0, sizeof(RIL_CDMA_New_Sms));

        char* dup = strdup(s);
        if (!dup)
            return 1;
        line = dup;
        at_tok_start(&line);

        err = at_tok_nextstr(&line, &(NewSms.memType));
        if (err < 0)
            return 1; //error at command format

        err = at_tok_nextint(&line, &(NewSms.index));
        if (err < 0)
            return 1; //error at command format

        err = at_tok_nextint(&line, &(NewSms.length));
        if (err < 0)
            return 1; //error at command format

        err = at_tok_nextstr(&line, &(NewSms.pdu));
        if (err < 0)
            return 1; //error at command format

        oemDispatchNewCmdaSMS(NewSms.pdu);

        /*!!! SHOULD NOT free line, because the pointer line was moved by at_tok_nextxxx functions !!!*/
        free(dup); //run into here, dup can NOT be NULL
        return 1;
    } else if (strStartsWith(s, "+CMGR:")) {
        oemDispatchNewCmdaSMS(sms_pdu);
        return 1;
    } else if (strStartsWith(s, "+CVMI:")) {
        char *dup = strdup(s);
        int num = 0;
        char pdu[64];
        memset(pdu, '\0', 64);

        line = dup;

        at_tok_start(&line);

        err = at_tok_nextint(&line, &num);
        free(dup);
        if (err < 0) {
            LOGD("Handle +CVMI error!");
            return 1;
        }
        LOGD("Handle +CVMI: num = 0x%x", num);
        if (num > 99) {
            num = 99;
        }
        //to bcd code
        num = ((num / 10) << 4) | (num % 10);
        sprintf(pdu, "000002100302020040080c0003100000010210000B01%02x", num);
        LOGD("%s", pdu);
        oemDispatchNewCmdaSMS(pdu);
        return 1;
    } else if (strStartsWith(s, "+CMTI")) {
        /**
         * +CMTI  format:
         * +CMTI: <mem>,<index>
         */
        char *storeType;

        char* dup = strdup(s);
        if (!dup)
            return 1;
        line = dup;
        at_tok_start(&line);

        /** We can ignore storeType because SM is default storage type*/
        err = at_tok_nextstr(&line, &storeType);
        if (err < 0) {
            free(dup);
            return 1; //error at command format
        }
        NewMsgStorageStruct *sms_newMsgStore;
        sms_newMsgStore = (NewMsgStorageStruct *) malloc(
                sizeof(NewMsgStorageStruct));
        memset(sms_newMsgStore, 0, sizeof(NewMsgStorageStruct));
        strcpy(sms_newMsgStore->sms_storeType, storeType);
        err = at_tok_nextint(&line, &(sms_newMsgStore->sms_index));
        if (err < 0) {
            free(dup);
            return 1; //error at command format
        }

        if (!oemSupportEnable(g_oem_support_flag)
                || oemSmsInSupportEnable(g_oem_support_flag)) {
            RIL_requestProxyTimedCallback(requestReadDelNewSms,
                    (void*) sms_newMsgStore, NULL, MISC_CHANNEL);
        }

        /** pass index value to callback to read sms*/
        /*RIL_requestProxyTimedCallback (requestReadSms, (void*)&sms_index, NULL, getDefaultChannelCtx()->id);*/

        /** pass index value to callback to delete sms*/
        /*RIL_requestProxyTimedCallback (requestDelIndexedSms, (void*)&sms_index, NULL, getDefaultChannelCtx()->id);*/
        free(dup);
        return 1;
    }
    return 0;
}

static void requestSendSMS(void *data, size_t datalen, RIL_Token t)
{
    int err;
    const char *smsc;
    const char *pdu;
    int tpLayerLength;
    char *cmd1, *cmd2;
    RIL_SMS_Response response;
    ATResponse *p_response = NULL;

    smsc = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    tpLayerLength = strlen(pdu)/2;

    // "NULL for default SMSC"
    if (smsc == NULL) {
        smsc= "00";
    }

    asprintf(&cmd1, "AT+CMGS=%d", tpLayerLength);
    asprintf(&cmd2, "%s%s", smsc, pdu);

    err = at_send_command_sms(cmd1, cmd2, "+CMGS:", &p_response, SMS_CHANNEL_CTX);
    free(cmd1);
    free(cmd2);
    if (err != 0 || p_response->success == 0)
    {
        goto error;
    }

    memset(&response, 0, sizeof(response));

    /* FIXME fill in messageRef and ackPDU */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/*Liqi: Apollo for writing SMS into UIM*/
static void requestCdmaWriteSmsToRuim(void *data, size_t datalen, RIL_Token t)
{
    //char * s_pdu = (char *)data;
    char *cmd = NULL, *line = NULL;
    char *storage = NULL;
    int err = 0, response = 0;
    ATResponse *p_response = NULL;
    RIL_CDMA_SMS_WriteArgs* p_args = (RIL_CDMA_SMS_WriteArgs *)data;

    char* s_pdu = NULL;
    char* s_number = NULL;


    char *indexStr = NULL;
    char *result[2];


    /* funcs malloced memory for s_pdu & s_number should free at end of these piece of code */
    err = RILEncodeCdmaSmsPdu(&(p_args->message), &s_pdu, &s_number);

    if(err < 0){
        goto end;
    }
    at_send_command("AT+CPMS=\"ME\", \"SM\"", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CMGW=\"0\",\"%s\",%d" , s_pdu, p_args->status);
    err = at_send_command_singleline(cmd, "+CMGW:", &p_response, SMS_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0) {
        err = -1;
        goto end;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto end;

    err = at_tok_nextstr(&line, &storage);
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &response);
    if (err < 0) goto end;

end:
    at_send_command("AT+CPMS=\"ME\", \"ME\"", NULL, SMS_CHANNEL_CTX);

    if(!err){
        LOGD("[CDMA SMS] write sms into UIM which index=%d", response);

        // transfer int index to string
        asprintf(&indexStr, "%d" , response);
        result[0] = indexStr;
        result[1] = s_pdu;
        RIL_onRequestComplete(t, RIL_E_SUCCESS, result, 2*sizeof(char*));

    }else{
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    at_response_free(p_response);


    if(indexStr != NULL) {
        free(indexStr);
        indexStr = NULL;
    }


    if(s_pdu)
        free(s_pdu);

    if(s_number)
        free(s_number);

    if(cmd)
        free(cmd);

    #ifdef ADD_MTK_REQUEST_URC
    if(1 == isSimSmsFull())
    {
        RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);
    }
    #endif
}

static void requestSMSAcknowledge(void *data, size_t datalen, RIL_Token t)
{
    int ackSuccess;
    int err;

    ackSuccess = ((int *)data)[0];

    if (ackSuccess == 1) {
        err = at_send_command("AT+CNMA=1", NULL, SMS_CHANNEL_CTX);
    } else if (ackSuccess == 0)  {
        err = at_send_command("AT+CNMA=2", NULL, SMS_CHANNEL_CTX);
    } else {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return ;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

static void requestWriteSmsToSim(void *data, size_t datalen, RIL_Token t)
{
    RIL_SMS_WriteArgs *p_args = NULL;
    char *cmd = NULL;
    int length = 0;
    int err = 0;
    ATResponse *p_response = NULL;

    p_args = (RIL_SMS_WriteArgs *)data;

    length = strlen(p_args->pdu)/2;
    asprintf(&cmd, "AT+CMGW=%d,%d", length, p_args->status);

    err = at_send_command_sms(cmd, p_args->pdu, "+CMGW:", &p_response, SMS_CHANNEL_CTX);

    free(cmd);
    if (err != 0 || p_response->success == 0)
    {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void oemRequestSendCdmaSMS(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_SMS_Response response;
    RIL_CDMA_SMS_Message* p_message = (RIL_CDMA_SMS_Message*)data;
    char* cmd = NULL;
    char* s_pdu = NULL;
    char* s_number = NULL;

    /* funcs malloced memory for s_pdu & s_number should free at end of these piece of code */
    err = RILEncodeCdmaSmsPdu(p_message, &s_pdu, &s_number);
    if(err < 0){
        goto error;
    }

    at_send_command("at+cmgf = 0", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CMGS=\"0\", \"%s\"", s_pdu);
    //err = at_send_command_notimeout(cmd, &p_response);
    /* Use a big enough value to make sure we can get the HCMGSS/HCMGSF after CMGS */
    err = at_send_command_with_specified_timeout(cmd, MAX_TIMEOUTSECS_TO_RECEIVE_HCMGSS, &p_response, SMS_CHANNEL_CTX);
    free(cmd);
    if (s_pdu != NULL) {
        free(s_pdu);
        s_pdu = NULL;
    }
    if (s_number != NULL) {
        free(s_number);
        s_number = NULL;
    }

    memset(&response, 0, sizeof(response));
    if (err < 0 || p_response->success == 0) goto error;

    char* dup = strdup(p_response->finalResponse);
    if (!dup)  goto error;

    char* respline = dup;
    if(strStartsWith(respline, "^HCMGSF:"))
    {
      at_tok_start(&respline);

      err = at_tok_nextint(&respline, &(response.errorCode));
      if (err < 0) {
        free(dup);
        goto error;
      }
      RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, &response, sizeof(response));
    }
    else
    {
      response.messageRef = atoi(respline + 8);
      RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    }

    free(dup);

    /* ignore the unsolicited msg of network change in the first 4 seconds */
    s_wait_repoll_flag = 1;
    RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL, MISC_CHANNEL);

    at_response_free(p_response);
    return;

error:
    if (s_pdu != NULL) free(s_pdu);
    if (s_number != NULL) free(s_number);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void oemDispatchNewCmdaSMS(const char *sms_pdu)
{
    char* dup  = strdup(sms_pdu);
    char* pdu  = dup;
    int pdulen = strlen(pdu);
    RIL_CDMA_SMS_Message *cdma_sms = NULL;
    unsigned char* bytePdu = NULL;

    cdma_sms = (RIL_CDMA_SMS_Message *)alloca(sizeof(RIL_CDMA_SMS_Message));
    memset(cdma_sms, 0, sizeof(RIL_CDMA_SMS_Message));

    // must remove the 2 quotes char
    // 2 "
    if (pdu[0] == 0x22 && pdu[pdulen-1] == 0x22) {
        pdu[pdulen -1] = '\0';
        pdu++;
        pdulen -= 2;
    }

    bytePdu = (unsigned char*)alloca(pdulen / 2);
    memset(bytePdu, 0, pdulen / 2);

    ConvertByte(pdu, bytePdu, pdulen / 2);

    ProcessCdmaIncomingSms(bytePdu, pdulen / 2, cdma_sms);

    RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_CDMA_NEW_SMS,
                cdma_sms, sizeof(RIL_CDMA_SMS_Message));
    free(dup);
}

static void requestGetSmsSimMemStatus(void *data, size_t datalen, RIL_Token t)
{
    int memStatu[2] = {0};
    if(!getSmsSimMemStatus(&memStatu[1], &memStatu[0]))
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, memStatu, sizeof(memStatu));
    }
    else
    {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

/*
 * !requesCdmatSMSAcknowledge is replaced by requestSendCNMA callback
 * !we can delete it anytimewa
 */
static void requesCdmatSMSAcknowledge(void *data, size_t datalen, RIL_Token t)
{
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/*
 * !requestSendCNMA is callback func using to replace requesCdmatSMSAcknowledge RIL Command
 *
 */
static void requestSendCNMA(void* param)
{
    /*
     * !beacause we send CNMA in unsoilicited thread
     * !the result of this command can not returned to framework
     * !so we IGNOR the result of this command
     *
     **/
    at_send_command("AT+CNMA", NULL, getChannelCtxbyId(MISC_CHANNEL));
}

/*
*function:set sms status from unread to read in uim card
*argument from framework:the index of sms
*return to framework: SUCCESS or GENERIC_FAILURE
*/
static void requestSetUimSmsRead(void *data, size_t datalen, RIL_Token t)
{
    int id2set = 0;
    char *cmd = NULL;
    int err = 0;

    id2set = ((int *)data)[0];
    LOGD("the sms's index is %d", id2set);

    asprintf(&cmd, "AT+CMSM=0,%d,1", id2set);
    err = at_send_command(cmd, NULL, SMS_CHANNEL_CTX);
    if(err < 0){
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSendCdmaSMS(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    const char* textPdu = ((const char**)data)[0];
    ATResponse* p_response = NULL;
    char* cmd = NULL;
    RIL_SMS_Response response;

    at_send_command("at+cmgf = 0", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CMGS=\"0\", \"%s\"", textPdu);
    /* Use a big enough value to make sure we can get the HCMGSS/HCMGSF after CMGS */
    err = at_send_command_with_specified_timeout(cmd,MAX_TIMEOUTSECS_TO_RECEIVE_HCMGSS,&p_response, SMS_CHANNEL_CTX);
    free(cmd);

    memset(&response, 0, sizeof(response));
    if (err < 0 || p_response->success == 0) goto error;

    char* dup = strdup(p_response->finalResponse);
    if (!dup)  goto error;

    char* respline = dup;
    if(strStartsWith(respline, "^HCMGSF:"))
    {
      at_tok_start(&respline);

      err = at_tok_nextint(&respline, &(response.errorCode));
      if (err < 0) {
        free(dup);
        goto error;
      }
      RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, &response, sizeof(response));
    }
    else
    {
      response.messageRef = atoi(respline + 8);
      LOGD("response.messageRef = %d\n",response.messageRef);
      RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    }

    free(dup);

    /* ignore the unsolicited msg of network change in the first 4 seconds */
    //s_wait_repoll_flag = 1;
    //RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL, getDefaultChannelCtx()->id);

    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void repollNetworkState(void *param)
{
    LOGD("In %s, submit unsol network state change", __func__);
    s_wait_repoll_flag = 0;
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, \
            NULL, 0);
}

static int isSimSmsFull(void)
{
    int totalMem = 0;
    int usedMem = -1;
    int ret = 0;
    if(!getSmsSimMemStatus(&totalMem, &usedMem))
    {
        if(usedMem >= totalMem)
        {
            LOGD("sim sms full");
            ret = 1;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static int getSmsSimMemStatus(int *totalMem, int *usedMem)
{
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_sim_response = NULL;
    char *line = NULL;
    char * sim_line = NULL;
    char *mem[3] = {NULL, NULL, NULL};
    int skip_int = 0;
    char * skip_str = NULL;
    int i = 0;
    char* cmd = NULL;
    int simStatusChanged = 0;

    if(NULL == usedMem || NULL == totalMem)
    {
        LOGD("param error!");
        return -1;
    }

    /*get initial info*/
    err = at_send_command_singleline ("AT+CPMS?", "+CPMS:", &p_response, getChannelCtxbyId(MISC_CHANNEL));
    if ((err < 0) || (p_response->success == 0))
    {
        err = -1;
        goto end;
    }
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto end;
    }
    for(i = 0; i< 3; i++)
    {
        err = at_tok_nextstr(&line, &mem[i]);
        if (err < 0)
        {
            goto end;
        }
        err = at_tok_nextint(&line, &skip_int);
        if (err < 0)
        {
            goto end;
        }
        err = at_tok_nextint(&line, &skip_int);
        if (err < 0)
        {
            goto end;
        }
    }

    /*get sim info*/
    at_send_command("AT+CPMS=\"SM\"", NULL, getChannelCtxbyId(MISC_CHANNEL));

    simStatusChanged = 1;

    err = at_send_command_singleline ("AT+CPMS?", "+CPMS:", &p_sim_response, getChannelCtxbyId(MISC_CHANNEL));
    if ((err < 0) || (p_sim_response->success == 0))
    {
        err = -1;
        goto end;
    }
    sim_line = p_sim_response->p_intermediates->line;
    err = at_tok_start(&sim_line);
    if (err < 0)
    {
        goto end;
    }
    err = at_tok_nextstr(&sim_line, &skip_str);
    if (err < 0)
    {
        goto end;
    }
    err = at_tok_nextint(&sim_line, &skip_int);
    if (err < 0)
    {
        goto end;
    }
    *usedMem = skip_int;
    err = at_tok_nextint(&sim_line, &skip_int);
    if (err < 0)
    {
        goto end;
    }
    *totalMem = skip_int;
    LOGD("uim used is %d, total is %d", *usedMem, *totalMem);

end:
    /*recover initial setting*/
    if(simStatusChanged)
    {
        asprintf(&cmd, "AT+CPMS=\"%s\",\"%s\",\"%s\"", mem[0], mem[1], mem[2]);
        at_send_command(cmd, NULL, getChannelCtxbyId(MISC_CHANNEL));
        free(cmd);
        cmd = NULL;
    }

    at_response_free(p_sim_response);
    at_response_free(p_response);
    return err;
}

static void requestQuerySmsAndPB(void *data, size_t datalen, RIL_Token t)
{
    int i;
    int err = 0;
    ATResponse *p_response = NULL;


    int response[2] = {0};
    int size = sizeof(response) / sizeof(response[0]);
    char *line = NULL;

    err = at_send_command_singleline("AT+VSPST?", "+VSPST:", &p_response, SMS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        LOGE("requestQuerySmsAndPB at_send_command_singleline err  = %d", err );
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        LOGE("requestQuerySmsAndPB at_tok_start err");
        goto error;
    }
    for (i = 0; i < size; i++) {
        err = at_tok_nextint(&line, &response[i]);
        if (err < 0) {
            LOGE("requestQuerySmsAndPB at_tok_nextint err i = %d", i);
            goto error;
        }
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("requestQuerySmsAndPB return error = %d", err);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestConvertRealPdu(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    const char* textPdu = ((const char**)data)[0];
    LOGD("textPdu = %s\n", textPdu);
    char* pduResultStr = NULL;

    char* pdu  = strdup(textPdu);
    int pdulen = strlen(pdu);
    RIL_CDMA_SMS_Message *cdma_sms = NULL;
    unsigned char* bytePdu = NULL;

    cdma_sms = (RIL_CDMA_SMS_Message *)malloc(sizeof(RIL_CDMA_SMS_Message));
    memset(cdma_sms, 0, sizeof(RIL_CDMA_SMS_Message));

    bytePdu = (unsigned char*)malloc((pdulen / 2)*sizeof(unsigned char));
    memset(bytePdu, 0, pdulen / 2);

    ConvertByte((unsigned char*)pdu, bytePdu, pdulen / 2);
    ProcessCdmaIncomingSms(bytePdu, pdulen / 2, cdma_sms);

    char* s_pdu = NULL;
    encodeUimPduFromRilSmsStruc((RIL_CDMA_SMS_Message* )cdma_sms, (char **)&s_pdu);

    if (s_pdu != NULL) {
        pduResultStr = s_pdu;
        RIL_onRequestComplete(t,RIL_E_SUCCESS, pduResultStr, sizeof(char *));
    } else {
        LOGE("null str");
        goto error;
    }

    if(pdu) { free(pdu); pdu = NULL; }
    if(s_pdu) { free(s_pdu); s_pdu = NULL; }
    if(bytePdu) { free(bytePdu); bytePdu = NULL; }
    if(cdma_sms) { free(cdma_sms); s_pdu = NULL; }

    return;

error:
    if(pdu) { free(pdu); pdu = NULL; }
    if(s_pdu) { free(s_pdu); s_pdu = NULL; }
    if(bytePdu) { free(bytePdu); bytePdu = NULL; }
    if(cdma_sms) { free(cdma_sms); s_pdu = NULL; }

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestReadDelNewSms(void* param)
{
    NewMsgStorageStruct *pNewMsgStorage = (NewMsgStorageStruct*)(param);
    char* cmd1 = NULL;
    char* cmd2 = NULL;
    char* cmd3 = NULL;

    if(NULL != pNewMsgStorage)
    {
      asprintf(&cmd1, "AT+CPMS=\"%s\"", pNewMsgStorage->sms_storeType);
      at_send_command(cmd1, NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(cmd1);

      asprintf(&cmd2, "AT+CMGR=%d", pNewMsgStorage->sms_index);
      at_send_command(cmd2, NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(cmd2);

      asprintf(&cmd3, "AT+CMGD=%d", pNewMsgStorage->sms_index);
      at_send_command(cmd3, NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(cmd3);

      at_send_command("AT+CPMS=\"ME\"", NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(pNewMsgStorage);
    }
}

static void requestReadSms(void* param)
{
    int index = *((int*)(param));
    char* cmd = NULL;

    if (index != -1)
    {
      asprintf(&cmd, "AT+CMGR=%d", index);

      /** DON'T care execution fails because this invocation fails just like no new sms unsolicited */
      at_send_command(cmd, NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(cmd);
    }
}

static void requestDelIndexedSms(void* param)
{
    char* cmd = NULL;
    int index = *((int*)(param));
    if (index != -1)
    {
      asprintf(&cmd, "AT+CMGD=%d", index);
      at_send_command(cmd, NULL, getChannelCtxbyId(MISC_CHANNEL));
      free(cmd);

      sms_index = -1;
    }

}

void encodeUimPdu(char *simResponse) {
    char *simresponse = simResponse;
    char status[3] = {0};
    char reslenstr[3] = {0};
    strncpy(status, simresponse, 2);
    simresponse+=2;
    strncpy(reslenstr, simresponse, 2);
    simresponse+=2;

    int iccpdulen = ConvertNum((unsigned char *)&reslenstr[1]) + ConvertNum((unsigned char *)&reslenstr[0])*16;
    if ((iccpdulen <= 0) || !(status[1] & 0x1))
    {
        LOGD("%s:  iccpdulen = %d, status is %s", __FUNCTION__, iccpdulen, status);
        return ;
    }

    char *iccpdu = (char *)malloc(((iccpdulen*2) + 1)*sizeof(char));
    memset(iccpdu, 0, iccpdulen*2 + 1);
    strncpy(iccpdu, simresponse, iccpdulen*2);

    char* pdu  = iccpdu;
    int pdulen = strlen(pdu);
    RIL_CDMA_SMS_Message *cdma_sms = NULL;
    unsigned char* bytePdu = NULL;

    cdma_sms = (RIL_CDMA_SMS_Message *)malloc(sizeof(RIL_CDMA_SMS_Message));
    memset(cdma_sms, 0, sizeof(RIL_CDMA_SMS_Message));

    bytePdu = (unsigned char*)malloc((pdulen / 2)*sizeof(unsigned char));
    memset(bytePdu, 0, pdulen / 2);

    ConvertByte((unsigned char *)pdu, bytePdu, pdulen / 2);
    ProcessCdmaIncomingSms(bytePdu, pdulen / 2, cdma_sms);

    char* s_pdu = NULL;
    encodeUimPduFromRilSmsStruc((RIL_CDMA_SMS_Message *)cdma_sms, (char **)&s_pdu);
    int newlen = strlen(s_pdu);
    sprintf(reslenstr,"%0x",newlen/2);
    memset(simResponse + 2, 'f', strlen(simResponse)-2);
    strncpy(simResponse+2, reslenstr, 2);
    strncpy(simResponse+4, s_pdu, newlen);

    if(s_pdu) { free(s_pdu); s_pdu = NULL; }
    if(iccpdu) { free(iccpdu); iccpdu = NULL; }
    if(bytePdu) { free(bytePdu); bytePdu = NULL; }
    if(cdma_sms) { free(cdma_sms); s_pdu = NULL; }
}

void encodeUimPduFromRilSmsStruc(RIL_CDMA_SMS_Message* SmsMsg, char** s_pdu)
{
    if ( SmsMsg == NULL )
        return ;

    unsigned char pdu[255] = {0};
    unsigned char length = 0;
    char strPdu[255*2 + 1] = {0};
    int i = 0;

    //{
    // messageType
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;

    // TeleserviceID
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint16(pdu, length*8, 16, SmsMsg->uTeleserviceID);
    length += 2;

    // servicePresent
/*  putuint8(pdu, length*8, 8, 0);  length += 1;
    putuint8(pdu, length*8, 8, 0);  length += 1;
    putuint8(pdu, length*8, 8, 0);  length += 1;
    putuint8(pdu, length*8, 8, SmsMsg->bIsServicePresent);  length += 1;
*/
    // uServicecategory
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uServicecategory)>>24); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uServicecategory)>>16); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uServicecategory)>>8); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uServicecategory)); length += 1;

    // digit_mode
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->sAddress.digit_mode)); length += 1;

    // number_mode
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->sAddress.number_mode)); length += 1;

    // number_type
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->sAddress.number_type)); length += 1;

    // number_plan
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->sAddress.number_plan)); length += 1;

    // number_of_digits
    //putuint8(pdu, length*8, 8, SmsMsg->sAddress.number_of_digits); length += 1;
    pdu[length] = SmsMsg->sAddress.number_of_digits;
    length++;
    // digits
    for(i=0; i < SmsMsg->sAddress.number_of_digits; i++){
        pdu[length] = SmsMsg->sAddress.digits[i];
        length++;
    }
    // ignore bearerReply
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    // ignore CauseCode values
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    putuint8(pdu, length*8, 8, 0); length += 1;
    /*
    // Subaddress
    // subaddressType
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->sSubAddress.subaddressType)); length += 1;

    //subaddr_odd
    //putuint8(pdu, length*8, 8, SmsMsg->sSubAddress.odd); length += 1;
    pdu[length] = SmsMsg->sSubAddress.odd;
    length++;

    // number_of_digits
    //putuint8(pdu, length*8, 8, SmsMsg->sSubAddress.number_of_digits); length += 1;
    pdu[length] = SmsMsg->sSubAddress.number_of_digits;
    length++;
    // digits
    for(i=0; i < SmsMsg->sSubAddress.number_of_digits; i++){
        pdu[length] = SmsMsg->sSubAddress.digits[i];
        length++;
    }
    */
    // uBearerDataLen
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uBearerDataLen)>>24); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uBearerDataLen)>>16); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uBearerDataLen)>>8); length += 1;
    putuint8(pdu, length*8, 8, (unsigned char)(SmsMsg->uBearerDataLen)); length += 1;
    for(i=0; i < SmsMsg->uBearerDataLen; i++){
        pdu[length] = SmsMsg->aBearerData[i];
        length++;
    }

    ConvertAscii(pdu, length, strPdu);

    //already done this job
    *s_pdu = (char*)malloc(length*2 + 1);
    memset(*s_pdu, 0, length + 1);
    strcpy(*s_pdu, strPdu);
    //}
}

static void requestSetPreferStorage(void *data, size_t datalen, RIL_Token t)
{
    int inSim = 0;
#if 0
    inSim = ((int *)data)[0];
    if (inSim == 0) {
        //at_send_command("AT+CNMI=3,2,1,1,0", NULL);
        setSmsStorage(0);
    } else if (inSim == 1) {
        //at_send_command("AT+CNMI=3,3,1,1,0", NULL);
        setSmsStorage(1);
    }
    smsStorageStatus = inSim;
#endif
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}
