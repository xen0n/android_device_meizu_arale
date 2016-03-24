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
#include <camera/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkEngCamAdapter.h"
using namespace NSMtkEngCamAdapter;
//


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
*   Function Prototype.
*******************************************************************************/
//
bool
createShotInstance(
    sp<IShot>&          rpShot,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);
//

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
updateShotInstance()
{
    String8 const s8ShotMode = getParamsManager()->getShotModeStr();
    uint32_t const u4ShotMode = getParamsManager()->getShotMode();
    MY_LOGI("<shot mode> %#x(%s)", u4ShotMode, s8ShotMode.string());
    //
    MY_LOGD("Engineer mode use EngShot only");
    return  createShotInstance(mpShot, eShotMode_EngShot, getOpenId(), getParamsManager()); // In engineer mode, force using EngShot
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
prepareToCapture()
{
    status_t status = NO_ERROR;
    sp<ZipImageCallbackThread> pCallbackThread;
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    pCallbackThread = new ZipImageCallbackThread(
                            mpStateManager,
                            mpCamMsgCbInfo);
    //
    if  ( !pCallbackThread.get() || OK != (status = pCallbackThread->run() ) )
    {
        MY_LOGE(
            "Fail to run CallbackThread - CallbackThread.get(%p), status[%s(%d)]",
            pCallbackThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    mpZipCallbackThread = pCallbackThread;
    pCallbackThread = 0;
    //
lbExit:
    return status;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
waitForShotDone()
{
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    //  wait for capture callback is done
    if  ( pCallbackThread.get() )
    {
        //post a fake data to make sure thread can exit
        ZipImageCallbackThread::callback_data callbackData;
        callbackData.type = ZipImageCallbackThread::callback_type_image;
        callbackData.pImage = NULL;
        callbackData.u4CallbackIndex = 0;
        callbackData.isFinalImage = true;
        pCallbackThread->addCallbackData(&callbackData);
        MY_LOGD("wait for capture callback done +");
        pCallbackThread->join();
        MY_LOGD("wait for capture callback done -");
        pCallbackThread = 0;
        mpZipCallbackThread = 0;
    }
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isTakingPicture() const
{
    bool ret =  mpStateManager->isState(IState::eState_PreCapture) ||
                mpStateManager->isState(IState::eState_Capture) ||
                mpStateManager->isState(IState::eState_PreviewCapture) ||
                mpStateManager->isState(IState::eState_VideoSnapshot);
    if  ( ret )
    {
        MY_LOGD("isTakingPicture(1):%s", mpStateManager->getCurrentState()->getName());
    }
    //
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
takePicture()
{
    status_t status = OK;
    //
    // wait for previous callback is done
    waitForShotDone();
    //
    if( OK != (status = prepareToCapture()) )
    {
        MY_LOGE("failed to prepareToCapture");
        goto lbExit;
    }
    //
    if( !mpEngCtrlNode->updateShotMode() )
    {
        MY_LOGE("update shot mode failed");
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    mShotMode = mpEngCtrlNode->getShotMode();
    mbTakePicPrvNotStop = false;
    //
    if(mpStateManager->isState(IState::eState_Recording))
    {
        if( ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT), MtkCameraParameters::PIXEL_FORMAT_BITSTREAM) == 0 ||
            mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30)
        {
            MY_LOGE("Not support VSS");
            status = INVALID_OPERATION;
            goto lbExit;
        }
        else
        {
            MY_LOGD("It is recording state, just capture.");
            mbTakePicPrvNotStop = true;
        }
    }
    else
    {
        if(mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) != MtkCameraParameters::CAMERA_MODE_NORMAL)
        {
            if(mShotMode == eShotMode_ZsdShot || mShotMode == eShotMode_ContinuousShotCc)
            {
                MY_LOGD("Take picture without stopping preview");
                mbTakePicPrvNotStop = true;
            }
        }
        //
        if(!mbTakePicPrvNotStop)
        {
            status = mpStateManager->getCurrentState()->onPreCapture(this);
            if(OK != status)
            {
                goto lbExit;
            }
            //
            status = mpStateManager->getCurrentState()->onStopPreview(this);
            if(OK != status)
            {
                goto lbExit;
            }
        }
    }
    //
    status = mpStateManager->getCurrentState()->onCapture(this);
    //
    if(OK != status)
    {
        goto lbExit;
    }
    //
lbExit:
    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
cancelPicture()
{
    if( mpStateManager->isState(IState::eState_Capture) ||
        mpStateManager->isState(IState::eState_PreviewCapture) ||
        mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        mpStateManager->getCurrentState()->onCancelCapture(this);
    }
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setCShotSpeed(int32_t i4CShotSpeed)
{
    if(i4CShotSpeed <= 0 )
    {
        MY_LOGE("cannot set continuous shot speed as %d fps)", i4CShotSpeed);
        return BAD_VALUE;
    }

    sp<IShot> pShot = mpShot;
    if( pShot != 0 )
    {
        pShot->sendCommand(eCmd_setCShotSpeed, i4CShotSpeed, 0);
    }

    return OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   IStateHandler::onHandleCapture() -> CamAdapter::onHandleCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCapture()
{
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(mShotMode, IState::eState_Idle);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    return  status;
}

/******************************************************************************
*
*******************************************************************************/
int
CamAdapter::
getFlashQuickCalibrationResult()
{
    MY_LOGD("+");
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return 1;
    }
    int result = pHal3a->getFlashQuickCalibrationResult();

    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return result;

}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCaptureDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_IDLE;
        mpResMgrDrv->setMode(&mode);
    }

    //
    mpStateManager->transitState(IState::eState_Idle);
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelCapture()
{
    MY_LOGD("");
    sp<IShot> pShot = mpShot;
    if  ( pShot != 0 )
    {
        pShot->sendCommand(eCmd_cancel);
    }
    //
    return  OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   IStateHandler::onHandleCapture() -> CamAdapter::onHandleCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreviewCapture()
{
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(getParamsManager()->getShotMode(), IState::eState_Preview);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    return  status;

}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandlePreviewCaptureDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        mpResMgrDrv->setMode(&mode);
    }
    //
    mpStateManager->transitState(IState::eState_Preview);
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelPreviewCapture()
{
    return  onHandleCancelCapture();
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshot()
{
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(getParamsManager()->getShotMode(), IState::eState_Recording);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    return  status;

}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshotDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= 30)
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC;
        }
        else
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC_HS;
        }
        mpResMgrDrv->setMode(&mode);
    }
    //
    mpStateManager->transitState(IState::eState_Recording);
    return  OK;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelVideoSnapshot()
{
    //TBD
    return  OK;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
onCaptureThreadLoop()
{
    bool ret = false;
    MUINT32 transform = 0;
    ResMgrDrv::MODE_STRUCT mode;
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->getMode(&mode);
    }
    //
    //  [1] transit to "Capture" state.
    if(mpStateManager->isState(IState::eState_Idle))
    {
        mpStateManager->transitState(IState::eState_Capture);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_CAP;
    }
    else
    if(mpStateManager->isState(IState::eState_Preview))
    {
        mpStateManager->transitState(IState::eState_PreviewCapture);
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
    }
    else
    if(mpStateManager->isState(IState::eState_Recording))
    {
        mpStateManager->transitState(IState::eState_VideoSnapshot);
        mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_VSS;
    }
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->setMode(&mode);
    }
    //
    if( mpStateManager->isState(IState::eState_VideoSnapshot) &&
        mpEngCtrlNode->isRecBufSnapshot())
    {
        mpRecBufSnapshotlNode->takePicture(
                                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH),
                                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT),
                                    mpParamsMgr->getInt(CameraParameters::KEY_ROTATION),
                                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY),
                                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY),
                                    this);
    }
    else
    {
        //*** Start of engineer mode part
        sp<IParamsManager> pParamsMgr = getParamsManager();
        // only for testing
        //pParamsMgr->set(MtkCameraParameters::KEY_ENG_CAPTURE_SHUTTER_SPEED, 100);
        //pParamsMgr->set(MtkCameraParameters::KEY_ENG_CAPTURE_SENSOR_GAIN, 2048);
        // manual exposure time / gain
        engParam.u4Exposuretime =  pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SHUTTER_SPEED);
        engParam.u4SensorGain = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SENSOR_GAIN);

        engParam.u4EVvalue = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_EV_VALUE); //

        //EVBreacket enable
        engParam.mi4EngRawEVBEn = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_EVB_ENABLE);

        MY_LOGD("get ev-value from ap: %d ", engParam.u4EVvalue);

        // Multi-NR
        engParam.mi4ManualMultiNREn = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_ENABLE);
        if (1 == engParam.mi4ManualMultiNREn)
        {
            engParam.mi4ManualMultiNRType = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_TYPE);
        }

        // CCT MFLL
        engParam.mi4CamShotType = EngParam::ENG_CAMSHOT_TYPE_SINGLE; // default value
        if (pParamsMgr->getStr(MtkCameraParameters::KEY_MFB_MODE) == MtkCameraParameters::KEY_MFB_MODE_MFLL)
        {
            engParam.mi4CamShotType = EngParam::ENG_CAMSHOT_TYPE_MFLL;
        }
        MY_LOGD("engParam.mi4CamShotType = %d", engParam.mi4CamShotType);

        String8 ms8IspMode = pParamsMgr->getStr(MtkCameraParameters::KEY_ISP_MODE);
        const char *strIspMode = ms8IspMode.string();
        engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_PURE_RAW; // Default value
        switch (strIspMode[0])
        {
            case EngParam::ENG_ISP_MODE_PROCESSED_RAW:
                //engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_PROCESSED_RAW;
                engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_FORCE_PROCESSED_RAW;
                break;
            case EngParam::ENG_ISP_MODE_PURE_RAW:
                engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_PURE_RAW;

                if (pParamsMgr->getStr(MtkCameraParameters::KEY_MFB_MODE) == MtkCameraParameters::KEY_MFB_MODE_MFLL)
                {
                    pParamsMgr->set(MtkCameraParameters::KEY_ISP_MODE, EngParam::ENG_ISP_MODE_PROCESSED_RAW - '0');
                    engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_PROCESSED_RAW;
                    MY_LOGW("MFLL forces to process raw from pure raw");
                }
                break;
        }
        MY_LOGD("engParam.mi4EngIspMode = %d", engParam.mi4EngIspMode);

        engParam.u4VHDState = pParamsMgr->getVHdr();
        MY_LOGD("u4VHDState = %d", engParam.u4VHDState);

        String8 ms8SaveMode = pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);
        const char *strSaveMode = ms8SaveMode.string();
        #warning "[TODO] Magic number: need to enumerize"
        switch (atoi(strSaveMode))
        {
            case 1: // 1: "Preview Mode",
                engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_PREVIEW;
                engParam.mi4EngRawSaveEn = 1;
                break;
            case 2: // 2: "Capture Mode",
                engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE;
                engParam.mi4EngRawSaveEn = 1;
                break;
            case 4: // 4: "Video Preview Mode"
                engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO;
                engParam.mi4EngRawSaveEn = 1;
                break;
            case 5: // 4: "Slim Video 1"
                engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1;
                engParam.mi4EngRawSaveEn = 1;
                break;
            case 6: // 4: "Slim Video 2"
                engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2;
                engParam.mi4EngRawSaveEn = 1;
                break;
            case 0: // 0: do not save
            case 3: // 3: "JPEG Only"
                engParam.mi4EngRawSaveEn = 0;
                break;
            default:
                   // Extension for sensor scenario, map to driver scenario
                   if (atoi(strSaveMode) > 6)
                   {
                       engParam.mi4EngSensorMode = atoi(strSaveMode) - 2;
                       engParam.mi4EngRawSaveEn = 1;
                   }
                   else
                   {
                       engParam.mi4EngRawSaveEn = 0;
                   }
                break;
        }
        MY_LOGD("mi4EngRawSaveEn, mi4EngSensorMode =(%d, %d)", engParam.mi4EngRawSaveEn, engParam.mi4EngSensorMode);
        //*** End of engineer mode part

        //  [2.1] update mpShot instance.
        ret = updateShotInstance();
        sp<IShot> pShot = mpShot;
        //
        //  [2.2] return if no shot instance.
        if  ( ! ret || pShot == 0 )
        {
            #warning "[TODO] perform a dummy compressed-image callback or CAMERA_MSG_ERROR to inform app of end of capture?"
            MY_LOGE("updateShotInstance(%d), pShot.get(%p)", ret, pShot.get());
            goto lbExit;
        }
        else
        {
            //  [3.1] prepare parameters
            sp<IParamsManager> pParamsMgr = getParamsManager();
            //
            int iPictureWidth = 0, iPictureHeight = 0;
            if(mpStateManager->isState(IState::eState_VideoSnapshot))
            {
                mpEngCtrlNode->getVideoSnapshotSize(iPictureWidth, iPictureHeight);
            }
            else
            {
                pParamsMgr->getPictureSize(&iPictureWidth, &iPictureHeight);
            }
            //
            int iPreviewWidth = 0, iPreviewHeight = 0;
            pParamsMgr->getPreviewSize(&iPreviewWidth, &iPreviewHeight);
            String8 s8DisplayFormat = mpImgBufProvidersMgr->queryFormat(IImgBufProvider::eID_DISPLAY);
            if  ( String8::empty() == s8DisplayFormat ) {
                MY_LOGW("Display Format is empty");
            }
            //
            bool setCapBufMgr = MFALSE;
            if(mName == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD ||       //ZSD preview
                    mpStateManager->isState(IState::eState_VideoSnapshot) ) //VSS shot
            {
                setCapBufMgr = MTRUE;
            }
            // convert rotation to transform
            MINT32 rotation = pParamsMgr->getInt(CameraParameters::KEY_ROTATION);
            //
            switch(rotation)
            {
                case 0:
                    transform = 0;
                    break;
                case 90:
                    transform = eTransform_ROT_90;
                    break;
                case 180:
                    transform = eTransform_ROT_180;
                    break;
                case 270:
                    transform = eTransform_ROT_270;
                    break;
                default:
                    break;
            }

            //  [3.2] prepare parameters: ShotParam
            // ShotParam shotParam;
            engParam.miPictureFormat           = NSCam::Utils::Format::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PICTURE_FORMAT));
            if  ( 0 != iPictureWidth && 0 != iPictureHeight )
            {
                engParam.mi4PictureWidth       = iPictureWidth;
                engParam.mi4PictureHeight      = iPictureHeight;
            }
            else
            {   //  When 3rd-party apk sets "picture-size=0x0", replace it with "preview-size".
                engParam.mi4PictureWidth       = iPreviewWidth;
                engParam.mi4PictureHeight      = iPreviewHeight;
            }
            engParam.miPostviewDisplayFormat   = NSCam::Utils::Format::queryImageFormat(s8DisplayFormat.string());
            engParam.miPostviewClientFormat    = NSCam::Utils::Format::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PREVIEW_FORMAT));
            engParam.mi4PostviewWidth          = iPreviewWidth;
            engParam.mi4PostviewHeight         = iPreviewHeight;
            engParam.ms8ShotFileName           = pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH);
            engParam.mu4ZoomRatio              = pParamsMgr->getZoomRatio();
            engParam.mu4ShotCount              = pParamsMgr->getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
            engParam.mu4Transform              = transform;
            engParam.mu4MultiFrameBlending     = pParamsMgr->getMultFrameBlending();
            //
            //  [3.3] prepare parameters: JpegParam
            JpegParam jpegParam;
            jpegParam.mu4JpegQuality            = pParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY);
            jpegParam.mu4JpegThumbQuality       = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
            jpegParam.mi4JpegThumbWidth         = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
            jpegParam.mi4JpegThumbHeight        = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
            jpegParam.ms8GpsLatitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE);
            jpegParam.ms8GpsLongitude           = pParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE);
            jpegParam.ms8GpsAltitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE);
            jpegParam.ms8GpsTimestamp           = pParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP);
            jpegParam.ms8GpsMethod              = pParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD);
            //
            //  [4.1] perform Shot operations.
            if(!mbTakePicPrvNotStop)
            {
                mpEngBufHdl->mapPort(EngBufHandler::eBuf_Disp, PASS2_PRV_DST_0, MTRUE);
                mpEngBufHdl->mapPort(EngBufHandler::eBuf_Disp, PASS2_CAP_DST_0);
            }
            //
            ret =
                    pShot->sendCommand(eCmd_reset)
                &&  pShot->setCallback(this)
                &&  pShot->sendCommand(eCmd_setShotParam, (MUINTPTR)&engParam, sizeof(EngParam))
                &&  pShot->sendCommand(eCmd_setJpegParam, (MUINTPTR)&jpegParam, sizeof(JpegParam))
                &&  pShot->sendCommand(eCmd_setPrvBufHdl, (MUINTPTR)mpEngBufHdl, sizeof(EngBufHandler))
                &&  ( setCapBufMgr ?
                        pShot->sendCommand(eCmd_setCapBufMgr, (MUINTPTR)mpCapBufMgr, sizeof(CapBufMgr)) :
                        MTRUE )
                &&  pShot->sendCommand(eCmd_capture)
                    ;
            if  ( ! ret )
            {
                MY_LOGE("fail to perform shot operations");
            }
            //
            if(!mbTakePicPrvNotStop)
            {
                mpEngBufHdl->unmapPort(EngBufHandler::eBuf_Disp);
            }
        }
        //
        //
        lbExit:
        //
        //  [5.1] uninit shot instance.
        MY_LOGD("free shot instance: (mpShot/pShot)=(%p/%p)", mpShot.get(), pShot.get());
        mpShot = NULL;
        pShot  = NULL;
        //  [5.11] update focus steps.
        //
        pParamsMgr->updateBestFocusStep();
        //
        //  [5.2] notify capture done.
    }

    // update flash calibration result.
    int flashCaliEn = mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_FLASH_CALIBRATION);
    if (flashCaliEn == 1)
    {
        if (getFlashQuickCalibrationResult() != 0)
        {
            onCB_Error(CAMERA_ERROR_CALI_FLASH, 0);
        }
    }

    //
    mpStateManager->getCurrentState()->onCaptureDone(this);
    //
    //
    return  true;
}

