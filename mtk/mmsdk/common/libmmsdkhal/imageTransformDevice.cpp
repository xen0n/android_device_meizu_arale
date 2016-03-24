/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "mmsdk/ImageTransformDevice"

#define MTK_LOG_ENABLE 1
#include <cstdlib>
#include <pthread.h>
#include <sync/sync.h>
#include "mmsdkHAL.h"

#include <common.h>
#include <imageTransform_device.h>

//
#include <SImager/ISImager.h>
using namespace NSCam::NSIoPipe::NSSImager; 

//#define LOG_NDEBUG 0
#include <cutils/log.h>


#include "imageTransformDevice.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

namespace NSMMSdkHAL {

extern "C" {
// Shim passed to the framework to close an opened device.
static int close_device(hw_device_t* dev)
{
    imageTransform_device_t* imageTransform_dev = reinterpret_cast<imageTransform_device_t*>(dev);
    ImageTransformDevice* device = static_cast<ImageTransformDevice*>(imageTransform_dev->priv);
    return device->close();
}
} // extern "C"

/******************************************************************************
 *
 ******************************************************************************/
ImageTransformDevice::ImageTransformDevice()
{
    memset(&mDevice, 0, sizeof(mDevice));
    mDevice.common.tag    = HARDWARE_DEVICE_TAG;
    mDevice.common.version = IMAGE_TRANSFORM_DEVICE_API_VERSION_1_0;
    mDevice.common.close  = close_device;
    mDevice.ops           = const_cast<imageTransfrom_device_ops_t*>(&sOps);
    mDevice.priv          = this;
}

/******************************************************************************
 *
 ******************************************************************************/
ImageTransformDevice::~ImageTransformDevice()
{
}

/******************************************************************************
 *
 ******************************************************************************/
int ImageTransformDevice::open(const hw_module_t *module, hw_device_t **device)
{    
    mDevice.common.module = const_cast<hw_module_t*>(module);
    *device = &mDevice.common;

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
int ImageTransformDevice::close()
{    
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool ImageTransformDevice::applyTransform
(
    NSCam::IImageBuffer const *pSrcImage, 
    NSCam::IImageBuffer const *pDestImage, 
    transformOptions const *pOptions
)
{
    MY_LOGD("srcImage(%x,%d,%d), destImage(%x,%d,%d)", 
             pSrcImage->getImgFormat(), pSrcImage->getImgSize().w,  
             pSrcImage->getImgSize().h,
             pDestImage->getImgFormat(), pDestImage->getImgSize().w, 
             pDestImage->getImgSize().h); 
    //
    MY_LOGD("options[rect]=(%d,%d,%d,%d)", 
             pOptions->rect.p.x, pOptions->rect.p.y, pOptions->rect.s.w, 
             pOptions->rect.s.h); 
    //
    MY_LOGD("options[trans, quality, isDither, sharpnessLevel]=(%d,%d,%d,%d)",
             pOptions->transform, pOptions->encodeQuality, 
             pOptions->isDither, pOptions->sharpnessLevel); 
 
    // (1). create Simager instance 
    ISImager *pSImager = ISImager::createInstance(pSrcImage); 
    if(pSImager == NULL)
    { 
        MY_LOGE("Can not allocate SImager object"); 
        return false; 
    }

    bool ret = pSImager->setCropROI(pOptions->rect) 
               && pSImager->setTransform(pOptions->transform) 
               && pSImager->setEncodeParam(1, pOptions->encodeQuality) 
               && pSImager->setTargetImgBuffer(pDestImage) 
               && pSImager->execute(); 

    pSImager->destroyInstance(); 
    return ret; 
}



extern "C" {
// Get handle to camera from device priv data
/******************************************************************************
 *
 ******************************************************************************/
static ImageTransformDevice *imageTransformDev_to_device(const imageTransform_device_t *dev)
{
    return reinterpret_cast<ImageTransformDevice*>(dev->priv);
}

/******************************************************************************
 *
 ******************************************************************************/
static bool applyTransform(
    struct imageTransform_device *dev, 
    NSCam::IImageBuffer const *pSrcImage, 
    NSCam::IImageBuffer const *pDestImage, 
    transformOptions_t const *pOptions
)
{
    return imageTransformDev_to_device(dev)->applyTransform(pSrcImage, pDestImage, pOptions);
}

} // extern "C"

const imageTransfrom_device_ops ImageTransformDevice::sOps = {
    .applyTransform              = NSMMSdkHAL::applyTransform
};

} // namespace NSMMSdkHAL
