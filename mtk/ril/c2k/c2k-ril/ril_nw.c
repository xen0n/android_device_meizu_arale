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

#include "hardware/ccci_intf.h"
#include "ril_oem.h"

#define NW_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int no_service_times;
extern int unlock_pin_outtimes;
extern int s_ps_on_desired;
extern int uim_modual_initialized;
extern int inemergency;
extern int currentNetworkMode;
extern struct timeval TIMEVAL_0;
extern int cta_no_uim;
extern int waiting_localdata_disconn;
extern int localindatacall;
extern pthread_cond_t s_pcmodem_signalcond;
extern PcModem_State s_PcModemState;
#ifdef ANDROID_KK
extern ModemInfo *sMdmInfo;
#endif /* ANDROID_KK */
extern int iccid_readable;
extern int cardtype;
extern int pluguinfo;
extern int cpisct;
extern int plugsinfo[2];

extern int uimInsertedStatus;
extern int rildSockConnect;
extern int smsReadyReported;
extern int arsithrehd;
extern int uimpluginreport;
extern int gsmpluginreport;
extern int s_screenState;
extern int phbready;

/*maintain the dbm value reported to framework last time*/
static int last_cdma_dbm = 0;

/*maintain the radio technology reported to framework last time*/
static int last_radio_technology = 0;

/*indicate there exits a repolling network status action in queue*/
static int repollinqueue = 0;

/*maintain the service state reported to framework last time*/
static int last_service_state = 0;

static int last_flight_mode = 0;

//add for VSER & MODE
static int NetworkMode = -1;

static int ps_state = 0;

int s_md3_off = 0;

extern void trigger_update_cache();
extern int getCdmaModemSlot(void);
extern int waitUimModualInitialized(void);
extern RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus);
extern void setNetworkMode(void *param);
extern int waitIccidReadable(void);
extern void reportPbSmsReady(void);
extern void reportCdmaSimPlugIn(void);
extern void reportGSMSimPlugIn(void);
extern void turnPSEnable(void *param);
extern void onPCModemDisconn(void *param);
extern void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid);
extern void updateRadioStatus(void);
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature.
extern int switchStkUtkMode(int mode, RIL_Token t);

#ifdef ANDROID_KK
extern int parse_technology_response(const char *response, int *current, int32_t *preferred);
#endif /* ANDROID_KK */

static void repollNetworkState(void *param);
static void handleEmergencyExit(void *param);
static void queryIccid(void *param);
static void requestQueryNetworkSelectionMode(void *data, size_t datalen, RIL_Token t);
static void requestBasebandVersion(void *data, size_t datalen, RIL_Token t);
static void requestSetPreferredNetworkType(void *data, size_t datalen, RIL_Token t);
static void requestGetPreferredNetworkType(void *data, size_t datalen, RIL_Token t);
static void requestIPORadioPowerOff(void *data, size_t datalen, RIL_Token t);
static void requestGetNitzTime(void *data, size_t datalen, RIL_Token t);
static void requestQueryNetworkRegistration(void *data, size_t datalen, RIL_Token t);
/// M: [C2K][IR] Support SVLTE IR feature. @{
static void requestResumeCdmaRegistration(void *data, size_t datalen, RIL_Token t);
static void requestSetCdmaRegSuspendEnabled(void *data, size_t datalen, RIL_Token t);
static int isCdmaRegSuspendEnabled(void);
/// M: [C2K][IR] Support SVLTE IR feature. @}
static void requestSetArsiThreshold(void *data, size_t datalen, RIL_Token t);
static void setNetworkServiceProperty(VIA_NETWORK_TYPE type);
static void requestQueryAvoidSys(void *data, size_t datalen, RIL_Token t);
static void requestSetAvoidSys(void *data, size_t datalen, RIL_Token t);
static void requestQueryNetworkInfo(void *data, size_t datalen, RIL_Token t);

#ifdef ANDROID_KK
static int techFromModemType(int mdmtype);
static void setRadioTechnology(ModemInfo *mdm, int newtech);
#endif /* ANDROID_KK */


#ifdef AT_AUTO_SCRIPT_MODE
static void SetModeUnreported(void* param);
#endif /* AT_AUTO_SCRIPT_MODE */

static void onEngModeInfoUpdate(char* urc);

static void onNetworkInfo(char* urc);

static void onNetworkExistence(char * urc);

extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_SIGNAL_STRENGTH:
            requestSignalStrength(data, datalen, t);
            break;
        case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        case RIL_REQUEST_DATA_REGISTRATION_STATE:
            requestRegistrationState(request, data, datalen, t);
            break;
        case RIL_REQUEST_OPERATOR:
            requestOperator(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWER:
            requestRadioPower(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEI:
            requestGetIMEI(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
            requestQueryNetworkSelectionMode(data, datalen, t);
            break;
        case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: {
            ATResponse *p_response = NULL;
            int err = at_send_command("AT+COPS=0", &p_response, NW_CHANNEL_CTX);
            if (err < 0 || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
        }
            break;
        case RIL_REQUEST_BASEBAND_VERSION:
            requestBasebandVersion(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            requestSetPreferredNetworkType(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            requestGetPreferredNetworkType(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWERON:
            requestIPORadioPowerOn(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWEROFF:
            requestIPORadioPowerOff(data, datalen, t);
            break;
        case RIL_REQUEST_MODEM_POWEROFF:
            requestModemPowerOff(data, datalen, t);
            break;
        case RIL_REQUEST_MODEM_POWERON:
            requestModemPowerOn(data, datalen, t);
            break;
        case RIL_REQUEST_GET_NITZ_TIME:
            requestGetNitzTime(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_NETWORK_REGISTRATION:
            requestQueryNetworkRegistration(data, datalen, t);
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case RIL_REQUEST_RESUME_REGISTRATION_CDMA:
            requestResumeCdmaRegistration(data, datalen, t);
            break;
        case RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA:
            requestSetCdmaRegSuspendEnabled(data, datalen, t);
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        case RIL_REQUEST_SET_ARSI_THRESHOLD:
            requestSetArsiThreshold(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_CDMA_NETWORK_INFO:
            requestQueryNetworkInfo(data, datalen, t);
            break;
        case RIL_REQUEST_SET_AVOID_SYS:
            requestSetAvoidSys(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_AVOID_SYS:
            requestQueryAvoidSys(data, datalen, t);
            break;
        /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
        case RIL_REQUEST_SET_SVLTE_RAT_MODE:
            requestSetSvlteRatMode(data, datalen, t);
        break;
        /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}
        default:
            return 0; /* no matched request */
    }
    return 1;
}

extern int rilNwUnsolicited(const char *s, const char *sms_pdu) {
    char *line = NULL;
    char *line_csq = NULL;
    char * output_string = NULL;
    int err;

    if (strStartsWith(s, "^SYSINFO:")) {
        int sysinfo_service_mode = 0;
        int sysinfo_roaming = 0;
        int skip = 0;
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
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sysinfo_roaming);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sysinfo_service_mode);
        if (err < 0) {
            free(dup);
            return 1;
        }

        LOGD("inemergency = %d", inemergency);

        setNetworkServiceProperty(sysinfo_service_mode);

        //s_unsol_msg_cache.evdo_service_state:1--> Do in service
        //s_unsol_msg_cache.evdo_service_state:0--> Do no service
        //s_unsol_msg_cache.x1_service_state:2-->1x is searching
        //s_unsol_msg_cache.x1_service_state:1--> 1x in service
        //s_unsol_msg_cache.x1_service_state:0--> 1x no service
        //s_unsol_msg_cache.service_state:2-->framework searching
        //s_unsol_msg_cache.service_state:1-->framework in service
        //s_unsol_msg_cache.service_state:0-->framework no service
        LOGD("NetworkMode is:%d", NetworkMode);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            LOGD("SYSINFO:network mode is:1x only, ignore it");
            break;
        case NET_TYPE_EVDO_ONLY:
            if ((sysinfo_service_mode == 0)
                    || (getRadioState() != RADIO_STATE_RUIM_READY
                            && (inemergency == 0))) {
                //((Do is no service) ||(not RUIM_READY && not in inemergency), set no service
                LOGD("SYSINFO:Do only mode, Do no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (sysinfo_service_mode != 0) {
                //Do is in service, set in service
                LOGD("SYSINFO:Do only mode, Do in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            }
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (((sysinfo_service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 0))
                    || (getRadioState() != RADIO_STATE_RUIM_READY
                            && (inemergency == 0))) {
                //((Do is no service) && (1x is no service)) ||(not RUIM_READY  && not in inemergency), set no service
                LOGD("SYSINFO:hybrid mode, Do no service, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (sysinfo_service_mode != 0) {
                //Do or 1x is in service, set in service
                LOGD("SYSINFO:hybrid mode, Do/1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            } else if ((sysinfo_service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 2)) {
                //beacuse if Do is no service, but 1x in searching(VSER: 1, x1_service_state == 2), sysinfo_service_mode == 0
                LOGD(
                        "SYSINFO:hybrid mode, Do is no service, 1x is in searching");
                //in this situation, keep service_state value as before
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                LOGD("SYSINFO:other case......sysinfo_service_mode:%d, s_unsol_msg_cache.x1_service_state:%d",
                        sysinfo_service_mode,
                        s_unsol_msg_cache.x1_service_state);
            }
            break;
        default:
            LOGD("SYSINFO:why come here? NetworkMode:%d", NetworkMode);
            break;
        }

        LOGD("SYSINFO:service_state is:%d", s_unsol_msg_cache.service_state);

        int radio_technology;
        if (sysinfo_service_mode == 8 || sysinfo_service_mode == 4) {
            radio_technology = 8;
        } else if (sysinfo_service_mode == 2) {
            radio_technology = 6;
        } else {
            radio_technology = 0;
        }
        if (!isCdmaLteDcSupport()) {
            s_unsol_msg_cache.radio_technology = radio_technology;
        } else {
            if (s_unsol_msg_cache.radio_technology < 13) {
                s_unsol_msg_cache.radio_technology = radio_technology;
            }
        }
        if (sysinfo_roaming == 1)
            s_unsol_msg_cache.roaming_indicator = 0;
        else
            s_unsol_msg_cache.roaming_indicator = 1;

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);

        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CREG:")) {
        char *dup = strdup(s);

        int skip = 0;
        int len = 0;
        int commas_count = 0;
        char * p = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',') {
                commas_count++;
            }
        }
        LOGD("commas_count is %d", commas_count);
        switch (commas_count) {
        case 0: /* +CREG: <stat> */
        case 2: /* +CREG: <stat>, <lac>, <cid> */
            err = at_tok_nextint(&line, &(s_unsol_msg_cache.register_state));
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        case 1: /* +CREG: <n>, <stat>*/
        case 3: /* +CREG: <n>, <stat>, <lac>, <cid> */
        case 4: /* +CREG: <n>, <stat>, <lac>, <cid>, <networkType> */
            err = at_tok_nextint(&line, &skip);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &(s_unsol_msg_cache.register_state));
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        default:
            LOGD("unsupport +CREG");
            break;
        }
        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
    } else if (strStartsWith(s, "+ECGREG:")) {
        if (isCdmaLteDcSupport()) {
            char *dup = strdup(s);
            int state = 0;
            int ps_type = 0;
            int response[1];
            int commas_count = 0;
            char * p = NULL;
            if (!dup) {
                return 1;
            }
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                return 1;
            }
            for (p = line; *p != '\0'; p++) {
                if (*p == ',') {
                    commas_count++;
                }
            }
            LOGD("commas_count is %d", commas_count);
            switch (commas_count) {
            case 0: /* +ECGREG: <state> */
                err = at_tok_nextint(&line, &state);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            case 1: /* +ECGREG: <state>, <ps_type> */
                err = at_tok_nextint(&line, &state);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &ps_type);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            default:
                LOGD("unsupport ECGREG");
                break;
            }
            free(dup);

            if (state != ps_state) {
                int ps_state_response[1];
                ps_state_response[0] = state;
                RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED,
                        ps_state_response, sizeof(ps_state_response));
            }

            if (state == 0) {
                ps_state = 0;
                s_unsol_msg_cache.radio_technology = 0;
            } else if (state == 1) {
                ps_state = 1;
                if (ps_type == 2) {
                    s_unsol_msg_cache.radio_technology = 13;
                } else if (ps_type == 1) {
                    s_unsol_msg_cache.radio_technology = 8;
                } else if (ps_type == 0) {
                    s_unsol_msg_cache.radio_technology = 6;
                }
            }
            response[0] = s_unsol_msg_cache.radio_technology;
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED, response, sizeof(response));
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        }
       return 1;
     } else if (strStartsWith(s, "+VMCCMNC:")) {
        char *dup = strdup(s);
        int skip = 0;
        int commas_count = 0;
        char * p = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',')
                commas_count++;
        }
        LOGD("commas_count is %d", commas_count);
        switch (commas_count) {
            case 1: /* +VMCCMNC:<MCC>,<MNC> */
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strcpy(s_unsol_msg_cache.mcc, output_string);
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strcpy(s_unsol_msg_cache.mnc, output_string);
                break;
            case 2: /* +VMCCMNC:<MccMnc>,<MCC>,<MNC> */
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strcpy(s_unsol_msg_cache.mcc, output_string);
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strcpy(s_unsol_msg_cache.mnc, output_string);
                break;
            default:
                LOGD("unsupport VMCCMNC");
                break;
            }
        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s, "+VROM:")) {
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
        err = at_tok_nextint(&line, &(s_unsol_msg_cache.roaming_indicator));
        if (err < 0) {
            free(dup);
            return 1;
        }
        free(dup);
        if (s_unsol_msg_cache.roaming_indicator > 12) {
            s_unsol_msg_cache.roaming_indicator = 1;
        }
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s,"^MODE:")) {
        int service_mode = 0;
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
        err = at_tok_nextint(&line, &service_mode);
        if (err < 0) {
            free(dup);
            return 1;
        }
        free(dup);
        LOGD("inemergency = %d", inemergency);

        setNetworkServiceProperty(service_mode);

        //s_unsol_msg_cache.evdo_service_state:1--> Do in service
        //s_unsol_msg_cache.evdo_service_state:0--> Do no service
        //s_unsol_msg_cache.x1_service_state:2-->1x is searching
        //s_unsol_msg_cache.x1_service_state:1--> 1x in service
        //s_unsol_msg_cache.x1_service_state:0--> 1x no service
        //s_unsol_msg_cache.service_state:2-->framework searching
        //s_unsol_msg_cache.service_state:1-->framework in service
        //s_unsol_msg_cache.service_state:0-->framework no service
        LOGD("MODE:NetworkMode is:%d", NetworkMode);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            LOGD("MODE:network mode is:1x only, ignore it");
            break;
        case NET_TYPE_EVDO_ONLY:
            if ((service_mode == 0)
                    || (getRadioState() != RADIO_STATE_RUIM_READY
                            && (inemergency == 0))) {
                //((Do is no service) ||(not RUIM_READY && not in inemergency), set no service
                LOGD("MODE:Do only mode, Do no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                //Do is in service, set in service
                LOGD("MODE:Do only mode, Do in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            }
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (((service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 0))
                    || (getRadioState() != RADIO_STATE_RUIM_READY
                            && (inemergency == 0))) {
                //((Do is no service) && (1x is no service)) ||(not RUIM_READY  && not in inemergency), set no service
                LOGD("MODE:hybrid mode, Do no service, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (service_mode != 0) {
                //Do/1x is in service, set in service
                LOGD("MODE:hybrid mode, Do/1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            } else if ((service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 2)) {
                //Do is no service, but 1x may in searching
                LOGD("MODE:hybrid mode, DO is no service, 1x is searching");
                //in this situation, keep service_state value as before
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                LOGD(
                        "MODE:hybrid mode, other case, service_mode:%d, s_unsol_msg_cache.evdo_service_state:%d",
                        service_mode, s_unsol_msg_cache.evdo_service_state);
            }
            break;
        default:
            LOGD("MODE:why come here? NetworkMode:%d", NetworkMode);
            break;
        }

        LOGD("MODE:service_state is:%d", s_unsol_msg_cache.service_state);

        int radio_technology;
        if (service_mode == 8 || service_mode == 4) {
            radio_technology = 8;
        } else if (service_mode == 2) {
            radio_technology = 6;
        } else {
            radio_technology = 0;
        }
        if (!isCdmaLteDcSupport()) {
            s_unsol_msg_cache.radio_technology = radio_technology;
        } else {
            if (s_unsol_msg_cache.radio_technology < 13) {
                s_unsol_msg_cache.radio_technology = radio_technology;
            }
        }

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
#ifdef ADD_MTK_REQUEST_URC
        RIL_onUnsolicitedResponse(RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE,
                &service_mode, sizeof(int));
#endif
#ifdef AT_AUTO_SCRIPT_MODE
        if(service_mode == ctclientServiceMode)
        {
            ctclientServiceMode = 0;
            if(!s_screenState)
            RIL_requestProxyTimedCallback (SetModeUnreported, NULL, NULL, getDefaultChannelCtx()->id);
        }
#endif
#if 0
        if(s_wait_repoll_flag == 0) /* while wait repoll, don't send other unsol */
        {
            RIL_onUnsolicitedResponse (
                    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
                    NULL, 0);
        }
        else
        {
            LOGD("Watting repoll network state, don't send other unsol");
        }
#endif
        return 1;
     } else if (strStartsWith(s, "+VSER:")) {
        char *skip = NULL;
        int service_state = 0;
        char *dup = NULL;
        char *p = NULL;
        int commas_count = 0;
        int service_state_1x = 0;

        dup = strdup(s);
        if (!dup) {
            return 1;
        }

        /*if Card is NOT READY or LOCK CARD, UI should still show no service */
        if ((getRadioState() != RADIO_STATE_RUIM_READY) && (inemergency == 0)) {
            s_unsol_msg_cache.service_state = 0;
            goto vserdone;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',')
                commas_count++;
        }
        LOGD("commas_count is %d", commas_count);

        switch (commas_count) {
        case 0:
            err = at_tok_nextint(&line, &service_state_1x);
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        case 1:
            err = at_tok_nextint(&line, &skip);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &service_state_1x);
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        default:
            break;
        }
        LOGD("service_state_1x is:%d", service_state_1x);

        /*
         the value of "+VSER:" could't match with framework's, do converting here.
         +VSER: 0 in service,
         1 out of service,but searching
         2 out of service, and stop searching
         3 limited service
         s_unsol_msg_cache.evdo_service_state:1--> Do in service
         s_unsol_msg_cache.evdo_service_state:0--> Do no service
         s_unsol_msg_cache.x1_service_state:2-->1x is searching
         s_unsol_msg_cache.x1_service_state:1--> 1x in service
         s_unsol_msg_cache.x1_service_state:0--> 1x no service
         s_unsol_msg_cache.service_state:2-->framework searching
         s_unsol_msg_cache.service_state:1-->framework in service
         s_unsol_msg_cache.service_state:0-->framework no service
         */
        LOGD("VSER:NetworkMode is:%d", NetworkMode);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            if (service_state_1x == 0) {
                LOGD("VSER:1X only mode, 1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.x1_service_state = 1;
            } else if (service_state_1x == 1) {
                LOGD("VSER:1X only mode, 1x searching");
                s_unsol_msg_cache.service_state = 2;
                s_unsol_msg_cache.x1_service_state = 2;
            } else if (service_state_1x == 2) {
                LOGD("VSER:1X only mode, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.x1_service_state = 0;
            } else if (service_state_1x == 3) {
                // [ALPS02039951]
                LOGD("VSER:1X only mode, 1x limited service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.x1_service_state = 0;
            }
            break;
        case NET_TYPE_EVDO_ONLY:
            LOGD("VSER:NetworkMode Do Only, ignore it");
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (service_state_1x == 0) {
                LOGD("VSER:hybrid mode, 1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.x1_service_state = 1;
            } else if (service_state_1x == 1) {
                if (s_unsol_msg_cache.evdo_service_state == 1) {
                    //1x searching, DO in service, set in service
                    LOGD("VSER:hybrid mode, 1x searching Do in service");
                    s_unsol_msg_cache.service_state = 1;
                } else {
                    //1x searching, DO no service, set searching
                    LOGD("VSER:hybrid mode, 1x searching Do no service");
                    s_unsol_msg_cache.service_state = 2;
                }
                s_unsol_msg_cache.x1_service_state = 2;
            } else if (service_state_1x == 2) {
                if (s_unsol_msg_cache.evdo_service_state == 0) {
                    //1x No service, DO no service, set no service
                    LOGD("VSER:hybrid mode, 1x no service, Do in service");
                    s_unsol_msg_cache.service_state = 0;
                }
                //otherwise keep service_state value as before
                LOGD("VSER:hybrid mode, 1x no service Do in service");
                s_unsol_msg_cache.x1_service_state = 0;
            } else if (service_state_1x == 3) {
                // [ALPS02039951]
                if (s_unsol_msg_cache.evdo_service_state == 0) {
                    // 1x limited service, DO no service, set limited service
                    LOGD("VSER:hybrid mode, 1x limited service, Do no service");
                    s_unsol_msg_cache.service_state = 0;
                } else {
                    // 1x limited service, DO in service, set in service
                    LOGD("VSER:hybrid mode, 1x limited service, Do in service");
                    s_unsol_msg_cache.service_state = 1;
                }
                // otherwise keep service_state value as before
                LOGD("VSER:hybrid mode, 1x limited service");
                s_unsol_msg_cache.x1_service_state = 0;
            }
            break;
        default:
            LOGD("VSER:why come here?currentNetworkMode:%d",
                    currentNetworkMode);
            break;
        }

        LOGD("VSER:%s, s_unsol_msd_cache.service_state:%d", __FUNCTION__,
                s_unsol_msg_cache.service_state);

        vserdone: free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s,"+HDRCSQ") ){
         int evdo_dbm = 0;
         int evdo_ber = 0;
         int evdo_ecio = 0;
         int commas_count = 0;
         char *dup = strdup(s);
         char *p = NULL;

         if (!dup) {
             return 1;
         }

         line = dup;
         err = at_tok_start(&line);
         if (err < 0) {
             free(dup);
             return 1;
         }
         for (p = line; *p != '\0'; p++) {
             if (*p == ',') {
                 commas_count++;
             }
         }
         LOGD("commas_count is %d", commas_count);
         switch (commas_count) {
         case 0: /* +HDRCSQ: <rssi> */
         case 1:
             err = at_tok_nextint(&line, &evdo_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         case 2: /* +HDRCSQ: <rssi>,<ber>,<ecio> */
             err = at_tok_nextint(&line, &evdo_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &evdo_ber);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &evdo_ecio);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         default:
             LOGD("unsupported +HDRCSQ");
             break;
         }

         evdo_dbm = 114 - evdo_dbm * 2;
         s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.dbm = evdo_dbm;
         LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.dbm = %d",
                 s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.dbm);

         if (commas_count == 2) {
             evdo_ecio = evdo_ecio / 8;
             s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.ecio = evdo_ecio;
             LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.ecio = %d",
                     s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.ecio);
         }

         free(dup);

 #ifndef ADD_MTK_REQUEST_URC
         if(s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm > 100)
 #else
         if (0)
 #endif
         {
             //if no signal
             LOGD("1X signal is 0, poll network state");
             RIL_onUnsolicitedResponse(
                     RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
         } else {
             RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH,
                     &(s_unsol_msg_cache.signal_strength),
                     sizeof(RIL_SignalStrength_v5));
         }
 #if 0
         RIL_SignalStrength *signal = NULL;
         signal = (RIL_SignalStrength *)alloca(sizeof(RIL_SignalStrength));
         memset(signal, 0, sizeof(RIL_SignalStrength));
         line_csq = strdup(s);
         at_tok_start(&line_csq);
         pthread_mutex_lock(&s_signal_change_mutex);
         err = at_tok_nextint(&line_csq, &evdo_dbm);
         signal->GW_SignalStrength.signalStrength=0;
         signal->GW_SignalStrength.bitErrorRate=0;
         evdo_dbm = 125-(evdo_dbm*50)/31;
         signal->CDMA_SignalStrength.dbm = cdma_dbm;
         signal->CDMA_SignalStrength.ecio= cdma_ecio;
         signal->EVDO_SignalStrength.dbm=evdo_dbm;
         signal->EVDO_SignalStrength.ecio=evdo_ecio;
         signal->EVDO_SignalStrength.signalNoiseRatio=evdo_ratio;
         pthread_mutex_unlock(&s_signal_change_mutex);
         free(line_csq);
         if(s_wait_repoll_flag == 0) /* while wait repoll, don't send other unsol */
         {
             RIL_onUnsolicitedResponse (
                     RIL_UNSOL_SIGNAL_STRENGTH,
                     signal, sizeof(RIL_SignalStrength));
         }
         else
         {
             LOGD("Watting repoll signal strength, don't send other unsol");
         }
 #endif
         return 1;
     } else if (strStartsWith(s, "+CSQ:")) {
         int cdma_dbm = 0;
         int cdma_ber = 0;
         int cdma_ecio = 0;
         int commas_count = 0;
         char *dup = strdup(s);
         char *p = NULL;

         if (!dup) {
             return 1;
         }

         line = dup;
         err = at_tok_start(&line);
         if (err < 0) {
             free(dup);
             return 1;
         }
         for (p = line; *p != '\0'; p++) {
             if (*p == ',') {
                 commas_count++;
             }
         }
         LOGD("commas_count is %d", commas_count);
         switch (commas_count) {
         case 0: /* +CSQ: <rssi> */
         case 1:
             err = at_tok_nextint(&line, &cdma_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         case 2: /* +CSQ: <rssi>,<ber>,<ecio> */
             err = at_tok_nextint(&line, &cdma_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &cdma_ber);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &cdma_ecio);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         default:
             LOGD("unsupported +CSQ");
             break;
         }
         cdma_dbm = 113 - cdma_dbm * 2; // 31 -->  -51dbm
         cdma_ecio = cdma_ecio / 2;
 #if 0
         if (s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm <= 12) {
             s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = 120 - (s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm * 5) / 2;
         } else if (s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm <= 19) {
             s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = 116 - (s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm * 15) / 7;
         } else {
           s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = 107 - (s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm * 5) / 3;
         }
 #endif
         free(dup);
 #ifndef ADD_MTK_REQUEST_URC
         if(cdma_dbm> 100)
 #else
         if (0)
 #endif
         {
             //if no signal
             LOGD("1X signal is 0, poll network state");
             RIL_onUnsolicitedResponse(
                     RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
         } else {
             s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = cdma_dbm;
             LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = %d",
                     s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm);
             last_cdma_dbm = cdma_dbm;
             if (commas_count == 2) {
                 s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio = cdma_ecio;
                 LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio = %d",
                         s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio);
             }
             RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH,
                     &(s_unsol_msg_cache.signal_strength),
                     sizeof(RIL_SignalStrength_v6));
         }
 #if 0
         RIL_SignalStrength *signal = NULL;
         signal = (RIL_SignalStrength *)alloca(sizeof(RIL_SignalStrength));
         memset(signal, 0, sizeof(RIL_SignalStrength));
         line_csq = strdup(s);
         at_tok_start(&line_csq);
         pthread_mutex_lock(&s_signal_change_mutex);
         err = at_tok_nextint(&line_csq, &cdma_dbm);
         signal->GW_SignalStrength.signalStrength=0;
         signal->GW_SignalStrength.bitErrorRate=0;
         //cdma_dbm = 125-(cdma_dbm*50)/31;
         cdma_dbm = 113 - cdma_dbm*2;
         signal->CDMA_SignalStrength.dbm = cdma_dbm;
         signal->CDMA_SignalStrength.ecio= cdma_ecio;
         signal->EVDO_SignalStrength.dbm=evdo_dbm;
         signal->EVDO_SignalStrength.ecio=evdo_ecio;
         signal->EVDO_SignalStrength.signalNoiseRatio=evdo_ratio;
         pthread_mutex_unlock(&s_signal_change_mutex);
         free(line_csq);
         RIL_onUnsolicitedResponse (
                 RIL_UNSOL_SIGNAL_STRENGTH,
                 signal, sizeof(RIL_SignalStrength));
 #endif
         return 1;
     } else if (strStartsWith(s,"+CCLK:")) {
        char *dup = strdup(s);
        char yearstring[5];
        int year = 2011;

        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }

        strncpy(yearstring, line, 4);
        year = atoi(yearstring);
        LOGD("line = %s  year = %d", line, year);
        if (year < 2012) {
            LOGE("wrong time, return!!");
            return 1;
        }
        RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, line,
                strlen(line));
        free(dup);
        return 1;
    } else if (strStartsWith(s,"+CTZV:")) {
        char *dup = strdup(s);
        /*
         *   0: year, two last digits of year
         *   1: month
         *   2: day
         *   3: hour
         *   4: minute
         *   5: second
         */
        int times[6] = { 0 };
        int time_zone = 0;
        int i = 0;
        char *sign_flag = NULL;
        char *timeString = NULL;

        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        LOGD("line = %s", line);

        for (i = 0; i < 6; i++) {
            err = at_tok_nextint(&line, &times[i]);
            if (err < 0) {
                free(dup);
                return 1;
            }
        }
        err = at_tok_nextstr(&line, &sign_flag);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &time_zone);
        if (err < 0) {
            free(dup);
            return 1;
        }
        asprintf(&timeString, "20%d/%d/%d,%d:%d:%d%s%d", times[0], times[1],
                times[2], times[3], times[4], times[5], sign_flag, time_zone);
        LOGD("timeString is %s", timeString);
        RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, timeString,
                strlen(timeString));
        free(timeString);
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+WPRL: ")) {
        int version = -1;
        char *dup = NULL;
        line = dup = strdup(s);
        if (!dup) {
            LOGE("+WPRL: Unable to allocate memory");
            return 1;
        }
        if (at_tok_start(&dup) < 0) {
            LOGE("invalid +WPRL response: %s", s);
            free(dup);
            return 1;
        }
        if (at_tok_nextint(&dup, &version) < 0) {
            LOGE("invalid +WPRL response: %s", s);
            free(dup);
            return 1;
        }
        free(dup);
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PRL_CHANGED, &version,
                sizeof(version));
        return 1;
    } else if (strStartsWith(s, "+CIEV:")) {
        int ind;
        int val;
        int prl;

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
        err = at_tok_nextint(&line, &ind);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &val);
        if (err < 0) {
            free(dup);
            return 1;
        }

        /* <ind> 102: E911 mode indication; <value> 0:disabled; 1:enabled */
        if (ind == 102 && val == 1) {
            s_unsol_msg_cache.service_state = 1;
            RIL_onUnsolicitedResponse(RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE,
                    NULL, 0);
        } else if (ind == 102 && val == 0) {
            if (inemergency) {
                inemergency = 0;
#ifndef OPEN_PS_ON_CARD_ABSENT
                RIL_requestProxyTimedCallback(handleEmergencyExit, NULL,
                        NULL, getDefaultChannelCtx()->id);
#endif
                RIL_requestProxyTimedCallback(trigger_update_cache, NULL,
                        &TIMEVAL_0, getDefaultChannelCtx()->id);
            }
        } else if (ind == 12 && val == 0) {
            iccid_readable = 0;
            cta_no_uim = 1;
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                    NULL, 0);
        } else if (ind == 12 && val == 1) {
            iccid_readable = 0;
            cta_no_uim = 0;
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                    NULL, 0);
        } else if (ind == 13) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CALL_ACCEPTED, NULL, 0);
        } else if (ind == 103 && val == 0) {
            LOGD(
                    "onUnsolicited: datacall disconnected deactivedata_inprogress = %d waiting_localdata_disconn = %d",
                    deactivedata_inprogress, waiting_localdata_disconn);
            if (deactivedata_inprogress) {
                deactivedata_inprogress = 0;
                if (waiting_localdata_disconn) {
                    LOGD("broadcast s_pcmodem_signalcond");
                    localindatacall = 0;
                    pthread_cond_signal(&s_pcmodem_signalcond);
                }
                return 1;
            }
            if (s_PcModemState != PCMODEM_DISCONNECT) { //No Carrier isn't caused by ATH
                s_PcModemState = PCMODEM_DISCONNECT;
                LOGD("PC Modem disconnected");
                /*Fix HANDROID#1989 : Quick response to avoid swich bypass action affect PC datacall connection*/
                //sleep(4);
                onPCModemDisconn(NULL);
                RIL_requestProxyTimedCallback(setNetworkMode,
                        (void *) &currentNetworkMode, NULL, getDefaultChannelCtx()->id);
                return 1;
            }

            /* Send notify data connection changed also */
            if (localindatacall == 1) {
                localindatacall = 0;
                RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                        NULL, 0);
            }
        }
        /*iccid is unready when exec +VRESETUIM*/
        else if (ind == 109 && val == 0) {
            if (iccid_readable)
                iccid_readable = 0;
        }
        /*iccid is readable after modem boot up or exec +VRESETUIM complete*/
        else if (ind == 109 && val == 1) {
            if (!iccid_readable) {
                iccid_readable = 1;
                if (UIM_STATUS_CARD_INSERTED == uimInsertedStatus) {
                    RIL_requestProxyTimedCallback(queryIccid, NULL, &TIMEVAL_0, getDefaultChannelCtx()->id);
                }
            }
        } else if (ind == 130) {
            NetworkMode = val;
            LOGD("CIEV:NetworkMode:%d", val);
        } else if (ind == 131) {
            prl = val;
            LOGD("PRL:%d", prl);
            RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PRL_CHANGED, &prl,
                    sizeof(prl));
        }
