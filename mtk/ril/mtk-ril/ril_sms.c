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
#include <termios.h>

#include <ril_callbacks.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>


enum{
    CMS_ERROR_NON_CMS = -1,
    CMS_SUCCESS = 0,
    CMS_CM_UNASSIGNED_NUM = 1,                  // Unassigned (unallocated) number
    CMS_CM_OPR_DTR_BARRING = 8,                 //Operator determined barring
    CMS_CM_CALL_BARRED = 10,                       // Call barred
    CMS_CM_CALL_REJECTED = 21,                    // Short message transfer rejected
    CMS_CM_DEST_OUT_OF_ORDER = 27,           //Destination out of order
    CMS_CM_INVALID_NUMBER_FORMAT = 28,   //Unidentified subscriber
    CMS_CM_FACILITY_REJECT = 29,                 // Facility rejected
    CMS_CM_RES_STATUS_ENQ = 30,                 // Unknown subscriber
    CMS_CM_NETWORK_OUT_OF_ORDER = 38,   // Network out of order
    CMS_CM_REQ_FAC_NOT_SUBS = 50,           // Requested facility not subscribed
    CMS_CM_REQ_FACILITY_UNAVAIL = 69,       // Requested facility not implemented
    CMS_CM_INVALID_TI_VALUE = 81,               // Invalid short message transfer reference value
    CMS_CM_SEMANTIC_ERR = 95,                     // Semantically incorrect message
    CMS_CM_MSG_TYPE_UNIMPL = 97,               // Message type non-existent or not implemented
    CMS_CM_IE_NON_EX = 99,                          // Information element non-existent or not implemented
    CMS_CM_SIM_IS_FULL = 322,
    CMS_UNKNOWN = 500,
    SMS_MO_SMS_NOT_ALLOW = 529,
    CMS_CM_MM_CAUSE_START = 2048,
    CMS_CM_MM_IMSI_UNKNOWN_IN_HLR = 0x02 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ILLEGAL_MS = 0x03 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ILLEGAL_ME = 0x06 + CMS_CM_MM_CAUSE_START,
    CMS_CM_RR_PLMN_SRCH_REJ_EMERGENCY = 0x74 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_AUTH_FAILURE = 0x76 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_IMSI_DETACH = 0x77 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_EMERGENCY_NOT_ALLOWED = 0x7d + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ACCESS_CLASS_BARRED = 0x7f + CMS_CM_MM_CAUSE_START,
    CMS_MTK_FDN_CHECK_FAILURE = 2601
}AT_CMS_ERROR;

enum{
    SMS_NONE,
    SMS_IMS,
    SMS_3G
}SMS_TYPE;

/* TODO
* The following constant variables also defined in the ril_callbacks.c
* We should figure out a method to merge them
*/
#ifdef MTK_RIL
static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
#endif /* MTK_RIL */
static const struct timeval TIMEVAL_0 = {2,0};

// default set as available
int phone_storage_status = 0;
int sms_type = SMS_NONE;
/*
 * This is defined on RILD and use to know MD is off or on.
 * In flight mode, it will turns off MD if feature option is ON
*/
extern int s_md_off;
extern void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);

void showCurrIncomingSmsType()
{
    if (sms_type == SMS_NONE) {
        LOGD("current sms_type = SMS_NONE");
    } else if (sms_type == SMS_IMS) {
        LOGD("current sms_type = SMS_IMS");
    } else if (sms_type == SMS_3G) {
        LOGD("current sms_type = SMS_3G");
    } else {
        LOGD("current sms_type = UNKNOWN");
    }
}

/****************************************
* Temporaily, we didn't return a enum for cms ERROR
* because there are too much error casue we can't expected from modem
*
****************************************/
static int at_get_cms_error(const ATResponse *p_response)
{
    int ret;
    int err;
    char *p_cur;

    if (p_response->success > 0) {
        return CMS_SUCCESS;
    }

    if (p_response->finalResponse == NULL)
    {
        return CMS_ERROR_NON_CMS;
    }

    if( strStartsWith(p_response->finalResponse, "ERROR" ) )
    {
        return CMS_UNKNOWN;
    }

    if( !strStartsWith(p_response->finalResponse, "+CMS ERROR:" ) )
    {
        return CMS_ERROR_NON_CMS;
    }

    p_cur = p_response->finalResponse;
    err = at_tok_start(&p_cur);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    err = at_tok_nextint(&p_cur, &ret);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    return ret;
}


int rild_sms_hexCharToDecInt(char *hex, int length)
{
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++)
    {
        if (hex[i]>='0' && hex[i]<='9')
        {
            digit = hex[i] - '0';
        }
        else if ( hex[i]>='A' && hex[i] <= 'F')
        {
            digit = hex[i] - 'A' + 10;
        }
        else if ( hex[i]>='a' && hex[i] <= 'f')
        {
            digit = hex[i] - 'a' + 10;
        }
        else
        {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

/*****************************************************************************
* FUNCTION
*  rild_sms_fo_check
* DESCRIPTION
*   This function check the Fist Octet.
*
* PARAMETERS
*  a  IN       fo (first octet)
* RETURNS
*  true:  fo is valid
*  false: fo is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool rild_sms_fo_check(int fo)
{
    /* Check Message Type Identifier */
    if ((fo & TPDU_MTI_BITS) >= TPDU_MTI_RESERVED)
        return false;

    /* Check Validity Period , currently only relative format supported */
    if ((((fo & TPDU_VPF_BITS) >> 3) != TPDU_VPF_NOT_PRESENT) &&
        (((fo & TPDU_VPF_BITS) >> 3) != TPDU_VPF_RELATIVE))
        return false;

    return true;
}

/*****************************************************************************
* FUNCTION
*  rild_sms_pid_check
* DESCRIPTION
*   This function checks the Protocol IDentifier.
*
* PARAMETERS
*  a  IN       pid
*  b  IN/OUT   *fail_cause_ptr
*  c  IN/OUT   *aux_info, useful for caller
* RETURNS
*  true:  pid is valid
*  false: pid is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool rild_sms_pid_check(int pid)
{
    bool ret_val = true;
    LOGD("rild_sms_submit_pdu_check, pid=%d", pid);

    if (pid != TPDU_PID_DEFAULT_PID)
    {
        /*
         * Check for reserved and SC specific use of
         * * PID value
         */
        if (((pid & TPDU_PID_CHECK) == TPDU_PID_CHECK) ||
            ((pid & TPDU_PID_CHECK) == TPDU_PID_RESERVED))
        {
            ret_val = false;
        }
    }

    return ret_val;
}

