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

#include <dirent.h>
#include <stdlib.h>
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
#include "hardware/ccci_intf.h"
#include <cutils/properties.h>
#include <com_intf.h>
#include <linux/serial.h>

#include "ril_callbacks.h"
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include "hardware/ccci_intf.h"

#ifdef  RIL_SHLIB
const struct RIL_Env *s_rilenv;
#endif  /* RIL_SHLIB */

#define SYS_DATA_CHANNEL "/sys/class/usb_rawbulk/data/enable"
#define SYS_ETS_CHANNEL  "/sys/class/usb_rawbulk/ets/enable"
#define SYS_AT_CHANNEL   "/sys/class/usb_rawbulk/atc/enable"
#define SYS_PCV_CHANNEL  "/sys/class/usb_rawbulk/pcv/enable"
#define SYS_GPS_CHANNEL  "/sys/class/usb_rawbulk/gps/enable"

#define SYSFS_USB_DRIVER_OLD    "/sys/bus/usb/drivers/option/"
#define SYSFS_USB_DRIVER_NEW    "/sys/bus/usb/drivers/via-cbp/"

#define GW_SIGNAL_DEFAULT          0
#define GW_BIT_ERR_RATE_DEFAULT    0
#define CDMA_DBM_DEFAULT           55
#define EVDO_DBM_DEFAULT           65

#ifdef ANDROID_KK

ModemInfo *sMdmInfo;

static int net2modem[] = {
    MDM_GSM | MDM_WCDMA,                                 // 0  - GSM / WCDMA Pref
    MDM_GSM,                                             // 1  - GSM only
    MDM_WCDMA,                                           // 2  - WCDMA only
    MDM_GSM | MDM_WCDMA,                                 // 3  - GSM / WCDMA Auto
    MDM_CDMA | MDM_EVDO,                                 // 4  - CDMA / EvDo Auto
    MDM_CDMA,                                            // 5  - CDMA only
    MDM_EVDO,                                            // 6  - EvDo only
    MDM_GSM | MDM_WCDMA | MDM_CDMA | MDM_EVDO,           // 7  - GSM/WCDMA, CDMA, EvDo
    MDM_LTE | MDM_CDMA | MDM_EVDO,                       // 8  - LTE, CDMA and EvDo
    MDM_LTE | MDM_GSM | MDM_WCDMA,                       // 9  - LTE, GSM/WCDMA
    MDM_LTE | MDM_CDMA | MDM_EVDO | MDM_GSM | MDM_WCDMA, // 10 - LTE, CDMA, EvDo, GSM/WCDMA
    MDM_LTE,                                             // 11 - LTE only
};

static int32_t net2pmask[] = {
    MDM_GSM | (MDM_WCDMA << 8),                          // 0  - GSM / WCDMA Pref
    MDM_GSM,                                             // 1  - GSM only
    MDM_WCDMA,                                           // 2  - WCDMA only
    MDM_GSM | MDM_WCDMA,                                 // 3  - GSM / WCDMA Auto
    MDM_CDMA | MDM_EVDO,                                 // 4  - CDMA / EvDo Auto
    MDM_CDMA,                                            // 5  - CDMA only
    MDM_EVDO,                                            // 6  - EvDo only
    MDM_GSM | MDM_WCDMA | MDM_CDMA | MDM_EVDO,           // 7  - GSM/WCDMA, CDMA, EvDo
    MDM_LTE | MDM_CDMA | MDM_EVDO,                       // 8  - LTE, CDMA and EvDo
    MDM_LTE | MDM_GSM | MDM_WCDMA,                       // 9  - LTE, GSM/WCDMA
    MDM_LTE | MDM_CDMA | MDM_EVDO | MDM_GSM | MDM_WCDMA, // 10 - LTE, CDMA, EvDo, GSM/WCDMA
    MDM_LTE,                                             // 11 - LTE only
};

static int is3gpp2(int radioTech) {
    switch (radioTech) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            return 1;
        default:
            return 0;
    }
}
#endif /* ANDROID_KK */

// M: For multi channel support
#define DEFAULT_CHANNEL_CTX getRILChannelCtxFromToken(t)
// All channels' device sys property list

#ifdef MTK_ECCCI_C2K
int s_mux_fd_property_list[RIL_SUPPORT_CHANNELS] =
{
    USR_C2K_AT,
    USR_C2K_AT2,
    USR_C2K_AT3
};
#else /* MTK_ECCCI_C2K */
int s_mux_fd_property_list[RIL_SUPPORT_CHANNELS] =
{
    VIATEL_CHANNEL_AT,
    VIATEL_CHANNEL_AT2,
    VIATEL_CHANNEL_AT3
};
#endif /* MTK_ECCCI_C2K */

static char *s_mux_path[RIL_SUPPORT_CHANNELS][32];

int inemergency = 0;

/* cache for unsolicited message from BP */
RIL_UNSOL_Msg_Cache s_unsol_msg_cache = {
    0, 0, 0, 1,
    1,
    "", "",
    "",
    {
        { GW_SIGNAL_DEFAULT, GW_BIT_ERR_RATE_DEFAULT, },
        { CDMA_DBM_DEFAULT, CDMA_ECIO_DEFAULT, },
        { EVDO_DBM_DEFAULT, EVDO_ECIO_DEFAULT, EVDO_SNR_DEFAULT, },
        { 0, 44, 20, 0, 0, },
    },
    0, 0,
    2,0
};

static int cdma_dbm = 0;
static int cdma_ecio = 0;
static int evdo_dbm = 0;
static int evdo_ecio = 0;
static int evdo_ratio = 0;

int cta_no_uim = 0;
// framework wants to enable ps
int s_ps_on_desired = 0;
//0: not yet, 1: initialized
int uim_modual_initialized = 0;
//0: not yet, 1: initialized
int iccid_readable = 0;
#ifdef AT_AUTO_SCRIPT_RESET
static int atResetMode = 0;
#endif /*AT_AUTO_SCRIPT_RESET*/
int currentNetworkMode = -1;

static pthread_mutex_t s_vser_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_signal_repoll_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_signal_change_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_pcmodem_signalcond = PTHREAD_COND_INITIALIZER;
int s_wait_repoll_flag = 0; /*1: waiting for repoll, so return a fake value*/
const struct timeval TIMEVAL_NETWORK_REPOLL = { 4, 0 }; /* 4 second */
static int s_port = -1;
const char s_device_path[32];
const char s_data_device_path[32];
static char atpath[32];
static char datapath[32];

#ifdef FLASHLESS_SUPPORT
static char* atDevPath = NULL;
static char* dataDevPath = NULL;

#ifdef AT_AUTO_SCRIPT_RESET
static int noLoop = 0;
#endif /* AT_AUTO_SCRIPT_RESET */

#endif /* FLASHLESS_SUPPORT */

static int s_device_socket = 0;
int voicetype = 0;
int localindatacall = 0;
int invoicecall = 0;
int waiting_localdata_disconn = 0;
static int pcmodem_inprogress = 0;
/* trigger change to this with s_state_cond */
int s_closed = 0;

//TODO: SIM/Nw both access this variabe, should extract set/get APIs
/*Indicate how many  times registration queried since last time no service*/
int no_service_times = 0;

/*fix HANDROID#2225,if unlock pin-code more than 3 times, service_state should set to 0(out of service)*/
int unlock_pin_outtimes = 0;

/**
 * "1":mean cp is card lock version, also called China Telecom(CT) customization version
 * "0":mean anyother cp version but except CT version
 * initialized cpisct to 1 mean RIL always choose CT as the default cp version in the beginning stage.
 * actually, value of cpisct was affirmed by "+VCPCOMPILE:" URC in the beginning stage
 */
int cpisct = 1;

/*Indicate current card type, initialized by "UIMST:" URC in onUnsolicited function*/
int cardtype = 0;

/*power up with gsm card*/
int plugsinfo[2] = { 0 };
//the description about pluguinfo & plugsinfo pls refer to "+UIMST:" URC in onUnsolicited
int pluguinfo = 0;

static int sFD; /* file desc of AT channel */
static char sATBuffer[MAX_AT_RESPONSE + 1];
static char *sATBufferCur = NULL;

static const struct timeval TIMEVAL_GPSRESTART = { 3, 0 };
const struct timeval TIMEVAL_0 = { 0, 0 };

#ifdef MT6589
static const struct timeval TIMEVAL_QUERYICCID = {3,0};
#endif

#ifdef ADD_MTK_REQUEST_URC
static const struct timeval TIMEVAL_5 = { 5, 0 };
int smsReadyReported = 1;
int rildSockConnect = 0;
int uimInsertedReported = 1;
int invalidSimReported = 1;
int uimInsertedStatus = UIM_STATUS_INITIALIZER;
int arsithrehd = -1; //arsi threshold value, anywhere sent to set cp should use this value
static int arsithrehddef = 2; //default arsi threshold value, only used in atcommand_init() func
int uimpluginreport = 1;
int gsmpluginreport = 1;
int cardtypereport = 1;
#endif /* ADD_MTK_REQUEST_URC */

int deactivedata_inprogress = 0;

PcModem_State s_PcModemState = PCMODEM_DISCONNECT;

int g_oem_support_flag = 0;

int s_dataState = DATA_STATE_INACTIVE;
#ifdef ANDROID_KK
static int s_ims_registered = 0;         // 0==unregistered
static int s_ims_services = 1;           // & 0x1 == sms over ims supported
static int s_ims_format = 1;             // FORMAT_3GPP(1) vs FORMAT_3GPP2(2);
static int s_ims_cause_retry = 0;        // 1==causes sms over ims to temp fail
static int s_ims_cause_perm_failure = 0; // 1==causes sms over ims to permanent fail
static int s_ims_gsm_retry = 0;          // 1==causes sms over gsm to temp fail
static int s_ims_gsm_fail = 0;           // 1==causes sms over gsm to permanent fail

static int s_cell_info_rate_ms = INT_MAX;
static int s_mcc = 0;
static int s_mnc = 0;
static int s_lac = 0;
static int s_cid = 0;

static void setRadioTechnology(ModemInfo *mdm, int newtech);
static int query_ctec(ModemInfo *mdm, int *current, int32_t *preferred);
int parse_technology_response(const char *response, int *current, int32_t *preferred);
#endif

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static RIL_RadioState sState = RADIO_STATE_UNAVAILABLE;
static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static const struct timeval TIMEVAL_UPDATE_CACHE = {10, 0};   /* 10second */
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;

/**
 * M: Fix RILD NE after reset: AT channel init handling in main looper, and RIL Request
 * handling in proxy looper, if both looper send AT command, conflict will happen
 */
pthread_mutex_t s_at_init_Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_at_init_Cond = PTHREAD_COND_INITIALIZER;
int s_at_init_finish = 0;
pthread_t mainloop_thread_id;

#ifdef MTK_ECCCI_C2K
/**
 * M: RILD should send AT command after MD3 +VPUP URC reported; add a wait/notify mechanism
 * to make sure this sequence flow.
 */
pthread_mutex_t s_md3_init_Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_md3_init_Cond = PTHREAD_COND_INITIALIZER;
int s_md3_init_finish = 0;
#endif

extern int s_closed;
extern int processing_md5_cmd;
extern int s_md3_off;

extern void trigger_update_cache();
extern const char * requestToString(int request);
// For interface polling
extern int ifc_init(void);
extern void ifc_close(void);
extern int ifc_get_info(const char *name, unsigned int *addr,
        unsigned int *mask, unsigned int *flags);
// For multi channel support
extern void initRILChannels(void);


static void onRequest (int request, void *data, size_t datalen, RIL_Token t);
static void reportUSBDisconn();
static RIL_RadioState currentState();
static int onSupports (int requestCode);
static void onCancel (RIL_Token t);
static const char *getVersion();
static void reportRILDConn(int conn);

/*** Static Variables ***/
static RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    reportUSBDisconn,
    reportRILDConn
};

