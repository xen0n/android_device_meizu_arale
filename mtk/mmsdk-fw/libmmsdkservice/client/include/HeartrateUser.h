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

#ifndef _MEDIATEK_MMSDK_HEARTRATE_USER_H_
#define _MEDIATEK_MMSDK_HEARTRATE_USER_H_

#include <mmsdk/IHeartrateUser.h>
#include <mmsdk/IHeartrateDetectionListener.h>

#include <device/Cam1Device.h>
#include <heartrate_device.h>



namespace android {

class Cam1Device;

namespace NSMMSdk {
namespace NSHeartrate {

class HeartrateUser :
    public BnHeartrateUser
{
public:

    HeartrateUser();
    //
    //virtual     ~HeartrateUser(){mpCamDev = NULL;};
    virtual     ~HeartrateUser();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // disconnect from mmsdk service

    virtual void            disconnect();

	virtual bool            start();

	virtual void            stop();

	virtual void            query(int *HasCamDev, int *IsHRStart);

	virtual void            setmode(int mode);
    //
    virtual bool            addHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener,
                                                     int const arg);
    //
    virtual bool            removeHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener,
                                                        int const arg);
	virtual void            RegisterCamDev(NSCam::Cam1Device *dev);

	virtual void            unRegisterCamDev(NSCam::Cam1Device *dev);

protected:

    static bool             fgHeartrateCb(HR_detection_result_t const &result, void* user);

    virtual void             onHeartrateCb(HR_detection_result_t const &result);

private:

    typedef KeyedVector<sp<IHeartrateDetectionListener>,  int > ListenerListMap_t;

    ListenerListMap_t mListenerListMap;

    NSCam::Cam1Device *mpCamDev;

    mutable Mutex               mLock;
	mutable Mutex               mCBLock;
    bool                       mClientEnabled;
	int                        mHRMode;
};

}; // namespace NSHeartrate
}; // namespace NSMMSdk
}; // namespace android

#endif   //_MEDIATEK_MMSDK_HEARTRATE_USER_H_
