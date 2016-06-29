/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#define MTK_LOG_ENABLE 1
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <linux/mtk_agps_common.h>
#include <private/android_filesystem_config.h>
// #include <linux/mtgpio.h>

#if EPO_SUPPORT
   // for EPO file
// xen0n: fix build
#include <curl/curl.h>
#include <curl/easy.h>

#define EPO_FILE "/data/misc/gps/EPO.DAT"
#define EPO_FILE_HAL "/data/misc/gps/EPOHAL.DAT"
#define MTK_EPO_SET_PER_DAY  4
#define MTK_EPO_MAX_DAY      30
#define MTK_EPO_ONE_SV_SIZE  72
#define MTK_EPO_SET_MAX_SIZE 2304     // 72*32, One SET
#define MTK_EPO_MAX_SET (MTK_EPO_MAX_DAY * MTK_EPO_SET_PER_DAY)
#define MTK_EPO_EXPIRED 3*24*60*60    // 7 days(s)
#define BUF_SIZE MTK_EPO_SET_MAX_SIZE
#define GPS_CONF_FILE_SIZE 100
#define EPO_CONTROL_FILE_PATH "/data/misc/gps.conf"
#define IS_SPACE(ch) ((ch == ' ') || (ch == '\t') || (ch == '\n'))
#endif

#define LOCATION_NLP_FIX "/data/misc/gps/LOCATION.DAT"
#define C_INVALID_FD -1
#define LOG_TAG  "gps_mtk"
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

#include <hardware/gps_mtk.h>

/* the name of the controlled socket */
#define  GPS_CHANNEL_NAME       "/dev/gps"
#define  GPS_MNL_DAEMON_NAME    "mnld"
#define  GPS_MNL_DAEMON_PROP    "init.svc.mnld"

#define  GPS_AT_COMMAND_SOCK    "/data/server"
#define  MNL_CONFIG_STATUS      "persist.radio.mnl.prop"

#define  GPS_OP   "AT%GPS"
#define  GNSS_OP  "AT%GNSS"
#define  GPS_AT_ACK_SIZE        60

#define  GPS_DEBUG  0
#define  NEMA_DEBUG 0   /*the flag works if GPS_DEBUG is defined*/
#if GPS_DEBUG
#define  TRC(f)       ALOGD("%s", __func__)
#define  ERR(f, ...)  ALOGE("%s: line = %d" f, __func__, __LINE__, ##__VA_ARGS__)
#define  WAN(f, ...)  ALOGW("%s: line = %d" f, __func__, __LINE__, ##__VA_ARGS__)
#define DBG(f, ...) ALOGD("%s: line = %d" f, __func__, __LINE__, ##__VA_ARGS__)
#define VER(f, ...) ((void)0)    // ((void)0)   //
#else
#  define TRC(...)    ((void)0)
#  define ERR(...)    ((void)0)
#  define WAN(...)    ((void)0)
#  define DBG(...)    ((void)0)
#  define VER(...)    ((void)0)
#endif
#define SEM 0
#define NEED_IPC_WITH_CODEC 1
#if (NEED_IPC_WITH_CODEC)
#include <private/android_filesystem_config.h>
#define EVDO_CODEC_SOC      "/data/misc/codec_sock"
#endif
static int flag_unlock = 0;
GpsStatus sta;

#define GPS_AOSP_MODE
#define GPS_AT_COMMAND_SOCK    "/data/server"                 // For receive AT command
#define MTK_HAL2MNLD           "/data/gps_mnl/hal2mnld"     // HAL forward AGPS Info to MNLD, FWK->JNI->HAL->MNLD
#define MTK_MNLD2HAL           "/data/gps_mnl/mnld2hal"     // MNLD forward AGPS Info to HAL, MNLD->HAL->JNI->FWK

typedef struct {
    int send_fd;
    AGpsCallbacks* agps_callbacks;
    GpsNiCallbacks* gps_ni_callbacks;
    AGpsRilCallbacks* agps_ril_callbacks;
} agps_context;

typedef  unsigned int             UINT4;
typedef  signed int               INT4;

typedef unsigned char           UINT8;
typedef signed char             INT8;

typedef unsigned short int      UINT16;
typedef signed short int        INT16;

typedef unsigned int            UINT32;
typedef signed int              INT32;

typedef signed long long       INT64;

#pragma pack(4)    //  Align by 4 byte
typedef struct
{
   UINT32 size;
   UINT32 flag;
   INT8 PRN;
   double TimeOffsetInNs;
   UINT16 state;
   INT64 ReGpsTowInNs;                       // Re: Received
   INT64 ReGpsTowUnInNs;                  // Re: Received, Un:Uncertainty
   double Cn0InDbHz;
   double PRRateInMeterPreSec;          //  PR: Pseuderange
   double PRRateUnInMeterPreSec;      //  PR: Pseuderange Un:Uncertainty
   UINT16 AcDRState10;                         //  Ac:Accumulated, DR:Delta Range
   double AcDRInMeters;                       //  Ac:Accumulated, DR:Delta Range
   double AcDRUnInMeters;                   //  Ac:Accumulated, DR:Delta Range, Un:Uncertainty
   double PRInMeters;                           //  PR: Pseuderange
   double PRUnInMeters;
   double CPInChips;                             //  CP: Code Phase
   double CPUnInChips;                         //  CP: Code Phase
   float CFInhZ;                                     //  CP: Carrier Frequency
   INT64 CarrierCycle;
   double CarrierPhase;
   double CarrierPhaseUn;                    //  Un:Uncertainty
   UINT8 LossOfLock;
   INT32 BitNumber;
   INT16 TimeFromLastBitInMs;
   double DopperShiftInHz;
   double DopperShiftUnInHz;              //  Un:Uncertainty
   UINT8 MultipathIndicater;
   double SnrInDb;
   double ElInDeg;                                //  El: elevation
   double ElUnInDeg;                            //  El: elevation, Un:Uncertainty
   double AzInDeg;                               //  Az: Azimuth
   double AzUnInDeg;                           //  Az: Azimuth
   char UsedInFix;
}MTK_GPS_MEASUREMENT;

typedef struct
{
   UINT32 size;
   UINT16 flag;
   INT16 leapsecond;
   UINT8 type;
   INT64 TimeInNs;
   double TimeUncertaintyInNs;
   INT64 FullBiasInNs;
   double BiasInNs;
   double BiasUncertaintyInNs;
   double DriftInNsPerSec;
   double DriftUncertaintyInNsPerSec;
}MTK_GPS_CLOCK;
typedef struct
{
   UINT32 size;
   INT8 type;
   UINT8 prn;
   INT16 messageID;
   INT16 submessageID;
   UINT32 length;
   UINT8 data[40];
} MTK_GPS_NAVIGATION_EVENT;
#pragma pack()
typedef struct{
    GpsUtcTime time;
    int64_t timeReference;
    int uncertainty;
} ntp_context;

typedef struct{
    double latitude;
    double longitude;
    float accuracy;
    struct timespec ts;
    unsigned char  type;
    unsigned char  started;
} nlp_context;


agps_context g_agps_ctx;

#if EPO_SUPPORT
#define  GPS_EPO_FILE_LEN  20
#define C_INVALID_TIMER -1  /*invalid timer */
static int gps_epo_period = 3;
static int wifi_epo_period = 1;
static int gps_epo_download_days = 30;
static int gps_epo_enable = 1;
static int gps_epo_wifi_trigger = 0;
static int gps_epo_file_count = 0;
static char gps_epo_file_name[GPS_EPO_FILE_LEN] = {0};
static char gps_epo_md_file_name[GPS_EPO_FILE_LEN] = {0};
static int gps_epo_type = 0;    // o for G+G;1 for GPS only, default is G+G
static int gnss_mode = 2;
const char *mnl_prop_path[] = {
    "/data/misc/gps/mnl.prop",   /*mainly for target*/
    "/sbin/mnl.prop",   /*mainly for emulator*/
};
typedef struct retry_alarm
{
    struct itimerspec expire;
    struct sigevent evt;
    timer_t fd;
}RETRY_ALARM_T;
static RETRY_ALARM_T retry_timer;
#endif
/*****************************************************************************/
/*    MTK device control                                                  */
/*****************************************************************************/
enum {
    MNL_CMD_UNKNOWN = -1,
    /*command send from GPS HAL*/
    MNL_CMD_INIT            = 0x00,
    MNL_CMD_CLEANUP         = 0x01,
    MNL_CMD_STOP            = 0x02,
    MNL_CMD_START           = 0x03,
    MNL_CMD_RESTART         = 0x04,    /*restart MNL process*/
    MNL_CMD_RESTART_HOT     = 0x05,    /*restart MNL by PMTK command: hot start*/
    MNL_CMD_RESTART_WARM    = 0x06,    /*restart MNL by PMTK command: warm start*/
    MNL_CMD_RESTART_COLD    = 0x07,    /*restart MNL by PMTK command: cold start*/
    MNL_CMD_RESTART_FULL    = 0x08,    /*restart MNL by PMTK command: full start*/
    MNL_CMD_RESTART_AGPS    = 0x09,    /*restart MNL by PMTK command: AGPS start*/
    MNL_CMD_GPSMEASUREMENT_INIT = 0x15,
    MNL_CMD_GPSMEASUREMENT_CLOSE = 0x16,

    MNL_CMD_GPSNAVIGATION_INIT = 0x17,
    MNL_CMD_GPSNAVIGATION_CLOSE = 0x18,

#if EPO_SUPPORT
   MNL_CMD_READ_EPO_TIME = 0x33,
   MNL_CMD_UPDATE_EPO_FILE = 0x34,
#endif
    MNL_CMD_GPS_LOG_WRITE = 0x40,

   MNL_CMD_ENABLE_AGPS_DEBUG = 0x42,
   MNL_CMD_DISABLE_AGPS_DEBUG = 0x43,

    /*please see mnld.c for other definition*/
};

enum {
    HAL_CMD_STOP_UNKNOWN = -1,
#if EPO_SUPPORT
    HAL_CMD_READ_EPO_TIME_DONE = 0x35,
    HAL_CMD_UPDATE_EPO_FILE_DONE = 0x36,

    HAL_CMD_READ_EPO_TIME_FAIL = 0x37,
    HAL_CMD_UPDATE_EPO_FILE_FAIL = 0x38,
#endif
    HAL_CMD_MNL_DIE = 0x41,
    HAL_CMD_GPS_ICON = 0x42,

    MNL_CMD_GPS_INJECT_TIME = 0x46,
    MNL_CMD_GPS_INJECT_LOCATION = 0x47,
    MNL_CMD_GPS_INJECT_TIME_REQ = 0x48,
    MNL_CMD_GPS_INJECT_LOCATION_REQ = 0x49,

    MNL_CMD_GPS_NLP_LOCATION_REQ = 0x4a,
    HAL_CMD_MEASUREMENT= 0x76,
    HAL_CMD_NAVIGATION= 0x77,
    HAL_CMD_SWITCH_AGPS_DEBUG_DONE = 0x44,
    HAL_CMD_SWITCH_AGPS_DEBUG_FAIL = 0X45,
};

   // HAL->MNLD->AGPSD
enum {
    AGPS_FRAMEWORK_INFO_DATA_CONN_OPEN = 125,
    AGPS_FRAMEWORK_INFO_DATA_CONN_FAILED,
    AGPS_FRAMEWORK_INFO_DATA_CONN_CLOSED,
    AGPS_FRAMEWORK_INFO_NI_MESSAGE,
    AGPS_FRAMEWORK_INFO_NI_RESPOND,
    AGPS_FRAMEWORK_INFO_SET_REF_LOC,
    AGPS_FRAMEWORK_INFO_SET_SET_ID,
    AGPS_FRAMEWORK_INFO_SET_SERVER,
    AGPS_FRAMEWORK_INFO_UPDATE_NETWORK_STATE,
    AGPS_FRAMEWORK_INFO_UPDATE_NETWORK_AVAILABILITY,
    AGPS_FRAMEWORK_INFO_DATA_CONN_OPEN_IP_TYPE,
    AGPS_FRAMEWORK_INFO_INSTALL_CERTIFICATES,
    AGPS_FRAMEWORK_INFO_REVOKE_CERTIFICATES,
    AGPS_FRAMEWORK_INFO_COUNT,
};

   // AGPSD->MNLD->HAL
enum {
    MNL_AGPS_CMD_NOTIFICATION               = 230,
    MNL_AGPS_CMD_DATA_CONN_REQ,
    MNL_AGPS_CMD_DATA_CONN_RELEASE,
    MNL_AGPS_CMD_REQUEST_SET_ID,
    MNL_AGPS_CMD_REQUEST_REFLOC,
    MNL_AGPS_CMD_AGPS_LOC,
    MNL_AGPS_CMD_NOTIFICATION_2,
    MNL_AGPS_CMD_DATA_CONN_REQ2,
};


typedef enum
{
    AGPS_SUPL_NOTIFY_ONLY = 1,
    AGPS_SUPL_NOTIFY_ALLOW_NO_ANSWER,
    AGPS_SUPL_NOTIFY_DENY_NO_ANSWER,
    AGPS_SUPL_NOTIFY_PRIVACY,
    AGPS_SUPL_NOTIFY_END
} agps_supl_notify_enum;


typedef struct {
    double          latitude;              // Represents latitude in degrees
    double          longitude;             // Represents longitude in degrees
    char            altitude_used;         // 0=disabled 1=enabled
    double          altitude;              // Represents altitude in meters above the WGS 84 reference
    char            speed_used;            // 0=disabled 1=enabled
    float           speed;                 // Represents speed in meters per second
    char            bearing_used;          // 0=disabled 1=enabled
    float           bearing;               // Represents heading in degrees
    char            accuracy_used;         // 0=disabled 1=enabled
    float           accuracy;              // Represents expected accuracy in meters
    char            timestamp_used;        // 0=disabled 1=enabled
    long long       timestamp;             // Milliseconds since January 1, 1970
} mnl_agps_agps_location;


#define M_START 0
#define M_STOP 1
#define M_CLEANUP 2
#define M_INIT  3
#define M_THREAD_EXIT 4
#define M_MNLDIE 5
typedef struct sync_lock
{
    pthread_mutex_t mutx;
    pthread_cond_t con;
    int condtion;
}SYNC_LOCK_T;
static SYNC_LOCK_T lock_for_sync[] = {{PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0},
                                {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0},
                                {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0},
                                {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0},
                                {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0},
                                {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0}};

const char* gps_native_thread = "GPS NATIVE THREAD";
static GpsCallbacks_mtk callback_backup_mtk;
static GpsMeasurementCallbacks measurement_callbacks;
static GpsNavigationMessageCallbacks navigation_callbacks;
static int start_flag = 0;
   // for different SV parse
typedef enum{
    GPS_SV = 0,
    GLONASS_SV,
    BDS_SV,
    GALILEO_SV,
}SV_TYPE;
static float report_time_interval = 0;
static int started = 0;
static int nw_connected = 0;
static int nw_type = 0;
#define nw_wifi 0x00000001
#define nw_data 0x00000000

#if EPO_SUPPORT
   // zqh: download EPO by request
static GpsXtraCallbacks mGpsXtraCallbacks;
typedef struct
{
    int length;
    char* data;
}EpoData;
EpoData epo_data;
static int epo_download_failed = 0;
static int epo_download_retry = 1;
char chip_id[PROPERTY_VALUE_MAX];

#endif
/*---------------------------------------------------------------------------*/
typedef struct {
    int sock;
} MTK_GPS;
static void measurement_callback(GpsData* data) {
    DBG("measurement_callback is called");
}

GpsMeasurementCallbacks sGpsMeasurementCallbacks = {
    sizeof(GpsMeasurementCallbacks),
    measurement_callback,
};

static void navigation_message_callback(GpsNavigationMessage* message) {
    DBG("navigation_message_callback is called");
}

GpsNavigationMessageCallbacks sGpsNavigationMessageCallbacks = {
    sizeof(GpsNavigationMessageCallbacks),
    navigation_message_callback,
};
/*****************************************************************************/

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2,
    CMD_RESTART = 3,
    CMD_DOWNLOAD = 4,

    CMD_TEST_START = 10,
    CMD_TEST_STOP = 11,
    CMD_TEST_SMS_NO_RESULT = 12,
};

static MTK_GPS mtk_gps = {
    .sock = -1,
};
static int gps_nmea_end_tag = 0;
struct sockaddr_un cmd_local;
struct sockaddr_un remote;
socklen_t remotelen;


/*****************************************************************************/
/*AT command test state*/
static int MNL_AT_TEST_FLAG = 0;
#define GPS_TEST_PRN 1
#ifdef GPS_AT_COMMAND
static int MNL_AT_SIGNAL_MODE = 0;
static int MNL_AT_SIGNAL_TEST_BEGIN = 0;

int MNL_AT_SET_ALARM       = 0;
int MNL_AT_CANCEL_ALARM    = 0;


enum {
    MNL_AT_TEST_UNKNOWN = -1,
    MNL_AT_TEST_START       = 0x00,
    MNL_AT_TEST_STOP        = 0x01,
    MNL_AT_TEST_INPROGRESS   = 0x02,
    MNL_AT_TEST_DONE       = 0x03,
    MNL_AT_TEST_RESULT_DONE = 0x04,
};
int MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
typedef struct {
    int test_num;
    int prn_num;
    int time_delay;
}HAL_AT_TEST_T;

static HAL_AT_TEST_T hal_test_data = {
    .test_num = 0,
    .prn_num = 0,
    .time_delay = 0,
};
#endif

static time_t start_time;
static time_t end_time;
#ifdef GPS_AT_COMMAND
int* Dev_CNr = NULL;
int prn[32] = {0};
int snr[32] = {0};
int cn = 0;
/*
* Test result array: error_code, theta(0), phi(0), Success_Num,
* Complete_Num, Avg_CNo, Dev_CNo, Avg_Speed(0)
*/
static int result[8] = {1, 0, 0, 0, 0, 0, 0, 0};
static int test_num = 0;
static int CNo, DCNo;
static int Avg_CNo = 0;
static int Dev_CNo = 0;
static int Completed_Num = 0;
static int Success_Num = 0;
static int Failure_Num = 0;
static int Wait_Num = 0;
static int sig_suc_num = 0;
static int Err_Code = 1;
#define MAX_VALID_STATUS_WAIT_COUNT 20   // 12
int test_mode_flag = 1;    // 0: USB mode, 1: SMS mode
#endif
static int gpsmeasurement_init_flag = 0; /*1:init, 0:close*/
static int gpsnavigation_init_flag = 0; /*1:init, 0:close*/
int sv_used_in_fix[256] = {0};       //  for multiple sv display
static int get_prop()
{
       // Read property
    char result[PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    if (property_get(MNL_CONFIG_STATUS, result, NULL)) {
        ret = result[6] - '0';
        DBG("gps.log: %s, %d\n", &result[6], ret);
    } else {
        ret = 0;
        DBG("Config is not set yet, use default value");
    }

    return ret;
}

static void get_condition(SYNC_LOCK_T *lock)
{
    int ret = 0;

    ret = pthread_mutex_lock(&(lock->mutx));
    DBG("ret mutex lock = %d\n", ret);

    while (!lock->condtion) {
        ret = pthread_cond_wait(&(lock->con), &(lock->mutx));
        DBG("ret cond wait = %d\n" , ret);
    }

    lock->condtion = 0;
    ret = pthread_mutex_unlock(&(lock->mutx));
    DBG("ret mutex unlock = %d\n", ret);

    return;
}

static void release_condition(SYNC_LOCK_T *lock)
{
    int ret = 0;


    ret = pthread_mutex_lock(&(lock->mutx));
    DBG("ret mutex lock = %d\n", ret);

    lock->condtion = 1;
    ret = pthread_cond_signal(&(lock->con));
    DBG("ret cond_signal = %d\n", ret);

    ret = pthread_mutex_unlock(&(lock->mutx));
    DBG("ret unlock= %d\n", ret);

    return;
}

   // -1 means failure
int bind_udp_socket(char* path) {
    int sockfd;
    struct sockaddr_un soc_addr;
    socklen_t addr_len;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ERR("ERR: socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }

    strcpy(soc_addr.sun_path, path);
    soc_addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(soc_addr.sun_path) + 1);

    unlink(soc_addr.sun_path);
    if (bind(sockfd, (struct sockaddr *)&soc_addr, addr_len) < 0) {
        ERR("ERR: bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        close(sockfd);
        return -1;
    }
    if (chmod(path, 0660) < 0)
        ERR("chmod error: %s", strerror(errno));
    if (chown(path, -1, AID_INET))
        ERR("chown error: %s", strerror(errno));

    return sockfd;
}

   // -1 means failure
int safe_read(int fd, void* buf, int len) {
    int n, retry = 10;

    if (fd < 0 || buf == NULL || len < 0) {
        ERR("ERR: safe_read fd=%d buf=%p len=%d\n", fd, buf, len);
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        ERR("ERR: safe_read fcntl failure reason=[%s]\n", strerror(errno));
    }

    while ((n = read(fd, buf, len)) <= 0) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
            goto exit;
        }
        goto exit;
    }
    return n;

exit:
    ERR("ERR: safe_read reason=[%s]\n", strerror(errno));
    return -1;
}

   // -1 means failed, 0 means success
int mtk_daemon_send(int sockfd, void* dest, char* buf, int size) {
    int ret = 0;
    int len = 0;
    struct sockaddr_un soc_addr;
    socklen_t addr_len;
    int retry = 10;

    strcpy(soc_addr.sun_path, dest);
    soc_addr.sun_family = AF_UNIX;
    addr_len = (offsetof(struct sockaddr_un, sun_path) + strlen(soc_addr.sun_path) + 1);

    DBG("mtk_daemon_send %d", sockfd);
    while ((len = sendto(sockfd, buf, size, 0,
        (const struct sockaddr *)&soc_addr, (socklen_t)addr_len)) == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        ERR("[hal2mnld] ERR: sendto dest=[%s] len=%d reason =[%s]\n",
            (char *)dest, size, strerror(errno));
        ret = -1;
        break;
    }
       // unlink(soc_addr.sun_path);
    return ret;
}
static void mtk_gps_update_location(nlp_context * location)
{
    FILE *fp = NULL;

    fp = fopen(LOCATION_NLP_FIX, "w");
    if (fp != NULL)
    {
        size_t count = 0;
        count = fwrite(location, sizeof(nlp_context), 1, fp);
        if (count != 1)
        {
            DBG("write count:%d, errno:%s\n", count, strerror(errno));
        }
        fclose(fp);
    }
}

void buff_put_int(int input, char* buff, int* offset) {
    *((int*)&buff[*offset]) = input;
    *offset += 4;
}

void buff_put_string(const char* str, char* buff, int* offset) {
    int len = strlen(str) + 1;

    *((int*)&buff[*offset]) = len;
    *offset += 4;

    memcpy(&buff[*offset], str, len);
    *offset += len;
}
void buff_put_binary(void* input, int len, char* buff, int* offset) {
    *((int*)&buff[*offset]) = len;
    *offset += 4;

    memcpy(&buff[*offset], input, len);
    *offset += len;
}

