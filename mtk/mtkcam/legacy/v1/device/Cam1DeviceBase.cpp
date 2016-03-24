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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include <device/Cam1DeviceBase.h>
#include <camera/MtkCamera.h>
//
using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[Cam1DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
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
 ******************************************************************************/
Cam1DeviceBase::
Cam1DeviceBase(
    String8 const&          rDevName,
    int32_t const           i4OpenId
)
    : Cam1Device()
    //
    , mpDeviceManager(NULL)
    , mDevName(rDevName)
    , mi4OpenId(i4OpenId)
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    //
    , mpParamsMgr(IParamsManager::createInstance(rDevName, i4OpenId))
    , mpCamAdapter(NULL)
    , mpCamClient()
    , mpDisplayClient()
    , vmpCamClient()
    , mIsPreviewEnabled(false)
    , mIsRaw16CBEnabled(false)
    //
    , mTodoCmdMap()
    , mTodoCmdMapLock()
    //
    , mOrientation(0)
    //
    , mLastEnableMsg(0)
    , mStartPreviewTThreadHandle(0)
    , mbWindowReady(false)
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
Cam1DeviceBase::
~Cam1DeviceBase()
{
    MY_LOGD("");
    mpDisplayClient.clear();
    mpCamClient.clear();
    mpCamAdapter.clear();
    vmpCamClient.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
setDeviceManager(ICamDeviceManager* manager)
{
    mpDeviceManager = manager;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
onInit()
{
    MY_LOGD("+");
    //
    bool    ret = false;
    //
    //  (0) Initialize Device Callback.
    mpCamMsgCbInfo->mCam1DeviceCb = static_cast<ICam1DeviceCallback*>(this);
    //
    //  (1) Initialize Parameters.
    if  ( ! mpParamsMgr->init() )
    {
        goto lbExit;
    }
    //
    //  (2) Create & Initialize ICamClient.
    mpCamClient = ICamClient::createInstance(mpParamsMgr);
    if  ( mpCamClient == 0 || ! mpCamClient->init() )
    {
        MY_LOGE("mpCamClient(%p)->init() fail", mpCamClient.get());
        goto lbExit;
    }
    //
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
onUninit()
{
    MY_LOGD("+");
    //
    //
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->uninit();
        mpCamClient.clear();
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->uninit();
    }
    vmpCamClient.clear();
    //
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    //
    //
    mpParamsMgr->uninit();
    //
    //  Uninitialize Device Callback.
    mpCamMsgCbInfo->mCam1DeviceCb = NULL;
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
closeDevice()
{
    return  mpDeviceManager->close(this);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
initialize()
{
    CAM_TRACE_CALL();
    if  ( ! onInit() )
    {
        return  -ENODEV;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
uninitialize()
{
    CAM_TRACE_CALL();
    onUninit();
}


/******************************************************************************
 *  Set the notification and data callbacks
 ******************************************************************************/
void
Cam1DeviceBase::
setCallbacks(
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    camera_request_memory get_memory,
    void*user
)
{
    mpCamMsgCbInfo->mCbCookie       = user;
    mpCamMsgCbInfo->mNotifyCb       = notify_cb;
    mpCamMsgCbInfo->mDataCb         = data_cb;
    mpCamMsgCbInfo->mDataCbTimestamp= data_cb_timestamp;
    mpCamMsgCbInfo->mRequestMemory  = get_memory;
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->setCallbacks(mpCamMsgCbInfo);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->setCallbacks(mpCamMsgCbInfo);
    }

    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
    }
}


void
Cam1DeviceBase::
setCallbacks(
    mtk_camera_metadata_callback metadata_cb,
    void*user
)
{
    mpCamMsgCbInfo->mMetadataCb = metadata_cb;
}
/******************************************************************************
 *  Enable a message, or set of messages.
 ******************************************************************************/
void
Cam1DeviceBase::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->enableMsgType(msgType);
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->enableMsgType(msgType);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->enableMsgType(msgType);
    }
    //
    mLastEnableMsg = msgType;
}


/******************************************************************************
 *  Disable a message, or a set of messages.
 *
 *  Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
 *  should not rely on its client to call releaseRecordingFrame() to release
 *  video recording frames sent out by the cameral hal before and after the
 *  disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
 *  modify/access any video recording frame after calling
 *  disableMsgType(CAMERA_MSG_VIDEO_FRAME).
 ******************************************************************************/
void
Cam1DeviceBase::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->disableMsgType(msgType);
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->disableMsgType(msgType);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->disableMsgType(msgType);
    }
}


/******************************************************************************
 * Query whether a message, or a set of messages, is enabled.
 * Note that this is operates as an AND, if any of the messages
 * queried are off, this will return false.
 ******************************************************************************/
bool
Cam1DeviceBase::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
 *  Set the preview_stream_ops to which preview frames are sent.
 ******************************************************************************/
status_t
Cam1DeviceBase::
setPreviewWindow(preview_stream_ops* window)
{
    CAM_TRACE_CALL();
    MY_LOGI("+ window(%p)", window);
    //
    status_t status = initDisplayClient(window);
    if  ( OK == status && previewEnabled() && mpDisplayClient != 0 )
    {
        status = enableDisplayClient();
        if(mbWindowReady)
        {
            waitStartPreviewDone();
        }
    }
    //
    return  status;
}


/******************************************************************************
 *  Start preview mode.
 ******************************************************************************/
status_t
Cam1DeviceBase::
startPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    status_t status = OK;
    bool usePreviewThread = false;
    //
    if( mpParamsMgr->getIfFirstPreviewFrameAsBlack() &&
        mbWindowReady == false)
    {
        usePreviewThread = true;
        disableWaitSensorThread(true);
    }
    //
    {
        CAM_TRACE_NAME("deviceStartPreview");
        if  ( mpCamAdapter != 0 && mpCamAdapter->isTakingPicture() )
        {
            MY_LOGE("Capture is not done");
            status = INVALID_OPERATION;
            return  status;
        }
        //
        if  ( previewEnabled() )
        {
            MY_LOGD("Preview already running");
            status = ALREADY_EXISTS;
            return  status;
        }
        //
        if  ( ! onStartPreview() )
        {
            MY_LOGE("onStartPreviewLocked() fail");
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStartPreview");
        if  ( mpDisplayClient == 0 )
        {
            MY_LOGD("DisplayClient is not ready.");
        }
        else if ( OK != (status = enableDisplayClient()) )
        {
            goto lbExit;
        }
        //
        if  ( mpCamClient != 0 )
        {
            if  ( ! mpCamClient->startPreview() )
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startPreview();
        }
    }

    //
    //  startPreview in Camera Adapter.
    {
        if(usePreviewThread)
        {
            if( pthread_create(&mStartPreviewTThreadHandle, NULL, startPreviewThread, this) != 0 )
            {
                ALOGE("startPreview pthread create failed");
            }
        }
        else
        {
            CAM_TRACE_NAME("adapterStartPreview");
            status = mpCamAdapter->startPreview();
            if  ( OK != status )
            {
                MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
                goto lbExit;
            }
        }
    }
    //
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    mIsPreviewEnabled = true;
    //
    status = OK;
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup after error");
        //
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopPreview();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        //
        disableDisplayClient();
    }
    //
    MY_LOGI("- status(%d)", status);
    return  status;
}


