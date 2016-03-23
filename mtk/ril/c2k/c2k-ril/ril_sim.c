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
#include <stdlib.h>
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
#include "icc.h"
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

#define SIM_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define CDMA_UIM_ALPHAID_LENGTH 14
#define CDMA_UIM_NUMBER_LENGTH 20

extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int s_ps_on_desired;
extern int uim_modual_initialized;
extern int cardtype;
extern int rildSockConnect;
extern const struct timeval TIMEVAL_0;
extern int gsmpluginreport;
extern int cpisct;
extern int uimInsertedStatus;
extern int invalidSimReported;
extern int cardtypereport;
extern int uimInsertedReported;
extern int unlock_pin_outtimes;
extern int uimpluginreport;
extern int no_service_times;
extern int plugsinfo[2];
extern int pluguinfo;

int phbready = 0;

static int isFirstReport = 1;

/**
 * When access UIM card PHB, we need check the PHB params frequently,
 * so we add 3 static global variables to remeber the params
 * of UIM card PHB, when use them, we can get them directly,
 * and dont need to send AT commands to modem .
 *
 * Use static global variables instead of using system property.[2013-11-8]
 *
 */
// the max count the UIM card can save
static int s_PHB_max_capacity;
// the max bytes count of the number field
static int s_PHB_number_max_bytes;
// the max bytes count of the alphaid(name) field
static int s_PHB_alphaid_max_bytes;

void reportRuimStatus(void* param);
void reportInvalidSimDetected(void);
void reportCdmaSimPlugIn(void);
void reportGSMSimPlugIn(void);
void reportCardType (void);

static int isSimPinEnable(RIL_Token  t);
static void requestEnterSimPin(void*  data, size_t  datalen, RIL_Token  t);
static void requestEnterSimPuk(void*  data, size_t  datalen, RIL_Token  t);
static void requestSetFacilityLock(void *data, size_t datalen, RIL_Token t);
static void requestGetIMSI(void *data, size_t datalen, RIL_Token t);
static void requestSIM_IO(void *data, size_t datalen, RIL_Token t);
/* Adapt MTK Multi-Application, 2015/01/10 {*/
static void requestSimIoEx(void *data, size_t datalen, RIL_Token t);
/* Adapt MTK Multi-Application, 2015/01/10 }*/
static void requestQueryFacilityLock(void *data, size_t datalen, RIL_Token t);
static void requestSubscription(void *data, size_t datalen, RIL_Token t);
static void requestSendUTKURCREG(void* param);
static int getCardStatus(VIA_RIL_CardStatus **pp_card_status);
static void freeCardStatus(VIA_RIL_CardStatus *p_card_status);
static void getChvCount(int * pin1Count, int * pin2Count, int * puk1Count, int * puk2Count);
static void storePinCode(const char*  string );
static int isRildReset(void);
static void setCardStateProperty(VIA_CARD_STATE state);
static void requestMTKGetPbStorageInfoFromRuim(void *data, size_t datalen, RIL_Token t);
static void requestWriteMTKPbToRuim(void *data, size_t datalen, RIL_Token t);
static void requestReadMTKPbFromRuim(void *data, size_t datalen, RIL_Token t);
static int checkNameLength(char *name);
static unsigned int getMTKPbAlphaIDMaxLength();
static void readAllUnreadSmsOnME(void);
static int parsePb(char *line, RIL_Ruim_Phonebook_Record *readPb);
static void requestReadPbFromRuim(void *data, size_t datalen, RIL_Token t);
static void requestWritePbToRuim(void *data, size_t datalen, RIL_Token t);
static void requestGetPbSizeFromRuim(void *data, size_t datalen, RIL_Token t);
static int getPbSize(int pbSize[]);

/***********C+W begin***********/
static void requestUpdateSSD(void *data, size_t datalen, RIL_Token t);
static void requestUpdateSSDCON(void *data, size_t datalen, RIL_Token t);
static void requestGenerateKEY(void *data, size_t datalen, RIL_Token t);
static void requestGetUimid(void *data, size_t datalen, RIL_Token t);
static void requestGetPREFMODE(void *data, size_t datalen, RIL_Token t);
static void requestMakeMD5(void *data, size_t datalen, RIL_Token t);
static void requestMakeCAVE(void *data, size_t datalen, RIL_Token t);
static void requestGetUIMAUTH(void *data, size_t datalen, RIL_Token t);
static void requestReadIMSI(void *data, size_t datalen, RIL_Token t);
static void itoa(char *buf, int base, int d);
/***********C+W begin***********/


#ifdef ADD_MTK_REQUEST_URC
static void requestGetICCID(void *data, size_t datalen, RIL_Token t);
static void requestSimGetATRFORNFC(void *data, size_t datalen, RIL_Token t);
static void requestSimOpenNFCChannel(void *data, size_t datalen, RIL_Token t);
static void requestSimOpenNFCChannelWithSw(void *data, size_t datalen, RIL_Token t);
static void requestSimCloseNFCChannel(void *data, size_t datalen, RIL_Token t);
static void requestCardSwitch(void *data, size_t datalen, RIL_Token t);
static void requestQueryUimInserted(void *data, size_t datalen, RIL_Token t);
static void requestSetMEID(void *data, size_t datalen, RIL_Token t);
static void requestWriteMDN(void *data, size_t datalen, RIL_Token t);
static void requestNotifySimHotPlug(void *data, size_t datalen, RIL_Token t);
static void requestSimTransmitBasic(void *data, size_t datalen, RIL_Token t);
static void requestSimTransmitChannel(void *data, size_t datalen, RIL_Token t);
static int requestCGLA_OpenChannelWithSw(RIL_SIM_IO_Response *sr, int *len,
        int NFCchannel, int length, int cla, int ins, int p1, int p2, int p3,
        char **data, RIL_Token t);
static int requestSimChannelAccess(int sessionid, char *senddata, RIL_SIM_IO_Response *output);
static int getExternalModemSlotTelephonyMode(void);
static void setGsmRilUicctype(void);
static void setMTKPbStorageInfo(int max, int numberMax, int alphaMax);
static int getMTKPbMaxCount();
#endif

/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
static bool isNeedToSetRadio(VIA_CARD_STATE card_type);
static void TriggerMD3BootPower();
/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/

/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
static SimPowerController s_sim_power_ctrl = {
        CARD_NOT_INSERTED,
        _FALSE,
        updateCardState,
        setSimPower,
        refreshSimPower,
        triggerSetSimPower};
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

