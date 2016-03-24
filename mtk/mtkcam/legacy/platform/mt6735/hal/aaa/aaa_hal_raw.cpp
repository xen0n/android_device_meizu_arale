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
#define LOG_TAG "Hal3ARaw"

#include <aaa_hal_if.h>
#include <aaa_hal_raw.h>
#include <IEventIrq.h>
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <kd_camera_feature.h>
//#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/hal/IHalSensor.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/StdExif.h>
#include <aaa_state.h>
#include <aaa_state_mgr.h>
#include <af_tuning_custom.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <mcu_drv.h>
#include <flash_awb_param.h>
#include <af_mgr_if.h>
#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <awb_mgr_if.h>
#include <flicker_hal_base.h>
#include <lsc_mgr2.h>

#include <aao_buf_mgr.h>
#include <afo_buf_mgr.h>
#include <aaa_sensor_buf_mgr.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/imageio/ispio_pipe_ports.h>
//#include <aaa_common_custom.h>
#include <sys/errno.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/utils/common.h>
#include <sys/prctl.h>

using namespace NS3Av3;
using namespace NSIspTuningv3;
using namespace android;
using namespace NSCam::Utils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ERROR_CHECK(API)\
    {\
    MRESULT err = API;\
    if (FAILED(err))\
    {\
        MY_ERR(#API);\
        return MFALSE;\
    }}\

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3ARaw*
Hal3ARaw::
createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    MY_LOG("Hal3ARaw::createInstance i4SensorDevId(%d), i4SensorOpenIndex(%d)\n"
           , i4SensorDevId
           , i4SensorOpenIndex);

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            Hal3ARawDev<SENSOR_DEV_MAIN>::getInstance()->initRaw(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ARawDev<SENSOR_DEV_MAIN>::getInstance();
        break;
        case SENSOR_DEV_SUB:
            Hal3ARawDev<SENSOR_DEV_SUB>::getInstance()->initRaw(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ARawDev<SENSOR_DEV_SUB>::getInstance();
        break;
        case SENSOR_DEV_MAIN_2:
            Hal3ARawDev<SENSOR_DEV_MAIN_2>::getInstance()->initRaw(i4SensorDevId, i4SensorOpenIndex);
            return Hal3ARawDev<SENSOR_DEV_MAIN_2>::getInstance();
        break;
        default:
            MY_ERR("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3ARaw::
destroyInstance()
{
    uninitRaw();
}

Hal3ARaw*
Hal3ARaw::
getInstance(MINT32 i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            return Hal3ARawDev<SENSOR_DEV_MAIN>::getInstance();
        case SENSOR_DEV_SUB:
            return Hal3ARawDev<SENSOR_DEV_SUB>::getInstance();
        case SENSOR_DEV_MAIN_2:
            return Hal3ARawDev<SENSOR_DEV_MAIN_2>::getInstance();
        default:
            MY_ERR("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3ARaw::Hal3ARaw()
    : Hal3AFlowCtrl()
    , m_Users(0)
    , m_Lock()
    , m_ValidateLock()
    //, m_errorCode(S_3A_OK)
    //, m_rParam()
    //, m_bReadyToCapture(MFALSE)
    //, m_i4SensorDev(0)
    //, m_bDebugEnable(MFALSE)
    //, m_i4FrmId(0)
    //, m_3ACapMode(ECapMode_P2_Cal|ECapMode_P2_Set)
    //, m_PvMode(EPv_Normal)
    //, m_b3APvInitOK(MFALSE)
    //, mpVSTimer(NULL)
    //, mbEnable3ASetParams(MTRUE)
    //, mbInCaptureProcess(MFALSE)
    //, mpIspDrv(NULL)
    , mbEnAESenThd(MTRUE)
    //, mpIspDrv_forAF(NULL)
    , mbAFThreadLoop(0)
    , mpStateMgr(NULL)
    , m_bFaceDetectEnable(MFALSE)
    , mpAFEventIrq(NULL)
    , mi4InCaptureProcess(0)
    //, mpScheduler(NULL)
    //, m_rResultBuf(6)
    //, m_3ALogEnable(0)
    //, m_3ACtrlEnable(0)

{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3ARaw::~Hal3ARaw()
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3ARaw::
initRaw(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    m_3ALogEnable = atoi(cLogLevel);
    if ( m_3ALogEnable == 0 ) {
        ::property_get("debug.camera.log.hal3a", cLogLevel, "0");
        m_3ALogEnable = atoi(cLogLevel);
    }

    property_get("debug.aaa.ctrl.enable", value, "1023");
    m_3ACtrlEnable = atoi(value);
    MY_LOG("m_3ALogEnable = %d, m_3ACtrlEnable = %d\n", m_3ALogEnable, m_3ACtrlEnable);

    MY_LOG_IF(1,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG_IF(1,"%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_u4FlashSeq = 0;

    init(i4SensorDevId, i4SensorOpenIndex);
    mbEnAESenThd = MTRUE;
    createThreadRaw();
    EnableAFThread(1);

    //mpScheduler = new AAA_Scheduler;
    //mpScheduler should be assigned before creating StateMgr, since IState will copy m_pHal3A->mpScheduler as m_pScheduler in constructor
    mpStateMgr = new StateMgr(i4SensorDevId);

    // init
    bRet = postCommand(ECmd_Init);
    if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");

    //mpIspDrv = IspDrv::createInstance();
    //bRet = mpIspDrv->init(/*"Hal3A"*/);
    if (!bRet) AEE_ASSERT_3A_HAL("IspDrv->init fail.");

if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
    bRet = IspTuningMgr::getInstance().init(m_i4SensorDev, m_i4SensorOpenIdx);
    if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr init fail.");
}

    //ret = EnableAFThread(1);
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
    //FlashMgr::getInstance().setStrobeCallbackFunc(m_i4SensorDev, notifyStrobeMode);
}
}


    MY_LOG_IF(1,"[Hal3ARaw::init] done\n");

    android_atomic_inc(&m_Users);
    return S_3A_OK;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3ARaw::
uninitRaw()
{
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

if (ENABLE_ISPTUNE & m_3ACtrlEnable) {
        bRet = IspTuningMgr::getInstance().uninit(m_i4SensorDev);
        if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr uninit fail.");
}
if (ENABLE_FLICKER & m_3ACtrlEnable) {
        FlickerHalBase::getInstance().releaseBuf(m_i4SensorDev);
}

        //ret = EnableAFThread(0);
        if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(0) fail.");
        bRet = postCommand(ECmd_Uninit);
        if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");

/*
        delete mpScheduler;
        mpScheduler = NULL;
*/
        destroyThreadRaw();
        EnableAFThread(0);
        uninit();

        delete mpStateMgr;
        mpStateMgr = NULL;

        MY_LOG_IF(1,"[Hal3ARaw::uninit] done\n");

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
MBOOL Hal3ARaw::validatePass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf)
{
    mP2Mtx.lock();
    IspTuningMgr::getInstance().validatePerFrameP2(m_i4SensorDev, flowType, rCamInfo, pRegBuf);
    mP2Mtx.unlock();
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::setSensorAndPass1(const ParamIspProfile_T& rParamIspProfile_, MBOOL fgPerframe)
{
    IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, rParamIspProfile_.eIspProfile);
    IspTuningMgr::getInstance().notifyRPGEnable(m_i4SensorDev, rParamIspProfile_.iEnableRPG);
    IspTuningMgr::getInstance().validatePerFrame(m_i4SensorDev, rParamIspProfile_.i4MagicNum, fgPerframe);
    return MTRUE;
}

MVOID
Hal3ARaw::notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    Mutex::Autolock autoLock(m3AOperMtx1);
    m_u4FrmIdStat = u4MagicNum;
    updateImmediateResult(u4MagicNum);
    if (u4MagicNum == mLastFlashOnFrmIdx)
    {
        setFlashLightOnOff(0, 1); // don't care main or pre
        mLastFlashOnFrmIdx = -1;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ARaw::
get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const
{
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_SENSOR_PARAM_T rAESensorInfo;

    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rExifInfo.u4CapExposureTime = rAESensorInfo.u8ExposureTime / 1000;  // naro sec change to micro sec
    if (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev))
        rExifInfo.u4FlashLightTimeus = 30000;
    else
        rExifInfo.u4FlashLightTimeus = 0;

    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev, rDeviceInfo);
    rExifInfo.u4FNumber     = rDeviceInfo.u4LensFno; // Format: F2.8 = 28
    rExifInfo.u4FocalLength = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
    //rExifInfo.u4SceneMode   = m_rParam.u4SceneMode; // Scene mode   (SCENE_MODE_XXX)
    switch (IAeMgr::getInstance().getAEMeterMode(m_i4SensorDev))
    {
    case LIB3A_AE_METERING_MODE_AVERAGE:
        rExifInfo.u4AEMeterMode = eMeteringMode_Average;
        break;
    case LIB3A_AE_METERING_MODE_CENTER_WEIGHT:
        rExifInfo.u4AEMeterMode = eMeteringMode_Center;
        break;
    case LIB3A_AE_METERING_MODE_SOPT:
        rExifInfo.u4AEMeterMode = eMeteringMode_Spot;
        break;
    default:
        rExifInfo.u4AEMeterMode = eMeteringMode_Other;
        break;
    }
    rExifInfo.i4AEExpBias   = IAeMgr::getInstance().getEVCompensateIndex(m_i4SensorDev); // Exposure index  (AE_EV_COMP_XX)
    rExifInfo.u4AEISOSpeed  = rAESensorInfo.u4Sensitivity;

    rExifInfo.u4AWBMode     = (m_rParam.u4AwbMode == MTK_CONTROL_AWB_MODE_AUTO) ? 0 : 1;
    switch (m_rParam.u4AwbMode)
    {
    case MTK_CONTROL_AWB_MODE_AUTO:
    case MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT:
    case MTK_CONTROL_AWB_MODE_TWILIGHT:
    case MTK_CONTROL_AWB_MODE_INCANDESCENT:
        rExifInfo.u4LightSource = eLightSourceId_Other;
        break;
    case MTK_CONTROL_AWB_MODE_DAYLIGHT:
        rExifInfo.u4LightSource = eLightSourceId_Daylight;
        break;
    case MTK_CONTROL_AWB_MODE_FLUORESCENT:
        rExifInfo.u4LightSource = eLightSourceId_Fluorescent;
        break;
#if 0
    case MTK_CONTROL_AWB_MODE_TUNGSTEN:
        rExifInfo.u4LightSource = eLightSourceId_Tungsten;
        break;
#endif
    case MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
        rExifInfo.u4LightSource = eLightSourceId_Cloudy;
        break;
    case MTK_CONTROL_AWB_MODE_SHADE:
        rExifInfo.u4LightSource = eLightSourceId_Shade;
        break;
    default:
        rExifInfo.u4LightSource = eLightSourceId_Other;
        break;
    }

    switch (m_rParam.u4SceneMode)
    {
    case MTK_CONTROL_SCENE_MODE_PORTRAIT:
        rExifInfo.u4ExpProgram = eExpProgramId_Portrait;
        break;
    case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
        rExifInfo.u4ExpProgram = eExpProgramId_Landscape;
        break;
    default:
        rExifInfo.u4ExpProgram = eExpProgramId_NotDefined;
        break;
    }

    switch (m_rParam.u4SceneMode)
    {
    case MTK_CONTROL_SCENE_MODE_DISABLED:
    case MTK_CONTROL_SCENE_MODE_NORMAL:
    case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
    case MTK_CONTROL_SCENE_MODE_THEATRE:
    case MTK_CONTROL_SCENE_MODE_BEACH:
    case MTK_CONTROL_SCENE_MODE_SNOW:
    case MTK_CONTROL_SCENE_MODE_SUNSET:
    case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
    case MTK_CONTROL_SCENE_MODE_FIREWORKS:
    case MTK_CONTROL_SCENE_MODE_SPORTS:
    case MTK_CONTROL_SCENE_MODE_PARTY:
    case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
        rExifInfo.u4SceneCapType = eCapTypeId_Standard;
        break;
    case MTK_CONTROL_SCENE_MODE_PORTRAIT:
        rExifInfo.u4SceneCapType = eCapTypeId_Portrait;
        break;
    case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
        rExifInfo.u4SceneCapType = eCapTypeId_Landscape;
        break;
    case MTK_CONTROL_SCENE_MODE_NIGHT:
        rExifInfo.u4SceneCapType = eCapTypeId_Night;
        break;
    default:
        rExifInfo.u4SceneCapType = eCapTypeId_Standard;
        break;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3ARaw::
getDebugInfo(android::Vector<MINT32>& keys, android::Vector< android::Vector<MUINT8> >& data) const
{
    MY_LOG_IF(m_3ALogEnable, "[%s]", __FUNCTION__);

    MUINT8* pBuf = NULL;

    keys.resize(3);
    data.resize(3);

    MINT32* pKeys = keys.editArray();
    Vector<MUINT8>* pData = data.editArray();
    // ======================== allocate buffers of IDbgInfoContainer ==============================
    pKeys[0] = AAA_DEBUG_KEYID;
    pData[0].resize(sizeof(AAA_DEBUG_INFO_T));
    pBuf = pData[0].editArray();
    if (!pBuf)
    {
        MY_ERR("[%s] alloc AAA_DEBUG_INFO_T fail", __FUNCTION__);
        return MFALSE;
    }
    AAA_DEBUG_INFO_T& r3ADebugInfo = *(AAA_DEBUG_INFO_T*) pBuf;

    pBuf = NULL;
    pKeys[1] = DEBUG_CAM_RESERVE1_MID;
    pData[1].resize(sizeof(DEBUG_RESERVEA_INFO_T));
    pBuf = pData[1].editArray();
    if (!pBuf)
    {
        MY_ERR("[%s] alloc DEBUG_RESERVEA_INFO_T fail", __FUNCTION__);
        return MFALSE;
    }
    SHADING_DEBUG_INFO_T& rShadigDebugInfo = *(SHADING_DEBUG_INFO_T*) pBuf;

    // ======================== start to fill in IDbgInfoContainer ==================================
    // 3A debug info
    //static AAA_DEBUG_INFO_T r3ADebugInfo;

    r3ADebugInfo.hdr.u4KeyID = AAA_DEBUG_KEYID;
    r3ADebugInfo.hdr.u4ModuleCount = MODULE_NUM(6,5);

    r3ADebugInfo.hdr.u4AEDebugInfoOffset        = sizeof(r3ADebugInfo.hdr);
    r3ADebugInfo.hdr.u4AFDebugInfoOffset        = r3ADebugInfo.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugInfoOffset       = r3ADebugInfo.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlashDebugInfoOffset     = r3ADebugInfo.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlickerDebugInfoOffset   = r3ADebugInfo.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugDataOffset       = r3ADebugInfo.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);

    AE_PLINE_DEBUG_INFO_T tmpBuff;
    // AE
    IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAEDebugInfo, tmpBuff);

    // AF
    IAfMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAFDebugInfo);

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAWBDebugInfo, r3ADebugInfo.rAWBDebugData);

    // Flash
    FlashMgr::getInstance().getDebugInfo(m_i4SensorDev, &r3ADebugInfo.rFlashDebugInfo);

    // Flicker
    //Flicker::getInstance()->getDebugInfo(&r3ADebugInfo.rFlickerDebugInfo);

    // LSC
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugInfo(rShadigDebugInfo);
    //LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugTbl(rShadingDbgTbl, r3ADebugInfo.rShadingRestTbl);

    // ISP debug info
    pBuf = NULL;
    pData[2].resize(sizeof(NSIspExifDebug::IspExifDebugInfo_T));
    pBuf = pData[2].editArray();
    if (!pBuf)
    {
        MY_ERR("[%s] alloc NSIspExifDebug::IspExifDebugInfo_T fail", __FUNCTION__);
        return MFALSE;
    }
    NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo = *(NSIspExifDebug::IspExifDebugInfo_T*) pBuf;
    IspTuningMgr::getInstance().getDebugInfo(m_i4SensorDev, rIspExifDebugInfo, MFALSE);
    pKeys[2] = rIspExifDebugInfo.hdr.u4KeyID;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3ARaw::
getCurrResult(MUINT32 u4FrmId, Result_T& rResult) const
{
    MY_LOG_IF(m_3ALogEnable, "[%s] i4MagicNum(%d)", __FUNCTION__, u4FrmId);
    mtk_camera_metadata_enum_android_control_awb_state_t eAwbState;
    rResult.i4FrmId = u4FrmId;

    // clear memory
    rResult.vecShadingMap.clear();
    rResult.vecExifInfo.clear();
    rResult.vecTonemapCurveRed.clear();
    rResult.vecTonemapCurveGreen.clear();
    rResult.vecTonemapCurveBlue.clear();
    rResult.vecColorCorrectMat.clear();

    rResult.u1SceneMode = m_rParam.u4SceneMode;

    // AE
    MUINT8 u1AeState = IAeMgr::getInstance().getAEState(m_i4SensorDev);
    if (IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev) && u1AeState == MTK_CONTROL_AE_STATE_CONVERGED)
        rResult.u1AeState = MTK_CONTROL_AE_STATE_FLASH_REQUIRED;
    else
        rResult.u1AeState = u1AeState;

    AE_SENSOR_PARAM_T rAESensorInfo;
    IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
    rResult.i8SensorExposureTime = rAESensorInfo.u8ExposureTime;
    rResult.i8SensorFrameDuration = rAESensorInfo.u8FrameDuration;
    rResult.i4SensorSensitivity = rAESensorInfo.u4Sensitivity;
    rResult.i8SensorRollingShutterSkew = IAeMgr::getInstance().getSensorRollingShutter(m_i4SensorDev);

    // AWB
    IAwbMgr::getInstance().getAWBState(m_i4SensorDev, eAwbState);
    rResult.u1AwbState= eAwbState;
    AWB_GAIN_T rAwbGain;
    IAwbMgr::getInstance().getAWBGain(m_i4SensorDev, rAwbGain, rResult.i4AwbGainScaleUint);
    rResult.i4AwbGain[0] = rAwbGain.i4R;
    rResult.i4AwbGain[1] = rAwbGain.i4G;
    rResult.i4AwbGain[2] = rAwbGain.i4B;
    IAwbMgr::getInstance().getColorCorrectionGain(m_i4SensorDev, rResult.fColorCorrectGain[0],rResult.fColorCorrectGain[1],rResult.fColorCorrectGain[3]);
    rResult.fColorCorrectGain[2] = rResult.fColorCorrectGain[1];
    if (m_rParam.u1ColorCorrectMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        rResult.vecColorCorrectMat.resize(9);
        MFLOAT* pfColorCorrectMat = rResult.vecColorCorrectMat.editArray();
        IspTuningMgr::getInstance().getColorCorrectionTransform(m_i4SensorDev,
            pfColorCorrectMat[0], pfColorCorrectMat[1], pfColorCorrectMat[2],
            pfColorCorrectMat[3], pfColorCorrectMat[4], pfColorCorrectMat[5],
            pfColorCorrectMat[6], pfColorCorrectMat[7], pfColorCorrectMat[8]
        );
    }

    // AF
    rResult.u1AfState = IAfMgr::getInstance().getAFState(m_i4SensorDev);
    if (mpStateMgr->getStateStatus().eCurrState == eState_AF)
        rResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
    rResult.u1LensState = IAfMgr::getInstance().getLensState(m_i4SensorDev);
    rResult.fLensFocusDistance = IAfMgr::getInstance().getFocusDistance(m_i4SensorDev);
    IAfMgr::getInstance().getFocusRange(m_i4SensorDev, &rResult.fLensFocusRange[0], &rResult.fLensFocusRange[1]);

    // Flash
    //rResult.u1FlashState = FlashMgr::getInstance()->getFlashState();
#if 1 //mark out temporarily
    rResult.u1FlashState =
        (FlashMgr::getInstance().getFlashState(m_i4SensorDev) == MTK_FLASH_STATE_UNAVAILABLE) ?
        MTK_FLASH_STATE_UNAVAILABLE :
        (FlashMgr::getInstance().isAFLampOn(m_i4SensorDev) ? MTK_FLASH_STATE_FIRED : MTK_FLASH_STATE_READY);
#endif
    // Flicker
    MINT32 i4FlkStatus = IAeMgr::getInstance().getAEAutoFlickerState(m_i4SensorDev);
    //FlickerHalBase::getInstance().getFlickerResult(i4FlkStatus);
    MUINT8 u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_NONE;
    if (i4FlkStatus == 0) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_50HZ;
    if (i4FlkStatus == 1) u1ScnFlk = MTK_STATISTICS_SCENE_FLICKER_60HZ;
    rResult.u1SceneFlk = (MUINT8)u1ScnFlk;

    // Shading map
    if (m_rParam.u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON)
    {
        #warning "Shading map hardcode"
        rResult.vecShadingMap.resize(1*1*4);
        MFLOAT* pfShadingMap = rResult.vecShadingMap.editArray();
        NSIspTuningv3::LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getGainTable(3, 1, 1, pfShadingMap);
    }

    // Tonemap
    if (m_rParam.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveBlue.push_back(*pIn++);
            rResult.vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveGreen.push_back(*pIn++);
            rResult.vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            rResult.vecTonemapCurveRed.push_back(*pIn++);
            rResult.vecTonemapCurveRed.push_back(*pOut++);
        }
    }

    // Cam Info
    RAWIspCamInfo rCamInfo;
    if (!IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rCamInfo))
    {
        MY_ERR("Fail to get CamInfo");
    }
    UtilConvertCamInfo(rCamInfo, rResult.rCamInfo);

    // Exif
    if (m_rParam.u1IsGetExif)
    {
        rResult.vecExifInfo.resize(1);
        get3AEXIFInfo(rResult.vecExifInfo.editTop());
    }

    return 0;
}

MVOID
Hal3ARaw::
updateResult(MUINT32 u4MagicNum)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] #(%d)", __FUNCTION__, u4MagicNum);
    // pass1 result
    getCurrResult(u4MagicNum, m_rResult);
    m_rResultBuf.updateResult(u4MagicNum, m_rResult);
    // result for high quality
    m_rResultBufCur.updateResult(m_u4FrmIdStat, m_rResult);
/*
    MUINT8 u1FlashState;
    if (m_rResult.u1FlashState != MTK_FLASH_STATE_UNAVAILABLE)
    {
        Result_T* pResult = NULL;
        MUINT32 u4FlashSeq = m_u4FlashSeq;
        u4FlashSeq = (u4FlashSeq << 1) | (m_rResult.u1FlashState == MTK_FLASH_STATE_FIRED ? 1 : 0);
        u1FlashState = ((u4FlashSeq & 0x7) == 0x7 ? MTK_FLASH_STATE_FIRED :
            ((u4FlashSeq & 0x7) == 0x0 ? MTK_FLASH_STATE_READY : MTK_FLASH_STATE_PARTIAL));
        m_u4FlashSeq = u4FlashSeq;
        if (u1FlashState == MTK_FLASH_STATE_PARTIAL)
        {
            pResult = m_rResultBuf.getResult(u4MagicNum-2);
            if (pResult)
            {
                pResult->u1FlashState = u1FlashState;
            }
        }
        MY_LOG("[%s] #(%d), pResult(%p), Flash(%d), FlashSeq(0x%08x)", __FUNCTION__, u4MagicNum, pResult, u1FlashState, u4FlashSeq);
    }
*/
}

