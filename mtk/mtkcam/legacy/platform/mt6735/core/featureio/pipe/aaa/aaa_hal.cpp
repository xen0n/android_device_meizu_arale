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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "aaa_hal"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <state_mgr/aaa_state.h>
#include <state_mgr/aaa_state_mgr.h>
#include <camera_custom_nvram.h>
/*NeedUpdate*///#include <mtkcam/custom/hal/inc/aaa/awb_param.h>
#include <awb_mgr_if.h>
#include <kd_camera_feature.h>
/*NeedUpdate*///#include <mtkcam/custom/hal/inc/aaa/af_param.h>
/*NeedUpdate*///#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
/*NeedUpdate*///#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>
#include <af_mgr_if.h>
/*NeedUpdate*///#include <mtkcam/custom/hal/inc/aaa/flash_param.h>
/*NeedUpdate*///#include <mtkcam/custom/hal/inc/aaa/ae_param.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
/*NeedUpdate*///#include <mtkcam/drv/sensor_hal.h>
/*NeedUpdate*///#include <CamDefs.h>
#include <ae_mgr_if.h>
/*NeedUpdate*///#include <mtkcam/custom/hal/inc/aaa/flash_tuning_custom.h>
#include <flash_mgr.h>
#include <lsc_mgr2.h>
/*NeedUpdate*///#include <mtkcam/v1/camutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include <aao_buf_mgr.h>
#include <afo_buf_mgr.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/featureio/flicker_hal_base.h>
#include <mtkcam/featureio/ISync3A.h>
#include <aaa_scheduler.h>
#include <debug_exif/cam/dbg_cam_param.h>
#include <debug_exif/cam/dbg_cam_n3d_param.h>
#include <aaa_common_custom.h>

#include <sys/errno.h>
#include "aaa_sensor_buf_mgr.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/utils/common.h>
#include <sys/prctl.h>
MINT32        g_bAFThreadLoop = 0;
MINT32        g_semAFIRQWait = 0;
#ifdef MTK_SLOW_MOTION_VIDEO
MINT32        g_AFMinFps = 0;
#endif
pthread_t     g_AFThread;
sem_t         g_semAFThreadstart;
IspDrv*       g_pIspDrv;

//MINT32 NS3A::Hal3A::sm_3AThreadLogEnable = 0;

