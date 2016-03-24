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
#define LOG_TAG "Hal3AAdapter1"

#include "Hal3AAdapter1.h"

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <camera_feature.h>
#include <utils/Atomic.h>

using namespace android;
using namespace NSCam;
using namespace NS3A;

/*******************************************************************************
*
********************************************************************************/
Hal3AAdapter1*
Hal3AAdapter1::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    switch (i4SensorIdx)
    {
    case 0:
        {
            static Hal3AAdapter1 _singleton(0);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 1:
        {
            static Hal3AAdapter1 _singleton(1);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 2:
        {
            static Hal3AAdapter1 _singleton(2);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 3:
        {
            static Hal3AAdapter1 _singleton(3);
            _singleton.init(strUser);
            return &_singleton;
        }
    default:
        return NULL;
    }
}

Hal3AAdapter1::
Hal3AAdapter1(MINT32 const i4SensorIdx)
    : mpHal3aObj(NULL)
    , mi4FrmId(-1)
    , mi4SensorIdx(i4SensorIdx)
    , mi4User(0)
    , mLock()
{
    CAM_LOGD("[%s] mi4SensorIdx(0x%04x)\n",
        __FUNCTION__, i4SensorIdx);
}


MVOID
Hal3AAdapter1::
destroyInstance(const char* strUser)
{
    uninit(strUser);
}

MBOOL
Hal3AAdapter1::
init(const char* strUser)
{
    Mutex::Autolock lock(mLock);

    CAM_LOGD("[%s] %s(%d)", __FUNCTION__, strUser, mi4User);

    if (mi4User > 0)
    {

    }
    else
    {
        CAM_LOGD("[%s] mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)\n",
            __FUNCTION__, mi4SensorIdx, mpHal3aObj);
        mpHal3aObj = Hal3AIf::createInstance(mi4SensorIdx);
    }

    android_atomic_inc(&mi4User);

    return MTRUE;
}

MBOOL
Hal3AAdapter1::
uninit(const char* strUser)
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            CAM_LOGD("[%s] mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)\n",
                __FUNCTION__, mi4SensorIdx, mpHal3aObj);
            mpHal3aObj->destroyInstance();
            mpHal3aObj = NULL;
        }
        else    // There are still some users.
        {
            //CAM_LOGD(m_bDebugEnable,"Still %d users \n", mi4User);
        }
    }

    CAM_LOGD("[%s] %s(%d)", __FUNCTION__, strUser, mi4User);

    return MTRUE;
}

MVOID
Hal3AAdapter1::
setFrameId(MINT32 i4FrmId)
{
    mpHal3aObj->setFrameId(i4FrmId);
}

MINT32
Hal3AAdapter1::
getFrameId() const
{
    return mpHal3aObj->getFrameId();
}


MBOOL
Hal3AAdapter1::
sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    return mpHal3aObj->sendCommand(eCmd, i4Arg);
}

MBOOL
Hal3AAdapter1::
getParams(Param_T &rParam) const
{
#if 0
    Param_T rParamOrg;
    // todo: convert param enum
    MBOOL fgRet = mpHal3aObj->getParams(rParamOrg);
    rParam = rParamOrg;
    return fgRet;
#else
    rParam = mParam;
    return MTRUE;
#endif
}

MBOOL
Hal3AAdapter1::
setParams(Param_T const &rNewParam)
{
    Param_T &rParamConvert = mParamCvt;

    // todo: convert param enum
    rParamConvert = rNewParam;
    mParam = rNewParam;

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

    // Cam1 af mode
    #if 0
    switch (rNewParam.u4AfMode)
    {
    case NSFeature::AF_MODE_AFS:
        break;
    case NSFeature::AF_MODE_AFC:
        break;
    case NSFeature::AF_MODE_AFC_VIDEO:
        break;
    case NSFeature::AF_MODE_MACRO:
        break;
    case NSFeature::AF_MODE_INFINITY:
        break;
    case NSFeature::AF_MODE_MF:
        break;
    case NSFeature::AF_MODE_FULLSCAN:
        break;
    }
    #endif

    MBOOL fgRet;

    if (mpHal3aObj->getSensorType() == ESensorType_RAW)     // For RAW
    {
        fgRet = mpHal3aObj->setParams(rParamConvert);
    }
    else    // For YUV
    {
        fgRet = mpHal3aObj->setParams(mParam);
    }

    return fgRet;
}

MVOID
Hal3AAdapter1::
setFDEnable(MBOOL bEnable)
{
    mpHal3aObj->setFDEnable(bEnable);
}

MVOID
Hal3AAdapter1::
setSensorMode(MINT32 i4SensorMode)
{
    mpHal3aObj->setSensorMode(i4SensorMode);
}

MVOID
Hal3AAdapter1::
set3APreviewMode(E3APreviewMode_T PvMode)
{
    mpHal3aObj->set3APreviewMode(PvMode);
}

MINT32
Hal3AAdapter1::SetAETargetMode(MUINT32 AeTargetMode)
{
    return mpHal3aObj->SetAETargetMode(AeTargetMode);
}


MBOOL
Hal3AAdapter1::
getSupportedParams(FeatureParam_T &rFeatureParam)
{
    return mpHal3aObj->getSupportedParams(rFeatureParam);
}

