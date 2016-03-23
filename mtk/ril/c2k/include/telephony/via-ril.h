
/**
 * some definitions only used in VIA-Telecom's RIL library.
 *
 * Notice: All the structure, enum or some other definitions which defined by VIA-Telecom
 * can be removed to this header file.
 */


#ifndef ANDROID_VIA_RIL_H
#define ANDROID_VIA_RIL_H 1

#include <telephony/ril.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ANDROID_SIM_COUNT_2)
#define SIM_COUNT 2
#elif defined(ANDROID_SIM_COUNT_3)
#define SIM_COUNT 3
#elif defined(ANDROID_SIM_COUNT_4)
#define SIM_COUNT 4
#else
#define SIM_COUNT 1
#endif

#define REGISTRATION_EVDO_REV_A "8"
#define REGISTRATION_CDMA_1XTT "6"
#define REGISTRATION_NETWORK_UNKNOWN "0"
#define DEFAULT_ROAMING_INDICATOR "1"
#define REGISTRATION_DEFAULT_VALUE "0"
#define REGISTRATION_DEFAULT_PRL_VALUE "1"

#define SIGNAL_STRENGTH_DEFAULT 0
#define EVDO_ECIO_DEFAULT 750
#define EVDO_SNR_DEFAULT 8
#define CDMA_ECIO_DEFAULT 90

#define RIL_CDMA_SMS_TELESERVICE_ID	   0
#define RIL_CDMA_SMS_SERVICE_CAT	     1
#define RIL_CDMA_SMS_ORIG_ADDR	       2
#define RIL_CDMA_SMS_ORIG_SUBADDR	     3
#define RIL_CDMA_SMS_DEST_ADDR	       4
#define RIL_CDMA_SMS_DEST_SUBADDR	     5
#define RIL_CDMA_SMS_BEARER_RPLY_OPT	 6
#define RIL_CDMA_SMS_CAUSE_CODE	       7
#define RIL_CDMA_SMS_BEARER_DATA       8


/**
 * RIL_REQUEST_GET_NITZ_TIME
 *
 * Get NITZ time frome current network
 *
 * "data" is NULL
 * "response" is const char * containing version string for log reporting
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */

#define RIL_REQUEST_GET_NITZ_TIME 146

/**
 * RIL_REQUEST_SET_VOICERECORD
 *
 * Turn on or off voice call recording.
 **
 * "data" is an int *
 * (int *)data)[0] is 1 for "enable recording" and 0 for "disable recording"
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

#define RIL_REQUEST_SET_VOICERECORD 147

/*
 * RIL_REQUEST_READ_SMS_STATUS_REPORT
 *
 * "data" is an int *
 *
 * 
 *
 *
 */
#define RIL_REQUEST_READ_SMS_STATUS_REPORT 148

#define RIL_REQUEST_GET_LOCAL_INFO   149

 /**
 * RIL_REQUEST_SET_ETS_DEV
 *
 * Set ETS log out put device.
 **
 * "data" is an int *
 * (int *)data)[0] is 1 for "USB" and 0 for "serial"
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

 #define RIL_REQUEST_SET_ETS_DEV   150

/**
 * RIL_REQUEST_GPS_START
 *
 * GPS hardware reset.
 **
 * "data" is an int *
 * (int *)data)[0] is 1 for "enable recording" and 0 for "disable recording"
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

#define RIL_REQUEST_GPS_START 151

/*
 * RIL_REQUEST_READ_PB_FROM_RUIM
 *
 * Read phonebook from SIM card, this is used for
 * phonebook manager
 *
 * "data" is an (int *)
 *
 * "response" is a structure 
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_READ_PB_FROM_RUIM   152

/*
 * RIL_REQUEST_WRITE_PB_TO_RUIM
 *
 * Write phonebook to SIM card, this is used for
 * phonebook manager
 *
 * "data" is const char **
 * ((const char **)data)[0] is the number string
 * ((const char **)data)[1] is the text string
 * ((int *)data)[2] is the encoding type of text string, 0 for UNICODE, 1 for 7-bit ASCII
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_WRITE_PB_TO_RUIM   153

/*
 * RIL_REQUEST_GET_PB_SIZE_FROM_RUIM
 *
 * Get phonebook record counts from SIM card, this is used for
 * phonebook manager
 *
 * "data" is NULL
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_GET_PB_SIZE_FROM_RUIM   154

/**
 * RIL_REQUEST_AGPS_TCP_CONNIND
 *
 * AGPS TCP connected.
 **
 * "data" is an int *
 * (int *)data)[0] is 0-1
 *    0 - disconnected
 *    1 - connected
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

#define RIL_REQUEST_AGPS_TCP_CONNIND 155


/**
 * RIL_REQUEST_AGPS_SET_MPC_IPPORT
 *
 * AGPS TCP connected.
 **
 * "data" is const char **
 * ((const char **)data)[0] is the ip string: xxx.xxx.xxx.xxx
 * ((const char **)data)[1] is the port string
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

#define RIL_REQUEST_AGPS_SET_MPC_IPPORT 156


/**
 * RIL_REQUEST_AGPS_GET_MPC_IPPORT
 *
 * AGPS TCP connected.
 **
 * "data" is NULL
 *
 * "response" is const char **
 * ((const char **)response)[0] is the ip string: xxx.xxx.xxx.xxx
 * ((const char **)response)[1] is the port string
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */

#define RIL_REQUEST_AGPS_GET_MPC_IPPORT 157

/**
 * RIL_REQUEST_PHONE_TEST_INFO
 *
 * Request current test information
 *
 * "data" is NULL
 * "response" is a "char **
 * ((const char **)response)[0] is BT (board test) status 0-1,
 *                                    0 - negative
 *                                    1 - done
 * ((const char **)response)[1] is FT (final test) status 0-1.
 *                                    0 - negative
 *                                    1 - done
 * ((const char **)response)[2] is MEID status 0-1.
 *                                    0 - negative
 *                                    1 - done
 */

#define RIL_REQUEST_PHONE_TEST_INFO 158

/**
 * RIL_REQUEST_UTK_REFRESH
 *
 * Request UTK refresh
 *
 * "data" is an int *
 * (int *)data)[0] is 0-2
 *      0 - refresh SMS
 *      1 - refresh Phonebook
 *      2 - reset system
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */


#define RIL_REQUEST_UTK_REFRESH 159


/**
 * RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS
 *
 * Request the status of sms and phonebook in modem
 *
 * "data" is NULL
 *
 * "response" is int *
 * ((int *)response)[0] is == 0 for sms is not ready
 * ((int *)response)[0] is == 1 for sms is ready
 * ((int *)response)[1] is == 0 for phonebook is not ready
 * ((int *)response)[1] is == 1 for phonebook is ready
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_QUERY_SMS_AND_PHONEBOOK_STATUS    160

/**
 * RIL_REQUEST_CDMA_SEND_REAL_PDU
 *
 * Send a CDMA SMS message with the real pdu byte array
 *
 * "data" is const pdu byte array *
 *
 * "response" is a const RIL_SMS_Response *
 *
 * Based on the return error, caller decides to resend if sending sms
 * fails. The CDMA error class is derived as follows,
 * SUCCESS is error class 0 (no error)
 * SMS_SEND_FAIL_RETRY is error class 2 (temporary failure)
 * and GENERIC_FAILURE is error class 3 (permanent and no retry)
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  SMS_SEND_FAIL_RETRY
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_CDMA_SEND_REAL_PDU 161

/**
 * RIL_REQUEST_CDMA_CONVERT_REAL_PDU
 *
 * Convert one real pdu to an android pdu
 *
 * Only valid when radio state is "RADIO_STATE_SIM_READY"
 *
 * "data" is NULL
 * "response" is a const char * containing the pdu after converted
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_CDMA_CONVERT_REAL_PDU 162

/*
 * RIL_REQUEST_SET_AUDIO_PATH
 *
 * Set CP's audio path
 *
 *
 * "data" is an int *
 * (int *)data)[0] is 0-3
 *    0 - Earpiece
 *    1 - Headset
 *    2 - Speaker
 *    3 - Bluetooth
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/


#define RIL_REQUEST_SET_AUDIO_PATH	168

/*
 * RIL_REQUEST_SET_GPS
 *
 * Enable/Disable GPS
 *
 *
 * "data" is an int *
 * (int *)data)[0] is 0-1
 *    0 - Disable
 *    1 - Enable
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_SET_GPS   169

/*
 * RIL_REQUEST_SET_ETS_WAKE
 *
 * Enable/Disable ETS wake AP
 *
 *
 * "data" is an int *
 * (int *)data)[0] is 0-1
 *    0 - Disable
 *    1 - Enable
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_SET_ETS_WAKE   170

/*
 * RIL_REQUEST_SET_ETS_WAKE
 *
 * Set datacall mode to hybrid or 1x, this is used for
 *user mode
 *
 * "data" is an int *
 * (int *)data)[0] is 0-1
 *    0 - 1x mode
 *    1 - Hybrid mode
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
*/
#define RIL_REQUEST_SET_DATAMODE_HYBRID   171


