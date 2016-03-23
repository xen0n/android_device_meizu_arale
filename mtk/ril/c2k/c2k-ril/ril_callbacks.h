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

#ifndef RIL_CALLBACKS_H
#define RIL_CALLBACKS_H 1

#include <telephony/via-ril.h>
#include <telephony/oem-ril.h>
#include <telephony/ril.h>

#include <c2k_ril_data.h>
#include <ril_nw.h>
#include <ril_cc.h>
#include <ril_sim.h>
#include <ril_sms.h>
#include <ril_ss.h>
#include <ril_utk.h>
#include <ril_oem.h>

//#define AT_AUTO_SCRIPT_RESET
#define AT_AUTO_SCRIPT_MODE
#define FLASHLESS_SUPPORT

#ifdef FLASHLESS_SUPPORT
#include <c2kutils.h>
#endif

/* do not turn off ps even in absent or pin locked state
to display signal icon for emergency call in CTA test*/
#define OPEN_PS_ON_CARD_ABSENT
#define LOG_TAG "C2K_RIL"
#include <utils/Log.h>
#define MAX_AT_RESPONSE 0x1000
#ifdef AT_AUTO_SCRIPT_MODE
static int forceEnableModeReport = 0;
static int ctclientServiceMode = 0;
#endif

#define USB_FS_EXCEPTION    /*reset or power off and on cbp if something wrong with usb file*/

#ifdef ANDROID_KK
/* Modem Technology bits */
#define MDM_GSM         0x01
#define MDM_WCDMA       0x02
#define MDM_CDMA        0x04
#define MDM_EVDO        0x08
#define MDM_LTE         0x10

typedef struct {
    int supportedTechs; // Bitmask of supported Modem Technology bits
    int currentTech;    // Technology the modem is currently using (in the format used by modem)
    int isMultimode;

    // Preferred mode bitmask. This is actually 4 byte-sized bitmasks with different priority values,
    // in which the byte number from LSB to MSB give the priority.
    //
    //          |MSB|   |   |LSB
    // value:   |00 |00 |00 |00
    // byte #:  |3  |2  |1  |0
    //
    // Higher byte order give higher priority. Thus, a value of 0x0000000f represents
    // a preferred mode of GSM, WCDMA, CDMA, and EvDo in which all are equally preferrable, whereas
    // 0x00000201 represents a mode with GSM and WCDMA, in which WCDMA is preferred over GSM
    int32_t preferredNetworkMode;
    int subscription_source;

} ModemInfo;

// TECH returns the current technology in the format used by the modem.
// It can be used as an l-value
#define TECH(mdminfo)                 ((mdminfo)->currentTech)
// TECH_BIT returns the bitmask equivalent of the current tech
#define TECH_BIT(mdminfo)            (1 << ((mdminfo)->currentTech))
#define IS_MULTIMODE(mdminfo)         ((mdminfo)->isMultimode)
#define TECH_SUPPORTED(mdminfo, tech) ((mdminfo)->supportedTechs & (tech))
#define PREFERRED_NETWORK(mdminfo)    ((mdminfo)->preferredNetworkMode)
// CDMA Subscription Source
#define SSOURCE(mdminfo)              ((mdminfo)->subscription_source)

#endif /* ANDROID_KK */

#define UIM_STATUS_NO_CARD_INSERTED 0
#define UIM_STATUS_CARD_INSERTED 1
#define UIM_STATUS_INITIALIZER 2

typedef struct {
    int enable;
    int force;
} PS_PARAM;

/*PCModem state*/
typedef enum {
    PCMODEM_DISCONNECT = 0,
    PCMODEM_CONNECTING = 1,
    PCMODEM_CONNECTED = 2,
} PcModem_State;

typedef struct{
    int isDataCall;
    int isVoice;
} RIL_CALL_STATUS;

typedef enum {
    AT_CHANNEL_TIMEOUT = 1,
    NO_DEVICE_PORT = 2,
    DATA_CALL_FAILED_CONTINOUSLY = 3
} CBP_RESET_CASE;

typedef struct {
    int register_state;
    int service_state;
    int radio_technology;
    int roaming_indicator;

    int need_read_desp_fr_bp;

    char mcc[8];
    char mnc[8];

    char esn[16];

    RIL_SignalStrength_v6 signal_strength;  //need check
    RIL_LastCallFailCause last_call_fail_cause;
    RIL_DataCallFailCause last_data_call_activate_fail_cause;
    int x1_service_state;
    int evdo_service_state;
} RIL_UNSOL_Msg_Cache;

