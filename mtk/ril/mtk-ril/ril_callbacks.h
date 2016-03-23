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

#ifndef RIL_CALLBACKS_H
#define RIL_CALLBACKS_H 1

#include <ril_sim.h>
#include <ril_stk.h>
#include <ril_cc.h>
#include <ril_ss.h>
#include <ril_sms.h>
#include <ril_data.h>
#include <ril_nw.h>
#include <ril_oem.h>

#ifdef RIL_SHLIB
extern const struct RIL_EnvSocket *s_rilenv;

#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c,d) s_rilenv->OnUnsolicitedResponse(a,b,c,d)
#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)
#ifdef MTK_RIL
#define RIL_queryMyChannelId(a) s_rilenv->QueryMyChannelId(a)
#define RIL_queryMyProxyIdByThread  s_rilenv->QueryMyProxyIdByThread()
#endif
#endif /* RIL_SHLIB */

extern const char *proxyIdToString(int id);
#define RIL_requestProxyTimedCallback(a,b,c,d,e) \
        RLOGD("%s request timed callback %s to %s", __FUNCTION__, e, proxyIdToString((int)d)); \
        s_rilenv->RequestProxyTimedCallback(a,b,c,(int)d)

extern const struct RIL_EnvSocket *s_rilsapenv;
#define RIL_SAP_onRequestComplete(t, e, response, responselen) s_rilsapenv->OnRequestComplete(t, e \
        , response, responselen)
#define RIL_SAP_onUnsolicitedResponse(unsolResponse, data, datalen, socket_id) \
        s_rilsapenv->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id)
#define RIL_SAP_requestTimedCallback(a,b,c) s_rilsapenv->RequestTimedCallback(a,b,c)
#define RIL_SAP_requestProxyTimedCallback(a,b,c,d,e) \
        RLOGD("%s request timed callback %s to %s", __FUNCTION__, e, proxyIdToString((int)d)); \
        s_rilsapenv->RequestProxyTimedCallback(a,b,c,(int)d)
#define RIL_SAP_queryMyChannelId(a) s_rilsapenv->QueryMyChannelId(a)
#define RIL_SAP_queryMyProxyIdByThread  s_rilsapenv->QueryMyProxyIdByThread()

static void *noopRemoveWarning( void *a ) { return a; }
#define RIL_UNUSED_PARM(a) noopRemoveWarning((void *)&(a));

/* RILcheckPendingRequest() used to query if any RIL request is pending currently */
extern int RILcheckPendingRequest(int request);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
extern RIL_RadioState getRadioState(RIL_SOCKET_ID rid);

static char PROPERTY_ICCID_SIM[4][25] = {
    "ril.iccid.sim1",
    "ril.iccid.sim2",
    "ril.iccid.sim3",
    "ril.iccid.sim4",
};

static char PROPERTY_SET_RC_SESSION_ID[4][25] = {
    "ril.rc.session.id1",
    "ril.rc.session.id2",
    "ril.rc.session.id3",
    "ril.rc.session.id4",
};

static const char PROPERTY_RIL_SIM_MCCMNC[4][25] = {
    "gsm.sim.ril.mcc.mnc",
    "gsm.sim.ril.mcc.mnc.2",
    "gsm.sim.ril.mcc.mnc.3",
    "gsm.sim.ril.mcc.mnc.4",
};
/// M: SVLTE solution2 modification. @{
typedef enum {
    TELEPHONY_MODE_UNKNOWN = -1,
    TELEPHONY_MODE_LC_G,
    TELEPHONY_MODE_LWTG_C,
    TELEPHONY_MODE_LWTG_G,
    TELEPHONY_MODE_G_LC,
    TELEPHONY_MODE_C_LWTG,
    TELEPHONY_MODE_G_LWTG,
    TELEPHONY_MODE_LC_SINGLE = 6,
    TELEPHONY_MODE_LWTG_SINGLE
} TELEPHONY_MODE_ID;
/// @}

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

#define PROPERTY_FLIGHT_MODE_POWER_OFF_MD "ril.flightmode.poweroffMD"

