/*
**
** Copyright (C) 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "mmsdk/MMSdkService"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>

#include <binder/AppOpsManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include <common.h>

using namespace android;
//
#if '1' == MTKCAM_HAVE_SDK_CLIENT
using namespace android;
#include "MyUtils.h"
using namespace MtkCamUtils;
#include <v1/IParamsManager.h>
#include <v1/ICamClient.h>
#include <v1/sdkClient/IGestureClient.h>
using namespace android::NSSdkClient;
#endif

// scaler client
#include <mmsdk/IImageTransformUser.h>
#include "./client/include/ImageTransformUser.h"

// Effect client
#include <mmsdk/IEffectUser.h>
#include "./client/include/EffectFaceBeautyUser.h"

// Gesture client
#include <mmsdk/IGestureUser.h>
#include "./client/include/GestureUser.h"

// Heartrate client
#include <mmsdk/IHeartrateUser.h>
#include "./client/include/HeartrateUser.h"

//
#include <device/Cam1Device.h>

// Feature Factory
#include <mmsdk/IEffectHal.h>


//
#include "MMSdkService.h"

using namespace android;
using namespace NSMMSdk;

namespace android {


// ----------------------------------------------------------------------------
// Logging support -- this is for debugging only
// Use "adb shell dumpsys media.mmsdk -v 1" to change it.
volatile int32_t gLogLevel = 1;

#include <cutils/log.h>
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define LOG1(...) ALOGD_IF(gLogLevel >= 1, __VA_ARGS__);
#define LOG2(...) ALOGD_IF(gLogLevel >= 2, __VA_ARGS__);

static void setLogLevel(int level) {
    android_atomic_write(level, &gLogLevel);
}

#if '1' == MTKCAM_HAVE_SDK_CLIENT
   static sp<android::NSSdkClient::IGestureClient> mpGestureClient = 0;
#endif

// ----------------------------------------------------------------------------

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::MMSdkService()
    :mpCam1Device(0)
    ,mLock()
    ,mpHRuser(NULL)
    ,mbHRConnected(false)
    , mpFeatureManager(0)
{
    mpFeatureManager = new NSCam::FeatureManager();

    ALOGI("MMSdkService started (pid=%d)", getpid());
}

/******************************************************************************
 *
 ******************************************************************************/
