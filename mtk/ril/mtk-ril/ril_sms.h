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

#ifndef RIL_SMS_H
#define RIL_SMS_H 1

#define MAX_SMSC_LENGTH 11
#define MAX_TPDU_LENGTH 164

#define TPDU_MAX_TPDU_SIZE      (175)
#define TPDU_MAX_ADDR_LEN       (11)
#define TPDU_ONE_MSG_OCTET		(140)

#define TPDU_MTI_BITS			(0x03)
#define TPDU_MTI_DELIVER		(0x00)
#define TPDU_MTI_SUBMIT 		(0x01)
#define TPDU_MTI_UNSPECIFIED 	(0x02)
#define TPDU_MTI_RESERVED		(0x03)

#define TPDU_VPF_BITS			(0x18)
#define TPDU_VPF_NOT_PRESENT	(0x00)
#define TPDU_VPF_ENHANCED		(0x01)
#define TPDU_VPF_RELATIVE		(0x02)
#define TPDU_VPF_ABSOLUTE		(0x03)

/*------------------------------
 * Protocol Identifier (PID)
 *------------------------------*/
#define TPDU_PID_TYPE_0            (0x40)
#define TPDU_PID_REP_TYPE_1        (0x41)   /* Replace Type 1 */
#define TPDU_PID_REP_TYPE_2        (0x42)   /* Replace Type 2 */
#define TPDU_PID_REP_TYPE_3        (0x43)   /* Replace Type 3 */
#define TPDU_PID_REP_TYPE_4        (0x44)   /* Replace Type 4 */
#define TPDU_PID_REP_TYPE_5        (0x45)   /* Replace Type 5 */
#define TPDU_PID_REP_TYPE_6        (0x46)   /* Replace Type 6 */
#define TPDU_PID_REP_TYPE_7        (0x47)   /* Replace Type 7 */
#define TPDU_PID_RCM               (0x5f)   /* Return Call Message */
#define TPDU_PID_ANSI_136_RDATA    (0x7c)   /* ANSI-136 R-DATA */
#define TPDU_PID_ME_DOWNLOAD       (0x7d)   /* ME Data Download */
#define TPDU_PID_ME_DE_PERSONAL    (0x7e)   /* ME De-personalization */
#define TPDU_PID_SIM_DOWNLOAD      (0x7f)   /* SIM Data Download */

#define TPDU_PID_CHECK             (0xC0)
#define TPDU_PID_MASK              (0xE0)
#define TPDU_PID_RESERVED          (0x80)

/*------------------------------------
 * Data Coding Scheme (DCS) Checking
 *------------------------------------*/
#define TPDU_DCS_DEFAULT		 (0x00)
#define TPDU_DCS_CODING1         (0xc0)
#define TPDU_DCS_CODING2         (0xf0)
#define TPDU_DCS_RESERVE_BIT     (0x08)
#define TPDU_DCS_ALPHABET_CHECK  (0x0c)
#define TPDU_DCS_COMPRESS_CHECK  (0x20)


/******************************
* Although CB CHANNEL ID can be 65535 defined in the spec.
* But currently, all application only support the ID 0-999
* so we define the MAX_CB_CHANNEL_ID as 6500
* > MTK60048 - Change the value of MAX_CB_CHANNEL_ID to support CB service in EU
*   EU-Alert level 1                                            : 4370
*   EU-Alert level 2                                            : 4371-4372
*   EU-Alert level 3                                            : 4373-4378
*   EU-Info Message                                             : 6400
*   EU-Amber                                                    : 4379
*   CMAS CBS Message Identifier for the Required Monthly Test.  : 4380
******************************/
#define MAX_CB_CHANNEL_ID       6500
#define MAX_CB_DCS_ID           256

typedef enum
{
    RIL_SMS_REC_UNREAD,
    RIL_SMS_REC_RDAD,
    RIL_SMS_STO_UNSENT,
    RIL_SMS_STO_SENT,
    RIL_SMS_MESSAGE_MAX
}RIL_SMS_MESSAGE_STAT;

typedef enum
{
    SMS_ENCODING_7BIT,
    SMS_ENCODING_8BIT,
    SMS_ENCODING_16BIT,
    SMS_ENCODING_UNKNOWN
}SMS_ENCODING_ENUM;

typedef enum{
    SMS_MESSAGE_CLASS_0,
    SMS_MESSAGE_CLASS_1,
    SMS_MESSAGE_CLASS_2,
    SMS_MESSAGE_CLASS_3,
    SMS_MESSAGE_CLASS_UNSPECIFIED
}SMS_MESSAGE_CLASS_ENUM;

typedef enum
{
    TPDU_PID_DEFAULT_PID = 0x00,   /* Text SMS */
    TPDU_PID_TELEX_PID   = 0x21,   /* Telex */
    TPDU_PID_G3_FAX_PID  = 0x22,   /* Group 3 telefax */
    TPDU_PID_G4_FAX_PID  = 0x23,   /* Group 4 telefax */
    TPDU_PID_VOICE_PID   = 0x24,   /* Voice Telephone */
    TPDU_PID_ERMES_PID   = 0x25,   /* ERMES (European Radio Messaging System) */
    TPDU_PID_PAGING_PID  = 0x26,   /* National Paging system */
    TPDU_PID_X400_PID    = 0x31,   /* Any public X.400-based message system */
    TPDU_PID_EMAIL_PID   = 0x32    /* E-mail SMS */
}TPDU_PID_ENUM;

