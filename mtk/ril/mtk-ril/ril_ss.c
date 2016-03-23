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

/* //hardware/ril/reference-ril/ril_ss.c
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

#include <telephony/mtk_ril.h>
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
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <termios.h>

#include <ril_callbacks.h>

//for UCS2 de/coding
#include <ril_gsm_util.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>


typedef enum {
    CCFC_E_QUERY,
    CCFC_E_SET
} CallForwardOperationE;

/*
 * 0    unconditional
 * 1    mobile busy
 * 2    no reply
 * 3    not reachable
 * 4    all call forwarding (refer 3GPP TS 22.030 [19])
 * 5    all conditional call forwarding (refer 3GPP TS 22.030 [19])
 */
typedef enum {
    CF_U        = 0,
    CF_BUSY     = 1,
    CF_NORPLY   = 2,
    CF_NOTREACH = 3,
    CF_ALL      = 4,
    CF_ALLCOND  = 5
} CallForwardReasonE;

/*
 * status is:
 * 0 = disable
 * 1 = enable
 * 2 = interrogate
 * 3 = registeration
 * 4 = erasure
*/
typedef enum {
    SS_DEACTIVATE   = 0,    // disable
    SS_ACTIVATE     = 1,    // enable
    SS_INTERROGATE  = 2,    // interrogate
    SS_REGISTER     = 3,    // registeration
    SS_ERASE        = 4     // erasure
} SsStatusE;

typedef enum {
    HAS_NONE    = 0,
    HAS_SIA     = 1,
    HAS_SIB     = 2,
    HAS_SIC     = 4
} HasSIFlagE;

/*
 * Check if CBS data coding scheme is UCS2 3GPP 23.038
 *
 * Coding Group Use of bits 3..0
 * Bits 7..4
 *
 * 0000         Language using the GSM 7 bit default alphabet
 *
 *              Bits 3..0 indicate the language:
 *              0000 German
 *              0001 English
 *              0010 Italian
 *              0011 French
 *              0100 Spanish
 *              0101 Dutch
 *              0110 Swedish
 *              0111 Danish
 *              1000 Portuguese
 *              1001 Finnish
 *              1010 Norwegian
 *              1011 Greek
 *              1100 Turkish
 *              1101 Hungarian
 *              1110 Polish
 *              1111 Language unspecified
 *
 * 0001         0000 GSM 7 bit default alphabet; message preceded by language indication.
 *
 *                   The first 3 characters of the message are a two-character representation
 *                   of the language encoded according to ISO 639 [12], followed by a CR character.
 *                   The CR character is then followed by 90 characters of text.
 *
 *              0001 UCS2; message preceded by language indication
 *
 *                   The message starts with a two GSM 7-bit default alphabet character representation
 *                   of the language encoded according to ISO 639 [12]. This is padded to the octet
 *                   boundary with two bits set to 0 and then followed by 40 characters of UCS2-encoded message.
 *                   An MS not supporting UCS2 coding will present the two character language identifier followed
 *                   by improperly interpreted user data.
 *
 *              0010..1111 Reserved
 *
 * 0010..       0000 Czech
 *              0001 Hebrew
 *              0010 Arabic
 *              0011 Russian
 *              0100 Icelandic
 *
 *              0101..1111 Reserved for other languages using the GSM 7 bit default alphabet, with
 *                         unspecified handling at the MS
 *
 * 0011         0000..1111 Reserved for other languages using the GSM 7 bit default alphabet, with
 *                         unspecified handling at the MS
 *
 * 01xx         General Data Coding indication
 *              Bits 5..0 indicate the following:
 *
 *              Bit 5, if set to 0, indicates the text is uncompressed
 *              Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in 3GPP TS 23.042 [13]
 *
 *              Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
 *              Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      Class 0
 *              0      1      Class 1 Default meaning: ME-specific.
 *              1      0      Class 2 (U)SIM specific message.
 *              1      1      Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
 *
 *              Bits 3 and 2 indicate the character set being used, as follows:
 *              Bit 3  Bit 2  Character set:
 *              0      0      GSM 7 bit default alphabet
 *              0      1      8 bit data
 *              1      0      UCS2 (16 bit) [10]
 *              1      1      Reserved
 *
 *  1000        Reserved coding groups
 *
 *  1001        Message with User Data Header (UDH) structure:
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      Class 0
 *              0      1      Class 1 Default meaning: ME-specific.
 *              1      0      Class 2 (U)SIM specific message.
 *              1      1      Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
 *
 *              Bits 3 and 2 indicate the alphabet being used, as follows:
 *              Bit 3  Bit 2  Alphabet:
 *              0      0      GSM 7 bit default alphabet
 *              0      1      8 bit data
 *              1      0      USC2 (16 bit) [10]
 *              1      1      Reserved
 *
 *  1010..1101  Reserved coding groups
 *
 *  1101        l1 protocol message defined in 3GPP TS 24.294[19]
 *
 *  1110        Defined by the WAP Forum [15]
 *
 *  1111        Data coding / message handling
 *
 *              Bit 3 is reserved, set to 0.
 *
 *              Bit 2  Message coding:
 *              0      GSM 7 bit default alphabet
 *              1      8 bit data
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      No message class.
 *              0      1      Class 1 user defined.
 *              1      0      Class 2 user defined.
 *              1      1      Class 3
 *              default meaning: TE specific
 *              (see 3GPP TS 27.005 [8])
*/
typedef enum {
    DCS_GSM7,
    DCS_8BIT,
    DCS_UCS2,
    MAX_DCS_SUPPORT
} GsmCbsDcsE;

#define SS_OP_DEACTIVATION     "#"
#define SS_OP_ACTIVATION       "*"
#define SS_OP_INTERROGATION    "*#"
#define SS_OP_REGISTRATION     "**"
#define SS_OP_ERASURE          "##"

#define BS_ALL                   ""
#define BS_TELE_ALL              "10"
#define BS_TELEPHONY             "11"
#define BS_TELE_DATA_ALL         "12"
#define BS_TELE_FAX              "13"
#define BS_TELE_SMS              "16"
#define BS_TELE_VGCS             "17" /* Not supported by framework */
#define BS_TELE_VBS              "18" /* Not supported by framework */
#define BS_TELE_ALL_EXCEPT_SMS   "19"
#define BS_DATA_ALL              "20"
#define BS_DATA_ASYNC_ALL        "21"
#define BS_DATA_SYNC_ALL         "22"
#define BS_DATA_CIRCUIT_SYNC     "24" /* This is also for VT call */
#define BS_DATA_CIRCUIT_ASYNC    "25"
#define BS_DATA_SYNC_TELE        "26" /* Supported by framework */
#define BS_GPRS_ALL              "99"

#define CALL_FORWAED_NONE               ""
#define CALL_FORWARD_UNCONDITIONAL      "21"
#define CALL_FORWARD_BUSY               "67"
#define CALL_FORWARD_NOREPLY            "61"
#define CALL_FORWARD_NOT_REACHABLE      "62"
#define CALL_FORWARD_ALL                "002"
#define CALL_FORWARD_ALL_CONDITIONAL    "004"

#define TYPE_ADDRESS_INTERNATIONAL 145

#define SS_CHANNEL_CTX getRILChannelCtxFromToken(t)

/***
 * "AO"  BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"  BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"  BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"  BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"  BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"  All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"  All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"  All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 */
const char * callBarFacilityStrings[CB_SUPPORT_NUM] = {
    "AO",
    "OI",
    "OX",
    "AI",
    "IR",
    "AB",
    "AG",
    "AC"
};

const char * callBarServiceCodeStrings[CB_SUPPORT_NUM] = {
    "33",
    "331",
    "332",
    "35",
    "351",
    "330",
    "333",
    "353"
};

static const char * GsmCbsDcsStringp[MAX_DCS_SUPPORT] = {"GSM7","8BIT","UCS2"};

