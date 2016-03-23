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

#ifndef RIL_NW_H
#define RIL_NW_H 1

extern void requestSignalStrength(void * data, size_t datalen, RIL_Token t);
extern void requestRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestGprsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestOperator(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPower(void * data, size_t datalen, RIL_Token t);
extern void requestQueryNetworkSelectionMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionAutomatic(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionManual(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t);
extern void requestSetBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetNeighboringCellIds(void * data, size_t datalen, RIL_Token t);
extern void requestSetLocationUpdates(void * data, size_t datalen, RIL_Token t);
extern void requestGetPacketSwitchBearer(RIL_SOCKET_ID rid);
extern void requestQueryAvailableNetworksWithAct(void * data, size_t datalen, RIL_Token t);
extern void requestAbortQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t);
extern void onNetworkStateChanged(char *urc, RIL_SOCKET_ID rid);
extern void onNitzTimeReceived(char *urc, RIL_SOCKET_ID rid);
extern void onRestrictedStateChanged(RIL_SOCKET_ID rid);
extern void onMMRRStatusChanged(char *urc, RIL_SOCKET_ID rid);
extern void onNitzTzReceived(char *urc, RIL_SOCKET_ID rild);
extern void updateNitzOperInfo(RIL_SOCKET_ID rid);
extern int isRadioOn(RIL_SOCKET_ID rid);
extern int queryRadioState(RIL_SOCKET_ID rid);
extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilNwUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern int getSingnalStrength(char *line, int *response);

/* MTK proprietary start */
extern void requestSetNetworkSelectionManualWithAct(void * data, size_t datalen, RIL_Token t);
extern void requestVoiceRadioTech(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPowerOff(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPowerOn(void * data, size_t datalen, RIL_Token t);
extern void requestGetCellInfoList(void * data, size_t datalen, RIL_Token t);
extern void requestSetCellInfoListRate(void * data, size_t datalen, RIL_Token t);
extern void requestGetFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestAbortFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestSelectFemtoCell(void * data, size_t datalen, RIL_Token t);
extern void requestGetPOLCapability(void *data, size_t datalen, RIL_Token t);
extern void requestGetPOLList(void *data, size_t datalen, RIL_Token t);
extern void requestSetPOLEntry(void *data, size_t datalen, RIL_Token t);
extern void requestSetRegSuspendEnabled(void * data, size_t datalen, RIL_Token t);
extern void requestResumeRegistration(void * data, size_t datalen, RIL_Token t);
extern void requestImsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void onSignalStrenth(char* urc, const RIL_SOCKET_ID rid);
extern void onNeighboringCellInfo(char* urc, const RIL_SOCKET_ID rid);
extern void onNetworkInfo(char* urc, const RIL_SOCKET_ID rid);
extern void onSimInsertChanged(const char *s,RIL_SOCKET_ID rid);
extern void onInvalidSimInfo(char *urc,RIL_SOCKET_ID rid); //ALPS00248788
extern void onACMT(char *urc,RIL_SOCKET_ID rid);
extern void onPlmnListChanged(char *urc, const RIL_SOCKET_ID rid);
extern void onRegistrationSuspended(char *urc, const RIL_SOCKET_ID rid);
extern void onFemtoCellInfo(char *urc, const RIL_SOCKET_ID rid);
extern void onCellInfoList(char *urc, const RIL_SOCKET_ID rid);
extern void onImsRegistrationInfo(char *urc, const RIL_SOCKET_ID rid);
extern void requestSendOplmn(void *data, size_t datalen, RIL_Token t);
extern void requestGetOplmnVersion(void *data, size_t datalen, RIL_Token t);
void requestRadioMode(void * data, size_t datalen, RIL_Token t);
extern int getPLMNNameFromNumeric(char *numeric, char *longname, char *shortname, int max_length);
extern int getOperatorNamesFromNumericCode(char *code,char *longname,char *shortname,int max_length,RIL_SOCKET_ID rid);
extern int convert3GRssiValue(int RSCP_in_qdbm,int EcN0_in_qdbm);
/// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @{
extern void onGmssRat(char* urc, const RIL_SOCKET_ID rid);
/// @}

/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
extern void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t);
extern void onLteRadioCapabilityChanged(char *urc, const RIL_SOCKET_ID rid);
extern void setLteRadioCapability(void *data);
/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}

/// M: [C2K][SVLTE] Set the STK UTK mode. @{
extern void requestSetStkUtkMode(void * data, size_t datalen, RIL_Token t);
/// M: [C2K][SVLTE] Set the STK UTK mode. @}

/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @{
extern int getRatSwitching();
/// M: [C2K][IR] for get switching state. 0 is not switching. 1 is switching @}

extern int  getWorldPhonePolicy();
extern void onWorldModeChanged(char *urc, const RIL_SOCKET_ID rid);
extern void handleWorldModeChanged(void *param);

// M: [LTE][Low Power][UL traffic shaping] Start
extern void onLteAccessStratumResult(char *urc, const RIL_SOCKET_ID rid);
// M: [LTE][Low Power][UL traffic shaping] End

/// M: EPDG feature. Update PS state from MAL @{
extern void requestMalGprsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void onMalPsStateChanged(void *data, size_t datalen, RIL_Token t);
extern void onPsbearerChangeForEpdg(char *urc, RIL_SOCKET_ID rid);
/// @}

/* MTK proprietary end */

/* RIL Network Structure */

typedef struct
{
    unsigned short arfcn;
    unsigned char bsic;
    unsigned char rxlev;
} gas_nbr_cell_meas_struct;

typedef struct
{
    unsigned short mcc;
    unsigned short mnc;
    unsigned short lac;
    unsigned short ci;
} global_cell_id_struct;

typedef struct
{
    unsigned char nbr_meas_num;
    gas_nbr_cell_meas_struct nbr_cells[15];
} gas_nbr_meas_struct;

typedef struct
{
    global_cell_id_struct gci;
    unsigned char nbr_meas_rslt_index;
} gas_cell_info_struct;

typedef struct
{
    gas_cell_info_struct serv_info;
    unsigned char ta;
    unsigned char ordered_tx_pwr_lev;
    unsigned char nbr_cell_num;
    gas_cell_info_struct nbr_cell_info[6];
    gas_nbr_meas_struct nbr_meas_rslt;
} gas_nbr_cell_info_struct;

typedef struct
{
    unsigned char need_revise; // Lexel: this fake, can remove if have other element
} uas_nbr_cell_info_struct; //Lexel: Not define uas_nbr_cell_info_struct yet

typedef union
{
    gas_nbr_cell_info_struct gas_nbr_cell_info;
    uas_nbr_cell_info_struct uas_nbr_cell_info;
} ps_nbr_cell_info_union_type;

/* RIL Network Enumeration */

typedef enum
{
    GSM_BAND_900    = 0x02,
    GSM_BAND_1800   = 0x08,
    GSM_BAND_1900   = 0x10,
    GSM_BAND_850    = 0x80
} GSM_BAND_ENUM;

typedef enum
{
    UMTS_BAND_I     = 0x0001,
    UMTS_BAND_II    = 0x0002,
    UMTS_BAND_III   = 0x0004,
    UMTS_BAND_IV    = 0x0008,
    UMTS_BAND_V     = 0x0010,
    UMTS_BAND_VI    = 0x0020,
    UMTS_BAND_VII   = 0x0040,
    UMTS_BAND_VIII  = 0x0080,
    UMTS_BAND_IX    = 0x0100,
    UMTS_BAND_X     = 0x0200
} UMTS_BAND_ENUM;

typedef enum
{
    BM_AUTO_MODE,
    BM_EURO_MODE,
    BM_US_MODE,
    BM_JPN_MODE,
    BM_AUS_MODE,
    BM_AUS2_MODE,
    BM_CELLULAR_MODE,
    BM_PCS_MODE,
    BM_CLASS_3,
    BM_CLASS_4,
    BM_CLASS_5,
    BM_CLASS_6,
    BM_CLASS_7,
    BM_CLASS_8,
    BM_CLASS_9,
    BM_CLASS_10,
    BM_CLASS_11,
    BM_CLASS_15,
    BM_CLASS_16,
    BM_40_BROKEN = 100,
    BM_FOR_DESENSE_RADIO_ON = 200,
    BM_FOR_DESENSE_RADIO_OFF = 201,
    BM_FOR_DESENSE_RADIO_ON_ROAMING = 202,
    BM_FOR_DESENSE_B8_OPEN = 203
} BAND_MODE;

typedef enum
{
    NT_WCDMA_PREFERRED_TYPE,   //   WCDMA preferred (auto mode)
    NT_GSM_TYPE,    //   GSM only
    NT_WCDMA_TYPE,  //   WCDMA only
    NT_AUTO_TYPE,  //    AUTO
    NT_LTE_GSM_WCDMA_TYPE = 9,
    NT_LTE_CDMA_EVDO_GSM_WCDMA = 10,
    NT_LTE_ONLY_TYPE = 11,
    NT_LTE_WCDMA_TYPE = 12
} NETWORK_TYPE;

typedef enum
{
    RAT_NONE = 0,
    RAT_GSM,
    RAT_UMTS,
    RAT_GSM_UMTS
} rat_enum;

/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
typedef enum
{
    SVLTE_RAT_MODE_4G = 0,
    SVLTE_RAT_MODE_3G = 1,
    SVLTE_RAT_MODE_4G_DATA_ONLY = 2
} svlte_rat_mode;

typedef enum RoamingMode {
    ROAMING_MODE_HOME = 0,
    ROAMING_MODE_NORMAL_ROAMING = 1,
    ROAMING_MODE_JPKR_CDMA = 2,	    // only for 4M version.
    ROAMING_MODE_UNKNOWN = 3,
} roaming_mode;

typedef enum RadioTechMode {
    RADIO_TECH_MODE_UNKNOWN  = 1,
    RADIO_TECH_MODE_CSFB  = 2,
    RADIO_TECH_MODE_SVLTE = 3
} radio_tech_mode;

/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}

/* RIL Network Constant */

#define NW_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RIL_NW_ALL_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define RIL_NW_ALL_CS_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY)

#define RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define MAX_OPER_NAME_LENGTH    50
#define PROPERTY_GSM_BASEBAND_CAPABILITY    "gsm.baseband.capability"
#define PROPERTY_GSM_BASEBAND_CAPABILITY_MD2    "gsm.baseband.capability.md2"

//ALPS00269882
#define PROPERTY_GSM_CURRENT_ENBR_RAT    "gsm.enbr.rat"
#define PROPERTY_GSM_GCF_TEST_MODE  "gsm.gcf.testmode"
#define PROPERTY_SERIAL_NUMBER "gsm.serial"
#define PROPERTY_RIL_TESTSIM "gsm.sim.ril.testsim"

#define RIL_NW_NUM              MAX_SIM_COUNT
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,\
                                 PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0, 0, 0, 0}
#define RIL_NW_INIT_STRING      {{0},{0},{0},{0}}
#define RIL_NW_INIT_STRUCT      {NULL, NULL, NULL, NULL}
#define RIL_NW_INIT_STATE   {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS,\
                             RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}

#define PROPERTY_NITZ_OPER_CODE     "persist.radio.nitz_oper_code"
#define PROPERTY_NITZ_OPER_LNAME    "persist.radio.nitz_oper_lname"
#define PROPERTY_NITZ_OPER_SNAME    "persist.radio.nitz_oper_sname"

/*RIL Network MACRO */
#define cleanCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] &= ~flags)
#define setCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] |= flags)



typedef enum
{
    NO_SIM_INSERTED     = 0x00,
    SIM1_INSERTED       = 0x01,
    SIM2_INSERTED       = 0x02,
    SIM3_INSERTED       = 0x04,
    SIM4_INSERTED       = 0x08,
    DUAL_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED),
    TRIPLE_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED),
    QUAD_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED| SIM4_INSERTED)
} sim_inserted_status_enum;

#endif /* RIL_NW_H */

