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
#define LOG_TAG "MtkCam/P2Node"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <cutils/properties.h>  // For property_get().
using namespace NSCam;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include "./inc/pass2nodeImpl.h"
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#endif

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")
#define FUNC_NAME  MY_LOGD("")

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
FeaturePass2::
FeaturePass2(Pass2NodeType const type)
    : PrvPass2(type)
    , mspCamMsgCbInfo(NULL)
    , mFeatureMask(0)
    , mSensorType(SENSOR_TYPE_UNKNOWN)
    , mbFeature(MFALSE)
    , mbGDPreview(MFALSE)
    , mbRecordingHint(MFALSE)
    , mbInit(MFALSE)
{
    mFeaturePipeTuning.SmoothLevel      = 0;
    mFeaturePipeTuning.SkinColor        = 0;
    mFeaturePipeTuning.EnlargeEyeLevel  = 0;
    mFeaturePipeTuning.SlimFaceLevel    = 0;
    mFeaturePipeTuning.ExtremeBeauty    = MFALSE;
    mFeaturePipeTuning.Rotation         = 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
moduleStart()
{
    FUNC_START;

    mbInit = MFALSE;
    // trigger first loop for init
    triggerLoop();

    FUNC_END;
lbExit:
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
doInitialization()
{
    MBOOL doInitial = MFALSE;

    if( !mbInit )
    {
        MY_LOGD("doInitialization");
        mbInit = MTRUE;
        configFeature();
        doInitial = MTRUE;
    }

lbExit:
    return doInitial;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
enableFeature(MBOOL en)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("en(%d)",en);
    mbFeature = en;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePass2::
enableGDPreview(MBOOL en)
{
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("en(%d),FeatureMask0x%08X)",en,mFeatureMask);
    mbGDPreview = en;
    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
    {
        MY_LOGD("GD should be handled by FDClient");
        return MFALSE;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
setParameters(
    sp<IParamsManager>  spParamsMgr,
    MUINT32             openId)
{
    Mutex::Autolock _l(mLock);
    //
    if(!mbFeature)
    {
        FEATURE_MASK_CLEAR(mFeatureMask);
        MY_LOGD("All features are disabled FeatureMask(0x%08X)",mFeatureMask);
        return;
    }
    //
    if(mSensorType == SENSOR_TYPE_UNKNOWN)
    {
        SensorStaticInfo sensorInfo;
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(openId),
                &sensorInfo);
        mSensorType = sensorInfo.sensorType;
        MY_LOGD("mSensorType(%d)",mSensorType);
    }
    //
    mFeaturePipeTuning.SmoothLevel      = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL);
    mFeaturePipeTuning.SkinColor        = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SKIN_COLOR);
    mFeaturePipeTuning.EnlargeEyeLevel  = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_ENLARGE_EYE);
    mFeaturePipeTuning.SlimFaceLevel    = spParamsMgr->getInt(MtkCameraParameters::KEY_FB_SLIM_FACE);
    mFeaturePipeTuning.Rotation         = spParamsMgr->getInt(MtkCameraParameters::KEY_ROTATION);
    //
    if(::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FB_EXTREME_BEAUTY), MtkCameraParameters::TRUE) == 0)
    {
        mFeaturePipeTuning.ExtremeBeauty = MTRUE;
    }
    else
    {
        mFeaturePipeTuning.ExtremeBeauty = MFALSE;
    }
    //
    spParamsMgr->getFaceBeautyTouchPosition(mTouch);
    //
    FEATURE_MASK_CLEAR(mFeatureMask);
    //
    MINT32 vdoWidth, vdoHeight;
    spParamsMgr->getVideoSize(
                    &vdoWidth,
                    &vdoHeight);
    if(!mbPreview)
    {
        mVdoSizeBeforePreview.w = vdoWidth;
        mVdoSizeBeforePreview.h = vdoHeight;
    }
    MY_LOGD("bPreivew(%d),VdoSize before preview:%dx%d,Cur VdoSize:%dx%d",
            mbPreview,
            mVdoSizeBeforePreview.w,
            mVdoSizeBeforePreview.h,
            vdoWidth,
            vdoHeight);
    //
    if( ::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_3DNR_MODE), "on") == 0 &&
        mSensorType == SENSOR_TYPE_RAW)
    {
        if( (   spParamsMgr->getRecordingHint()  &&
                vdoWidth*vdoHeight <= IMG_1080P_SIZE     ) ||
            !(spParamsMgr->getRecordingHint()))
        {
            #if 1   //Vent@20140228: For 3DNR enable.
            char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.3dnr.enable", EnableOption, "1");
            if (EnableOption[0] == '1')
            {
                #ifdef  MTK_CAM_NR3D_SUPPORT
                FEATURE_MASK_ENABLE_3DNR(mFeatureMask);
                #endif  // MTK_CAM_NR3D_SUPPORT
            }
            #else   // Original.
                #ifdef  MTK_CAM_NR3D_SUPPORT
                FEATURE_MASK_ENABLE_3DNR(mFeatureMask);
                #endif  // MTK_CAM_NR3D_SUPPORT
            #endif  //Vent@20140228: End.
        }
    }