typedef enum
{
    TPDU_NO_ERROR			= 0x00,

    /* TP-DA Error */
    TPDU_DA_LENGTH_ERROR    = 0x01,

    /* TP-VPF Error */
    TPDU_VPF_NO_SUPPORT		= 0x02,

    /* TP-MTI Error */
    TPDU_MTI_SUBMIT_ERROR	= 0x03,
    TPDU_MTI_DELIVER_ERROR	= 0x04,

    /* TP-OA Error */
    TPDU_OA_LENGTH_ERROR    = 0x05,

    /* Length Error */
    TPDU_MSG_LEN_EXCEEDED	= 0x06,

    /* TP-PID Error */
    TELEMATIC_INT_WRK_NOT_SUPPORT = 0x80, /* telematic interworking not support */
    SMS_TYPE0_NOT_SUPPORT         = 0x81, /* short message type 0 not support */
    CANNOT_REPLACE_MSG            = 0x82,
    UNSPECIFIED_PID_ERROR         = 0x8F,

    /* DCS error */
    ALPHABET_NOT_SUPPORT          = 0x90, /* data coding scheme (alphabet) not support */
    MSG_CLASS_NOT_SUPPORT         = 0x91, /* message class not support */
    UNSPECIFIED_TP_DCS            = 0x9f,
}TPDU_ERROR_CAUSE_ENUM;

typedef enum
{
    /* Check for telematics support */
    TPDU_TELEMATICS_CHECK = 0x20,

    /* Check for TYPE0 short messages */
    TPDU_MSG_TYPE0_CHECK  = 0x3F,

    /* Check for optional paramter
     * of TPDU header PID, DCS, UDL */
    TPDU_PARAM_CHECK      = 0x03,
    TPDU_DCS_PRESENT      = 0x02,
    TPDU_PID_PRESENT      = 0x01,

    /* Check for report messages
     * coming for SMS COMMAND */
    TPDU_COMMAND_CHECK    = 0xFF,

    /* Check for length of TPDU */
    TPDU_TPDU_LEN_CHECK   = 175
}TPDU_MSG_CHECK_ENUM;

typedef struct
{
    void *data;
    void *next;
}RIL_SMS_Linked_list;

#define SMS_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RIL_SMS_MEM_TYPE_TOTAL          3
#define RIL_SMS_GOTO_CHECK(condition, label)    \
            if (condition)                      \
                goto label

#define RIL_SMS_GOTO_DONE_CHECK(error)  RIL_SMS_GOTO_CHECK((error < 0), done)
#define RIL_SMS_GOTO_ERR_CHECK(error)   RIL_SMS_GOTO_CHECK((error < 0), err)

extern void requestSendSMS(void * data, size_t datalen, RIL_Token t);

extern void requestSendSmsExpectMore(void * data, size_t datalen, RIL_Token t);

extern void requestSMSAcknowledge(void * data, size_t datalen, RIL_Token t);

extern void requestSMSAcknowledgeWithPdu(void * data,size_t datalen, RIL_Token t);

extern void requestWriteSmsToSim(void * data, size_t datalen, RIL_Token t);

extern void requestDeleteSmsOnSim(void *data, size_t datalen, RIL_Token t);

extern void requestGSMSMSBroadcastActivation(void *data, size_t datalen, RIL_Token t);

extern void requestGetSMSCAddress(void *data, size_t datalen, RIL_Token t);

extern void requestSetSMSCAddress(void *data, size_t datalen, RIL_Token t);

extern void onNewSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid);

extern void onNewSmsStatusReport(const char *urc, const char *smspdu, RIL_SOCKET_ID rid);

extern void onNewSmsOnSim(const char *urc, RIL_SOCKET_ID rid);

extern void onSimSmsStorageStatus(const char *line, RIL_SOCKET_ID rid);

extern void onNewBroadcastSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid);

extern int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

/* MTK proprietary start */
extern void requestGetSmsSimMemoryStatus(void *data, size_t datalen, RIL_Token t);
extern void requestReportSMSMemoryStatus(void *data, size_t datalen, RIL_Token t);
extern void requestGetSmsParams(void *data, size_t datalen, RIL_Token t);
extern void requestSetSmsParams(void *data, size_t datalen, RIL_Token t);
extern void requestSetEtws(void *data, size_t datalen, RIL_Token t);
extern void requestSetCbChannelConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestSetCbLanguageConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestGetCellBroadcastConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestSetAllCbLanguageOn(void *data, size_t datalen, RIL_Token t);
extern void requestRemoveCbMsg(void *data, size_t datalen, RIL_Token t);

extern void requestGSMGetBroadcastSMSConfigEx(void *data, size_t datalen, RIL_Token t);
extern void requestGSMSetBroadcastSMSConfigEx(void *data, size_t datalen, RIL_Token t);


extern void onNewEtwsNotification(const char *line, RIL_SOCKET_ID rid);
/* MTK proprietary end */

#endif /* RIL_SMS_H */