MVOID
Hal3ARaw::
updateImmediateResult(MUINT32 u4MagicNum)
{
/*
    Result_T* pResult = m_rResultBuf.getResult(u4MagicNum);

    if (pResult)
    {
        if (pResult->u1FlashState == MTK_FLASH_STATE_FIRED &&
            !FlashMgr::getInstance()->isAFLampOn())
        {
            MY_LOG("[%s] #(%d) Override to partial", __FUNCTION__, u4MagicNum);
            pResult->u1FlashState = MTK_FLASH_STATE_PARTIAL;
        }
    }
*/
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::postCommandDerived(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] enter, eCmd(%d)\n", eCmd);

    if (eCmd == ECmd_CameraPreviewStart)
    {
        updateTGInfo();
        IspTuningMgr::getInstance().validate(m_i4SensorDev, MTRUE);
    }

    if (eCmd == ECmd_CameraPreviewEnd)
    {
        MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "3A PreviewEnd, start wait 3A sub-threads");
        //Once 3A receive PreviewEnd, 3A cannot return until AF/AESen thread also gets AFirq/finish I2C in this frame

        Mutex::Autolock autoLock1(m3AOperMtx1); //FIXME: lock P1done and PreviewEnd
          Mutex::Autolock autoLock2(m3AOperMtx2); //FIXME: lock setParams and PreviewEnd

        MBOOL bRet;
        //break wait when it become 1, i.e., AF thread finish and post
        bRet = sem_wait_relativeTime(&semAFProcEnd, SEM_TIME_LIMIT_NS, "postCommand::semAFProcEnd");
        if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAFProcEnd).");
        //bRet = sem_wait_relativeTime(&semAESenThdEnd, SEM_TIME_LIMIT_NS, "postCommand::semAESenThdEnd");
        //if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAESenThdEnd).");

        MY_LOG("3A PreviewEnd, wait 3A sub-threads done");

        //FIXME: copy from the code below
        MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] before StateMgr::sendCmd(%d)\n", eCmd);
        //if (mbMainFlashOnThisFrame && (eCmd == ECmd_Update)) mpStateMgr->sendCmd(ECmd_CaptureStart);
        //else
        mpStateMgr->sendCmd(eCmd);
        MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] after StateMgr::sendCmd(%d)\n", eCmd);

        MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] exit, eCmd(%d)\n", eCmd);
        return MTRUE;
    }

    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] before StateMgr::sendCmd(%d)\n", eCmd);
    if (mbMainFlashOnThisFrame && (eCmd == ECmd_Update) && (m_rParam.u4AeMode != MTK_CONTROL_AE_MODE_OFF))
    {
        if (mpStateMgr->getStateStatus().eCurrState == eState_Precapture)
        {
            MY_ERR("Unexpected Operation since precapture is not finished.");
        }
        else
        {
            mi4InCaptureProcess = 1;
            FlashMgr::getInstance().capCheckAndFireFlash_Start(m_i4SensorDev);
            mpStateMgr->sendCmd(ECmd_CaptureStart);
        }
    }
    else mpStateMgr->sendCmd(eCmd);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] after StateMgr::sendCmd(%d)\n", eCmd);

    if (eCmd == ECmd_Update)
    {
        if (mpStateMgr->getFrameCount() >= 0)
        {
            FlashMgr::getInstance().doPreviewOneFrame(m_i4SensorDev);

            #if 0//def MTK_SLOW_MOTION_VIDEO
            g_AFMinFps = m_rParam.i4MinFps/1000;
            #endif
            {
                int Val;
                MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] start waiting mAFMtx lock\n");
                Mutex::Autolock autoLock(mAFMtx); //lock for critical section
                MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] get mAFMtx lock\n");
                if (mbSemAFIRQWait == 0)
                {
                    //AF thread start wait AF irq
                    ::sem_getvalue(&semAFProcEnd, &Val);
                    if (Val == 1) ::sem_wait(&semAFProcEnd); //to be 0, it won't block
                    ::sem_post(&mSemAFThreadstart);
                    MY_LOG_IF(m_3ALogEnable, "[AFThread] sem_post mSemAFThreadstart, semAFProcEnd before wait = %d\n", Val);
                }
            }

            const ParamIspProfile_T* pParamIspProf = reinterpret_cast<const ParamIspProfile_T*>(i4Arg);
            //ToDo: add fence check mechanism before validatePass1 and sensor
            {
                AaaTimer rTmVld;
                rTmVld.start("setSensorAndPass1", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                setSensorAndPass1(*pParamIspProf, MTRUE);
                rTmVld.End();
            }

            if (mbPostAESenThd)
            {
                MBOOL bRet = sem_wait_relativeTime(&semAESenThdEnd, SEM_TIME_LIMIT_NS, "postCommand::semAESenThdEnd");
                if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(semAESenThdEnd).");
                mbPostAESenThd = MFALSE;
            }
        }
