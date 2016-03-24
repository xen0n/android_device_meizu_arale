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

#ifndef _MEDIATEK_MMSDK_IIMAGETRANSFORM_USER_H_
#define _MEDIATEK_MMSDK_IIMAGETRANSFORM_USER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <system/graphics.h>
#include <ui/Rect.h>

#include "ImageBufferInfo.h"
namespace android {
namespace NSMMSdk {
namespace NSImageTransform {

//
enum ERotation
{
    eROT_0        = 0,
    eROT_90       = 0x04,
    eROT_180      = 0x03,
    eROT_270      = 0x07
};

//
enum EFlip
{
    eFLIP_H       = 0x01,
    eFLIP_V       = 0x02
};

struct TrasformOptions
{
    Rect rect;
    int transform;
    int encQuality;
    int isDither;
    int sharpnessLevel;
    TrasformOptions()
    :rect()
    ,transform(0)
    ,encQuality(50)
    ,isDither(0)
    ,sharpnessLevel(0)
    {
    }
};


//
class IImageTransformUser : public IInterface
{
public:
    DECLARE_META_INTERFACE(ImageTransformUser);

    // disconnect from mmsdk service
    virtual void            disconnect() = 0;

    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool            applyTransform(ImageInfo const &rSrcImage,
                                           const sp<IMemory>&srcData,
                                           ImageInfo const &rDestImage,
                                           const sp<IMemory>& destData,
                                           TrasformOptions const &rOptions

                                          ) = 0;
};

// ----------------------------------------------------------------------------

class BnImageTransformUser: public BnInterface<IImageTransformUser>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace NSImageTransform
}; // namespace NSMMSdk
}; // namespace android

#endif   //_MEDIATEK_MMSDK_IIMAGETRANSFORM_USER_H_