#define RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT    172

/*for C+W*/
#define RIL_REQUEST_READ_IMSI 173
#define RIL_REQUEST_GET_UIMAUTH 174
#define RIL_REQUEST_MAKE_CAVE    175
#define RIL_REQUEST_MAKE_MD5      176
#define RIL_REQUEST_GET_PREFMODE  177
#define RIL_REQUEST_GET_UIMID 178
#define RIL_REQUEST_GENERATE_KEY   179
#define RIL_REQUEST_UPDATE_SSD    180
#define RIL_REQUEST_UPDCON_SSD    181

// add utk from 2.3
#define RIL_REQUEST_UTK_SET_PROFILE 182
#define RIL_REQUEST_REPORT_UTK_SERVICE_IS_RUNNING 183

#define ADD_MTK_REQUEST_URC

#ifdef ADD_MTK_REQUEST_URC
#define RIL_REQUEST_SET_VOICE_VOLUME    184
#define RIL_REQUEST_PLAY_DTMF_TONE  185
#define RIL_REQUEST_PLAY_TONE_SEQ 186
#define RIL_REQUEST_SET_VOICE_RECORD 187
#define RIL_REQUEST_SET_MUTE_FOR_RPC 188
/* Power on/off MD notified by thermal service, 2015/07/08 {*/
#define RIL_REQUEST_SET_MODEM_THERMAL 189
/* Power on/off MD notified by thermal service, 2015/07/08 }*/
#endif

#ifdef ADD_MTK_REQUEST_URC
//add mtk request
#define RIL_REQUEST_HANGUP_ALL 201
#define RIL_REQUEST_EMERGENCY_DIAL 202
#define RIL_REQUEST_QUERY_ICCID 203
#define RIL_REQUEST_GET_SMS_SIM_MEM_STATUS 204
#define RIL_REQUEST_QUERY_PHB_STORAGE_INFO 205
#define RIL_REQUEST_WRITE_PHB_ENTRY  206
#define RIL_REQUEST_READ_PHB_ENTRY 207
#define RIL_REQUEST_QUERY_UIM_INSERTED 208
#define RIL_REQUEST_RADIO_POWER_CARD_SWITCH 209
#define RIL_REQUEST_RADIO_POWEROFF 210
#define RIL_REQUEST_RADIO_POWERON 211
#define RIL_REQUEST_SWITCH_HPF  212
#define RIL_REQUEST_SET_AVOID_SYS   213
#define RIL_REQUEST_QUERY_AVOID_SYS   214
#define RIL_REQUEST_QUERY_CDMA_NETWORK_INFO 215
#define RIL_REQUEST_QUERY_NETWORK_REGISTRATION 216
/// M: [C2K][IR] Support SVLTE IR feature. @{
#define RIL_REQUEST_RESUME_REGISTRATION_CDMA 217
#define RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA 218
/// M: [C2K][IR] Support SVLTE IR feature. @}
#define RIL_REQUEST_WRITE_MDN 219
#define RIL_REQUEST_SET_TRM 220        //trigger reset modem
#define RIL_REQUEST_SET_ARSI_THRESHOLD 221

