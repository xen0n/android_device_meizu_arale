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

#ifndef RIL_SIM_H
#define RIL_SIM_H 1

#include <stdbool.h>
#include <hardware/ril/librilutils/proto/sap-api.pb.h>

#define PROPERTY_RIL_ECCLIST    "ro.ril.ecclist"
#define PROPERTY_RIL_ECCLIST_2  "ro.ril.ecclist.2"
#define PROPERTY_RIL_ISIM_SESSION_ID "gsm.sim.ril.isim.session.id"

static void *noopRemoveSimWarning( void *a ) { return a; }
#define RIL_SIM_UNUSED_PARM(a) noopRemoveSimWarning((void *)&(a));

/* -1: invalid, 0: non test sim, 1:  test sim */
static int isTestSim[4] = {-1,-1,-1,-1};

static const char PROPERTY_RIL_TEST_SIM[4][25] = {
    "gsm.sim.ril.testsim",
    "gsm.sim.ril.testsim.2",
    "gsm.sim.ril.testsim.3",
    "gsm.sim.ril.testsim.4",
};

static const char PROPERTY_RIL_SIM_PIN1[4][25] = {
    "gsm.sim.retry.pin1",
    "gsm.sim.retry.pin1.2",
    "gsm.sim.retry.pin1.3",
    "gsm.sim.retry.pin1.4",
};

static const char PROPERTY_RIL_SIM_PUK1[4][25] = {
    "gsm.sim.retry.puk1",
    "gsm.sim.retry.puk1.2",
    "gsm.sim.retry.puk1.3",
    "gsm.sim.retry.puk1.4",
};

static const char PROPERTY_RIL_SIM_PIN2[4][25] = {
    "gsm.sim.retry.pin2",
    "gsm.sim.retry.pin2.2",
    "gsm.sim.retry.pin2.3",
    "gsm.sim.retry.pin2.4",
};

static const char PROPERTY_RIL_SIM_PUK2[4][25] = {
    "gsm.sim.retry.puk2",
    "gsm.sim.retry.puk2.2",
    "gsm.sim.retry.puk2.3",
    "gsm.sim.retry.puk2.4",
};

static const char PROPERTY_RIL_UICC_TYPE[4][25] = {
    "gsm.ril.uicctype",
    "gsm.ril.uicctype.2",
    "gsm.ril.uicctype.3",
    "gsm.ril.uicctype.4",
};

static const char PROPERTY_RIL_FULL_UICC_TYPE[4][25] = {
    "gsm.ril.fulluicctype",
    "gsm.ril.fulluicctype.2",
    "gsm.ril.fulluicctype.3",
    "gsm.ril.fulluicctype.4",
};

static const char PROPERTY_RIL_CT3G[4][25] = {
    "gsm.ril.ct3g",
    "gsm.ril.ct3g.2",
    "gsm.ril.ct3g.3",
    "gsm.ril.ct3g.4",
};


static const char PROPERTY_RIL_PHB_READY[4][25] = {
    "gsm.sim.ril.phbready",
    "gsm.sim.ril.phbready.2",
    "gsm.sim.ril.phbready.3",
    "gsm.sim.ril.phbready.4",
};

static const char PROPERTY_ECC_LIST[4][25] = {
    "ril.ecclist",
    "ril.ecclist1",
    "ril.ecclist2",
    "ril.ecclist3",
};

// SVLTE
static const char PROPERTY_RIL_UICC_3GPP_TYPE[4][25] = {
    "gsm.ril.uicc.3gpptype",
    "gsm.ril.uicc.3gpptype.2",
    "gsm.ril.uicc.3gpptype.3",
    "gsm.ril.uicc.3gpptype.4",
};

static const char PROPERTY_RIL_UICC_3GPP2_TYPE[4][25] = {
    "gsm.ril.uicc.3gpp2type",
    "gsm.ril.uicc.3gpp2type.2",
    "gsm.ril.uicc.3gpp2type.3",
    "gsm.ril.uicc.3gpp2type.4",
};