#if 0
        // Each update requires one callback
        //if (mpStateMgr->getFrameCount() >= 0)
        {
            {
                AaaTimer rTmProcFinish;
                rTmProcFinish.start("aaa.on3AProcFinish", m_i4SensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                on3AProcFinish(pParamIspProf->i4MagicNum);
                rTmProcFinish.End();
            }
        }
#endif
    }
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3ARaw::postCommand] exit, eCmd(%d)\n", eCmd);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::setParams(Param_T const &rNewParam)
{
    Mutex::Autolock autoLock(m3AOperMtx2);

    if (!mbEnable3ASetParams) return MTRUE;

    // ====================================== Shading =============================================
    NSIspTuningv3::LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(rNewParam.u1ShadingMode ? MTRUE : MFALSE);


    // ====================================== ISP tuning =============================================
    IspTuningMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    IspTuningMgr::getInstance().setEffect(m_i4SensorDev, rNewParam.u4EffectMode);
    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewParam.u4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewParam.u4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewParam.u4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewParam.u4EdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewParam.u4ContrastMode);
    //IspTuningMgr::getInstance().setCamMode(rNewParam.u4CamMode);
    IspTuningMgr::getInstance().setEdgeMode(m_i4SensorDev, rNewParam.u1EdgeMode ? MTK_EDGE_MODE_FAST : MTK_EDGE_MODE_OFF);
    IspTuningMgr::getInstance().setNoiseReductionMode(m_i4SensorDev, rNewParam.u1NRMode);
    IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewParam.u1TonemapMode);
    if (rNewParam.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        android::Vector<MFLOAT> vecIn, vecOut;
        MINT32 i4Cnt = rNewParam.vecTonemapCurveBlue.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        MFLOAT* pArrayIn = vecIn.editArray();
        MFLOAT* pArrayOut = vecOut.editArray();
        const MFLOAT* pCurve = rNewParam.vecTonemapCurveBlue.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            MY_LOG_IF(m_3ALogEnable, "[Blue]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewParam.vecTonemapCurveGreen.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewParam.vecTonemapCurveGreen.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            MY_LOG_IF(m_3ALogEnable, "[Green]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Green(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);

        i4Cnt = rNewParam.vecTonemapCurveRed.size() / 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = vecIn.editArray();
        pArrayOut = vecOut.editArray();
        pCurve = rNewParam.vecTonemapCurveRed.array();
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            MY_LOG_IF(m_3ALogEnable, "[Red]#%d(%f,%f)", rNewParam.u4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Red(m_i4SensorDev, vecIn.editArray(), vecOut.editArray(), &i4Cnt);
    }

    // ====================================== AE ==============================================
    IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps);
    IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev, rNewParam.u4AeMeterMode);
    IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev, rNewParam.i4RotateDegree);
    IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev, rNewParam.u4IsoSpeedMode);
    IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev, &rNewParam.rMeteringAreas);
    IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock);
    IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
    IAeMgr::getInstance().setAEMode(m_i4SensorDev, rNewParam.u4AeMode);
    IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);
    IAeMgr::getInstance().setAECamMode(m_i4SensorDev, rNewParam.u4CamMode);
    IAeMgr::getInstance().setAEShotMode(m_i4SensorDev, rNewParam.u4ShotMode);
    IAeMgr::getInstance().setSceneMode(m_i4SensorDev, rNewParam.u4SceneMode);
    IAeMgr::getInstance().bBlackLevelLock(m_i4SensorDev, rNewParam.u1BlackLvlLock);
    if (rNewParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        AE_SENSOR_PARAM_T strSensorParams;
        strSensorParams.u4Sensitivity   = rNewParam.i4Sensitivity;
        strSensorParams.u8ExposureTime  = rNewParam.i8ExposureTime;
        strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
        IAeMgr::getInstance().UpdateSensorParams(m_i4SensorDev, strSensorParams);
    }

    // ====================================== AWB ==============================================
    IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock);
    IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rNewParam.u4AwbMode);
    IAwbMgr::getInstance().setColorCorrectionMode(m_i4SensorDev, rNewParam.u1ColorCorrectMode);
    IspTuningMgr::getInstance().setColorCorrectionMode(m_i4SensorDev, rNewParam.u1ColorCorrectMode);
    if (rNewParam.u4AwbMode == MTK_CONTROL_AWB_MODE_OFF &&
        rNewParam.u1ColorCorrectMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        IAwbMgr::getInstance().setColorCorrectionGain(m_i4SensorDev, rNewParam.fColorCorrectGain[0], rNewParam.fColorCorrectGain[1], rNewParam.fColorCorrectGain[3]);
        IspTuningMgr::getInstance().setColorCorrectionTransform(m_i4SensorDev,
            rNewParam.fColorCorrectMat[0], rNewParam.fColorCorrectMat[1], rNewParam.fColorCorrectMat[2],
            rNewParam.fColorCorrectMat[3], rNewParam.fColorCorrectMat[4], rNewParam.fColorCorrectMat[5],
            rNewParam.fColorCorrectMat[6], rNewParam.fColorCorrectMat[7], rNewParam.fColorCorrectMat[8]);
    }

    // ====================================== AF ==============================================
    IAfMgr::getInstance().setAFMode(m_i4SensorDev, rNewParam.u4AfMode);
    if (m_bFaceDetectEnable == MFALSE) IAfMgr::getInstance().setAFArea(m_i4SensorDev, rNewParam.rFocusAreas);
    //AfMgr::getInstance().setCamMode(rNewParam.u4CamMode);
    IAfMgr::getInstance().setFullScanstep(m_i4SensorDev, rNewParam.i4FullScanStep);
    IAfMgr::getInstance().setMFPos(m_i4SensorDev, rNewParam.i4MFPos);
    IAfMgr::getInstance().setAndroidServiceState(m_i4SensorDev, rNewParam.bIsSupportAndroidService);
    if (rNewParam.u4AfMode == MTK_CONTROL_AF_MODE_OFF)
    {
        // focus dist
        IAfMgr::getInstance().setFocusDistance(m_i4SensorDev, rNewParam.fFocusDistance);
    }

    // ====================================== Flash ==============================================
    FlashMgr::getInstance().setAeFlashMode(m_i4SensorDev, rNewParam.u4AeMode, rNewParam.u4StrobeMode);
    int bMulti;
    if(rNewParam.u4CapType == ECapType_MultiCapture)
     bMulti=1;
    else
     bMulti=0;
    //FlashMgr::getInstance().setMultiCapture(bMulti);
    FlashMgr::getInstance().setCamMode(m_i4SensorDev, rNewParam.u4CamMode);
    FlashMgr::getInstance().setEvComp(m_i4SensorDev, rNewParam.i4ExpIndex, rNewParam.fExpCompStep);
