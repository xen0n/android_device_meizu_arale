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

#define LOG_TAG "MtkCam/CamClient/OTClient"
//
#include "OTClient.h"
#include "camera/MtkCamera.h"
//
using namespace NSCamClient;
using namespace NSOTClient;
//
/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
sp<IOTClient>
IOTClient::
createInstance(sp<IParamsManager> pParamsMgr)
{
    return  new OTClient(pParamsMgr);
}


/******************************************************************************
 *
 ******************************************************************************/
OTClient::
OTClient(sp<IParamsManager> pParamsMgr)
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(pParamsMgr)
    , mIsOTStarted(0)
    //
    , mi4CallbackRefCount(0)
    , mi8CallbackTimeInMs(0)
    //
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mpDetectedObjs(NULL)
    , mIsDetected_OT(false)
    , mIsStartRecord(false)
    , mbuf_count(0)
{
    MY_LOGD("+ this(%p)", this);
    mbuf_count = mBufCnt;
}


/******************************************************************************
 *
 ******************************************************************************/
OTClient::
~OTClient()
{
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
init()
{
    MY_LOGD("+");
    bool ret = true;
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_FD, "OTBuf@ImgBufQue");
    //mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_OT, "OTBuf@ImgBufQue");
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("Fail to new ImgBufQueue");
        ret = false;
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
uninit()
{
    MY_LOGD("+");
/*
    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
    }
*/
    {
        MY_LOGD("getThreadId(%d), getStrongCount(%d), this(%p)", getThreadId(), getStrongCount(), this);
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        ::android_atomic_write(0, &mIsOTStarted);
        requestExit();
        status_t status = join();
        if  ( OK != status )
        {
            MY_LOGW("Not to wait thread(tid:%d), status[%s(%d)]", getThreadId(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
    }


    //
    //
    if  ( 0 != mi4CallbackRefCount )
    {
        int64_t const i8CurrentTimeInMs = NSCam::Utils::getTimeInMs();
        MY_LOGW(
            "Preview Callback: ref count(%d)!=0, the last callback before %lld ms, timestamp:(the last, current)=(%lld ms, %lld ms)",
            mi4CallbackRefCount, (i8CurrentTimeInMs-mi8CallbackTimeInMs), mi8CallbackTimeInMs, i8CurrentTimeInMs
        );
    }
    //
    //
    sp<IImgBufQueue> pImgBufQueue;
    {
        //Mutex::Autolock _l(mModuleMtx);
        pImgBufQueue = mpImgBufQueue;
    }
    //
    if  ( pImgBufQueue != 0 )
    {
        pImgBufQueue->stopProcessor();
        pImgBufQueue = NULL;
    }
    //
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
setImgBufProviderClient(sp<IImgBufProviderClient>const& rpClient)
{
    bool ret = false;

    //
    MY_LOGD("+ ImgBufProviderClient(%p)", rpClient.get());
    //
    //
    if  ( rpClient == 0 )
    {
        MY_LOGE("NULL ImgBufProviderClient");
        goto lbExit;
    }
    //
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("NULL ImgBufQueue");
        goto lbExit;
    }
    //
    mpImgBufPvdrClient = rpClient;
    //
    //
/*
    if  ( mpImgBufPvdrClient != 0 && ! mpImgBufPvdrClient->onImgBufProviderCreated(mpImgBufQueue) )
    {
        MY_LOGE("onImgBufProviderCreated failed");
        ret = false;
        goto lbExit;
    }
*/
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
OTClient::
setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    Mutex::Autolock _l(mModuleMtx);
    //
    //  value copy
    *mpCamMsgCbInfo = *rpCamMsgCbInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
void
OTClient::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
 *
 ******************************************************************************/
void
OTClient::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
isMsgEnabled()
{
    if(!mIsStartRecord)
        return CAMERA_MSG_PREVIEW_METADATA == (CAMERA_MSG_PREVIEW_METADATA & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
    else
        return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
onStateChanged()
{
    bool ret = true;
    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //
    if  ( isEnabledState() )
    {
        status_t status = run();
        if ( INVALID_OPERATION == status)
        {
            MY_LOGW("run(): OT is running");
        }
        else if ( OK != status )
        {
            MY_LOGE("Fail to run thread, status[%s(%d)]", ::strerror(-status), -status);
            ret = false;
            goto lbExit;
        }
        //
        postCommand(Command::eID_WAKEUP);
    }
    else
    {
        if  ( mpImgBufQueue != 0 )
        {
            MY_LOGD("PauseProcessor En");
            mpImgBufQueue->pauseProcessor();
        }
    }
    //
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
OTClient::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    bool ret = false;

    MY_LOGD("cmd(0x%x) +", cmd);

    switch  (cmd)
    {
    //
    case CAMERA_CMD_START_OT:
        ret = startOT(arg1, arg2);
        break;

    case CAMERA_CMD_STOP_OT:
        ret = stopOT();
        break;

    default:
        break;
    }

    MY_LOGD("-");

    return ret? OK : INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
startOT(int32_t arg1, int32_t arg2)
{
    MY_LOGD("+");
    bool ret = true;

    OT_StartX = arg1;
    OT_StartY = arg2;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //

    if  ( mpImgBufPvdrClient != 0 && ! mpImgBufPvdrClient->onImgBufProviderCreated(mpImgBufQueue) )
    {
        MY_LOGE("onImgBufProviderCreated failed");
        ret = false;
        goto lbExit;
    }

    //
    if ( !isEnabledState() )
    {
        ::android_atomic_write(1, &mIsOTStarted);
        ret = onStateChanged();
    }
    else
    {
        MY_LOGW("OT is running");
        ret = false;
    }
    //
    MY_LOGD("-");
    //
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
stopOT()
{
    MY_LOGD("+");
    bool ret = true;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    //

    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
    }

    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //
    //
    if ( isEnabledState() )
    {
        ::android_atomic_write(0, &mIsOTStarted);
        ret = onStateChanged();
    }
    else
    {
        MY_LOGW("OT was not running");
        ret = false;
    }
    //
    MY_LOGD("-");
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
stopPreview()
{
    return stopOT();
}

/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
startRecording()
{
    MY_LOGD("+");
    bool ret = true;
    mIsStartRecord = true;
    MY_LOGD("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
stopRecording()
{
    MY_LOGD("+");
    bool ret = true;
    mIsStartRecord = false;
    MY_LOGD("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::takePicture()
{
    if(!mIsStartRecord)
      return stopOT();
    else
      return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
isEnabledState()
{
    return  0 != ::android_atomic_release_load(&mIsOTStarted);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
performCallback(bool isDetected_OT)
{
    bool ret = true;


    // (1) OT callback:
    //     (-) always do whenever face is detected
    //     (-) do when object is not detected, but last time was detected
    //     (-) otherwise, do no-op
    bool performOTcb = isDetected_OT ? true : mIsDetected_OT ? true : false;

    if (performOTcb && isMsgEnabled())
    {

        camera_memory_t* pmem = mpCamMsgCbInfo->mRequestMemory(-1,  2*sizeof(int)+mpDetectedObjs->number_of_faces*sizeof(camera_face_t), 1, NULL);
        uint32_t*const retFaces = reinterpret_cast<uint32_t*>(pmem->data);

        retFaces[0] = MTK_CAMERA_MSG_EXT_DATA_OT;
        retFaces[1] = mpDetectedObjs->number_of_faces;
        memcpy(&retFaces[2], mpDetectedObjs->faces, mpDetectedObjs->number_of_faces*sizeof(camera_face_t));


        mpCamMsgCbInfo->mDataCb(
             MTK_CAMERA_MSG_EXT_DATA,
             pmem,
             0,
             NULL,
             mpCamMsgCbInfo->mCbCookie
        );

        pmem->release(pmem);
        mIsDetected_OT = isDetected_OT;
    }
    else
    {
        ret = false;
        MY_LOGW_IF(ENABLE_LOG_PER_FRAME, "No OT CB: isDetected_OT(%d), mIsDetected_OT(%d), isMsgEnabled(%d)", isDetected_OT, mIsDetected_OT, isMsgEnabled());
    }

    return ret;
}