using namespace NS3A;
using namespace NSIspTuning;
using namespace android;
using namespace NSCam::Utils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if  0
#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}
#else
#define ERROR_CHECK(API) {API;}
#endif

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_FTRACE, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A*
Hal3A::
createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    MY_LOG("Hal3A::createInstance() \n");

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            Hal3ADev<SENSOR_DEV_MAIN>::getInstance()->init(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ADev<SENSOR_DEV_MAIN>::getInstance();
        break;
        case SENSOR_DEV_SUB:
            Hal3ADev<SENSOR_DEV_SUB>::getInstance()->init(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ADev<SENSOR_DEV_SUB>::getInstance();
        break;
        case SENSOR_DEV_MAIN_2:
            Hal3ADev<SENSOR_DEV_MAIN_2>::getInstance()->init(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ADev<SENSOR_DEV_MAIN_2>::getInstance();
        break;
        default:
            MY_ERR("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }


#if 0
    Hal3A *pHal3A  = Hal3A::getInstance();

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            pHal3A->init(ESensorDev_Main);
        break;
        case SENSOR_DEV_SUB:
            pHal3A->init(ESensorDev_Sub);
        break;
        case SENSOR_DEV_MAIN_2:
            pHal3A->init(ESensorDev_MainSecond);
        break;
        case SENSOR_DEV_MAIN_3D:
            pHal3A->init(ESensorDev_Main3D);
        break;
        default:
            MY_ERR("Unsupport sensor device: %d\n", i4SensorDevId);
            return MNULL;
        break;
    }

    return pHal3A;
#endif
}

Hal3A*
Hal3A::
getInstance(MINT32 i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            return Hal3ADev<SENSOR_DEV_MAIN>::getInstance();
        case SENSOR_DEV_SUB:
            return Hal3ADev<SENSOR_DEV_SUB>::getInstance();
        case SENSOR_DEV_MAIN_2:
            return Hal3ADev<SENSOR_DEV_MAIN_2>::getInstance();
        default:
            MY_ERR("Unsupport sensor device: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }
#if 0
    static Hal3A singleton;
    return &singleton;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3A::
destroyInstance()
{
    uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A::Hal3A()
    : Hal3AIf()
    , m_Users(0)
    , m_Lock()
    , m_ValidateLock()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    /*NeedUpdate*///    , mpFlickerHal(NULL)
    , m_i4FrmId(0)
    , m_3ACapMode(ECapMode_P2_Cal|ECapMode_P2_Set)
    , m_PvMode(EPv_Normal)
    , m_bFaceDetectEnable(MFALSE)
    , m_b3APvInitOK(MFALSE)
    , mpVSTimer(NULL)
    , mbEnable3ASetParams(MTRUE)
    , mbInCaptureProcess(MFALSE)
    , mbInCapturing(MFALSE)
    , mbEnAESenThd(MTRUE)
    , mpIspDrv_forAF(NULL)
    , mbAFThreadLoop(0)
    , mpStateMgr(NULL)
    , mEnFlushVSIrq(0)
    , mFlushVSIrqDone(0)
    , mEnFlushAFIrq(0)
    , mFlushAFIrqDone(0)
    , mpScheduler(NULL)
    , m_3ALogEnable(0)
    , m_3ACtrlEnable(0)
    , mpStateLockTimer(NULL)
    , m_i4SensorType(ESensorType_RAW)

{


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A::~Hal3A()
{

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3A::
init(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa_hal.enable", value, "0");
    m_bDebugEnable = atoi(value);
    property_get("debug.aaa.pvlog.enable", value, "0");
    /*IState::sm_3APvLogEnable = */m_3ALogEnable = atoi(value);
    property_get("debug.aaa.ctrl.enable", value, "1023");
    m_3ACtrlEnable = atoi(value);
    property_get("debug.aaa.timeout.enable", value, "0");
    m_3ATimeOutEnable= atoi(value);
    MY_LOG("m_3ALogEnable = %d, m_3ACtrlEnable = %d, m_3ATimeOutEnable = %d\n", m_3ALogEnable, m_3ACtrlEnable, m_3ATimeOutEnable);

    MY_LOG_IF(1,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    #ifdef MTK_SLOW_MOTION_VIDEO
    g_AFMinFps = 30;
    #endif
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG_IF(1,"%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_i4SensorDev = i4SensorDevId;
    //IState::setSensorDev(i4SensorDevId);
    m_i4SensorOpenIdx = i4SensorOpenIndex;

    MY_LOG_IF(1,"m_i4SensorDev = %d, i4SensorOpenIndex = %d \n", m_i4SensorDev, i4SensorOpenIndex);

    mpVSTimer = new AaaTimer;
    mpStateLockTimer = new AaaTimer;
    mpScheduler = new AAA_Scheduler;
    //mpScheduler should be assigned before creating StateMgr, since IState will copy m_pHal3A->mpScheduler as m_pScheduler in constructor
    if (ISync3AMgr::getInstance()->isActive())
    {
        MY_LOG("[%s] N3D is active", __FUNCTION__);
        m_i4StereoWarning = 0;
        mpStateMgr = new StateMgrN3d(i4SensorDevId);
    }
    else
    {
        MY_LOG("[%s] 2D", __FUNCTION__);
        mpStateMgr = new StateMgr(i4SensorDevId);
    }
    mpCbSet = new CallBackSet;

    // init
    bRet = postCommand(ECmd_Init);
    if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");
    mbEnAESenThd = MTRUE;
    mbEnPDThd = MTRUE;
    createThread();

    m_TgInfo = CAM_TG_ERR; //need to call Hal3A::queryTGInfoFromSensorHal() to get correct TG info

    mpIspDrv = IspDrv::createInstance();
    bRet = mpIspDrv->init("Hal3A");
    if (!bRet) AEE_ASSERT_3A_HAL("IspDrv->init fail.");

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
    bRet = IspTuningMgr::getInstance().init(m_i4SensorDev, m_i4SensorOpenIdx);
    if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr init fail.");
}
}

    ret = EnableAFThread(1);
    if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(1) fail.");

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLICKER & m_3ACtrlEnable) {
    //flicker create buf
    FlickerHalBase::getInstance().createBuf(m_i4SensorDev);
    //FlickerHalBase *pFlk;
    //pFlk = FlickerHalBase::createInstance();
    //pFlk->createBuf();
}
if (ENABLE_AF & m_3ACtrlEnable) {
    IAfMgr::getInstance().setCallbacks(m_i4SensorDev, mpCbSet);
    //
}
if (ENABLE_FLASH & m_3ACtrlEnable) {
    FlashMgr::getInstance().create(m_i4SensorDev);
}
}

    android_atomic_inc(&m_Users);

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3A::
uninit()
{

    MY_LOG_IF(1,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        destroyThread();
        ret = EnableAFThread(0);

    mbEnPDThd = MFALSE;
    ::sem_post(&semPD);
    pthread_join(mPDThread, NULL);

        if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(0) fail.");
        bRet = postCommand(ECmd_Uninit);
        if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");

        delete mpCbSet;
        mpCbSet = NULL;

        delete mpScheduler;
        mpScheduler = NULL;

        delete mpVSTimer;
        mpVSTimer = NULL;

        delete mpStateLockTimer;
        mpStateLockTimer = NULL;

        delete mpStateMgr;
        mpStateMgr = NULL;

        if ( mpIspDrv != NULL )
        {
            mpIspDrv->uninit("Hal3A");
            mpIspDrv=NULL;
        }
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLICKER & m_3ACtrlEnable) {
        FlickerHalBase::getInstance().releaseBuf(m_i4SensorDev);
}
if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
        bRet = IspTuningMgr::getInstance().uninit(m_i4SensorDev);
        if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr uninit fail.");
}
if (ENABLE_FLASH & m_3ACtrlEnable) {
    FlashMgr::getInstance().destroy(m_i4SensorDev);
}
}

    }
    else    // There are still some users.
    {
        MY_LOG_IF(1,"Still %d users \n", m_Users);
    }

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::validate(const ParamIspProfile_T& rParamIspProfile_, MBOOL fgPerframe)
{
    ParamIspProfile_T rParamIspProfile = rParamIspProfile_;
    //queryTGInfoFromSensorHal();//get TG info before setIspProfile
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
    ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, rParamIspProfile.eIspProfile))
    ERROR_CHECK(IspTuningMgr::getInstance().notifyRPGEnable(m_i4SensorDev, rParamIspProfile.iEnableRPG))

    if (rParamIspProfile.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
    {
        if (fgPerframe)
        {
            AaaTimer rTm;
            rTm.start("Isp.validatePerFrame", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            ERROR_CHECK(IspTuningMgr::getInstance().validatePerFrame(m_i4SensorDev, rParamIspProfile.i4MagicNum, (ENABLE_FORCEVALIDATE & m_3ACtrlEnable)?MTRUE:MFALSE));
            rTm.End();
        }
        else
        {
            AaaTimer rTm;
            rTm.start("Isp.validateF", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            ERROR_CHECK(IspTuningMgr::getInstance().validate(m_i4SensorDev, rParamIspProfile.i4MagicNum));
            rTm.End();
        }

        if (rParamIspProfile.iValidateOpt == ParamIspProfile_T::EParamValidate_All)
        {
            AaaTimer rTm;
            rTm.start("configUpdate", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            configUpdate(m_TgInfo, rParamIspProfile.i4MagicNum);
            rTm.End();
        }
    }
}
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3A::on3AProcFinish(MINT32 i4MagicNum)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {

    // result
    m_rResult.i4FrmId = i4MagicNum;
    //m_rResult.i4PrecaptureId
if (ENABLE_AF & m_3ACtrlEnable) {
    //m_rResult.eAfState = (EAfState_T) IAfMgr::getInstance().getAFState();
}
if (ENABLE_AE & m_3ACtrlEnable) {
    //m_rResult.eAeState = (EAeState_T) IAeMgr::getInstance()->getAeState();
}
if (ENABLE_AWB & m_3ACtrlEnable) {
    //m_rResult.eAwbState = (EAwbState_T) IAwbMgr::getInstance().getAWBState(m_i4SensorDev);
}
    m_rResultBuf.updateResult(m_rResult.i4FrmId, m_rResult);

    // TODO: send 3A finish
    MUINT32 fgOK = MTRUE;
    MUINT32 fgAeAwbReady = m_b3APvInitOK;
    MUINT32 u4Notify =
        (fgOK << I3ACallBack::e3AProcOK) |
        (fgAeAwbReady << I3ACallBack::e3APvInitReady);
#if NOTIFY_3A_DONE
    mpCbSet->doNotifyCb(
        I3ACallBack::eID_NOTIFY_3APROC_FINISH,
        m_rResult.i4FrmId,
        u4Notify,
        0);
#endif

    if (ISync3AMgr::getInstance()->isActive() &&
        m_i4SensorDev == ISync3AMgr::getInstance()->getMasterDev())
    {
        MINT32 i4Warning = getStereo3DWarning();
        if (i4Warning != m_i4StereoWarning)
        {
            MY_LOG_IF(m_3ALogEnable & EN_3A_STEREO_LOG, "Warning(%08x)", i4Warning);
            m_i4StereoWarning = i4Warning;
            mpCbSet->doNotifyCb(
                I3ACallBack::eID_NOTIFY_STEREO_WARNING,
                i4Warning,
                0,
                0);
        }
    }
}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::postCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    MBOOL fgLogEn = (m_3ALogEnable & EN_3A_FLOW_LOG) ? MTRUE : MFALSE;
    MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] enter, eCmd(%d)\n", eCmd);

    if( eCmd == ECmd_CameraPreviewStart || eCmd == ECmd_CaptureStart)
    {
        mEnFlushVSIrq = mFlushVSIrqDone = 0;
        mEnFlushAFIrq = mFlushAFIrqDone = 0;
        mbEnAESenThd = MTRUE;
        createAEThread();
    }

    if(eCmd == ECmd_CaptureStart){
        mbInCapturing = MTRUE;
    }

    if(eCmd == ECmd_CaptureEnd)
    {
        // destroy AE Thread
        mbEnAESenThd = MFALSE;
        if(!m_3ATimeOutEnable)
            ::sem_wait(&semAESenThdEnd); //break wait when it become 1, i.e., AF thread finish and post
        else
        {
            MBOOL bRet = sem_wait_relativeTime(&semAESenThdEnd, SEM_TIME_LIMIT_NS, "postCommand::semAESenThdEnd");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAESenThdEnd).");
        }
        destroyAEThread();
        mbInCapturing = MFALSE;
    }

    if ((eCmd == ECmd_CameraPreviewEnd))
    {

        //Flush AF Irq
        {
            #define MAX_AF_FLUSH_TIMES 3 //3ms*3 = 9ms
            MINT32 AFFlushCnt = 0;
            mEnFlushAFIrq = 1;
            while ((AFFlushCnt<MAX_AF_FLUSH_TIMES) && (!mFlushAFIrqDone))
            {
                AFFlushCnt++;
                MY_LOG_IF(fgLogEn, "3A PreviewEnd, before flushAFirq");
                flushAFirq();
                MY_LOG_IF(fgLogEn, "3A PreviewEnd, after flushAFirq");
                ::usleep(3000);//3ms
            }
            MY_LOG_IF(fgLogEn, "3A PreviewEnd, after flushAFirq while");
        }


        MY_LOG_IF(fgLogEn, "3A PreviewEnd, start wait semAFProcEnd");
        //Once 3A receive PreviewEnd, 3A cannot return until AF thread also gets AFirq and finish this frame
        //MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] start waiting semAFProcEnd semaphore\n");
        if(!m_3ATimeOutEnable)
            ::sem_wait(&semAFProcEnd); //break wait when it become 1, i.e., AF thread finish and post
        else
        {
            MBOOL bRet = sem_wait_relativeTime(&semAFProcEnd, SEM_TIME_LIMIT_NS, "postCommand::semAFProcEnd");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAFProcEnd).");
        }

        // destroy AE Thread
        mbEnAESenThd = MFALSE;
        {
            mEnFlushVSIrq = 1;
            while (!mFlushVSIrqDone)
            {
                MY_LOG_IF(fgLogEn, "3A PreviewEnd, before flushVSirq");
                flushVSirq();
                MY_LOG_IF(fgLogEn, "3A PreviewEnd, after flushVSirq");
                ::usleep(3000);//3ms
            }
            MY_LOG_IF(fgLogEn, "3A PreviewEnd, after flushVSirq while");
        }

        if(!m_3ATimeOutEnable)
            ::sem_wait(&semAESenThdEnd); //break wait when it become 1, i.e., AF thread finish and post
        else
        {
            MBOOL bRet = sem_wait_relativeTime(&semAESenThdEnd, SEM_TIME_LIMIT_NS, "postCommand::semAESenThdEnd");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAESenThdEnd).");
        }
        destroyAEThread();

        if(!m_3ATimeOutEnable)
            ::sem_wait(&semPDThdEnd); //break wait when it become 1, i.e., AF thread finish and post
        else
        {
            MBOOL bRet = sem_wait_relativeTime(&semPDThdEnd, SEM_TIME_LIMIT_NS, "postCommand::semPDThdEnd");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semPDThdEnd).");
        }

        MY_LOG("3A PreviewEnd, wait semAFProcEnd done");

        //reset scheduler
        mpScheduler->resetWork();
    }

	MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] before StateMgr::sendCmd(%d)\n", eCmd);
    if (eCmd == ECmd_Update) mpStateLockTimer->start("WaitStateLock",m_i4SensorDev,(m_3ALogEnable & EN_3A_SCHEDULE_LOG));
    ERROR_CHECK(mpStateMgr->sendCmd(eCmd))
	MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] after StateMgr::sendCmd(%d)\n", eCmd);

    if (eCmd == ECmd_Update)
    {
        const ParamIspProfile_T* pParamIspProf = reinterpret_cast<const ParamIspProfile_T*>(i4Arg);
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
        if (mpStateMgr->getFrameCount() >= 0)
        {
            if (mpScheduler->jobAssignAndTimerStart(E_Job_IspValidate, MFALSE))
            {
                Mutex::Autolock lock(m_ValidateLock);
                AaaTimer rTmVld;
                rTmVld.start("aaa.validate", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                validate(*pParamIspProf, MTRUE);
                 rTmVld.End();
            }
            mpScheduler->jobTimerEnd(E_Job_IspValidate);
        }
}
        if(!mpScheduler->makeJobTimeSummary()) MY_ERR("3A job time exceeds limit\n");

        AaaTimer mAFMtxTimer("mAFMtxTimer", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        #ifdef MTK_SLOW_MOTION_VIDEO
        g_AFMinFps = m_rParam.i4MinFps/1000;
        #endif
        if (!mbInCaptureProcess)
            if (mpScheduler->jobAssignAndTimerStart(E_Job_Af))
            {
                int Val;
                MY_LOG_IF(fgLogEn, "[Hal3A::postCommand] start waiting mAFMtx lock\n");
                Mutex::Autolock autoLock(mAFMtx); //lock for critical section
                MY_LOG_IF(fgLogEn, "[Hal3A::postCommand] get mAFMtx lock\n");
                if (mbSemAFIRQWait == 0)
                {
                    //AF thread start wait AF irq
                    ::sem_getvalue(&semAFProcEnd, &Val);
                    if (Val == 1) ::sem_wait(&semAFProcEnd); //to be 0, it won't block
                    ::sem_post(&mSemAFThreadstart);
                    MY_LOG_IF(fgLogEn, "[AFThread] sem_post g_semAFThreadstart, semAFProcEnd before wait = %d\n", Val);
                }
            }
        mAFMtxTimer.End();

        if (mpStateMgr->getFrameCount() >= 0)
        {
            if (mpScheduler->jobAssignAndTimerStart(E_Job_IspValidate, MFALSE))
            {
                AaaTimer rTmProcFinish;
                rTmProcFinish.start("aaa.on3AProcFinish", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                on3AProcFinish(pParamIspProf->i4MagicNum);
                rTmProcFinish.End();
            }
            mpScheduler->jobTimerEnd(E_Job_IspValidate);
        }
}
    }

	MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] exit, eCmd(%d)\n", eCmd);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setParams(Param_T const &rNewParam)
{
    if (!mbEnable3ASetParams) return MTRUE;

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_AWB & m_3ACtrlEnable) {
   // ======================== AWB parameter ==============================
   // AWB mode
   ERROR_CHECK(IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rNewParam.u4AwbMode))

   // AWB lock
   ERROR_CHECK(IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock))

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] AWB done\n");
}
if (ENABLE_ISPTUNE& m_3ACtrlEnable) {
   // ======================== ISP Tuning parameter ==============================
   // ISP tuning
   ERROR_CHECK(IspTuningMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setEffect(m_i4SensorDev, rNewParam.u4EffectMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewParam.u4BrightnessMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewParam.u4HueMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewParam.u4SaturationMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewParam.u4EdgeMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewParam.u4ContrastMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setCamMode(m_i4SensorDev, rNewParam.u4CamMode))

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] ISP tuning done, Scene(%d)\n", rNewParam.u4SceneMode);
}
if (ENABLE_AE & m_3ACtrlEnable) {
   // ======================== AE parameter ==============================
   // AE
   // AE frame rate range   // if the max frame rate equal min frame rate, the frame rate is fix
   ERROR_CHECK(IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps))

   // AE metering mode
   ERROR_CHECK(IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode))

   // AE rotate weighting control
   ERROR_CHECK(IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev, rNewParam.i4RotateDegree))

   // AE ISO speed
   ERROR_CHECK(IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.u4IsoSpeedMode))

   // AE Meter Area
   ERROR_CHECK(IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas))

   // AE Exp mode
   ERROR_CHECK(IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep))

   // AE mode
   ERROR_CHECK(IAeMgr::getInstance().setAEMode(m_i4SensorDev, rNewParam.u4AeMode))

   // AE anti banding
   ERROR_CHECK(IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode))

   // APAE lock
   ERROR_CHECK(IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock))

   // AE Cam mode
   ERROR_CHECK(IAeMgr::getInstance().setAECamMode(m_i4SensorDev, rNewParam.u4CamMode))

   // AE Shot mode
   //ERROR_CHECK(IAeMgr::getInstance().setAEShotMode(m_i4SensorDev, rNewParam.u4ShotMode))

   // AE Scene
   ERROR_CHECK(IAeMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode))

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] AE done\n");
}
if (ENABLE_AF & m_3ACtrlEnable) {
   // ======================== AF parameter ==============================
   // AF
   ERROR_CHECK(IAfMgr::getInstance().setAFMode(m_i4SensorDev, rNewParam.u4AfMode))
   if(m_bFaceDetectEnable==MFALSE)
   {
       ERROR_CHECK(IAfMgr::getInstance().setAFArea(m_i4SensorDev, rNewParam.rFocusAreas))
   }
   //ERROR_CHECK(IAfMgr::getInstance().setCamMode(m_i4SensorDev, rNewParam.u4CamMode))
   ERROR_CHECK(IAfMgr::getInstance().setFullScanstep(m_i4SensorDev, rNewParam.i4FullScanStep))
   ERROR_CHECK(IAfMgr::getInstance().setMFPos(m_i4SensorDev, rNewParam.i4MFPos))
   IAfMgr::getInstance().setAndroidServiceState(m_i4SensorDev,rNewParam.bIsSupportAndroidService);

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] AF done\n");
}
if (ENABLE_FLASH & m_3ACtrlEnable) {
   // ======================== Flash parameter ==============================
   //flash
   if (mpStateMgr->getStateStatus().eCurrState == eState_AF && m_rParam.u4AeMode != rNewParam.u4AeMode) {
       cancelAutoFocus();
       ERROR_CHECK(FlashMgr::getInstance().setAeFlashMode(m_i4SensorDev, rNewParam.u4AeMode, rNewParam.u4StrobeMode))
       autoFocus();
   } else {
       ERROR_CHECK(FlashMgr::getInstance().setAeFlashMode(m_i4SensorDev, rNewParam.u4AeMode, rNewParam.u4StrobeMode))
   }
   int bMulti;
   if(rNewParam.u4CapType == ECapType_MultiCapture)
    bMulti=1;
   else
    bMulti=0;
   ERROR_CHECK(FlashMgr::getInstance().setMultiCapture(m_i4SensorDev, bMulti))
   ERROR_CHECK(FlashMgr::getInstance().setCamMode(m_i4SensorDev, rNewParam.u4CamMode))
   ERROR_CHECK(FlashMgr::getInstance().setEvComp(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep))

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] Flash done\n");
}
if (ENABLE_FLICKER & m_3ACtrlEnable) {
   // ======================== Others  ==============================
    FlickerHalBase::getInstance().setFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);

   MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::setParams] Flicker done\n");
}
}
    m_rParam = rNewParam;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getSupportedParams(FeatureParam_T &rFeatureParam)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    rFeatureParam.bAutoWhiteBalanceLockSupported = IAwbMgr::getInstance().isAWBLockSupported();

    rFeatureParam.u4MaxFocusAreaNum = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);
    rFeatureParam.i4MaxLensPos = IAfMgr::getInstance().getMaxLensPos(m_i4SensorDev);
    rFeatureParam.i4MinLensPos = IAfMgr::getInstance().getMinLensPos(m_i4SensorDev);
    rFeatureParam.i4AFBestPos = IAfMgr::getInstance().getAFBestPos(m_i4SensorDev);
    rFeatureParam.u4PureRawInterval = IAfMgr::getInstance().PDPureRawInterval(m_i4SensorDev);
    //rFeatureParam.i8BSSVlu = IAfMgr::getInstance().getBestShotValue(m_i4SensorDev);

    // FIXME:
    rFeatureParam.bExposureLockSupported = IAeMgr::getInstance().isAELockSupported();

    rFeatureParam.u4MaxMeterAreaNum = IAeMgr::getInstance().getAEMaxMeterAreaNum(m_i4SensorDev);
    IAeMgr::getInstance().getNvramData(m_i4SensorDev);
    AE_DEVICES_INFO_T rDeviceInfo;
    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev, rDeviceInfo);
    rFeatureParam.u4FocusLength_100x = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350

    rFeatureParam.bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::autoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if ((m_rParam.u4AfMode != AF_MODE_AFC) &&
        (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO) &&
        (IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev) > 0))
    {
        ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFStart));
    }
    IAfMgr::getInstance().autoFocus(m_i4SensorDev);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::cancelAutoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    //if ((m_rParam.u4AfMode != AF_MODE_AFC) && (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO))   {
        ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFEnd));
    //}
    IAfMgr::getInstance().cancelAutoFocus(m_i4SensorDev);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {

    ERROR_CHECK(IspTuningMgr::getInstance().setZoomRatio(m_i4SensorDev, u4ZoomRatio_x100))

    ERROR_CHECK(IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, u4XOffset, u4YOffset, u4Width, u4Height))
    ERROR_CHECK(IAfMgr::getInstance().setZoomWinInfo(m_i4SensorDev, u4XOffset, u4YOffset, u4Width, u4Height))

    ERROR_CHECK(FlashMgr::getInstance().setDigZoom(m_i4SensorDev, u4ZoomRatio_x100))
}
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {

    EXIF_INFO_T rEXIFInfo;
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_MODE_CFG_T rCaptureInfo;

    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev, rDeviceInfo);
    rEXIFInfo.u4FNumber = rDeviceInfo.u4LensFno; // Format: F2.8 = 28
    rEXIFInfo.u4FocalLength = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
    rEXIFInfo.u4SceneMode = m_rParam.u4SceneMode; // Scene mode   (SCENE_MODE_XXX)
    rEXIFInfo.u4AEMeterMode = IAeMgr::getInstance().getAEMeterMode(m_i4SensorDev); // Exposure metering mode   (AE_METERING_MODE_XXX)
    rEXIFInfo.i4AEExpBias = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev); // Exposure index  (AE_EV_COMP_XX)
    rEXIFInfo.u4AEISOSpeed = IAeMgr::getInstance().getAEISOSpeedMode(m_i4SensorDev);

    rEXIFInfo.u4AWBMode = m_rParam.u4AwbMode;

    IAeMgr::getInstance().getCaptureParams(m_i4SensorDev, rCaptureInfo);
    rEXIFInfo.u4CapExposureTime = rCaptureInfo.u4Eposuretime;
    if(FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev))
        rEXIFInfo.u4FlashLightTimeus=30000;
    else
        rEXIFInfo.u4FlashLightTimeus=0;

    rEXIFInfo.u4RealISOValue = rCaptureInfo.u4RealISO;

    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::set3AEXIFInfo] Scene(%d)\n", m_rParam.u4SceneMode);

    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const
{
    MY_LOG("[%s] +", __FUNCTION__);
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if (!pDbgInfoCtnr) { MY_ERR("[%s()] pDbgInfoCtnr is NULL\n", __FUNCTION__); return MFALSE; }

    // ======================== allocate buffers of IDbgInfoContainer ==============================
    MVOID* pBuf = NULL;
    AAA_DEBUG_INFO_T& r3ADebugInfo = *(AAA_DEBUG_INFO_T*)(pBuf = pDbgInfoCtnr->queryMemory(AAA_DEBUG_KEYID, sizeof(AAA_DEBUG_INFO_T)));
    if (!pBuf) { MY_ERR("[%s()] alloc AAA_DEBUG_INFO_T fail\n", __FUNCTION__); return MFALSE; }


    DEBUG_SHAD_ARRAY_INFO_T& rShadingDbgTbl =
        *(DEBUG_SHAD_ARRAY_INFO_T*)(pBuf = pDbgInfoCtnr->queryMemory(DEBUG_SHAD_TABLE_KEYID, sizeof(DEBUG_SHAD_ARRAY_INFO_T)));
    if (!pBuf) { MY_ERR("[%s()] alloc DEBUG_SHAD_ARRAY_INFO_T fail\n", __FUNCTION__); return MFALSE; }



    // ======================== start to fill in IDbgInfoContainer ==================================
    // 3A debug info
    //static AAA_DEBUG_INFO_T r3ADebugInfo;

    r3ADebugInfo.hdr.u4KeyID = AAA_DEBUG_KEYID;
    r3ADebugInfo.hdr.u4ModuleCount = MODULE_NUM(9,6);

    r3ADebugInfo.hdr.u4AEDebugInfoOffset        = sizeof(r3ADebugInfo.hdr);
    r3ADebugInfo.hdr.u4AFDebugInfoOffset        = r3ADebugInfo.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugInfoOffset       = r3ADebugInfo.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlashDebugInfoOffset     = r3ADebugInfo.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlickerDebugInfoOffset   = r3ADebugInfo.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4ShadingDebugInfoOffset   = r3ADebugInfo.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugDataOffset       = r3ADebugInfo.hdr.u4ShadingDebugInfoOffset + sizeof(SHADING_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AEPlineDataOffset        = r3ADebugInfo.hdr.u4AWBDebugDataOffset + sizeof(AWB_DEBUG_DATA_T);
    r3ADebugInfo.hdr.u4ShadingTableOffset       = r3ADebugInfo.hdr.u4AEPlineDataOffset + sizeof(AE_PLINE_DEBUG_INFO_T);

    // AE
    IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAEDebugInfo, r3ADebugInfo.rAEPlineDebugInfo);

    // AF
    IAfMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAFDebugInfo);

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAWBDebugInfo, r3ADebugInfo.rAWBDebugData);

    // Flash
    FlashMgr::getInstance().getDebugInfo(m_i4SensorDev, &r3ADebugInfo.rFlashDebugInfo);

    // Flicker