static void atcommand_init();
static int isRadioOn();
static SIM_Status getSIMStatus();
RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus);
static void prefmodetocdma(void *param);
static int iccCmdGetResponse(RIL_SIM_IO_v5 * p_args, RIL_SIM_IO_Response* p_sr); //need check
static int sendIccCRSMCommand(RIL_SIM_IO_v5 * p_args, RIL_SIM_IO_Response* p_sr); //need check
static int selectIccEfFile(RIL_SIM_IO_v5 * p_args, char DFid[]); //need check
static int isIccCmdSuccess(int sw1);
static int selectIccFile(char* FileId, int* pSw1, int* pSw2);
char *myitoa(int num, char *str, int radix);
void setPinPukCountLeftProperty(int pin1, int pin2, int puk1, int puk2);
void getRUimStatusAhead(void);
void getChvCountAhead(int * pin1Count, int * pin2Count, int * puk1Count,
        int * puk2Count);
void turnPSEnable(void *param);
int waitUimModualInitialized(void);
int waitIccidReadable(void);
#ifdef ADD_MTK_REQUEST_URC
int getCdmaModemSlot(void);
static int isDualtalkMode(void);
#endif /* ADD_MTK_REQUEST_URC */
static void onATReaderClosed(RILChannelCtx *p_channel);
void setNetworkMode(void *param);
static void autoEnterPinCode(void);
void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid);

#ifdef ADD_MTK_REQUEST_URC
/* System property "mediatek.evdo.mode.dualtalk" will be set by MTK to
 distinguish C + G or G + G. In C + G dualtalk mode, the property set to "1". */
static int isDualtalkMode(void)
{
    char DualtalkMode[PROPERTY_VALUE_MAX] = { 0 };

    property_get("mediatek.evdo.mode.dualtalk", DualtalkMode, "1");

    int mode = atoi(DualtalkMode);
    LOGD("%s: mode is %d", __FUNCTION__, mode);
    return mode;
}

/**
 * Return VIA cdma modem`s slot.
 * For E+G double talk
 * <return>
 *         1 : cdma in slot1
 *         2 : cdma in slot2
 */
