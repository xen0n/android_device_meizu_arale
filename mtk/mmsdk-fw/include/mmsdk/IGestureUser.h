/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _MEDIATEK_MMSDK_IGESTURE_USER_H_
#define _MEDIATEK_MMSDK_IGESTURE_USER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {
namespace NSMMSdk {
namespace NSGesture {

class IHandDetectionListener;

//
enum EHandPose
{
    ePOSE_OPENPALM      = 0x00,
    ePOSE_VICTORY       = 0x01,
};


//
class IGestureUser : public IInterface
{
public:
    DECLARE_META_INTERFACE(GestureUser);

    // disconnect from mmsdk service
    virtual void            disconnect() = 0;

    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    virtual bool            addHandDetectionListener(const sp<IHandDetectionListener> &listener,
                                                     EHandPose const pose) = 0;
    //
    virtual bool            removeHandDetectionListener(const sp<IHandDetectionListener> &listener,
                                                        EHandPose const pose) = 0;
};

// ----------------------------------------------------------------------------

class BnGestureUser: public BnInterface<IGestureUser>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android

#endif   // _MEDIATEK_MMSDK_IGESTURE_USER_H_
