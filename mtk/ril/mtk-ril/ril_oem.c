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
#include <cutils/properties.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>

#include <ril_callbacks.h>

#include "ril_nw.h"
#include "ril_sim.h"
#if !defined(PURE_AP_USE_EXTERNAL_MODEM)
#include "hardware/ccci_intf.h"
#endif

/// M: EPDG feature. Dispatch screen state to mal.
#include <mal.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern int isModemResetStarted;

extern int s_md_off;
extern int s_main_loop;
extern int m_msim_mode[RIL_SOCKET_NUM];

char* s_imei[RIL_SOCKET_NUM] = {0};
char* s_imeisv[RIL_SOCKET_NUM] = {0};
char* s_basebandVersion[RIL_SOCKET_NUM] = {0};
char* s_projectFlavor[RIL_SOCKET_NUM] = {0};
char* s_calData = 0;
int gcf_test_mode = 0;

int isNumericSet = 0;

// IMS VoLTE refactoring
DIAL_SOURCE dial_source = GSMPHONE;
extern dispatch_flg dispatch_flag;
extern volte_call_state call_state;

int recevSimSwitchReqCount = 0;
int targetSimRid = 0;
extern char s_logicalModemId[SIM_COUNT][MAX_UUID_LENGTH];

extern void IMS_RIL_onRequestComplete(RIL_Token t, RIL_Errno e,
                           void *response, size_t responselen);
extern bool IMS_isRilRequestFromIms(RIL_Token t);
extern int combineAttachAndFollowOn(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel);
extern int defineAttachApnIfIACacheExisted(RIL_SOCKET_ID rid, RILChannelCtx *pChannel);

//MTK-START [mtk80950][120410][ALPS00266631]check whether download calibration data or not
void requestGetCalibrationData(void * data, size_t datalen, RIL_Token t)
{
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (s_calData == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_calData, sizeof(char *));
}

void requestBasebandVersion(void * data, size_t datalen, RIL_Token t)
{
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (s_basebandVersion[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_basebandVersion[rid], sizeof(char *));
}

void requestSetMute(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;

    asprintf(&cmd, "AT+CMUT=%d", ((int *)data)[0]);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
    } else {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
    }
    at_response_free(p_response);
}

void requestGetMute(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    int response;
    char *line;

    err = at_send_command_singleline("AT+CMUT?", "+CMUT:", &p_response, OEM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &response);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));

    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

RIL_Errno resetRadio() {
    int err;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = NULL;
    int i =0;
    p_channel = getRILChannelCtx(RIL_SIM,getMainProtocolRid());
    int cur3GSim = queryMainProtocol();

    RLOGI("start to reset radio");

    // only do power off when it is on
    if (s_md_off != 1)
    {
        //power off modem
        if (isCCCIPowerOffModem()) {
            s_md_off = 1;
        }

        /* Reset the modem, we will do the following steps
         * 1. AT+EPOF,
         *      do network detach and power-off the SIM
         *      By this way, we can protect the SIM card
         * 2. AT+EPON
         *      do the normal procedure of boot up
         * 3. stop muxd
         *      because we will re-construct the MUX channels
         * 4. The responsibility of Telephony Framework
         *    i. stop rild
         *    ii. start muxd to re-construct the MUX channels and start rild
         *    iii. make sure that it is OK if there is any request in the request queue
         */
        if (SIM_COUNT >= 2) {
           isModemResetStarted = 1;
        }
        err = at_send_command("AT+EPOF", &p_response, p_channel);

        if (err != 0 || p_response->success == 0) {
            RLOGE("There is something wrong with the exectution of AT+EPOF");
        }
        at_response_free(p_response);

        if (isCCCIPowerOffModem()) {
            RLOGD("Flight mode power off modem, trigger CCCI power on modem");
            triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
        }
    }
    RLOGD("update 3G Sim property : %d", cur3GSim);
    setSimSwitchProp(cur3GSim);
    for(i=0; i<SIM_COUNT; i++){
        resetPhbReady(RIL_SOCKET_1+i);
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SETUP_MENU_RESET, NULL, 0, RIL_SOCKET_1+i);
    }

    //power on modem
    if (isCCCIPowerOffModem()) {
        RLOGD("Flight mode power on modem, trigger CCCI power on modem");
        triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
        if (SIM_COUNT >= 2) {
            isModemResetStarted = 0;
        }
    } else {
        err = at_send_command("AT+EPON", &p_response, p_channel);
        if (SIM_COUNT >= 2) {
            isModemResetStarted = 0;
        }

        if (err != 0 || p_response->success == 0) {
            LOGW("There is something wrong with the exectution of AT+EPON");
        }
        at_response_free(p_response);
    }
    s_main_loop = 0;
    return RIL_E_SUCCESS;
}

RIL_Errno resetRadioForSvlte() {
    if (SIM_COUNT >= 2) {
        int cur3GSim = queryMainProtocol();
        ATResponse *p_response = NULL;
        RILChannelCtx* p_channel = NULL;
        int err = -1;
        p_channel = getRILChannelCtx(RIL_SIM, getMainProtocolRid());

        RLOGI("start to reset radio - resetRadioForSvlte");
        // only do power off when it is on

        if (s_md_off != 1)
        {
            // power off modem
            if (isCCCIPowerOffModem()) {
                s_md_off = 1;
            }

            isModemResetStarted = 1;
            err = at_send_command("AT+EPOF", &p_response, p_channel);
            if (err != 0 || p_response->success == 0) {
                RLOGW("There is something wrong with the exectution of AT+EPOF");
        }
            at_response_free(p_response);
        }
        RLOGD("update 3G Sim property : %d", cur3GSim);
        setSimSwitchProp(cur3GSim);
        property_set("ril.mux.report.case", "2");
        property_set("ril.muxreport", "1");
        s_main_loop = 0;
    }
    return RIL_E_SUCCESS;
}

void requestResetRadio(void * data, size_t datalen, RIL_Token t)
{
    property_set("ril.getccci.response","0");
    RIL_Errno err = resetRadio();
    RIL_onRequestComplete(t, err, NULL, 0);
    property_set("ril.getccci.response","1");
}