int getCdmaModemSlot(void) {
    if (isCdmaLteDcSupport()) {
        return getCdmaSocketSlotId();
    }

    int ret = 0;
    char tempstr[PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    property_get("ril.external.md", tempstr, "2");

    ret = atoi(tempstr);
    LOGD("via cdma modem is in slot%d", ret);
    return ret;
}

/// M: SVLTE solution2 modification. @{
/**
 * Get the SVLTE slot id.
 * @return SVLTE slot id.
 *         1 : svlte in slot1
 *         2 : svlte in slot2
 */
int getActiveSvlteModeSlotId() {
    int i = 1;
    char tempstr[PROPERTY_VALUE_MAX];
    char *tok;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    property_get("persist.radio.svlte_slot", tempstr, "3,2");
    tok = strtok(tempstr, ",");
    while(tok != NULL)
    {
        if (3 == atoi(tok)) {
            LOGD("getActiveSvlteModeSlotId : %d", i);
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    LOGD("getActiveSvlteModeSlotId : -1");
    return -1;
}

/**
 * Get slot id which connect to c2k rild socket.
 * @return slot id which connect to c2k rild socket
 *         1 : slot1 connect to c2k rild socket
 *         2 : slot2 connect to c2k rild socket
 */
int getCdmaSocketSlotId() {
    int ret = 0;
    char tempstr[PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.radio.cdma_slot", tempstr, "1");

    ret = atoi(tempstr);
    LOGD("getCdmaSocketSlotId : %d", ret);
    return ret;
}

/**
 * Get the actual SVLTE slot id.
 * @return SVLTE slot id.
 *         1 : svlte in slot1
 *         2 : svlte in slot2
 */
int getActualSvlteModeSlotId() {
    int i = 1;
    char tempstr[PROPERTY_VALUE_MAX];
    char *tok;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    property_get("persist.radio.actual.svlte_slot", tempstr, "3,2");
    tok = strtok(tempstr, ",");
    while(tok != NULL)
    {
        if (3 == atoi(tok)) {
            LOGD("getActualSvlteModeSlotId : %d", i);
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    LOGD("getActualSvlteModeSlotId : -1");
    return -1;
}

/**
 * Get SIM count.
 * @return the number of SIM.
 */
int getSimCount() {
    int simCount = 1;
    char tempstr[PROPERTY_VALUE_MAX];

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.radio.multisim.config", tempstr, "");
    if (0 == strcmp(tempstr, "dsds")
        || 0 == strcmp(tempstr, "dsda")) {
        simCount = 2;
    } else if (0 == strcmp(tempstr, "tsts")) {
        simCount = 3;
    } else {
        simCount = 1;
    }
    LOGD("getSimCount : %d", simCount);
    return simCount;
}
/// @}
#endif /* ADD_MTK_REQUEST_URC */

void turnPSEnable(void *param)
{
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;
    int radioOn = 0;
    RIL_RadioState newRadioState = getRadioState();
    PS_PARAM* psParm = (PS_PARAM *) param;

    LOGD("%s: force is %d, enable is %d, inemergency is %d", __FUNCTION__,
            psParm->force, psParm->enable, inemergency);
    if (!psParm->force && !psParm->enable && inemergency)
    {
        LOGD("%s: in emergency mode, do not close ps!", __FUNCTION__);
        return;
    }

    if (psParm->enable)
    {
        asprintf(&cmd, "%s", "AT+CPON");
        newRadioState = RADIO_STATE_RUIM_NOT_READY;
    }
    else
    {
        asprintf(&cmd, "%s", "AT+CPOF");
        //newRadioState = RADIO_STATE_SIM_LOCKED_OR_ABSENT;
    }

    radioOn = isRadioOn();
    LOGD("%s: isEnable = %d, radioOn = %d", __FUNCTION__, psParm->enable, radioOn);
    if((psParm->enable && !radioOn) || (!psParm->enable && radioOn)) {
        err = at_send_command(cmd, &p_response, getDefaultChannelCtx());
        if((err != 0) || (p_response->success == 0)) {
            LOGD("%s: failed to %s", __FUNCTION__, cmd);
        } else {
            setRadioState(newRadioState);
        }
        at_response_free(p_response);
    }

    free(cmd);
}

char *myitoa(int num, char *str, int radix)
{
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0, j, k;
    if (radix == 10 && num < 0) {
        unum = (unsigned) -num;
        str[i++] = '-';
    } else
        unum = (unsigned) num;
    do {
        str[i++] = index[unum % (unsigned) radix];
        unum /= radix;
    } while (unum);
    str[i] = '\0';
    if (str[0] == '-')
        k = 1;
    else
        k = 0;
    for (j = k; j < (i - 1) / 2.0 + k; j++)
    {
        num = str[j];
        str[j] = str[i - j - 1 + k];
        str[i - j - 1 + k] = num;
    }
    return str;
}

#define TMP_BUF_SIZE 128
static int recvNoCarrier(void)
{
    int fd = -1;
    fd_set read_fd;
    struct timeval timeout;
    int ret = 0;
    int err = 0;
    int len = 0;
    char tmp_buf[TMP_BUF_SIZE];

    fd = open(s_data_device_path, O_RDWR | O_NONBLOCK);
    LOGD("data port open with non-block mode, fd = %d, devpath = %s", fd,
            s_data_device_path);
    if (fd >= 0) {
        /* disable echo on serial ports */
        struct termios ios;
        tcgetattr(fd, &ios);
        ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
        ios.c_cflag &= (~HUPCL); /*don't sent DTR=0 while close fd*/
        tcsetattr(fd, TCSANOW, &ios);
        tcflush(fd, TCIOFLUSH);
    } else {
        /* fail to open data device */
        LOGE("fail to open data device errno = %d", errno);
        return -1;
    }

    FD_ZERO(&read_fd);
    FD_SET(fd, &read_fd);
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    ret = select(fd + 1, &read_fd, NULL, NULL, &timeout);
    if (ret == 0)
    {
        LOGE("recv NO Carrier timeout!");
        err = -1;
        goto error;
    }
    else if (FD_ISSET(fd, &read_fd))
    {
        len = read(fd, tmp_buf, TMP_BUF_SIZE);
        if (len <= 0) {
            LOGD("read data port error!!");
            err = -1;
            goto error;
        }
        LOGD("<%s> <<<<<<<< %s >>>>>>>>>", s_data_device_path, tmp_buf);
    }

    close(fd);
    return 0;

error:
    if (fd >= 0) {
        LOGE("recv No Carrier error!!");
        close(fd);
    }
    return err;
}

/*** Callback methods from the RIL library to us ***/

/**
 * Call from RIL to us to make a RIL_REQUEST
 *
 * Must be completed with a call to RIL_onRequestComplete()
 *
 * RIL_onRequestComplete() may be called from any thread, before or after
 * this function returns.
 *
 * Will always be called from the same thread, so returning here implies
 * that the radio is ready to process another command (whether or not
 * the previous command has completed).
 */
static void onRequest(int request, void *data, size_t datalen, RIL_Token t)
{
    // Wait until AT channel initialization finished
    waitAtInitDone();

    if (s_md3_off && request != RIL_REQUEST_MODEM_POWERON && request != RIL_REQUEST_SET_MODEM_THERMAL)
    {
        RLOGD("MD off and ignore %s", requestToString(request));
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    ATResponse *p_response;
    int err = 0;
    LOGD("C2K onRequest: %s", requestToString(request));
    LOGD("C2K sState= %d", getRadioState());

#ifdef ADD_MTK_REQUEST_URC
    if (request > 0 && request < RIL_REQUEST_SET_AUDIO_PATH)
    {
        rildSockConnect = 1;
        LOGD("smsReadyReported= %d, uimInsertedReported= %d", smsReadyReported,
                uimInsertedReported);
        if (!smsReadyReported)
        {
            RIL_requestProxyTimedCallback(reportPbSmsReady, NULL,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            smsReadyReported = 1;
        }
        if (!uimInsertedReported)
        {
            RIL_requestProxyTimedCallback(reportRuimStatus, (void *) &uimInsertedStatus,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            uimInsertedReported = 1;
        }
        if (!invalidSimReported)
        {
            RIL_requestProxyTimedCallback(reportInvalidSimDetected, NULL,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            invalidSimReported = 1;
        }
        if (!uimpluginreport)
        {
            RIL_requestProxyTimedCallback(reportCdmaSimPlugIn, NULL,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            uimpluginreport = 1;
        }
        if (!gsmpluginreport)
        {
            RIL_requestProxyTimedCallback(reportGSMSimPlugIn, NULL,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            gsmpluginreport = 1;
        }
        if (!cardtypereport)
        {
            RIL_requestProxyTimedCallback(reportCardType, NULL,
                    &TIMEVAL_0, getRILChannelCtxFromToken(t)->id);
            cardtypereport = 1;
        }
    }
#endif

    /* Ignore all requests except RIL_REQUEST_GET_SIM_STATUS
     * when RADIO_STATE_UNAVAILABLE.
     */
    if (getRadioState() == RADIO_STATE_UNAVAILABLE
            && !(request == RIL_REQUEST_GET_SIM_STATUS
                    || request == RIL_REQUEST_ENTER_SIM_PIN
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_SET_ETS_DEV
                    || request == RIL_REQUEST_SET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_GET_RADIO_CAPABILITY
#ifdef AT_AUTO_SCRIPT_RESET
                    || request == RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT
#endif
#ifdef ADD_MTK_REQUEST_URC
                    || request == RIL_REQUEST_QUERY_ICCID
                    || request == RIL_REQUEST_QUERY_UIM_INSERTED
                    || request == RIL_REQUEST_DEVICE_IDENTITY
                    || request == RIL_REQUEST_SWITCH_HPF
                    || request == RIL_REQUEST_SET_MEID
                    /// M: [C2K][IR] Support SVLTE IR feature. @{
                    || request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA
                    || request == RIL_REQUEST_RESUME_REGISTRATION_CDMA
                    /// M: [C2K][IR] Support SVLTE IR feature. @}
                    || request == RIL_REQUEST_SET_TRM
                    || request == RIL_REQUEST_SET_ARSI_THRESHOLD
                    || request == RIL_REQUEST_NOTIFY_SIM_HOTPLUG
                    || request == RIL_REQUEST_MODEM_POWEROFF
                    || request == RIL_REQUEST_MODEM_POWERON
                    || request == RIL_REQUEST_CONFIG_MODEM_STATUS
                    || request == RIL_REQUEST_SWITCH_CARD_TYPE
                    || request == RIL_REQUEST_ENABLE_MD3_SLEEP
                    || request == RIL_REQUEST_ALLOW_DATA
                    || request == RIL_REQUEST_SET_MODEM_THERMAL
#endif
            ))
    {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /* Ignore all non-power requests when RADIO_STATE_OFF
     * (except RIL_REQUEST_GET_SIM_STATUS)
     */
    if (getRadioState() == RADIO_STATE_OFF
            && !(request == RIL_REQUEST_RADIO_POWER
                    || request == RIL_REQUEST_GET_SIM_STATUS
                    || request == RIL_REQUEST_ENTER_SIM_PIN
                    || request == RIL_REQUEST_SET_FACILITY_LOCK
                    || request == RIL_REQUEST_BASEBAND_VERSION
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_SET_ETS_DEV
                    || request == RIL_REQUEST_CONFIG_MODEM_STATUS
                    || request == RIL_REQUEST_OEM_HOOK_RAW
                    || request == RIL_REQUEST_OEM_HOOK_STRINGS
                    || request == RIL_REQUEST_CDMA_SUBSCRIPTION
                    || request == RIL_REQUEST_GET_IMSI
                    || request == RIL_REQUEST_SET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_GET_RADIO_CAPABILITY
#ifdef AT_AUTO_SCRIPT_RESET
                    || request == RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT
#endif
#ifdef ADD_MTK_REQUEST_URC
                    || request == RIL_REQUEST_QUERY_ICCID
                    || request == RIL_REQUEST_OPERATOR
                    || request == RIL_REQUEST_QUERY_UIM_INSERTED
                    || request == RIL_REQUEST_RADIO_POWEROFF
                    || request == RIL_REQUEST_RADIO_POWERON
                    || request == RIL_REQUEST_MODEM_POWEROFF
                    || request == RIL_REQUEST_MODEM_POWERON
                    || request == RIL_REQUEST_DEVICE_IDENTITY
                    || request == RIL_REQUEST_SWITCH_HPF
                    || request == RIL_REQUEST_QUERY_PHB_STORAGE_INFO
                    || request == RIL_REQUEST_SET_MEID
                    || request == RIL_REQUEST_RADIO_POWER_CARD_SWITCH
                    || request == RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
                    /// M: [C2K][IR] Support SVLTE IR feature. @{
                    || request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA
                    || request == RIL_REQUEST_RESUME_REGISTRATION_CDMA
                    /// M: [C2K][IR] Support SVLTE IR feature. @}
                    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
                    || request == RIL_REQUEST_SET_SVLTE_RAT_MODE
                    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}
                    || request == RIL_REQUEST_SET_TRM
                    || request == RIL_REQUEST_SET_ARSI_THRESHOLD
                    || request == RIL_REQUEST_NOTIFY_SIM_HOTPLUG
                    || request == RIL_REQUEST_WRITE_PHB_ENTRY
                    || request == RIL_REQUEST_READ_PHB_ENTRY
                    || request == RIL_REQUEST_CONFIG_IRAT_MODE
                    || request == RIL_REQUEST_GET_LOCAL_INFO
                    || request == RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND
                    || request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE
                    || request == RIL_REQUEST_QUERY_UTK_MENU_FROM_MD
                    || request == RIL_REQUEST_QUERY_STK_MENU_FROM_MD
                    || request == RIL_REQUEST_UTK_REFRESH
                    || request == RIL_REQUEST_STK_SET_PROFILE
                    || request == RIL_REQUEST_CONFIG_EVDO_MODE
                    || request == RIL_REQUEST_CONFIG_MODEM_STATUS
                    || request == RIL_REQUEST_SIM_IO
                    || request == RIL_REQUEST_SIM_IO_EX
                    || request == RIL_REQUEST_ENTER_SIM_PIN
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_ENTER_SIM_PIN2
                    || request == RIL_REQUEST_ENTER_SIM_PUK2
                    || request == RIL_REQUEST_CHANGE_SIM_PIN
                    || request == RIL_REQUEST_CHANGE_SIM_PIN2
                    || request == RIL_REQUEST_QUERY_FACILITY_LOCK
                    || request == RIL_REQUEST_SET_FACILITY_LOCK
                    || request == RIL_REQUEST_SWITCH_CARD_TYPE
                    || request == RIL_REQUEST_ENABLE_MD3_SLEEP
                    || request == RIL_REQUEST_ALLOW_DATA
                    || request == RIL_REQUEST_SET_MODEM_THERMAL
#endif
            ))
    {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }
    LOGD("processing_md5_cmd=%d", processing_md5_cmd);
    /*If now is processing md5 commands, ignore other ril commands*/
    if (processing_md5_cmd && (request != RIL_REQUEST_SEND_COMMAND_FOR_CTCLIENT))
    {
        LOGD("processing md5 command, ignore other ril commands except md5!!!");
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        return;
    }

    if (!(rilNwMain(request, data, datalen, t)
            || rilDataMain(request, data, datalen, t)
            || rilCcMain(request, data, datalen, t)
            || rilSimMain(request, data, datalen, t)
            || rilSmsMain(request, data, datalen, t)
            || rilSsMain(request, data, datalen, t)
            || rilUtkMain(request, data, datalen, t)
            || rilOemMain(request, data, datalen, t)))
    {
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    }
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState currentState()
{
    return getRadioState();
}
/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */

static int onSupports (int requestCode)
{
    //@@@ todo

    return 1;
}

static void onCancel (RIL_Token t)
{
    //@@@todo

}

#ifdef ANDROID_KK
/**
 * Parse the response generated by a +CTEC AT command
 * The values read from the response are stored in current and preferred.
 * Both current and preferred may be null. The corresponding value is ignored in that case.
 *
 * @return: -1 if some error occurs (or if the modem doesn't understand the +CTEC command)
 *          1 if the response includes the current technology only
 *          0 if the response includes both current technology and preferred mode
 */
int parse_technology_response( const char *response, int *current, int32_t *preferred )
{
    int err;
    char *line, *p;
    int ct;
    int32_t pt = 0;
    char *str_pt;

    line = p = strdup(response);
    LOGD("Response: %s", line);
    err = at_tok_start(&p);
    if (err || !at_tok_hasmore(&p))
    {
        LOGD("err: %d. p: %s", err, p);
        free(line);
        return -1;
    }

    err = at_tok_nextint(&p, &ct);
    if (err)
    {
        free(line);
        return -1;
    }
    if (current) *current = ct;

    LOGD("line remaining after int: %s", p);

    err = at_tok_nexthexint(&p, &pt);
    if (err)
    {
        free(line);
        return 1;
    }
    if (preferred)
    {
        *preferred = pt;
    }
    free(line);

    return 0;
}

int query_supported_techs( ModemInfo *mdm, int *supported )
{
    ATResponse *p_response;
    int err, val, techs = 0;
    char *tok;
    char *line;

    LOGD("query_supported_techs");
    err = at_send_command_singleline("AT+CTEC=?", "+CTEC:", &p_response, DEFAULT_CHANNEL_CTX);
    if (err || !p_response->success)
        goto error;
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err || !at_tok_hasmore(&line))
        goto error;
    while (!at_tok_nextint(&line, &val)) {
        techs |= ( 1 << val );
    }
    if (supported) *supported = techs;
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

/**
 * query_ctec. Send the +CTEC AT command to the modem to query the current
 * and preferred modes. It leaves values in the addresses pointed to by
 * current and preferred. If any of those pointers are NULL, the corresponding value
 * is ignored, but the return value will still reflect if retreiving and parsing of the
 * values suceeded.
 *
 * @mdm Currently unused
 * @current A pointer to store the current mode returned by the modem. May be null.
 * @preferred A pointer to store the preferred mode returned by the modem. May be null.
 * @return -1 on error (or failure to parse)
 *         1 if only the current mode was returned by modem (or failed to parse preferred)
 *         0 if both current and preferred were returned correctly
 */
int query_ctec(ModemInfo *mdm, int *current, int32_t *preferred)
{
    ATResponse *response = NULL;
    int err;
    int res;

    LOGD("query_ctec. current: %d, preferred: %d", (int)current, (int) preferred);
    err = at_send_command_singleline("AT+CTEC?", "+CTEC:", &response, DEFAULT_CHANNEL_CTX);
    if (!err && response->success) {
        res = parse_technology_response(response->p_intermediates->line, current, preferred);
        at_response_free(response);
        return res;
    }
    LOGE("Error executing command: %d. response: %x. status: %d", err, (int)response, response? response->success : -1);
    at_response_free(response);
    return -1;
}

int is_multimode_modem(ModemInfo *mdm)
{
    ATResponse *response;
    int err;
    char *line;
    int tech;
    int32_t preferred;

    if (query_ctec(mdm, &tech, &preferred) == 0)
    {
        mdm->currentTech = tech;
        mdm->preferredNetworkMode = preferred;
        if (query_supported_techs(mdm, &mdm->supportedTechs))
        {
            return 0;
        }
        return 1;
    }
    return 0;
}

/**
 * Find out if our modem is GSM, CDMA or both (Multimode)
 */
static void probeForModemMode(ModemInfo *info)
{
    ATResponse *response;
    int err;
    assert (info);
    // Currently, our only known multimode modem is qemu's android modem,
    // which implements the AT+CTEC command to query and set mode.
    // Try that first

    if (is_multimode_modem(info))
    {
        LOGI("Found Multimode Modem. Supported techs mask: %8.8x. Current tech: %d",
                info->supportedTechs, info->currentTech);
        return;
    }

    /* Being here means that our modem is not multimode */
    info->isMultimode = 0;

    /* CDMA Modems implement the AT+WNAM command */
    err = at_send_command_singleline("AT+WNAM","+WNAM:", &response, DEFAULT_CHANNEL_CTX);
    if (!err && response->success) {
        at_response_free(response);
        // TODO: find out if we really support EvDo
        info->supportedTechs = MDM_CDMA | MDM_EVDO;
        info->currentTech = MDM_CDMA;
        LOGI("Found CDMA Modem");
        return;
    }
    if (!err) at_response_free(response);
    // TODO: find out if modem really supports WCDMA/LTE
    info->supportedTechs = MDM_GSM | MDM_WCDMA | MDM_LTE;
    info->currentTech = MDM_GSM;
    LOGI("Found GSM Modem");
}
#endif

static const char * getVersion(void)
{
    static char version_info[50] = { 0 };
    sprintf(version_info, "Viatelecom-cdma-ril suffix Version-%s",
            VIA_SUFFIX_VERSION);
    return version_info;
}

static void reportRILDConn(int conn)
{
    rildSockConnect = conn;
    LOGD("rildSockConnect:%d", rildSockConnect);
}

void trigger_update_cache()
{
    LOGD("trigger_update_cache");
    at_send_command("AT^SYSINFO", NULL, getDefaultChannelCtx());
    at_send_command("AT+CSQ?", NULL, getDefaultChannelCtx());
    at_send_command("AT+HDRCSQ?", NULL, getDefaultChannelCtx());
    at_send_command("AT+CREG?", NULL, getDefaultChannelCtx());
    at_send_command("AT+VMCCMNC?", NULL, getDefaultChannelCtx());
    at_send_command("AT+VSER?", NULL, getDefaultChannelCtx());
}

/** Returns SIM_NOT_READY on error */
static SIM_Status getSIMStatus()
{
    ATResponse *p_response = NULL;
    int err = 0;
    int ret = 0;
    char *cpinLine = NULL;
    char *cpinResult = NULL;

    if (getRadioState() == RADIO_STATE_OFF
            || getRadioState() == RADIO_STATE_UNAVAILABLE)
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy());

    if ((err != 0) || (p_response->success == 0))
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    switch (at_get_cme_error(p_response))
    {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            ret = SIM_ABSENT;
            goto done;

        default:
            ret = SIM_NOT_READY;
            goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0)
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0)
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp(cpinResult, "SIM PIN"))
    {
        ret = SIM_PIN;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "SIM PUK"))
    {
        ret = SIM_PUK;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "PH-NET PIN"))
    {
        return SIM_NETWORK_PERSONALIZATION;
    }
    else if (0 != strcmp(cpinResult, "READY"))
    {
        /* we're treating unsupported lock types as "sim absent" */
        ret = SIM_ABSENT;
        goto done;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;

    ret = SIM_READY;

done:
    at_response_free(p_response);
    return ret;
}

/** Returns SIM_NOT_READY on error */
RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus)
{
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };
    LOGD("getUIMStatus sState = %d, isReportRadioStatus = %d", getRadioState(),
            isReportRadioStatus);
    property_get("net.cdma.boottimes", boottimes, "0");
    if ((getRadioState() == RADIO_STATE_UNAVAILABLE) && (strcmp(boottimes, "2") != 0)) {
        ret = RUIM_NOT_READY;
        goto done;
    }

    LOGD("getUIMStatus1");
    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy());
    LOGD("getUIMStatus2");
    if (err != 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }
    if (cta_no_uim)
    {
        LOGD("fake value uim status");
        ret = RUIM_ABSENT;
        goto done;
    }
    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            ret = RUIM_ABSENT;
#ifndef OPEN_PS_ON_CARD_ABSENT
            psParam.enable = 0;
            turnPSEnable((void *)&psParam);
#endif /* OPEN_PS_ON_CARD_ABSENT */
            goto done;

        default:
            ret = RUIM_NOT_READY;
            goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp(cpinResult, "SIM PIN"))
    {
        ret = RUIM_PIN;
        //LOGD("sState = %d",sState);
        //setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
#ifndef OPEN_PS_ON_CARD_ABSENT
        if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT)
        {
            psParam.enable = 0;
            turnPSEnable((void *)&psParam);
        }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        goto done;
    }
    else if (0 == strcmp(cpinResult, "SIM PUK"))
    {
#ifndef OPEN_PS_ON_CARD_ABSENT
        if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT)
        {
            psParam.enable = 0;
            turnPSEnable((void *)&psParam);
        }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        ret = RUIM_PUK;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "UIM Error"))
    {
        LOGD("UIM Error");
        ret = RUIM_NOT_READY; //cwen add uim error notification that UI not display no card
        goto done;
    }
    else if (0 != strcmp(cpinResult, "READY"))
    {
        /* we're treating unsupported lock types as "sim absent" */
        ret = RUIM_ABSENT;
        goto done;
    }

    p_response = NULL;
    cpinResult = NULL;

    /* ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 {*/
    //TODO: To complete align with GSM to remove Radio ON check
    LOGD("%s: Radio state: %d", __FUNCTION__, getRadioState());
    if (1 == isRadioOn()) {
        if (UPDATE_RADIO_STATUS == isReportRadioStatus) {
            setRadioState(RADIO_STATE_RUIM_READY); //cwen add CPOF before pin is checked so that it can find the net work after check pin
        }
        LOGD("%s: SIM detected and Radio is on", __FUNCTION__);
        ret = RUIM_READY;
    } else if (RADIO_STATE_UNAVAILABLE == getRadioState()) {
        LOGD("%s: RADIO_STATE_UNAVAILABLE", __FUNCTION__);
        ret = RUIM_NOT_READY;
    } else {
        LOGD("%s:UIM_READY", __FUNCTION__);
        ret = RUIM_READY;
    }
    /*  ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 }*/

done:
    at_response_free(p_response);
    LOGD("%s: ret = %d", __FUNCTION__, ret);
    return ret;
}

static void atcommand_init()
{
    ATResponse *p_response = NULL;

    char *cmd = NULL;
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };
    char pincode[10] = { 0 };
    char *sp = NULL;
    int pin;
    int a;
    int err;
    int cardstatus;
    int flight_mode = -1;
    int testSilentReboot = -1;
    int svlteSimPinLocked = -1;
    int modemEE  = -1;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    /**
     * note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed
     */

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL, getChannelCtxbyId(AT_CHANNEL));

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL, getChannelCtxbyId(AT_CHANNEL));

    /*  Network registration events */
    err = at_send_command("AT+CREG=2", &p_response, getChannelCtxbyId(AT_CHANNEL));

    /* some handsets -- in tethered mode -- don't support CREG=2 */
    if (err < 0 || p_response->success == 0) {
        at_send_command("AT+CREG=1", NULL, getChannelCtxbyId(AT_CHANNEL));
    }

    at_response_free(p_response);

    /*  GPRS registration events */
    //at_send_command("AT+CGREG=1", NULL);

    /*  Call Waiting notifications */
    //at_send_command("AT+CCWA=1", NULL);

    /*  Alternating voice/data off */
    //at_send_command("AT+CMOD=0", NULL);

    /*Report CSQ when RSSI changed*/
    //at_send_command("AT+ARSI=1,4", NULL);
    if (arsithrehd < 0)
    {
        arsithrehd = arsithrehddef;
    }
    LOGD("ARSI value:%d", arsithrehd);
    asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
    at_send_command(cmd, NULL, getChannelCtxbyId(AT_CHANNEL));
    /*Set prefer network registrate mode*/
    //at_send_command("AT^PREFMODE=8", NULL);
    /*  +CSSU unsolicited supp service notifications */
    //at_send_command("AT+CSSN=0,1", NULL);

    /*  no connected line identification */
    //at_send_command("AT+COLP=0", NULL);

    /*  HEX character set */
    //at_send_command("AT+CSCS=\"HEX\"", NULL);

    /*  USSD unsolicited */
    //at_send_command("AT+CUSD=1", NULL);

    /*  Enable +CGEV GPRS event notifications, but don't buffer */
    //at_send_command("AT+CGEREP=1,0", NULL);

    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL, getChannelCtxbyId(MISC_CHANNEL));

    /* clear all sms store in cbp */
    //at_send_command("AT+CMGD=,4", NULL); // delete for CRTS#20150 by djia

    /*Send URC +VMCCMNC when MCC/MNC changed in network */
    at_send_command("AT+VMCCMNC=1", NULL, getChannelCtxbyId(AT_CHANNEL));

    /*disable CBP wakeup AP because of ETS message*/
    at_send_command("AT+VUSBETS=0", NULL, getChannelCtxbyId(AT_CHANNEL));

    /* set cp rm interface protocol - Relay Layer Rm interface, PPP */
    at_send_command("AT+CRM=1", NULL, getChannelCtxbyId(AT_CHANNEL));


    /* Config IRAT mode: 0 AP IRAT(default), 1 MD IRAT, 2 MD IRAT LCG */
    if (getSvlteProjectType() >= SVLTE_PROJ_SC_3M
            && getSvlteProjectType() <= SVLTE_PROJ_SC_6M) {
        at_send_command("AT+EIRATMODE=0", NULL, getChannelCtxbyId(AT_CHANNEL));
    } else if (isCdmaLteDcSupport()) {
        if (isSvlteCdmaOnlySetFromEngMode() == 1) {
            at_send_command("AT+EIRATMODE=3", NULL, getChannelCtxbyId(AT_CHANNEL));
        } else if (isSvlteLcgSupport()) {
            at_send_command("AT+EIRATMODE=2", NULL, getChannelCtxbyId(AT_CHANNEL));
        } else {
            at_send_command("AT+EIRATMODE=1", NULL, getChannelCtxbyId(AT_CHANNEL));
        }
    }

    /*ETS log from USB*/
    //at_send_command("AT+VETSDEV=1", NULL);
    /*enable +VSER URC*/
    at_send_command("AT+VSER=1", NULL, getChannelCtxbyId(AT_CHANNEL));

    /*add for new service_state mechanism*/
    at_send_command("AT^PREFMODE?", NULL, getChannelCtxbyId(AT_CHANNEL));

    property_get("net.cdma.boottimes",boottimes, "0");
    LOGD("%s: boottimes is %s", __FUNCTION__, boottimes);
    if(!strcmp(boottimes,"2")) {
        if (isCdmaLteDcSupport() == 1) {
        LOGD("C2K rild init, svlte boottimes=2 skip autoEnterPinCode");
        } else {
            autoEnterPinCode();
        }
    }

    ///for test broadcast when user trigger the modem ,send EBOOT command
    property_get("ril.cdma.report.case", boottimes, "0");
    testSilentReboot = atoi(boottimes);

    ///for modem EE
    property_get("ril.cdma.report", boottimes, "0");
    modemEE = atoi(boottimes);

    ///for when user unlock the sim pin and enter flight mode
    property_get("cdma.ril.eboot", boottimes, "0");
    flight_mode = atoi(boottimes);

    LOGD("%s: testSilentReboot:%d, flight_mode:%d, modemEE:%d.",
            __FUNCTION__, testSilentReboot, flight_mode, modemEE);
    if (testSilentReboot == 1 || flight_mode == 1 || modemEE == 1) {
        LOGD("cdma.ril.eboot = 1 ,send command AT+EBOOT=1");
        at_send_command("AT+EBOOT=1", NULL, getDefaultChannelCtx());
    } else {
        LOGD("cdma.ril.eboot = 0 ,send command AT+EBOOT=0");
        at_send_command("AT+EBOOT=0", NULL, getDefaultChannelCtx());
    }
    property_set("cdma.ril.eboot", "0");
    property_set("ril.cdma.report.case", "0");
    ///for modem EE
    property_set("ril.cdma.report", "0");
    ///for svlte,c card is also need G modem unlock sim pin
