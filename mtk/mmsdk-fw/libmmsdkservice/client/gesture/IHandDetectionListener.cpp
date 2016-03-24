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
#define LOG_TAG "mmsdk/IHandDetectionListener"
#include <binder/Parcel.h>
#include <utils/Log.h>

//
#include <mmsdk/IHandDetectionListener.h>


namespace android {
namespace NSMMSdk {
namespace NSGesture {


enum {
    ON_HAND_DETECTED = IBinder::FIRST_CALL_TRANSACTION,
};

class BpHandDetectionListener: public BpInterface<IHandDetectionListener>
{
public:
    BpHandDetectionListener(const sp<IBinder>& impl)
        : BpInterface<IHandDetectionListener>(impl)
    {
    }

    void onHandDetected(HandDetectionEvent const &event)
    {
        ALOGV("onHandDetected");
        Parcel data, reply;
        data.writeInterfaceToken(IHandDetectionListener::getInterfaceDescriptor());
        data.write(&event, sizeof(HandDetectionEvent));
        remote()->transact(ON_HAND_DETECTED, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(HandDetectionListener, "android.hardware.IHandDetectionListener");

// ----------------------------------------------------------------------

status_t BnHandDetectionListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ON_HAND_DETECTED: {
            ALOGV("ON_HAND_DETECTED");
            CHECK_INTERFACE(IHandDetectionListener, data, reply);
            HandDetectionEvent event;
            data.read(&event, sizeof(HandDetectionEvent));
            onHandDetected(event);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android

