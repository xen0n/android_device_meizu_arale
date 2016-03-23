
/* //device/libs/telephony/ril_unsol_commands.h
**
** Copyright 2006, The Android Open Source Project
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
    {RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_SMS, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_ON_USSD, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_ON_USSD_REQUEST, responseVoid, DONT_WAKE},
    {RIL_UNSOL_NITZ_TIME_RECEIVED, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_SIGNAL_STRENGTH, responseRilSignalStrength, DONT_WAKE},
    {RIL_UNSOL_DATA_CALL_LIST_CHANGED, responseDataCallList, WAKE_PARTIAL},
    {RIL_UNSOL_SUPP_SVC_NOTIFICATION, responseSsn, WAKE_PARTIAL},
    {RIL_UNSOL_STK_SESSION_END, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_STK_PROACTIVE_COMMAND, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_STK_EVENT_NOTIFY, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_STK_CALL_SETUP, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_SMS_STORAGE_FULL, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SIM_REFRESH, responseSimRefresh, WAKE_PARTIAL},
    {RIL_UNSOL_CALL_RING, responseCallRing, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, responseCdmaSms, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, responseRaw, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RESTRICTED_STATE_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_CALL_WAITING, responseCdmaCallWaiting, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_INFO_REC, responseCdmaInformationRecords, WAKE_PARTIAL},
    {RIL_UNSOL_OEM_HOOK_RAW, responseRaw, WAKE_PARTIAL},
    {RIL_UNSOL_RINGBACK_TONE, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_RESEND_INCALL_MUTE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CDMA_PRL_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_RIL_CONNECTED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CELL_INFO_LIST, responseCellInfoList, WAKE_PARTIAL},
    {RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, responseVoid, WAKE_PARTIAL}, //1037
    {RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_SRVCC_STATE_NOTIFY, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_HARDWARE_CONFIG_CHANGED, responseHardwareConfig, WAKE_PARTIAL},
    {RIL_UNSOL_DC_RT_INFO_CHANGED, responseDcRtInfo, WAKE_PARTIAL},
    {RIL_UNSOL_RADIO_CAPABILITY, responseRadioCapability, WAKE_PARTIAL},
    {RIL_UNSOL_ON_SS, responseSSData, WAKE_PARTIAL},
    {RIL_UNSOL_STK_CC_ALPHA_NOTIFY, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_LCEDATA_RECV, responseLceData, WAKE_PARTIAL},
    {NULL, NULL, DONT_WAKE},    //1046, 1046~1049 just reserved for android update from M to next version
    {NULL, NULL, DONT_WAKE},
    {NULL, NULL, DONT_WAKE},
    {NULL, NULL, DONT_WAKE},    //1049
/********************* VIA add begin ******************/
#ifdef ADD_MTK_REQUEST_URC
    {RIL_UNSOL_FOR_CTCLIENT, responseInts, DONT_WAKE},
    {RIL_UNSOL_CDMA_CALL_ACCEPTED, responseVoid, WAKE_PARTIAL},// 5000
    {RIL_UNSOL_VIA_GPS_EVENT, responseInts, WAKE_PARTIAL},//5004
    {NULL, NULL, DONT_WAKE},    
    {RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED, responseInts, WAKE_PARTIAL}, //5009
    {NULL, NULL, DONT_WAKE},    
    {NULL, NULL, DONT_WAKE},    
    {RIL_UNSOL_RESPONSE_READ_NEW_SMS_STATUS_REPORT, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_UTK_SESSION_END, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_UTK_PROACTIVE_COMMAND, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_UTK_EVENT_NOTIFY, responseString, WAKE_PARTIAL},
    {NULL, NULL, DONT_WAKE},//1061
    {NULL, NULL, DONT_WAKE}, //1062
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, //1065
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, //1070
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, //1075
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, 
    {NULL, NULL, DONT_WAKE}, //1080
    {RIL_UNSOL_SPEECH_INFO, responseInts, WAKE_PARTIAL},//1081: 3006 //TODO: Not used anymore, Phase out in GSM RIL
    {RIL_UNSOL_SIM_INSERTED_STATUS, responseInts, WAKE_PARTIAL},//1082: 3003
    {RIL_UNSOL_SMS_READY_NOTIFICATION, responseVoid, WAKE_PARTIAL}, //1083: 3006
    {RIL_UNSOL_PHB_READY_NOTIFICATION, responseVoid, WAKE_PARTIAL},//1084: 3002
    {RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE, responseInts, WAKE_PARTIAL},//1085: 5005
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    {RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED, responseStrings, WAKE_PARTIAL},//1086: 5006
    /// M: [C2K][IR] Support SVLTE IR feature. @}
    {RIL_UNSOL_VIA_INVALID_SIM_DETECTED, responseVoid, WAKE_PARTIAL},//1087: 5007
    {RIL_UNSOL_CDMA_SIM_PLUG_IN, responseInts, WAKE_PARTIAL},//1088:3028 //TODO: Not used anymore
    {RIL_UNSOL_CDMA_SIM_PLUG_OUT, responseInts, WAKE_PARTIAL},//1089:3027 //TODO: Not used anymore
    {RIL_UNSOL_GSM_SIM_PLUG_IN, responseInts, WAKE_PARTIAL},//1090:5010 //TODO: Not used anymore
    {RIL_UNSOL_CDMA_CARD_TYPE, responseInts, WAKE_PARTIAL},//1091:3035 //TODO: Not used in current version, need check
    /// M: [C2K] DSDA support. @{
    {RIL_UNSOL_DATA_ALLOWED, responseVoid, WAKE_PARTIAL}, //1092:3046
    /// @}
    {RIL_UNSOL_ENG_MODE_NETWORK_INFO, responseStrings, WAKE_PARTIAL}, //1093:5008
    /// M: [C2K] IRAT feature. @{
    {RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE, responseIratStateChange, WAKE_PARTIAL}, //1094:3060
    /// @}
    {RIL_UNSOL_LTE_EARFCN_INFO, responseInts, WAKE_PARTIAL},//1095:5010
    {RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, responseInts, WAKE_PARTIAL},//1096:3015
    {RIL_UNSOL_CDMA_IMSI_READY, responseVoid, WAKE_PARTIAL},//1097:5011
    {RIL_UNSOL_SPEECH_CODEC_INFO, responseInts, WAKE_PARTIAL},//1098:3052
    {RIL_UNSOL_CDMA_SIGNAL_FADE, responseInts, WAKE_PARTIAL},//1099:5012
    {RIL_UNSOL_CDMA_TONE_SIGNALS, responseInts, WAKE_PARTIAL},//1100:5013
    {RIL_UNSOL_MD_STATE_CHANGE, responseInts, WAKE_PARTIAL},  // 1101:3053
    {RIL_UNSOL_NETWORK_INFO, responseStrings, WAKE_PARTIAL},  // 1102:3001
    {RIL_UNSOL_NETWORK_EXIST, responseInts, WAKE_PARTIAL},  // 1103:5014
    {NULL, NULL, DONT_WAKE},  // 1103
#endif