// TODO: requestOemHookRaw
void requestOemHookRaw(void * data, size_t datalen, RIL_Token t)
{
    /* atci start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line;
    int i;
    int strLength = 0;
    int err = -1;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("data = %s, length = %d", buffer, datalen);

    if (strncmp(buffer, "ATV0", 4) == 0 || strncmp(buffer, "atv0", 4) == 0) {
        RLOGD("set isNumericSet on");
        isNumericSet = 1;
    } else if (strncmp(buffer, "ATV1", 4) == 0 || strncmp(buffer, "atv1", 4) == 0) {
        RLOGD("set isNumericSet off");
        isNumericSet = 0;
    }

#if defined(MTK_WORLD_PHONE) && defined(MTK_CMCC_WORLD_PHONE_TEST)
    //If EFUN=0 is before this point, then put it in a Queue, and post it at right time.
    extern int worldPhoneFlag;
    extern CfunParam cfunParamInQ;
    if(((strcmp(buffer, "AT+CFUN=0") == 0) || (strcmp(buffer, "at+cfun=0") == 0))
        && IS_SET_RIL_FLAG(worldPhoneFlag, FLAG_AT_EFUN_DONE))
    {
        RLOGW("[WorldPhoneATCI]cfun = 0 from ATCI and Q for later");
        SET_RIL_FLAG(worldPhoneFlag, FLAG_AT_CFUN_IN_Q);
        cfunParamInQ.token = t;
        strncpy(cfunParamInQ.cfun, data, datalen);
        return;
    }
#endif //MTK_WORLD_PHONE

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {
        RLOGE("OEM_HOOK_RAW fail");
        goto error;
    }

    RLOGD("p_response->success = %d", p_response->success);
    RLOGD("p_response->finalResponse = %s", p_response->finalResponse);

    strLength += 2; //for the pre tag of the first string in response.

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        RLOGD("p_response->p_intermediates = <%s>", p_cur->line);
        strLength += (strlen(p_cur->line) + 2); //M:To append \r\n
    }
    strLength += (strlen(p_response->finalResponse) + 2);
    RLOGD("strLength = %d", strLength);

    int size = strLength * sizeof(char) + 1;
    line = (char *) alloca(size);
    memset(line, 0, size);

    strcpy(line, "\r\n");

        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
       strcat(line, p_cur->line);
       strcat(line, "\r\n");
       RLOGD("line[%d] = <%s>", i, line);
    }
    strcat(line, p_response->finalResponse);
    strcat(line, "\r\n");
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, size);

     at_response_free(p_response);
     return;

error:
    line = (char *) alloca(10);
    memset(line, 0, 10);
    strcpy(line, "\r\nERROR\r\n");
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));

    at_response_free(p_response);
    return;
}

void requestAtCmdWithProxy(void * data, size_t datalen, RIL_Token t)
{
    /* sending AT command start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line = NULL;
    int i;
    int strLength = 0;
    int err = -1;

    RLOGD("AT_CMD_P: data = %s, length = %d", buffer, datalen);

    if (strncmp(buffer, "ATV0", 4) == 0 || strncmp(buffer, "atv0", 4) == 0) {
        RLOGD("set isNumericSet on");
        isNumericSet = 1;
    } else if (strncmp(buffer, "ATV1", 4) == 0 || strncmp(buffer, "atv1", 4) == 0) {
        RLOGD("set isNumericSet off");
        isNumericSet = 0;
    }

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {
        RLOGE("AT_CMD_P: AT_COMMAND_WITH_PROXY fail");
        goto error;
    }

    RLOGD("AT_CMD_P: p_response->success = %d", p_response->success);
    RLOGD("AT_CMD_P: p_response->finalResponse = %s", p_response->finalResponse);

    strLength += 2; //for the pre tag of the first string in response.

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        RLOGD("AT_CMD_P: p_response->p_intermediates = <%s>", p_cur->line);
        strLength += (strlen(p_cur->line) + 2);
    }
    strLength += (strlen(p_response->finalResponse)+ 4); //  \r\n + OK + \r\n
    RLOGD("AT_CMD_P: strLength = %d", strLength);

    int size = strLength + 1; //+1 for '\0'
    line = (char *) calloc(1, size);

    strcpy(line, "\r\n"); //for the pre tag of the first string in response.

    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next, i++)
    {
        strcat(line, p_cur->line);
        strcat(line, "\r\n");
        RLOGD("AT_CMD_P: line[%d] = <%s>", i, line);
    }
    strcat(line, "\r\n");
    strcat(line, p_response->finalResponse);

    strcat(line, "\r\n");
    RLOGD("AT_CMD_P: line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength);

    at_response_free(p_response);

    if (line) free(line);
    return;

error:
    line = (char *) calloc(1, 10);
    strcpy(line, "\r\nERROR\r\n");
    RLOGD("AT_CMD_P: line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));
    at_response_free(p_response);
    if (line) free(line);
    return;
}
void requestOemHookStrings(void * data, size_t datalen, RIL_Token t)
{
    int i;
    const char ** cur;
    ATResponse *    p_response = NULL;
    int             err = -1;
    ATLine*         p_cur = NULL;
    char**          line;
    int             strLength = datalen / sizeof(char *);
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RLOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long)datalen);

    for (i = strLength, cur = (const char **)data ;
         i > 0 ; cur++, i --) {
            RLOGD("> '%s'", *cur);
    }


    if (strLength != 2) {
        /* Non proietary. Loopback! */

        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
        }

        return;

    }

    /* For AT command access */
    cur = (const char **)data;

    if (strncmp(cur[0], "DIALSOURCE_IMS", 14) == 0) {
        RLOGD("IMS: DialSource is from IMS phone, requestOemHookStrings");
        dial_source = IMSPHONE;
        dispatch_flag = IMS;
        call_state = pending_mo;
    }

    if (NULL != cur[1] && strlen(cur[1]) != 0) {

        if ((strncmp(cur[1],"+CIMI",5) == 0) ||(strncmp(cur[1],"+CGSN",5) == 0)) {

            err = at_send_command_numeric(cur[0], &p_response, OEM_CHANNEL_CTX);

        } else {

        err = at_send_command_multiline(cur[0],cur[1], &p_response, OEM_CHANNEL_CTX);

        }

    } else {

        err = at_send_command(cur[0],&p_response,OEM_CHANNEL_CTX);

    }

    if (strncmp(cur[0],"AT+EPIN2",8) == 0) {
        SimPinCount retryCounts;
        RLOGW("OEM_HOOK_STRINGS: PIN operation detect");
        getPINretryCount(&retryCounts, t, rid);
    }

    if (err < 0 || NULL == p_response) {
            RLOGE("OEM_HOOK_STRINGS fail");
            goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN_REQUIRED:
        case CME_SIM_PUK_REQUIRED:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_SIM_PIN2_REQUIRED:
            ret = RIL_E_SIM_PIN2;
            break;
        case CME_SIM_PUK2_REQUIRED:
            ret = RIL_E_SIM_PUK2;
            break;
        case CME_DIAL_STRING_TOO_LONG:
            ret = RIL_E_DIAL_STRING_TOO_LONG;
            break;
        case CME_TEXT_STRING_TOO_LONG:
            ret = RIL_E_TEXT_STRING_TOO_LONG;
            break;
        case CME_MEMORY_FULL:
            ret = RIL_E_SIM_MEM_FULL;
            break;
        case CME_BT_SAP_UNDEFINED:
            ret = RIL_E_BT_SAP_UNDEFINED;
            break;
        case CME_BT_SAP_NOT_ACCESSIBLE:
            ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
            break;
        case CME_BT_SAP_CARD_REMOVED:
            ret = RIL_E_BT_SAP_CARD_REMOVED;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
    }

    if (ret != RIL_E_SUCCESS) {
        goto error;
    }

    if (strncmp(cur[0],"AT+ESSP",7) == 0) {
        RLOGI("%s , %s !",cur[0], cur[1]);
        if(strcmp(cur[1], "") == 0) {
            updateCFUQueryType(cur[0]);
        }
    }

    /* Count response length */
    strLength = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next)
        strLength++;

    if (strLength == 0) {
        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
    } else {

        RLOGI("%d of %s received!",strLength, cur[1]);

        line = (char **) alloca(strLength * sizeof(char *));

        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
        }

        if (IMS_isRilRequestFromIms(t)) {
            IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
        }
    }

    at_response_free(p_response);

    return;

error:
    if (IMS_isRilRequestFromIms(t)) {
        IMS_RIL_onRequestComplete(t, ret, NULL, 0);
    } else {
        RIL_onRequestComplete(t, ret, NULL, 0);
    }

    at_response_free(p_response);

}

void updateCFUQueryType(const char *cmd)
{
    int fd;
    int n;
    struct env_ioctl en_ctl;
    char *name = NULL;
    char *value = NULL;

    do {
        value = calloc(1, BUF_MAX_LEN);
        if(value == NULL) {
            RLOGE("malloc value fail");
            break;
        }
        value[0] = *(cmd+8);

        property_set(SETTING_QUERY_CFU_TYPE, value);

        name = calloc(1, BUF_MAX_LEN);
        if(name == NULL) {
            RLOGE("malloc name fail");
            free(value);
            break;
        }

        memset(&en_ctl,0x00,sizeof(struct env_ioctl));

        fd= open("/proc/lk_env",O_RDWR);

        if(fd < 0) {
            RLOGE("ERROR open fail %d\n", fd);
            free(name);
            free(value);
            break;
        }

        if (BUF_MAX_LEN > strlen(SETTING_QUERY_CFU_TYPE)) {   // Coverity
            strncpy(name,SETTING_QUERY_CFU_TYPE, strlen(SETTING_QUERY_CFU_TYPE));
            name[strlen(SETTING_QUERY_CFU_TYPE)] = 0;
        } else {
            strncpy(name,SETTING_QUERY_CFU_TYPE, BUF_MAX_LEN - 1);
            name[BUF_MAX_LEN - 1] = 0;
        }

        en_ctl.name = name;
        en_ctl.value = value;
        en_ctl.name_len = strlen(name)+1;

        en_ctl.value_len = strlen(value)+1;
        LOGD("write %s = %s\n",name,value);
        n=ioctl(fd,ENV_WRITE,&en_ctl);
        if(n<0) {
            printf("ERROR write fail %d\n",n);
        }
        free(name);
        free(value);
        close(fd);
    }while(0);
}

void initCFUQueryType()
{
    int fd;
    int n;
    struct env_ioctl en_ctl;
    char *name = NULL;
    char *value = NULL;

    do {
        memset(&en_ctl,0x00,sizeof(struct env_ioctl));

        fd= open("/proc/lk_env",O_RDONLY);

        if(fd < 0) {
            RLOGE("ERROR open fail %d\n", fd);
            break;
        }

        name = calloc(1, BUF_MAX_LEN);
        if(name == NULL) {
            RLOGE("malloc name fail");
            close(fd);
            break;
        }
        value = calloc(1, BUF_MAX_LEN);
        if(value == NULL) {
            RLOGE("malloc value fail");
            free(name);
            close(fd);
            break;
        }
        memcpy(name,SETTING_QUERY_CFU_TYPE, strlen(SETTING_QUERY_CFU_TYPE));

        en_ctl.name = name;
        en_ctl.value = value;
        en_ctl.name_len = strlen(name)+1;
        en_ctl.value_len = BUF_MAX_LEN;

        RLOGD("read %s \n",name);

        n=ioctl(fd,ENV_READ,&en_ctl);
        if(n<0){
            RLOGE("ERROR read fail %d\n",n);
        }
        else {
            property_set(name, value);
        }

        free(name);
        free(value);
        close(fd);
    }while(0);
}