/*
    i4PreFlashDuty
    i4PreFlashStep
    i4MainFlashDuty
    i4MainFlashStep
*/

    // ====================================== Flicker ==============================================
    FlickerHalBase::getInstance().setFlickerMode(m_i4SensorDev, rNewParam.u4AntiBandingMode);


    // ====================================== FlowCtrl ==============================================


    m_rParam = rNewParam;

    MY_LOG_IF(m_3ALogEnable, "[%s] m_rParam.u1ShadingMapMode(%d)", __FUNCTION__, m_rParam.u1ShadingMapMode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3ARaw::setFDEnable(MBOOL fgEnable)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    m_bFaceDetectEnable = fgEnable;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::setFDInfo(MVOID* prFaces)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if (m_bFaceDetectEnable)
    {
        IAfMgr::getInstance().setFDInfo(m_i4SensorDev, prFaces);
        IAeMgr::getInstance().setFDInfo(m_i4SensorDev, prFaces);
    }
}
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::autoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        mpStateMgr->sendCmd(ECmd_AFStart);
    }
    IAfMgr::getInstance().autoFocus(m_i4SensorDev);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::cancelAutoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        mpStateMgr->sendCmd(ECmd_AFEnd);
    }
    IAfMgr::getInstance().cancelAutoFocus(m_i4SensorDev);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MBOOL Hal3ARaw::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
