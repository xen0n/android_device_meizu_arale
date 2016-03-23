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

{RIL_REQUEST_GET_COLP, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_COLP, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_COLR, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_CCM, dispatchVoid, responseString, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_ACM, dispatchVoid, responseString, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_ACMMAX, dispatchVoid, responseString, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_PPU_AND_CURRENCY, dispatchVoid, responseStrings, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_ACMMAX, dispatchStrings, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_RESET_ACM, dispatchString, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_PPU_AND_CURRENCY, dispatchStrings, responseVoid, RIL_CMD_PROXY_2}, //10
{RIL_REQUEST_MODEM_POWEROFF, dispatchVoid, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_DUAL_SIM_MODE_SWITCH, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_QUERY_PHB_STORAGE_INFO, dispatchInts, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_WRITE_PHB_ENTRY, dispatchPhbEntry, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_READ_PHB_ENTRY, dispatchInts, responsePhbEntries, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_GPRS_CONNECT_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_SET_GPRS_TRANSFER_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_MOBILEREVISION_AND_IMEI, dispatchInts, responseString, RIL_CMD_PROXY_3},
{RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, dispatchInts, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_SIM_NETWORK_LOCK, dispatchStrings, responseVoid, RIL_CMD_PROXY_1}, //20
{RIL_REQUEST_SET_SCRI, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_BTSIM_CONNECT, dispatchVoid, responseString, RIL_CMD_PROXY_1},
{RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF, dispatchString, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM, dispatchStrings, responseString, RIL_CMD_PROXY_1},
{RIL_REQUEST_BTSIM_TRANSFERAPDU, dispatchStrings, responseString, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT, dispatchStrings, responseVoid,RIL_CMD_PROXY_3},
{RIL_REQUEST_QUERY_ICCID,dispatchVoid, responseString,RIL_CMD_PROXY_1},
{RIL_REQUEST_USIM_AUTHENTICATION, dispatchStrings, responseString, RIL_CMD_PROXY_1},
{RIL_REQUEST_MODEM_POWERON,dispatchVoid, responseVoid,RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, dispatchVoid, responseGetSmsSimMemStatusCnf, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_PHONE_CAPABILITY, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_PHONE_CAPABILITY, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_GET_POL_CAPABILITY, dispatchVoid, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_POL_LIST, dispatchVoid, responseStrings, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_POL_ENTRY, dispatchStrings, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_QUERY_UPB_CAPABILITY, dispatchVoid, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_EDIT_UPB_ENTRY, dispatchStrings, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_DELETE_UPB_ENTRY, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_READ_UPB_GAS_LIST, dispatchInts, responseStrings, RIL_CMD_PROXY_1},
{RIL_REQUEST_READ_UPB_GRP, dispatchInts, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_WRITE_UPB_GRP, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_SIM_RECOVERY_ON, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_SIM_RECOVERY_ON, dispatchVoid, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_TRM, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_DETECT_SIM_MISSING, dispatchVoid, responseInts, RIL_CMD_PROXY_1},  // ALPS00294581
{RIL_REQUEST_GET_CALIBRATION_DATA, dispatchVoid, responseString,RIL_CMD_PROXY_3},//add by mtk80950 for calibration data
{RIL_REQUEST_GET_PHB_STRING_LENGTH, dispatchVoid, responseInts, RIL_CMD_PROXY_3},
{RIL_REQUEST_GET_PHB_MEM_STORAGE, dispatchVoid, responseGetPhbMemStorage, RIL_CMD_PROXY_3},
{RIL_REQUEST_SET_PHB_MEM_STORAGE, dispatchStrings, responseVoid, RIL_CMD_PROXY_3},
{RIL_REQUEST_READ_PHB_ENTRY_EXT, dispatchInts, responseReadPhbEntryExt, RIL_CMD_PROXY_3},
{RIL_REQUEST_WRITE_PHB_ENTRY_EXT, dispatchWritePhbEntryExt, responseVoid, RIL_CMD_PROXY_3},
{RIL_REQUEST_GET_SMS_PARAMS, dispatchVoid, responseSmsParams, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_SMS_PARAMS, dispatchSmsParams, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SIM_TRANSMIT_BASIC, dispatchSIM_IO, responseSIM_IO, RIL_CMD_PROXY_2}, // NFC SEEK start
{RIL_REQUEST_SIM_TRANSMIT_CHANNEL, dispatchSIM_IO, responseSIM_IO, RIL_CMD_PROXY_2},
{RIL_REQUEST_SIM_GET_ATR, dispatchVoid, responseString, RIL_CMD_PROXY_2}, // NFC SEEK end
{RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO, dispatchStrings, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO, dispatchString, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_CB_CONFIG_INFO, dispatchVoid, responseCbConfigInfo, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON, dispatchVoid, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_ETWS, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_FD_MODE, dispatchFD_Mode, responseInts, RIL_CMD_PROXY_3}, //[New R8 modem FD]
{RIL_REQUEST_DETACH_PS, dispatchVoid, responseVoid, RIL_CMD_PROXY_1}, // TODO: need to move to OEM channel as RIL OEM channel change feature ready
{RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW, dispatchString, responseSIM_IO, RIL_CMD_PROXY_2},
/// M: [C2K][IR] Support SVLTE IR feature. @{
{RIL_REQUEST_SET_REG_SUSPEND_ENABLED, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_RESUME_REGISTRATION, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// M: [C2K][IR] Support SVLTE IR feature. @}
{RIL_REQUEST_STORE_MODEM_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_QUERY_MODEM_TYPE, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
{RIL_REQUEST_SIM_INTERFACE_SWITCH, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_UICC_SELECT_APPLICATION, dispatchString, responseInts, RIL_CMD_PROXY_1}, //WiFi Calling start
{RIL_REQUEST_UICC_DEACTIVATE_APPLICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_UICC_APPLICATION_IO, dispatchUiccIo, responseSIM_IO, RIL_CMD_PROXY_1},
{RIL_REQUEST_UICC_AKA_AUTHENTICATE, dispatchUiccAuthentication, responseStrings, RIL_CMD_PROXY_1},
{RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP, dispatchUiccAuthentication, responseStrings, RIL_CMD_PROXY_1},
{RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF, dispatchUiccAuthentication, responseString, RIL_CMD_PROXY_1}, //WiFi Calling end
{RIL_REQUEST_STK_EVDL_CALL_BY_AP, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_FEMTOCELL_LIST, dispatchStrings, responseStrings, RIL_CMD_PROXY_3}, // Femtocell (CSG)
{RIL_REQUEST_ABORT_FEMTOCELL_LIST, dispatchVoid, responseVoid, RIL_CMD_PROXY_1}, // Femtocell (CSG) : abort command shall be sent in differenent channel
{RIL_REQUEST_SELECT_FEMTOCELL, dispatchStrings, responseVoid, RIL_CMD_PROXY_3}, // Femtocell (CSG)
{RIL_REQUEST_SEND_OPLMN, dispatchString, responseString, RIL_CMD_PROXY_1}, //For oplmn update
{RIL_REQUEST_GET_OPLMN_VERSION, dispatchVoid, responseString, RIL_CMD_PROXY_1}, //For get oplmn version
{RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseVoid, RIL_CMD_PROXY_1},//for PLMN List abort
{RIL_REQUEST_DIAL_UP_CSD, dispatchStrings, responseSetupDataCall, RIL_CMD_PROXY_2},//CSD
{RIL_REQUEST_SET_TELEPHONY_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_HANGUP_ALL, dispatchVoid, responseVoid, RIL_CMD_PROXY_4},      //0 in alps/hardware/ril/include/telephony/ril.h
{RIL_REQUEST_FORCE_RELEASE_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_CALL_INDICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_EMERGENCY_DIAL, dispatchDial, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_ECC_SERVICE_CATEGORY, dispatchInts, responseVoid, RIL_CMD_PROXY_2}, //ECC servie category
{RIL_REQUEST_SET_ECC_LIST, dispatchStrings, responseVoid, RIL_CMD_PROXY_2}, //ECC List
{RIL_REQUEST_GENERAL_SIM_AUTH, dispatchSimAuth, responseSIM_IO, RIL_CMD_PROXY_2},
{RIL_REQUEST_OPEN_ICC_APPLICATION, dispatchInts, responseInts, RIL_CMD_PROXY_1},
{RIL_REQUEST_GET_ICC_APPLICATION_STATUS, dispatchInts, responseSimStatus, RIL_CMD_PROXY_1},
{RIL_REQUEST_SIM_IO_EX, dispatchSIM_IO_EX, responseSIM_IO, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_IMS_ENABLE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT , dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
{RIL_REQUEST_SEND_CNAP, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_CLIP, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SETUP_DEDICATE_DATA_CALL, dispatchSetupDedicateDataCall, responseSetupDedicateDataCall, RIL_CMD_PROXY_5},
{RIL_REQUEST_DEACTIVATE_DEDICATE_DATA_CALL, dispatchStrings, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_MODIFY_DATA_CALL, dispatchModifyDedicateDataCall, responseSetupDedicateDataCall, RIL_CMD_PROXY_5},
{RIL_REQUEST_ABORT_SETUP_DATA_CALL, dispatchStrings, responseVoid, RIL_CMD_PROXY_1}, // TODO: need to move to OEM channel as RIL OEM channel change feature ready
{RIL_REQUEST_PCSCF_DISCOVERY_PCO, dispatchInts, responseString, RIL_CMD_PROXY_5},
{RIL_REQUEST_CLEAR_DATA_BEARER, dispatchVoid, responseVoid, RIL_CMD_PROXY_5}, 
{RIL_REQUEST_REMOVE_CB_MESSAGE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SET_DATA_CENTRIC, dispatchInts, responseVoid, RIL_CMD_PROXY_2}, 
{RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseInts, RIL_CMD_PROXY_2}, //IMS conference call - add new member
{RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, dispatchStrings, responseInts, RIL_CMD_PROXY_2}, //IMS conference call - remove member
{RIL_REQUEST_DIAL_WITH_SIP_URI, dispatchString, responseVoid, RIL_CMD_PROXY_2}, //[VoLTE]Make call with SIP uri
{RIL_REQUEST_RESUME_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_SPEECH_CODEC_INFO, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_SET_DATA_ON_TO_MD, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_SET_IMS_CALL_STATUS, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// M: for 3G VT only @{
{RIL_REQUEST_VT_DIAL, dispatchDial, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_VOICE_ACCEPT, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_REPLACE_VT_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// @}
{RIL_REQUEST_CONFIG_MODEM_STATUS, dispatchInts, responseVoid, RIL_CMD_PROXY_2}, 
/// M: [C2K] MD IRAT feature. @{
{RIL_REQUEST_SET_ACTIVE_PS_SLOT, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
/// @}
/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
{RIL_REQUEST_SET_SVLTE_RAT_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}
/// M: [C2K] AP IRAT feature. @{
{RIL_REQUEST_TRIGGER_LTE_BG_SEARCH, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
/// @}
{RIL_REQUEST_CONFERENCE_DIAL, dispatchStrings, responseVoid, RIL_CMD_PROXY_2},
/// M: CC072: Add Customer proprietary-IMS RIL interface. @{
{RIL_REQUEST_SET_SRVCC_CALL_CONTEXT_TRANSFER, dispatchStrings, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_UPDATE_IMS_REGISTRATION_STATUS, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_RELOAD_MODEM_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
/// @}
/// M: IMS feature. @{
{RIL_REQUEST_HOLD_CALL, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// @}
{RIL_REQUEST_SET_STK_UTK_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
{RIL_REQUEST_SWITCH_ANTENNA, dispatchInts, responseVoid, RIL_CMD_PROXY_2},

// M: [C2K] Deactivate link down PDN for CT requirement. @{
{RIL_REQUEST_DEACTIVATE_LINK_DOWN_PDN, dispatchVoid, responseVoid, RIL_CMD_PROXY_5},
// @}
{RIL_REQUEST_AT_COMMAND_WITH_PROXY, dispatchRaw, responseRaw, RIL_CMD_PROXY_6},
{RIL_REQUEST_SWITCH_CARD_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
{RIL_REQUEST_ENABLE_MD3_SLEEP, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// M: ViLTE. @{
{RIL_REQUEST_VIDEO_CALL_ACCEPT, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
/// @}
// M: [LTE][Low Power][UL traffic shaping] Start
{RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
{RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
// M: [LTE][Low Power][UL traffic shaping] End
/// M: [EPDG] Hand over ind @{
{RIL_REQUEST_HANDOVER_IND, dispatchInts, responseVoid, RIL_CMD_PROXY_5},
/// M: [EPDG] Hand over ind @}
// M: [EPDG] WiFi disconnect ind @{
{RIL_REQUEST_WIFI_DISCONNECT_IND, dispatchStrings, responseVoid, RIL_CMD_PROXY_5},
/// M: [EPDG] WiFi disconnect ind @}
/// M: EPDG feature. Update PS state from MAL @{
{RIL_REQUEST_MAL_PS_RGEGISTRATION_STATE, dispatchString, responseVoid, RIL_CMD_PROXY_3},
/// @}