static const char PROPERTY_RIL_CARD_TYPE_SET[25] = "gsm.ril.cardtypeset";
static const char PROPERTY_RIL_CARD_TYPE_SET_2[25] = "gsm.ril.cardtypeset.2";
static const char PROPERTY_CONFIG_EMDSTATUS_SEND[25] = "ril.cdma.emdstatus.send";

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    /* Add for USIM support */
    USIM_READY = 6,
    USIM_PIN = 7,
    USIM_PUK = 8,
    SIM_BUSY = 9,
    SIM_NP = 10,
    SIM_NSP = 11,
    SIM_SP = 12,
    SIM_CP = 13,
    SIM_SIMP =14,
    SIM_PERM_BLOCKED = 15, // PERM_DISABLED
    USIM_PERM_BLOCKED = 16, // PERM_DISABLED
    RUIM_ABSENT = 17,
    RUIM_NOT_READY = 18,
    RUIM_READY = 19,
    RUIM_PIN = 20,
    RUIM_PUK = 21,
    RUIM_NETWORK_PERSONALIZATION = 22,
    USIM_NP = 23,
    USIM_NSP = 24,
    USIM_SP = 25,
    USIM_CP = 26,
    USIM_SIMP =27,
    USIM_NOT_READY =28
} SIM_Status;

typedef enum {
    ISIM_ABSENT = 0,
    ISIM_NOT_READY = 1,
    ISIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    ISIM_PIN = 3,
    ISIM_PUK = 4,
    ISIM_BUSY = 5,
    ISIM_PERM_BLOCKED = 6, // PERM_DISABLED
} ISIM_Status;

typedef enum {
    ENTER_PIN1,
    ENTER_PIN2,
    ENTER_PUK1,
    ENTER_PUK2,
    CHANGE_PIN1,
    CHANGE_PIN2
} SIM_Operation;

typedef enum {
    PINUNKNOWN,
    PINCODE1,
    PINCODE2,
    PUKCODE1,
    PUKCODE2
}SimPinCodeE;

typedef enum {
    AID_USIM,
    //AID_SIM,
    AID_ISIM,

    AID_MAX
} AidIndex;


typedef struct {
    int pin1;
    int pin2;
    int puk1;
    int puk2;
} SimPinCount;

typedef struct{
    int catagory;
    int state;
    int retry_cnt;
    int autolock_cnt;
    int num_set;
    int total_set;
    int key_state;
}LockCatInfo;

typedef struct{
    LockCatInfo catagory[7];
    char imsi[16];
    int isgid1;
    char gid1[16];
    int isgid2;
    char gid2[16];
    int mnclength;
}SimLockInfo;

typedef enum {
    AUTHENTICATE_AKA,
    AUTHENTICATE_GBA_BOOTSTRAP,
    AUTHENTICATE_GBA_NAF,

    AUTENTICATE_END
}RilUiccAuthenticatioMode;

typedef enum
{
   CPBW_ENCODE_IRA,
   CPBW_ENCODE_UCS2,
   CPBW_ENCODE_UCS2_81,
   CPBW_ENCODE_UCS2_82,
   CPBW_ENCODE_GSM7BIT,
   CPBW_ENCODE_MAX
}RilPhbCpbwEncode;

typedef struct {
    int rid;
    char* urc;
} TimedCallbackParam;

#define RIL_PHB_UCS2_81_MASK    0x7f80

#define RIL_MAX_PHB_NAME_LEN 40   // Max # of characters in the NAME
#define RIL_MAX_PHB_ENTRY 10

#define AID_PREFIX_LEN 14

extern void pollSIMState(void * param);
extern void getPINretryCount(SimPinCount *result, RIL_Token t, RIL_SOCKET_ID rid);

extern int rilSimMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilSimUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern int requrstSimChannelAccess(RILChannelCtx *p_channel, int sessionid, char * senddata, RIL_SIM_IO_Response* output); // NFC SEEK


extern void resetSIMProperties(RIL_SOCKET_ID rid);

