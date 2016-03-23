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
#include <ctype.h>

/*ADD-BEGIN-JUNGO-20101008-CTZV support */
#include <time.h>

#include <ril_callbacks.h>
#include <ril_nw.h>

/// M: EPDG feature. Query PS state from MAL
#include <mal.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#include <cutils/properties.h>


/* Global data Begin */

/* NITZ Operator Name */
static pthread_mutex_t ril_nw_nitzName_mutex[MAX_GEMINI_SIM_NUM] = RIL_NW_INIT_MUTEX;
static char ril_nw_nitz_oper_code[MAX_GEMINI_SIM_NUM][MAX_OPER_NAME_LENGTH] = RIL_NW_INIT_STRING;
static char ril_nw_nitz_oper_lname[MAX_GEMINI_SIM_NUM][MAX_OPER_NAME_LENGTH]= RIL_NW_INIT_STRING;
static char ril_nw_nitz_oper_sname[MAX_GEMINI_SIM_NUM][MAX_OPER_NAME_LENGTH]= RIL_NW_INIT_STRING;


/* Restrition State */
int ril_nw_cur_state[RIL_NW_NUM] = RIL_NW_INIT_STATE;
int ril_prev_nw_cur_state[RIL_NW_NUM] = RIL_NW_INIT_STATE;

/* GPRS network registration status */
int ril_data_reg_status[RIL_NW_NUM] = RIL_NW_INIT_INT;

/// M: EPDG feature. PS state from MAL @{
int ril_mal_data_reg_status[RIL_NW_NUM] = RIL_NW_INIT_INT;
/// @}

// ALPS00353868 START
#define PROPERTY_GSM_CURRENT_COPS_LAC    "gsm.cops.lac"
static int plmn_list_format[RIL_NW_NUM] = RIL_NW_INIT_INT;
// ALPS00353868 END
int bPSBEARERSupport = 1;
int s_md_off = 0;
int s_main_loop = 1;
int bCREGType3Support = 1;
int csgListAbort = 0;
int csgListOngoing = 0;
int bEopsSupport = 0;
int s_emergencly_only[RIL_SOCKET_NUM] = {0};
int plmnListOngoing = 0;
int plmnListAbort = 0;
int bUbinSupport = 1; //support Universal BIN(worldmode)
int bWorldModeSwitching = 0;  //[ALPS02277365]

//Add for band de-sense feature.
int bands[4] = RIL_NW_INIT_INT;
/*RIL Network Static functions */

#ifdef MTK_DTAG_DUAL_APN_SUPPORT
static const struct timeval dualApn_TIMEVAL_0 = {2,0};
#endif

extern int max_pdn_support_23G;
extern int max_pdn_support_4G;
extern void RIL_onUnsolicitedResponseSocket(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);

/// M: EPDG feature. Update PS state from MAL @{
extern int isEpdgSupport();
/// @}

// ALPS00353868 START
int setPlmnListFormat(RIL_SOCKET_ID rid, int format){
    if(rid < RIL_NW_NUM){
        /* 0: standard +COPS format , 1: standard +COPS format plus <lac> */
        plmn_list_format[rid] = format;
    }
    return 1;
}
// ALPS00353868 END

int getSingnalStrength(char *line, int *response)
{
    int err;
    int is_Lte = 0;

    //Use int max, as -1 is a valid value in signal strength
    int INVALID = 0x7FFFFFFF;


    // R8 modem
    // AT+ECSQ:<rssi>,<bar>,<rssi_qdbm>,<rscp_qdbm>,<ecn0_qdbm>
    // GSM : +ECSQ:<rssi>,<bar>,<rssi_qdbm>
    // UMTS: +ECSQ:<rssi>,<bar>,<rssi_qdbm>,<rscp_qdbm>,<ecn0_qdbm>

    // For LTE (MT6592)
    // AT+ECSQ:<sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<Act>
    // GSM : +ECSQ:<rxlev>,<bar>,<rssi_in_qdbm>,<rscp_in_qdbm>,1,1,1,<AcT:0>
    // UMTS: +ECSQ:<rscp>,<ecn0>,1,<rscp_in_qdbm>,<ecn0_in_qdbm>,1,1,<AcT:2>
    // LTE : +ECSQ:<rsrq>,<rsrp>,1,1,1,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<AcT:7>

    // for consistance with SignalStrength.makeSignalStrengthFromRilParcel(Parcel in)
    int *sig1 = &response[0];    //mGsmSignalStrength
    int *sig2 = &response[1];    //mGsmBitErrorRate
    int *rssi_in_qdbm = &response[13];    //mGsmRssiQdbm
    int *rscp_in_qdbm = &response[14];    //mGsmRscpQdbm
    int *ecn0_in_qdbm = &response[15];    //mGsmEcn0Qdbm
    int *rsrq_in_qdbm = &response[9];    //mLteRsrq --> -1*rsrq_in_qdbm /4 = mLteRsrq
    int *rsrp_in_qdbm = &response[8];    //mLteRsrp --> -1*rsrp_in_qdbm /4 = mLteRsrp
    int *rssnr_in_qdbm = &response[10];  // mLteRssnr --> -1*rssnr_in_qdbm /4 = mLteRssnr


    int *act = &response[4];    //mEvdoDbm --> for GSM this field will not use

    err = at_tok_start(&line);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, sig1);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, sig2);
    if (err < 0) return -1;

    err = at_tok_nextint(&line, rssi_in_qdbm);
    if (err < 0) return -1;

    // 2G sim has only three parameter,3G sim has five parameters
    err = at_tok_nextint(&line, rscp_in_qdbm);
    if (err < 0) {
        *rscp_in_qdbm = 0;
    }

    err = at_tok_nextint(&line, ecn0_in_qdbm);
    if (err < 0) {
        *ecn0_in_qdbm = 0;
    }

    if(at_tok_hasmore(&line)){
        // for LTE
        is_Lte = 1;

        err = at_tok_nextint(&line, rsrq_in_qdbm);
        if (err < 0) return -1;
        // for consistance with google's define of mLteRsrq
        //abs(rsrq_in_qdbm /4) = mLteRsrq
        *rsrq_in_qdbm = ((*rsrq_in_qdbm)/4)*(-1);

        err = at_tok_nextint(&line, rsrp_in_qdbm);
        if (err < 0) return -1;
        // for consistance with google's define of mLteRsrp
        //abs(rsrp_in_qdbm /4) = mLteRsrp
        *rsrp_in_qdbm = ((*rsrp_in_qdbm)/4)*(-1);

        err = at_tok_nextint(&line, act);
        if (err < 0) return -1;

        err = at_tok_nextint(&line, rssnr_in_qdbm);
        if (err < 0) {
            // for consistance with google's define of mLteRssnr
            // assign 301 to make sure mLteRssnr will be assign INVALID value
            // in SignalStrength.validateInput()
            response[10] = 301;
        } else {
            // for consistance with google's define of mLteRssnr
            // abs(rssnr_in_qdbm /4) = mLteRssnr
            *rssnr_in_qdbm = ((*rssnr_in_qdbm)/4) * 10;
        }
    }

    //set default value
    response[7] = 99;

    response[11] = INVALID;

    // check if the value is valid or not
    if (is_Lte != 1){
        // for 2/3G only modem ==> +ECSQ: <rssi>,<ber>
        if((*sig1 < 0  || *sig1 > 31) && *sig1 != 99){
            LOGE("Recevice an invalid value from modem for <rssi>");
            return -1;
        }
        if((*sig2 < 0 || *sig2 > 7) && *sig2 != 99){
            LOGE("Recevice an invalid value from modem for <ber>");
            return -1;
        }
        *rsrq_in_qdbm = INVALID;
        *rsrp_in_qdbm = INVALID;
    } else {
        if ((*act == 0) || (*act == 1) || (*act == 3)){  // for GSM: <rxlevel>,<ber>
            if((*sig1 < 0  || *sig1 > 63) || *sig1 == 99){
                LOGE("Recevice an invalid value from modem for <rxlevel>");
                return -1;
            }
            //[ALPS01264470]
            if((*sig2 < 0 || *sig2 > 7) && *sig2 != 99){
            //if((*sig2 < 0 || *sig2 > 7) || *sig2 == 99){
                LOGE("Recevice an invalid value from modem for <ber>");
                return -1;
            }

            //[ALPS01416607] dbm=<rxlev>-111, but framework mapping dbm as -113+(2*<rxlev>)
            // Ex: +ECSQ: 47,99,-256,1,1,1,1,3 --> dbm=47-111 = -64
            // so we need to set <rxlev> = (47/2)+1 = 25 --> -113+(2*25) = -63
            *sig1 = (*sig1/2)+1;

        } else if ((*act == 2) || ((*act >= 4) && (*act <= 6))){ // for UMTS:<rscp>,<ecno>
            if((*sig1 < 0  || *sig1 > 96) || *sig1 == 255){
                LOGE("Recevice an invalid value from modem for <rscp>");
                return -1;
            }
            //[ALPS01264470]
            if((*sig2 < 0 || *sig2 > 49) && *sig2 != 255){
            //if((*sig2 < 0 || *sig2 > 49) || *sig2 == 255){
                LOGE("Recevice an invalid value from modem for <ecno>");
                return -1;
            }

            int rssi = convert3GRssiValue(*rscp_in_qdbm,*ecn0_in_qdbm);

            //[ALPS01416607] dbm=<rscp>-121, but framework mapping dbm as <rscq_qdbm>/4
            // Ex: +ECSQ: 46,34,1,-297,-30,1,1,6 --> dbm=46-121 = -75
            // so we need to set <rscp_in_qdbm> = (<rscp>-121)*4 = -75*4 = -300 --> -300/4=-75
            *rscp_in_qdbm = (*sig1-121)*4;

            /*ALPS01457335: New version MTK modem report +ECSQ based on AT command spec 27.007 AT+CESQ format.
              But Android framework expect spec 27.007  AT+CSQ <rssi> 0~31 format when handling 3G signal strength. So we convert 3G signal to <rssi> in RILD */
            *sig1 = rssi;

            *rsrq_in_qdbm = INVALID;
            *rsrp_in_qdbm = INVALID;
        } else if (*act == 7){ // for LTE:<rsrq>,<rsrp>
            //[ALPS01264470]
            if((*sig1 < 0  || *sig1 > 34) && *sig1 != 255){
            //if((*sig1 < 0  || *sig1 > 34) || *sig1 == 255){
                LOGE("Recevice an invalid value from modem for <rsrq>");
                return -1;
            }

            //mLteSignalSrength --> for LTE
            //[ALPS01264470]
            //[ALPS01416607] dbm=<rsrp>-141, but framework mapping dbm directly
            // Ex: +ECSQ: 19,47,1,1,1,-42,-375,7 --> dbm=47-141 = -94
            // so we need to set <rsrp> = (<rscp>-141)
            response[7] = *sig2 - 141;
            //response[7] = *sig2;
            //response[7] = *sig1;

            if((*sig2 < 0 || *sig2 > 97) || *sig2 == 255){
                LOGE("Recevice an invalid value from modem for <rsrp>");
                return -1;
            }
        } else {
            LOGE("Recevice an invalid value from modem for <act>");
            return -1;
        }
    }

    //isGsm
    response[12] = 1;

    return 0;

}


/*  ALPS01457335 START */
/* For FDD 3G conversion */
#define CUSTOM_3G_SIGNAL_LEVEL_MINUS_1_THRESHOLD (-72)
#define CUSTOM_3G_SIGNAL_LEVEL_MINUS_2_THRESHOLD  (-88)
#define CUSTOM_3G_SIGNAL_LEVEL_MINUS_3_THRESHOLD  (-104)
#define CUSTOM_3G_SIGNAL_LEVEL_1_MAX_STRENGTH (8)
#define CUSTOM_3G_SIGNAL_LEVEL_2_MAX_STRENGTH (11)
#define CUSTOM_3G_SIGNAL_LEVEL_3_MAX_STRENGTH (14)

/* For TDD 3G conversion */
#define CUSTOM_3G_TDD_RSCP_THRESHOLD_1 (-51)
#define CUSTOM_3G_TDD_RSCP_THRESHOLD_2 (-75)
#define CUSTOM_3G_TDD_RSCP_THRESHOLD_3 (-85)
#define CUSTOM_3G_TDD_RSCP_THRESHOLD_4 (-95)
#define CUSTOM_3G_TDD_RSCP_THRESHOLD_5 (-108)
#define CUSTOM_3G_TDD_CSQ_LEVEL_1 (31)
#define CUSTOM_3G_TDD_CSQ_LEVEL_2 (15)
#define CUSTOM_3G_TDD_CSQ_LEVEL_3 (7)
#define CUSTOM_3G_TDD_CSQ_LEVEL_4 (3)
#define CUSTOM_3G_TDD_CSQ_LEVEL_5 (0)


/*ALPS01457335: New version MTK modem report +ECSQ based on AT command spec 27.007 AT+CESQ format.
  But Android framework expect spec 27.007  AT+CSQ <rssi> 0~31 format when handling 3G signal strength. So we convert 3G signal to <rssi> in RILD */
