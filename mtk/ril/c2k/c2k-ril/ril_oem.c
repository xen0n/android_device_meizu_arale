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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
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

#include <telephony/ril.h>
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
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "oem-support.h"
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#include "hardware/ccci_intf.h"

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RESET_MODE_ASSERT 31
#define RESET_MODE_RESET 32
#define TRIGGER_CDMA_RILD_NE 103

extern int arsithrehd;
extern int voicetype;
extern int invoicecall;
extern int localindatacall;
extern int inemergency;
extern PcModem_State s_PcModemState;
extern int waiting_localdata_disconn;
extern struct timeval TIMEVAL_0;
extern const char s_data_device_path[32];
extern int currentNetworkMode;
extern pthread_cond_t s_pcmodem_signalcond;
extern int s_md3_off;

extern void sendPincode();

/*Indicate current screen state, update in requestScreenState*/
int s_screenState = 1;

int processing_md5_cmd = 0;

static const struct timeval TIMEVAL_UPDATE_TIME = {60, 0};   /* 60second */
/*Indicate whether Nitztime callback is in process*/
static int s_triggerNitztime = 0;

static pthread_mutex_t s_pcmodem_signal_mutex = PTHREAD_MUTEX_INITIALIZER;

/* gps data of cp side, set by AT command */
static VIA_GPS_Data s_gps_data = { 0, 1, 1 };

/* gps data of factory mode */
static VIA_GPS_FACTORY_MODE_Data s_gps_fac_data = { 0, 0, 0, 0, 0, 1, 1 };

//clclient MODEM����ִ�в���
typedef struct
{
    const char* cmdPrefix;                //�����ַ�
    int  OverTime;                  //����ִ�г�ʱ��λms
    ATCommandType commandType;      //���������
    const char* responsePrefix;        //����ؽ��ǰ�?    int isCheckDie;                 //�������Ƿ�ִ�г�ʱ�Ƿ���Ҫ�������?0-����Ҫ��1����Ҫ
    RILChannelId portNo;               //����ͨ��,ͨ��0,1. .
} CTClient_CommandParam;