int rilSimMain(int request, void *data, size_t datalen, RIL_Token t)
{
    LOGD("rilSimMain enter");
    switch (request) {
        case RIL_REQUEST_GET_SIM_STATUS: {
            VIA_RIL_CardStatus *p_card_status;
            RIL_CardStatus_v6 *p_oem_card_status = NULL;
            char *p_buffer;
            int buffer_size;
            char tmp[PROPERTY_VALUE_MAX] = {0};
            char tmp2[PROPERTY_VALUE_MAX] = {0};

            property_get(PROPERTY_RIL_CT3G[getCdmaSocketSlotId()-1], tmp, "");
            property_get(PROPERTY_RIL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp2, "");
            RLOGD("ct3g: %s, uicc type: %s", tmp, tmp2);

            if ((strcmp("1", tmp) == 0) && ((strcmp("CSIM", tmp2) != 0) && (strcmp("RUIM", tmp2) != 0))) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                break;
            }

            int result = getCardStatus(&p_card_status);
            if (result == RIL_E_SUCCESS) {
                if (1/*oemSupportEnable(g_oem_support_flag) && oemCardStatusSupportEnalbe(g_oem_support_flag)*/) {
                    dispatch_oem_card_status(p_card_status, &p_oem_card_status);
                    p_buffer = (char *) p_oem_card_status;
                    buffer_size = sizeof(*p_oem_card_status);
                } else {
                    p_buffer = (char *) p_card_status;
                    buffer_size = sizeof(*p_card_status);
                }
            } else {
                p_buffer = NULL;
                buffer_size = 0;
            }
            RIL_onRequestComplete(t, result, p_buffer, buffer_size);
            freeCardStatus(p_card_status);
            if (p_oem_card_status != NULL) {
                free(p_oem_card_status);
                p_oem_card_status = NULL;
            }
            break;
        }
        case RIL_REQUEST_ENTER_SIM_PIN:
        case RIL_REQUEST_ENTER_SIM_PIN2:
        case RIL_REQUEST_ENTER_SIM_PUK2:
        case RIL_REQUEST_CHANGE_SIM_PIN:
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            requestEnterSimPin(data, datalen, t);
            break;
        case RIL_REQUEST_ENTER_SIM_PUK:
            requestEnterSimPuk(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMSI:
            requestGetIMSI(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_IO:
            requestSIM_IO(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
            requestQueryFacilityLock(data, datalen, t);
            break;
        case RIL_REQUEST_SET_FACILITY_LOCK:
            requestSetFacilityLock(data, datalen, t);
           break;
        case RIL_REQUEST_CDMA_SUBSCRIPTION:
            requestSubscription(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
            requestSimTransmitBasic(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
            requestSimTransmitChannel(data, datalen, t);
            break;
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_SIM_TRANSMIT_BASIC:
            requestSimTransmitBasic(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
            requestSimOpenNFCChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
            requestSimCloseNFCChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_CHANNEL:
            requestSimTransmitChannel(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_ICCID:
            requestGetICCID(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_GET_ATR:
            requestSimGetATRFORNFC(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW:
            requestSimOpenNFCChannelWithSw(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWER_CARD_SWITCH:
            requestCardSwitch(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_UIM_INSERTED:
            requestQueryUimInserted(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MEID:
            requestSetMEID(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_MDN:
            requestWriteMDN(data, datalen, t);
            break;
        case RIL_REQUEST_NOTIFY_SIM_HOTPLUG:
            requestNotifySimHotPlug(data, datalen, t);
            break;
        /* Adapt MTK Multi-Application, 2015/01/10 {*/
        case RIL_REQUEST_SIM_IO_EX:
            requestSimIoEx(data, datalen, t);
        break;
        /* Adapt MTK Multi-Application, 2015/01/10 }*/
#endif
        /************************C+W begin***********************/
        case RIL_REQUEST_READ_IMSI:
            requestReadIMSI(data, datalen, t);
            break;
         case RIL_REQUEST_GET_UIMAUTH:
            requestGetUIMAUTH(data, datalen, t);
            break;
         case RIL_REQUEST_MAKE_CAVE:
            requestMakeCAVE(data, datalen,t);
            break;
         case RIL_REQUEST_MAKE_MD5:
            requestMakeMD5(data, datalen,t);
            break;
         case RIL_REQUEST_GET_PREFMODE:
            requestGetPREFMODE(data, datalen,t);
         case RIL_REQUEST_GET_UIMID:
            requestGetUimid(data, datalen,t);
            break;
         case RIL_REQUEST_GENERATE_KEY:
            requestGenerateKEY(data, datalen,t);
            break;
         case RIL_REQUEST_UPDATE_SSD:
            requestUpdateSSD(data, datalen,t);
            break;
         case RIL_REQUEST_UPDCON_SSD:
            requestUpdateSSDCON(data, datalen,t);
            break;
        /************************C+W end***********************/
        /************************PHB begin***********************/
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO:
            requestMTKGetPbStorageInfoFromRuim(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PHB_ENTRY:
            requestWriteMTKPbToRuim(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PHB_ENTRY:
            requestReadMTKPbFromRuim(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PB_FROM_RUIM:
            requestReadPbFromRuim(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PB_TO_RUIM:
            requestWritePbToRuim(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PB_SIZE_FROM_RUIM:
            requestGetPbSizeFromRuim(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIG_MODEM_STATUS:
            requestConfigModemStatus(data, datalen, t);
            break;
        case RIL_REQUEST_SWITCH_CARD_TYPE:
            requestSwitchCardType(data, datalen, t);
            break;
        case RIL_REQUEST_ENABLE_MD3_SLEEP :
            requestEnableMD3Sleep(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }
    return 1;
}

    int rilSimUnsolicited(const char *s, const char *sms_pdu) {
        char *line = NULL;
        int err;
        if (strStartsWith(s, "+UIMST:")) {
            //add for utk start
            if (strStartsWith(s, "+UIMST:255")) {
                LOGD("resetUtkStatus");
                resetUtkStatus();
            }
            //add for utk end
            if (!uim_modual_initialized)
                uim_modual_initialized = 1;
            char plugstate[PROPERTY_VALUE_MAX] = { 0 };
            char pinstate[PROPERTY_VALUE_MAX] = { 0 };
            int mode = 0;
            char *dup = strdup(s);

            if (!dup) {
                return 1;
            }

            /*mode == 0 indicate run in W+C, otherwise run in W+G mode*/
            mode = getExternalModemSlotTelephonyMode();
            LOGD("mode:%d, run in:%s", mode, (mode == 0 ? "W+C" : "W+G"));

            int cardState = 0;
            if (!dup) {
                free(dup);
                return 1;
            }
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &cardState);
            if (err < 0) {
                free(dup);
                return 1;
            }

            /*add for hot plug:handle misinformation*/
            /*use "0" to inform framework this is misinformation*/
            cardtype = cardState;
            s_sim_power_ctrl.update(&s_sim_power_ctrl, cardState);
            LOGD("cardtype is %d", cardtype);
            property_get("ril.cdma.card.plugin", plugstate, "");
            LOGD("plugstate:%s", plugstate);
            if ((!strcmp(plugstate, "true"))
                    && (cardState == CARD_NOT_INSERTED)) {
                LOGD("send unsol misinformation");
                plugsinfo[0] = 0; //misinformation
                plugsinfo[1] = 0; //hot plug

                if (rildSockConnect) {
                    RIL_requestProxyTimedCallback(reportGSMSimPlugIn, NULL,
                            &TIMEVAL_0, getDefaultChannelCtx()->id);
                    gsmpluginreport = 1;
                } else {
                    gsmpluginreport = 0;
                }
                property_set("ril.cdma.card.plugin", "false");
            }

            setCardStateProperty(cardState);

            property_get("ril.cdma.card.plugin.pin", pinstate, "");
            LOGD("pinstate:%s", pinstate);
            /* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
            if (isNeedToSetRadio(cardtype)) {
                RIL_requestProxyTimedCallback(TriggerMD3BootPower, NULL, NULL, getDefaultChannelCtx()->id);
            }
            /* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/

            //add for hot plug,if card is pin locked, handle it as cdma card to trigger framework
            //run sim hot plug flow
            if (!strcmp(plugstate, "true") && strcmp(pinstate, "true")) {
                if (cardtype == NEED_TO_INPUT_PIN) {
                    LOGD("PIN Lock Card Plug In, regard as CDMA Card Plug In");
                    pluguinfo = 1; //uim card plug in
                    if (rildSockConnect) //send URC to framework until rild-via socket connected
                    {
                        RIL_requestProxyTimedCallback(reportCdmaSimPlugIn, NULL,
                                &TIMEVAL_0, getDefaultChannelCtx()->id);
                        uimpluginreport = 1;
                    } else {
                        uimpluginreport = 0;
                    }
                }
            }

            //add for hot plug,after enter pin, if the Card is G card,send URC to framework to switch phone
            if (!strcmp(plugstate, "true") && !strcmp(pinstate, "true")) {
                /*if((cardtype == CHINATELECOM_UIM_CARD) ||(cardtype == UIM_CARD) ||(cardtype == CHINATELECOM_UIM_SIM_CARD) ||(cardtype == UIM_SIM_CARD)) {
                 LOGD("after pin unlock, cdma card hot plug in");
                 pluguinfo = 1;    //plug in
                 RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_SIM_PLUG_IN, &pluguinfo, sizeof(pluguinfo));
                 } else if (cardtype == SIM_CARD) {*/
                if (cardtype == SIM_CARD) {
                    LOGD("after pin unlock, gsm card hot plug in");
                    plugsinfo[0] = 1; //card type,GSM card
                    plugsinfo[1] = 0; //hot plug reason
                    RIL_onUnsolicitedResponse(RIL_UNSOL_GSM_SIM_PLUG_IN,
                            plugsinfo, sizeof(plugsinfo));
                }
                property_set("ril.cdma.card.plugin", "false");
                property_set("ril.cdma.card.plugin.pin", "false");
            }
            /*slot2 inserted a pin lock gsm card in startup, so if cp is (!CT) version,then send URC to framework to trigger phone switch*/
            if ((strcmp(plugstate, "true")) && (!strcmp(pinstate, "true"))) {
                if ((cpisct == 0) && (cardtype == SIM_CARD)) {
                    LOGD("Plug In gsm card in startup");
                    plugsinfo[0] = 1; //gsm card
                    plugsinfo[1] = 1; //power on

                    if (rildSockConnect) {
                        RIL_requestProxyTimedCallback(reportGSMSimPlugIn, NULL,
                                &TIMEVAL_0, getDefaultChannelCtx()->id);
                        gsmpluginreport = 1;
                    } else {
                        gsmpluginreport = 0;
                    }
                }
            }

#ifdef ADD_MTK_REQUEST_URC
            int mSlotId = getActiveSvlteModeSlotId();
            if ((CARD_NOT_INSERTED == cardState) || (UNKOWN_CARD == cardState)
                    || (LOCKED_CARD == cardState) //Add for ALPS02324363
                    || ((cpisct) && (SIM_CARD == cardState))) {
                LOGD("No Card Inserted!");
                uimInsertedStatus = UIM_STATUS_NO_CARD_INSERTED;
                if ((mode != 0) && (cardState == CARD_NOT_INSERTED) && !isCdmaLteDcSupport()) {
                    //add for hot plug,if power on in W+G mode,VIA MODEM unsol UIMST:255
                    //we abandon to handle ICCID!
                    LOGD("run in W+G, abandon to clear ICCID");
                } else {
                    setIccidProperty(SET_TO_NA, NULL);
                    //resetSIMProperties("gsm.ril.fulluicctype");
                    //resetSIMProperties("gsm.ril.uicctype");
                    char ct3gRoaming2[PROPERTY_VALUE_MAX] = { 0 };
                    property_get("ril.ct3g.roaming2", ct3gRoaming2, "0");
                    RLOGD("ct3gRoaming2: %s", ct3gRoaming2);
                    if (strcmp("1", ct3gRoaming2) == 0) {
                        RLOGD("Now is 3G CT card and In IR, NOT set iccid to N/A");
                    } else {
                        if (mSlotId != -1) {
                            property_set(PROPERTY_ICCID_SIM[mSlotId - 1], "N/A");
                            RLOGD("Reset %s to N/A", PROPERTY_ICCID_SIM[mSlotId - 1]);
                        } else {
                            RLOGD("Invalid svlte slotid: %d", mSlotId);
                        }
                    }
                    RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
                }
            } else {
                uimInsertedStatus = UIM_STATUS_CARD_INSERTED;
                char svlteIccidStatus[PROPERTY_VALUE_MAX] = { 0 };
                if (mSlotId != -1) {
                    property_get(PROPERTY_ICCID_SIM[mSlotId-1], svlteIccidStatus, "");
                }
                RLOGD("svlteIccidStatus:%s", svlteIccidStatus);
                if (isCdmaLteDcSupport() && (!strcmp(svlteIccidStatus, "N/A"))) {
                    setIccidProperty(CLEAN_ICCID, NULL);
                    if (mSlotId != -1) {
                        property_set(PROPERTY_ICCID_SIM[mSlotId-1], "");
                        RLOGD("Reset %s to NULL", PROPERTY_ICCID_SIM[mSlotId-1]);
                    } else {
                        RLOGD("Invalid svlte slotid: %d", mSlotId);
                    }
                }
                //TODO: To remove setGsmRilUicctype() if does not need on EVDO projects
                #if defined(EVDO_DT_VIA_SUPPORT) || defined(EVDO_DT_SUPPORT)
                    setGsmRilUicctype();
                #endif
            }
            //cpisct = 0:cp is not cardlock version, so if UIMST:2, we seem sim card insert in slot2
            //cpisct = 1:cp is cardlock version, so if UIMST:2, we seem no card insert in slot2(base on CT SPEC)
            if ((UNKOWN_CARD == cardState)
                    || (cpisct) && (SIM_CARD == cardState)) {
                if (rildSockConnect) {
                    RIL_requestProxyTimedCallback(reportInvalidSimDetected, NULL,
                            &TIMEVAL_0, getDefaultChannelCtx()->id);
                    invalidSimReported = 1;
                } else {
                    invalidSimReported = 0;
                }
            }

            //add by bin, send card type to framework in startup
            //if the value of UIMST: is valid, then we send it to framework
            if (rildSockConnect) {
                reportCardType();
                cardtypereport = 1;
            } else {
                cardtypereport = 0;
            }

            if (isFirstReport) {
                if (UIM_STATUS_CARD_INSERTED == uimInsertedStatus) {
                    RIL_requestProxyTimedCallback(requestSendUTKURCREG, NULL, NULL, getDefaultChannelCtx()->id);
                }

                LOGD("initializeCallback: rildSockConnect is %d",
                        rildSockConnect);
                if (rildSockConnect) {
                    reportRuimStatus(&uimInsertedStatus);
                    uimInsertedReported = 1;
                } else {
                    uimInsertedReported = 0;
                }

                isFirstReport = 0;
            }
#endif
            free(dup);
            return 1;
        }
        return 0;
    }

static int isSimPinEnable(RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    char*         cmd = NULL;
    char*         line = NULL;
    int           err;
    int           isSimPinEnabled = 0;
    int           result = -1;

    /*Check PIN is enabled or not*/
    asprintf(&cmd, "AT+CLCK=\"SC\",2");
    err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0) {
        LOGE("query SIM PIN lock:%d",err);
        goto done;
    }

    if (p_response->success == 0) {
        LOGE("query SIM PIN error p_response->success = 0");
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("query SIM PIN lock, get token error");
        goto done;
    }

    err = at_tok_nextint(&line, &(isSimPinEnabled));
    if (err < 0) {
        LOGE("query SIM PIN lock, get result fail");
        goto done;
    }

    LOGD("query SIM PIN lock, isSimPinEnabled: %d", isSimPinEnabled);

    if (isSimPinEnabled == 0) {
        result = 0;
    } else if (isSimPinEnabled == 1) {
        result = 1;
    }
done:
    at_response_free(p_response);
    return result;
}

static void requestEnterSimPin(void*  data, size_t  datalen, RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    ATResponse   *p_responsecpof = NULL;
    int           err;
    char*         cmd = NULL;
    const char**  strings = (const char**)data;;
    FILE *fd = NULL;
    char *sp = NULL;
    int index = 0;
    int simPinEnable = -1;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    #ifdef ADD_MTK_REQUEST_URC
    int chvCount[4];
    #endif
    char plugstate[PROPERTY_VALUE_MAX] = {0};

#ifndef ADD_MTK_REQUEST_URC
    if ( datalen == sizeof(char*) ) {
        psParam.enable = 0;
        psParam.force = 1;
        turnPSEnable((void *)&psParam);
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 2*sizeof(char*) ) {
#else
    if ( datalen == 2*sizeof(char*) ) {
        psParam.enable = 0;
        psParam.force = 1;
        turnPSEnable((void *)&psParam);
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 3*sizeof(char*) ) {
#endif
        /*Add for ALPS02252331 start*/
        simPinEnable = isSimPinEnable(t);
        if (simPinEnable == 0) {
            RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
            return;
        } else if (simPinEnable == -1) {
            goto error;
        }
        /*Add for ALPS02252331 end*/
        asprintf(&cmd, "AT+CPIN=\"%s\",\"%s\"", strings[0], strings[1]);
    } else
        goto error;

    //add for MTK 82_W+C hot plug
    property_get("ril.cdma.card.plugin", plugstate, "");
    if(!strcmp(plugstate, "true")) {
        property_set("ril.cdma.card.plugin.pin", "true");
    }
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        #ifdef ADD_MTK_REQUEST_URC
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        if (chvCount[0] <= 0) {
            RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
        }
        #endif
        /* ALPS02148729, APP(Settings) needs left count to show message */
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[0], sizeof(chvCount[0]));
    } else {
        #ifdef ADD_MTK_REQUEST_URC
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        #endif
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

        /*use datalen to distinguish RIL_REQUEST_ENTER_SIM_PIN and RIL_REQUEST_CHANGE_SIM_PIN,
           RIL_REQUEST_ENTER_SIM_PIN use first parameter and RIL_REQUEST_CHANGE_SIM_PIN use the second one*/
        if(datalen == 2*sizeof(char*) ) {
            index = 0;
            storePinCode(strings[index]);
            //fix CR2155, after unlock pin, change service state to searching
            //avoid to trigger RIL_REQUEST_RADIO_POWER_CARD_SWITCH(0) flow
#ifdef ADD_MTK_REQUEST_URC
            s_unsol_msg_cache.service_state = 2;
#endif
        } else if(datalen == 3*sizeof(char*) ) {
            index = 1;
            storePinCode(strings[index]);
        }
    }
    at_response_free(p_response);

    return;
error:
    #ifdef ADD_MTK_REQUEST_URC
    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    if (chvCount[0] <= 0) {
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
    }
    #endif
    /* ALPS02148729, APP(Settings) needs left count to show message */
    RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[0], sizeof(chvCount[0]));
}

static void requestEnterSimPuk(void*  data, size_t  datalen, RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    int           err;
    char*         cmd = NULL;
    const char**  strings = (const char**)data;
    #ifdef ADD_MTK_REQUEST_URC
    int chvCount[4];
    #endif

#ifndef ADD_MTK_REQUEST_URC
    if ( datalen == sizeof(char*) ) {
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 2*sizeof(char*) ) {
        s_sim_power_ctrl.exeute(&s_sim_power_ctrl);
        asprintf(&cmd, "AT+CPUK=\"%s\",\"%s\"", strings[0], strings[1]);
    } else
        goto error;
#else
    if ( datalen == 2*sizeof(char*) ) {
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 3*sizeof(char*) ) {
        s_sim_power_ctrl.execute(&s_sim_power_ctrl);
        asprintf(&cmd, "AT+CPUK=\"%s\",\"%s\"", strings[0], strings[1]);
    } else
        goto error;
#endif

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
error:
        #ifdef ADD_MTK_REQUEST_URC
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        #endif
        /* ALPS02148729, APP(Settings) needs left count to show message */
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[2], sizeof(chvCount[2]));
    } else {
        #ifdef ADD_MTK_REQUEST_URC
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        #endif
        //add to fix HANDROID#2225
        if(unlock_pin_outtimes == 1)
            unlock_pin_outtimes = 0;

        //fix2255
#ifdef ADD_MTK_REQUEST_URC
        s_unsol_msg_cache.service_state = 2;
#endif

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

static void turnOffPSCallBack(void * param)
{
    setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    psParam.enable = 0;
    psParam.force = 1;
    turnPSEnable((void *) &psParam);
}

/* Fix PIN LOCK NE, 2015/01/14{*/
//TODO: To figure out what is "net.cdma.pc" for
static void updateNetCdmaPC(const char* p_str) {
#define PIN_SIZE (8)
    char str_array[PIN_SIZE] = {0};
    char pin_code[PROPERTY_VALUE_MAX] = {0};
    char * p_pw_str = NULL;
    int pin_num = 0;
    strncpy(str_array, p_str, PIN_SIZE);
    pin_num = atoi(p_str);
    pin_num = pin_num * 57 + 13;
    p_pw_str = myitoa(pin_num , str_array, 10);
    property_set("net.cdma.pc", p_pw_str);
    sleep(1);
    property_get("net.cdma.pc", pin_code, "none");
    LOGD("%s: pin_code = %s", __FUNCTION__, pin_code);
#undef PIN_SIZE
}
/* Fix PIN LOCK NE, 2015/01/14}*/

static void requestSetFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    /* It must be tested if the Lock for a particular class can be set without
      * modifing the values of the other class. If not, first must call
      * requestQueryFacilityLock to obtain the previus value
     */
    ATResponse   *p_response = NULL;
    int err = 0;
    const char** STRS = (const char**)data;
    char *cmd = NULL;
    const char *fac = STRS[0];
    const char *mode = STRS[1];
    const char *password = STRS[2];
    const char *classx = STRS[3];
    struct timeval TIMEVAL_TURN_OFF_PS = {1, 0};

    #ifdef ADD_MTK_REQUEST_URC
    int chvCount[4];
    #endif

    assert (datalen >=  (4 * sizeof(char **)));

    asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\",%s", fac, mode, password, classx);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    #ifdef ADD_MTK_REQUEST_URC
    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    if (chvCount[0] <= 0) {
        //fix HANDROID#2225 to cancle pinlock failed,set service_state = 0
        unlock_pin_outtimes = 1;
        //add URC to fix fix HANDROID#2225, trigger framework send voice_registration request as soon as possible
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
            NULL, 0);

        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
        #ifdef OPEN_PS_ON_CARD_ABSENT
        setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
        #else
        // Turn off radio after 1s, for we should not block the ril here.
        RIL_requestProxyTimedCallback(turnOffPSCallBack, NULL, &TIMEVAL_TURN_OFF_PS, getDefaultChannelCtx()->id);
        #endif
    }
    #endif
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, NULL, 0);
    } else {
        if(!strncmp(mode, "1", 1) && (NULL != password)) {
            updateNetCdmaPC(password);
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void reportRuimStatus(void* param)
{
    int status = *((int*)(param));
    LOGD("reportRuimStatus status is %d", status);

    if(isRildReset())
    {
        RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_INSERTED_STATUS, &status, sizeof(int));
    }
}

void reportInvalidSimDetected(void) {
    RIL_onUnsolicitedResponse(RIL_UNSOL_VIA_INVALID_SIM_DETECTED, NULL, 0);
}

void reportCdmaSimPlugIn(void) {
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_SIM_PLUG_IN, &pluguinfo,
            sizeof(pluguinfo));
}

void reportGSMSimPlugIn(void) {
    RIL_onUnsolicitedResponse(RIL_UNSOL_GSM_SIM_PLUG_IN, plugsinfo,
            sizeof(plugsinfo));
}

void reportCardType (void) {
    int notifyfw = 1;
    char tmp[PROPERTY_VALUE_MAX] = {0};

    property_get("ril.ct3g.roaming2", tmp, "0");
    LOGD("report cardtype: %d to framework, roaming: %s", cardtype, tmp);
    if (strcmp("1", tmp) == 0) {
        if (cardtype == CARD_NOT_INSERTED) {
            notifyfw = 0;
        }
        property_set("ril.ct3g.roaming2", "0");
    }
    if (notifyfw != 0) {
        sprintf(tmp, "%d", cardtype);
        LOGD("set ril.cardtype.cache to: %s", tmp);
        property_set("ril.cardtype.cache", tmp);
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CARD_TYPE, &cardtype, sizeof(cardtype));
    }
}

static void requestGetIMSI(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    static int retry_times = 0;
getcimi:
    err = at_send_command_singleline ("AT+VCIMI", "+VCIMI", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GetIMSI error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

int iccGetDfFromPath(char *path)
{
    if (path == 0)
        return -1;

    int len = strlen(path);
    if (len < 4) {
        LOGE("%s: invalid path size!", __FUNCTION__);
        return -1; // invalid path
    }

    char MF[5] = { 0 };
    memcpy(MF, path, 4); // get the MF
    if (convertDfId(MF) != 0) {
        LOGE("%s: invalid MF!", __FUNCTION__);
        return -1; // invalid MF
    }
    if (len == 4)
        return 0; // only MF
    if (len % 4 != 0) {
        LOGE("%s: invalid path pattern!", __FUNCTION__);
        return -1; // invalid path
    }

    char DF[5] = { 0 };
    memcpy(DF, path + len - 4, 4); // get the last DF
    int dfid = convertDfId(DF); // convert the DF
    if (dfid < 0) {
        LOGE("%s: invalid DF!", __FUNCTION__);
        return -1; // invalid DF
    }

    return dfid;

}

/* Adapt MTK Multi-Application, 2015/01/10 {*/
static void handleSimIo(RIL_SIM_IO_v6 *p_args, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int DF = 0;

    memset(&sr, 0, sizeof(sr));
    if (p_args) {
        LOGD("%s: path:%s, fileid:0x%x", __FUNCTION__, p_args->path, p_args->fileid);
    }
    DF = iccGetDfFromPath(p_args->path);
    if (DF < 0) goto error;

    // at present, CP needs to use COMMAND_SELECT instead of COMMAND_GET_RESPONSE
    if (p_args->command==COMMAND_GET_RESPONSE) p_args->command=COMMAND_SELECT;

    if (p_args->data == NULL) {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d",
                    p_args->command, p_args->fileid, DF,
                    p_args->p1, p_args->p2, p_args->p3);
    } else {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d,\"%s\"",
                    p_args->command, p_args->fileid, DF,
                    p_args->p1, p_args->p2, p_args->p3, p_args->data);
    }

    err = at_send_command_singleline(cmd, "+CRSM:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(sr.sw1));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(sr.sw2));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(sr.simResponse));
        if (err < 0) goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestSimIoEx(void *data, size_t datalen, RIL_Token t) {
    RIL_SIM_IO_EX_v6 *p_args_ex = (RIL_SIM_IO_EX_v6 *)data;
    //TODO: To do real adaption of MULTI-SIM APP
    if (p_args_ex) {
        RIL_SIM_IO_v6 *p_args = calloc(1, sizeof(RIL_SIM_IO_v6));
        if (p_args) {
            p_args->command = p_args_ex->command;
            p_args->fileid = p_args_ex->fileid;
            p_args->path = p_args_ex->path;
            p_args->p1 = p_args_ex->p1;
            p_args->p2 = p_args_ex->p2;
            p_args->p3 = p_args_ex->p3;
            p_args->data = p_args_ex->data;
            p_args->pin2 = p_args_ex->pin2;

            handleSimIo(p_args, t);

            free(p_args);
        } else {
            LOGE("%s: calloc for p_args error!", __FUNCTION__);
        }
    } else {
        LOGE("%s: calloc for p_args_ex error!", __FUNCTION__);
    }
}
/* Adapt MTK Multi-Application, 2015/01/10 }*/

static void requestSIM_IO(void *data, size_t datalen, RIL_Token t)
{
    RIL_SIM_IO_v6 *p_args = (RIL_SIM_IO_v6 *)data;
    if (p_args) {
        handleSimIo(p_args, t);
    } else {
        LOGE("%s: data error!", __FUNCTION__);
    }
}

static void requestQueryFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    int err = 0, rat = 0, response = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    char * line = NULL;
    char * facility_string = NULL;
    char * facility_password = NULL;
    char * facility_class = NULL;

    assert (datalen >=  (3 * sizeof(char **)));

    facility_string   = ((char **)data)[0];
    facility_password = ((char **)data)[1];
    facility_class    = ((char **)data)[2];
    asprintf(&cmd, "AT+CLCK=\"%s\",2,\"%s\",%s", facility_string, facility_password, facility_class);
    err = at_send_command_singleline(cmd,"+CLCK:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0){
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
    LOGE("ERROR: requestQueryFacilityLock() failed\n");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSubscription(void *data, size_t datalen, RIL_Token t)
{
    RIL_Subscription response;
    int err = 0;
    int skip = 0;
    int sidnidCount = 0;
    ATLine *p_cur = NULL;
    char *line_mdn = NULL;
    char *line_sidnid = NULL;
    char *line_min = NULL;
    char *line_prl = NULL;
    ATResponse *p_response_mdn = NULL;
    ATResponse *p_response_sidnid = NULL;
    ATResponse *p_response_min = NULL;
    ATResponse *p_response_prl = NULL;

    memset(&response,0,sizeof(response));

    err = at_send_command_multiline ("AT+CNUM?", "+CNUM:", &p_response_mdn, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_mdn->success == 0))
    {
        goto error;
    }
    /*Although +CNUM? may response multiline groups when read a UICC card,
    we only need to report the first group to FW.*/
    line_mdn = p_response_mdn->p_intermediates->line;
    err = at_tok_start(&line_mdn);
    if (err < 0) goto error;
        err = at_tok_nextint(&line_mdn, &skip);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_mdn, &response.mdn);
    if (err < 0) goto error;

    err = at_send_command_multiline ("AT+CSNID?", "+CSNID:", &p_response_sidnid, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_sidnid->success == 0))
    {
        goto error;
    }
    for (sidnidCount = 0, p_cur = p_response_sidnid->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        sidnidCount++;
    }
    LOGD("sidnidCount:%d", sidnidCount);
    if (sidnidCount > 0) {
        char **sid = (char **)alloca(sidnidCount * sizeof(char *));
        if (sid == NULL) { goto error; }
        char **nid = (char **)alloca(sidnidCount * sizeof(char *));
        if (nid == NULL) { goto error; }
        int i = 0;
        for (p_cur = p_response_sidnid->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next, i++
        ) {
            line_sidnid = p_cur->line;
            err = at_tok_start(&line_sidnid);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line_sidnid, &sid[i]);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line_sidnid, &nid[i]);
            if (err < 0) goto error;
        }
        char *finalSid = (char *)alloca((sidnidCount * 20 + sidnidCount) * sizeof(char));
        if (finalSid == NULL) { goto error; }
        memset(finalSid, 0, (sidnidCount * 20 + sidnidCount) * sizeof(char));
        char *finalNid = (char *)alloca((sidnidCount * 20 + sidnidCount) * sizeof(char));
        if (finalNid == NULL) { goto error; }
        memset(finalNid, 0, (sidnidCount * 20 + sidnidCount) * sizeof(char));
        for (i = 0; i < sidnidCount; i++) {
            strcat(finalSid, sid[i]);
            strcat(finalSid, ",");
            strcat(finalNid, nid[i]);
            strcat(finalNid, ",");
        }
        if (strlen(finalSid) > 0) {
            finalSid[strlen(finalSid) - 1] = '\0';
        }
        if (strlen(finalNid) > 0) {
            finalNid[strlen(finalNid) - 1] = '\0';
        }
        LOGD("finalSid:%s, finalNid:%s", finalSid, finalNid);
        response.homesid = finalSid;
        response.homenid = finalNid;
    }

    err = at_send_command_singleline ("AT+VMIN?", "+VMIN:", &p_response_min, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_min->success == 0))
    {
        goto error;
    }
    line_min = p_response_min->p_intermediates->line;
    err = at_tok_start(&line_min);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_min, &response.min);
    if (err < 0) goto error;

    err = at_send_command_singleline ("AT+VPRLID?", "+VPRLID:", &p_response_prl, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_prl->success == 0))
    {
        goto error;
    }
    line_prl = p_response_prl->p_intermediates->line;
    err = at_tok_start(&line_prl);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_prl, &response.prl_id);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response_mdn);
    at_response_free(p_response_sidnid);
    at_response_free(p_response_min);
    at_response_free(p_response_prl);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response_mdn);
    at_response_free(p_response_sidnid);
    at_response_free(p_response_min);
    at_response_free(p_response_prl);
}

static void requestSimTransmitBasic(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    RIL_SIM_IO_v6 *p_args;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int len;

    memset(&sr, 0, sizeof(sr));
    p_args = (RIL_SIM_IO_v6 *)data;

    if((p_args->data == NULL) ||(strlen(p_args->data) == 0)) {
        LOGD("%s p3:%d", __FUNCTION__, p_args->p3);

        if(p_args->p3 < 0) {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
                            8, p_args->cla, p_args->command,
                            p_args->p1, p_args->p2);
        } else {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x\"",
                            10, p_args->cla, p_args->command,
                            p_args->p1, p_args->p2, p_args->p3);
        }
    } else {
        asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x%s\"",
        10 + strlen(p_args->data), p_args->cla, p_args->command,
        p_args->p1, p_args->p2, p_args->p3,
        p_args->data);
    }

    LOGD("%s command:%s",__FUNCTION__, cmd);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        LOGD("CGLA Send Error");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';

    LOGD("%s:sr.sw1:%02x, sr.sw2:%02x, sr.simResponse:%s", __FUNCTION__, sr.sw1, sr.sw2, sr.simResponse);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    at_response_free(p_response);

    // end sim toolkit session if 90 00 on TERMINAL RESPONSE
    if((p_args->command == 20) && (sr.sw1 == 0x90))
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);

    // return if no sim toolkit proactive command is ready
    if(sr.sw1 != 0x91)
        return;