/* To check if new SS service class feature is supported or not */
extern int bNewSSServiceClassFeature;

extern void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);

extern void IMS_RIL_onUnsolicitedResponse(int unsolResponse, void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);

static const char *
ssStatusToOpCodeString(SsStatusE status)
{
    /**
     *   Activation:    *SC*SI#
     *   Deactivation:  #SC*SI#
     *   Interrogation: *#SC*SI#
     *   Registration:  *SC*SI# and **SC*SI#
     *   Erasure:       ##SC*SI#
     */
    switch (status) {
        case SS_ACTIVATE:
            return SS_OP_ACTIVATION;
            break;
        case SS_DEACTIVATE:
            return SS_OP_DEACTIVATION;
            break;
        case SS_INTERROGATE:
            return SS_OP_INTERROGATION;
            break;
        case SS_REGISTER:
            return SS_OP_REGISTRATION;
            break;
        case SS_ERASE:
            return SS_OP_ERASURE;
            break;
        default:
            assert(0);
            break;
    }

    return "";
}


/* This table shall be sync with siToServiceClass() in GsmMmiCode.java */
extern const char *
InfoClassToMmiBSCodeString (AtInfoClassE infoClass)
{
    /**
     * Basic Service
     * group number (note)  Telecommunication Service       MMI Service Code
     *
     * 1 to 12              All tele and bearer services    no code required
     *
     *                      Teleservices
     * 1 to 6, 12           All teleservices                10
     * 1                    Telephony                       11
     * 2 to 6               All data teleservices           12
     * 6                    Facsimile services              13
     * 2                    Short Message Services          16
     * 1, 3 to 6, 12        All teleservices except SMS     19
     * 12                   Voice group services
     *                      Voice Group Call Service (VGCS) 17
     *                      Voice Broadcast Service (VBS)   18
     *
     *                      Bearer Service
     * 7 to 11              All bearer services             20
     * 7                    All async services              21
     * 8                    All sync services               22
     * 8                    All data circuit sync           24
     * 7                    All data circuit async          25
     * 13                   All GPRS bearer services        99
     */

    switch(infoClass)
    {
        case CLASS_NONE:
            return BS_ALL;
            break;
        case CLASS_VOICE:
            return BS_TELEPHONY;
            break;
        case (CLASS_DATA_ASYNC | CLASS_DATA_SYNC):
            return BS_DATA_ALL;
            break;
        case CLASS_FAX:
            return BS_TELE_FAX;
            break;
        case CLASS_SMS:
            return BS_TELE_SMS;
            break;
        case (CLASS_VOICE | CLASS_SMS | CLASS_FAX):
            return BS_TELE_ALL;
            break;
        case (CLASS_SMS | CLASS_FAX):
            return BS_TELE_DATA_ALL;
            break;
        case (CLASS_VOICE | CLASS_FAX):
            return BS_TELE_ALL_EXCEPT_SMS;
            break;
        case CLASS_DATA_SYNC:
            return BS_DATA_CIRCUIT_SYNC;
            break;
        case CLASS_DATA_ASYNC:
            return BS_DATA_CIRCUIT_ASYNC;
            break;
        case (CLASS_DATA_SYNC | CLASS_DEDICATED_PACKET_ACCESS):
            return BS_DATA_SYNC_ALL;
            break;
        case (CLASS_DATA_ASYNC | CLASS_DEDICATED_PAD_ACCESS):
            return BS_DATA_ASYNC_ALL;
            break;
        case (CLASS_DATA_SYNC | CLASS_VOICE):
            return BS_DATA_SYNC_TELE;
            break;
        case CLASS_DEDICATED_PACKET_ACCESS:
            return BS_GPRS_ALL;
            break;
        case (CLASS_MTK_VIDEO | CLASS_DATA_SYNC):
            return BS_DATA_CIRCUIT_SYNC;
            break;
        case CLASS_MTK_VIDEO:
            return BS_DATA_CIRCUIT_SYNC;
            break;
        default:
            LOGE("RILD unknown infoClass: %d", infoClass);
            break;
    }
    return "";
}

extern int
MmiBSCodeToInfoClassX (int serviceCode)
{
    if (bNewSSServiceClassFeature == 1) {
    LOGD("[MmiBSCodeToInfoClassX]Return %d directly.", serviceCode);
    return serviceCode;
    } else { 	
    switch (serviceCode) {
        /* BS_ALL_E = BS_TELE_ALL_E + BS_DATA_ALL_E */
        case BS_ALL_E:
            return (int)(CLASS_SMS + CLASS_FAX + CLASS_VOICE + CLASS_DATA_ASYNC + CLASS_DATA_SYNC);
        case BS_TELE_ALL_E:
            return (int)(CLASS_SMS + CLASS_FAX + CLASS_VOICE);
        case BS_TELEPHONY_E:
            return (int) CLASS_VOICE;
        case BS_TELE_DATA_ALL_E:
            return (int) (CLASS_SMS + CLASS_FAX);
        case BS_TELE_FAX_E:
            return (int) CLASS_FAX;
        case BS_TELE_SMS_E:
            return (int) CLASS_SMS;
        case BS_TELE_ALL_EXCEPT_SMS_E:
            return (int) (CLASS_FAX + CLASS_VOICE);
        /**
         * Note for code 20:
         * From TS 22.030 Annex C:
         *   "All GPRS bearer services" are not included in "All tele and bearer services"
         *   and "All bearer services"."
         *   So SERVICE_CLASS_DATA, which (according to 27.007) includes GPRS
         */
        case BS_DATA_ALL_E:
            return (int)(CLASS_DATA_ASYNC + CLASS_DATA_SYNC);
        case BS_DATA_ASYNC_ALL_E:
            return (int)(CLASS_DEDICATED_PAD_ACCESS + CLASS_DATA_ASYNC);
        case BS_DATA_SYNC_ALL_E:
            return (int)(CLASS_DEDICATED_PACKET_ACCESS + CLASS_DATA_SYNC);
        case BS_DATA_CIRCUIT_SYNC_E:
            return (int)(CLASS_DATA_SYNC + CLASS_MTK_VIDEO); /* Also for video call */
        case BS_DATA_CIRCUIT_ASYNC_E:
            return (int) CLASS_DATA_ASYNC;
        case BS_DATA_SYNC_TELE_E:
            return (int)(CLASS_DATA_SYNC + CLASS_VOICE);
        case BS_GPRS_ALL_E:
            return (int) CLASS_DEDICATED_PACKET_ACCESS;
        default:
            return (int) CLASS_NONE;
    }
}
}


static const char *
callForwardReasonToServiceCodeString(CallForwardReasonE cfreason)
{
    switch (cfreason) {
        case CF_U:
            return CALL_FORWARD_UNCONDITIONAL;
            break;
        case CF_BUSY:
            return CALL_FORWARD_BUSY;
            break;
        case CF_NORPLY:
            return CALL_FORWARD_NOREPLY;
            break;
        case CF_NOTREACH:
            return CALL_FORWARD_NOT_REACHABLE;
            break;
        case CF_ALL:
            return CALL_FORWARD_ALL;
            break;
        case CF_ALLCOND:
            return CALL_FORWARD_ALL_CONDITIONAL;
            break;
        default:
            return CALL_FORWAED_NONE;
            break;
    }
}


static GsmCbsDcsE
checkCbsDcs(int dcs)
{
    GsmCbsDcsE result = DCS_GSM7;

    if ((dcs == 0x11) ||((dcs & 0x4C) == 0x48) ||((dcs & 0x9C) == 0x98)) {
        result = DCS_UCS2;
    } else if (((dcs & 0x4C) == 0x44) ||((dcs & 0x9C) == 0x94) ||((dcs & 0xF4) == 0xF4)) {
        result = DCS_8BIT;
    }

    return result;
}