int convert3GRssiValue(int RSCP_in_qdbm,int EcN0_in_qdbm){
    char prop[PROPERTY_VALUE_MAX] = {0};
    int baseband=0;
    bool isFdd3G = false;
    bool isTdd3G = false;
    int rssi = 0;

    #ifdef MTK_RIL_MD2
    property_get(PROPERTY_GSM_BASEBAND_CAPABILITY_MD2,prop , "0");
    #else
    property_get(PROPERTY_GSM_BASEBAND_CAPABILITY,prop , "0");
    #endif

    baseband = atoi(prop);

    if(baseband & 0x04){
        isFdd3G = true;
    }else if(baseband & 0x08){
        isTdd3G = true;
    }

    LOGD("convert3GRssiValue baseband=%d ,isFdd3G=%d,isTdd3G=%d ,RSCP_in_qdbm=%d ,EcN0_in_qdbm=%d",baseband,isFdd3G,isTdd3G,RSCP_in_qdbm,EcN0_in_qdbm);

    if(isTdd3G == true){
        int RSCP_in_dbm = RSCP_in_qdbm / 4;

        if (RSCP_in_dbm >= CUSTOM_3G_TDD_RSCP_THRESHOLD_1){
            rssi = 31;
        } else if (RSCP_in_dbm >= CUSTOM_3G_TDD_RSCP_THRESHOLD_2) {
            rssi = (CUSTOM_3G_TDD_CSQ_LEVEL_2+1) +
                   (CUSTOM_3G_TDD_CSQ_LEVEL_1-CUSTOM_3G_TDD_CSQ_LEVEL_2-1) * (RSCP_in_dbm-CUSTOM_3G_TDD_RSCP_THRESHOLD_2) /
                   (CUSTOM_3G_TDD_RSCP_THRESHOLD_1-CUSTOM_3G_TDD_RSCP_THRESHOLD_2);
        } else if (RSCP_in_dbm >= CUSTOM_3G_TDD_RSCP_THRESHOLD_3) {
            rssi = (CUSTOM_3G_TDD_CSQ_LEVEL_3+1) +
                   (CUSTOM_3G_TDD_CSQ_LEVEL_2-CUSTOM_3G_TDD_CSQ_LEVEL_3-1) * (RSCP_in_dbm-CUSTOM_3G_TDD_RSCP_THRESHOLD_3) /
                   (CUSTOM_3G_TDD_RSCP_THRESHOLD_2-CUSTOM_3G_TDD_RSCP_THRESHOLD_3-1);
        } else if (RSCP_in_dbm >= CUSTOM_3G_TDD_RSCP_THRESHOLD_4){
            rssi = (CUSTOM_3G_TDD_CSQ_LEVEL_4+1) +
                   (CUSTOM_3G_TDD_CSQ_LEVEL_3-CUSTOM_3G_TDD_CSQ_LEVEL_4-1) * (RSCP_in_dbm-CUSTOM_3G_TDD_RSCP_THRESHOLD_4) /
                   (CUSTOM_3G_TDD_RSCP_THRESHOLD_3-CUSTOM_3G_TDD_RSCP_THRESHOLD_4-1);
        } else if (RSCP_in_dbm >= CUSTOM_3G_TDD_RSCP_THRESHOLD_5){
            rssi = (CUSTOM_3G_TDD_CSQ_LEVEL_5+1) +
                   (CUSTOM_3G_TDD_CSQ_LEVEL_4-CUSTOM_3G_TDD_CSQ_LEVEL_5-1) * (RSCP_in_dbm-CUSTOM_3G_TDD_RSCP_THRESHOLD_5) /
                   (CUSTOM_3G_TDD_RSCP_THRESHOLD_4-CUSTOM_3G_TDD_RSCP_THRESHOLD_5-1);
        }else{
            rssi = 0;
        }
    }else if(isFdd3G == true){
        rssi = ((EcN0_in_qdbm / 4) + 25) + (((RSCP_in_qdbm / 4) + 120) / 8);

        if (rssi > 31) {
            rssi = 31;
        }

        /* To reflect the real network environment more accurately,
        *  based on the experiment results, we modify the calculated signal level for MMI display
        *  When RSCP < -72 and the original signal strength > 9,
        *  a new formula is defined to modify the original signal strength.
        *  More detailed information are shown in the excel file for the new formula */

        if (rssi >= (CUSTOM_3G_SIGNAL_LEVEL_1_MAX_STRENGTH+1))
        {
            int RSCP_in_dbm = RSCP_in_qdbm / 4;
            if (RSCP_in_dbm < CUSTOM_3G_SIGNAL_LEVEL_MINUS_3_THRESHOLD)
            {
                rssi = CUSTOM_3G_SIGNAL_LEVEL_1_MAX_STRENGTH;
            }
            else if (RSCP_in_dbm < CUSTOM_3G_SIGNAL_LEVEL_MINUS_2_THRESHOLD)
            {
                if ((CUSTOM_3G_SIGNAL_LEVEL_3_MAX_STRENGTH) >= rssi)
                {
                    rssi=CUSTOM_3G_SIGNAL_LEVEL_1_MAX_STRENGTH;
                }
                else
                {
                    rssi=CUSTOM_3G_SIGNAL_LEVEL_2_MAX_STRENGTH;
                }
            }
            else if (RSCP_in_dbm < CUSTOM_3G_SIGNAL_LEVEL_MINUS_1_THRESHOLD)
            {
                if (CUSTOM_3G_SIGNAL_LEVEL_2_MAX_STRENGTH >= rssi)
                {
                     rssi=CUSTOM_3G_SIGNAL_LEVEL_1_MAX_STRENGTH;
                }
                else if (CUSTOM_3G_SIGNAL_LEVEL_3_MAX_STRENGTH >= rssi)
                {
                     rssi=CUSTOM_3G_SIGNAL_LEVEL_2_MAX_STRENGTH;
                }
                else
                {
                    rssi=CUSTOM_3G_SIGNAL_LEVEL_3_MAX_STRENGTH;
                }
            }
        }

    }

    return rssi;
}
/*  ALPS01457335 END */

int getOperatorNamesFromNumericCode(
    char *code,
    char *longname,
    char *shortname,
    int max_length,
    RIL_SOCKET_ID rid)
{
    char nitz[PROPERTY_VALUE_MAX];
    char oper_file_path[PROPERTY_VALUE_MAX];
    char oper[128], name[MAX_OPER_NAME_LENGTH];
    char *line, *tmp;
    FILE *list;
    int err;

    char *oper_code, *oper_lname, *oper_sname;

    if (max_length > MAX_OPER_NAME_LENGTH)
    {
        LOGE("The buffer size %d is not valid. We only accept the length less than or equal to %d",
             max_length, MAX_OPER_NAME_LENGTH);
        return -1;
    }

    oper_code = ril_nw_nitz_oper_code[rid];
    oper_lname = ril_nw_nitz_oper_lname[rid];
    oper_sname = ril_nw_nitz_oper_sname[rid];

    longname[0] = '\0';
    shortname[0] = '\0';

    pthread_mutex_lock(&ril_nw_nitzName_mutex[rid]);
    LOGD("Get ril_nw_nitzName_mutex in the getOperatorNamesFromNumericCode");

    /* Check if there is a NITZ name*/
    /* compare if the operator code is the same with <oper>*/
    if(strcmp(code, oper_code) == 0)
    {
        /* there is a NITZ Operator Name*/
        /*get operator code and name*/
        /*set short name with long name when short name is null and long name isn't, and vice versa*/
        int nlnamelen = strlen(oper_lname);
        int nsnamelen = strlen(oper_sname);
        if(nlnamelen != 0 && nsnamelen != 0)
        {
            strncpy(longname,oper_lname, max_length);
            strncpy(shortname, oper_sname, max_length);
        }
        else if(strlen(oper_sname) != 0)
        {
            strncpy(longname, oper_sname, max_length);
            strncpy(shortname, oper_sname, max_length);
        }
        else if(strlen(oper_lname) != 0)
        {
            strncpy(longname, oper_lname, max_length);
            strncpy(shortname, oper_lname, max_length);
        }

        LOGD("Return NITZ Operator Name: %s %s %s, lname length: %d, sname length: %d", oper_code,
                                                                                        oper_lname,
                                                                                        oper_sname,
                                                                                        nlnamelen,
                                                                                        nsnamelen);
    }
    else
    {
        strcpy(longname, code);
        strcpy(shortname, code);
        //getPLMNNameFromNumeric(code, longname, shortname, max_length);
    }

    pthread_mutex_unlock(&ril_nw_nitzName_mutex[rid]);
    return 0;
}

void updateNitzOperInfo(RIL_SOCKET_ID rid) {
	int phoneId = getMappingSIMByCurrentMode(rid);
	getMSimProperty(phoneId, PROPERTY_NITZ_OPER_CODE, ril_nw_nitz_oper_code[rid]);
	getMSimProperty(phoneId, PROPERTY_NITZ_OPER_LNAME, ril_nw_nitz_oper_code[rid]);
	getMSimProperty(phoneId, PROPERTY_NITZ_OPER_SNAME, ril_nw_nitz_oper_code[rid]);
    LOGD("[RIL%d] NITZ Operator Name : %s %s %s", (getMappingSIMByCurrentMode(rid)+1),
        ril_nw_nitz_oper_code[rid], ril_nw_nitz_oper_lname[rid], ril_nw_nitz_oper_sname[rid]);
}

/*RIL Network functions */

/** returns 1 if on, 0 if off, 4 for airplane mode. and -1 on error */
int queryRadioState(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;

    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    at_response_free(p_response);

    return ret;

error:
    at_response_free(p_response);
    return -1;
}


/*RIL Network functions */

/** returns 1 if on, 0 if off, and -1 on error */
int isRadioOn(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;

    err = at_send_command_singleline("AT+CFUN?", "+CFUN:", &p_response, getRILChannelCtx(RIL_SIM,getMainProtocolRid()));

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    at_response_free(p_response);

    ret = (ret == 4 || ret == 0) ? 0 :    // phone off
          (ret == 1) ? 1 :              // phone on
          -1;                           // invalid value

    return ret;

error:
    at_response_free(p_response);
    return -1;
}

void requestSignalStrength(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    //MTK-START [ALPS00506562][ALPS00516994]
    //int response[12]={0};
    int response[16] ={0};
    char *line;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT+ECSQ", "+ECSQ:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0 ||
            p_response->p_intermediates  == NULL) {
        //RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0); //mtk02514
        goto error;
    }

    line = p_response->p_intermediates->line;
    //err = getSingnalStrength(line, &response[0], &response[1], &response[2], &response[3], &response[4]);
    err = getSingnalStrength(line, response);

    if (err < 0) goto error;

    // 2G sim has only three parameter,3G sim has five parameters
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));

    at_response_free(p_response);
    return;

