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

#ifndef _MEDIATEK_MMSDK_IEFFECT_USER_H_
#define _MEDIATEK_MMSDK_IEFFECT_USER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <system/graphics.h>

#include "ImageBufferInfo.h"



namespace android {
namespace NSMMSdk {
namespace NSEffect {

class IEffectUpdateListener;

/*
 * [FIXME], should sync to /hardware/include/libmmsdkhal/effect_device.h
 */
#define FACE_EFFECT_SMOOTH_LEVEL            "SMOOTH_LEVEL"
#define FACE_EFFECT_SKIN_COLOR              "SKIN_COLOR_LEVEL"
#define FACE_EFFECT_ENLARGE_EYE_LEVEL       "ENLARGE_EYE_LEVEL"
#define FACE_EFFECT_SLIM_FACE_LEVEL         "SLIM_FACE_LEVEL"
#define FACE_EFFECT_MODE                    "EFFECT_MODE"


//
class IEffectUser : public IInterface
{
public:
    DECLARE_META_INTERFACE(EffectUser);

    // disconnect from mmsdk service
    virtual void            disconnect() = 0;

    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    virtual String8         getName() = 0;
    //
    virtual bool            apply(ImageInfo const &rSrcImage,
                                  const sp<IMemory>& srcData,
                                  ImageInfo const &rDestImage,
                                  const sp<IMemory>& destData
                                 ) = 0;
    //
    virtual bool            setParameter(String8 const &parameterKey, void *value) = 0;

    virtual void            setUpdateListener(sp<IEffectUpdateListener>const &listener) = 0;

    virtual bool            release() = 0;
};

// ----------------------------------------------------------------------------

class BnEffectUser: public BnInterface<IEffectUser>
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

#endif   // _MEDIATEK_MMSDK_IEFFECT_USER_H_
