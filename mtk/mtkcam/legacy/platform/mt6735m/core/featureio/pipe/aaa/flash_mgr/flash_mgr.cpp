#ifdef WIN32
#include "stdafx.h"
#include "FlashSim.h"
#include "sim_MTKAECommon.h"
#include "sim_MTKAE.h"
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#else
#define LOG_TAG "flash_mgr.cpp"

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <camera_custom_AEPlinetable.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
//#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <aaa_sensor_mgr.h>
#include <flash_param.h>
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#include "flash_tuning_custom.h"
#include "strobe_drv.h"
#include <time.h>
#include <kd_camera_feature.h>
#include "dbg_flash_param.h"
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <nvram_drv.h>
//#include <nvram_drv_mgr.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>
#include "nvbuf_util.h"
#include "flash_mgr_m.h"
#include <algorithm>
#include <vector>
#include <deque>
#endif

using namespace NS3A;
using namespace NSIspTuning;

#define logI(fmt, arg...) ALOGD(fmt, ##arg)

enum
{
    e_SensorMain=1,
    e_SensorSub=DUAL_CAMERA_SUB_SENSOR,
    e_SensorMain2=DUAL_CAMERA_MAIN_2_SENSOR,
};

#define def_Imp(function) \
int FlashMgr::function(int senorDev)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorMain)    \
        errMain = FlashMgrM::getInstance(e_SensorMain)->function();    \
    if (senorDev & e_SensorSub)  \
        errSub = FlashMgrM::getInstance(e_SensorSub)->function();  \
    if (senorDev & e_SensorMain2)    \
        errMain2 = FlashMgrM::getInstance(e_SensorMain2)->function();  \
    return errOp(errMain, errSub, errMain2);    \
}


#define def_Imp1(function) \
int FlashMgr::function(int senorDev, int v1)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorMain)    \
        errMain = FlashMgrM::getInstance(e_SensorMain)->function(v1);    \
    if (senorDev & e_SensorSub)  \
        errSub = FlashMgrM::getInstance(e_SensorSub)->function(v1);  \
    if (senorDev & e_SensorMain2)    \
        errMain2 = FlashMgrM::getInstance(e_SensorMain2)->function(v1);  \
    return errOp(errMain, errSub, errMain2);    \
}


#define def_ImpTypeGet(function) \
int FlashMgr::function(int senorDev)  \
{   \
    if (senorDev & e_SensorMain)    \
        return FlashMgrM::getInstance(e_SensorMain)->function();    \
    else if (senorDev & e_SensorSub)  \
        return FlashMgrM::getInstance(e_SensorSub)->function();  \
    else \
        return FlashMgrM::getInstance(e_SensorMain2)->function();  \
}


#define def_ImpBody1(function,senorDev,p1) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorMain)    \
        errMain = FlashMgrM::getInstance(e_SensorMain)->function(p1);    \
    if (senorDev & e_SensorSub)  \
        errSub = FlashMgrM::getInstance(e_SensorSub)->function(p1);  \
    if (senorDev & e_SensorMain2)    \
        errMain2 = FlashMgrM::getInstance(e_SensorMain2)->function(p1);  \
    return errOp(errMain, errSub, errMain2);    \


#define def_ImpBody2(function,senorDev,p1,p2) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorMain)    \
        errMain = FlashMgrM::getInstance(e_SensorMain)->function(p1,p2);    \
    if (senorDev & e_SensorSub)  \
        errSub = FlashMgrM::getInstance(e_SensorSub)->function(p1,p2);  \
    if (senorDev & e_SensorMain2)    \
        errMain2 = FlashMgrM::getInstance(e_SensorMain2)->function(p1,p2);  \
    return errOp(errMain, errSub, errMain2);    \

inline int errOp(int e1, int e2, int e3)
{
    if(e1!=0)
        return 0;
    if(e2!=0)
        return 0;
    if(e3!=0)
        return 0;
    return 1;
}

//=======================================================
//  implementation
//=======================================================
FlashMgr::FlashMgr()
{
    mChildMain=0;
    mChildSub=0;
    mChildMain2=0;
}

int FlashMgr::setChildMain(void* child)
{
    mChildMain = child;
    return 0;
}
int FlashMgr::setChildSub(void* child)
{
    mChildSub = child;
    return 0;
}
int FlashMgr::setChildMain2(void* child)
{
    mChildMain2 = child;
    return 0;
}

FlashMgr& FlashMgr::getInstance()
{
    static FlashMgr obj;
    return obj;
}

int FlashMgr::getFlashSpMode()
{
    return FlashMgrM::getFlashSpMode();
}

int FlashMgr::setTorchLevel(int level)
{
    Mutex::Autolock lock(mLock);
    //logI("setTorchLevel %d %d %d %d", level, (int)mChildMain, (int)mChildSub, (int)mChildMain2);
    mTorchLevel=level;
    if(mChildMain!=0)
        ((FlashMgrM*)mChildMain)->setTorchLevel(level);
    if(mChildSub!=0)
        ((FlashMgrM*)mChildSub)->setTorchLevel(level);
    if(mChildMain2!=0)
        ((FlashMgrM*)mChildMain2)->setTorchLevel(level);
    return 0;
}
int FlashMgr::getTorchLevel(int& level)
{
    Mutex::Autolock lock(mLock);
    level = mTorchLevel;
    return 0;
}