static char*
convertToUCS2(char* data)
{
    /** USSD messages using the default alphabet are coded with the
       * GSM 7-bit default alphabet  given in clause 6.2.1. The message can then consist of
       * up to 182 user characters (3GPP 23.038).
       * Callee expects const char * in UCS2 Hex decimal format.
      */
    #define MAX_RIL_USSD_STRING_LENGTH 255

    size_t  stringlen     = 0;
    bytes_t ucs2String    = NULL;
    bytes_t ucs2HexString = NULL;

    // data equals string8, since it passed by dispatchString
    const char* string8 = (const char *)(data);

    ucs2String = (bytes_t) calloc(2*(MAX_RIL_USSD_STRING_LENGTH+1),sizeof(char));
    if (ucs2String == NULL) {
        LOGE("convertToUCS2:ucs2String malloc fail");
        return NULL;
    }
    // memory overwrite if strlen(string8) is larger than MAX_RIL_USSD_STRING_LENGTH
    stringlen = utf8_to_ucs2((cbytes_t)string8, MIN(strlen(string8),
            MAX_RIL_USSD_STRING_LENGTH), ucs2String);
    ucs2HexString = (bytes_t) calloc(2*stringlen*2+1,sizeof(char));
    if (ucs2HexString == NULL) {
        LOGE("convertToUCS2:ucs2HexString malloc fail");
        // free here in the case of ucs2HexString is null.
        free(ucs2String);
        return NULL;
    }

    gsm_hex_from_bytes((char *)ucs2HexString, ucs2String, 2*stringlen);

    free(ucs2String);

    LOGW("convertToUCS2= %s", ucs2HexString);

    return (char *)ucs2HexString;
}

/**
 * RIL_REQUEST_SEND_USSD
 *
 * Send a USSD message
 *
 * If a USSD session already exists, the message should be sent in the
 * context of that session. Otherwise, a new session should be created.
 *
 * The network reply should be reported via RIL_UNSOL_ON_USSD
 *
 * Only one USSD session may exist at a time, and the session is assumed
 * to exist until:
 *   a) The android system invokes RIL_REQUEST_CANCEL_USSD
 *   b) The implementation sends a RIL_UNSOL_ON_USSD with a type code
 *      of "0" (USSD-Notify/no further action) or "2" (session terminated)
 *
 * "data" is a const char * containing the USSD request in UTF-8 format
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  FDN_CHECK_FAILURE
 *  GENERIC_FAILURE
 *
 * See also: RIL_REQUEST_CANCEL_USSD, RIL_UNSOL_ON_USSD
 */