static const CTClient_CommandParam CTCLIENT_CommandParams[] =
{
    {"AT+CTA",                30,    NO_RESULT, NULL, 0, AT_CHANNEL},
    {"ATD",                     30,  NO_RESULT,  NULL,  0,AT_CHANNEL},

     // 5
    {"ATE0",                    3, NO_RESULT, NULL, 0,AT_CHANNEL},//not used, for via
    {"AT+CGMR",                 3, SINGLELINE, "+CGMR:",0,AT_CHANNEL},  //for via, change at
    {"AT+GSN",                  5, SINGLELINE, "+GSN:", 0,AT_CHANNEL},
    {"AT+GSN=",                 5, SINGLELINE, "+GSN:", 0,AT_CHANNEL},
    {"AT+CIMI",                 5, SINGLELINE, "",0,AT_CHANNEL},
    // 10
    {"AT+CMUT=",                3, NO_RESULT,  NULL,  0,AT_CHANNEL},//����
    {"AT+CSQ",                  3, SINGLELINE, "+CSQ:", 0,AT_CHANNEL},
    //*************network*********
    {"AT+CREG?",                5,   SINGLELINE, "+CREG:",0,AT_CHANNEL},
    {"AT+VSER?",                3,   SINGLELINE, "+VSER:", 0,AT_CHANNEL},   //for via, modified from CAD
    {"AT+ARSI=",            3,   NO_RESULT, NULL,   0,AT_CHANNEL},  //for via, change at

    // 15
    {"AT+SRVIND=",          3,   NO_RESULT, NULL,  0,AT_CHANNEL},
    {"AT+CDMAPARA?",            5,   SINGLELINE, "+CDMAPARA:",0,AT_CHANNEL},
    {"AT+NETPAR=",          5,   MULTILINE, "+NETPAR:",  0,AT_CHANNEL},     //for via, modify
    {"AT+CSYSMODE?",            3,   SINGLELINE, "+CSYSMODE:",0,AT_CHANNEL},
    {"AT+CSYSMODE=",            3,   NO_RESULT, NULL,0,AT_CHANNEL},

    // 20
    {"AT+ROAMST?",              5,   SINGLELINE, "+ROAMST:",0,AT_CHANNEL},
    {"AT+PRLS?",                5,   SINGLELINE, "+PRLS:",0,AT_CHANNEL},
    {"AT+VPRLID?",              5,   SINGLELINE, "+VPRLID:",0,AT_CHANNEL},  //for via, change at, need check???
    //*************secur*********
    {"AT+CPIN=?",               10, NO_RESULT, NULL,0,AT_CHANNEL},
    {"AT+CPIN?",                10, SINGLELINE, "+CPIN:",0,AT_CHANNEL},
    {"AT+CPIN=",            100,NO_RESULT, NULL,0,AT_CHANNEL},
    {"AT^CPIN?",                10, SINGLELINE, "^CPIN:",0,AT_CHANNEL},

    // 25
    //{"AT+CPIN=",  60, NO_RESULT, NULL,0,AT_CHANNEL},//AT+CPIN=<puk1>,<pin1>
    {"AT+CLCK=",            15, SINGLELINE, "+CLCK:",0,AT_CHANNEL},
    //{"AT+CLCK=\"%s\",%d,\"%s\",%d", 15, NO_RESULT, NULL, 0,AT_CHANNEL},
    //{"AT+CLCK=\"%s\",%d,\"%s\"",  6,  NO_RESULT, NULL, 0,AT_CHANNEL},
    //{"AT+CPWD=\"%s\",\"%s\",\"%s\"",60,   NO_RESULT, NULL, 0,AT_CHANNEL},

    //*************Call***********
    // 30
    {"AT+CDV=",             30, NO_RESULT, NULL,0,AT_CHANNEL},//��绰��Ҫ����Ƿ�����
    //{"AT+CDV=%s,1",               30, NO_RESULT, NULL,0,AT_CHANNEL},//��绰��Ҫ����Ƿ�����
    {"AT+CHV",                  6,  NO_RESULT, NULL,0,AT_CHANNEL},
    {"ATA",                     4,  NO_RESULT, NULL,0,AT_CHANNEL},//��������Ӧ��,����OK��NOCARRIER,�������?����CONNECT��NOCARRIER
    {"AT+CLCC",             15, MULTILINE, "+CLCC:",0,AT_CHANNEL},//�г���ǰ��·�б�

    // 35
    {"AT+CHLD=",                15, NO_RESULT, NULL,0,AT_CHANNEL},
    {"AT+VTS=",              3, NO_RESULT, NULL,0,AT_CHANNEL},//˫������
    {"AT+CFSH=",             3, NO_RESULT, NULL,0,AT_CHANNEL},//˫������
    //*************Voice**********
    {"AT+CMUT?",                 3, SINGLELINE, "+CMUT:",0,AT_CHANNEL},
    {"AT+SPEAKER=",            3, NO_RESULT, NULL,0,AT_CHANNEL},

    // 40
    {"AT+VOL=",                3, NO_RESULT, NULL,   0,AT_CHANNEL},
    //************SMS*************
    //{"AT+CPMS=",          25, SINGLELINE, "+CPMS:",0,AT_CHANNEL},
    {"AT+CMGR=",                20, SINGLELINE, "+CMGR:",0,AT_CHANNEL},
    {"AT+CMGRPDU=",         20, SINGLELINE, "+CRSM:",0,AT_CHANNEL},
    {"AT+CMGL=",                4,  SINGLELINE, "+CMGL:",1,AT_CHANNEL},

    //{"AT+CMGF=",          3, NO_RESULT, NULL,0,AT_CHANNEL},
    //{"AT+CNMI=",          3, NO_RESULT, NULL,0,AT_CHANNEL},
    {"AT+CMEE=",          3, NO_RESULT, NULL,0,AT_CHANNEL},
    //{"AT+CMEE?",          3, SINGLELINE, "+CMEE:",0,AT_CHANNEL},
    //{"AT+CMEE=?",          3, SINGLELINE, "+CMEE:",0,AT_CHANNEL},
    {"AT+CGMI",                 5, SINGLELINE, "VIA",0,AT_CHANNEL},
    {"AT^HCMGL=",         100, MULTILINE, "^HCMGL:",0,AT_CHANNEL},
    // 45
    //{"AT+CMGSEX=",    15, SINGLELINE, "+CMGS:",0,AT_CHANNEL},
    //{"%s\x01A",                   40, SINGLELINE, "+CMGS:",0,AT_CHANNEL},//��������Ҫ����Ƿ�����?    //{"\x01B",                 8,  NO_RESULT, NULL,0,AT_CHANNEL},//��������Ҫ����Ƿ�����?    //{"AT+CMGW=%d,%d",/*length, status*/35,    SINGLELINE, "+CMGW",0,AT_CHANNEL},
    //{"%s\x01A",                   20, SINGLELINE, "+CMGW:",0,AT_CHANNEL},//д������Ҫ����Ƿ�����?
    // 50
    {"AT+CMGD=",                20, NO_RESULT, NULL,0,AT_CHANNEL},
    //***************misc**********
    {"AT+CFUN=",                    5,  NO_RESULT, NULL,0,AT_CHANNEL},//(1)fun 0 close 1 wack up ,5,6,7,8 sleep
    {"AT^CPBR=?",                   18, SINGLELINE, "^CPBR:",0,AT_CHANNEL},

    // 55
    {"AT^CPBR=",                10, SINGLELINE, "^CPBR:",0,AT_CHANNEL},
    {"AT+CPBS=",                20, SINGLELINE, "+CPBS:",0,AT_CHANNEL},
    {"AT+CPBS?",                    60, SINGLELINE, "+CPBS:",0,AT_CHANNEL},
    {"AT^CPBW=?",              60, SINGLELINE, "^CPBW:",0,AT_CHANNEL},
    {"AT^CPBW=",                    20, NO_RESULT, NULL,0,AT_CHANNEL},//ɾ����?    //{"AT^CPBW=%d,\"%s\",%d,\"%s\",%d",    10, NO_RESULT, NULL,0,AT_CHANNEL},//д��¼

    // 60
    //{"AT^CPBW=",  20, NO_RESULT, NULL,0,AT_CHANNEL},//��д��¼
    {"AT+CRSM=",                  20,   SINGLELINE, "+CRSM:",0,AT_CHANNEL},//
    //{"AT+CRSM=%d,%d",               20,   SINGLELINE, "+CRSM:",0,AT_CHANNEL},//
    //{"AT+CRSM=%d,%d,%d,%d,%d",      20,   SINGLELINE, "+CRSM:",0,AT_CHANNEL},//
    //{"AT+CRSM=%d,%d,%d,%d,%d,\"%s\"", 20, SINGLELINE, "+CRSM:",0,AT_CHANNEL},//

    // 65
    {"AT+CSIM=",               20,  SINGLELINE, "+CSIM:",0,AT_CHANNEL},//
    {"AT+UTSS",                    10, NO_RESULT, NULL,0,AT_CHANNEL},//
    {"AT+UTGI=",                 10, SINGLELINE, "+UTGI",0,AT_CHANNEL},//
    {"AT+UTGR=",                 10, NO_RESULT, NULL,0,AT_CHANNEL},//
    //{"AT+UTGR=%d,%d",              10, NO_RESULT, NULL,0,AT_CHANNEL},//

    // 70
    //{"AT+UTGR=%d,\"%s\"",          10, NO_RESULT, NULL,0,AT_CHANNEL},//
    {"AT+CCID?",                   3, SINGLELINE, "+CCID:", 0, AT_CHANNEL},
    {"AT+CGMM",                    3, SINGLELINE,    "CBP",    0, AT_CHANNEL},
    {"AT+CCLK?",                    3, SINGLELINE,    "+CCLK:",    0, AT_CHANNEL},  //for via, change AT
    {"AT+UIMID",                   3, SINGLELINE,    "+UIMID:",    0, AT_CHANNEL},

    //75 for GPS
    {"At+GPSVER=",               3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"At+ALTOPENOK",               3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT+ALTCLOSEOK",              3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT+GPSPPPCNT=",            3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT+GPSPPPDISC=",           3, NO_RESULT, NULL, 0, AT_CHANNEL},

    //80
    {"AT+GPSNOTIFY=",            3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT+GPSLOCRES=",            3, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT+RFCAL?",                  3, SINGLELINE, "+RFCAL:", 0, AT_CHANNEL},
    {"AT+SETAUDIO?",               3, SINGLELINE, "+AUDIO:", 0, AT_CHANNEL},

    //c+w zfj add 2010.08.24
    {"AT^CAVE=?",                 5, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT^CAVE=",                 5, SINGLELINE, "^CAVE:", 0, AT_CHANNEL},
    //85
    {"AT^MD5=?",               5, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT^MD5=",               5, NO_RESULT, NULL, 0, AT_CHANNEL},
    {"AT^MODE",                    5, SINGLELINE, "^MODE:", 0, AT_CHANNEL},
    {"AT^VPM",                     5, SINGLELINE, "^VPM:", 0, AT_CHANNEL},
    {"AT^SSDUPDCFM=",            5, NO_RESULT, NULL, 0, AT_CHANNEL},      //SSD����ȷ������
    {"AT^SSDUPD=",               5, SINGLELINE, "^SSDUPD:", 0, AT_CHANNEL}, //SSD��������
    //90
    {"AT+DATASTATS",               5, SINGLELINE, "+DATASTATS:", 0, AT_CHANNEL}, //
    {"AT+CARDTYPE=",            18, SINGLELINE, "+CARDTYPE:",  0, AT_CHANNEL}, //

    //added for via cbp, begin from 92
    //{"AT+CPOF",                   15, NO_RESULT, NULL,        0, AT_CHANNEL}, //
    //{"AT+CPON",                   15, NO_RESULT, NULL,        0, AT_CHANNEL}, //
    {"AT^HDRCSQ=?",                  5, SINGLELINE, "^HDRCSQ:", 0, AT_CHANNEL}, //
    {"AT^HDRCSQ",                  5, SINGLELINE, "^HDRCSQ:",   0, AT_CHANNEL}, //
    //95
    {"AT^SYSINFO",                 5, SINGLELINE, "^SYSINFO:",  0, AT_CHANNEL},
    {"AT^PREFMODE=?",              5, SINGLELINE, "^PREFMODE:",  0, AT_CHANNEL},   //����ѡ��
    {"AT^PREFMODE?",               5, SINGLELINE, "^PREFMODE:",  0, AT_CHANNEL},   //����ѡ��
    {"AT^PREFMODE=",               5, NO_RESULT, NULL,  0, AT_CHANNEL},   //����ѡ��
    {"AT+CSNID?",                  5, SINGLELINE, "+CSNID:",    0, AT_CHANNEL},
    {"AT+VROM?",                   5, SINGLELINE, "+VROM:",     0, AT_CHANNEL},
    {"AT+COPS?",                   5, SINGLELINE, "+COPS:",     0, AT_CHANNEL},

    //100
    {"ATH",                       5,  NO_RESULT, NULL,  0, AT_CHANNEL}, //
    {"AT+VTC=1,",              5,  NO_RESULT, NULL,  0, AT_CHANNEL}, //
    {"AT+VTC=0,",              5,  NO_RESULT, NULL,  0, AT_CHANNEL}, //
    {"AT+VMCCMNC?",              5,  SINGLELINE, "+VMCCMNC",  0, AT_CHANNEL}, //
    {"AT+VPON?",              5,  SINGLELINE, "+VPON",  0, AT_CHANNEL}, //
    //105
    {"AT^MEID",                  5, SINGLELINE, "^MEID:",  0, AT_CHANNEL},
    {"AT+VMDN?",                5, SINGLELINE, "+VMDN:",  0, AT_CHANNEL},
    {"AT+VMIN?",                5, SINGLELINE, "+VMIN:",  0, AT_CHANNEL},
    {"AT+CPINC?",               5, MULTILINE, "+CPINC:",  0, AT_CHANNEL},
    //110
    {"AT+DATACNT?",             5, SINGLELINE, "+DATACNT:",  0, AT_CHANNEL},
    {"AT^DSDORMANT",                5, SINGLELINE, "^DSDORMANT:",  0, AT_CHANNEL}, //����ѡ��
    {"AT^UIMAUTH=?",                5, SINGLELINE, "^UIMAUTH:",  0, AT_CHANNEL},
    {"AT^GSN=?",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"AT^GSN",                5, SINGLELINE, "^GSN:",  0, AT_CHANNEL},
    {"AT^HWVER",                5, SINGLELINE, "^HWVER:",  0, AT_CHANNEL},
    {"AT^RESET",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"AT+UTRFSH",                     5,  NO_RESULT,  NULL,  0,AT_CHANNEL},
    {"AT+FCLASS=?", 5, SINGLELINE, "+FCLASS:", 0, AT_CHANNEL},
    {"ATS0=0",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"ATQ0E0V1",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"ATQ0V1E0",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"ATZE0Q0V1",                5, NO_RESULT, NULL,  0, AT_CHANNEL},
    {"AT+GMM",  5, SINGLELINE, "+GMM:", 0, AT_CHANNEL},
    {"ATI",         5, MULTILINE, "+GMI:", 0, AT_CHANNEL},      //report cp platform infomation
};

static void requestScreenState(void *data, size_t datalen, RIL_Token t);
static void requestIdentity(void *data, size_t datalen, RIL_Token t);
static void requestPhoneTestInfo( void *data,size_t datalen, RIL_Token t);
static void trigger_nitztime_report();
static void requestSetVoiceRecord(void *data, size_t datalen, RIL_Token t);
static void requestCoolResetGps(void *data, size_t datalen, RIL_Token t);
static void requestSetAudioPath(void *data, size_t datalen, RIL_Token t);
static void requestSetVoiceVolume(void *data, size_t datalen, RIL_Token t);
static void requestPlayToneSeq(void *data, size_t datalen, RIL_Token t);
static void requestPlayDTMFTone(void *data, size_t datalen, RIL_Token t);
static void requestAgpsTcpConnected(void *data, size_t datalen, RIL_Token t);
static void requestAgpsGetMpcIpPort(void *data, size_t datalen, RIL_Token t);
static void requestAgpsSetMpcIpPort(void *data, size_t datalen, RIL_Token t);
static void requestSetEtsWake(void *data, size_t datalen, RIL_Token t);
static void requestSetGps(void *data, size_t datalen, RIL_Token t);
static void requestSetTrm(void *data, size_t datalen, RIL_Token t);
static void requestSendCommandForCTClient(void * data, size_t datalen, RIL_Token t);
static void requestOemHookStrings(void * data, size_t datalen, RIL_Token t);
static void requestOemHookRaw(void * data, size_t datalen, RIL_Token t);
static void requestConfigEvdoMode(void * data, size_t datalen, RIL_Token t);
/* Power on/off MD notified by thermal service, 2015/07/08 {*/
static void requestSetThermalModem(void *data, size_t datalen, RIL_Token t);
/* Power on/off MD notified by thermal service, 2015/07/08 }*/

int rilOemMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_SCREEN_STATE:
            requestScreenState(data, datalen, t);
            break;
        case RIL_REQUEST_OEM_HOOK_RAW:
            // echo back data
            LOGD("rilOemMain, RIL_REQUEST_OEM_HOOK_RAW");
            requestOemHookRaw(data, datalen, t);
            //RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS: {
            int i;
            const char ** cur;

            LOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long) datalen);

            for (i = (datalen / sizeof(char *)), cur = (const char **) data; i > 0;
                    cur++, i--) {
                LOGD("> '%s'", *cur);
            }
            requestOemHookStrings(data, datalen, t);
            }
            break;
        case RIL_REQUEST_DEVICE_IDENTITY:
            requestIdentity(data, datalen, t);
            break;
        case RIL_REQUEST_PHONE_TEST_INFO:
            requestPhoneTestInfo(data,datalen,t);
            break;
        case RIL_REQUEST_SET_ETS_DEV:
            requestSetEtsDev(data, datalen, t);
            break;
        case RIL_REQUEST_SET_VOICERECORD:
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_SET_VOICE_RECORD:
#endif
            requestSetVoiceRecord(data, datalen, t);
            break;
        case RIL_REQUEST_GPS_START:
            requestCoolResetGps(data, datalen, t);
            break;
        case RIL_REQUEST_SET_AUDIO_PATH:
            requestSetAudioPath(data, datalen, t);
            break;
        case RIL_REQUEST_SET_GPS:
            requestSetGps(data, datalen, t);
            break;
        case RIL_REQUEST_SET_ETS_WAKE:
            requestSetEtsWake(data, datalen, t);
            break;
        case RIL_REQUEST_SET_TRM:
            requestSetTrm(data, datalen, t);
            break;