#ifdef ADD_MTK_REQUEST_URC
        else if (ind == 101 && val == 1) {
            LOGD("rildSockConnect is %d", rildSockConnect);
            phbready = 1;
            LOGD("phbready = %d", phbready);
            if (rildSockConnect) {
                int slotid = 0;
                slotid = getCdmaModemSlot();
                if (phbready) {
                    switch (slotid) {
                    case 1:
                        property_set("gsm.sim.ril.phbready", "true");
                        break;
                    case 2:
                        property_set("gsm.sim.ril.phbready.2", "true");
                        break;
                    case 3:
                        property_set("gsm.sim.ril.phbready.3", "true");
                        break;
                    case 4:
                        property_set("gsm.sim.ril.phbready.4", "true");
                        break;
                    default:
                        LOGE("reportPbSmsReady, unsupport slot id %d", slotid);
                        break;
                    }
                    LOGD("phb ready, slotid:%d", slotid);
                }
                RIL_requestProxyTimedCallback(reportPbSmsReady, NULL, &TIMEVAL_0, MISC_CHANNEL);
                smsReadyReported = 1;
            } else {
                smsReadyReported = 0;
            }
        }else if (ind == 107 && val == 1) {
            LOGD("rilNwUnsolicited: +CIEV:107,1");
            RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_IMSI_READY, NULL,0);
        } else if (ind == 110 && val == 0) {
            LOGD("rilNwUnsolicited: +CIEV:110,0");
            resetUtkStatus();
        }