typedef enum {
    RUIM_ABSENT = 0,
    RUIM_NOT_READY = 1,
    RUIM_READY = 2, /* RUIM_READY means the radio state is RADIO_STATE_RUIM_READY */
    RUIM_PIN = 3,
    RUIM_PUK = 4,
    RUIM_NETWORK_PERSONALIZATION = 5
} RUIM_Status;

typedef enum {
    DO_NOT_UPDATE = 0,
    UPDATE_RADIO_STATUS = 1
} RADIO_STATUS_UPDATE;

typedef enum {
    //CDMA non-CT card
    UIM_CARD = 1,
    SIM_CARD = 2,
    //CDMA non-CT card
    UIM_SIM_CARD = 3,
    UNKOWN_CARD = 4,
    //CT single mode card
    CT_3G_UIM_CARD = 5,
    //CT dual mode card
    CT_UIM_SIM_CARD = 6,
    NEED_TO_INPUT_PIN = 7,
    //CT dual mode card
    CT_4G_UICC_CARD = 8,
    NOT_CT_UICC_CARD = 9,
    /* legacy types, begin 2015/01/29 {*/
#ifdef MTK_CBP
    CDMA_CSIM_CARD = 10,
    CDMA_CSIM_USIM_CARD = 11,
    CDMA_CSIM_USIM_ISIM_CARD = 12,
    CDMA_USIM_CARD = 13,
    CDMA_USIM_ISIM_CARD = 14,
    CT_CSIM_CARD = 15,
    CT_CSIM_USIM_CARD = 16,
    CT_CSIM_USIM_ISIM_CARD = 17,
#endif
    /* legacy types, end 2015/01/29 }*/
    LOCKED_CARD = 18,
    IVSR_LOST = 19,
    ROMSIM_VERSION = 240,
    CARD_NOT_INSERTED = 255
} VIA_CARD_STATE;

typedef enum {
    SET_VALID_ICCID = 1,
    SET_TO_NA = 2,
    CLEAN_ICCID = 3
} VIA_ICCID_TYPE;

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
#ifdef ANDROID_KK
    SIM_NETWORK_PERSONALIZATION = 5,
    RUIM_ABSENT = 6,
    RUIM_NOT_READY = 7,
    RUIM_READY = 8,
    RUIM_PIN = 9,
    RUIM_PUK = 10,
    RUIM_NETWORK_PERSONALIZATION = 11
#else
    SIM_NETWORK_PERSONALIZATION = 5
#endif
} SIM_Status;

typedef enum {
    SVLTE_PROJ_DC_3M = 3,
    SVLTE_PROJ_DC_4M = 4,
    SVLTE_PROJ_DC_5M = 5,
    SVLTE_PROJ_DC_6M = 6,
    SVLTE_PROJ_SC_3M = 103,
    SVLTE_PROJ_SC_4M = 104,
    SVLTE_PROJ_SC_5M = 105,
    SVLTE_PROJ_SC_6M = 106,
} SVLTE_PROJ_TYPE;

#ifdef RIL_SHLIB
extern const struct RIL_Env *s_rilenv;
#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c) s_rilenv->OnUnsolicitedResponse(a,b,c)
#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)
#define RIL_requestProxyTimedCallback(a,b,c,d) s_rilenv->RequestProxyTimedCallback(a,b,c,(int)d)
#endif /*RIL_SHLIB*/

/*Get and set Radio state**/
extern RIL_RadioState getRadioState(void);
extern void setRadioState(RIL_RadioState newState);

extern void waitForClose();

/* for cbp reset */
void reset_cbp(CBP_RESET_CASE type);
int IsDatacallDialed(RIL_CALL_STATUS* callStatus);

#ifdef FLASHLESS_SUPPORT
void waitForAdjustPortPath(int interface_num, char devicePath[]);
#else
int waitForAdjustPortPath(int interface_num, char *sysfsdrv, char devpath[]);
#endif

#ifdef USB_FS_EXCEPTION
/*for power off and on cbp*/
extern void powerCbp(int type);
extern int CmpBypassMode(void);
#endif /* USB_FS_EXCEPTION */

extern int CmpPcModemMode(void);
extern int CmpCbpResetMode(void);

extern void findPidByName( char* pidName, pid_t *number);

extern int isCdmaLteDcSupport();
extern int isSrlteSupport();
extern int isSvlteSupport();
extern int isCdmaIratSupport();
extern int isSvlteLcgSupport();
/// M: SVLTE solution2 modification. @{
extern int getActiveSvlteModeSlotId(void);
extern int getCdmaSocketSlotId(void);
extern int getActualSvlteModeSlotId(void);
extern int getSimCount(void);
/// @}
extern int getSvlteProjectType();
#endif /* RIL_CALLBACKS_H */