/* MTK proprietary start */
extern void queryNetworkLock(void *data, size_t datalen, RIL_Token t);
extern void simNetworkLock(void *data, size_t datalen, RIL_Token t);
extern void requestConnectSIM(void *data, size_t datalen, RIL_Token t);
extern void requestDisconnectOrPowerOffSIM(void *data, size_t datalen, RIL_Token t);
extern void requestPowerOnOrResetSIM(void *data, size_t datalen, RIL_Token t);
extern void requestTransferApdu(void *data, size_t datalen, RIL_Token t);
extern void requestConnectSIM(void *data, size_t datalen, RIL_Token t);
extern void requestDisconnectOrPowerOffSIM(void *data, size_t datalen, RIL_Token t);
extern void requestPowerOnOrResetSIM(void *data, size_t datalen, RIL_Token t);
extern void requestTransferApdu(void *data, size_t datalen, RIL_Token t);
extern void sendBTSIMProfile(void *data, size_t datalen, RIL_Token t);
extern void requestIccId(void *data, size_t datalen, RIL_Token t);
extern void localRequestSimAuthentication(void *data, size_t datalen, RIL_Token t);
extern void localRequestUsimAuthentication(void *data, size_t datalen, RIL_Token t);
extern void requestGetPOLCapability(void *data, size_t datalen, RIL_Token t);
extern void requestGetPOLList(void *data, size_t datalen, RIL_Token t);
extern void requestSetPOLEntry(void *data, size_t datalen, RIL_Token t);
extern void requestDetectSimMissing(void *data, size_t datalen, RIL_Token t);
extern void requestGeneralSimAuth(void *data, size_t datalen, RIL_Token t);

//ISIM
#define MAX_AID_LEN 33
#define MAX_AID_LABEL_LEN 256

typedef struct {
    int appId;
    int session;
}SessionInfo;

typedef struct {
    int aid_len;
    int app_label_len;
    char aid[MAX_AID_LEN];
    char appLabel[MAX_AID_LABEL_LEN];
}AidInfo;

extern int queryIccApplicationChannel(int appId, RIL_Token t);
extern void requestOpenIccApplication(void *data, size_t datalen, RIL_Token t);
extern void requestGetIccApplicationStatus(void *data, size_t datalen, RIL_Token t);
extern void onSessionIdChanged(const char *s, RIL_SOCKET_ID rid);
extern void resetAidInfo(RIL_SOCKET_ID rid);
extern AidInfo* getAidInfo(RIL_SOCKET_ID rid, AidIndex index);
extern SessionInfo* getIsimSessionInfo(RIL_SOCKET_ID rid);
extern int turnOnIsimApplication(RIL_Token t, RIL_SOCKET_ID rid);
extern int queryAppType(char* pAid);
extern void queryEfDir(RIL_SOCKET_ID rid);
extern int getIccApplicationStatus(RIL_CardStatus_v6 **pp_card_status, RIL_SOCKET_ID rid,
        int sessionId);
extern int hexStringToByteArrayEx(unsigned char* hexString, int hexStringLen,
        unsigned char ** byte);
extern void setSimInsertedStatus(RIL_SOCKET_ID rid, int isInserted);


// NFC SEEK start
extern void requestSIM_OpenChannel(void *data, size_t datalen, RIL_Token t);
extern void requestSIM_CloseChannel(void *data, size_t datalen, RIL_Token t);
extern void requestSIM_TransmitBasic(void *data, size_t datalen, RIL_Token t);
extern void requestSIM_TransmitChannel(void *data, size_t datalen, RIL_Token t);
extern void requestSIM_GetATR(void *data, size_t datalen, RIL_Token t);
extern void requestSIM_OpenChannelWithSw(void *data, size_t datalen, RIL_Token t);
// NFC SEEK end
extern void requestSimInterfaceSwitch(void *data, size_t datalen, RIL_Token t);
extern void onUsimDetected(const char *s, RIL_SOCKET_ID rid);
extern void onEfCspPlmnModeBitDetected(const char *s, RIL_SOCKET_ID rid);
extern void onTestSimDetected(const char *s, RIL_SOCKET_ID rid);
extern void onCt3gDetected(const char *s, RIL_SOCKET_ID rid);
extern bool RIL_isTestSim(RIL_SOCKET_ID rid);