void updateSignalStrength(RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    //MTK-START [ALPS00506562][ALPS00516994]
    //int response[12]={0};
    int response[16]={0};
    //MTK-START [ALPS00506562][ALPS00516994]

    char *line;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, OEM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    //err = getSingnalStrength(line, &response[0], &response[1], &response[2], &response[3], &response[4]);
    err = getSingnalStrength(line, response);

    if (err < 0) goto error;

    if (99 == response[0])
        LOGD("Ignore rssi 99(unknown)");
    else
    RIL_onUnsolicitedResponseSocket (RIL_UNSOL_SIGNAL_STRENGTH,
                                 response,
                                   sizeof(response),
                                   getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));

    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);
    RLOGE("updateSignalStrength ERROR: %d", err);
}

void requestScreenState(void * data, size_t datalen, RIL_Token t)
{
    /************************************
    * Disable the URC: ECSQ,CREG,CGREG,CTZV
    * For the URC +CREG and +CGREG
    * we will buffer the URCs when the screen is off
    * and issues URCs when the screen is on
    * So we can reflect the ture status when screen is on
    *************************************/

    int on_off, err;
    ATResponse *p_response = NULL;
    const int long_format = 1;
    const int short_format = 0;

    on_off = ((int*)data)[0];

    if (on_off)
    {
        // screen is on

         /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=3", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0){
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CREG=2");
        }
        at_response_free(p_response);
        p_response = NULL;

        if (isImsSupport()) {
            err = at_send_command("AT+CIREG=2", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CIREG=2");
            at_response_free(p_response);
            p_response = NULL;
        }

        /// M: Epdg feature, long format for screen on
        if (isEpdgSupport()) {
            int res = mal_set_ps_format(long_format);
            RLOGI("Screen on, set long format to mal. result: %d", res);
        } else {
            err = at_send_command("AT+CGREG=3", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0) {
                at_response_free(p_response);
                p_response = NULL;
                err = at_send_command("AT+CGREG=2", &p_response, OEM_CHANNEL_CTX);
                if (err != 0 || p_response->success == 0)
                    LOGW("There is something wrong with the exectution of AT+CGREG=2");
            }
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Enable get ECSQ URC */
        err = at_send_command("AT+ECSQ=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=2");
        at_response_free(p_response);
        p_response = NULL;
        updateSignalStrength(t);

        /* Enable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=1");
        at_response_free(p_response);
        p_response = NULL;

        /// M: Epdg feature, dispatch to mal
        if (!isEpdgSupport()) {
            err = at_send_command("AT+CEREG=3", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0) {
                at_response_free(p_response);
                p_response = NULL;
                err = at_send_command("AT+CEREG=2", &p_response, OEM_CHANNEL_CTX);
                if (err != 0 || p_response->success == 0)
                    LOGW("There is something wrong with the exectution of AT+CEREG=2");
           }
           at_response_free(p_response);
           p_response = NULL;
        }

        /* Enable ECSG URC */
    #ifdef MTK_FEMTO_CELL_SUPPORT
        at_send_command("AT+ECSG=4,1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSG=4,1");
        at_response_free(p_response);
        p_response = NULL;
    #endif
    }
    else
    {
        // screen is off

        /* Disable Network registration events of the changes in LAC or CID */
        err = at_send_command("AT+CREG=1", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+CREG=1");
        at_response_free(p_response);
        p_response = NULL;

        if (isImsSupport()) {
            // For IOT test : send AT+CIREG=0 only when there is no test sim
            int i = 0;
            int is_testsim = 0;
            char test_sim[PROPERTY_VALUE_MAX] = {0};
            for (i=0; i<4; i++) {
                property_get(PROPERTY_RIL_TEST_SIM[i], test_sim, "0");
                is_testsim = atoi(test_sim);
                if (is_testsim == 1) {
                    RLOGD("Has a test sim when screen off, did not turn off CIREG for IOT test.");
                    break;
                }
            }

            if (is_testsim == 0) {
                err = at_send_command("AT+CIREG=0", &p_response, OEM_CHANNEL_CTX);
                if (err != 0 || p_response->success == 0)
                    LOGW("There is something wrong with the exectution of AT+CIREG=0");
                at_response_free(p_response);
                p_response = NULL;
            }
        }
        /// M: Epdg feature, short format for screen off
        if (isEpdgSupport()) {
            int res = mal_set_ps_format(short_format);
            RLOGI("Screen off, set short format to mal. result: %d", res);
        } else {
            err = at_send_command("AT+CGREG=1", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=1");
            at_response_free(p_response);
            p_response = NULL;
        }

        /* Disable get ECSQ URC */
        err = at_send_command("AT+ECSQ=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSQ=0");
        at_response_free(p_response);
        p_response = NULL;

        /* Disable PSBEARER URC */
        err = at_send_command("AT+PSBEARER=0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+PSBEARER=0");
        at_response_free(p_response);
        p_response = NULL;

        if (!isEpdgSupport()) {
            err = at_send_command("AT+CEREG=1", &p_response, OEM_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=1");
            at_response_free(p_response);
            p_response = NULL;
        }

    #ifdef MTK_FEMTO_CELL_SUPPORT
        /* Disable ECSG URC */
        at_send_command("AT+ECSG=4,0", &p_response, OEM_CHANNEL_CTX);
        if (err != 0 || p_response->success == 0)
            LOGW("There is something wrong with the exectution of AT+ECSG=4,0");
        at_response_free(p_response);
        p_response = NULL;
    #endif
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

extern int queryMainProtocol()
{
    ATResponse *p_response = NULL;
    int err;
    int response;
    char *line;

    if (SIM_COUNT == 1) {
        /* Only Gemini modem support AT+ES3G */
        return CAPABILITY_3G_SIM1;
    } else {
        err = at_send_command_singleline("AT+ES3G?", "+ES3G:", &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));

        if (err < 0 || p_response->success == 0) {
            goto error;
        }

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &response);
        if (err < 0) goto error;

        /* Gemini+ , +ES3G response 1: SIM1 , 2: SIM2 , 4:SIM3 ,8: SIM4. For SIM3 and SIM4 we convert to 3 and 4 */
        if(response == 4){
            response = CAPABILITY_3G_SIM3;
        }else if(response == 8){
            response = CAPABILITY_3G_SIM4;
        }

        at_response_free(p_response);

        return response;

    error:
        at_response_free(p_response);
        return CAPABILITY_3G_SIM1;
    }
}

// TBD, remove
extern void set3GCapability(RIL_Token t, int setting)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    asprintf(&cmd, "AT+ES3G=%d, %d", setting, NETWORK_MODE_WCDMA_PREF);
    if (t) {
        err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    } else {
        RLOGI("The ril token is null, use URC instead");
        err = at_send_command(cmd, &p_response, getChannelCtxbyProxy(getMainProtocolRid()));
    }

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RLOGI("Set 3G capability to [%d] failed", setting);
    } else {
        RLOGI("Set 3G capability to [%d] successfully", setting);
    }
    at_response_free(p_response);
}

void initRadioCapabilityResponse(RIL_RadioCapability* rc, RIL_RadioCapability* copyFromRC) {
    memset(rc, 0, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = copyFromRC->session;
    rc->phase = copyFromRC->phase;
    rc->rat = copyFromRC->rat;
    strcpy(rc->logicalModemUuid, copyFromRC->logicalModemUuid);
    rc->status = copyFromRC->status;
}

extern void requestGetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int sim3G = RIL_get3GSIM();
    RLOGI("3G Sim : %d, current RID : %d", sim3G, rid);

    RIL_RadioCapability* rc = (RIL_RadioCapability*) calloc(1, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = 0;
    rc->phase = RC_PHASE_CONFIGURED;
    rc->rat = RAF_GSM;
    rc->status = RC_STATUS_NONE;

    RLOGI("requestGetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
            rc->version, rc->session, rc->phase, rc->rat, rc->logicalModemUuid, rc->status, rid);

    if (SIM_COUNT == 1) {
        /* Only Gemini modem support AT+ES3G */
        if (isLteSupport()) {
            rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
        } else {
            rc->rat = RAF_GSM | RAF_UMTS;
        }
        strcpy(rc->logicalModemUuid, s_logicalModemId[rid]);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(RIL_RadioCapability));
    } else {
        if (rid == (sim3G-1)) {
            if (isLteSupport()) {
                rc->rat = RAF_GSM | RAF_UMTS | RAF_LTE;
            } else {
                rc->rat = RAF_GSM | RAF_UMTS;
            }
            strcpy(rc->logicalModemUuid, s_logicalModemId[0]);
        } else {
            rc->rat = RAF_GSM;
            if (rid == 0) {
                strcpy(rc->logicalModemUuid, s_logicalModemId[(sim3G-1)]);
            } else {
                strcpy(rc->logicalModemUuid, s_logicalModemId[rid]);
            }
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(RIL_RadioCapability));
    }
}

extern ApplyRadioCapabilityResult applyRadioCapability(RIL_RadioCapability* rc, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int old3GSim = queryMainProtocol();
    RLOGI("applyRadioCapability : %d, %d, %d, %d", rc->rat, old3GSim, rid, recevSimSwitchReqCount);

    recevSimSwitchReqCount++;
    if (isLteSupport()) {
        if ((rc->rat & RAF_LTE) == RAF_LTE) {
            targetSimRid = rid;
        }
    } else {
        if ((rc->rat & RAF_UMTS) == RAF_UMTS) {
            targetSimRid = rid;
        }
    }
    if (recevSimSwitchReqCount != SIM_COUNT) {
        RLOGI("only handle last request for sim switch");
        return ApplyRC_NONEED;
    }
    /* Gemini+ , +ES3G response 1: SIM1 , 2: SIM2 , 4:SIM3 ,8: SIM4.  */
    int modem_setting_value = CAPABILITY_3G_SIM1 << (targetSimRid);
    if (isLteSupport()) {
        asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_GSM_UMTS_LTE);
    } else {
        asprintf(&cmd, "AT+ES3G=%d, %d", modem_setting_value, NETWORK_MODE_WCDMA_PREF);
    }

    err = at_send_command(cmd, &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        at_response_free(p_response);
        return APPLYRC_FAIL;
    }
    at_response_free(p_response);
    return ApplyRC_SUCCESS;
}

extern void requestSetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    ApplyRadioCapabilityResult applyRcResult = ApplyRC_NONEED;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    char sRcSessionId[32] = {0};
    RIL_RadioCapability rc;

    memcpy(&rc, data, sizeof(RIL_RadioCapability));
    RLOGI("requestSetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
        rc.version, rc.session, rc.phase, rc.rat, rc.logicalModemUuid, rc.status, rid);

    memset(sRcSessionId, 0, sizeof(sRcSessionId));
    sprintf(sRcSessionId,"%d",rc.session);


    RIL_RadioCapability* responseRc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
    initRadioCapabilityResponse(responseRc, &rc);
    int sim3G = RIL_get3GSIM();
    if (SIM_COUNT == 1) {
        strcpy(responseRc->logicalModemUuid, s_logicalModemId[rid]);
    } else {
        if (rid == (sim3G-1)) {
            strcpy(responseRc->logicalModemUuid, s_logicalModemId[0]);
        } else {
            if (rid == 0) {
                strcpy(responseRc->logicalModemUuid, s_logicalModemId[(sim3G-1)]);
            } else {
                strcpy(responseRc->logicalModemUuid, s_logicalModemId[rid]);
            }
        }
    }

    switch (rc.phase) {
        case RC_PHASE_START:
            RLOGI("requestSetRadioCapability RC_PHASE_START");
            //property_set(PROPERTY_SET_RC_SESSION_ID[0], sRcSessionId);
            // keep session id with system property
            // after modem reset, send session id back to framework with urc
            property_set(PROPERTY_SET_RC_SESSION_ID[0], sRcSessionId);
            // init recevSimSwitchReqCount
            recevSimSwitchReqCount = 0;
            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;
        case RC_PHASE_FINISH:
            RLOGI("requestSetRadioCapability RC_PHASE_FINISH");

            // transaction of sim switch is done, reset system property
            property_set(PROPERTY_SET_RC_SESSION_ID[0], "-1");
            responseRc->phase = RC_PHASE_CONFIGURED;
            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;
        case RC_PHASE_APPLY:
            responseRc->status = RC_STATUS_SUCCESS;
            applyRcResult = applyRadioCapability(responseRc, t);

            // send request back to socket before reset radio,
            // or the response may be lost due to modem reset

            RLOGI("requestSetRadioCapability applyRcResult:%d, s_md_off:%d",applyRcResult, s_md_off);
            switch (applyRcResult) {
                case ApplyRC_SUCCESS:
                case ApplyRC_NONEED:
                    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
                    break;
                case APPLYRC_FAIL:
                    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, responseRc, sizeof(RIL_RadioCapability));
                    break;
            }

            if (applyRcResult == ApplyRC_SUCCESS) {
                char tmp[PROPERTY_VALUE_MAX] = { 0 };
                RLOGI("requestSetRadioCapability resetRadio");
                property_set("gsm.ril.eboot", "1");
                // remember main protocol ICCID
                int cur3GSim = queryMainProtocol();
                property_get(PROPERTY_ICCID_SIM[cur3GSim-1], tmp, "");
                if (strlen(tmp) > 0) {
                    property_set("persist.radio.simswitch.iccid", tmp);
                    property_set("persist.radio.capability.iccid", tmp);
                    RLOGI("write simswitch iccid : %s", tmp);
                }
                if (isCdmaLteDcSupport()) {
                    resetRadioForSvlte();
                } else {
                    resetRadio();
                }
            }
            break;
        default:
            RLOGI("requestSetRadioCapability default");
            responseRc->status = RC_STATUS_FAIL;
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            break;
    }
    free(responseRc);
}