#if 0    // this chip not support EIS2.0
    MBOOL isVFB = (::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0) ? MTRUE : MFALSE;
    if( spParamsMgr->getVideoStabilization() &&
        spParamsMgr->getRecordingHint() &&
        (mVdoSizeBeforePreview.w*mVdoSizeBeforePreview.h) <= IMG_1080P_SIZE &&
        !isVFB)
    {
        FEATURE_MASK_ENABLE_EIS(mFeatureMask);
    }
#endif
    //
    mbRecordingHint = spParamsMgr->getRecordingHint();
    //
    if(::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0)
    {
        FEATURE_MASK_ENABLE_VFB(mFeatureMask);
        //
        if(mbGDPreview)
        {
            FEATURE_MASK_ENABLE_GESTURE_SHOT(mFeatureMask);
        }
    }
    //
    if( !(CamManager::getInstance()->isMultiDevice()) &&
        spParamsMgr->getVHdr() == SENSOR_VHDR_MODE_IVHDR)
    {
        FEATURE_MASK_ENABLE_VHDR(mFeatureMask);
    }
    MY_LOGD("FeatureMask(0x%08X)",mFeatureMask);
}


/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
setCamMsgCb(sp<CamMsgCbInfo> spCamMsgCbInfo)
{
    Mutex::Autolock _l(mLock);
    //
    FUNC_NAME;
    mspCamMsgCbInfo = spCamMsgCbInfo;
}


/*******************************************************************************
*
********************************************************************************/
IHalPostProcPipe*
FeaturePass2::
createPipe()
{
    if( mPass2Type == PASS2_FEATURE )
    {
        mpFeaturePipe = IFeaturePipe::createInstance(getName(), getSensorIdx());
        return mpFeaturePipe;
    }

    MY_LOGE("not support type(0x%x)", mPass2Type);
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
FeaturePass2::
configFeature()
{
    Mutex::Autolock _l(mLock);
    //
    if(mspCamMsgCbInfo != NULL)
    {
        mpFeaturePipe->setCallbacks(mspCamMsgCbInfo);
    }
    //
    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureMask))
    {
        if( mbGDPreview &&
            !FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(mFeatureMask))
        {
            FEATURE_MASK_ENABLE_GESTURE_SHOT(mFeatureMask);
            MY_LOGD("FeatureMask(0x%08X)",mFeatureMask);
        }
    }
    //
    if(!mpFeaturePipe->setTuning(mFeaturePipeTuning))
    {
        MY_LOGE("FeaturePipe->setTuning fail");
    }
    //
    if(!mpFeaturePipe->setTouch(mTouch.x, mTouch.y))
    {
        MY_LOGE("FeaturePipe->setTouch fail");
    }
    // Must be before setFeature
    if(!mpFeaturePipe->set(RECORDING_HINT, (MINTPTR) mbRecordingHint))
    {
        MY_LOGE("FeaturePipe->set RECORDING_HINT fail");
    }
    //
    if(!mpFeaturePipe->setFeature(mFeatureMask))
    {
        MY_LOGE("FeaturePipe->setFeature fail");
    }
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