//    property_set("ril.mux.report.case", "0");

    // RIL data initialization.
    rilDataInitialization(getDefaultChannelCtx());

#if 0
    /*GPS related (for testing by ldq)*/

    LOGD("GPS initializing");
    at_send_command("AT^GPSMODE=?",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSPROT=?",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSLOC=?",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSLUPD=?",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSQOS=?",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSFMT=?",NULL, getChannelCtxbyId(AT_CHANNEL));

    at_send_command("AT^GPSMODE=1",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSPROT=2",NULL, getChannelCtxbyId(AT_CHANNEL));
    at_send_command("AT^GPSLOC",NULL, getChannelCtxbyId(AT_CHANNEL));
#endif

#ifndef ADD_MTK_REQUEST_URC
    // add for HREF#19293
    getRUimStatusAhead();
#endif /* ADD_MTK_REQUEST_URC */

}

#ifdef ADD_MTK_REQUEST_URC
void setPinPukCountLeftProperty(int pin1, int pin2, int puk1, int puk2) {
    int slotid = 0;
    char str[16] = { 0 };

    slotid = getCdmaModemSlot();

    LOGD("%s: slotid:%d, pin1:%d, pin2:%d, puk1:%d, puk2:%d.",
            __FUNCTION__, slotid, pin1, pin2, puk1, puk2);

    switch (slotid) {
        case 1:
            // pin1
            sprintf(str, "%d", pin1);
            property_set("gsm.sim.retry.pin1", str);
            memset(str, 0, 16);
            // pin2
            sprintf(str, "%d", pin2);
            property_set("gsm.sim.retry.pin2", str);
            memset(str, 0, 16);
            // puk1
            sprintf(str, "%d", puk1);
            property_set("gsm.sim.retry.puk1", str);
            memset(str, 0, 16);
            // puk2
            sprintf(str, "%d", puk2);
            property_set("gsm.sim.retry.puk2", str);
            break;
        case 2:
            // pin1
            sprintf(str, "%d", pin1);
            property_set("gsm.sim.retry.pin1.2", str);
            memset(str, 0, 16);
            // pin2
            sprintf(str, "%d", pin2);
            property_set("gsm.sim.retry.pin2.2", str);
            memset(str, 0, 16);
            // puk1
            sprintf(str, "%d", puk1);
            property_set("gsm.sim.retry.puk1.2", str);
            memset(str, 0, 16);
            // puk2
            sprintf(str, "%d", puk2);
            property_set("gsm.sim.retry.puk2.2", str);
            break;
        default:
            LOGE("setPinPukCountLeftProperty, unsupport slot id %d", slotid);
            break;
    }
}
#endif /* ADD_MTK_REQUEST_URC */

// add for HREF#19293
void getRUimStatusAhead(void)
{
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char result[2] = { 0 };
    char pinCountRes[3] = { 0 };
    char *cpinLine;
    char *cpinResult;
    int pin1count, pin2count, puk1count, puk2count;
    int pinCount;
    LOGD("getUimStatusAhead");

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyId(AT_CHANNEL));

    if (err != 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    if (cta_no_uim)
    {
        LOGD("fake value uim status in getUimStatusAhead");
        ret = RUIM_ABSENT;
        goto done;
    }

    switch (at_get_cme_error(p_response))
    {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            ret = RUIM_ABSENT;
            goto done;

        default:
            ret = RUIM_NOT_READY;
            goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp(cpinResult, "SIM PIN"))
    {
        ret = RUIM_PIN;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "SIM PUK"))
    {
        ret = RUIM_PUK;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "UIM Error"))
    {
        LOGD("UIM Error");
        ret = RUIM_NOT_READY;
        goto done;
    }
    else if (0 != strcmp(cpinResult, "READY"))
    {
        /* we're treating unsupported lock types as "sim absent" */
        ret = RUIM_ABSENT;
        goto done;
    }

    ret = RUIM_READY;