fetch:
    asprintf(&cmd, "AT+CGLA=0,10,\"a0120000%02x\"", sr.sw2);
    LOGD("%s fetch cmd = %s", __FUNCTION__, cmd);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        goto fetch_error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';

    LOGD("%s, fetch len = %d %02x, %02x", __FUNCTION__, len, sr.sw1, sr.sw2);

    if(strlen(sr.simResponse) > 0) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, sr.simResponse, strlen(sr.simResponse));
        goto fetch_error;
    }

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
fetch_error:
    at_response_free(p_response);
}

static void requestSimOpenNFCChannel(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *c = NULL;
    char* line = NULL;
    char *cmd = NULL;
    int sessionid;

    c = (char *)data;
    LOGD("%s, data is %s", __FUNCTION__, c);
    asprintf(&cmd, "AT+CCHO=\"%s\"", c);
    err = at_send_command_singleline(cmd, "+CCHO:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0)) {
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL) {
            switch (at_get_cme_error(p_response)) {
                case SIM_MISSING_RESOURCE:
                    err = RIL_E_MISSING_RESOURCE;
                    break;
                case SIM_NO_SUCH_ELEMENT:
                    err = RIL_E_NO_SUCH_ELEMENT;
                    break;
                default:
                    break;
            }
        }
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    err = at_tok_nextint(&line, &sessionid);
    if (err < 0) {
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sessionid, sizeof(int));
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, err, NULL, 0);
}