//    Flicker::getInstance()->getDebugInfo(&r3ADebugInfo.rFlickerDebugInfo);

    // LSC
    //DEBUG_SHAD_ARRAY_INFO_T rShadingDbgTbl;
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugInfo(r3ADebugInfo.rShadigDebugInfo);
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugTbl(rShadingDbgTbl, r3ADebugInfo.rShadingRestTbl);

#if 0 //legacy code for reference
    //MINT32 ID;
    //pIBaseCamExif->sendCommand(CMD_REGISTER, AAA_DEBUG_KEYID, reinterpret_cast<MINT32>(&ID));
    //pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&r3ADebugInfo), sizeof(AAA_DEBUG_INFO_T));

    // LSC table
    //pIBaseCamExif->sendCommand(CMD_REGISTER, AAA_DEBUG_SHADTABLE_MODULE_ID, reinterpret_cast<MINT32>(&ID));
    //pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rShadingDbgTbl), sizeof(rShadingDbgTbl));
#endif

    // ISP debug info
    NSIspExifDebug::IspExifDebugInfo_T rIspExifDebugInfo;
    IspTuningMgr::getInstance().getDebugInfo(m_i4SensorDev, rIspExifDebugInfo, fgReadFromHW);

    // ======================== start to allocate IspExifDebugInfo_T and fill in IDbgInfoContainer ==================================
    if (!(pBuf = pDbgInfoCtnr->queryMemory(static_cast<MINT32>(rIspExifDebugInfo.hdr.u4KeyID), sizeof(NSIspExifDebug::IspExifDebugInfo_T))))
        { MY_ERR("[%s()] alloc IspExifDebugInfo_T fail\n", __FUNCTION__); return MFALSE; }
    *(NSIspExifDebug::IspExifDebugInfo_T*)pBuf = rIspExifDebugInfo;

    if (ISync3AMgr::getInstance()->isActive())
    {
        MY_LOG("[%s] Stereo Debug Info", __FUNCTION__);
        void* pStereoBuf = pDbgInfoCtnr->queryMemory(DEBUG_CAM_N3D_MID, sizeof(N3D_DEBUG_INFO_T));
        if (!pStereoBuf)
        {
            MY_ERR("[%s()] alloc N3D_DEBUG_INFO_T fail\n", __FUNCTION__);
            return MFALSE;
        }
        ISync3AMgr::getSync3A()->setDebugInfo(pStereoBuf);
    }