done:
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
    sprintf(result, "%d", ret);
    LOGD("getUimStatusAhead, ret = %s", result);
    property_set("net.cdma.uim.status", result);
    if (ret == RUIM_PIN || ret == RUIM_PUK)
    {
        pinCount = -1;
        pin1count = pin2count = puk1count = puk2count = -1;
        getChvCountAhead(&pin1count, &pin2count, &puk1count, &puk2count);
        if (ret == RUIM_PIN)
        {
            pinCount = pin1count;
        }
        else
        {
            pinCount = puk1count;
        }
        sprintf(pinCountRes, "%d", pinCount);
        property_set("net.cdma.uim.pincount", pinCountRes);
    }
}

// add for HREF#19293
void getChvCountAhead(int * pin1Count, int * pin2Count, int * puk1Count,
        int * puk2Count)
{
    ATResponse *p_response = NULL;
    int err;
    char *cpinLine;
    char *cpinResult;
    int chvCount[3];
    int i;

    *pin1Count = 0;
    *pin2Count = 0;
    *puk1Count = 0;
    *puk2Count = 0;

    LOGD("getChvCountAhead");

    err = at_send_command_singleline("AT+CPINC?", "+CPINC:", &p_response, getChannelCtxbyId(AT_CHANNEL));

    if (err != 0)
    {
        LOGD("getChvCountAhead error : AT+CPINC failed");
        goto error;
    }

    switch (at_get_cme_error(p_response))
    {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            LOGD("getChvCountAhead error : CME_SIM_NOT_INSERTED");
            goto error;

        default:
            LOGD("getChvCountAhead error : not CME_SUCCESS");
            goto error;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0)
    {
        LOGD("getChvCountAhead error : at result error");
        goto error;
    }

    *pin1Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if (!cpinResult)
    {
        LOGD("getChvCountAhead error : puk1");
        goto error;
    }
    *puk1Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if (!cpinResult)
    {
        LOGD("getChvCountAhead error : pin2");
        goto error;
    }
    *pin2Count = atoi(cpinLine);
    at_tok_nextstr(&cpinLine, &cpinResult);
    if (!cpinLine)
    {
        LOGD("getChvCountAhead error : puk2");
        goto error;
    }
    *puk2Count = atoi(cpinLine);
    LOGD("getChvCountAhead : %d, %d, %d, %d", *pin1Count, *pin2Count,
            *puk1Count, *puk2Count);

    goto done;

error :
    *pin1Count = *pin2Count = *puk1Count = *puk2Count = -1;

done:
#ifdef ADD_MTK_REQUEST_URC
    setPinPukCountLeftProperty(*pin1Count, *pin2Count, *puk1Count, *puk2Count);
#endif
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
}

//ALPS02110463: Check and reset EMDSTATUS, 2015/06/12
static void resetRemoteSimProperties() {
    const char *PROPERTY_CONFIG_EMDSTATUS_SEND = "ril.cdma.emdstatus.send";
    property_set(PROPERTY_CONFIG_EMDSTATUS_SEND, "0");
    RLOGD("%s: Reset %s.", __FUNCTION__, PROPERTY_CONFIG_EMDSTATUS_SEND);
}

static void resetSystemProperties()
{
    LOGD("[C2K RIL_CALLBACK] resetSystemProperties");
	int slotid = getCdmaModemSlot();
    switch (slotid) {
        case 1:
            property_set("ril.ipo.radiooff", "0");
            LOGD("C2K set ril.ipo.radiooff to 0");
            break;
        case 2:
            property_set("ril.ipo.radiooff.2", "0");
            LOGD("C2K set ril.ipo.radiooff.2 to 0");
            break;
        default:
            LOGE("C2K resetSystemProperties, unsupport slot id %d", slotid);
            break;
    }
}

/**
 * Initialize everything that can be configured while we're still in
 * AT+CFUN=0
 */
static void initializeCallback(void *param)
{
    /*set radio state off to make the URC from readloop can be handled by onUnsolicited */
    setRadioState(RADIO_STATE_OFF);

    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    int cpbypass = 0;
    int bypass_ets_only = 0;
    int uimstatus = RUIM_ABSENT;
#if 0
    /* Called when CBP bypass */
    property_get("net.cdma.cpbypass.mode", tempstr, "0");
    cpbypass = atoi(tempstr);

    memset(tempstr, 0, sizeof(tempstr));
    property_get("net.cdma.cpbypass.etsonly", tempstr, "0");
    bypass_ets_only = atoi(tempstr);
#endif
    property_get("cbp.at.bypass", tempstr, "0");
    LOGD("initializeCallback cbp.at.bypass = %s", tempstr);
    cpbypass = atoi(tempstr);

    LOGD("cbp bypass mode=%d, etsonly=%d\n", cpbypass, bypass_ets_only);
    if ((0 != cpbypass) && (0 == bypass_ets_only))
    {
        LOGE("CBP bypass detected ; closing\n");

        at_close(getChannelCtxbyId(AT_CHANNEL));
        s_closed = 1;
        setRadioState(RADIO_STATE_UNAVAILABLE);
        sleep(5);

        return;
    }

    resetSystemProperties();

    //setRadioState (RADIO_STATE_OFF);
    atcommand_init();

    /* assume radio is off on error */
    if (isRadioOn() > 0)
    {
        setRadioState(RADIO_STATE_RUIM_NOT_READY);
    }
}

void waitAtInitDone() {
    if (s_at_init_finish == 0) {
        LOGD("C2K onRequest wait");
        int ret = pthread_mutex_lock(&s_at_init_Mutex);
        LOGD("C2K onRequest get lock");
        assert(ret == 0);
        if (s_at_init_finish == 0) {
            LOGD("C2K onRequest s_at_init_finish still 0 and wait for ready");
            pthread_cond_wait(&s_at_init_Cond, &s_at_init_Mutex);
            LOGD("C2K onRequest finish wait");
        }
        ret = pthread_mutex_unlock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("C2K onRequest finish unlock and ready to send");
    }
}

void notifyAtInitDone() {
    if (s_at_init_finish == 0) {
        // Only main loop modify s_at_init_finish, do not need to check its value again
        LOGD("at channel initialization done and try get lock");
        int ret = pthread_mutex_lock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("at channel initialization lock and get lock");
        s_at_init_finish = 1;
        pthread_cond_broadcast(&s_at_init_Cond);
        LOGD("at channel initialization broadcast done");
        ret = pthread_mutex_unlock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("at channel initialization lock and unlock");
    }
}

static void setTimespecRelative(struct timespec *p_ts, long long msec)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    p_ts->tv_sec = tv.tv_sec + (msec / 1000);
    p_ts->tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L) * 1000L;
}

void waitMd3InitDone() {
#ifndef MTK_ECCCI_C2K
    return;
#else /* MTK_ECCCI_C2K */
    LOGD("waitMd3InitDone start");
    if (s_md3_init_finish == 0) {
        int ret = pthread_mutex_lock(&s_md3_init_Mutex);
        assert(ret == 0);
        struct timespec ts;
        setTimespecRelative(&ts, VPUP_TIMEOUT_MSEC);
        // Wait +VPUP 15s and trigger assert if timeout
        int err = pthread_cond_timedwait(&s_md3_init_Cond, &s_md3_init_Mutex, &ts);
        if (err == ETIMEDOUT) {
            char modemException[PROPERTY_VALUE_MAX] = { 0 };
            property_get(PROPERTY_MODEM_EE, modemException, "");
            if (isMtkInternalSupport() || !isUserLoad()) {
                if (strcmp(modemException, "exception") != 0) {
                    LOG_ALWAYS_FATAL("waitMd3InitDone Wait +VPUP timeout", modemException);
                } else {
                    LOG_ALWAYS_FATAL("waitMd3InitDone Wait +VPUP timeout with modem EE",
                            modemException);
                }
            } else {
                if (!isModemPoweredOff()) {
                    LOGE("waitMd3InitDone Wait +VPUP timeout, reset modem");
                    triggerIoctl(CCCI_IOC_MD_RESET);
                } else {
                    // If modem power on, rild process will restart
                    LOGE("waitMd3InitDone Modem already powered off, ignore");
                }
            }
        } else {
            LOGD("waitMd3InitDone get signal");
        }
        ret = pthread_mutex_unlock(&s_md3_init_Mutex);
        assert(ret == 0);
    }
    LOGD("waitMd3InitDone done");
#endif /* MTK_ECCCI_C2K */
}

void notifyMd3InitDone() {
#ifndef MTK_ECCCI_C2K
    return;
#else /* MTK_ECCCI_C2K */
    if (s_md3_init_finish == 0) {
        int ret = pthread_mutex_lock(&s_md3_init_Mutex);
        assert(ret == 0);
        s_md3_init_finish = 1;
        pthread_cond_signal(&s_md3_init_Cond);
        ret = pthread_mutex_unlock(&s_md3_init_Mutex);
        assert(ret == 0);
        LOGD("notifyMd3InitDone +VPUP received, notify mainloop");
    } else {
        LOGD("notifyMd3InitDone MD3 already initialized");
    }
#endif /* MTK_ECCCI_C2K */
}

void sendPincode()
{
    char *cmd;
    ATResponse *p_response = NULL;
    int err;
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };
    char pincode[10] = { 0 };

    property_get("net.cdma.boottimes", boottimes, "0");
    LOGD("%s: boottimes is %s", __FUNCTION__, boottimes);
    if (strcmp(boottimes, "0") == 0)
    {
        property_set("net.cdma.boottimes", "1");
    }
    else if (strcmp(boottimes, "1") == 0)
    {
        property_set("net.cdma.boottimes", "2");
    }
}

/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
static void onUnsolicited(const char *s, const char *sms_pdu)
{
    LOGD("onUnsolicited enter s=%s", s);
    char *line = NULL;
    char *line_csq = NULL;
    char * output_string = NULL;
    int err;
    /**
     * Ignore unsolicited responses until we're initialized.
     * This is OK because the RIL library will poll for initial state
     */
    if (getRadioState() == RADIO_STATE_UNAVAILABLE && !strStartsWith(s, "+VPUP"))
    {
        LOGD("onUnsolicited return for radio");
        return;
    }

    if (!(rilNwUnsolicited(s, sms_pdu) || rilDataUnsolicited(s, sms_pdu)
            || rilCcUnsolicited(s, sms_pdu) || rilSimUnsolicited(s, sms_pdu)
            || rilSmsUnsolicited(s, sms_pdu) || rilSsUnsolicited(s, sms_pdu)
            || rilUtkUnsolicited(s, sms_pdu) || rilOemUnsolicited(s, sms_pdu)))
    {
        LOGE("Unhandled unsolicited result code: %s\n", s);
        LOGD("onUnsolicited exit unhandled");
    } else {
        LOGD("onUnsolicited exit normal");
    }
}

/* Called on command or reader thread */
static void onATReaderClosed(RILChannelCtx *p_channel)
{
    setIccidProperty(CLEAN_ICCID, NULL);
    LOGE("AT channel closed\n");
    at_close(p_channel);
    s_closed = 1;

    setRadioState (RADIO_STATE_UNAVAILABLE);
}

/* Called on command thread */
static void onATTimeout(RILChannelCtx *p_channel)
{
    int i = 0;
    int fd = -1;

    LOGE("AT channel timeout; closing\n");
    at_close(p_channel);

    s_closed = 1;
    if (!CmpBypassMode())
    {
        LOGE("reset CP, not in cp bypass mode");
        if (!CmpCbpResetMode()) {
            reset_cbp(AT_CHANNEL_TIMEOUT);
        }
    }
    else
    {
        LOGD("timed out but not caused by bypass");
    }

    /* FIXME cause a radio reset here */
    setRadioState(RADIO_STATE_UNAVAILABLE);
}