void buff_put_struct(void* input, int size, char* buff, int* offset) {
    memcpy(&buff[*offset], input, size);
    *offset += size;
}


int buff_get_int(char* buff, int* offset) {
    int ret = *((int*)&buff[*offset]);
    *offset += 4;
    return ret;
}

int buff_get_string(char* str, char* buff, int* offset) {
    int len = *((int*)&buff[*offset]);
    *offset += 4;

    memcpy(str, &buff[*offset], len);
    *offset += len;
    return len;
}
void buff_get_struct(char* output, int size, char* buff, int* offset) {
    memcpy(output, &buff[*offset], size);
    *offset += size;
}

int buff_get_binary(void* output, char* buff, int* offset) {
    int len = *((int*)&buff[*offset]);
    *offset += 4;

    memcpy(output, &buff[*offset], len);
    *offset += len;
    return len;
}

static void mtk_gps_send_location_to_mnl(void) {
    FILE *fp = NULL;

    fp = fopen(LOCATION_NLP_FIX, "r");
    if (fp != NULL) {
        nlp_context nlp_location;
        char buff[1024] = {0};
        int offset = 0;
        size_t count = 0;
        memset(&nlp_location, 0, sizeof(nlp_context));
        count = fread(&nlp_location, sizeof(nlp_context), 1, fp);
        if (count != 1) {
            DBG("read count:%d, errno:%s\n", count, strerror(errno));
        }
        nlp_location.started = started;
        buff_put_int(MNL_CMD_GPS_INJECT_LOCATION, buff, &offset);
        buff_put_struct(&nlp_location, sizeof(nlp_context), buff, &offset);
        if (g_agps_ctx.send_fd >= 0) {
            int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
            DBG("send location to mnl successfully\n");
        } else {
            ERR("g_agps_ctx.send_fd is not initialized\n");
        }
        fclose(fp);
    }
}

// return strlen without ending character
int asc_str_to_usc2_str(char* output, char* input) {
    int len = 2;

    output[0] = 0xfe;
    output[1] = 0xff;

    while (*input != 0) {
        output[len++] = 0;
        output[len++] = *input;
        input++;
    }

    output[len+1] = 0;
    output[len+2] = 0;
    return len;
}

void raw_data_to_hex_string(char* output, char* input, int input_len) {
    int i = 0;
    for (i = 0; i < input_len; i++) {
        int tmp = (input[i] >> 4) & 0xf;

        if (tmp >= 0 && tmp <= 9) {
            output[i*2] = tmp + '0';
        } else {
            output[i*2] = (tmp - 10) + 'A';
        }

        tmp = input[i] & 0xf;
        if (tmp >= 0 && tmp <= 9) {
            output[(i*2)+1] = tmp + '0';
        } else {
            output[(i*2)+1] = (tmp - 10) + 'A';
        }
    }
}

void agps_init(AGpsCallbacks* callbacks) {
    DBG("agps_init\n");
    g_agps_ctx.agps_callbacks = callbacks;
}
int agps_data_conn_open(const char* apn) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_data_conn_open\n");

    buff_put_int(AGPS_FRAMEWORK_INFO_DATA_CONN_OPEN, buff, &offset);
    buff_put_string(apn, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    return 0;
}
int agps_data_conn_open_with_apn_ip_type(const char* apn, ApnIpType apnIpType) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_data_conn_open_with_apn_ip_type");
    buff_put_int(AGPS_FRAMEWORK_INFO_DATA_CONN_OPEN_IP_TYPE, buff, &offset);
    buff_put_string(apn, buff, &offset);
    buff_put_int(apnIpType, buff, &offset);
    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
return 0;
}
int agps_data_conn_closed() {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_data_conn_closed\n");

    buff_put_int(AGPS_FRAMEWORK_INFO_DATA_CONN_CLOSED, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    return 0;
}
int agps_data_conn_failed() {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_data_conn_failed\n");

    buff_put_int(AGPS_FRAMEWORK_INFO_DATA_CONN_FAILED, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
        return 0;
}
int agps_set_server(AGpsType type, const char* hostname, int port) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_set_server  type=%d hostname=[%s] port=%d\n", type, hostname, port);

    buff_put_int(AGPS_FRAMEWORK_INFO_SET_SERVER, buff, &offset);
    buff_put_int(type, buff, &offset);
    buff_put_string(hostname, buff, &offset);
    buff_put_int(port, buff, &offset);

    int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    return res;
}
static const AGpsInterface mtkAGpsInterface = {
    sizeof(AGpsInterface),
    agps_init,
    agps_data_conn_open,
    agps_data_conn_closed,
    agps_data_conn_failed,
    agps_set_server,
    agps_data_conn_open_with_apn_ip_type,
};

int agps_install_certificates(const DerEncodedCertificate* certificates, size_t length) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_install_certificates");

    buff_put_int(AGPS_FRAMEWORK_INFO_INSTALL_CERTIFICATES, buff, &offset);
    buff_put_int(length, buff, &offset);
    buff_put_struct(certificates, length, buff, &offset);

    int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    return res;
}
int agps_revoke_certificates(const Sha1CertificateFingerprint* fingerprints, size_t length) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_revoke_certificates");

    buff_put_int(AGPS_FRAMEWORK_INFO_REVOKE_CERTIFICATES, buff, &offset);
    buff_put_int(length, buff, &offset);
    buff_put_struct(fingerprints, length, buff, &offset);
    int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    return res;
}

static const SuplCertificateInterface mtkSuplCertificateInterface = {
    sizeof(SuplCertificateInterface),
    agps_install_certificates,
    agps_revoke_certificates,
};

void gps_ni_init(GpsNiCallbacks *callbacks) {
    DBG("gps_ni_init\n");
    g_agps_ctx.gps_ni_callbacks = callbacks;
}
void gps_ni_respond(int notif_id, GpsUserResponseType user_response) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("gps_ni_respond  notif_id=%d user_response=%d\n", notif_id, user_response);

    buff_put_int(AGPS_FRAMEWORK_INFO_NI_RESPOND, buff, &offset);
    buff_put_int(notif_id, buff, &offset);
    buff_put_int(user_response, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
}



static const GpsNiInterface  mtkGpsNiInterface = {
    sizeof(GpsNiInterface),
    gps_ni_init,
    gps_ni_respond,
};


void agps_ril_init(AGpsRilCallbacks* callbacks) {
    DBG("agps_ril_init\n");
    g_agps_ctx.agps_ril_callbacks = callbacks;
}
void agps_ril_set_ref_location(const AGpsRefLocation *agps_reflocation, size_t sz_struct) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_ril_set_ref_location\n");

    buff_put_int(AGPS_FRAMEWORK_INFO_SET_REF_LOC, buff, &offset);
    buff_put_struct((void *)agps_reflocation, sizeof(AGpsRefLocation), buff, &offset);
    buff_put_int(sz_struct, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));

}
void agps_ril_set_set_id(AGpsSetIDType type, const char* setid) {
    char buff[1024] = {0};
    int offset = 0;
       // DBG("agps_ril_set_set_id  type=%d setid=[%s]\n", type, setid);

    buff_put_int(AGPS_FRAMEWORK_INFO_SET_SET_ID, buff, &offset);
    buff_put_int(type, buff, &offset);
    buff_put_string(setid, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
}
void agps_ril_ni_message(uint8_t *msg, size_t len) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_ril_ni_message  len=%d\n", len);

    buff_put_int(AGPS_FRAMEWORK_INFO_NI_MESSAGE, buff, &offset);
    buff_put_binary(msg, len, buff, &offset);

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
}
void agps_network_connected_wifi_tragger_nlp(void)
{
    char buff[1024] = {0};
    int offset = 0;

    TRC();
    buff_put_int(MNL_CMD_GPS_NLP_LOCATION_REQ, buff, &offset);
    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));

}
void agps_ril_update_network_state(int connected, int type, int roaming, const char* extra_info) {
    char buff[1024] = {0};
    int offset = 0;
    DBG("agps_ril_update_network_state  connected=%d type=%d roaming=%d extra=%s\n",
        connected, type, roaming, extra_info);
    DBG("nw_connected =%d\n", nw_connected);
    if (nw_connected == 0 && connected == 1 && type == nw_wifi)
    {
        /*send nlp request to mnld */
        agps_network_connected_wifi_tragger_nlp();
    }
    nw_connected = connected;
    nw_type = type;
    buff_put_int(AGPS_FRAMEWORK_INFO_UPDATE_NETWORK_STATE, buff, &offset);
    buff_put_int(connected, buff, &offset);
    buff_put_int(type, buff, &offset);
    buff_put_int(roaming, buff, &offset);
    if (extra_info == NULL) {
        buff_put_string("", buff, &offset);
    } else {
        buff_put_string(extra_info, buff, &offset);
    }

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
}
void agps_ril_update_network_availability(int avaiable, const char* apn) {
    char buff[1024] = {0};
    int offset = 0;

    if (apn == NULL) {
        DBG("The apn is NULL");
    } else {
        DBG("agps_ril_update_network_availability  avaiable=%d apn=[%s]\n",
            avaiable, apn);
    }

    buff_put_int(AGPS_FRAMEWORK_INFO_UPDATE_NETWORK_AVAILABILITY, buff, &offset);
    buff_put_int(avaiable, buff, &offset);

    if (apn == NULL) {
        buff_put_string("", buff, &offset);
    } else {
        buff_put_string(apn, buff, &offset);
    }

    mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
}

static const AGpsRilInterface  mtkAGpsRilInterface = {
    sizeof(AGpsRilInterface),
    agps_ril_init,
    agps_ril_set_ref_location,
    agps_ril_set_set_id,
    agps_ril_ni_message,
    agps_ril_update_network_state,
    agps_ril_update_network_availability,
};

struct prop_info {
    unsigned volatile serial;
    char value[PROP_VALUE_MAX];
    char name[0];
};