/*****************************************************************************
* FUNCTION
*  rild_sms_dcs_check
* DESCRIPTION
*   This function checks the data coding scheme.
*
* PARAMETERS
*  a  IN       dcs
*  b  IN/OUT   *fail_cause_ptr
* RETURNS
*  KAL_TRUE:  dcs is valid
*  KAL_FALSE: dcs is invalid
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool rild_sms_dcs_check(int dcs)
{
    bool ret_val = true;
    unsigned char coding_group;
    LOGD("rild_sms_submit_pdu_check, dcs=%d", dcs);

    if (dcs == 0)                       /* Default DCS */
        return true;

    coding_group = dcs & 0xf0;

    /* bit7..bit4=1111: Data coding/message class */
    if ((dcs & TPDU_DCS_CODING2) == TPDU_DCS_CODING2)
    {
        /* Bit3 is reserved, shall equals to zero */
        if ((dcs & TPDU_DCS_RESERVE_BIT) == TPDU_DCS_RESERVE_BIT)
        {
            ret_val = false;
        }
    }
    /* bit7..bit4=00xx: General Data coding indication */
    else if ((dcs & TPDU_DCS_CODING1) == 0x00)
    {
        /* bit 3 and bit 2 are reserved */
        if ((dcs & TPDU_DCS_ALPHABET_CHECK) == TPDU_DCS_ALPHABET_CHECK)
        {
            ret_val = false;
        }
        else if ((dcs & TPDU_DCS_COMPRESS_CHECK) == TPDU_DCS_COMPRESS_CHECK)
        {
            /* Currently, not support compression */
            ret_val = false;
        }
    }
    else if ((coding_group & 0xc0) == 0x80)
    {
        ret_val = false;
    }
    else if ((coding_group == 0xc0) || (coding_group == 0xd0) || (coding_group == 0xe0))
    {
        /* Bit2 is reserved, shall equals to zero */
        if ((dcs & 0x04) == 0x04)
        {
            ret_val = false;
        }
    }

    return ret_val;
}

/*****************************************************************************
* FUNCTION
*  rild_sms_decode_dcs
* DESCRIPTION
*   This function decodes the data coding scheme.
*
* PARAMETERS
*  a  IN       dcs
*  b  IN/OUT   alphabet_type
*  c  IN/OUT   msg_class
*  d  IN/OUT   compress
* RETURNS
*  none
* GLOBALS AFFECTED
*  none
*****************************************************************************/
void rild_sms_decode_dcs(int             dcs,
                      SMS_ENCODING_ENUM         *alphabet_type,
                      SMS_MESSAGE_CLASS_ENUM    *msg_class,
                      bool                   *is_compress)
{
    unsigned char coding_group;

    /* Default DCS value */
    *alphabet_type = SMS_ENCODING_7BIT;
    *msg_class = SMS_MESSAGE_CLASS_UNSPECIFIED;
    *is_compress = false;

    if (dcs == 0)
        return;

    if (dcs == 0x84)
    {
        *alphabet_type = SMS_ENCODING_16BIT;
        return;
    }

    coding_group = dcs >> 4;

    if (coding_group == 0x0f)           /* Data Coding/Message Class */
    {
        /*
         * SCR: 2001,
         * * ensure the reserved bit is zero
         */
        if ((dcs & 0x08) == 0x08)
        {
            *alphabet_type = SMS_ENCODING_UNKNOWN;
        }
        else
        {
            *msg_class = (SMS_MESSAGE_CLASS_ENUM) (dcs & 0x03);
            *alphabet_type = (SMS_ENCODING_ENUM) ((dcs & 0x04) >> 2);
        }
    }
    else if ((coding_group | 0x07) == 0x07)     /* General Data Coding Scheme */
    {
        if ((dcs & 0x10) == 0x10)
            *msg_class = (SMS_MESSAGE_CLASS_ENUM) (dcs & 0x03);
        else
            *msg_class = SMS_MESSAGE_CLASS_UNSPECIFIED;

        *alphabet_type = (SMS_ENCODING_ENUM) ((dcs >> 2) & 0x03);
        if (((coding_group & 0x02) >> 1) == 1)
            *is_compress = true;
    }
    else if ((coding_group & 0x0c) == 0x08)
    {
        /*
         * according GSM 23.038 clause 4:
         * "Any reserved codings shall be assumed to be the GSM 7 bit default alphabet."
         */
    }
    else if (((coding_group & 0x0f) == 0x0c) || /* discard */
             ((coding_group & 0x0f) == 0x0d) || /* store, gsm-7 */
             ((coding_group & 0x0f) == 0x0e))   /* store, ucs2 */
    {
        /* 1110: msg wait ind (store, ucs2) */
        if ((coding_group & 0x0f) == 0x0e)
            *alphabet_type = SMS_ENCODING_16BIT;
    }

    /*
     * if the reserved bit been set or the alphabet_type uses the reserved one,
     * then according GSM 23.038 clause 4:
     * "Any reserved codings shall be assumed to be the GSM default alphabet."
     * we change it as SMSAL_GSM7_BIT
     */

    if (*alphabet_type == SMS_ENCODING_UNKNOWN)
    {
        *alphabet_type = SMS_ENCODING_7BIT;
    }

}

