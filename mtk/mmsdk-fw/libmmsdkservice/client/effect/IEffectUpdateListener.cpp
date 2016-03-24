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

//#define LOG_NDEBUG 0
#define LOG_TAG "mmsdk/IEffectUpdateListener"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <utils/Log.h>

//
#include <mmsdk/IEffectUser.h>
#include <mmsdk/IEffectUpdateListener.h>


namespace android {
namespace NSMMSdk {
namespace NSEffect {


enum {
    ON_EFFECT_UPDATED = IBinder::FIRST_CALL_TRANSACTION,
};

class BpEffectUpdateListener: public BpInterface<IEffectUpdateListener>
{
public:
    BpEffectUpdateListener(const sp<IBinder>& impl)
        : BpInterface<IEffectUpdateListener>(impl)
    {
    }

    void onEffectUpdated(const sp<IEffectUser>& effect, void *info)
    {
        ALOGV("onEffectUpdated");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUpdateListener::getInterfaceDescriptor());
        //data.writeStrongBinder(effect->asBinder());
        data.writeStrongBinder(IInterface::asBinder(effect));
        data.writeInt32(reinterpret_cast<intptr_t>(info));
        remote()->transact(ON_EFFECT_UPDATED, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(EffectUpdateListener, "android.hardware.IEffectUpdateListener");

// ----------------------------------------------------------------------

status_t BnEffectUpdateListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ON_EFFECT_UPDATED: {
            ALOGV("ON_EFFECT_UPDATE");
            CHECK_INTERFACE(IEffectUpdateListener, data, reply);
            sp<IEffectUser> effectUser = interface_cast<IEffectUser>(data.readStrongBinder());
            int info;
            data.readInt32(&info);
            onEffectUpdated(effectUser, reinterpret_cast<void*>(info));
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSEffect
}; // namespace NSMMSdk
}; // namespace android