static void requestSendUSSD(void * data, size_t datalen, RIL_Token t)
{
    const char* p_ussdRequest = convertToUCS2(data);
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int strLen = 0;
    char* pTmpStr = NULL;

    if (p_ussdRequest == NULL) {
        LOGE("requestSendUSSD:p_ussdRequest malloc fail");
        goto error;
    }

    /**
     * AT+ECUSD=<m>,<n>,<str>,<dcs>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     * 01xx    General Data Coding indication
     *
     * Bits 5..0 indicate the following:
     *   Bit 5, if set to 0, indicates the text is uncompressed
     *   Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in 3GPP TS 23.042 [13]
     *
     *   Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
     *   Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
     *
     *     Bit 1   Bit 0       Message Class:
     *       0       0           Class 0
     *       0       1           Class 1 Default meaning: ME-specific.
     *       1       0           Class 2 (U)SIM specific message.
     *       1       1           Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
     *
     *   Bits 3 and 2 indicate the character set being used, as follows:
     *
     *     Bit 3   Bit 2       Character set:
     *       0       0           GSM 7 bit default alphabet
     *       0       1           8 bit data
     *       1       0           UCS2 (16 bit) [10]
     *       1       1           Reserved
     */
    //BEGIN mtk08470 [20130109][ALPS00436983]
    // USSD string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
    // We convert input char to unicode hex string and store it to p_ussdRequest.
    // For example, convert input "1" to "3100"; So len of p_ussdRequest is 4 times of input
    strLen = strlen(p_ussdRequest)/4;
    if (strLen > MAX_RIL_USSD_NUMBER_LENGTH) {
        LOGW("USSD stringlen = %d, max = %d", strLen, MAX_RIL_USSD_NUMBER_LENGTH);
        strLen = MAX_RIL_USSD_NUMBER_LENGTH;
    }
    pTmpStr = calloc(1, (4*strLen+1));
    if(pTmpStr == NULL) {
        LOGE("Malloc fail");
        free((char *)p_ussdRequest);
        goto error;
    }
    memcpy(pTmpStr, p_ussdRequest, 4*strLen);
    //END mtk08470 [20130109][ALPS00436983]
    asprintf(&cmd, "AT+ECUSD=2,1,\"%s\",72", pTmpStr); /* <dcs> = 0x48 */

    err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

    free(cmd);
    free(pTmpStr);
    free((char *)p_ussdRequest);

    if (err < 0 || NULL == p_response) {
        LOGE("requestSendUSSD Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            at_send_command("AT+ECUSD=2,2", NULL, SS_CHANNEL_CTX);
            break;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

///M: For query CNAP
static void requestSendCNAPUSSD(void * data, size_t datalen, RIL_Token t)
{
    const char* p_ussdRequest = (const char *)(data);
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    //asprintf(&cmd, "AT+ECUSD=1,1,\"%s\",72", p_ussdRequest); /* <dcs> = 0x48 */
    asprintf(&cmd, "AT+ECUSD=1,1,\"*#300#\"");

    err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

    free(cmd);

    if (err < 0 || NULL == p_response) {
       LOGE("requestSendCNAPUSSD Fail");
       goto error;
    }

    switch (at_get_cme_error(p_response)) {
       case CME_SUCCESS:
          ret = RIL_E_SUCCESS;
          break;
       case CME_CALL_BARRED:
       case CME_OPR_DTR_BARRING:
          ret = RIL_E_CALL_BARRED;
          break;
       case CME_PHB_FDN_BLOCKED:
          ret = RIL_E_FDN_CHECK_FAILURE;
          break;
       default:
          at_send_command("AT+ECUSD=2,2", NULL, SS_CHANNEL_CTX);
          break;
    }

error:
    //RIL_onRequestComplete(t, ret, NULL, 0);
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}

/**
 * RIL_REQUEST_CANCEL_USSD
 *
 * Cancel the current USSD session if one exists
 *
 * "data" is null
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestCancelUssd(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    err = at_send_command("AT+ECUSD=2,2", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}


static void requestClirOperation(void * data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    if (datalen != 0) {

        /**
         * Set CLIR: +CLIR=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
         *  <n> (parameter sets the adjustment for outgoing calls)
         */
        asprintf(&cmd, "AT+CLIR=%d", n[0]);

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

        free(cmd);

    } else {

        /**
         * Get CLIR: +CLIR?
         * This action will trigger CLIR interrogation. Need to check FDN so use proprietary command
         */

        /**
         * AT+ECUSD=<m>,<n>,<str>
         * <m>: 1 for SS, 2 for USSD
         * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
         * <str>: string type parameter, the SS or USSD string
         */
        err = at_send_command_singleline("AT+ECUSD=1,1,\"*#31#\"", "+CLIR:", &p_response, SS_CHANNEL_CTX);

    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestClirOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
        default:
            goto error;
    }

    /* For Get CLIR only */
    if (p_response->p_intermediates != NULL) {

        line = p_response->p_intermediates->line;

        assert(line);

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> parameter sets the adjustment for outgoing calls
         * 0   presentation indicator is used according to the subscription of the CLIR service
         * 1   CLIR invocation
         * 2   CLIR suppression
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> parameter shows the subscriber CLIR service status in the network
         * 0   CLIR not provisioned
         * 1   CLIR provisioned in permanent mode
         * 2   unknown (e.g. no network, etc.)
         * 3   CLIR temporary mode presentation restricted
         * 4   CLIR temporary mode presentation allowed
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
           goto error;
        }

    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET CLIR responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_GET_CLIR
 *
 * Gets current CLIR status
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
 * ((int *)data)[1] is "m" parameter from TS 27.007 7.7
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestGetClir(void * data, size_t datalen, RIL_Token t)
{
    requestClirOperation(data, datalen, t);
}


/**
 * RIL_REQUEST_SET_CLIR
 *
 * "data" is int *
 * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetClir(void * data, size_t datalen, RIL_Token t)
{
    requestClirOperation(data, datalen, t);
}


static void requestCallForwardOperation(void * data, size_t datalen, RIL_Token t, CallForwardOperationE op)
{
    RIL_CallForwardInfo* p_args = (RIL_CallForwardInfo*) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    char* precmd = NULL;
    ATLine* p_cur = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_CallForwardInfo ** pp_CfInfoResponses = NULL;
    RIL_CallForwardInfo * p_CfInfoResponse = NULL;
    HasSIFlagE eSiStatus = HAS_NONE;
    char* pStrTmp = NULL;
    int resLength = 0;
    int dnlen = 0;
    int serviceClass = 0;

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     *                      SC  SIA  SIB  SIC
     * CFU                  21  DN   BS   -
     * CF Busy              67  DN   BS   -
     * CF No Reply          61  DN   BS   T
     * CF Not Reachable     62  DN   BS   -
     * All CF               002 DN   BS   T
     * All conditional CF   004 DN   BS   T
     */

    /**
     * 3GPP 24.082 and 3GPP 24.030
     * Registration         **SC*DN*BS(*T)#
     * Erasure              ##SC**BS#
     * Activation           *SC**BS#
     * Deactivation         #SC**BS#
     * Interrogation        *#SC**BS#
     */
    if ((CCFC_E_QUERY == op) && (p_args->reason >= CF_ALL)) {
        LOGE("CF_ALL & CF_ALLCOND cannot be used in QUERY");
        goto error;
    }

    if ((p_args->number != NULL) && (p_args->status == SS_ACTIVATE)) {
        LOGW("Call Forwarding: change ACTIVATE to REGISTER");
        p_args->status = SS_REGISTER;
    }

    /* Check Op Code and MMI Service Code */
    asprintf(&cmd,"AT+ECUSD=1,1,\"%s%s",
             ssStatusToOpCodeString((SsStatusE) p_args->status),
             callForwardReasonToServiceCodeString((CallForwardReasonE) p_args->reason));

    precmd = cmd;

    /* Check SIA: Dial number. Only Registration need to pack DN and others are ignored. */
    if ((p_args->number != NULL)
        && ((p_args->status == SS_REGISTER) || (p_args->status == SS_ACTIVATE))) {

        eSiStatus |= HAS_SIA;
        dnlen = strlen((const char *) p_args->number);
        //BEGIN mtk08470 [20130109][ALPS00436983]
        // number string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
        if (dnlen > MAX_RIL_USSD_NUMBER_LENGTH) {
            LOGE("cur number len = %d, max = %d", dnlen, MAX_RIL_USSD_NUMBER_LENGTH);
            free(precmd);
            goto error;
        }
        //END mtk08470 [20130109][ALPS00436983]
        if ((p_args->toa == TYPE_ADDRESS_INTERNATIONAL) && (strncmp((const char *)p_args->number, "+", 1))) {
            asprintf(&cmd, "%s*+%s", precmd, p_args->number);
            dnlen++;
        } else {
            asprintf(&cmd, "%s*%s", precmd, p_args->number);
        }
        LOGD("toa:%d, number:%s, len:%d", p_args->toa, p_args->number, dnlen);

        free(precmd);
        precmd = cmd;
    } else {
        if ((p_args->number == NULL) && (p_args->status == SS_REGISTER)) {
            LOGE("Call Forwarding Error: Address cannot be NULL in registration!");
            free(cmd);
            goto error;
        }
    }

    /* Check SIB: Basic Sevice Group */
    if (p_args->serviceClass != 0) {
        if (eSiStatus == HAS_SIA) {
            asprintf(&cmd, "%s*%s", precmd, InfoClassToMmiBSCodeString(p_args->serviceClass));
        } else {
            asprintf(&cmd, "%s**%s", precmd, InfoClassToMmiBSCodeString(p_args->serviceClass));
        }

        eSiStatus |= HAS_SIB;
        serviceClass = p_args->serviceClass;
        LOGD("Reserve serviceClass. serviceClass = %d", serviceClass);
        LOGD("BS code from serviceClass = %s", InfoClassToMmiBSCodeString(serviceClass));

        free(precmd);
        precmd = cmd;
    }

    /* Check SIC: No reply timer */
    /* shall we check CF_ALL and CF_ALLCOND ? In ril.h time is for CF_NORPLY only. */
    if (((p_args->reason == CF_NORPLY) || (p_args->reason == CF_ALL) || (p_args->reason == CF_ALLCOND))
        && (p_args->status == SS_REGISTER || p_args->status == SS_ACTIVATE) && (p_args->timeSeconds!=0)) {

        if (eSiStatus == HAS_NONE) {
            asprintf(&cmd, "%s***%d", precmd, p_args->timeSeconds);
        } else if (eSiStatus == HAS_SIA) {
            asprintf(&cmd, "%s**%d", precmd, p_args->timeSeconds);
        } else {
            asprintf(&cmd, "%s*%d", precmd, p_args->timeSeconds);
        }

        free(precmd);
        precmd = cmd;
    }

    /* Check END */
    asprintf(&cmd, "%s#\"", precmd);

    free(precmd);

    if (CCFC_E_QUERY == op) {

        /**
         * RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
         *
         * "data" is const RIL_CallForwardInfo *
         *
         * "response" is const RIL_CallForwardInfo **
         * "response" points to an array of RIL_CallForwardInfo *'s, one for
         * each distinct registered phone number.
         *
         * For example, if data is forwarded to +18005551212 and voice is forwarded
         * to +18005559999, then two separate RIL_CallForwardInfo's should be returned
         *
         * If, however, both data and voice are forwarded to +18005551212, then
         * a single RIL_CallForwardInfo can be returned with the service class
         * set to "data + voice = 3")
         *
         * Valid errors:
         *  SUCCESS
         *  RADIO_NOT_AVAILABLE
         *  GENERIC_FAILURE
         */

        err = at_send_command_multiline(cmd, "+CCFC:", &p_response, SS_CHANNEL_CTX);

    } else {

        /* add DN length */
        if (dnlen != 0) {
            precmd = cmd;
            asprintf(&cmd, "%s,,%d", precmd, dnlen);
            free(precmd);
        }

        /**
         * RIL_REQUEST_SET_CALL_FORWARD
         *
         * Configure call forward rule
         *
         * "data" is const RIL_CallForwardInfo *
         * "response" is NULL
         *
         * Valid errors:
         *  SUCCESS
         *  RADIO_NOT_AVAILABLE
         *  GENERIC_FAILURE
         */

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
    }

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestCallForwardOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    if (CCFC_E_QUERY == op) {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            resLength++;
        }

        LOGI("%d of +CCFC: received!", resLength);

        pp_CfInfoResponses = (RIL_CallForwardInfo **) alloca(resLength * sizeof(RIL_CallForwardInfo *));
        memset(pp_CfInfoResponses, 0, resLength * sizeof(RIL_CallForwardInfo *));

        resLength = 0; /* reset resLength for decoding */

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line  = NULL;
            int  bsCode = 0;

            line = p_cur->line;

            if (line == NULL) {
                LOGE("CCFC: NULL line");
                break;
            }

            if (p_CfInfoResponse == NULL) {
                p_CfInfoResponse = (RIL_CallForwardInfo *) alloca(sizeof(RIL_CallForwardInfo));
                memset(p_CfInfoResponse, 0, sizeof(RIL_CallForwardInfo));
                p_CfInfoResponse->reason = p_args->reason;
            }

            ((RIL_CallForwardInfo   **)pp_CfInfoResponses)[resLength] = p_CfInfoResponse;

            /**
             * For Query CCFC only
             * +CCFC: <status>,<class1>[,<number>,<type>
             * [,<subaddr>,<satype>[,<time>]]]
             */

            if (at_tok_start(&line) < 0) {
                LOGE("+CCFC: fail");
                continue;
            }

            if (at_tok_nextint(&line, &(p_CfInfoResponse->status)) < 0) {
                LOGE("+CCFC: status fail!");
                //continue;
            }

            if (at_tok_nextint(&line, &bsCode) < 0) {
                LOGE("+CCFC: bsCode fail!");
                //continue;
            }

            if (serviceClass != 0 && p_CfInfoResponse->status == 0 && bsCode == 0) {
                p_CfInfoResponse->serviceClass = serviceClass;
            } else {
                p_CfInfoResponse->serviceClass = MmiBSCodeToInfoClassX(bsCode);
            }

            if (at_tok_hasmore(&line)) {
                if (at_tok_nextstr(&line, &(p_CfInfoResponse->number)) < 0) {
                    LOGE("+CCFC: number fail!");
                }

                if (at_tok_nextint(&line, &(p_CfInfoResponse->toa)) < 0) {
                    LOGE("+CCFC: toa fail!");
                }
            }

            if (at_tok_hasmore(&line)) {
                /* skip subaddr */
                if(at_tok_nextstr(&line, &(pStrTmp)) < 0) {
                    LOGE("+CCFC: sub fail!");
                }

                /* skip satype */
                if(at_tok_nextint(&line,&(p_CfInfoResponse->timeSeconds)) < 0) {
                    LOGE("+CCFC: sa type fail!");
                }

                if(at_tok_nextint(&line,&(p_CfInfoResponse->timeSeconds)) < 0) {
                    LOGE("+CCFC: time fail!");
                }
            }

#ifdef MTK_VT3G324M_SUPPORT
           if ((bNewSSServiceClassFeature == 1) && 
               (p_CfInfoResponse->serviceClass == CLASS_DATA_SYNC)) {
              p_CfInfoResponse->serviceClass = CLASS_MTK_VIDEO;
           }
#endif

            LOGD("CfInfoResponse status:%d class:%d num:%s toa:%d time:%d",
                 p_CfInfoResponse->status,
                 p_CfInfoResponse->serviceClass,
                 p_CfInfoResponse->number,
                 p_CfInfoResponse->toa,
                 p_CfInfoResponse->timeSeconds);

            p_CfInfoResponse = NULL;
            resLength++;
        }

        LOGI("%d of +CCFC: decoded!", resLength);
    }