/**
 * RIL_REQUEST_SET_UIM_SMS_READ
 *
 * Set SMS status from unread to read in UIM card
 *
 * "data" is int *
 * data is the index of sms which to be set
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_SET_UIM_SMS_READ 222

/**
 * RIL_REQUEST_NOTIFY_SIM_HOTPLUG
 *
 * handle card hot plug
 *
 * "data" is int
 *      1: plug in
 *      0: plug out
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_NOTIFY_SIM_HOTPLUG    223

/**
 * RIL_REQUEST_UIM_TRANSMIT_BASIC
 *
 * handle card hot plug
 *
 * "data" is RIL_SIM_IO_V6
 *
 * "response" is RIL_SIM_IO_Response
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_SIM_TRANSMIT_BASIC    224

/**
 * RIL_REQUEST_UIM_TRANSMIT_CHANNEL
 *
 * handle card hot plug
 *
 * "data" is RIL_SIM_IO_V6
 *
 * "response" is RIL_SIM_IO_Response
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_SIM_TRANSMIT_CHANNEL    227

/**
 * RIL_REQUEST_SIM_GET_ATR
 *
 * handle get ATR
 *
 * "data" is NULL
 *
 *
 *
 * "response" is string
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_SIM_GET_ATR 228

/**
 * RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW
 *
 * handle open NFC channel with sw
 *
 * "data" is <sessionid>, <length>,<command>
 *
 *       AT+CGLA=1,10,"00B000000A"
 *       +CGLA:24,90000000003303006380e703
 *
 * "response" is
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW 229


#endif

/**
 * RIL_REQUEST_SET_MEID
 *
 * Add end for China Telecom auto-register sms
 * */
#define RIL_REQUEST_SET_MEID 230

/**
 * RIL_REQUEST_MODEM_POWEROFF
 *
 * Power off modem
 * "data" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_MODEM_POWEROFF 231

/**
 * RIL_REQUEST_MODEM_POWERON
 *
 * Power on modem
 * "data" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_MODEM_POWERON 232

/**
 * RIL_REQUEST_CONFIG_MODEM_STATUS
 *
 * C2K SVLTE remote SIM access
 */
#define RIL_REQUEST_CONFIG_MODEM_STATUS 233

/**
 * RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE
 *
 * C2K IRAT confirm request
 */
#define RIL_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE 234

/**
 * RIL_REQUEST_SET_LTE_EARFCN_ENABLED
 *
 * For AP IRAT, to set lte earfcn enable.
 */
#define RIL_REQUEST_SET_LTE_EARFCN_ENABLED 235

/* Adapt MTK Multi-Application, 2015/01/10 {*/
/**
 * RIL_REQUEST_SIM_IO_EX
 *
 * For Adapting multi-app SIM interface.
 */
#define RIL_REQUEST_SIM_IO_EX 236
/* Adapt MTK Multi-Application, 2015/01/10 }*/

/**
 * RIL_REQUEST_CONFIG_IRAT_MODE
 *
 * For SVLTE to config IRAT mode
 */
#define RIL_REQUEST_CONFIG_IRAT_MODE 237

/**
 * RIL_REQUEST_CONFIG_EVDO_MODE
 *
 * For SVLTE to config CDMA eHPRD mode
 */
#define RIL_REQUEST_CONFIG_EVDO_MODE 238

/**
 * RIL_REQUEST_SET_SPEECH_CODEC_INFO
 *
 * For set HD voice icon feature
 */
#define RIL_REQUEST_SET_SPEECH_CODEC_INFO 239

/**
 * RIL_REQUEST_QUERY_UTK_MENU_FROM_MD
 *
 * Requests to query Utk menu
 *
 * "data" is null
 *
 * "response" is a String containing SAT/USAT response
 *
 * Valid errors:
 *  RIL_E_SUCCESS
 *  RIL_E_RADIO_NOT_AVAILABLE (radio resetting)
 *  RIL_E_GENERIC_FAILURE
 */
#define RIL_REQUEST_QUERY_UTK_MENU_FROM_MD 240

/**
 * RIL_REQUEST_QUERY_STK_MENU_FROM_MD
 *
 * Requests to query Stk menu
 *
 * "data" is null
 *
 * "response" is a String containing SAT/USAT response
 *
 * Valid errors:
 *  RIL_E_SUCCESS
 *  RIL_E_RADIO_NOT_AVAILABLE (radio resetting)
 *  RIL_E_GENERIC_FAILURE
 */
#define RIL_REQUEST_QUERY_STK_MENU_FROM_MD 241

/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
#define RIL_REQUEST_SET_SVLTE_RAT_MODE 242
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature.@}