#ifdef ADD_MTK_REQUEST_URC
        case RIL_REQUEST_SET_VOICE_VOLUME:
            requestSetVoiceVolume(data, datalen, t);
            break;
        case RIL_REQUEST_PLAY_DTMF_TONE:
            requestPlayDTMFTone(data, datalen, t);
            break;
        case RIL_REQUEST_PLAY_TONE_SEQ:
            requestPlayToneSeq(data, datalen, t);
            break;
#endif /* ADD_MTK_REQUEST_URC */
        case RIL_REQUEST_AGPS_TCP_CONNIND:
            requestAgpsTcpConnected(data, datalen, t);
            break;

        case RIL_REQUEST_AGPS_SET_MPC_IPPORT:
            requestAgpsSetMpcIpPort(data, datalen, t);
            break;

        case RIL_REQUEST_AGPS_GET_MPC_IPPORT:
            requestAgpsGetMpcIpPort(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT:
            requestSendCommandForCTClient(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIG_EVDO_MODE:
            requestConfigEvdoMode(data, datalen, t);
            break;
        case RIL_REQUEST_GET_RADIO_CAPABILITY:
            requestGetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            requestSetRadioCapability(data, datalen, t);
            break;
        /* Power on/off MD notified by thermal service, 2015/07/08 {*/
        case RIL_REQUEST_SET_MODEM_THERMAL:
            requestSetThermalModem(data, datalen, t);
            break;
        /* Power on/off MD notified by thermal service, 2015/07/08 }*/
        default:
            return 0; /* no matched request */
    }
    return 1;
}

int rilOemUnsolicited(const char *s, const char *sms_pdu)
{
    char *line = NULL;

    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    char magicFlag[PROPERTY_VALUE_MAX] = { 0 };
    property_get("net.ctclientd", tempstr, "0");
    property_get("net.cdma.via.at.reset", magicFlag, "disable");
    LOGD("net.ctclientd = %s, at.reset = %s", tempstr, magicFlag);

    if (strStartsWith(s, "+VPUP"))
    {
        // Notify +VPUP received from MD3 to mainloop.
        notifyMd3InitDone();
        return 1;
    }
    else if (strStartsWith(s, "^GPSTCPCONNREQ"))
    {
        VIA_GPS_EVENT_Data gps_event_data;
        gps_event_data.event = REQUEST_DATA_CONNECTION;
        gps_event_data.gps_status = 0;
        RIL_onUnsolicitedResponse(RIL_UNSOL_VIA_GPS_EVENT, &gps_event_data,
                sizeof(gps_event_data));
        return 1;
    }
    else if (strStartsWith(s, "^GPSTCPCLOSEREQ"))
    {
        VIA_GPS_EVENT_Data gps_event_data;
        gps_event_data.event = CLOSE_DATA_CONNECTION;
        gps_event_data.gps_status = 0;
        RIL_onUnsolicitedResponse(RIL_UNSOL_VIA_GPS_EVENT, &gps_event_data,
                sizeof(gps_event_data));
        return 1;
    }
#ifndef AT_AUTO_SCRIPT_RESET
    else if (!strStartsWith(s, "OK") && !strStartsWith(s, "NO CARRIER")
            && !strStartsWith(s, "ERROR") && !strStartsWith(s, "+UTK")
            && !strcmp(tempstr, "enable")) {
        asprintf(&line, "\r\n%s\r\n", s);
        RIL_onUnsolicitedResponse(RIL_UNSOL_FOR_CTCLIENT, line, strlen(line));
        free(line);
        line = NULL;
        return 1;
    }
#else /* AT_AUTO_SCRIPT_RESET */
    if (!strcmp(tempstr,"enable"))
    {
        if (strncmp(magicFlag, "enable", 6))
        {
            if (!strStartsWith(s, "OK") && !strStartsWith(s, "ERROR") && !strStartsWith(s, "+UTK"))
            {
                asprintf(&line, "\r\n%s\r\n", s);
                RIL_onUnsolicitedResponse(RIL_UNSOL_FOR_CTCLIENT, line, strlen(line));
                free(line);
                line = NULL;
            }
        }
        else
        {
            if (strStartsWith(s, "^MODE") && !atResetMode)
            {
                asprintf(&line, "\r\n%s\r\n", s);
                RIL_onUnsolicitedResponse(RIL_UNSOL_FOR_CTCLIENT, line, strlen(line));
                free(line);
                line = NULL;
                atResetMode = 1;
            }
        }
        return 1;
    }
#endif
    return 0;
}