//{
//    return MTRUE;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MBOOL Hal3ARaw::setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const
//{
//    return MTRUE;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3ARaw::getDelayFrame(EQueryType_T const eQueryType) const
{
    return 0;
}

MINT32
Hal3ARaw::
attachCb(I3ACallBack* cb)
{
    MY_LOG("[%s] mpCbSet(0x%08x), cb(0x%08x)", __FUNCTION__, mpCbSet, cb);
    mpCbSet = cb;
    IAfMgr::getInstance().setCallbacks(m_i4SensorDev, mpCbSet);
    return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::setIspPass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf)
{
    MBOOL res = validatePass2(flowType, rCamInfo, pRegBuf);
//    if (pDbgInfoCtnr)
//        setDebugInfo(pDbgInfoCtnr, MFALSE); //read from working buffer

    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3ARaw::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    return S_3A_OK;
}

MVOID Hal3ARaw::setSensorMode(MINT32 i4SensorMode)
{
    MY_LOG("[%s]+ m_i4SensorDev(%d), i4SensorMode(%d)", __FUNCTION__, m_i4SensorDev, i4SensorMode);
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IAeMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IspTuningMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    FlickerHalBase::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
}
    IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, NSIspTuning::EIspProfile_Preview);
//    IspTuningMgr::getInstance().validate(m_i4SensorDev, MTRUE);
    MY_LOG("[%s]-", __FUNCTION__);
}