#if 0 //legacy code for reference
    pIBaseCamExif->sendCommand(CMD_REGISTER, static_cast<MINT32>(rIspExifDebugInfo.hdr.u4KeyID), reinterpret_cast<MINT32>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rIspExifDebugInfo), sizeof(rIspExifDebugInfo));
#endif
}
    MY_LOG("[%s] -", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getDelayFrame(EQueryType_T const eQueryType) const
{
    switch (eQueryType)
    {
    case EQueryType_Init:
        return 3;
        break;
    case EQueryType_Effect:
    case EQueryType_AWB:
    default:
        return 0;
        break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr)
{
    Mutex::Autolock lock(m_ValidateLock);

    if(rParamIspProfile.eIspProfile == EIspProfile_Video) {
        IAeMgr::getInstance().updateAEScenarioMode(m_i4SensorDev, rParamIspProfile.eIspProfile);
    }

    MBOOL res = validate(rParamIspProfile, MFALSE);
    if (pDbgInfoCtnr)
        setDebugInfo(pDbgInfoCtnr, MFALSE); //read from working buffer

    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    return IAeMgr::getInstance().getExposureInfo(m_i4SensorDev, strHDRInputSetting);
}
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {

    AE_MODE_CFG_T rCaptureInfo;

    IAeMgr::getInstance().getCaptureParams(m_i4SensorDev, rCaptureInfo);

    a_rCaptureInfo.u4ExposureMode = rCaptureInfo.u4ExposureMode;
    a_rCaptureInfo.u4Eposuretime = rCaptureInfo.u4Eposuretime;
    a_rCaptureInfo.u4AfeGain = rCaptureInfo.u4AfeGain;
    a_rCaptureInfo.u4IspGain = rCaptureInfo.u4IspGain;
    a_rCaptureInfo.u4RealISO = rCaptureInfo.u4RealISO;
    a_rCaptureInfo.u4FlareGain = (MUINT32) rCaptureInfo.i2FlareGain;
    a_rCaptureInfo.u4FlareOffset = (MUINT32) rCaptureInfo.i2FlareOffset;
    a_rCaptureInfo.i4LightValue_x10 = IAeMgr::getInstance().getCaptureLVvalue(m_i4SensorDev);
    a_rCaptureInfo.u43ACapMode = m_3ACapMode;
}

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {

    AE_MODE_CFG_T rCaptureInfo;

    rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
    rCaptureInfo.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    rCaptureInfo.u4AfeGain = a_rCaptureInfo.u4AfeGain;
    rCaptureInfo.u4IspGain = a_rCaptureInfo.u4IspGain;
    rCaptureInfo.u4RealISO = a_rCaptureInfo.u4RealISO;
    rCaptureInfo.i2FlareGain = (MINT16)a_rCaptureInfo.u4FlareGain;
    rCaptureInfo.i2FlareOffset = (MINT16)a_rCaptureInfo.u4FlareOffset;

    IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, rCaptureInfo);
    m_3ACapMode = a_rCaptureInfo.u43ACapMode;
}
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAeMgr::getInstance().getHDRCapInfo(m_i4SensorDev, a_strHDROutputInfo);
}
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getRTParams(FrameOutputParam_T &a_strFrameOutputInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAeMgr::getInstance().getRTParams(m_i4SensorDev, a_strFrameOutputInfo);
}
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3A::setFDEnable(MBOOL a_sFacesEn)
{
    m_bFaceDetectEnable=a_sFacesEn;
    IAeMgr::getInstance().setFDenable(m_i4SensorDev, a_sFacesEn);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setFDInfo(MVOID* a_sFaces)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAfMgr::getInstance().setFDInfo(m_i4SensorDev, a_sFaces);
    IAeMgr::getInstance().setFDInfo(m_i4SensorDev, a_sFaces);
}
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setOTInfo(MVOID* a_sOT)
{
    IAfMgr::getInstance().setOTInfo(m_i4SensorDev,a_sOT);
    IAeMgr::getInstance().setOTInfo(m_i4SensorDev,a_sOT);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::enableAELimiterControl(MBOOL  bIsAELimiter)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAeMgr::getInstance().setAELimiterMode(m_i4SensorDev, bIsAELimiter);
}
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getASDInfo(ASDInfo_T &a_rASDInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    // AWB
    AWB_ASD_INFO_T rAWBASDInfo;
    IAwbMgr::getInstance().getASDInfo(m_i4SensorDev, rAWBASDInfo);
    a_rASDInfo.i4AWBRgain_X128 = rAWBASDInfo.i4AWBRgain_X128;
    a_rASDInfo.i4AWBBgain_X128 = rAWBASDInfo.i4AWBBgain_X128;
    a_rASDInfo.i4AWBRgain_D65_X128 = rAWBASDInfo.i4AWBRgain_D65_X128;
    a_rASDInfo.i4AWBBgain_D65_X128 = rAWBASDInfo.i4AWBBgain_D65_X128;
    a_rASDInfo.i4AWBRgain_CWF_X128 = rAWBASDInfo.i4AWBRgain_CWF_X128;
    a_rASDInfo.i4AWBBgain_CWF_X128 = rAWBASDInfo.i4AWBBgain_CWF_X128;
    a_rASDInfo.bAWBStable = rAWBASDInfo.bAWBStable;

    a_rASDInfo.i4AFPos = IAfMgr::getInstance().getAFPos(m_i4SensorDev);
    a_rASDInfo.pAFTable = IAfMgr::getInstance().getAFTable(m_i4SensorDev);
    a_rASDInfo.i4AFTableOffset = IAfMgr::getInstance().getAFTableOffset(m_i4SensorDev);
    a_rASDInfo.i4AFTableMacroIdx = IAfMgr::getInstance().getAFTableMacroIdx(m_i4SensorDev);
    a_rASDInfo.i4AFTableIdxNum = IAfMgr::getInstance().getAFTableIdxNum(m_i4SensorDev);
    a_rASDInfo.bAFStable = IAfMgr::getInstance().getAFStable(m_i4SensorDev);

    a_rASDInfo.i4AELv_x10 = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
    a_rASDInfo.bAEBacklit = IAeMgr::getInstance().getAECondition(m_i4SensorDev, AE_CONDITION_BACKLIGHT);
    a_rASDInfo.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
    a_rASDInfo.i2AEFaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4SensorDev);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAeMgr::getInstance().modifyAEPlineTableLimitation(m_i4SensorDev, bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
}
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getLCEInfo(LCEInfo_T &a_rLCEInfo)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAeMgr::getInstance().getLCEPlineInfo(m_i4SensorDev, a_rLCEInfo);
}
    return MTRUE;
}