typedef struct prop_info prop_info;
/*****************************************************************************/
int mtk_start_daemon() { /*gps driver must exist before running the function*/
#ifndef GPS_AOSP_MODE
    int start = 0;
    char status[PROPERTY_VALUE_MAX] = {0};
    int count = 100, len;
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    const prop_info *pi = NULL;
    unsigned serial = 0;
#endif
    TRC();

#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    pi = __system_property_find(GPS_MNL_DAEMON_PROP);
    if (pi != NULL)
        serial = pi->serial;
#endif

    property_set("ctl.start", GPS_MNL_DAEMON_NAME);
    sched_yield();

    while (count-- > 0) {
 #ifdef HAVE_LIBC_SYSTEM_PROPERTIES
        if (pi == NULL) {
            pi = __system_property_find(GPS_MNL_DAEMON_PROP);
        }
        if (pi != NULL) {
            __system_property_read(pi, NULL, status);
            if (strcmp(status, "running") == 0) {
                DBG("running\n");
                return 0;
            } else if (pi->serial != serial &&
                    strcmp(status, "stopped") == 0) {
                return -1;
            }
        }
#else
        if (property_get(GPS_MNL_DAEMON_PROP, status, NULL)) {
            if (strcmp(status, "running") == 0)
                return 0;
        }
#endif
        WAN("[%02d] '%s'\n", count, status);
        usleep(100000);
    }
    ERR("start daemon timeout!!\n");
    return -1;
#else
    return 0;
#endif
}
/*****************************************************************************/
int mtk_stop_daemon() {
#ifndef GPS_AOSP_MODE
    char status[PROPERTY_VALUE_MAX] = {0};
    int count = 50;

    TRC();
    if (property_get(GPS_MNL_DAEMON_PROP, status, NULL) &&
        strcmp (status, "stopped") == 0) {
        property_set(GPS_MNL_PROCESS_STATUS, "0000");
        DBG("daemon is already stopped!!");
        return 0;
    }

    property_set("ctl.stop", GPS_MNL_DAEMON_NAME);
    sched_yield();

    while (count-- > 0) {
        if (property_get(GPS_MNL_DAEMON_PROP, status, NULL)) {
            if (strcmp(status, "stopped") == 0) {
                property_set(GPS_MNL_PROCESS_STATUS, "0000");
                DBG("daemon is stopped, set %s 0000", GPS_MNL_PROCESS_STATUS);
                return 0;
            }
        }
        usleep(100000);
    }
    ERR("stop daemon timeout!!\n");
    return -1;
#else
    return 0;
#endif
}
/*****************************************************************************/
int mtk_daemon_init() {
#if 0
    int err = -1, lsocket;
    int count = 10;

    if (mtk_start_daemon() < 0) {
        ERR("start daemon fail: %s\n", strerror(errno));
        return -1;
    }

    lsocket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (lsocket < 0) {
        ERR("fail to create socket: %s\n", strerror(errno));
        return -1;
    }

    while (count-- > 0) {
        err = socket_local_client_connect(lsocket, GPS_MNL_DAEMON_NAME,
                                    ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
        /*connect success*/
        if (err >= 0)
            break;
        /*connect retry*/
        WAN("[%02d] retry\n", count);
        err = mt3326_start_daemon();
        if (err < 0)
            break;
        usleep(100000);
    }
    if (err < 0) {
        ERR("socket_local_client_connect fail: %s\n", strerror(errno));
        close(lsocket);
        return -1;
    }

    mtk_gps.sock = lsocket;
    DBG("socket : %d\n", lsocket);
#endif
    return 0;
}
/*****************************************************************************/
int mtk_daemon_cleanup() {
    TRC();

/* for received NI message when GPS is disabled
    if (mtk_gps.sock != -1)
        close(mtk_gps.sock);
*/
    return mtk_stop_daemon();
}
/*****************************************************************************/
int mtk_init()
{
    int err;
    char buf[] = {MNL_CMD_INIT};
    TRC();
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_cleanup()
{
    int err, res;
    char buf[] = {MNL_CMD_CLEANUP};
    TRC();
    if ((err = mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf))))
        ERR("mtk_cleanup send cleanup fails\n");
    DBG("let MNLD die\n");
    usleep(100000);
    res = mtk_daemon_cleanup();
       // DBG("cleanup: %d, %d\n", err, res);
    return 0;
}
/*****************************************************************************/
int mtk_start()
{
    char buf[] = {MNL_CMD_START};
    int err;
    int idx = 0, max = 5;
    TRC();
    err = mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
    /*sometimes, the mnld is restarted, so */
    while (err && (errno == EPIPE) && (idx++ < max)) {
        sleep(1);
        err = mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
    }
    return err;
}
/*****************************************************************************/
int mtk_stop()
{
    char buf[] = {MNL_CMD_STOP};
    TRC();
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_restart(unsigned char cmd)
{
    char buf[] = {cmd};
    TRC();
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_measurement_init()
{
    char buf[] = {MNL_CMD_GPSMEASUREMENT_INIT};
    TRC();
    DBG("Hal send MNL_CMD_GPSMEASUREMENT_INIT");
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_measurement_close()
{
    char buf[] = {MNL_CMD_GPSMEASUREMENT_CLOSE};
    TRC();
    DBG("Hal send MNL_CMD_GPSMEASUREMENT_CLOSE");
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_navigation_init()
{
    char buf[] = {MNL_CMD_GPSNAVIGATION_INIT};
    TRC();
    DBG("Hal send MNL_CMD_GPSNAVIGATION_INIT");
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}
/*****************************************************************************/
int mtk_navigation_close()
{
    char buf[] = {MNL_CMD_GPSNAVIGATION_CLOSE};
    TRC();
    DBG("Hal send MNL_CMD_GPSNAVIGATION_CLOSE");
    return mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buf, sizeof(buf));
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
    const char*  p;
    const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  24

typedef struct {
    int     count;
    Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init(NmeaTokenizer*  t, const char*  p, const char*  end)
{
    int    count = 0;
    char*  q;

       //  the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

       //  remove trailing newline
    if (end > p && (*(end-1) == '\n')) {
        end -= 1;
        if (end > p && (*(end-1) == '\r'))
            end -= 1;
    }

       //  get rid of checksum at the end of the sentecne
    if (end >= p+3 && (*(end-3) == '*')) {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (q >= p) {
            if (count < MAX_NMEA_TOKENS) {
                t->tokens[count].p   = p;
                t->tokens[count].end = q;
                count += 1;
            }
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    return count;
}

static Token
nmea_tokenizer_get(NmeaTokenizer*  t, int  index)
{
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}


static int
str2int(const char*  p, const char*  end)
{
    int   result = 0;
    int   len    = end - p;
    int   sign = 1;

    if (*p == '-')
    {
        sign = -1;
        p++;
        len = end - p;
    }

    for (; len > 0; len--, p++)
    {
        int  c;

        if (p >= end)
            goto Fail;

        c = *p - '0';
        if ((unsigned)c >= 10)
            goto Fail;

        result = result*10 + c;
    }
    return  sign*result;

Fail:
    return -1;
}

static double
str2float(const char*  p, const char*  end)
{
    int   result = 0;
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy(temp, p, len);
    temp[len] = 0;
    return strtod(temp, NULL);
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

   // #define  NMEA_MAX_SIZE  83
#define  NMEA_MAX_SIZE  128
/*maximum number of SV information in GPGSV*/
#define  NMEA_MAX_SV_INFO 4
#define  LOC_FIXED(pNmeaReader) ((pNmeaReader->fix_mode == 2) || (pNmeaReader->fix_mode ==3))
typedef struct {
    int     pos;
    int     overflow;
    int     utc_year;
    int     utc_mon;
    int     utc_day;
    int     utc_diff;
    GpsLocation  fix;

    /*
     * The fix flag extracted from GPGSA setence: 1: No fix; 2 = 2D; 3 = 3D
     * if the fix mode is 0, no location will be reported via callback
     * otherwise, the location will be reported via callback
     */
    int     fix_mode;
    /*
     * Indicate that the status of callback handling.
     * The flag is used to report GPS_STATUS_SESSION_BEGIN or GPS_STATUS_SESSION_END:
     * (0) The flag will be set as true when callback setting is changed via nmea_reader_set_callback
     * (1) GPS_STATUS_SESSION_BEGIN: receive location fix + flag set + callback is set
     * (2) GPS_STATUS_SESSION_END:   receive location fix + flag set + callback is null
     */
    int     cb_status_changed;
    int     sv_count;           /*used to count the number of received SV information*/
    GpsSvStatus   sv_status_gps;
    GnssSvStatus  sv_status_gnss;
    GpsCallbacks  callbacks;
#ifdef GPS_AT_COMMAND
   //     GpsTestResult test_result;
#endif
    char    in[ NMEA_MAX_SIZE+1 ];
} NmeaReader;


static void
nmea_reader_update_utc_diff(NmeaReader* const r)
{
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    unsigned long  time_local, time_utc;

    gmtime_r(&now, &tm_utc);
    localtime_r(&now, &tm_local);


    time_local = mktime(&tm_local);


    time_utc = mktime(&tm_utc);

    r->utc_diff = time_utc - time_local;
}


static void
nmea_reader_init(NmeaReader* const r)
{
    memset(r, 0, sizeof(*r));

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
    r->utc_diff = 0;
    r->callbacks.location_cb= NULL;
    r->callbacks.status_cb= NULL;
    r->callbacks.sv_status_cb= NULL;
    r->sv_count = 0;
    r->fix_mode = 0;    /*no fix*/
    r->cb_status_changed = 0;
    memset((void*)&r->sv_status_gps, 0x00, sizeof(r->sv_status_gps));
    memset((void*)&r->sv_status_gnss, 0x00, sizeof(r->sv_status_gnss));
    memset((void*)&r->in, 0x00, sizeof(r->in));

    nmea_reader_update_utc_diff(r);
}

GpsStatus sta;
static void
nmea_reader_set_callback(NmeaReader* const r, GpsCallbacks* const cbs)
{
    if (!r) {           /*this should not happen*/
        return;
    } else if (!cbs) {  /*unregister the callback */
#ifdef GPS_AT_COMMAND
        if (MNL_AT_TEST_FLAG) {
            DBG("**GPS AT Command test mode, unregister the test_cb!!");
            /* if (r->callbacks.test_cb != NULL) {
                DBG("**Unregister test_callback!!");
                r->callbacks.test_cb = NULL;
            } else {
                DBG("**test_callback is NULL");
            }*/
        }
#endif
        return;
    } else {/*register the callback*/

        if (MNL_AT_TEST_FLAG) {
#ifdef GPS_AT_COMMAND
               // r->callbacks.test_cb = cbs->test_cb;
            DBG("**GPS AT Command test mode!");
#endif
        }

        else
        {
            r->fix.flags = 0;
            r->sv_count = 0;
            r->sv_status_gps.num_svs = 0;
            r->sv_status_gnss.num_svs = 0;
        }
    }
}


static int
nmea_reader_update_time(NmeaReader* const r, Token  tok)
{
    int        hour, minute;
    double     seconds;
    struct tm  tm;
    struct tm  tm_local;
    time_t     fix_time;

    if (tok.p + 6 > tok.end)
        return -1;

    memset((void*)&tm, 0x00, sizeof(tm));
    if (r->utc_year < 0) {
           //  no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r(&now, &tm);
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    hour    = str2int(tok.p,   tok.p+2);
    minute  = str2int(tok.p+2, tok.p+4);
    seconds = str2float(tok.p+4, tok.end);

    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = (int) seconds;
    tm.tm_year = r->utc_year - 1900;
    tm.tm_mon  = r->utc_mon - 1;
    tm.tm_mday = r->utc_day;
    tm.tm_isdst = -1;

    if (mktime(&tm) == (time_t)-1)
        ERR("mktime error: %d %s\n", errno, strerror(errno));

    fix_time = mktime(&tm);
    localtime_r(&fix_time, &tm_local);

    fix_time += tm_local.tm_gmtoff;
    DBG("fix_time: %d\n", fix_time);
    r->fix.timestamp = (long long)fix_time * 1000;
    return 0;
}

static int
nmea_reader_update_date(NmeaReader* const r, Token  date, Token  time)
{
    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    return nmea_reader_update_time(r, time);
}


static double
convert_from_hhmm(Token  tok)
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}


static int
nmea_reader_update_latlong(NmeaReader* const r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi)
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        ERR("latitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        ERR("longitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    return 0;
}
/* this is the state of our connection to the daemon */
typedef struct {
    int                     init;
    int                     fd;
    GpsCallbacks            callbacks;
    pthread_t               thread;
#if EPO_SUPPORT
    pthread_t               thread_epo;
#endif
    int                     control[2];
    int                     sockfd;
    int                     test_time;
    int                     epoll_hd;
    int                     flag;
    int                     start_flag;
   //   int                     thread_exit_flag;
#if NEED_IPC_WITH_CODEC
    int                     sock_codec;
#endif
#if EPO_SUPPORT
    int                     epo_data_updated;
    int                     thread_epo_exit_flag;
#endif
} GpsState;

static GpsState  _gps_state[1];
#ifdef GPS_AT_COMMAND
/*
static int
nmea_reader_update_at_test_result(NmeaReader* const r,
                                  int Err_Code,
                                  int Success_Num,
                                  int Completed_Num,
                                  int Avg_CNo,
                                  int Dev_CNo)
{
    if (r==NULL) {
        DBG("**NmeaReader is NULL !!");
        return 0;
    }

    r->test_result.error_code = Err_Code;
    r->test_result.theta = 0;
    r->test_result.phi = 0;
    r->test_result.success_num = Success_Num;
    r->test_result.completed_num = Completed_Num;
    r->test_result.avg_cno = Avg_CNo;
    r->test_result.dev_cno = Dev_CNo;
    r->test_result.avg_speed = 0;
   /* if (r->callbacks.test_cb) {
        r->callbacks.test_cb(&r->test_result);
        DBG("**AT command test set callback!!");
    } else {
        VER("**AT Command test: no test result callback !!");
    }
    return 0;
}
*/
static void
sms_airtest_no_signal_report(int Err_Code,
                                  int Success_Num,
                                  int Completed_Num,
                                  int Avg_CNo,
                                  int Dev_CNo)
{
    GpsState*  s = _gps_state;
    // GpsTestResult result={0, };

    if (test_mode_flag == 0) {
         DBG("**Not SMS AirTest Mode, return!!");
        return;
    }

/*
    result.error_code = Err_Code;
    result.theta = 0;
    result.phi = 0;
    result.success_num = Success_Num;
    result.completed_num = Completed_Num;
    result.avg_cno = Avg_CNo;
    result.dev_cno = Dev_CNo;
    result.avg_speed = 0; */
       // s->callbacks.test_cb(&result);

    return;
}
#endif

static int
nmea_reader_update_altitude(NmeaReader* const r,
                             Token        altitude,
                             Token        units)
{
    double  alt;
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    r->fix.altitude = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_bearing(NmeaReader* const r,
                            Token        bearing)
{
    double  alt;
    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    r->fix.bearing  = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_speed(NmeaReader* const r,
                          Token        speed)
{
    double  alt;
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_SPEED;

       // Modify by ZQH to convert the speed unit from knot to m/s
       // r->fix.speed    = str2float(tok.p, tok.end);
    r->fix.speed = str2float(tok.p, tok.end) / 1.942795467;
    return 0;
}

   // Add by LCH for accuracy
static int
nmea_reader_update_accuracy(NmeaReader* const r,
                             Token accuracy)
{
    double  alt;
    Token   tok = accuracy;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
    r->fix.accuracy = str2float(tok.p, tok.end);
    return 0;
}
#ifdef GPS_AT_COMMAND
static void
gps_state_test_sms_airtest_no_result_report()
{
    char cmd = CMD_TEST_SMS_NO_RESULT;
    GpsState*  s = _gps_state;
    int ret;

    do {ret = write(s->control[0], &cmd, 1);}
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_TEST_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

// No use now
void alarm_handler() {
    DBG("Cannot detect GPS signal in 12s, stop test");
    if (MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN) {
        mtk_gps_test_stop();
        Err_Code = (1 << 5);   //  12secs timer timeout error bit.  Brian test
        result[0] = (1 << 5);   //  12secs timer timeout error bit.
        if (test_mode_flag) {
            //  soolim.you@lge.com, [SMSAirTest]
            //  In a weak GPS signal, If there is no CN measurement(alarming),
            //  null string should be reported using test_cb()
            gps_state_test_sms_airtest_no_result_report();
        }
    }
}

static void
gps_at_command_search_satellite(NmeaReader*  r)
{
    int i = 0, j = 0;
    for (i = 0; i < r->sv_count; i++) {
        if (prn[i] == hal_test_data.prn_num) {
            DBG("**AT Command test SvID: %d", prn[i]);
            if (snr[i] != 0) {
                if (MNL_AT_SIGNAL_MODE && (!MNL_AT_SIGNAL_TEST_BEGIN)) {
                    DBG("Set state to MNL_AT_TEST_INPROGRESS for read result");
                    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;
                }

                if (MNL_AT_SIGNAL_MODE && MNL_AT_SIGNAL_TEST_BEGIN) {
                    DBG("Set state to MNL_AT_TEST_RESULT_DONE for read result");
                    MNL_AT_TEST_STATE = MNL_AT_TEST_RESULT_DONE;
                    sig_suc_num = 1;
                       // sig_com_num = 1;
                }

                if (MNL_AT_TEST_FLAG) {
                    Err_Code = 0;
                    DBG("cn = %d", cn);
                    CNo += snr[i]*10;
                    Dev_CNr[Success_Num] = snr[i]*10;
                    Success_Num++;
                    Avg_CNo = CNo / Success_Num;
                       // test_num++;
                    DBG("CNo = %d, Avg_CNo /= %d, Success_Num = %d", CNo, Avg_CNo, Success_Num);
                }
                cn = snr[i]*10;
                DBG("cn = %d", cn);
            } else {
                DBG("**SNR is 0, ignore!!!");
                if (MNL_AT_CANCEL_ALARM == 1) {    // It's not timeout, just no signal after 12s
                    if (MNL_AT_TEST_FLAG == 1)
                        test_num++;
                }
                sig_suc_num = 0;
            }

            if (Success_Num != 0) {
                for (j = 0; j < Success_Num; j++) {
                    DCNo += (Dev_CNr[j]-Avg_CNo) * (Dev_CNr[j]-Avg_CNo);
                    DBG("Dev_CNr[%d] = %d, Dev_CNo2 += %d", j, Dev_CNr[j], DCNo);
                }
                Dev_CNo = DCNo / Success_Num;
                DCNo = 0;
                Dev_CNo = sqrt(Dev_CNo);
            }
            DBG("**AT Command find SvID: %d, exit", prn[i]);
            break;
        } else {
            DBG("**AT Command ignore SvID: %d", prn[i]);
        }
    }
}

static void
gps_at_command_test_proc(NmeaReader* const r) {
       // For AT command test
    int i = 0;
    int j = 0;
    int time_diff;
    time_t current_time;

    if (MNL_AT_TEST_STATE != MNL_AT_TEST_DONE) {
        DBG("**AT Command test mode!!");

        if (MNL_AT_SIGNAL_MODE == 1) {
            if (MNL_AT_SIGNAL_TEST_BEGIN) {
                gps_at_command_search_satellite(r);
                DBG("Update test result per second");
                result[0] = 0;
                result[3] = sig_suc_num;
                result[4] = 1;
                result[5]= cn;
                DBG("result[5] = %d", result[5]);
                result[6] = 0;
            } else {
                DBG("Wait AT%%%GNSS=1...");
                return;
            }
        } else {
            DBG("Not in MNL_AT_SIGNAL_MODE");
                DBG("**AT Command Continue, search satellites...");
                   // Search satellites
                gps_at_command_search_satellite(r);
        }

    } else {
        DBG("**AT Command test, test mode is MNL_AT_TEST_DONE");
    }
}
#endif

static int
nmea_reader_update_sv_status_gps(NmeaReader* r, int sv_index,
                              int id, Token elevation,
                              Token azimuth, Token snr)
{
       // int prn = str2int(id.p, id.end);
    int prn = id;
    if ((prn <= 0) || (prn < 65 && prn > GPS_MAX_SVS)|| (prn > 96) || (r->sv_count >= GPS_MAX_SVS)) {
        VER("sv_status_gps: ignore (%d)", prn);
        return 0;
    }
    sv_index = r->sv_count+r->sv_status_gps.num_svs;
    if (GPS_MAX_SVS <= sv_index) {
        ERR("ERR: sv_index=[%d] is larger than GPS_MAX_SVS.\n", sv_index);
        return 0;
    }
    r->sv_status_gps.sv_list[sv_index].prn = prn;
    r->sv_status_gps.sv_list[sv_index].snr = str2float(snr.p, snr.end);
    r->sv_status_gps.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status_gps.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    r->sv_count++;
    VER("sv_status_gps(%2d): %2d, %2f, %3f, %2f", sv_index,
       r->sv_status_gps.sv_list[sv_index].prn, r->sv_status_gps.sv_list[sv_index].elevation,
       r->sv_status_gps.sv_list[sv_index].azimuth, r->sv_status_gps.sv_list[sv_index].snr);
    return 0;
}

static int
nmea_reader_update_sv_status_gnss(NmeaReader* r, int sv_index,
                              int id, Token elevation,
                              Token azimuth, Token snr)
{
       // int prn = str2int(id.p, id.end);
    int prn = id;

    if ((prn <= 0) || (prn < 65 && prn > GPS_MAX_SVS)|| ((prn > 96) && (prn < 200))
       || (prn > 232) || (r->sv_count >= GNSS_MAX_SVS)) {
        VER("sv_status_gnss: ignore (%d)", prn);
        return 0;
    }
    sv_index = r->sv_count+r->sv_status_gnss.num_svs;
    if (GNSS_MAX_SVS <= sv_index) {
        ERR("ERR: sv_index=[%d] is larger than GNSS_MAX_SVS.\n", sv_index);
        return 0;
    }
    r->sv_status_gnss.sv_list[sv_index].prn = prn;
    r->sv_status_gnss.sv_list[sv_index].snr = str2float(snr.p, snr.end);
    r->sv_status_gnss.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status_gnss.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    if (1 == sv_used_in_fix[prn]) {
        r->sv_status_gnss.sv_list[sv_index].used_in_fix = true;
    } else {
        r->sv_status_gnss.sv_list[sv_index].used_in_fix = false;
    }
    r->sv_count++;
    VER("sv_status_gnss(%2d): %2d, %2f, %3f, %2f, %2d",
       sv_index, r->sv_status_gnss.sv_list[sv_index].prn,
       r->sv_status_gnss.sv_list[sv_index].elevation,
       r->sv_status_gnss.sv_list[sv_index].azimuth,
       r->sv_status_gnss.sv_list[sv_index].snr,
       r->sv_status_gnss.sv_list[sv_index].used_in_fix);
    return 0;
}

static void
nmea_reader_parse(NmeaReader* const r)
{
   /* we received a complete sentence, now parse it to generate
    * a new GPS fix...
    */
#ifdef GPS_AT_COMMAND
    int time_diff;
    time_t current_time;
    static int prev_success_num = 0;

    if ((1 == MNL_AT_TEST_FLAG) ||(1 == MNL_AT_SIGNAL_TEST_BEGIN)) {
        // (1 == MNL_AT_SIGNAL_MODE)) {
        DBG("MNL_AT_TEST_FLAG = %d, MNL_AT_SIGNAL_TEST_BEGIN = %d", MNL_AT_TEST_FLAG, MNL_AT_SIGNAL_TEST_BEGIN);
        time(&current_time);
        time_diff = current_time - start_time;
        if (time_diff >= hal_test_data.time_delay) {
                    DBG("MNL_AT_SET_ALARM == 1, gps_nmea_end_tag(%d)", gps_nmea_end_tag);
            if (gps_nmea_end_tag == 1) {
                int test_stopped = 0;
                DBG("Success_Num = %d, Prev_Success_Num = %d, Wait_Num = %d, \
                     Failure_Num = %d, Completed_Num = %d, Avg_CNo = %d, \
                     Dev_CNo = %d, Err_Code = %d, MNL_AT_TEST_STATE = %d",
                     Success_Num, prev_success_num, Wait_Num, Failure_Num, Completed_Num,
                     Avg_CNo, Dev_CNo, Err_Code, MNL_AT_TEST_STATE);
                if (Success_Num > 0) {
                    if (prev_success_num == Success_Num) {
                        Failure_Num++;
                }
                Completed_Num = Success_Num + Failure_Num;
                if (Completed_Num == hal_test_data.test_num) {
                    // 1. Call reportTestResult callback with detected SNR info
                    sms_airtest_no_signal_report(Err_Code, Success_Num, Completed_Num, Avg_CNo, Dev_CNo);
                    // 2. Test Stop
                    test_stopped = 1;
                }
             }
                else {
                    Wait_Num++;
                    if (Wait_Num == MAX_VALID_STATUS_WAIT_COUNT) {
                           // 1. Call reportTestResult callback with <32, 0, 0, 0, 0, 0>
                        DBG("TimeOut!! Wait_Num = %d", Wait_Num);
                        Completed_Num = hal_test_data.test_num;
                        Err_Code = (1 << 5);
                        Completed_Num = 1;
                        sms_airtest_no_signal_report(Err_Code, Success_Num, Completed_Num, 0, 0);
                           // 2. Test Stop
                        test_stopped = 1;
                    }
                }
                prev_success_num = Success_Num;

                if (test_stopped == 1) {
                    result[0] = Err_Code;
                    result[3] = Success_Num;
                    result[4] = Completed_Num;
                    result[5] = Avg_CNo;
                    result[6] = Dev_CNo;
                    Wait_Num = 0;
                    MNL_AT_TEST_STATE = MNL_AT_TEST_DONE;

                    DBG("**AT Command test_start done, Success_Num = %d, Completed_Num = %d, \
                       Avg_CNo = %d, Dev_CNo = %d, Err_Code = %d, test_num = %d, MNL_AT_TEST_STATE = %d",
                       Success_Num, Completed_Num, Avg_CNo, Dev_CNo, Err_Code, test_num, MNL_AT_TEST_STATE);
                    Err_Code = 1;
                    if ((MNL_AT_TEST_STATE == MNL_AT_TEST_DONE) && (1 == MNL_AT_TEST_FLAG)) {
                        DBG("** AT Command test done, stop GPS driver **");
                        mtk_gps_test_stop();
                        Failure_Num = 0;
                        prev_success_num = 0;
                    }
                }
            }

        } else {    //  2sec waiting
            DBG("static time, return...");
            return;
        }
    }
#endif
    NmeaTokenizer  tzer[1];
    Token          tok;
    Token          mtok;
    SV_TYPE sv_type = 0;

#if NEMA_DEBUG
    DBG("Received: '%.*s'", r->pos, r->in);
#endif
    if (r->pos < 9) {
        ERR("Too short. discarded. '%.*s'", r->pos, r->in);
        return;
    }
    if (r->pos < sizeof(r->in)) {
        nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
    }
#if NEMA_DEBUG
    {
        int  n;
        DBG("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer, n);
            DBG("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        ERR("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
        return;
    }

       //  ignore first two characters.
    mtok.p = tok.p;    // Mark the first two char for GPS, GLONASS, BDS SV parse.
    if (!memcmp(mtok.p, "BD", 2)) {
        sv_type = BDS_SV;
        DBG("BDS SV type");
    }
    tok.p += 2;
    if (!memcmp(tok.p, "GGA", 3)) {
           //  GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer, 1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer, 2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer, 4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 5);
        Token  tok_altitude      = nmea_tokenizer_get(tzer, 9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer, 10);

        nmea_reader_update_time(r, tok_time);
        nmea_reader_update_latlong(r, tok_latitude,
                                      tok_latitudeHemi.p[0],
                                      tok_longitude,
                                      tok_longitudeHemi.p[0]);
        nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

    } else if ((callback_backup_mtk.base.size == sizeof(GpsCallbacks_mtk)) &&
               (!memcmp(mtok.p, "GPGSA", 5)||!memcmp(mtok.p, "BDGSA", 5)||!memcmp(mtok.p, "GLGSA", 5))) {
        Token tok_fix = nmea_tokenizer_get(tzer, 2);
        int idx, max = 12;  /*the number of satellites in GPGSA*/

        r->fix_mode = str2int(tok_fix.p, tok_fix.end);

        if (LOC_FIXED(r)) {  /* 1: No fix; 2: 2D; 3: 3D*/
            for (idx = 0; idx < max; idx++) {
                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end) {
                    DBG("GSA: found %d active satellites\n", idx);
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                if (sv_type == BDS_SV) {
                                        sate_id += 200;
                }
                if (sate_id >= 1 && sate_id <= 32) {     // GP
                        sv_used_in_fix[sate_id] = 1;
                } else if (sate_id >= 193 && sate_id <= 197) {
                    sv_used_in_fix[sate_id] = 0;
                    DBG("[debug mask]QZSS, just ignore. satellite id is %d\n ", sate_id);
                    continue;
                } else if (sate_id >= 65 && sate_id <= 96) {     // GL
                        sv_used_in_fix[sate_id] = 1;
                } else if (sate_id >= 201 && sate_id <= 232) {     // BD
                        sv_used_in_fix[sate_id] = 1;

                }
                else {
                    VER("GSA: invalid sentence, ignore!!");
                    break;
                }
                DBG("GSA:sv_used_in_fix[%d] = %d\n", sate_id, sv_used_in_fix[sate_id]);
            }
        }
    } else if ((callback_backup_mtk.base.size == sizeof(GpsCallbacks)) &&
               (!memcmp(mtok.p, "GPGSA", 5))) {
        Token tok_fix = nmea_tokenizer_get(tzer, 2);
        int idx, max = 12;  /*the number of satellites in GPGSA*/

        r->fix_mode = str2int(tok_fix.p, tok_fix.end);
        r->sv_status_gps.used_in_fix_mask = 0;
        if (LOC_FIXED(r)) {  /* 1: No fix; 2: 2D; 3: 3D*/
            for (idx = 0; idx < max; idx++) {
                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end) {
                    DBG("GSA: found %d active satellites\n", idx);
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                if (sate_id >= 1 && sate_id <= 32) {
                    r->sv_status_gps.used_in_fix_mask |= (1 << (sate_id-1));
                } else {
                        if (sate_id >= 193 && sate_id <= 197) {
                            DBG("[debug mask]QZSS, just ignore. satellite id is %d\n ", sate_id);
                            continue;
                        } /*else {
                            r->sv_status_gps.used_in_fix_mask = 0;
                            DBG("[debug mask] satellite is invalid & mask = %d\n",
                               r->sv_status_gps.used_in_fix_mask);
                        }*/
                        VER("GPGSA: invalid sentence, ignore!!");
                        break;
                }
            }
        }
    }
    // VER("GPGSA: mask 0x%x", r->sv_status_gps.used_in_fix_mask);
    else if (!memcmp(tok.p, "RMC", 3)) {
        Token  tok_time          = nmea_tokenizer_get(tzer, 1);
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer, 2);
        Token  tok_latitude      = nmea_tokenizer_get(tzer, 3);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 4);
        Token  tok_longitude     = nmea_tokenizer_get(tzer, 5);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 6);
        Token  tok_speed         = nmea_tokenizer_get(tzer, 7);
        Token  tok_bearing       = nmea_tokenizer_get(tzer, 8);
        Token  tok_date          = nmea_tokenizer_get(tzer, 9);

        VER("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
        if (tok_fixStatus.p[0] == 'A')
        {
            nmea_reader_update_date(r, tok_date, tok_time);

            nmea_reader_update_latlong(r, tok_latitude,
                                           tok_latitudeHemi.p[0],
                                           tok_longitude,
                                           tok_longitudeHemi.p[0]);

            nmea_reader_update_bearing(r, tok_bearing);
            nmea_reader_update_speed(r, tok_speed);
        }
    } else if ((callback_backup_mtk.base.size == sizeof(GpsCallbacks_mtk)) &&
               (!memcmp(tok.p, "GSV", 3))) {
        Token tok_num = nmea_tokenizer_get(tzer, 1);    // number of messages
        Token tok_seq = nmea_tokenizer_get(tzer, 2);    // sequence number
        Token tok_cnt = nmea_tokenizer_get(tzer, 3);    // Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;
        if (sv_num > NMEA_MAX_SV_INFO)
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)   /*if sequence number is 1, a new set of GSV will be parsed*/
            r->sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {
            base_idx = base*(idx+1);
            Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
            int sv_id = str2int(tok_id.p, tok_id.end);
            if (sv_type == BDS_SV) {
                sv_id += 200;
                DBG("It is BDS SV: %d", sv_id);
            }
            Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
            Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
            Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
#ifdef GPS_AT_COMMAND
            prn[r->sv_count] = str2int(tok_id.p, tok_id.end);
            snr[r->sv_count] = (int)str2float(tok_snr.p, tok_snr.end);
#endif
            nmea_reader_update_sv_status_gnss(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
            if (r->sv_count <= cnt) {
                DBG("r->sv_count = %d", r->sv_count);
                r->sv_status_gnss.num_svs += r->sv_count;


#ifdef GPS_AT_COMMAND
                if ((1 == MNL_AT_TEST_FLAG) || (1 == MNL_AT_SIGNAL_MODE)) {
                        DBG("MNL_AT_TEST_FLAG = %d, MNL_AT_SIGNAL_MODE = %d", MNL_AT_TEST_FLAG, MNL_AT_SIGNAL_MODE);
                        gps_at_command_test_proc(r);
                }
#endif
            } else {
                ERR("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
                r->sv_count = r->sv_status_gnss.num_svs = 0;
            }
        }
    } else if ((callback_backup_mtk.base.size == sizeof(GpsCallbacks)) &&
               (!memcmp(mtok.p, "GPGSV", 5)||!memcmp(mtok.p, "GLGSV", 5))) {
        Token tok_num = nmea_tokenizer_get(tzer, 1);    // number of messages
        Token tok_seq = nmea_tokenizer_get(tzer, 2);    // sequence number
        Token tok_cnt = nmea_tokenizer_get(tzer, 3);    // Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;
        if (sv_num > NMEA_MAX_SV_INFO)
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)   /*if sequence number is 1, a new set of GSV will be parsed*/
            r->sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {
            base_idx = base*(idx+1);
            Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
            int sv_id = str2int(tok_id.p, tok_id.end);
            if (sv_type == BDS_SV) {
                sv_id += 200;
                DBG("It is BDS SV: %d", sv_id);
            }
            Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
            Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
            Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
#ifdef GPS_AT_COMMAND
            prn[r->sv_count] = str2int(tok_id.p, tok_id.end);
            snr[r->sv_count] = (int)str2float(tok_snr.p, tok_snr.end);
#endif
            nmea_reader_update_sv_status_gps(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
            if (r->sv_count <= cnt) {
                r->sv_status_gps.num_svs += r->sv_count;
                r->sv_status_gps.almanac_mask = 0;
                r->sv_status_gps.ephemeris_mask = 0;
#ifdef GPS_AT_COMMAND
                if ((1 == MNL_AT_TEST_FLAG) || (1 == MNL_AT_SIGNAL_MODE)) {
                        DBG("MNL_AT_TEST_FLAG = %d, MNL_AT_SIGNAL_MODE = %d", MNL_AT_TEST_FLAG, MNL_AT_SIGNAL_MODE);
                        gps_at_command_test_proc(r);
                }
#endif
            } else {
                ERR("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
                r->sv_count = r->sv_status_gps.num_svs = 0;
            }
        }
    }
       // Add for Accuracy
    else if (!memcmp(tok.p, "ACCURACY", 8)) {
        if ((r->fix_mode == 3) || (r->fix_mode == 2)) {
        Token  tok_accuracy = nmea_tokenizer_get(tzer, 1);
        nmea_reader_update_accuracy(r, tok_accuracy);
            DBG("GPS get accuracy from driver:%f\n", r->fix.accuracy);
        }
        else {
            DBG("GPS get accuracy failed, fix mode:%d\n", r->fix_mode);
        }
    }
    else {
        tok.p -= 2;
        VER("unknown sentence '%.*s", tok.end-tok.p, tok.p);
    }
    if (!LOC_FIXED(r)) {
        VER("Location is not fixed, ignored callback\n");
    } else if (r->fix.flags != 0 && gps_nmea_end_tag) {
#if NEMA_DEBUG
        char   temp[256];
        char*  p   = temp;
        char*  end = p + sizeof(temp);
        struct tm   utc;

        p += snprintf(p, end-p, "sending fix");
        if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
            p += snprintf(p, end-p, " accuracy=%g", r->fix.accuracy);
            DBG("GPS accuracy=%g\n", r->fix.accuracy);
        }
        gmtime_r((time_t*) &r->fix.timestamp, &utc);
        p += snprintf(p, end-p, " time=%s", asctime(&utc));
        VER(temp);
#endif
    if (get_prop()) {
        char pos[64] = {0};
        sprintf(pos, "%lf, %lf", r->fix.latitude, r->fix.longitude);
        DBG("gps postion str = %s", pos);
        char buff[1024] = {0};
        int offset = 0;
        DBG("gps postion lati= %f, longi = %f", r->fix.latitude, r->fix.longitude);
        buff_put_int(MNL_CMD_GPS_LOG_WRITE, buff, &offset);
        buff_put_string(pos, buff, &offset);
        mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buff, sizeof(buff));
    }
    callback_backup_mtk.base.location_cb(&r->fix);
    nlp_context fix_location;
    fix_location.latitude = r->fix.latitude;
    fix_location.longitude = r->fix.longitude;
    fix_location.accuracy = r->fix.accuracy;
    fix_location.type = 1;
    fix_location.started = started;
    fix_location.ts.tv_sec = r->fix.timestamp/1000;
    fix_location.ts.tv_nsec = (r->fix.timestamp%1000) * 1000*1000;
    mtk_gps_update_location(&fix_location);

    r->fix.flags = 0;
    }

    if (callback_backup_mtk.base.size == sizeof(GpsCallbacks_mtk)) {
        if (r->sv_status_gnss.num_svs != 0 && gps_nmea_end_tag) {
            DBG("r->sv_status_gnss.num_svs = %d, gps_nmea_end_tag = %d", r->sv_status_gnss.num_svs, gps_nmea_end_tag);
            r->sv_status_gnss.size = sizeof(GnssSvStatus);
            callback_backup_mtk.base.gnss_sv_status_cb(&r->sv_status_gnss);
            r->sv_count = r->sv_status_gnss.num_svs = 0;
            memset(sv_used_in_fix, 0, 256*sizeof(int));
        }
    } else {
        if (r->sv_status_gps.num_svs != 0 && gps_nmea_end_tag) {
            int idx;
            DBG("r->sv_status_gps.num_svs = %d, gps_nmea_end_tag = %d", r->sv_status_gps.num_svs, gps_nmea_end_tag);
            for (idx = 0; idx < r->sv_status_gps.num_svs; idx++) {
                int prn = r->sv_status_gps.sv_list[idx].prn;
                if (prn < 1 || ((prn > 32) && (prn < 65))|| prn > 96) {
                    ERR("Satellite invalid id: %d", prn);
                } else {
                    DBG("Satellite (%2d) = %2f, %3f, %2f", r->sv_status_gps.sv_list[idx].prn,
                                                       r->sv_status_gps.sv_list[idx].elevation, r->sv_status_gps.sv_list[idx].azimuth,
                                                       r->sv_status_gps.sv_list[idx].snr);
                }
                callback_backup_mtk.base.sv_status_cb(&r->sv_status_gps);
                r->sv_count = r->sv_status_gps.num_svs = 0;
                r->sv_status_gps.used_in_fix_mask = 0;
            }
        }
    }
}


static void
nmea_reader_addc(NmeaReader* const r, int  c)
{
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if ((r->pos >= (int) sizeof(r->in)-1 ) || (r->pos < 0)) {
        r->overflow = 1;
        r->pos      = 0;
        DBG("nmea sentence overflow\n");
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos       += 1;

    if (c == '\n') {
        nmea_reader_parse(r);

        DBG("start nmea_cb\n");
        callback_backup_mtk.base.nmea_cb(r->fix.timestamp, r->in, r->pos);
        r->pos = 0;
    }
}


static void
gps_state_done(GpsState*  s)
{
    char   cmd = CMD_QUIT;

    write(s->control[0], &cmd, 1);
    get_condition(&lock_for_sync[M_CLEANUP]);
    close(s->control[0]); s->control[0] = -1;
    close(s->control[1]); s->control[1] = -1;
    close(s->fd); s->fd = -1;
    close(s->sockfd); s->sockfd = -1;
    close(s->epoll_hd); s->epoll_hd = -1;
#if NEED_IPC_WITH_CODEC
    close(s->sock_codec); s->sock_codec = -1;
#endif
    s->init = 0;
    s->test_time -= 1;
    return;
}


static void
gps_state_start(GpsState*  s)
{
    char  cmd = CMD_START;
    int   ret;

    do { ret = write(s->control[0], &cmd, 1); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}
#ifdef GPS_AT_COMMAND
static void
gps_state_test_start(GpsState*  s)
{
    char cmd = CMD_TEST_START;
    int ret;

    do { ret = write(s->control[0], &cmd, 1); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_TEST_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}
static void
gps_state_test_stop(GpsState*  s)
{
    char cmd = CMD_TEST_STOP;
    int ret;

    do { ret = write(s->control[0], &cmd, 1); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_TEST_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}
#endif

static void
gps_state_stop(GpsState*  s)
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret = write(s->control[0], &cmd, 1); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_restart(GpsState*  s)
{
    char  cmd = CMD_RESTART;
    int   ret;

    do {ret = write(s->control[0], &cmd, 1);}
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_RESTART command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}
#if EPO_SUPPORT
int get_val(char *pStr, char** ppKey, char** ppVal)
{
    int len = (int)strlen(pStr);
    char *end = pStr + len;
    char *key = NULL, *val = NULL;
    int stage = 0;

    DBG("pStr = %s, len=%d!!\n", pStr, len);

    if (!len) {
        return -1;       // no data
    } else if (pStr[0] == '#') {   /*ignore comment*/
        *ppKey = *ppVal = NULL;
        return 0;
    }
    else if (pStr[len-1] != '\n') {
        if (len >= GPS_CONF_FILE_SIZE-1) {
            DBG("buffer is not enough!!\n");
            return -1;
        } else {
            pStr[len] = '\n';
        }
    }
    key = pStr;

    DBG("key = %s!!\n", key);
    while ((*pStr != '=') && (pStr < end)) pStr++;
    if (pStr >= end) {
        DBG("'=' is not found!!\n");
        return -1;       // format error
    }

    *pStr++ = '\0';
    while (IS_SPACE(*pStr) && (pStr < end)) pStr++;       // skip space chars
    val = pStr;
    while (!IS_SPACE(*pStr) && (pStr < end)) pStr++;
    *pStr = '\0';
    *ppKey = key;
    *ppVal = val;

    DBG("val = %s!!\n", val);
    return 0;

}

static int
gps_download_epo_enable(void)
{
    char result[GPS_CONF_FILE_SIZE] = {0};

    FILE *fp = fopen(EPO_CONTROL_FILE_PATH, "r");
    char *key, *val;
    if (!fp) {
           // DBG("%s: open %s fail!\n", __FUNCTION__, EPO_CONTROL_FILE_PATH);
        return 1;
    }

    while (fgets(result, sizeof(result), fp)) {
        if (get_val(result, &key, &val)) {
            DBG("%s: Get data fails!!\n", __FUNCTION__);
            fclose(fp);
            return 1;
        }
        if (!key || !val)
            continue;
        if (!strcmp(key, "EPO_ENABLE")) {
            int len = strlen(val)-1;

            DBG("gps_epo_enablebg = %d, len =%d\n", gps_epo_enable, len);
            gps_epo_enable = str2int(val, val+len);   // *val-'0';
            if ((gps_epo_enable != 1) && (gps_epo_enable != 0)) {
                gps_epo_enable = 1;
            }
            DBG("gps_epo_enableend = %d\n", gps_epo_enable);
        }
        if (!strcmp(key, "DW_DAYS")) {
            int len = strlen(val)-1;
            gps_epo_download_days = str2int(val, val+len);         // *val-'0';
            if (gps_epo_download_days > 30 || gps_epo_download_days < 0) {
                gps_epo_download_days = 30;
            }
        }
        if (!strcmp(key, "GPS_EPO_PERIOD")) {
            int len = strlen(val)-1;
            gps_epo_period = str2int(val, val+len);         // *val-'0';
            if ((gps_epo_period != 30) && (gps_epo_period != 3) && (gps_epo_period != 15)) {
                gps_epo_period = 3;
            }
        }
        if (!strcmp(key, "WIFI_EPO_PERIOD")) {
            int len = strlen(val)-1;
            wifi_epo_period = str2int(val, val+len);         // *val-'0';
            if ((wifi_epo_period != 30) && (wifi_epo_period != 3) && (wifi_epo_period != 15)) {
                wifi_epo_period = 1;
            }
        }
        if (!strcmp(key, "EPO_WIFI_TRIGGER")) {
            int len = strlen(val)-1;
            DBG("gps_epo_wifi_triggerbg = %d, len =%d\n", gps_epo_wifi_trigger, len);
            gps_epo_wifi_trigger = str2int(val, val+len);   // *val-'0';
            if ((gps_epo_wifi_trigger != 1) && (gps_epo_wifi_trigger != 0)) {
                gps_epo_wifi_trigger = 1;
            }
            DBG("gps_epo_wifi_triggerend = %d\n", gps_epo_wifi_trigger);
        }
        DBG("gps_epo_enable = %d, gps_epo_period = %d, \
            wifi_epo_period = %d, gps_epo_wifi_trigger = %d\n", gps_epo_enable, gps_epo_period,
            wifi_epo_period, gps_epo_wifi_trigger);
    }
    fclose(fp);
    return gps_epo_enable;
}

static void
gps_download_epo_file_name(int count)
{
    //  DBG("count is %d\n", count);
    if (gps_epo_type == 1) {
        if (count == 0) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_1.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_1.MD5");
        } else if (count == 1) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_2.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_2.MD5");
        } else if (count == 2) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_3.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_3.MD5");
        } else if (count == 3) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_4.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_4.MD5");
        } else if (count == 4) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_5.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_5.MD5");
        } else if (count == 5) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_6.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_6.MD5");
        } else if (count == 6) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_7.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_7.MD5");
        } else if (count == 7) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_8.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_8.MD5");
        } else if (count == 8) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_9.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_9.MD5"); }
        else if (count == 9) {
            strcpy(gps_epo_file_name, "EPO_GPS_3_10.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GPS_3_10.MD5");
        }
        DBG("download request for file %d, gps_epo_file_name=%s\n", gps_epo_file_count, gps_epo_file_name);
    }
    else if (gps_epo_type == 0) {
        if (count == 0) {
            strcpy(gps_epo_file_name, "EPO_GR_3_1.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_1.MD5");
        } else if (count == 1) {
            strcpy(gps_epo_file_name, "EPO_GR_3_2.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_2.MD5");
        } else if (count == 2) {
            strcpy(gps_epo_file_name, "EPO_GR_3_3.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_3.MD5");
        } else if (count == 3) {
            strcpy(gps_epo_file_name, "EPO_GR_3_4.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_4.MD5");
        } else if (count == 4) {
            strcpy(gps_epo_file_name, "EPO_GR_3_5.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_5.MD5");
        } else if (count == 5) {
            strcpy(gps_epo_file_name, "EPO_GR_3_6.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_6.MD5");
        } else if (count == 6) {
            strcpy(gps_epo_file_name, "EPO_GR_3_7.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_7.MD5");
        } else if (count == 7) {
            strcpy(gps_epo_file_name, "EPO_GR_3_8.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_8.MD5");
        } else if (count == 8) {
            strcpy(gps_epo_file_name, "EPO_GR_3_9.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_9.MD5");
        } else if (count == 9) {
            strcpy(gps_epo_file_name, "EPO_GR_3_10.DAT");
            strcpy(gps_epo_md_file_name, "EPO_GR_3_10.MD5");
        }
        DBG("download request for file %d, gps_epo_file_name=%s, gps_epo_md_file_name=%s\n",
            gps_epo_file_count, gps_epo_file_name, gps_epo_md_file_name);
    }
}

static void
gps_download_epo(GpsState* s)
{
#if 0
    char cmd = CMD_DOWNLOAD;
    int   ret;

    do { ret = write(s->control[0], &cmd, 1); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
#else
    s->epo_data_updated = 1;
    gps_epo_file_count = 0;
#endif
}
#endif
#ifdef GPS_AT_COMMAND
void
at_command_send_ack(char* ack, int len) {
    GpsState*  s = _gps_state;

    remotelen = sizeof(remote);
    if (sendto(s->sockfd, ack, len, 0, (struct sockaddr*)&remote, remotelen) < 0) {
        DBG("** AT Command send ack to USB failed: %s**", strerror(errno));
    } else {
        DBG("** AT Command send ack to USB sucess **");
    }
}

void
at_command_send_cno(char* cmdline) {
    GpsState*  s = _gps_state;
    char* command = cmdline;
    /*
    if (MNL_AT_SIGNAL_MODE && MNL_AT_SIGNAL_TEST_BEGIN) {
        DBG("** AT signal mode, report SNR **");
        char buff[10];
        sprintf(buff, "%d, NA", CNo);
                                DBG("** result = %s**", buff);
                                at_command_send_ack(buff, sizeof(buff));
                                return;
    }*/
    int ret = mtk_gps_test_inprogress();
    if (MNL_AT_TEST_INPROGRESS == ret) {
        DBG("** AT Command test is inprogress **");
        char buff[] = "GNSS Test In Progress";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }


    if (MNL_AT_SIGNAL_MODE == 1) {
        if (0 == cn) {
        DBG("** AT Command, cn is invalid **");
        char buff[] = "0, NA";
        at_command_send_ack(buff, sizeof(buff));
        } else {
        DBG("** AT Command, CN is valid **");
        char buff[10];
           // sprintf(buff, "%d", Avg_CNo/10);    //  unit of AT%GNSS, AT%GNSS=? is 1dB
        sprintf(buff, "%d", cn/10);
        if (!memcmp(command, GNSS_OP, 7)) {
            DBG("** GNSS test, report CN and NA**");
            int size = strlen(buff);
            strcpy(buff+size, ", NA");
            DBG("** result = %s**", buff);
        }
        at_command_send_ack(buff, sizeof(buff));
        }
    } else {
        if (0 == Avg_CNo) {
        DBG("** AT Command, cn is invalid **");
        char buff[] = "0, NA";
        at_command_send_ack(buff, sizeof(buff));
        } else {
        DBG("** AT Command, CN is valid **");
        char buff[10];
        sprintf(buff, "%d", Avg_CNo/10);    //  unit of AT%GNSS, AT%GNSS=? is 1dB
           // sprintf(buff, "%d", cn/10);
                                if (!memcmp(command, GNSS_OP, 7)) {
            DBG("** GNSS test, report CN and NA**");
            int size = strlen(buff);
            strcpy(buff+size, ", NA");
            DBG("** result = %s**", buff);
        }
        at_command_send_ack(buff, sizeof(buff));
        }
    }
}

void
at_command_send_test_result()
{
    int ret = mtk_gps_test_inprogress();
    if (MNL_AT_TEST_INPROGRESS == ret) {
        DBG("** AT Command test is inprogress **");
        char buff[] = "GNSS Test In Progress";
        at_command_send_ack(buff, sizeof(buff));
    } else if (MNL_AT_TEST_DONE == ret || MNL_AT_TEST_RESULT_DONE == ret) {
        DBG("** AT Command test done");
        char buff[GPS_AT_ACK_SIZE];
        sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
        result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
        at_command_send_ack(buff, sizeof(buff));
    } else {
        DBG("** AT Command test status unknown");
        if (result[0] !=0) {   // Brian test
            char buff[GPS_AT_ACK_SIZE]={0};
            DBG("Normal: Return the result");
            sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
            result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            DBG("** result =[%s] **\n", buff);
            at_command_send_ack(buff, sizeof(buff));
        } else if ((result[5] != 0) && (!(MNL_AT_SIGNAL_MODE || MNL_AT_SIGNAL_TEST_BEGIN))) {
            DBG("Normal: Return the result");
            char buff[GPS_AT_ACK_SIZE]={0};
            sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
            result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            DBG("** result =[%s] **\n", buff);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            char buff[] = "[0, 0, 0, 0, 0, 0, 0, 0][0, 0, 0, 0, 0, 0, 0, 0]";
            at_command_send_ack(buff, sizeof(buff));
        }
    }
}

int
at_command_parse_test_num(char* cmdline) {
    unsigned long int res;
    char* command = cmdline;
    char** pos = (char**)malloc(sizeof(char)*strlen(command));

    if (!memcmp(command, GNSS_OP, 7)) {
           // AT%GNSS=n
        res = strtoul(command+8, pos, 10);
    } else {
           // AT%GPS=n
        res = strtoul(command+7, pos, 10);
    }

    if ((res != 0) && ((**pos) =='\0')) {
        DBG("** AT Command Parse: get test_num = %d success!**", res);
        return res;
    } else {
        DBG("** AT Command Parse: the test num may incorrect**");
        return -1;
    }

    if (NULL != pos) {
        DBG("free pos!!");
        free(pos);
    }

}
#endif
#if 0
   // TO-DO
int
send_cmd_to_agpsd(UINT16 type, UINT16 length, char *data)
{
    int sock2supl = -1;
    struct sockaddr_un local;
    mtk_agps_msg *pMsg = NULL;
    UINT16 total_length = length + sizeof(mtk_agps_msg);
    pMsg = (mtk_agps_msg *)malloc(total_length);
    if (pMsg) {
        if (type == MTK_AGPS_SUPL_RAW_DBG) {
            pMsg->type = MTK_AGPS_SUPL_RAW_DBG;
            pMsg->srcMod = MTK_MOD_GPS;
            pMsg->dstMod = MTK_MOD_SUPL;
            pMsg->length = length;


            if (pMsg->length != 0) {
                memcpy(pMsg->data, data, length);
                DBG("SendMsg (%d %d %d %d %s)\r\n", pMsg->srcMod, pMsg->dstMod, pMsg->type, pMsg->length, pMsg->data);
            } else {
                DBG("SendMsg (%d %d %d %d) no data\r\n", pMsg->srcMod, pMsg->dstMod, pMsg->type, pMsg->length);
            }

            if ((sock2supl = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1) {
                ERR("SendMsg:open sock2supl fails\r\n");
                free(pMsg);
                pMsg = NULL;
                return -1;
            }
            memset(&local, 0, sizeof(local));
            local.sun_family = AF_LOCAL;
            strcpy(local.sun_path, MTK_MNL2SUPL);
            if (sendto(sock2supl, (void *)pMsg, total_length, 0, (struct sockaddr*)&local, sizeof(local)) < 0) {
                ERR("send message fail:%s\r\n", strerror(errno));
                return -1;
            }
            close(sock2supl);
        }
    }

    if (pMsg) {
            free(pMsg);
        pMsg = NULL;
    }

    return 0;
}

void
at_command_debug_switch(int state) {
    int status = state;
    int ret;
    char cmd = HAL_CMD_STOP_UNKNOWN;

if (sta.status == GPS_STATUS_ENGINE_ON || sta.status == GPS_STATUS_SESSION_BEGIN
    || MNL_AT_SIGNAL_MODE || MNL_AT_TEST_FLAG) {
    if (1 == status) {
        char buff[] = {MNL_CMD_ENABLE_AGPS_DEBUG};
        ret  == mt3326_daemon_send(buff, sizeof(buff));
    } else if (0 == status) {
        char buff[] = {MNL_CMD_DISABLE_AGPS_DEBUG};
        ret  == mt3326_daemon_send(buff, sizeof(buff));
    }

    if (-1 == ret) {
        DBG("send MNL_CMD_ENABLE_AGPS_DEBUG fail");
        char res[] = "Failed";
        at_command_send_ack(res, sizeof(res));
    } else {
        char res[] = "Success";
        at_command_send_ack(res, sizeof(res));


       // read result here
    /*
    ret = read(mt3326_gps.sock, &cmd, sizeof(cmd));

    if (-1 == ret) {
        DBG("Read fail");
        char res[] = "Failed";
        at_command_send_ack(res, sizeof(res));
    }

    if (cmd == HAL_CMD_SWITCH_AGPS_DEBUG_DONE) {
        DBG("send MNL_CMD_ENABLE_AGPS_DEBUG success");
        char buff[] = "Success";
        at_command_send_ack(buff, sizeof(buff));
    } else if (cmd == HAL_CMD_SWITCH_AGPS_DEBUG_FAIL) {
        DBG("send MNL_CMD_ENABLE_AGPS_DEBUG fail");
        char buff[] = "Fail";
        at_command_send_ack(buff, sizeof(buff));
    }
    */
    }
} else {

           // send to AGPSD directly
        DBG("Send command to AGPSD directly");
        if (status == 1) {
            ret = send_cmd_to_agpsd(MTK_AGPS_SUPL_RAW_DBG, 1, "1");
    } else if (status == 0) {
        ret = send_cmd_to_agpsd(MTK_AGPS_SUPL_RAW_DBG, 1, "0");
        } else {
            DBG("Unknown status: %d", status);
        ret = -1;
        }

        if (-1 == ret) {
            char res[] = "Failed";
            at_command_send_ack(res, sizeof(res));
        } else {
            char res[] = "Success";
            at_command_send_ack(res, sizeof(res));
        }

    }
}
#endif

#ifdef GPS_AT_COMMAND
void
at_gps_command_parser(char* cmdline)
{
    char* command = cmdline;
    test_mode_flag = 0;

    if (!memcmp(command+6, "=", 1)) {
        if ((!memcmp(command+7, "?", 1)) && (!memcmp(command+8, "\0", 1))) {
               // AT%GPS=?
            DBG("** AT Command Parse: AT%%GPS=?**");
            at_command_send_cno(command);
        } else if (((!memcmp(command+7, "E", 1)) ||(!memcmp(command+7, "e", 1))) && (!memcmp(command+8, "\0", 1))) {
               // AT%GPS=E
            DBG("Open AGPS raw data");
            #if 0    // TO-DO
            at_command_debug_switch(0);
            #endif
        } else if (((!memcmp(command+7, "D", 1)) ||(!memcmp(command+7, "d", 1))) && (!memcmp(command+8, "\0", 1))) {
               // AT%GNS=D
        DBG("Close AGPS raw data");
            #if 0    // TO-DO
            at_command_debug_switch(0);
            #endif
        } else {
               // AT%GPS=n
            DBG("** AT Command Parse: AT%%GPS=num**");
            int test_num = at_command_parse_test_num(command);
            if (test_num >= 0) {
                int ret = mtk_gps_test_start(test_num, GPS_TEST_PRN, 2, 0);
                if (0 == ret) {
                    DBG("** AT Command gps test start success **");
                    char buff[] = "GPS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    DBG("** AT Command gps test start fail **");
                    char buff[] = "GPS ERROR";
                    at_command_send_ack(buff, sizeof(buff));
                }
            } else {
                char buff[] = "Invalid Test Num =0";
                at_command_send_ack(buff, sizeof(buff));
            }
        }
    } else if (!memcmp(command+6, "?", 1) && (!memcmp(command+7, "\0", 1))) {
           // AT%GPS?
        DBG("** AT Command Parse: AT%%GPS? **");
        int ret = mtk_gps_test_inprogress();

        if (MNL_AT_TEST_INPROGRESS == ret) {
            DBG("** AT Command test is inprogress **");
            char buff[] = "GPS Test In Progress";
            at_command_send_ack(buff, sizeof(buff));
        } else if (MNL_AT_TEST_DONE == ret) {
            DBG("** AT Command test done");
            char buff[GPS_AT_ACK_SIZE];
            sprintf(buff, "<%d, %d, %d, %d, %d, %d, %d, %d>",
                result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            DBG("** AT Command test status unknown");
            char buff[] = "ERROR";
            at_command_send_ack(buff, sizeof(buff));
        }
    } else if (!memcmp(command+6, "\0", 1)) {
           // AT%GPS
        DBG("** AT Command Parse: AT%%GPS **");
        at_command_send_cno(command);
    } else {
        DBG("** AT Command Parse: illegal command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void
at_gnss_command_parser(char* cmdline)
{
    char* command = cmdline;
    test_mode_flag = 0;

    if (!memcmp(command+7, "=", 1)) {
        if ((!memcmp(command+8, "?", 1)) && (!memcmp(command+9, "\0", 1))) {
               // AT%GNSS=?
            DBG("** AT Command Parse: AT%%GNSS=?**");
               // at_command_send_cno(command);
            at_command_send_test_result();
        } else if (((!memcmp(command+8, "S", 1)) || (!memcmp(command+8, "s", 1))) && (!memcmp(command+9, "\0", 1))) {
            DBG("AT%%GNSS=S is set!!");
            int ret = 0;
            if (0 == MNL_AT_TEST_FLAG) {
                DBG("Open GPS and set signal test mode");
                ret = mtk_gps_test_start(0, 1, 2, 1);
            } else {
                DBG("GPS driver is opened, change mode");
                MNL_AT_SIGNAL_MODE = 1;
                MNL_AT_TEST_FLAG = 0;
                   // Cancel alarm if needed
                if (0 == MNL_AT_CANCEL_ALARM) {
                    DBG("Cancel alarm!!");
                    alarm(0);
                    MNL_AT_CANCEL_ALARM = 0;
                    MNL_AT_SET_ALARM = 0;
                }
            }

            if (0 == ret) {
                DBG("** AT%%GNSS=S set success ! **");
                char buff[] = "GNSS START START OK";
                at_command_send_ack(buff, sizeof(buff));
            } else {
                DBG("** AT%%GNSS=S set fail **");
                char buff[] = "GNSS START START FAIL";
                at_command_send_ack(buff, sizeof(buff));
            }
        } else if (((!memcmp(command+8, "E", 1)) || (!memcmp(command+8, "e", 1))) && (!memcmp(command+9, "\0", 1))) {
            DBG("AT%%GNSS=E is set, stop test!!");
            if (MNL_AT_TEST_STATE == MNL_AT_TEST_UNKNOWN) {
                DBG("** MNL_AT_TEST_UNKNOWN **");
            char buff[] = "GNSS Test is Not In Progress";
            at_command_send_ack(buff, sizeof(buff));
            } else {
                   // if (MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN) {   // To avoid close gps driver many times
                int ret = mtk_gps_test_stop();
                if (0 == ret) {
                    DBG("** AT%%GNSS=E set success ! **");
                    char buff[] = "GNSS TEST END OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    DBG("** AT%GNSS=E set fail **");
                    char buff[] = "GNSS TEST END FAIL";
                    at_command_send_ack(buff, sizeof(buff));
                }
                   // }
            }
        } else {
            // AT%GNSS = n
            DBG("** AT Command Parse: AT%%GNSS=n**");
            int test_num = at_command_parse_test_num(command);
            // initialzation of result whenever starting test again.
            memset(result, 0, sizeof(int)*8);
            result[0] = 1;
            Avg_CNo = 0;
            Success_Num = 0;
            Completed_Num = 0;
            CNo = 0;
            DCNo = 0;
            Dev_CNo = 0;
            cn = 0;
            // initialzation of result whenever starting test again.

            if (MNL_AT_SIGNAL_MODE == 1) {   // && (test_num != 0)) {
                if (test_num <= 0) {
                    char buff[] = "Invalid Test Num = 0";
                    at_command_send_ack(buff, sizeof(buff));
                    if (MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN)
                        mtk_gps_test_stop();
                } else {
                    DBG("MNL_AT_SIGNAL_MODE_BEGIN");
                    MNL_AT_SIGNAL_TEST_BEGIN = 1;
                    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;   //  Brian test
                    char buff[] = "GNSS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                    time(&start_time);
                }
            } else {
                if (test_num >= 0) {
                    int ret = mtk_gps_test_start(test_num, GPS_TEST_PRN, 2, 0);
                    if (0 == ret) {
                        DBG("** AT Command gps test start success **");
                        char buff[] = "GNSS TEST START OK";
                        at_command_send_ack(buff, sizeof(buff));
                    } else {
                        DBG("** AT Command gps test start fail **");
                        char buff[] = "GNSS ERROR";
                        at_command_send_ack(buff, sizeof(buff));
                    }
                } else {
                    char buff[] = "Invalid Test Num =0";
                    at_command_send_ack(buff, sizeof(buff));
                }
            }
        }
    } else if (!memcmp(command+7, "?", 1) && (!memcmp(command+8, "\0", 1))) {
        // AT%GNSS?
        DBG("** AT Command Parse: AT%%GNSS? **");
        at_command_send_test_result();

    } else if (!memcmp(command+7, "\0", 1)) {
        // AT%GNSS
        DBG("** AT Command Parse: AT%%GNSS **");
        at_command_send_cno(command);
    } else {
        DBG("** AT Command Parse: illegal command **");
        char buff[] = "GNSS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}



static void
at_command_parser(char* cmdline)
{
    char* command = cmdline;
    DBG("** AT Command, receive command %s**", command);
    /* begin to parse the command */
    if (!memcmp(command, GPS_OP, 6)) {
        at_gps_command_parser(command);
    } else if (!memcmp(command, GNSS_OP, 7)) {
        at_gnss_command_parser(command);
    } else {
        DBG("** AT Command Parse: Not GPS/GNSS AT Command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}
#endif

void mnld_to_gps_handler(int fd, GpsState* s) {   // from AGPSD->MNLD->HAL->FWK
    int ret;
    int type;
    int offset = 0;
    char buff[9000] = {0};

    ret = safe_read(fd, buff, sizeof(buff));
    type = buff_get_int(buff, &offset);
    DBG("WARNING: mnld_to_gps_handler  ret=%d type=%d\n", ret, type);

    switch (type) {
        case MNL_AGPS_CMD_NOTIFICATION: {
            int usc2_len;
            char ucs2_buff[1024];
            char requestor_id[1024];
            char client_name[1024];

            GpsNiNotification notify;
            memset(&notify, 0, sizeof(notify));

            notify.notification_id = buff_get_int(buff, &offset);    // session_id
            notify.ni_type = GPS_NI_TYPE_UMTS_SUPL;
            int type = buff_get_int(buff, &offset);    // mnl_agps_notify_type
            switch (type) {
            case AGPS_SUPL_NOTIFY_ONLY:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY;
                break;
            case AGPS_SUPL_NOTIFY_ALLOW_NO_ANSWER:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                break;
            case AGPS_SUPL_NOTIFY_DENY_NO_ANSWER:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                break;
            case AGPS_SUPL_NOTIFY_PRIVACY:
                notify.notify_flags     = GPS_NI_PRIVACY_OVERRIDE;
                break;
            default:
                notify.notify_flags     = 0;
                break;
            }
            notify.default_response = GPS_NI_RESPONSE_NORESP;
            notify.timeout = 8;
            notify.requestor_id_encoding = GPS_ENC_SUPL_UCS2;
            notify.text_encoding = GPS_ENC_SUPL_UCS2;

            buff_get_string(requestor_id, buff, &offset);
            buff_get_string(client_name, buff, &offset);

            memset(ucs2_buff, 0, sizeof(ucs2_buff));
            usc2_len = asc_str_to_usc2_str(ucs2_buff, requestor_id);
            raw_data_to_hex_string(notify.requestor_id, ucs2_buff, usc2_len);

            memset(ucs2_buff, 0, sizeof(ucs2_buff));
            usc2_len = asc_str_to_usc2_str(ucs2_buff, client_name);
            raw_data_to_hex_string(notify.text, ucs2_buff, usc2_len);

            notify.size = sizeof(GpsNiNotification);
            g_agps_ctx.gps_ni_callbacks->notify_cb(&notify);
            break;
        }
        case MNL_AGPS_CMD_NOTIFICATION_2: {
            GpsNiNotification notify;
            memset(&notify, 0, sizeof(notify));

            notify.notification_id = buff_get_int(buff, &offset);    // session_id
            notify.ni_type = GPS_NI_TYPE_UMTS_SUPL;

            int type = buff_get_int(buff, &offset);    // mnl_agps_notify_type
            switch (type) {
            case AGPS_SUPL_NOTIFY_ONLY:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY;
                break;
            case AGPS_SUPL_NOTIFY_ALLOW_NO_ANSWER:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                break;
            case AGPS_SUPL_NOTIFY_DENY_NO_ANSWER:
                notify.notify_flags     = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
                break;
            case AGPS_SUPL_NOTIFY_PRIVACY:
                notify.notify_flags     = GPS_NI_PRIVACY_OVERRIDE;
                break;
            default:
                notify.notify_flags     = 0;
                break;
            }

            buff_get_string(notify.requestor_id, buff, &offset);
            buff_get_string(notify.text, buff, &offset);

            notify.default_response = GPS_NI_RESPONSE_NORESP;
            notify.timeout = 8;
            notify.requestor_id_encoding = buff_get_int(buff, &offset);
            notify.text_encoding = buff_get_int(buff, &offset);
            notify.size = sizeof(GpsNiNotification);
            g_agps_ctx.gps_ni_callbacks->notify_cb(&notify);
            break;
        }
        case MNL_AGPS_CMD_DATA_CONN_REQ: {
            AGpsStatus_v2 agps_status;
            agps_status.type = AGPS_TYPE_SUPL;
            agps_status.status = GPS_REQUEST_AGPS_DATA_CONN;
            agps_status.ipaddr = buff_get_int(buff, &offset);
            // no need currently
            int is_emergency = buff_get_int(buff, &offset);
            agps_status.size = sizeof(AGpsStatus_v2);
            g_agps_ctx.agps_callbacks->status_cb(&agps_status);
            break;
        }
        case MNL_AGPS_CMD_DATA_CONN_REQ2: {
            AGpsStatus agps_status;
            memset(&agps_status, 0, sizeof(AGpsStatus));
            agps_status.size = sizeof(AGpsStatus);
            agps_status.type = AGPS_TYPE_SUPL;
            agps_status.status = GPS_REQUEST_AGPS_DATA_CONN;
            buff_get_struct(&agps_status.addr, sizeof(struct sockaddr_storage), buff, &offset);
            // no need currently
            int is_emergency = buff_get_int(buff, &offset);
            g_agps_ctx.agps_callbacks->status_cb(&agps_status);
            break;
        }
        case MNL_AGPS_CMD_DATA_CONN_RELEASE: {
            AGpsStatus_v1 agps_status;
            agps_status.type = AGPS_TYPE_SUPL;
            agps_status.status = GPS_RELEASE_AGPS_DATA_CONN;
            // agps_status.ipaddr = buff_get_int(buff, &offset);
            agps_status.size = sizeof(AGpsStatus_v1);
            DBG("AGPS data connection release with AGpsStatus_v1");
            g_agps_ctx.agps_callbacks->status_cb(&agps_status);
            break;
        }
        case MNL_AGPS_CMD_REQUEST_SET_ID: {
            int flags = buff_get_int(buff, &offset);
            g_agps_ctx.agps_ril_callbacks->request_setid(flags);
            break;
        }
        case MNL_AGPS_CMD_REQUEST_REFLOC: {
            int flags = buff_get_int(buff, &offset);
            g_agps_ctx.agps_ril_callbacks->request_refloc(flags);
            break;
        }
        case MNL_AGPS_CMD_AGPS_LOC: {   // 235
            mnl_agps_agps_location location;
            GpsLocation mlocation;
            memset(&location, 0, sizeof(mnl_agps_agps_location));
            memset(&mlocation, 0, sizeof(GpsLocation));

            buff_get_struct((char *)&location, sizeof(mnl_agps_agps_location), buff, &offset);

            DBG("latitude = %lf, longitude= %lf, altitude_used = %d, altitude = %lf, accuracy = %d", location.latitude,
                location.longitude, location.altitude_used, location.altitude, location.accuracy_used);

            mlocation.flags    |= GPS_LOCATION_HAS_LAT_LONG;
            mlocation.latitude  = location.latitude;
            mlocation.longitude = location.longitude;

            if (location.altitude_used) {
                mlocation.flags |= GPS_LOCATION_HAS_ALTITUDE;
                mlocation.altitude = location.altitude;
            }

            if (location.speed_used) {
                mlocation.flags |= GPS_LOCATION_HAS_SPEED;
                mlocation.speed = location.speed;
            }

            if (location.bearing_used) {
                mlocation.flags |= GPS_LOCATION_HAS_BEARING;
                mlocation.bearing = location.bearing;
            }

            if (location.accuracy_used) {
                mlocation.flags |= GPS_LOCATION_HAS_ACCURACY;
                mlocation.accuracy = location.accuracy;
            }

            if (location.timestamp_used)
                mlocation.timestamp = location.timestamp;

            mlocation.size = sizeof(GpsLocation);

            if (callback_backup_mtk.base.location_cb) {
                DBG("Update location data to UI");
                callback_backup_mtk.base.location_cb(&mlocation);
            } else {
                DBG("Location CB is null");
            }
            break;
        }
#if EPO_SUPPORT
        case HAL_CMD_UPDATE_EPO_FILE_DONE: {
            ERR("Update EPO file ok\n");
            if ((epo_download_failed == 0) && (gps_epo_file_count == 0)) {
                unlink(EPO_FILE_HAL);
            }
               // epo_download_failed = 0;
            break;
        }
        case HAL_CMD_UPDATE_EPO_FILE_FAIL: {
            ERR("Update EPO file fail\n");
            break;
        }
#endif
        case MNL_CMD_GPS_INJECT_TIME_REQ:{
            if (callback_backup_mtk.base.request_utc_time_cb) {
                DBG("request_utc_time_cb\n");
                callback_backup_mtk.base.request_utc_time_cb();
            }
            break;
        }
        case MNL_CMD_GPS_INJECT_LOCATION_REQ:{
            DBG("mnl request location\n");
            mtk_gps_send_location_to_mnl();
            break;
        }
        case HAL_CMD_MNL_DIE:{
            DBG("MNLD died!!");
            flag_unlock = 0;
            // GpsStatus sta;
            // Modify to global
            sta.status = GPS_STATUS_SESSION_END;
            callback_backup_mtk.base.status_cb(&sta);
            DBG("sta.status = GPS_STATUS_SESSION_END\n");
            sta.status = GPS_STATUS_ENGINE_OFF;
            DBG("sta.status = GPS_STATUS_ENGINE_OFF\n");
            callback_backup_mtk.base.status_cb(&sta);
            release_condition(&lock_for_sync[M_STOP]);
            callback_backup_mtk.base.release_wakelock_cb();
            s->start_flag = 0;
            DBG("s->start_flag = 0\n");
            DBG("HAL_CMD_MNL_DIE has been receiving from MNLD, release lock let stop done\n");
            break;
        }
        case HAL_CMD_GPS_ICON:{
            int status = buff_get_int(buff, &offset);
            DBG("ICON STATUS = %d", status);
            if (status == 1) {   // start
                if (started == 0) {
                    sta.status = GPS_STATUS_ENGINE_ON;
                    DBG("sta.status = GPS_STATUS_ENGINE_ON\n");
                    callback_backup_mtk.base.status_cb(&sta);
                    sta.status = GPS_STATUS_SESSION_BEGIN;
                    DBG("sta.status = GPS_STATUS_SESSION_BEGIN\n");
                    callback_backup_mtk.base.status_cb(&sta);
                }
            } else if (status == 0) {   // stop
                if (started == 0) {
                    sta.status = GPS_STATUS_ENGINE_OFF;
                    DBG("sta.status = GPS_STATUS_ENGINE_OFF\n");
                    callback_backup_mtk.base.status_cb(&sta);
                    sta.status = GPS_STATUS_SESSION_END;
                    DBG("sta.status = GPS_STATUS_SESSION_END\n");
                    callback_backup_mtk.base.status_cb(&sta);
                }
            }
            break;
        }
        case HAL_CMD_MEASUREMENT: {
            DBG("HAL_CMD_MEASUREMENT message recieved\n");
            if (gpsmeasurement_init_flag == 1) {
            int i = 0;
            MTK_GPS_MEASUREMENT mtk_gps_measurement[32];
            buff_get_struct(mtk_gps_measurement, 32*sizeof(MTK_GPS_MEASUREMENT), buff, &offset);
            MTK_GPS_CLOCK mtk_gps_clock;
            buff_get_struct(&mtk_gps_clock, sizeof(MTK_GPS_CLOCK), buff, &offset);
            GpsClock gpsclock;
            memset(&gpsclock, 0, sizeof(GpsClock));
            gpsclock.size = sizeof(GpsClock);
            gpsclock.bias_ns = mtk_gps_clock.BiasInNs;
            gpsclock.bias_uncertainty_ns = mtk_gps_clock.BiasUncertaintyInNs;
            gpsclock.drift_nsps = mtk_gps_clock.DriftInNsPerSec;
            gpsclock.flags = mtk_gps_clock.flag;
            gpsclock.leap_second = mtk_gps_clock.leapsecond;
            gpsclock.time_ns = mtk_gps_clock.TimeInNs;
            gpsclock.time_uncertainty_ns = mtk_gps_clock.TimeUncertaintyInNs;
            gpsclock.type = mtk_gps_clock.type;
            gpsclock.full_bias_ns = mtk_gps_clock.FullBiasInNs;
            gpsclock.drift_uncertainty_nsps = mtk_gps_clock.DriftUncertaintyInNsPerSec;

            GpsData gpsdata;
            memset(&gpsdata, 0, sizeof(GpsData));
            gpsdata.size = sizeof(GpsData);

            for (i = 0; i < 1; i++) {
                DBG("gpsdata measurements[%d] memcpy completed", i);
                gpsdata.measurements[i].size = sizeof(GpsMeasurement);
                gpsdata.measurements[i].accumulated_delta_range_m = mtk_gps_measurement[i].AcDRInMeters;
                gpsdata.measurements[i].accumulated_delta_range_state = mtk_gps_measurement[i].AcDRState10;
                gpsdata.measurements[i].accumulated_delta_range_uncertainty_m = mtk_gps_measurement[i].AcDRUnInMeters;
                gpsdata.measurements[i].azimuth_deg = mtk_gps_measurement[i].AzInDeg;
                gpsdata.measurements[i].azimuth_uncertainty_deg = mtk_gps_measurement[i].AzUnInDeg;
                gpsdata.measurements[i].bit_number = mtk_gps_measurement[i].BitNumber;
                gpsdata.measurements[i].carrier_cycles = mtk_gps_measurement[i].CarrierCycle;
                gpsdata.measurements[i].carrier_phase = mtk_gps_measurement[i].CarrierPhase;
                gpsdata.measurements[i].carrier_phase_uncertainty = mtk_gps_measurement[i].CarrierPhaseUn;
                gpsdata.measurements[i].carrier_frequency_hz = mtk_gps_measurement[i].CFInhZ;
                gpsdata.measurements[i].c_n0_dbhz = mtk_gps_measurement[i].Cn0InDbHz;
                gpsdata.measurements[i].code_phase_chips = mtk_gps_measurement[i].CPInChips;
                gpsdata.measurements[i].code_phase_uncertainty_chips = mtk_gps_measurement[i].CPUnInChips;
                gpsdata.measurements[i].doppler_shift_hz = mtk_gps_measurement[i].DopperShiftInHz;
                gpsdata.measurements[i].doppler_shift_uncertainty_hz = mtk_gps_measurement[i].DopperShiftUnInHz;
                gpsdata.measurements[i].elevation_deg = mtk_gps_measurement[i].ElInDeg;
                gpsdata.measurements[i].elevation_uncertainty_deg = mtk_gps_measurement[i].ElUnInDeg;
                gpsdata.measurements[i].flags = mtk_gps_measurement[i].flag;
                gpsdata.measurements[i].loss_of_lock = mtk_gps_measurement[i].LossOfLock;
                gpsdata.measurements[i].multipath_indicator = mtk_gps_measurement[i].MultipathIndicater;
                gpsdata.measurements[i].pseudorange_m = mtk_gps_measurement[i].PRInMeters;
                gpsdata.measurements[i].prn = mtk_gps_measurement[i].PRN;
                gpsdata.measurements[i].pseudorange_rate_mps = mtk_gps_measurement[i].PRRateInMeterPreSec;
                gpsdata.measurements[i].pseudorange_rate_uncertainty_mps = mtk_gps_measurement[i].PRRateUnInMeterPreSec;
                gpsdata.measurements[i].pseudorange_uncertainty_m = mtk_gps_measurement[i].PRUnInMeters;
                gpsdata.measurements[i].received_gps_tow_ns = mtk_gps_measurement[i].ReGpsTowInNs;
                gpsdata.measurements[i].received_gps_tow_uncertainty_ns = mtk_gps_measurement[i].ReGpsTowUnInNs;
                gpsdata.measurements[i].snr_db = mtk_gps_measurement[i].SnrInDb;
                gpsdata.measurements[i].state = mtk_gps_measurement[i].state;
                gpsdata.measurements[i].time_from_last_bit_ms = mtk_gps_measurement[i].TimeFromLastBitInMs;
                gpsdata.measurements[i].time_offset_ns = mtk_gps_measurement[i].TimeOffsetInNs;
                gpsdata.measurements[i].used_in_fix = mtk_gps_measurement[i].UsedInFix;
                if (gpsdata.measurements[i].prn != 0) {
                    gpsdata.measurement_count++;
                }
            }
            memcpy(&gpsdata.clock , &gpsclock, sizeof(gpsclock));

        #if 0
                DBG("clock.size = %d, clock.bias_ns = %lf, clock.bias_uncertainty_ns = %lf,
                    clock.drift_nsps = %lf, clock.flags = %d, clock.leap_second = %d, \
                    clock.time_ns = %d, clock.time_uncertainty_ns = %lf, clock.type = %d, \
                    clock.full_bias_ns = %d, clock.drift_uncertainty_nsps = %lf",
                    gpsdata.clock.size, gpsdata.clock.bias_ns, gpsdata.clock.bias_uncertainty_ns,
                    gpsdata.clock.drift_nsps, gpsdata.clock.flags, gpsdata.clock.leap_second,
                    gpsdata.clock.time_ns, gpsdata.clock.time_uncertainty_ns, gpsdata.clock.type,
                    gpsdata.clock.full_bias_ns, gpsdata.clock.drift_uncertainty_nsps);

                for (i = 0; i < 32; i++) {
                    if (gpsdata.measurements[i].size > 0) {
                        DBG("i = %d, measurements.CFInhZ = %f, measurements.Cn0InDbHz = %lf, \
                            measurements.size = %d, measurements.prn = %d, \
                            measurements.time_offset_ns = %lf,  measurements.used_in_fix = %d",
                            i, gpsdata.measurements[i].carrier_frequency_hz,
                            gpsdata.measurements[i].c_n0_dbhz, gpsdata.measurements[i].size,
                            gpsdata.measurements[i].prn, gpsdata.measurements[i].time_offset_ns,
                            gpsdata.measurements[i].used_in_fix);
                    }
                }
        #endif

                DBG("if measurement_count = %d > 0 , Send measurement_callback data to FWK", gpsdata.measurement_count);
                if (gpsdata.measurement_count > 0) {
                    measurement_callbacks.measurement_callback(&gpsdata);
                    DBG("Send measurement_callback data to FWK success\n");
                }
            }
            break;
        }
        case HAL_CMD_NAVIGATION: {
            DBG("HAL_CMD_NAVIGATION message recieved1\n");
            if (gpsnavigation_init_flag == 1) {
                int i;
                MTK_GPS_NAVIGATION_EVENT *gps_navigation_event =
                (MTK_GPS_NAVIGATION_EVENT*)malloc(sizeof(MTK_GPS_NAVIGATION_EVENT));
                if (gps_navigation_event == NULL) {
                    DBG("point gps_navigation_event is null, return!");
                    break;
                }
                buff_get_struct(gps_navigation_event, sizeof(MTK_GPS_NAVIGATION_EVENT), buff, &offset);

                GpsNavigationMessage gpsnavigation;
                memset(&gpsnavigation, 0, sizeof(GpsNavigationMessage));
                gpsnavigation.size = sizeof(GpsNavigationMessage);
                gpsnavigation.prn = gps_navigation_event->prn;
                gpsnavigation.type = gps_navigation_event->type;
                gpsnavigation.message_id = gps_navigation_event->messageID;
                gpsnavigation.submessage_id = gps_navigation_event->submessageID;
                gpsnavigation.data_length = (size_t)gps_navigation_event->length;
                gpsnavigation.data = (char*)malloc(sizeof(char)*gps_navigation_event->length);
                memcpy(gpsnavigation.data, gps_navigation_event->data, gps_navigation_event->length);

    #if 0
                DBG("size_t = %d, gpsnavigation.size = %d, %p, gpsnavigation.prn = %d, %p, \
                    gpsnavigation.type = %d, %p, gpsnavigation.message_id = %d, %p, \
                    gpsnavigation.submessage_id = %d, %p, gpsnavigation.data_length = %d, %p, data = %p",
                    sizeof(size_t), gpsnavigation.size, &gpsnavigation.size, gpsnavigation.prn,
                    &gpsnavigation.prn, gpsnavigation.type, &gpsnavigation.type,
                    gpsnavigation.message_id, &gpsnavigation.message_id, gpsnavigation.submessage_id,
                    &gpsnavigation.submessage_id, gpsnavigation.data_length,
                    &gpsnavigation.data_length, gpsnavigation.data);
                for (i = 0; i < 40; i++) {
                    DBG("HAL: gpsnavigation.data[%d] = %x", i, gpsnavigation.data[i]);
                }
    #endif
                navigation_callbacks.navigation_message_callback(&gpsnavigation);
                DBG("navigation_message_callback done\n");
                free(gpsnavigation.data);
                free(gps_navigation_event);
            }
            break;
        }

        default: {
            ERR("ERR: unknown type=%d\n", type);
            break;
        }
    }
}

static int
epoll_register(int  epoll_fd, int  fd)
{
    struct epoll_event  ev;
    int                 ret, flags;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0)
        ERR("epoll ctl error, error num is %d\n, message is %s\n", errno, strerror(errno));
    return ret;
}


static int
epoll_deregister(int  epoll_fd, int  fd)
{
    int  ret;
    do {
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

/*for reducing the function call to get data from kernel*/
static char buff[2048];
/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
void
gps_state_thread(void*  arg)
{
    static float count = 0;
    GpsState*   state = (GpsState*) arg;
    state->test_time += 1;
   //   state->thread_exit_flag=0;
    NmeaReader  reader[1];
#if NEED_IPC_WITH_CODEC
    char buf_for_codec[2048];
#endif
#ifdef GPS_AT_COMMAND
       // int         started    = 0;
#endif
    int         gps_fd     = state->fd;
    int         control_fd = state->control[1];
    int         atc_fd = state->sockfd;

    int epoll_fd = state->epoll_hd;
    int         test_started = 0;

    nmea_reader_init(reader);

    int mnld_fd = bind_udp_socket(MTK_MNLD2HAL);
    DBG("WARNING: mnld_fd=%d\n", mnld_fd);
    if (mnld_fd >= 0) {
        if (epoll_register(epoll_fd, mnld_fd) < 0)
            ERR("ERR: epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
        else
            DBG("WARNING: epoll_register successfully\n");
    }

#if NEED_IPC_WITH_CODEC
    int sock_codec, size_codec;
    struct sockaddr_un un;
    socklen_t client_addr_len;
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, EVDO_CODEC_SOC);
    if ((state->sock_codec = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        ERR("create socket for communicate with codec error, message %s\n", strerror(errno));
    if (mnld_fd != C_INVALID_FD)
        close(mnld_fd);
        return;
    }
    unlink(EVDO_CODEC_SOC);
    size_codec = sizeof(un.sun_family)+strlen(un.sun_path);
    if (bind(state->sock_codec, (struct sockaddr *)&un, size_codec) < 0) {
        ERR("bind fail, message = %s\n", strerror(errno));
    if (mnld_fd != C_INVALID_FD)
        close(mnld_fd);
        return;
    }
    if (listen(state->sock_codec, 5) == -1) {
        ERR("listern error, message is %s\n", strerror(errno));
    if (mnld_fd != C_INVALID_FD)
        close(mnld_fd);
        return;
    }
    DBG("listen done\n");
    int a = chmod(EVDO_CODEC_SOC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
    DBG("chmod res = %d\n", a);    // 770<--mode

    if (chown(EVDO_CODEC_SOC, -1, AID_INET)) {
        ERR("chown error: %s", strerror(errno));
    }

    if (epoll_register(epoll_fd, state->sock_codec) < 0)
        ERR("epoll register state->sock_codec error, error num is %d\n, message is %s\n",
        errno, strerror(errno));
#endif
       //  register control file descriptors for polling
    if (epoll_register(epoll_fd, control_fd) < 0)
        ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    if (epoll_register(epoll_fd, gps_fd) < 0)
        ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    if (epoll_register(epoll_fd, atc_fd) < 0)
        ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));

    DBG("gps thread running: PPID[%d], PID[%d]\n", getppid(), getpid());
    release_condition(&lock_for_sync[M_INIT]);
    DBG("HAL thread is ready, realease lock, and CMD_START can be handled\n");
#if SEM
    sem_t *sem;
    sem = sem_open("/data/misc/gps/read_dev_gps", O_CREAT, S_IRWXU|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, 1);
    if (sem == SEM_FAILED) {
        ERR("init semaphore FAIL, error message is %s \n", strerror(errno));
        return;
    } else
        DBG("create semaphore ok\n");
#endif
       //  now loop
    for (;;) {
#if NEED_IPC_WITH_CODEC
        struct epoll_event   events[3];
#else
        struct epoll_event   events[4];
#endif
        int                  ne, nevents;
#if NEED_IPC_WITH_CODEC
        nevents = epoll_wait(epoll_fd, events, 3, -1);
#else
        nevents = epoll_wait(epoll_fd, events, 4, -1);
#endif
        if (nevents < 0) {
            if (errno != EINTR)
                ERR("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }
        VER("gps thread received %d events", nevents);
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                ERR("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto Exit;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;

                if (fd == control_fd) {
                    char  cmd = 255;
                    int   ret;
                    DBG("gps control fd event");
                    do {
                        ret = read(fd, &cmd, 1);
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        DBG("gps thread quitting on demand");
                        goto Exit;
                    }
                    else if (cmd == CMD_START) {
                        if (!started) {
                            DBG("gps thread starting  location_cb=%p", &callback_backup_mtk.base);
                            started = 1;
                            nmea_reader_set_callback(reader, &state->callbacks);
                        }
                    }
                    #ifdef GPS_AT_COMMAND
                    else if (cmd == CMD_TEST_START) {
                        if ((!test_started) && (1 == test_mode_flag)) {
                              //  DBG("**AT Command test_start: test_cb=%p", state->callbacks.test_cb);
                            test_started = 1;
                            nmea_reader_set_callback(reader, &state->callbacks);
                        }
                    }
                    else if (cmd == CMD_TEST_STOP) {
                        if (test_started) {
                            DBG("**AT Command test_stop");
                            test_started = 0;
                            nmea_reader_set_callback(reader, NULL);
                        }
                    }
                    else if (cmd == CMD_TEST_SMS_NO_RESULT) {
                        DBG("**CMD_TEST_SMS_NO_RESULT, update result!!");
                        sms_airtest_no_signal_report(Err_Code, 0, 0, 0, 0);
                           // nmea_reader_update_at_test_result(reader, Err_Code, 0, 0, 0, 0);
                    }
                    #endif
#if EPO_SUPPORT
                    else if (cmd == CMD_DOWNLOAD) {
                        DBG("Send download request in HAL.");
                        mGpsXtraCallbacks.download_request_cb();
                    }
#endif
                    else if (cmd == CMD_STOP) {
                        if (started) {
                            DBG("gps thread stopping");
                            started = 0;
                            nmea_reader_set_callback(reader, NULL);
                            DBG("CMD_STOP has been receiving from HAL thread, release lock so can handle CLEAN_UP\n");
                        }
                    }
                    else if (cmd == CMD_RESTART) {
                        reader->fix_mode = 0;
                    }
                }
                else if (fd == gps_fd) {
                    if (!flag_unlock) {
                        release_condition(&lock_for_sync[M_START]);
                        flag_unlock = 1;
                        DBG("got first NMEA sentence, release lock to set state ENGINE ON, SESSION BEGIN");
                    }
                    VER("gps fd event");
                    if (report_time_interval > ++count) {
                        DBG("[trace]count is %f\n", count);
                        int ret = read(fd, buff, sizeof(buff));
                        continue;
                    }
                    count = 0;
                    for (;;) {
                        int  nn, ret;
#if SEM
                        if (sem_wait(sem) != 0) {
                            ERR("sem wait error, message is %s \n", strerror(errno));
                            close(fd);
                            return;
                        } else
                            DBG("get semaphore, can read now\n");
#endif
                        ret = read(fd, buff, sizeof(buff));
#if NEED_IPC_WITH_CODEC
                        memset(buf_for_codec, 0, sizeof(buf_for_codec));
                        memcpy(buf_for_codec, buff, sizeof(buff));
#endif
#if SEM
                        if (sem_post(sem) != 0) {
                            ERR("sem post error, message is %s\n", strerror(errno));
                            close(fd);
                            return;
                        } else
                            DBG("post semaphore, read done\n");
#endif
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ERR("error while reading from gps daemon socket: %s: %p", strerror(errno), buff);
                            break;
                        }
                        DBG("received %d bytes:\n", ret);
                        gps_nmea_end_tag = 0;
                        for (nn = 0; nn < ret; nn++)
                        {
                            if (nn == (ret-1))
                                gps_nmea_end_tag = 1;

                            nmea_reader_addc(reader, buff[nn]);
                        }
                    }
                    VER("gps fd event end");
                }
                #ifdef GPS_AT_COMMAND
                else if (fd == atc_fd) {
                    char cmd[20];
                    DBG("** AT Command received **");
                    /* receive and parse ATCM here */
                    for (;;) {
                        int  i, ret;

                        remotelen = sizeof(remote);
                        ret = recvfrom(fd, cmd, sizeof(cmd), 0, (struct sockaddr *)&remote, &remotelen);
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ERR("error while reading AT Command socket: %s: %p", strerror(errno), cmd);
                            break;
                        }
                        DBG("received %d bytes: %.*s", ret, ret, cmd);
                        cmd[ret] = 0x00;
                        at_command_parser(cmd);                // need redefine
                    }
                    DBG("** AT Command event done **");

                }
                #endif
                else if (fd == mnld_fd) {
                    mnld_to_gps_handler(mnld_fd, state);
                }
#if NEED_IPC_WITH_CODEC
                else if (fd == state->sock_codec) {
                    client_addr_len = sizeof(un);
                    int accept_ret = accept(state->sock_codec, (struct sockaddr*)&un, &client_addr_len);
                    if (accept_ret == -1) {
                        ERR("accept error, message is %s\n", strerror(errno));
                        continue;
                    }
                    DBG("accept done\n");
                    int cmd = 0, write_len;
                    GpsLocation tLocation;

                    if (recv(accept_ret, &cmd, sizeof(cmd), 0) < 0) {
                        ERR("read from codec error, message = %s\n", strerror(errno));
                        close(accept_ret);
                        continue;
                    }
                    DBG("read done, cmd: %d\n", cmd);
                    switch (cmd) {
                    case 1:   // need NMEA sentences
                        memset(buf_for_codec, 0, sizeof(buf_for_codec));
                        write_len = send(accept_ret, buf_for_codec, sizeof(buff), 0);
                        DBG("write %d bytes to codec\n", write_len);
                        break;
                    case 2:    // For AGPS location froward
                        DBG("Snd to UI");
                        char ack_buf[10] = {0};
                        strcpy(ack_buf, "cmd2_ack");
                        write_len = send(accept_ret, ack_buf, sizeof(ack_buf), 0);
                        DBG("wait rcv location data");
                        if (recv(accept_ret, &tLocation, sizeof(tLocation), 0) < 0) {
                            ERR("read from codec error, message = %s\n", strerror(errno));
                        } else {
                            if (callback_backup_mtk.base.location_cb) {
                                DBG("Update location data to UI");
                                callback_backup_mtk.base.location_cb(&tLocation);
                            } else {
                                DBG("Location CB is null");
                            }
                        }
                        break;
                    default:
                        ERR("unknonwn codec message, codec send %d to me\n", cmd);
                        break;
                    }
                    close(accept_ret);
                }
#endif
                else
                {
                    ERR("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }
    }
Exit:
    DBG("HAL thread is exiting, release lock to clean resources\n");
    if (mnld_fd != C_INVALID_FD)
        close(mnld_fd);
    release_condition(&lock_for_sync[M_CLEANUP]);
    return;
}


static void
gps_state_init(GpsState*  state)
{
    state->control[0] = -1;
    state->control[1] = -1;
    state->fd         = -1;

    state->fd = open(GPS_CHANNEL_NAME, O_RDONLY);    // support poll behavior
    int epoll_fd   = epoll_create(2);
    state->epoll_hd = epoll_fd;

    if (state->fd < 0) {
        ERR("no gps hardware detected: %s:%d, %s", GPS_CHANNEL_NAME, state->fd, strerror(errno));
        return;
    }

    if (mtk_init() != 0) {
        ERR("could not initiaize mtk !!");
        goto Fail;
    }

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, state->control) < 0) {
        ERR("could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }
    #ifdef GPS_AT_COMMAND
    /* Create socket with generic service for AT Command */
    if ((state->sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        ERR("gps_state_init: hal2usb socket create failed\n");
        goto Fail;
    }

    unlink(GPS_AT_COMMAND_SOCK);
    memset(&cmd_local, 0, sizeof(cmd_local));
    cmd_local.sun_family = AF_LOCAL;
    strcpy(cmd_local.sun_path, GPS_AT_COMMAND_SOCK);

    if (bind(state->sockfd, (struct sockaddr *)&cmd_local, sizeof(cmd_local)) < 0)
    {
        ERR("gps_state_init: hal2usb socket bind failed\n");
        state->sockfd = -1;
        goto Fail;
    }

    int res = chmod(GPS_AT_COMMAND_SOCK, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP);
    DBG("chmod res = %d, %s", res, strerror(errno));
    #endif
    state->thread = callback_backup_mtk.base.create_thread_cb(gps_native_thread, gps_state_thread, state);
    if (!state->thread) {
        ERR("could not create gps thread: %s", strerror(errno));
        goto Fail;
    }

    DBG("gps state initialized, the thread is %d\n", (int)state->thread);
    return;

Fail:
    gps_state_done(state);
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/
#if EPO_SUPPORT
static int
read_prop(char* name)
{
    char result[GPS_CONF_FILE_SIZE] = {0};

    FILE *fp = fopen(name, "r");
    char *key, *val;
    if (!fp) {
        DBG("%s: open %s fail!\n", __FUNCTION__, EPO_CONTROL_FILE_PATH);
        return -1;
    }

    while (fgets(result, sizeof(result), fp)) {
        if (get_val(result, &key, &val)) {
            DBG("%s: Get data fails!!\n", __FUNCTION__);
            fclose(fp);
            return -1;
        }
        if (!key || !val)
            continue;
        if (!strcmp(key, "GNSS_MODE")) {
            gnss_mode = atoi(val);
            DBG("gnss_mode =%d\n", gnss_mode);
        }
    }
    fclose(fp);
    return 0;

}
#endif

static int
copy_GpsCallbacks_mtk(GpsCallbacks_mtk* dst, GpsCallbacks_mtk* src)
{
    if (src->base.size == sizeof(GpsCallbacks_mtk)) {
        *dst = *src;
        DBG("Use GpsCallbacks_mtk\n");
        return 0;
    }
// xen0n: this is unnecessary as the callback is already in GpsCallbacks
#if 0
    if (src->base.size == sizeof(GpsCallbacks)) {
        dst->base = src->base;
        dst->gnss_sv_status_cb = NULL;
        DBG("Use GpsCallbacks\n");
        return 0;
    }
#endif
    ERR("Bad callback, size: %d, expected: %d or %d", src->base.size, sizeof(GpsCallbacks_mtk), sizeof(GpsCallbacks));
    return -1;    //  error
}

static int
mtk_gps_init(GpsCallbacks* callbacks)
{
    GpsState*  s = _gps_state;
    int get_time = 20;
    int res = 0;
#if EPO_SUPPORT
    int cnt = sizeof(mnl_prop_path)/sizeof(mnl_prop_path[0]);
    int idx;
#endif
    if (s->init)
        return 0;

    if (copy_GpsCallbacks_mtk(&callback_backup_mtk, (GpsCallbacks_mtk*)callbacks) != 0) {
        return -1;    //  error
    }
    s->callbacks = callback_backup_mtk.base;

    gps_state_init(s);
    get_condition(&lock_for_sync[M_INIT]);
    usleep(1000*1);
    s->init = 1;
    DBG("Set GPS_CAPABILITY_SCHEDULING \n");
    callback_backup_mtk.base.set_capabilities_cb(GPS_CAPABILITY_SCHEDULING);
#if EPO_SUPPORT
    // get chipid here
    while ((get_time--!= 0) && ((res = property_get("persist.mtk.wcn.combo.chipid", chip_id, NULL)) < 6)) {
        ERR("get chip id fail, retry");
        usleep(200000);
    }
    DBG("get chip id is:%s\n", chip_id);
    if (strcmp(chip_id, "0x6572") == 0 || strcmp(chip_id, "0x6582") == 0 ||
        strcmp(chip_id, "0x6580") == 0 || strcmp(chip_id, "0x6592") == 0 || strcmp(chip_id, "0x6571") == 0 ||
        strcmp(chip_id, "0x8127") == 0 || strcmp(chip_id, "0x0335") == 0 ||strcmp(chip_id, "0x8163") == 0) {
        gps_epo_type = 1;    // GPS only
    } else if (strcmp(chip_id, "0x6630") == 0 || strcmp(chip_id, "0x6752") == 0 || strcmp(chip_id, "0x6755") == 0) {
        gps_epo_type = 0;   // G+G
    } else {
        gps_epo_type = 0;   // Default is G+G
    }
    if (gps_epo_type == 0) {
        for (idx = 0; idx < cnt; idx++) {
            if (!read_prop(mnl_prop_path[idx]))
                break;
        }
        if ((gnss_mode != 0) && (gnss_mode != 2)) {
            gps_epo_type = 1;
        }
    }
#endif
    return 0;
}

static void
mtk_gps_cleanup(void)
{
    GpsState*  s = _gps_state;
    TRC();

    if (mtk_cleanup() != 0)
          ERR("mtk cleanup error!!");
    // make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF by callback function
    if (s->start_flag)
        get_condition(&lock_for_sync[M_STOP]);
    if (lock_for_sync[M_STOP].condtion == 1) {
        // make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF in next time
        lock_for_sync[M_STOP].condtion = 0;
    }
    if (s->init)
        gps_state_done(s);
#if EPO_SUPPORT
    s->thread_epo_exit_flag = 1;
    get_condition(&lock_for_sync[M_THREAD_EXIT]);
#endif
    DBG("mtk_gps_cleanup done");
   //     return NULL;
}
#if EPO_SUPPORT
int mtk_gps_epo_file_time_hal(long long uTime[]);
int
mtk_epo_is_expired(int wifi_tragger) {
    long long uTime[3];
    memset(uTime, 0, sizeof(uTime));
    //     time_t time_st;
    time_t         now = time(NULL);
    struct tm      tm_utc;
    unsigned long  time_utc;
    long long etime = gps_epo_period*24*60*60;
    long long expired_set = 0;
    int download_day = 0;

    //     time(&time_st);
    gmtime_r(&now, &tm_utc);
    time_utc = mktime(&tm_utc);
    mtk_gps_epo_file_time_hal(uTime);

    if (wifi_tragger) {
        expired_set = (long long)wifi_epo_period*24*60*60;    // for wifi tragger we change checking expired time to 1 day.
    } else if ((uTime[2] - uTime[0]) < etime) {
        download_day = (uTime[2] - uTime[0])/(24*60*60);
           // DBG("epo data downloaded dat: %d\n", download_day);
        if (download_day < 3) {
            expired_set = 0;
        } else if (download_day < 6) {
            expired_set = 2*24*60*60;
        } else if ((6 <= download_day) && (download_day < 9)) {
            expired_set = 5*24*60*60;
        } else if ((9 <= download_day) && (download_day < 12)) {
            expired_set = 7*24*60*60;
        } else if ((12 <= download_day) && (download_day < 15)) {
            expired_set = 7*24*60*60;
        } else if ((15 <= download_day) && (download_day < 18)) {
            expired_set = 7*24*60*60;
        } else if (download_day >= 18) {
            expired_set = 7*24*60*60;
        }
    } else {
        expired_set = etime;
    }

    DBG("current time: %ld, current time:%s", time_utc, ctime(&time_utc));
    DBG("EPO start time: %lld, EPO start time: %s", uTime[0], ctime(&uTime[0]));
      //  DBG("EPO expired_set: %lld", expired_set);
    if (time_utc >= (expired_set + uTime[0])) {
        DBG("EPO file is expired");
        gps_epo_file_count = 0;
        return 1;
    } else {
        DBG("EPO file is valid, no need update");
        return 0;
    }
}
#endif
int
mtk_gps_start()
{
    GpsState*  s = _gps_state;
    int err;
    int count=0;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    DBG("mtk_gps_start enter while loop");
    while (s->start_flag == 1) {
        usleep(100000);
        count++;
        DBG("mtk_gps_start:start_flag = %d delay=%d \n", s->start_flag, count*100);
    }

    if ((err = mtk_start())) {
        ERR("mtk_start err = %d", err);
        return -1;
    }
#if EPO_SUPPORT
    gps_download_epo_enable();
    DBG("gps_epo_enable=%d, s->epo_data_updated=%d\n", gps_epo_enable, s->epo_data_updated);
    DBG("nw_type=%d, nw_connected=%d\n", nw_type, nw_connected, epo_download_failed);
    if (gps_epo_enable) {  //  && (s->epo_data_updated == 0))
        if (access(EPO_FILE_HAL, 0) == -1) {
            DBG("no EPOHAL file, the EPO.DAT is not exsited, or is the latest one\n");
               // check if EPO.DAT existed
            if (access(EPO_FILE, 0) == -1) {
                   // request download
                DBG("Both EPOHAL.DAT and EPO.DAT are not existed, download request 1");
                gps_download_epo(s);
            }
            else {
               // check if EPO.DAT is expired
               if (mtk_epo_is_expired(0)) {
                    DBG("EPOHAL.DAT is not existed and EPO.DAT expired, download request 2");
                    gps_download_epo(s);
                }
            }
        } else {
            if (((nw_type == nw_data) && (nw_connected == 1) && epo_download_failed)
               || ((nw_connected == 0) && epo_download_failed)|| (epo_download_failed == 0)) {
                // to check if EPOHAL.DAT is expired.
                if (mtk_epo_is_expired(0)) {
                    DBG("EPOHAL is expired, download request 3");
                    unlink(EPO_FILE_HAL);
                    gps_download_epo(s);
                } else {
                    DBG("EPOHAL is existed and no expired, tell agent to update");
                       // char buf[] = {MNL_CMD_UPDATE_EPO_FILE};
                       // char cmd = HAL_CMD_STOP_UNKNOWN;
                    int offset = 0;
                    char buff[1024] = {0};

                    unlink(EPO_FILE);
                    buff_put_int(MNL_CMD_UPDATE_EPO_FILE, buff, &offset);
                    err = mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buff, sizeof(buff));
                    if (-1 == err) {
                        ERR("Request update epo file fail\n");
                    } else {
                        DBG("Request update epo file successfully\n");
                    }
                }
            }
       }
    }
#endif
    get_condition(&lock_for_sync[M_START]);
    DBG("HAL thread has initialiazed\n");
    gps_state_start(s);

    sta.status = GPS_STATUS_ENGINE_ON;
    DBG("sta.status = GPS_STATUS_ENGINE_ON\n");
    callback_backup_mtk.base.status_cb(&sta);
    sta.status = GPS_STATUS_SESSION_BEGIN;
    DBG("sta.status = GPS_STATUS_SESSION_BEGIN\n");
    callback_backup_mtk.base.status_cb(&sta);

    callback_backup_mtk.base.acquire_wakelock_cb();   // avoid cpu to sleep
    if (lock_for_sync[M_STOP].condtion == 1) {
        lock_for_sync[M_STOP].condtion = 0;    // make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF in next time
    }
    s->start_flag = 1;
    DBG("s->start_flag = 1\n");
    return 0;
}

#ifdef GPS_AT_COMMAND
/*for GPS AT command test*/
int mtk_gps_test_start(int test_num, int prn_num, int time_delay, int test_mode) {

    GpsState*  s = _gps_state;
    int err;

    if ((MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN) && test_mode_flag) {
        DBG("[SMS test mode] Timeout, test_stop() before test_start()");
        mtk_gps_test_stop();
    }

    hal_test_data.test_num = test_num;
    hal_test_data.prn_num = prn_num;
    hal_test_data.time_delay = time_delay;
    time(&start_time);

    //  ithis code is moved from stop function to here to keep avg value for AT%GPS(GNSS) or AT%GPS=?(GNSS=?)
    Avg_CNo = 0;
    Dev_CNr = (int*)malloc(sizeof(int)*hal_test_data.test_num);
    memset(Dev_CNr, 0, test_num*sizeof(int));

    if ((0 == hal_test_data.test_num) && (0 == test_mode)) {
        ERR("%s: test number is 0!!", __FUNCTION__);
        return -1;
    }
    if (1 == test_mode) {
        DBG("Signal test mode");
        MNL_AT_SIGNAL_MODE = 1;
    } else {
        DBG("Normal test mode");
        MNL_AT_TEST_FLAG = 1;
    }

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;

    if ((err = mtk_start())) {
        ERR("mtk_start err = %d", err);
        MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
        MNL_AT_TEST_FLAG = 0;
        return -1;
    }

    TRC();
    gps_state_test_start(s);
    return 0;
}
int
mtk_gps_test_stop()
{
    GpsState*  s = _gps_state;
    int err;
    test_mode_flag = 1;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }
    if ((err = mtk_stop())) {
        ERR("mtk_stop err = %d", err);
        return -1;
    }

    TRC();
    gps_state_test_stop(s);

    hal_test_data.test_num = 0;
    hal_test_data.prn_num = 0;
    hal_test_data.time_delay = 0;
    MNL_AT_TEST_FLAG = 0;
    MNL_AT_SIGNAL_MODE = 0;
    MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;

    MNL_AT_SET_ALARM       = 0;
    MNL_AT_CANCEL_ALARM    = 0;
    MNL_AT_SIGNAL_TEST_BEGIN = 0;

    // release the variable
    Success_Num = 0;
    Completed_Num = 0;
    Wait_Num = 0;
    CNo = 0;
    DCNo = 0;
    Dev_CNo = 0;
    Err_Code = 1;
    test_num = 0;

    if (NULL != Dev_CNr) {
        DBG("Free Dev_CNr");
        free(Dev_CNr);
    }
    return 0;
}

int
mtk_gps_test_inprogress()
{
    int ret = -1;

    if ((MNL_AT_TEST_STATE == MNL_AT_TEST_DONE) || (MNL_AT_TEST_STATE == MNL_AT_TEST_RESULT_DONE)) {
        DBG("**AT Command test done!!");
        ret = MNL_AT_TEST_DONE;
    } else if (MNL_AT_TEST_STATE == MNL_AT_TEST_INPROGRESS) {
        DBG("**AT Command test is in progress!!");
        ret = MNL_AT_TEST_INPROGRESS;
    } else {
        DBG("**AT Command test status unknown!!");
        ret = MNL_AT_TEST_UNKNOWN;
    }
    return ret;
}
#endif
int
mtk_gps_stop()
{
    GpsState*  s = _gps_state;
    int err;
    int count=0;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    DBG("mtk_gps_stop enter while loop");
    while (s->start_flag == 0) {
        usleep(100000);
        count++;
        DBG("mtk_gps_stop:start_flag = %d delay=%d \n", s->start_flag, count*100);
    }

    if ((err = mtk_stop())) {
        ERR("mtk_stop err = %d", err);
        return -1;
    }

    TRC();
    gps_state_stop(s);
    return 0;
}

static int
mtk_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    ntp_context inject_ntp;
    long long time_s = 0;
    char buff[1024] = {0};
    int offset = 0;

    TRC();
    time_s = time/1000;
    DBG("inject time= %lld, ctime = %s, timeReference = %lld, uncertainty =%d\n",
       time, ctime(&time_s), timeReference, uncertainty);
    memcpy(&(inject_ntp.time), &time, sizeof(GpsUtcTime));
    inject_ntp.timeReference = timeReference;
    inject_ntp.uncertainty = uncertainty;

    buff_put_int(MNL_CMD_GPS_INJECT_TIME, buff, &offset);
    buff_put_struct(&inject_ntp, sizeof(ntp_context), buff, &offset);
    if (g_agps_ctx.send_fd >= 0) {
        int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    } else {
        ERR("g_agps_ctx.send_fd is not initialized\n");
    }
    return 0;
}

static int
mtk_gps_inject_location(double latitude, double longitude, float accuracy)
{
    nlp_context nlp_location;
    char buff[1024] = {0};
    int offset = 0;

    if (clock_gettime(CLOCK_MONOTONIC , &nlp_location.ts) == -1) {
        ERR("clock_gettime failed reason=[%s]\n", strerror(errno));
        return -1;
    }
    DBG("ts.tv_sec= %lld, ts.tv_nsec = %lld\n", nlp_location.ts.tv_sec, nlp_location.ts.tv_nsec);
    DBG("inject location lati= %f, longi = %f, accuracy =%f\n", latitude, longitude, accuracy);

    nlp_location.latitude = latitude;
    nlp_location.longitude = longitude;
    nlp_location.accuracy = accuracy;
    nlp_location.type= 0;
    nlp_location.started = started;
    mtk_gps_update_location(&nlp_location);
    buff_put_int(MNL_CMD_GPS_INJECT_LOCATION, buff, &offset);
    buff_put_struct(&nlp_location, sizeof(nlp_context), buff, &offset);
    if (g_agps_ctx.send_fd >= 0) {
        int res = mtk_daemon_send(g_agps_ctx.send_fd, MTK_HAL2MNLD, buff, sizeof(buff));
    }
    else {
        ERR("g_agps_ctx.send_fd is not initialized\n");
    }
    return 0;
}

static void
mtk_gps_delete_aiding_data(GpsAidingData flags)
{
    /*workaround to trigger hot/warm/cold/full start*/
    #define FLAG_HOT_START  GPS_DELETE_RTI
    #define FLAG_WARM_START GPS_DELETE_EPHEMERIS
    #define FLAG_COLD_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC | GPS_DELETE_HEALTH)
    #define FLAG_FULL_START (GPS_DELETE_ALL)
    #define FLAG_AGPS_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_ALMANAC | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC)
    GpsState*  s = _gps_state;

    DBG("%s:0x%X\n", __FUNCTION__, flags);

    gps_state_restart(s);

    if (flags == FLAG_HOT_START) {
        DBG("Send MNL_CMD_RESTART_HOT in HAL\n");
        mtk_restart(MNL_CMD_RESTART_HOT);
    } else if (flags == FLAG_WARM_START) {
        DBG("Send MNL_CMD_RESTART_WARM in HAL\n");
        mtk_restart(MNL_CMD_RESTART_WARM);
    } else if (flags == FLAG_COLD_START) {
        DBG("Send MNL_CMD_RESTART_AGPS/COLD in HAL\n");
        mtk_restart(MNL_CMD_RESTART_COLD);
    } else if (flags == FLAG_FULL_START) {
        DBG("Send MNL_CMD_RESTART_FULL in HAL\n");
        mtk_restart(MNL_CMD_RESTART_FULL);
    } else if (flags == FLAG_AGPS_START) {
        DBG("Send MNL_CMD_RESTART_AGPS in HAL\n");
        mtk_restart(MNL_CMD_RESTART_AGPS);
    }
}

static int mtk_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
       //  FIXME - support fix_frequency
       //  only standalone supported for now.
    DBG("set report location time interval is %d\n", min_interval);
    if (min_interval <= 1000)
        report_time_interval = 1;
    else
        report_time_interval = (float)min_interval/1000;
    DBG("set report location time interval is %f s\n", report_time_interval);
    TRC();
    return 0;
}

#if EPO_SUPPORT
void retry_alarm_handler(sigval_t v)
{
    int timeout = (int)v.sival_int;

    // DBG("timeout =%d\n", timeout);
    epo_download_retry = 1;
    DBG("epo_download_retry is =%d\n", epo_download_retry);
}

static int set_retry_alarm_handler(int timeout)
{
    int err = 0;

    if (retry_timer.fd != C_INVALID_TIMER) {
        if (err = timer_delete(retry_timer.fd)) {
            DBG("timer_delete(%d) = %d (%s)\n", retry_timer.fd, errno, strerror(errno));
            return -1;
        }
        retry_timer.fd = C_INVALID_TIMER;
    }
    if (retry_timer.fd == C_INVALID_TIMER) {
        memset(&retry_timer.evt, 0x00, sizeof(retry_timer.evt));
        retry_timer.evt.sigev_value.sival_ptr = &retry_timer.fd;
        retry_timer.evt.sigev_value.sival_int = timeout;
        retry_timer.evt.sigev_notify = SIGEV_THREAD;
        retry_timer.evt.sigev_notify_function = retry_alarm_handler;
        if ((err = timer_create(CLOCK_REALTIME, &retry_timer.evt, &retry_timer.fd))) {
            DBG("timer_create = %d(%s)\n", errno, strerror(errno));
            return -1;
        }
    }
    retry_timer.expire.it_interval.tv_sec = 0;
    retry_timer.expire.it_interval.tv_nsec = 0;
    retry_timer.expire.it_value.tv_sec = timeout;
    retry_timer.expire.it_value.tv_nsec = 0;
    if ((err = timer_settime(retry_timer.fd, 0, &retry_timer.expire, NULL))) {
        DBG("timer_settime = %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    DBG("(%d, %d)\n", retry_timer.fd, retry_timer.expire);
    return 0;
}
int mtk_gps_epo_file_update();
static void *thread_epo_file_update(void* arg) {

    GpsState* s = (GpsState *)arg;
    int schedule_delay = 100;
    DBG("EPO thread start");
    while (1) {
        usleep(schedule_delay*1000);
        if (s->thread_epo_exit_flag == 1) {
            DBG("EPO thread exit\n");
            break;
        }
        gps_download_epo_enable();
        if (s->epo_data_updated == 1) {
            DBG("EPO data download begin...");
            epo_download_failed = 0;
               // s->epo_data_updated = 0;
            gps_download_epo_file_name(gps_epo_file_count);
            mtk_gps_epo_file_update();
            if (epo_download_failed == 1) {
                s->epo_data_updated = 0;
            }
        }
        // DBG("epo_download_retry =%d, started=%d\n", epo_download_retry, started);
        if (started || (nw_type == nw_wifi)) {
            if (nw_connected && epo_download_failed && epo_download_retry) {
                // if download has failed last time, we should complete downloading.
                long long uTime[3] ={0};
                //  time_t time_st;
                time_t         now = time(NULL);
                struct tm      tm_utc;
                unsigned long  time_utc;
                int file_count_temp = 0;
                static int file_retrying = 50;   // 50 for first restore
                static int retry_time = 0;

                if (file_retrying == 50) {
                    file_retrying = gps_epo_file_count;
                }
                file_count_temp = gps_epo_file_count;
                DBG("EPO data download resume...file_count_temp=%d\n", file_count_temp);
                   // time(&time_st);
                gmtime_r(&now, &tm_utc);
                time_utc = mktime(&tm_utc);
                mtk_gps_epo_file_time_hal(uTime);

                if (time_utc >= (uTime[0] + 24*60*60)) {
                    // if epo date is expired > 1 day, we should begin with first file.
                    epo_download_failed = 0;
                    gps_epo_file_count = 0;
                    s->epo_data_updated = 1;
                    unlink(EPO_FILE_HAL);
                } else {   // if epo data is expired < 1 day, we can begin with failed file last time.
                    gps_download_epo_file_name(gps_epo_file_count);
                    mtk_gps_epo_file_update();
                }
                DBG("gps_epo_file_count=%d\n", gps_epo_file_count);
                if (file_count_temp == gps_epo_file_count) {
                        int time_out = 15;   // delay 15 s

                        if (file_retrying == gps_epo_file_count) {
                            retry_time++;
                        }
                        else {
                            file_retrying = gps_epo_file_count;
                            retry_time = 0;
                        }
                        DBG("retry_time=%d, time_out=%d\n", retry_time, time_out);
                        if (retry_time < 10) {
                            epo_download_retry = 0;
                            set_retry_alarm_handler(time_out);
                        }
                        else {
                            epo_download_failed = 0;
                            retry_time = 0;
                            file_retrying = 50;
                        }
                }
            }
        }
        // if wifi connection, we trigger epo download.
        if ((s->epo_data_updated == 0) && gps_epo_wifi_trigger && gps_epo_enable) {
            if ((epo_download_failed == 0) && nw_connected && (nw_type == nw_wifi)) {
                if (mtk_epo_is_expired(1)) {
                        DBG("EPO data download wifi tragger...");
                        gps_download_epo(s);
                        unlink(EPO_FILE_HAL);
                }
            }
        }
        if ((s->epo_data_updated == 0) && (epo_download_failed == 0))
        {
            schedule_delay = 2000;
        }
        else
        {
            schedule_delay = 100;
        }
    }
       // pthread_exit(NULL);
    release_condition(&lock_for_sync[M_THREAD_EXIT]);
    DBG("EPO thread exit done");
    return NULL;
}

   // zqh: download EPO by request
int mtk_gps_epo_interface_init(GpsXtraCallbacks* callbacks) {
    TRC();
    int ret = -1;
    GpsState*  s = _gps_state;
    if (s->init) {
        mGpsXtraCallbacks = *callbacks;
        ret = 0;
    }
    retry_timer.fd = C_INVALID_TIMER;

    // start thread to write data to file
    ret = pthread_create(&s->thread_epo, NULL, thread_epo_file_update, s);
    if (0 != ret) {
    ERR("EPO thread create fail: %s\n", strerror(errno));
        return ret;
    }
    s->thread_epo_exit_flag = 0;
    DBG("mtk_gps_epo_interface_init done");
    return ret;
}


int mtk_gps_inject_epo_data(char* data, int length) {
#if 0
    GpsState* s = _gps_state;
    if (length <= 0) {
        ERR("EPO data lengh error!!");
        return -1;
    }

    epo_data.length = length;
    epo_data.data = data;

    s->epo_data_updated = 1;
    DBG("length = %d, epo_data.length = %d", length, epo_data.length);
    DBG("EPO download done, epo_data_updated = %d\n", s->epo_data_updated);
#endif
    TRC();
    return 0;
}

static const GpsXtraInterface mtkGpsXtraInterface = {
    sizeof(GpsXtraInterface),
    mtk_gps_epo_interface_init,
    mtk_gps_inject_epo_data,
};
#endif

// download EPO by request end
int mtk_gps_measurement_init(GpsMeasurementCallbacks* callbacks) {
    DBG("mtk_gps_measurement_init\n");
    int err = -1;
    GpsState*  s = _gps_state;
    if ((err = mtk_measurement_init())) {
        ERR("mtk_measurement_init err = %d", err);
        return -1;
    }
    measurement_callbacks = *callbacks;
    gpsmeasurement_init_flag = 1;
    DBG("mtk_gps_measurement_init done\n");
    return 0;
}
void mtk_gps_measurement_close() {
    DBG("mtk_gps_measurement_close\n");

    int err = -1;
    GpsState*  s = _gps_state;

    if ((err = mtk_measurement_close())) {
        ERR("mtk_measurement_close err = %d", err);
    }

    gpsmeasurement_init_flag = 0;
    DBG("mtk_gps_measurement_close done\n");
    return;
}

static const GpsMeasurementInterface mtkGpsMeasurementInterface = {
    sizeof(GpsMeasurementInterface),
    mtk_gps_measurement_init,
    mtk_gps_measurement_close,
};

int mtk_gps_navigation_init(GpsNavigationMessageCallbacks* callbacks) {
    DBG("mtk_gps_navigation_init\n");
    int err = -1;
    GpsState*  s = _gps_state;
    if ((err = mtk_navigation_init())) {
        ERR("mtk_navigation_init err = %d", err);
        return -1;
    }
    navigation_callbacks = *callbacks;
    gpsnavigation_init_flag = 1;
    DBG("mtk_gps_navigation_init done");
    return 0;
}

void mtk_gps_navigation_close() {
    DBG("mtk_gps_navigation_close\n");

    int err = -1;
    GpsState*  s = _gps_state;
    if ((err = mtk_navigation_close())) {
        ERR("mtk_navigation_close err = %d", err);
    }

    gpsnavigation_init_flag = 0;
    DBG("mtk_gps_navigation_close done\n");
    return NULL;
}

static const GpsNavigationMessageInterface mtkGpsNavigationMessageInterface = {
    sizeof(GpsNavigationMessageInterface),
    mtk_gps_navigation_init,
    mtk_gps_navigation_close,
};

static const void*
mtk_gps_get_extension(const char* name)
{
    TRC();
    DBG("mtk_gps_get_extension name=[%s]\n", name);
#if EPO_SUPPORT
    if (!strcmp(name, GPS_XTRA_INTERFACE))
        return (void*)(&mtkGpsXtraInterface);
#endif
    if (strncmp(name, "agps", strlen(name)) == 0) {
        return &mtkAGpsInterface;
    }
    if (strncmp(name, "gps-ni", strlen(name)) == 0) {
        return &mtkGpsNiInterface;
    }
    if (strncmp(name, "agps_ril", strlen(name)) == 0) {
        return &mtkAGpsRilInterface;
    }
    if (strncmp(name, "supl-certificate", strlen(name)) == 0) {
       return &mtkSuplCertificateInterface;
    }
    if (strncmp(name, GPS_MEASUREMENT_INTERFACE, strlen(name)) == 0) {
       return &mtkGpsMeasurementInterface;
    }
    if (strncmp(name, GPS_NAVIGATION_MESSAGE_INTERFACE, strlen(name)) == 0) {
       return &mtkGpsNavigationMessageInterface;
    }
    return NULL;
}
#if EPO_SUPPORT
int
mtk_gps_sys_read_lock(int fd, off_t offset, int whence, off_t len) {

    struct flock lock;

    lock.l_type = F_RDLCK;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    if (fcntl(fd, F_SETLK, &lock) < 0) {
        return -1;
    }

    return 0;
}

int
mtk_gps_sys_write_lock(int fd, off_t offset, int whence, off_t len) {
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    if (fcntl(fd, F_SETLK, &lock) < 0) {
        return -1;
    }

    return 0;
}

static unsigned int
mtk_gps_sys_get_file_size() {
    unsigned int fileSize;
    int res_epo, res_epo_hal;
    struct stat st;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_FILE_HAL;
    char epofile[32] = {0};
    res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_FILE_HAL, F_OK);
    if (res_epo < 0 && res_epo_hal < 0) {
        DBG("no EPO data yet\n");
        return -1;
    }
    if (res_epo_hal == 0) {  /*EPOHAL.DAT is here*/
        // DBG("find EPOHAL.DAT here\n");
        strcpy(epofile, epo_file_hal);
    }
    else if (res_epo == 0) {  /*EPO.DAT is here*/
        // DBG("find EPO.DAT here\n");
        strcpy(epofile, epo_file);
    }
    else
        ERR("unknown error happened\n");
    if (stat(epofile, &st) < 0) {
        ERR("Get file size error, return\n");
        return 0;
        }

    fileSize = st.st_size;
       // DBG("EPO file size: %d\n", fileSize);
    return fileSize;
}
void GpsToUtcTime(int i2Wn, double dfTow, time_t* uSecond)
{
    struct tm target_time;
    int iYearsElapsed;        //  Years since 1980.
    unsigned int iDaysElapsed;         //  Days elapsed since Jan 1, 1980.
    double dfSecElapsed;
    unsigned int fgLeapYear;
    int pi2Yr;
    int pi2Mo;
    int pi2Day = 0;
    int pi2Hr;
    int pi2Min;
    double pdfSec;
    int i;


    //  Number of days into the year at the start of each month (ignoring leap
    //  years).
    unsigned int doy[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    //  Convert time to GPS weeks and seconds
    iDaysElapsed = i2Wn * 7 + ((int)dfTow / 86400) + 5;
    dfSecElapsed = dfTow - ((int)dfTow / 86400) * 86400;


    //  decide year
    iYearsElapsed = 0;       //  from 1980
    while (iDaysElapsed >= 365)
    {
        if ((iYearsElapsed % 100) == 20) {   //  if year % 100 == 0
            if ((iYearsElapsed % 400) == 20) {   //  if year % 400 == 0
                if (iDaysElapsed >= 366) {
                    iDaysElapsed -= 366;
                } else {
                    break;
                }
            } else {
                iDaysElapsed -= 365;
            }
        } else if ((iYearsElapsed % 4) == 0) {   //  if year % 4 == 0
            if (iDaysElapsed >= 366) {
                iDaysElapsed -= 366;
            } else {
                break;
            }
        } else {
            iDaysElapsed -= 365;
        }
        iYearsElapsed++;
    }
    pi2Yr = 1980 + iYearsElapsed;


    //  decide month, day
    fgLeapYear = 0;
    if ((iYearsElapsed % 100) == 20) {    //  if year % 100 == 0
        if ((iYearsElapsed % 400) == 20) {    //  if year % 400 == 0
           fgLeapYear = 1;
        }
    }
    else if ((iYearsElapsed % 4) == 0) {   //  if year % 4 == 0
        fgLeapYear = 1;
    }

    if (fgLeapYear) {
        for (i = 2; i < 12; i++) {
            doy[i] += 1;
        }
    }
    for (i = 0; i < 12; i++)
    {
        if (iDaysElapsed < doy[i]) {
            break;
        }
    }
    pi2Mo = i;
    if (i > 0) {
        pi2Day = iDaysElapsed - doy[i-1] + 1;
    }

    //  decide hour, min, sec
    pi2Hr = dfSecElapsed / 3600;
    pi2Min = ((int)dfSecElapsed % 3600) / 60;
    pdfSec = dfSecElapsed - ((int)dfSecElapsed / 60) * 60;

    // change the UTC time to seconds
    memset(&target_time, 0, sizeof(target_time));
    target_time.tm_year = pi2Yr - 1900;
    target_time.tm_mon = pi2Mo - 1;
    target_time.tm_mday = pi2Day;
    target_time.tm_hour = pi2Hr;
    target_time.tm_min = pi2Min;
    target_time.tm_sec = pdfSec;
    target_time.tm_isdst = -1;
    *uSecond = mktime(&target_time);
    if (*uSecond < 0) {
        ERR("Convert UTC time to seconds fail, return\n");
    }

}


static int
mtk_gps_sys_epo_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {         // no file lock
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    // if (fread(szBuf, 1, MTK_EPO_ONE_SV_SIZE, pFile) != MTK_EPO_ONE_SV_SIZE) {
    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    // DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}


static int
mtk_gps_sys_epo_period_end(int fd, unsigned int *u4GpsSecs, time_t* uSecond) {   // no file lock
    int fileSize;
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    fileSize = mtk_gps_sys_get_file_size();
    if (fileSize < MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    if (lseek(fd, (fileSize - MTK_EPO_ONE_SV_SIZE), SEEK_SET) == -1)
        ERR("lseek error: %s\n", strerror(errno));
    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    (*u4GpsSecs) += 21600;

    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    // TRC();
    // DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);

    return 0;
}

int
mtk_gps_epo_file_time_hal(long long uTime[]) {

    TRC();
    struct stat filestat;
    int fd = 0;
    int addLock, res_epo, res_epo_hal;
    unsigned int u4GpsSecs_start;    // GPS seconds
    unsigned int u4GpsSecs_expire;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_FILE_HAL;
    char epofile[32] = {0};
    time_t uSecond_start;      // UTC seconds
    time_t uSecond_expire;
    int ret = 0;
    pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

    ret = pthread_mutex_lock(&mutx);
    res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_FILE_HAL, F_OK);
    if (res_epo < 0 && res_epo_hal < 0) {
        DBG("no EPO data yet\n");
        ret = pthread_mutex_unlock(&mutx);
        return -1;
    }
    if (res_epo_hal== 0) {  /*EPOHAL.DAT is here*/
        // DBG("find EPOHAL.DAT here\n");
        strcpy(epofile, epo_file_hal);
    } else if (res_epo == 0) {  /*EPO.DAT is here*/
           // DBG("find EPO.DAT here\n");
        strcpy(epofile, epo_file);
    } else
        ERR("unknown error happened\n");

    // open file
    fd = open(epofile, O_RDONLY);
    if (fd < 0) {
        ERR("Open EPO fail, return\n");
        ret = pthread_mutex_unlock(&mutx);
        return -1;
    }

    // Add file lock
    if (mtk_gps_sys_read_lock(fd, 0, SEEK_SET, 0) < 0) {
    ERR("Add read lock failed, return\n");
       close(fd);
    ret = pthread_mutex_unlock(&mutx);
    return -1;
   }

    // EPO start time
    if (mtk_gps_sys_epo_period_start(fd, &u4GpsSecs_start, &uSecond_start)) {
        ERR("Get EPO file start time error, return\n");
        close(fd);
        ret = pthread_mutex_unlock(&mutx);
        return -1;
    } else {
        uTime[0] = (long long)uSecond_start;
        //  DBG("The Start time of EPO file is %lld", uTime[0]);
        //  DBG("The start time of EPO file is %s", ctime(&uTime[0]));
    }

    // download time
    stat(epofile, &filestat);
    uTime[1] = (long long)(filestat.st_mtime);
    // uTime[1] = uTime[1] - 8 * 3600;
    // DBG("Download time of EPO file is %lld", uTime[1]);
    // DBG("Download time of EPO file is %s\n", ctime(&uTime[1]));

    // EPO file expire time
    if (mtk_gps_sys_epo_period_end(fd, &u4GpsSecs_expire, &uSecond_expire)) {
        ERR("Get EPO file expire time error, return\n");
        close(fd);
        ret = pthread_mutex_unlock(&mutx);
        return -1;
    } else {
        uTime[2] = (long long)uSecond_expire;
        // DBG("The expire time of EPO file is %lld", uTime[2]);
        //  DBG("The expire time of EPO file is %s", ctime(&uTime[2]));
    }

    close(fd);
    ret = pthread_mutex_unlock(&mutx);
    return 0;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
CURLcode curl_easy_download(char* url, char* filename)
{
    CURL *curl = NULL;
    FILE *fp = NULL;
    CURLcode res;
    TRC();

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "w+");
        if (fp == NULL) {
            curl_easy_cleanup(curl);
            return CURLE_FAILED_INIT;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        //   curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        return res;
    } else {
        return CURLE_FAILED_INIT;
    }

}
extern char* getEpoUrl(char* filename, char* key);
static int counter = 1;
CURLcode curl_easy_download_epo(void)
{
    int res_val;
    CURLcode res;
    char gps_epo_md_file_temp[60] = {0};
    char gps_epo_md_key[70] = {0};
    char gps_epo_data_file_name[60] = {0};
    char* url = NULL;
    char* key = NULL;
    char* md_url = NULL;
    char count_str[5]={0};
    TRC();

    strcat(gps_epo_md_file_temp, "/data/misc/gps/");
    strcat(gps_epo_md_file_temp, gps_epo_md_file_name);
    // DBG("gps_epo_md_file_name = %s\n", gps_epo_md_file_name);
	strcpy(gps_epo_md_key, "0000000000000000");
    memset(count_str,0,sizeof(count_str));
    sprintf(count_str,"%d", counter);
    strcat(gps_epo_md_key, "&counter=");
    strcat(gps_epo_md_key, count_str);
    md_url = getEpoUrl(gps_epo_md_file_name, gps_epo_md_key);
    // DBG("md_url = %s\n", md_url);
    if (md_url == NULL) {
        DBG("getEpoUrl failed!\n");
        return CURLE_FAILED_INIT;
    }
    res = curl_easy_download(md_url, gps_epo_md_file_temp);
    // DBG("md file curl_easy_download res = %d\n", res);
    free(md_url);
    memset(gps_epo_md_key, 0 , sizeof(gps_epo_md_key));
    if (res == 0) {
        FILE *fp = NULL;
        char* key_temp = NULL;
        int len = 0;
        fp = fopen(gps_epo_md_file_temp, "r");
        if (fp != NULL) {
            len = fread(gps_epo_md_key, sizeof(char), sizeof(char)*48, fp);
            key_temp = gps_epo_md_key;
            fclose(fp);
            unlink(gps_epo_md_file_temp);
            DBG("gps_epo_md_key before cpy= %s, len=%d\n", gps_epo_md_key, len);
            memcpy(gps_epo_md_key, key_temp+32, 16);    // comment by rayjf li
            gps_epo_md_key[16] = '\0';
        }
        else {
            strcpy(gps_epo_md_key, "0000000000000000");
        }
        counter = 1;
    }
    else {
        strcpy(gps_epo_md_key, "0000000000000000");
        counter ++;
    }
    // DBG("gps_epo_md_key = %s\n", gps_epo_md_key);
    memset(count_str,0,sizeof(count_str));
    sprintf(count_str,"%d", counter);
    strcat(gps_epo_md_key, "&counter=");
    strcat(gps_epo_md_key, count_str);
    url = getEpoUrl(gps_epo_file_name, gps_epo_md_key);

    DBG("url = %s\n", url);
    if (url == NULL) {
        DBG("getEpoUrl failed!\n");
        return CURLE_FAILED_INIT;
    }
    strcat(gps_epo_data_file_name, "/data/misc/gps/");
    strcat(gps_epo_data_file_name, gps_epo_file_name);
    res = curl_easy_download(url, gps_epo_data_file_name);
    DBG("'epo' epo file curl_easy_download res = %d\n", res);
    free(url);
    if (res == CURLE_OK) {
        FILE *fp_temp = NULL;
        FILE *fp = NULL;

        counter = 1;
        if (gps_epo_file_count == 0) {
            unlink(EPO_FILE_HAL);
        }
        res_val = chmod(gps_epo_data_file_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        DBG("chmod res_val = %d, %s\n", res_val, strerror(errno));
        fp_temp = fopen(EPO_FILE_HAL, "at");
        if (fp_temp != NULL) {
            fp = fopen(gps_epo_data_file_name, "r");
            if (fp != NULL) {
                #define buf_size  256
                char data[buf_size] = {0};
                int bytes_in = 0, bytes_out = 0;
                int len = 0;

                while ((bytes_in = fread(data, 1, sizeof(data), fp)) > 0 && (bytes_in <= (buf_size* sizeof(char)))) {
                    bytes_out = fwrite(data, 1, bytes_in, fp_temp);
                    if (bytes_in != bytes_out) {
                        DBG("bytes_in = %d,bytes_out = %d\n", bytes_in, bytes_out);
                    }
                    len += bytes_out;
                    // DBG("copying file...%d bytes copied\n",len);
                }
                fclose(fp);
            } else {
                ERR("Open merged file fp=NULL\n");
            }
            fclose(fp_temp);
        }
        else {
            ERR("Open merged file failed\n");
        }
        res_val = chmod(EPO_FILE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
        DBG("chmod res_val = %d, %s\n", res_val, strerror(errno));
    } else {
        unlink(gps_epo_data_file_name);
        counter ++;
    }
    return res;
}

int
mtk_gps_epo_file_update_hal() {
     //   TRC();
#if 0
    int fd_write, res;
    ssize_t bytes_read;
    ssize_t bytes_write;
    char* data = epo_data.data;
    int length = epo_data.length;

    int result;
    DBG("length = %d", length);
    DBG("Update EPO HAL file...\n");

    fd_write = open(EPO_FILE_HAL, O_WRONLY | O_TRUNC | O_CREAT | O_NONBLOCK | O_APPEND, 0641);
    if (fd_write < 0) {
        ERR("open /data/misc/EPOHAL.DAT error!\n");
    return -1;
    }

    if (mtk_gps_sys_write_lock(fd_write, 0, SEEK_SET, 0) < 0) {
        ERR("Add read lock failed, return\n");
        close(fd_write);
        return -1;
    }

       // start copy new data from framework to EPOHAL.DAT
    while (length > 0) {
        bytes_write = write(fd_write, data, length);
        if (bytes_write < 0) {
            ERR("Write EPOHAL.DAT error: %s\n", strerror(errno));
            length = 0;
            data = NULL;
            close(fd_write);
            return -1;
        }

        DBG("bytes_write = %d\n", bytes_write);
        DBG("EPO data: %s", data);

        if (bytes_write <= length) {
            data = data + length;
            length = length - bytes_write;
            DBG("Remain length: %d\n", length);
        }
    }

    data = NULL;
    length = 0;

       // release write lock
    close(fd_write);
#else
    CURLcode res;

    res = curl_easy_download_epo();
    // DBG("curl_easy_download_epo res = %d\n", res);
    if (res != CURLE_OK) {
        epo_download_failed = 1;
        return -1;
    } else {
        DBG("download piece file%d done\n", (gps_epo_file_count+1));
        return 0;
    }
 #endif
}
static unsigned int mtk_gps_epo_get_piece_file_size()
{
    struct stat st;
    unsigned int fileSize;
    char gps_epo_data_file_name[60] = {0};

    strcat(gps_epo_data_file_name, "/data/misc/gps/");
    strcat(gps_epo_data_file_name, gps_epo_file_name);

    if (stat(gps_epo_data_file_name, &st) < 0) {
        ERR("Get file size error, return\n");
        return 0;
    }
    fileSize = st.st_size;
    DBG("EPO piece file size: %d\n", fileSize);
    return fileSize;
}
static int
mtk_gps_epo_piece_data_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond) {
    char szBuf[MTK_EPO_ONE_SV_SIZE];
    int pi2WeekNo;
    unsigned int pu4Tow;

    // if (fread(szBuf, 1, MTK_EPO_ONE_SV_SIZE, pFile) != MTK_EPO_ONE_SV_SIZE) {
    if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;

    TRC();
    DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);   // to get UTC second
    return 0;
}

static int
mtk_gps_epo_piece_data_end(int fd, unsigned int *u4GpsSecs, time_t* uSecond) {

    int fileSize = 0;
    char szBuf[MTK_EPO_ONE_SV_SIZE] = {0};
    int pi2WeekNo;
    unsigned int pu4Tow;

    if (-1 != fd) {

        fileSize = mtk_gps_epo_get_piece_file_size();
        if (fileSize < MTK_EPO_ONE_SV_SIZE) {
            ERR("Get file size is error\n");
            return -1;
        }
        if (lseek(fd, (fileSize - MTK_EPO_ONE_SV_SIZE), SEEK_SET) == -1)
            ERR("lseek error: %s\n", strerror(errno));

        if (read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
            ERR("read epo file end data faied\n");
            return -1;
        }

        *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);
        (*u4GpsSecs) += 21600;

        pi2WeekNo = (*u4GpsSecs) / 604800;
        pu4Tow = (*u4GpsSecs) % 604800;

        TRC();
        DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
        GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);
    }
    return 0;
}
int mtk_gps_epo_server_data_is_changed()
{
    long long uTime_end = 0;
    long long uTime_start = 0;
    int fd_end = -1;
    int fd_start = -1;
    char gps_epo_data_file_name_end[60] = {0};
    char gps_epo_data_file_name_start[60] = {0};
    time_t uSecond_start;
    time_t uSecond_end;
    unsigned int u4GpsSecs_start;
    unsigned int u4GpsSecs_end;
    int ret = 0;

    strcat(gps_epo_data_file_name_start, "/data/misc/gps/");
    strcat(gps_epo_data_file_name_start, gps_epo_file_name);

    fd_start = open(gps_epo_data_file_name_start, O_RDONLY);
    if (fd_start >= 0) {
        int res = 0;
        res = mtk_gps_epo_piece_data_start(fd_start, &u4GpsSecs_start, &uSecond_start);
        if (res == 0) {
            uTime_start = (long long)uSecond_start;
        }
       else {
            epo_download_failed = 1;
            ret = 1;
            ERR("Get start time failed\n");
        }
        close(fd_start);
    } else {
        ERR("Open start file failed\n");
    }
    if (gps_epo_file_count > 0) {
        gps_download_epo_file_name(gps_epo_file_count - 1);
        strcat(gps_epo_data_file_name_end, "/data/misc/gps/");
        strcat(gps_epo_data_file_name_end, gps_epo_file_name);
        // open file
        fd_end = open(gps_epo_data_file_name_end, O_RDONLY);
        if (fd_end >= 0) {
            int res = 0;
            res = mtk_gps_epo_piece_data_end(fd_end, &u4GpsSecs_end, &uSecond_end);
            if (res == 0) {
                uTime_end = (long long)uSecond_end;
            } else {
                epo_download_failed = 1;
                ERR("Get end time failed\n");
                ret = 1;
            }
            close(fd_end);
        } else {
            ERR("Open end file failed\n");
        }
    } else if (gps_epo_file_count == 0) {
        uTime_end = uTime_start;
    }

    // DBG("gps_epo_data_file_start =%s, end =%s\n", gps_epo_data_file_name_start, gps_epo_data_file_name_end);
    DBG("The end time of EPO file is %s, The start time of EPO file is %s\n",
        ctime(&uTime_end), ctime(&uTime_start));
    if ((uTime_start - uTime_end) >= (24*60*60)) {
        int i;
        DBG("The epo data is updated on the server!!!\n");
        for (i = gps_epo_file_count; 0 <= i; i--) {
            char gps_epo_piece_file_name[40] = {0};

            gps_download_epo_file_name(i);
            strcat(gps_epo_piece_file_name, "/data/misc/gps/");
            strcat(gps_epo_piece_file_name, gps_epo_file_name);
            unlink(gps_epo_piece_file_name);
        }
        unlink(EPO_FILE_HAL);
        gps_epo_file_count = 0;
        return 1;
    }
    return ret;
}

int mtk_gps_epo_file_update() {
    GpsState*  s = _gps_state;
    int ret;
    int res;
    int is_changed = 0;
    int count_temp = 10;
    int count_mini = 0;
    DBG("sta.status = %d\n", sta.status);

    // update framework downlaod data to EPOHAL.DAT
    ret = mtk_gps_epo_file_update_hal();
    if (ret < 0) {
        ERR("Update EPOHAL.DAT error\n");
        return -1;
    }
    is_changed = mtk_gps_epo_server_data_is_changed();
    if (is_changed == 1) {
        return -1;
    }
    gps_epo_file_count++;
    count_temp = gps_epo_download_days/3;
    count_mini = gps_epo_download_days%3;
    if (count_mini > 0) {
        count_temp++;
    }
    if (gps_epo_file_count < 10 && gps_epo_file_count < count_temp) {
        // gps_download_epo_file_name();
        // gps_download_epo(s);
        DBG("Download next epo file continue...\n");
        return 0;
    }
    else {
        int i = 0;
        DBG("download epo file completed!file count=%d, epo_download_failed=%d, epo_data_updated=%d\n",
            gps_epo_file_count, epo_download_failed, s->epo_data_updated);

        for (i = 0; i < count_temp; i++) {
            char gps_epo_data_file_name[60] = {0};

            gps_download_epo_file_name(i);
            strcat(gps_epo_data_file_name, "/data/misc/gps/");
            strcat(gps_epo_data_file_name, gps_epo_file_name);
            unlink(gps_epo_data_file_name);
        }
        s->epo_data_updated = 0;
        gps_epo_file_count = 0;
        if (epo_download_failed == 1) {
            epo_download_failed = 0;
        }
        if (started) {
            // send cmd to MNLD
            char cmd = HAL_CMD_STOP_UNKNOWN;

            DBG("GPS driver is running, update epo file via GPS driver\n");
            char buff[1024] = {0};
            int offset = 0;

            unlink(EPO_FILE);
            buff_put_int(MNL_CMD_UPDATE_EPO_FILE, buff, &offset);
            ret = mtk_daemon_send(mtk_gps.sock, MTK_HAL2MNLD, buff, sizeof(buff));

            if (-1 == ret) {
                ERR("Request update epo file fail\n");
            } else {
                DBG("Request update epo file successfully\n");
            }
        }
    }
    return ret;
}
#endif
static const GpsInterface  mtkGpsInterface = {
    sizeof(GpsInterface),
    mtk_gps_init,
    mtk_gps_start,
    mtk_gps_stop,
    mtk_gps_cleanup,
    mtk_gps_inject_time,
    mtk_gps_inject_location,
    mtk_gps_delete_aiding_data,
    mtk_gps_set_position_mode,
    mtk_gps_get_extension,
#ifdef GPS_AT_COMMAND
    mtk_gps_test_start,
    mtk_gps_test_stop,
    mtk_gps_test_inprogress,
#endif
};

const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
    DBG("gps__get_gps_interface HAL\n");
    /*send message(start, stop, restart, agps info) to MNLD*/
    mtk_gps.sock = socket(PF_LOCAL, SOCK_DGRAM, 0);    // for gps message
    g_agps_ctx.send_fd = mtk_gps.sock;                 // for agps message
    DBG("HAL<->MNLD sock_fd: %d\n", mtk_gps.sock);

    return &mtkGpsInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device) {
    DBG("open_gps HAL 1\n");
    struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
    if (dev != NULL) {
        memset(dev, 0, sizeof(*dev));

        dev->common.tag = HARDWARE_DEVICE_TAG;
        dev->common.version = 0;
        dev->common.module = (struct hw_module_t*)module;
        //   dev->common.close = (int (*)(struct hw_device_t*))close_lights;
        DBG("open_gps HAL 2\n");
        dev->get_gps_interface = gps__get_gps_interface;
        DBG("open_gps HAL 3\n");
        *device = (struct hw_device_t*)dev;
    } else {
        DBG("malloc failed dev = NULL!\n");
    }
    return 0;
}


static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};


struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "Hardware GPS Module",
    .author = "The MTK GPS Source Project",
    .methods = &gps_module_methods,
};
