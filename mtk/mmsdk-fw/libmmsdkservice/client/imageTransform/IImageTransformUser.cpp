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
#define LOG_TAG "mmsdk/IImageTransform"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <mmsdk/IImageTransformUser.h>

namespace android {
using namespace android::NSMMSdk;
using namespace android::NSMMSdk::NSImageTransform;


enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    APPLY_TRANSFORM,
};

class BpImageTransformUser: public BpInterface<IImageTransformUser>
{
public:
    BpImageTransformUser(const sp<IBinder>& impl)
        : BpInterface<IImageTransformUser>(impl)
    {
    }

    // disconnect from mmsdk service
    void disconnect()
    {
        ALOGV("disconnect");
        Parcel data, reply;
        data.writeInterfaceToken(IImageTransformUser::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
        reply.readExceptionCode();
    }


    // Transfer the source image into a target image by setting properties
    bool applyTransform(ImageInfo const &rSrcImage,
                        const sp<IMemory>& srcData,
                        ImageInfo const &rDestImage,
                        const sp<IMemory>& destData,
                        TrasformOptions const &rOptions
                        )
    {
        ALOGV("applyTransform");
        Parcel data, reply;
        data.writeInterfaceToken(IImageTransformUser::getInterfaceDescriptor());
        data.write(&rSrcImage, sizeof(ImageInfo));
        data.writeStrongBinder(IInterface::asBinder(srcData));
        data.write(&rDestImage, sizeof(ImageInfo));
        data.writeStrongBinder(IInterface::asBinder(destData));
        data.write(&rOptions, sizeof(TrasformOptions));
        remote()->transact(APPLY_TRANSFORM, data, &reply);
        return reply.readInt32();
    }

};

IMPLEMENT_META_INTERFACE(ImageTransformUser, "android.hardware.IImageTransformUser");

// ----------------------------------------------------------------------

status_t BnImageTransformUser::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case DISCONNECT:
        {
            ALOGV("DISCONNECT");
            CHECK_INTERFACE(IImageTransformUser, data, reply);
            disconnect();
            reply->writeNoException();
            return NO_ERROR;
        }
        break;
        case APPLY_TRANSFORM:
        {
            ALOGV("APPLY_TRANSFORM");
            CHECK_INTERFACE(IImageTransformUser, data, reply);
            ImageInfo rSrc;
            ImageInfo rDst;
            sp<IMemory> srcData;
            sp<IMemory> dstData;
            TrasformOptions rOptions;
            data.read(&rSrc, sizeof(ImageInfo));
            srcData = interface_cast<IMemory>(data.readStrongBinder());
            data.read(&rDst, sizeof(ImageInfo));
            dstData = interface_cast<IMemory>(data.readStrongBinder());
            data.read(&rOptions, sizeof(TrasformOptions));
            reply->writeInt32(applyTransform(rSrc, srcData,rDst,dstData, rOptions ));
            return NO_ERROR;
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