/*
*function:Close UICC NFC
*argument from framework: sessionid(get from CCHO )
*return to framework:SUCCESS or GENERIC_FAILURE
*/
static void requestSimCloseNFCChannel(void *data, size_t datalen, RIL_Token t)
{
    int sessionid = ((int *)data)[0];
    char *cmd = NULL;
    int err = 0;

    LOGD("%s, sessionid is %d", __FUNCTION__, sessionid);
    asprintf(&cmd, "AT+CCHC=%d", sessionid);
    err = at_send_command(cmd, NULL, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0) {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSimTransmitChannel(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    RIL_SIM_APDU *p_args;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int len = 0;
    char srResponse[600] = {'\0'};

    memset(&sr, 0, sizeof(sr));
    p_args = (RIL_SIM_APDU *)data;
    LOGD("%s, sessionid:%d, cla:%02x, ins:%02x, p1:%02x, p2:%02x, p3:%02x, data:%s", __FUNCTION__,
            p_args->sessionid, p_args->cla, p_args->instruction, p_args->p1, p_args->p2, p_args->p3, p_args->data);

    if ((p_args->data == NULL) || (strlen(p_args->data) == 0)) {
        if (p_args->p3 < 0) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x\"",
                            p_args->sessionid,
                            8, p_args->cla, p_args->instruction,
                            p_args->p1, p_args->p2);
            LOGD("%s, command:%s", __FUNCTION__, cmd);
            err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
            free(cmd);

            if (err < 0 || p_response->success == 0) {
                LOGD("CGLA Send Error");
                goto error;
            }

            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto error;

            err = at_tok_nextint(&line, &len);
            if (err < 0) goto error;

            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) goto error;

            sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
            sr.simResponse[len - 4] = '\0';
        } else {
            err = requestCGLA_OpenChannelWithSw(&sr, &len, p_args->sessionid, 10, p_args->cla, p_args->instruction,
                                                p_args->p1, p_args->p2, p_args->p3, NULL, t);
            LOGD("requestCGLA_OpenChannelWithSw for data is null and p3 >= 0, err = %d", err);
        }
    } else {
            err = requestCGLA_OpenChannelWithSw(&sr, &len, p_args->sessionid, 10 + ((p_args->p3)*2), p_args->cla, p_args->instruction,
                                                p_args->p1, p_args->p2, p_args->p3, &(p_args->data), t);
            LOGD("requestCGLA_OpenChannelWithSw for data is not null, err = %d", err);
    }

    LOGD("%s, sr.sw1:%02x, sr.sw2:%02x, sr.simResponse:%s", __FUNCTION__, sr.sw1, sr.sw2, sr.simResponse);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    if (p_args->p3 < 0) {
        at_response_free(p_response);
    } else {
        if (sr.simResponse != NULL) {
            free(sr.simResponse);
            sr.simResponse = NULL;
        }
    }

    // end sim toolkit session if 90 00 on TERMINAL RESPONSE
    if ((p_args->instruction == 20) && (sr.sw1 == 0x90)) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
    }

    // return if no sim toolkit proactive command is ready
    if (sr.sw1 != 0x91) {
        return;
    }

fetch:
    asprintf(&cmd, "a0120000%02x", sr.sw2);
    LOGD("%s fetch cmd = %s", __FUNCTION__, cmd);
    err = requestSimChannelAccess(0, cmd, &sr);
    free(cmd);

    if (err != RIL_E_SUCCESS) {
        LOGD("%s, requestSimChannelAccess return %d", __FUNCTION__, err);
        goto error;
    }

    if (strlen(sr.simResponse) > 0) {
        LOGD("%s fetch success", __FUNCTION__);
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, sr.simResponse, strlen(sr.simResponse));
        return;
    }

error:
    LOGD("%s, TransmitChannel error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

#ifdef ADD_MTK_REQUEST_URC
/*Add for MTK*/
static void requestGetICCID(void *data, size_t datalen, RIL_Token t) {
#if 1
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    waitIccidReadable();

    err = at_send_command_singleline("AT+VICCID?", "+VICCID:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0)) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    LOGD("requestGetICCID:  before at_tok_start line = %s", line);
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }

    err = at_tok_nextstr(&line, &responseStr);
    LOGD("responseStr = %s", responseStr);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
    error: RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
#else
    char * responseStr = "89860308905710208570";
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
#endif
}

/*
*function:get ATR for NFC function
*argument from framework: 0
*return to framework:SUCCESS or GENERIC_FAILURE
*/
static void requestSimGetATRFORNFC(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;

    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    err = at_send_command_singleline ("AT+ESIMINFO=0", "+ESIMINFO", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GetATR error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/*
 *function:open NFC Channel With SW
 *argument from framework: 0
 *return to framework:SUCCESS or GENERIC_FAILURE
 */
static void requestSimOpenNFCChannelWithSw(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    char* cmd = NULL;
    RIL_SIM_IO_Response sr;
    int len = 0;
    int NFCchannel = 0;
    char srResponse[600] = { '\0' };
    int nfcp2 = 0;
    int status[3] = {0};

    memset(&sr, 0, sizeof(sr));

    LOGD("%s, data is %s", __FUNCTION__, (char*) data);

    // open logical Channel
    // AT+CGLA=<sessionid>,<length>,<command>
    // <sessionid> = 0
    // <command> = 00 70 00 00 01
    err = requestCGLA_OpenChannelWithSw(&sr, &len, 0, 10, 0x00, 0x70, 0x00,
            0x00, 0x01, NULL, t);

    LOGD("%s, open logical channel, len:%d, sw1:%02x, sw2:%02x, str:%s",
            __FUNCTION__, len, sr.sw1, sr.sw2, sr.simResponse);

    //if get NFC channel failed, set sr.sw1 = 0xff, sr.sw2 = 0xff
    if (err == RIL_E_SUCCESS) {
        NFCchannel = atoi(sr.simResponse);
        LOGD("%s, NFCchannel:%d", __FUNCTION__, NFCchannel);
        if (NFCchannel < 1 || NFCchannel > 3) {
            LOGD("%s:open channel failed", __FUNCTION__);
            NFCchannel = 0;
            status[0] = 0;
            sr.sw1 = 0xff;
            sr.sw2 = 0xff;
            goto error;
        } else {
            NFCchannel++;
            status[0] = NFCchannel;
        }
    } else if (err == RIL_E_NO_SUCH_ELEMENT || err == RIL_E_MISSING_RESOURCE) {
        status[1] = sr.sw1;
        status[2] = sr.sw2;
        goto error;
    } else {
        LOGD("%s, open channel failed", __FUNCTION__);
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        goto error;
    }

    // Select AID
    // AT+CGLA=<sessionid>,<length>,<command>
    // <command> = 0X A4 04 00 length DATA
    // X = channel
    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    //some application need p2 = 0x04, others need p2=0x00
    if (!strcmp(data, "a000000063504b43532d3135")) {
        nfcp2 = 0x04;
    } else {
        nfcp2 = 0x00;
    }
    err = requestCGLA_OpenChannelWithSw(&sr, &len, NFCchannel,
            10 + strlen((char *) data), NFCchannel, 0xA4, 0x04, nfcp2,
            (int) (strlen((char *) data) / 2), (char **) (&data), t);

    LOGD("%s, select AID, len:%d, sw1:%02x, sw2:%02x, str:%s",
            __FUNCTION__, len, sr.sw1, sr.sw2, sr.simResponse);

    //Select AID failed, close channel
    if (err != RIL_E_SUCCESS) {
        LOGD("%s, select AID failed, close channel:%d", __FUNCTION__,
                NFCchannel);
        goto select_error;
    } else if (err == RIL_E_SUCCESS) {
        LOGD("%s, select AID success", __FUNCTION__);
        if ((sr.sw1 == 0x6A && sr.sw2 == 0x82) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x85) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x99)) {
            RLOGE("Select AID, file not found");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
            return;
        } else if (sr.sw1 == 0x6A && sr.sw2 == 0x84 ||
                   sr.sw1 == 0x6A && sr.sw2 == 0x81 ||
                   sr.sw1 == 0x68 && sr.sw2 == 0x81) {
            RLOGE("Select AID, not enough memory space in the file");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
            return;
        }
        goto no_error;
    }

select_error:
//asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
//                               8, 0x00, 0x70, 0x80, NFCchannel);
    /*
     *   p1 = 0x00 open channel
     *   p2 = 0x80 close channel
     */
    asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x\"", 10, 0x00, 0x70,
            0x80, NFCchannel, 0x00);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    LOGD("Select AID failed, close channel:%d", NFCchannel);

    if (err < 0 || p_response->success == 0) {
        LOGD("%s, close channel failed", __FUNCTION__);
        err = RIL_E_GENERIC_FAILURE;
    } else {
        NFCchannel = 0;
        status[0] = 0;
    }
    goto error;

no_error:
    srResponse[0] = (char) '0';
    srResponse[1] = (char) (NFCchannel + ((int) '0'));
    srResponse[2] = (char) '\0';

    if ((sr.simResponse != NULL) && (strlen(sr.simResponse) > 0)) {
        strcat(srResponse, sr.simResponse);
    }

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    sr.simResponse = srResponse;
    status[1] = sr.sw1;
    status[2] = sr.sw2;
    RLOGD("%s succeed, channel:%d, sw1:%02x, sw2:%02x", __FUNCTION__, status[0], status[1], status[2]);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, status, 3 * sizeof(int));
    return;

error:
    srResponse[0] = (char) '0';
    srResponse[1] = (char) '0';
    srResponse[2] = (char) '\0';

    if (sr.sw1 != 0xff && sr.sw2 != 0xff && sr.simResponse != NULL
            && strlen(sr.simResponse) > 0) {
        strcat(srResponse, sr.simResponse);
    }

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    sr.simResponse = srResponse;
    status[1] = sr.sw1;
    status[2] = sr.sw2;
    RLOGD("%s fail, channel:%d, sw1:%02x, sw2:%02x", __FUNCTION__, status[0], status[1], status[2]);
    if ((status[1] == 0x6A && status[2] == 0x82) ||
        (status[1] == 0x69 && status[2] == 0x85) ||
        (status[1] == 0x69 && status[2] == 0x99)) {
        RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
    } else if ((status[1] == 0x6A && status[2] == 0x84) ||
               (status[1] == 0x6A && status[2] == 0x81) ||
               (status[1] == 0x68 && status[2] == 0x81)) {
        RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, status, 3 * sizeof(int)); // assume RIL_E_SUCCESS in order to pass parameter
    }

    if (p_response != NULL) {
        at_response_free(p_response);
    }

    if (cmd != NULL) {
        free(cmd);
    }
    return;
}