static void usage(char *s)
{
#ifdef RIL_SHLIB
    fprintf(stderr, "reference-ril requires: -p <tcp port> or -d /dev/tty_device\n");
#else /* RIL_SHLIB */
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif /* RIL_SHLIB */
}

#ifndef MTK_ECCCI_C2K
static int ril_com_callback(char *src_module,char *dst_module,char *data_type,int *data_len,unsigned char *buffer)
{
    int i = 0;
    char type=*data_type;
    unsigned char event=buffer[0];

    LOGD("msg from %s:%s\n", name_inquery(*src_module),type_inquery(event));
    if((type == STATUS_DATATYPE_CMD) && (event==CMD_KILL_CLIENT))
    {
        LOGD("set ril to exit");
        onATReaderClosed(getDefaultChannelCtx());
    }
    return 0;
}
#endif /* MTK_ECCCI_C2K */

void initATChannelPathes()
{
    int i = 0;
    for (i = 0; i < RIL_SUPPORT_CHANNELS; i++)
    {
        waitForAdjustPortPath(s_mux_fd_property_list[i], s_mux_path[i]);
    }
}

static void *mainLoop(void *param)
{
    int fd;
    int ret;
    int err_cnt = 0;
    int bypass_status = 0;
    int i = 0;
    int err = 0;
#ifdef USB_FS_EXCEPTION
    int atChannelCnt = 0;
    int dataChannelCnt = 0;
    int atResetCnt = 0;
    int dataResetCnt = 0;
    int otherResetCnt = 0;
    int sleepTime = 10; /*s*/
    int powerWaitTime = 10; /*s*/
    int maxRetryTime = 5;
#endif /* USB_FS_EXCEPTION */

#ifdef ADD_MTK_REQUEST_URC
    rildSockConnect = 0;
#endif /* ADD_MTK_REQUEST_URC */

    int bypassed = 0;

    AT_DUMP("== ", "entering mainLoop()", -1);
#ifndef MTK_ECCCI_C2K
    /**
     * Tell statusd ril is on the way
     */
    ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_INIT);
    if(ret < 1)
    {
        LOGD("CLIENT %s::send CMD_CLIENT_INIT cmd failed ret=%d\n",LOG_TAG,ret);
    }
#endif /* MTK_ECCCI_C2K */
    sendPincode();

    /* ALPS02110463: Check and reset EMDSTATUS, 2015/06/12 { */
    if (isCdmaLteDcSupport()) {
        resetRemoteSimProperties();
    } else {
        LOGD("Non SVLTE, by pass reset REMOTE SIM properties.");
    }
    /* ALPS02110463: Check and reset EMDSTATUS, 2015/06/12 } */

    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);

    /// M: Save main loop thread id. Used to lock and wait until init done before sending AT to MD
    ret = pthread_mutex_lock(&s_at_init_Mutex);
    LOGD("C2K mainloop get lock");
    assert(ret == 0);
    mainloop_thread_id = pthread_self();
    ret = pthread_mutex_unlock(&s_at_init_Mutex);
    assert(ret == 0);
    LOGD("C2K mainloop update thread_id and unlock");

    // If condition for google default workflow
    if (s_port > 0 || s_device_socket) {
        for (;;) {
            fd = -1;
            while (fd < 0) {
                if (s_port > 0) {
                    fd = socket_loopback_client(s_port, SOCK_STREAM);
                } else if (s_device_socket) {
                    if (!strcmp(s_device_path, "/dev/socket/qemud")) {
                        /* Qemu-specific control socket */
                        fd = socket_local_client("qemud",
                                ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
                        if (fd >= 0) {
                            char answer[2];

                            if (write(fd, "gsm", 3) != 3
                                    || read(fd, answer, 2) != 2
                                    || memcmp(answer, "OK", 2) != 0) {
                                close(fd);
                                fd = -1;
                            }
                        }
                    } else {
                        fd = socket_local_client(s_device_path,
                                ANDROID_SOCKET_NAMESPACE_FILESYSTEM,
                                SOCK_STREAM);
                    }
                } else if (s_device_path != NULL) {
                    fd = open(s_device_path, O_RDWR);
                    if (fd >= 0 && !memcmp(s_device_path, "/dev/ttyS", 9)) {
                        /* disable echo on serial ports */
                        struct termios ios;
                        tcgetattr(fd, &ios);
                        ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                        ios.c_iflag = 0;
                        tcsetattr(fd, TCSANOW, &ios);
                    }
                }
#if 0
                else if (s_device_range_begin >= 0 && s_device_range_end >= 0) {
                    RLOGD("Open ttyS....");
                    ttys_index = s_device_range_begin;
                    while (ttys_index <= s_device_range_end) {
                        sprintf(path, "/dev/ttyS%d", ttys_index);
                        fd = open(path, O_RDWR);
                        if (fd >= 0) {
                            /* disable echo on serial ports */
                            struct termios ios;
                            tcgetattr(fd, &ios);
                            ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                            ios.c_iflag = 0;
                            tcsetattr(fd, TCSANOW, &ios);
                        } else {
                            RLOGE("Can't open the device /dev/ttyS%d: %s", ttys_index, strerror(errno));
                        }
                        ttys_index++;
                    }
                }
#endif

                if (fd < 0) {
                    perror("opening AT interface. retrying...");
                    sleep(10);
                    /* never returns */
                }
            }

            RLOGD("FD: %d", fd);

            s_closed = 0;
            ret = at_open_old(fd, onUnsolicited);

            if (ret < 0) {
                RLOGE("AT error %d on at_open\n", ret);
                return 0;
            }

            RIL_requestTimedCallback(initializeCallback, NULL, &TIMEVAL_0);

            // Give initializeCallback a chance to dispatched, since we don't presently have a cancellation mechanism
            sleep(1);

            waitForClose();
            RLOGI("Re-opening after close");
        }
    } else {
        initRILChannels();
        initATChannelPathes();
        RILChannelCtx *p_channel;
        for (i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
            p_channel = getChannelCtxbyId(i);
            while (p_channel->fd < 0) {
                do {
                    p_channel->fd = open(s_mux_path[i], O_RDWR);
                } while (p_channel->fd < 0 && errno == EINTR);

                LOGD("Channel %d path=%s, fd=%d", i, s_mux_path[i], p_channel->fd);
                if (p_channel->fd < 0) {
                    perror("opening AT interface. retrying...");
                    RLOGE("could not connect to %s: %s", s_mux_path[i],
                            strerror(errno));
                    sleep(10);
                    /* never returns */
                } else {
                    err_cnt = 0;
#ifdef MTK_ECCCI_C2K
                    /* disable echo on serial ports */
                    struct termios ios;
                    tcgetattr(p_channel->fd, &ios);
                    ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                    ios.c_iflag = 0;
                    tcsetattr(p_channel->fd, TCSANOW, &ios);
#else /* MTK_ECCCI_C2K */
                    /* disable echo on serial ports */
                    struct termios ios;
                    if (tcgetattr(p_channel->fd, &ios) < 0) {
                        LOGE("setMuxPorts : ERROR GET DEVICE TTY ATTR (fd = %d)", fd);
                    }

                    LOGD("AT WILL USE %s", s_device_path);
                    ios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
                    ios.c_cflag |= CREAD | CLOCAL | CS8;
                    ios.c_cflag &= ~(CRTSCTS);
                    ios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL
                            | ISIG);
                    ios.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY
                            | ICRNL);
                    ios.c_iflag &= ~(IXON | IXOFF);
                    ios.c_oflag &= ~(OPOST | OCRNL);
                    ios.c_cc[VMIN] = 1;
                    ios.c_cc[VTIME] = 0;
                    cfsetispeed(&ios, B115200);
                    cfsetospeed(&ios, B115200);
#ifndef USE_STATUSD
                    tcflush(p_channel->fd, TCIFLUSH);
#endif
                    if (tcsetattr(p_channel->fd, TCSANOW, &ios) < 0) {
                        // affect at now
                        LOGE("setMuxPorts: ERROR SET ATTR (fd = %d)", p_channel->fd);
                    }

                    struct serial_struct ss;
                    if (ioctl(p_channel->fd, TIOCGSERIAL, &ss) < 0) {
                        LOGE("%s: ERROR GET SERIAL STRUCT(fd = %d)",
                                __FUNCTION__, p_channel->fd);
                    }
                    ss.closing_wait = 3 * 100;
                    if (ioctl(p_channel->fd, TIOCSSERIAL, &ss) < 0) {
                        LOGE("%s: ERROR SET SERIAL STRUCT(fd = %d), errno is %d",
                                __FUNCTION__, p_channel->fd, errno);
                    }
#endif /* MTK_ECCCI_C2K */
                }
                s_closed = 0;

                ret = at_open(p_channel->fd, onUnsolicited, p_channel);
                if (ret < 0) {
                    LOGE("AT error %d on at_open\n", ret);
                    return 0;
                }
            }
        }
        // AT channels initialization done, notify other thread to begin send request
        notifyAtInitDone();

        RIL_requestProxyTimedCallback(initializeCallback, NULL, &TIMEVAL_0, getDefaultChannelCtx()->id);

#ifndef MTK_ECCCI_C2K
        /*tell statusd ril is ready*/
        ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_READY);
        if(ret < 1) {
            LOGD("CLIENT %s::send CMD_CLIENT_READY cmd failed ret=%d\n",LOG_TAG,ret);
        } else {
            LOGD("CLIENT %s::send CMD_CLIENT_READY\n",LOG_TAG,ret);
        }
#endif /* MTK_ECCCI_C2K */
        // Give initializeCallback a chance to dispatched, since
        // we don't presently have a cancellation mechanism
        sleep(1);

        waitForClose();
#if defined(FLASHLESS_SUPPORT) && defined(AT_AUTO_SCRIPT_RESET)
        LOGD("wait to die"); //wait to killed by flashlessd
        while(noLoop);
#endif

#ifndef MTK_ECCCI_C2K
        /*wait to receive kill msg from and be killed by statusd*/
        //sleep(2);
        /*deregister interface which is used for communicating with statusd*/

        statusd_deregister_cominf(MODULE_TYPE_RIL);
#endif /* MTK_ECCCI_C2K */
    }


    return NULL;
}

#ifdef RIL_SHLIB

pthread_t s_tid_mainloop;

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;
    pthread_attr_t attr;

    s_rilenv = env;

    g_oem_support_flag = get_oem_support_flag();

    /* if oem support enabled, get ril version property system */
    if (1/*oemSupportEnable(g_oem_support_flag)*/)
    {
        s_callbacks.version = get_oem_ril_version();
    }
    while ( -1 != (opt = getopt(argc, argv, "p:d:s:c:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                    return NULL;
                }
                LOGI("Opening loopback port %d\n", s_port);
                break;
            case 'd':
                memset(s_device_path, 0, sizeof(s_device_path));
                strcat(s_device_path, optarg);
                LOGI("Opening tty device %s\n", s_device_path);
                break;
            case 's':
                memset(s_device_path, 0, sizeof(s_device_path));
                strcat(s_device_path, optarg);
                s_device_socket = 1;
                LOGI("Opening socket %s\n", s_device_path);
                break;
            case 'c':
                LOGI("c2k skip the argument");
                break;
            default:
                usage(argv[0]);
                LOGI("RIL_Init return null");
                return NULL;
        }
    }

    if (s_port < 0 && s_device_path == NULL)
    {
        usage(argv[0]);
        return NULL;
    }

#ifndef MTK_ECCCI_C2K
    /*register interface which is used for communicating with statusd*/
    ret = statusd_register_cominf(MODULE_TYPE_RIL,(statusd_data_callback)ril_com_callback);
    if(ret)
    {
        LOGD("CLIENT %s :: register com interface failed\n",LOG_TAG);
    }
#endif /* MTK_ECCCI_C2K */

#ifdef ANDROID_KK
    sMdmInfo = calloc(1, sizeof(ModemInfo));
    if (!sMdmInfo)
    {
        LOGE("Unable to alloc memory for ModemInfo");
        return NULL;
    }