MBOOL Hal3ARaw::isStrobeBVTrigger()
{
    return IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev);
}

MBOOL Hal3ARaw::setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/)
{
    if (!bOnOff)
    {
        if (mi4InCaptureProcess)
        {
            //modified to update strobe state after capture for ae/flash manager
            FlashMgr::getInstance().cameraPreviewStart(m_i4SensorDev);
            IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, MFALSE);
            mi4InCaptureProcess = 0;
        }
        FlashMgr::getInstance().turnOffFlashDevice(m_i4SensorDev);
    }
    else //flash on
        if (bMainPre) FlashMgr::getInstance().setCaptureFlashOnOff(m_i4SensorDev, 1);
        else FlashMgr::getInstance().setTorchOnOff(m_i4SensorDev, 1);

    return MTRUE;
}

MINT32 Hal3ARaw::getCurrentHwId() const
{
    MUINT32 u4Id = 0;
    NSIoPipe::NSCamIOPipe::IHalCamIO* pPipe = (NSIoPipe::NSCamIOPipe::IHalCamIO*)NSIoPipe::NSCamIOPipe::INormalPipe_FrmB::createInstance(m_i4SensorOpenIdx, LOG_TAG, 1);
    MBOOL fgRet = pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_CUR_SOF_IDX, (MINTPTR)&u4Id, 0, 0);
    pPipe->destroyInstance(LOG_TAG);
    return u4Id;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3ARaw::updateTGInfo()
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

    MINT32 i4EventTg = (m_TgInfo == CAM_TG_1) ? IEventIrq::E_TG1 : IEventIrq::E_TG2;
    mpIEventIrq->setTgInfo(i4EventTg);
    mpAFEventIrq->setTgInfo(i4EventTg);

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3ARaw::getASDInfo(ASDInfo_T &a_rASDInfo) const
{
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

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
Hal3ARaw::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2)
{
    MINT32 i4Ret = 0;
    switch (e3ACtrl)
    {
    case E3ACtrl_ConvertToIspGamma:
        {
            E3ACtrl_ConvertToIspGamma_ARG_T* pArg = reinterpret_cast<E3ACtrl_ConvertToIspGamma_ARG_T*>(i4Arg1);
            if (pArg)
            {
                i4Ret = IspTuningMgr::getInstance().convertPtPairsToGMA(m_i4SensorDev, pArg->pPtPairs, pArg->u4NumPts, pArg->pOutGMA);
            }
        }
        break;

    case E3ACtrl_GetAEPLineTable:
        i4Ret = IAeMgr::getInstance().getAEPlineTable(m_i4SensorDev, static_cast<eAETableID>(i4Arg1),  *reinterpret_cast<strAETable*>(i4Arg2));
        break;
    case E3ACtrl_GetExposureInfo:
        i4Ret = IAeMgr::getInstance().getExposureInfo(m_i4SensorDev, *reinterpret_cast<ExpSettingParam_T*>(i4Arg1));
        break;

    case E3ACtrl_GetAsdInfo:
        i4Ret = getASDInfo(*reinterpret_cast<ASDInfo_T*>(i4Arg1));
        break;
    default:
        return -1;
    }

    return i4Ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3ARaw::EnableAFThread(MINT32 a_bEnable)
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
/*
            mpIspDrv_forAF = IspDrv::createInstance();

            if (!mpIspDrv_forAF) {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (mpIspDrv_forAF->init("Hal3AAFThd") < 0) {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }
*/
            IEventIrq::ConfigParam IrqConfig(m_i4SensorDev, 0, IEventIrq::E_Event_Af);
            mpAFEventIrq = IEventIrq::createInstance(IrqConfig, "AFIrq");
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
/*
            if (mpIspDrv_forAF)   {
                mpIspDrv_forAF->uninit("Hal3AAFThd");
                mpIspDrv_forAF = NULL;
            }
*/

            mbAFThreadLoop = 0;
            mpAFEventIrq->destroyInstance("AFIrq");
            mpAFEventIrq = NULL;
            ::sem_post(&mSemAFThreadstart);

            pthread_join(mAFThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

}
}
    return ret;
}

