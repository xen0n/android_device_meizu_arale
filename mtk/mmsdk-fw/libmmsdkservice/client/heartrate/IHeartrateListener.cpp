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
#define LOG_TAG "mmsdk/IHeartRateDetectionListener"
#include <binder/Parcel.h>
#include <utils/Log.h>

//
#include <mmsdk/IHeartrateDetectionListener.h>


namespace android {
namespace NSMMSdk {
namespace NSHeartrate {


enum {
    ON_HEARTRATE_DETECTED = IBinder::FIRST_CALL_TRANSACTION,
};

class BpHeartrateDetectionListener: public BpInterface<IHeartrateDetectionListener>
{
public:
    BpHeartrateDetectionListener(const sp<IBinder>& impl)
        : BpInterface<IHeartrateDetectionListener>(impl)
    {
    }

    void onHeartrateDetected(HeartrateDetectionEvent const &event)
    {
        ALOGV("onHeartrateDetected");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateDetectionListener::getInterfaceDescriptor());
        data.write(&event, sizeof(HeartrateDetectionEvent));
        remote()->transact(ON_HEARTRATE_DETECTED, data, &reply, IBinder::FLAG_ONEWAY);
    }
};

IMPLEMENT_META_INTERFACE(HeartrateDetectionListener, "android.hardware.IHeartrateDetectionListener");

// ----------------------------------------------------------------------

status_t BnHeartrateDetectionListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ON_HEARTRATE_DETECTED: {
            ALOGV("ON_HEARTRATE_DETECTED");
            CHECK_INTERFACE(IHeartrateDetectionListener, data, reply);
            HeartrateDetectionEvent event;
            data.read(&event, sizeof(HeartrateDetectionEvent));
            onHeartrateDetected(event);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSHeartrate
}; // namespace NSMMSdk
}; // namespace android