static void requestScreenState(void *data, size_t datalen, RIL_Token t)
{
  int err = 0;
  char *cmd;
  char tempstr[PROPERTY_VALUE_MAX] = {0};
  int pagingoffmode = 0;
  property_get("persist.radio.via.paging.off", tempstr, "0");
  LOGD("requestScreenState: persist.radio.via.paging.off = %s", tempstr);

    pagingoffmode = atoi(tempstr);
  assert (datalen >= sizeof(int *));
  s_screenState = ((int*)data)[0];
  LOGD("requestScreenState");
  if(s_screenState == 1)
  {
      LOGD("requestScreenState on");
      //system("echo LCD > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_on");

      //err = at_send_command("AT+ARSI=1,4", NULL, OEM_CHANNEL_CTX);
      asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
      err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      err = at_send_command("AT+VMCCMNC=1", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      //err = at_send_command("AT+SAMEVSER=1", NULL);
      //if (err < 0) goto error;
      err = at_send_command("AT+CREG=1", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      //err = at_send_command("AT+SAMECREG=1", NULL);
      //if (err < 0) goto error;
      //err = at_send_command("AT+SAMEVMCCMNC=1", NULL);
      //if (err < 0) goto error;
      err = at_send_command("AT+MODE=1", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      err = at_send_command("AT+DORMONOFF=1", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      err = at_send_command("AT+CIEV=1,106", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      if(pagingoffmode) {
      err = at_send_command("AT+VDATADISABLE=0", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      }
      err = at_send_command("AT+VSER=1", NULL, OEM_CHANNEL_CTX);
      if(err < 0) goto error;
      trigger_update_cache();
#if 0
      if(!s_triggerNitztime)        //If nitztime callback is inprogress, don't trigger again
      trigger_nitztime_report(t);
#endif
      at_send_command("AT+CCLK?", NULL, OEM_CHANNEL_CTX);
      RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0); //by Hyan, query network status again once screen is on
  }
  else if(s_screenState == 0)
  {
      LOGD("requestScreenState off");
     // system("echo LCD > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_off");

      //err = at_send_command("AT+ARSI=0,4", NULL);
      asprintf(&cmd, "AT+ARSI=0,%d", arsithrehd);
      err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      err = at_send_command("AT+VSER=0", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      //err = at_send_command("AT+SAMEVSER=0", NULL);
      //if (err < 0) goto error;
      err = at_send_command("AT+CREG=0", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      //err = at_send_command("AT+SAMECREG=0", NULL);
      //if (err < 0) goto error;
      err = at_send_command("AT+VMCCMNC=0", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      err = at_send_command("AT+CIEV=0,106", NULL, OEM_CHANNEL_CTX);
      if (err < 0) goto error;
      //err = at_send_command("AT+SAMEVMCCMNC=0", NULL);
      //if (err < 0) goto error;
      #ifdef AT_AUTO_SCRIPT_MODE
      if(!ctclientServiceMode)
        err = at_send_command("AT+MODE=0", NULL, OEM_CHANNEL_CTX);
      #else
        err = at_send_command("AT+MODE=0", NULL, OEM_CHANNEL_CTX);
      #endif
      if (err < 0) goto error;
      if(!CmpPcModemMode())
      {
          err = at_send_command("AT+DORMONOFF=0", NULL, OEM_CHANNEL_CTX);
          if (err < 0) goto error;
          if(pagingoffmode) {
              err = at_send_command("AT+VDATADISABLE=1", NULL, OEM_CHANNEL_CTX);
              if (err < 0) goto error;
          }
      }
  }
  else
  {
      /* Not a defined value - error */
      goto error;
  }
  RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

  return;

error:
  LOGE("ERROR: requestScreenState failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestIdentity(void *data, size_t datalen, RIL_Token t)
{
    RIL_Identity response;
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_response_imei = NULL;
    ATResponse *p_responsenext = NULL;
    ATResponse *p_responseuimid = NULL;
    ATLine *p_cur;
    char *line;
    char *skip = NULL;
    char *line_imei;
    char *linenext;
    char *lineuimid;
    memset(&response, 0, sizeof(response));

    err = at_send_command_singleline("AT+VGMUID?", "+VGMUID:", &p_response_imei, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response_imei->success == 0))
    {
        goto error;
    }

    line_imei = p_response_imei->p_intermediates->line;
    err = at_tok_start(&line_imei);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line_imei, &skip);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line_imei, &response.imei);
    if(err < 0)
    {
        goto error;
    }

    if(!strncmp(response.imei, "0x", 2))
    {
        response.imei = response.imei + 2;
    }


    err = at_send_command_singleline ("AT+GSN", "+GSN:", &p_response, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &response.esn);
    if(err < 0)
    {
        goto error;
    }

    err = at_send_command_singleline ("AT^MEID", "^MEID:", &p_responsenext, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_responsenext->success == 0))
    {
        goto error;
    }
    linenext = p_responsenext->p_intermediates->line;
    err = at_tok_start(&linenext);
    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextstr(&linenext, &response.meid);
    if (err < 0)
    {
        goto error;
    }
    #ifdef ADD_MTK_REQUEST_URC
    if(!strncmp(response.meid, "0x", 2))
    {
        response.meid = response.meid + 2;
    }
    #endif
    err = at_send_command_singleline ("AT+CCID?", "+CCID:", &p_responseuimid, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_responseuimid->success == 0))
    {
        goto error;
    }
    lineuimid = p_responseuimid->p_intermediates->line;
    err = at_tok_start(&lineuimid);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&lineuimid, &response.uimid);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);
    at_response_free(p_response_imei);
    at_response_free(p_responsenext);
    at_response_free(p_responseuimid);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    at_response_free(p_response_imei);
    at_response_free(p_responsenext);
    at_response_free(p_responseuimid);
}

static void requestPhoneTestInfo( void *data,size_t datalen, RIL_Token t)
{
    RIL_Phone_Test_Info response;
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_response_dbg = NULL;
    ATResponse *p_response_field_active = NULL;
    ATResponse *p_response_field_neighbor = NULL;
    ATResponse *p_response_field_alternate = NULL;
    char *line = NULL;
    char *skip= NULL;

    memset(&response,0,sizeof(RIL_Phone_Test_Info));

    /*get phone test info:bt_status,ft_status,meid_status*/
    err = at_send_command_singleline ("AT+VPHONEINFO?", "+VPHONEINFO:", &p_response, OEM_CHANNEL_CTX);
    if((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    //if (err < 0) goto error;
    err = at_tok_nextstr(&line, &skip);
    //if (err < 0)   goto error;
    err = at_tok_nextstr(&line, &skip);
    //if (err < 0)   goto error;
    err = at_tok_nextstr(&line, &response.bt_status);
    if (err < 0){
        strcpy(response.bt_status,"0");
    }
    err = at_tok_nextstr(&line, &response.ft_status);
    if (err < 0){
        strcpy(response.ft_status,"0");
    }
    err = at_tok_nextstr(&line, &response.meid_status);
    if (err < 0){
        strcpy(response.meid_status,"0");
    }

    LOGD("phone test info: bt=%s, ft=%s, meid=%s \n  ",response.bt_status,
        response.ft_status,response.meid_status);

    if(strcmp(response.bt_status,"48")==0){
        strcpy(response.bt_status,"0");
    }else if (strcmp(response.bt_status,"49")==0){
        strcpy(response.bt_status,"1");
    }
    if(strcmp(response.ft_status,"48")==0){
        strcpy(response.ft_status,"0");
    }else if (strcmp(response.ft_status,"49")==0){
        strcpy(response.ft_status,"1");
    }
    if(strcmp(response.meid_status,"48")==0){
        strcpy(response.meid_status,"0");
    }else if (strcmp(response.meid_status,"49")==0){
        strcpy(response.meid_status,"1");
    }

    LOGD("phone test info: bt=%s, ft=%s, meid=%s \n  ",response.bt_status,
        response.ft_status,response.meid_status);

    /*get debug info:CH, FER, TXP*/
    line = NULL;
    skip = NULL;
    err = at_send_command_singleline ("AT+VDEBINFO?", "+VDEBINFO:", &p_response_dbg, OEM_CHANNEL_CTX);
    if((err != 0) || (p_response_dbg->success == 0))
    {
        goto error;
    }

    line = p_response_dbg->p_intermediates->line;
    err = at_tok_start(&line);
    err = at_tok_nextstr(&line, &skip);
    err = at_tok_nextstr(&line, &skip);
    err = at_tok_nextstr(&line, &response.current_frequency_channel);
    if (err < 0){
        LOGD("get  current_frequency_channel failed\n  ");
        strcpy(response.bt_status,"0");
    }
    err = at_tok_nextstr(&line, &response.current_band_class);
    if (err < 0){
        LOGD("get  current_band_class failed\n  ");
        strcpy(response.current_band_class,"0");
    }
    err = at_tok_nextstr(&line, &response.frame_error_rate);
    if (err < 0){
        LOGD("get  frame_error_rate failed\n  ");
        strcpy(response.frame_error_rate,"0");
    }
    err = at_tok_nextstr(&line, &response.tx_power);
    if (err < 0){
        LOGD("get  tx_power failed\n  ");
        strcpy(response.tx_power,"0");
    }

    LOGD("debug info: ch=%s, bc=%s, fer=%s, txp=%s \n  ",response.current_frequency_channel,
        response.current_band_class,response.frame_error_rate, response.tx_power);

    /*get active field test info*/
    err = at_send_command_singleline ("AT+VFIELDTST=1", "+VFIELDTST:", &p_response_field_active, OEM_CHANNEL_CTX);
    if((err != 0) || (p_response_field_active->success == 0))
    {
        goto error;
    }

    response.vfield_tst_info_active = p_response_field_active->p_intermediates->line;
    err = at_tok_start(&response.vfield_tst_info_active);
    if (err < 0){
        LOGD("get active field test info failed\n  ");
        strcpy(response.vfield_tst_info_active,"0");
    }
    LOGD("active field test info=%s \n  ",response.vfield_tst_info_active);

    /*get neighbor field test info*/
    err = at_send_command_singleline ("AT+VFIELDTST=2", "+VFIELDTST:", &p_response_field_neighbor, OEM_CHANNEL_CTX);
    if((err != 0) || (p_response_field_neighbor->success == 0))
    {
        goto error;
    }

    response.vfield_tst_info_neighbor = p_response_field_neighbor->p_intermediates->line;
    err = at_tok_start(&response.vfield_tst_info_neighbor);
    if (err < 0){
        LOGD("get neighbor field test info failed\n  ");
        strcpy(response.vfield_tst_info_neighbor,"0");
    }
    LOGD("neighbor field test info=%s \n  ",response.vfield_tst_info_neighbor);

    /*get alternate field test info*/
    err = at_send_command_singleline ("AT+VFIELDTST=3", "+VFIELDTST:", &p_response_field_alternate, OEM_CHANNEL_CTX);
    if((err != 0) || (p_response_field_alternate->success == 0))
    {
        goto error;
    }

    response.vfield_tst_info_alternate = p_response_field_alternate->p_intermediates->line;
    err = at_tok_start(&response.vfield_tst_info_alternate);
    if (err < 0){
        LOGD("get alternate field test info failed\n  ");
        strcpy(response.vfield_tst_info_alternate,"0");
    }
    LOGD("alternate field test info=%s \n  ",response.vfield_tst_info_alternate);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Phone_Test_Info));
    at_response_free(p_response);
    at_response_free(p_response_dbg);
    at_response_free(p_response_field_active);
    at_response_free(p_response_field_neighbor);
    at_response_free(p_response_field_alternate);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    at_response_free(p_response_dbg);
    at_response_free(p_response_field_active);
    at_response_free(p_response_field_neighbor);
    at_response_free(p_response_field_alternate);
}

static void trigger_nitztime_report(RIL_Token t)
{
    LOGD("trigger_nitztime_report");
    if(s_screenState) {
        at_send_command("AT+CCLK?", NULL, OEM_CHANNEL_CTX);                     //Trigger NITZ Time unsolicited reporting
        RIL_requestProxyTimedCallback(trigger_nitztime_report, NULL,
                &TIMEVAL_UPDATE_TIME, getDefaultChannelCtx()->id);
        s_triggerNitztime = 1;
    } else {
        s_triggerNitztime = 0;
    }
}

static void requestSetVoiceRecord(void *data, size_t datalen, RIL_Token t)
{
    int err;
    char *cmd;

    assert(datalen >= sizeof(int *));
    LOGD("requestSetVoiceRecord ((int*)data)[0] = %d", ((int*) data)[0]);
    asprintf(&cmd, "AT+CSVOC=%d,1", ((int*) data)[0]);
    err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
    free(cmd);

    if (err != 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    return;

error:
    LOGE("ERROR: requestSetVoiceRecord failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

static void requestCoolResetGps(void *data, size_t datalen, RIL_Token t)
{
    int err;
    char *cmd;

    assert(datalen >= sizeof(int *));
    LOGD("requestCoolResetGps");
    // asprintf(&cmd, "AT^GPSSTART=3", ((int*)data)[0]);//fix me
    asprintf(&cmd, "AT^GPSSTART=3"); //fix me
    err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
    free(cmd);

    if (err != 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    return;

error:
    LOGE("requestCoolResetGps failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetAudioPath(void *data, size_t datalen, RIL_Token t)
{
    char *cmd;
    int ret;
    voicetype = ((int*) data)[0];
    LOGD("requestSetAudioPath ((int*)data)[0] = %d, invoicecall = %d",
            ((int*) data)[0], invoicecall);
    asprintf(&cmd, "AT+SPEAKER=%d", voicetype);
    ret = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
    free(cmd);
    if (ret != 0)
        goto error;
    /**
     * For reducing pop noise
     * now cp is ready ,finish the audio path routing
     */
    if (invoicecall) {
#if 0
        set_twl5030_path(voicetype);
#endif
    } else if (inemergency) {
#if 0
        set_twl5030_path(voicetype);
#endif
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    LOGE("ERROR: requestSetAudioPath failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetVoiceVolume(void *data, size_t datalen, RIL_Token t)
{
    char *cmd = NULL;
    int ret = 0;

    voicetype = ((int *) data)[0];

    LOGE("voicetype=%d, invoicecall=%d", voicetype, invoicecall);
    asprintf(&cmd, "AT+VGR=%d", voicetype);
    ret = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);

    if (cmd)
    {
        free(cmd);
        cmd = NULL;
    }

    if (ret != 0)
        goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("ERROR: %s failed!", __func__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    return;
}

static void dtmfIndexparser(int index, char* dtmf)
{
    char lists[] = "0123456789*#";

    if (NULL == dtmf || index > 11)
    {
        LOGD("%s: parameter error! index = %d", __FUNCTION__, index);
        return;
    }
    LOGD("index =%d", index);
    sprintf(dtmf, "%c", lists[index]);

    return;
}

static void requestPlayDTMFTone(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_Play_DTMF_Tone* dtmfTone = NULL;
    char dtmf[4] = { 0 };
    char *cmd = NULL;
    int err = 0;

    dtmfTone = (RIL_Play_DTMF_Tone*) data;

    if (1 == dtmfTone->mode)
    {
        dtmfIndexparser(dtmfTone->dtmfIndex, dtmf);
        asprintf(&cmd, "AT+CDTMF=%d, \"%s\", %d, %d", dtmfTone->mode, dtmf,
                dtmfTone->volume, dtmfTone->duration);
    }
    else if (0 == dtmfTone->mode)
    {
        asprintf(&cmd, "AT+CDTMF=%d", dtmfTone->mode);
    }
    else
    {
        LOGD("%s: invalid mode = %d", __FUNCTION__, dtmfTone->mode);
        goto error;
    }

    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    LOGE("ERROR: %s failed!", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}
static void requestPlayToneSeq(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_Play_Tone_Seq* toneSeq = NULL;
    char toneCmd[128] = { 0 };
    int err = 0;
    int i = 0;

    toneSeq = (RIL_Play_Tone_Seq*) data;
    if (toneSeq->num <= 0 || toneSeq->num > MAX_TONE_DATA_NUM)
    {
        LOGD("%s: invalid num is %d", __FUNCTION__, toneSeq->num);
        goto error;
    }

    sprintf(toneCmd, "AT+CTONESEQ=%d, %d, %d", toneSeq->num,
            toneSeq->interation, toneSeq->volume);
    for (i = 0; i < toneSeq->num; i++)
    {
        sprintf(toneCmd, "%s, %d, %d, %d", toneCmd,
                toneSeq->toneData[i].duration, toneSeq->toneData[i].freq[0],
                toneSeq->toneData[i].freq[1]);
    }
    LOGD("toneCmd is %s", toneCmd);
    err = at_send_command(toneCmd, &p_response, OEM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    LOGE("ERROR: %s failed!", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

static void requestAgpsTcpConnected(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int connected = ((int*) data)[0];

    asprintf(&cmd, "AT^GPSTCPCONNIND=%d", connected);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestAgpsSetMpcIpPort(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    const char * ip = ((char**) data)[0];
    const char * port = ((char**) data)[1];

    asprintf(&cmd, "AT^GPSMPCSET=\"%s\",%s", ip, port);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
    error: at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestAgpsGetMpcIpPort(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * response[2];

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline("AT^GPSMPCSET?", "^GPSMPCSET:",
            &p_response, OEM_CHANNEL_CTX);
    if ((err < 0) || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextstr(&line, &(response[0]));
    if (err < 0)
    {
        goto error;
    }

    err = at_tok_nextstr(&line, &(response[1]));
    if (err < 0)
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    return;
    error: at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}


static void requestOemHookRaw(void * data, size_t datalen, RIL_Token t)
{
    /* atci start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line;
    int i;
    int strLength = 0;
    int err = -1;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("data = %s, length = %d", buffer, datalen);


    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {
        RLOGE("OEM_HOOK_RAW fail");
        goto error;
    }

    RLOGD("p_response->success = %d", p_response->success);
    RLOGD("p_response->finalResponse = %s", p_response->finalResponse);

    strLength += 2; //for the pre tag of the first string in response.

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        RLOGD("p_response->p_intermediates = <%s>", p_cur->line);
        strLength += (strlen(p_cur->line) + 2); //M:To append \r\n
    }
    strLength += (strlen(p_response->finalResponse) + 2);
    RLOGD("strLength = %d", strLength);

    int size = strLength * sizeof(char) + 1;
    line = (char *) alloca(size);
    memset(line, 0, size);

    strcpy(line, "\r\n");

        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
       strcat(line, p_cur->line);
       strcat(line, "\r\n");
       RLOGD("line[%d] = <%s>", i, line);
    }
    strcat(line, p_response->finalResponse);
    strcat(line, "\r\n");
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, size);

     at_response_free(p_response);
     return;

error:
    line = (char *) alloca(10);
    memset(line, 0, 10);
    strcpy(line, "\r\nERROR\r\n");
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));

    at_response_free(p_response);
    return;
}


static void requestOemHookStrings(void * data, size_t datalen, RIL_Token t)
{
    int i;
    const char ** cur;
    ATResponse *    p_response = NULL;
    int             err = -1;
    ATLine*         p_cur = NULL;
    char**          line;
    int             strLength = datalen / sizeof(char *);
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;

    LOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long)datalen);
    for (i = strLength, cur = (const char **)data ;
         i > 0 ; cur++, i --) {
        LOGD("> '%s'", *cur);
    }

    if (strLength != 2) {
        /* Non proietary. Loopback! */
        RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
        return;
    }

    /* For AT command access */
    cur = (const char **)data;
    if (NULL != cur[1] && strlen(cur[1]) != 0) {
        if ((strncmp(cur[1],"+CIMI",5) == 0) ||(strncmp(cur[1],"+CGSN",5) == 0)) {
            err = at_send_command_numeric(cur[0], &p_response, OEM_CHANNEL_CTX);
        } else {
            err = at_send_command_multiline(cur[0],cur[1], &p_response, OEM_CHANNEL_CTX);
        }
    } else {
        err = at_send_command(cur[0],&p_response, OEM_CHANNEL_CTX);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("OEM_HOOK_STRINGS fail");
        goto error;
    }

    /* Count response length */
    strLength = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next)
        strLength++;

    if (strLength == 0) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        LOGI("%d of %s received!",strLength, cur[1]);
        line = (char **) alloca(strLength * sizeof(char *));
        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    }
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

static void requestSetEtsWake(void *data, size_t datalen, RIL_Token t)
{
    int operation = ((int*) data)[0];
    int ret;
    LOGD("requestSetEtsWake ((int*)data)[0] = %d", ((int*) data)[0]);
    if (operation)
    {
        LOGD("requestSetEtsWake: enable");
        ret = at_send_command("AT+VUSBETS=1", NULL, OEM_CHANNEL_CTX);
        if (ret != 0)
            goto error;
    }
    else
    {
        LOGD("requestSetEtsWake: disable");
        ret = at_send_command("AT+VUSBETS=0", NULL, OEM_CHANNEL_CTX);
        if (ret != 0)
            goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
    error: LOGE("ERROR: requestSetEtsWake failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static unsigned int get_gps_factory_mode_data()
{
    char tmp[PROPERTY_VALUE_MAX] = { 0 };

    /* get property for factory mode check */
    property_get("fac.gps.in.factory.mode", tmp, "0");
    s_gps_fac_data.is_in_factory_mode = atoi(tmp);

    if (s_gps_fac_data.is_in_factory_mode) {
        /* get property for cold start check */
        property_get("fac.gps.cold.start", tmp, "0");
        s_gps_fac_data.is_cold_start = atoi(tmp);

        /* get property for auto restart when gps fix failed */
        property_get("fac.gps.auto.restart", tmp, "0");
        s_gps_fac_data.is_auto_restart = atoi(tmp);

        /* get property for MSA trackin mode */
        property_get("fac.gps.msa.trackin.mode", tmp, "0");
        s_gps_fac_data.is_trackin_mode = atoi(tmp);

        /* get property for gps mode */
        property_get("fac.gps.mode", tmp, "1");
        s_gps_fac_data.mode = atoi(tmp);

        /* get property for MSA trackin mode fix times & interval */
        if (s_gps_fac_data.is_trackin_mode) {
            property_get("fac.gps.msa.tkin.mode.interval", tmp, "0");
            s_gps_fac_data.fix_interval = atoi(tmp);
            s_gps_fac_data.fix_interval = s_gps_fac_data.fix_interval / 1000;

            property_get("fac.gps.msa.tkin.mode.times", tmp, "0");
            s_gps_fac_data.fix_max_times = atoi(tmp);
        } else {
            s_gps_fac_data.fix_interval = s_gps_data.fix_interval;
            s_gps_fac_data.fix_max_times = s_gps_data.fix_max_times;
        }
    }

    LOGD("[GPS DATA] gps_mode = %d, fix_interval = %d, fix_max_times = %d",
            s_gps_data.mode, s_gps_data.fix_interval, s_gps_data.fix_max_times);
    LOGD("[GPS FAC DATA] factory_mode = %d, cold_start = %d, auto_restart = %d, trackin_mode = %d, gps_mode = %d, fix_interval = %d, fix_max_times = %d",
            s_gps_fac_data.is_in_factory_mode, s_gps_fac_data.is_cold_start,
            s_gps_fac_data.is_auto_restart, s_gps_fac_data.is_trackin_mode,
            s_gps_fac_data.mode, s_gps_fac_data.fix_interval,
            s_gps_fac_data.fix_max_times);

    return s_gps_fac_data.is_in_factory_mode;
}

static void requestSetGps(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int value = ((int*) data)[0];
    int gps_enable = value & 0xFFFF;
    int gps_mode = (value >> 16) & 0xFFFF;
    int fix_interval = s_gps_data.fix_interval;
    int fix_max_times = s_gps_data.fix_max_times;

    /* get property values of gps factory mode */
    get_gps_factory_mode_data();

    if (s_gps_fac_data.is_in_factory_mode) {
        gps_mode = s_gps_fac_data.mode;
        fix_interval = s_gps_fac_data.fix_interval;
        fix_max_times = s_gps_fac_data.fix_max_times;
    } else {
        s_gps_data.mode = gps_mode;
    }

    if (gps_mode == GPS_MODE_MSS || gps_mode == GPS_MODE_MSB) {
        fix_max_times = 0; /* cp return fix data for ever */
    }

    if (gps_enable) { /* gps started */

        if (s_gps_fac_data.is_in_factory_mode && s_gps_fac_data.is_cold_start) {

            /* send AT commands for gps assist data clear, about cold start */
            err = at_send_command("AT^GPSRES", &p_response, OEM_CHANNEL_CTX);
            if ((err < 0) || (p_response->success == 0)) {
                goto error;
            }
            at_response_free(p_response);
            sleep(5); /* sleep 5 seconds after clear sys data of gps in cp side */
        }

        asprintf(&cmd, "AT^GPSMODE=%d", gps_mode);
        err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
        free(cmd);
        if ((err < 0) || (p_response->success == 0)) {
            goto error;
        }
        at_response_free(p_response);

        err = at_send_command("AT^GPSQOS=3,16", &p_response, OEM_CHANNEL_CTX);
        if ((err < 0) || (p_response->success == 0)) {
            goto error;
        }
        at_response_free(p_response);

        asprintf(&cmd, "AT^GPSLOC=1,%d,%d", fix_interval, fix_max_times);
        err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
        free(cmd);
        if ((err < 0) || (p_response->success == 0)) {
            goto error;
        }
        at_response_free(p_response);
    } else { /* gps closed */
        err = at_send_command("AT^GPSLOC=0", &p_response, OEM_CHANNEL_CTX);
        if ((err < 0) || (p_response->success == 0)) {
            goto error;
        }
        at_response_free(p_response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
    error: at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void triggerIoctl(int param){
    RLOGD("triggerIoctl, param=%d", param);

#ifdef MTK_ECCCI_C2K
    char ret[32];
    int fd = -1;
    snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
    fd = open(ret, O_RDWR);
    if(fd >= 0) {
        ioctl(fd, param);
        close(fd);
    } else {
        LOGE("open ccci_get_node_name USR_RILD_IOCTL failed");
    }
#else /* MTK_ECCCI_C2K */
    switch (param)
    {
    case CCCI_IOC_MD_RESET:
        C2KReset();
        break;
    case CCCI_IOC_FORCE_MD_ASSERT:
        C2KForceAssert();
        break;
    case CCCI_IOC_ENTER_DEEP_FLIGHT:
        C2KEnterFlightMode();
        break;
    case CCCI_IOC_LEAVE_DEEP_FLIGHT:
        C2KLeaveFlightMode();
        break;
    default:
        exit(0);
        break;
    }
#endif
}

/**
 *description:support framework to reset modem both active & inactive ways
 *parameter 1:means framework just want an exception from modem
 *parameter 2:means framework want reset modem
 */
static void requestSetTrm(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    int resetType = 0;

    resetType = ((int *) data)[0];
    LOGD("requestSetTrm resetType is %d", resetType);
    ///for sim pin test
    property_set("ril.cdma.report.case", "1");

    ///for svlte,c card is also need G modem unlock sim pin
    if (isCdmaLteDcSupport()) {
        property_set("ril.mux.report.case", "2");
    }

    if (RESET_MODE_ASSERT == resetType) { //MD3 assert(active mode)
        LOGD("Request CDMA MD assert new.");

        triggerIoctl(CCCI_IOC_FORCE_MD_ASSERT);

    } else if (RESET_MODE_RESET == resetType) { //MD3 reset(inactive mode)
    	LOGD("Request CDMA MD reset new.");
        triggerIoctl(CCCI_IOC_MD_RESET);

    } else if (TRIGGER_CDMA_RILD_NE == resetType) { // RILD NE
        LOGD("Request CDMA RILD NE.");
        // Make RILD NE here, %s format with int value caused NE.
        LOGD("requestSetTrm resetType is %s", resetType);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
	at_response_free(p_response);
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void onPCModemDisconn(void *param)
{
    system("echo 0 > /sys/class/usb_rawbulk/data/enable");
    RIL_Data_Call_Response_v11 *datacall_response = NULL;
    char name[PROPERTY_VALUE_MAX];
    datacall_response = (RIL_Data_Call_Response_v11 *) alloca(
            sizeof(RIL_Data_Call_Response_v11));
    memset(datacall_response, 0, sizeof(RIL_Data_Call_Response_v11));
    datacall_response->active = 4;
    datacall_response->cid = -1;

    property_get("net.cdma.ppp.ifname", name, "ppp0");
    datacall_response->ifname = name;
    s_PcModemState = PCMODEM_DISCONNECT;
    RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
            datacall_response, sizeof(RIL_Data_Call_Response_v11));
}

static void flush_data_channel(void)
{
    int fd = -1;
    LOGD("flush_data_channel");
    waitForAdjustPortPath(VIATEL_CHANNEL_DATA, s_data_device_path);
    fd = open(s_data_device_path, O_RDWR);
    if (fd >= 0) {
        /* disable echo on serial ports */
        struct termios ios;
        tcgetattr(fd, &ios);
        ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
        ios.c_cflag &= (~HUPCL); /*don't sent DTR=0 while close fd*/
        tcsetattr(fd, TCSANOW, &ios);
        tcflush(fd, TCIFLUSH);
        close(fd);
        LOGD("flush_data_channel successfully");
    } else {
        LOGE("flush_data_channel failed errno = %d", errno);
    }
}

static void * setupPCModemThread(void * arg)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *dataCallCmd = (char *) arg;
    LOGD("setupPCModemThread: dataCallCmd is %s", dataCallCmd);
    waiting_localdata_disconn = 1;
    sleep(2); //Wait 2 seconds because CP  may report  "No Carrier " from last  local datacall deactive request and this will puzzle ctclient.
    flush_data_channel();
    waiting_localdata_disconn = 0;
    LOGD("after wait local datacall disconnected, sState = %d",
            getRadioState());
    if ((getRadioState() != RADIO_STATE_OFF)
            && (s_PcModemState == PCMODEM_CONNECTING)) {
        system("echo 1 > /sys/class/usb_rawbulk/data/enable");
        err = at_send_command(dataCallCmd, &p_response, getDefaultChannelCtx());
        if ((err < 0) || (p_response->success == 0)) {
            LOGD("setupPCModemThread send cmd %s error, reurn", dataCallCmd);
            RIL_requestProxyTimedCallback(onPCModemDisconn, NULL, &TIMEVAL_0, getDefaultChannelCtx()->id);
            return 0;
        }
        LOGD("%s: after data bypass", __FUNCTION__);
    }
    free(dataCallCmd);
    return 0;
}

static CTClient_CommandParam* CTClient_search_command_param(const char *command)
{
    const int CMD_NUM = sizeof(CTCLIENT_CommandParams)
            / sizeof(CTCLIENT_CommandParams[0]);
    int i;

    for (i = 0; i < CMD_NUM; i++) {
        CTClient_CommandParam* cmd_param =
                (CTClient_CommandParam *) &CTCLIENT_CommandParams[i];
        if (cmd_param->cmdPrefix
                && strStartsWith(command, cmd_param->cmdPrefix))
            return cmd_param;
    }
    return NULL;
}

static void getCurrentNetworkMode(int *param) {
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int mode = 0;

    err = at_send_command_singleline("AT^PREFMODE?", "^PREFMODE:", &p_response, getDefaultChannelCtx());
    if ((err < 0) || (p_response->success == 0)) {
        goto exit;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto exit;
    }
    err = at_tok_nextint(&line, &mode);
    if (err < 0) {
        goto exit;
    }
    LOGD("%s: mode = %d", __FUNCTION__, mode);
    *param = mode;

    exit: at_response_free(p_response);
    return;
}

//RIL_Data_Call_Response_v4, need check
static void requestSendCommandForCTClient(void * data, size_t datalen,
        RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_Data_Call_Response_v11 *datacall_response = NULL;
    int err = 0;
    char *cmd = (char*) data;
    CTClient_CommandParam* cmd_param = NULL;
    char buf[PROPERTY_VALUE_MAX] = { '\0' };
    static int index = 0;
    pthread_t tid;
    pthread_attr_t attr;
    char name[PROPERTY_VALUE_MAX];
    char magicFlag[PROPERTY_VALUE_MAX] = { 0 };
    char *dataCallCmd = NULL;

    property_get("net.cdma.via.at.reset", magicFlag, "disable");
    LOGD("requestSendCommandForCTClient  cmd = %s, via.at.reset = %s", cmd,
            magicFlag);
#ifdef AT_AUTO_SCRIPT_RESET
    if(NULL != cmd) {
        if(strStartsWith(cmd, "AT^RESET")) {
            property_set("net.cdma.via.at.reset", "enable");
        }
        if(!strncmp(magicFlag, "enable", 6) && strStartsWith(cmd, "AT+CPBS=")) {
            property_set("net.cdma.via.at.reset", "disable");
        }
    }
#endif
    if (getRadioState() != RADIO_STATE_SIM_READY
            && getRadioState() != RADIO_STATE_RUIM_READY) {
        char card_not_ready[] = "\r\nERROR\r\n";
        RIL_onRequestComplete(t, RIL_E_SUCCESS, card_not_ready,
                strlen(card_not_ready));
        return;
    }
    if (strStartsWith(cmd, "AT^MD5=") && (strncmp(cmd, "AT^MD5=?", 8) != 0)) {
        LOGD("set processing_md5_cmd 1");
        processing_md5_cmd = 1;
        index = 1;
    }
    LOGD("requestSendCommandForCTClient: s_PcModemState = %d", s_PcModemState);
    if (strStartsWith(cmd, "ATD") && (s_PcModemState == PCMODEM_DISCONNECT)) {
        LOGD("PC request cdma data call");
        s_PcModemState = PCMODEM_CONNECTING;
        datacall_response = (RIL_Data_Call_Response_v11 *) alloca(
                sizeof(RIL_Data_Call_Response_v11));
        memset(datacall_response, 0, sizeof(RIL_Data_Call_Response_v11));
        datacall_response->active = 3;
        datacall_response->cid = -1;

        property_get("net.cdma.ppp.ifname", name, "ppp0");
        datacall_response->ifname = name;

        getCurrentNetworkMode(&currentNetworkMode);
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                datacall_response, sizeof(RIL_Data_Call_Response_v11));

        s_PcModemState = PCMODEM_CONNECTING;

        flush_data_channel();

        if (getRadioState() != RADIO_STATE_OFF) {
            asprintf(&dataCallCmd, "%s", cmd);
            LOGD("before switch bypass, dataCallCmd:%s", dataCallCmd);
            system("echo 1 > /sys/class/usb_rawbulk/data/enable");
            err = at_send_command(dataCallCmd, &p_response, OEM_CHANNEL_CTX);
            if ((err < 0) || (p_response->success == 0)) {
                LOGD("setupPCModemThread send cmd %s error, return",
                        dataCallCmd);
                RIL_requestProxyTimedCallback(onPCModemDisconn, NULL,
                        &TIMEVAL_0, getDefaultChannelCtx()->id);
                return 0;
            }
        }
    }

    if (strStartsWith(cmd, "ATE") || strStartsWith(cmd, "ATZ")
            || strStartsWith(cmd, "ATQ0") || strcmp(cmd, "AT") == 0) {
        // cheat because echo is required by specification, but may have negative impact on rild.
        char fake_ok[] = "\r\nOK\r\n";
        RIL_onRequestComplete(t, RIL_E_SUCCESS, fake_ok, strlen(fake_ok));
        return;
    }
    if (strStartsWith(cmd, "ATD")) {
        LOGD("PC Modem connect request, return first");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
#ifdef AT_AUTO_SCRIPT_MODE
    if (13 == strlen(cmd) && strStartsWith(cmd, "AT^PREFMODE="))
    {
        int mode = atoi(cmd + 12);
        LOGD("%s: mode = %d", __FUNCTION__, mode);
        if ((2 == mode) || (4 == mode) || (8 == mode))
        {
            forceEnableModeReport = 1;
            ctclientServiceMode = mode;
            LOGD("%s: ctclientServiceMode = %d", __FUNCTION__,
                    ctclientServiceMode);
            at_send_command("AT+MODE=1", NULL, OEM_CHANNEL_CTX);
        }
    }
#endif
    if (index == 2) {/*for MD5 command*/
        LOGD("need ^z ");
        err = at_send_command_for_ctclient(cmd, SINGLELINE, "^MD5", NULL, 5000,
                &p_response, 0, OEM_CHANNEL_CTX);
        if (err < 0) {
            char error_response[] = "\r\nERROR\r\n";
            RIL_onRequestComplete(t, RIL_E_SUCCESS, error_response,
                    strlen(error_response));
            goto exit;
        }
    } else {
        LOGD("not need ^z ");
        cmd_param = CTClient_search_command_param(cmd);
        if (cmd_param == NULL) {
            LOGD("command not supported for ctclient: %s", cmd);
            char not_supported[] = "\r\nERROR\r\n"; //"\r\nCOMMAND NOT SUPPORT\r\n"; //
            RIL_onRequestComplete(t, RIL_E_SUCCESS, not_supported,
                    strlen(not_supported));
            return;
        }
        err = at_send_command_for_ctclient(cmd, cmd_param->commandType,
                cmd_param->responsePrefix, NULL, cmd_param->OverTime * 1000,
                &p_response, 0, OEM_CHANNEL_CTX);
        if (err < 0) {
            char error_response[] = "\r\nERROR\r\n";
            RIL_onRequestComplete(t, RIL_E_SUCCESS, error_response,
                    strlen(error_response));
            goto exit;
        }
#if defined(FLASHLESS_SUPPORT) && defined(AT_AUTO_SCRIPT_RESET)
        if(strStartsWith(cmd, "AT^RESET")) {
            noLoop = 1;
            onATReaderClosed();
        }
#endif
    }
    goto nomal_response;

    nomal_response: {
        int buf_size = 0;
        char* response = NULL;
        ATLine *p_cur = NULL;
        char* p;

        // compute buf_size
        for (p_cur = p_response->p_intermediates; p_cur != NULL;
                p_cur = p_cur->p_next) {
            buf_size += 2 + strlen(p_cur->line) + 2;
            //LOGD("ctclient: intermediate line=%s", p_cur->line);
        }
        if (p_response->finalResponse) {
            buf_size += 2 + strlen(p_response->finalResponse) + 2;
            //LOGD("ctclient: finalResponse line= %s", p_response->finalResponse);
        }
        buf_size += 1;
        LOGD("ctclient: buf_size=%d", buf_size);

        // composite response
        response = (char*) malloc(buf_size);
        p = response;
        for (p_cur = p_response->p_intermediates; p_cur != NULL;
                p_cur = p_cur->p_next) {
            p += sprintf(p, "\r\n%s\r\n", p_cur->line);
        }
        if (p_response->finalResponse) {
            p += sprintf(p, "\r\n%s\r\n", p_response->finalResponse);
        }
        //*p = '\0';
        LOGD("ctclient: write to buffer: %d, content=[%s]", p - response,
                response);

        assert(p - response == buf_size - 1);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, p - response);
        free(response);
    }
exit:
    at_response_free(p_response);
    if (index == 2) {
        processing_md5_cmd = 0;
        LOGD("set processing_md5_cmd 0");
        index = 0;
    }
    if (index == 1)
        index++;
}

static void requestConfigEvdoMode(void * data, size_t datalen,
        RIL_Token t)
{
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int value = ((int *)data)[0];
    LOGD("%s: value is %d", __FUNCTION__, value);
    asprintf(&cmd, "AT+EVDOMODE=%d", value);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
        LOGE("%s: send at error", __FUNCTION__);
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/* Power on/off MD notified by thermal service, 2015/07/08 {*/
static void requestSetThermalModem(void *data, size_t datalen, RIL_Token t)
{
    assert(datalen >= sizeof(int *));
    int modem_on = ((int*) data)[0];
    LOGD("%s: modem_on = %d", __FUNCTION__, modem_on);
    if (modem_on) {
        requestModemPowerOn(NULL, 0, t);
    } else {
        requestModemPowerOff(NULL, 0, t);
    }
}
/* Power on/off MD notified by thermal service, 2015/07/08 }*/

void initRadioCapabilityResponse(RIL_RadioCapability* rc, RIL_RadioCapability* copyFromRC) {
    memset(rc, 0, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = copyFromRC->session;
    rc->phase = copyFromRC->phase;
    rc->rat = copyFromRC->rat;
    strcpy(rc->logicalModemUuid, copyFromRC->logicalModemUuid);
    rc->status = copyFromRC->status;
}

extern void requestSetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    char sRcSessionId[32] = {0};
    RIL_RadioCapability rc;
    int err = 0;
    ATResponse *p_rfs_response = NULL;
    char *cbp_version = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *line = NULL;
    memcpy(&rc, data, sizeof(RIL_RadioCapability));
    LOGD("requestSetRadioCapability : %d, %d, %d, %d, %s, %d",
        rc.version, rc.session, rc.phase, rc.rat, rc.logicalModemUuid, rc.status);

    memset(sRcSessionId, 0, sizeof(sRcSessionId));
    sprintf(sRcSessionId,"%d",rc.session);

    RIL_RadioCapability* responseRc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
    initRadioCapabilityResponse(responseRc, &rc);
    switch (rc.phase) {
        case RC_PHASE_START:
            LOGD("requestSetRadioCapability RC_PHASE_START");
            err = at_send_command_singleline("AT+VCGMM", "+VCGMM",
                    &p_cbp_version_response, getRILChannelCtxFromToken(t));
            if (err < 0 || p_cbp_version_response->success == 0)
            {
                LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
                break;
            }
            line = p_cbp_version_response->p_intermediates->line;
            err = at_tok_start(&line);
            if (err < 0)
            {
                LOGD("%s: failed to at_tok_start", __FUNCTION__);
                break;
            }
            err = at_tok_nextstr(&line, &cbp_version);
            if (err < 0)
            {
                LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
                break;
            }
            // there is no need to do this before CBP8.X
            if((NULL != cbp_version) && (!strncmp(cbp_version, "CBP8", 4)))
            {
                err = at_send_command("AT+RFSSYNC", &p_rfs_response, getRILChannelCtxFromToken(t));
                if (err < 0 || p_rfs_response->success == 0)
                {
                    LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                    break;
                }
            }

            if (s_md3_off != 1) {
                s_md3_off = 1;
                LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
                err = at_send_command("AT+EPOF", NULL, getRILChannelCtxFromToken(t));
                LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);
                property_set("cdma.ril.eboot", "1");
                setRadioState(RADIO_STATE_OFF);
            }
            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;

        default:
            LOGD("requestSetRadioCapability default, phase %d", rc.phase);
            responseRc->status = RC_STATUS_FAIL;
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            break;
    }
    free(responseRc);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
}

extern void requestGetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    RILChannelId rid = RIL_queryMyChannelId(t);
    LOGD("current RID : %d", rid);

    RIL_RadioCapability* rc = (RIL_RadioCapability*) calloc(1, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = 0;
    rc->phase = RC_PHASE_CONFIGURED;
    rc->rat = (1 << RADIO_TECH_GSM);
    rc->status = RC_STATUS_NONE;

    LOGD("requestGetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
            rc->version, rc->session, rc->phase, rc->rat, rc->logicalModemUuid, rc->status, rid);

    //C2K rild always return the same capability
    rc->rat = ((1 << RADIO_TECH_GSM)) | (1 << RADIO_TECH_UMTS);
    //C2K rild always return this string,because MD3 not has uid
    strcpy(rc->logicalModemUuid, "modem_sys3");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(RIL_RadioCapability));
}
