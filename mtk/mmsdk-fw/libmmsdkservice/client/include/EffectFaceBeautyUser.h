/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef _MEDIATEK_MMSDK_EFFECT_FACEBEAUTY_USER_H_
#define _MEDIATEK_MMSDK_EFFECT_FACEBEAUTY_USER_H_

#include <mmsdk/IEffectUser.h>
//
#include <mmsdk/IEffectUpdateListener.h>
//
#include <mmsdk_common.h>
#include <effect_device.h>


namespace android {
namespace NSMMSdk {
namespace NSEffect {

class EffectFaceBeautyUser :
    public BnEffectUser
{
public:
    EffectFaceBeautyUser();
    //
    virtual     ~EffectFaceBeautyUser(){};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // disconnect from mmsdk service
    virtual void            disconnect();
    //
    virtual String8         getName();
    //
    virtual bool            apply(ImageInfo const &rSrcImage,
                                  const sp<IMemory>& srcData,
                                  ImageInfo const &rDestImage,
                                  const sp<IMemory>& destData
                                 );
    //
    virtual bool            setParameter(String8 const &parameterKey, void *value);
    //
    virtual void            setUpdateListener(sp<IEffectUpdateListener>const &listener);
    //
    virtual bool            release();

protected:
    virtual bool                    loadHALModule(mmsdk_module_t **module);
    virtual bool                    checkHALModuleVersion(mmsdk_module_t const *module);
    virtual bool                    openDevice(hw_module_t *module, effect_device_t **device);
    virtual bool                    closeDevice(effect_device_t  **device);
    virtual mmsdk_module_t*         getHALModule() {return mpMMSdkModule;};

    //
    virtual void                    convertImgBufToIImageBuffer(ImageInfo const &rImg,
                                                                const sp<IMemory>& dataPrt,
                                                                NSCam::IImageBuffer **pImageBuffer);

    virtual bool                    checkIfSupportedFormat(int format);
    virtual int                     mapToImageFormat(int format);

private:
    mmsdk_module_t                  *mpMMSdkModule;
    effect_device_t                   *mpDevice;
    sp<IEffectUpdateListener>       mpListener;
    mutable Mutex               mLock;

};

}; // namespace NSImageTransform
}; // namespace NSMMSdk
}; // namespace android

#endif   //_MEDIATEK_MMSDK_EFFECT_FACEBEAUTY_USER_H_