extern int getActiveLogicalChannelId(char *aid); // WiFi Calling, export to stk
extern void requestUiccSelectApp(void *data, size_t datalen, RIL_Token t);
extern void requestUiccDeactivateApp(void *data, size_t datalen, RIL_Token t);
extern void reqeustUiccIO(void *data, size_t datalen, RIL_Token t);
extern void requestUiccAuthentication(void *data, size_t datalen, RIL_Token t, RilUiccAuthenticatioMode mode);

/* MTK proprietary end */

extern void requestMccMncForBootAnimation(RIL_SOCKET_ID rid); // Regional Phone: boot animation

extern int isSimInserted(RIL_SOCKET_ID rid);
extern int sim_inserted_status;
extern void requestSimReset(RIL_SOCKET_ID rid);
extern void resetSimForCt3g(RIL_SOCKET_ID rid);
extern void requestSimInsertStatus(RIL_SOCKET_ID rid);

//External SIM [Start]
extern void requestSwitchExternalSim(RIL_SOCKET_ID rid);
extern void onVsimEventDetected(const char *s, RIL_SOCKET_ID rid);
extern void requestVsimNotification(void *data, size_t datalen, RIL_Token t);
extern void requestVsimOperation(void *data, size_t datalen, RIL_Token t);
//External SIM [End]


// PHB Start
extern void resetPhbReady(RIL_SOCKET_ID rid);
extern void requestPhbStatus(RIL_SOCKET_ID rid);
extern void requestQueryPhbInfo(void *data, size_t datalen, RIL_Token t);
extern void requestClearPhbEntry(int index, RIL_Token t);
extern void requestWritePhbEntry(void *data, size_t datalen, RIL_Token t);
extern void requestReadPhbEntry(void *data, size_t datalen, RIL_Token t);
extern void requestQueryUPBCapability(void *data, size_t datalen, RIL_Token t);
extern void requestEditUPBEntry(void *data, size_t datalen, RIL_Token t);
extern void requestDeleteUPBEntry(void *data, size_t datalen, RIL_Token t);
extern void requestReadGasList(void *data, size_t datalen, RIL_Token t);
extern void requestReadUpbGrpEntry(void *data, size_t datalen, RIL_Token t);
extern void requestWriteUpbGrpEntry(void *data, size_t datalen, RIL_Token t);
extern void requestGetPhoneBookStringsLength(void *data, size_t datalen, RIL_Token t);
extern void requestGetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t);
extern void requestSetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t);
extern void loadUPBCapability(RIL_Token t);
extern void requestReadPhoneBookEntryExt(void *data, size_t datalen, RIL_Token t);
extern void requestWritePhoneBookEntryExt(void *data, size_t datalen, RIL_Token t);
// PHB End
extern void requestConfigModemStatus(void *data, size_t datalen, RIL_Token t);

// BT SIM Access Profile - START
extern int rilBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
extern bool isBtSapConnectionSetup(RIL_SOCKET_ID rid);
extern void notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status message,
        RIL_SOCKET_ID socket_id);
extern void resetBtSapContext(RIL_SOCKET_ID rid);

extern void requestSwitchCardType(void *data, size_t datalen, RIL_Token t);

typedef enum
{
   BT_SAP_INIT,
   BT_SAP_CONNECTION_SETUP,
   BT_SAP_ONGOING_CONNECTION,
   BT_SAP_DISCONNECT,
   BT_SAP_POWER_ON,
   BT_SAP_POWER_OFF,
} BtSapStatus;

typedef struct _LocalBtSapMsgHeader {
    RIL_SOCKET_ID socket_id;
    RIL_Token t;
    void *param;
} LocalBtSapMsgHeader;

extern BtSapStatus queryBtSapStatus(RIL_SOCKET_ID rid);

// BT SIM Access Profile - END

#endif /* RIL_SIM_H */
