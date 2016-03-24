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
#define LOG_TAG "mmsdk/IGestureUser"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <mmsdk/IGestureUser.h>
#include <mmsdk/IHandDetectionListener.h>

namespace android {
namespace NSMMSdk {
namespace NSGesture {


enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    ADD_HAND_DETECTION_LISTENER,
    REMOVE_HAND_DETECTION_LISTENER,
};

class BpGestureUser: public BpInterface<IGestureUser>
{
public:
    BpGestureUser(const sp<IBinder>& impl)
        : BpInterface<IGestureUser>(impl)
    {
    }

    // disconnect from mmsdk service
    void disconnect()
    {
        ALOGV("disconnect");
        Parcel data, reply;
        data.writeInterfaceToken(IGestureUser::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
        reply.readExceptionCode();
    }

    // add hand detection listener
    bool addHandDetectionListener(const sp<IHandDetectionListener> &listener, EHandPose const pose)
    {
        ALOGV("addHandDetectionListener");
        Parcel data, reply;
        data.writeInterfaceToken(IGestureUser::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        data.writeInt32(static_cast<int>(pose));
        remote()->transact(ADD_HAND_DETECTION_LISTENER, data, &reply);
        return reply.readInt32();
    }

    // remove hand detection listener
    bool removeHandDetectionListener(const sp<IHandDetectionListener> &listener, EHandPose const pose)
    {
        ALOGV("removeHandDetectionListener");
        Parcel data, reply;
        data.writeInterfaceToken(IGestureUser::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        data.writeInt32(static_cast<int>(pose));
        remote()->transact(REMOVE_HAND_DETECTION_LISTENER, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(GestureUser, "android.hardware.IGestureUser");

// ----------------------------------------------------------------------

status_t BnGestureUser::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case DISCONNECT:
        {
            ALOGV("DISCONNECT");
            CHECK_INTERFACE(IGestureUser, data, reply);
            disconnect();
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case ADD_HAND_DETECTION_LISTENER:
        {
            ALOGV("ADD_HAND_DETECTION_LISTENER");
            CHECK_INTERFACE(IGestureUser, data, reply);
            sp<IHandDetectionListener> listener = interface_cast<IHandDetectionListener>(data.readStrongBinder());
            EHandPose ePose = static_cast<EHandPose>(data.readInt32());
            reply->writeInt32(addHandDetectionListener(listener, ePose));
            return NO_ERROR;
        }
        break;
        case REMOVE_HAND_DETECTION_LISTENER:
        {
            ALOGV("REMOVE_HAND_DETECTION_LISTENER");
            CHECK_INTERFACE(IGestureUser, data, reply);
            sp<IHandDetectionListener> listener = interface_cast<IHandDetectionListener>(data.readStrongBinder());
            EHandPose ePose = static_cast<EHandPose>(data.readInt32());
            reply->writeInt32(removeHandDetectionListener(listener, ePose));
            return NO_ERROR;
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSGesture
}; // namespace NSMMSdk
}; // namespace android