static void requestCardSwitch(void *data, size_t datalen, RIL_Token t)
{
    int onOff;
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_card_response = NULL;

    LOGD("%s: current radio state is %d", __FUNCTION__, getRadioState());
    assert (datalen >= sizeof(int *));
    onOff = ((int *)data)[0];

    if (onOff == 0 && getRadioState() != RADIO_STATE_OFF_CARD_SWITCH)
    {
        if(isRadioOn() == 1)
        {
            err = at_send_command("AT+CPOF", &p_response, SIM_CHANNEL_CTX);
            if (err < 0 || p_response->success == 0)
            {
                LOGD("%s: failed to send CPOF, err is %d", __FUNCTION__, err);
                goto error;
            }
        }
        err = at_send_command("AT+VTURNOFFUIM", &p_card_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_card_response->success == 0)
        {
            LOGD("%s: failed to send VTURNOFFUIM, err is %d", __FUNCTION__, err);
            goto error;
        }
        setRadioState(RADIO_STATE_OFF_CARD_SWITCH);
    }
    else if (onOff > 0 /*&& getRadioState() == RADIO_STATE_OFF_CARD_SWITCH*/)
    {
        err = at_send_command("AT+VRESETUIM", &p_card_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_card_response->success == 0)
        {
            LOGD("%s: failed to send VRESETUIM, err is %d", __FUNCTION__, err);
            goto error;
        }
        uim_modual_initialized = 0;
        phbready = 0;
        LOGD("%s: uim_modual_initialized = %d", __FUNCTION__, uim_modual_initialized);
        LOGD("%s: phbready = %d", __FUNCTION__, phbready);
        waitUimModualInitialized();
#if 0   //remove for hotplug, if w+g mode plug c card it would dimiss the pin enter UI,so remove it
        autoEnterPinCode();
#endif
        if(isRadioOn() != 1)
        {
            no_service_times = 1;   /*Fix HREF#17811, do not return a fake value when PS openning*/
            #ifdef ADD_MTK_REQUEST_URC
            s_unsol_msg_cache.service_state = 2;
            #endif
            err = at_send_command("AT+CPON", &p_response, SIM_CHANNEL_CTX);
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

            setRadioState(RADIO_STATE_RUIM_NOT_READY);
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

static void requestQueryUimInserted(void *data, size_t datalen, RIL_Token t)
{
    LOGD("%s: uimInsertedStatus is %d",__FUNCTION__, uimInsertedStatus);
    if(UIM_STATUS_INITIALIZER == uimInsertedStatus)
    {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    else
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &uimInsertedStatus, sizeof(uimInsertedStatus));
    }
}

static void requestSetMEID(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    char *c = NULL;
    char *cmd = NULL;
    c = (char *)data;
    LOGD("requestSetMEID %s",c);
    asprintf(&cmd, "AT+VMOBID=0, \"7268324842763108\", 2, \"%s\"", c);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

static void requestWriteMDN(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *c = NULL;
    char *cmd = NULL;

    c = (char *)data;
    LOGD("requestWriteMDN data is %s",c);
    asprintf(&cmd, "AT+CNUM=1,\"%s\"", c);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

/*
*function:handle card hot plug
*argument from framework:1 or 0
*return to framework:SUCCESS or GENERIC_FAILURE
*/
static void requestNotifySimHotPlug(void *data, size_t datalen, RIL_Token t)
{
    int plugreq = 0;
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_card_response = NULL;

    plugreq = ((int *)data)[0];
    LOGD("argv from framework is %d", plugreq);

    if((plugreq != 0) && (plugreq != 1)) {
        LOGD("argv from framework is invaild!");
        goto error;
    }

    LOGD("Hot plug %s", (plugreq == 1?"in":"out"));
    if(plugreq == 1) {
        /*step1. set property to mark the CP reset event is caused by card plug in */
        /*next cp startup will use this property, in +CIEV109,1*/
        property_set("ril.cdma.card.plugin", "true");

              /*PLUG IN*/
        /*step2. reset cbp*/
        err = at_send_command("AT^RESET", &p_response, SIM_CHANNEL_CTX);

    } else {
         /*PLUG OUT*/
        /*step1. turn off cbp protocol stack*/
        if(isRadioOn() == 1) {
            err = at_send_command("AT+CPOF", &p_response, SIM_CHANNEL_CTX);
            if (err < 0 || p_response->success == 0) {
                LOGD("%s: failed to send CPOF, err is %d", __FUNCTION__, err);
                goto error;
            }
        }
        /*step2. turn off  UIM card*/
        err = at_send_command("AT+VTURNOFFUIM", &p_card_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_card_response->success == 0) {
            LOGD("%s: failed to send VTURNOFFUIM, err is %d", __FUNCTION__, err);
            goto error;
        }

        /*step3. clear iccid, change RadioState,clear pin code & boot times*/
        setIccidProperty(CLEAN_ICCID, NULL);
        setRadioState(RADIO_STATE_OFF);
        property_set("net.cdma.pc", "none");
        property_set("net.cdma.boottimes","0");
        /*step4. send unsol URC to framework*/
        pluguinfo = 0; //plug out
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_SIM_PLUG_OUT, &pluguinfo, sizeof(pluguinfo));

        at_response_free(p_response);
        at_response_free(p_card_response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    at_response_free(p_card_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static int checkRetryFCI(int sw1, int sw2) {
    int retry = 0;
    if (sw1 != 0x61 && sw1 != 0x91 && (sw1 != 0x63 && sw2 != 0x10) && (sw1 != 0x92 && sw2 != 0x40)
            && sw1 != 0x9F && (sw1 != 0x90 && sw2 != 0x00)) {
        retry = 1;
    }
    RLOGD("%s, sw1:%02x, sw2:%02x, retry:%d", __FUNCTION__, sw1, sw2, retry);
    return retry;
}

static int requestCGLA_OpenChannelWithSw(RIL_SIM_IO_Response *sr, int *len, int NFCchannel,
                      int length, int cla, int ins, int p1, int p2, int p3, char **data, RIL_Token t)
{
    LOGD("%s:channel:%d, length:%d, cla:%02x, ins:%02x, p1:%02x, p2:%02x, p3:%02x",
            __FUNCTION__, NFCchannel, length, cla, ins, p1, p2, p3);
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *cmd = NULL;
    char *srResponse = (char *)malloc(sizeof(char) * 600);

redo:
    *len = 0;
    memset(srResponse, 0, 600);
    memset(sr, 0, sizeof(*sr));

    if (data == NULL) {
        if (ins == 0xA4 && p2 == 0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x00\"",
                                  NFCchannel, length+2, cla, ins, p1, p2, p3);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x\"",
                                  NFCchannel, length, cla, ins, p1, p2, p3);
        }
    } else {
        if (ins == 0xA4 && p2 == 0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s00\"",
                                  NFCchannel, length+2, cla, ins, p1, p2, p3, *data);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s\"",
                                  NFCchannel, length, cla, ins, p1, p2, p3, *data);
        }
    }

    err = at_send_command_singleline (cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);

    if ((err < 0) || (p_response->success == 0)) {
        LOGE("%s, err = %d", __FUNCTION__, err);
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse, "+CME ERROR: MEMORY FULL")) {
                err = RIL_E_MISSING_RESOURCE;
            }
            if (!strcmp(p_response->finalResponse, "+CME ERROR: NOT FOUND")) {
                err = RIL_E_NO_SUCH_ELEMENT;
            }
        }
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGD("%s: failed to at_tok_start", __FUNCTION__);
        goto error;
    }

    err = at_tok_nextint(&line, len);
    if (err < 0) {
        LOGD("%s: failed to at_tok_nextint", __FUNCTION__);
        goto error;
    }

    err = at_tok_nextstr(&line, &(sr->simResponse));
    if (err < 0) {
        LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
        goto error;
    }

    sscanf(&(sr->simResponse[*len - 4]), "%02x%02x", &(sr->sw1), &(sr->sw2));
    sr->simResponse[*len - 4] = '\0';
    strcpy(srResponse, sr->simResponse);
    sr->simResponse = srResponse;

    at_response_free(p_response);
    p_response = NULL;
    free(cmd);
    cmd = NULL;

    if (sr->sw1 == 0x61) {
        length = 10;
        ins = 0xC0;
        p1 = 0x00;
        p2 = 0x00;
        p3 = sr->sw2;
        data = NULL;
        goto redo;
    } else if (sr->sw1 == 0x6c) {
        p3 = sr->sw2;
        goto redo;
    } else if ((sr->sw1 == 0x6A && sr->sw2 == 0x82) ||
                (sr->sw1 == 0x69 && sr->sw2 == 0x85) ||
                (sr->sw1 == 0x69 && sr->sw2 == 0x99)) {
        RLOGE("File not found");
        return RIL_E_NO_SUCH_ELEMENT;
    } else if (sr->sw1 == 0x6A && sr->sw2 == 0x84) {
        RLOGE("Not enough memory space in the file");
        return RIL_E_MISSING_RESOURCE;
    } else if (sr->sw1 != 0x90 && sr->sw1 != 0x91) { // wrong sw1, sw2
        if (ins == 0xA4 && data != NULL && p2 == 0x04) {
            // Retry only for "Select AID"
            if (checkRetryFCI(sr->sw1, sr->sw2) == 1) {
                p2 = 0x00;
                goto redo;
            }
        }
        if (sr->sw1 == 0x62 && sr->sw2 == 0x83) {
            LOGD("%s, select file deactivated!", __FUNCTION__);
        } else {
            return RIL_E_GENERIC_FAILURE;
        }
    }

    return RIL_E_SUCCESS;

error:
    sr->sw1 = 0x6f;
    sr->sw2 = 0x00;
    sr->simResponse = NULL;
    at_response_free(p_response);
    free(cmd);
    if (srResponse != NULL) {
        free(srResponse);
        srResponse = NULL;
    }
    return err;
}

static int requestSimChannelAccess(int sessionid, char * senddata, RIL_SIM_IO_Response * output)
{
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    char *line = NULL;
    int len = 0;
    int err = 0;

    LOGD("%s sessionid:%d, senddata:%s", __FUNCTION__, sessionid, senddata);

    if(senddata == NULL) {
        err = AT_ERROR_GENERIC;
        return err;
    }

    len = strlen(senddata);
    asprintf(&cmd,  "AT+CGLA=%d,%d,\"%s\"", sessionid, len, senddata);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, getChannelCtxbyId(AT_CHANNEL));
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        LOGD("%s, AT+CGLA send failed", __FUNCTION__);
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(output->simResponse));
    if (err < 0) goto error;

    sscanf(&(output->simResponse[len - 4]), "%02x%02x", &(output->sw1), &(output->sw2));
    output->simResponse[len - 4] = '\0';
    at_response_free(p_response);
    return RIL_E_SUCCESS;

error:
    at_response_free(p_response);
    return RIL_E_GENERIC_FAILURE;
}

/*return 0 W+C else W+G or other mode*/
static int getExternalModemSlotTelephonyMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    if (1 == getCdmaSocketSlotId()) {
        property_get("mtk_telephony_mode_slot1", property_value, "1");
    } else {
        property_get("mtk_telephony_mode_slot2", property_value, "1");
    }
    LOGD("%s property_value:%s", __FUNCTION__, property_value);
    return atoi(property_value);
}

static void setGsmRilUicctype(void)
{
    switch (getCdmaModemSlot())
    {
        case 1:
            if((cardtype == CT_UIM_SIM_CARD) || (cardtype == CT_4G_UICC_CARD)) {       //support app to visit UICC card phonebook
                property_set("gsm.ril.uicctype","CSIM");
            } else {
                property_set("gsm.ril.uicctype","RUIM");
            }
            break;
        case 2:
            if((cardtype == CT_UIM_SIM_CARD) || (cardtype == CT_4G_UICC_CARD)) {
                property_set("gsm.ril.uicctype.2","CSIM");
            } else {
                property_set("gsm.ril.uicctype.2","RUIM");
            }
            break;
        default:
            LOGE("unsupport slot id");
            break;
    }
}

#endif /** ADD_MTK_REQUEST_URC **/

static void requestSendUTKURCREG(void* param)
{
    /*
     * !beacause we send UTKURCREG in unsoilicited thread
     * !the result of this command can not returned to framework
     * !so we IGNOR the result of this command
     *
     **/
    at_send_command("AT+UTKURCREG", NULL, getChannelCtxbyId(AT_CHANNEL));
}

/**
 * Get the current card status.
 *
 * This must be freed using freeCardStatus.
 * @return: On success returns RIL_E_SUCCESS
 */
static int getCardStatus(VIA_RIL_CardStatus **pp_card_status) {
#ifndef ADD_MTK_REQUEST_URC
    static RIL_AppStatus app_status_array[] = {
        // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_RUIM_NETWORK1,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN }
/*    #ifdef ANDROID_KK
        // RUIM_ABSENT = 6
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_NOT_READY = 7
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_READY = 8
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_PIN = 9
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // RUIM_PUK = 10
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_NETWORK_PERSONALIZATION = 11
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN }
    #endif
    */
    };
#else
    static RIL_AppStatus app_status_array[] = {
  // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_RUIM_NETWORK1,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN }
    };

#endif
    RIL_CardState card_state;
    int num_apps;
    int chvCount[4];
    int i;
    int appstatusarraylen = 0;
    int ratSwitching = 0;
    char tmp[PROPERTY_VALUE_MAX] = {0};

    ratSwitching = getRatSwitching();
    property_get(PROPERTY_RIL_CT3G[getCdmaSocketSlotId() - 1], tmp, "");

    //MTK6592 add for UICC card, if uicc card insert, we change RIL_APPTYPE_RUIM to RIL_APPTYPE_CSIM
    LOGD("cardtype is:%d before change:%d", cardtype, app_status_array[1].app_type);
    char tmp_csim[PROPERTY_VALUE_MAX] = { 0 };
    char *isexist;
    property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp_csim, "");
    LOGD("%s: %s", PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp_csim);
    isexist = strstr(tmp_csim, "CSIM");
    if((cardtype == NOT_CT_UICC_CARD) || (cardtype == CT_4G_UICC_CARD)
    	|| ((cardtype == NEED_TO_INPUT_PIN) && isexist != NULL)) {
        appstatusarraylen = sizeof(app_status_array) / sizeof(RIL_AppStatus);
        LOGD("appstatusarraylen is:%d",appstatusarraylen);
        for (i = 1; i < appstatusarraylen; i++) {
            app_status_array[i].app_type = RIL_APPTYPE_CSIM;
        }
    }
    LOGD("cardtype is:%d after change:%d", cardtype, app_status_array[1].app_type);

    int sim_status = getRUIMStatus(UPDATE_RADIO_STATUS);
    if (sim_status == RUIM_ABSENT) {
        RLOGD("ratSwitching: %d, ct3g: %s", ratSwitching, tmp);
        if ((strcmp("1", tmp) == 0) && ratSwitching == 1) {
            RLOGD("ABSENT due to rat switch so change as PRESENT!");
            card_state = RIL_CARDSTATE_PRESENT;
        } else {
            card_state = RIL_CARDSTATE_ABSENT;
        }
        num_apps = 0;
    } else {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 1;
    }

    // Allocate and initialize base card status.
    VIA_RIL_CardStatus *p_card_status = malloc(sizeof(VIA_RIL_CardStatus));
    p_card_status->card_state = card_state;
    p_card_status->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    p_card_status->gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->cdma_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->num_applications = num_apps;

    // Initialize application status
    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_card_status->applications[i] = app_status_array[RUIM_ABSENT];
    }

    // Pickup the appropriate application status
    // that reflects sim_status for gsm.
    if (num_apps != 0) {
        if (isCdmaLteDcSupport()) {
            int typeCount = 0;
            char cardType[PROPERTY_VALUE_MAX] = {0};
            property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], cardType, "");
            LOGD("C2K getCardStatus cardType is:%s", cardType);

            char* type = cardType;
            char* out;
            int err = 0;
            while (at_tok_hasmore(&type)) {
                err = at_tok_nextstr(&type, &out);
                if (err < 0) {
                    continue;
                }
            	LOGD("C2K getCardStatus out is:%s", out);
                if (strcmp("USIM", out) == 0 || strcmp("SIM", out) == 0) {
                    typeCount|= 0x01;
                    continue;
                } else if (strcmp("CSIM", out) == 0 || strcmp("RUIM", out) == 0) {
                    typeCount|= 0x02;
                    continue;
                }
            }
            LOGD("C2K getCardStatus typeCount is:%d", typeCount);

            if (typeCount == 3) {
            	p_card_status->num_applications = 2;
                p_card_status->gsm_umts_subscription_app_index = 0;
                p_card_status->cdma_subscription_app_index = 1;
                // Get the correct app status
                p_card_status->applications[1] = app_status_array[sim_status];
	    } else {
	        // Only support one app, gsm
		p_card_status->num_applications = 1;
                p_card_status->gsm_umts_subscription_app_index = 0;
                p_card_status->cdma_subscription_app_index = 0;
                // Get the correct app status
                p_card_status->applications[0] = app_status_array[sim_status];
            }
        } else {
            // Only support one app, gsm
            p_card_status->num_applications = 1;
            p_card_status->gsm_umts_subscription_app_index = 0;
            p_card_status->cdma_subscription_app_index = 0;
            // Get the correct app status
            p_card_status->applications[0] = app_status_array[sim_status];
        }
    }

    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    p_card_status->pin1Count = chvCount[0];
    p_card_status->pin2Count = chvCount[1];
    p_card_status->puk1Count = chvCount[2];
    p_card_status->puk2Count = chvCount[3];

    *pp_card_status = p_card_status;
    return RIL_E_SUCCESS;
}

/**
 * Free the card status returned by getCardStatus
 */
static void freeCardStatus(VIA_RIL_CardStatus *p_card_status) {
    free(p_card_status);
}

/**
 * Get the CHV count
 *
 */
