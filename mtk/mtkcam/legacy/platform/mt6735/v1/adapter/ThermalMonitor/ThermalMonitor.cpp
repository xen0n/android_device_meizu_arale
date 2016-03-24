/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/TM"
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <ThermalMonitor_imp.h>
#define MTK_LOG_ENABLE 1
//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//-----------------------------------------------------------------------------
sp<ThermalMonitor>
ThermalMonitor::
createInstance(
    IHal3A::E_VER   aaaVersion,
    MINT32          sensorIdx)
{
    return  new ThermalMonitorImp(aaaVersion, sensorIdx);
}
//-----------------------------------------------------------------------------
ThermalMonitorImp::
ThermalMonitorImp(
    IHal3A::E_VER   aaaVersion,
    MINT32          sensorIdx)
    : mState(STATE_STOP)
    , mFpThermal(NULL)
    , mOrigFps(0)
    , mCurFps(0)
    , mpHal3a(NULL)
{
    FUNC_START;
    //
    mpHal3a = IHal3A::createInstance(
                        aaaVersion,
                        sensorIdx,
                        LOG_TAG);
    //
    run();
    //
    FUNC_END;
}
//-----------------------------------------------------------------------------
ThermalMonitorImp::
~ThermalMonitorImp()
{
    FUNC_START;
    //
    if(mpHal3a)
    {
        mpHal3a->destroyInstance(LOG_TAG);
        mpHal3a = NULL;
    }
    //
    FUNC_END;
}
//-----------------------------------------------------------------------------
MBOOL
ThermalMonitorImp::
setFrameRate(MUINT32 fps)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD("FPS(%d)",fps);
    mOrigFps = fps;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ThermalMonitorImp::
start()
{
    Mutex::Autolock lock(mLock);
    //
    FUNC_START;
    //
    mState = STATE_START;
    mCond.broadcast();
    //
    FUNC_END;
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ThermalMonitorImp::
stop()
{
    Mutex::Autolock lock(mLock);
    //
    FUNC_START;
    //
    mState = STATE_STOP;
    //
    FUNC_END;
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ThermalMonitorImp::
exit()
{
    FUNC_START;
    //
    requestExit();
    //
    {
        Mutex::Autolock lock(mLock);
        mState = STATE_EXIT;
        mCond.broadcast();
    }
    //
    MY_LOGD("join() E");
    status_t status = join();
    if( OK != status )
    {
        MY_LOGW("Not to wait , status[%s(%d)]", ::strerror(-status), -status);
    }
    MY_LOGD("join() X");
    //
    FUNC_END;
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ThermalMonitorImp::
isRunning()
{
    Mutex::Autolock lock(mLock);
    //
    if(mState == STATE_START)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
//-----------------------------------------------------------------------------
bool
ThermalMonitorImp::
threadLoop()
{
    while(1)
    {
        MY_LOGD("State(%d)",mState);
        if(mState == STATE_EXIT)
        {
            break;
        }
        else
        if(mState == STATE_STOP)
        {
            if(mOrigFps != 0)
            {
                NS3A::Param_T cam3aParam;
                if(mpHal3a->getParams(cam3aParam))
                {
                    cam3aParam.i4MinFps = mOrigFps*1000;
                    cam3aParam.i4MaxFps = mOrigFps*1000;
                    if(!mpHal3a->setParams(cam3aParam))
                    {
                        MY_LOGE("setParams fail");
                    }
                }
                else
                {
                    MY_LOGE("getParams fail");
                }
                mOrigFps = 0;
            }
            //
            {
                Mutex::Autolock lock(mLock);
                if(mState == STATE_START)
                {
                    MY_LOGD("Start");
                }
                else
                if(mState == STATE_EXIT)
                {
                    MY_LOGD("Exit");
                    break;
                }
                else
                {
                    MY_LOGD("Wait lock");
                    mCond.wait(mLock);
                }
            }
        }
        else
        if(mState == STATE_START)
        {
            usleep(SLEEP_TIME_US);
            //
            char thermalStatus;
            mFpThermal = fopen(FILENAME_THERMAL, "rb");
            if(1 != (MINT32)fread(&thermalStatus, sizeof(signed char), 1, mFpThermal))
            {
                MY_LOGE("read %s file fail", FILENAME_THERMAL);
            }
            fclose(mFpThermal);
            //
            MUINT32 wantFps = 0;
            //
            if(thermalStatus == '0')
            {
                wantFps = mOrigFps;
            }
            else
            //if(thermalStatus == '1')
            {
                wantFps = mOrigFps*ADJUST_RATIO;
            }
            //
            MY_LOGD("thermalStatus(%c),FPS(%d,%d,%d)",
                    thermalStatus,
                    mOrigFps,
                    mCurFps,
                    wantFps);
            //
            if(mCurFps != wantFps)
            {
                NS3A::Param_T cam3aParam;
                if(mpHal3a->getParams(cam3aParam))
                {
                    cam3aParam.i4MinFps = wantFps*1000;
                    cam3aParam.i4MaxFps = wantFps*1000;
                    if(mpHal3a->setParams(cam3aParam))
                    {
                        mCurFps = wantFps;
                    }
                    else
                    {
                        MY_LOGE("setParams fail");
                    }
                }
                else
                {
                    MY_LOGE("getParams fail");
                }
            }
        }
    }
    //
    return true;
}
//-----------------------------------------------------------------------------

