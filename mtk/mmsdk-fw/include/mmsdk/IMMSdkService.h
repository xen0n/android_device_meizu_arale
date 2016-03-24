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

#ifndef _MEDIATEK_MMSDK_IMMSDKSERVICE_H_
#define _MEDIATEK_MMSDK_IMMSDKSERVICE_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <mmsdk/IImageTransformUser.h>
#include <mmsdk/IEffectUser.h>
#include <mmsdk/IGestureUser.h>
#include <mmsdk/IHeartrateUser.h>
#include <device/Cam1Device.h>

namespace NSCam {
    class IFeatureManager;
};

class Cam1Device;
namespace android {
namespace NSMMSdk {

class IImageTransformUser;
class IEffectUser;
class IGestureUser;

class IMMSdkService : public IInterface
{
public:
    enum {
        CONNECT_IMAGETRANSFORMUSER = IBinder::FIRST_CALL_TRANSACTION,
        CONNECT_EFFECT,
        CONNECT_GESTURE,
        CONNECT_HEARTRATE,
        DISCONNECT_HEARTRATE,
        GET_MMSDK_FEAUTRE_MANAGER,
        REGISTER_CAMERA_1_DEVICE,
        UNREGISTER_CAMERA_1_DEVICE,
    };

public:
    DECLARE_META_INTERFACE(MMSdkService);

    /// Interface
    virtual status_t connectImageTransformUser(sp<NSImageTransform::IImageTransformUser>& client) = 0;
    virtual status_t connectEffect(String16 const &clientName, sp<NSEffect::IEffectUser>& client)  = 0;
    virtual status_t connectGesture(sp<NSGesture::IGestureUser>& client) = 0;
	virtual status_t connectHeartrate(sp<NSHeartrate::IHeartrateUser>& client) = 0;
    virtual status_t disconnectHeartrate() = 0;
    virtual status_t connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager) = 0;
    virtual status_t registerCamera1Device(NSCam::ICamDevice *device) = 0;
    virtual status_t unRegisterCamera1Device(NSCam::ICamDevice *device) = 0;

};

// ----------------------------------------------------------------------------

class BnMMSdkService: public BnInterface<IMMSdkService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSMMSdk
}; // namespace android

#endif  //_MEDIATEK_MMSDK_IMMSDKSERVICE_H_