error:
    RIL_onRequestComplete(t, ret, pp_CfInfoResponses, resLength*sizeof(RIL_CallForwardInfo *));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
 *
 * "data" is const RIL_CallForwardInfo *
 *
 * "response" is const RIL_CallForwardInfo **
 * "response" points to an array of RIL_CallForwardInfo *'s, one for
 * each distinct registered phone number.
 *
 * For example, if data is forwarded to +18005551212 and voice is forwarded
 * to +18005559999, then two separate RIL_CallForwardInfo's should be returned
 *
 * If, however, both data and voice are forwarded to +18005551212, then
 * a single RIL_CallForwardInfo can be returned with the service class
 * set to "data + voice = 3")
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestQueryCallForwardStatus(void * data, size_t datalen, RIL_Token t)
{
    requestCallForwardOperation(data, datalen, t, CCFC_E_QUERY);
}


/**
 * RIL_REQUEST_SET_CALL_FORWARD
 *
 * Configure call forward rule
 *
 * "data" is const RIL_CallForwardInfo *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetCallForward(void * data, size_t datalen, RIL_Token t)
{
    requestCallForwardOperation(data, datalen, t, CCFC_E_SET);
}


static void requestCallWaitingOperation(void * data, size_t datalen, RIL_Token t)
{
    int* p_int = (int *) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};
    ATLine* p_cur = NULL;
    int resLength = 0;
    int sendBsCode = 0;
    int responseForAll = 0;

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     *       SC    SIA SIB SIC
     * WAIT  43    BS  -   -
     */

    if (datalen == sizeof(int)) {

        sendBsCode = p_int[0];
        LOGD("sendBsCode = %d", sendBsCode);

        /* 
           From call settings: sendBsCode = 512(CLASS_MTK_VIDEO)
           From MMI command: sendBsCode = 528(CLASS_MTK_VIDEO + CLASS_DATA_SYNC)
        */
        if (bNewSSServiceClassFeature == 1) {
           if ((sendBsCode == CLASS_MTK_VIDEO) || 	
               (sendBsCode == (CLASS_MTK_VIDEO + CLASS_DATA_SYNC))) {
            sendBsCode = CLASS_DATA_SYNC;
        }
        }

        asprintf(&cmd, "AT+ECUSD=1,1,\"*#43#\"");
        err = at_send_command_multiline(cmd, "+CCWA:", &p_response, SS_CHANNEL_CTX);

    } else if(datalen == 2 * sizeof(int)) {
        LOGD("p_int[0] = %d, p_int[1] = %d", p_int[0], p_int[1]);
        if (p_int[1] != 0) {
            /* with InfoClass */
            asprintf(&cmd, "AT+ECUSD=1,1,\"%s43*%s#\"",
                     ssStatusToOpCodeString(p_int[0]),
                     InfoClassToMmiBSCodeString(p_int[1]));
        } else {
            /* User did not input InfoClass */
            asprintf(&cmd, "AT+ECUSD=1,1,\"%s43#\"",
                     ssStatusToOpCodeString(p_int[0]));
        }

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
    } else {
        goto error;
    }

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestCallWaitingOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    /* For Query CCWA only */
    if ( p_response->p_intermediates != NULL ) {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            resLength++;
        }

        LOGI("%d of +CCWA: received!", resLength);

        resLength = 0; /* reset resLength for decoding */
		
        if (bNewSSServiceClassFeature == 1) {
        response[1] = 0;
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line  = NULL;
            int  bsCode = 0;

            line = p_cur->line;

            if (line == NULL) {
                LOGE("CCWA: NULL line");
                break;
            }

            if (at_tok_start(&line) < 0) {
                goto error;
            }

            /**
             * <status>
             * 0   not active
             * 1   active
             */
            if (at_tok_nextint(&line, &response[0]) < 0) {
                goto error;
            }

            /**
             * <classx> is a sum of integers each representing a class of information (default 7):
             * 1   voice (telephony)
             * 2   data (refers to all bearer services; with <mode>=2 this may refer only
             *     to some bearer service if TA does not support values 16, 32, 64 and 128)
             * 4   fax (facsimile services)
             * 8   short message service
             * 16  data circuit sync
             * 32  data circuit async
             * 64  dedicated packet access
             * 128 dedicated PAD access
             */
            if (at_tok_nextint(&line, &bsCode) < 0) {
                goto error;
            }

            if (bNewSSServiceClassFeature == 1) {
               if (sendBsCode != 0) {
            if (sendBsCode == bsCode) {
               //Set response[1] to 1 to indicated that the call waiting is enabled(Refer to CallWaitingCheckBoxPreference.java).
               response[1] = 1;

                     //Check if call waiting is queried via MMI command
                     if (p_int[0] == CLASS_MTK_VIDEO + CLASS_DATA_SYNC) {
                        response[1] = CLASS_MTK_VIDEO + CLASS_DATA_SYNC;
                     }
				  
                     LOGD("[Found]response = %d, %d", response[0], response[1]);
               break;
            }
               }
               else {	 /* For call wating query by MMI command */
                  if (response[0] == 1) {
                     response[1] |= (bsCode == CLASS_DATA_SYNC) ? CLASS_MTK_VIDEO + CLASS_DATA_SYNC : bsCode;
                  }
                  if (responseForAll == 0) {
                     responseForAll = response[0];
                  }
               }
            } else {
            response[1] |= MmiBSCodeToInfoClassX(bsCode);
            }
            LOGD("response = %d, %d", response[0], response[1]);
            resLength++;
        }

        LOGI("%d of +CCWA: decoded!", resLength);

        /*
           For solving [ALPS00113964]Call waiting of VT hasn't response when turn on call waiting item, MTK04070, 2012.01.12
           sendBsCode = 0   --> Voice Call Waiting, refer to SERVICE_CLASS_NONE  in CommandInterface.java, GsmPhone.java
           sendBsCode = 512 --> Video Call Waiting, refer to SERVICE_CLASS_VIDEO in CommandInterface.java, GsmPhone.java

           Query Call Waiting: Network returned +CCWA: 1, 11 or/and +CCWA: 1, 24
           MmiBSCodeToInfoClassX method will convert 11 to 1(CLASS_VOICE), and convert 24 to 16(CLASS_DATA_SYNC) + 512(CLASS_MTK_VIDEO)

           CallWaiting settings checked response[1] value, 0 as disabled and 1 as enabled.
        */
        if (bNewSSServiceClassFeature == 1) {
           /* For call wating query by MMI command */
           if (sendBsCode == 0) {
              response[0] = responseForAll;
           }
        } else {
        if (sendBsCode != 0) {
            LOGD("sendBsCode = %d", sendBsCode);
            int tmpValue = response[1] & sendBsCode;
            response[1] = (tmpValue != 0);
            LOGD("response[1] = %d, tmpValue = %d", response[1], tmpValue);
        }
    }
    }

    ret = RIL_E_SUCCESS;

