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
#include <effect_device.h>

//
#include <SImager/ISImager.h>
using namespace NSCam::NSIoPipe::NSSImager;
//
//#include <IFaceBeautySDKHal.h>
using namespace NSCam;

//#define LOG_NDEBUG 0
#include <cutils/log.h>


#include "effectDevice.h"


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
    effect_device_t* effect_dev = reinterpret_cast<effect_device_t*>(dev);
    EffectDevice* device = static_cast<EffectDevice*>(effect_dev->priv);
    return device->close();
}
} // extern "C"

/******************************************************************************
 *
 ******************************************************************************/
EffectDevice::EffectDevice()
    :mSmoothLevel(0)
    ,mSkinColor(0)
    ,mEnlargeEyeLevel(0)
    ,mSlimFaceLevel(0)
    ,mMode(0)
{
    memset(&mDevice, 0, sizeof(mDevice));
    mDevice.common.tag    = HARDWARE_DEVICE_TAG;
    mDevice.common.version = EFFECT_DEVICE_API_VERSION_1_0;
    mDevice.common.close  = close_device;
    mDevice.ops           = const_cast<effect_device_ops_t*>(&sOps);
    mDevice.priv          = this;
}

/******************************************************************************
 *
 ******************************************************************************/
EffectDevice::~EffectDevice()
{
}

/******************************************************************************
 *
 ******************************************************************************/
