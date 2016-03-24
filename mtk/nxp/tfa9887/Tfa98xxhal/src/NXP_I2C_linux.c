
/* implementation of the NXP_I2C API on Linux */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <inttypes.h>   //TODO fix/converge Types
#include <sys/stat.h>
#include <fcntl.h>
#include "../inc/NXP_I2C.h"
#include "../inc/lxScribo.h"
#include "lxScribo/Scribo.h"

#include <utils/Log.h>

#define LOG_TAG "NXPI2Clinux"

//#include "Tfa98xx.h"  // for device driver interface special file

//#define I2CVERBOSE

#ifndef __BOOL_DEFINED
typedef unsigned char bool;
#define true ((bool)(1==1))
#define false ((bool)(1==0))
#endif

static NXP_I2C_Error_t translate_error(uint32_t error);

typedef struct
{
    char msg[32];
} NXP_I2C_ErrorStr_t;


NXP_I2C_Error_t i2cExecuteRS(int NrOfWriteBytes,
                             const unsigned char *WriteData, int NrOfReadBytes, unsigned char *ReadData);

/* set to true to show the actually I2C message in stdout
 * set to false otherwise
 */
static int bDebug = false;
/* for debug logging, keep track of number of I2C messages and when sent */
static long cnt = 0;
static long start;
static char debug_buffer[5000];

static int bInit = false;

static int i2cTargetFd = -1;                     /* file descriptor for target device */
#ifdef I2CVERBOSE
int NXP_I2C_verbose = 1;
#else
int NXP_I2C_verbose = 0;
#endif
//#define VERBOSE(format, args...) if (NXP_I2C_verbose) ALOGD(format, ##args)
#define VERBOSE if (NXP_I2C_verbose)

#define TFA_I2CDEVICE		"/dev/nxpspk"

static NXP_I2C_Error_t init_I2C(void)
{
    ALOGD("init_I2C");
#ifdef TFA_CHECK_REV
    unsigned char wbuf[2], rbuf[3];
#endif
    VERBOSE ALOGD("%s\n", __FUNCTION__);

    /*  TODO properly define open device configs */
#if 1
    if (i2cTargetFd < 0)
    {
        ALOGD("lxScriboRegister %s", TFA_I2CDEVICE);
        i2cTargetFd = lxScriboRegister(TFA_I2CDEVICE);
    }
#else
    if (i2cTargetFd < 0)
    {
        ALOGD("using /dev/Scribo\n");
        i2cTargetFd = lxScriboRegister("/dev/Scribo");
        //  i2cTargetFd = lxScriboGetFd();
    }
#endif
    if (i2cTargetFd < 0)
    {
        fprintf(stderr, "!i2c device was not opened\n");
        return NXP_I2C_NoInterfaceFound;
    }

#ifdef TFA_CHECK_REV
    // check the contents of the ID regsiter
    wbuf[0] = TFA_I2CSLAVE << 1;
    wbuf[1] = 0x03; // ID register
    rbuf[0] = (TFA_I2CSLAVE << 1) + 1; // read
    i2cExecuteRS(sizeof(wbuf), wbuf, sizeof(rbuf), rbuf);
    if (rbuf[1] != (TFA_CHECK_REV >> 8) || rbuf[2] != (TFA_CHECK_REV & 0xff))
    {
        fprintf(stderr, "!wrong ID expected:0x%04x received:0x%04x, register 3 of slave 0x%02x\n",
                TFA_CHECK_REV ,  rbuf[1] << 8 | rbuf[2], TFA_I2CSLAVE);
        return NXP_I2C_NoInterfaceFound;
    }
#endif
    bInit = true;
    return NXP_I2C_Ok;
}

static const NXP_I2C_ErrorStr_t errorStr[NXP_I2C_ErrorMaxValue] =
{
    { "UnassignedErrorCode" },
    { "Ok" },
    { "NoAck" },
    { "SclStuckAtOne" },
    { "SdaStuckAtOne" },
    { "SclStuckAtZero" },
    { "SdaStuckAtZero" },
    { "TimeOut" },
    { "ArbLost" },
    { "NoInit" },
    { "Disabled" },
    { "UnsupportedValue" },
    { "UnsupportedType" },
    { "NoInterfaceFound" },
    { "NoPortnumber" }
};

#define DUMMIES
#ifdef DUMMIES
/*
 * dummies TODO fix dummies
 */
int GetTickCount(void)
{
    return 0;
}

void  i2cDebugStr(char *GetString)
{
    GetString = "TBD" ;
}
void  i2cDebug(int err)
{
    ALOGD("%s %d\n", __FUNCTION__, err);
}
static void hexdump(int num_write_bytes, const unsigned char *data)
{
    int i;

    for (i = 0; i < num_write_bytes; i++)
    {
        ALOGD("0x%02x ", data[i]);
    }

}
NXP_I2C_Error_t i2cExecute(int num_write_bytes, unsigned char *data)
{
    uint32_t error = eNone;
    int ret;
    //ALOGD("%s   W %d bytes:\t", __FUNCTION__, num_write_bytes);

    //hexdump(num_write_bytes, data);

    ret =  (*lxWrite)(i2cTargetFd, num_write_bytes, data, &error);

    /*
    if (0 == (*lxWrite)(i2cTargetFd, num_write_bytes, data, &error))
    {
        ALOGD(" NoAck\n");
        return translate_error(error);
    }
    */

    return translate_error(error);
}

