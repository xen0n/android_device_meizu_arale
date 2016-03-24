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
#include <aaa_state.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
//#include "isp_tuning.h"
#include <awb_mgr_if.h>
#include <camera_feature.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv_common/isp_reg.h>
//#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>
#include <af_mgr_if.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
//#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/common.h>
using namespace NSCam;

#include <ae_mgr_if.h>
#include <flash_tuning_custom.h>
#include <flash_mgr.h>
#include <lsc_mgr2.h>
#include <mtkcam/featureio/flicker_hal_base.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include <mtkcam/Modes.h>
using namespace CPTool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
MINT32        g_bAFThreadLoop = 0;
MINT32        g_semAFIRQWait = 0;

pthread_t     g_AFThread;
pthread_t     g_AFNotifyThread;
sem_t         g_semAFThreadstart;
IspDrv*       g_pIspDrv;

using namespace NS3A;
using namespace NSIspTuning;

MINT32 Hal3A::sm_3ALogEnable = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1
#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}
#else
#define ERROR_CHECK(API)\
  {API;}
#endif

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
Hal3A*
Hal3A::
createInstance(MINT32 const i4SensorDevId)
{
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
            AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
        break;
    }

    return pHal3A;
}

Hal3A*
Hal3A::
getInstance()
{
    static Hal3A singleton;

    return &singleton;
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
    : Hal3ABase()
    , m_Users(0)
    , m_Lock()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    , mpFlickerHal(NULL)
    , m_bFaceDetectEnable(MFALSE)

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
init(MINT32 i4SensorDev)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa_hal.enable", value, "0");
    m_bDebugEnable = atoi(value);
    property_get("debug.aaa.pvlog.enable", value, "0");
    IState::sm_3APvLogEnable = sm_3ALogEnable = atoi(value);
    //MY_LOG("IState::sm_3APvLogEnable = %d\n", IState::sm_3APvLogEnable);

    MY_LOG_IF(m_bDebugEnable,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG_IF(m_bDebugEnable,"%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_i4SensorDev = i4SensorDev;
    bAfNeedSingleNotify = MFALSE;

    MY_LOG_IF(m_bDebugEnable,"m_i4SensorDev: %d \n", m_i4SensorDev);

    // init
    bRet = postCommand(ECmd_Init);
    if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");
    mpIspDrv = IspDrv::createInstance();
    bRet = mpIspDrv->init();
    if (!bRet) AEE_ASSERT_3A_HAL("IspDrv->init fail.");



    createThread();
    ret = EnableAFThread(1);
    if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(1) fail.");
/*
    //flicker create buf
    FlickerHalBase *pFlk;
    pFlk = FlickerHalBase::getInstance();
    pFlk->createBuf();
*/
        FlickerHalBase::getInstance().createBuf(getSensorDev());
    //
    bRet = IspTuningMgr::getInstance().init(getSensorDev());
    if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr init fail.");

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
    MY_LOG_IF(m_bDebugEnable,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

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
        if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(0) fail.");
        bRet = postCommand(ECmd_Uninit);
        if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");
        if ( mpIspDrv != NULL )
        {
            mpIspDrv->uninit();
            mpIspDrv=NULL;
        }
        bRet = IspTuningMgr::getInstance().uninit(getSensorDev());
        if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr uninit fail.");

        //flicker release buf
        FlickerHalBase::getInstance().releaseBuf(getSensorDev());
    }
    else    // There are still some users.
    {
        MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
    }

    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::postCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    MBOOL fgLogEn = sm_3ALogEnable & EN_3A_FLOW_LOG;
    MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] enter, eCmd = %d\n", eCmd);
/*
    if (eCmd == ECmd_CameraPreviewStart || eCmd == ECmd_CamcorderPreviewStart)  {
        //EnableAFThread(1);
        mpFlickerHal = &(FlickerHalBase::getInstance());
        if    ( mpFlickerHal->open(m_i4SensorDev,0,0)!=0 )
         {
             MY_ERR("[Hal3A::sendCommand-init] NULL mpFlickerHal");
         }

    }
    else if  (eCmd == ECmd_PrecaptureStart || eCmd == ECmd_CameraPreviewEnd || eCmd == ECmd_CamcorderPreviewEnd)
    {
        //EnableAFThread(0);

        if(mpFlickerHal!=NULL) //&&(eCmd != ECmd_PrecaptureStart) )
            {


                ERROR_CHECK(mpFlickerHal->close(m_i4SensorDev));
                mpFlickerHal=NULL;
            }
    }
*/
    if ((eCmd == ECmd_CameraPreviewStart) || (eCmd == ECmd_CamcorderPreviewStart))
    {
        IspTuningMgr::getInstance().setFlashDynamicInfo(m_i4SensorDev, MFALSE); //reset status for flash CCM/PCA
    }
    if ((eCmd == ECmd_CameraPreviewEnd) || (eCmd == ECmd_CamcorderPreviewEnd))
    {
        MY_LOG_IF(fgLogEn, "3A PreviewEnd, start wait semAFProcEnd");
        //Once 3A receive PreviewEnd, 3A cannot return until AF thread also gets AFirq and finish this frame
        MY_LOG_IF(fgLogEn, "[Hal3A::postCommand] start waiting semAFProcEnd semaphore\n");
        ::sem_wait(&semAFProcEnd); //break wait when it become 1, i.e., AF thread finish and post
        MY_LOG("3A PreviewEnd, wait semAFProcEnd done");
    }

    MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] before StateMgr::sendCmd(%d)\n", eCmd);
    ERROR_CHECK(StateMgr::getInstance().sendCmd(eCmd))
    MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] after StateMgr::sendCmd(%d)\n", eCmd);

    // FIXME: temp added before AF thread is ready
    if (eCmd == ECmd_Update)
    {
        //ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFUpdate))
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);    // Profiling Start.
        ERROR_CHECK(IspTuningMgr::getInstance().validatePerFrame(m_i4SensorDev, MFALSE))
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);     // Profiling End.
        MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] IspTuningMgr::validatePerFrame(MFALSE) done\n");


        {
            int Val;
            MY_LOG_IF(fgLogEn, "[Hal3A::postCommand] start waiting mAFMtx lock\n");
            Mutex::Autolock autoLock(mAFMtx); //lock for critical section
            MY_LOG_IF(fgLogEn, "[Hal3A::postCommand] get mAFMtx lock\n");
            if (g_semAFIRQWait == 0)
            {
                //AF thread start wait AF irq
                ::sem_getvalue(&semAFProcEnd, &Val);
                if (Val == 1) ::sem_wait(&semAFProcEnd); //to be 0, it won't block
            ::sem_post(&g_semAFThreadstart);
                MY_LOG_IF(fgLogEn, "[AFThread] sem_post g_semAFThreadstart, semAFProcEnd before wait = %d\n", Val);
            }
        }
