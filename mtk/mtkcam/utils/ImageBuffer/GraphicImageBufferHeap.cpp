/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/GraphicImageBufferHeap"
//
#include "MyUtils.h"
#include <utils/include/imagebuf/IGraphicImageBufferHeap.h>
#include <utils/include/imagebuf/BaseImageBufferHeap.h>
//
using namespace android;
using namespace mmutils;
using namespace NSCam;
//


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  Image Buffer Heap.
 ******************************************************************************/
namespace {
class GraphicImageBufferHeap : public IGraphicImageBufferHeap
                             , protected NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class IGraphicImageBufferHeap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGraphicImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Accessors.
    virtual buffer_handle_t         getBufferHandle()   const   { return *mpBufferHandle; }
    virtual buffer_handle_t*        getBufferHandlePtr()const   { return mpBufferHandle; }
    virtual MINT                    getAcquireFence()   const   { return mAcquireFence; }
    virtual MVOID                   setAcquireFence(MINT fence) { mAcquireFence = fence; }
    virtual MINT                    getReleaseFence()   const   { return mReleaseFence; }
    virtual MVOID                   setReleaseFence(MINT fence) { mReleaseFence = fence; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual char const*             impGetMagicName()                   const   { return magicName(); }
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return *(HeapInfoVect_t*)(&mvHeapInfo); }
    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo);

public:     ////
    virtual MBOOL                   impLockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    );
    virtual MBOOL                   impUnlockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Heap Info.
                                    struct MyHeapInfo : public HeapInfo
                                    {
                                    };
    typedef Vector<sp<MyHeapInfo> > MyHeapInfoVect_t;