/*****************************************************************************
* FUNCTION
*  rild_sms_is_len_in8bit
* DESCRIPTION
*   This function determines the unit of TP-User-Data.
*
* PARAMETERS
*  a  IN          dcs
* RETURNS
*  KAL_TRUE: length in octet
*  KAL_TRUE: length in septet
* GLOBALS AFFECTED
*  none
*****************************************************************************/
bool rild_sms_is_len_in8bit(int dcs)
{
    bool is_compress;
    SMS_ENCODING_ENUM alphabet_type;
    SMS_MESSAGE_CLASS_ENUM mclass;

    rild_sms_decode_dcs(dcs, &alphabet_type, &mclass, &is_compress);

    if ((is_compress == true) ||
        (alphabet_type == SMS_ENCODING_8BIT) ||
        (alphabet_type == SMS_ENCODING_16BIT))
    {
        return true;
    }
    else
    {
        return false;
    }

}

unsigned int rild_sms_msg_len_in_octet(int dcs, int len)
{
    if (rild_sms_is_len_in8bit(dcs))
    {
        return len;
    }
    else
    {
        return (len * 7 + 7) / 8;
    }
}

bool rild_sms_submit_pdu_check(int pdu_len, char *pdu_ptr, TPDU_ERROR_CAUSE_ENUM  *error_cause)
{
    unsigned char off = 0;                  /* offset */
    int dcs;
    int fo;
    unsigned char udl, user_data_len;
    int i=0;
    int pdu_array[pdu_len/2];

    *error_cause = TPDU_NO_ERROR;
    LOGD("rild_sms_submit_pdu_check, pdu_len=%d", pdu_len);

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0)
    {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++)
    {
        pdu_array[i] = rild_sms_hexCharToDecInt((pdu_ptr+2*i), 2);
    }
    /* check sca */
    LOGD("SMSC address len =%d", pdu_array[0]);
    if (pdu_array[0] > TPDU_MAX_ADDR_LEN)
        return false;

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-SUBMIT or not */
    LOGD("First Octet =%d, off=%d", pdu_array[off], off);
    if ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_SUBMIT)
    {
        LOGD("TPDU_MTI_SUBMIT fail");
        return false;
    }

    /* check fo */
    fo = pdu_array[off];
    if (rild_sms_fo_check(pdu_array[off]) == false)
    {
        LOGD("rild_sms_fo_check fail");
        return false;
    }

    off += 2;                           /* move to da, (skip fo and msg_ref) */

    /* check da */
    LOGD("DA length =%d, off=%d", pdu_array[off], off);
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2)
    {
        LOGD("DA length check fail");
        return false;
    }

    off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */

    /* check pid */
    LOGD("pid =%d, off=%d", pdu_array[off], off);
    if (rild_sms_pid_check(pdu_array[off]) == false)
    {
        LOGD("rild_sms_pid_check check fail");
        return false;
    }

    off++;                              /* move to dcs */

    /* check dcs */
    LOGD("dcs =%d, off=%d", pdu_array[off], off);
    dcs = pdu_array[off];
    if (rild_sms_dcs_check(dcs) == false)
    {
        LOGD("rild_sms_dcs_check check fail");
        return false;
    }

    /* move to udl */
    if (((fo & TPDU_VPF_BITS) >> 3) == TPDU_VPF_RELATIVE)
        off += 2;                       /* VP is relative format (1 byte) */
    else
        off++;                          /* VP not present */


    /* udl */
    LOGD("udl=%d, off=%d", pdu_array[off], off);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) rild_sms_msg_len_in_octet(dcs, udl);
    LOGD("user_data_len=%d", user_data_len);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off)))
    {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

bool rild_sms_deliver_pdu_check(int pdu_len, char *pdu, TPDU_ERROR_CAUSE_ENUM *error_cause)
{
    int off = 0;                  /* offset */
    int dcs;
    unsigned char udl, user_data_len;
    int pdu_array[pdu_len/2];
    int i=0;

    LOGD("rild_sms_deliver_pdu_check, pdu_len=%d", pdu_len);

    *error_cause = TPDU_NO_ERROR;

    /* check length */
    if (pdu_len/2 > TPDU_MAX_TPDU_SIZE || pdu_len == 0)
    {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    for (i = 0; i < pdu_len/2 ;i ++)
    {
        pdu_array[i] = rild_sms_hexCharToDecInt((pdu+2*i), 2);
    }

    LOGD("SMSC address len =%d", pdu_array[0]);
    if (pdu_array[0] > TPDU_MAX_ADDR_LEN)
        return false;

    off += (1 + pdu_array[0]);              /* move to head of TPDU (skip sca_len, sca) */

    /* check this pdu is SMS-DELIVER or not */
    LOGD("first octet =%d", pdu_array[off]);
    if (((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_DELIVER) &&
        ((pdu_array[off] & TPDU_MTI_BITS) != TPDU_MTI_UNSPECIFIED))
    {
        LOGD("first octet check fail");
        return false;
    }

    off += 1;                           /* move to oa, (skip fo ) */

    /* check oa */
    LOGD("OA length =%d", pdu_array[off]);
    if (pdu_array[off] > (TPDU_MAX_ADDR_LEN - 1) * 2)
    {
        LOGD("OA length check fail");
        return false;
    }

    if ((pdu_array[off] % 2) >0)
        off += 1 + 1 + ((pdu_array[off]+1)/2);    /* skip addr_len, addr digits & addr type */
    else
        off += 1 + 1 + (pdu_array[off]/2);    /* skip addr_len, addr digits & addr type */

    off++;                              /* skip pid, move to dcs */

    /* check dcs */
    dcs = pdu_array[off];
    LOGD("dcs =%d", pdu_array[off]);

    off += (1 + 7);                     /* skip dcs, move to uhl */

    /* udl */
    LOGD("udl =%d", pdu_array[off]);
    udl = pdu_array[off];

    /* move to user data */
    off++;

    user_data_len = (unsigned char) rild_sms_msg_len_in_octet(dcs, udl);
    LOGD("user_data_len=%d", user_data_len);

    if (user_data_len > TPDU_ONE_MSG_OCTET || (user_data_len != (pdu_len/2 - off)))
    {
        *error_cause = TPDU_MSG_LEN_EXCEEDED;
        return false;
    }

    return true;

}

int rild_sms_queryCBInfo(int *mode, char **ch_str, char **dcs_str, int *alllanguageon, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    char *line, *chIDs, *dcsIDs;

    *ch_str = *dcs_str = NULL;

    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get mode
    err = at_tok_nextint(&line, mode);
    if(err < 0 || (*mode) < 0 || (*mode) > 1) goto error;

    // get <mids> ID string
    err = at_tok_nextstr(&line, &chIDs);
    if(err < 0) goto error;
    asprintf(ch_str, "%s", chIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str
    * after using this function
    *******************************************/

    // get <dcss> ID string
    err = at_tok_nextstr(&line, &dcsIDs);
    if(err < 0) goto error;
    asprintf(dcs_str, "%s", dcsIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str
    * after using this function
    *******************************************/

    // get alllanguageon
    err = at_tok_nextint(&line, alllanguageon);
    if(err < 0)
        *alllanguageon = 0;

    at_response_free(p_response);
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

int rild_sms_getIDTable(char *table, char *id_string, int maximum_id)
{
    int len, i, j;
    int value, begin;

    memset(table, 0x00, maximum_id);
    len = strlen(id_string);

    if (len == 0)
        return 0;

    for(i = 0, begin = -1, value = 0; i <= len; i++, id_string++)
    {
        //LOGD("%s", id_string);
        if ( (*id_string) == ' ' )
            continue;
        else if ( (*id_string) == ',' || (*id_string == '\0'))
        {
            if (begin == -1)
            {
                /* %d, */
                begin = value;
            }
            else
            {
                /* %d-%d,*/
                begin = value + 1; /* ignore MIR value*/
            }

            for (j = begin; j <= value; j++)
            {
                if (j < maximum_id) {
                    table[j] = 1;
                    //LOGD("Set table valud %d", j);
                }
                else {
                    ;/* ignore */
                }
            }

            begin = -1;
            value = 0;
        }
        else if ( (*id_string) == '-' )
        {
            begin = value;
            value = 0;
        }
        else if ( (*id_string) >= '0' && (*id_string) <= '9' )
        {
            value = value*10 + (*id_string) - '0';
        }
        else
        {
            LOGE("There is some invalid characters: %x", (*id_string) );
            return -1;
        }
    }

    return 0;
}

static int  rild_sms_packPDU(const char* smsc, const char *tpdu, char *pdu, bool check)
{
    int len=0;
    TPDU_ERROR_CAUSE_ENUM  error_cause = 0;
    int i=0;

    //Check SMSC
    if (smsc == NULL)
    {
        // "NULL for default SMSC"
        smsc= "00";
    }
    else
    {
        len = strlen(smsc);
        if ( (len%2) != 0 || len<2 || (len/2-1) > MAX_SMSC_LENGTH)
        {
            LOGE("The length of smsc is not valid: %d", len);
            return -1;
        }
    }

    // Check PDU
    if (tpdu == NULL)
    {
        LOGE("PDU should not be NULL");
        return -1;
    }
    len = strlen(tpdu);
    if ( (len%2) != 0)
    {
        LOGE("The length of TPDU is not valid: %d", len);
        return -1;
    }
    len /= 2;
    if ( len > MAX_TPDU_LENGTH)
    {
        LOGE("The length of TPDU is too long: %d", len);
        return -1;
    }

    sprintf(pdu, "%s%s", smsc, tpdu);

    LOGD("pdu len=%d", strlen(pdu));

    if (check)
    {
        if (rild_sms_submit_pdu_check(strlen(pdu), pdu, &error_cause) == false)
        {
            LOGE("rild_sms_submit_pdu_check error, error_cause = %d", error_cause);
            return -1;
        }
        else
        {
            LOGD("rild_sms_submit_pdu_check pass");
        }
    }

    return len;
}

static int rild_sms_checkReceivedPDU(int length, char* pdu, bool check)
{
    int pdu_length;
    int sca_length;
    TPDU_ERROR_CAUSE_ENUM  error_cause = 0;
    int i=0;

    pdu_length = strlen(pdu);
    if (pdu_length < 2)
    {
        LOGW("The PDU structure might be wrong");
        return -1;
    }

    sca_length = rild_sms_hexCharToDecInt(pdu, 2);
    if (sca_length < 0)
    {
        LOGW("The PDU structure might be wrong with invalid digits: %d", sca_length);
        return -1;
    }

    if (check)
    {
        if (rild_sms_deliver_pdu_check(pdu_length, pdu, &error_cause) == false)
        {
            LOGE("rild_sms_deliver_pdu_check error, error_cause = %d", error_cause);
            return -1;
        }
        else
        {
            LOGD("rild_sms_deliver_pdu_check pass");
        }
    }

    /***************************************************
     * It's does not need to check this part due to ..
     * 1. rild_sms_deliver_pdu_check has already checked
     *    the legnth of this pdu
     * 2. Use emulator to receive a SMS, the length is
     *    hardcord and wrong on +CMT commands.
     *    It will never receive any MT SMS from emulator.
     *    It needs to fix on emulator coding and takes
     *    time.
     ***************************************************/
    pdu_length -= (sca_length + 1)*2;
    if (pdu_length != length*2)
    {
        // Still print the log for debug usage
        LOGW("The specified TPDU length is not matched to the TPDU data:  %d", length);
    }

    return 0;
}

static void rild_sms_sendSMS(int request, void * data, size_t datalen, RIL_Token t)
{
    int err, len=0, cause;
    const char *smsc;
    const char *pdu;
    char *cmd1, *line;
    char buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0
    RIL_SMS_Response response;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    RIL_UNUSED_PARM(datalen);

    smsc = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    /* fill error code first */
    response.errorCode = ril_errno;

    len = rild_sms_packPDU(smsc, pdu, buffer, true);
    if(len < 0) goto error;

    /********************************************
    * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
    * We should send AT+CMMS=1 to enable keeping relay protocol link
    *********************************************/
    if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE)
    {
        err = at_send_command("AT+CMMS=1", &p_response, SMS_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0)
        {
            LOGE("Error occurs while executing AT+CMMS=1");
            goto error;
        }
        at_response_free(p_response);
    }


    asprintf(&cmd1, "AT+CMGS=%d, \"%s\"", len, buffer);
    err = at_send_command_singleline(cmd1, "+CMGS:", &p_response, SMS_CHANNEL_CTX);
    free(cmd1);
    if (err < 0) {
        LOGE("Error occurs while executing AT+CMGS=%d, \"%s\"", len, buffer);
        goto error;
    }

    if (p_response->success == 0) {
        /****************************************
        * FIXME
        * Need to check what cause should be retry latter.
        ****************************************/
        cause = at_get_cms_error(p_response);
        if (cause == CMS_MTK_FDN_CHECK_FAILURE)
        {
            ril_errno = RIL_E_FDN_CHECK_FAILURE;
        }
        else if (!(cause == CMS_CM_UNASSIGNED_NUM ||
            cause == CMS_CM_OPR_DTR_BARRING ||
            cause == CMS_CM_CALL_BARRED ||
            cause == CMS_CM_CALL_REJECTED ||
            cause == CMS_CM_DEST_OUT_OF_ORDER ||
            cause == CMS_CM_INVALID_NUMBER_FORMAT ||
            cause == CMS_CM_FACILITY_REJECT ||
            cause == CMS_CM_RES_STATUS_ENQ ||
            cause == CMS_CM_NETWORK_OUT_OF_ORDER ||
            cause == CMS_CM_REQ_FAC_NOT_SUBS ||
            cause == CMS_CM_REQ_FACILITY_UNAVAIL ||
            cause == CMS_CM_INVALID_TI_VALUE ||
            cause == CMS_CM_SEMANTIC_ERR ||
            cause == CMS_CM_MSG_TYPE_UNIMPL ||
            cause == CMS_CM_IE_NON_EX ||
            cause == CMS_CM_MM_IMSI_UNKNOWN_IN_HLR ||
            cause == CMS_CM_MM_ILLEGAL_MS ||
            cause == CMS_CM_MM_ILLEGAL_ME ||
            cause == CMS_CM_RR_PLMN_SRCH_REJ_EMERGENCY ||
            cause == CMS_CM_MM_AUTH_FAILURE ||
            cause == CMS_CM_MM_IMSI_DETACH ||
            cause == CMS_CM_MM_EMERGENCY_NOT_ALLOWED ||
            cause == CMS_CM_MM_ACCESS_CLASS_BARRED ||
            cause == SMS_MO_SMS_NOT_ALLOW) )
        {
            ril_errno = RIL_E_SMS_SEND_FAIL_RETRY;
        }

        /********************************************
        * free because it needs to send another AT
        * commands again
        ********************************************/
        at_response_free(p_response);
        p_response = NULL;
        /********************************************
        * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
        * We should send AT+CMMS=0 to disable keeping relay protocol link
        *********************************************/
        if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE)
        {
            err = at_send_command("AT+CMMS=0", &p_response, SMS_CHANNEL_CTX);
            if (err != 0 || p_response->success == 0)
            {
                LOGW("Warning: error occurs while executing AT+CMMS=0");
            }
        }

        response.errorCode = ril_errno;

        goto error;
    }

    // Get message reference and ackPDU
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);

    err = at_tok_nextint(&line, &(response.messageRef));
    LOGD("message reference number: %d ", response.messageRef);
    if (err < 0) goto error;

    if ( at_tok_hasmore(&line) )
    {
        err = at_tok_nextstr(&line, &(response.ackPDU));
        if (err < 0) goto error;
    }
    else
    {
        response.ackPDU = NULL;
    }

    /* fill success code */
    response.errorCode = RIL_E_SUCCESS;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSendSMS(void * data, size_t datalen, RIL_Token t)
{
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS, data, datalen, t);
}

void requestSendSmsExpectMore(void * data, size_t datalen, RIL_Token t)
{
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS_EXPECT_MORE, data, datalen, t);
}

void requestSMSAcknowledge(void * data, size_t datalen, RIL_Token t)
{
    int ackSuccess, cause;
    int err = 0;
    ATResponse *p_response = NULL;
    char *cmd;

    RIL_UNUSED_PARM(datalen);

    ackSuccess = ((int *)data)[0];
    cause = ((int *)data)[1] & 0xFF;

    showCurrIncomingSmsType();
    if (ackSuccess == 1)
    {
        if (sms_type == SMS_3G)
        {
            err = at_send_command("AT+CNMA=1, 2, \"0000\"", &p_response, SMS_CHANNEL_CTX);
        }
        else if (sms_type == SMS_IMS)
        {
            err = at_send_command("AT+EIMSCNMA=1, 2, \"0000\"", &p_response, SMS_CHANNEL_CTX);
        }
        else
        {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
    }
    else if (ackSuccess == 0)
    {
        if (sms_type == SMS_3G)
        {
            asprintf(&cmd, "AT+CNMA=2, 3, \"00%02X00\"", cause);
        }
        else if (sms_type == SMS_IMS)
        {
            asprintf(&cmd, "AT+EIMSCNMA=2, 3, \"00%02X00\"", cause);
        }
        else
        {
            LOGE("SMS type is SMS_NONE");
            goto error;
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    }
    else
    {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response->success == 0)
        goto error;

    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestSMSAcknowledgeWithPdu(void * data, size_t datalen, RIL_Token t)
{
    const char *ackSuccess;
    const char *pdu;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;

    RIL_UNUSED_PARM(datalen);

    ackSuccess = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    showCurrIncomingSmsType();
    if (*ackSuccess == '1')
    {
        if (sms_type == SMS_3G)
        {
            asprintf(&cmd, "AT+CNMA=1, 2, \"%s\"", pdu);
        }
        else if (sms_type == SMS_IMS)
        {
            asprintf(&cmd, "AT+EIMSCNMA=1, 2, \"%s\"", pdu);
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    }
    else if (*ackSuccess == '0')
    {
        if (sms_type == SMS_3G)
        {
            asprintf(&cmd, "AT+CNMA=2, 3, \"%s\"", pdu);
        }
        else if (sms_type == SMS_IMS)
        {
            asprintf(&cmd, "AT+EIMSCNMA=2, 3, \"%s\"", pdu);
        }
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    }
    else
    {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response->success == 0)
        goto error;

    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    sms_type = SMS_NONE;
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestWriteSmsToSim(void * data, size_t datalen, RIL_Token t)
{
    RIL_SMS_WriteArgs *p_args;
    char *cmd, buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0;
    int length, err, response[1], ret = 0;
    ATResponse *p_response = NULL;
    char *line;

    RIL_UNUSED_PARM(datalen);

    p_args = (RIL_SMS_WriteArgs *)data;

    //check stat
    if (p_args->status < RIL_SMS_REC_UNREAD || p_args->status >= RIL_SMS_MESSAGE_MAX)
    {
        LOGE("The status is invalid: %d", p_args->status);
        goto error;
    }

    // pack PDU with SMSC
    length = rild_sms_packPDU(p_args->smsc, p_args->pdu, buffer, false);
    if (length < 0) goto error;

    asprintf(&cmd, "AT+CMGW=%d,%d, \"%s\"", length, p_args->status, buffer);
    err = at_send_command_singleline(cmd, "+CMGW:", &p_response, SMS_CHANNEL_CTX);
    free(cmd);

    ret = at_get_cms_error(p_response);
    if (err < 0 || p_response->success == 0) goto error;

    // get +CMGW: <index>
    line = p_response->p_intermediates->line;

    err = at_tok_start( &line);
    if( err < 0) goto error;

    err = at_tok_nextint(&line, &response[0]);
    if( err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    if(CMS_CM_SIM_IS_FULL == ret) {
        RIL_onRequestComplete(t, RIL_E_SIM_MEM_FULL, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
}

void requestDeleteSmsOnSim(void *data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err, index;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    RIL_UNUSED_PARM(datalen);

    index = ((int *)data)[0];

    if(index > 0)
    {
        asprintf(&cmd, "AT+CMGD=%d", index);
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);

        if ( !(err < 0 || p_response->success == 0) )
        {
            ril_errno = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
    } else if(-1 == index) {
        // delete all sms on SIM
        asprintf(&cmd, "AT+CMGD=0,4");
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);

        if ( !(err < 0 || p_response->success == 0) )
        {
            ril_errno = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
}

void requestGSMSMSBroadcastActivation(void *data, size_t datalen, RIL_Token t)
{
    int activation;
    int err;
    char *line;
    int mode, allLanguageOn;
    char *ch_string = NULL, *dcs_string = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    RIL_UNUSED_PARM(datalen);

    activation = ((int *)data)[0];
    LOGD("activation: %d", activation);

    // asprintf(&line, "AT+CSCB=%d", activation);
    if(1 == activation) {
        asprintf(&line, "AT+CSCB=1");
    } else if(0 == activation){
        LOGD("query previous config info");
        err = rild_sms_queryCBInfo(&mode, &ch_string, &dcs_string, &allLanguageOn, t);
        if (err < 0) goto error;
        LOGD("Current setting: %s, %s", ch_string, dcs_string);

        asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", ch_string, dcs_string);
        if(NULL != ch_string) {
            free(ch_string);
        }
        if(NULL != dcs_string) {
            free(dcs_string);
        }
    } else {
        LOGD("unhandled mode fot AT+CSCB");
        ril_errno = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response->success <= 0)
        ril_errno =RIL_E_GENERIC_FAILURE;

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);

    return;

error:
    if(NULL != ch_string) {
        free(ch_string);
    }
    if(NULL != dcs_string) {
        free(dcs_string);
    }

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestGetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{
    int err, tosca;
    char *line, *sca;
    ATResponse *p_response = NULL;

    RIL_UNUSED_PARM(data);
    RIL_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+CSCA?", "+CSCA:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response->success <= 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    // get <SCA>
    err = at_tok_nextstr(&line, &sca);
    if (err < 0) goto error;

    // get <TOSCA>
    err = at_tok_nextint(&line, &tosca);
    if (err < 0) goto error;

    if (tosca == 0x91 && sca[0] != '+')
    {
        // add '+' in front of the sca
        sca--;
        sca[0] = '+';
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, sca, sizeof(char*));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{
    int err, tosca = 0x81, len;
    char *line, *sca;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    RIL_UNUSED_PARM(datalen);

    sca = (char*) data;

    // get <tosca>
    if (sca[0] == '+')
    {
        tosca = 0x91;
        sca++;
    }

    len = strlen(sca);
    if (len > (MAX_SMSC_LENGTH-1)*2)
    {
        LOGE("The input length of the SCA is too long: %d", len);

    }
    else
    {
        asprintf(&line, "AT+CSCA=\"%s\", %d", sca, tosca);
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);
        if ( !(err < 0 || p_response->success <= 0) )
        {
            /********************************************
            * free because it needs to send another AT
            * commands again
            ********************************************/
            at_response_free(p_response);
            p_response = NULL;
            // ril_errno = RIL_E_SUCCESS;
            asprintf(&line, "AT+CSAS");
            err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
            free(line);
            if ( !(err < 0 || p_response->success <= 0) ) {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void sendNewSmsErrorAck(void *param)
{
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("sendNewSmsErroAck, rid = %d", rid);
    at_send_command("AT+CNMA=2, 3, \"00FF00\"", NULL, getRILChannelCtx(RIL_SMS, rid));
}

void sendNewSmsErrorAckAndReset(void *param)
{
    sendNewSmsErrorAck(param);
    showCurrIncomingSmsType();
    sms_type = SMS_NONE;
}

void sendNewImsSmsErrorAck(void *param)
{
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("sendNewImsSmsErroAck, rid = %d", rid);
    at_send_command("AT+EIMSCNMA=2, 3, \"00FF00\"", NULL, getRILChannelCtx(RIL_SMS, rid));
}

void sendNewImsSmsErrorAckAndReset(void *param)
{
    sendNewImsSmsErrorAck(param);
    showCurrIncomingSmsType();
    sms_type = SMS_NONE;
}

void sendNewSmsErroAckWithTimerCallback(RIL_SOCKET_ID socketId, int newSmsDomain, bool reset)
{
    RIL_SOCKET_ID *pollSimId;

    if (socketId == RIL_SOCKET_1) {
        pollSimId = &s_pollSimId;
    } else if (socketId == RIL_SOCKET_2) {
        pollSimId = &s_pollSimId2;
    } else if (socketId == RIL_SOCKET_3) {
        pollSimId = &s_pollSimId3;
    } else if (socketId == RIL_SOCKET_4) {
        pollSimId = &s_pollSimId4;
    }

    if (newSmsDomain == SMS_3G)
    {
        if (reset)
        {
            RIL_requestProxyTimedCallback(sendNewSmsErrorAckAndReset, pollSimId, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, socketId)->id, "sendNewSmsErrorAckAndReset");
        }
        else
        {
            RIL_requestProxyTimedCallback(sendNewSmsErrorAck, pollSimId, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, socketId)->id, "sendNewSmsErrorAck");
        }
    }
    else if (newSmsDomain == SMS_IMS)
    {
        if (reset)
        {
            RIL_requestProxyTimedCallback(sendNewImsSmsErrorAckAndReset, pollSimId, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, socketId)->id, "sendNewImsSmsErrorAckAndReset");
        }
        else
        {
            RIL_requestProxyTimedCallback(sendNewImsSmsErrorAck, pollSimId, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, socketId)->id, "sendNewImsSmsErrorAck");
        }
    }
    else
    {
        LOGD("sendNewSmsErroAckWithTimerCallback, Type is not reasonable, newSmsDomain = %d",
                newSmsDomain);
    }
}

void onNewSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid)
{
    int length;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    // skip <alpha>
    err = at_tok_nextint(&pUrc, &length);

    // get <length>
    err = at_tok_nextint(&pUrc, &length);

    err = rild_sms_checkReceivedPDU(length,(char *) smspdu, true);
    if (err < 0) goto error;

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_RESPONSE_NEW_SMS,
        smspdu, sizeof(char*),
        rid);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    sendNewSmsErroAckWithTimerCallback(rid, sms_type, true);

    LOGE("onNewSms check fail");
}

void onNewSmsStatusReport(const char *urc, const char *smspdu, RIL_SOCKET_ID rid)
{
    int length;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    err = rild_sms_checkReceivedPDU(length,(char *) smspdu, false);
    if (err < 0) goto error;

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,
        smspdu, sizeof(char*),
        rid);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    sendNewSmsErroAckWithTimerCallback(rid, sms_type, true);

    LOGE("onNewSmsStatusReport check fail");
}

void onNewSmsOnSim(const char *urc, RIL_SOCKET_ID rid)
{
    int err, index[1];
    char *mem;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);
    if (err < 0) goto error;

    err = at_tok_nextstr(&pUrc, &mem);
    if (err < 0) goto error;

    if (strncmp(mem, "SM", 2) != 0)
    {
        LOGE("The specified storage is not SIM storage");
        return;
    }

    err = at_tok_nextint(&pUrc, &index[0]);
    if (err < 0) goto error;

    sms_type = SMS_3G;

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM,
        index, sizeof(index),
        rid);

    return;

error:
    LOGE("There is something wrong with the input URC");
}

void onSimSmsStorageStatus(const char *line, RIL_SOCKET_ID rid)
{
    int err;
    int status, ciev_id;
    int urc_id = -1;
    char *pUrc = (char *)line;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &ciev_id);
    if (err < 0) return;

    err = at_tok_nextint(&pUrc, &status);
    if (err < 0) return;

    if (err < 0)
    {
        LOGE("There is something wrong with the URC: +CIEV:7, <status>");
    }
    else
    {
        switch(status)
        {
            case 0: // available
                break;
            case 1: // full
            case 2: // SIM exceed
                /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                break;
            case 3: // ME exceed
                LOGD("Phone storage status: %d", phone_storage_status);
                if (phone_storage_status == 1)
                {
                    /*********************************************************
                     * It is a workaround solution here.
                     * for mem1, mem2 and mem3, even if all are SIM card
                     * storage due to set as AT+CPMS="SM", "SM", "SM", we still
                     * need to check the phone_storage_status.
                     * It is the current limitation on modem, it should
                     * upgrate the +CIEV for better usage.
                     ********************************************************/
                    urc_id = RIL_UNSOL_ME_SMS_STORAGE_FULL;
                }
                else
                {
                    /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                    urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                }
                break;
            default:
                LOGW("The received <status> is invalid: %d", status);
                break;
        }
        if (urc_id >= 0)
        {
            RIL_onUnsolicitedResponseSocket (urc_id, NULL, 0, rid);
        }
    }
}

int hexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);

    return 0;
}

void hexStringToBytes(const char *in, int inLength, char *out, int outLength)
{
    int i;

    if (in == NULL || out == NULL)
    {
        return;
    }

    if (inLength != outLength * 2)
    {
        return;
    }

    for (i = 0 ; i < inLength ; i += 2)
    {
        out[i/2] = (char)((hexCharToInt(in[i]) << 4)
                           | hexCharToInt(in[i+1]));
    }
}

void onNewBroadcastSms(const char *urc, const char *smspdu, RIL_SOCKET_ID rid)
{
    int length;
    int err;
    char *line;
    char *pUrc = (char *)urc;
    char *byteSmsPdu;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    asprintf(&line, "00%s", smspdu);
    err = rild_sms_checkReceivedPDU(length, line, false);
    free(line);
    if (err < 0) goto error;
    /* Transfer to byte array for responseRaw */
    byteSmsPdu = calloc(1, sizeof(char)*length);
    hexStringToBytes(smspdu, length*2, byteSmsPdu, length);

    RIL_onUnsolicitedResponseSocket (
        RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS ,
        byteSmsPdu, sizeof(char)*length,
        rid);

    free(byteSmsPdu);
    return;

error:
    sms_type = SMS_NONE;
    LOGE("onNewBroadcastSms check fail");
}

void onSmsReady(void *param)
{
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);

    LOGD("On Sms Ready rid: %d", rid);

    /*
     * If modem turns off, RILD should not send any AT command to modem.
     * If RILD sends the at commands to Modem(off), it will block the
     * MUXD/CCCI channels
     */
    if (s_md_off)
    {
        LOGD("Modem turns off");
        return;
    }
    LOGD("Modem turns on");

    RIL_onUnsolicitedResponseSocket(
                RIL_UNSOL_SMS_READY_NOTIFICATION,
                NULL, 0, rid);

    /*
     * Always send SMS messages directly to the TE
     *
     * mode = 1 // discard when link is reserved (link should never be
     *             reserved)
     * mt = 2   // most messages routed to TE
     * bm = 2   // new cell BM's routed to TE
     * ds = 1   // Status reports routed to TE
     * bfr = 1  // flush buffer
     */
    at_send_command_singleline("AT+CSMS=1", "+CSMS:", NULL,getChannelCtxbyProxy(rid));

    at_send_command("AT+CNMI=1,2,2,1,1", NULL,getChannelCtxbyProxy(rid));

    at_send_command("AT+CPMS=\"SM\", \"SM\", \"SM\"", NULL, getChannelCtxbyProxy(rid));
}

extern int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_SEND_SMS:
            requestSendSMS(data, datalen, t);
            break;
        case RIL_REQUEST_SMS_ACKNOWLEDGE:
            requestSMSAcknowledge(data, datalen, t);
            break;
        case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU:
            requestSMSAcknowledgeWithPdu(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_SMS_TO_SIM:
            requestWriteSmsToSim(data, datalen, t);
            break;
        case RIL_REQUEST_DELETE_SMS_ON_SIM:
            requestDeleteSmsOnSim(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
            requestSendSmsExpectMore(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            requestGSMGetBroadcastSMSConfigEx(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            requestGSMSetBroadcastSMSConfigEx(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
            requestGSMSMSBroadcastActivation(data, datalen, t);
            break;
        case RIL_REQUEST_GET_SMSC_ADDRESS:
            requestGetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_SET_SMSC_ADDRESS:
            requestSetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
            requestReportSMSMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS:
            requestGetSmsSimMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_GET_SMS_PARAMS:
            requestGetSmsParams(data, datalen, t);
            break;

        case RIL_REQUEST_SET_SMS_PARAMS:
            requestSetSmsParams(data, datalen, t);
            break;

        case RIL_REQUEST_SET_ETWS:
            requestSetEtws(data, datalen, t);
            break;

        case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO:
            requestSetCbChannelConfigInfo(data, datalen, t);
            break;

        case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO:
            requestSetCbLanguageConfigInfo(data, datalen, t);
            break;

        case RIL_REQUEST_GET_CB_CONFIG_INFO:
            requestGetCellBroadcastConfigInfo(data, datalen, t);
            break;

        case RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON:
            requestSetAllCbLanguageOn(data, datalen, t);
            break;

        case RIL_REQUEST_REMOVE_CB_MESSAGE:
            requestRemoveCbMsg(data, datalen, t);
            break;

        default:
            return 0; /* no matched request */
        break;
     }

    return 1; /* request found and handled */

}

extern int rilSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+CIEV: 7"))
    {
        onSimSmsStorageStatus(s,rid);
    }
    else if (strStartsWith(s, "+CMT:"))
    {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE)
        {
            sms_type = SMS_3G;
            onNewSms(s, sms_pdu,rid);
        }
        else
        {
            LOGE("One 3G or IMS SMS on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_3G, false);
        }
    }
    else if (strStartsWith(s, "+EIMSCMT:"))
    {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE)
        {
            sms_type = SMS_IMS;
            onNewSms(s, sms_pdu,rid);
        }
        else
        {
            LOGE("One 3G or IMS SMS on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_IMS, false);
        }
    }
    else if (strStartsWith(s, "+CMTI:"))
    {
        onNewSmsOnSim(s,rid);
    }
    else if (strStartsWith(s, "+CDS:"))
    {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE)
        {
            sms_type = SMS_3G;
            onNewSmsStatusReport(s, sms_pdu,rid);
        }
        else
        {
            LOGE("One 3G or IMS SMS report on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_3G, false);
        }
    }
    else if (strStartsWith(s, "+EIMSCDS:"))
    {
        showCurrIncomingSmsType();
        if (sms_type == SMS_NONE)
        {
            sms_type = SMS_IMS;
            onNewSmsStatusReport(s, sms_pdu,rid);
        }
        else
        {
            LOGE("One 3G or IMS SMS report on AP, reject");
            // Use the timer callback to send CNMA fail to modem and will reset the
            // sms_type value while the CNMA is sent
            sendNewSmsErroAckWithTimerCallback(rid, SMS_IMS, false);
        }
    }
    else if (strStartsWith(s, "+CBM:"))
    {
        onNewBroadcastSms(s, sms_pdu,rid);
    }
    else if (strStartsWith(s, "+EIND: 1"))
    {
        if (rid == RIL_SOCKET_1)
        {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId, &TIMEVAL_0,
            		getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        } else if (rid == RIL_SOCKET_2) {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId2, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        } else if (rid == RIL_SOCKET_3) {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId3, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        } else if (rid == RIL_SOCKET_4) {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId4, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        }
    }
    else if (strStartsWith(s, "+ETWS:"))
    {
        onNewEtwsNotification(s, rid);
    }
    else
    {
        return 0;
    }

    return 1;
}