/******************************************************************************
 *  Stop a previously started preview.
 ******************************************************************************/
void
Cam1DeviceBase::
stopPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    waitStartPreviewDone();
    //
    if  ( ! previewEnabled() )
    {
        MY_LOGD("Preview already stopped, perhaps!");
        MY_LOGD("We still force to clean up again.");
    }
    //
    {
        CAM_TRACE_NAME("adapterStopPreview");
        if  ( mpCamAdapter != 0 )
        {
            if(recordingEnabled())
            {
                stopRecording();
            }
            mpCamAdapter->stopPreview();
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStopPreview");
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopPreview();
        }
        //
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        //
        disableDisplayClient();
    }
    //
    //
    {
        CAM_TRACE_NAME("deviceStopPreview");
        onStopPreview();
    }
    //
    //
#if 1
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->waitUntilDrained();
    }
#endif
    //
    //
lbExit:
    //  Always set it to false.
    mIsPreviewEnabled = false;
    MY_LOGI("-");
}


/******************************************************************************
 *  Returns true if preview is enabled.
 ******************************************************************************/
bool
Cam1DeviceBase::
previewEnabled()
{
    MY_LOGV("mIsPreviewEnabled:%d, mpCamAdapter:%p", mIsPreviewEnabled, mpCamAdapter.get());
    //
    if  ( ! mIsPreviewEnabled )
    {
        return  false;
    }
    //
    if(mStartPreviewTThreadHandle != 0)
    {
        return true;
    }
    //
lbExit:
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->previewEnabled()
        ;
}