error:
    LOGE("requestSignalStrength must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static unsigned int convertNetworkType(unsigned int uiResponse)
{
    unsigned int uiRet = 0;

    /* mapping */
    switch(uiResponse)
    {
    case 0:     //GSM
    case 1:     //GSM compact
        uiRet = 1;        // GPRS only
        break;
    case 2:     //UTRAN
        uiRet = 3;        // UMTS
        break;
    case 3:     //GSM w/EGPRS
        uiRet = 2;        // EDGE
        break;
    case 4:     //UTRAN w/HSDPA
        uiRet = 9;        // HSDPA
        break;
    case 5:     //UTRAN w/HSUPA
        uiRet = 10;        // HSUPA
        break;
    case 6:     //UTRAN w/HSDPA and HSUPA
        uiRet = 11;        // HSPA
        break;
    //for LTE
    case 7:     //E-UTRAN
        uiRet = 14;        // LTE
        break;
    default:
        uiRet = 0;        // Unknown
        break;
    }

    return uiRet;
}

static unsigned int convertCellSppedSupport(unsigned int uiResponse)
{
    // Cell speed support is bitwise value of cell capability:
    // bit7 0x80  bit6 0x40  bit5 0x20  bit4 0x10  bit3 0x08  bit2 0x04  bit1 0x02  bit0 0x01
    // Dual-Cell  HSUPA+     HSDPA+     HSUPA      HSDPA      UMTS       EDGE       GPRS
    unsigned int uiRet = 0;

    unsigned int RIL_RADIO_TECHNOLOGY_MTK = 128;

    if ((uiResponse & 0x2000) != 0) {
        //support carrier aggregation (LTEA)
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+11; // ServiceState.RIL_RADIO_TECHNOLOGY_LTEA
    } else if ((uiResponse & 0x1000) != 0) {
        uiRet = 14; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE
    } else if ((uiResponse & 0x80) != 0 ||
            (uiResponse & 0x40) != 0 ||
            (uiResponse & 0x20) != 0) {
        uiRet = 15; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
    } else if ((uiResponse & 0x10) != 0 &&
            (uiResponse & 0x08) != 0) {
        uiRet = 11; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
    } else if ((uiResponse & 0x10) != 0) {
        uiRet = 10; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
    } else if ((uiResponse & 0x08) != 0) {
        uiRet = 9;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
    } else if ((uiResponse & 0x04) != 0) {
        uiRet = 3;  // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
    } else if ((uiResponse & 0x02) != 0) {
        uiRet = 2;  // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
    } else if ((uiResponse & 0x01) != 0) {
        uiRet = 1;  // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
    } else {
        uiRet = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
    }

    return uiRet;
}

static unsigned int convertPSBearerCapability(unsigned int uiResponse)
{
    /*
     *typedef enum
     *{
  *    L4C_NONE_ACTIVATE = 0,
  *    L4C_GPRS_CAPABILITY,
  *    L4C_EDGE_CAPABILITY,
  *    L4C_UMTS_CAPABILITY,
  *    L4C_HSDPA_CAPABILITY, //mac-hs
  *    L4C_HSUPA_CAPABILITY, //mac-e/es
  *    L4C_HSDPA_HSUPA_CAPABILITY, //mac-hs + mac-e/es

  *    L4C_HSDPAP_CAPABILITY, //mac-ehs
  *    L4C_HSDPAP_UPA_CAPABILITY, //mac-ehs + mac-e/es
  *    L4C_HSUPAP_CAPABILITY, //mac-i/is
  *    L4C_HSUPAP_DPA_CAPABILITY, //mac-hs + mac-i/is
  *    L4C_HSPAP_CAPABILITY, //mac-ehs + mac-i/is
  *    L4C_DC_DPA_CAPABILITY, //(DC) mac-hs
  *    L4C_DC_DPA_UPA_CAPABILITY, //(DC) mac-hs + mac-e/es
  *    L4C_DC_HSDPAP_CAPABILITY, //(DC) mac-ehs
  *    L4C_DC_HSDPAP_UPA_CAPABILITY, //(DC) mac-ehs + mac-e/es
  *    L4C_DC_HSUPAP_DPA_CAPABILITY, //(DC) mac-hs + mac-i/is
  *    L4C_DC_HSPAP_CAPABILITY, //(DC) mac-ehs + mac-i/is
  *    L4C_LTE_CAPABILITY
     *    } l4c_data_bearer_capablility_enum;
    */

    unsigned int uiRet = 0;

    unsigned int RIL_RADIO_TECHNOLOGY_MTK = 128;

    switch (uiResponse)
    {
    case 1:
        uiRet = 1;  // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
        break;
    case 2:
        uiRet = 2;  // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
        break;
    case 3:
        uiRet = 3;  // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
        break;
    case 4:
        uiRet = 9;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
        break;
    case 5:
        uiRet = 10; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
        break;
    case 6:
        uiRet = 11; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
        break;
    case 7:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+1; // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPAP
        break;
    case 8:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+2; // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPAP_UPA
        break;
    case 9:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+3; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPAP
        break;
    case 10:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+4; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPAP_DPA
        break;
    case 11:
        uiRet = 15; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
        break;
    case 12:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+5; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_DPA
        break;
    case 13:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+6; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_UPA
        break;
    case 14:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+7; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP
        break;
    case 15:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+8; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP_UPA
        break;
    case 16:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+9; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP_DPA
        break;
    case 17:
        uiRet = RIL_RADIO_TECHNOLOGY_MTK+10; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSPAP
        break;
    case 18:
        uiRet = 14; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE
        break;
    default:
        uiRet = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
        break;
    }

    return uiRet;
}


static unsigned int convertRegState(unsigned int uiRegState)
{
    unsigned int uiRet = 0;

    switch (uiRegState)
    {
    case 6:         // Registered for "SMS only", home network
        uiRet = 1;  // Registered
        break;
    case 7:         // Registered for "SMS only", roaming
        uiRet = 5;  // roaming
        break;
    case 8:         // attached for emergency bearer service only
        uiRet = 0;  // not registered
        break;
    case 9:         // registered for "CSFB not prefereed", home network
        uiRet = 1;  // Registered
        break;
    case 10:       // registered for "CSFB not prefereed", roaming
        uiRet = 5;  // roaming
        break;
    default:
        uiRet = uiRegState;
        break;
    }

    LOGD("convertRegState() uiRegState=%d , uiRet=%d ",uiRegState,uiRet);
    return uiRet;
}

static unsigned int convertToRilRegState(unsigned int uiRegState,RIL_SOCKET_ID rid)
{
    unsigned int uiRet = 0;

    /* RIL defined RIL_REQUEST_VOICE_REGISTRATION_STATE response state 0-6,
         *              0 - Not registered, MT is not currently searching
         *                  a new operator to register
         *              1 - Registered, home network
         *              2 - Not registered, but MT is currently searching
         *                  a new operator to register
         *              3 - Registration denied
         *              4 - Unknown
         *              5 - Registered, roaming
         *             10 - Same as 0, but indicates that emergency calls
         *                  are enabled.
         *             12 - Same as 2, but indicates that emergency calls
         *                  are enabled.
         *             13 - Same as 3, but indicates that emergency calls
         *                  are enabled.
         *             14 - Same as 4, but indicates that emergency calls
         *                  are enabled.
         */
    if (s_emergencly_only[rid] == 1) {
        switch (uiRegState)
        {
        case 0:
            uiRet = 10;
            break;
        case 2:
            uiRet = 12;
            break;
        case 3:
            uiRet = 13;
            break;
        case 4:
            uiRet = 14;
            break;
        default:
            uiRet = uiRegState;
            break;
        }
    } else {
        uiRet = uiRegState;
    }
    LOGD("convertToRilRegState() uiRegState=%d , uiRet=%d ",uiRegState,uiRet);
    return uiRet;
}


static void registrationStateRequest(int request, RIL_Token t)
{
    int err;
    unsigned int response[14];
    int i = 0;
    char * responseStr[14];
    ATResponse *p_response = NULL;
    const char *cmd;
    const char *prefix;
    char *line, *p;
    int commas;
    int skip;
    int count = 3;
    int is_cs = 0;
    int cause_type;

    //For LTE
    int dontChangeFormat=1;

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    if (request == RIL_REQUEST_VOICE_REGISTRATION_STATE) {
        cmd = "AT+CREG?";
        prefix = "+CREG:";
        is_cs = 1;
    } else if (request == RIL_REQUEST_DATA_REGISTRATION_STATE) {
        cmd = "AT+CGREG?";
        prefix = "+CGREG:";
        is_cs = 0;
    } else {
        assert(0);
        goto error;
    }

    err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* Ok you have to be careful here
     * The solicited version of the CREG response is
     * +CREG: n, stat, [lac, cid]
     * and the unsolicited version is
     * +CREG: stat, [lac, cid]
     * The <n> parameter is basically "is unsolicited creg on?"
     * which it should always be
     *
     * Now we should normally get the solicited version here,
     * but the unsolicited version could have snuck in
     * so we have to handle both
     *
     * Also since the LAC and CID are only reported when registered,
     * we can have 1, 2, 3, or 4 arguments here
     *
     * finally, a +CGREG: answer may have a fifth value that corresponds
     * to the network type, as in;
     *
     *   +CGREG: n, stat [,lac, cid [,networkType]]
     */

    /***********************************
    * In the mtk solution, we do not need to handle the URC
    * Since the URC will received in the URC channel.
    * So we don't need to follow the above command from google.
    * But we might return <AcT> for +CREG if the stat==2
    * while <lac> is present
    ************************************/

    /* +CREG: <n>, <stat> */
    /* +CREG: <n>, <stat>, <lac>, <cid>, <Act> */
    /* +CGREG: <n>, <stat>, <lac>, <cid>, <Act> */

    /* <n> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) &response[0]);

    //if (err < 0 || response[0] > 5 )
    if (err < 0 || response[0] > 10 )  //for LTE
    {
        LOGE("The value in the field <stat> is not valid: %d", response[0] );
        goto error;
    }

    //For Lte
    response[0] = convertRegState(response[0] );

    // change to short GREG when <n> =3 and not registed
    if ((skip == 3) && (response[0] != 1 && response[0] != 5))
    {
        dontChangeFormat=0;
        LOGD("dontChangeFormat=0");
    }

    //if( at_tok_hasmore(&line) )
    if (( at_tok_hasmore(&line) ) && (dontChangeFormat) )
    {
        /* <lac> */
        LOGE("The value in the field <lac> :%d", response[1]);
        err = at_tok_nexthexint(&line, (int*)&response[1]);
        if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff) )
        {
            LOGE("The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
                 response[0], response[1] );
            goto error;
        }

        /* <cid> */
        err = at_tok_nexthexint(&line, (int*)&response[2]);
        LOGD("cid: %d", response[2] );
        if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff) )
        {
            LOGE("The value in the field <cid> is not valid: %d", response[2] );
            goto error;
        }

        //if (request == RIL_REQUEST_GPRS_REGISTRATION_STATE)
        {
            /* <Act> */
            err = at_tok_nextint(&line, (int*)&response[3]);
            LOGE("The value of act: %d", response[3] );
            if (err < 0)
            {
                LOGE("No act in command");
                goto error;
            }
            count = 4;
//            if (response[3] > 6)
            if (response[3] > 7)  //for LTE
            {
                LOGE("The value in the act is not valid: %d", response[3] );
                goto error;
            }

            /* mapping */
            response[3] = convertNetworkType(response[3]);
        }

        if (at_tok_hasmore(&line))
        {
            /* <cause_type> */
            err = at_tok_nextint(&line, &cause_type);
            LOGD("cause_type: %d", cause_type);
            if (err < 0 || cause_type != 0)
            {
                LOGE("The value in the field <cause_type> is not valid: %d", cause_type );
                goto error;
            }

            /* <reject_cause> */
            err = at_tok_nextint(&line, (int*) &response[13]);
            LOGD("reject_cause: %d", response[13]);
            if (err < 0)
            {
                LOGE("The value in the field <reject_cause> is not valid: %d", response[13] );
                goto error;
            }
            count = 14;
        }
    }
    else
    {
        /* +CREG: <n>, <stat> */
        /* +CGREG: <n>, <stat> */
        LOGE("it is short CREG CGREG");
        response[1] = -1;
        response[2] = -1;
        response[3] = 0;
        //BEGIN mtk03923 [20120119][ALPS00112664]
        count = 4;
        //END   mtk03923 [20120119][ALPS00112664]
    }

    //[ALPS01763285]-remark for assign new value after convert state
    //asprintf(&responseStr[0], "%d", response[0]);
    if (response[1] != 0xffffffff)
        asprintf(&responseStr[1], "%x", response[1]);
    else
        asprintf(&responseStr[1], "-1");
    if (response[2] != 0xffffffff)
        asprintf(&responseStr[2], "%x", response[2]);
    else
        asprintf(&responseStr[2], "-1");

    if (count >= 4)
    {
        asprintf(&responseStr[3], "%d", response[3]);
        //count = 4;
    }
    if (count == 14)
    {
        // set "" to indicate "the field is not present"
        for (i = 4; i < 13; i++)
        {
            asprintf(&responseStr[i], "");
        }
        asprintf(&responseStr[13], "%d", response[13]);
    }
    LOGE("rid: %d",rid);
    if (response[0] == 1 || response[0] == 5)
    {
        //LOGE("!!!!in service");
        if (is_cs)
        {
            //    LOGE("!!!!IN1 service");
            cleanCurrentRestrictionState(RIL_NW_ALL_CS_RESTRICTIONS, rid);
        }
        else
        {
            //    LOGE("!!!!IN2 service");
            cleanCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
        }
    }
    else
    {
        // LOGE("!!!!out service");
        if (is_cs)
        {
            //   LOGE("!!!!out1 service");
            setCurrentRestrictionState(RIL_RESTRICTED_STATE_CS_NORMAL, rid);
        }
        else
        {
            //   LOGE("!!!!ou2t service");
            setCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
        }
    }
    onRestrictedStateChanged(rid);
    //[ALPS01763285] -START : fix ECC button display abnormal
    if (is_cs) {
        response[0] = convertToRilRegState(response[0],rid);
    }
    asprintf(&responseStr[0], "%d", response[0]);
    //[ALPS01763285] -END

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, count*sizeof(char*));

    at_response_free(p_response);
    free(responseStr[0]);
    free(responseStr[1]);
    free(responseStr[2]);
    if(count >= 4)
        free(responseStr[3]);
    if (count == 14) {
        for (i = 4; i < 14; i++)
        {
            free(responseStr[i]);
        }
    }

    return;

error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    registrationStateRequest(RIL_REQUEST_VOICE_REGISTRATION_STATE, t);
}

void requestGprsRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    /*
     * A +CGREG: answer may have a fifth value that corresponds
     * to the network type, as in;
     *
     * +CGREG: n, stat [,lac, cid [,networkType]]
     */

    /***********************************
    * In the mtk solution, we do not need to handle the URC
    * Since the URC will received in the URC channel.
    * So we don't need to follow the above command from google.
    * But we might return <AcT> for +CREG if the stat==2
    * while <lac> is present
    ************************************/

    /* +CGREG: <n>, <stat>, <lac>, <cid>, <Act> */

    int err;
    unsigned int response[6];
    char * responseStr[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    ATResponse *p_response = NULL;
    const char *cmd;
    const char *prefix;
    char *line, *p;
    int commas;
    int skip, i;
    int count = 3;
    int psBearerCount = 0;
    int cell_data_speed = 0;
    int max_data_bearer = 0;
    int cause_type;

    // Don't change cgreg format
    int dontChangeCgregFormat = 1;

    //support carrier aggregation (LTEA)
    int ignoreMaxDataBearerCapability = 0;

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    cmd = "AT+CGREG?";
    prefix = "+CGREG:";

    err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <n> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) &response[0]);
    //if (err < 0 || response[0] > 5 )
    if (err < 0 || response[0] > 10 )  //for Lte
    {
        LOGE("The value in the field <stat> is not valid: %d", response[0] );
        goto error;
    }

    //For Lte
    response[0] = convertRegState(response[0] );

    // change to short GREG when <n> =3 and not registed
    if ((skip == 3) && (response[0] != 1 && response[0] != 5)) {
        dontChangeCgregFormat = 0;
        LOGD("dontChangeCgregFormat=0");
    }

    // if( at_tok_hasmore(&line) )
    if (at_tok_hasmore(&line) && dontChangeCgregFormat)
    {
        /* <lac> */
        LOGE("The value in the field <lac> :%d", response[1]);
        err = at_tok_nexthexint(&line, (int*)&response[1]);
        if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff) )
        {
            LOGE("The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
                 response[0], response[1] );
            goto error;
        }

        /* <cid> */
        err = at_tok_nexthexint(&line, (int*)&response[2]);
        LOGD("cid: %d", response[2] );
        if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff) )
        {
            LOGE("The value in the field <cid> is not valid: %d", response[2] );
            goto error;
        }

        /* <Act> */
        err = at_tok_nextint(&line, (int*)&response[3]);
        LOGE("The value of act: %d", response[3] );
        if (err < 0)
        {
            LOGE("No act in command");
            goto error;
        }
        count = 4;
        if (response[3] > 7)
        {
            LOGE("The value in the act is not valid: %d", response[3] );
            goto error;
        }

        /* mapping */
        response[3] = convertNetworkType(response[3]);

        if (at_tok_hasmore(&line))
        {
            /* <rac> */
            err = at_tok_nexthexint(&line, &skip);
            if (err < 0) goto error;

            if (at_tok_hasmore(&line))
            {
                /* <cause_type> */
                err = at_tok_nextint(&line, &cause_type);
                LOGD("cause_type: %d", cause_type);
                if (err < 0 || cause_type != 0)
                {
                    LOGE("The value in the field <cause_type> is not valid: %d", cause_type );
                    goto error;
                }

                /* <reject_cause> */
                err = at_tok_nextint(&line, (int*) &response[4]);
                LOGD("reject_cause: %d", response[4]);
                if (err < 0)
                {
                    LOGE("The value in the field <reject_cause> is not valid: %d", response[4] );
                    goto error;
                }
            }
        }

        /* maximum number of simultaneous Data Calls  */
        if (response[3] == 14)
            response[5] = max_pdn_support_4G;
        else
            response[5] = max_pdn_support_23G;
        count = 6;
    }
    else
    {
        /* +CGREG: <n>, <stat> */
    LOGE("it is short CREG CGREG");
        response[1] = -1;
        response[2] = -1;
        response[3] = 0;
        //BEGIN mtk03923 [20120119][ALPS00112664]
        count = 4;
        //END   mtk03923 [20120119][ALPS00112664]
    }

    // Query by +PSBEARER as PS registered and R7R8 support
    if (bPSBEARERSupport == 1 &&
            (response[0] == 1 || response[0] == 5)) {
        at_response_free(p_response);
        p_response = NULL;
        cmd = "AT+PSBEARER?";
        prefix = "+PSBEARER:";

        err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

        if (err != 0 || p_response->success == 0 ||
                p_response->p_intermediates == NULL) goto skipR7R8;

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto skipR7R8;

        /* <cell_data_speed_support> */
        err = at_tok_nextint(&line, &cell_data_speed);
        if (err < 0) goto skipR7R8;
        psBearerCount++;

        // <max_data_bearer_capability> is only support on 3G
        if (cell_data_speed >= 0x1000){
            ignoreMaxDataBearerCapability = 1;
        }

        cell_data_speed = convertCellSppedSupport(cell_data_speed);

        /* <max_data_bearer_capability> */
        err = at_tok_nextint(&line, &max_data_bearer);
        if (err < 0) goto skipR7R8;
        psBearerCount++;

        if (!ignoreMaxDataBearerCapability) {
            max_data_bearer = convertPSBearerCapability(max_data_bearer);
        } else {
            max_data_bearer = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
        }
    }


skipR7R8:

    asprintf(&responseStr[0], "%d", response[0]);
    if (response[1] != 0xffffffff)
        asprintf(&responseStr[1], "%x", response[1]);
    else
        asprintf(&responseStr[1], "-1");
    if (response[2] != 0xffffffff)
        asprintf(&responseStr[2], "%x", response[2]);
    else
        asprintf(&responseStr[2], "-1");

    if (psBearerCount == 2) {
        asprintf(&responseStr[3], "%d", (cell_data_speed > max_data_bearer)? cell_data_speed : max_data_bearer);
    } else {
        asprintf(&responseStr[3], "%d", response[3]);
    }

    if (count >= 5) {
        asprintf(&responseStr[4], "%d", response[4]);
    }

    if (count == 6) {
        asprintf(&responseStr[5], "%d", response[5]);
    }

    LOGE("rid: %d",rid);
    if (response[0] == 1 || response[0] == 5) {
        cleanCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    } else {
        setCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    }
    onRestrictedStateChanged(rid);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, count*sizeof(char*));

    at_response_free(p_response);
    for (i=0; i<count; ++i) {
        if (responseStr[i] != NULL)
        free(responseStr[i]);
    }

    return;