//============================
//cct
def_Imp(cctWriteNvram)

int FlashMgr::cctSetNvdataMeta(int senorDev, void* in, int inSize)
{
    def_ImpBody2(cctSetNvdataMeta, senorDev, in, inSize);
}


int FlashMgr::cctReadNvramToPcMeta(int senorDev, void* out, MUINT32* realOutSize)
{
    def_ImpBody2(cctReadNvramToPcMeta, senorDev, out, realOutSize);
}

int FlashMgr::cctFlashLightTest(int senorDev, void* in)
{
    def_ImpBody1(cctFlashLightTest, senorDev, in);
}

def_Imp1(cctFlashEnable)

int FlashMgr::cctGetFlashInfo(int senorDev, int* isOn)
{
    def_ImpBody1(cctGetFlashInfo, senorDev, isOn);
}

def_Imp(cctSetSpModeCalibration)
def_Imp(cctSetSpModeQuickCalibration)
def_Imp(cctSetSpModeQuickCalibration2)
def_Imp(cctSetSpModeNormal)


int FlashMgr::cctGetQuickCalibrationResult(int senorDev)
{
    int errMain=0;
    int errSub=0;
    int errMain2=0;
    if (senorDev & e_SensorMain)
        errMain = FlashMgrM::getInstance(e_SensorMain)->cctGetQuickCalibrationResult();
    if (senorDev & e_SensorSub)
        errSub = FlashMgrM::getInstance(e_SensorSub)->cctGetQuickCalibrationResult();
    if (senorDev & e_SensorMain2)
        errMain2 = FlashMgrM::getInstance(e_SensorMain2)->cctGetQuickCalibrationResult();

    return errMain|errSub|errMain2;

}


//============================
def_Imp1(setFlashMode)
def_Imp1(init)
def_Imp(uninit)
def_Imp(create)
def_Imp(destroy)
def_Imp(turnOffFlashDevice)
def_Imp(cameraPreviewStart)
def_Imp(cameraPreviewEnd)
def_Imp(videoPreviewStart)
def_Imp(capCheckAndFireFlash_End)

def_Imp(cctCaliFixAwb2)



def_Imp(videoRecordingStart)
def_Imp(videoPreviewEnd)
def_Imp(videoRecordingEnd)
def_Imp(endPrecapture)
def_Imp(capCheckAndFireFlash_Start)

def_Imp(notifyAfEnter)
def_Imp(notifyAfExit)
def_Imp(setCapPara)
def_Imp(setPfParaToAe)



def_Imp1(setMultiCapture)




int FlashMgr::hasFlashHw(int senorDev, int& hasHw)
{
    def_ImpBody1(hasFlashHw, senorDev, hasHw);
}



//===================================================
// one para
//===================================================
def_Imp1(setAFLampOnOff)
//def_Imp1(setFlashMode)
def_Imp1(setCamMode)
def_Imp1(setDigZoom)
def_Imp1(setTorchOnOff)

//def_Imp1(setAFLampOffSch)


int FlashMgr::setManualFlash(int senorDev, int duty, int dutyLt)
{
    def_ImpBody2(setManualFlash, senorDev, duty, dutyLt);
}

def_Imp(clearManualFlash)



//===================================================
// type get
//===================================================
def_ImpTypeGet(getFlashMode)
def_ImpTypeGet(isAFLampOn)
def_ImpTypeGet(isFlashOnCapture)
def_ImpTypeGet(isNeedFiringFlash)


//===================================================
// type : 1 para with body
//===================================================



int FlashMgr::setStrobeCallbackFunc(int senorDev, void (* pFunc)(MINT32 en))
{
    def_ImpBody1(setStrobeCallbackFunc, senorDev, pFunc);
}


//===================================================
// type : 2 para with body
//===================================================
int FlashMgr::doPfOneFrame(int senorDev, FlashExePara* para, FlashExeRep* rep)
{
    def_ImpBody2(doPfOneFrame, senorDev, para, rep);
}

int FlashMgr::setEvComp(int senorDev, int ind, float ev_step)
{
    def_ImpBody2(setEvComp, senorDev, ind, ev_step);
}

int FlashMgr::egSetMfDutyStep(int senorDev, int duty, int step)
{
    def_ImpBody1(egSetMfDuty, senorDev, duty);
}

int FlashMgr::setAeFlashMode(int senorDev, int seMode, int flashMode)
{
    def_ImpBody2(setAeFlashMode, senorDev, seMode, flashMode);
}


int FlashMgr::getDebugInfo(int senorDev, FLASH_DEBUG_INFO_T* p)
{
    def_ImpBody1(getDebugInfo, senorDev, p);
}
/*
int FlashMgr::egGetDutyRange(int senorDev, int* st, int* ed)
{
    def_ImpBody2(egGetDutyRange, senorDev, st, ed);
}*/


int FlashMgr::getAfLampMode(int senorDev)
{
    return 0;
}

int FlashMgr::egGetDutyRange(int senorDev, int* st, int* ed)
{
    *st=0;
    *ed=1;
    return 0;
}
int FlashMgr::egGetStepRange(int senorDev, int* st, int* ed)
{
    *st=0;
    *ed=0;
    return 0;
}