status_t
Cam1DeviceBase::
storeMetaDataInBuffers(bool enable)
{
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamClient != 0 )
    {
        //  Get recording format & size.
        //  Set.
        if  ( OK != mpCamClient->storeMetaDataInBuffers(enable) )
        {
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
lbExit:
    return  status;
}


/******************************************************************************
 * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
 * message is sent with the corresponding frame. Every record frame must be released
 * by a cameral hal client via releaseRecordingFrame() before the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
 * to manage the life-cycle of the video recording frames, and the client must
 * not modify/access any video recording frames.
 ******************************************************************************/
status_t
Cam1DeviceBase::
startRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    // zsd -> default
    if ( !strcmp(mpCamAdapter->getName(), MtkCameraParameters::APP_MODE_NAME_MTK_ZSD) )
    {
        MY_LOGW("invalid camera adapter: %s", mpCamAdapter->getName());
        stopPreview();
        mpParamsMgr->setForceHalAppMode(String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT));
        startPreview();
    }
    //  startRecording in Camera Adapter.
    {
        CAM_TRACE_NAME("adapterStartRecording");
        status = mpCamAdapter->startRecording();
        if  ( OK != status )
        {
            MY_LOGE("startRecording() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStartRecording");
        if  ( mpCamClient != 0 )
        {
            //  Get recording format & size.
            //  Set.
            if  ( ! mpCamClient->startRecording() )
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startRecording();
        }
    }
lbExit:
    return  status;
}


/******************************************************************************
 *  Stop a previously started recording.
 ******************************************************************************/
void
Cam1DeviceBase::
stopRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    {
        CAM_TRACE_NAME("adapterStopRecording");
        if  ( mpCamAdapter != 0 )
        {
            mpCamAdapter->stopRecording();
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStopRecording");
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopRecording();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopRecording();
        }
    }
    // check if need default -> zsd
    if  ( mpCamAdapter != 0 )
    {
        String8 halappmode;
        if  ( mpParamsMgr->evaluateHalAppMode(halappmode) &&
                halappmode != mpCamAdapter->getName() )
        {
            stopPreview();
            startPreview();
        }
    }
}


/******************************************************************************
 *  Returns true if recording is enabled.
 ******************************************************************************/
bool
Cam1DeviceBase::
recordingEnabled()
{
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->recordingEnabled()
        ;
}


/******************************************************************************
 *  Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
 *
 *  It is camera hal client's responsibility to release video recording
 *  frames sent out by the camera hal before the camera hal receives
 *  a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
 *  the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
 *  responsibility of managing the life-cycle of the video recording
 *  frames.
 ******************************************************************************/