#endif
        free(dup);
        return 1;
    }
#ifdef ANDROID_KK
    else if (strStartsWith(s, "+CTEC: ")) {
        int tech, mask;
        switch (parse_technology_response(s, &tech, NULL)) {
            case -1: // no argument could be parsed.
                LOGE("invalid CTEC line %s\n", s);
                break;
            case 1: // current mode correctly parsed
            case 0: // preferred mode correctly parsed
                mask = 1 << tech;
                if (mask != MDM_GSM && mask != MDM_CDMA && mask != MDM_WCDMA
                        && mask != MDM_LTE) {
                    LOGE("Unknown technology %d\n", tech);
                } else {
                    setRadioTechnology(sMdmInfo, tech);
                }
                break;
        }
        return 1;
    }
    else if (strStartsWith(s, "+CFUN: 0"))
    {
        setRadioState(RADIO_STATE_OFF);
    }
#endif /* ANDROID_KK */
     else if (strStartsWith(s, "^PREFMODE")) {
        char *dup = strdup(s);
        if (!dup) {
            LOGD("strdup error!");
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            LOGD("at_tok_start error!");
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &NetworkMode);
        if (err < 0) {
            LOGD("at_tok_nextstr error!");
            free(dup);
            return 1;
        }
        LOGD("NetworkMode:%d", NetworkMode);
        return 1;
    }
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    else if (strStartsWith(s,"+REGPAUSEIND:")) {
        RIL_Mccmnc mcc_mnc;
        memset(&mcc_mnc, 0, sizeof(RIL_Mccmnc));
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
        LOGD("line = %s", line);

        err = at_tok_nextstr(&line, &mcc_mnc.mcc);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &mcc_mnc.mnc);
        if (err < 0) {
            free(dup);
            return 1;
        }
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED, &mcc_mnc,
                sizeof(RIL_Mccmnc));
        free(dup);
        return 1;
    }
    /// M: [C2K][IR] Support SVLTE IR feature. @}
    else if (strStartsWith(s, "+VCPCOMPILE:"))
    {
        char *dup = NULL;
        char *line_cp_lock;
        char *line_cp_1x;

        dup = strdup(s);
        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &line_cp_lock);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &line_cp_1x);
        if (err < 0) {
            free(dup);
            return 1;
        }

        if (!strcmp(line_cp_lock, "CardLock")) {
            cpisct = 1;
        } else if (!strcmp(line_cp_lock, "Not CardLock")) {
            cpisct = 0;
        } else {
            LOGD("unkonw cp_lock version");
            free(dup);
            return 1;
        }
        free(dup);
        LOGD("cp version is: %s, %s, cpisct:%d", line_cp_lock, line_cp_1x,
                cpisct);
        return 1;
    } else if (strStartsWith(s, "+ECENGINFO:")) {
        LOGD("ECENGINFO: %s", s);
        onEngModeInfoUpdate(s);
        return 1;
    } else if (strStartsWith(s, "+ENWINFO:")) {
        onNetworkInfo(s);
        return 1;
    } else if (strStartsWith(s, "+VPON:1")) {
        LOGD("VPON: %s", s);
        RIL_requestProxyTimedCallback(updateRadioStatus, NULL, &TIMEVAL_0, getDefaultChannelCtx()->id);
        return 1;
    } else if (strStartsWith(s, "+ENWKEXISTENCE:")) { //only for ECC Feature
        LOGD("ENWKEXISTENCE: %s", s);
        onNetworkExistence(s);
        return 1;
    }
    return 0;
}

static void onNetworkExistence(char* urc) {
    /* +ENWKEXISTENCE: <exist> */
    char* p = NULL;
    char* dup = NULL;
    char* line = NULL;
    int exsit = 0;
    int err;

    dup = strdup(urc);
    if (dup == NULL) {
        LOGD("onNetworkExistence: memory is leak");
        return;
    }
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
        free(dup);
        return;
    }
    err = at_tok_nextint(&line, &exsit);
    if (err < 0) {
        free(dup);
        return;
    }
    LOGD("onNetworkExistence: exsit:%d", exsit);
    RIL_onUnsolicitedResponse(RIL_UNSOL_NETWORK_EXIST, &exsit, sizeof(int));
    free(dup);
}
void updateRadioStatus(void)
{
    LOGD("updateRadioStatus");
    int sim_status = getRUIMStatus(UPDATE_RADIO_STATUS);
}


static void onNetworkInfo(char* urc) {
    /* +ENWINFO: <type>,<nw_info> */
    int err;
    int type;
    char *responseStr[2];
    char *dup = NULL;
    char *line = NULL;

    *dup = strdup(urc);
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
        free(dup);
        return;
    }

    err = at_tok_nextint(&line, &type);
    if (err < 0) {
        free(dup);
        return;
    }
    asprintf(&responseStr[0], "%d", type);

    // get raw data of structure of NW info
    err = at_tok_nextstr(&line, &(responseStr[1]));
    if (err < 0) {
        free(dup);
        return;
    }

    LOGD("NWInfo: %s, len:%d ,%s", responseStr[0], strlen(responseStr[1]),
            responseStr[1]);
    RIL_onUnsolicitedResponse(RIL_UNSOL_NETWORK_INFO, responseStr,
            sizeof(responseStr));
    free(responseStr[0]);
    free(dup);
}

static void onEngModeInfoUpdate(char* urc)
{
    RIL_Eng_Mode_Info engModeInfo;
    char *tmp_urc = NULL;
    char *cmd = NULL;
    char *info = NULL;

    if (NULL == urc)
    {
        LOGE("ECENGINFO input urc is NULL\r\n");
        return;
    }

    LOGD("ECENGINFO input urc = \"%s\"\r\n", urc);

    memset(&engModeInfo, 0, sizeof(engModeInfo));

    tmp_urc = strdup(urc);
    if (NULL == tmp_urc)
    {
        LOGE("+ECENGINFO: Unable to allocate memory");
        return;
    }

    if (0 != parseEngModeUrc(tmp_urc, &cmd, &info))
    {
        /* processing error here, free tmp_urc first if return */
        LOGE("ECENGINFO invalid urc format\r\n");
        free(tmp_urc);
        return;
    }
    else
    {
        LOGD("ECENGINFO cmd = \"%s\", info = \"%s\"\r\n", cmd, info);
        engModeInfo.at_command= cmd;
        engModeInfo.info = info;
        RIL_onUnsolicitedResponse(RIL_UNSOL_ENG_MODE_NETWORK_INFO, &engModeInfo,
            sizeof(engModeInfo));
        free(tmp_urc); 
    }
}

int parseEngModeUrc(char *urc, char **cmd, char **info)
{
    char *colon;

    if ((NULL == urc) || (NULL == cmd) || (NULL == info))
    {
        return -1;
    }

    *cmd = NULL;
    *info = NULL;

    if ('\0' == urc[0])
    {
        return -1;
    }

    colon = strchr(urc, ':');
    if (NULL == colon)
    {
        return -1;
    }

    if (colon == urc)
    {
        return -1;
    }

    *colon = '\0';
    if ('\0' != *(colon + 1))
    {
        *info = colon + 1;
    }

    *cmd = urc;

    return 0;
}