MBOOL
Hal3AAdapter1::
isReadyToCapture() const
{
    return mpHal3aObj->isReadyToCapture();
}

MBOOL
Hal3AAdapter1::
autoFocus()
{
    return mpHal3aObj->autoFocus();
}

MBOOL
Hal3AAdapter1::
cancelAutoFocus()
{
    return mpHal3aObj->cancelAutoFocus();
}

MBOOL
Hal3AAdapter1::
setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return mpHal3aObj->setZoom(u4ZoomRatio_x100, u4XOffset, u4YOffset, u4Width, u4Height);
}

MBOOL
Hal3AAdapter1::
set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
    // todo: need i4SensorDevId ?
    return mpHal3aObj->set3AEXIFInfo(pIBaseCamExif);
}

MBOOL
Hal3AAdapter1::
setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const
{
    // todo: need i4SensorDevId ?
    return mpHal3aObj->setDebugInfo(pDbgInfoCtnr, fgReadFromHW);
}

MINT32
Hal3AAdapter1::
getDelayFrame(EQueryType_T const eQueryType) const
{
    return mpHal3aObj->getDelayFrame(eQueryType);
}
/*
MBOOL
Hal3AAdapter1::
setCallbacks(I3ACallBack::ECallBack_T eMsg, I3ACallBack* cb)
{
    return mpHal3aObj->setCallbacks(eMsg, cb);
}
*/

MINT32
Hal3AAdapter1::
addCallbacks(I3ACallBack* cb)
{
    return mpHal3aObj->addCallbacks(cb);
}

MINT32
Hal3AAdapter1::
removeCallbacks(I3ACallBack* cb)
{
    return mpHal3aObj->removeCallbacks(cb);
}

MBOOL
Hal3AAdapter1::
setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr)
{
    return mpHal3aObj->setIspProfile(rParamIspProfile, pDbgInfoCtnr);
}

MINT32
Hal3AAdapter1::
getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
    return mpHal3aObj->getExposureInfo(strHDRInputSetting);
}

MINT32
Hal3AAdapter1::
getCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    return mpHal3aObj->getCaptureParams(a_rCaptureInfo);
}

MINT32
Hal3AAdapter1::
updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    return mpHal3aObj->updateCaptureParams(a_rCaptureInfo);
}

MINT32
Hal3AAdapter1::
getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
    return mpHal3aObj->getHDRCapInfo(a_strHDROutputInfo);
}

MBOOL
Hal3AAdapter1::
setFDInfo(MVOID* a_sFaces)
{
    return mpHal3aObj->setFDInfo(a_sFaces);
}

MBOOL
Hal3AAdapter1::
setOTInfo(MVOID* a_sOT)
{
    return mpHal3aObj->setOTInfo(a_sOT);
}

MINT32
Hal3AAdapter1::
getRTParams(FrameOutputParam_T &a_strFrameOutputInfo)
{
    return mpHal3aObj->getRTParams(a_strFrameOutputInfo);
}

MINT32
Hal3AAdapter1::
isNeedFiringFlash(MBOOL bEnCal)
{
    return mpHal3aObj->isNeedFiringFlash(bEnCal);
}

MINT32
Hal3AAdapter1::
enableFlashQuickCalibration(MINT32 bEn)
{
    return mpHal3aObj->enableFlashQuickCalibration(bEn);
}

MINT32
Hal3AAdapter1::
getFlashQuickCalibrationResult()
{
    MINT32 ret;
    ret = mpHal3aObj->getFlashQuickCalibrationResult();
    CAM_LOGD("getFlashQuickCalibrationResult ln=%d ret=%d", __LINE__, ret);
    return ret;
}

MBOOL
Hal3AAdapter1::
getASDInfo(ASDInfo_T &a_rASDInfo)
{
    return mpHal3aObj->getASDInfo(a_rASDInfo);
}

MINT32
Hal3AAdapter1::
modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    return mpHal3aObj->modifyPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
}

MBOOL
Hal3AAdapter1::
getLCEInfo(LCEInfo_T &a_rLCEInfo)
{
    return mpHal3aObj->getLCEInfo(a_rLCEInfo);
}

MVOID
Hal3AAdapter1::
endContinuousShotJobs()
{
}

MINT32
Hal3AAdapter1::
enableAELimiterControl(MBOOL  bIsAELimiter)
{
    return mpHal3aObj->enableAELimiterControl(bIsAELimiter);
}

MVOID
Hal3AAdapter1::
enterCaptureProcess()
{
    mpHal3aObj->enterCaptureProcess();
}

MVOID
Hal3AAdapter1::
exitCaptureProcess()
{
    mpHal3aObj->exitCaptureProcess();
}
MUINT32
Hal3AAdapter1::
queryFramesPerCycle(MUINT32 fps)
{
    return mpHal3aObj->queryFramesPerCycle(fps);
}

MINT32
Hal3AAdapter1::send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    return mpHal3aObj->send3ACtrl(e3ACtrl, iArg1, iArg2);
}

MVOID
Hal3AAdapter1::
setAELock(MBOOL bIsAELock)
{
    mpHal3aObj->setAELock(bIsAELock);
}

MVOID
Hal3AAdapter1::
setAWBLock(MBOOL bIsAWBLock)
{
    mpHal3aObj->setAWBLock(bIsAWBLock);
}