void
Cam1DeviceBase::
releaseRecordingFrame(const void *opaque)
{
    if  ( mpCamClient != 0 )
    {
        mpCamClient->releaseRecordingFrame(opaque);
    }
    // forward to registered clients
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->releaseRecordingFrame(opaque);
    }
}


/******************************************************************************
 *  Start auto focus, the notification callback routine is called
 *  with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
 *  will be called again if another auto focus is needed.
 ******************************************************************************/
status_t
Cam1DeviceBase::
autoFocus()
{
    MY_LOGD("+");
    //
    if  ( ! previewEnabled() )
    {
        MY_LOGW("preview is not enabled");
        return OK;
    }
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->autoFocus()
        ;
}


/******************************************************************************
 * Cancels auto-focus function. If the auto-focus is still in progress,
 * this function will cancel it. Whether the auto-focus is in progress
 * or not, this function will return the focus position to the default.
 * If the camera does not support auto-focus, this is a no-op.
 ******************************************************************************/
status_t
Cam1DeviceBase::
cancelAutoFocus()
{
    status_t status = ( mpCamAdapter == 0 )
        ?   (status_t)OK
        :   mpCamAdapter->cancelAutoFocus()
        ;
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    return  status;
}


/******************************************************************************
 *  Take a picture.
 ******************************************************************************/
