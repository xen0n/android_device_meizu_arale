/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef _MEDIATEK_MMSDK_IEFFECT_UPDATE_LISTENER_H_
#define _MEDIATEK_MMSDK_IEFFECT_UPDATE_LISTENER_H_

#include <binder/IInterface.h>
#include <stdint.h>
#include <utils/RefBase.h>

namespace android {

class Parcel;

namespace NSMMSdk {
namespace NSEffect {

class IEffectUser;



class IEffectUpdateListener: public IInterface
{
public:
    DECLARE_META_INTERFACE(EffectUpdateListener);

    virtual void onEffectUpdated(const sp<IEffectUser>& effect, void *info) = 0;
};

// ----------------------------------------------------------------------------

class BnEffectUpdateListener: public BnInterface<IEffectUpdateListener>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSEffect
}; // namespace NSMMSdk
}; // namespace android

#endif