extern int getSimSwitchMode(int ccci_sys_fd) {
    int val_to_ret = 0;

    int open_fd_locally = 0;
    if (ccci_sys_fd == 0) {
        open_fd_locally = 1;

    //get ccci sys fd by ccci interface
    #ifdef MTK_RIL_MD2
        char dev_node[32] = {0};
        #if MTK_ENABLE_MD5
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS5));
        LOGD("MD1/SYS5 IOCTL [%s]", dev_node);
        #else
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS2));
        LOGD("MD2 IOCTL [%s]", dev_node);
        #endif
        ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    #else
        char dev_node[32] = {0};
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS1));
        LOGD("MD1/SYS1 IOCTL [%s]", dev_node);
        ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    #endif
    }

    if (ccci_sys_fd < 0) {
        LOGD("open CCCI ioctl port failed [%d, %d]", ccci_sys_fd, errno);
        return -1;
    }
    unsigned int simMode = 0; // set to 0 (a invalid number). real mode range: 1~4.
    int ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_GET_SIM_MODE, &simMode);
    if (ret_ioctl_val<0)
    {
        LOGD("CCCI ioctl result: ret_val=%d, request=%d, [err: %s]", ret_ioctl_val, CCCI_IOC_GET_SIM_MODE, strerror(errno));
    }
    else
    {
        LOGD("CCCI ioctl result: ret_val=%d, request=%d, simMode=%d", ret_ioctl_val, CCCI_IOC_GET_SIM_MODE, simMode);
        val_to_ret = simMode & (0xffff);
    }

    char* mode;
    asprintf(&mode, "%d", val_to_ret);
    property_set(PROPERTY_SIM_SWITCH_MODE, mode);
    free(mode);
    if ((getExternalModemSlot() >= 0)) {// only dsda with external modem need to set
        if (SIM_SWITCH_MODE_DUAL_TALK_SWAP == val_to_ret){
            setExternalModemSlot(1);
        }
        else {
            setExternalModemSlot(2);
        }
    }
    if (open_fd_locally==1)
        close(ccci_sys_fd); // remember to close it if it is opened locally.
    return val_to_ret;
}

void requestSN(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *sv;
    // type 5: Serial Number

    err = at_send_command_singleline("AT+EGMR=0,5" , "+EGMR:",&p_response, getChannelCtxbyProxy(rid));
    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    err = at_tok_nextstr(&line, &sv);
    if(err < 0) goto error;

    property_set(PROPERTY_SERIAL_NUMBER, sv);
    LOGD("[RIL%d] Get serial number: %s", rid+1, sv);

    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
}


void requestGetImei(void * data, size_t datalen, RIL_Token t)
{
    /*RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (s_imei[rid] == NULL) {
        ATResponse *p_response = NULL;
        int err;

        err = at_send_command_numeric("AT+CGSN", &p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0) {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            err = asprintf(s_imei[rid], "%s", p_response->p_intermediates->line);
            if(err >= 0)
                RIL_onRequestComplete(t, RIL_E_SUCCESS, p_response->p_intermediates->line, sizeof(char *));
        }
        at_response_free(p_response);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imei[rid], sizeof(char *));
    }*/

    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    LOGD("Get imei (%d)", rid);
    if (s_imei[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imei[rid], sizeof(char *));

}

void requestGetImeisv(void * data, size_t datalen, RIL_Token t)
{
    /*ATResponse *p_response = NULL;
    int err;
    char *line, *sv;

    err = at_send_command_singleline("AT+EGMR=0,9", "+EGMR:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextstr(&line, &sv);
        if(err < 0) goto error;

        RIL_onRequestComplete(t, RIL_E_SUCCESS,
                              sv, sizeof(char *));
    }
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);*/

    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    LOGD("Get imeisv (%d)", rid);
    if (s_imeisv[rid] == NULL)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, s_imeisv[rid], sizeof(char *));
}