MVOID Hal3A::setSensorMode(MINT32 i4SensorMode)
{
    MY_LOG("setSensorMode %d\n", i4SensorMode);
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IAeMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IAfMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
  IAEBufMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IspTuningMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    FlickerHalBase::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3A::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_AF & m_3ACtrlEnable) {
    if (a_bEnable)  {

        if (mbAFThreadLoop== 0)
        {
            /*ret = AfMgr::getInstance().init();
            if (FAILED(ret)) {
                MY_ERR("AfMgr::getInstance().init() fail\n");
                return ret;
            }*/

            mpIspDrv_forAF = IspDrv::createInstance();

            if (!mpIspDrv_forAF) {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (mpIspDrv_forAF->init("Hal3AAFThd") < 0) {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }

            // create AF thread
            MY_LOG("[AFThread] Create");
            mbAFThreadLoop= 1;
            sem_init(&mSemAFThreadstart, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
            pthread_create(&mAFThread, &attr, AFThreadFunc, this);
        }
    }
    else   {

        if (mbAFThreadLoop == 1)
        {

            if (mpIspDrv_forAF)   {
                mpIspDrv_forAF->uninit("Hal3AAFThd");
                mpIspDrv_forAF = NULL;
            }
            mbAFThreadLoop = 0;
            ::sem_post(&mSemAFThreadstart);

            pthread_join(mAFThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

}
}
    return ret;
}

MVOID * Hal3A::AFThreadFunc(void *arg)
{

    MY_LOG("[AFThread] tid: %d \n", gettid());
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);
    ::prctl(PR_SET_NAME,"AFthread", 0, 0, 0);
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOG(
            "[AFThreadFunc] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

if (ENABLE_3A_GENERAL & _this->m_3ACtrlEnable) {
if (ENABLE_AF & _this->m_3ACtrlEnable) {

    if (!_this->mpIspDrv_forAF) {
        MY_LOG("[AFThread] m_pIspDrv null\n");
        return NULL;
    }

    // wait AFO done
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    waitIrq.Type = (_this->m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status = (_this->m_TgInfo == CAM_TG_1) ?
        CAM_CTL_INT_P1_STATUS_AF_DON_ST : CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
    waitIrq.Timeout = 500; // 200 msec
    waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;
    waitIrq.UserName=const_cast<char *>("Hal3AAF");

    while (_this->mbAFThreadLoop) {
        //
        MBOOL fgLogEn = (_this->m_3ALogEnable & EN_3A_FLOW_LOG) ? MTRUE : MFALSE;
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (1)\n");
        {Mutex::Autolock autoLock(_this->mAFMtx); _this->mbSemAFIRQWait = 0;}
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] get mAFMtx lock (1)\n");


        ::sem_wait(&_this->mSemAFThreadstart);
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] sem_wait mSemAFThreadstart done\n");

        {Mutex::Autolock autoLock(_this->mAFMtx); _this->mbSemAFIRQWait = 1;}
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] get mAFMtx lock (2)\n");


        ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

        if ((_this->m_TgInfo != CAM_TG_1) && (_this->m_TgInfo != CAM_TG_2))
        {
            MY_ERR("m_TgInfo = %d, AFThreadFunc err", _this->m_TgInfo);
            waitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
            waitIrq.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
            waitIrq.Status = CAM_CTL_INT_P1_STATUS_AF_DON_ST;
            waitIrq.Timeout = 500;
            waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;
            waitIrq.UserName=const_cast<char *>("Hal3AAF");
        }
        else
        {
            waitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
            waitIrq.Type = (_this->m_TgInfo == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
            waitIrq.Status = (_this->m_TgInfo == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_AF_DON_ST : CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
            waitIrq.Timeout = 500;
            waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;
            waitIrq.UserName=const_cast<char *>("Hal3AAF");
        }


        #ifdef MTK_SLOW_MOTION_VIDEO
        if(i4preMinfps != g_AFMinFps)
        {
            MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc]AF frame rate %d\n",g_AFMinFps);
            i4preMinfps = g_AFMinFps;
            if(g_AFMinFps>33)
                i4SkipAFframe = (g_AFMinFps-1)/30;
        }

        if(i4AFfrmcount)
        {
            i4AFfrmcount--;
            MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] skip AF frame %d\n", i4AFfrmcount);
        }
        else
        {
            i4AFfrmcount=i4SkipAFframe;
        #endif
        if (_this->mbAFThreadLoop != 0)
        {
            IAfMgr::getInstance().DoCallback(_this->m_i4SensorDev);
            MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");
            if ( _this->mpIspDrv_forAF->waitIrq(&waitIrq) > 0) // success
            {
                if (_this->mEnFlushAFIrq)
                {
                    _this->mFlushAFIrqDone = 1;
                    MY_LOG_IF(1, "[Hal3A::AFThreadFunc] mFlushAFIrqDone = 1\n");
                }

                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
                _this->mpScheduler->jobAssignAndTimerStart(E_Job_Af);
                _this->mpStateMgr->sendCmd(ECmd_AFUpdate);
                _this->mpScheduler->jobTimerEnd(E_Job_Af);
                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");
            }
            else
            {
                MY_LOG("[AFThread] AF irq timeout\n");
                IAfMgr::getInstance().TimeOutHandle(_this->m_i4SensorDev);
                /*need modification*///IAfMgr::getInstance().printAFConfigLog0(_this->m_i4SensorDev);
            }

            {
                int Val2;
                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (3)\n");
                Mutex::Autolock autoLock(_this->mAFMtx);
                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] get mAFMtx lock (3)\n");

                ::sem_getvalue(&_this->semAFProcEnd, &Val2);
                MY_LOG_IF(fgLogEn, "[AFThread] semAFProcEnd before post = %d\n", Val2);
                if (Val2 == 0) ::sem_post(&_this->semAFProcEnd); //to be 1
            }
        }
        #ifdef MTK_SLOW_MOTION_VIDEO
        }
        #endif
    }

    //::sem_post(&g_semAFThreadEnd);

    MY_LOG("[AFThread] End \n");


}
}
    return NULL;
}
/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3A::PDThreadLoop(MVOID *arg)
{
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);
    // (1) change thread setting
    _this->changePDThreadSetting();

    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->semPD);
        if ( ! _this->mbEnPDThd) break;

        MY_LOG_IF(1, "[PDThreadLoop] PD task, dev:%d\n", _this->m_i4SensorDev);
        //
        IAfMgr::getInstance().doPDTask(_this->m_i4SensorDev);

        //
        //sync with 3A thread when PreviewEnd
        {
            int Val;
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::PDThreadLoop] start waiting mPDMtx lock (3)\n");
            Mutex::Autolock autoLock(_this->mPDMtx);
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::PDThreadLoop] get mPDMtx lock (3)\n");

            ::sem_getvalue(&_this->semPDThdEnd, &Val);
            MY_LOG_IF(1, "[PDThreadLoop] semPDThdEnd before post = %d\n", Val);
            if (Val == 0) ::sem_post(&_this->semPDThdEnd); //to be 1, 1 means PD task done
        }
    }
    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MBOOL Hal3A::setCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);

    return MTRUE;/*NeedUpdate*///IAfMgr::getInstance().setCallbacks(m_i4SensorDev, cb);

}
#endif
MINT32
Hal3A::
addCallbacks(I3ACallBack* cb)
{
    MINT32 i4Ret;
    MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, cb);
    i4Ret = mpCbSet->addCallback(cb);
    MY_LOG("[%s] i4Ret(%d)", __FUNCTION__, i4Ret);
    return i4Ret;
}

