#define MTK_LOG_ENABLE 1

#ifdef WIN32

#include "stdio.h"
#include "time.h"
#include "strobe_drv_2led.h"
#include "flashlight_drv_2led.h"
#include "kd_flashlight.h"
#include "windows.h"
#include "win_test_cpp.h"
#include "strobe_global_driver.h"

#else
#define LOG_TAG "strobe_global_driver.cpp"
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>

//
#include "ae_feature.h"
#include "../inc/strobe_drv.h"
#include "flashlight_drv.h"
#include "kd_flashlight.h"

#include <mtkcam/Log.h>

#include "camera_custom_nvram.h"
#include "camera_custom_types.h"

#include "camera_custom_AEPlinetable.h"
#include "camera_custom_nvram.h"

#include <cutils/log.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>
#include "strobe_global_driver.h"
#endif

#ifdef WIN32
    #define logI(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n");}
    #define logW(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n");}
    #define logE(fmt, ...)    {printf("merror: line=%d, ", __LINE__); printf(fmt, __VA_ARGS__); printf("\n");}
#else
    #define logI(fmt, arg...)  CAM_LOGD( fmt, ##arg)
    #define logW(fmt, arg...)  CAM_LOGD("warning: line=%d, " fmt, __LINE__, ##arg)
    #define logE(fmt, arg...)  CAM_LOGD("error: line=%d, " fmt, __LINE__, ##arg)
#endif


#define STROBE_DEV_NAME    "/dev/kd_camera_flashlight"

StrobeGlobalDriver* StrobeGlobalDriver::getInstance()
{
    static StrobeGlobalDriver gobalDriver;
    return &gobalDriver;
}

StrobeGlobalDriver::StrobeGlobalDriver()
: mStrobeHandle(-1),
mUsers(0)
{
}
int StrobeGlobalDriver::sendCommandRet(int cmd, int sensorDev, int strobeId, int* arg)
{
    Mutex::Autolock lock(mLock);
    return sendCommandRet_nolock(cmd, sensorDev, strobeId, arg);
}
int StrobeGlobalDriver::sendCommandRet_nolock(int cmd, int sensorDev, int strobeId, int* arg)
{
    logI("sendCommandRet_nolock()");
    if (mStrobeHandle <= 0)
    {
        logE("sendCommand() mStrobeHandle <= 0 ~");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    kdStrobeDrvArg stbArg;
    stbArg.sensorDev=sensorDev;
    stbArg.strobeId = strobeId;
    stbArg.arg=0;
    int ret;
    ret = ioctl(mStrobeHandle, cmd, &stbArg);
    *arg = stbArg.arg;
    return ret;
}


int StrobeGlobalDriver::sendCommand(int cmd, int sensorDev, int strobeId, int arg)
{
    Mutex::Autolock lock(mLock);
    logI("sendCommand, cmd=%d, sensorDev=%d, strobeId=%d, arg=%d", cmd, sensorDev, strobeId, arg);
    return sendCommand_nolock(cmd, sensorDev, strobeId, arg);
}

int StrobeGlobalDriver::sendCommand_nolock(int cmd, int sensorDev, int strobeId, int arg)
{
    logI("sendCommand_nolock()");
    if (mStrobeHandle <= 0)
    {
        logE("sendCommand() mStrobeHandle <= 0 ~");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    kdStrobeDrvArg stbArg;
    stbArg.sensorDev=sensorDev;
    stbArg.strobeId = strobeId;
    stbArg.arg=arg;
    return ioctl(mStrobeHandle, cmd, &stbArg);
}

int StrobeGlobalDriver::openkd_nolock()
{
    if(mUsers==0)
    {
        mStrobeHandle = open(STROBE_DEV_NAME, O_RDWR);
        logI("open flash driver kd=%d", mStrobeHandle);
    }

    if (mStrobeHandle <= 0)
    {
        logE("error openkd_nolock %s: %s", STROBE_DEV_NAME, strerror(errno));
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    android_atomic_inc(&mUsers);
    return 0;
}
int StrobeGlobalDriver::closekd_nolock()
{
    if(mUsers<=0)
    {
        logW("closekd_nolock user<=0");
        return 0;
    }
    if(mUsers == 1)
    {
        if (mStrobeHandle > 0)
        {
            logI("close flash driver kd=%d", mStrobeHandle);
            close(mStrobeHandle);
        }
        mStrobeHandle = -1;
    }
    android_atomic_dec(&mUsers);
    return 0;
}
int StrobeGlobalDriver::openkd()
{
    Mutex::Autolock lock(mLock);
    openkd_nolock();
    return 0;
}
int StrobeGlobalDriver::closekd()
{
    Mutex::Autolock lock(mLock);
    closekd_nolock();
    return 0;
}
int StrobeGlobalDriver::init(int sensorDev, int strobeId)
{
    logI("init dev=%d id=%d", sensorDev, strobeId);
    Mutex::Autolock lock(mLock);
    openkd_nolock();
    int err;
    err = sendCommand_nolock(FLASHLIGHTIOC_X_SET_DRIVER, sensorDev, strobeId, 0);
    if(err!=0)
    {
        logE("FLASHLIGHTIOC_X_SET_DRIVER kd_err=%d", err);
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    return 0;
}
int StrobeGlobalDriver::uninit(int sensorDev, int strobeId)
{
    logI("uninit dev=%d id=%d", sensorDev, strobeId);
    Mutex::Autolock lock(mLock);
    closekd_nolock();
    return 0;
}