extern void requestConfigSimSwitch(void * data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    int isSwapped = ((int *)data)[0];

    int ccci_sys_fd = -1;
    //get ccci sys fd by ccci interface
#ifdef MTK_RIL_MD2
    char dev_node[32] = {0};
    #if MTK_ENABLE_MD5
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS5));
    LOGD("MD1/SYS5 IOCTL [%s]", dev_node);
    #else
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS2));
    LOGD("MD2 IOCTL [%s]", dev_node);
    ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    #endif
#else
    char dev_node[32] = {0};
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS1));
    LOGD("MD1/SYS1 IOCTL [%s]", dev_node);
    ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
#endif
    if (ccci_sys_fd < 0) {
        LOGD("Open CCCI ioctl port failed [%d], [err: %s]", ccci_sys_fd, strerror(errno));
        return;
    }

    // get sim switch type
    unsigned int switchType = SIM_SWITCH_TYPE_INTERNAL; // set default internal type.
    int ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_SIM_SWITCH_TYPE, &switchType);
    if (ret_ioctl_val<0)
    {
        LOGD("CCCI ioctl result: ret_val=%d, request=%d, [err: %s]", ret_ioctl_val, CCCI_IOC_SIM_SWITCH_TYPE, strerror(errno));
    }
    else
    {
        LOGD("CCCI ioctl result: ret_val=%d, request=%d, switchType=%d", ret_ioctl_val, CCCI_IOC_SIM_SWITCH_TYPE, switchType);
    }

    // set sim mode & store sim mode
    unsigned int switchMode = isSwapped ? SIM_SWITCH_MODE_DUAL_TALK_SWAP : SIM_SWITCH_MODE_DUAL_TALK;
    switchMode = switchMode | (switchType << 16);
    ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_SIM_SWITCH, &switchMode);
    if (ret_ioctl_val == -1) {
        LOGE("Config SIM Switch failed [err: %s]", strerror(errno));
        err = 1;
    }

    if (!err) {
        ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_STORE_SIM_MODE, &switchMode);
        LOGD("CCCI ioctl result: ret_val=%d, request=%d, switchMode=%d (store sim mode)", ret_ioctl_val, CCCI_IOC_STORE_SIM_MODE, switchMode);
        if (ret_ioctl_val == -1) {
            LOGE("Store SIM Switch mode failed [err: %s]", strerror(errno));
            err = 1;
        } else {
            getSimSwitchMode(ccci_sys_fd); //query SIM Switch mode & update system property
        }
    }

    close(ccci_sys_fd);

    if (err)
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

// add for dsda 4G switch
extern void setCardSlot(int cardSlot) {
    if (isDualTalkMode() && (getExternalModemSlot() >= 0)){
        LOGI("setCardSlot, cardSlot = %d", cardSlot);
        char *cmd;
        ATResponse *p_response = NULL;
        int err = 0;
        asprintf(&cmd, "AT+CSUS=%d", cardSlot);
        err = at_send_command(cmd, &p_response, getChannelCtxbyProxy(RIL_SOCKET_1));
        free(cmd);
        if (err != 0 || p_response->success == 0) {
            LOGW("Set Card Slot:[%d] failed", cardSlot);
        }
        at_response_free(p_response);
        return;
    }
}

extern int getCardSlot() {
    if (isDualTalkMode() && (getExternalModemSlot() >= 0)){
        ATResponse *p_response = NULL;
        int err;
        char *line;
        int response = 1;
        err = at_send_command_singleline("AT+CSUS?", "+CSUS:", &p_response, getChannelCtxbyProxy(RIL_SOCKET_1));

        if (err < 0 || p_response->success == 0) goto error;

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &response);
        if (err < 0) goto error;

        at_response_free(p_response);
        return response;
    error:
        at_response_free(p_response);
        LOGW("get Card Slot failed");
        return -1;
    }
    return -1;
}

extern void setExternalModemSlot(int externalModemSlot)
{
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    sprintf(prop_val, "%d", externalModemSlot);
    property_set("ril.external.md", prop_val);
    LOGD("Update property ril.external.md=%s", prop_val);
}

extern int isSimSwitchSwapped() {
    int  switchMode = SIM_SWITCH_MODE_DUAL_TALK;
    char prop_val[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_SIM_SWITCH_MODE, prop_val, "3");
    switchMode = atoi(prop_val);
    LOGD("get prop ril.simswitch.mode = %d", switchMode);
    if (SIM_SWITCH_MODE_DUAL_TALK_SWAP == switchMode){
        return 1;
    }
    return 0;
}