MINT32
Hal3A::
removeCallbacks(I3ACallBack* cb)
{
    MINT32 i4Ret;
    MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, cb);
    i4Ret = mpCbSet->removeCallback(cb);
    MY_LOG("[%s] i4Ret(%d)", __FUNCTION__, i4Ret);
    return i4Ret;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::flushVSirq()
{
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        MY_ERR("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return;
    }

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.Type = (m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status = (m_TgInfo == CAM_TG_1) ?
        CAM_CTL_INT_P1_STATUS_VS1_INT_ST : CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;

    if (mpIspDrv==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    mpIspDrv->flushIrq(waitIrq);

}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::flushAFirq()
{
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        MY_ERR("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return;
    }

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.Type = (m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status = (m_TgInfo == CAM_TG_1) ?
        CAM_CTL_INT_P1_STATUS_AF_DON_ST : CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
    waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;

    if (mpIspDrv_forAF==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    mpIspDrv_forAF->flushIrq(waitIrq);

}



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::waitVSirq()
{

    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        MY_ERR("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return;
    }

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.Clear=ISP_DRV_IRQ_CLEAR_WAIT;
    waitIrq.Timeout=5000; // 5000 msec
    waitIrq.Type = (m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status = (m_TgInfo == CAM_TG_1) ?
        CAM_CTL_INT_P1_STATUS_VS1_INT_ST : CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;
    waitIrq.UserName=const_cast<char *>("Hal3AVsync");

    if (mpIspDrv==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    if (mpIspDrv->waitIrq(&waitIrq) <= 0)
    {
        MY_ERR("wait vsync timeout");
    }

}

/******************************************************************************
*
*******************************************************************************/
MRESULT Hal3A::queryTGInfoFromSensorHal()
{
    //Before wait for VSirq of IspDrv, we need to query IHalsensor for the current TG info
    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList)
    {
        MY_ERR("IHalSensorList::get() == NULL");
        return E_3A_ERR;
    }
    const char* const callerName = "Hal3AQueryTG";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorOpenIdx);
    //Note that Middleware has configured sensor before
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
    pHalSensor->destroyInstance(callerName);

    MY_LOG("m_i4SensorDev = %d, senInfo.TgInfo = %d\n", m_i4SensorDev, senInfo.TgInfo);

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        MY_ERR("RAW sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
        return E_3A_ERR;
    }

    m_TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    IAwbMgr::getInstance().setTGInfo(m_i4SensorDev, m_TgInfo);
    IAAOBufMgr::getInstance().setTGInfo(m_i4SensorDev, m_TgInfo);
    IAFOBufMgr::getInstance().setTGInfo(m_i4SensorDev, m_TgInfo);
    IAEBufMgr::getInstance().setTGInfo(m_i4SensorDev, m_TgInfo);
    IspTuningMgr::getInstance().setTGInfo(m_i4SensorDev, m_TgInfo);

    return S_3A_OK;
}

/******************************************************************************
*
*******************************************************************************/
MRESULT Hal3A::configUpdate(MUINT32 u4TgInfo, MINT32 i4MagicNum)
{
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

using namespace NSImageio;
using namespace NSIspio;

    switch (u4TgInfo)
    {
        case CAM_TG_1: u4TgInfo = EPortIndex_TG1I; break;
        case CAM_TG_2: u4TgInfo = EPortIndex_TG2I; break;
        case CAM_SV_1: u4TgInfo = EPortIndex_CAMSV_TG1I; break;
        case CAM_SV_2: u4TgInfo = EPortIndex_CAMSV_TG2I; break;
        default:
            MY_ERR("[%s] Err tg idx(%d) err, force using EPortIndex_TG1I", __FUNCTION__, u4TgInfo);
            u4TgInfo = EPortIndex_TG1I;
            break;
    }

    MRESULT result = S_3A_OK;
#if ISP_P1_UPDATE
    INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorOpenIdx, "aaa_hal");
    if ( MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_P1_UPDATE, u4TgInfo, i4MagicNum, 0) )
    {
        MY_ERR("[%s] sensorIdx(%d) tg(%d) magic(%d)", __FUNCTION__, m_i4SensorOpenIdx, u4TgInfo, i4MagicNum);
        result = E_3A_ERR;
    }
    pPipe->destroyInstance("aaa_hal");
#endif
    return result;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 Hal3A::isNeedFiringFlash(MBOOL bEnCal)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLASH & m_3ACtrlEnable) {
    return (bEnCal ?
        FlashMgr::getInstance().isNeedFiringFlash(m_i4SensorDev) : FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev));
}
}
    /*TEMP ADD*/ return 0;
}


