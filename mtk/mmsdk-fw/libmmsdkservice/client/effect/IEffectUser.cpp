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
#define LOG_TAG "mmsdk/IEffectUser"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <utils/String8.h>
#include <mmsdk/IEffectUser.h>
#include <mmsdk/IEffectUpdateListener.h>

namespace android {
namespace NSMMSdk {
namespace NSEffect {


enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    GET_NAME,
    APPLY,
    SET_PARAMETER,
    SET_UPDATE_LISTENER,
    RELEASE,
};

class BpEffectUser: public BpInterface<IEffectUser>
{
public:
    BpEffectUser(const sp<IBinder>& impl)
        : BpInterface<IEffectUser>(impl)
    {
    }

    // disconnect from mmsdk service
    void disconnect()
    {
        ALOGV("disconnect");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
        reply.readExceptionCode();
    }

    // get the effect
    String8 getName()
    {
        ALOGV("getName");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        remote()->transact(GET_NAME, data, &reply);
        return reply.readString8();
    }

    // apply the effect on dest image
    bool apply(ImageInfo const &rSrcImage, const sp<IMemory>& srcData, ImageInfo const &rDestImage, const sp<IMemory>& destData)
    {
        ALOGV("apply");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        data.write(&rSrcImage, sizeof(ImageInfo));
        data.writeStrongBinder(IInterface::asBinder(srcData));
        data.write(&rDestImage, sizeof(ImageInfo));
        data.writeStrongBinder(IInterface::asBinder(destData));
        remote()->transact(APPLY, data, &reply);
        return reply.readInt32();
    }

    // set the parameter for the effect
    bool setParameter(String8 const &parameterKey, void *value)
    {
        ALOGV("setParameter");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        data.writeString8(parameterKey);
        data.write(value, sizeof(int32_t));
        remote()->transact(SET_PARAMETER, data, &reply);
        return reply.readInt32();
    }

    void setUpdateListener(sp<IEffectUpdateListener>const &listener)
    {
        ALOGV("setUpdateListener");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(listener));
        remote()->transact(SET_UPDATE_LISTENER, data, &reply);
        reply.readExceptionCode();
    }

    // release the effect
    bool release()
    {
        ALOGV("release");
        Parcel data, reply;
        data.writeInterfaceToken(IEffectUser::getInterfaceDescriptor());
        remote()->transact(RELEASE, data, &reply);
        return reply.readInt32();
    }


};

IMPLEMENT_META_INTERFACE(EffectUser, "android.hardware.IEffectUser");

// ----------------------------------------------------------------------

status_t BnEffectUser::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case DISCONNECT:
        {
            ALOGV("DISCONNECT");
            CHECK_INTERFACE(IEffectUser, data, reply);
            disconnect();
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case GET_NAME:
        {
            ALOGV("GET_NAME");
            CHECK_INTERFACE(IEffectUser, data, reply);
            reply->writeString8(getName());
            return NO_ERROR;
        }
        break;
        case APPLY:
        {
            ALOGV("APPLY");
            CHECK_INTERFACE(IEffectUser, data, reply);
            ImageInfo rSrcImg;
            sp<IMemory> rSrcData;
            ImageInfo rDstImg;
            sp<IMemory> rDestData;
            data.read(&rSrcImg, sizeof(ImageInfo));
            rSrcData = interface_cast<IMemory>(data.readStrongBinder());
            data.read(&rDstImg, sizeof(ImageInfo));
            rDestData = interface_cast<IMemory>(data.readStrongBinder());
            reply->writeInt32(apply(rSrcImg, rSrcData, rDstImg, rDestData));
            return NO_ERROR;
        }
        break;
        case SET_PARAMETER:
        {
            ALOGV("SET_PARAMETER");
            CHECK_INTERFACE(IEffectUser, data, reply);
            String8 paramKey(data.readString8());
            int value;
            data.read(&value, sizeof(int32_t));
            ALOGD("SET_PARAMETER value:%d", value);
            reply->writeInt32(setParameter(paramKey, reinterpret_cast<void*>(&value)));
            return NO_ERROR;
        }
        break;
        case SET_UPDATE_LISTENER:
        {
            ALOGV("SET_UPDATE_LISTENER");
            CHECK_INTERFACE(IEffectUser, data, reply);
            sp<IEffectUpdateListener> listener = interface_cast<IEffectUpdateListener>(data.readStrongBinder());
            setUpdateListener(listener);
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case RELEASE:
        {
            ALOGV("RELEASE");
            CHECK_INTERFACE(IEffectUser, data, reply);
            reply->writeInt32(release());
            return NO_ERROR;
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace NSEffect
}; // namespace NSMMSdk
}; // namespace android