void requestSignalStrength(void *data, size_t datalen, RIL_Token t)
{
    struct timeval TIMEVAL_SIGNAL_REPOLL = {4, 0};   /* 4 second */
    static int no_signal_times = 0;
    if((s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm > 100)&&(getRadioState() == RADIO_STATE_NV_READY || getRadioState() == RADIO_STATE_RUIM_READY)){
        if(no_signal_times == 0){
            LOGD("no signal first time, return fake value");
            s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = last_cdma_dbm;
        } else {
            LOGD("no signal %d time, return true value", no_signal_times);
            TIMEVAL_SIGNAL_REPOLL.tv_sec = 4*no_signal_times;
        }
        if(!repollinqueue) {
            LOGD("before registrater repollNetworkState!!!!");
            RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_SIGNAL_REPOLL, getDefaultChannelCtx()->id);
            repollinqueue = 1;
            no_signal_times++;
        } else {
            LOGD("There is a repoll signal action in queue");
            s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = last_cdma_dbm;
        }
    }  else {
        no_signal_times = 0;
    }
    last_cdma_dbm = s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm;
    RIL_onRequestComplete(t, RIL_E_SUCCESS,
        &(s_unsol_msg_cache.signal_strength),
        sizeof(RIL_SignalStrength_v6));
}

void requestRegistrationState(int request, void *data,
                                        size_t datalen, RIL_Token t)
{
    RIL_Rgistration_state response;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int skip = 0;
    struct timeval TIMEVAL_NETWORK_REPOLL = {4, 0};   /* 4 second */
    LOGD("requestRegistrationState");
    memset(&response, 0, sizeof(RIL_Rgistration_state));

#if 0
    /*Get System ID and Network ID*/
    err = at_send_command_singleline ("AT+CSNID?", "+CSNID:", &p_response, NW_CHANNEL_CTX);
    if((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.system_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.network_id);
    if (err < 0) goto error;
#else
    /*Get System ID and Network ID*/
    err = at_send_command_singleline ("AT+VLOCINFO?", "+VLOCINFO:", &p_response, NW_CHANNEL_CTX);
    if((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.system_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.network_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_latitude);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_longitude);
    if (err < 0) goto error;
#endif

    #ifdef ADD_MTK_REQUEST_URC
    LOGD("%s: s_unsol_msg_cache.service_state is %d, s_unsol_msg_cache.roaming_indicator is %d"
        ,__func__, s_unsol_msg_cache.service_state, s_unsol_msg_cache.roaming_indicator);
    if((0 == s_unsol_msg_cache.roaming_indicator) && (1 == s_unsol_msg_cache.service_state)) {
        LOGD("in roaming state");
        s_unsol_msg_cache.service_state = 5;
    } else if ( (5 == s_unsol_msg_cache.service_state) && (1 == s_unsol_msg_cache.roaming_indicator)) {
        LOGD("in roaming off state");
        s_unsol_msg_cache.service_state = 1;
    }
    #if 0
    else if ((0 == s_unsol_msg_cache.service_state) && (2 == s_unsol_msg_cache.register_state)
        && (RADIO_STATE_RUIM_LOCKED_OR_ABSENT != getRadioState())) {
        LOGD("in searching state");
        s_unsol_msg_cache.service_state = 2;
    }
    #endif
    #endif

    if(s_unsol_msg_cache.radio_technology == 0&&(getRadioState() == RADIO_STATE_NV_READY || getRadioState() == RADIO_STATE_RUIM_READY)){
        if(no_service_times == 0){
            LOGD("no service first time, return fake value");
            asprintf(&(response.radio_technology), "%d", last_radio_technology);
            asprintf(&(response.register_state), "%d", last_service_state);

            no_service_times++;
            RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL, getDefaultChannelCtx()->id);
            repollinqueue = 1;
        } else {
            if(!repollinqueue){
                LOGD("no service %d time, return true value", no_service_times);
                TIMEVAL_NETWORK_REPOLL.tv_sec = 4*no_service_times;
                asprintf(&(response.radio_technology), "%d", s_unsol_msg_cache.radio_technology);
                asprintf(&(response.register_state), "%d", s_unsol_msg_cache.service_state);
                last_service_state = s_unsol_msg_cache.service_state;
                last_radio_technology = s_unsol_msg_cache.radio_technology;
                LOGD("before registrater repollNetworkState!!!!");
                RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL, getDefaultChannelCtx()->id);
                repollinqueue = 1;
                no_service_times++;
            } else {
                LOGD("There is a repoll action in queue");
                LOGD("last_service_state:%d, last_radio_technology:%d", last_service_state, s_unsol_msg_cache.service_state);
                last_service_state = s_unsol_msg_cache.service_state;
                last_radio_technology = s_unsol_msg_cache.radio_technology;
                asprintf(&(response.radio_technology), "%d", last_radio_technology);
                asprintf(&(response.register_state), "%d", last_service_state);
            }
        }

    } else {
        asprintf(&(response.radio_technology), "%d", s_unsol_msg_cache.radio_technology);
        asprintf(&(response.register_state), "%d", s_unsol_msg_cache.service_state);
        last_service_state = s_unsol_msg_cache.service_state;
        last_radio_technology = s_unsol_msg_cache.radio_technology;
        no_service_times = 0;
    }

    //fix HANDROID#2225, if unlock pin-code more than 3 time,we should set service_state to 0
    if(unlock_pin_outtimes == 1) {
        s_unsol_msg_cache.service_state = 0;
        last_service_state = s_unsol_msg_cache.service_state;
    }

    if (isCdmaLteDcSupport()) {
        if (request == RIL_REQUEST_DATA_REGISTRATION_STATE) {
            asprintf(&(response.register_state), "%d", ps_state);
            asprintf(&(response.radio_technology), "%d", s_unsol_msg_cache.radio_technology);
        } else {
            asprintf(&(response.register_state), "%d", s_unsol_msg_cache.x1_service_state);
            asprintf(&(response.radio_technology), "%d",s_unsol_msg_cache.x1_service_state == 1 ? 6 : 0);
        }
    }

    asprintf(&(response.roaming_indicator), "%d", s_unsol_msg_cache.roaming_indicator);

    response.lac = REGISTRATION_DEFAULT_VALUE;
    response.cid = REGISTRATION_DEFAULT_VALUE;
    //response.basestation_id = REGISTRATION_DEFAULT_VALUE;
    //response.basestation_latitude = REGISTRATION_DEFAULT_VALUE;
    //response.basestation_longitude = REGISTRATION_DEFAULT_VALUE;
    response.concurrent_service = REGISTRATION_DEFAULT_VALUE;
    response.prl_state = REGISTRATION_DEFAULT_PRL_VALUE;
    response.prl_roaming_indicator = REGISTRATION_DEFAULT_VALUE;
    response.deny_reason = DEFAULT_ROAMING_INDICATOR;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Rgistration_state));
    at_response_free(p_response);
    free(response.register_state);
    free(response.radio_technology);
    free(response.roaming_indicator);
    return;
error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

bool isValidMCC(char* mcc) {
   if (mcc != NULL && strcmp(mcc, "000") != 0
       && strcmp(mcc, "N/A") != 0
       && strcmp(mcc, "") != 0) {
       return true;
   }
   LOGD("requestOperator, isValidMCC return false.");
   return false;
}

void requestOperator(void *data, size_t datalen, RIL_Token t)
{
    RIL_Operationinfo response;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    int commas_count = 0;
    char *p = NULL;
    int skip = 0;
    memset(&response, 0, sizeof(response));

    LOGD("%s: uimInsertedStatus is %d, sState is %d",__FUNCTION__,uimInsertedStatus, getRadioState());
    asprintf(&(response.longons), "%s", "");
    asprintf(&(response.shortons), "%s", "");
    if ((UIM_STATUS_NO_CARD_INSERTED == uimInsertedStatus) || (RADIO_STATE_RUIM_READY != getRadioState())) {
        asprintf(&(response.mccmnc), "%s", "");
    } else {
        asprintf(&(response.mccmnc), "%s%s", s_unsol_msg_cache.mcc, s_unsol_msg_cache.mnc);
        LOGD("requestOperator, mccmnc is %s", response.mccmnc);
        if (!isValidMCC(s_unsol_msg_cache.mcc)) {
            int err = at_send_command_singleline("AT+VMCCMNC?", "+VMCCMNC:", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || (p_response->success == 0)) {
                LOGD("requestOperator, error when send at command is %d", err);
                goto error;
            }
            char *dup = strdup(p_response->p_intermediates->line);
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                goto error;
            }
            for (p = line; *p != '\0'; p++) {
                if (*p == ',')
                    commas_count++;
            }
            LOGD("requestOperator, %s, commas_count is %d", line, commas_count);
            switch (commas_count) {
            case 1: /* +VMCCMNC:<MCC>,<MNC> */
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                   free(dup);
                   goto error;
                }
                strcpy(s_unsol_msg_cache.mcc, out);
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strcpy(s_unsol_msg_cache.mnc, out);
                break;
            case 2: /* +VMCCMNC:<MccMnc>,<MCC>,<MNC> */
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strcpy(s_unsol_msg_cache.mcc, out);
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strcpy(s_unsol_msg_cache.mnc, out);
                break;
            default:
                LOGD("requestOperator, unsupport VMCCMNC");
                break;
            }

            asprintf(&(response.mccmnc), "%s%s", s_unsol_msg_cache.mcc, s_unsol_msg_cache.mnc);
            LOGD("requestOperator, mccmnc is %s", response.mccmnc);
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Operationinfo));
    free(response.mccmnc);
    #ifdef ADD_MTK_REQUEST_URC
    free(response.longons);
    free(response.shortons);
#endif
    at_response_free(p_response);
    return;

error:
    free(response.mccmnc);
#ifdef ADD_MTK_REQUEST_URC
    free(response.longons);
    free(response.shortons);