MINT32 Hal3A::enableFlashQuickCalibration(MINT32 bEn)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLASH & m_3ACtrlEnable) {
    if(bEn==1)
        return FlashMgr::getInstance().cctSetSpModeQuickCalibration(m_i4SensorDev);
    else
        return FlashMgr::getInstance().cctSetSpModeNormal(m_i4SensorDev);
}
}
    return 0;
}
MINT32 Hal3A::getFlashQuickCalibrationResult()
{

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLASH & m_3ACtrlEnable) {
    MINT32 ret;
    ret = FlashMgr::getInstance().cctGetQuickCalibrationResult(m_i4SensorDev);
    MY_LOG("getFlashQuickCalibrationResult ret=%d", ret);
    return ret;
}
}
    return 0;
}
/******************************************************************************
*
*******************************************************************************/
MVOID Hal3A::endContinuousShotJobs()
{
}
/******************************************************************************
*
*******************************************************************************/
MVOID Hal3A::enterCaptureProcess() { mbInCaptureProcess = MTRUE; }
MVOID Hal3A::exitCaptureProcess()
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
if (ENABLE_FLASH & m_3ACtrlEnable) {
    FlashMgr::getInstance().turnOffFlashDevice(m_i4SensorDev);
}
}
    mbInCaptureProcess = MFALSE;
}
MUINT32 Hal3A::queryFramesPerCycle(MUINT32 fps)
{
    return mpScheduler->queryFramesPerCycle(fps);
}

/******************************************************************************
*
*******************************************************************************/
MINT32
Hal3A::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;
    switch (e3ACtrl)
    {
    // --------------------------------- Shading ---------------------------------
    case E3ACtrl_SetShadingOnOff:
        LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(iArg1 ? MTRUE : MFALSE);
        break;
    case E3ACtrl_SetShadingSdblkCfg:
        i4Ret = LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetSdblkFileCfg((iArg1 ? MTRUE : MFALSE), reinterpret_cast<const char*>(iArg2));
        break;
    case E3ACtrl_SetShadingTSFOnOff:
        LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfOnOff(iArg1 ? MTRUE : MFALSE);
        break;
    case E3ACtrl_SetShadingDynamic:
        i4Ret = IspTuningMgr::getInstance().enableDynamicShading(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
        break;
    case E3ACtrl_SetShadingColorTemp:
        i4Ret = IspTuningMgr:: getInstance().setIndex_Shading(m_i4SensorDev, iArg1);
        break;
    case E3ACtrl_SetShadingByp123:
        i4Ret = LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->CCTOPSetBypass123(iArg1 ? MTRUE : MFALSE);
        break;

    // --------------------------------- AWB ---------------------------------
    case E3ACtrl_SetAwbBypCalibration:
        i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
        break;

    // --------------------------------- AE ---------------------------------
    case E3ACtrl_GetAEPLineTable:
        i4Ret = IAeMgr::getInstance().getAEPlineTable(m_i4SensorDev, static_cast<eAETableID>(iArg1),  *reinterpret_cast<strAETable*>(iArg2));
        break;
    case E3ACtrl_EnableDisableAE:
        if (iArg1) i4Ret = IAeMgr::getInstance().enableAE(m_i4SensorDev);
        else       i4Ret = IAeMgr::getInstance().disableAE(m_i4SensorDev);
        break;
    case E3ACtrl_SetIsAEMultiCapture:
        i4Ret = IAeMgr::getInstance().IsMultiCapture(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
        break;
    case E3ACtrl_GetAECapDelay:
        i4Ret = IAeMgr::getInstance().get3ACaptureDelayFrame(m_i4SensorDev);
        break;
    case E3ACtrl_GetSensorDelayFrame:
        IAeMgr::getInstance().getAESensorActiveCycle(m_i4SensorDev, &i4Ret);
        break;
    case E3ACtrl_SetSensorDirectly:
        i4Ret = IAeMgr::getInstance().setSensorDirectly(m_i4SensorDev, *reinterpret_cast<CaptureParam_T*>(iArg1), (iArg2 ? MTRUE : MFALSE));
        break;
    case E3ACtrl_SetAEContinueShot:
        i4Ret = IAeMgr::getInstance().IsAEContinueShot(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
        break;
    // --------------------------------- ISP ---------------------------------------
    case E3ACtrl_GetIspGamma:
        i4Ret = IspTuningMgr::getInstance().getIspGamma(m_i4SensorDev, reinterpret_cast<MUINT32*>(iArg1), reinterpret_cast<MBOOL*>(iArg2));
        break;
    case E3ACtrl_SetIspTuningISO:
        i4Ret = IspTuningMgr::getInstance().setISO(m_i4SensorDev, iArg1);
        break;
    // --------------------------------- AF ---------------------------------
    case E3ACtrl_QueryAFStatus:
        i4Ret = IAfMgr::getInstance().queryAFStatusByMagicNum(m_i4SensorDev, iArg1);
        break;
    // --------------------------------- PDAF ---------------------------------------
    case E3ACtrl_PostPDAFtask:
        ::sem_post(&semPD);
        break;


    // --------------------------------- flow control ---------------------------------
    case E3ACtrl_Enable3ASetParams:
        mbEnable3ASetParams = iArg1;
        break;
    case E3ACtrl_SetOperMode:
        i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
        break;
    case E3ACtrl_GetOperMode:
        i4Ret = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
        break;
    case E3ACtrl_GetStereo3DWarning:
        i4Ret = getStereo3DWarning();
        break;
    case E3ACtrl_GetEvCapture:
        i4Ret = IAeMgr::getInstance().switchCapureDiffEVState(m_i4SensorDev, (MINT8) iArg1,  *reinterpret_cast<strAEOutput*>(iArg2));
        break;
    case E3ACtrl_GetDAFTBL:
        i4Ret = IAfMgr::getInstance().getDAFtbl(m_i4SensorDev, (MVOID**)iArg1);
        break;
    case ECmd_Set3ACollectInfoFile:
        // if Cmd existed then Free Cmd.
        freeCollectCmd();

        createCollectCmd(iArg1);
        break;
    default:
        return -1;
    }

        return i4Ret;

}

MVOID
Hal3A::
createCollectCmd(MINTPTR i4Arg)
{
    char* rInputPath;
    MINT32 length;
    rInputPath = reinterpret_cast<char*>(i4Arg);
    if(rInputPath == NULL)
    {
        MY_LOG("[%s]rInputPath = null", __FUNCTION__);
        return;
    }
    length = std::strlen(rInputPath) + 1;
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "rInputPath = %s", rInputPath);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "length = %d", length);
    MBOOL bSaveAE = MFALSE;
    MBOOL bSaveAF = MFALSE;
    MBOOL bSaveAWB = MFALSE;
    MBOOL bSaveFlash = MFALSE;

    // copy path
    m_rCollectCmd.path = new char[length];
    strcpy(m_rCollectCmd.path, rInputPath);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "m_rCollectCmd.path = %s", m_rCollectCmd.path);

    // parse path
    if(std::strstr(rInputPath,"/AE/"))
    {
        m_rCollectCmd.i4Type = E3ACollectInfo_AE;
        bSaveAE = MTRUE;
        bSaveAF = MTRUE;
    }
    else if(std::strstr(rInputPath,"/AF/"))
    {
        m_rCollectCmd.i4Type = E3ACollectInfo_AF;
        bSaveAF = MTRUE;
    }
    else if(std::strstr(rInputPath,"/AWB/"))
    {
        m_rCollectCmd.i4Type = E3ACollectInfo_AWB;
        bSaveAF = MTRUE;
        bSaveAWB = MTRUE;
    }
    else if(std::strstr(rInputPath,"/Flash/"))
    {
        m_rCollectCmd.i4Type = E3ACollectInfo_FLASH;
        bSaveAF = MTRUE;
        bSaveAWB = MTRUE;
        bSaveFlash = MTRUE;
    }
    else
        m_rCollectCmd.i4Type = E3ACollectInfo_NONE;

    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "bSaveAE = %d, bSaveAF = %d, bSaveAWB = %d, bSaveFlash = %d", bSaveAE, bSaveAF, bSaveAWB, bSaveFlash);

    if(bSaveAE)
        IAeMgr::getInstance().SaveAEMgrInfo(m_i4SensorDev, rInputPath);
    if(bSaveAF)
        IAfMgr::getInstance().SaveAfMgrInfo(m_i4SensorDev, rInputPath);
    if(bSaveAWB)
        IAwbMgr::getInstance().SaveAwbMgrInfo(m_i4SensorDev, rInputPath);
    if(bSaveFlash)
        FlashMgr::getInstance().SaveFlashMgrInfo(m_i4SensorDev, rInputPath);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "m_rCollectCmd.i4Type = %d", m_rCollectCmd.i4Type);
}