/*
        CPTLog(Event_Pipe_3A_Flicker, CPTFlagStart);    // Profiling Start.
        AaaTimer localTimer("Flicker", getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
        if((mpFlickerHal!=NULL) )
        {
            //MY_LOG("[3a HAL]: Flicker update \n");
            if(m_rParam.u4AntiBandingMode==AE_FLICKER_MODE_AUTO//2//)
            {
                FlickerInput flkIn;
                FlickerOutput flkOut;
                //ERROR_CHECK(mpFlickerHal->enable(m_i4SensorDev,1))
                //ERROR_CHECK(mpFlickerHal->update(m_i4SensorDev,0,0))
        mpFlickerHal->enable(m_i4SensorDev,1);
        mpFlickerHal->update(m_i4SensorDev,&flkIn,&flkOut);
                //MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] mpFlickerHal Enable & Update done\n");
            }
            else
            {
                ERROR_CHECK(mpFlickerHal->enable(m_i4SensorDev,0))
                //MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] mpFlickerHal disable done\n");
            }
        }
    MY_LOG("line=%d\n", __LINE__);
        localTimer.printTime();
        CPTLog(Event_Pipe_3A_Flicker, CPTFlagEnd);    // Profiling End.
        MY_LOG("line=%d\n", __LINE__);
*/
    }



    MY_LOG_IF(fgLogEn,"[Hal3A::postCommand] exit, eCmd = %d\n", eCmd);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setParams(Param_T const &rNewParam)
{
    Param_T &rParamConvert = mParamCvt;

    // todo: convert param enum
    rParamConvert = rNewParam;
/*
    // Cam1 u4AeMode & u4FlashMode mapping
    switch (rNewParam.u4StrobeMode)
    {
    default:
    case NSFeature::FLASHLIGHT_AUTO:
        rParamConvert.u4AeMode = MTK_CONTROL_AE_MODE_ON_AUTO_FLASH;
        rParamConvert.u4StrobeMode = MTK_FLASH_MODE_SINGLE;
        break;
    case NSFeature::FLASHLIGHT_FORCE_OFF:
        rParamConvert.u4AeMode = MTK_CONTROL_AE_MODE_ON;
        rParamConvert.u4StrobeMode = MTK_FLASH_MODE_OFF;
        break;
    case NSFeature::FLASHLIGHT_FORCE_ON:
        rParamConvert.u4AeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH;
        rParamConvert.u4StrobeMode = MTK_FLASH_MODE_SINGLE;
        break;
    case NSFeature::FLASHLIGHT_REDEYE:
        rParamConvert.u4AeMode = MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE;
        rParamConvert.u4StrobeMode = MTK_FLASH_MODE_SINGLE;
        break;
    case NSFeature::FLASHLIGHT_TORCH:
        rParamConvert.u4AeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH;
        rParamConvert.u4StrobeMode = MTK_FLASH_MODE_TORCH;
        break;
    }
*/
    // Cam1 AWB mode
    switch (rNewParam.u4AwbMode)
    {
    case NSFeature::AWB_MODE_OFF:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_OFF;
        break;
    default:
    case NSFeature::AWB_MODE_AUTO:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_AUTO;
        break;
    case NSFeature::AWB_MODE_DAYLIGHT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_DAYLIGHT;
        break;
    case NSFeature::AWB_MODE_CLOUDY_DAYLIGHT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT;
        break;
    case NSFeature::AWB_MODE_SHADE:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_SHADE;
        break;
    case NSFeature::AWB_MODE_TWILIGHT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_TWILIGHT;
        break;
    case NSFeature::AWB_MODE_FLUORESCENT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_FLUORESCENT;
        break;
    case NSFeature::AWB_MODE_WARM_FLUORESCENT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT;
        break;
    case NSFeature::AWB_MODE_INCANDESCENT:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_INCANDESCENT;
        break;
    case NSFeature::AWB_MODE_TUNGSTEN:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_INCANDESCENT;
        break;
    case NSFeature::AWB_MODE_GRAYWORLD:
        rParamConvert.u4AwbMode = MTK_CONTROL_AWB_MODE_GRAYWORLD;
        break;
    }

    // Cam1 Effect mode
    switch (rNewParam.u4EffectMode)
    {
    default:
    case NSFeature::MEFFECT_OFF:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_OFF;
        break;
    case NSFeature::MEFFECT_MONO:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_MONO;
        break;
    case NSFeature::MEFFECT_SEPIA:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_SEPIA;
        break;
    case NSFeature::MEFFECT_NEGATIVE:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_NEGATIVE;
        break;
    case NSFeature::MEFFECT_SOLARIZE:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_SOLARIZE;
        break;
    case NSFeature::MEFFECT_POSTERIZE:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_POSTERIZE;
        break;
    case NSFeature::MEFFECT_AQUA:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_AQUA;
        break;
    case NSFeature::MEFFECT_BLACKBOARD:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_BLACKBOARD;
        break;
    case NSFeature::MEFFECT_WHITEBOARD:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_WHITEBOARD;
        break;
    case NSFeature::MEFFECT_SEPIAGREEN:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_SEPIAGREEN;
        break;
    case NSFeature::MEFFECT_SEPIABLUE:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_SEPIABLUE;
        break;
    case NSFeature::MEFFECT_NASHVILLE:  //For Lomo
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_NASHVILLE;
        break;
    case NSFeature::MEFFECT_HEFE:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_HEFE;
        break;
    case NSFeature::MEFFECT_VALENCIA:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_VALENCIA;
        break;
    case NSFeature::MEFFECT_XPROII:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_XPROII;
        break;
    case NSFeature::MEFFECT_LOFI:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_LOFI;
        break;
    case NSFeature::MEFFECT_SIERRA:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_SIERRA;
        break;
    case NSFeature::MEFFECT_KELVIN:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_KELVIN;
        break;
    case NSFeature::MEFFECT_WALDEN:
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_WALDEN;
        break;
    case NSFeature::MEFFECT_F1977:   //For Lomo
        rParamConvert.u4EffectMode = MTK_CONTROL_EFFECT_MODE_F1977;
        break;
    }

    // Cam1 Scene mode
    switch (rNewParam.u4SceneMode)
    {
    default:
    case NSFeature::SCENE_MODE_OFF:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
        break;
    case NSFeature::SCENE_MODE_NORMAL:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_NORMAL;
        break;
    case NSFeature::SCENE_MODE_ACTION:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_ACTION;
        break;
    case NSFeature::SCENE_MODE_PORTRAIT:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_PORTRAIT;
        break;
    case NSFeature::SCENE_MODE_LANDSCAPE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_LANDSCAPE;
        break;
    case NSFeature::SCENE_MODE_NIGHTSCENE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_NIGHT;
        break;
    case NSFeature::SCENE_MODE_NIGHTPORTRAIT:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT;
        break;
    case NSFeature::SCENE_MODE_THEATRE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_THEATRE;
        break;
    case NSFeature::SCENE_MODE_BEACH:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_BEACH;
        break;
    case NSFeature::SCENE_MODE_SNOW:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_SNOW;
        break;
    case NSFeature::SCENE_MODE_SUNSET:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_SUNSET;
        break;
    case NSFeature::SCENE_MODE_STEADYPHOTO:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_STEADYPHOTO;
        break;
    case NSFeature::SCENE_MODE_FIREWORKS:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_FIREWORKS;
        break;
    case NSFeature::SCENE_MODE_SPORTS:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_SPORTS;
        break;
    case NSFeature::SCENE_MODE_PARTY:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_PARTY;
        break;
    case NSFeature::SCENE_MODE_CANDLELIGHT:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_CANDLELIGHT;
        break;
    //  (Unsupported legacy symbol; don't use this symbol if possible)
    case NSFeature::SCENE_MODE_ISO_ANTI_SHAKE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
        break;
    //  (Unsupported legacy symbol; don't use this symbol if possible)
    case NSFeature::SCENE_MODE_BRACKET_AE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
        break;
    case NSFeature::SCENE_MODE_HDR:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_HDR;
        break;
    case NSFeature::SCENE_MODE_FACE_PRIORITY:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_FACE_PRIORITY;
        break;
    case NSFeature::SCENE_MODE_BARCODE:
        rParamConvert.u4SceneMode = MTK_CONTROL_SCENE_MODE_BARCODE;
        break;
    }

    // Cam1 anti flicker mode
    switch (rNewParam.u4AntiBandingMode)
    {
    default:
    case NSFeature::AE_FLICKER_MODE_60HZ:
        rParamConvert.u4AntiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ;
        break;
    case NSFeature::AE_FLICKER_MODE_50HZ:
        rParamConvert.u4AntiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ;
        break;
    case NSFeature::AE_FLICKER_MODE_AUTO:
        rParamConvert.u4AntiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO;
        break;
    case NSFeature::AE_FLICKER_MODE_OFF:
        rParamConvert.u4AntiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_OFF;
        break;
    }

   // AE
   // AE frame rate range   // if the max frame rate equal min frame rate, the frame rate is fix

   ERROR_CHECK(IAeMgr::getInstance().setAEMinMaxFrameRate(m_i4SensorDev, rParamConvert.i4MinFps, rNewParam.i4MaxFps))

   // AE metering mode
   ERROR_CHECK(IAeMgr::getInstance().setAEMeteringMode(m_i4SensorDev,rParamConvert.u4AeMeterMode))

   // AE rotate weighting control
   ERROR_CHECK(IAeMgr::getInstance().setAERotateDegree(m_i4SensorDev,rParamConvert.i4RotateDegree))

   // AE ISO speed
   ERROR_CHECK(IAeMgr::getInstance().setAEISOSpeed(m_i4SensorDev,rParamConvert.u4IsoSpeedMode))

   // AE Meter Area
   ERROR_CHECK(IAeMgr::getInstance().setAEMeteringArea(m_i4SensorDev,&rParamConvert.rMeteringAreas))

   // AE Exp mode
   ERROR_CHECK(IAeMgr::getInstance().setAEEVCompIndex(m_i4SensorDev,rParamConvert.i4ExpIndex, rNewParam.fExpCompStep))

   // AE mode
   ERROR_CHECK(IAeMgr::getInstance().setAEMode(m_i4SensorDev,rParamConvert.u4AeMode))

   // AE anti banding
   ERROR_CHECK(IAeMgr::getInstance().setAEFlickerMode(m_i4SensorDev,rParamConvert.u4AntiBandingMode))

   // AE lock
   ERROR_CHECK(IAeMgr::getInstance().setAPAELock(m_i4SensorDev,rParamConvert.bIsAELock))

   // AE Cam mode
   ERROR_CHECK(IAeMgr::getInstance().setAECamMode(m_i4SensorDev,rParamConvert.u4CamMode))

   // AE Scene
   ERROR_CHECK(IAeMgr::getInstance().setSceneMode(m_i4SensorDev,rNewParam.u4SceneMode))

   // AE EIS info for fixed frame rate
   ERROR_CHECK(IAeMgr::getInstance().setEISon(m_i4SensorDev,rParamConvert.bVideoStabilization))

   // AE Shot mode
   //ERROR_CHECK(IAeMgr::getInstance().setAEShotMode(m_i4SensorDev,rParamConvert.u4ShotMode))

   MY_LOG_IF(1,"[%s()] AE done\n", __FUNCTION__);

   // AF
   ERROR_CHECK(IAfMgr::getInstance().setAFMode(m_i4SensorDev, rParamConvert.u4AfMode))
   if(m_bFaceDetectEnable==MFALSE)
   ERROR_CHECK(IAfMgr::getInstance().setAFArea(m_i4SensorDev,rParamConvert.rFocusAreas))
   ERROR_CHECK(IAfMgr::getInstance().setCamMode(m_i4SensorDev, rParamConvert.u4CamMode))
   ERROR_CHECK(IAfMgr::getInstance().setFullScanstep(m_i4SensorDev, rParamConvert.i4FullScanStep))
   ERROR_CHECK(IAfMgr::getInstance().setMFPos(m_i4SensorDev, rParamConvert.i4MFPos))
   IAfMgr::getInstance().setAndroidServiceState(m_i4SensorDev, rParamConvert.bIsSupportAndroidService);

   MY_LOG_IF(1,"[%s()] AF done\n", __FUNCTION__);

   // AWB mode
   ERROR_CHECK(IAwbMgr::getInstance().setAWBMode(m_i4SensorDev, rParamConvert.u4AwbMode))

   // AWB lock
   ERROR_CHECK(IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rParamConvert.bIsAWBLock))

   MY_LOG_IF(1,"[%s()] AWB done\n", __FUNCTION__);

   // ISP tuning
   ERROR_CHECK(IspTuningMgr::getInstance().setSceneMode(m_i4SensorDev, rParamConvert.u4SceneMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setEffect(m_i4SensorDev, rParamConvert.u4EffectMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rParamConvert.u4BrightnessMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rParamConvert.u4HueMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rParamConvert.u4SaturationMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rParamConvert.u4EdgeMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rParamConvert.u4ContrastMode))

   MY_LOG_IF(1,"[%s()] ISP Tuning done\n", __FUNCTION__);

   //flash
   //ERROR_CHECK(FlashMgr::getInstance().setSensorDev(m_i4SensorDev, m_i4SensorDev))
   ERROR_CHECK(FlashMgr::getInstance().setFlashMode(m_i4SensorDev, rParamConvert.u4StrobeMode))
   //ERROR_CHECK(FlashMgr::getInstance().setShotMode(m_i4SensorDev, rNewParam.u4ShotMode))
   int bMulti;
   if(rNewParam.u4ShotMode == NSCam::eShotMode_ContinuousShot)
    bMulti=1;
   else
    bMulti=0;
   ERROR_CHECK(FlashMgr::getInstance().setMultiCapture(m_i4SensorDev, bMulti))
   ERROR_CHECK(FlashMgr::getInstance().setCamMode(m_i4SensorDev, rParamConvert.u4CamMode))
   ERROR_CHECK(FlashMgr::getInstance().setEvComp(m_i4SensorDev, rParamConvert.i4ExpIndex, rParamConvert.fExpCompStep))

   MY_LOG_IF(1,"[%s()] Flash done\n", __FUNCTION__);

   ERROR_CHECK(FlickerHalBase::getInstance().setFlickerMode(m_i4SensorDev,rParamConvert.u4AntiBandingMode))

   MY_LOG_IF(1,"[%s()] Flicker done\n", __FUNCTION__);

    m_rParam = rNewParam;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getSupportedParams(FeatureParam_T &rFeatureParam)
{
    rFeatureParam.u4MaxFocusAreaNum = IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev);
    rFeatureParam.i4MaxLensPos = IAfMgr::getInstance().getMaxLensPos(m_i4SensorDev);
    rFeatureParam.i4MinLensPos = IAfMgr::getInstance().getMinLensPos(m_i4SensorDev);
    rFeatureParam.i4AFBestPos = IAfMgr::getInstance().getAFBestPos(m_i4SensorDev);
    rFeatureParam.i8BSSVlu = IAfMgr::getInstance().getBestShotValue(m_i4SensorDev);

    // FIXME:
    rFeatureParam.bExposureLockSupported = IAeMgr::getInstance().isAELockSupported();
    rFeatureParam.bAutoWhiteBalanceLockSupported = IAwbMgr::getInstance().isAWBLockSupported();
    //rFeatureParam.u4MaxFocusAreaNum;
    rFeatureParam.u4MaxMeterAreaNum = IAeMgr::getInstance().getAEMaxMeterAreaNum(m_i4SensorDev);

    IAeMgr::getInstance().getNvramData(m_i4SensorDev);
    AE_DEVICES_INFO_T rDeviceInfo;
    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev,rDeviceInfo);
    rFeatureParam.u4FocusLength_100x = rDeviceInfo.u4FocusLength_100x;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::autoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

    if ((m_rParam.u4AfMode != NSFeature::AF_MODE_AFC) && (m_rParam.u4AfMode != NSFeature::AF_MODE_AFC_VIDEO) &&
        (IAfMgr::getInstance().getAFMaxAreaNum(m_i4SensorDev) > 0))   {
        ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFStart))
    }
    IAfMgr::getInstance().autoFocus(m_i4SensorDev);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::cancelAutoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);
    if ((m_rParam.u4AfMode != NSFeature::AF_MODE_AFC) && (m_rParam.u4AfMode != NSFeature::AF_MODE_AFC_VIDEO))   {
        ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFEnd))
    }
    IAfMgr::getInstance().cancelAutoFocus(m_i4SensorDev);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    ERROR_CHECK(IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev,u4XOffset, u4YOffset, u4Width, u4Height))
    ERROR_CHECK(IAfMgr::getInstance().setZoomWinInfo(m_i4SensorDev,u4XOffset, u4YOffset, u4Width, u4Height))
    ERROR_CHECK(IspTuningMgr::getInstance().setZoomRatio(m_i4SensorDev, u4ZoomRatio_x100))
    ERROR_CHECK(FlashMgr::getInstance().setDigZoom(m_i4SensorDev, u4ZoomRatio_x100))

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
    EXIF_INFO_T rEXIFInfo;
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_MODE_CFG_T rCaptureInfo;

    IAeMgr::getInstance().getSensorDeviceInfo(m_i4SensorDev,rDeviceInfo);
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

    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setDebugInfo(IBaseCamExif *pIBaseCamExif) const
{
    // 3A debug info
    static AAA_DEBUG_INFO_T r3ADebugInfo;
    MY_LOG("AAA_DEBUG_INFO_T(%d)",sizeof(AAA_DEBUG_INFO_T));
    r3ADebugInfo.hdr.u4KeyID = AAA_DEBUG_KEYID;
    r3ADebugInfo.hdr.u4ModuleCount = MODULE_NUM(6,5);

    r3ADebugInfo.hdr.u4AEDebugInfoOffset        = sizeof(r3ADebugInfo.hdr);
    r3ADebugInfo.hdr.u4AFDebugInfoOffset        = r3ADebugInfo.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugInfoOffset       = r3ADebugInfo.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlashDebugInfoOffset     = r3ADebugInfo.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlickerDebugInfoOffset   = r3ADebugInfo.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugDataOffset       = r3ADebugInfo.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);

    MY_LOG("AE_DEBUG_INFO_T(%d) AF_DEBUG_INFO_T(%d) AWB_DEBUG_INFO_T(%d) FLASH_DEBUG_INFO_T(%d) FLICKER_DEBUG_INFO_T(%d) AWB_DEBUG_DATA_T(%d)",
            sizeof(AE_DEBUG_INFO_T),
            sizeof(AF_DEBUG_INFO_T),
            sizeof(AWB_DEBUG_INFO_T),
            sizeof(FLASH_DEBUG_INFO_T),
            sizeof(FLICKER_DEBUG_INFO_T),
            sizeof(AWB_DEBUG_DATA_T) );
    AE_PLINE_DEBUG_INFO_T tmpBuff;
    // AE
    IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAEDebugInfo, tmpBuff);

    // AF
    IAfMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAFDebugInfo);

    // AWB
    IAwbMgr::getInstance().getDebugInfo(m_i4SensorDev, r3ADebugInfo.rAWBDebugInfo, r3ADebugInfo.rAWBDebugData);

    // Flash
    FlashMgr::getInstance().getDebugInfo(m_i4SensorDev,&r3ADebugInfo.rFlashDebugInfo);

    // Flicker