extern void flightModeBoot()
{
    ATResponse *p_response = NULL;
    RLOGI("Start flight modem boot up");

#ifndef MTK_DISABLE_SW_SIM_SWITCH
    if (isSimSwitchSwapped()){
        #ifdef MTK_RIL_MD2
            setCardSlot(GEMINI_SIM_1);
        #else
            setCardSlot(GEMINI_SIM_2);
        #endif
    }
    else {
        #ifdef MTK_RIL_MD2
            setCardSlot(GEMINI_SIM_2);
        #else
            setCardSlot(GEMINI_SIM_1);
        #endif
    }
#endif

    int err;
    if (SIM_COUNT >= 2) {
        err = at_send_command("AT+EFUN=0", &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
#if defined(MTK_WORLD_PHONE) && defined(MTK_CMCC_WORLD_PHONE_TEST)
        //remember EFUN=0 is done.
        extern void clearWorldPhoneFlagCallback(void* param);
        extern int worldPhoneFlag;

        SET_RIL_FLAG(worldPhoneFlag, FLAG_AT_EFUN_DONE);
        static const struct timeval clearFlagTimer = {3, 0};
        RIL_requestProxyTimedCallback(clearWorldPhoneFlagCallback, NULL, &clearFlagTimer
            , getChannelCtxbyProxy(RIL_SOCKET_1)->id, "clearWorldPhoneFlagCallback");
        LOGW("[WorldPhoneATCI]flightModeBoot for set flag when EFUN=0 done. flag: %d", worldPhoneFlag);
#endif //MTK_WORLD_PHONE
    } else {
        err = at_send_command("AT+CFUN=4", &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
    }
    if (err != 0 || p_response->success == 0)
        LOGW("Start flight modem boot up failed");
    at_response_free(p_response);

    int telephonyMode = getTelephonyMode();
    switch (telephonyMode) {
        case TELEPHONY_MODE_6_TGNG_DUALTALK:
#ifdef MTK_RIL_MD1
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
#endif
            break;
        case TELEPHONY_MODE_7_WGNG_DUALTALK:
#ifdef MTK_RIL_MD2
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
#endif
            break;
        case TELEPHONY_MODE_8_GNG_DUALTALK:
            err = at_send_command("AT+ERAT=0", &p_response, getChannelCtxbyProxy(RIL_SOCKET_1));
            if (err != 0 || p_response->success == 0)
                LOGW("Set default RAT mode failed");
            at_response_free(p_response);
            break;
    }
}

extern int getMappingSIMByCurrentMode(RIL_SOCKET_ID rid) {

    RLOGI("getMappingSIMByCurrentMode[isDualTalkMode, getFirstModem, getTelephonyMode, getExternalModemSlot] [%d, %d, %d, %d]", isDualTalkMode(), getFirstModem(), getTelephonyMode(), getExternalModemSlot());
    if (isDualTalkMode()) {
        int firstMD = getFirstModem();
        if (firstMD == FIRST_MODEM_NOT_SPECIFIED) {
            //no first MD concept, use external MD mechanism
            int telephonyMode = getTelephonyMode();
            switch (telephonyMode) {
                case TELEPHONY_MODE_100_TDNC_DUALTALK:
                case TELEPHONY_MODE_101_FDNC_DUALTALK:
                    if (rid == RIL_SOCKET_2) {
                        //for international roaming, the socket2 is connected with framework
                        if (getExternalModemSlot() == GEMINI_SIM_1)
                            return GEMINI_SIM_2;
                        else
                            return GEMINI_SIM_1;
                    } else {
                        LOGW("Get mapping SIM but no match case[a]");
                    }
                    break;
                default:
                    //TELEPHONY_MODE_0_NONE, TELEPHONY_MODE_102_WNC_DUALTALK, TELEPHONY_MODE_103_TNC_DUALTALK
                    if (rid == RIL_SOCKET_1
                        ||( rid == RIL_SOCKET_2 && isEVDODTSupport())
                  ) {
                        if (isCdmaLteDcSupport()) {
                            // SVLTE socket 1 should get SIM1 info
                        LOGW("getMappingSIMByCurrentMode rid: %d", rid);
                            if (rid == RIL_SOCKET_1) {
                                return GEMINI_SIM_1;
                        } else if (rid == RIL_SOCKET_2) {
                            return GEMINI_SIM_2;
                            }
                        }
                        if (getExternalModemSlot() == GEMINI_SIM_2){
                            #ifdef MTK_RIL_MD1
                                return GEMINI_SIM_1;
                            #else
                                return GEMINI_SIM_2;
                            #endif
                        } else {
                            #ifdef MTK_RIL_MD1
                                return GEMINI_SIM_2;
                            #else
                                return GEMINI_SIM_1;
                            #endif
                         }
                    } else {
                        LOGW("Get mapping SIM but no match case[b]");
                    }
                    break;
            }
        } else if (firstMD == FIRST_MODEM_MD1) {
            #ifdef MTK_RIL_MD1
                return GEMINI_SIM_1;
            #else
                return GEMINI_SIM_2;
            #endif
        } else {
            #ifdef MTK_RIL_MD1
                return GEMINI_SIM_2;
            #else
                return GEMINI_SIM_1;
            #endif
        }
    } else {
        return GEMINI_SIM_1+rid;
    }
    return -1;
}

extern void upadteSystemPropertyByCurrentMode(int rid, char* key1, char* key2, char* value) {
    if (isDualTalkMode()) {
        int firstMD = getFirstModem();

        if (firstMD == FIRST_MODEM_NOT_SPECIFIED) {
            //no first MD concept, use external MD mechanism
            int telephonyMode = getTelephonyMode();
            switch (telephonyMode) {
                case TELEPHONY_MODE_100_TDNC_DUALTALK:
                case TELEPHONY_MODE_101_FDNC_DUALTALK:
                    if (rid == RIL_SOCKET_2) {
                        //for international roaming, the socket2 is connected with framework
                        if (getExternalModemSlot() == GEMINI_SIM_1) {
                            RLOGI("Update property SIM2 (exMD) [%s, %s]", key2, value != NULL ? value : "");
                            property_set(key2, value);
                        } else {
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        }
                    } else {
                        LOGW("Update property but no match case[a]");
                    }
                    break;
                default:
                    //TELEPHONY_MODE_0_NONE, TELEPHONY_MODE_102_WNC_DUALTALK, TELEPHONY_MODE_103_TNC_DUALTALK
                    if (rid == RIL_SOCKET_1
						|| ((rid == RIL_SOCKET_2) && isEVDODTSupport())
						) {
                        if (getExternalModemSlot() == GEMINI_SIM_1) {
                        #ifdef MTK_RIL_MD1
                                RLOGI("Update property SIM2 (exMD)[%s, %s]", key2, value != NULL ? value : "");
                                property_set(key2, value);
                        #else
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        #endif
                        } else {
                        #ifdef MTK_RIL_MD1
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        #else
                            RLOGI("Update property SIM2 (exMD)[%s, %s]", key2, value != NULL ? value : "");
                            property_set(key2, value);
                        #endif
                        }
                    } else {
                        LOGW("Update property but no match case[b]");
                    }
                    break;
            }
        } else if (firstMD == FIRST_MODEM_MD1) {
            #ifdef MTK_RIL_MD1
                RLOGI("Update property SIM1 (dt)[%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #else
                RLOGI("Update property SIM2 (dt) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #endif
        } else {
            #ifdef MTK_RIL_MD1
                RLOGI("Update property SIM2 (dt switched) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #else
                RLOGI("Update property SIM1 (dt switched) [%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #endif
        }
    } else {
        if (rid == RIL_SOCKET_1) {
            RLOGI("Update property SIM1 [%s, %s]", key1, value != NULL ? value : "");
            property_set(key1, value);
        } else {
            RLOGI("Update property SIM2 [%s, %s]", key2, value != NULL ? value : "");
            property_set(key2, value);
        }
    }
}

extern void upadteSystemPropertyByCurrentModeGemini(int rid, char* key1, char* key2, char* key3, char* key4, char* value) {
    int pivot = 1;
    int pivotSim;;

    if (isDualTalkMode() || isCdmaLteDcSupport()) {
        int firstMD = getFirstModem();

        if (firstMD == FIRST_MODEM_NOT_SPECIFIED) {
            //no first MD concept, use external MD mechanism
            int telephonyMode = getTelephonyMode();
            switch (telephonyMode) {
                case TELEPHONY_MODE_100_TDNC_DUALTALK:
                case TELEPHONY_MODE_101_FDNC_DUALTALK:
                    if (rid == RIL_SOCKET_2) {
                        //for international roaming, the socket2 is connected with framework
                        if (getExternalModemSlot() == GEMINI_SIM_1) {
                            RLOGI("Update property SIM2 (exMD) [%s, %s]", key2, value != NULL ? value : "");
                            property_set(key2, value);
                        } else {
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        }
                    } else {
                        LOGW("Update property but no match case[a]");
                    }
                    break;
                default:
                    //TELEPHONY_MODE_0_NONE, TELEPHONY_MODE_102_WNC_DUALTALK, TELEPHONY_MODE_103_TNC_DUALTALK
                    if (rid == RIL_SOCKET_1
                            ||( rid == RIL_SOCKET_2 && (isEVDODTSupport() || isCdmaLteDcSupport()))
				   ){
                        if (isCdmaLteDcSupport()) {
                        #ifdef MTK_RIL_MD1
                            if (rid == RIL_SOCKET_1) { // TODO: add SVLTE judgement here
                                RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                                        property_set(key1, value);
                            } else {
                                RLOGI("Update property SIM2 (exMD)[%s, %s]", key2, value != NULL ? value : "");
                                        property_set(key2, value);
                            }
                        #else
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        #endif
                        } else {
                        #ifdef MTK_RIL_MD1
                            RLOGI("Update property SIM1 (exMD)[%s, %s]", key1, value != NULL ? value : "");
                            property_set(key1, value);
                        #else
                            RLOGI("Update property SIM2 (exMD)[%s, %s]", key2, value != NULL ? value : "");
                            property_set(key2, value);
                        #endif
                        }
                    } else {
                        LOGW("Update property but no match case[b]");
                    }
                    break;
            }
        } else if (firstMD == FIRST_MODEM_MD1) {
            #ifdef MTK_RIL_MD1
                RLOGI("Update property SIM1 (dt)[%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #else
                RLOGI("Update property SIM2 (dt) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #endif
        } else {
            #ifdef MTK_RIL_MD1
                RLOGI("Update property SIM2 (dt switched) [%s, %s]", key2, value != NULL ? value : "");
                property_set(key2, value);
            #else
                RLOGI("Update property SIM1 (dt switched) [%s, %s]", key1, value != NULL ? value : "");
                property_set(key1, value);
            #endif
        }
    } else {
        pivotSim = pivot << rid;
        RLOGI("Update property SIM%d [%s]", pivotSim, value != NULL ? value : "");
        switch(pivotSim) {
            case 1:
                property_set(key1, value);
                break;
            case 2:
                property_set(key2, value);
                break;
            case 4:
                property_set(key3, value);
                break;
            case 8:
                property_set(key4, value);
                break;
            default:
                RLOGE("Update property SIM%d it is unexpected", pivotSim);
                break;
        }
    }
}

extern void bootupGetIccid(RIL_SOCKET_ID rid) {
    int result = 0;
    ATResponse *p_response = NULL;
    int err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response != NULL) {
        if (at_get_cme_error(p_response) == CME_SUCCESS) {
            char *line;
            char *iccId;
            line = p_response->p_intermediates->line;
            err = at_tok_start (&line);
            if (err >= 0) {
                err = at_tok_nextstr(&line, &iccId);
                if (err >= 0) {
                    RLOGD("bootupGetIccid[%d] iccid is %s", rid, iccId);
                    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_ICCID_SIM[0], PROPERTY_ICCID_SIM[1], PROPERTY_ICCID_SIM[2], PROPERTY_ICCID_SIM[3], iccId);
                    result = 1;
                } else
                    RLOGD("bootupGetIccid[%d]: get iccid error 2", rid);
            } else {
                RLOGD("bootupGetIccid[%d]: get iccid error 1", rid);
            }
        } else {
            RLOGD("bootupGetIccid[%d]: Error or no SIM inserted!", rid);
        }
    } else {
        RLOGE("bootupGetIccid[%d] Fail", rid);
    }
    at_response_free(p_response);

    if (!result) {
        RLOGE("bootupGetIccid[%d] fail and write default string", rid);
        char cardType[PROPERTY_VALUE_MAX] = {0};
        char *isRuimCard;
        property_get(PROPERTY_RIL_FULL_UICC_TYPE[rid], cardType, "");
        RLOGD("%s: %s", PROPERTY_RIL_FULL_UICC_TYPE[rid], cardType);
        isRuimCard = strstr(cardType, "RUIM");
        if (isSvlteSupport() && (isRuimCard != NULL)) {
            RLOGD("bootupGetIccid RUIM card, not set iccid.sim1 to N/A");
        } else {
            upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_ICCID_SIM[0], PROPERTY_ICCID_SIM[1],
                PROPERTY_ICCID_SIM[2], PROPERTY_ICCID_SIM[3], "N/A");
        }
    }
}

// send EFUN fristly, for speed up modem searching network, only for OP01 FT case5.1.1
extern void bootupSetRadio(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    char *cmd;
    int err;
    RILChannelCtx* p_channel;
    RIL_SOCKET_ID main_rid = getMainProtocolRid();
    LOGD("bootupSetRadio (%d), main protocol rid (%d)", rid, main_rid);

    char handleDone[PROPERTY_VALUE_MAX] = {0};
    // This property means we only send EFUN in normal bootup stage,
    // Donot send EFUN in IPO bootup, or in Flightmode leave stage etc.
    property_get("ril.poo.handle.done", handleDone, "0");
    if (strcmp(handleDone, "0")) {
        LOGD("bootupSetRadio, return by not in normal bootup stage");
        return;
    }
    char prop[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.operator.optr", prop, "");
    if (strcmp(prop, "OP01")) {
        LOGD("bootupSetRadio, return by not OP01 project");
        return;
    }
    if (SIM_COUNT > 2) {
        LOGD("bootupSetRadio, return by SIM_COUN>2");
        return;
    } else if (SIM_COUNT == 2 && RIL_SOCKET_1 == rid) {
        LOGD("bootupSetRadio, return by RIL_SOCKET_1");
        return;
    }
    char msim_mode[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.radio.multisim.config", msim_mode, "");
    if (!strcmp(prop, "dsda")) {
        LOGD("bootupSetRadio, return by dsda project");
        return;
    }
    char fligt_mode[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.radio.airplane.mode.on", fligt_mode, "false");
    if (strcmp(fligt_mode, "false")) {
        LOGD("bootupSetRadio, return by flightmode current");
        return;
    }
    char iccid[PROPERTY_VALUE_MAX] = { 0 };
    property_get(PROPERTY_ICCID_SIM[main_rid], iccid, "");
    if (strlen(iccid) < 10) {
        LOGD("bootupSetRadio, return by no invalid iccid in PROPERTY_ICCID_SIM[%d]", main_rid);
        return;
    }

    p_channel = getRILChannelCtx(RIL_SIM, main_rid);
    // for data attach, EGTYPE needs applied before CFUN
    combineAttachAndFollowOn(main_rid, 1, p_channel);
    defineAttachApnIfIACacheExisted(main_rid, p_channel);

    // Reuse modem suspend and resum machniasm, means to let modem searching network fristly,
    // then when ap phone process ready, ap sent EMSR to resume modem to start register network.
    // if not handle like this, it will happen a issue, modem already register network,
    // but ap have not ready, so when modem start recevie sms or mms, rild_sms will happen
    // state confusion, so it will cause the phone cannot recevie sms and mms
    at_send_command("AT+ECOPS=1", NULL, getRILChannelCtx(RIL_SIM, main_rid));
    at_send_command("AT+EMSR=0,1", NULL, getRILChannelCtx(RIL_SIM, main_rid));
    LOGD("bootupSetRadio, AT+ECOPS=1 and AT+EMSR=0,1 sent");

    if (SIM_COUNT >= 2) {
        char subiccid[PROPERTY_VALUE_MAX] = { 0 };
        // Only support Dual SIM project
        RIL_SOCKET_ID sub_rid = RIL_SOCKET_2;
        if (main_rid == RIL_SOCKET_2) {
            sub_rid = RIL_SOCKET_1;
        }
        property_get(PROPERTY_ICCID_SIM[sub_rid], subiccid, "");
        if (strlen(subiccid) > 10) {
            at_send_command("AT+ECOPS=1", NULL, getDefaultChannelCtx(sub_rid));
            at_send_command("AT+EMSR=0,1", NULL, getDefaultChannelCtx(sub_rid));
            asprintf(&cmd, "AT+EFUN=3");
            m_msim_mode[RIL_SOCKET_1] = 1;
            m_msim_mode[RIL_SOCKET_2] = 1;
        } else {
            asprintf(&cmd, "AT+EFUN=1");
            m_msim_mode[main_rid] = 1;
        }
    } else {
        asprintf(&cmd, "AT+CFUN=1");
    }

    int fd = open("/proc/bootprof", O_RDWR);
    if (fd == -1){
        LOGD("bootupSetRadio fail to open /proc/bootprof");
    } else if (fd > 0){
        char buf[64] = {0};
        strcpy(buf, cmd);
        write(fd, buf, 32);
        close(fd);
    }

    err = at_send_command(cmd, &p_response, p_channel);
    if (err != 0 || p_response->success == 0)
        LOGW("bootupSetRadio failed");
    at_response_free(p_response);
    property_set("ril.poo.handle.done", "1"); // we only handle once
}

extern void bootupGetImei(RIL_SOCKET_ID rid) {
    RLOGD("bootupGetImei[%d]", rid);
    ATResponse *p_response = NULL;
    int err = at_send_command_numeric("AT+CGSN", &p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response->success != 0) {
        err = asprintf(&s_imei[rid], "%s", p_response->p_intermediates->line);
        if(err < 0)
            RLOGE("bootupGetImei[%d] set fail", rid);
    } else {
        RLOGE("bootupGetImei[%d] Fail", rid);
    }
    at_response_free(p_response);
}

extern void bootupGetImeisv(RIL_SOCKET_ID rid) {
    RLOGD("bootupGetImeisv[%d]", rid);
    ATResponse *p_response = NULL;
    int err = at_send_command_singleline("AT+EGMR=0,9", "+EGMR:",&p_response, getDefaultChannelCtx(rid));

    if (err >= 0 && p_response->success != 0) {
        char* sv = NULL;
        char* line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &sv);
            if(err >= 0) {
                err = asprintf(&s_imeisv[rid], "%s", sv);
                if(err < 0)
                    RLOGE("bootupGetImeisv[%d] set fail", rid);
            } else {
                RLOGE("bootupGetImeisv[%d] get token fail", rid);
            }
        } else {
            RLOGE("bootupGetImeisv[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetImeisv[%d] Fail", rid);
    }
    at_response_free(p_response);
}

void bootupGetProjectFlavor(RIL_SOCKET_ID rid){
    RLOGE("bootupGetProjectAndMdInfo[%d]", rid);
    ATResponse *p_response = NULL;
    ATResponse *p_response2 = NULL;
    char* line = NULL;
    char* line2 = NULL;
    char* projectName= NULL;
    char* flavor= NULL;
    int err;

    // Add for MD-team query project/flavor properties : project name(flavor)
    err = at_send_command_singleline("AT+EGMR=0,4", "+EGMR:",&p_response, getDefaultChannelCtx(rid));
    if (err >= 0 && p_response->success != 0) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err >= 0) {
            err = at_tok_nextstr(&line, &projectName);
            if(err < 0)
                RLOGE("bootupGetProjectName[%d] get token fail", rid);
        } else {
            RLOGE("bootupGetProjectName[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetProjectName[%d] Fail", rid);
    }

    //query project property (flavor)
    err = at_send_command_singleline("AT+EGMR=0,13", "+EGMR:",&p_response2, getDefaultChannelCtx(rid));
    if (err >= 0 && p_response2->success != 0) {
        line2 = p_response2->p_intermediates->line;
        err = at_tok_start(&line2);
        if(err >= 0) {
            err = at_tok_nextstr(&line2, &flavor);
            if(err < 0)
                RLOGE("bootupGetFlavor[%d] get token fail", rid);
        } else {
            RLOGE("bootupGetFlavor[%d] AT CMD fail", rid);
        }
    } else {
        RLOGE("bootupGetFlavor[%d] Fail", rid);
    }

    //combine string: projectName(flavor)
    err = asprintf(&s_projectFlavor[rid], "%s(%s)",projectName ,flavor);
    if(err < 0) RLOGE("bootupGetProject[%d] set fail", rid);

    if (getMappingSIMByCurrentMode(rid) == GEMINI_SIM_2){
        err = property_set("gsm.project.baseband.2",s_projectFlavor[rid]);
        if(err < 0) RLOGE("SystemProperty: PROPERTY_PROJECT_2 set fail");
    }else{
        err = property_set("gsm.project.baseband" ,s_projectFlavor[rid]);
        if(err < 0) RLOGE("SystemProperty: PROPERTY_PROJECT set fail");
    }

    at_response_free(p_response);
    at_response_free(p_response2);

}

extern void bootupGetBasebandVersion(RIL_SOCKET_ID rid) {
    RLOGE("bootupGetBasebandVersion[%d]", rid);
    ATResponse *p_response = NULL;
    int err, i, len;
    char *line, *ver, null;

    ver = &null;
    ver[0] = '\0';

    //Add for MD-team query project/flavor properties : project name(flavor)
    bootupGetProjectFlavor(rid);

    err = at_send_command_multiline("AT+CGMR", "+CGMR:",&p_response, getDefaultChannelCtx(rid));

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }
    else if (p_response->p_intermediates != NULL)
    {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        //remove the white space from the end
        len = strlen(line);
        while( len > 0 && isspace(line[len-1]) )
            len --;
        line[len] = '\0';

        //remove the white space from the beginning
        while( (*line) != '\0' &&  isspace(*line) )
            line++;

        ver = line;
    }
    else
    {
        // ALPS00295957 : To handle AT+CGMR without +CGMR prefix response
        at_response_free(p_response);
        p_response = NULL;

        RLOGE("Retry AT+CGMR without expecting +CGMR prefix");

        err = at_send_command_raw("AT+CGMR", &p_response, getDefaultChannelCtx(rid));

        if (err < 0) {
            RLOGE("Retry AT+CGMR ,fail");
            goto error;
        }

        if(p_response->p_intermediates != NULL)
        {
            line = p_response->p_intermediates->line;

            RLOGD("retry CGMR response = %s", line);

            //remove the white space from the end
            len = strlen(line);
            while( len > 0 && isspace(line[len-1]) )
                len --;
            line[len] = '\0';

            //remove the white space from the beginning
            while( (*line) != '\0' &&  isspace(*line) )
                line++;

            ver = line;
        }
    }
    asprintf(&s_basebandVersion[rid], "%s", ver);
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
}

/* ALPS00582073 START */
extern void bootupGetCalData(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;

    err = at_send_command_singleline("AT+ECAL?", "+ECAL:", &p_response, getDefaultChannelCtx(rid));

    if (err < 0 || p_response->success == 0){
        RLOGE("bootupGetCalData fail,err=%d", err);
    }
    else{
        err = asprintf(&s_calData, "%s", p_response->p_intermediates->line);
        if(err < 0)
            RLOGE("bootupGetCalData set fail,err=%d", err);
        else
            RLOGD("bootupGetCalData s_calData =%s", s_calData);
    }
    at_response_free(p_response);
}
/* ALPS00582073 END */

extern void setActiveModemType(int mdType)
{
    char activeModem[PROPERTY_VALUE_MAX] = {0};

    sprintf(activeModem, "%d", mdType);
	property_set("ril.active.md", activeModem);
    LOGD("[LteMSH][setActiveModemType] activeModem=%s", activeModem);

    return ;
}

extern int getActiveModemType()
{
    int  activeModem = 0;
    char prop_val[PROPERTY_VALUE_MAX] = {0};

    property_get("ril.active.md", prop_val, "0");
    activeModem = atoi(prop_val);
    LOGD("[LteMSH][getActiveModemType] activeModem=%d", activeModem);

    return activeModem;
}

extern void requestQueryThermal(void *data, size_t datalen, RIL_Token t){
    ATResponse *p_response = NULL;
    char* cmd = NULL;
    int err = 0;

    LOGD("requestQueryThermal Enter,%s", data);

    if(atoi(data) == -1){
        err = at_send_command_singleline("AT+ETHERMAL", "+ETHERMAL:", &p_response,
        getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
        if (err < 0 || p_response->success == 0) {
            LOGD("requestQueryThermal error");
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            int err;
            char *line;
            LOGD("requestQueryThermal success");

            line = p_response->p_intermediates->line;
            err = at_tok_start (&line);
            if (err == 0){
                RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strlen(line));
            } else {
                LOGD("requestQueryThermal token start error");
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
        }
    } else {
        asprintf(&cmd, "AT+ETHERMAL=%s", data);
        err = at_send_command_singleline(cmd, "+ETHERMAL:", &p_response,
                getRILChannelCtx(RIL_SIM,getMainProtocolRid()));
        if (err < 0) {
            LOGD("requestQueryThermal error");
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            LOGD("requestQueryThermal success");
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
    }

    at_response_free(p_response);

    return;
}

extern int triggerCCCIIoctlEx(int request, int *param)
{
    int ret_ioctl_val = -1;
    int ccci_sys_fd = -1;
    char dev_node[32] = {0};

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    LOGD("Open CCCI MD1 ioctl port[%s]",CCCI_MD1_POWER_IOCTL_PORT);
    ccci_sys_fd = open(CCCI_MD1_POWER_IOCTL_PORT, O_RDWR);
#else
#ifdef MTK_RIL_MD2
#if MTK_ENABLE_MD5
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS5));
    LOGD("MD2/SYS5 IOCTL [%s, %d]", dev_node, request);
#else
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS2));
    LOGD("MD2 IOCTL [%s, %d]", dev_node, request);
#endif /* MTK_ENABLE_MD5 */
#else /* MTK_RIL_MD2 */
#if MTK_ENABLE_MD1
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS1));
    LOGD("MD1/SYS1 IOCTL [%s, %d]", dev_node, request);
#elif MTK_ENABLE_MD2
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXR_IOCTL, MD_SYS2));
    LOGD("MD2/SYS2 IOCTL [%s, %d]", dev_node, request);
