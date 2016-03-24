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

#define LOG_TAG "MtkSdk/Gesture"
//
#include <cutils/properties.h>
//
#include "MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
using namespace NSCam::Utils;
//
#include "ImgBufManager.h"
//
#include "GestureClient.h"
//
using namespace NSSdkClient;
using namespace NSGestureClient;
//


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
sp<IGestureClient>
IGestureClient::
createInstance()
{
    static GestureClient singleton;
    return  &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
GestureClient::
GestureClient()
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(0)
    , mIsMsgEnabled(0)
    , mIsPrvStarted(0)
    , mIsGestureHalStarted(0)
    , ms8PrvTgtFmt("")
    , mi4PrvWidth(0)
    , mi4PrvHeight(0)
    //
    , mi4CallbackRefCount(0)
    , mi8CallbackTimeInMs(0)
    //
    , muImgBufIdx(0)
    , mpImgBufMgr(0)
    , mImgBufList()
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mMMSdkModule(NULL)
    , mGestureDevice(NULL)
    , mMaxNumOfGestureDetected(0)
    , mGestureCb(0)
    , mpGestureUser(0)
    //
    , mProfile_callback("performImageCallback")
    , mProfile_dequeProcessor("handleReturnBuffers")
    , mProfile_buffer_timestamp("handleReturnBuffers")
    //
    , miLogLevel(1)
    //
    , mDumpMtx()
    , mi4DumpImgBufCount(30)
    , mi4DumpImgBufIndex(0)
    , ms8DumpImgBufPath("/sdcard/client")
    //
{
    MY_LOGD("+ this(%p)", this);
    char cLogLevel[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("debug.camera.previewclient.loglevel", cLogLevel, "1");
    miLogLevel = ::atoi(cLogLevel);
    MY_LOGD("- debug.camera.previewclient.loglevel=%s", cLogLevel);
    mGestureRunning = false;
}


/******************************************************************************
 *
 ******************************************************************************/
GestureClient::
~GestureClient()
{
    MY_LOGD("+");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
init()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_AP_CLIENT, "APClient@ImgBufQue");
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("Fail to new ImgBufQueue");
        goto lbExit;
    }
    //
    //
    status = run();
    if  ( OK != status )
    {
        MY_LOGW("Fail to run thread, status[%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    ret = loadHALModule(&mMMSdkModule)
          && checkHALModuleVersion(mMMSdkModule);
    //
    if (!ret)
    {
        MY_LOGW("Fail to load HAL module or version check fail");
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    if(!ret)
    {
        uninit();
    }
    MY_LOGD("-");
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
init(sp<IParamsManager> pParamsMgr)
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_AP_CLIENT, "APClient@ImgBufQue");
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("Fail to new ImgBufQueue");
        goto lbExit;
    }
    //
    //
    status = run();
    if  ( OK != status )
    {
        MY_LOGW("Fail to run thread, status[%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    ret = loadHALModule(&mMMSdkModule)
          && checkHALModuleVersion(mMMSdkModule);
    //
    if (!ret)
    {
        MY_LOGW("Fail to load HAL module or version check fail");
        goto lbExit;
    }
    mpParamsMgr = pParamsMgr;
lbExit:
    if(!ret)
    {
        uninit();
    }
    MY_LOGD("-");
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
uninit()
{
    MY_LOGD("+");
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
    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
        mpImgBufPvdrClient = NULL;
    }
    //
    //
    if  ( mpImgBufQueue != 0 )
    {
        mpImgBufQueue->stopProcessor();
        mpImgBufQueue = NULL;
    }
    //
    //
    {
        MY_LOGD("getThreadId(%d), getStrongCount(%d), this(%p)", getThreadId(), getStrongCount(), this);
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
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
    MY_LOGD("-");
    return  true;
}

/******************************************************************************
 * Query whether a message, or a set of messages, is enabled.
 * Note that this is operates as an AND, if any of the messages
 * queried are off, this will return false.
 ******************************************************************************/
bool
GestureClient::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
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
    if  ( ! rpClient->onImgBufProviderCreated(mpImgBufQueue) )
    {
        goto lbExit;
    }
    mpImgBufPvdrClient = rpClient;
    //
    //
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
GestureClient::
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
bool
GestureClient::
startPreview()
{
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD("+ current mIsPrvStarted=%d", mIsPrvStarted);
        ::android_atomic_write(1, &mIsPrvStarted);
        //
        ms8PrvTgtFmt = mpParamsMgr->getPreviewFormat();
        mpParamsMgr->getPreviewSize(&mi4PrvWidth, &mi4PrvHeight);
        //
        MY_LOGD("+ preview: WxH=%dx%d, format(%s)", mi4PrvWidth, mi4PrvHeight, ms8PrvTgtFmt.string());
    }
    return  onStateChanged();
}


/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
stopPreview()
{
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD("+ current mIsPrvStarted=%d", mIsPrvStarted);
        ::android_atomic_write(0, &mIsPrvStarted);
    }
    onStateChanged();
}


/******************************************************************************
 *  Returns true if preview is enabled.
 ******************************************************************************/
bool
GestureClient::
previewEnabled()
{
    MY_LOGD("%d", ::android_atomic_release_load(&mIsPrvStarted));
    return 0 != ::android_atomic_release_load(&mIsPrvStarted);
}


/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
takePicture()
{
    stopPreview();
}

/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
enableMsgType(int32_t msgType)
{
    int32_t const oldMsgType = mpCamMsgCbInfo->mMsgEnabled;
    int32_t const newMsgType = mpCamMsgCbInfo->mMsgEnabled | msgType;
    ::android_atomic_write(newMsgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    updateMsg(oldMsgType, newMsgType);
}


/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
disableMsgType(int32_t msgType)
{
    int32_t const oldMsgType = mpCamMsgCbInfo->mMsgEnabled;
    int32_t const newMsgType = mpCamMsgCbInfo->mMsgEnabled & ~msgType;
    ::android_atomic_write(newMsgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    updateMsg(oldMsgType, newMsgType);
}


/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
updateMsg(int32_t const oldMsgType, int32_t const newMsgType)
{
    int32_t const i4TargetMsgType = CAMERA_MSG_PREVIEW_FRAME;
    bool const isToggleOn   = 0 != (i4TargetMsgType & (~oldMsgType & newMsgType));
    bool const isToggleOff  = 0 != (i4TargetMsgType & (oldMsgType & ~newMsgType));
    //
    MY_LOGD_IF(
        (2<=miLogLevel),
        "+ oldMsgType = 0x%08x, newMsgType = 0x%08x, (isToggleOn/isToggleOff)=(%d/%d)",
        oldMsgType, newMsgType, isToggleOn, isToggleOff
    );
    //
    //
    if  (isToggleOn)
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD_IF((1<=miLogLevel), "+ current mIsMsgEnabled=%d", mIsMsgEnabled);
        ::android_atomic_write(1, &mIsMsgEnabled);
        onStateChanged();
    }
    //
    if  (isToggleOff)
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD_IF((1<=miLogLevel), "+ current mIsMsgEnabled=%d", mIsMsgEnabled);
        ::android_atomic_write(0, &mIsMsgEnabled);
        onStateChanged();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
//  enable if both preview started && message enabled; otherwise disable.
bool
GestureClient::
isEnabledState() const
{
    /*
    return  0 != ::android_atomic_release_load(&mIsMsgEnabled)
        &&  0 != ::android_atomic_release_load(&mIsPrvStarted)
            ;
    */
    return 0 != ::android_atomic_release_load(&mIsPrvStarted)
        &&  0 != ::android_atomic_release_load(&mIsGestureHalStarted);
}


/******************************************************************************
 *
 ******************************************************************************/
//  enable if both preview started && message enabled; otherwise disable.
bool
GestureClient::
onStateChanged()
{
    bool ret = true;
    //
    if  ( isEnabledState() )
    {
//        if  ( initBuffers() )
        {
            postCommand(Command(Command::eID_WAKEUP));
        }
    }
    else
    {
        if  ( mpImgBufQueue != 0 )
        {
            mpImgBufQueue->pauseProcessor();
        }

//        uninitBuffers();
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::loadHALModule(mmsdk_module_t **module)
{
    //
    if (::hw_get_module(MMSDK_HARDWARE_MODULE_ID,
           (const hw_module_t **)module) < 0)
    {
        MY_LOGW("Could not load mmsdk HAL module");
        return false;
    }
    //
    MY_LOGD("Load \"%s\" mmsdk moudle", (*module)->common.name);
    //
    return true;

}


/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
checkHALModuleVersion(mmsdk_module_t const *module)
{
    if (module->common.module_api_version == MMSDK_MODULE_API_VERSION_1_0)
    {
        MY_LOGD("MMSDK API is 1.0");
    }
    else
    {
        MY_LOGD("MMSDK API is not 1.0");
        return false;
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
openGestureDevice(hw_module_t *module, gesture_device_t **device)
{
    //
    if (0 == module)
    {
        MY_LOGE("Null module object");
        return false;
    }
    //
    return (0 == module->methods->open(module, MMSDK_HARDWARE_GESTURE,
                                   (hw_device_t**)device));
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
closeGestureDevice(gesture_device_t  **device)
{
    //
    if (0 == *device)
    {
        MY_LOGE("Null device object");
        return false;
    }
    //
    int rc = (*device)->common.close(&((*device)->common));
    if (rc != OK)
    {
        MY_LOGE("Could not close image transform device:%d", rc);
        return false;
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
GestureClient::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    return  INVALID_OPERATION;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
start()
{
    MY_LOGD("+ current mIsGestureHalStarted=%d", mIsGestureHalStarted);
    //if preview is not enable return failse
    if (0 == ::android_atomic_release_load(&mIsPrvStarted))
    {
        MY_LOGE("preview is not enable");
        return false;
    }
    //
    if (0 != ::android_atomic_release_load(&mIsGestureHalStarted))
    {
        MY_LOGW("gesture hal already enable");
        return true;
    }
    //
    ::android_atomic_write(1, &mIsGestureHalStarted);

    //
    if (mMMSdkModule != 0)
    {
        //
        if (!openGestureDevice(&mMMSdkModule->common, &mGestureDevice))
        {
            MY_LOGE("open gesture device fail");
            return false;
        }
        //
        initBuffers();
        //
#if 0
        getGestureDevice()->ops->init(getGestureDevice(),
                                 GESTURE_SW_ONLY_MODE,
                                 mi4PrvWidth,
                                 mi4PrvHeight,
                                 &mMaxNumOfGestureDetected);
#else
        getGestureDevice()->ops->init(getGestureDevice(),
                                 GESTURE_SW_ONLY_MODE,
                                 mi4BufWidth,
                                 mi4BufHeight,
                                 &mMaxNumOfGestureDetected);
#endif
    }
    if(mMaxNumOfGestureDetected != 0) {
        mpDetectedResult = new hand_detection_result_t[mMaxNumOfGestureDetected];
    } else {
        mpDetectedResult = new hand_detection_result_t[15];
    }
    onStateChanged();
    return true; 

}

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureClient::
stop()
{
    MY_LOGD("+ current mIsGestureHalStarted=%d", mIsGestureHalStarted);
    if (0 !=  ::android_atomic_release_load(&mIsGestureHalStarted))
    {
        MY_LOGD("Stop gesture HAL");
        ::android_atomic_write(0, &mIsGestureHalStarted);
        //
        if (mMMSdkModule != 0)
        {
            //
            onStateChanged();
            //
            while(mGestureRunning) {
                usleep(10000); //sleep 10ms
            }
            uninitBuffers();        
            //
            getGestureDevice()->ops->uninit(getGestureDevice()); 
            //
            if(mpDetectedResult != NULL) {
                delete mpDetectedResult;
                mpDetectedResult = NULL;
            }
            if (!closeGestureDevice(&mGestureDevice))
            {
                return true;
            }
        }
   }
   else
   {
        MY_LOGW("geture hal already stop");
   }

   return onStateChanged();
}

/******************************************************************************
 *
 ******************************************************************************/
void
GestureClient::
setCallbacks(GestureCallback_t gesture_cb, MVOID* user)
{
    mGestureCb = gesture_cb;
    mpGestureUser = user;

}


/*******************************************************************************
*
********************************************************************************/
bool
GestureClient::
onGestureCallback(hand_detection_result_t const& result) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mGestureCb )
    {
        mGestureCb(result, mpGestureUser);
        ret = MTRUE;
    }
    else
    {
        MY_LOGW("Gesture Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}



