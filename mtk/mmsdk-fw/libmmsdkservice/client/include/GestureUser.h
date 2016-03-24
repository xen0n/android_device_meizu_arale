/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MEDIATEK_MMSDK_GESTURE_USER_H_
#define _MEDIATEK_MMSDK_GESTURE_USER_H_

#include <mmsdk/IGestureUser.h>
#include <mmsdk/IHandDetectionListener.h>

namespace android {
namespace NSMMSdk {
namespace NSGesture {

class GestureUser :
    public BnGestureUser
{
public:
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    GestureUser(sp<IGestureClient> const &client);
    //
    virtual     ~GestureUser(){mpClient = NULL;};
#else
    GestureUser() {};
    virtual     ~GestureUser(){};
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // disconnect from mmsdk service
    virtual void            disconnect();
    //
    virtual bool            addHandDetectionListener(const sp<IHandDetectionListener> &listener,
                                                     EHandPose const pose);
    //
    virtual bool            removeHandDetectionListener(const sp<IHandDetectionListener> &listener,
                                                        EHandPose const pose);

protected:
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    static bool             fgGestureCb(hand_detection_result_t const &result, void* user);

    virtual void             onGestureCb(hand_detection_result_t const &result);
#endif
private:

    typedef KeyedVector<sp<IHandDetectionListener>,  EHandPose > ListenerListMap_t;

    ListenerListMap_t mListenerListMap;
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    sp<IGestureClient> mpClient;
#endif
    mutable Mutex               mLock;
    bool                       mClientEnabled;
};

}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android

#endif   //_MEDIATEK_MMSDK_GESTURE_USER_H_