static void getChvCount(int * pin1Count, int * pin2Count, int * puk1Count, int * puk2Count) {
    ATResponse *p_response = NULL;
    int err;
    char *cpinLine;
    char *cpinResult;
    int  chvCount[3];
    int  i;

    *pin1Count = 0;
    *pin2Count = 0;
    *puk1Count = 0;
    *puk2Count = 0;

    LOGD("getChvCount");
    if (getRadioState() == RADIO_STATE_OFF || getRadioState() == RADIO_STATE_UNAVAILABLE) {
        LOGD("getChvCount error : RADIO_STATE OFF or UNAVAILABLE");
        goto error;
    }

    err = at_send_command_singleline("AT+CPINC?", "+CPINC:", &p_response, getChannelCtxbyId(AT_CHANNEL));

    if (err != 0) {
        LOGD("getChvCount error : AT+CPINC failed");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            LOGD("getChvCount error : CME_SIM_NOT_INSERTED");
            goto error;

        default:
            LOGD("getChvCount error : not CME_SUCCESS");
            goto error;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        LOGD("getChvCount error : at result error");
        goto error;
    }

    *pin1Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if(!cpinResult){
        LOGD("getChvCount error : puk1");
        goto error;
    }
    *puk1Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if(!cpinResult){
        LOGD("getChvCount error : pin2");
        goto error;
    }
    *pin2Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if(!cpinLine){
        LOGD("getChvCount error : puk2");
        goto error;
    }
    *puk2Count = atoi(cpinLine);
    LOGD("getChvCount : %d, %d, %d, %d", *pin1Count, *pin2Count, *puk1Count, *puk2Count);

    goto done;

error :
    *pin1Count = *pin2Count = *puk1Count = *puk2Count = -1;

done:
    #ifdef ADD_MTK_REQUEST_URC
    setPinPukCountLeftProperty(*pin1Count, *pin2Count, *puk1Count, *puk2Count);
    #endif
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
}

static void storePinCode(const char*  string )
{
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    updateNetCdmaPC(string);
    if (s_ps_on_desired) {
        psParam.enable = 1;
        psParam.force = 1;
        turnPSEnable((void *) &psParam);
    }
}


static int isRildReset(void)
{
    char magicFlag[PROPERTY_VALUE_MAX] = {0};
    int ret = 0;

    property_get("net.cdma.poker", magicFlag, "black.joker");

    if(strncmp(magicFlag, "red.joker", 9))
    {
        property_set("net.cdma.poker", "red.joker");
    }
    else
    {
        ret = 1;
    }

    LOGD("%s: ret is %d", __FUNCTION__, ret);
    return ret;
}


static void setCardStateProperty(VIA_CARD_STATE state)
{
    switch (state) {
        case SIM_CARD:                  //we should treat SIM_CARD(GSM card) as absent when cp is CT version(the requirement of CT spec)
            if(cpisct == 0) {                       //anyother cp except CT we should treat SIM_CARD as a valid card
                property_set("net.cdma.via.card.state", "valid");
                break;
            }
        case UNKOWN_CARD:
        case NEED_TO_INPUT_PIN:
        case CARD_NOT_INSERTED:
            property_set("net.cdma.via.card.state", "locked_or_absent");
            break;
        default:
            property_set("net.cdma.via.card.state", "valid");
            break;
    }
}


void reportPbSmsReady(void)
{
    int slotid = 0;
    RIL_onUnsolicitedResponse(RIL_UNSOL_SMS_READY_NOTIFICATION, NULL, 0);

    slotid = getCdmaModemSlot();
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

    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, NULL, 0);

    // make sure the ME have no unread smss
    readAllUnreadSmsOnME();
}

static void requestMTKGetPbStorageInfoFromRuim(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    int pbSize[4] = {0};
    int storageInfo[4] = {0};
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *storage = NULL;

    err = getPbSize(pbSize);
    if (err<0) goto error;

    LOGD( "requestMTKGetPbStorageInfoFromRuim: %d, %d, %d, %d", pbSize[0], pbSize[1], pbSize[2], pbSize[3] );
    storageInfo[1] = pbSize[1] - pbSize[0] + 1;
    storageInfo[2] = pbSize[2] - 1;
    storageInfo[3] = pbSize[3];

    // to get used
    //err = at_send_command_singleline("AT+CPBS=\"ME\"", "+CPBS:", &p_response);
    err = at_send_command_singleline("AT+CPBS?", "+CPBS:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || p_response==NULL || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("requestMTKGetPbStorageInfoFromRuim at_tok_start err");
        goto error;
    }

    // read "SM" eg.
    err = at_tok_nextstr(&line, &storage);
    if (err < 0) goto error;

    // read used length
    err = at_tok_nextint(&line, &storageInfo[0]);
    if (err < 0) {
        LOGE("requestMTKGetPbStorageInfoFromRuim at_tok_nextint err : number length");
        goto error;
    }

    // read total length
    err = at_tok_nextint(&line, &storageInfo[1]);
    if (err < 0) {
        LOGE("requestMTKGetPbStorageInfoFromRuim at_tok_nextint err : text length");
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, storageInfo, sizeof(storageInfo));

    at_response_free(p_response);
    setMTKPbStorageInfo(storageInfo[1], storageInfo[2], storageInfo[3]);
    return;

error:
    LOGE("requestGetPbSizeFromRuim return error = %d", err);
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestWriteMTKPbToRuim(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int ton = 0;
    int index = 0;
    char * number = NULL;
    char * name = NULL;
    int    coding = 1;
    RIL_MTK_Ruim_Phonebook_Record *pb = (RIL_MTK_Ruim_Phonebook_Record*)data;
    unsigned int alphaMaxLength = CDMA_UIM_ALPHAID_LENGTH;
    int numberMaxLength = CDMA_UIM_NUMBER_LENGTH;

    if (pb == NULL) {
        LOGE("mtk feature, requestWriteMTKPbToRuim: we got error data, pb = NULL.");
        return;
    }

    if (pb->type != 0) {
        LOGE("mtk feature, requestWriteMTKPbToRuim: just for ADN, type error.");
        return;
    }

    /*
        AT^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]]
    */
    index = pb->index -1;
    if (index < 0) {
        LOGE("mtk feature, requestWriteMTKPbToRuim: index < 0. index = %d", index);
        return;
    }

    numberMaxLength = getMTKPbNumberMaxLength();

    if (pb->number != NULL && strlen(pb->number) > numberMaxLength) {
        err = 1001;
        goto error;
    }

    number = (char *)malloc(numberMaxLength*sizeof(char)*4 + 1);
    memset(number, 0, numberMaxLength*sizeof(char)*4 + 1);
    if (pb->number != NULL && strlen(pb->number) > 0) {
        LOGD("mtk feature, requestWriteMTKPbToRuim: number before convert = %s", pb->number);
        convertNumberFromMTKFeature(pb->number, number, pb->ton);
        LOGD("mtk feature, requestWriteMTKPbToRuim: number after convert = %s", number);
    } else {
        LOGD("mtk feature, requestWriteMTKPbToRuim: to write NULL PhoneNumber");
    }

    if (strlen(number) > 0) {
        ton = pb->ton;
    } else {
        ton = 0;
    }

    if (pb->alphaid != NULL && strlen(pb->alphaid) > 0) {
        coding = getCodingFromMTKAlphaId(pb->alphaid);
    } else {
        coding = 2;
    }

    LOGD("mtk feature, requestWriteMTKPbToRuim: alphaid before convert = %s", pb->alphaid);
    alphaMaxLength = getMTKPbAlphaIDMaxLength();

    if (pb->alphaid != NULL) {
        if (coding == 1) {
            if ((strlen(pb->alphaid)/4) > (alphaMaxLength-1)/2) {
                err = 1002;
                goto error;
            }
        } else if (coding == 2) {
            if ((strlen(pb->alphaid)/4) > (alphaMaxLength)) {
                err = 1002;
                goto error;
            }
        }
    }

    name = (char *)malloc(alphaMaxLength*sizeof(char)*4 + 1);
    memset(name, 0, alphaMaxLength*sizeof(char)*4 + 1);
    if (pb->alphaid != NULL) {
        convertNameFromMTKFeature(name, pb->alphaid, coding);
    }

    LOGD("mtk feature, requestWriteMTKPbToRuim: alphaid after convert = %s", name);
    err = checkNameLength(name);
    if (err != 0) {
        goto error;
    }

    LOGD("mtk feature, requestWriteMTKPbToRuim: index = %d, number = %s, ton = %d, name = %s, coding = %d",
            index, number, ton, name, coding);

    if (0==ton)
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",,\"%s\",%d", index, number, name, coding);
    else
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",%d,\"%s\",%d", index, number, ton, name, coding);

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || p_response==NULL || (p_response->success == 0))
    {
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    if (number != NULL) {
        free(number);
        number = NULL;
    }

    if (name != NULL) {
        free(name);
        name = NULL;
    }

    LOGD("requestWriteMTKPbToRuim, write done, return!");
    return;

error:
    LOGE("requestWriteMTKPbToRuim ERROR");

    if (number != NULL) {
        free(number);
        number = NULL;
    }

    if (name != NULL) {
        free(name);
        name = NULL;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, err, NULL, 0);
}

/*
 *    add for mtk feature, just for mtk framework
 *    ready N phbs at once
*/
static void requestReadMTKPbFromRuim(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int bindex = 0;
    int eindex = 0;
    int count = 0;
    int currentindex = 0;
    char * number = NULL;
    int    type = 0;
    char * name = NULL;
    int    coding = 1;
    int i = 0;
    int countValide = 0;
    RIL_Ruim_Phonebook_Record  * o_pb = NULL;
    RIL_MTK_Ruim_Phonebook_Record  * pb = NULL;
    ATLine *p_cur = NULL;
    unsigned int maxAlphaIdLength = CDMA_UIM_ALPHAID_LENGTH;
    int maxNumberLength = CDMA_UIM_NUMBER_LENGTH;

    int *readInfo = (int*)data;
    // check is 3 paramer from rilj ?
    LOGD("type = %d, bindex = %d, eindex = %d", readInfo[0], readInfo[1], readInfo[2]);
    if (readInfo[0] != 0) {
        LOGE("mtk feature, requestReadNPbFromRuim: just for ADN, type error.");
        goto error;
    }

    bindex = readInfo[1] - 1;
    eindex = readInfo[2] - 1;

    if (bindex < 0) {
        LOGE("mtk feature, requestReadNPbFromRuim: bindex must >= 0, because we read from 0 through AT");
        goto error;
    }

    count = eindex -bindex + 1;
    if (count < 0) {
        LOGE("mtk feature, requestReadNPbFromRuim: error range read index from %d - %d.", bindex, eindex);
        goto error;
    }

    o_pb = (RIL_Ruim_Phonebook_Record *)alloca(count * sizeof(RIL_Ruim_Phonebook_Record));
    memset (o_pb, 0, count * sizeof(RIL_Ruim_Phonebook_Record));

    countValide = 0;
    for (currentindex = bindex; currentindex <= eindex; currentindex++) {
        //at_response_free(p_response);
        //p_response = NULL;
        asprintf(&cmd, "AT^CPBR=%d", currentindex);

        err = at_send_command_multiline (cmd, "^CPBR:", &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;

        if (err != 0 || p_response->success == 0) {
            LOGD("%s: err = %d", __FUNCTION__, err);
            goto error;
        }
        p_cur = p_response->p_intermediates;
        if (p_cur != NULL) {
            err = parsePb(p_cur->line, o_pb + countValide);
            if (err != 0) {
                continue;
            }
            countValide++;
        } else {
            continue;
        }
    }
    /* count the responses */
/*    for (i = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        i++;
    }

    if (i != count) {
        if (i < count) {
            count = i;
        }
    }


    for (countValide = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        err = parsePb(p_cur->line, o_pb + countValide);
        if (err != 0) {
            continue;
        }
        countValide++;
    }
*/
    pb = (RIL_MTK_Ruim_Phonebook_Record *)alloca(countValide * sizeof(RIL_MTK_Ruim_Phonebook_Record));
    memset (pb, 0, countValide * sizeof(RIL_MTK_Ruim_Phonebook_Record));

    maxAlphaIdLength = getMTKPbAlphaIDMaxLength();
    maxNumberLength = getMTKPbNumberMaxLength();

    for(i = 0; i < countValide ; i++) {
        pb[i].type = 0;
        pb[i].index = o_pb[i].index + 1;

        pb[i].number = (char *)malloc(maxNumberLength*4 + 1);
        if(NULL == pb[i].number)
        {
            LOGD("%s: failed to malloc pb[%d].number", __FUNCTION__, i);
            goto error;
        }
        memset(pb[i].number, 0, maxNumberLength*4 + 1);
        convertNumberToMTKFeature(o_pb[i].number, pb[i].number);

        pb[i].ton = o_pb[i].type;

        pb[i].alphaid = (char *)malloc(maxAlphaIdLength*4 +1);
        if(NULL == pb[i].alphaid)
        {
            LOGD("%s: failed to malloc pb[%d].alphaid", __FUNCTION__, i);
            goto error;
        }
        memset(pb[i].alphaid, 0, maxAlphaIdLength*4 + 1);
        convertUimAlphaIdToMTKFeature(pb[i].alphaid , o_pb[i].name);
        if ((strlen(pb[i].alphaid))%4 != 0) {
            LOGE("convertUimAlphaIdToMTKFeature error! after convert strlen = %d", strlen(pb[i].alphaid));
            memset(pb[i].alphaid, 0, maxAlphaIdLength*4 + 1);
        }
        if (true) {
            LOGD("mtk feature: o_pb[%d].index = %d pb[%d].index = %d", i, o_pb[i].index, i, pb[i].index);
            LOGD("mtk feature: o_pb[%d].number = %s pb[%d].number = %s", i, o_pb[i].number, i, pb[i].number);
            LOGD("mtk feature: o_pb[%d].ton = %d pb[%d].ton = %d", i, o_pb[i].type, i, pb[i].ton);
            LOGD("mtk feature: o_pb[%d].name = %s pb[%d].alphaid = %s", i, o_pb[i].name, i, pb[i].alphaid);
        }
    }

    LOGD("mtk feature: to complete read request, count = %d", countValide);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, pb, countValide * sizeof (RIL_MTK_Ruim_Phonebook_Record));

    at_response_free(p_response);

    for(i = 0; i < countValide ; i++) {
        if (pb != NULL) {
            if (pb[i].number != NULL) free(pb[i].number);
            if (pb[i].alphaid != NULL) free(pb[i].alphaid);
        }
    }

    return;

error:
    LOGE("mtk feature, requestReadNPbFromRuim ERROR");
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    for(i = 0; i < countValide ; i++) {
        if (pb != NULL) {
            if (pb[i].number != NULL) free(pb[i].number);
            if (pb[i].alphaid != NULL) free(pb[i].alphaid);
        }
    }
}