error:
    /* For SET CCWA responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_QUERY_CALL_WAITING
 *
 * Query current call waiting state
 *
 * "data" is const int *
 * ((const int *)data)[0] is the TS 27.007 service class to query.
 * "response" is a const int *
 * ((const int *)response)[0] is 0 for "disabled" and 1 for "enabled"
 *
 * If ((const int *)response)[0] is = 1, then ((const int *)response)[1]
 * must follow, with the TS 27.007 service class bit vector of services
 * for which call waiting is enabled.
 *
 * For example, if ((const int *)response)[0]  is 1 and
 * ((const int *)response)[1] is 3, then call waiting is enabled for data
 * and voice and disabled for everything else
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestQueryCallWaiting(void * data, size_t datalen, RIL_Token t)
{
    requestCallWaitingOperation(data, datalen, t);
}


/**
 * RIL_REQUEST_SET_CALL_WAITING
 *
 * Configure current call waiting state
 *
 * "data" is const int *
 * ((const int *)data)[0] is 0 for "disabled" and 1 for "enabled"
 * ((const int *)data)[1] is the TS 27.007 service class bit vector of services to modify
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetCallWaiting(void * data, size_t datalen, RIL_Token t)
{
    requestCallWaitingOperation(data, datalen, t);
}


extern const char * callBarFacToServiceCodeStrings(const char * fac)
{
    int i;

    for (i = 0; i < CB_SUPPORT_NUM; i++) {
        if (0 == strcmp(fac, callBarFacilityStrings[i])) {
            break;
        }
    }

    if (i < CB_SUPPORT_NUM) {
        return callBarServiceCodeStrings[i];
    } else {
        /* not found! return default */
        return callBarServiceCodeStrings[CB_ABS];
    }
}


/**
 * RIL_REQUEST_CHANGE_BARRING_PASSWORD
 *
 * Change call barring facility password
 *
 * "data" is const char **
 *
 * ((const char **)data)[0] = facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
 * ((const char **)data)[1] = old password
 * ((const char **)data)[2] = new password
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestChangeBarringPassword(void * data, size_t datalen, RIL_Token t)
{
    const char** strings = (const char**)data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    /**
     * "data" is const char **
     *
     * ((const char **)data)[0] = facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
     * ((const char **)data)[1] = old password
     * ((const char **)data)[2] = new password
     * ((const char **)data)[3] = new password confirmed
     */
    if (datalen == 3 * sizeof(char*)) {
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[2]);
    } else if (datalen == 4 * sizeof(char*)) {
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[3]);
    } else {
        goto error;
    }

    err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestChangeBarringPassword Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            break;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_QUERY_CLIP
 *
 * Queries the status of the CLIP supplementary service
 *
 * (for MMI code "*#30#")
 *
 * "data" is NULL
 * "response" is an int *
 * (int *)response)[0] is 1 for "CLIP provisioned"
 *                     and 0 for "CLIP not provisioned"
 *                     and 2 for "unknown, e.g. no network etc"
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
static void requestQueryClip(void * data, size_t datalen, RIL_Token t)
{
    ATResponse* p_response = NULL;
    int err;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    /**
     * AT+ECUSD=<m>,<n>,<str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */
    err = at_send_command_singleline("AT+ECUSD=1,1,\"*#30#\"", "+CLIP:", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestQueryClip Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    if ( p_response->p_intermediates != NULL ) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> (parameter sets/shows the result code presentation status in the MT/TA):
         * 0   disable
         * 1   enable
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> (parameter shows the subscriber CLIP service status in the network):
         * 0   CLIP not provisioned
         * 1   CLIP provisioned
         * 2   unknown (e.g. no network, etc.)
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, &response[1], sizeof(int));
    at_response_free(p_response);
}