MVOID
Hal3A::
freeCollectCmd()
{
    if(m_rCollectCmd.i4Type != E3ACollectInfo_NONE)
    {
        MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s()] i4Type = %d", __FUNCTION__, m_rCollectCmd.i4Type);
        switch(m_rCollectCmd.i4Type)
        {
            case E3ACollectInfo_AE:
                IAeMgr::getInstance().SaveAEMgrInfo(m_i4SensorDev, "");
                break;
            case E3ACollectInfo_AF:
                IAfMgr::getInstance().SaveAfMgrInfo(m_i4SensorDev, "");
                break;
            case E3ACollectInfo_AWB:
                IAwbMgr::getInstance().SaveAwbMgrInfo(m_i4SensorDev, "");
                break;
            case E3ACollectInfo_FLASH:
                FlashMgr::getInstance().SaveFlashMgrInfo(m_i4SensorDev, "");
                break;
        }
        m_rCollectCmd.i4Type = E3ACollectInfo_NONE;
    }
    if(m_rCollectCmd.path != NULL)
    {
        delete m_rCollectCmd.path;
        m_rCollectCmd.path = NULL;
    }
}

MVOID
Hal3A::
notifyStrobeMode (MINT32 bEnable)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, bEnable ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);
    IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, bEnable);
}
}

MVOID Hal3A::capCheckAndFireFlash_End()
{
    FlashMgr::getInstance().capCheckAndFireFlash_End(m_i4SensorDev);
}

MINT32 Hal3A::SetAETargetMode(MUINT32 AeTargetMode)
{
    // set AWB NVRAM index for normal and video HDR
    if (AeTargetMode == AE_MODE_NORMAL) {
        IAwbMgr::getInstance().setAWBNvramIdx(m_i4SensorDev, AWB_NVRAM_IDX_NORMAL);
    }
    else {
        IAwbMgr::getInstance().setAWBNvramIdx(m_i4SensorDev, AWB_NVRAM_IDX_VHDR);
    }

    return IAeMgr::getInstance().SetAETargetMode(m_i4SensorDev, static_cast<eAETARGETMODE>(AeTargetMode));
}
MVOID Hal3A::setMagicNum2AFMgr(MINT32 magicNum)
{
    IAfMgr::getInstance().setCurFrmNum(m_i4SensorDev, (MUINT32)magicNum);
}


MBOOL  Hal3A::sem_wait_relativeTime(sem_t *pSem, nsecs_t reltime, const char* info, MBOOL isWaitVsync)
{
    MY_LOG_IF(!isWaitVsync, "[%s] pSem(%p), reltime(%lld), info(%s)\n", __FUNCTION__, pSem, reltime, info);
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        MY_ERR("error in clock_gettime! Please check\n");

    ts.tv_sec  += reltime/1000000000;
    ts.tv_nsec += reltime%1000000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }
    int s = sem_timedwait(pSem, &ts);
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            if(!isWaitVsync)
                MY_ERR("[%s][%s]sem_timedwait() timed out\n", __FUNCTION__, info);
            else
                MY_LOG("[%s][%s]AE_sem_timedwait finish\n", __FUNCTION__, info);
            return MFALSE;
        }
        else
            MY_ERR("[%s][%s]sem_timedwait() errno = %d\n", __FUNCTION__, info, errno);
    }
    else
        MY_LOG("[%s][%s]sem_timedwait() succeeded\n", __FUNCTION__, info);

    return MTRUE;

}

MINT32
Hal3A::
getStereo3DWarning()
{
    MINT32 i4Warninig = 0;
    if (ISync3AMgr::getInstance()->isActive())
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getSlaveDev();
        // large lv diff
        MINT32 i4LvMaster = IAeMgr::getInstance().getLVvalue(i4Master, MTRUE);
        MINT32 i4LvSlave = IAeMgr::getInstance().getLVvalue(i4Slave, MTRUE);
        i4Warninig |= ((abs(i4LvMaster - i4LvSlave) > 50) ? 0x1 : 0);
        // low light
        i4Warninig |= ((max(i4LvMaster, i4LvSlave) < 10) ? 0x2 : 0);
        // close shot
        MINT32 i4FocusDistance = IAfMgr::getInstance().getAFBestPos(i4Master);
        i4Warninig |= ((i4FocusDistance > 400) ? 0x4 : 0);

        MY_LOG_IF(m_3ALogEnable & EN_3A_STEREO_LOG,
        "[%s] lv(%d,%d), fd(%d)", __FUNCTION__, i4LvMaster, i4LvSlave, i4FocusDistance);
    }
    return i4Warninig;
}

MVOID
Hal3A::
setAELock(MBOOL bIsAELock)
{
    Param_T rNewParam;
    rNewParam.bIsAELock = bIsAELock;
    if (ENABLE_3A_GENERAL & m_3ACtrlEnable)
    if (ENABLE_AE & m_3ACtrlEnable)
    // APAE lock
    ERROR_CHECK(IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock))
}

MVOID
Hal3A::
setAWBLock(MBOOL bIsAWBLock)
{
    Param_T rNewParam;
    rNewParam.bIsAWBLock = bIsAWBLock;
    if (ENABLE_3A_GENERAL & m_3ACtrlEnable)
    if (ENABLE_AWB & m_3ACtrlEnable)
    // AWB lock
    ERROR_CHECK(IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock))
}