#endif
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestRadioPower(void *data, size_t datalen, RIL_Token t)
{
    int onOff;

    int err;
    ATResponse *p_response = NULL;
    RUIM_Status uimStatus = RUIM_ABSENT;
    #ifdef OPEN_PS_ON_CARD_ABSENT
    int emergencyOpen = 0;
    #endif

    assert (datalen >= sizeof(int *));
    onOff = ((int *)data)[0];

    LOGD("%s: onOff is %d, sState is %d", __FUNCTION__, onOff, getRadioState());
    if (onOff == 0 && getRadioState() != RADIO_STATE_OFF) {
        s_ps_on_desired = 0;
        LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
        if(isRadioOn() == 1) {
            err = at_send_command("AT+CPOF", &p_response, NW_CHANNEL_CTX);
            if (err < 0 || p_response->success == 0) goto error;
        }
        last_flight_mode = 1;
        setRadioState(RADIO_STATE_OFF);
        //fix HANDROID#2352 .
        /* ALPS02034980
        {
            int slotid = 0;
            slotid = getCdmaModemSlot();
             switch (slotid) {
                case 1:
                    property_set("gsm.sim.ril.phbready", "false");
            break;
                case 2:
                    property_set("gsm.sim.ril.phbready.2", "false");
                    break;
                default:
                    LOGE("reportPbSmsReady, unsupport slot id %d", slotid);
                    break;
            }
            LOGD("set phb property to false, slotid:%d", slotid);
        }*/

    } else if (onOff > 0) {
        last_flight_mode = 0;
        s_ps_on_desired = 1;
        LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
        if (getRadioState() != RADIO_STATE_RUIM_READY) {
            if (isRadioOn() != 1) {
                no_service_times = 1; /*Fix HREF#17811, do not return a fake value when PS openning*/

                /*Actually it should be done before CPIN? (getRUIMStatus)*/
                waitUimModualInitialized();
                uimStatus = getRUIMStatus(DO_NOT_UPDATE);
                if ((RUIM_PIN == uimStatus) || (RUIM_PUK == uimStatus)
                        || (RUIM_ABSENT == uimStatus)) {
                    //s_ps_on_desired = 1;
                    //LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
#ifdef OPEN_PS_ON_CARD_ABSENT
                    emergencyOpen = 1;
                    setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
#else
                    goto error;
#endif
                }
#ifdef ADD_MTK_REQUEST_URC
                s_unsol_msg_cache.service_state = 2;
#endif

                // lock frequency for lab test. @{
                char lockFreqProp[PROPERTY_VALUE_MAX] = { 0 };
                property_get("persist.sys.lock.freq.c2k", lockFreqProp, "0");
                int nLockFrep = atoi(lockFreqProp);
                LOGD("Lock C2K frequency: lockFreqProp = %s, nLockFrep = %d.", lockFreqProp, nLockFrep);
                if (nLockFrep == 1) {
                    at_send_command("AT+ELOCKCH=0,1,293", NULL, NW_CHANNEL_CTX);
                    at_send_command("AT+ELOCKCH=1,1,47", NULL, NW_CHANNEL_CTX);
                }
                // lock frequency for lab test. @}

                /// M:attach c2k if needed.
                combineDataAttach(NW_CHANNEL_CTX);

                err = at_send_command("AT+CPON", &p_response, NW_CHANNEL_CTX);
                if (err < 0 || p_response->success == 0) {
                    LOGD("%s: failed to send CPON, err is %d", __FUNCTION__, err);
                    // Some stacks return an error when there is no SIM,
                    // but they really turn the RF portion on
                    // So, if we get an error, let's check to see if it
                    // turned on anyway

                    if (isRadioOn() != 1) {
                        goto error;
                    }
                }

                //fix HANDROID#2352
                /*int slotid = 0;
                slotid = getCdmaModemSlot();
                if (phbready) {
                    switch (slotid) {
                    case 1:
                        property_set("gsm.sim.ril.phbready", "true");
                        break;
                    case 2:
                        property_set("gsm.sim.ril.phbready.2", "true");
                        break;
                    default:
                        LOGE("reportPbSmsReady, unsupport slot id %d", slotid);
                        break;
                    }
                    LOGD("phb ready, slotid:%d", slotid);
                    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION,
                            NULL, 0);
                } else {
                    switch (slotid) {
                    case 1:
                        property_set("gsm.sim.ril.phbready", "false");
                        break;
                    case 2:
                        property_set("gsm.sim.ril.phbready.2", "false");
                        break;
                    default:
                        LOGE("reportPbSmsReady, unsupport slot id %d", slotid);
                        break;
                    }
                    LOGD("phb not ready ,set phb property to false, slotid:%d",
                            slotid);
                }*/

#ifdef OPEN_PS_ON_CARD_ABSENT
                if (!emergencyOpen)
#endif
                {
                    //remove RADIO_STATE_ON, CR#2426
                    setRadioState(RADIO_STATE_RUIM_NOT_READY);
                }
            }
        }
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestModemPowerOff(void *data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    char* line = NULL;

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_cbp_version_response, NW_CHANNEL_CTX);
        if (err < 0 || p_cbp_version_response->success == 0)
        {
            LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            break;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_start", __FUNCTION__);
            break;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
            break;
        }
        if((NULL != cbp_version) &&
                (!strncmp(cbp_version, "CBP8", 4) || !strncmp(cbp_version, "MT6735", 6) || !strncmp(cbp_version, "MT6753", 6)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, NW_CHANNEL_CTX);
            if (err < 0 || p_rfs_response->success == 0)
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while(0);

    if (s_md3_off == 1)
    {
        LOGD("Flight mode power off modem but already modem powered off");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    s_md3_off = 1;

	int slotid = getCdmaModemSlot();
    switch (slotid) {
    case 1:
        property_set("ril.ipo.radiooff", "-1");
        LOGD("C2K set ril.ipo.radiooff to -1");
        break;
    case 2:
        property_set("ril.ipo.radiooff.2", "-1");
        LOGD("C2K set ril.ipo.radiooff.2 to -1");
        break;
    }

    LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
    err = at_send_command("AT+EPOF", &p_response, NW_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) goto error;
    LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);

    switch (slotid) {
        case 1:
            property_set("gsm.sim.ril.phbready", "false");
			property_set("ril.ipo.radiooff", "1");
			LOGD("C2K set ril.ipo.radiooff to 1");
            break;
        case 2:
            property_set("gsm.sim.ril.phbready.2", "false");
			property_set("ril.ipo.radiooff.2", "1");
			LOGD("C2K set ril.ipo.radiooff.2 to 1");
            break;
        default:
            LOGE("reportPbSmsReady, unsupport slot id %d", slotid);
            break;
    }
    LOGD("%s: set phb property to false, slotid:%d", __FUNCTION__, slotid);

    last_flight_mode = 1;
    setRadioState(RADIO_STATE_OFF);
    LOGD("%s: setRadioState to RADIO_STATE_OFF.", __FUNCTION__);


    LOGD("%s: enter flight mode.", __FUNCTION__);
    triggerIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestModemPowerOn(void *data, size_t datalen, RIL_Token t)
{
    LOGD("%s: leave flight mode.", __FUNCTION__);

    if (s_md3_off == 0)
    {
        LOGD("Flight mode power on modem but modem is already power on");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    s_md3_off = 0;

    LOGD("%s: leave flight mode.", __FUNCTION__);
    triggerIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
    int slotid = getCdmaModemSlot();
    switch (slotid) {
        case 1:
			property_set("ril.ipo.radiooff", "0");
			LOGD("C2K set ril.ipo.radiooff to 0");
            break;
        case 2:
			property_set("ril.ipo.radiooff.2", "0");
			LOGD("C2K set ril.ipo.radiooff.2 to 0");
            break;
        default:
            LOGE("requestModemPowerOn, unsupport slot id %d", slotid);
            break;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
}

void requestGetIMEI(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    err = at_send_command_singleline ("AT+GSN", "+GSN:", &p_response, NW_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    LOGD("requestGetIMEI:  before at_tok_start line = %s",line);
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextstr(&line, &responseStr);
    LOGD("requestGetIMEI: responseStr = %s",responseStr);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/* repoll the registration state, while found no service */
static void repollNetworkState(void *param)
{
    LOGD("In %s, submit unsol network state change", __func__);
    repollinqueue = 0;
    trigger_update_cache();
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, \
            NULL, 0);
}

void requestIPORadioPowerOn(void *data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_card_response = NULL;
    RUIM_Status uimStatus = RUIM_ABSENT;
    #ifdef OPEN_PS_ON_CARD_ABSENT
    int emergencyOpen = 0;
    #endif
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    int ret = 0;

    LOGD("%s: current radio state is %d", __FUNCTION__, getRadioState());
    int slotid = getCdmaModemSlot();
    switch (slotid) {
    case 1:
        property_set("ril.ipo.radiooff", "0");
        LOGD("requestIPORadioPowerOn set  ril.ipo.radiooff 0");
    case 2:
        property_set("ril.ipo.radiooff.2", "0");
        LOGD("requestIPORadioPowerOn set  ril.ipo.radiooff.2 0");
    default:
        LOGD("can not get right value from getCdmaModemSlot");
        goto error;
    }

    err = at_send_command("AT+VRESETUIM", &p_card_response, NW_CHANNEL_CTX);
    if (err < 0 || p_card_response->success == 0)
    {
        LOGD("%s: failed to send VRESETUIM, err is %d", __FUNCTION__, err);
        goto error;
    }
    uim_modual_initialized = 0;
    phbready = 0;
    LOGD("%s: uim_modual_initialized = %d", __FUNCTION__, uim_modual_initialized);
    LOGD("%s: phbready = %d", __FUNCTION__, phbready);

    if(last_flight_mode != 1)
    {
        waitUimModualInitialized();
        if(isRadioOn() != 1)
        {
            no_service_times = 1;   /*Fix HREF#17811, do not return a fake value when PS openning*/

            uimStatus = getRUIMStatus(DO_NOT_UPDATE);
            if((RUIM_PIN == uimStatus) || (RUIM_PUK == uimStatus) || (RUIM_ABSENT == uimStatus)) {
                #ifdef OPEN_PS_ON_CARD_ABSENT
                emergencyOpen = 1;
                setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
                #else
                /*as framework would send radio power off and on after ipo radio poweron, so do not enable ps or
                set radio state to locked_or_absent in ipo radio poweron when pin locked. Otherwise, pin interface
                would be open twice as radio state would be set to radio_off in radio power off request*/
                //setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
                goto error;
                #endif
            }

            #ifdef ADD_MTK_REQUEST_URC
            s_unsol_msg_cache.service_state = 2;
            #endif
            /// M:attach c2k if needed.
            combineDataAttach(NW_CHANNEL_CTX);
            err = at_send_command("AT+CPON", &p_response, NW_CHANNEL_CTX);
            if (err < 0|| p_response->success == 0)
            {
                LOGD("%s: failed to send CPON, err is %d", __FUNCTION__, err);
                // Some stacks return an error when there is no SIM,
                // but they really turn the RF portion on
                // So, if we get an error, let's check to see if it
                // turned on anyway

                if (isRadioOn() != 1)
                {
                    goto error;
                }
            }

            #ifdef OPEN_PS_ON_CARD_ABSENT
            if (!emergencyOpen)
            #endif
            {
                setRadioState(RADIO_STATE_RUIM_NOT_READY);
            }
        }
    }

    at_response_free(p_response);
    at_response_free(p_card_response);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    at_response_free(p_card_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/** returns 1 if on, 0 if off, and -1 on error */
int isRadioOn()
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    char display_en, ps_en;
    LOGD("isRadioOn");
    err = at_send_command_singleline("AT+VPON?", "+VPON:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &display_en);
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &ps_en);
    if (err < 0) goto error;

    at_response_free(p_response);

    return (int)ps_en;

error:

    at_response_free(p_response);
    return -1;
}

static void requestQueryNetworkSelectionMode(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line = NULL;

    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response, NW_CHANNEL_CTX);

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
    LOGE("requestQueryNetworkSelectionMode must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestBasebandVersion(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * response = NULL;
    char *response_hard = NULL;
    char* line = NULL;
    ATResponse *p_response_soft = NULL;
    char * response_sof = NULL;
    char * response_string1 = NULL;
    char * response_string2 = NULL;
    char * response_string3 = NULL;
    char * response_string4 = NULL;
    char * response_string5 = NULL;
    char * response_string6 = NULL;
    char * response_string7 = NULL;
    char * response_string8 = NULL;
    char * response_string11 = NULL;
    char * line_sof = NULL;
    ATResponse *p_response_cp = NULL;

    err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_response, NW_CHANNEL_CTX);
    if ((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &response_hard);
    if (err < 0)
    {
        goto error;
    }

    err = at_send_command_singleline("AT+VCUSVER?", "+VCUSVER", &p_response_soft, NW_CHANNEL_CTX);
    if ((err != 0) || (p_response_soft->success == 0))
    {
        goto error;
    }

    line_sof = p_response_soft->p_intermediates->line;
    err = at_tok_start(&line_sof);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextstr(&line_sof, &response_string1);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string2);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string3);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string4);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string5);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string6);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string7);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string8);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string11);
    if (err < 0) goto error;

    #ifndef ADD_MTK_REQUEST_URC
    asprintf(&response, "Hw: %s  Sw %s Month: %s Date: %s Time %s:%s", response_hard, response_string11,response_string4,response_string5,response_string7,response_string8);
    #else
    asprintf(&response, "Hw: %s  Sw:%s, 20%s/%s/%s %s:%s",
                response_hard, response_string11, response_string6,
                response_string4, response_string5, response_string7,
                response_string8);
    #endif
    LOGD("SVER response = %s, %s",response, response_string11);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
    at_response_free(p_response);
    at_response_free(p_response_soft);
    free(response);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_response_soft);
    LOGE("ERROR: requestBasebandVersion failed\n");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int mode = ((int*)data)[0];
    int value = 0;
    char *cmd;
    switch(mode){
        case NETWORK_MODE_CDMA_NO_EVDO:
            value = 2;
            break;
        case NETWORK_MODE_EVDO_NO_CDMA:
            value = 4;
            break;
        case NETWORK_MODE_HYBRID:
            value = 8;
            break;
        default:
            goto error;
    }
    asprintf(&cmd, "AT^PREFMODE=%d", value);
    err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);
    free(cmd);

    if (err != 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}