#endif /* ANDROID_KK */

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);

    return &s_callbacks;
}
#else /* RIL_SHLIB */
int main (int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while (-1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
        case 'p':
            s_port = atoi(optarg);
            if (s_port == 0) {
                usage(argv[0]);
            }
            LOGI("Opening loopback port %d\n", s_port);
            break;
        case 'd':
            s_device_path = optarg;
            LOGI("Opening tty device %s\n", s_device_path);
            break;
        case 's':
            s_device_path = optarg;
            s_device_socket = 1;
            LOGI("Opening socket %s\n", s_device_path);
            break;
        default:
            usage(argv[0]);
            break;
        }
    }

    if (s_port < 0 && s_device_path == NULL) {
        usage(argv[0]);
    }

    RIL_register(&s_callbacks);

#ifdef ANDROID_KK
    sMdmInfo = calloc(1, sizeof(ModemInfo));
    if (!sMdmInfo) {
        LOGE("Unable to alloc memory for ModemInfo");
        return NULL;
    }
#endif /* ANDROID_KK */

    mainLoop(NULL);

    return 0;
}

#endif /* RIL_SHLIB */

int waitUimModualInitialized(void)
{
    int ret = 0;
    int i = 0;
    int waitInterval = 200; //ms
    int maxWaitCount = 50; //waitInterval*maxWaitCount = 10s;

    for (i = 0; i < maxWaitCount; i++) {
        if (uim_modual_initialized
                || (UIM_STATUS_NO_CARD_INSERTED == uimInsertedStatus)) {
            break;
        }
        usleep(waitInterval * 1000);
    }
    LOGD("%s: uim_modual_initialized = %d, uimInsertedStatus = %d,wait %d ms",
            __FUNCTION__, uim_modual_initialized, uimInsertedStatus,
            i * waitInterval);
    return ret;
}

void setNetworkMode(void *param)
{
    int err = 0;
    ATResponse *p_response = NULL;
    int mode = *((int*) (param));
    char *cmd = NULL;

     if(mode < 0) {
        LOGD("%s: mode is %d", __FUNCTION__, mode);
        goto exit;
    }
    asprintf(&cmd, "AT^PREFMODE=%d", mode);
    err = at_send_command(cmd, &p_response, getChannelCtxbyId(AT_CHANNEL));
    free(cmd);

    if ((err < 0) || (p_response->success == 0))
    {
        goto exit;
    }

exit:
    at_response_free(p_response);
    return;
}

static void reportUSBDisconn()
{
    RIL_CALL_STATUS callStatus;

    LOGD("%s: s_PcModemState = %d", __FUNCTION__, s_PcModemState);

    if(getRadioState() != RADIO_STATE_UNAVAILABLE)
    {
         IsDatacallDialed(&callStatus);
         if(!callStatus.isDataCall && (s_PcModemState != PCMODEM_DISCONNECT))
         {
            s_PcModemState = PCMODEM_DISCONNECT;
            LOGD("%s: PC Modem disconnected", __FUNCTION__);
            sleep(3);
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

            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                    datacall_response, sizeof(RIL_Data_Call_Response_v11));
            RIL_requestProxyTimedCallback(setNetworkMode, (void *) &currentNetworkMode, NULL,
                    getDefaultChannelCtx()->id);
        }
    }
}

int waitIccidReadable(void)
{
    int ret = 0;
    int i = 0;
    int waitInterval = 200; //ms
    int maxWaitCount = 15;  //waitInterval*maxWaitCount = 3s;

    for(i = 0; i < maxWaitCount; i++)
    {
        if(iccid_readable)
        {
            break;
        }
        usleep(waitInterval * 1000);
    }
    LOGD("%s: iccid_readable = %d, uimInsertedStatus = %d,wait %d ms"
        , __FUNCTION__, iccid_readable, uimInsertedStatus, i * waitInterval);
    return ret;
}

static void autoEnterPinCode()
{
    ATResponse *p_response = NULL;

    char *cmd = NULL;
    char pincode[PROPERTY_VALUE_MAX] = { 0 };
    int pin;
    int err;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));

    waitUimModualInitialized();
    if(RUIM_PIN == getRUIMStatus(UPDATE_RADIO_STATUS))
    {
        property_get("net.cdma.pc", pincode, "none");
        LOGD("pincode = %s", pincode);
        /**
         * Actual pin code would never be "none" because
         * only (decimal) digits (0-9) allowed to be used.
         */
        if(strncmp(pincode, "none", 4))
        {
            if (1 != isRadioOn())
            {
                pin = atoi(pincode);
                pin = (pin -13) / 57;
                myitoa(pin, pincode, 10);
                asprintf(&cmd, "AT+CPIN=\"%s\"", pincode);
                err = at_send_command(cmd, &p_response, getChannelCtxbyId(AT_CHANNEL));
                free(cmd);
            }
            else
            {
                psParam.enable = 0;
                turnPSEnable((void *) &psParam);
                pin = atoi(pincode);
                pin = (pin - 13) / 57;
                myitoa(pin, pincode, 10);
                asprintf(&cmd, "AT+CPIN=\"%s\"", pincode);
                err = at_send_command(cmd, &p_response, getChannelCtxbyId(AT_CHANNEL));
                free(cmd);
                at_response_free(p_response);
                psParam.enable = 1;
                turnPSEnable((void *) &psParam);
            }
        }
    }
}

void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid)
{
    int temp = 0;
    char* iccidStr = NULL;

    if(1 == isDualtalkMode())
    {
        switch (type) {
            case SET_VALID_ICCID:
                asprintf(&iccidStr, "%s", pIccid);
                break;
            case SET_TO_NA:
                asprintf(&iccidStr, "%s", "N/A");
                break;
            case CLEAN_ICCID:
                asprintf(&iccidStr, "%s", "");
                break;
            default:
                LOGD("%s: unsupport type is %d", __FUNCTION__, type);
                break;
        }

        LOGD("%s: iccidStr is %s", __FUNCTION__, iccidStr);
        if (isCdmaLteDcSupport()) {
            /// M: For C2K SVLTE @{
            property_set("ril.iccid.sim1_c2k", iccidStr);
            RLOGI("Update property ril.iccid.sim1_c2k, value=%s", iccidStr);
            /// @}
        } else {
            if (getCdmaModemSlot() == 1) {
                property_set("ril.iccid.sim1", iccidStr);
                LOGD("set true iccid %s to ril.iccid.sim1", iccidStr);
            } else if (getCdmaModemSlot() == 2) {
                property_set("ril.iccid.sim2", iccidStr);
                LOGD("set true iccid %s to ril.iccid.sim2", iccidStr);
            } else {
                LOGD("can not get right value from getCdmaModemSlot");
            }
        }

        if (NULL != iccidStr) {
            free(iccidStr);
            iccidStr = NULL;
        }
    }
}

RIL_RadioState getRadioState(void)
{
    return sState;
}

void setRadioState(RIL_RadioState newState)
{
    RIL_RadioState oldState;
    LOGD("%s: newState = %d, sState = %d",__FUNCTION__, newState, sState);
    LOGD("%s: s_closed = %d",__FUNCTION__, s_closed);
    pthread_mutex_lock(&s_state_mutex);

    oldState = sState;

    if (s_closed > 0)
    {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (sState != newState || s_closed > 0)
    {
        sState = newState;
        /* when radio changed to NV_READY or RUIM_READY state, read subscription automaticlly */
        if (sState == RADIO_STATE_NV_READY || sState == RADIO_STATE_RUIM_READY)
        {
#if 0
            if(!s_triggerNitztime)        //If nitztime callback is inprogress, don't trigger again
                trigger_nitztime_report();
#endif
           at_send_command("AT+CCLK?", NULL, getChannelCtxbyId(AT_CHANNEL));
           RIL_requestProxyTimedCallback(trigger_update_cache, NULL,
                   &TIMEVAL_UPDATE_CACHE, getDefaultChannelCtx()->id);
        }
        pthread_cond_broadcast (&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);

    /* do these outside of the mutex */
    if (sState != oldState)
    {
        LOGD("setRadioState: before RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,sState = %d",sState);
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                NULL, 0);

        /* FIXME onRUimReady() and onRadioPowerOn() cannot be called
         * from the AT reader thread
         * Currently, this doesn't happen, but if that changes then these
         * will need to be dispatched on the request thread
         */
        if (sState == RADIO_STATE_RUIM_READY)
        {
            onRUIMReady();
        }
        else if (sState == RADIO_STATE_RUIM_NOT_READY)
        {
            onRadioPowerOn();
        }
    }
}

void waitForClose()
{
    pthread_mutex_lock(&s_state_mutex);

    while (s_closed == 0) {
        pthread_cond_wait(&s_state_cond, &s_state_mutex);
    }

    pthread_mutex_unlock(&s_state_mutex);
}

#ifndef MTK_ECCCI_C2K
void reset_cbp(CBP_RESET_CASE type)
{
    int fd = -1;
    int i = 0;
    char *errstr = NULL;
#ifdef USE_STATUSD
    int ret = -1;
    if( type == AT_CHANNEL_TIMEOUT)
    {
        ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_ERROR);
        LOGD("CLIENT %s::send CMD_CLIENT_ERROR cmd ret=%d\n",LOG_TAG,ret);
        if(ret != 1)
        {
            LOGD("CLIENT %s::send CMD_CLIENT_ERROR cmd failed ret=%d\n",LOG_TAG,ret);
        }
    }
#elif defined(FLASHLESS_SUPPORT)
    LOGD("reset_cbp start");
    /*Let flashlessd to handle the other type of CBP_RESET_CASE*/
    if( type == AT_CHANNEL_TIMEOUT)
    {
        viatelModemReset();
    }
    LOGD("reset_cbp stop");
    return ;
#else
    fd = open("/sys/power/modem_reset", O_RDWR);
    if(fd < 0)
    {
        errstr = strerror(errno);
        LOGE("Open modem_reset error, errno=%d, %s\n", errno, errstr);
    }
    else
    {
        i = write(fd, "reset", sizeof("reset"));
        if(i < 0)
        {
            errstr = strerror(errno);
            LOGE("write modem_reset errror, errno=%d, %s\n", errno, errstr);
        }
        else
        {
            LOGD("Reset the modem...\n");
        }
    }
    if(fd >=0)
    {
        close(fd);
    }
#endif
}
#else /* MTK_ECCCI_C2K */
void reset_cbp(CBP_RESET_CASE type)
{
    //TODO: Need implementation
}
#endif /* MTK_ECCCI_C2K */