void MMSdkService::onFirstRef()
{
    LOG1("MMSdkService::onFirstRef");
    BnMMSdkService::onFirstRef();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::~MMSdkService()
{
    LOG1("MMSdkService::~MMSdkService()");
    if(mpHRuser != NULL)
        mpHRuser = NULL;

    LOG1("MMSdkService::~MMSdkService()");
    mpCam1Device = NULL;
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    if (mpGestureClient != 0)
    {
        mpGestureClient = 0;
    }
#endif
    mpFeatureManager = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectImageTransformUser (
    sp<NSImageTransform::IImageTransformUser>& client
)
{

    LOG1("MMSdkService::connectSImager E");
    using namespace android::NSMMSdk::NSImageTransform;
    sp <ImageTransformUser> user = new ImageTransformUser();

    client = user;
    LOG1("MMSdkService::connectSImager X");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectEffect (
    String16 const &clientName,
    sp<NSEffect::IEffectUser>& client
)
{
    String8 clientName8(clientName);
    LOG1("MMSdkService::connectEffect[%s] E", clientName8.string());

    using namespace android::NSMMSdk::NSEffect;
    if (clientName8 == "faceBeauty")
    {
        sp<NSEffect::EffectFaceBeautyUser> user = new EffectFaceBeautyUser();
        client = user;
    }
    else
    {
        LOG1("unknown client name");
    }

    LOG1("MMSdkService::connectEffect X");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectGesture (
    sp<NSGesture::IGestureUser>& client
)
{

    LOG1("MMSdkService::connectGesture E");
    using namespace android::NSMMSdk::NSGesture;
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    if (mpGestureClient == 0)
    {
        MY_LOGE("no native gesture client");
        return UNKNOWN_ERROR;
    }
    sp <GestureUser> user = new GestureUser(mpGestureClient);
#else
    sp <GestureUser> user = new GestureUser();
#endif

    client = user;
    LOG1("MMSdkService::connectGesture X");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectHeartrate (
    sp<NSHeartrate::IHeartrateUser>& client
)
{
    LOG1("MMSdkService::connectHeartrate E");
    Mutex::Autolock _l(mLock);

    using namespace android::NSMMSdk::NSHeartrate;

    if(mpHRuser == NULL) {
        mpHRuser = new HeartrateUser();
    }
#if '1' == MTKCAM_HR_MONITOR_SUPPORT
    if(mpCam1Device)
        mpHRuser->RegisterCamDev(mpCam1Device);
#else
    MY_LOGD("MMSdkService::connectHeartrate Not Support");
#endif

    client = mpHRuser;

    mbHRConnected = true;

    MY_LOGD("MMSdkService::connectHeartrate user : %p", mpHRuser.get());

    LOG1("MMSdkService::connectHeartrate X");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::disconnectHeartrate (
)
{
    LOG1("MMSdkService::disconnectHeartrate E");
    Mutex::Autolock _l(mLock);
    using namespace android::NSMMSdk::NSHeartrate;

    if(mbHRConnected) {
        mpHRuser->disconnect();
    }

    mbHRConnected = false;

    MY_LOGD("MMSdkService::disconnectHeartrate user : %p", mpHRuser.get());

    LOG1("MMSdkService::disconnectHeartrate X");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager)
{
    LOG1("MMSdkService::connectFeatureManager E");
    featureManager = mpFeatureManager;
    LOG1("MMSdkService::connectFeatureManager X");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::
registerCamera1Device(
    NSCam::ICamDevice *device
)
{
    LOG1("MMSdkService::registerCamera1Device E");
    Mutex::Autolock _l(mLock);
    mpCam1Device = reinterpret_cast<NSCam::Cam1Device*>(device);
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    using namespace NSSdkClient;
    mpGestureClient = android::NSSdkClient::IGestureClient::createInstance();
    mpCam1Device->addCamClient(mpGestureClient, "GestureClient");

#endif

#if '1' == MTKCAM_HR_MONITOR_SUPPORT
    if(mbHRConnected)
        mpHRuser->RegisterCamDev(mpCam1Device);
#endif

    MY_LOGD("MMSdkService::registerCamera1Device:%p", mpCam1Device);


    LOG1("MMSdkService::registerCamera1Device X");

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::
unRegisterCamera1Device(
    NSCam::ICamDevice *device
)
{
    LOG1("MMSdkService::unRegisterCamera1Device E");
    Mutex::Autolock _l(mLock);
    if (mpCam1Device == device)
    {
        MY_LOGD("MMSdkService::unregisterCamera1Device:%p", mpCam1Device);
#if '1' == MTKCAM_HR_MONITOR_SUPPORT
        if(mbHRConnected)
            mpHRuser->unRegisterCamDev(mpCam1Device);
#endif

        mpCam1Device = 0;
    }
    LOG1("MMSdkService::unRegisterCamera1Device X");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
#if 0
status_t MMSdkService::connectSample(
)
{
    LOG1("MMSdkService::connectSample E");

    //LOG1("MMSdkService::connectSImager E (pid %d \"%s\")", callingPid,
    //        clientName8.string());
    return OK;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::onTransact(
    uint32_t code,
    const Parcel& data,
    Parcel* reply,
    uint32_t flags)
{
    // Permission checks
    switch (code)
    {
        case BnMMSdkService::CONNECT_IMAGETRANSFORMUSER:
            break;
        case BnMMSdkService::CONNECT_EFFECT:
            break;
        case BnMMSdkService::CONNECT_GESTURE:
            break;
        case BnMMSdkService::CONNECT_HEARTRATE:
            break;
        case BnMMSdkService::DISCONNECT_HEARTRATE:
            break;
    }

    return BnMMSdkService::onTransact(code, data, reply, flags);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::dump(
    int fd,
    const Vector<String16>& args
)
{
    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
/*virtual*/void MMSdkService::binderDied
(
    const wp<IBinder> &who
)
{
    /**
      * While tempting to promote the wp<IBinder> into a sp,
      * it's actually not supported by the binder driver
      */
    ALOGD("java clients' binder died");
}


}; // namespace android
