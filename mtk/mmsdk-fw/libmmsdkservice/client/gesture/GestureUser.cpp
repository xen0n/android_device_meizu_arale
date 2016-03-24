/*
**
** Copyright 2008, The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "mmsdk/GestureUser"

#define MTK_LOG_ENABLE 1
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IInterface.h>

#include <utils/log.h>
#include <common.h>

using namespace android;
//
#if '1' == MTKCAM_HAVE_SDK_CLIENT
#include "MyUtils.h"
using namespace MtkCamUtils;
#include <v1/IParamsManager.h>
#include <v1/ICamClient.h>
#include <v1/sdkClient/IGestureClient.h>
using namespace android::NSSdkClient;
#endif


//
#include <mmsdk/IGestureUser.h>
#include <mmsdk/IHandDetectionListener.h>
#include "../include/GestureUser.h"



static int getCallingPid() {
    return android::IPCThreadState::self()->getCallingPid();
}

#include <cutils/log.h>
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define FUNCTION_LOG_START    MY_LOGD("[%s]+, (pid=%d, tid=%d)", __FUNCTION__, getCallingPid(), gettid());
#define FUNCTION_LOG_END      MY_LOGD("[%s]-", __FUNCTION__);

namespace android {
namespace NSMMSdk {
namespace NSGesture {

#if '1' == MTKCAM_HAVE_SDK_CLIENT
GestureUser::
GestureUser(sp<IGestureClient> const &client)
    :mListenerListMap()
    ,mpClient(client)
    ,mLock()
    ,mClientEnabled(false)
{

}
#endif


/******************************************************************************
 *
 ******************************************************************************/
void
GestureUser::
disconnect(
)
{
    FUNCTION_LOG_START
    MY_LOGD("disconnect");
    Mutex::Autolock _l(mLock);
    mListenerListMap.clear();

    FUNCTION_LOG_END
    return ;
}

/******************************************************************************
 *
 ******************************************************************************/
 #if '1' == MTKCAM_HAVE_SDK_CLIENT
bool
GestureUser::
fgGestureCb(hand_detection_result_t const &result, void* user)
{
    GestureUser *pGestureUser = reinterpret_cast <GestureUser *>(user);
    if (NULL != pGestureUser)
    {
        pGestureUser->onGestureCb(result);
    }

    return MTRUE;
}


void
GestureUser::
onGestureCb(hand_detection_result_t const &result)
{
     FUNCTION_LOG_START
    for (int i = 0; i < mListenerListMap.size(); i++)
    {
          sp<IHandDetectionListener> pListener = (mListenerListMap.keyAt(i));
          EHandPose ePose = mListenerListMap.valueAt(i);

          if (ePose == static_cast<EHandPose>(result.pose))
          {
              HandDetectionEvent event;
              event.pose =  result.pose;
              event.id = result.id;
              event.confidence = result.confidence;
              event.boundBox = Rect(result.rect.leftTop().x,
                                           result.rect.leftTop().y,
                                           result.rect.rightBottom().x,
                                           result.rect.rightBottom().y);

              MY_LOGD("callback to listener");
              pListener->onHandDetected(event);
          }
     }
     FUNCTION_LOG_END
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
bool
GestureUser::
addHandDetectionListener(
    const sp<IHandDetectionListener> &listener,
    EHandPose ePose
)
{
    FUNCTION_LOG_START
    MY_LOGD("listener:%p, ePose:%d", listener.get(), ePose);

    Mutex::Autolock _l(mLock);
    for (int i = 0; i < mListenerListMap.size(); i++)
    {
          sp<IHandDetectionListener> pListener = (mListenerListMap.keyAt(i));
         if (IInterface::asBinder(pListener) == IInterface::asBinder(listener))
         {
             MY_LOGE("listener already added");
             return false;
         }
    }
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    //
    if ((mpClient != 0) && (mClientEnabled == false))
    {
        MY_LOGD("enable gesture client");
        mpClient->start();
        mClientEnabled = true;
        mpClient->setCallbacks(fgGestureCb, this);
    }
#endif
    mListenerListMap.add(listener, ePose);

    FUNCTION_LOG_END
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
GestureUser::
removeHandDetectionListener(
    const sp<IHandDetectionListener> &listener,
    EHandPose ePose
)
{
    FUNCTION_LOG_START
    MY_LOGD("listener:%p, ePose:%d", listener.get(), ePose);
    Mutex::Autolock _l(mLock);
    //
    int listenerFound = 0;
    for (int i = 0; i < mListenerListMap.size(); i++)
    {
         sp<IHandDetectionListener> pListener = (mListenerListMap.keyAt(i));
         if (IInterface::asBinder(pListener) == IInterface::asBinder(listener))
         {
             MY_LOGD("listen found");
             mListenerListMap.removeItemsAt(i);
             listenerFound = 1;
             break;
         }
    }
    if (listenerFound == 0)
    {
        MY_LOGE("no listener add before");
        return false;
    }
    MY_LOGD("num of listener:%d, clientEnable:%d", mListenerListMap.size(), mClientEnabled);
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    if (0 == mListenerListMap.size() && mClientEnabled)
    {
        MY_LOGD("last listener, disable gesture client");
        mpClient->stop();
        mClientEnabled = false;
    }
#endif
    FUNCTION_LOG_END
    return true;
}



// ----------------------------------------------------------------------------
}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android