#ifdef USB_FS_EXCEPTION
void powerCbp(int type)
{
    int fd = -1;
    int len = 0;
    char buf[8];
    char *errstr = NULL;

#ifdef FLASHLESS_SUPPORT
    return ;
#endif
    memset(buf , 0, sizeof(buf));
    if(type)
    {
        strcpy(buf, "on");
    }
    else
    {
        strcpy(buf, "off");
    }

    do
    {
        fd = open("/sys/power/modem_power", O_RDWR);
        if( fd < 0)
        {
            errstr = strerror(errno);
            LOGE("Open modem_power error, errno=%d, %s\n", errno, errstr);
            break;
        }

        len = strlen(buf);
        if(len == write(fd, buf, len))
        {
            LOGD("Power %s the modem...\n", buf);
        }
        else
        {
            errstr = strerror(errno);
            LOGE("write modem_reset errror, errno=%d, %s\n", errno, errstr);
            break;
        }
    } while(0);

    if(fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    return;
}

int CmpBypassMode(void)
{
    int bypassed = 0;
    char tempstr = 0;
    int ret;


    int sys_fd_at = open(SYS_AT_CHANNEL, O_RDONLY);
    if (sys_fd_at == -1)
    {
        LOGE("fail to open SYS_AT_CHANNEL!");
        return -1;
    }

    ret = read(sys_fd_at, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_AT_CHANNEL!");
        goto exit_at;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
    {
        LOGD("CmpBypassMode at channel is bypassed");
        goto exit_at;
    }
#if 0 //RIL Recovery mechanism should work even  either  DATA or  ETS  is bypassed.

    int sys_fd_data = open(SYS_DATA_CHANNEL, O_RDONLY);
    if (sys_fd_data == -1)
    {
        LOGE("fail to open SYS_DATA_CHANNEL!");
        goto exit_at;
    }

    ret = read(sys_fd_data, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_DATA_CHANNEL!");
        goto exit_data;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
    {
        LOGD("CmpBypassMode data port is bypassed");
        goto exit_data;
    }

    int sys_fd_ets = open(SYS_ETS_CHANNEL, O_RDONLY);
    if (sys_fd_ets == -1)
    {
        LOGE("fail to open SYS_ETS_CHANNEL!");
        goto exit_data;
    }

    ret = read(sys_fd_ets, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_ETS_CHANNEL!");
        goto exit_ets;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
        LOGD("CmpBypassMode ets port is bypassed");

exit_ets:
    close(sys_fd_ets);

exit_data:
    close(sys_fd_data);
#endif

exit_at:
    close(sys_fd_at);

    return bypassed;
}
#endif /* USB_FS_EXCEPTION */

int CmpPcModemMode(void)
{
    char tempstr = 0;
    int ret;

    int sys_fd_data = open(SYS_DATA_CHANNEL, O_RDONLY);
    if (sys_fd_data == -1)
    {
        LOGE("fail to open SYS_DATA_CHANNEL!");
        return -1;
    }

    ret = read(sys_fd_data, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_DATA_CHANNEL!");
        goto exit;
    }
    LOGD("CmpPcModemMode cbp.modem.bypass = %c", tempstr);

exit:
    close(sys_fd_data);
    return atoi(&tempstr);
}

int CmpCbpResetMode(void)
{
    char tempstr[PROPERTY_VALUE_MAX] = {0};
    int cbpResetMode = 0;

    property_get("persist.cp.reset.mode", tempstr, "0");
    LOGD("CmpCbpResetMode: persist.cp.reset.mode = %s", tempstr);

    cbpResetMode = atoi(tempstr);
    if(cbpResetMode)
    {
        LOGE("CmpCbpResetMode: not in cbp reset mode");
        return 1;
    }

    return 0;
}

#ifdef MTK_ECCCI_C2K
void waitForAdjustPortPath(int interface_num, char devicePath[])
{
    int atChannelCnt = 0;
    int sleepTime = 10; /*s*/
    int maxRetryTime = 5;
    char *atDevPath = NULL;
    for (;;) {
        LOGD("waitForAdjustPortPath interface_num=%d", interface_num);
        atDevPath = ccci_get_node_name(interface_num, MD_SYS3);
        if (NULL == atDevPath && atChannelCnt < maxRetryTime) {
            LOGD("failed to get at device node");
            atChannelCnt++;
            sleep(sleepTime);
            continue;
        } else {
            if (NULL == atDevPath) {
                LOG_ALWAYS_FATAL("waitForAdjustPortPath interface_num %d get fail", interface_num);
            } else {
                atChannelCnt = 0;
                memset(devicePath, 0, sizeof(devicePath));
                strcat(devicePath, atDevPath);
                atDevPath = NULL;
                LOGD("waitForAdjustPortPath devicePath = %s", devicePath);
                break;
            }
        }
    }
}
#else /* MTK_ECCCI_C2K */
void waitForAdjustPortPath(int interface_num, char devicePath[])
{
#ifdef USB_FS_EXCEPTION
    int atChannelCnt = 0;
    int atResetCnt = 0;
    int otherResetCnt = 0;
    int sleepTime = 10; /*s*/
    int powerWaitTime = 10; /*s*/
    int maxRetryTime = 5;
#endif
    char *atDevPath = NULL;
    for (;;) {
        LOGD("waitForAdjustPortPath interface_num=%d", interface_num);
        atDevPath = viatelAdjustDevicePathFromProperty(interface_num);
        if (NULL == atDevPath) {
            LOGD("failed to get at device from property");
#ifdef USB_FS_EXCEPTION
            atChannelCnt++;
            LOGE("adjust AT port error, retrying %d...\n", atChannelCnt);
            if ((atChannelCnt >= maxRetryTime) && (!CmpBypassMode())
                    && (!CmpCbpResetMode())) {
                atResetCnt++;
                LOGE("retry %d more times, reset CP for %d more times...\n",
                        atChannelCnt, atResetCnt);
                atChannelCnt = 0;
                reset_cbp(NO_DEVICE_PORT);
            }
            if ((atResetCnt >= maxRetryTime) && (!CmpBypassMode())
                    && (!CmpCbpResetMode())) {
                LOGE("reset CP for %d more times\n", atResetCnt);
                atResetCnt = 0;
                LOGE("start power off cbp...\n");
                powerCbp(0);
                sleep(powerWaitTime);
                LOGE("start power on cbp...\n");
                powerCbp(1);
            }
            sleep(sleepTime);
#endif /* USB_FS_EXCEPTION */
            continue;
        }
#ifdef USB_FS_EXCEPTION
        atChannelCnt = 0;
        atResetCnt = 0;
#endif /* USB_FS_EXCEPTION */
        memset(devicePath, 0, sizeof(devicePath));
        strcat(devicePath, atDevPath);
        atDevPath = NULL;
        LOGD("waitForAdjustPortPath devicePath = %s", devicePath);
        break;
    }
}
#endif /* MTK_ECCCI_C2K */

/* ppp start/stop is moved to ui */
void findPidByName( char* pidName, pid_t *number)
{
    DIR *dir;
    struct dirent *next;
    const int READ_BUF_SIZE = 512;

    dir = opendir("/proc");
    if (!dir)
    {
        LOGE(">>>>>>Cannot open /proc<<<<<<");
        return;
    }
    LOGD("findPidByName: finding pppd pid ");

    while ((next = readdir(dir)) != NULL) {
        FILE *status;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        char name[READ_BUF_SIZE];

        /* Must skip ".." since that is outside /proc */
        if (strcmp(next->d_name, "..") == 0)
            continue;

        /* If it isn't a number, we don't want it */
        if (!isdigit(*next->d_name))
            continue;

        sprintf(filename, "/proc/%s/status", next->d_name);
        if (!(status = fopen(filename, "r")))
        {
            continue;
        }
        if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL)
        {
            fclose(status);
            continue;
        }
        fclose(status);

        /* Buffer should contain a string like "Name:   binary_name" */
        sscanf(buffer, "%*s %s", name);
        if (strcmp(name, pidName) == 0) {
            *number=strtol(next->d_name, NULL, 0);
            LOGD("findPidByName: pppd pid is %d", *number);
            break;
        }
    }
    closedir(dir);
}

/** do post- RUIM ready initialization */
void onRUIMReady()
{
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
    at_send_command("AT+CNMI=1,1,0,2,0", NULL, getChannelCtxbyProxy());
}

/** do post-AT+CFUN=1 initialization */
void onRadioPowerOn()
{
    LOGD("onRadioPowerOn");

    pollRUIMState(NULL);
}

/**
 * RUIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS?, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
void pollRUIMState (void *param)
{
    ATResponse *p_response;
    int ret;
    LOGD("pollUIMState");
    if (getRadioState() != RADIO_STATE_RUIM_NOT_READY ) {
        // no longer valid to poll
        return;
    }

    switch(getRUIMStatus(UPDATE_RADIO_STATUS)) {
        case RUIM_NOT_READY:
        RIL_requestProxyTimedCallback(pollRUIMState, NULL, &TIMEVAL_SIMPOLL,
                getDefaultChannelCtx()->id);
            break;
        case RUIM_READY:
            setRadioState(RADIO_STATE_RUIM_READY);
            break;
        case RUIM_ABSENT:
        case RUIM_PIN:
        case RUIM_PUK:
        case RUIM_NETWORK_PERSONALIZATION:
        default:
            setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
            break;
    }
}

/**
 * SIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
static void pollSIMState (void *param)
{
    ATResponse *p_response;
    int ret;

    if (getRadioState() != RADIO_STATE_SIM_NOT_READY) {
        // no longer valid to poll
        return;
    }

    switch(getSIMStatus()) {
        case SIM_NOT_READY:
            RIL_requestProxyTimedCallback(pollSIMState, NULL,
                    &TIMEVAL_SIMPOLL, getDefaultChannelCtx()->id);
            break;
        case SIM_READY:
            setRadioState(RADIO_STATE_SIM_READY);
            break;
        case SIM_ABSENT:
        case SIM_PIN:
        case SIM_PUK:
        case SIM_NETWORK_PERSONALIZATION:
        default:
            setRadioState(RADIO_STATE_SIM_LOCKED_OR_ABSENT);
            break;
    }
}

int isCdmaLteDcSupport()
{
    int isSvlteSupport = 0;
    int isSrlteSupport = 0;

    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_support", property_value, "0");
    isSvlteSupport = atoi(property_value);
    RLOGI("isSvlteSupport: %d", isSvlteSupport);

    property_get("ro.mtk_srlte_support", property_value, "0");
    isSrlteSupport = atoi(property_value);
    RLOGI("isSrlteSupport: %d", isSrlteSupport);

    return (isSvlteSupport || isSrlteSupport) ? 1 : 0;
}

int isSrlteSupport()
{
    int isSrlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_srlte_support", property_value, "0");
    isSrlteSupport = atoi(property_value);
    RLOGI("isSrlteSupport: %d", isSrlteSupport);
    return isSrlteSupport;
}

int isSvlteSupport()
{
    int isSvlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_support", property_value, "0");
    isSvlteSupport = atoi(property_value);
    RLOGI("isSvlteSupport: %d", isSvlteSupport);
    return isSvlteSupport;
}

int isCdmaIratSupport()
{
    int isIratSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.c2k.irat.support", property_value, "0");
    isIratSupport = atoi(property_value);
    RLOGI("isCdmaIratSupport: %d", isIratSupport);
    return isIratSupport;
}

int isSvlteCdmaOnlySetFromEngMode()
{
    int isSetFromEngMode = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.radio.ct.ir.engmode", property_value, "0");
    isSetFromEngMode = atoi(property_value);
    RLOGI("isSvlteLcgSetFromEngMode(): %d", isSetFromEngMode);
    return isSetFromEngMode;
}

int isSvlteLcgSupport()
{
    int isSvlteLcgSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk_svlte_lcg_support", property_value, "0");
    isSvlteLcgSupport = atoi(property_value);
    RLOGI("isSvlteLcgSupport: %d", isSvlteLcgSupport);
    return isSvlteLcgSupport;
}

int isC2KWorldPhoneP2Support() {
    int isC2KWorldPhoneP2Support = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk.c2kwp.p2.support", property_value, "0");
    isC2KWorldPhoneP2Support = atoi(property_value);
    RLOGI("isC2KWorldPhoneP2Support: %d", isC2KWorldPhoneP2Support);
    return isC2KWorldPhoneP2Support;
}

int isC2KWorldPhoneSimSwitchSupport() {
    int isC2KWpSimSwitchSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.mtk.c2kwp.simswitch.support", property_value, "0");
    isC2KWpSimSwitchSupport = atoi(property_value);
    RLOGI("isC2KWpSimSwitchSupport: %d", isC2KWpSimSwitchSupport);
    return isC2KWpSimSwitchSupport;
}

int getSvlteProjectType() {
    int type = -1;
    char property_value_mode[PROPERTY_VALUE_MAX] = { 0 };
    char property_value_nw_sel[PROPERTY_VALUE_MAX] = { 0 };

    property_get("ro.mtk.c2k.om.mode", property_value_mode, "cllwtg");
    property_get("ro.mtk_c2k_om_nw_sel_type", property_value_nw_sel, "0");
    if (strcmp(property_value_nw_sel, "1") == 0) {
        if (strcmp(property_value_mode, "cllwtg") == 0) {
            type = SVLTE_PROJ_SC_6M;
        } else if (strcmp(property_value_mode, "cllwg") == 0) {
            type = SVLTE_PROJ_SC_5M;
        } else if (strcmp(property_value_mode, "cllg") == 0) {
            type = SVLTE_PROJ_SC_4M;
        } else if (strcmp(property_value_mode, "cwg") == 0) {
            type = SVLTE_PROJ_SC_3M;
        }
    } else {
        if (strcmp(property_value_mode, "cllwtg") == 0) {
            type = SVLTE_PROJ_DC_6M;
        } else if (strcmp(property_value_mode, "cllwg") == 0) {
            type = SVLTE_PROJ_DC_5M;
        } else if (strcmp(property_value_mode, "cllg") == 0) {
            type = SVLTE_PROJ_DC_4M;
        } else if (strcmp(property_value_mode, "cwg") == 0) {
            type = SVLTE_PROJ_DC_3M;
        }
    }
    RLOGI("getSvlteWwopType(): %s %s %d", property_value_mode, property_value_nw_sel, type);
    return type;
}