static void requestGetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int mode = 0;
    int response = 0;
    err = at_send_command_singleline ("AT^PREFMODE?", "^PREFMODE:", &p_response, NW_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextint(&line, &mode);
    if (err < 0)
    {
        goto error;
    }
    LOGD("mode = %d",mode);
    switch(mode){
        case 2:
            LOGD("CDMA mode");
            response = NETWORK_MODE_CDMA_NO_EVDO;
            break;
        case 4:
            LOGD("HDR mode");
            response = NETWORK_MODE_EVDO_NO_CDMA;
            break;
        case 8:
            LOGD("CDMA/HDR HYBRID mode");
            response = NETWORK_MODE_HYBRID;
            break;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestIPORadioPowerOff(void *data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_card_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    char* line = NULL;
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    int ret = 0;

    LOGD("%s: current radio state is %d", __FUNCTION__, getRadioState());

    if(isRadioOn() == 1)
    {
        err = at_send_command("AT+CPOF", &p_response, NW_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0)
        {
            LOGD("%s: failed to send CPOF, err is %d", __FUNCTION__, err);
            goto error;
        }
    }
    setRadioState(RADIO_STATE_OFF);

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_cbp_version_response, NW_CHANNEL_CTX);
        if (err < 0 || p_cbp_version_response->success == 0)
        {
            LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            break;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_start", __FUNCTION__);
            break;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
            break;
        }
        if((NULL != cbp_version) && (!strncmp(cbp_version, "CBP8", 4)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, NW_CHANNEL_CTX);
            if (err < 0 || p_rfs_response->success == 0)
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while(0);

    err = at_send_command("AT+VTURNOFFUIM", &p_card_response, NW_CHANNEL_CTX);
    if (err < 0 || p_card_response->success == 0)
    {
        LOGD("%s: failed to send VTURNOFFUIM, err is %d", __FUNCTION__, err);
        goto error;
    }

    int slotid = getCdmaModemSlot();
    switch (slotid) {
    case 1:
        property_set("ril.ipo.radiooff", "1");
        LOGD("requestIPORadioPowerOFF set  ril.ipo.radiooff 1");
    case 2:
        property_set("ril.ipo.radiooff.2", "1");
        LOGD("requestIPORadioPowerOFF set  ril.ipo.radiooff.2 1");
    default:
        LOGD("can not get right value from getCdmaModemSlot");
        goto error;
    }

    /*Clear this property to prevent auto-setting old pin code when next IPO power up*/
    property_set("net.cdma.pc", "none");
    at_response_free(p_response);
    at_response_free(p_card_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    at_response_free(p_card_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestGetNitzTime(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err = 0;
    char *line = NULL;
    char * s = NULL;
    char *response = NULL;

    err = at_send_command_singleline("AT+CCLK?", "+CCLK:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    s = strdup(line);
    response = s;
    err = at_tok_start(&response);
    if (err < 0) goto error;


    //err = at_tok_nextstr(&s, &response);
    //if (err < 0) goto error;
    //LOGD("requestGetNitzTime s = %s",s);
    //LOGD("requestGetNitzTime response = %s",response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
    at_response_free(p_response);
    free(s);
    return;
error:
    if (s != NULL)
    {
        free(s);
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestQueryNetworkRegistration(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    int len = 0;
    int skip = 0;
    int registrationStatus = 0; //0: not registered, 1: registered
    char *line = NULL;
    ATResponse *p_response = NULL;

    err = at_send_command_singleline("AT+CREG?", "+CREG:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        LOGE("%s:  err  = %d", __FUNCTION__, err );
        goto error;
    }

    line = p_response->p_intermediates->line;

    len = strlen(line);
    LOGD("%s: line is %s, len is %d", __FUNCTION__, line, len);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("%s: at_tok_start err", __FUNCTION__);
        goto error;
    }

    if(len > 7)
    {
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            LOGE("%s: at_tok_nextint err, skip is %d", __FUNCTION__, skip);
            goto error;
        }
    }

    err = at_tok_nextint(&line, &registrationStatus);
    if (err < 0) {
        LOGE("%s: at_tok_nextint err, registrationStatus is %d", __FUNCTION__, registrationStatus);
        goto error;
    }

    if((1 == registrationStatus) || (5 == registrationStatus))
    {
        registrationStatus = 1;
    }
    else
    {
        registrationStatus = 0;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &registrationStatus, sizeof(registrationStatus));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("%s: return error = %d", __FUNCTION__, err);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/// M: [C2K][IR] Support SVLTE IR feature. @{

static void requestResumeCdmaRegistration(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;

    err = at_send_command("AT+REGRESUME", &p_response, NW_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("%s: send at error is %d", __FUNCTION__, err);
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetCdmaRegSuspendEnabled(void *data, size_t datalen, RIL_Token t)
{
    int value;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;

    value = ((int *)data)[0];

    LOGD("%s: value is %d", __FUNCTION__, value);

    if(1 == value)
    {
        if(!isCdmaRegSuspendEnabled())
        {
            err = at_send_command("AT+VREGCTR=1", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || (p_response->success == 0))
            {
                LOGE("%s: send at error", __FUNCTION__);
                goto error;
            }
        }
    }
    else if (0 == value)
    {
        if(1 == isCdmaRegSuspendEnabled())
        {
            err = at_send_command("AT+VREGCTR=0", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || (p_response->success == 0))
            {
                LOGE("%s: send at error", __FUNCTION__);
                goto error;
            }
        }
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/*
+VREGCTR=<enable>   OK
+VREGCTR?   + VREGCTR: < enable >
+VREGCTR=?  +VREGCTR: (0 - 1)

<enable>:
0   disable register pause function
1   enable register pause function
*/
static int isCdmaRegSuspendEnabled(void)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int enable = -1;

    err = at_send_command_singleline ("AT+VREGCTR?", "+VREGCTR:", &p_response, getChannelCtxbyId(AT_CHANNEL));
    if ((err < 0) || (p_response->success == 0))
    {
        LOGD("%s: failed to send +VREGCTR?, err = %d", __FUNCTION__, err);
        goto exit;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        LOGD("%s: failed to at_tok_start", __FUNCTION__);
        goto exit;
    }
    err = at_tok_nextint(&line, &enable);
    if (err < 0)
    {
        LOGD("%s: failed to at_tok_nextint", __FUNCTION__);
        goto exit;
    }
    LOGD("%s: enable = %d", __FUNCTION__, enable);

exit:
    at_response_free(p_response);
    return enable;
}

/// M: [C2K][IR] Support SVLTE IR feature. @}

/*
*description:framework send the expected value to set cp report the varity of rssi
*ril only send the value between 0~31
*/
static void requestSetArsiThreshold(void *data, size_t datalen, RIL_Token t)
{
    char *cmd = NULL;
    int err = 0;

    arsithrehd = ((int *)data)[0];
    LOGD("the expected ARSI value is %d", arsithrehd);

    if(arsithrehd < 0) {
        arsithrehd = 0;
    } else if (arsithrehd > 31) {
        arsithrehd = 31;
    }
    LOGD("the factual ARSI value is %d", arsithrehd);

    asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
    err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);       //ignore the result of this command
    if(err < 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

#ifdef ANDROID_KK
// TODO: Use all radio types
static int techFromModemType(int mdmtype)
{
    int ret = -1;
    switch (1 << mdmtype) {
        case MDM_CDMA:
            ret = RADIO_TECH_1xRTT;
            break;
        case MDM_EVDO:
            ret = RADIO_TECH_EVDO_A;
            break;
        case MDM_GSM:
            ret = RADIO_TECH_GPRS;
            break;
        case MDM_WCDMA:
            ret = RADIO_TECH_HSPA;
            break;
        case MDM_LTE:
            ret = RADIO_TECH_LTE;
            break;
    }
    return ret;
}

static void setRadioTechnology(ModemInfo *mdm, int newtech)
{
    LOGD("setRadioTechnology(%d)", newtech);

    int oldtech = TECH(mdm);

    if (newtech != oldtech) {
        LOGD("Tech change (%d => %d)", oldtech, newtech);
        TECH(mdm) = newtech;
        if (techFromModemType(newtech) != techFromModemType(oldtech)) {
            int tech = techFromModemType(TECH(sMdmInfo));
            if (tech > 0 ) {
                RIL_onUnsolicitedResponse(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,
                                          &tech, sizeof(tech));
            }
        }
    }
}
#endif /* ANDROID_KK */

static void setNetworkServiceProperty(VIA_NETWORK_TYPE type)
{
    switch (type) {
        case NET_TYPE_NO_SERVICE:
            property_set("net.cdma.via.service.state", "out of service");
            break;
        case NET_TYPE_CDMA_ONLY:
        case NET_TYPE_EVDO_ONLY:
        case NET_TYPE_CDMA_EVDO_HYBRID:
        default:
            property_set("net.cdma.via.service.state", "in service");
            break;
    }
}

static void handleEmergencyExit(void *param)
{
    LOGD("%s: enter", __FUNCTION__);
    RUIM_Status uimStatus = RUIM_ABSENT;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));

    uimStatus = getRUIMStatus(DO_NOT_UPDATE);
    if((RUIM_PIN == uimStatus) || (RUIM_PUK == uimStatus) || (RUIM_ABSENT == uimStatus)) {
        turnPSEnable((void *)&psParam);
        setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
    }
    LOGD("%s: leave", __FUNCTION__);
}

static void queryIccid(void *param) {
    int err;
    char *iccid_line = NULL;
    char *iccidStr = NULL;
    ATResponse *p_response = NULL;
    char plugstate[PROPERTY_VALUE_MAX] = { 0 };
    char cardstate[PROPERTY_VALUE_MAX] = { 0 };

    err = at_send_command_singleline("AT+VICCID?", "+VICCID:", &p_response, getChannelCtxbyId(AT_CHANNEL));
    if ((err < 0) || (p_response->success == 0)) {
        LOGD("failed to get iccid from uim card!!!");
    } else {
        iccid_line = p_response->p_intermediates->line;

        err = at_tok_start(&iccid_line);
        if (err < 0) {
            LOGD("failed to at_tok_start!!");
            goto done;
        }

        err = at_tok_nextstr(&iccid_line, &iccidStr);
        if (err < 0) {
            LOGD("failed to at_tok_nextstr!!");
            goto done;
        }

        //add for hotplug, if GSM card, needn't to set iccid.
        if (cardtype == SIM_CARD) {
            LOGD("GSM card, abandon to set ICCID");
        } else {
            setIccidProperty(SET_VALID_ICCID, iccidStr);
        }
        /*add for MTK6582 W+C SIM Hot Plug*/
        property_get("net.cdma.via.card.state", cardstate, "");
        property_get("ril.cdma.card.plugin", plugstate, "");
        /*if plugstate is true && cardstate is konwn,so it isn't a pin lock card, send unsol message after socker connected*/
        LOGD("%s: cardstate:%s, plugstate:%s", __FUNCTION__, cardstate,
                plugstate);
        if ((!strcmp(plugstate, "true")) && (!strcmp(cardstate, "valid"))) {
            if ((cardtype == UIM_CARD) || (cardtype == UIM_SIM_CARD)
                    || (cardtype == CT_3G_UIM_CARD)
                    || (cardtype == CT_UIM_SIM_CARD)
                    || (cardtype == CT_4G_UICC_CARD)) {
                LOGD("Cdma Card Hot Plug In");
                pluguinfo = 1; //uim card plug in
                if (rildSockConnect) //send URC to framework until rild-via socket connected
                {
                    RIL_requestProxyTimedCallback(reportCdmaSimPlugIn, NULL,
                            &TIMEVAL_0, getDefaultChannelCtx()->id);
                    uimpluginreport = 1;
                } else {
                    uimpluginreport = 0;
                }
                goto done;
            } else if (cardtype == SIM_CARD) {
                LOGD("GSM Card Hot Plug In");
                plugsinfo[0] = 1; //gsm card
                plugsinfo[1] = 0; //plug in
                if (rildSockConnect) {
                    RIL_requestProxyTimedCallback(reportGSMSimPlugIn, NULL,
                            &TIMEVAL_0, getDefaultChannelCtx()->id);
                    gsmpluginreport = 1;
                } else {
                    gsmpluginreport = 0;
                }
                goto done;
            }
            property_set("ril.cdma.card.plugin", "false");
        }

        /*if slot2 inserted gsm card in startup, and cp is  (!CT) version, send URC to framework to trigger phone switch */
        if ((strcmp(plugstate, "true")) && (cpisct == 0)
                && (cardtype == SIM_CARD)) {
            plugsinfo[0] = 1; //gsm card
            plugsinfo[1] = 1; //power on
            LOGD("GSM Card Insert in startup");
            if (rildSockConnect) {
                RIL_requestProxyTimedCallback(reportGSMSimPlugIn, NULL, &TIMEVAL_0, getDefaultChannelCtx()->id);
                gsmpluginreport = 1;
            } else {
                gsmpluginreport = 0;
            }
        }

    }

    done: at_response_free(p_response);
    p_response = NULL;
}

#ifdef AT_AUTO_SCRIPT_MODE
static void SetModeUnreported(void* param)
{
    at_send_command("AT+MODE=0", NULL, getChannelCtxbyId(AT_CHANNEL));
}
#endif

/*
Test command:
AT+VAVOIDSYS=?
Response:
OK

read command:
AT+VAVOIDSYS?
Response: +VAVOIDSYS: <Number>,<SID1>,<SID1>,...\r\n

Write command:
AT+ VAVOIDSYS =<value>
Response:
1) If successful:
OK
2) If failed
ERROR

Parameters:
<value>   1   Add current system to the aovid system list.
                 0   clear the aovid system list.
*/
static void requestSetAvoidSys(void *data, size_t datalen, RIL_Token t)
{
    int value;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;

    value = ((int *)data)[0];

    LOGD("%s: value is %d", __FUNCTION__, value);
    asprintf(&cmd, "AT+VAVOIDSYS=%d", value);
    err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("%s: send at error", __FUNCTION__);
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestQueryAvoidSys(void *data, size_t datalen, RIL_Token t)
{
    int i = 0;
    int err = 0;
    int num = 0;
    int *avoidSysList = NULL;
    char *line = NULL;
    ATResponse *p_response = NULL;

    err = at_send_command_singleline("AT+VAVOIDSYS?", "+VAVOIDSYS:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response==NULL || p_response->success == 0)
    {
        LOGE("%s: send at error", __FUNCTION__);
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        LOGE("%s at_tok_start err", __FUNCTION__);
        goto error;
    }

    //get num
    err = at_tok_nextint(&line, &num);
    if (err < 0 || num < 0)
    {
        LOGE("%s at_tok_nextint err = %d, num = %d", __FUNCTION__, err, num);
        goto error;
    }

    avoidSysList = (int *)alloca((num + 1) * sizeof(int));
    avoidSysList[0] = num;
    for(i = 0; i < num; i ++)
    {
        err = at_tok_nextint(&line, &avoidSysList[1 + i]);
        if (err < 0)
        {
            LOGE("%s at_tok_nextint err = %d, i = %d", __FUNCTION__, err, i);
            goto error;
        }
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, avoidSysList, (num + 1) * sizeof(int));

    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestQueryNetworkInfo(void *data, size_t datalen, RIL_Token t)
{
    RIL_Netwrok_Info response;
    int err = 0;
    ATResponse *p_vlocinfo_response = NULL;
    ATResponse *p_prl_response = NULL;
    char *line = NULL;
    int skip = 0;
    int i = 0;

    memset(&response, 0, sizeof(RIL_Netwrok_Info));

    err = at_send_command_singleline ("AT+VLOCINFO?", "+VLOCINFO:", &p_vlocinfo_response, NW_CHANNEL_CTX);
    if((err != 0) || (p_vlocinfo_response->success == 0))
    {
        LOGD("%s: failed to send +VLOCINFO, err = %d", __FUNCTION__, err);
        goto error;
    }
    line = p_vlocinfo_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    for(i = 0; i< 3; i++)
    {
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
    }
    err = at_tok_nextstr(&line, &response.system_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.network_id);
    if (err < 0) goto error;

    line = NULL;
    err = at_send_command_singleline ("AT+VPRLID?", "+VPRLID:", &p_prl_response, NW_CHANNEL_CTX);
    if((err != 0) || (p_prl_response->success == 0))
    {
        LOGD("%s: failed to send +PRL, err = %d", __FUNCTION__, err);
        goto error;
    }
    line = p_prl_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.prl_version);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Netwrok_Info));
    at_response_free(p_vlocinfo_response);
    at_response_free(p_prl_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_vlocinfo_response);
    at_response_free(p_prl_response);
}

/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t) {
    char* cmd;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    int err = 0;

    int preRoaming = ((int *) data)[3];
    int roaming = ((int *) data)[4];

    LOGD("c2k requestSetSvlteRatMode(), preRoaming =%d, roaming=%d.",preRoaming, roaming);

    // switch UTK/STK mode.
    err = switchStkUtkMode((roaming == ROAMING_MODE_HOME || roaming == ROAMING_MODE_JPKR_CDMA) ? 1 : 0, t);

    if(err >= 0) {
        if(preRoaming != roaming || roaming == 1) {
            //radio off
            err = asprintf(&cmd, "AT+CPOF");
            LOGD("c2k requestSetSvlteRatMode(), send command %s.", cmd);
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
            setRadioState(RADIO_STATE_OFF);
        }
    }

    LOGD("c2k requestSetSvlteRatMode(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}

/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @{
int getRatSwitching() {
    char switching[PROPERTY_VALUE_MAX] = { 0 };
    int switchingVal = 0;

    property_get("ril.rat.switching", switching, "0");
    switchingVal = atoi(switching);
    LOGD("getRatSwitching, switchingVal is %d", switchingVal);
    return switchingVal;
}
/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @}