#define TELEPHONY_MODE_0_NONE 0
#define TELEPHONY_MODE_1_WG_GEMINI 1
#define TELEPHONY_MODE_2_TG_GEMINI 2
#define TELEPHONY_MODE_3_FDD_SINGLE 3
#define TELEPHONY_MODE_4_TDD_SINGLE 4
#define TELEPHONY_MODE_5_WGNTG_DUALTALK 5
#define TELEPHONY_MODE_6_TGNG_DUALTALK 6
#define TELEPHONY_MODE_7_WGNG_DUALTALK 7
#define TELEPHONY_MODE_8_GNG_DUALTALK 8

#define TELEPHONY_MODE_100_TDNC_DUALTALK 100
#define TELEPHONY_MODE_101_FDNC_DUALTALK 101
#define TELEPHONY_MODE_102_WNC_DUALTALK 102
#define TELEPHONY_MODE_103_TNC_DUALTALK 103

#define FIRST_MODEM_NOT_SPECIFIED 0
#define FIRST_MODEM_MD1 1
#define FIRST_MODEM_MD2 2

#define NETWORK_MODE_GSM_ONLY 1
#define NETWORK_MODE_WCDMA_PREF 3
#define NETWORK_MODE_GSM_UMTS_LTE 7

#define NETWORK_MODE_GSM_ONLY 1
#define NETWORK_MODE_WCDMA_PREF 3
#define CAPABILITY_3G_SIM1 1
#define CAPABILITY_3G_SIM2 2
#define CAPABILITY_3G_SIM3 3
#define CAPABILITY_3G_SIM4 4
#define CAPABILITY_NO_3G -1

/* SIM mode. It's bitmap value. bit1 for SIM1, bit 2 for SIM2 ,bit 3 for SIM3 ... */
#define RADIO_MODE_POWER_OFF -1
#define RADIO_MODE_FLIGHT_MODE 0
#define RADIO_MODE_SIM1_ONLY 1
#define RADIO_MODE_SIM2_ONLY 2
#define RADIO_MODE_SIM3_ONLY 4
#define RADIO_MODE_SIM4_ONLY 8
#define RADIO_MODE_DUAL_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY
#define RADIO_MODE_TRIPLE_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY | RADIO_MODE_SIM3_ONLY
#define RADIO_MODE_QUAD_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY | RADIO_MODE_SIM3_ONLY | RADIO_MODE_SIM4_ONLY

extern int RIL_get3GSIM();
extern void setSimSwitchProp(int SimId);
extern int RIL_is3GSwitched();
extern void sendSetPhoneRatUrc(int sim3G);
extern int getTelephonyMode();
extern int getFirstModem();

extern int isEmulatorRunning();
extern int isCCCIPowerOffModem();
extern int isDualTalkMode();
extern void mtk_initializeCallback(void *param);
extern int getExternalModemSlot();
extern int isInternationalRoamingEnabled();
extern int isEVDODTSupport();
extern int getExternalModemSlotTelephonyMode();
extern int isSupportSimHotSwapC2k();
extern int isSupportCommonSlot();
extern int isBootupWith3GCapability();
extern int isWorldPhoneSupport();
extern int isGeminiSupport();
extern void setMSimProperty(int phoneId, char *pPropertyName, char *pUpdateValue);
extern void getMSimProperty(int phoneId, char *pPropertyName,char *pPropertyValue);
extern int isLteSupport();
extern int isImsSupport();
extern int isEpdgSupport();
extern void onDualApnCheckNetworkChange(void* param);
extern int handleAee(const char *modem_warning, const char *modem_version);
extern int isCdmaLteDcSupport();
extern int isSrlteSupport();
extern int isSvlteSupport();
extern int isCdmaSupport();
extern int isNeedCombineAttach();
extern int isCdmaIratSupport();
extern int isSvlteLcgSupport();
/// M: SVLTE solution2 modification. @{
extern int getActiveSvlteModeSlotId(void);
extern int getActualSvlteModeSlotId(void);
extern int getCdmaSocketSlotId(void);
extern int getSvlteTelephonyMode(void);
extern int getSimCount();
/// M: SAP start
extern void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);
extern int getMtkShareModemCurrent();
extern void getLogicModemId();
extern int isDisableCapabilitySwitch();
/// @}
/// M: Wfc @{
extern bool isWfcSupport();
/// @}
/// M: Ims init flow @{
extern void imsInit(RIL_SOCKET_ID rid);
/// @}
extern int getSvlteProjectType();
extern int isWorldModeSwitching(const char *s);
#endif /* RIL_CALLBACKS_H */
