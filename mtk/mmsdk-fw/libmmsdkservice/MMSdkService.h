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

#ifndef _MEDIATEK_SERVERS_MMSDK_MMSDKSERVICE_H_
#define _MEDIATEK_SERVERS_MMSDK_MMSDKSERVICE_H_

#include <utils/Vector.h>
#include <binder/BinderService.h>
#include <mmsdk/IMMSdkService.h>

#include "./client/include/HeartrateUser.h"


namespace NSCam {
    class FeatureManager;
};

namespace android {
extern volatile int32_t gLogLevel;
class Cam1Device;
class IGestureClient;

namespace NSMMSdk {

class IImageTransformUser;
class IEffectUser;
class IGestureUser;
class Cam1Device;
class IFeatureManager;

class MMSdkService :
    public BinderService<MMSdkService>,
    public BnMMSdkService,
    public IBinder::DeathRecipient
{
    friend class BinderService<MMSdkService>;
public:
    // Implementation of BinderService<T>
    static char const* getServiceName() { return "media.mmsdk"; }

                        MMSdkService();
    virtual             ~MMSdkService();

    /* connect to image transform user
     * @param[out] client: the image transform user
     */
    virtual status_t connectImageTransformUser(sp<NSImageTransform::IImageTransformUser>& client);
    virtual status_t connectEffect(String16 const &clientName, sp<NSEffect::IEffectUser>& client);
    virtual status_t connectGesture(sp<NSGesture::IGestureUser>& client) ;
	virtual status_t connectHeartrate(sp<NSHeartrate::IHeartrateUser>& client) ;
	virtual status_t disconnectHeartrate() ;
    virtual status_t connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager);
    //virtual status_t connectSample()  ;
    /* Function for cam1Device to regist its instance to MMSDK service
     * @param[out] deivce: the device of camera1
     */
    virtual status_t registerCamera1Device(NSCam::ICamDevice *deivce);
    //
    virtual status_t unRegisterCamera1Device(NSCam::ICamDevice *device);

    // Extra permissions checks
    virtual status_t    onTransact(uint32_t code, const Parcel& data,
                                   Parcel* reply, uint32_t flags);

    virtual status_t    dump(int fd, const Vector<String16>& args);

    /////////////////////////////////////////////////////////////////////


private:

    NSCam::Cam1Device *mpCam1Device;

	mutable Mutex               mLock;

	sp<NSHeartrate::HeartrateUser>  mpHRuser;

	bool			   mbHRConnected;

    // Delay-lofor the service , will be called by refs->mBase->onFirstRef()
    virtual void onFirstRef();

    // IBinder::DeathRecipient implementation
    virtual void        binderDied(const wp<IBinder> &who);

   sp<NSCam::FeatureManager> mpFeatureManager;

};

}; // namespace NSMMSdk

}; // namespace android

#endif
