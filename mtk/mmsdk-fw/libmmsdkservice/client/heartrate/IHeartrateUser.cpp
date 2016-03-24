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
#define LOG_TAG "mmsdk/IHeartrateUser"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <mmsdk/IHeartrateUser.h>
#include <mmsdk/IHeartrateDetectionListener.h>

namespace android {
namespace NSMMSdk {
namespace NSHeartrate {


enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    HEARTRATE_START,
    HEARTRATE_STOP,
    HEARTRATE_QUERY,
    HEARTRATE_SETMODE,
    ADD_HEARTRATE_DETECTION_LISTENER,
    REMOVE_HEARTRATE_DETECTION_LISTENER,
};

class BpHeartrateUser: public BpInterface<IHeartrateUser>
{
public:
    BpHeartrateUser(const sp<IBinder>& impl)
        : BpInterface<IHeartrateUser>(impl)
    {
    }

    // disconnect from mmsdk service
    void disconnect()
    {
        ALOGV("disconnect");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
        reply.readExceptionCode();
    }

    // start from mmsdk service
    bool start()
    {
        ALOGV("start");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        remote()->transact(HEARTRATE_START, data, &reply);
        return reply.readInt32();
    }

    // stop from mmsdk service
    void stop()
    {
        ALOGV("stop");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        remote()->transact(HEARTRATE_STOP, data, &reply);
        reply.readExceptionCode();
    }

    void query(int *HasCamDev, int *IsHRStart)
    {
        ALOGV("query");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        remote()->transact(HEARTRATE_QUERY, data, &reply);
        *HasCamDev = reply.readInt32();
        *IsHRStart = reply.readInt32();
        reply.readExceptionCode();
    }

    void setmode(int mode)
    {
        ALOGV("semode");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        data.writeInt32(mode);
        remote()->transact(HEARTRATE_SETMODE, data, &reply);
        reply.readExceptionCode();
    }

    // add heartrate detection listener
    bool addHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener, int const arg)
    {
        ALOGV("addHeartrateDetectionListener");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        data.writeInt32(static_cast<int>(arg));
        remote()->transact(ADD_HEARTRATE_DETECTION_LISTENER, data, &reply);
        return reply.readInt32();
    }

    // remove heartrate detection listener
    bool removeHeartrateDetectionListener(const sp<IHeartrateDetectionListener> &listener, int const arg)
    {
        ALOGV("removeHeartrateDetectionListener");
        Parcel data, reply;
        data.writeInterfaceToken(IHeartrateUser::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        data.writeInt32(static_cast<int>(arg));
        remote()->transact(REMOVE_HEARTRATE_DETECTION_LISTENER, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(HeartrateUser, "android.hardware.IHeartrateUser");

// ----------------------------------------------------------------------

status_t BnHeartrateUser::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case DISCONNECT:
        {
            ALOGV("DISCONNECT");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            disconnect();
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case HEARTRATE_START:
        {
            ALOGV("START");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            reply->writeInt32(start());
            return NO_ERROR;
        }
        break;
        case HEARTRATE_STOP:
        {
            ALOGV("STOP");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            stop();
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case HEARTRATE_QUERY:
        {
            int HasCamDev = 0, IsHRStart = 0;
            ALOGV("QUERY");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            query(&HasCamDev, &IsHRStart);
            reply->writeInt32(HasCamDev);
            reply->writeInt32(IsHRStart);
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case HEARTRATE_SETMODE:
        {
            ALOGV("SET MODE");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            int mode= static_cast<int>(data.readInt32());
            setmode(mode);
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case ADD_HEARTRATE_DETECTION_LISTENER:
        {
            ALOGV("ADD_HEARTRATE_DETECTION_LISTENER");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            sp<IHeartrateDetectionListener> listener = interface_cast<IHeartrateDetectionListener>(data.readStrongBinder());
            int arg = static_cast<int>(data.readInt32());
            reply->writeInt32(addHeartrateDetectionListener(listener, arg));
            return NO_ERROR;
        }
        break;
        case REMOVE_HEARTRATE_DETECTION_LISTENER:
        {
            ALOGV("REMOVE_HEARTRATE_DETECTION_LISTENER");
            CHECK_INTERFACE(IHeartrateUser, data, reply);
            sp<IHeartrateDetectionListener> listener = interface_cast<IHeartrateDetectionListener>(data.readStrongBinder());
            int arg = static_cast<int>(data.readInt32());
            reply->writeInt32(removeHeartrateDetectionListener(listener, arg));
            return NO_ERROR;
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSHeartrate
}; // namespace NSMMSdk
}; // namespace android