/// M: [C2K] Switch ruim to sim or sim to ruim @{
#define RIL_REQUEST_SWITCH_CARD_TYPE 243
/// @}


/// M: [C2K] Enable or disable MD3 Sleep @{
#define RIL_REQUEST_ENABLE_MD3_SLEEP 244
/// @}


/**
 * RIL_UNSOL_RADIO_CAPABILITY
 *
 * Sent when RIL_REQUEST_SET_RADIO_CAPABILITY completes.
 * Returns the phone radio capability exactly as
 * RIL_REQUEST_GET_RADIO_CAPABILITY and should be the
 * same set as sent by RIL_REQUEST_SET_RADIO_CAPABILITY.
 *
 * "data" is the RIL_RadioCapability structure
 *
 * data[0] is the Version of structure, RIL_RADIO_CAPABILITY_VERSION
 * data[1] is the Unique session value defined by framework returned in all "responses/unsol"
 * data[2] is the RadioCapabilityPhase that the value list are CONFIGURED(0)/START(1)/APPLY(2)/FINISH(3)
 * data[3] is the RIL_RadioAccessFamily for the radio
 * data[4] is the Logical modem this radio is be connected to
 * data[5] is the return status and an input parameter for RC_PHASE_FINISH
 *
 */
#define RIL_UNSOL_RADIO_CAPABILITY 1042

/*
 * RIL_UNSOL_FOR_CTCLIENT
 *
 *  "data" is char *
 *  data is a unsolicited message string for RILJ
 *
 */
#define RIL_UNSOL_FOR_CTCLIENT   1050

/**
 * RIL_UNSOL_CDMA_CALL_ACCEPTED
 *
 * Indicates that the voice call has been accepted.
 *
 * "data" is null
 *
 */
#define RIL_UNSOL_CDMA_CALL_ACCEPTED   1051

/*
 * RIL_UNSOL_VIA_GPS_EVENT
 *
 *  "data" is const int *
 *  data is a indicator for VIA GPS event
 *
 */
#define RIL_UNSOL_VIA_GPS_EVENT   1052

/*
 * RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED
 *
 *  "data" is const int *
 *  data is a indicator for data network type
 *  only for via-telecom cdma baseband
 *
 */
#define RIL_UNSOL_RESPONSE_DATA_NETWORK_TYPE_CHANGED   1054

/*
 * RIL_UNSOL_RESPONSE_READ_NEW_SMS_STATUS_REPORT
 *
 *  "data" is const int *
 *  data is a index to read status in ME
 *
 */
#define RIL_UNSOL_RESPONSE_READ_NEW_SMS_STATUS_REPORT 1057

// add utk , format according to the STK 
#define RIL_UNSOL_UTK_SESSION_END 1058
#define RIL_UNSOL_UTK_PROACTIVE_COMMAND 1059
#define RIL_UNSOL_UTK_EVENT_NOTIFY 1060

#define RIL_UNSOL_VIA_ADDED_END     1061

#ifdef ADD_MTK_REQUEST_URC
#define RIL_UNSOL_SPEECH_INFO 1081
#define RIL_UNSOL_SIM_INSERTED_STATUS 1082
#define RIL_UNSOL_SMS_READY_NOTIFICATION 1083
#define RIL_UNSOL_PHB_READY_NOTIFICATION 1084
#define RIL_UNSOL_VIA_NETWORK_TYPE_CHANGE 1085
/// M: [C2K][IR] Support SVLTE IR feature. @{
#define RIL_UNSOL_CDMA_PLMN_CHANGE_REG_SUSPENDED 1086
/// M: [C2K][IR] Support SVLTE IR feature. @}
#define RIL_UNSOL_VIA_INVALID_SIM_DETECTED 1087

/*
 * RIL_UNSOL_CDMA_SIM_PLUG_IN
 *
 *  "data" is const int*
 *
 */
#define RIL_UNSOL_CDMA_SIM_PLUG_IN 1088

/*
 * RIL_UNSOL_CDMA_SIM_PLUG_OUT
 *
 *   "data" is const int*
 *
 */
#define RIL_UNSOL_CDMA_SIM_PLUG_OUT 1089

