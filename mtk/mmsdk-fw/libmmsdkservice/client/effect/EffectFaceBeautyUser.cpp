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
#define LOG_TAG "mmsdk/EffectFaceBeautyUser"

#define MTK_LOG_ENABLE 1
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>

#include <utils/log.h>
//
#include <mmsdk/IEffectUser.h>
#include <mmsdk/IEffectUpdateListener.h>
//
#include <common.h>
#include <ImageBufferHeap.h>
using namespace NSCam;

#include "../include/EffectFaceBeautyUser.h"

static int getCallingPid() {
    return android::IPCThreadState::self()->getCallingPid();
}

/******************************************************************************
 *
 ******************************************************************************/
#include <cutils/log.h>
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define FUNCTION_LOG_START    MMSDK_LOGD("[%s]+, (pid=%d, tid=%d)", __FUNCTION__, getCallingPid(), gettid());
#define FUNCTION_LOG_END      MMSDK_LOGD("[%s]-", __FUNCTION__);

namespace android {
namespace NSMMSdk {
namespace NSEffect {

/******************************************************************************
 *
 ******************************************************************************/
EffectFaceBeautyUser::
EffectFaceBeautyUser()
    :mpListener(0)
    ,mLock()
{
    //
    mpMMSdkModule = 0;
    mpDevice = 0;
    //
    bool ret =    loadHALModule(&mpMMSdkModule)
               && checkHALModuleVersion(mpMMSdkModule);
    //
    if (!ret)
    {
        MY_LOGE("error to load mmsdk hal module");
        return;
    }
    //
    if (!openDevice(&mpMMSdkModule->common, &mpDevice))
    {
        MY_LOGE("open effect device fail");
        return;
    }
    //

}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectFaceBeautyUser::
disconnect(
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    //
    if (mpDevice != 0)
    {
        closeDevice(&mpDevice);
    }
    FUNCTION_LOG_END
    return ;
}

/******************************************************************************
 *
 ******************************************************************************/
String8
EffectFaceBeautyUser::
getName(
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    String8 effectName("faceBeauty");

    FUNCTION_LOG_END
    return effectName;

}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
apply
(
    ImageInfo const &rSrcImage,
    const sp<IMemory>& srcData,
    ImageInfo const &rDestImage,
    const sp<IMemory>& destData
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    MY_LOGD("srcImgInfo, (w, h, plane, stride)= (%d,%d,%d,(%d,%d,%d))",
                rSrcImage.width, rSrcImage.height, rSrcImage.numOfPlane,
                rSrcImage.stride[0], rSrcImage.stride[1], rSrcImage.stride[2]);

    MY_LOGD("destmgInfo, (w, h, plane, stride)= (%d,%d,%d,(%d,%d,%d))",
                rDestImage.width, rDestImage.height, rDestImage.numOfPlane,
                rDestImage.stride[0], rDestImage.stride[1], rDestImage.stride[2]);

    // check if supported format
    if (!checkIfSupportedFormat(rSrcImage.format))
    {
        MY_LOGE("UnSupport source image format");
        return false;
    }
    //
    if (!checkIfSupportedFormat(rDestImage.format))
    {
        MY_LOGE("UnSupport dest image format");
        return false;
    }
    //
    if (mpDevice == 0)
    {
        MY_LOGE("No device open");
        return false;
    }
    //
    IImageBuffer *pSrcBuffer = NULL;
    IImageBuffer *pDestBuffer = NULL;
    //
    convertImgBufToIImageBuffer(rSrcImage, srcData, &pSrcBuffer);
    convertImgBufToIImageBuffer(rDestImage, destData, &pDestBuffer);
    //
    pSrcBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);
    pDestBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);

    bool ret = mpDevice->ops->apply(mpDevice, pSrcBuffer, pDestBuffer, 0);
    //
    if (mpListener != 0)
    {
        //
        MY_LOGD("callback for effect updated");
        mpListener->onEffectUpdated(this, 0);
    }
    //
    pSrcBuffer->unlockBuf(LOG_TAG);
    pDestBuffer->unlockBuf(LOG_TAG);

