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
#define LOG_TAG "ResMgrDrv"
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <utils/threads.h>
//
#include <list>
#include <queue>
using namespace std;
//
#include "bandwidth_control.h"
#include <mtkcam/common.h>
#define MTK_LOG_ENABLE 1
//
#include "res_mgr_drv_imp.h"
#ifdef USING_MTK_PERFSERVICE
#include "PerfServiceNative.h"
#include <dlfcn.h>
static MBOOL   initPerf();
static MBOOL   uninitPerf();
static MBOOL   setPerf(ResMgrDrv::SCEN_SW_ENUM scenario);
#endif
//-----------------------------------------------------------------------------
static Mutex               resMgrDrvImplock;
static list<ResMgrDrvImp*> vpResMgrDrvImpSingleton;
//-----------------------------------------------------------------------------
ResMgrDrvImp::
ResMgrDrvImp(MUINT32 sensorIdx)
{
    //MY_LOGD("");
    mUser = 0;
    mSensorIdx = sensorIdx;
}
//----------------------------------------------------------------------------
ResMgrDrvImp::
~ResMgrDrvImp()
{
    //MY_LOGD("");
}
//-----------------------------------------------------------------------------
ResMgrDrv*
ResMgrDrv::
createInstance(MUINT32 sensorIdx)
{
    return ResMgrDrvImp::getInstance(sensorIdx);
}
//-----------------------------------------------------------------------------
ResMgrDrv*
ResMgrDrvImp::
getInstance(MUINT32 sensorIdx)
{
    Mutex::Autolock _l(resMgrDrvImplock);
    //
#ifdef USING_MTK_PERFSERVICE
    if( vpResMgrDrvImpSingleton.size() == 0 )
    {
        initPerf();
    }
#endif
    //
    ResMgrDrvImp* pResMgrDrvImp = NULL;
    list<ResMgrDrvImp*>::const_iterator iter;
    for(iter = vpResMgrDrvImpSingleton.begin(); iter != vpResMgrDrvImpSingleton.end(); iter++)
    {
        if((*iter)->getSensorIdx() == sensorIdx)
            pResMgrDrvImp = (*iter);
    }
    //
    if(pResMgrDrvImp == NULL)
    {
        //create new
        pResMgrDrvImp = new ResMgrDrvImp(sensorIdx);
        vpResMgrDrvImpSingleton.push_back(pResMgrDrvImp);
    }
    //
    pResMgrDrvImp->mUser++;
    MY_LOGD("this(0x%x), userCnt(%d)",
            pResMgrDrvImp,
            pResMgrDrvImp->mUser);
    //
    return pResMgrDrvImp;
}
//----------------------------------------------------------------------------
MVOID
ResMgrDrvImp::
destroyInstance()
{
    Mutex::Autolock _l(resMgrDrvImplock);
    //
    mUser--;
    MY_LOGD("this(0x%x), userCnt(%d)",
            this,
            mUser);
    //
    if(mUser == 0)
    {
        list<ResMgrDrvImp*>::iterator iter;
        for(iter = vpResMgrDrvImpSingleton.begin(); iter != vpResMgrDrvImpSingleton.end(); iter++)
        {
            if((*iter) == this)
            {
                vpResMgrDrvImpSingleton.erase(iter);
                break;
            }
        }
        delete this;
    }
    //
#ifdef USING_MTK_PERFSERVICE
    // no need to uninitPerf since this is mediaserver
    //if( vpResMgrDrvImpSingleton.size() == 0 )
    //{
    //    uninitPerf();
    //}
#endif
}
//----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
init()
{
    Mutex::Autolock lock(mLock);
    //
    MBOOL Result = MTRUE;
    //
    //
    EXIT:
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
uninit()
{
    Mutex::Autolock lock(mLock);
    //
    //
    EXIT:
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
getMode(MODE_STRUCT* pMode)
{
    Mutex::Autolock lock(mLock);
    //
    MBOOL Result = MTRUE;
    //
    MY_LOGD("scenSw(%d),dev(%d)",
            mMode.scenSw,
            mMode.dev);
    //
    if(pMode)
    {
        pMode->scenSw = mMode.scenSw;
        pMode->dev = mMode.dev;
    }
    //
    EXIT:
    //MY_LOGD("Result(%d)",Result);
    return Result;

}
//-----------------------------------------------------------------------------
MBOOL
ResMgrDrvImp::
setMode(MODE_STRUCT* pMode)
{
    Mutex::Autolock lock(mLock);
    //
    MBOOL Result = MTRUE;
    //
    MY_LOGD("scenSw(%d),dev(%d)",
            pMode->scenSw,
            pMode->dev);
    //
    if( mMode.scenSw == (pMode->scenSw) &&
        mMode.dev == (pMode->dev))
    {
        MY_LOGD("OK:Same(%d/%d,%d/%d)",
                mMode.scenSw,
                mMode.dev,
                pMode->scenSw,
                pMode->dev);
        goto EXIT;
    }
    //
#ifdef USING_MTK_PERFSERVICE
    setPerf(pMode->scenSw);
#endif
    //
    if(Result)
    {
        BWC BwcIns;
        //
        if(pMode->dev == DEV_VT)
        {
            if( pMode->scenSw == SCEN_SW_NONE ||
                pMode->scenSw == SCEN_SW_CAM_IDLE)
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,false);
            }
            else
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,true);
            }
        }
        else
        {
            switch(mMode.scenSw)
            {
                case SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case SCEN_SW_VIDEO_PRV:
                case SCEN_SW_VIDEO_REC:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,false);
                    break;
                }
                case SCEN_SW_VIDEO_REC_HS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_SLOWMOTION,false);
                    break;
                }
                case SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_SNAPSHOT,false);
                    break;
                }
                case SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,false);
                    break;
                }
                case SCEN_SW_CAM_CSHOT:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,false);
                    break;
                }
                case SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,false);
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
            //
            switch(pMode->scenSw)
            {
                case SCEN_SW_NONE:
                case SCEN_SW_CAM_IDLE:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,true);
                    break;
                }
                case SCEN_SW_VIDEO_PRV:
                case SCEN_SW_VIDEO_REC:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,true);
                    break;
                }
                case SCEN_SW_VIDEO_REC_HS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_SLOWMOTION,true);
                    break;
                }
                case SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_SNAPSHOT,true);
                    break;
                }
                case SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,true);
                    break;
                }
                case SCEN_SW_CAM_CSHOT:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,true);
                    break;
                }
                case SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,true);
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
        }
    }
    //
    mMode.scenSw = pMode->scenSw;
    mMode.dev = pMode->dev;
    //
    EXIT:
    //MY_LOGD("Result(%d)",Result);
    return Result;

}
#ifdef USING_MTK_PERFSERVICE
#define PERF_LIB_FULLNAME "libperfservicenative.so"
#define STR_FUNC_REG      "PerfServiceNative_userRegBigLittle"
#define STR_FUNC_ENABLE   "PerfServiceNative_userEnable"
#define STR_FUNC_DISABLE  "PerfServiceNative_userDisable"
#define STR_FUNC_UNREG    "PerfServiceNative_userUnreg"
typedef int (*funcPerfRegBL)(int, int, int, int);
typedef void (*funcEnable)(int);
typedef void (*funcDisable)(int);
typedef int (*funcPerfUnreg)(int);
static Mutex               gPerfLock;
static void*               gPerfLib = NULL;
static int                 gPerfHandle = -1;
static bool                gPerfEnabled = false;
//-----------------------------------------------------------------------------
MBOOL
initPerf()
{
    MY_LOGD("");
    Mutex::Autolock _l(gPerfLock);
    if( !gPerfLib )
    {
        gPerfLib = ::dlopen(PERF_LIB_FULLNAME, RTLD_NOW);
        if  ( ! gPerfLib )
        {
            char const *err_str = ::dlerror();
            MY_LOGE("dlopen: %s error=%s", PERF_LIB_FULLNAME, (err_str ? err_str : "unknown"));
            goto lbExit;
        }
    }

    if( gPerfHandle == - 1 )
    {
        funcPerfRegBL pReg = NULL;
        // register scenario
        pReg = reinterpret_cast<funcPerfRegBL>(dlsym(gPerfLib, STR_FUNC_REG));
        if( pReg == NULL )
        {
            MY_LOGW("cannot get %s", STR_FUNC_REG);
            goto lbExit;
        }
        //
        gPerfHandle = pReg(0, 0, 3, 0); // register 3 little core scenario
        if( gPerfHandle == -1 )
        {
            MY_LOGW("register scenario failed");
            goto lbExit;
        }
    }
    //
lbExit:
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
uninitPerf()
{
    MY_LOGD("");
    Mutex::Autolock _l(gPerfLock);
    //
    if( gPerfEnabled )
    {
        MY_LOGD("force disable PerfService");
        funcDisable pDisable = reinterpret_cast<funcDisable>(dlsym(gPerfLib, STR_FUNC_DISABLE));
        if( pDisable == NULL )
        {
            MY_LOGW("cannot get %s", STR_FUNC_DISABLE);
        }
        else
        {
            pDisable(gPerfHandle);
            gPerfEnabled = false;
        }
    }
    //
    if( gPerfHandle != -1 )
    {
        // register scenario
        funcPerfUnreg pUnreg = NULL;
        pUnreg = reinterpret_cast<funcPerfUnreg>(dlsym(gPerfLib, STR_FUNC_UNREG));
        if( pUnreg == NULL )
        {
            MY_LOGW("cannot get %s", STR_FUNC_UNREG);
        }
        else
        {
            pUnreg(gPerfHandle);
            gPerfHandle = -1;
        }
    }
    //
    if( gPerfLib )
    {
        ::dlclose(gPerfLib);
        gPerfLib = NULL;
    }
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
setPerf(ResMgrDrv::SCEN_SW_ENUM scenario)
{
    Mutex::Autolock _l(gPerfLock);
    if( !gPerfLib )
    {
        MY_LOGD("perf not opened");
        return MTRUE;
    }
    //
    switch(scenario)
    {
        // normal/zsd preview enable perf
        case ResMgrDrv::SCEN_SW_CAM_PRV:
        case ResMgrDrv::SCEN_SW_ZSD:
            {
                if( gPerfEnabled )
                {
                    break;
                }
                if( gPerfHandle == -1 )
                {
                    MY_LOGW("handle == -1");
                    break;
                }
                //
                MY_LOGD("enable PerfService");
                funcEnable pEnable = reinterpret_cast<funcEnable>(dlsym(gPerfLib, STR_FUNC_ENABLE));
                if( pEnable == NULL )
                {
                    MY_LOGW("cannot get %s", STR_FUNC_ENABLE);
                    break;
                }
                pEnable(gPerfHandle);
                gPerfEnabled = true;
            }
            break;
        // stopPreview disable perf
        case ResMgrDrv::SCEN_SW_CAM_IDLE:
            {
                if( !gPerfEnabled )
                {
                    break;
                }
                //
                MY_LOGD("disable PerfService");
                funcDisable pDisable = reinterpret_cast<funcDisable>(dlsym(gPerfLib, STR_FUNC_DISABLE));
                if( pDisable == NULL )
                {
                    MY_LOGW("cannot get %s", STR_FUNC_DISABLE);
                    break;
                }
                pDisable(gPerfHandle);
                gPerfEnabled = false;
            }
            break;
        default:
            break;
    }
    return MTRUE;
}
#endif
//-----------------------------------------------------------------------------
MUINT32
ResMgrDrvImp::
getSensorIdx()
{
    return mSensorIdx;
}
//-----------------------------------------------------------------------------

