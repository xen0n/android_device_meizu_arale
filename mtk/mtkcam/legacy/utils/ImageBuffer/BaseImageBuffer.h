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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFER_H_
//
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <utils/imagebuf/BaseImageBufferHeap.h>


/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSImageBuffer {
using namespace android;
using namespace NSImageBufferHeap;

/******************************************************************************
 *  Image Buffer (Base).
 ******************************************************************************/
class BaseImageBuffer : public IImageBuffer, protected virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBuffer Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Reference Counting.
    virtual MVOID                   incStrong(MVOID const* id)          const   { RefBase::incStrong(id); }
    virtual MVOID                   decStrong(MVOID const* id)          const   { RefBase::decStrong(id); }
    virtual MINT32                  getStrongCount()                    const   { return RefBase::getStrongCount(); }

public:     ////                    Image Attributes.
    virtual MINT                    getImgFormat()                      const   { return mImgFormat; }
    virtual MSize const&            getImgSize()                        const   { return mImgSize; }
    virtual size_t                  getImgBitsPerPixel()                const;
    virtual size_t                  getPlaneBitsPerPixel(size_t index)   const;
    virtual size_t                  getPlaneCount()                     const   { return mPlaneCount; }
    virtual size_t                  getBitstreamSize()                  const   { return mBitstreamSize; }
    virtual MBOOL                   setBitstreamSize(size_t const bitstreamsize);
    virtual IImageBufferHeap*       getImageBufferHeap()                const   { return mspImgBufHeap.get(); }
    virtual MBOOL                   setExtParam(MSize const& imgSize, size_t offsetInBytes);
    virtual size_t                  getExtOffsetInBytes(size_t index)   const;

public:     ////                    Buffer Attributes.
    virtual char const*             getMagicName()                      const   { return mspImgBufHeap->getMagicName(); }
    virtual MINT32                  getFD(size_t index = 0)              const   { return mspImgBufHeap->getHeapID(index); }
    virtual size_t                  getFDCount()                        const   { return mspImgBufHeap->getHeapIDCount(); }
    virtual size_t                  getBufOffsetInBytes(size_t index)   const;
    virtual MINTPTR                 getBufPA(size_t index)              const;
    virtual MINTPTR                 getBufVA(size_t index)              const;
    virtual size_t                  getBufSizeInBytes(size_t index)     const;
    virtual size_t                  getBufStridesInBytes(size_t index)  const;

public:     ////                    Buffer Operations.
    virtual MBOOL                   lockBuf(
                                        char const* szCallerName,
                                        MINT usage
                                    );
    virtual MBOOL                   unlockBuf(
                                        char const* szCallerName
                                    );
    virtual MBOOL                   syncCache(eCacheCtrl const ctrl)            { return mspImgBufHeap->syncCache(ctrl); }

public:     ////                    File Operations.
    virtual MBOOL                   saveToFile(char const* filepath);
    virtual MBOOL                   loadFromFile(char const* filepath);

public:     ////                    Timestamp Accesssors.
    virtual MINT64                  getTimestamp()                      const   { return mTimestamp; }
    virtual MVOID                   setTimestamp(MINT64 const timestamp)        { mTimestamp = timestamp; }

public:     ////                    Fence Operations.
    virtual MINT                    getAcquireFence()                   const   { return mAcquireFence; }
    virtual MVOID                   setAcquireFence(MINT fence)                 { mAcquireFence = fence; }
    virtual MINT                    getReleaseFence()                   const   { return mReleaseFence; }
    virtual MVOID                   setReleaseFence(MINT fence)                 { mReleaseFence = fence; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Called inside lock.
    virtual MBOOL                   lockBufLocked(char const* szCallerName, MINT usage);
    virtual MBOOL                   unlockBufLocked(char const* szCallerName);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  ////                       Destructor/Constructors.
    virtual                         ~BaseImageBuffer();
                                    BaseImageBuffer(sp<BaseImageBufferHeap> _spImgBufHeap,
                                                         MSize _imgSize,
                                                         MINT _imgFormat,
                                                         size_t _bufSize,
                                                         size_t const _strides[3],
                                                         size_t _offset = 0)
                                        : IImageBuffer()
                                        , mspImgBufHeap(_spImgBufHeap)
                                        , mLockMtx()
                                        , mLockCount(0)
                                        , mLockUsage(0)
                                        , mvImgBufInfo()
                                        , mvBufHeapInfo()
                                        //
                                        , mImgSize(_imgSize)
                                        , mBufHeight(_imgSize.h)
                                        , mImgFormat(_imgFormat)
                                        , mPlaneCount(NSCam::Utils::Format::queryPlaneCount(_imgFormat))
                                        , mBitstreamSize(_bufSize)
                                        , mOffset(_offset)
                                        , mTimestamp(0)
                                        , mAcquireFence(0)
                                        , mReleaseFence(0)
                                        //
                                    {
                                        for (size_t i = 0; i < 3; ++i)
                                        {
                                            mStrides[i] = _strides[i];
                                        }
                                    }
public:  ////                       Callback (LastStrongRef@RefBase)
    virtual void                    onLastStrongRef(const void* id);

public:  ////                       Callback (Create)
    virtual MBOOL                   onCreate();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Buffer Info.
                                    struct ImgBufInfo : public BaseImageBufferHeap::BufInfo
                                    {
                                    size_t      offsetInBytes;      // (plane) offset in bytes
                                    size_t      extOffsetInBytes;   // (plane) offset in bytes for valid image region.
                                                //
                                                ImgBufInfo(
                                                    size_t _offsetInBytes = 0,
                                                    size_t _extOffsetInBytes = 0
                                                )
                                                    : BufInfo()
                                                    , offsetInBytes(_offsetInBytes)
                                                    , extOffsetInBytes(_extOffsetInBytes)
                                                {
                                                }
                                    };
    typedef Vector<sp<ImgBufInfo> >    ImgBufInfoVect_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Heap Info.
    sp<BaseImageBufferHeap>             mspImgBufHeap;
    mutable Mutex                       mLockMtx;
    MINT32 volatile                     mLockCount;
    MINT32                              mLockUsage;
    ImgBufInfoVect_t                    mvImgBufInfo;   // from image buffer.
    BaseImageBufferHeap::BufInfoVect_t  mvBufHeapInfo;  // from heap. use to lock/unlock buffer.

private:    ////                    Image Attributes.
    MSize                           mImgSize;           // in pixels
    MINT32                          mBufHeight;         // in pixels
    MINT                            mImgFormat;
    size_t                          mPlaneCount;
    size_t                          mBitstreamSize;     // in bytes
    size_t                          mStrides[3];        // in bytes
    size_t                          mOffset;            // in bytes
    MINT64                          mTimestamp;
    MINT                            mAcquireFence;
    MINT                            mReleaseFence;
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSImageBuffer
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFER_H_