//    Flicker::getInstance()->getDebugInfo(&r3ADebugInfo.rFlickerDebugInfo);

    MINT32 ID;
    pIBaseCamExif->sendCommand(CMD_REGISTER, AAA_DEBUG_KEYID, reinterpret_cast<MINTPTR>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINTPTR>(&r3ADebugInfo), sizeof(AAA_DEBUG_INFO_T));

    // LSC
    DEBUG_RESERVEA_INFO_T rShadingDbgInfo;
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugInfo(rShadingDbgInfo);
    pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_CAM_RESERVE1_MID, reinterpret_cast<MINTPTR>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINTPTR>(&rShadingDbgInfo), sizeof(rShadingDbgInfo));

    // LSC table
#if 0
    DEBUG_SHAD_ARRAY_INFO_T rShadingDbgTbl;
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getDebugTbl(rShadingDbgTbl);
    //me14@@pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_SHAD_TABLE_KEYID, reinterpret_cast<MINT32>(&ID));
    //me14@@pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rShadingDbgTbl), sizeof(rShadingDbgTbl));
#endif
    // ISP debug info
    NSIspExifDebug::IspExifDebugInfo_T rIspExifDebugInfo;
    IspTuningMgr::getInstance().getDebugInfo(m_i4SensorDev, rIspExifDebugInfo);

    pIBaseCamExif->sendCommand(CMD_REGISTER, static_cast<MINT32>(rIspExifDebugInfo.hdr.u4KeyID), reinterpret_cast<MINTPTR>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINTPTR>(&rIspExifDebugInfo), sizeof(rIspExifDebugInfo));

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
MBOOL Hal3A::setIspProfile(EIspProfile_T const eIspProfile)
{
    MY_LOG("[%s] + eIspProfile = %d", __FUNCTION__, eIspProfile);

    ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, eIspProfile))
    ERROR_CHECK(IspTuningMgr::getInstance().validate(m_i4SensorDev))

    MY_LOG("[%s] - ", __FUNCTION__);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo)
{
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
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    AE_MODE_CFG_T rCaptureInfo;

    rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
    rCaptureInfo.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    rCaptureInfo.u4AfeGain = a_rCaptureInfo.u4AfeGain;
    rCaptureInfo.u4IspGain = a_rCaptureInfo.u4IspGain;
    rCaptureInfo.u4RealISO = a_rCaptureInfo.u4RealISO;
    rCaptureInfo.i2FlareGain = (MINT16)a_rCaptureInfo.u4FlareGain;
    rCaptureInfo.i2FlareOffset = (MINT16)a_rCaptureInfo.u4FlareOffset;

    IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, rCaptureInfo);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
    IAeMgr::getInstance().getHDRCapInfo(m_i4SensorDev, a_strHDROutputInfo);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
    return IAeMgr::getInstance().getExposureInfo(m_i4SensorDev, strHDRInputSetting);
    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getRTParams(FrameOutputParam_T &a_strFrameOutputInfo)
{
    IAeMgr::getInstance().getRTParams(m_i4SensorDev, a_strFrameOutputInfo);
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

MBOOL Hal3A::setFDInfo(MVOID* a_sFaces)
{
    IAfMgr::getInstance().setFDInfo(m_i4SensorDev, a_sFaces);
    IAeMgr::getInstance().setFDInfo(m_i4SensorDev, a_sFaces);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setOTInfo(MVOID* a_sOT)
{
    IAfMgr::getInstance().setOTInfo(m_i4SensorDev, a_sOT);
    IAeMgr::getInstance().setOTInfo(m_i4SensorDev, a_sOT);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::enableAELimiterControl(MBOOL  bIsAELimiter)
{
    IAeMgr::getInstance().setAELimiterMode(m_i4SensorDev, bIsAELimiter);
    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getFlashFrameNumBeforeCapFrame()
{
    //this flash API is valid after 3A StatePrecapture
    return FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev) ? 3 : 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3A::onFireCapFlashIfNeeded()
{
    FlashMgr::getInstance().capCheckAndFireFlash_Start(m_i4SensorDev); //Originally, it's in 3A ECmd_CaptureStart
    //PS: in ECmd_CaptureEnd, we have FlashMgr::getInstance().capCheckAndFireFlash_End();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getASDInfo(ASDInfo_T &a_rASDInfo)
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

    a_rASDInfo.i4AELv_x10 = IAeMgr::getInstance().getLVvalue(m_i4SensorDev,MTRUE);
    a_rASDInfo.bAEBacklit = IAeMgr::getInstance().getAECondition(m_i4SensorDev,AE_CONDITION_BACKLIGHT);
    a_rASDInfo.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
    a_rASDInfo.i2AEFaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4SensorDev);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getLCEInfo(LCEInfo_T &a_rLCEInfo)
{
    IAeMgr::getInstance().getLCEPlineInfo(m_i4SensorDev, a_rLCEInfo);
    return MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3A::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;


    if (a_bEnable)  {

        if (g_bAFThreadLoop == 0)
        {
            /*ret = AfMgr::getInstance().init();
            if (FAILED(ret)) {
                MY_ERR("AfMgr::getInstance().init() fail\n");
                return ret;
            }*/

            g_pIspDrv = IspDrv::createInstance();

            if (!g_pIspDrv) {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (g_pIspDrv->init() < 0) {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }

            // create AF thread
            MY_LOG("[AFThread] Create");
            g_bAFThreadLoop = 1;
            sem_init(&g_semAFThreadstart, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
            pthread_create(&g_AFThread, &attr, AFThreadFunc, this);

            bEnAFNotifyThreadLoop = MTRUE;
            sem_init(&mSemAFNotify, 0, 0);
            pthread_create(&g_AFNotifyThread, &attr, AFNotifyThreadLoop, this);
        }
    }
    else   {

        if (g_bAFThreadLoop == 1)
        {

            if (g_pIspDrv)   {
                g_pIspDrv->uninit();
                g_pIspDrv = NULL;
            }
            g_bAFThreadLoop = 0;
            ::sem_post(&g_semAFThreadstart);

            bEnAFNotifyThreadLoop = MFALSE;
            ::sem_post(&mSemAFNotify);

            pthread_join(g_AFThread, NULL);
            pthread_join(g_AFNotifyThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

    return ret;
}

MVOID * Hal3A::AFThreadFunc(void *arg)
{
    MY_LOG("[AFThread] tid: %d \n", gettid());
    ::prctl(PR_SET_NAME,"AFthread", 0, 0, 0);
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);

    if (!g_pIspDrv) {
        MY_LOG("[AFThread] m_pIspDrv null\n");
        return NULL;
    }

    // wait AFO done
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_AF_DON_ST;
    WaitIrq.Timeout = 500; // 200 msec

    while (g_bAFThreadLoop) {
        MBOOL fgLogEn = (sm_3ALogEnable & EN_3A_FLOW_LOG) ? MTRUE : MFALSE;
        //
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (1)\n");
        {Mutex::Autolock autoLock(_this->mAFMtx); g_semAFIRQWait = 0;}
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] get mAFMtx lock (1)\n");

        ::sem_wait(&g_semAFThreadstart);
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] sem_wait g_semAFThreadstart done\n");

        {Mutex::Autolock autoLock(_this->mAFMtx); g_semAFIRQWait = 1;}
        MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] get mAFMtx lock (2)\n");

        if (g_bAFThreadLoop != 0)
        {
            //AfMgr::getInstance().DoCallback();
            ::sem_post(&_this->mSemAFNotify);
            MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] sem_post(&mSemAFNotify)\n");
            if (g_pIspDrv->waitIrq(WaitIrq) > 0) // success
            {
                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
                StateMgr::getInstance().sendCmd(ECmd_AFUpdate);
                MY_LOG_IF(fgLogEn, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");
            }
            else
            {
                MY_LOG("[AFThread] AF irq timeout\n");
                IAfMgr::getInstance().TimeOutHandle(_this->m_i4SensorDev);
                //TODO: IAfMgr::getInstance().printAFConfigLog0(_this->m_i4SensorDev);
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
    }

    //::sem_post(&g_semAFThreadEnd);

    MY_LOG("[AFThread] End \n");

    return NULL;
}

MVOID* Hal3A::AFNotifyThreadLoop(void *arg)
{
    MY_LOG("[AFNotifyThreadLoop] tid: %d \n", gettid());
    ::prctl(PR_SET_NAME,"AFNotifyThread", 0, 0, 0);
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);

    MY_LOG("[%s] thread starts\n", __FUNCTION__);

    while (_this->bEnAFNotifyThreadLoop)
    {
        ::sem_wait(&_this->mSemAFNotify);
        MY_LOG("[%s] mSemAFNotify wait done\n", __FUNCTION__);
        if (!_this->bEnAFNotifyThreadLoop) break;
        _this->sendAFCallbacks();
    }
    MY_LOG("[%s] thread terminates\n", __FUNCTION__);
    return NULL;
}

MVOID  Hal3A::sendAFCallbacks()
{
    MY_LOG("Hal3A::sendAFCallbacks, (%d)\n", bAfNeedSingleNotify);
    IAfMgr::getInstance().DoCallback(m_i4SensorDev);

    if (bAfNeedSingleNotify)
    {
        MY_LOG("Hal3A::SingleAF_CallbackNotify, before\n");
        IAfMgr::getInstance().SingleAF_CallbackNotify(m_i4SensorDev);
        MY_LOG("Hal3A::SingleAF_CallbackNotify, after\n");
        bAfNeedSingleNotify = MFALSE;
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);

    return IAfMgr::getInstance().setCallbacks(m_i4SensorDev, cb);
}




/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::waitVSirq()
{

    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Timeout = 3636; // 5000 msec
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    //no ISP_DRV_IRQ_INT_STATUS_VS2_ST;


    if (mpIspDrv==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    if (mpIspDrv->waitIrq(WaitIrq) <= 0)
    {
        MY_ERR("wait vsync timeout");
    }

}

/******************************************************************************
*
*******************************************************************************/
MINT32 Hal3A::isNeedFiringFlash()
{
    return FlashMgr::getInstance().isNeedFiringFlash(m_i4SensorDev);
}
/******************************************************************************
*
*******************************************************************************/
MVOID Hal3A::endContinuousShotJobs()
{
    FlashMgr::getInstance().turnOffFlashDevice(m_i4SensorDev);
}

/******************************************************************************
*
*******************************************************************************/
MVOID Hal3A::setSensorMode(MINT32 i4SensorMode)
{
    MY_LOG("[%s] i4SensorMode = %d", __FUNCTION__, i4SensorMode);
    IAwbMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IAeMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    IspTuningMgr::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
    FlickerHalBase::getInstance().setSensorMode(m_i4SensorDev, i4SensorMode);
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
    case E3ACtrl_SetShadingTSFCapByp:
        i4Ret = LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setTsfCapBypass(iArg1 ? MTRUE : MFALSE);
        break;

    /*// --------------------------------- AWB ---------------------------------
    case E3ACtrl_SetAwbBypCalibration:
        i4Ret = IAwbMgr::getInstance().CCTOPAWBBypassCalibration(m_i4SensorDev, (iArg1 ? MTRUE : MFALSE));
        break;*/

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
    // --------------------------------- AF ---------------------------------
    case E3ACtrl_QueryAFStatus:
        i4Ret = IAfMgr::getInstance().queryAFStatusByMagicNum(m_i4SensorDev, iArg1);
        break;
    // --------------------------------- PDAF ---------------------------------------
    /*case E3ACtrl_PostPDAFtask:
        ::sem_post(&semPD);
        break;

    // --------------------------------- flow control ---------------------------------
    case E3ACtrl_Enable3ASetParams:
        mbEnable3ASetParams = iArg1;
        break;*/
    case E3ACtrl_SetOperMode:
        i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
        break;
    case E3ACtrl_GetOperMode:
        i4Ret = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
        break;
    /*case E3ACtrl_GetStereo3DWarning:
        i4Ret = getStereo3DWarning();
        break;
    case E3ACtrl_GetEvCapture:
        i4Ret = IAeMgr::getInstance().switchCapureDiffEVState(m_i4SensorDev, (MINT8) iArg1,  *reinterpret_cast<strAEOutput*>(iArg2));
        break;*/
    case E3ACtrl_GetDAFTBL:
        i4Ret = IAfMgr::getInstance().getDAFtbl(m_i4SensorDev, (MVOID**)iArg1);
        break;
    /*case ECmd_Set3ACollectInfoFile:
        // if Cmd existed then Free Cmd.
        freeCollectCmd();

        createCollectCmd(iArg1);
        break;*/
    default:
        return -1;
    }

        return i4Ret;

}

/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3A::
setAELock(MBOOL bIsAELock)
{
    Param_T rNewParam;
    rNewParam.bIsAELock = bIsAELock;
    // APAE lock
    ERROR_CHECK(IAeMgr::getInstance().setAPAELock(m_i4SensorDev, rNewParam.bIsAELock))
    return MTRUE;
}

MBOOL
Hal3A::
setAWBLock(MBOOL bIsAWBLock)
{
    Param_T rNewParam;
    rNewParam.bIsAWBLock = bIsAWBLock;
    // AWB lock
    ERROR_CHECK(IAwbMgr::getInstance().setAWBLock(m_i4SensorDev, rNewParam.bIsAWBLock))
    return MTRUE;
}