status_t
Cam1DeviceBase::
takePicture()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    if(!(mLastEnableMsg & CAMERA_MSG_SHUTTER))
    {
        MY_LOGD("Disable shutter sound");
        disableMsgType(CAMERA_MSG_SHUTTER);
    }
    //
    {
        CAM_TRACE_NAME("clientTakePicture");
        if(!recordingEnabled())
        {
            disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
        }
        //
        if  ( mpCamClient != 0 )
        {
            mpCamClient->takePicture();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->takePicture();
        }
    }
    //
    {
        CAM_TRACE_NAME("adapterTakePicture");
        //  takePicture in Camera Adapter.
        if(mIsRaw16CBEnabled)
        {
            MY_LOGD("CAMERA_CMD_ENABLE_RAW16_CALLBACK:1");
            mpCamAdapter->sendCommand(CAMERA_CMD_ENABLE_RAW16_CALLBACK, 1, 0);
        }
        //
        status = mpCamAdapter->takePicture();
        if  ( OK != status )
        {
            MY_LOGE("CamAdapter->takePicture() returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }

lbExit:
    return  status;
}


/******************************************************************************
 *  Cancel a picture that was started with takePicture.  Calling this
 *  method when no picture is being taken is a no-op.
 ******************************************************************************/
status_t
Cam1DeviceBase::
cancelPicture()
{
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->cancelPicture()
        ;
}


/******************************************************************************
 * Set the camera parameters. This returns BAD_VALUE if any parameter is
 * invalid or not supported.
 ******************************************************************************/
status_t
Cam1DeviceBase::
setParameters(const char* params)
{
    CAM_TRACE_CALL();
    status_t status = OK;
    //
    //  (1) Update params to mpParamsMgr.
    status = mpParamsMgr->setParameters(String8(params));
    if  ( OK != status ) {
        goto lbExit;
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    {
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
    }

lbExit:
    return  status;
}


/******************************************************************************
 *  Return the camera parameters.
 ******************************************************************************/
char*
Cam1DeviceBase::
getParameters()
{
    String8 params_str8 = mpParamsMgr->flatten();
    // camera service frees this string...
    uint32_t const params_len = sizeof(char) * (params_str8.length()+1);
    char*const params_string = (char*)::malloc(params_len);
    if  ( params_string )
    {
        ::strcpy(params_string, params_str8.string());
    }
    //
    MY_LOGV_IF(0, "- params(%p), len(%d)", params_string, params_len);
    return params_string;
}


/******************************************************************************
 * Put the camera parameters.
 ******************************************************************************/
void
Cam1DeviceBase::
putParameters(char *params)
{
    if  ( params )
    {
        ::free(params);
    }
    MY_LOGV_IF(0, "- params(%p)", params);
}


/******************************************************************************
 * Send command to camera driver.
 ******************************************************************************/
status_t
Cam1DeviceBase::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    status_t status = DEAD_OBJECT;
    //for display rotation
    switch  (cmd)
    {
    case CAMERA_CMD_SET_DISPLAY_ORIENTATION:
        mOrientation = arg1;
        MY_LOGD("CAMERA_CMD_SET_DISPLAY_ORIENTATION mOrientation = %d",mOrientation);
        break;
    default:
        break;
    }
    //check raw16 CB enable
    switch  (cmd)
    {
    case CAMERA_CMD_ENABLE_RAW16_CALLBACK:
         if(arg1 == 1)
            mIsRaw16CBEnabled = true;
         else if(arg1 == 0)
            mIsRaw16CBEnabled = false;
         MY_LOGD("CAMERA_CMD_ENABLE_RAW16_CALLBACK mIsRaw16CBEnabled = %d",mIsRaw16CBEnabled);
         break;
    default:
         break;
    }
    //
    switch  (cmd)
    {
    case CAMERA_CMD_PLAY_RECORDING_SOUND:
        return  OK;
    default:
        break;
    }
    //
    //  (1) try to see if Camera Adapter can handle this command.
    sp<ICamAdapter> pCamAdapter = mpCamAdapter;
    if  ( pCamAdapter != 0 && INVALID_OPERATION != (status = pCamAdapter->sendCommand(cmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        return  status;
    }
    //
    //  (2) try to see if Camera Client can handle this command.
    sp<ICamClient> pCamClient = mpCamClient;
    if  ( pCamClient != 0 && INVALID_OPERATION != (status = pCamClient->sendCommand(cmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        return  status;
    }
    //
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->sendCommand(cmd, arg1, arg2);
    }
    //
    switch  (cmd)
    {
    case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            ssize_t index = mTodoCmdMap.indexOfKey(cmd);
            if  (index < 0)
            {
                MY_LOGD("queue cmd(%#x),args(%d,%d)", cmd, arg1, arg2);
                mTodoCmdMap.add(cmd, CommandInfo(cmd, arg1, arg2));
            }
            else
            {
                MY_LOGW("queue the same cmd(%#x),args(%d,%d) again", cmd, arg1, arg2);
                mTodoCmdMap.editValueAt(index) = CommandInfo(cmd, arg1, arg2);
            }
        }
        status = OK;
        break;
    default:
        MY_LOGW("not handle cmd(%#x),args(%d,%d)", cmd, arg1, arg2);
        break;
    }
    //
    //return  OK;
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height)
{
    mpParamsMgr->getPreviewSize(&ri4Width, &ri4Height);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
initCameraAdapter()
{
    bool ret = false;
    //
    //  (1) Check to see if CamAdapter has existed or not.
    if  ( mpCamAdapter != 0 )
    {
        if  ( ICamAdapter::isValidInstance(mpCamAdapter) )
        {   // do nothing & just return true if the same app.
            MY_LOGD("valid camera adapter: %s", mpCamAdapter->getName());
            ret = true;
            goto lbExit;
        }
        else
        {   // cleanup the original one if different app.
            MY_LOGW("invalid camera adapter: %s", mpCamAdapter->getName());
            mpCamAdapter->uninit();
            mpCamAdapter.clear();
        }
    }
    //
    //  (2) Create & init a new CamAdapter.
    mpCamAdapter = ICamAdapter::createInstance(mDevName, mi4OpenId, mpParamsMgr);
    if  ( mpCamAdapter != 0 && mpCamAdapter->init() )
    {
        //  (.1) init.
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
        mpCamAdapter->enableMsgType(mpCamMsgCbInfo->mMsgEnabled);

        //  (.2) Invoke its setParameters
        if  ( OK != mpCamAdapter->setParameters() )
        {
            //  If fail, it should destroy instance before return.
            MY_LOGE("mpCamAdapter->setParameters() fail");
            goto lbExit;
        }

        //  (.3) Send to-do commands.
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            for (size_t i = 0; i < mTodoCmdMap.size(); i++)
            {
                CommandInfo const& rCmdInfo = mTodoCmdMap.valueAt(i);
                MY_LOGD("send queued cmd(%#x),args(%d,%d)", rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
                mpCamAdapter->sendCommand(rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
            }
            mTodoCmdMap.clear();
        }

        //  (.4) [DisplayClient] set Image Buffer Provider Client if needed.
        if  ( mpDisplayClient != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpDisplayClient->setImgBufProviderClient() fail");
            goto lbExit;
        }

        //  (.5) [CamClient] set Image Buffer Provider Client if needed.
        if  ( mpCamClient != 0 && ! mpCamClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpCamClient->setImgBufProviderClient() fail");
            goto lbExit;
        }
        //
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->setImgBufProviderClient(mpCamAdapter) ;
        }

    }
    else
    {
        MY_LOGE("mpCamAdapter(%p)->init() fail", mpCamAdapter.get());
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
initDisplayClient(preview_stream_ops* window)
{
    CAM_TRACE_CALL();
#if '1'!=MTKCAM_HAVE_DISPLAY_CLIENT
    #warning "Not Build Display Client"
    MY_LOGD("Not Build Display Client");
    return  OK;
#else
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+ window(%p)", window);
    //
    //
    //  [1] Check to see whether the passed window is NULL or not.
    if  ( ! window )
    {
        MY_LOGW("NULL window is passed into...");
        mbWindowReady = false;
        //
        if  ( mpDisplayClient != 0 )
        {
            MY_LOGW("destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
        status = OK;
        goto lbExit;
    }
    mbWindowReady = true;
    //
    //
    //  [2] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    //
    //  [3] Initialize Display Client.
    if  ( mpDisplayClient != 0 )
    {
        if  ( previewEnabled() )
        {
            MY_LOGW("Do nothing since Display Client(%p) is already created after startPreview()", mpDisplayClient.get());
//          This method must be called before startPreview(). The one exception is that
//          if the preview surface texture is not set (or set to null) before startPreview() is called,
//          then this method may be called once with a non-null parameter to set the preview surface.
            status = OK;
            goto lbExit;
        }
        else
        {
            MY_LOGW("New window is set after stopPreview or takePicture. Destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
    }
    //  [3.1] create a Display Client.
    mpDisplayClient = IDisplayClient::createInstance();
    if  ( mpDisplayClient == 0 )
    {
        MY_LOGE("Cannot create mpDisplayClient");
        status = NO_MEMORY;
        goto lbExit;
    }
    //  Display Rotation
    if(mpParamsMgr->getDisplayRotationSupported())
    {
        MY_LOGD("orientation = %d", mOrientation);
        mpDisplayClient->SetOrientationForDisplay(mOrientation);
    }
    //  [3.2] initialize the newly-created Display Client.
    if  ( ! mpDisplayClient->init() )
    {
        MY_LOGE("mpDisplayClient init() failed");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.3] set preview_stream_ops & related window info.
    if  ( ! mpDisplayClient->setWindow(window, previewSize.width, previewSize.height, queryDisplayBufCount()) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //  [3.4] set Image Buffer Provider Client if it exist.
    if  ( mpCamAdapter != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    //
    status = OK;
    //
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup...");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    MY_LOGD("- status(%d)", status);
    return  status;
#endif//MTKCAM_HAVE_DISPLAY_CLIENT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
enableDisplayClient()
{
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+");
    //
    //  [1] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    if(mpParamsMgr->getIfFirstPreviewFrameAsBlack())
    {
        mpDisplayClient->setFirstFrameBlack();
        mpParamsMgr->set(MtkCameraParameters::KEY_FIRST_PREVIEW_FRAME_BLACK, 0);
    }
    //  [2] Enable
    if  ( ! mpDisplayClient->enableDisplay(previewSize.width, previewSize.height, queryDisplayBufCount(), mpCamAdapter) )
    {
        MY_LOGE("mpDisplayClient(%p)->enableDisplay()", mpDisplayClient.get());
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    status = OK;
lbExit:
    MY_LOGD("- status(%d)",status);
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
disableDisplayClient()
{
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->disableDisplay();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
addCamClient(sp<ICamClient> const& client, const char* name)
{
    CAM_TRACE_CALL();
    sp<ICamClient> pClient = client;
    if (pClient == 0)
    {
        MY_LOGE("Null Client(%s):%p", name, client.get() );
        return BAD_VALUE;
    }

    Vector<sp<ICamClient> >::iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it) {
        if ((*it) == pClient) {
            MY_LOGE("client(%s) was already subscribed", pClient.get());
            return ALREADY_EXISTS;
        }
    }
    //
    MY_LOGD("addCamClient(%s):%p",  name, pClient.get());
    if (!pClient->init(mpParamsMgr))
    {
          MY_LOGW("pClient(%p)->init() fail", pClient.get());
          return UNKNOWN_ERROR;
    }

    vmpCamClient.push_back(pClient);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
removeCamClient(sp<ICamClient> const& client)
{
    sp<ICamClient> pClient = client;

    Vector<sp<ICamClient> >::iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it) {
        if ((*it) == pClient) {
            vmpCamClient.erase(it);
            return OK;
        }
    }
    return BAD_VALUE;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
onCam1Device_NotifyCb(
    int32_t const   msgType,
    intptr_t const  ext1)
{
    CAM_TRACE_NAME("Cam1DeviceBase::onCam1Device_NotifyCb");
    MY_LOGD("0x%08X",msgType);
    switch(msgType)
    {
        case MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE:
        {
            if( mpCamAdapter != 0 &&
                mpCamClient != 0)
            {
                if( mpCamAdapter->previewEnabled() == true &&
                    mpCamClient->previewEnabled() == false)
                {
                    MY_LOGD("clientStartPreview");
                    CAM_TRACE_NAME("clientStartPreview");
                    if (mpCamClient->startPreview() )
                    {
                        if(mIsPreviewEnabled)
                        {
                            MY_LOGD("enable CAMERA_MSG_PREVIEW_METADATA");
                            enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
                        }
                    }
                    else
                    {
                        MY_LOGE("mpCamClient->startPreview fail");
                    }
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
startPreviewImp()
{
    CAM_TRACE_CALL();
    MY_LOGD("+");
    //
    status_t status = OK;
    //
    if( !waitThreadInitDone() )
    {
        MY_LOGE("init in thread failed");
        return;
    }
    //
    status = mpCamAdapter->startPreview();
    if  ( OK != status )
    {
        MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
    }
    //
    MY_LOGD("- status(%d)", status);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
waitStartPreviewDone()
{
    if(mStartPreviewTThreadHandle != 0)
    {
        MY_LOGD("E");
        int s = pthread_join(mStartPreviewTThreadHandle, NULL);
        mStartPreviewTThreadHandle = 0;
        MY_LOGD("X");
        if( s != 0 )
        {
            MY_LOGE("startPreview pthread join error: %d", s);
            return false;
        }
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
void*
Cam1DeviceBase::
startPreviewThread(void* arg)
{
    CAM_LOGD("[Cam1DeviceBase::startPreviewThread]+");
    Cam1DeviceBase* pCam1DeviceBase = (Cam1DeviceBase*)arg;
    pCam1DeviceBase->startPreviewImp();
    CAM_LOGD("[Cam1DeviceBase::startPreviewThread]- done");
    pthread_exit(NULL);
    CAM_LOGD("[Cam1DeviceBase::startPreviewThread]-");
    return NULL;
}


