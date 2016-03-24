#define LOG_TAG "MtkCam/PREVIEWFEATUREBuffer"
//
#include <MyUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h>
#include <linux/cache.h>
//
#include "PreviewFeatureBufMgr.h"
//
#include <cutils/atomic.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

/******************************************************************************
*
*******************************************************************************/
void
PREVIEWFEATUREBuffer::
createBuffer()
{
    FUNCTION_IN;
    //
    mbufSize = (mbufSize + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    MY_LOGD("Image createBuffer w %d h %d\n",mWidth,mHeight);

    MSize imgSize(mWidth, mHeight);
    size_t bufStridesInBytes[3];
    bufStridesInBytes[0] = mWidth;
    bufStridesInBytes[1] = mWidth;
    bufStridesInBytes[2] = 0;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    IImageBufferAllocator::ImgParam imgParam(mbufSize, 0);


    sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create("PanoramaImgBuffer", imgParam);
    if  ( pHeap == 0 ) {
        MY_LOGE("NULL Heap\n");
        return;
    }

    PreviewBuffer = pHeap->createImageBuffer_FromBlobHeap(0, eImgFmt_NV21, imgSize, bufStridesInBytes);


    //IImageBufferAllocator::ImgParam imgParam(mbufSize, 0);
    //mpIMemDrv = IImageBufferAllocator::getInstance();
    //if ( ! mpIMemDrv ) {
    //    MY_LOGE("mpIMemDrv->init() error");
    //    return;
    //}
    //PreviewBuffer = mpIMemDrv->alloc("PreviewBuffer", imgParam);

    if  ( PreviewBuffer.get() == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return;
    }

    if ( !PreviewBuffer->lockBuf( "PanoJpg", (eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK) ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return;
    }


    MY_LOGD( "Preview bufAddr 0(0x%x) bufAddr 1(0x%x) bufAddr2(0x%x) size %d!",PreviewBuffer->getBufVA(0),PreviewBuffer->getBufVA(1), PreviewBuffer->getBufSizeInBytes(0));
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void
PREVIEWFEATUREBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    if( !PreviewBuffer->unlockBuf( "PanoBuffer" ) )
    {
        CAM_LOGE("unlock Buffer failed\n");
        return;
    }
      //pHeap->free(PreviewBuffer.get());
    //
    FUNCTION_OUT;
}