#else
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS5));
    LOGD("MD1/SYS5 IOCTL [%s, %d]", dev_node, request);
#endif
#endif
    ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
#endif

    if (ccci_sys_fd < 0) {
        RLOGD("Open CCCI ioctl port failed [%d]", ccci_sys_fd);
        return -1;
    }

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    if(request == CCCI_IOC_ENTER_DEEP_FLIGHT) {
        int pid = findPid("gsm0710muxd");
        RLOGD("MUXD pid=%d",pid);
        if(pid != -1) kill(pid,SIGUSR2);
        RLOGD("send SIGUSR2 to MUXD done");
        sleepMsec(100);    // make sure MUXD have enough time to close channel and FD
    }
#endif

    ret_ioctl_val = ioctl(ccci_sys_fd, request, param);
    LOGD("CCCI ioctl result: ret_val=%d, request=%d, param=%d", ret_ioctl_val, request, *param);

    close(ccci_sys_fd);
    return ret_ioctl_val;
}

extern int triggerCCCIIoctl(int request)
{
    int param = -1;
    int ret_ioctl_val;

    ret_ioctl_val = triggerCCCIIoctlEx(request, &param);

    return ret_ioctl_val;
}

extern int rilOemMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_OEM_HOOK_RAW:
            requestOemHookRaw(data, datalen, t);
            // echo back data
            //RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
            break;
        case RIL_REQUEST_AT_COMMAND_WITH_PROXY:
            requestAtCmdWithProxy(data, datalen, t);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS:
            requestOemHookStrings(data,datalen,t);
            break;
        case RIL_REQUEST_SCREEN_STATE:
            requestScreenState(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MUTE:
            requestSetMute(data,datalen,t);
            break;
        case RIL_REQUEST_GET_MUTE:
            requestGetMute(data, datalen, t);
            break;
        case RIL_REQUEST_RESET_RADIO:
            requestResetRadio(data, datalen, t);
            break; 
        case RIL_REQUEST_GET_RADIO_CAPABILITY:
            requestGetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            requestSetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_STORE_MODEM_TYPE:
            requestStoreModem(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_MODEM_TYPE:
            requestQueryModem(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL:
            requestQueryThermal(data, datalen, t);
            break;
        case RIL_REQUEST_SET_TRM:
            requestSetTrm(data, datalen, t);
            break;
        case RIL_REQUEST_BASEBAND_VERSION:
            requestBasebandVersion(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEI:
            requestGetImei(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEISV:
            requestGetImeisv(data, datalen, t);
            break;
        case RIL_REQUEST_MOBILEREVISION_AND_IMEI: //Add by mtk80372 for Barcode Number
            requestMobileRevisionAndIMEI(data,datalen,t);
            break;
        //MTK-START [mtk80950][120410][ALPS00266631]check whether download calibration data or not
        case RIL_REQUEST_GET_CALIBRATION_DATA:
            requestGetCalibrationData(data, datalen, t);
            break;
        //MTK-END [mtk80950][120410][ALPS00266631]check whether download calibration data or not
        case RIL_REQUEST_SET_IMS_ENABLE:
            requestSetImsEnabled(data, datalen, t);
            break;
        case RIL_REQUEST_RELOAD_MODEM_TYPE:
            requestReloadModem(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_SET_MODEM_THERMAL:
            requestSetModemThermal(data, datalen, t);
            break;
        default:
            return 0;  /* no matched request */
            break;
    }

    return 1; /* request found and handled */
}

extern int rilOemUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    return handleOemUnsolicited(s, sms_pdu, p_channel);
}

