/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkDefaultCamAdapter.h"
using namespace NSMtkDefaultCamAdapter;
//
#include "camera_custom_eis.h"

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setParameters()
{
    MY_LOGD("+");
    status_t ret = OK;
    //
    #define UPDATE_PARAMS(param, eMapXXX, key) \
    do { \
        String8 const s = mpParamsMgr->getStr(key); \
        if  ( ! s.isEmpty() ) { \
            param = PARAMSMANAGER_MAP_INST(eMapXXX)->valueFor(s); \
        } \
    } while (0)
    //
    if(mpPass2Node)
    {
        mpPass2Node->setParameters(
                        mpParamsMgr,
                        getOpenId());
    }
    //
    if(mpDefaultCtrlNode)
    {
        mpDefaultCtrlNode->setParameters();
    }
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    NS3A::Param_T cam3aParam;
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());
    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }
    //get Param from 3A
    if ( ! pHal3a->getParams(cam3aParam) )
    {
        MY_LOGE("getParams fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    //Check awb mode change
    {
        int32_t newParam = 0;
        UPDATE_PARAMS(newParam, eMapWhiteBalance, CameraParameters::KEY_WHITE_BALANCE);
        Param_T oldParamAll;
        int32_t oldParam = cam3aParam.u4AwbMode;
        if (newParam != oldParam)
        {
            mpDefaultCtrlNode->updateDelay(EQueryType_AWB);
            MY_LOGD("AWB mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }
    //check effect mode change
    {
        int32_t newParam = 0;
        UPDATE_PARAMS(newParam, eMapEffect, CameraParameters::KEY_EFFECT);
        Param_T oldParamAll;
        int32_t oldParam = cam3aParam.u4EffectMode;
        if (newParam != oldParam)
        {
            mpDefaultCtrlNode->updateDelay(EQueryType_Effect);
            MY_LOGD("EFFECT mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }
    //
    {
        //TBD: update ZOOM
    }
    // set Param to 3A
    // DEFAULT DEFINITION CATEGORY
    if( mpStateManager->isState(IState::eState_Recording) &&
        mspThermalMonitor->isRunning())
    {
        //Do nothing.
    }
    else
    {
        CamManager* pCamMgr = CamManager::getInstance();
        String8 const s = mpParamsMgr->getStr(MtkCameraParameters::KEY_DYNAMIC_FRAME_RATE);
        MBOOL const isDynamicFps = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? MTRUE : MFALSE;
        MUINT32 const previewFps = mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE);
        MUINT32 const fixedFps   = ( !mpParamsMgr->getRecordingHint() && (previewFps > 30) ) ? 30 : previewFps;
        MBOOL const isSlowMotion = ( mpParamsMgr->getRecordingHint() && (previewFps > 30) ) ? MTRUE : MFALSE;

        MBOOL isEisWithDfr;
        if(GetEisLinkWithDfr() && mpParamsMgr->getVHdr() != NSCam::SENSOR_VHDR_MODE_MVHDR)
        {
            isEisWithDfr = mpParamsMgr->getRecordingHint() ? (mpParamsMgr->getVideoStabilization() ? MFALSE : MTRUE) : MTRUE;
        }
        else
        {
            isEisWithDfr = MTRUE;
        }

        if ( isDynamicFps && !pCamMgr->isMultiDevice() && !isSlowMotion && isEisWithDfr)
        {
            mbFixFps = false;
            if(mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) != MtkCameraParameters::CAMERA_MODE_NORMAL)
            {
                cam3aParam.i4MinFps   = 5000;
                if(mPreviewMaxFps == 0)
                {
                    cam3aParam.i4MaxFps = mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE)*1000;
                }
                else
                if(mPreviewMaxFps <= previewFps)
                {
                    cam3aParam.i4MaxFps = mPreviewMaxFps*1000;
                }
                else
                {
                    cam3aParam.i4MaxFps = previewFps*1000;
                }
            }
            else //For M0.TK CTS android.hardware.cts.CameraGLTest # testCameraToSurfaceTextureMetadata
            {
                String8 const s = mpParamsMgr->getStr(CameraParameters::KEY_PREVIEW_FPS_RANGE);
                if (s.isEmpty())
                {
                    return INVALID_OPERATION;
                }
                const char* p = s;
                char* q;
                cam3aParam.i4MinFps = strtol(p,&q,10);
                cam3aParam.i4MaxFps = strtol(q+1,&q,10);
            }
        }
        else
        {
            mbFixFps = true;
            if(mPreviewMaxFps == 0)
            {
                cam3aParam.i4MinFps = fixedFps*1000;
                cam3aParam.i4MaxFps = fixedFps*1000;
            }
            else
            if(mPreviewMaxFps <= fixedFps)
            {
                cam3aParam.i4MinFps = mPreviewMaxFps*1000;
                cam3aParam.i4MaxFps = mPreviewMaxFps*1000;
            }
            else
            {
                cam3aParam.i4MinFps = fixedFps*1000;
                cam3aParam.i4MaxFps = fixedFps*1000;
            }
        }
        MY_LOGD("FPS:RH(%d),isDynamicFps(%d),isSlowMotion(%d),bFixFps(%d),mPreviewMaxFps(%d),previewFps(%d),Min(%d),Max(%d)",
                mpParamsMgr->getRecordingHint(),
                isDynamicFps,
                isSlowMotion,
                mbFixFps,
                mPreviewMaxFps,
                previewFps,
                cam3aParam.i4MinFps,
                cam3aParam.i4MaxFps);
    }
    //
    UPDATE_PARAMS(cam3aParam.u4AfMode, eMapFocusMode, CameraParameters::KEY_FOCUS_MODE);
    UPDATE_PARAMS(cam3aParam.u4AwbMode, eMapWhiteBalance, CameraParameters::KEY_WHITE_BALANCE);
    UPDATE_PARAMS(cam3aParam.u4SceneMode, eMapScene, CameraParameters::KEY_SCENE_MODE);
    UPDATE_PARAMS(cam3aParam.u4StrobeMode, eMapFlashMode, CameraParameters::KEY_FLASH_MODE);
    UPDATE_PARAMS(cam3aParam.u4EffectMode, eMapEffect, CameraParameters::KEY_EFFECT);
    UPDATE_PARAMS(cam3aParam.u4AntiBandingMode, eMapAntiBanding, CameraParameters::KEY_ANTIBANDING);
    //
    cam3aParam.i4ExpIndex = mpParamsMgr->getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    cam3aParam.fExpCompStep = mpParamsMgr->getFloat(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP);
    //
    // AE lock
    {
        String8 const s = mpParamsMgr->getStr(CameraParameters::KEY_AUTO_EXPOSURE_LOCK);
        cam3aParam.bIsAELock = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? 1 : 0;
    }
    // AWB lock
    {
        String8 const s = mpParamsMgr->getStr(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK);
        cam3aParam.bIsAWBLock = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? 1 : 0;
    }
    // AF Focus Areas
    {
        String8 const s8Area = mpParamsMgr->getStr(CameraParameters::KEY_FOCUS_AREAS);
        if  ( ! s8Area.isEmpty() )
        {
            MY_LOGD("Focus Areas:%s", s8Area.string());
            const int maxNumFocusAreas = mpParamsMgr->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
            List <camera_area_t> focusAreas;
            mpParamsMgr->parseCamAreas(s8Area.string(), focusAreas, maxNumFocusAreas);
            int index = 0;
            for (List<camera_area_t>::iterator it = focusAreas.begin(); it != focusAreas.end(); it++) {
                cam3aParam.rFocusAreas.rAreas[index].i4Left = it->left;
                cam3aParam.rFocusAreas.rAreas[index].i4Top = it->top;
                cam3aParam.rFocusAreas.rAreas[index].i4Right = it->right;
                cam3aParam.rFocusAreas.rAreas[index].i4Bottom = it->bottom;
                cam3aParam.rFocusAreas.rAreas[index].i4Weight = it->weight;
                index++;
            }
            cam3aParam.rFocusAreas.u4Count = focusAreas.size();
        }
    }
    // AE Metering Areas
    {
        String8 const s8Area = mpParamsMgr->getStr(CameraParameters::KEY_METERING_AREAS);
        if  ( ! s8Area.isEmpty() )
        {
            MY_LOGD("Metering Areas:%s", s8Area.string());
            const int maxNumMeteringAreas = mpParamsMgr->getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS);
            List <camera_area_t> meterAreas;
            mpParamsMgr->parseCamAreas(s8Area.string(), meterAreas, maxNumMeteringAreas);
            int index = 0;
            for (List<camera_area_t>::iterator it = meterAreas.begin(); it != meterAreas.end(); it++) {
                cam3aParam.rMeteringAreas.rAreas[index].i4Left = it->left;
                cam3aParam.rMeteringAreas.rAreas[index].i4Top = it->top;
                cam3aParam.rMeteringAreas.rAreas[index].i4Right = it->right;
                cam3aParam.rMeteringAreas.rAreas[index].i4Bottom = it->bottom;
                cam3aParam.rMeteringAreas.rAreas[index].i4Weight = it->weight;
                index++;
            }
            cam3aParam.rMeteringAreas.u4Count = meterAreas.size();
        }
    }

    // MTK DEFINITION CATEGORY
    UPDATE_PARAMS(cam3aParam.u4AeMode, eMapExpMode, MtkCameraParameters::KEY_SCENE_MODE);
    UPDATE_PARAMS(cam3aParam.u4IsoSpeedMode, eMapIso, MtkCameraParameters::KEY_ISO_SPEED);
    //
    UPDATE_PARAMS(cam3aParam.u4BrightnessMode, eMapLevel, MtkCameraParameters::KEY_BRIGHTNESS);
    UPDATE_PARAMS(cam3aParam.u4SaturationMode, eMapLevel, MtkCameraParameters::KEY_SATURATION);
    UPDATE_PARAMS(cam3aParam.u4ContrastMode, eMapLevel, MtkCameraParameters::KEY_CONTRAST);
    UPDATE_PARAMS(cam3aParam.u4EdgeMode, eMapLevel, MtkCameraParameters::KEY_EDGE);
    UPDATE_PARAMS(cam3aParam.u4HueMode, eMapLevel, MtkCameraParameters::KEY_HUE);
    //
    cam3aParam.u4CamMode    = eAppMode_DefaultMode;
    cam3aParam.i4RotateDegree = mpParamsMgr->getInt(MtkCameraParameters::KEY_ROTATION);
    //
    if( mpParamsMgr->getShotMode() != eShotMode_ContinuousShot )
    {
        cam3aParam.u4CapType = ECapType_SingleCapture;
    }
    else
    {
        cam3aParam.u4CapType = ECapType_MultiCapture;
    }
    //
    if ( ! pHal3a->setParams(cam3aParam) )
    {
        MY_LOGE("setParams fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    MY_LOGD("-");

    return ret;
}