protected:  ////                    Buffer Info.
    typedef Vector<sp<BufInfo> >    MyBufInfoVect_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~GraphicImageBufferHeap() {}
                                    GraphicImageBufferHeap(
                                        char const* szCallerName,
                                        buffer_handle_t*  pBufferHandle,
                                        sp<GraphicBuffer> pGraphicBuffer,
                                        MINT const acquire_fence,
                                        MINT const release_fence
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    MyHeapInfoVect_t                mvHeapInfo;
    MyBufInfoVect_t                 mvBufInfo;
    GrallocDynamicInfo              mGrallocInfo;

protected:  ////
    buffer_handle_t*                mpBufferHandle;
    sp<GraphicBuffer>               mpGraphicBuffer;
    MINT                            mAcquireFence;
    MINT                            mReleaseFence;

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
static
bool
validate_camera3_stream_buffer(camera3_stream_buffer const* stream_buffer)
{
    if  ( ! stream_buffer ) {
        CAM_LOGE("camera3_stream_buffer: NULL");
        return false;
    }
    //
    if  ( ! stream_buffer->stream ) {
        CAM_LOGE("camera3_stream_buffer: NULL stream");
        return false;
    }
    //
    if  ( ! stream_buffer->buffer ) {
        CAM_LOGE("camera3_stream_buffer: NULL buffer");
        return false;
    }
    //
    if  ( ! *stream_buffer->buffer ) {
        CAM_LOGE("camera3_stream_buffer: NULL *buffer");
        return false;
    }
    ///
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
create(
    char const* szCallerName,
    camera3_stream_buffer const* stream_buffer
)
{
    if  ( ! validate_camera3_stream_buffer(stream_buffer) ) {
        return NULL;
    }
    //
    //
    GrallocStaticInfo staticInfo;
    status_t status = IGrallocHelper::singleton()->query(*stream_buffer->buffer, &staticInfo, NULL);
    if  ( OK != status ) {
        CAM_LOGE("cannot query the real format from buffer_handle_t - status:%d(%s)", status, ::strerror(-status));
        return  NULL;
    }
    //
    sp<GraphicBuffer> pGraphicBuffer = new GraphicBuffer(
        stream_buffer->stream->width,
        stream_buffer->stream->height,
        staticInfo.format,
        stream_buffer->stream->usage,
        0,  //stride in pixels
            //raw format: meaningless
            //unused now: set to 0
        const_cast<native_handle_t*>(*stream_buffer->buffer),
        false
    );
    if  ( pGraphicBuffer == 0 ) {
        CAM_LOGE("Fail to new GraphicBuffer");
        return NULL;
    }
    //
    GraphicImageBufferHeap* pHeap = NULL;
    pHeap = new GraphicImageBufferHeap(
        szCallerName,
        stream_buffer->buffer,
        pGraphicBuffer,
        stream_buffer->acquire_fence,
        stream_buffer->release_fence
    );
    if  ( ! pHeap ) {
        CAM_LOGE("Fail to new a heap");
        return NULL;
    }
    MSize const imgSize(pGraphicBuffer->getWidth(), pGraphicBuffer->getHeight());
    //MINT format = pGraphicBuffer->getPixelFormat();
    MINT format = staticInfo.format;
    switch  (format)
    {
    case eImgFmt_BLOB:
        CAM_LOGW_IF(1, "create() based-on camera3_stream_buffer...");
#if 0
        CAM_LOGW_IF(1, "force to convert BLOB format to JPEG format");
        format = eImgFmt_JPEG;
#else
        CAM_LOGW_IF(1, "should we convert BLOB format to JPEG format ???");
#endif
        break;
    }
    if  ( ! pHeap->onCreate(imgSize, format, 0, /*enableLog*/MFALSE) ) {
        CAM_LOGE("onCreate width(%d) height(%d)", imgSize.w, imgSize.h);
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
create(
    char const* szCallerName,
    GraphicBuffer* pGraphicBuffer,
    MINT const acquire_fence,
    MINT const release_fence
)
{
    if  ( pGraphicBuffer == 0 ) {
        CAM_LOGE("NULL GraphicBuffer");
        return NULL;
    }
    //
    GrallocStaticInfo staticInfo;
    status_t status = IGrallocHelper::singleton()->query(pGraphicBuffer->handle, &staticInfo, NULL);
    if  ( OK != status ) {
        CAM_LOGE("cannot query the real format from buffer_handle_t - status:%d(%s)", status, ::strerror(-status));
        return  NULL;
    }
    //
    MINT const format = staticInfo.format;
    //MINT const format1 = pGraphicBuffer->getPixelFormat();
    MSize const imgSize(pGraphicBuffer->getWidth(), pGraphicBuffer->getHeight());
    //
    GraphicImageBufferHeap* pHeap = NULL;
    pHeap = new GraphicImageBufferHeap(
        szCallerName,
        &pGraphicBuffer->handle,
        pGraphicBuffer,
        acquire_fence,
        release_fence
    );
    if  ( ! pHeap ) {
        CAM_LOGE("Fail to new a heap");
        return NULL;
    }
    if  ( ! pHeap->onCreate(imgSize, format) ) {
        CAM_LOGE("onCreate width(%d) height(%d)", imgSize.w, imgSize.h);
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
GraphicImageBufferHeap::
GraphicImageBufferHeap(
    char const* szCallerName,
    buffer_handle_t*  pBufferHandle,
    sp<GraphicBuffer> pGraphicBuffer,
    MINT const acquire_fence,
    MINT const release_fence
)
    : IGraphicImageBufferHeap()
    , BaseImageBufferHeap(szCallerName)
    //
    , mvHeapInfo()
    , mvBufInfo()
    , mGrallocInfo()
    //
    , mpBufferHandle(pBufferHandle)
    , mpGraphicBuffer(pGraphicBuffer)
    , mAcquireFence(acquire_fence)
    , mReleaseFence(release_fence)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    GrallocStaticInfo staticInfo;
    IGrallocHelper*const pGrallocHelper = IGrallocHelper::singleton();
    //
    status_t status = OK;
    status = pGrallocHelper->setDirtyCamera(getBufferHandle());
    if  ( OK != status ) {
        MY_LOGE("cannot set dirty - status:%d(%s)", status, ::strerror(-status));
        return MFALSE;
    }
    //
    status = pGrallocHelper->setColorspace_JFIF(getBufferHandle());
    if  ( OK != status ) {
        MY_LOGE("cannot set colorspace JFIF - status:%d(%s)", status, ::strerror(-status));
        return MFALSE;
    }
    //
    Vector<int> const& ionFds = mGrallocInfo.ionFds;
    status = pGrallocHelper->query(getBufferHandle(), &staticInfo, &mGrallocInfo);
    if  ( OK != status ) {
        MY_LOGE("cannot query the real format from buffer_handle_t - status:%d[%s]", status, ::strerror(status));
        return MFALSE;
    }
    //
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    //
    if ( ionFds.size() == 1 )
    {
        size_t sizeInBytes = 0;
        for (size_t i = 0; i < staticInfo.planes.size(); i++) {
            sizeInBytes += staticInfo.planes[i].sizeInBytes;
        }
        //
        HelperParamMapPA param;
        ::memset(&param, 0, sizeof(param));
        param.phyAddr   = 0;
        param.virAddr   = 0;
        param.ionFd     = ionFds[0];
        param.size      = sizeInBytes;
        if  ( ! helpMapPhyAddr(__FUNCTION__, param) ) {
            MY_LOGE("helpMapPhyAddr");
            return  MFALSE;
        }
        //
        MINTPTR pa = param.phyAddr;
        for (size_t i = 0; i < getPlaneCount(); i++)
        {
            sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            pHeapInfo->heapID = ionFds[0];
            //
            sp<BufInfo> pBufInfo = new BufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes= staticInfo.planes[i].rowStrideInBytes;
            pBufInfo->sizeInBytes   = staticInfo.planes[i].sizeInBytes;
            pBufInfo->pa            = pa;
            pa += pBufInfo->sizeInBytes;
            //
            rvBufInfo[i]->stridesInBytes= pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes   = pBufInfo->sizeInBytes;
        }
    }
    else
    if  ( ionFds.size() == getPlaneCount() )
    {
        for (size_t i = 0; i < getPlaneCount(); i++)
        {
            HelperParamMapPA param;
            ::memset(&param, 0, sizeof(param));
            param.phyAddr   = 0;
            param.virAddr   = 0;
            param.ionFd     = ionFds[i];
            param.size      = staticInfo.planes[i].sizeInBytes;
            if  ( ! helpMapPhyAddr(__FUNCTION__, param) ) {
                MY_LOGE("helpMapPhyAddr");
                return  MFALSE;
            }
            MINTPTR pa = param.phyAddr;
            //
            sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            pHeapInfo->heapID = ionFds[i];
            //
            sp<BufInfo> pBufInfo = new BufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes= staticInfo.planes[i].rowStrideInBytes;
            pBufInfo->sizeInBytes   = staticInfo.planes[i].sizeInBytes;
            pBufInfo->pa            = pa;
            //
            rvBufInfo[i]->stridesInBytes= pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes   = pBufInfo->sizeInBytes;
        }
    }
    else
    {
        MY_LOGE("Unsupported ionFds:#%zu plane:#%zu", ionFds.size(), getPlaneCount());
        return MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeap::
impUninit(BufInfoVect_t const& /*rvBufInfo*/)
{
    Vector<int> const& ionFds = mGrallocInfo.ionFds;
    //
    if ( ionFds.size() == 1 )
    {
        size_t sizeInBytes = 0;
        for (size_t i = 0; i < mvBufInfo.size(); i++) {
            sizeInBytes += mvBufInfo[i]->sizeInBytes;
        }
        //
        HelperParamMapPA param;
        ::memset(&param, 0, sizeof(param));
        param.phyAddr   = mvBufInfo[0]->pa;
        param.virAddr   = 0;
        param.ionFd     = ionFds[0];
        param.size      = sizeInBytes;
        if  ( ! helpUnmapPhyAddr(__FUNCTION__, param) ) {
            MY_LOGE("helpUnmapPhyAddr");
        }
    }
    else
    if  ( ionFds.size() == getPlaneCount() )
    {
        for (size_t i = 0; i < getPlaneCount(); i++)
        {
            HelperParamMapPA param;
            ::memset(&param, 0, sizeof(param));
            param.phyAddr   = mvBufInfo[i]->pa;
            param.virAddr   = 0;
            param.ionFd     = ionFds[i];
            param.size      = mvBufInfo[i]->sizeInBytes;
            if  ( ! helpUnmapPhyAddr(__FUNCTION__, param) ) {
                MY_LOGE("helpUnmapPhyAddr");
            }
        }
    }
    else
    {
        MY_LOGE("Unsupported ionFds:#%zu plane:#%zu", ionFds.size(), getPlaneCount());
    }
    //
    mvBufInfo.clear();
    mvHeapInfo.clear();
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeap::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    void* vaddr = NULL;
    status_t status = mpGraphicBuffer->lock(usage, &vaddr);
    if  ( OK != status ) {
        MY_LOGE(
            "%s GraphicBuffer::lock - status:%d(%s) usage:%#x|%#x format:%#x",
            szCallerName, status, ::strerror(-status),
            usage, mpGraphicBuffer->getUsage(), mpGraphicBuffer->getPixelFormat()
        );
        return MFALSE;
    }
    //
    //  SW Access.
    if  ( 0 != (usage & (eBUFFER_USAGE_SW_MASK)) ) {
        MY_LOGF_IF(0==vaddr, "SW Access but va=0 - usage:%#x|%#x format:%#x", usage, mpGraphicBuffer->getUsage(), mpGraphicBuffer->getPixelFormat());
        MY_LOGF_IF(1<mGrallocInfo.ionFds.size(), "[Not Implement] ionFds:#%zu>1", mGrallocInfo.ionFds.size());
    }
    MINTPTR va = reinterpret_cast<MINTPTR>(vaddr);
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        //  SW Access.
        if  ( 0 != (usage & (eBUFFER_USAGE_SW_MASK)) ) {
            rvBufInfo[i]->va = va;
            va += mvBufInfo[i]->sizeInBytes;
        }
        //
        //  HW Access
        if  ( 0 != (usage & (~eBUFFER_USAGE_SW_MASK)) ) {
            rvBufInfo[i]->pa = mvBufInfo[i]->pa;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeap::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        rvBufInfo[i]->va = 0;
        rvBufInfo[i]->pa = 0;
    }
    //
    status_t status = mpGraphicBuffer->unlock();
    MY_LOGW_IF(
        OK!=status,
        "%s GraphicBuffer::unlock - status:%d(%s) usage:%#x",
        szCallerName, status, ::strerror(-status), usage
    );
    //
    return  MTRUE;
}