static int checkNameLength(char *name) {
    unsigned int nameMax = CDMA_UIM_ALPHAID_LENGTH;

    if (name == NULL) {
        return RIL_E_GENERIC_FAILURE;
    }

    nameMax = getMTKPbAlphaIDMaxLength();

    if (name != NULL) {
        if (name[0] == '8' && name[1] == '0') {
            if ((strlen(name) - 2)/4 > (nameMax/2 - 1) ) {
                return 1002;
            }
        } else if (name[0] == '8' && name[1] == '1') {

        } else if (name[0] == '8' && name[1] == '2') {

        } else {
            if(strlen(name) > nameMax) {
                return 1002;
            }
        }
    }
    return 0;
}


static int checkNumberLength(char *number) {
    unsigned int numberMax = CDMA_UIM_NUMBER_LENGTH;

    if (number == NULL) {
        return RIL_E_GENERIC_FAILURE;
    }

    numberMax = getMTKPbNumberMaxLength();

    if (number != NULL) {
        if(strlen(number) > numberMax) {
            return 1001;
        }
    }

    return 0;
}

static void setMTKPbStorageInfo(int max, int numberMax, int alphaMax) {
    s_PHB_max_capacity = max;
    s_PHB_number_max_bytes = numberMax;
    s_PHB_alphaid_max_bytes = alphaMax;
}

static int getMTKPbMaxCount() {
    return s_PHB_max_capacity;
}


void convertUimAlphaIdToMTKFeature(char * mtk_alphaid, char * via_alphaid) {
    int i = 0;
    int len = 0;
    int j = 0;
    int lenByte = 0;
    unsigned char alphaidBytes[256] = {0};
    unsigned char alphaidBytesAfterDecode[256] = {0};
    char resstr[3] = {0};
    char * temp = NULL;

    if (via_alphaid == NULL || mtk_alphaid == NULL)
    {
        return ;
    }

    len = strlen(via_alphaid);

    if (len <= 0) {
        return;
    }

    if (via_alphaid[0] == '8' && via_alphaid[1] == '0')
    {
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: to copy data, 0x80");
        for (i = 2; i < len; i++)
        {
            mtk_alphaid[i - 2] = via_alphaid[i];
        }
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: copyed data count = %d, 0x80", i);
    }
    else if (len > 4 && via_alphaid[0] == '8' && via_alphaid[1] == '1')
    {
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: to copy data, 0x81");
        lenByte = ConvertNum((unsigned char *)&via_alphaid[3]) + ConvertNum((unsigned char *)&via_alphaid[2])*16;
        if ((len - 4) < (lenByte + 1)*2) {
            LOGE("mtk feature, convertUimAlphaIdToMTKFeature: 0x81 , len = %d ,lenByte: %d , error string", len, lenByte);
            return;
        }
        alphaidBytes[0] = lenByte;
        temp = via_alphaid + 4;
        for ( i = 0; i < lenByte + 1; i++ )
        {
            alphaidBytes[i+1] = ConvertNum((unsigned char *)temp);
            temp++;
            alphaidBytes[i+1] = alphaidBytes[i+1] << 4 | ConvertNum((unsigned char *)temp);
            temp++;
        }
        ValPhbUniDecode0x81(alphaidBytesAfterDecode, alphaidBytes);
        for ( i = 0; i < lenByte*2; i++ ) {
            memset(resstr, 0, 3);
            if (alphaidBytesAfterDecode[i] == 0) {
                resstr[0] = resstr[1] = '0';
            } else {
                sprintf(resstr,"%0x",alphaidBytesAfterDecode[i]);
            }
            strncpy(mtk_alphaid+(2*i), resstr, 2);
        }
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: 0x81 , mtk_alphaid: %s ", mtk_alphaid);
    }
    else if (len > 4 && via_alphaid[0] == '8' && via_alphaid[1] == '2')
    {
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: to copy data, 0x82");
        lenByte = ConvertNum((unsigned char *)&via_alphaid[3]) + ConvertNum((unsigned char *)&via_alphaid[2])*16;
        if ((len - 4) < (lenByte + 2)*2) {
            LOGE("mtk feature, convertUimAlphaIdToMTKFeature: 0x82 , len = %d ,lenByte: %d , error string", len, lenByte);
            return;
        }
        alphaidBytes[0] = lenByte;
        temp = via_alphaid + 4;
        for ( i = 0; i < lenByte + 2; i++ )
        {
            alphaidBytes[i+1] = ConvertNum((unsigned char *)temp);
            temp++;
            alphaidBytes[i+1] = alphaidBytes[i+1] << 4 | ConvertNum((unsigned char *)temp);
            temp++;
        }
        ValPhbUniDecode0x82(alphaidBytesAfterDecode, alphaidBytes);
        for ( i = 0; i < lenByte*2; i++ ) {
            memset(resstr, 0, 3);
            if (alphaidBytesAfterDecode[i] == 0) {
                resstr[0] = resstr[1] = '0';
            } else {
                sprintf(resstr,"%0x",alphaidBytesAfterDecode[i]);
            }
            strncpy(mtk_alphaid+(2*i), resstr, 2);
        }
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: 0x82 , mtk_alphaid: %s ", mtk_alphaid);
    }
    else
    {
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: to copy data, assci");
        len = strlen(via_alphaid);
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: len = %d", len);
        for (i = 0; i < len; i++)
        {
            memset(resstr, 0, 3);
            mtk_alphaid[4*i] = mtk_alphaid[4*i + 1] = '0';
            sprintf(resstr,"%0x",via_alphaid[i]);
            strncpy(mtk_alphaid+(4*i+2), resstr, 2);
        }
        LOGD("mtk feature, convertUimAlphaIdToMTKFeature: resutl = %s, assci", mtk_alphaid);
    }

}

void convertNameFromMTKFeature(char * via_alphaid, char * mtk_alphaid, int coding) {
    unsigned int i = 0;
    unsigned int j = 0;
    int len = strlen(mtk_alphaid);

    LOGD("mtk alphaid length = %d", len);
    if (len == 0) {
        return;
    }
    if (coding == 1) {
        via_alphaid[0] = '8';
        via_alphaid[1] = '0';
        for(i = 0; i < strlen(mtk_alphaid); i++) {
            via_alphaid[i+2] = mtk_alphaid[i];
        }
    } else {
        for(j = 0; j < strlen(mtk_alphaid)/4; j++) {
            i = j*4 + 2;
            via_alphaid[j] = ConvertNum((unsigned char *)&mtk_alphaid[i+1]) + ConvertNum((unsigned char *)&mtk_alphaid[i])*16;
        }
        LOGD("mtk alphaid, have convert block = %d", j);
    }
}

void convertNumberFromMTKFeature(char *mtkNumber, char *ourNumber, int ton) {
    int i = 0;
    int j = 0;
    int len = 0;

    if (ourNumber == NULL || mtkNumber == NULL) {
        return ;
    }

    if (ton == 0x91) {
        ourNumber[j] = '+';
        j++;
    }

    len = strlen(mtkNumber);

    for(; i < len; i++) {
      switch (mtkNumber[i])
      {
        case 'P':
        case 'p':
          ourNumber[j++] = 'P';
          break;

        case 'W':
        case 'w':
          ourNumber[j++] = 'T';
          break;

        default:
          ourNumber[j++] = mtkNumber[i];
      }
    }
}

void convertNumberToMTKFeature(char *ourNumber, char *mtkNumber) {
    int i = 0;
    int j = 0;
    int len = 0;

    if (ourNumber == NULL || mtkNumber == NULL) {
        return ;
    }

    len = strlen(ourNumber);

    if (len <= 0) {
        return;
    }

    if (ourNumber[i] == '+') {
        i++;
    }

    for(; i < len; i++) {
      switch (ourNumber[i])
      {
        case 'P':
          mtkNumber[j++] = 'p'; // MTK framework just know lower case 'p'
          break;
        case 'T':
        case 't':
          mtkNumber[j++] = 'w'; // MTK framework just know lower case 'w'
          break;

        default:
          mtkNumber[j++] = ourNumber[i];
      }
    }
}

int getCodingFromMTKAlphaId(char *alphaid) {
    int i = 0;
    int countUS2 = 0;
    int len = strlen(alphaid);

    if (len%4 != 0 || len == 0) {
        return 2;
    }

    countUS2 = len/4;

    for (i = 0; i < countUS2; i++) {
        if (alphaid[i*4] != '0' || alphaid[i*4 + 1] != '0') {
            return 1;
        }
    }
    return 2;
}

/* recode the 0x81 to unicode, porting from feature phone*/
void ValPhbUniDecode0x81(unsigned char* DesStrP, unsigned char* SrcStrP)
{
  unsigned char i;
  unsigned char length;
  int basePoint;
  int tempValue;
  char*  charPtr = (char*)DesStrP;

  length = SrcStrP[0];

  basePoint = SrcStrP[1] << 7;

  for(i = 2; i < length + 2; i++)
  {
    if(SrcStrP[i] < 0x80)
    {
      *charPtr++ = 0;
      *charPtr++ = SrcStrP[i];
    }
    else
    {
      tempValue = basePoint + (SrcStrP[i] & 0x7f);
      *charPtr++ = (tempValue & 0x7f00) >> 8;
      *charPtr++ = tempValue & 0x00ff;
    }
  }
}

static unsigned int getMTKPbAlphaIDMaxLength() {
    return s_PHB_alphaid_max_bytes;
}

int getMTKPbNumberMaxLength() {
    return s_PHB_number_max_bytes;
}

/* recode the 0x82 to unicode, porting from feature phone */
void ValPhbUniDecode0x82(unsigned char* DesStrP, unsigned char* SrcStrP)
{
  unsigned char  i;
  int length;
  int basePoint;
  int tempValue;
  char*  charPtr = (char*)DesStrP;

  length = SrcStrP[0];

  basePoint = (SrcStrP[1] << 8) + SrcStrP[2];

  for(i = 3; i < length+3; i++)
  {
    if(SrcStrP[i] < 0x80)
    {
      *charPtr++ = 0;
      *charPtr++ = SrcStrP[i];
    }
    else
    {
      tempValue = basePoint + (SrcStrP[i]& 0x7f);
      *charPtr++ = (tempValue & 0xff00) >> 8;
      *charPtr++ = tempValue & 0x00ff;
    }
  }
}

/*
 * Because we use +CMTI for new CDMA sms reporting, when ril received the CMTI URC,
 * it will send CMGR and CMGD on receiving it, to read and delete this sms.
 * One word, the reading process is asynchronous. If the modem have reported the CMTI,
 * but some exceptions appeared before it send CMGR to read this sms, the modem sms space
 * will be occupyed forever.
 * So, in order to avoid this condition, we read all unread sms in CBP sms memory when
 * device is powered on, and delete them.
 *
 * NOTE: when we changed to use the +CMT to report new smss, we maybe not do this work,
 * for the +CMT is synchronous processing.
 *
 */
static void readAllUnreadSmsOnME(void) {
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    ATLine *p_cur = NULL;
    char *line = NULL;
    char* cmd = NULL;
    int i = 0;
    int indexCount = 0;
    int indexTemp = -1;
    int indexs[60];

    // if have no card, no need do this work
    // TODO: if changed another UIM card, how about that?
    if (uimInsertedStatus != UIM_STATUS_CARD_INSERTED) {
        LOGD("readAllUnreadSmsOnME, uimInsertedStatus != UIM_STATUS_CARD_INSERTED, return");
        return;
    }

    LOGD("to readAllUnreadSmsOnME");

    err = at_send_command_multiline ("AT^HCMGL=0", "^HCMGL:", &p_response,
            getChannelCtxbyId(MISC_CHANNEL));
    if ((err < 0) || (p_response->success == 0))
    {
        LOGD("readAllUnreadSmsOnME error, err = %d", err);
        goto end;
    }

    for ( p_cur = p_response->p_intermediates;
          p_cur != NULL;
          p_cur = p_cur->p_next) {
        char *line = p_cur->line;
        err = at_tok_start(&line);
        if (err < 0)
            goto end;

        err = at_tok_nextint(&line, &indexTemp);
        if (err < 0)
            goto end;

        if (indexTemp >= 0 && indexTemp < 60) {
            indexs[indexCount] = indexTemp;
            indexCount++;
        }
        indexTemp = -1;
    }

    LOGD("indexCount = %d", indexCount);

    for (i = 0; i < indexCount; i++) {
        LOGD("index[%d] = %d", i, indexs[i]);
        at_send_command("AT+CPMS=\"ME\"", NULL, getChannelCtxbyId(MISC_CHANNEL));

        asprintf(&cmd, "AT+CMGR=%d", indexs[i]);
        at_send_command(cmd, NULL, getChannelCtxbyId(MISC_CHANNEL));
        free(cmd);
        cmd = NULL;

        asprintf(&cmd, "AT+CMGD=%d", indexs[i]);
        at_send_command(cmd, NULL, getChannelCtxbyId(MISC_CHANNEL));
        free(cmd);
        cmd = NULL;
    }

    at_response_free(p_response);

    // sync rfs image only if have done read and delete unread sms operation,
    // and only for cbp version > CBP8.x
    // the function is referenced to requestIPORadioPowerOff(), copy from there
    if (indexCount > 0) {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM",
                &p_cbp_version_response, getChannelCtxbyId(MISC_CHANNEL));
        if (err < 0 || p_cbp_version_response->success == 0)
        {
            LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            goto end;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_start", __FUNCTION__);
            goto end;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
            goto end;
        }
        // there is no need to do this before CBP8.X
        if((NULL != cbp_version) && (!strncmp(cbp_version, "CBP8", 4)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, getChannelCtxbyId(MISC_CHANNEL));
            if (err < 0 || p_rfs_response->success == 0)
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                goto end;
            }
        }
    }

    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    return;

end:
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
}

static int parsePb(char *line, RIL_Ruim_Phonebook_Record *readPb)
{
    // set default value
    memset(readPb, 0, sizeof(RIL_Ruim_Phonebook_Record));
    readPb->index = -1;
    readPb->coding = 1;

    if (!strStartsWith(line, "^CPBR:")) {
        goto end;
    }

    int err = at_tok_start(&line);
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &(readPb->index));
    if (err < 0) goto end;

    err = at_tok_nextstr(&line, &(readPb->number));
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &(readPb->type));
    if (err < 0) goto end;

    err = at_tok_nextstr(&line, &(readPb->name));
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &(readPb->coding));
    if (err < 0) goto end;