static void requestSetClip(void * data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err = 0; //Coverity, follow the err init value in at_send_command_full_nolock().
    char* cmd = NULL;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    if (datalen != 0) {
/**
         * Set CLIP: +CLIP=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.6
         *  <n> (Set command enables or disables the presentation of the CLI at the TE)
         */
        asprintf(&cmd, "AT+CLIP=%d", n[0]);

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

        free(cmd);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestSetClip Fail");
        goto error;
    }
        
    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
        default:
            goto error;
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET CLIP responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION
 *
 * Enable/disable supplementary service related notifications
 * from the network.
 *
 * Notifications are reported via RIL_UNSOL_SUPP_SVC_NOTIFICATION.
 *
 * "data" is int *
 * ((int *)data)[0] is == 1 for notifications enabled
 * ((int *)data)[0] is == 0 for notifications disabled
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 * See also: RIL_UNSOL_SUPP_SVC_NOTIFICATION.
 */
static void requestSetSuppSvcNotification(void * data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    if (datalen != 0) {

        /**
         * +CSSN=[<n>[,<m>]]
         * "data" is int *
         * ((int *)data)[0] is == 1 for notifications enabled
         * ((int *)data)[0] is == 0 for notifications disabled
         */
        asprintf(&cmd, "AT+CSSN=%d,%d", n[0], n[0]);
        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
        free(cmd);
    }
    else {
        goto error;
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestSetSuppSvcNotification Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        default:
            goto error;
    }

    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

/**
 * RIL_REQUEST_GET_COLR
 *
 * Gets current COLR status
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is "n" parameter proprietary for provision status
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestGetColr(void * data, size_t datalen, RIL_Token t)
{
    ATResponse* p_response = NULL;
    int err;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    err = at_send_command_singleline("AT+ECUSD=1,1,\"*#77#\"", "+COLR:", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestColpOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    /* For Get COLR only */
    if ( p_response->p_intermediates != NULL ) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> parameter sets the adjustment for outgoing calls
         * 0   COLR not provisioned
         * 1   COLR provisioned
         * 2   unknown
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, response, sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_UNSOL_SUPP_SVC_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_SuppSvcNotification *
 *
 */
static void onSuppSvcNotification(char *s, int isMT, RIL_SOCKET_ID rid)
{
    RIL_SuppSvcNotification svcNotify;
    char* line = s;

    memset(&svcNotify, 0, sizeof(RIL_SuppSvcNotification));

    /**
     * +CSSN=[<n>[,<m>]]
     * +CSSN?  +CSSN: <n>,<m>
     * +CSSN=? +CSSN: (list of supported <n>s),(list of supported <m>s)
     */
    /**
     * When <n>=1 and a supplementary service notification is received
     * after a mobile originated call setup, intermediate result code
     * +CSSI: <code1>[,<index>] is sent to TE before any other MO call
     * setup result codes presented in the present document or in V.25ter [14].
     * When several different <code1>s are received from the network,
     * each of them shall have its own +CSSI result code.
     * <code1> (it is manufacturer specific, which of these codes are supported):
     * 0   unconditional call forwarding is active
     * 1   some of the conditional call forwardings are active
     * 2   call has been forwarded
     * 3   call is waiting
     * 4   this is a CUG call (also <index> present)
     * 5   outgoing calls are barred
     * 6   incoming calls are barred
     * 7   CLIR suppression rejected
     * 8   call has been deflected
     * <index>: refer "Closed user group +CCUG"
     */
     /**
      * When <m>=1 and a supplementary service notification is received
      * during a mobile terminated call setup or during a call, or when
      * a forward check supplementary service notification is received,
      * unsolicited result code +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]]
      * is sent to TE. In case of MT call setup, result code is sent after every +CLIP result code
      * (refer command "Calling line identification presentation +CLIP")
      * and when several different <code2>s are received from the network,
      * each of them shall have its own +CSSU result code.
      * <code2> (it is manufacturer specific, which of these codes are supported):
      * 0   this is a forwarded call (MT call setup)
      * 1   this is a CUG call (also <index> present) (MT call setup)
      * 2   call has been put on hold (during a voice call)
      * 3   call has been retrieved (during a voice call)
      * 4   multiparty call entered (during a voice call)
      * 5   call on hold has been released (this is not a SS notification) (during a voice call)
      * 6   forward check SS message received (can be received whenever)
      * 7   call is being connected (alerting) with the remote party in alerting state in explicit call transfer operation (during a voice call)
      * 8   call has been connected with the other remote party in explicit call transfer operation (also number and subaddress parameters may be present) (during a voice call or MT call setup)
      * 9   this is a deflected call (MT call setup)
      * 10  additional incoming call forwarded
      * 11  MT is a forwarded call (CF)
      * 12  MT is a forwarded call (CFU)
      * 13  MT is a forwarded call (CFC)
      * 14  MT is a forwarded call (CFB)
      * 15  MT is a forwarded call (CFNRy)
      * 16  MT is a forwarded call (CFNRc)
      * <number>: string type phone number of format specified by <type>
      * <type>: type of address octet in integer format (refer GSM 04.08 [8] subclause 10.5.4.7)
      */

    svcNotify.notificationType = isMT;

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    if (at_tok_nextint(&line, &(svcNotify.code)) < 0) {
        goto error;
    }

    if (at_tok_hasmore(&line)) {
        /* Get <index> field */
        at_tok_nextint(&line, &(svcNotify.index));
    }

    if (isMT) {
        if(at_tok_hasmore(&line)) {
            /* Get <number> */
            at_tok_nextstr(&line, &(svcNotify.number));

            /* Get <type> */
            at_tok_nextint(&line, &(svcNotify.type));
        }
    }

    RIL_onUnsolicitedResponseSocket (
            RIL_UNSOL_SUPP_SVC_NOTIFICATION,
            &svcNotify, sizeof(RIL_SuppSvcNotification),
            rid);

    return;

error:
    LOGE("Parse RIL_UNSOL_SUPP_SVC_NOTIFICATION fail: %s/n", s);
}


/**
 * RIL_UNSOL_ON_USSD
 *
 * Called when a new USSD message is received.
 *
 * "data" is const char **
 * ((const char **)data)[0] points to a type code, which is
 *  one of these string values:
 *      "0"   USSD-Notify -- text in ((const char **)data)[1]
 *      "1"   USSD-Request -- text in ((const char **)data)[1]
 *      "2"   Session terminated by network
 *      "3"   other local client (eg, SIM Toolkit) has responded
 *      "4"   Operation not supported
 *      "5"   Network timeout
 *
 * The USSD session is assumed to persist if the type code is "1", otherwise
 * the current session (if any) is assumed to have terminated.
 *
 * ((const char **)data)[1] points to a message string if applicable, which
 * should be coded as dcs in ((const char **)data)[2]
 */
static void onUssd(char *s, RIL_SOCKET_ID rid)
{
    LOGD("onUssd: %s", s);

    char* p_data[3];
    char* p_utf8Data = NULL;
    char* p_ucs2Data = NULL;
    char* p_str = NULL;
    char* line = s;
    const char* dcs;
    int iDCS;
    bool hasStr = false;
    int length = 0;
    char *dcsString = NULL;

    // Coverity for declaim in the begining.
    int      numStrings;
    bytes_t  utf8String = NULL;
    bytes_t  hexData = NULL;
    int      len = 0, maxLen = 0, i = 0;
    size_t   responselen = 0;
    char**   p_cur = NULL;
    char*    finalData[2];

    /**
     * USSD response from the network, or network initiated operation
     * +CUSD: <m>[,<str>,<dcs>] to the TE.
     */
    /*
     * <m>:
     * 0   no further user action required (network initiated USSD Notify, or no further information needed after mobile initiated operation)
     * 1   further user action required (network initiated USSD Request, or further information needed after mobile initiated operation)
     * 2   USSD terminated by network
     * 3   other local client has responded
     * 4   operation not supported
     * 5   network time out
     */

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <m> */
    if (at_tok_nextstr(&line, &p_data[0]) < 0) {
       goto error;
    }

    length++;

    /* Check if there is <str> */
    if (at_tok_hasmore(&line)) {
        /* Get <str> */
        if (at_tok_nextstr(&line, &p_str) < 0) {
            goto error;
        }
        hasStr = true;

        length++;

        /* Get <dcs> */
        if (at_tok_nextint(&line, (int*) &iDCS) < 0) {
            LOGE("No <dcs> information");
            goto error;
        }

        length++;
        LOGD("onUssd:length = %d", length);
        LOGD("onUssd: line = %s", line);
        LOGD("onUssd: p_data[0] = %s", p_data[0]);
        LOGD("onUssd: p_str = %s", p_str);
        LOGD("onUssd: dcs = %d", iDCS);

        /* Refer to GSM 23.038, section 5 CBS Data Coding Scheme
        *The message starts with a two GSM 7-bits default alphabet character.
        *Need to ignore these two bytes.
        */
        GsmCbsDcsE dcsType = checkCbsDcs(iDCS);
        if (iDCS == 0x11) {
           LOGD("Ignore the first two bytes for DCS_UCS2");
           p_str+=4;
        }

        p_data[1] = p_str;
        LOGD("onUssd: p_data[1] (p_str) = %s", p_data[1]);
        if (bNewSSServiceClassFeature == 1) {
            if (dcsType == DCS_8BIT) {     // GsmCbsDcsE: DCS_8BIT = 1
                p_data[2] = (char *) GsmCbsDcsStringp[dcsType];
                LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
            } else {
                /* DCS is set as "UCS2" by AT+CSCS in ril_callbacks.c */
                dcsString = strdup("UCS2");
                p_data[2] = dcsString;
                LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
            }
        } else {
            p_data[2] = (char *) GsmCbsDcsStringp[dcsType];
            LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
        }
    }

    // Decode response data by using ril_gsm_util.
    responselen = length * sizeof(char *);
    p_cur = (char **) p_data;
    // Only need m and str, dcs is only for str reference and no need to pass it.

    numStrings = responselen / sizeof(char *);

    if (numStrings > 1) {

        /* the last string is for dcs reference */
        numStrings = 2;
        hexData = (bytes_t) calloc(strlen(p_cur[1]), sizeof(char));
        if(hexData == NULL) {
            LOGE("onUssd:hexData malloc fail");
            goto error;
        }

        len = gsm_hex_to_bytes((cbytes_t) p_cur[1], strlen(p_cur[1]), hexData);
        LOGD("onUsdd add value to hexData = %s", hexData);
        LOGD("onUsdd len = %d", len);

        dcs = p_cur[2];
        LOGD("onUsdd dcs = p_cur[2] = %s", dcs);

        maxLen = (!strcmp(dcs,"UCS2")) ? len / 2 : len;
        if ((maxLen < 0) || (maxLen > MAX_RIL_USSD_STRING_LENGTH)) {
            free(hexData);
            hexData = NULL;
            LOGE("onUssd:max length is invalid, len = %d", maxLen);
            goto error;
            //return RIL_ERRNO_INVALID_RESPONSE;
        } else {
            LOGE("Ussd string length:%d/n", maxLen);
        }
    }

    /* each string*/
    for (i = 0 ; i < numStrings ; i++) {
        if (i == 1) {
            utf8String = (bytes_t) calloc(2*len+1, sizeof(char));
            if (utf8String == NULL) {
                free(hexData); // Coverity
                hexData = NULL; // Coverity
                LOGE("onUssd:utf8String malloc fail");
                goto error;
            }

            /* The USS strings need to be transform to utf8 */
            if (!strcmp(dcs,"GSM7")) {
                LOGD("Ussd GSM7");
                utf8_from_unpackedgsm7((cbytes_t) hexData, 0, len, utf8String);
                LOGD("onUssd for loop, utf8String= %s", utf8String);
            } else if (!strcmp(dcs,"UCS2")) {
                LOGD("Ussd UCS2");

                // Some character can't display when receive the ussd notification
                zero4_to_space(hexData, len);
                ucs2_to_utf8((cbytes_t) hexData, len/2, utf8String);
            }  else {
                LOGD("Ussd not GSM7 or UCS2");
                utf8_from_gsm8((cbytes_t) hexData, len, utf8String);
            }
            finalData[1] = (char *) utf8String;
            LOGD("onUssd, each string of finalData[1]= %s", finalData[1]);
        } else {
            finalData[i] = p_cur[i];
            LOGD("onUssd, each string of finalData[%d]= %s",i , finalData[i]);
        }
    }

    LOGD("onUssd finalData[0]= %s", finalData[0]);

    // Network might response empty str
    if (hasStr) {
        LOGD("onUssd finalData[1]= %s", finalData[1]);
    }

    RIL_onUnsolicitedResponseSocket (
            RIL_UNSOL_ON_USSD,
            finalData, length * sizeof(char *),
            rid);

    if (dcsString != NULL) {
        free(dcsString);
    }
    free(hexData);
    free(utf8String);
    return;

error:
    LOGE("Parse RIL_UNSOL_ON_USSD fail: %s/n", s);
}

extern int rilSsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_SEND_USSD:
            requestSendUSSD(data,datalen,t);
        break;
        case RIL_REQUEST_CANCEL_USSD:
            requestCancelUssd(data,datalen,t);
        break;
        case RIL_REQUEST_GET_CLIR:
            requestGetClir(data,datalen,t);
        break;
        case RIL_REQUEST_SET_CLIR:
            requestSetClir(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
            requestQueryCallForwardStatus(data,datalen,t);
        break;
        case RIL_REQUEST_SET_CALL_FORWARD:
            requestSetCallForward(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_CALL_WAITING:
            requestQueryCallWaiting(data,datalen,t);
        break;
        case RIL_REQUEST_SET_CALL_WAITING:
            requestSetCallWaiting(data,datalen,t);
        break;
        case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
            requestChangeBarringPassword(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_CLIP:
            requestQueryClip(data,datalen,t);
        break;
        case RIL_REQUEST_SET_CLIP:
            requestSetClip(data,datalen,t);
        break;
        case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION:
            requestSetSuppSvcNotification(data,datalen,t);
        break;
        /* MTK proprietary start */
        case RIL_REQUEST_GET_COLP:
            requestGetColp(data, datalen, t);
        break;
        case RIL_REQUEST_SET_COLP:
            requestSetColp(data, datalen, t);
        break;
        case RIL_REQUEST_GET_COLR:
            requestGetColr(data, datalen, t);
        break;
        case RIL_REQUEST_SEND_CNAP:
            ///M: For query CNAP
            requestSendCNAPUSSD(data, datalen, t);
        break;
        /* MTK proprietary end */
        default:
            return 0; /* no matched request */
        break;
    }

    return 1; /* request found and handled */
}


extern int rilSsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s,"+CSSI:")) {
        /* +CSSI is MO */
        onSuppSvcNotification((char *)s, 0, rid);
        return 1;

    } else if (strStartsWith(s,"+CSSU:")) {
        /* +CSSU is MT */
        onSuppSvcNotification((char *)s, 1, rid);
        return 1;

    } else if (strStartsWith(s,"+CUSD:")) {
        onUssd((char *)s,rid);
        return 1;

    } else if (strStartsWith(s,"+ECFU:")) {
        LOGD("Call Forwarding Flag:%s", s);
        onCfuNotify((char *)s,rid);
        return 1;
    } else if (strStartsWith(s,"+ECMCCSS:")) {
        LOGD("LTE SS Runtime Indication URC:%s", s);
        onLteSuppSvcNotification((char *) s, rid);
        return 1;
    } else if (strStartsWith(s, "+EIMSEMSIND:")) {
        /* +EIMSEMSIND: <call_id>[,"<pau>"] */
        LOGD("LTE Emergency call URC:%s", s);
        onLteSuppSvcNotification((char *) s, rid);
        return 1;
    }

    return 0;
}