NXP_I2C_Error_t i2cExecuteRS(int NrOfWriteBytes,
                             const unsigned char *WriteData, int NrOfReadBytes, unsigned char *ReadData)
{
    uint32_t error = eNone;
    int ret;
    //ALOGD("%s W %d bytes:\t", __FUNCTION__, NrOfWriteBytes);
    //hexdump(NrOfWriteBytes, WriteData);

    ret = (*lxWriteRead)(i2cTargetFd,  NrOfWriteBytes, WriteData ,
                         NrOfReadBytes, ReadData, &error);
    if (ret > 0)
    {
        //hexdump(NrOfReadBytes, ReadData);
    }
    else
    {
        //hexdump(NrOfReadBytes, ReadData);
        return translate_error(error);
    }
    return translate_error(error);
}
#endif

#if 0
i2cErrorTypes i2cError(void)
{
    return i2cNoError;
}
#endif
static NXP_I2C_Error_t translate_error(uint32_t error)
{
    NXP_I2C_Error_t retval;

    switch (error)
    {
        case eNone:
            retval = NXP_I2C_Ok;
            break;
        case eI2C_SLA_NACK:
            retval = NXP_I2C_NoAck;
            break;
#if 0
        case i2cSclStuckAtOne:
            retval = NXP_I2C_SclStuckAtOne;
            break;
        case i2cSdaStuckAtOne:
            retval = NXP_I2C_SdaStuckAtOne;
            break;
        case i2cSclStuckAtZero:
            retval = NXP_I2C_SclStuckAtZero;
            break;
        case i2cSdaStuckAtZero:
            retval = NXP_I2C_SdaStuckAtZero;
            break;
        case i2cTimeOut:
            retval = NXP_I2C_TimeOut;
            break;
        case i2cArbLost:
            retval = NXP_I2C_ArbLost;
            break;
        case i2cNoInit:
            retval = NXP_I2C_NoInit;
            break;
        case i2cDisabled:
            retval = NXP_I2C_Disabled;
            break;
        case i2cUnsupportedValue:
            retval = NXP_I2C_UnsupportedValue;
            break;
        case i2cUnsupportedType:
            retval = NXP_I2C_UnsupportedType;
            break;
        case i2cNoInterfaceFound:
            retval = NXP_I2C_NoInterfaceFound;
            break;
        case i2cNoPortnumber:
            retval = NXP_I2C_NoPortnumber;
            break;
#endif
        default:
            /* unexpected error */
            ALOGD("Got unexpected error 0x%lx\n", error);
            assert(0);
            retval = NXP_I2C_UnassignedErrorCode;
    }

    if (retval != NXP_I2C_Ok)
    {
        ALOGD("I2C error %ld (%s)\n", error, (char *)&errorStr[retval].msg);
    }
    return retval;
}

static NXP_I2C_Error_t init_if_firsttime(void)
{
    NXP_I2C_Error_t retval = NXP_I2C_Ok;

    if (!bInit)
    {
        retval = init_I2C();
    }
    return retval;
}

static void debugInfo(void)
{
    cnt++;
    if (bDebug)
    {
        long t = GetTickCount();
        debug_buffer[0] = '\0';
        i2cDebugStr(debug_buffer);
        ALOGD("%d (%d) %s\n", (int)cnt, (int)(t - start), (char *)debug_buffer);
        fflush(stdout);
    }
}

NXP_I2C_Error_t NXP_I2C_Write(int num_write_bytes,
                              unsigned char data[])
{
    NXP_I2C_Error_t retval;
    //ALOGD("NXP_I2C_Write");
    if (num_write_bytes > NXP_I2C_MAX_SIZE)
    {
        fprintf(stderr, "%s: too many bytes: %d\n", __FUNCTION__, num_write_bytes);
        return NXP_I2C_UnsupportedValue;
    }

    retval = init_if_firsttime();
    if (NXP_I2C_Ok == retval)
    {
        retval = i2cExecute(num_write_bytes, data);
        debugInfo();
    }
    //ALOGD("NXP_I2C_Write retval = %d",retval);
    return retval;
}

NXP_I2C_Error_t NXP_I2C_WriteRead(int num_write_bytes,
                                  unsigned char write_data[],
                                  int num_read_bytes,
                                  unsigned char read_data[])
{
    NXP_I2C_Error_t retval;
    //ALOGD("NXP_I2C_WriteRead num_write_bytes = %d num_read_bytes = %d", num_write_bytes, num_read_bytes);
    if (num_write_bytes > NXP_I2C_MAX_SIZE)
    {
        ALOGD("%s: too many bytes to write: %d\n", __FUNCTION__, num_write_bytes);
        return NXP_I2C_UnsupportedValue;
    }
    if (num_read_bytes > NXP_I2C_MAX_SIZE)
    {
        ALOGD("%s: too many bytes to read: %d\n", __FUNCTION__, num_read_bytes);
        return NXP_I2C_UnsupportedValue;
    }

    retval = init_if_firsttime();
    if (NXP_I2C_Ok == retval)
    {
        retval = i2cExecuteRS(num_write_bytes, write_data, num_read_bytes, read_data);
        //ALOGD("num_read_bytes retval = %d", retval);
        //debugInfo();
    }
    return retval;
}

NXP_I2C_Error_t NXP_I2C_EnableLogging(int bEnable)
{
    NXP_I2C_Error_t retval;

    retval = init_if_firsttime();
    if (NXP_I2C_Ok == retval)
    {
        bDebug = bEnable;
        i2cDebug(bDebug);
    }
    return retval;
}
