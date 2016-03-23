/* //device/libs/telephony/ril_commands.h
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
#ifndef MTK_IRAT_SUPPORT
#define DATA_AT_CHANNEL RIL_CHANNEL_1
#else
#define DATA_AT_CHANNEL RIL_CHANNEL_2
#endif

    {0, NULL, NULL, RIL_CHANNEL_1},                   //none
    {RIL_REQUEST_GET_SIM_STATUS, dispatchVoid, responseSimStatus, RIL_CHANNEL_1},
    {RIL_REQUEST_ENTER_SIM_PIN, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_ENTER_SIM_PUK, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_ENTER_SIM_PIN2, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_ENTER_SIM_PUK2, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_CHANGE_SIM_PIN, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_CHANGE_SIM_PIN2, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_CURRENT_CALLS, dispatchVoid, responseCallList, RIL_CHANNEL_1},
    {RIL_REQUEST_DIAL, dispatchDial, responseVoid, RIL_CHANNEL_1}, // 10
    {RIL_REQUEST_GET_IMSI, dispatchStrings, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_HANGUP, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CONFERENCE, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_UDUB, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_LAST_CALL_FAIL_CAUSE, dispatchVoid, responseFailCause, RIL_CHANNEL_1},
    {RIL_REQUEST_SIGNAL_STRENGTH, dispatchVoid, responseRilSignalStrength, RIL_CHANNEL_1},
    {RIL_REQUEST_VOICE_REGISTRATION_STATE, dispatchVoid, responseStrings, RIL_CHANNEL_1}, // 20
    {RIL_REQUEST_DATA_REGISTRATION_STATE, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_OPERATOR, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_RADIO_POWER, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_DTMF, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SEND_SMS, dispatchStrings, responseSMS, RIL_CHANNEL_3},
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, dispatchStrings, responseSMS, RIL_CHANNEL_3},
    {RIL_REQUEST_SETUP_DATA_CALL, dispatchDataCall, responseSetupDataCall, DATA_AT_CHANNEL},
    {RIL_REQUEST_SIM_IO, dispatchSIM_IO, responseSIM_IO, RIL_CHANNEL_1},
    {RIL_REQUEST_SEND_USSD, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CANCEL_USSD, dispatchVoid, responseVoid, RIL_CHANNEL_1}, //30
    {RIL_REQUEST_GET_CLIR, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_CLIR, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, dispatchCallForward, responseCallForwards, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_CALL_FORWARD, dispatchCallForward, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_CALL_WAITING, dispatchInts, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_CALL_WAITING, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SMS_ACKNOWLEDGE, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_GET_IMEI, dispatchVoid, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_IMEISV, dispatchVoid, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_ANSWER,dispatchVoid, responseVoid, RIL_CHANNEL_1}, // 40
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid, DATA_AT_CHANNEL},
    {RIL_REQUEST_QUERY_FACILITY_LOCK, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_FACILITY_LOCK, dispatchStrings, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, dispatchStrings, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_DTMF_START, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_DTMF_STOP, dispatchVoid, responseVoid, RIL_CHANNEL_1}, //50
    {RIL_REQUEST_BASEBAND_VERSION, dispatchVoid, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_SEPARATE_CONNECTION, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_MUTE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_MUTE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_CLIP, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts, DATA_AT_CHANNEL},
    {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList, DATA_AT_CHANNEL},
    {RIL_REQUEST_RESET_RADIO, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_OEM_HOOK_RAW, dispatchRaw, responseRaw, RIL_CHANNEL_1},
    {RIL_REQUEST_OEM_HOOK_STRINGS, dispatchStrings, responseStrings, RIL_CHANNEL_1}, // 60
    {RIL_REQUEST_SCREEN_STATE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_WRITE_SMS_TO_SIM, dispatchSmsWrite, responseInts, RIL_CHANNEL_3},
    {RIL_REQUEST_DELETE_SMS_ON_SIM, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_SET_BAND_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_STK_GET_PROFILE, dispatchVoid, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_STK_SET_PROFILE, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, dispatchString, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, dispatchString, responseVoid, RIL_CHANNEL_1}, //70
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_EXPLICIT_CALL_TRANSFER, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, dispatchVoid, responseCellList, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_LOCATION_UPDATES, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_TTY_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1}, //80
    {RIL_REQUEST_QUERY_TTY_MODE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_FLASH, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_BURST_DTMF, dispatchStrings, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, dispatchString, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_SEND_SMS, dispatchCdmaSms, responseSMS, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, dispatchCdmaSmsAck, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseGsmBrSmsCnf, RIL_CHANNEL_3},
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, dispatchGsmBrSmsCnf, responseVoid, RIL_CHANNEL_3}, //90
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseCdmaBrSmsCnf, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, dispatchCdmaBrSmsCnf, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_SUBSCRIPTION, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, dispatchRilCdmaSmsWriteArgs, responseStrings, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_DEVICE_IDENTITY, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_SMSC_ADDRESS, dispatchVoid, responseString, RIL_CHANNEL_3}, //100
    {RIL_REQUEST_SET_SMSC_ADDRESS, dispatchString, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, dispatchCdmaSubscriptionSource, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_ISIM_AUTHENTICATION, dispatchString, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, dispatchStrings, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, dispatchString, responseSIM_IO, RIL_CHANNEL_1},
    {RIL_REQUEST_VOICE_RADIO_TECH, dispatchVoiceRadioTech, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_CELL_INFO_LIST, dispatchVoid, responseCellInfoList, RIL_CHANNEL_1},  //109
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, dispatchInts, responseVoid, RIL_CHANNEL_1}, // 110
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, dispatchSetInitialAttachApn, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_IMS_REGISTRATION_STATE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_IMS_SEND_SMS, dispatchImsSms, responseSMS, RIL_CHANNEL_3}, //113
    {RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, dispatchSIM_APDU, responseSIM_IO, RIL_CHANNEL_1}, //114
    {RIL_REQUEST_SIM_OPEN_CHANNEL, dispatchString, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_SIM_CLOSE_CHANNEL, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, dispatchSIM_APDU, responseSIM_IO, RIL_CHANNEL_1},
    {RIL_REQUEST_NV_READ_ITEM, dispatchNVReadItem, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_NV_WRITE_ITEM, dispatchNVWriteItem, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_NV_WRITE_CDMA_PRL, dispatchRaw, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_NV_RESET_CONFIG, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_UICC_SUBSCRIPTION, dispatchUiccSubscripton, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_ALLOW_DATA, dispatchInts, responseVoid, DATA_AT_CHANNEL},
    {RIL_REQUEST_GET_HARDWARE_CONFIG, dispatchVoid, responseHardwareConfig, RIL_CHANNEL_1},
    {RIL_REQUEST_SIM_AUTHENTICATION, dispatchString, responseString,RIL_CHANNEL_1},
    {RIL_REQUEST_GET_DC_RT_INFO, dispatchVoid, responseDcRtInfo,RIL_CHANNEL_1},
    {RIL_REQUEST_SET_DC_RT_INFO_RATE, dispatchInts, responseVoid,RIL_CHANNEL_1},
    {RIL_REQUEST_SET_DATA_PROFILE, dispatchDataProfile, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SHUTDOWN, dispatchVoid, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_RADIO_CAPABILITY, dispatchVoid, responseRadioCapability, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_RADIO_CAPABILITY, dispatchRadioCapability, responseRadioCapability, RIL_CHANNEL_1},
    {RIL_REQUEST_START_LCE, dispatchInts, responseLceStatus},
    {RIL_REQUEST_STOP_LCE, dispatchVoid, responseLceStatus},
    {RIL_REQUEST_PULL_LCEDATA, dispatchVoid, responseLceData},
    {RIL_REQUEST_GET_ACTIVITY_INFO, dispatchVoid, responseActivityData},
    {NULL,NULL,NULL, RIL_CHANNEL_1},    //136~145 Todo: Replace ingenic commands with null now,
    {NULL,NULL,NULL, RIL_CHANNEL_1},    //for other module call by rpc_ril with just request id.
    {NULL,NULL,NULL, RIL_CHANNEL_1},    //If remove, the request id may not correct
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    /********************* VIA add begin ******************/
#ifdef ADD_MTK_REQUEST_URC
    {RIL_REQUEST_GET_NITZ_TIME,dispatchVoid,responseGetNitzTime, RIL_CHANNEL_1}, // :146: 4000
    {RIL_REQUEST_SET_VOICERECORD, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_READ_SMS_STATUS_REPORT, dispatchInts, responseVoid, RIL_CHANNEL_3},
    {RIL_REQUEST_GET_LOCAL_INFO,dispatchVoid,responseInts, RIL_CHANNEL_1},// 149: 4006
    {RIL_REQUEST_SET_ETS_DEV,dispatchInts,responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GPS_START,dispatchVoid,responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_READ_PB_FROM_RUIM, dispatchInts, responseReadPbFromRuim, RIL_CHANNEL_1},
    {RIL_REQUEST_WRITE_PB_TO_RUIM, dispatchWritePbToRuim, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_PB_SIZE_FROM_RUIM, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_AGPS_TCP_CONNIND,dispatchInts,responseVoid, RIL_CHANNEL_1},//155: 4010
    {RIL_REQUEST_AGPS_SET_MPC_IPPORT,dispatchStrings,responseVoid, RIL_CHANNEL_1},//156: 4011
    {RIL_REQUEST_AGPS_GET_MPC_IPPORT,dispatchVoid,responseStrings, RIL_CHANNEL_1},//157: 4012
    {RIL_REQUEST_PHONE_TEST_INFO,dispatchVoid,responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_UTK_REFRESH,dispatchInts,responseVoid, RIL_CHANNEL_1}, // 159: 4007
    {RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS, dispatchVoid, responseInts, RIL_CHANNEL_3},//160: 4008
    {RIL_REQUEST_CDMA_SEND_REAL_PDU,dispatchStrings,responseSMS, RIL_CHANNEL_3},
    {RIL_REQUEST_CDMA_CONVERT_REAL_PDU,dispatchStrings,responseString, RIL_CHANNEL_3},//162
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_AUDIO_PATH,dispatchInts,responseVoid, RIL_CHANNEL_1}, //168~188 For rpc_ril call
    {RIL_REQUEST_SET_GPS,dispatchInts,responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_ETS_WAKE,dispatchInts,responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_DATAMODE_HYBRID,dispatchInts,responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT, NULL, NULL, RIL_CHANNEL_1},
    {RIL_REQUEST_READ_IMSI, dispatchVoid, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_UIMAUTH, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_MAKE_CAVE, dispatchString, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_MAKE_MD5, dispatchStrings, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_PREFMODE, dispatchVoid, responseInts, RIL_CHANNEL_1},
    {RIL_REQUEST_GET_UIMID, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_GENERATE_KEY, dispatchVoid, responseStrings, RIL_CHANNEL_1},
    {RIL_REQUEST_UPDATE_SSD, dispatchString, responseString, RIL_CHANNEL_1},
    {RIL_REQUEST_UPDCON_SSD, dispatchString, responseVoid, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},    //182
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_VOICE_VOLUME, dispatchInts, responseVoid, RIL_CHANNEL_1},    //184~188 For audio driver rpc_ril call
    {RIL_REQUEST_PLAY_DTMF_TONE, dispatchPlayDTMFTone, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_PLAY_TONE_SEQ, dispatchPlayToneSeq, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_VOICE_RECORD, dispatchInts, responseVoid, RIL_CHANNEL_1},
    {RIL_REQUEST_SET_MUTE_FOR_RPC, dispatchInts, responseVoid, RIL_CHANNEL_1}, //188
    /* Power on/off MD notified by thermal service, 2015/07/08 {*/
    {RIL_REQUEST_SET_MODEM_THERMAL, dispatchInts, responseVoid, RIL_CHANNEL_1}, //189:2509
    /* Power on/off MD notified by thermal service, 2015/07/08 }*/
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},
    {NULL,NULL,NULL, RIL_CHANNEL_1},//200
    {RIL_REQUEST_HANGUP_ALL, dispatchVoid, responseVoid, RIL_CHANNEL_1},//201 : 2084
    {RIL_REQUEST_EMERGENCY_DIAL, dispatchDial, responseVoid, RIL_CHANNEL_1},//202 : 2087
    {RIL_REQUEST_QUERY_ICCID, dispatchVoid, responseString, RIL_CHANNEL_1},//203 : 2026
    {RIL_REQUEST_GET_SMS_SIM_MEM_STATUS, dispatchVoid, responseGetSmsStatus, RIL_CHANNEL_3},//204: 2029
    {RIL_REQUEST_QUERY_PHB_STORAGE_INFO, dispatchInts, responseInts, RIL_CHANNEL_1},//205 : 2012
    {RIL_REQUEST_WRITE_PHB_ENTRY, dispatchMTKWritePbToRuim, responseVoid, RIL_CHANNEL_1},//206 : 2013
    {RIL_REQUEST_READ_PHB_ENTRY, dispatchInts, responseMTKReadPbFromRuim, RIL_CHANNEL_1},//207: 2014
    {RIL_REQUEST_QUERY_UIM_INSERTED, dispatchVoid, responseInts, RIL_CHANNEL_1},//208: 4001
    {RIL_REQUEST_RADIO_POWER_CARD_SWITCH, dispatchInts, responseVoid, RIL_CHANNEL_1},//209: 2100
    {RIL_REQUEST_RADIO_POWEROFF, dispatchVoid, responseVoid, RIL_CHANNEL_1},//210: 2011 //TODO: Already phase out in GSM RIL
    {RIL_REQUEST_RADIO_POWERON, dispatchVoid, responseVoid, RIL_CHANNEL_1},//211: 2033 //TODO: Already phase out in GSM RIL
    {RIL_REQUEST_SWITCH_HPF, dispatchInts, responseVoid, RIL_CHANNEL_1},//212: 4002
    {RIL_REQUEST_SET_AVOID_SYS, dispatchInts, responseVoid, RIL_CHANNEL_1},//213: 4003
    {RIL_REQUEST_QUERY_AVOID_SYS, dispatchVoid, responseInts, RIL_CHANNEL_1},//214: 4004
    {RIL_REQUEST_QUERY_CDMA_NETWORK_INFO, dispatchVoid, responseStrings, RIL_CHANNEL_1},//215: 4005
    {RIL_REQUEST_QUERY_NETWORK_REGISTRATION, dispatchVoid, responseInts, RIL_CHANNEL_1},//216: 4009
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    {RIL_REQUEST_RESUME_REGISTRATION_CDMA, dispatchVoid, responseVoid, RIL_CHANNEL_1},//217: 4014
    {RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA, dispatchInts, responseVoid, RIL_CHANNEL_1},//218: 4015
    /// M: [C2K][IR] Support SVLTE IR feature. @}
    {RIL_REQUEST_WRITE_MDN, dispatchString, responseVoid, RIL_CHANNEL_1},//219: 4017
    {RIL_REQUEST_SET_TRM, dispatchInts, responseVoid, RIL_CHANNEL_1},//220:2043
    {RIL_REQUEST_SET_ARSI_THRESHOLD, dispatchInts, responseVoid, RIL_CHANNEL_1},//221:4019
    {RIL_REQUEST_SET_UIM_SMS_READ, dispatchInts, responseVoid, RIL_CHANNEL_3},//222:4020
    {RIL_REQUEST_NOTIFY_SIM_HOTPLUG, dispatchInts, responseVoid, RIL_CHANNEL_1},//223:4021
    {RIL_REQUEST_SIM_TRANSMIT_BASIC, dispatchSIM_IO, responseSIM_IO, RIL_CHANNEL_1},//224:2053
    {NULL, NULL, NULL, RIL_CHANNEL_1},//225:2064
    {NULL, NULL, NULL, RIL_CHANNEL_1},//226:2065
    {RIL_REQUEST_SIM_TRANSMIT_CHANNEL, dispatchSIM_IO, responseSIM_IO, RIL_CHANNEL_1},//227:2054
    {RIL_REQUEST_SIM_GET_ATR, dispatchVoid, responseString, RIL_CHANNEL_1},//228:2055
    {RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW, dispatchString, responseSIM_IO, RIL_CHANNEL_1},//229:2063
    {RIL_REQUEST_SET_MEID, dispatchString, responseVoid, RIL_CHANNEL_1},    //230: 4013
    {RIL_REQUEST_MODEM_POWEROFF, dispatchVoid, responseVoid, RIL_CHANNEL_1},    //231:2010
    {RIL_REQUEST_MODEM_POWERON, dispatchVoid, responseVoid, RIL_CHANNEL_1},    //232:2028
    {RIL_REQUEST_CONFIG_MODEM_STATUS, dispatchInts, responseVoid, RIL_CHANNEL_1},    //233: 2117
    {RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE, dispatchInts, responseVoid, DATA_AT_CHANNEL},    //234: 2119
    {RIL_REQUEST_SET_LTE_EARFCN_ENABLED, dispatchInts, responseVoid, RIL_CHANNEL_1},    //235: 4020
    /* Adapt MTK Multi-Application, 2015/01/10 {*/
    {RIL_REQUEST_SIM_IO_EX, dispatchSIM_IO_EX, responseSIM_IO, RIL_CHANNEL_1}, //236: 2093
    /* Adapt MTK Multi-Application, 2015/01/10 }*/
    {RIL_REQUEST_CONFIG_IRAT_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1}, //237: 4021
    {RIL_REQUEST_CONFIG_EVDO_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1}, //238: 4022
    {RIL_REQUEST_SET_SPEECH_CODEC_INFO, dispatchInts, responseVoid, RIL_CHANNEL_1}, //239: 2110
    {RIL_REQUEST_QUERY_UTK_MENU_FROM_MD, dispatchVoid, responseString, RIL_CHANNEL_1},    //240: 4023
    {RIL_REQUEST_QUERY_STK_MENU_FROM_MD, dispatchVoid, responseString, RIL_CHANNEL_1},    //241: 4024
    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
    {RIL_REQUEST_SET_SVLTE_RAT_MODE, dispatchInts, responseVoid, RIL_CHANNEL_1},//242:2120
    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}
    {RIL_REQUEST_SWITCH_CARD_TYPE, dispatchInts, responseVoid, RIL_CHANNEL_1},  // 243: 2130
    /// M: [C2K][SIM] Support for enable/disable MD3 sleep feature @}
    {RIL_REQUEST_ENABLE_MD3_SLEEP, dispatchInts, responseVoid, RIL_CHANNEL_1},  //244: 2132
#endif