void ssRuntimeIndTest(RIL_SOCKET_ID rid) {
    char str[PROPERTY_VALUE_MAX];
		int value;
		char *ss = NULL;
		property_get("ss.runtime.indication", str, 0);
		value = atoi(str);
		if ((value >= 1) && (value <= 4)) {
			switch (value) {
			case 1: 
				ss = strdup("+ECMCCSS: 1, 256, Reason=sip;cause=603;text=\"Decline\"");
				LOGD("[SS Runtime Indication]Call barring test");
				break;
			case 2:
				ss = strdup("+ECMCCSS: 1, 259, <urn:alert:Service:call-waiting>");
				LOGD("[SS Runtime Indication]Call waiting test");
				break;
			case 3:
				ss = strdup("+ECMCCSS: 1, 13, History-Info: <sip:UserB@ims.example.com>; index=1, <sip:UserC@ims.example.com?Reason=SIP;cause=302;text=\"CDIV\">; index=1.1, <sip:UserD@ims.example.com?Reason=SIP;cause=302;text=\"CDIV\">; index=1.2");				
				LOGD("[SS Runtime Indication]Call forwarding test");
				break;
			/*case 4:
				ss = strdup("+ECMCCSS: 1, 258, Messages-Waiting: yes\r\nMessage-Account: sip:kristiina@example.com\r\nVoice-Message: 2/1 (1/0)\r\nVideo-Message: 0/1 (0/0)");
				LOGD("[SS Runtime Indication]Message Waiting Indication test");
				break;*/
			}

            if (ss != NULL) {
                onLteSuppSvcNotification((char *) ss, rid);
            }
            if (ss != NULL) {
                LOGI("free ss: [%s]", ss);
                free(ss);  //Coverity: OK for free, reason:
                            // 1. onLteSuppSvcNotification is a blocking function.
                            // 2. cacheUrc will deep copy the ss object.
            }
		}
}