MVOID * Hal3ARaw::AFThreadFunc(void *arg)
{

    MY_LOG("[AFThread] tid: %d \n", gettid());
    Hal3ARaw *_this = reinterpret_cast<Hal3ARaw*>(arg);
    IEventIrq::Duration duration;
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
/*
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
*/
    while (_this->mbAFThreadLoop) {
        //
        MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (1)\n");
        {Mutex::Autolock autoLock(_this->mAFMtx); _this->mbSemAFIRQWait = 0;}
        MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (1)\n");


        ::sem_wait(&_this->mSemAFThreadstart);
        MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] sem_wait mSemAFThreadstart done\n");

        {Mutex::Autolock autoLock(_this->mAFMtx); _this->mbSemAFIRQWait = 1;}
        MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (2)\n");

/*
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
*/

        #if 0//def MTK_SLOW_MOTION_VIDEO
        if(i4preMinfps != g_AFMinFps)
        {
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc]AF frame rate %d\n",g_AFMinFps);
            i4preMinfps = g_AFMinFps;
            if(g_AFMinFps>33)
                i4SkipAFframe = (g_AFMinFps-1)/30;
        }

        if(i4AFfrmcount)
        {
            i4AFfrmcount--;
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] skip AF frame %d\n", i4AFfrmcount);
        }
        else
        {
            i4AFfrmcount=i4SkipAFframe;
        #endif
        if (_this->mbAFThreadLoop != 0)
        {
            IAfMgr::getInstance().DoCallback(_this->m_i4SensorDev);
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");
            if ( _this->mpAFEventIrq->wait(duration) >= 0) // success
            {
                MY_LOG_IF(_this->m_3ALogEnable, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
                //_this->mpAFEventIrq->mark();
                _this->mpStateMgr->sendCmd(ECmd_AFUpdate);

                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");
            }
            else
            {
                MY_LOG("[AFThread] AF irq timeout or error\n");
                IAfMgr::getInstance().TimeOutHandle(_this->m_i4SensorDev);
                /*need modification*///IAfMgr::getInstance().printAFConfigLog0(_this->m_i4SensorDev);
            }

            {
                int Val2;
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (3)\n");
                Mutex::Autolock autoLock(_this->mAFMtx);
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (3)\n");

                ::sem_getvalue(&_this->semAFProcEnd, &Val2);
                MY_LOG_IF(_this->m_3ALogEnable, "[AFThread] semAFProcEnd before post = %d\n", Val2);
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