    FUNCTION_LOG_END
    return ret;
};


/******************************************************************************
 * set flip 1:vertical flip, 2: horizontal flip);
 ******************************************************************************/
bool
EffectFaceBeautyUser::
setParameter(
    String8 const &parameterKey,
    void *value
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    MY_LOGD("setParameter:%s, %d", parameterKey.string(), *(int*)value);

    //
    if (mpDevice == 0)
    {
        MY_LOGE("No device open");
        return false;
    }
    //
    bool ret = mpDevice->ops->set_parameters(mpDevice, parameterKey.string(), value, 0);

    FUNCTION_LOG_END
    return ret;

}

/******************************************************************************
 *
 ******************************************************************************/
void
EffectFaceBeautyUser::
setUpdateListener(
    sp<IEffectUpdateListener>const &listener
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    mpListener = listener;

    FUNCTION_LOG_END
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
release
(
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mLock);
    //
    if (mpDevice != 0)
    {
        closeDevice(&mpDevice);
        mpDevice = 0;
    }
    FUNCTION_LOG_END
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectFaceBeautyUser::
convertImgBufToIImageBuffer
(
    ImageInfo const &rImg,
    const sp<IMemory> &dataPrt,
    IImageBuffer **pImageBuffer
)
{
    //IImageBuffer pImageBuffer;
#if '1' == MTKCAM_HAVE_IIMAGE_BUFFER
    MUINT32 bufStridesInBytes[3] = {static_cast<MUINT32>(rImg.stride[0]),
                                    static_cast<MUINT32>(rImg.stride[1]),
                                    static_cast<MUINT32>(rImg.stride[2])
                                   };
    //


    MINT32 bufBoundaryInBytes[3] = {0,0,0};
    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
                                                    mapToImageFormat(rImg.format),
                                                    MSize(rImg.width,
                                                          rImg.height),
                                                          bufStridesInBytes,
                                                          bufBoundaryInBytes,
                                                          rImg.numOfPlane);
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = dataPrt->getMemory(&offset, &size);
    void *data = ((uint8_t *)heap->base()) + offset;

    MY_LOGD("memory:(0x%x,%d)", (uintptr_t)data, size);

    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(-1,
                                                (uintptr_t)data,
                                                 0,
                                                 0,
                                                 0);
    //
    ImageBufferHeap *pHeap = ImageBufferHeap::create(LOG_TAG,
                                                     imgParam,
                                                     portBufInfo);
    if(pHeap == 0)
    {
        MY_LOGE("pHeap is NULL");
        return;
    }
    //
    *pImageBuffer = pHeap->createImageBuffer();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
checkIfSupportedFormat(int format)
{
    switch (format)
    {
        case eImgFmt_YUY2:
        case eImgFmt_NV16:
        case eImgFmt_NV21:
        case eImgFmt_RGB565:
        case eImgFmt_RGBA8888:
        case eImgFmt_YV12:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return true;
        default:
           return false;
    }
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
int
EffectFaceBeautyUser::
mapToImageFormat(int format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
             return eImgFmt_YUY2;
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
             return eImgFmt_NV16;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
             return eImgFmt_NV21;
        case HAL_PIXEL_FORMAT_RGB_565:
             return eImgFmt_RGB565;
        case HAL_PIXEL_FORMAT_YV12:
             return eImgFmt_YV12;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return eImgFmt_I420;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return eImgFmt_RGBA8888;
        default:
           return eImgFmt_UNKNOWN;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
loadHALModule
(
    mmsdk_module_t **module
)
{
    //
    if (::hw_get_module(MMSDK_HARDWARE_MODULE_ID,
           (const hw_module_t **)module) < 0)
    {
        MY_LOGW("Could not load mmsdk HAL module");
        return false;
    }
    //
    MY_LOGD("Load \"%s\" mmsdk moudle", (*module)->common.name);
    //
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
checkHALModuleVersion
(
    mmsdk_module_t const *module
)
{
    if (module->common.module_api_version == MMSDK_MODULE_API_VERSION_1_0)
    {
        MY_LOGD("MMSDK API is 1.0");
    }
    else
    {
        MY_LOGD("MMSDK API is not 1.0");
        return false;
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
openDevice
(
    hw_module_t *module,
    effect_device_t **device

)
{
    //
    if (0 == module)
    {
        MY_LOGE("Null module object");
        return false;
    }
    //
    return (0 == module->methods->open(module, MMSDK_HARDWARE_EFFECT,
                                   (hw_device_t**)device));
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectFaceBeautyUser::
closeDevice
(
    effect_device_t  **device
)
{
    //
    if (0 == *device)
    {
        MY_LOGE("Null device object");
        return false;
    }
    //
    int rc = (*device)->common.close(&((*device)->common));
    if (rc != OK)
    {
        MY_LOGE("Could not close image transform device:%d", rc);
        return false;
    }

    *device = 0;
    return true;
}

// ----------------------------------------------------------------------------
}; // namespace NSEffect
}; // namespace NSMMSdk
}; // namespace android