/*
 * RIL_UNSOL_GSM_SIM_PLUG_IN
 *
 *   "data" is const int*
 *
 */
#define RIL_UNSOL_GSM_SIM_PLUG_IN 1090

/*
*RIL_UNSOL_CDMA_CARD_TYPE
*
*"data" is const int
*
*/
#define RIL_UNSOL_CDMA_CARD_TYPE 1091

/*
*RIL_UNSOL_DATA_ALLOWED
*
* data allowed.
*
*/
#define RIL_UNSOL_DATA_ALLOWED 1092

/*
 * RIL_UNSOL_ENG_MODE_NETWORK_INFO
 *
 * To receive engineer mode information.
 *
 */
#define RIL_UNSOL_ENG_MODE_NETWORK_INFO 1093

/*
*RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE
*
*IRAT state changed.
*
*/
#define RIL_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE 1094

/*
*RIL_UNSOL_LTE_EARFCN_INFO
*
*For AP IRAT, to receive lte earfcn info.
*
*/
#define RIL_UNSOL_LTE_EARFCN_INFO 1095

/*
*RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED
*
*PS network type change.
*
*/
#define RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED 1096

/**
 * RIL_UNSOL_CDMA_IMSI_READY
 *
 * Sent when modem CIEV:107,1 is coming. AP will query cdma imsi when
 * to receive RIL_UNSOL_CDMA_IMSI_READY. Or modem will return error if to query 
 * before it.
 *
 */
#define RIL_UNSOL_CDMA_IMSI_READY 1097

#define RIL_UNSOL_SPEECH_CODEC_INFO 1098

#define RIL_UNSOL_CDMA_SIGNAL_FADE 1099

#define RIL_UNSOL_CDMA_TONE_SIGNALS 1100

#define RIL_UNSOL_MD_STATE_CHANGE 1101

#define RIL_UNSOL_NETWORK_INFO 1102

#define RIL_UNSOL_NETWORK_EXIST 1103

#endif

typedef struct {
  char* memType;  /* type of memory which store new sms */
  int index;      /* index of new sms in momory */
  int length;     /* length of new sms pdu in byte */
  char* pdu;      /* pdu in ascii char format */
} RIL_CDMA_New_Sms;

typedef struct{
	char *imei;
	char *imeisv;
	char *esn;
	char *meid;
	char *uimid;
}RIL_Identity;

typedef struct{
	char *mdn;
	char *homesid;
	char *homenid;
	char *min;
	char *prl_id;
}RIL_Subscription;

typedef struct{
	char *mcc;
	char *mnc;
}RIL_Mccmnc;

typedef struct{
	char *longons;
	char *shortons;
	char *mccmnc;
}RIL_Operationinfo;

typedef struct{
	char * register_state;
	char *lac;
	char * cid;
	char * radio_technology;
	char * basestation_id;
	char * basestation_latitude;
	char * basestation_longitude;
	char * concurrent_service;
	char * system_id;
	char * network_id;
	char * roaming_indicator;
	char * prl_state;
	char * prl_roaming_indicator;
	char * deny_reason;
    //char * service_state;    // only for via-telecom modem
}RIL_Rgistration_state;

typedef struct{
        char * bt_status;
        char * ft_status;
        char * meid_status;
        char * current_frequency_channel;
        char * frame_error_rate;
        char * tx_power;
        char * vfield_tst_info_active;
        char * vfield_tst_info_neighbor;
        char * vfield_tst_info_alternate;       
        char * current_band_class;
}RIL_Phone_Test_Info;//phone test info

/* Read phonebook from Ruim card */

typedef struct {
    int index;
    char *number;
    int type;
    char *name;
    int coding;
} RIL_Ruim_Phonebook_Record;

#ifdef ADD_MTK_REQUEST_URC
/* Read phonebook from Ruim card for MTK feature */
typedef struct {
    int type;
    int index;
    char *number;
    int ton;
    char *alphaid;
} RIL_MTK_Ruim_Phonebook_Record;

typedef struct {
    int mode;
    int dtmfIndex;
    int volume;
    int duration;
} RIL_Play_DTMF_Tone;

#define MAX_TONE_DATA_NUM 4
typedef struct {
    int duration;   /*# of 20 msec frames*/
    int freq[2];    /*Dual Tone Frequencies in HZ; for single tone, set 2nd freq to zero*/
} RIL_Tone_Data;