error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestOperator(void * data, size_t datalen, RIL_Token t)
{
    int err;
    int skip;
    char *response[3];
    char *line;
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    ATResponse *p_response = NULL;

    memset(response, 0, sizeof(response));

    /* ALPS00574862 Remove redundant +COPS=3,2;+COPS? multiple cmd. set format in  ril initalization */
    err = at_send_command_singleline(
              "AT+COPS?",
              "+COPS:", &p_response, NW_CHANNEL_CTX);

    /* we expect 1 lines here:
     * +COPS: 0,2,"310170"
     */

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 || skip > 4)
    {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    }
    else if(skip == 2)  // deregister
    {
        LOGW("The <mode> is 2 so we ignore the follwoing fields!!!");
    }
    else if (!at_tok_hasmore(&line))
    {
        // If we're unregistered, we may just get
        // a "+COPS: 0" response

        /* To support get PLMN when not registered to network via AT+EOPS?  START */
        if(bEopsSupport ==1){
            at_response_free(p_response);
            p_response = NULL;

            err = at_send_command_singleline("AT+EOPS?",
                  "+EOPS:", &p_response, NW_CHANNEL_CTX);

            if (err != 0 || p_response->success == 0 ||
                p_response->p_intermediates == NULL){
                LOGE("EOPS got error response");
            }else{
                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err >= 0){
                    err = at_tok_nextint(&line, &skip);
                    /* <mode> */
                    if ((err >= 0) &&( skip >= 0 && skip <= 4 && skip !=2)){
                        /* <format> */
                        err = at_tok_nextint(&line, &skip);
                        if (err >= 0 && skip == 2)
                        {
                            // a "+EOPS: 0, n" response is also possible
                            if (!at_tok_hasmore(&line)) {
                                ;
                            } else{
                                /* <oper> */
                                err = at_tok_nextstr(&line, &(response[0]));
                                LOGD("EOPS Get operator code %s", response[0]);

                                /* Modem might response invalid PLMN ex: "", "000000" , "??????", all convert to "000000" */
                                if(!((*response[0] >= '0') && (*response[0] <= '9'))){
                                    LOGE("EOPS got invalid plmn response");
                                    sprintf(longname,"000000");
                                    response[0] = longname;
                                }

                                RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
                                at_response_free(p_response);
                                return;
                            }
                        }
                    }
                }
            }
        }
        /* To support get PLMN when not registered to network via AT+EOPS?  END */

    }
    else
    {
        /* <format> */
        err = at_tok_nextint(&line, &skip);
        if (err < 0 || skip != 2)
        {
            LOGW("The <format> is incorrect: expect 2, receive %d", skip);
            goto error;
        }

        // a "+COPS: 0, n" response is also possible
        if (!at_tok_hasmore(&line)) {
            ;
        }
        else
        {
            /* <oper> */
            err = at_tok_nextstr(&line, &(response[2]));
            if (err < 0) goto error;

            LOGD("Get operator code %s", response[2]);

            err = getOperatorNamesFromNumericCode(
                      response[2], longname, shortname, MAX_OPER_NAME_LENGTH, rid);

            if(err>=0)
            {
                response[0] = longname;
                response[1] = shortname;
            }

        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    LOGE("requestOperator must not return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}


void requestQueryNetworkSelectionMode(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line;

    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if ( err < 0 || ( response < 0 || response > 4|| response == 3) )
    {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("requestQueryNetworkSelectionMode must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestSetNetworkSelectionAutomatic(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno;

    err = at_send_command("AT+COPS=0", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
    {
        ril_errno = RIL_E_GENERIC_FAILURE;
    }
    else
    {
        ril_errno = RIL_E_SUCCESS;
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSetNetworkSelectionManual(void * data, size_t datalen, RIL_Token t)
{
    int err;
    char * cmd, *numeric_code;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int len, i;

    numeric_code = (char *)data;
    len = strlen(numeric_code);
    if (datalen == sizeof (char*) &&
            (len == 5 || len == 6)  )
    {
        // check if the numeric code is valid digit or not
        for(i = 0; i < len ; i++)
        {
            if( numeric_code[i] < '0' || numeric_code[i] > '9')
                break;
        }

        if( i == len)
        {

            err = asprintf(&cmd, "AT+COPS=1, 2, \"%s\"", numeric_code);
            if(err >= 0)
            {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

                free(cmd);

                if ( !(err < 0 || p_response->success == 0) )
                {
                    ril_errno = RIL_E_SUCCESS;
                }
            }
        }
        else
        {
            LOGE("the numeric code contains invalid digits");
        }
    }
    else
    {
        LOGE("the data length is invalid for Manual Selection");
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestQueryAvailableNetworksWithAct(void * data, size_t datalen, RIL_Token t)
{
    int err, len, i, j, num;
    ATResponse *p_response = NULL;
    char *line;
    char **response = NULL;
    char *tmp, *block_p = NULL;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    char *lacStr = NULL;

    LOGD("requestQueryAvailableNetworksWithAct set plmnListOngoing flag");
    plmnListOngoing = 1;
    err = at_send_command_singleline("AT+COPS=?", "+COPS:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }

    line = p_response->p_intermediates->line;

    // count the number of operator
    len = strlen(line);
    for(i = 0, num = 0; i < len ; i++ )
    {
        // here we assume that there is no nested ()
        if (line[i] == '(')
        {
            num++;
        }
        else if (line[i] == ',' && line[i+1] == ',')
        {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)

    err = at_tok_start(&line);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * num *5); // for string, each one is 25 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*5*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    for(i = 0, j=0 ; i < num ; i++, j+=5)
    {
        /* get "(<stat>" */
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];
        response[j+4] = &block_p[(j+4)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1])
        {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            LOGE("The %d-th <stat> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }

        /* skip long name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        /* skip short name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        /* get <oper> numeric code*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        len = strlen(response[j+2]);
        if (len == 5 || len == 6)
        {
            err = getOperatorNamesFromNumericCode(
                      response[j+2], response[j+0], response[j+1], MAX_OPER_NAME_LENGTH, rid);
            if(err < 0) goto error;
        }
        else
        {
            LOGE("The length of the numeric code is incorrect");
            goto error;
        }

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if(plmn_list_format[rid] == 1){
            /* get <lac> numeric code*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0){
                LOGE("No <lac> in +COPS response");
				goto error;
            }
            memcpy(&(lacStr[i*4]),tmp,4);
        }
        // ALPS00353868 END


        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        switch(tmp[0])
        {
        case '0':
            sprintf(response[j+4], "2G");
            break;
        case '2':
            sprintf(response[j+4], "3G");
            break;
        case '7':    //for  LTE
            sprintf(response[j+4], "4G");
            break;
        default:
            LOGE("The %d-th <Act> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }
    }

    // ALPS00353868 START : save <lac1><lac2><lac3>.. in the property
    if(plmn_list_format[rid] == 1){
        LOGD("Set lacStr %s to property",lacStr);
        property_set(PROPERTY_GSM_CURRENT_COPS_LAC, lacStr);
    }
    // ALPS00353868 END

    LOGD("requestQueryAvailableNetworksWithAct sucess, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char*)*num*5);
    at_response_free(p_response);
    free(response);
    free(block_p);
    free(lacStr);
    return;
error:
    at_response_free(p_response);
    if(response)
    {
        LOGD("FREE!!");
        free(block_p);
        free(response);
        free(lacStr);
    }
    LOGE("requestQueryAvailableNetworksWithAct must never return error when radio is on, plmnListAbort=%d",plmnListAbort);
    if (plmnListAbort == 1){
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    LOGD("requestQueryAvailableNetworksWithAct fail, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */
}


void requestQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t)
{
    int err, len, i, j, k, num, num_filter;
    ATResponse *p_response = NULL;
    char *line;
    char **response = NULL, **response_filter = NULL;
    char *tmp, *block_p = NULL;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    char *lacStr = NULL;

    LOGD("requestQueryAvailableNetworks set plmnListOngoing flag");
    plmnListOngoing = 1;
    err = at_send_command_singleline("AT+COPS=?", "+COPS:",&p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
    {
        goto error;
    }

    line = p_response->p_intermediates->line;

    // count the number of operator
    len = strlen(line);
    for(i = 0, num = 0, num_filter = 0; i < len ; i++ )
    {
        // here we assume that there is no nested ()
        if (line[i] == '(')
        {
            num++;
            num_filter++;
        }
        else if (line[i] == ',' && line[i+1] == ',')
        {
            break;
        }
    }

    // +COPS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)

    err = at_tok_start(&line);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * num *4); // for string, each one is 20 bytes
    block_p = (char *) calloc(1, num* sizeof(char)*4*MAX_OPER_NAME_LENGTH);
    lacStr = (char *) calloc(1, num* sizeof(char)*4+1);

    for(i = 0, j=0 ; i < num ; i++, j+=4)
    {
        /* get "(<stat>" */
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        response[j+0] = &block_p[(j+0)*MAX_OPER_NAME_LENGTH];
        response[j+1] = &block_p[(j+1)*MAX_OPER_NAME_LENGTH];
        response[j+2] = &block_p[(j+2)*MAX_OPER_NAME_LENGTH];
        response[j+3] = &block_p[(j+3)*MAX_OPER_NAME_LENGTH];

        switch(tmp[1])
        {
        case '0':
            sprintf(response[j+3], "unknown");
            break;
        case '1':
            sprintf(response[j+3], "available");
            break;
        case '2':
            sprintf(response[j+3], "current");
            break;
        case '3':
            sprintf(response[j+3], "forbidden");
            break;
        default:
            LOGE("The %d-th <stat> is an invalid value!!!  : %d", i, tmp[1]);
            goto error;
        }

        /* get long name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+0], "%s", tmp);

        /* get short name*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+1], "%s", tmp);

        /* get <oper> numeric code*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;
        sprintf(response[j+2], "%s", tmp);

        len = strlen(response[j+2]);
        if (len == 5 || len == 6)
        {
            if (0 == strcmp(response[j+2], ril_nw_nitz_oper_code[rid]))
            {
            err = getOperatorNamesFromNumericCode(
                      response[j+2], response[j+0], response[j+1], MAX_OPER_NAME_LENGTH, rid);
            if(err < 0) goto error;
        }
        }
        else
        {
            LOGE("The length of the numeric code is incorrect");
            goto error;
        }

        // ALPS00353868 START
        /*plmn_list_format.  0: standard +COPS format , 1: standard +COPS format plus <lac> */
        if(plmn_list_format[rid] == 1){
            /* get <lac> numeric code*/
            err = at_tok_nextstr(&line, &tmp);
            if (err < 0){
                LOGE("No <lac> in +COPS response");
                goto error;
            }
            memcpy(&(lacStr[i*4]),tmp,4);
        }
        // ALPS00353868 END


        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        // check if this plmn is redundant
        for (k=0; k < j; k+=4)
        {
            // compare numeric
            if (0 == strcmp(response[j+2], response[k+2]))
            {
                response[j+0] = response[j+1] = response[j+2] = response[j+3] = "";
                num_filter--;
                break;
            }
        }
    }

    // filter the response
    response_filter = (char**)calloc(1, sizeof(char*) * num_filter * 4);
    if (NULL == response_filter) {
        LOGE("malloc response_filter failed");
        goto error;
    }
    for (i=0, j=0, k=0; i < num; i++, j+=4)
    {
        if (0 < strlen(response[j+2]))
        {
            response_filter[k+0] = response[j+0];
            response_filter[k+1] = response[j+1];
            response_filter[k+2] = response[j+2];
            response_filter[k+3] = response[j+3];
            k += 4;
        }
    }

    LOGD("requestQueryAvailableNetworks sucess, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response_filter, sizeof(char*)*num_filter*4);
    at_response_free(p_response);
    free(response);
    free(response_filter);
    free(block_p);
    free(lacStr);
    return;
error:
    at_response_free(p_response);
    if(response)
    {
        LOGD("FREE!!");
        free(block_p);
        free(response);
        free(lacStr);
    }
    LOGD("requestQueryAvailableNetworks fail, clear plmnListOngoing and plmnListAbort flag");
    plmnListOngoing = 0;
    plmnListAbort =0; /* always clear here to prevent race condition scenario */
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestSetBandMode(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int req, err, gsm_band, umts_band;
    unsigned int lte_band_1_32, lte_band_33_64;
    char *cmd;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    req = ((int *)data)[0];

    switch(req) {
        case BM_AUTO_MODE: //"unspecified" (selected by baseband automatically)
            gsm_band = 0xff;
            umts_band = 0xffff;
            break;
        case BM_EURO_MODE: //"EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I;
            break;
        case BM_US_MODE: //"US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
            gsm_band = GSM_BAND_850 | GSM_BAND_1900;
            umts_band = UMTS_BAND_II | UMTS_BAND_V;
            break;
        case BM_JPN_MODE: //"JPN band" (WCDMA-800 / WCDMA-IMT-2000)
            gsm_band = 0;
            umts_band = UMTS_BAND_I | UMTS_BAND_VI;
            break;
        case BM_AUS_MODE: //"AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I | UMTS_BAND_V;
            break;
        case BM_AUS2_MODE: //"AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_V;
            break;
        case BM_40_BROKEN:
        case BM_CELLULAR_MODE: //"Cellular (800-MHz Band)"
        case BM_PCS_MODE: //"PCS (1900-MHz Band)"
        case BM_CLASS_3: //"Band Class 3 (JTACS Band)"
        case BM_CLASS_4: //"Band Class 4 (Korean PCS Band)"
        case BM_CLASS_5: //"Band Class 5 (450-MHz Band)"
        case BM_CLASS_6: // "Band Class 6 (2-GMHz IMT2000 Band)"
        case BM_CLASS_7: //"Band Class 7 (Upper 700-MHz Band)"
        case BM_CLASS_8: //"Band Class 8 (1800-MHz Band)"
        case BM_CLASS_9: //"Band Class 9 (900-MHz Band)"
        case BM_CLASS_10: //"Band Class 10 (Secondary 800-MHz Band)"
        case BM_CLASS_11: //"Band Class 11 (400-MHz European PAMR Band)"
        case BM_CLASS_15: //"Band Class 15 (AWS Band)"
        case BM_CLASS_16: //"Band Class 16 (US 2.5-GHz Band)"
        default:
            gsm_band = -1;
            umts_band = -1;
            break;
    }

    if (gsm_band != -1 && umts_band != -1)
    {
        /******************************************************
        * If the modem doesn't support certain group of bands, ex. GSM or UMTS
        * It might just ignore the parameter.
        *******************************************************/
        err = asprintf(&cmd, "AT+EPBSE=%d, %d", gsm_band, umts_band);
        if(err >= 0)
        {
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

            free(cmd);

            if(err >= 0 && p_response->success != 0)
            {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    } else if (req == BM_40_BROKEN) {
        lte_band_1_32 = (unsigned int) (((int *) data)[1]);
        lte_band_33_64 = (unsigned int) (((int *) data)[2]);
        err = asprintf(&cmd, "AT+EPBSE=,,%u,%u", lte_band_1_32, lte_band_33_64);
        LOGD("cmd:%s", cmd);
        if(err >= 0) {
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
            LOGD("B40 broken. err:%d", err);
            free(cmd);

            if(err >= 0 && p_response->success != 0)
            {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    } else if (req == BM_FOR_DESENSE_RADIO_ON || req == BM_FOR_DESENSE_RADIO_OFF
            || req == BM_FOR_DESENSE_RADIO_ON_ROAMING || req == BM_FOR_DESENSE_B8_OPEN) {
        int c2k_radio_on;
        int need_config_umts = 0;
        int force_switch = ((int *)data)[1];
        gsm_band = bands[0];
        umts_band = bands[1];
        lte_band_1_32 = bands[2];
        lte_band_33_64 = bands[3];
        LOGD("BM FOR DESENCE, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d, req: %d ",
                gsm_band, umts_band, lte_band_1_32, lte_band_33_64, req);
        if (req == BM_FOR_DESENSE_RADIO_ON) {
            if(umts_band & 0x00000080) {
                need_config_umts = 1;
                umts_band = umts_band & 0xffffff7f;
            }
        } else {
            if((umts_band & 0x00000080) == 0) {
                need_config_umts = 1;
                umts_band = umts_band | 0x00000080;
            }
        }
        if (req == BM_FOR_DESENSE_RADIO_OFF) {
            c2k_radio_on = 0;
        } else {
            c2k_radio_on = 1;
        }
        LOGD("BM FOR DESENCE, need_config_umts: %d, force_switch : %d", need_config_umts, force_switch);
        if (need_config_umts == 1 || force_switch == 1) {
            if (req == BM_FOR_DESENSE_B8_OPEN) {
                err = asprintf(&cmd, "AT+EPBSE=%d,%d,%d,%d", gsm_band, umts_band,
                                    lte_band_1_32, lte_band_33_64);
            } else {
                err = asprintf(&cmd, "AT+EPBSE=%d,%d,%d,%d,%d", gsm_band, umts_band,
                        lte_band_1_32, lte_band_33_64, c2k_radio_on);
            }
            LOGD("BM FOR DESENCE, cmd:%s", cmd);
            if (err >= 0) {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
                LOGD("BM FOR DESENCE ERROR. err:%d", err);
                free(cmd);

                if (err >= 0 && p_response->success != 0) {
                    ril_errno = RIL_E_SUCCESS;
                }
            }
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);

    return;

}

void requestQueryAvailableBandMode(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, gsm_band, umts_band;
    char *cmd, *line;
    int band_mode[10], index=1;

    err = at_send_command_singleline("AT+EPBSE=?", "+EPBSE:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
        goto error;


    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) goto error;

    // get supported GSM bands
    err = at_tok_nextint(&line, &gsm_band);
    if(err < 0) goto error;

    // get supported UMTS bands
    err = at_tok_nextint(&line, &umts_band);
    if(err < 0) goto error;

    /// M: Add for band de-sense feature.@{
    bands[0] = gsm_band;
    bands[1] = umts_band;
    err = at_tok_nextint(&line, &bands[2]);
    err = at_tok_nextint(&line, &bands[3]);
    LOGD("requestQueryAvailableBandMode, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d",
            bands[0], bands[1], bands[2], bands[3]);
    /// @}

    //0 for "unspecified" (selected by baseband automatically)
    band_mode[index++] = BM_AUTO_MODE;

    if(gsm_band !=0 || umts_band != 0)
    {
        // 1 for "EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
            (umts_band == 0 || (umts_band | UMTS_BAND_I)==umts_band))
        {
            band_mode[index++] = BM_EURO_MODE;
        }

        // 2 for "US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_850 | GSM_BAND_1900)==gsm_band) &&
             (umts_band == 0 ||  (umts_band | UMTS_BAND_II | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BM_US_MODE;
        }

        // 3 for "JPN band" (WCDMA-800 / WCDMA-IMT-2000)
        if ( (umts_band | UMTS_BAND_I | UMTS_BAND_VI)==umts_band)
        {
            band_mode[index++] = BM_JPN_MODE;
        }

        // 4 for "AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 ||  (umts_band | UMTS_BAND_I | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BM_AUS_MODE;
        }

        // 5 for "AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
        if ( (gsm_band == 0 ||  (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 ||  (umts_band | UMTS_BAND_V)==umts_band) )
        {
            band_mode[index++] = BM_AUS2_MODE;
        }

    }

    band_mode[0] = index;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, band_mode, sizeof(int)*index);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

bool isRatPreferred(int req_type){
    char prop[PROPERTY_VALUE_MAX] = {0};
    int baseband=0;
    bool isTddMD = false;
    int isWcdmaPreferred = 0;
    bool isPreferred = false;

    #ifdef MTK_RIL_MD2
    property_get(PROPERTY_GSM_BASEBAND_CAPABILITY_MD2,prop , "0");
    #else
    property_get(PROPERTY_GSM_BASEBAND_CAPABILITY,prop , "0");
    #endif

    baseband = atoi(prop);

    if(baseband & 0x08){
        isTddMD = true;
    }

    memset(prop,0,sizeof(prop));
    property_get("ro.mtk_rat_wcdma_preferred",prop , "0");
    isWcdmaPreferred = atoi(prop);

    if(((isLteSupport() == 1) && (isTddMD == true)) || (isWcdmaPreferred == 1)){
        isPreferred = true;
    }
    LOGD("isLteSupport=%d,isTddMD=%d,isWcdmaPreferred=%d,baseband=%d",isLteSupport(),isTddMD,isWcdmaPreferred,baseband);

    return isPreferred;
}

void requestSetPreferredNetworkType(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int req_type, err, rat, rat1;
    char *cmd;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    bool isPreferred = false;

    req_type = ((int *)data)[0];
    rat1= 0;

    isPreferred = isRatPreferred(req_type);

    switch(req_type)
    {
    case NT_AUTO_TYPE:
        rat = 2;    // 2/3G AUTO
        break;
    case NT_WCDMA_PREFERRED_TYPE:
        rat = 2;    // 2/3G AUTO
        if(isPreferred){
            rat1= 2; // 3G preferred
        }
        break;
    case NT_GSM_TYPE:
        rat = 0;    // 2G only
        break;
    case NT_WCDMA_TYPE:
        rat = 1;    // 3G only
        break;
    case NT_LTE_GSM_WCDMA_TYPE:
        rat = 6;    // 2/3/4G AUTO
        if (isSvlteSupport()) {
            char prop[PROPERTY_VALUE_MAX] = { 0 };
            property_get("ro.mtk.c2k.om.mode", prop, "cllwtg");
            LOGD("Enter requestSetPreferredNetworkType(), prop = %s ", prop);
            if (strcmp(prop, "cllg") == 0) {
                rat = 4;
            }
        }
        if(isPreferred){
            rat1= 4; //4G preferred
        }
        break;
    case NT_LTE_CDMA_EVDO_GSM_WCDMA:
        rat = 14;    // LTE CDMA EVDO GSM/WCDMA mode
        break;
    case NT_LTE_ONLY_TYPE:
        rat = 3;    // LTE only for EM mode
        break;
    case NT_LTE_WCDMA_TYPE:
        rat = 5;    //  LTE/WCDMA for EM mode
        break;
    default:
        rat = -1;
        break;
    }
    if(rat >= 0)
    {
        err = asprintf(&cmd,  "AT+ERAT=%d,%d", rat, rat1);

        if(err >= 0)
        {
            err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));

            free(cmd);

            if(err >= 0 && p_response->success != 0)
            {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);

}

void requestGetPreferredNetworkType(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, skip, nt_type , prefer_type, return_type;
    char *cmd, *line;

    err = at_send_command_singleline("AT+ERAT?", "+ERAT:", &p_response, getRILChannelCtxFromToken(t));

    if (err < 0 || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;

        prefer_type=0;
    err = at_tok_start(&line);
    if(err < 0) goto error;

    //skip <curr_rat>
    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    //skip <gprs_status>
    err = at_tok_nextint(&line, &skip);
    if(err < 0) goto error;

    //get <rat>
    err = at_tok_nextint(&line, &nt_type);
    if(err < 0) goto error;

    err = at_tok_nextint(&line, &prefer_type);
    if(err < 0) goto error;

    if (nt_type == 0) {
        return_type = NT_GSM_TYPE;
    } else if(nt_type == 1) {
        return_type = NT_WCDMA_TYPE;
    } else if(nt_type == 2 && prefer_type == 0) {
        return_type = NT_AUTO_TYPE;
    } else if(nt_type == 2 && prefer_type == 1) {
        LOGE("Dual mode but GSM prefer, mount to AUTO mode");
        return_type = NT_AUTO_TYPE;
    } else if(nt_type == 2 && prefer_type == 2) {
        return_type = NT_WCDMA_PREFERRED_TYPE;

    //for LTE -- START
    } else if(nt_type == 6 && prefer_type == 4) {
        //4G Preferred (4G, 3G/2G) item
        //Bause we are not defind LTE preferred,
        //so return by NT_LTE_GSM_WCDMA_TYPE temporary
        return_type = NT_LTE_GSM_WCDMA_TYPE;
    } else if(nt_type == 6 && prefer_type == 0) {
        //4G/3G/2G(Auto) item
        return_type = NT_LTE_GSM_WCDMA_TYPE;
    } else if (nt_type == 14 && prefer_type == 0) {
        // LTE CDMA EVDO GSM/WCDMA mode
        return_type = NT_LTE_CDMA_EVDO_GSM_WCDMA;
    } else if(nt_type == 3 && prefer_type == 0) {
        //4G only
        return_type = NT_LTE_ONLY_TYPE;
    } else if(nt_type == 5 && prefer_type == 0) {
        // 4G/3G
        return_type = NT_LTE_WCDMA_TYPE;
    } else {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &return_type, sizeof(int) );
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);

}

void requestGetNeighboringCellIds(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, skip, nt_type;
    char *cmd, *line;
    ATLine *p_line;

    int rat,rssi,ci,lac,psc;
    int i = 0;
    int j = 0;
    RIL_NeighboringCell nbr[6];
    RIL_NeighboringCell *p_nbr[6];

    LOGD("Enter requestGetNeighboringCellIds()");

    err = at_send_command_multiline("AT+ENBR", "+ENBR:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0)
        goto error;

    p_line = p_response->p_intermediates;

    while(p_line != NULL)
    {
        line = p_line->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &rat);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &rssi);
        if(err < 0) goto error;

        if(( (rat == 1) && (rssi < 0  || rssi > 31) && (rssi != 99) )
                || ( (rat == 2) && (rssi < 0 || rssi > 91) ))
        {
            LOGE("The rssi of the %d-th is invalid: %d", i, rssi );
            goto error;
        }

        nbr[i].rssi = rssi;

        if (rat == 1)
        {
            err = at_tok_nextint(&line, &ci);
            if(err < 0) goto error;

            err = at_tok_nextint(&line, &lac);
            if(err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%04X%04X", lac, ci);
            if(err < 0)
            {
                LOGE("Using asprintf and getting ERROR");
                goto error;
            }

            //ALPS00269882 : to bring 'rat' info without changing the interface between RILJ (for backward compatibility concern)
            property_set(PROPERTY_GSM_CURRENT_ENBR_RAT, "1"); //NETWORK_TYPE_GPRS = 1
            LOGD("CURRENT_ENBR_RAT:1");

            LOGD("NC[%d], rssi:%d, cid:%s", i, nbr[i].rssi, nbr[i].cid);
        }
        else if (rat == 2)
        {
            err = at_tok_nextint(&line, &psc);
            if(err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%08X", psc);
            if(err < 0)
            {
                LOGE("Using asprintf and getting ERROR");
                goto error;
            }

            //ALPS00269882 : to bring 'rat' info without changing the interface between RILJ (for backward compatibility concern)
            property_set(PROPERTY_GSM_CURRENT_ENBR_RAT, "3"); //NETWORK_TYPE_UMTS = 3
            LOGD("CURRENT_ENBR_RAT:3");

            LOGD("NC[%d], rssi:%d, psc:%d", i, rssi, psc);
        }
        else
            goto error;

        p_nbr[i] = &nbr[i];
        i++;
        p_line = p_line->p_next;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, p_nbr, sizeof(RIL_NeighboringCell*) * i );
    at_response_free(p_response);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
    return;

error:
    LOGE("requestGetNeighboringCellIds has error occur!!");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
}

void requestSetLocationUpdates(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err, enabled;
    RIL_Errno err_no = RIL_E_GENERIC_FAILURE;
    char *cmd;

    if (datalen == sizeof(int*))
    {
        enabled = ((int*)data)[0];

        if (enabled == 1 || enabled == 0)
        {
            err = asprintf(&cmd, "AT+CREG=%d", enabled ? 2 : 1);
            if( err >= 0 )
            {
                err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
                free(cmd);

                if ( err >= 0 && p_response->success > 0)
                {
                    err_no = RIL_E_SUCCESS;
                }
            }
        }
    }
    RIL_onRequestComplete(t, err_no, NULL, 0);
    at_response_free(p_response);

}

void requestGetCellInfoList(void * data, size_t datalen, RIL_Token t)
{
    int err=0, i=0,num=0 ,act=0 ,cid=0,mcc=0,mnc=0,lacTac=0,pscPci=0,sig1=0,sig2=0,rsrp=0,rsrq=0,rssnr=0,cQi=0,timingAdvance=0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    RIL_CellInfo** response = NULL;

    err = at_send_command_singleline("AT+ECELL", "+ECELL:", &p_response, NW_CHANNEL_CTX);

    /* +ECELL: <num_of_cell>[,<Act>,<cid>,<lac_or_tac>,<mcc>,<mnc>,<psc_or_pci>,
           <sig1>,<sig2>,<sig1_in_dbm>,<sig2_in_dbm>,<ta>,<ext1>,<ext2>][,K]  */

    if (err < 0 || p_response->success == 0) goto error;
    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &num);
    if (err < 0) goto error;
    if (num < 1){
        LOGD("No cell info listed, num=%d",num);
        goto error;
    }

    LOGD("Cell info listed, number=%d",num);

    /*According to ril.h,  "response" shall be an array of  RIL_CellInfo. */
    response = (RIL_CellInfo**) calloc(1, num * sizeof(RIL_CellInfo*));

    for(i=0;i<num;i++){
        response[i] = calloc(1, sizeof(RIL_CellInfo));
 
        /*We think registered field is used to distinguish serving cell or neighboring cell.
                 The first cell info returned from modem is the serving cell, others are neighboring cell */
        if(i==0)
            response[i]->registered = 1;

        err = at_tok_nextint(&line, &act);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&line, &cid);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&line, &lacTac);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &mcc);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &mnc);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &pscPci);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &sig1);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &sig2);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &rsrp);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &rsrq);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &timingAdvance);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &rssnr);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &cQi);
        if (err < 0) goto error;

        LOGD("act=%d,cid=%d,mcc=%d,mnc=%d,lacTac=%d,pscPci=%d,sig1=%d,sig2=%d,sig1_dbm=%d,sig1_dbm=%d,ta=%d,ext1(rssnr)=%d,ext2(cqi)=%d",act,cid,mcc,mnc,lacTac,pscPci,sig1,sig2,rsrp,rsrq,timingAdvance,rssnr,cQi);

        /* <Act>  0: GSM , 2: UMTS , 7: LTE */
        if(act == 7){
            LOGD("RIL_CELL_INFO_TYPE_LTE act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            response[i]->CellInfo.lte.cellIdentityLte.ci = cid;
            response[i]->CellInfo.lte.cellIdentityLte.mcc = mcc;
            response[i]->CellInfo.lte.cellIdentityLte.mnc = mnc;
            response[i]->CellInfo.lte.cellIdentityLte.tac = lacTac;
            response[i]->CellInfo.lte.cellIdentityLte.pci = pscPci;
            response[i]->CellInfo.lte.signalStrengthLte.signalStrength = sig1;
            // for consistance with google's rsrp and rsrq define of RIL_LTE_SignalStrength_v8
            // abs(rsrp_in_qdbm /4), abs(rsrq_in_qdbm /4)
            response[i]->CellInfo.lte.signalStrengthLte.rsrp = (rsrp/4) * (-1);
            response[i]->CellInfo.lte.signalStrengthLte.rsrq = (rsrq/4) * (-1);
            response[i]->CellInfo.lte.signalStrengthLte.timingAdvance = timingAdvance;
            response[i]->CellInfo.lte.signalStrengthLte.rssnr = rssnr;
            response[i]->CellInfo.lte.signalStrengthLte.cqi = cQi;
        } else if(act == 2){
            LOGD("RIL_CELL_INFO_TYPE_WCDMA act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_WCDMA;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.cid = cid;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.mcc = mcc;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.mnc = mnc;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.lac = lacTac;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.psc = pscPci;
            response[i]->CellInfo.wcdma.signalStrengthWcdma.signalStrength = sig1;
            response[i]->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = sig2;
        } else{
            LOGD("RIL_CELL_INFO_TYPE_GSM act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_GSM;
            response[i]->CellInfo.gsm.cellIdentityGsm.cid = cid;
            response[i]->CellInfo.gsm.cellIdentityGsm.mcc = mcc;
            response[i]->CellInfo.gsm.cellIdentityGsm.mnc = mnc;
            response[i]->CellInfo.gsm.cellIdentityGsm.lac = lacTac;
            response[i]->CellInfo.gsm.signalStrengthGsm.signalStrength = sig1;
            response[i]->CellInfo.gsm.signalStrengthGsm.bitErrorRate = sig2;
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, num * sizeof(RIL_CellInfo));

    at_response_free(p_response);

    if(response != NULL){
        for(i=0;i<num;i++){
            if(response[i] != NULL){
                free(response[i]);
            }
        }
        free(response);
    }
    return;

error:
    at_response_free(p_response);
    if(response != NULL){
        for(i=0;i<num;i++){
            if(response[i] != NULL){
                free(response[i]);
            }
        }
        free(response);
    }
    LOGE("requestGetCellInfoList must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    return;
}

void requestSetCellInfoListRate(void * data, size_t datalen, RIL_Token t)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int err=0;
    int time = -1;
    char *cmd;
    ATResponse *p_response = NULL;
    RIL_Errno err_no = RIL_E_SUCCESS;

    time = ((int *)data)[0];

    LOGD("requestSetCellInfoListRate time: %d", time);

    /* According to ril.h
     * Sets the minimum time between when RIL_UNSOL_CELL_INFO_LIST should be invoked.
     * A value of 0, means invoke RIL_UNSOL_CELL_INFO_LIST when any of the reported
     * information changes. Setting the value to INT_MAX(0x7fffffff) means never issue
     * a RIL_UNSOL_CELL_INFO_LIST.
     */

    /* To prevent cell info chang report too much to increase the power consumption
       Modem only support disable/enable +ECELL URC. +ECELL URC will be reported for any of cell info change.
       For other rate , the cell info report is done by FW tol query cell info every time rate */
    if(time == 0){
        asprintf(&cmd, "AT+ECELL=1");
    }else if(time <= 0x7fffffff){
        asprintf(&cmd, "AT+ECELL=0");
    }else{
        goto error;
    }

    err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        LOGE("[RIL%d]AT+ECELL return ERROR", rid+1 );
        goto error;
    }
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestImsRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    int err;
    int skip;
    int response[2] = { 0 };
    char *line;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(NW_CHANNEL_CTX);
    ATResponse *p_response = NULL;
    char* tokenStr = NULL;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline(
              "AT+CIREG?",
              "+CIREG:", &p_response, NW_CHANNEL_CTX);

    /* +CIREG: <n>,<reg_info>[,<ext_info>] */
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 )
    {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else {
        /* <reg_info> value 0: not registered , 1: registered */
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0 ) goto error;

        response[1] = RADIO_TECH_3GPP;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    LOGE("requestImsRegistrationState must not return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}


RIL_RadioState getNewRadioState(RIL_RadioState state)
{
    return (state > RADIO_STATE_UNAVAILABLE? state : RADIO_STATE_SIM_NOT_READY);
}

void requestGetPacketSwitchBearer(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    char *bearer = NULL;

    err = at_send_command_singleline("AT+EPSB?", "+EPSB:", &p_response, getRILChannelCtx(RIL_NW,getMainProtocolRid()));

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    asprintf(&bearer, "%d", ret);

    #ifdef MTK_RIL_MD2
        property_set(PROPERTY_GSM_BASEBAND_CAPABILITY_MD2, bearer);
    #else
        property_set(PROPERTY_GSM_BASEBAND_CAPABILITY, bearer);
    #endif

    free(bearer);

    LOGD("[RIL%d] AT+EPSB return %d", rid+1, ret);
    at_response_free(p_response);

    return;

error:
    LOGE("[RIL%d] AT+EPSB return ERROR", rid+1);
    at_response_free(p_response);
}

void requestAbortQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    LOGD("requestAbortQueryAvailableNetworks execute while plmnListOngoing=%d",plmnListOngoing);
    if(plmnListOngoing == 1){
        LOGD("requestAbortQueryAvailableNetworks set plmnListAbort flag");
        plmnListAbort =1;

        err = at_send_command("AT+CAPL", &p_response, NW_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0)
        {
            ril_errno = RIL_E_GENERIC_FAILURE;
            plmnListAbort =0;
            LOGD("requestAbortQueryAvailableNetworks fail,clear plmnListAbort flag");
        }
        else
        {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}
void onNetworkStateChanged(char *urc, const RIL_SOCKET_ID rid)
{
    int err, is_cs, is_r8 = 0;
    int stat;
    unsigned int response[4] = {-1};
    char* responseStr[5];
    int i = 0;
#ifdef MTK_DTAG_DUAL_APN_SUPPORT
    static int first_register = 0;
    static unsigned int s_stat, s_act;
    int is_dual_apn_check;
#endif

    if (strStartsWith(urc,"+CREG:")) {
        is_cs = 1;
        is_r8 = 0;
#ifdef MTK_DTAG_DUAL_APN_SUPPORT
        is_dual_apn_check = 1;
#endif
    } else if (strStartsWith(urc,"+CGREG:")) {
        is_cs = 0;
        is_r8 = 0;
    } else if (strStartsWith(urc,"+CEREG:")) {
        is_cs = 0;
        is_r8 = 0;
    } else if (strStartsWith(urc,"+PSBEARER:")) {
        is_cs = 0;
        is_r8 = 1;
    } else {
        LOGE("The URC is wrong: %s", urc);
        return;
    }

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &stat);
    if (err < 0) goto error;
    LOGD("The URC is %s", urc);
    //CGREG:4 means the GPRS is detached
    if (is_r8 != 1 && is_cs == 0 && stat == 4) {
        RIL_onUnsolicitedResponseSocket (RIL_UNSOL_GPRS_DETACH, NULL, 0, rid);
    }

    //for Lte
    stat = convertRegState(stat);

    LOGD("onNetworkStateChanged() , stat=%d , is_cs=%d",stat,is_cs);

    //ALPS00283717
    if(is_cs == 1)
    {
        /* Note: There might not be full +CREG URC info when screen off
                   Full URC format: +CREG: <n>, <stat>, <lac>, <cid>, <Act>,<cause> */

        // set "-1" to indicate "the field is not present"
        for(i=1;i<5;i++)
            asprintf(&responseStr[i], "-1");

        // fill in <state>
        asprintf(&responseStr[0], "%d", stat);

        //get <lac>
        err = at_tok_nexthexint(&urc,(int*) &response[0]);
        if(err >= 0)
        {
            free(responseStr[1]);
            responseStr[1] = NULL;
            asprintf(&responseStr[1], "%x", response[0]);

            //get <ci>
            err = at_tok_nexthexint(&urc, (int*) &response[1]);
            if(err >= 0)
            {
                LOGD("+creg urc <ci>: %d", response[1]);
                free(responseStr[2]);
                asprintf(&responseStr[2], "%x", response[1]);

                //get <Act>
                err = at_tok_nextint(&urc, (int*) &response[2]);
                if(err >= 0)
                {
                    free(responseStr[3]);
                    responseStr[3] = NULL;
                    asprintf(&responseStr[3], "%d", response[2]);

                    //if LTE support the nextInt will be <cause_type>
                    if(bCREGType3Support){
                        //get <cause_type>
                        LOGD("bCREGType3Support=1");
                        err = at_tok_nextint(&urc,(int*) &response[3]);
                        if (err >= 0)
                        {
                            LOGD("now +creg urc <cause_type>: %d", response[3]);
                            //get <cause>
                            err = at_tok_nextint(&urc,(int*) &response[3]);
                            if(err >= 0)
                            {
                                LOGD("now +creg urc <cause>: %d", response[3]);
                                free(responseStr[4]);
                                responseStr[4] = NULL;
                                asprintf(&responseStr[4], "%d", response[3]);
                           }
                        }
                    } else {
                        //get <cause>
                        err = at_tok_nextint(&urc,(int*) &response[3]);
                        if(err >= 0)
                        {
                            LOGD("now +creg urc <cause>: %d", response[3]);
                            free(responseStr[4]);
                            responseStr[4] = NULL;
                            asprintf(&responseStr[4], "%d", response[3]);
                        }
                    }
                }
            }
        }
    }

    if (is_cs == 1)
    {
        int cid = response[1];
        int lac = response[0];
        if((cid == -1) || (stat == 4)){
            /* state(4) is 'unknown'  and cid(-1) means cid was not provided in +creg URC */
            LOGD("No valid info to distinguish limited service and no service");
        }else{
            //[ALPS01451327] need to handle different format of Cid
            // if cid is 0x0fffffff means it is invalid
            // for backward comptable we keep filter of oxffff
            if(((stat ==0)||(stat == 2)||(stat == 3)) && ( (((cid & 0xffff)!=0) && (cid!=0x0fffffff))&& (lac!=0xffff))){
                if(s_emergencly_only[rid]==0){
                    s_emergencly_only[rid]=1;
                    LOGD("Set s_emergencly_only[%d] to true",rid);
                }
            }
            else{
                if(s_emergencly_only[rid]==1){
                    s_emergencly_only[rid]=0;
                    LOGD("Set s_emergencly_only[%d] to false",rid);
                }
            }
        }

        RIL_onUnsolicitedResponseSocket (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, responseStr,  sizeof(responseStr), rid);
        for(i=0;i<5;i++)
            free(responseStr[i]);
    } else {
        if (is_r8 == 1) {
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, &ril_data_reg_status[rid], sizeof(ril_data_reg_status[rid]), rid);
        } else {
            ril_data_reg_status[rid] = stat;
            RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, &stat, sizeof(stat), rid);
        }
    }

#ifdef MTK_DTAG_DUAL_APN_SUPPORT
    if (1 == is_dual_apn_check)
    {
        //get <lac>
        err = at_tok_nexthexint(&urc,(int*) &response[0]);
        if (err >= 0)
        {
            //get <ci>
            err = at_tok_nexthexint(&urc, (int*) &response[1]);
            if (err >= 0)
            {
                //get <Act>
                err = at_tok_nextint(&urc, (int*) &response[2]);
                if (err >= 0)
                {
                    LOGD("+creg urc <Act>: %d", response[2]);
                }
            }
        }

        //Ignore the first time registered indication to get rid of confusion of NODATA activation
        if ((0 == first_register) && ((1 == stat) || (5 == stat)))
        {
            first_register = 1;
            LOGD("+creg urc: first camp on NW");
        }
        else if ((1 == first_register) && (((7 != s_act) && (7 == response[2])) || ((5 != s_stat) && (5 == stat) && (7 == response[2]))))
        {
            RILChannelCtx* pChannel = getChannelCtxbyId(sCmdChannel4Id[rid]);
            RIL_requestProxyTimedCallback (onDualApnCheckNetworkChange, pChannel, &dualApn_TIMEVAL_0, pChannel->id,
                    "onDualApnCheckNetworkChange");
        }

        s_stat = stat;
        s_act = response[2];
    }
#endif

   // onRestrictedStateChanged(rid);

    return;

error:
    LOGE("There is something wrong with the URC");
}

void onNitzTimeReceived(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int length, i, id;
    char nitz_string[30];
    char *line;
    /*+CIEV: 9, <time>, <tz>[, <dt>]
    <time>: "yy/mm/dd, hh:mm:ss"
    <tz>: [+,-]<value>, <value> is quarters of an hour between local time and GMT
    <dt>: day_light_saving value*/

    /* Final format :  "yy/mm/dd,hh:mm:ss(+/-)tz[,dt]" */

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 9) return;

    if (!at_tok_hasmore(&urc))
    {
        LOGE("There is no NITZ data");
        return;
    }

    length = strlen(urc);
    if (length >= (int) sizeof(nitz_string))
    {
        LOGE("The length of the NITZ URC is too long: %d", length);
        return;
    }
    nitz_string[0] = '\0';

    // eliminate the white space first
    line = urc;
    for (i = 0; i<length ; i++)
    {
        if(urc[i] != ' ')
        {
            *(line++) = urc[i];
        }
    }
    *line = '\0';

    length = strlen(urc);
    if(length == 0)
    {
        LOGE("Receiving an empty NITZ");
        return;
    }

    // get <time>
    err = at_tok_nextstr(&urc, &line);
    if (err < 0)
    {
        LOGE("ERROR occurs when parsing <time> of the NITZ URC");
        return;
    }
    strcat(nitz_string, line);

    // concatenate the remaining with <time>
    if (strlen(urc) > 0) {
        strcat(nitz_string, urc);
    } else {
        LOGE("There is no remaining data, said <tz>[,<dt>]");
        return;
    }

    RIL_onUnsolicitedResponseSocket (RIL_UNSOL_NITZ_TIME_RECEIVED, nitz_string, sizeof(char *), rid);
}

/* Add-BY-JUNGO-20101008-CTZV SUPPORT*/
void onNitzTzReceived(char *urc, const RIL_SOCKET_ID rid)
{
    int err, i = 0, length = 0;
    int response[2];
    char nitz_string[30];
    time_t calendar_time;
    struct tm *t_info = NULL;
    char *line = NULL;
    /* Final format :  "yy/mm/dd,hh:mm:ss(+/-)tz[,dt]" */

    err = at_tok_start(&urc);
    if(err < 0) return;

    if(strlen(urc) > 0)
    {

        length = strlen(urc);
        // eliminate the white space first
        line = urc;
        for (i = 0; i<length ; i++)
        {
            if(urc[i] != ' ')
            {
                *(line++) = urc[i];
            }
        }
        *line = '\0';

        //get the system time to fullfit the NITZ string format
        calendar_time = time(NULL);
        if(-1 == calendar_time) return;

        t_info = gmtime(&calendar_time);
        if(NULL == t_info) return;

        memset(nitz_string, 0, sizeof(nitz_string));
        sprintf(nitz_string, "%02d/%02d/%02d,%02d:%02d:%02d%s",  //"yy/mm/dd,hh:mm:ss"
                (t_info->tm_year)%100,
                t_info->tm_mon+1,
                t_info->tm_mday,
                t_info->tm_hour,
                t_info->tm_min,
                t_info->tm_sec,
                urc);

        LOGD("NITZ:%s", nitz_string);

        RIL_onUnsolicitedResponseSocket (RIL_UNSOL_NITZ_TIME_RECEIVED, nitz_string, sizeof(char *), rid);
    }
    else
    {
        LOGE("There is no remaining data, said <tz>[,<dt>]");
        return;
    }
}

void onNitzOperNameReceived(char* urc, const RIL_SOCKET_ID rid)
{
    int err;
    int length, i, id;
    char nitz_string[101];
    char *line;
    char *oper_code;
    char *oper_lname;
    char *oper_sname;
    int is_lname_hex_str=0;
    int is_sname_hex_str=0;
    char temp_oper_name[MAX_OPER_NAME_LENGTH]={0};

    /* +CIEV: 10,"PLMN","long_name","short_name" */

    err = at_tok_start(&urc);
    if (err < 0) return;

    err = at_tok_nextint(&urc, &id);
    if (err < 0 || id != 10) return;

    if (!at_tok_hasmore(&urc))
    {
        LOGE("There is no NITZ data");
        return;
    }

    oper_code   = ril_nw_nitz_oper_code[rid];
    oper_lname  = ril_nw_nitz_oper_lname[rid];
    oper_sname  = ril_nw_nitz_oper_sname[rid];

    /* FIXME: it is more good to transfer the OPERATOR NAME to the Telephony Framework directly */

    pthread_mutex_lock(&ril_nw_nitzName_mutex[rid]);
    LOGD("Get ril_nw_nitzName_mutex in the onNitzOperNameReceived");

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_code, line, MAX_OPER_NAME_LENGTH);
    oper_code[MAX_OPER_NAME_LENGTH-1] = '\0';

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_lname, line, MAX_OPER_NAME_LENGTH);
    oper_lname[MAX_OPER_NAME_LENGTH-1] = '\0';

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) goto error;
    strncpy(oper_sname, line, MAX_OPER_NAME_LENGTH);
    oper_sname[MAX_OPER_NAME_LENGTH-1] = '\0';

    /* ALPS00459516 start */
    if ((strlen(oper_lname)%8) == 0){
        LOGD("strlen(oper_lname)=%d", strlen(oper_lname));

        length = strlen(oper_lname);
        if (oper_lname[length-1] == '@'){
            oper_lname[length-1] = '\0';
            LOGD("remove @ new oper_lname:%s", oper_lname);
        }
    }

    if ((strlen(oper_sname)%8) == 0){
        LOGD("strlen(oper_sname)=%d", strlen(oper_sname));

        length = strlen(oper_sname);
        if (oper_sname[length-1] == '@'){
            oper_sname[length-1] = '\0';
            LOGD("remove @ new oper_sname:%s", oper_sname);
        }
    }
    /* ALPS00459516 end */

    /* ALPS00262905 start
       +CIEV: 10, <plmn_str>,<full_name_str>,<short_name_str>,<is_full_name_hex_str>,<is_short_name_hex_str> for UCS2 string */
    err = at_tok_nextint(&urc, &is_lname_hex_str);
    if (err >= 0)
    {
        LOGD("is_lname_hex_str=%d",is_lname_hex_str);

        if (is_lname_hex_str == 1)
        {
            /* ALPS00273663 Add specific prefix "uCs2" to identify this operator name is UCS2 format.  prefix + hex string ex: "uCs2806F767C79D1"  */
            memset(temp_oper_name, 0, sizeof(temp_oper_name));
            strncpy(temp_oper_name, "uCs2", 4);
            strncpy(&(temp_oper_name[4]), oper_lname, MAX_OPER_NAME_LENGTH-4);
            memset(oper_lname, 0, MAX_OPER_NAME_LENGTH);
            strncpy(oper_lname, temp_oper_name, MAX_OPER_NAME_LENGTH);
            LOGD("lname add prefix uCs2");
        }

        err = at_tok_nextint(&urc, &is_sname_hex_str);

        LOGD("is_sname_hex_str=%d",is_sname_hex_str);

        if ((err >= 0) && (is_sname_hex_str == 1))
        {
            /* ALPS00273663 Add specific prefix "uCs2" to identify this operator name is UCS2 format.  prefix + hex string ex: "uCs2806F767C79D1"  */
            memset(temp_oper_name, 0, sizeof(temp_oper_name));
            strncpy(temp_oper_name, "uCs2", 4);
            strncpy(&(temp_oper_name[4]), oper_sname, MAX_OPER_NAME_LENGTH-4);
            memset(oper_sname, 0, MAX_OPER_NAME_LENGTH);
            strncpy(oper_sname, temp_oper_name, MAX_OPER_NAME_LENGTH);
            LOGD("sname Add prefix uCs2");
        }
    }
    /* ALPS00262905 end */

    LOGD("Get NITZ Operator Name of RIL %d: %s %s %s", rid+1, oper_code, oper_lname, oper_sname);

    if (getMappingSIMByCurrentMode(rid) >= GEMINI_SIM_1) {
        int phoneId = getMappingSIMByCurrentMode(rid);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_CODE, oper_code);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_LNAME, oper_lname);
        setMSimProperty(phoneId, PROPERTY_NITZ_OPER_SNAME, oper_sname);
    }

    if (RADIO_STATE_SIM_READY == getRadioState(rid)) {
        RIL_onUnsolicitedResponseSocket (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0, rid);
    }

error:
    pthread_mutex_unlock(&ril_nw_nitzName_mutex[rid]);
    return;
}

void onRestrictedStateChanged(const RIL_SOCKET_ID rid)
{
    int state[1];
    state[0] = ril_nw_cur_state[rid] & RIL_NW_ALL_RESTRICTIONS;
    LOGD("RestrictedStateChanged: %d", state[0]);

    if (isSimInserted(rid)){
        /* ALPS01438966 prevent sending redundant RESTRICTED_STATE_CHANGED event */
        if(state[0] != ril_prev_nw_cur_state[rid]){
            RIL_onUnsolicitedResponseSocket (RIL_UNSOL_RESTRICTED_STATE_CHANGED, state, sizeof(state), rid);
            ril_prev_nw_cur_state[rid] = state[0];
        }
    }
}

void onCellInfoList(char *urc, const RIL_SOCKET_ID rid)
{
    int err=0, i=0,num=0 ,act=0 ,cid=0,mcc=0,mnc=0,lacTac=0,pscPci=0,sig1=0,sig2=0,rsrp=0,rsrq=0,rssnr=0,cQi=0,timingAdvance=0;
    RIL_CellInfo** response = NULL;

    /* +ECELL: <num_of_cell>[,<Act>,<signal_strength>,<cid>,<lac_or_tac>,<mcc>,<mnc>,<psc_or_pci>][,K]  */
    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &num);
    if (err < 0) goto error;
    if (num < 1){
        LOGD("No cell info listed, num=%d",num);
        goto error;
    }

    LOGD("Cell Info listed, number =%d",num);

    response = (RIL_CellInfo**) calloc(1, num * sizeof(RIL_CellInfo*)); 

    for(i=0;i<num;i++){
        response[i] = calloc(1, sizeof(RIL_CellInfo)); 
        /*We think registered field is used to distinguish serving cell or neighboring cell.
                 The first cell info returned from modem is the serving cell, others are neighboring cell */
        if(i==0)
            response[i]->registered = 1;

        err = at_tok_nextint(&urc, &act);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&urc, &cid);
        if (err < 0) goto error;

        err = at_tok_nexthexint(&urc, &lacTac);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &mcc);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &mnc);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &pscPci);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &sig1);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &sig2);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &rsrp);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &rsrq);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &timingAdvance);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &rssnr);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &cQi);
        if (err < 0) goto error;

        LOGD("act=%d,cid=%d,mcc=%d,mnc=%d,lacTac=%d,pscPci=%d,sig1=%d,sig2=%d,sig1_dbm=%d,sig1_dbm=%d,ta=%d,ext1(rssnr)=%d,ext2(cqi)=%d",act,cid,mcc,mnc,lacTac,pscPci,sig1,sig2,rsrp,rsrq,timingAdvance,rssnr,cQi);

        /* <Act>  0: GSM , 2: UMTS , 7: LTE */
        if(act == 7){
            LOGD("RIL_CELL_INFO_TYPE_LTE act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            response[i]->CellInfo.lte.cellIdentityLte.ci = cid;
            response[i]->CellInfo.lte.cellIdentityLte.mcc = mcc;
            response[i]->CellInfo.lte.cellIdentityLte.mnc = mnc;
            response[i]->CellInfo.lte.cellIdentityLte.tac = lacTac;
            response[i]->CellInfo.lte.cellIdentityLte.pci = pscPci;
            response[i]->CellInfo.lte.signalStrengthLte.signalStrength = sig1;
            response[i]->CellInfo.lte.signalStrengthLte.rsrp = rsrp;
            response[i]->CellInfo.lte.signalStrengthLte.rsrq = rsrq;
            response[i]->CellInfo.lte.signalStrengthLte.timingAdvance = timingAdvance;
            response[i]->CellInfo.lte.signalStrengthLte.rssnr = rssnr;
            response[i]->CellInfo.lte.signalStrengthLte.cqi = cQi;
        } else if(act == 2){
            LOGD("RIL_CELL_INFO_TYPE_WCDMA act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_WCDMA;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.cid = cid;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.mcc = mcc;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.mnc = mnc;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.lac = lacTac;
            response[i]->CellInfo.wcdma.cellIdentityWcdma.psc = pscPci;
            response[i]->CellInfo.wcdma.signalStrengthWcdma.signalStrength = sig1;
            response[i]->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = sig2;
        } else{
            LOGD("RIL_CELL_INFO_TYPE_GSM act=%d",act);
            response[i]->cellInfoType = RIL_CELL_INFO_TYPE_GSM;
            response[i]->CellInfo.gsm.cellIdentityGsm.cid = cid;
            response[i]->CellInfo.gsm.cellIdentityGsm.mcc = mcc;
            response[i]->CellInfo.gsm.cellIdentityGsm.mnc = mnc;
            response[i]->CellInfo.gsm.cellIdentityGsm.lac = lacTac;
            response[i]->CellInfo.gsm.signalStrengthGsm.signalStrength = sig1;
            response[i]->CellInfo.gsm.signalStrengthGsm.bitErrorRate = sig2;
        }
    }

    RIL_onUnsolicitedResponseSocket (RIL_UNSOL_CELL_INFO_LIST, response,  num * sizeof(RIL_CellInfo), rid);

    if(response != NULL){
        for(i=0;i<num;i++){
            if(response[i] != NULL){
                free(response[i]);
            }
        }
        free(response);
    }
    return;

error:
    if(response != NULL){
        for(i=0;i<num;i++){
            if(response[i] != NULL){
                free(response[i]);
            }
        }
        free(response);
    }
    LOGE("onCellInfoList parse error");
    return;
}


extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
    case RIL_REQUEST_SIGNAL_STRENGTH:
        requestSignalStrength(data, datalen, t);
        break;
    case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        requestRegistrationState(data, datalen, t);
        break;
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
        if (!isEpdgSupport()){
            requestGprsRegistrationState(data, datalen, t);
        } else {
            requestMalGprsRegistrationState(data, datalen, t);
        }
        break;
    case RIL_REQUEST_OPERATOR:
        requestOperator(data, datalen, t);
        break;
    case RIL_REQUEST_RADIO_POWER:
        requestRadioPower(data, datalen, t);
        break;
    case RIL_REQUEST_MODEM_POWEROFF:
        requestRadioPowerOff(data, datalen, t);
        break;
    case RIL_REQUEST_MODEM_POWERON:
        requestRadioPowerOn(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
        requestSetNetworkSelectionAutomatic(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
        requestQueryNetworkSelectionMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
        requestSetNetworkSelectionManual(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        requestQueryAvailableNetworks(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
        requestQueryAvailableNetworksWithAct(data, datalen, t);
        break;
    case RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS:
        requestAbortQueryAvailableNetworks(data, datalen, t);
        break;
    case RIL_REQUEST_SET_BAND_MODE:
        requestSetBandMode(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        requestQueryAvailableBandMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        requestSetPreferredNetworkType(data, datalen, t);
        break;
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
        requestGetPreferredNetworkType(data, datalen, t);
        break;
    case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
        requestGetNeighboringCellIds(data, datalen, t);
        break;
    case RIL_REQUEST_SET_LOCATION_UPDATES:
        requestSetLocationUpdates(data, datalen, t);
        break;
    case RIL_REQUEST_DUAL_SIM_MODE_SWITCH:
        requestRadioMode(data, datalen, t);
        break;
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT:
        requestSetNetworkSelectionManualWithAct(data, datalen, t);
        break;
    case RIL_REQUEST_VOICE_RADIO_TECH:
        requestVoiceRadioTech(data, datalen, t);
        break;
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    case RIL_REQUEST_SET_REG_SUSPEND_ENABLED:
        requestSetRegSuspendEnabled(data, datalen, t);
        break;
    case RIL_REQUEST_RESUME_REGISTRATION:
        requestResumeRegistration(data, datalen, t);
        break;
    /// M: [C2K][IR] Support SVLTE IR feature. @}
//Femtocell (CSG) feature START
    case RIL_REQUEST_GET_FEMTOCELL_LIST:
        requestGetFemtoCellList(data, datalen, t);
        break;
    case RIL_REQUEST_ABORT_FEMTOCELL_LIST:
        requestAbortFemtoCellList(data, datalen, t);
        break;
    case RIL_REQUEST_SELECT_FEMTOCELL:
        requestSelectFemtoCell(data, datalen, t);
        break;
//Femtocell (CSG) feature END
    case RIL_REQUEST_GET_POL_CAPABILITY:
        requestGetPOLCapability(data, datalen, t);
        break;
    case RIL_REQUEST_GET_POL_LIST:
        requestGetPOLList(data, datalen, t);
        break;
    case RIL_REQUEST_SET_POL_ENTRY:
        requestSetPOLEntry(data, datalen, t);
        break;
    case RIL_REQUEST_SEND_OPLMN:
        requestSendOplmn(data, datalen, t);
        break;
    case RIL_REQUEST_GET_OPLMN_VERSION:
        requestGetOplmnVersion(data, datalen, t);
        break;
    case RIL_REQUEST_GET_CELL_INFO_LIST:
        requestGetCellInfoList(data, datalen, t);
        break;
    case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE:
        requestSetCellInfoListRate(data, datalen, t);
        break;
    case RIL_REQUEST_IMS_REGISTRATION_STATE:
        requestImsRegistrationState(data, datalen, t);
        break;
    /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
    case RIL_REQUEST_SET_SVLTE_RAT_MODE:
    	requestSetSvlteRatMode(data, datalen, t);
        break;
    /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}
    /// M: [C2K][SVLTE] Set the STK UTK mode. @{
    case RIL_REQUEST_SET_STK_UTK_MODE:
        requestSetStkUtkMode(data, datalen, t);
        break;
    /// M: [C2K][SVLTE] Set the STK UTK mode. @}

    /// M: EPDG feature. Update PS state from MAL @{
    case RIL_REQUEST_MAL_PS_RGEGISTRATION_STATE:
        onMalPsStateChanged(data, datalen, t);
        break;
    /// @}

    default:
        return 0; /* no matched requests */
        break;
    }

    return 1; /* request find and handled */
}

int rilNwUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    LOGD("Nw URC:%s", s);

    if (strStartsWith(s, "+CREG:") || strStartsWith(s, "+CGREG:") || strStartsWith(s, "+PSBEARER:") || strStartsWith(s, "+CEREG:"))
    {
        /// M: EPDG feature. Only handle URC+CREG and URC+PSBERAER from MD1 @{
        if(!isEpdgSupport()){
            onNetworkStateChanged((char*) s,rid);
            return 1;
        } else {
            if (strStartsWith(s, "+CREG:")){
                onNetworkStateChanged((char*) s,rid);
            } else if (strStartsWith(s, "+PSBEARER:")){
                onPsbearerChangeForEpdg((char*) s,rid);
            }
            LOGD("ignore CGREG/CEREG for IWLAND");
            return 1;
        }
        /// @}
    }
    else if (strStartsWith(s, "+ECSQ:"))
    {
        onSignalStrenth((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+ECELLINFO:"))
    {
        onNeighboringCellInfo((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+ENWINFO:"))
    {
        onNetworkInfo((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+CIEV: 9"))
    {
        onNitzTimeReceived((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+CIEV: 10"))
    {
        onNitzOperNameReceived((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+CTZV:")) //Add by Jugno 20101008
    {
        onNitzTzReceived((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EIND: 16")) //ALPS00248788
    {
        onInvalidSimInfo((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EACMT:"))
    {
        onACMT((char*) s,rid);
        return 1;
    }
    else if (strStartsWith(s, "+EMMRRS:")) //ALPS00368272
    {
        onMMRRStatusChanged((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+ECOPS:"))
    {
        onPlmnListChanged((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+EMSR:"))
    {
        onRegistrationSuspended((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+ECSG:"))
    {
        onFemtoCellInfo((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+ECELL:"))
    {
        onCellInfoList((char*) s, rid);
        return 1;
    }
    else if (strStartsWith(s, "+CIREGU")) {
        LOGD("CIREG URC:%s",s);
        onImsRegistrationInfo((char *)s, rid);
        return 1;
    }
    /// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @{
    else if (strStartsWith(s, "+EGMSS:"))
    {
        onGmssRat((char*) s, rid);
        return 1;
    }
    /// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @}
    //WorldMode
    else if (strStartsWith(s, "+ECSRA:"))
    {
        if (bUbinSupport){
            onWorldModeChanged((char*) s, rid);
        } else {
            /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
            onLteRadioCapabilityChanged((char*) s, rid);
            /// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}
        }
        return 1;
    }
    // M: [LTE][Low Power][UL traffic shaping] Start
    else if (strStartsWith(s, "+EDRBSTATE:")) {
        onLteAccessStratumResult((char*) s, rid);
        return 1;
    }
    // M: [LTE][Low Power][UL traffic shaping] End

    return 0;
}
void requestGetOplmnVersion(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line, *version;

    err = at_send_command_singleline("AT+EPOL?", "+EPOL:", &p_response,
            getRILChannelCtxFromToken(t));
    if (err < 0 || p_response == NULL) {
        goto error;
    }

    //+EPOL: <version>
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
        goto error;

    err = at_tok_nextstr(&line, &version);
    if (err < 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, version, sizeof(char *));
    at_response_free(p_response);
    return;
    error: RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSendOplmn(void *data, size_t datalen, RIL_Token t) {
    int ret;
    char *cmd;
    ATResponse *p_response = NULL;

    LOGD("Request Send oplmn file: %s", (char*) data);

    asprintf(&cmd, "AT+EPOL=%s", (char*) data);

    ret = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);

    free(cmd);

    if (ret < 0 || p_response->success == 0)
    {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    else
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }

    at_response_free(p_response);
}

/// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @{
void onGmssRat(char* urc, const RIL_SOCKET_ID rid)
{
    /* +EGMSS:<rat>,<mcc>
     * <rat>
     *     0: Any RAT in 3GPP2 RAT group
     *     1: Any RAT in 3GPP RAT group
     *     2: CDMA2000 1x
     *     3: CDMA2000 HRPD
     *     4: GERAN
     *     5: UTRAN
     *     6: EUTRAN
     * <mcc>
     *     460: CT
     */
    int err;
    int rat;
    int mcc;
    int attached = 0;
    char * mcc_str = NULL;
    int response[3] = {0};
    err = at_tok_start(&urc);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &rat);
    if (err < 0) {
        LOGD("onGmssRat has error rat");
        return;
    }
    err = at_tok_nextstr(&urc, &mcc_str);
    if (err < 0) {
        LOGD("onGmssRat has error mcc");
        return;
    }
    err = at_tok_nextint(&urc, &attached);
    if (err < 0) {
        LOGD("onGmssRat has error attached");
        return;
    }
    if (mcc_str != NULL) {
        mcc = atoi(mcc_str);
    }
    response[0] = rat;
    response[1] = mcc;
    response[2] = attached;
    LOGD("onGmssRat, rat value is %d", rat);
    LOGD("onGmssRat, mcc value is %d", mcc);
    LOGD("onGmssRat, attached value is %d", attached);
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_GMSS_RAT_CHANGED, response, sizeof(int)*3, rid);
}
/// M: [C2K][IR][MD-IRAT] URC for GMSS RAT changed. @}

extern int getWorldPhonePolicy()
{
    int  policy = 0;
    int  op01_policy = 0;
    char optr[PROPERTY_VALUE_MAX] = {0};
    char prop_val[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.operator.optr", optr, "");

    if (strcmp("OP01", optr) == 0) {
        property_get("ro.mtk_world_phone_policy", prop_val, "0");
        op01_policy = atoi(prop_val);
        LOGD("[getWorldPhonePolicy] op01_policy=%d", op01_policy);
        if (op01_policy == 0) {
            policy = 1;
        } else {
            policy = 2;
        }
    } else if (strcmp("OP02", optr) == 0) {
        policy = 4;
    } else {
        policy = 3;
    }

    LOGD("[getWorldPhonePolicy] WorldPhonePolicy=%d", policy);

    return policy;
}

// M: [LTE][Low Power][UL traffic shaping] Start
void onLteAccessStratumResult(char *urc, const RIL_SOCKET_ID rid)
{
    int ret;
    unsigned int response[2];
    char* tokenStr = NULL;
    int count = 1;

    LOGD("onLteAccessStratumResult:%s",urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get <lteAccessStratum_status>
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    if (response[0] == 0) { // 2G/3G/4G connected but no DRB exists/4G AS idle
        // get <Act>
        ret = at_tok_nextint(&urc, &response[1]);
        if (ret < 0) goto error;
        count = 2;
        response[1] = convertNetworkType(response[1]);
    }

    if (1 == count) {
        LOGD("onLteAccessStratumResult (<lteAccessStratum_status>=%d)", response[0]);
    } else {
        LOGD("onLteAccessStratumResult (<lteAccessStratum_status>=%d, <PS network type>=%d)", response[0], response[1]);
    }

    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, &response, count*sizeof(int), rid); // MTK defined UNSOL EVENT
    return;

error:
    LOGE("There is something wrong with the +EDRBSTATE");
}
// M: [LTE][Low Power][UL traffic shaping] End

/// M: EPDG feature. Update PS state from MAL @{
void requestMalGprsRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    /*
     * A +CGREG: answer may have a fifth value that corresponds
     * to the network type, as in;
     *
     * +CGREG: n, stat [,lac, cid [,networkType]]
     */

    /***********************************
    * In the mtk solution, we do not need to handle the URC
    * Since the URC will received in the URC channel.
    * So we don't need to follow the above command from google.
    * But we might return <AcT> for +CREG if the stat==2
    * while <lac> is present
    ************************************/

    /* +CGREG: <n>, <stat>, <lac>, <cid>, <Act> */

    int err;
    unsigned int response[6];
    char * responseStr[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    ATResponse *p_response = NULL;
    const char *cmd;
    const char *prefix;
    char *line, *str, *p;
    int commas;
    int skip, i;
    int count = 3;
    int psBearerCount = 0;
    int cell_data_speed = 0;
    int max_data_bearer = 0;
    int cause_type;

    // Don't change cgreg format
    int dontChangeCgregFormat = 1;

    //support carrier aggregation (LTEA)
    int ignoreMaxDataBearerCapability = 0;

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    /// M: EPDG feature. Query PS state from MAL @{
    int isIwlanOn = 0;
    /// @}

    //cmd = "AT+CGREG?";
    //prefix = "+CGREG:";

    /// M: EPDG feature. Query PS state from MAL @{
    str = (char *)calloc(1, sizeof(char) * 128);
    // return value of rild_rds_sdc_req
    // 0: successfuly
    // 1: error
    LOGD("Call mal_get_ps_reg_state()");
    err = mal_get_ps_reg_state(str, 128);
    //err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

    if (err != 0) {
        LOGE("requestMalGprsRegistrationState: mal_get_ps_reg_state() return fail");
        goto error;
    }
    LOGD("mal_get_ps_reg_state: line is %s", str);
    line = str;
    //if (err != 0 || p_response->success == 0 ||
    //        p_response->p_intermediates == NULL) goto error;

    //line = p_response->p_intermediates->line;
    /// @}

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <n> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;

    /* <stat> */
    err = at_tok_nextint(&line, (int*) &response[0]);

    /// M: EPDG feature. Query PS state from MAL @{
    if (response[0] == 99) {
        response[0] = 1; //registered
        isIwlanOn = 1;
        LOGD("requestMalGprsRegistrationState: IWLAN is On");
    }
    /// @}

    //if (err < 0 || response[0] > 5 )
    if (err < 0 || response[0] > 10 )  //for Lte
    {
        LOGE("The value in the field <stat> is not valid: %d", response[0] );
        goto error;
    }

    //For Lte
    response[0] = convertRegState(response[0] );

    /// M: EPDG feature. Query PS state from MAL @{
    if (isIwlanOn == 1) {
        response[1] = -1;
        response[2] = -1;
        response[3] = 18; //IWLAN
        count = 4;
    }
    else
    {
        // change to short GREG when <n> =3 and not registed
        if ((skip == 3) && (response[0] != 1 && response[0] != 5)) {
            dontChangeCgregFormat = 0;
            LOGD("dontChangeCgregFormat=0");
        }

        // if( at_tok_hasmore(&line) )
        if (at_tok_hasmore(&line) && dontChangeCgregFormat)
        {
            /* <lac> */
            LOGE("The value in the field <lac> :%d", response[1]);
            err = at_tok_nexthexint(&line, (int*)&response[1]);
            if ( err < 0 || (response[1] > 0xffff && response[1] != 0xffffffff) )
            {
                LOGE("The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
                     response[0], response[1] );
                goto error;
            }

            /* <cid> */
            err = at_tok_nexthexint(&line, (int*)&response[2]);
            LOGD("cid: %d", response[2] );
            if (err < 0 || (response[2] > 0x0fffffff && response[2] != 0xffffffff) )
            {
                LOGE("The value in the field <cid> is not valid: %d", response[2] );
                goto error;
            }

            /* <Act> */
            err = at_tok_nextint(&line, (int*)&response[3]);
            LOGE("The value of act: %d", response[3] );
            if (err < 0)
            {
                LOGE("No act in command");
                goto error;
            }
            count = 4;
            if (response[3] > 7)
            {
                LOGE("The value in the act is not valid: %d", response[3] );
                goto error;
            }

            /* mapping */
            response[3] = convertNetworkType(response[3]);

            if (at_tok_hasmore(&line))
            {
                /* <rac> */
                err = at_tok_nexthexint(&line, &skip);
                if (err < 0) goto error;

                if (at_tok_hasmore(&line))
                {
                    /* <cause_type> */
                    err = at_tok_nextint(&line, &cause_type);
                    LOGD("cause_type: %d", cause_type);
                    if (err < 0 || cause_type != 0)
                    {
                        LOGE("The value in the field <cause_type> is not valid: %d", cause_type );
                        goto error;
                    }

                    /* <reject_cause> */
                    err = at_tok_nextint(&line, (int*) &response[4]);
                    LOGD("reject_cause: %d", response[4]);
                    if (err < 0)
                    {
                        LOGE("The value in the field <reject_cause> is not valid: %d", response[4] );
                        goto error;
                    }
                    count = 5;
                }
            }

            /* maximum number of simultaneous Data Calls  */
            if (response[3] == 14)
                response[5] = max_pdn_support_4G;
            else
                response[5] = max_pdn_support_23G;
            count = 6;
        }
        else
        {
            /* +CGREG: <n>, <stat> */
        LOGE("it is short CREG CGREG");
            response[1] = -1;
            response[2] = -1;
            response[3] = 0;
            //BEGIN mtk03923 [20120119][ALPS00112664]
            count = 4;
            //END   mtk03923 [20120119][ALPS00112664]
        }

        // Query by +PSBEARER as PS registered and R7R8 support
        if (bPSBEARERSupport == 1 &&
                (response[0] == 1 || response[0] == 5)) {
            at_response_free(p_response);
            p_response = NULL;
            cmd = "AT+PSBEARER?";
            prefix = "+PSBEARER:";

            err = at_send_command_singleline(cmd, prefix, &p_response, NW_CHANNEL_CTX);

            if (err != 0 || p_response->success == 0 ||
                    p_response->p_intermediates == NULL) goto skipR7R8;

            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto skipR7R8;

            /* <cell_data_speed_support> */
            err = at_tok_nextint(&line, &cell_data_speed);
            if (err < 0) goto skipR7R8;
            psBearerCount++;

            // <max_data_bearer_capability> is only support on 3G
            if (cell_data_speed >= 0x1000){
                ignoreMaxDataBearerCapability = 1;
            }

            cell_data_speed = convertCellSppedSupport(cell_data_speed);

            /* <max_data_bearer_capability> */
            err = at_tok_nextint(&line, &max_data_bearer);
            if (err < 0) goto skipR7R8;
            psBearerCount++;

            if (!ignoreMaxDataBearerCapability) {
                max_data_bearer = convertPSBearerCapability(max_data_bearer);
            } else {
                max_data_bearer = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
            }
        }
    }

skipR7R8:

    asprintf(&responseStr[0], "%d", response[0]);
    if (response[1] != 0xffffffff)
        asprintf(&responseStr[1], "%x", response[1]);
    else
        asprintf(&responseStr[1], "-1");
    if (response[2] != 0xffffffff)
        asprintf(&responseStr[2], "%x", response[2]);
    else
        asprintf(&responseStr[2], "-1");

    if (psBearerCount == 2) {
        asprintf(&responseStr[3], "%d", (cell_data_speed > max_data_bearer)? cell_data_speed : max_data_bearer);
    } else {
        asprintf(&responseStr[3], "%d", response[3]);
    }

    if (count >= 5) {
        asprintf(&responseStr[4], "%d", response[4]);
    }

    if (count == 6) {
        asprintf(&responseStr[5], "%d", response[5]);
    }

    LOGE("rid: %d",rid);
    if (response[0] == 1 || response[0] == 5) {
        cleanCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    } else {
        setCurrentRestrictionState(RIL_RESTRICTED_STATE_PS_ALL, rid);
    }
    onRestrictedStateChanged(rid);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, count*sizeof(char*));

    at_response_free(p_response);
    for (i=0; i<count; ++i) {
        if (responseStr[i] != NULL)
        free(responseStr[i]);
    }
    /// M: EPDG feature. Query PS state from MAL @{
    free(str);
    /// @}
    return;

error:
    LOGE("requestMalGprsRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    /// M: EPDG feature. Query PS state from MAL @{
    free(str);
    /// @}
}

extern void onMalPsStateChanged(void *data, size_t datalen, RIL_Token t){
        int err;
        int stat;
        /// M: EPDG feature. Update PS state from MAL @{
        char *urc;
        RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
        urc = (char *)data;
        /// @}

        LOGD("onMalPsStateChanged: URC is %s", urc);

        err = at_tok_start(&urc);
        if (err < 0) goto error;

        err = at_tok_nextint(&urc, &stat);
        if (err < 0) goto error;


        /// M: EPDG feature. Update PS state from MAL @{
        if (stat == 99) {
            stat = 1; //registered
        } else if (stat == 98){
            stat = 0;
        } else {
            //for Lte
            stat = convertRegState(stat);
        }
        ril_mal_data_reg_status[rid] = stat;
        /// @}

        LOGD("onMalPsStateChanged: stat=%d",stat);

        ril_data_reg_status[rid] = stat;
        RIL_onUnsolicitedResponseSocket(
    RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, &stat, sizeof(stat), rid);

        /// M: EPDG feature. Update PS state from MAL @{
        //response to MAL
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        /// @}

        return;

    error:
        LOGE("onMalPsStateChanged:There is something wrong with the URC");
        /// M: EPDG feature. Update PS state from MAL @{
        //response to MAL
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        /// @}

}

extern void onPsbearerChangeForEpdg(char *urc, RIL_SOCKET_ID rid){
    RIL_onUnsolicitedResponseSocket(RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED, &ril_mal_data_reg_status[rid], sizeof(ril_mal_data_reg_status[rid]), rid);
}
/// @}