int EffectDevice::open(const hw_module_t *module, hw_device_t **device)
{
    mDevice.common.module = const_cast<hw_module_t*>(module);
    *device = &mDevice.common;

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
int EffectDevice::close()
{
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EffectDevice::
createEffect(
    char const* effect_name,
    eEFFECT_MODE eMode,
    void *cookie
)
{
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
char*
EffectDevice::
getEffectName(void *cookie)
{

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectDevice::
setParameters
(
    const char *param,
    const void *value,
    void *cookie
)
{
    //
    if  ( param && 0 == ::strcmp(param, FACE_EFFECT_SMOOTH_LEVEL) )
    {
        mSmoothLevel = *((int*)value);
        MY_LOGD("mSmoothLevel:%d", mSmoothLevel);
    }
    //
    if  ( param && 0 == ::strcmp(param, FACE_EFFECT_SKIN_COLOR) )
    {

        mSkinColor = *((int*)value);
        MY_LOGD("mSkinColor:%d", mSkinColor);
    }
    //
    if  ( param && 0 == ::strcmp(param, FACE_EFFECT_ENLARGE_EYE_LEVEL) )
    {
        mEnlargeEyeLevel = *((int*)value);
        MY_LOGD("mSkinColor:%d", mEnlargeEyeLevel);
    }
    //
    if  ( param && 0 == ::strcmp(param, FACE_EFFECT_SLIM_FACE_LEVEL) )
    {
        mSlimFaceLevel = *((int*)value);
        MY_LOGD("mSlimFaceLevel:%d", mSlimFaceLevel);
    }
    //
    if ( param && 0 == ::strcmp(param, FACE_EFFECT_MODE) )
    {
        mMode = *((int*)value);
        MY_LOGD("mMode:%d", mMode);
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectDevice::
apply
(
    NSCam::IImageBuffer const *srcImage,
    NSCam::IImageBuffer const *destImage,
    void *cookie
)
{
    bool ret = false;
    IImageBuffer const *pSrcImg = srcImage;
    // fb hal only support I422, we need to convert it first
    IImageBuffer *pTempSrcBuffer = NULL;
    if (srcImage->getImgFormat() != eImgFmt_I422)
    {
        if (!allocBuf(&pTempSrcBuffer, srcImage->getImgSize(), eImgFmt_I422))
        {
            MY_LOGE("Fail to alloc temp source image buffer");
            return false;
        }
        ret = imageTransform(srcImage, pTempSrcBuffer);
        pTempSrcBuffer->saveToFile("/sdcard/fbTemp.yuv");

        pSrcImg = pTempSrcBuffer;
    }

    // fb hal only support I422 to I422
    if ( (destImage->getImgFormat() != eImgFmt_I422)
         || (srcImage->getImgSize() != destImage->getImgSize()))
    {
        MY_LOGD("dest fmt:0x%x, %dx%d", destImage->getImgFormat(), destImage->getImgSize().w,
                 destImage->getImgSize().h);
        IImageBuffer *pTempDestBuffer = NULL;
        if (!allocBuf(&pTempDestBuffer, srcImage->getImgSize(), eImgFmt_I422))
        {
            MY_LOGE("Fail to alloc temp dest image buffer");
            return false;
        }
        ret = applyFBEffect(pSrcImg, pTempDestBuffer)
              && imageTransform(pTempDestBuffer, destImage);
        //
        pTempDestBuffer->saveToFile("/sdcard/fbRes.yuv");
        //
        freeBuf(&pTempDestBuffer);
    }
    else
    {
        ret = applyFBEffect(pSrcImg, destImage);
    }

    //
    if (pTempSrcBuffer != NULL)
    {
        freeBuf(&pTempSrcBuffer);
    }

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
EffectDevice::
applyFBEffect(IImageBuffer const *pSrcImage, IImageBuffer const *pDestImage)
{
// Currently no FaceBeautySDKHal exist. Please use EffectHal to do cfb.
#if 0
    IFaceBeautySDKHal* pFBSDKHal = IFaceBeautySDKHal::createInstance();
    if (NULL == pFBSDKHal)
    {
        printf("failed to create FaceBeautySDK instance");
        return false;
    }
    //
    FACE_BEAUTY_SDK_HAL_PARAMS param;
    param.SmoothLevel = mSmoothLevel;
    param.SkinColor = mSkinColor;
    param.EnlargeEyeLevel = mEnlargeEyeLevel;
    param.SlimFaceLevel = mSlimFaceLevel;

    //
    if (!pFBSDKHal->init(static_cast<FACE_BEAUTY_SDK_HAL_MODE>(mMode)))
    {
        MY_LOGE("error to init FB hal");
        return false;
    }
    //
    if (!pFBSDKHal->apply(const_cast<IImageBuffer*>(pSrcImage), const_cast<IImageBuffer*>(pDestImage), param))
    {
        MY_LOGE("fail to apply FB ");
        return false;
    }

    //
    pFBSDKHal->uninit();
    pFBSDKHal->destroyInstance();
#endif
    return true;
}



/******************************************************************************
*
*******************************************************************************/
bool
EffectDevice::
imageTransform(IImageBuffer const *pSrcImage, IImageBuffer const *pDestImage)
{
    // (1). create Simager instance
    ISImager *pSImager = ISImager::createInstance(pSrcImage);
    if(pSImager == NULL)
    {
        MY_LOGE("Can not allocate SImager object");
        return false;
    }

    bool ret = pSImager->setTargetImgBuffer(pDestImage)
               && pSImager->execute();

    pSImager->destroyInstance();

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
EffectDevice::
allocBuf(IImageBuffer **pBuf, MSize const & imgSize, int const format)
{
    MUINT32 bufStridesInBytes[3] = {0};
using namespace NSCam::Utils::Format;

    MUINT32 plane = queryPlaneCount(format);
    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] = ( queryPlaneWidthInPixels(format,i, imgSize.w) * queryPlaneBitsPerPixel(format,i) ) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(
            format, imgSize, bufStridesInBytes, bufBoundaryInBytes, plane
            );


    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    *pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
    if  ( (*pBuf) == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return  MFALSE;
    }

    if ( !(*pBuf)->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return false;
    }

    //
    MY_LOGD("ImgBitsPerPixel:%d BufSizeInBytes:%d %d", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0), imgSize.size());
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
bool
EffectDevice::
freeBuf(IImageBuffer **pBuf)
{
    if( !(*pBuf)->unlockBuf( LOG_TAG ) )
    {
        MY_LOGE("unlock Buffer failed\n");
        return false;
    }
    pBuf = NULL;
    return true;
}



/******************************************************************************
 *
 ******************************************************************************/
bool
EffectDevice::
release(void *cookie)
{
    return 0;
}


extern "C" {
// Get handle to camera from device priv data
/******************************************************************************
 *
 ******************************************************************************/
static EffectDevice *effectDev_to_device(const effect_device_t *dev)
{
    return reinterpret_cast<EffectDevice*>(dev->priv);
}


/******************************************************************************
 *
 ******************************************************************************/
static bool create_effect(
    struct effect_device *dev,
    char const* effect_name,
    eEFFECT_MODE eMode,
    void *cookie
)
{

    return effectDev_to_device(dev)->createEffect(effect_name, eMode, cookie);
}

/******************************************************************************
 *
 ******************************************************************************/
static char* get_effect_name(
    struct effect_device *dev,
    void *cookie
)
{
    return effectDev_to_device(dev)->getEffectName(cookie);
}

/******************************************************************************
 *
 ******************************************************************************/
static bool set_parameters(
    struct effect_device *dev,
    const char *param,
    const void *value,
    void *cookie
)
{
    return effectDev_to_device(dev)->setParameters(param, value, cookie);
}

/******************************************************************************
 *
 ******************************************************************************/
static bool apply(
    struct effect_device *dev,
    NSCam::IImageBuffer const *srcImage,
    NSCam::IImageBuffer const *destImage,
    void *cookie
)
{
    return effectDev_to_device(dev)->apply(srcImage, destImage, cookie);
}

/******************************************************************************
 *
 ******************************************************************************/
static bool release(
    struct effect_device *dev,
    void *cookie
)
{
    return effectDev_to_device(dev)->release(cookie);
}

} // extern "C"

const effect_device_ops EffectDevice::sOps = {
    .create_effect               = NSMMSdkHAL::create_effect,
    .get_effect_name             = NSMMSdkHAL::get_effect_name,
    .set_parameters              = NSMMSdkHAL::set_parameters,
    .apply                       = NSMMSdkHAL::apply,
    .release                     = NSMMSdkHAL::release
};

} // namespace NSMMSdkHAL