typedef struct {
    int num;
    int interation;
    int volume;
    RIL_Tone_Data toneData[MAX_TONE_DATA_NUM];
} RIL_Play_Tone_Seq;

typedef struct{
    char * system_id;
    char * network_id;
    char * prl_version;
}RIL_Netwrok_Info;

#endif

typedef enum {
    GPS_MODE_MSS = 1,
    GPS_MODE_MSB,
    GPS_MODE_MSA
} GPS_Mode;

typedef enum {
    SESS_CLOSE_NORMAL = 0,
    TCP_OPEN_FAIL,
    PPM_PREF_QUAL_TIMER_OUT,
    SESS_OPEN_ERROR,
    BS_REJ_ERROR,
    OUT_REQ_TIMEROUT,
    ORIG_CANCEL,
    BS_DISCONN_EV,
    BS_CLOSE_SESS,
    MANUAL_BS_ALMANAC_FAIL,
    SESS_MSS_FALLBACK_EV,

    SESS_CLOSE_USER = 50,
    AP_DUN_DISCONNECTED,
    E911_CALL,
    VAL_SESS_ERR_STATE,
    AP_DUN_STATE_UNKNOWN,

    RES_AT_CMD = 60,
    LOC_AT_CMD,
    MSS_TIMER_EXP,

    START_POS_RESP_TIMER_EXP = 70,
    SESS_DONE_IND,
    CANCEL_NI_TRACKING_SESS,
    MPC_CANCELL,
    MPC_START_ERR,
    REQ_NOT_AUTH_OR_REFUSED,
    SERVICE_NOT_AVAIL,
    NET_ERR_STATE,
    SOCKET_ERR_STATE,
    NET_REGID_ERR,
    PPP_OPEN_STATE_ERR,
    PPP_ERROR_STATE,
    ERR_POS_REPORT_RESP,
    TCP_CLOSE_EVENT,
    CANCEL_BY_USER,

    BP_RETRY_60_TIMER = 100,
} VIA_GPS_Status;

typedef enum {
    REQUEST_DATA_CONNECTION = 0,
    CLOSE_DATA_CONNECTION,
    GPS_START,
    GPS_FIX_RESULT,
    GPS_STOP,
} VIA_GPS_Event;

typedef struct {
    VIA_GPS_Event event;
    int gps_status;
} VIA_GPS_EVENT_Data;

typedef struct {
    unsigned int mode;
    unsigned int fix_interval;
    unsigned int fix_max_times;
} VIA_GPS_Data;

typedef struct {
    int is_in_factory_mode;
    int is_cold_start;
    int is_auto_restart;
    int is_trackin_mode;
    unsigned int mode;
    unsigned int fix_interval;
    unsigned int fix_max_times;
} VIA_GPS_FACTORY_MODE_Data;

/* data network type, only for via-telecom baseband */
typedef enum {
    CDMA_DATA_NETWORK_UNKNOWN = 0,
    CDMA_DATA_NETWORK_1XRTT,
    CDMA_DATA_NETWORK_EVDO,
} CDMA_DATA_NETWORK_Type;

typedef struct
{
  RIL_CardState card_state;
  RIL_PinState  universal_pin_state;             /* applicable to USIM and CSIM: RIL_PINSTATE_xxx */
  int           pin1Count;
  int           pin2Count;
  int           puk1Count;
  int           puk2Count;
  int           gsm_umts_subscription_app_index; /* value < RIL_CARD_MAX_APPS */
  int           cdma_subscription_app_index;     /* value < RIL_CARD_MAX_APPS */
  int           num_applications;                /* value <= RIL_CARD_MAX_APPS */
  RIL_AppStatus applications[RIL_CARD_MAX_APPS];
} VIA_RIL_CardStatus;

// [C2K][IRAT] data structure define start {@
typedef struct {
	int sourceRat;	// Source RAT
	int targetRat;	// Target RAT
	int action;		// IRAT action
	int type;		// IRAT type, from source RAT to target RAT
} RIL_Pdn_IratInfo;
// @}

#ifdef __cplusplus
}
#endif

#endif  /* ANDROID_VIA_RIL_H */