end:
    return 0;
}

static void requestReadPbFromRuim(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_Ruim_Phonebook_Record readPb;
    char * cmd = NULL;
    int    pb_index = ((int   *)data)[0];

    memset(&readPb, 0, sizeof(RIL_Ruim_Phonebook_Record));
    readPb.index = pb_index;
    readPb.coding = 1;

    /*
        AT^CPBR=<index>
        ^CPBR: <index1>,<number>,<type>,<text>,<coding>
        ^CPBR:0,"880271",129,"80964879D1950B",1
    */
    // here we use at_send_command_multiline since AT^CPBR may return empty line
    asprintf(&cmd, "AT^CPBR=%d", pb_index);
    err = at_send_command_multiline(cmd, "^CPBR:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || p_response==NULL || (p_response->success == 0))
    {
        goto error;
    }

    ATLine *p_cur = p_response->p_intermediates;
    if (p_cur != NULL) {
        err = parsePb(p_cur->line, &readPb);
        if (err<0) goto error;
    }

end:
    LOGE( "requestReadPbFromRuim end: %d, %s, %d, %s, %d", readPb.index, readPb.number, readPb.type, readPb.name, readPb.coding );
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &readPb, sizeof(readPb));
    at_response_free(p_response);
    return;

error:
    LOGE( "requestReadPbFromRuim ERROR");
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestWritePbToRuim(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int index = 0;
    char * number = NULL;
    int    type = 0;
    char * name = NULL;
    int    coding = 1;

    assert (datalen >=  (5 * sizeof(char **)));
    /*
        AT^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]]
    */
    index     = ((int   *)data)[0];
    number = ((char **)data)[1];
    type     = ((int   *)data)[2];
    name   = ((char **)data)[3];
    coding     = ((int   *)data)[4];
    if (0==type)
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",,\"%s\",%d", index, number, name, coding);
    else
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",%d,\"%s\",%d", index, number, type, name, coding);

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || p_response==NULL || (p_response->success == 0))
    {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    LOGE("requestWritePbToRuim ERROR");
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestGetPbSizeFromRuim(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    int pbSize[4] = {0};

    err = getPbSize(pbSize);
    if (err<0) goto error;

    LOGE( "requestGetPbSizeFromRuim: %d, %d, %d, %d", pbSize[0], pbSize[1], pbSize[2], pbSize[3] );
    RIL_onRequestComplete(t, RIL_E_SUCCESS, pbSize, sizeof(pbSize));
    return;

error:
    LOGE("requestGetPbSizeFromRuim return error = %d", err);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static int getPbSize(int pbSize[])
{
    int i;
    int err = 0;
    int ret = 0;
    ATResponse *p_response = NULL;

    char *line = NULL;
    char *storage = NULL;

    /*
        AT+CPBS=SM
        OK
    */
    /*
    err = at_send_command("AT+CPBS=\"SM\"", NULL);
    if (err < 0) {
        LOGE("getPbSize error: AT+CPBS=\"SM\" ");
        goto error;
    }
    at_response_free(p_response);
    */

    /*
        AT^CPBR=?
        ^CPBR:(0-249),21,14
        OK
    */
    err = at_send_command_singleline("AT^CPBR=?", "^CPBR:", &p_response, getChannelCtxbyId(AT_CHANNEL));

    if (err < 0 || p_response==NULL || p_response->success == 0) {
        ret = -1;
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        LOGE("getPbSize at_tok_start err");
        ret = -1;
        goto error;
    }

    // read "(0-249)" eg.
    err = at_tok_nextstr(&line, &storage);
    if (err < 0) {
        ret = -1;
        goto error;
    }

    // read begin, end index
    err = sscanf(storage, "(%d-%d)", &pbSize[0], &pbSize[1]);
    if (err==EOF) {
        ret = -1;
        goto error;
    }

    // read number length
    err = at_tok_nextint(&line, &pbSize[2]);
    if (err < 0) {
        LOGE("getPbSize at_tok_nextint err : number length");
        ret = -1;
        goto error;
    }

    // read text length
    err = at_tok_nextint(&line, &pbSize[3]);
    if (err < 0) {
        LOGE("getPbSize at_tok_nextint err : text length");
        ret = -1;
        goto error;
    }

error:
    at_response_free(p_response);
    return ret;
}

static void requestReadIMSI(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    err = at_send_command_singleline ("AT+VCIMI", "+VCIMI", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GetIMSI error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr = %s",responseStr);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestGetUIMAUTH(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr[2] = {NULL};

    err = at_send_command_singleline ("AT^UIMAUTH=?", "^UIMAUTH:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GetUIMAUTH error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr[0]);
    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextstr(&line, &responseStr[1]);
    if (err < 0)
    {
        //not support MD5 maybe
    }

    //LOGD("responseStr[0] = %s, responseStr[1] = %s", responseStr[0], responseStr[1]);
    LOGD("responseStr[0] = %s", responseStr[0]);
    LOGD("responseStr[1] = %s", responseStr[1]);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(responseStr));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestMakeCAVE(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    char * cmd = NULL;

    char * pstrrandu = (char *)data;
    LOGD("pstrrandu = %s", pstrrandu);
    asprintf(&cmd, "AT^CAVE=%s", pstrrandu);
    err = at_send_command_singleline (cmd, "^CAVE:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("MakeCAVE error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr = %s",responseStr);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestMakeMD5(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    char * cmd = NULL;
    char * cmd1 = NULL;
    char str[5];

    const char** strings = (const char**)data;
    LOGW("strlen(strings[1])/2 = %d", strlen(strings[1])/2);
    itoa(str, 'd', strlen(strings[1])/2);
    LOGW("str = %s", str);
    LOGW("requestMakeMD51 strings[0] = %s , strings[1] = %s ,str = %s ",strings[0],strings[1],str);
    asprintf(&cmd, "AT^MD5=%s, %s", strings[0], str);
    LOGW("requestMakeMD51");
    err = at_send_command_singleline_cw (cmd, "^MD5:", strings[1], &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    LOGW("requestMakeMD52");
      /*
    line = p_response->p_intermediates->line;
    LOGW("requestMakeMD53");
    if (strStartsWith(line, "> "))
    {
    LOGW("strStartsWith");
        asprintf(&cmd1, "%s\0x1a", strings[1]);

        LOGW("cmd1 = %s", cmd1);

        err = at_send_command_singleline (cmd1, "^MD5:",  &p_response);
    }
    else
    {
        LOGW("error");
        goto  error;
    }
    */
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("MakeMD5 error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr = %s",responseStr);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestGetPREFMODE(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int responseStr = 0;

    err = at_send_command_singleline ("AT^PREFMODE?", "^PREFMODE:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GetPREFMODE error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr = %d",responseStr);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestGetUimid(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *  responseStr[2] = {NULL};

    err = at_send_command_singleline ("AT^GSN", "^GSN:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("requestGetUimid error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr[0]);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr[1]);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr[0] = %s, responseStr[1] = %s", responseStr[0], responseStr[1]);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *)*2);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestGenerateKEY(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr[2] = {NULL};

    err = at_send_command_singleline ("AT^VPM", "^VPM:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("GenerateKEY error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr[0]);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr[1]);
    if (err < 0)
    {
        goto error;
    }
    LOGW("responseStr");
    //responseStr[1] = responseStr[0];
    LOGD("responseStr[0] = %s, responseStr[1] = %s", responseStr[0], responseStr[1]);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *)*2);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestUpdateSSDCON(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    char* cmd = NULL;

    char * pcStrAuthbs = (char *)data;
    asprintf(&cmd, "AT^SSDUPDCFM=%s", pcStrAuthbs);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("UpdateSSDCON error!");
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}


static void requestUpdateSSD(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;
    char* cmd = NULL;

    char * pcStrRandssd = (char *)data;
    asprintf(&cmd, "AT^SSDUPD=%s", pcStrRandssd);
    err = at_send_command_singleline (cmd, "^SSDUPD:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("UpdateSSD error!");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    LOGD("responseStr = %s",responseStr);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void itoa(char *buf, int base, int d) {
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;

    /* If %d is specified and D is minus, put `-' in the head.  */
    if (base == 'd' && d < 0) {
        LOGW("base == d");
        *p++ = '-';
        buf++;
        ud = -d;
    } else if (base == 'x')
        divisor = 16;

    LOGW("divisor = %d", divisor);
    /* Divide UD by DIVISOR until UD == 0.  */
    do {
        int remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    /* Terminate BUF.  */
    *p = 0;

    /* Reverse BUF.  */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

void requestConfigModemStatus(void *data, size_t datalen, RIL_Token t)
{
    int modemStatus, remoteSimProtocol;
    int err;
    ATResponse *p_response = NULL;
    char *cmd;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    modemStatus = ((int *)data)[0];
    remoteSimProtocol = ((int *)data)[1];

    if ((modemStatus < 0 || modemStatus > 2) || (remoteSimProtocol < 0 || remoteSimProtocol > 2)) {
        LOGE("requestConfigModemStatus para error!");
    } else {
        err = asprintf(&cmd, "AT+EMDSTATUS=%d, %d", modemStatus, remoteSimProtocol);
        if(err >= 0) {
            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
            free(cmd);

            if ( !(err < 0 || p_response->success == 0) ) {
                ril_errno = RIL_E_SUCCESS;
            }
            property_set("ril.cdma.emdstatus.send", "1");
            LOGD("c2k rild set ril.cdma.emdstatus.send to 1");
        }
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
static bool isNeedToSetRadio(VIA_CARD_STATE card_type) {
    static VIA_CARD_STATE s_last_type = CARD_NOT_INSERTED;
    LOGD("%s: s_last_type(%d), card_type(%d).", __FUNCTION__, s_last_type,card_type);
    if ((NEED_TO_INPUT_PIN == s_last_type) &&
            ((CT_4G_UICC_CARD == card_type) || (NOT_CT_UICC_CARD == card_type))) {
        s_last_type = card_type;
        return true;
    } else {
        s_last_type = card_type;
        return false;
    }
}

static void TriggerMD3BootPower() {
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_card_response = NULL;

    LOGD("%s: current radio state is %d", __FUNCTION__, getRadioState());
    err = at_send_command("AT+CPOF", &p_response, getChannelCtxbyId(AT_CHANNEL));
    if (err < 0 || p_response->success == 0) {
        LOGE("%s: failed to send CPOF, err is %d", __FUNCTION__, err);
    } else {
        err = at_send_command("AT+CPON", &p_response, getChannelCtxbyId(AT_CHANNEL));
        if (err < 0|| p_response->success == 0) {
            LOGE("%s: failed to send CPON, err is %d", __FUNCTION__, err);
        }
    }
}
/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/

void resetSIMProperties(const char *p_sim) {
    RLOGD("%s: Reset %s to null", __FUNCTION__, p_sim);
    property_set(p_sim, "");
}
/**
 * Switch RUIM card to SIM or switch SIM to RUIM.
 */
void requestSwitchCardType(data, datalen, t) {
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    int cardType = ((int *) data)[0];
    LOGD("c2k requestSwitchCardType(), cardType = %d", cardType);
    // switch UTK/STK mode.
    err = switchStkUtkMode(cardType, t);
    if (err >= 0) {
        // radio off
        err = asprintf(&cmd, "AT+CPOF");
        LOGD("c2k requestSwitchCardType(), send command %s.", cmd);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    }
    LOGD("c2k requestSwitchCardType(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}


/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
//TODO: To merge code for ALPS01949738
void updateCardState(void *controller, int card_state) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;
    //Trigger to power on if (SIM PIN -> SIM READY) && (CPOF is sent)
    p_ctrl->refresh(p_ctrl, card_state);
    p_ctrl->card_state = card_state;
}

void setSimPower(void *controller) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;

    LOGD("%s: card_state:%d, is_poweroff:%d.",
            __FUNCTION__, p_ctrl->card_state, p_ctrl->is_poweroff);
    if ((NEED_TO_INPUT_PIN == p_ctrl->card_state)
            && (!p_ctrl->is_poweroff)) {
        p_ctrl->trigger(_FALSE);
        p_ctrl->is_poweroff = _TRUE;
    }
}

void refreshSimPower(void *controller, int current_card_state) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;

    LOGD("%s: current_card_state:%d, card_state:%d, is_poweroff:%d.",
            __FUNCTION__, current_card_state, p_ctrl->card_state, p_ctrl->is_poweroff);
    if ((NEED_TO_INPUT_PIN == p_ctrl->card_state)
            && ((UIM_CARD == current_card_state)
                    || (SIM_CARD == current_card_state)
                    || (UIM_SIM_CARD == current_card_state)
                    || (CT_3G_UIM_CARD == current_card_state)
                    || (CT_UIM_SIM_CARD == current_card_state)
                    || (CT_4G_UICC_CARD == current_card_state)
                    || (NOT_CT_UICC_CARD == current_card_state))
            && (p_ctrl->is_poweroff)) {
        RIL_requestProxyTimedCallback(
                requestSetSimPower, p_ctrl, NULL, getDefaultChannelCtx()->id);
    }
}

void triggerSetSimPower(int power) {
    PS_PARAM ps_param = {0, 1};
    ps_param.enable = power;
    turnPSEnable(&ps_param);
}

void requestSetSimPower(void *controller) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*) controller;
    LOGD("%s: is_poweroff:%d.", __FUNCTION__, p_ctrl->is_poweroff);
    p_ctrl->trigger(p_ctrl->is_poweroff);
    p_ctrl->is_poweroff = !p_ctrl->is_poweroff;
}
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

/**
  * Enable or disable MD3 Sleep
 */
void requestEnableMD3Sleep(data, datalen, t) {
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    int enable = ((int *) data)[0];
    LOGD("c2k requestEnableMD3Sleep(), enable = %d", enable);
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    err = asprintf(&cmd, "AT+ESLEEP=%d",enable);
    LOGD("c2k requestEnableMD3Sleep(), send command %s.", cmd);

    if (err >= 0) {
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);

        if (!(err < 0 || p_response->success == 0) ) {
             ril_errno = RIL_E_SUCCESS;
        }
     }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

