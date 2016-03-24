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

#define LOG_TAG "MtkCam/ImgBuf"
//
#include "MyUtils.h"
#include "BaseImageBuffer.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::NSImageBuffer;
using namespace NSCam::NSImageBufferHeap;
//
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>


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
//
#define BYTE2PIXEL(BPP, BYTE)   ((BYTE << 3) / BPP)
#define PIXEL2BYTE(BPP, PIXEL)  ((PIXEL * BPP) >> 3)


/******************************************************************************
 *
 ******************************************************************************/
BaseImageBuffer::
~BaseImageBuffer()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
BaseImageBuffer::
onLastStrongRef(const void* id)
{
    MY_LOGD_IF(mspImgBufHeap->getLogCond(), "this:%p %dx%d format:%#x", this, mImgSize.w, mImgSize.h, mImgFormat);
    //
    //
    mvImgBufInfo.clear();
    mvBufHeapInfo.clear();
    //
    if  ( 0 != mLockCount )
    {
        MY_LOGE("Not unlock before release heap - LockCount:%d", mLockCount);
        dumpCallStack(__FUNCTION__);
    }
    //
    if  ( mspImgBufHeap != 0 )
    {
        mspImgBufHeap = 0;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
onCreate()
{
    MY_LOGD_IF(mspImgBufHeap->getLogCond(), "%s@ this:%p %dx%d, offset(%zu) fmt(0x%x), plane(%zu)",
        getMagicName(), this, getImgSize().w, getImgSize().h, mOffset, getImgFormat(), getPlaneCount());
    //
    mvBufHeapInfo.clear();
    mvBufHeapInfo.setCapacity(mspImgBufHeap->getPlaneCount());
    for (size_t i = 0; i < mspImgBufHeap->getPlaneCount(); i++)
    {
        mvBufHeapInfo.push_back(new BaseImageBufferHeap::BufInfo);
        mvBufHeapInfo[i]->stridesInBytes  = mspImgBufHeap->getBufStridesInBytes(i);
        mvBufHeapInfo[i]->sizeInBytes     = mspImgBufHeap->getBufSizeInBytes(i);
        MY_LOGD_IF(mspImgBufHeap->getLogCond(), "heap stride(%zu)(bytes), sizeInBytes(%zu)",
            mvBufHeapInfo[i]->stridesInBytes, mvBufHeapInfo[i]->sizeInBytes);
    }
    //
    mvImgBufInfo.clear();
    mvImgBufInfo.setCapacity(getPlaneCount());
    for (size_t i = 0; i < getPlaneCount(); ++i)
    {
        mvImgBufInfo.push_back(new ImgBufInfo);
    }
    //
    size_t imgBufSize = 0; // buffer size of n planes.
    //
    for (size_t i = 0; i < mvImgBufInfo.size(); ++i)
    {
        // (plane) strides in bytes
        mvImgBufInfo[i]->stridesInBytes = mStrides[i];
        //
        // (plane) offset in bytes
        size_t const planeOffsetInPixels    = Format::queryPlaneWidthInPixels(getImgFormat(), i, BYTE2PIXEL(getPlaneBitsPerPixel(i), mOffset));
        mvImgBufInfo[i]->offsetInBytes      = PIXEL2BYTE(getPlaneBitsPerPixel(i), planeOffsetInPixels);  // size in bytes.
        MY_LOGW_IF(mOffset != mvImgBufInfo[i]->offsetInBytes,
                    "%s@ Bad offset at %zu-th plane: mOffset(%zu) -> (%f)(pixels) -> offsetInBytes(%zu)",
                    getMagicName(), i, mOffset, (MFLOAT)mOffset*8/getPlaneBitsPerPixel(i), mvImgBufInfo[i]->offsetInBytes);
        //
        // (plane) size in bytes
        size_t const imgWidthInPixels   = Format::queryPlaneWidthInPixels(getImgFormat(), i, (size_t)getImgSize().w);
        size_t const imgHeightInPixels  = Format::queryPlaneHeightInPixels(getImgFormat(), i, (size_t)getImgSize().h);
        size_t const planeBitsPerPixel  = getPlaneBitsPerPixel(i);
        size_t const roundUpValue       = (imgWidthInPixels*planeBitsPerPixel%8 > 0) ? 1 : 0;
        size_t const imgWidthInBytes    = (imgWidthInPixels*planeBitsPerPixel/8) + roundUpValue;
        //
        if  ( mvImgBufInfo[i]->stridesInBytes <= 0 || mvImgBufInfo[i]->stridesInBytes < imgWidthInBytes )
        {
            MY_LOGE("%s@ Bad result at %zu-th plane: bpp(%zu), width(%d pixels/%zu bytes), strides(%zu bytes)",
                getMagicName(), i, planeBitsPerPixel, imgWidthInPixels, imgWidthInBytes, mvImgBufInfo[i]->stridesInBytes);
            return  MFALSE;
        }
        switch ( getImgFormat() )
        {
            // [NOTE] create JPEG image buffer from BLOB heap.
            case eImgFmt_JPEG:
            case eImgFmt_BLOB:
                mvImgBufInfo[i]->sizeInBytes = mvImgBufInfo[i]->stridesInBytes;
            break;
            default:
                mvImgBufInfo[i]->sizeInBytes = ( 0 == mvImgBufInfo[i]->offsetInBytes )
                                            ? mvImgBufInfo[i]->stridesInBytes*imgHeightInPixels
                                            : mvImgBufInfo[i]->stridesInBytes*(imgHeightInPixels-1) + imgWidthInBytes;
            break;
        }
        imgBufSize += mvImgBufInfo[i]->sizeInBytes;
        //
        if ( eImgFmt_BLOB != mspImgBufHeap->getImgFormat() )
        {   // check  ROI(x,y) + ROI(w,h) <= heap stride(w,h)
            size_t const planeStartXInBytes    = mvImgBufInfo[i]->offsetInBytes % mspImgBufHeap->getBufStridesInBytes(i);
            size_t const planeStartYInBytes    = mvImgBufInfo[i]->offsetInBytes / mspImgBufHeap->getBufStridesInBytes(i);
            size_t const planeStartXInPixels   = BYTE2PIXEL(getPlaneBitsPerPixel(i), planeStartXInBytes);
            size_t const planeStartYInPixels   = BYTE2PIXEL(getPlaneBitsPerPixel(i), planeStartYInBytes);
            size_t const planeStridesInPixels  = BYTE2PIXEL(getPlaneBitsPerPixel(i), mspImgBufHeap->getBufStridesInBytes(i));
            size_t const planeHeightInPixels   = Format::queryPlaneHeightInPixels(getImgFormat(), i, getImgSize().h);
            MRect roi(MPoint(planeStartXInPixels, planeStartYInPixels), MSize(imgWidthInPixels, imgHeightInPixels));
            MY_LOGW_IF(
                    mspImgBufHeap->getLogCond() &&
                    mspImgBufHeap->getBufStridesInBytes(i) != (MUINT32)PIXEL2BYTE(getPlaneBitsPerPixel(i), planeStridesInPixels),
                    "%s@ Bad stride at %zu-th plane: heapStridesInBytes(%zu) -> (%f)(pixels) -> StridesInBytes(%zu)",
                    getMagicName(), i, mspImgBufHeap->getBufStridesInBytes(i),
                    (MFLOAT)mspImgBufHeap->getBufStridesInBytes(i)*8/getPlaneBitsPerPixel(i),
                    PIXEL2BYTE(getPlaneBitsPerPixel(i), planeStridesInPixels)
                    );
            if  ( (size_t)roi.leftTop().x + (size_t)roi.width() > planeStridesInPixels
               || (size_t)roi.leftTop().y + (size_t)roi.height() > planeHeightInPixels )
            {
                MY_LOGE(
                    "%s@ Bad image buffer at %zu-th plane: strides:%dx%d(pixels), roi:(%d,%d,%d,%d)",
                    getMagicName(), i, planeStridesInPixels, planeHeightInPixels,
                    roi.leftTop().x, roi.leftTop().y, roi.width(), roi.height()
                    );
                return  MFALSE;
            }
            if ( getBufOffsetInBytes(i) + getBufSizeInBytes(i) > mspImgBufHeap->getBufSizeInBytes(i) )
            {
                MY_LOGE(
                    "%s@ Bad image buffer at %zu-th plane: offset(%zu) + bufSize(%zu) > heap bufSize(%zu)",
                    getMagicName(), i, getBufOffsetInBytes(i), getBufSizeInBytes(i), mspImgBufHeap->getBufSizeInBytes(i)
                    );
                return  MFALSE;
            }
        }
        else if ( eImgFmt_BLOB == getImgFormat() || eImgFmt_JPEG == getImgFormat() )
        {   // check BLOB buffer size <= BLOB heap size
            if ( getBufOffsetInBytes(i) + getBufSizeInBytes(i) > mspImgBufHeap->getBufSizeInBytes(i) )
            {
                MY_LOGE(
                    "%s@ blob buffer offset(%zu)(bytes) + size(%zu) > blob heap buffer size(%zu)",
                    getMagicName(), getBufOffsetInBytes(i), getBufSizeInBytes(i), mspImgBufHeap->getBufSizeInBytes(i)
                    );
                return  MFALSE;
            }
        }
    }
    //
    if ( eImgFmt_BLOB == mspImgBufHeap->getImgFormat() && eImgFmt_BLOB != getImgFormat() )
    {   // create non-BLOB image buffer from BLOB heap.
        if ( imgBufSize > mspImgBufHeap->getBufSizeInBytes(0) )
        {
            for (size_t i = 0; i < getPlaneCount(); i++)
            {
                MY_LOGE("plane(%zu) bit(%zu), buf stride(%zu), bufSize(%zu)",
                        i, getPlaneBitsPerPixel(i), getBufStridesInBytes(i), getBufSizeInBytes(i));
            }
            MY_LOGE(
                "%s@ buffer size(%zu) > blob heap buffer size(%zu)",
                getMagicName(), imgBufSize, mspImgBufHeap->getBufSizeInBytes(0));
            return  MFALSE;
        }
    }
    //
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        MY_LOGD_IF(mspImgBufHeap->getLogCond(), "plane(%zu) bit(%zu), buf offset(%zu), stride(%zu), bufSize(%zu)",
                    i, getPlaneBitsPerPixel(i), getBufOffsetInBytes(i), getBufStridesInBytes(i), getBufSizeInBytes(i));
    }
    //
    return  MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
setBitstreamSize(size_t const bitstreamsize)
{
    if ( eImgFmt_JPEG != getImgFormat() )
    {
        MY_LOGE("%s@ wrong format(0x%x), can not set bitstream size", getMagicName(), getImgFormat());
        return MFALSE;
    }
    if ( bitstreamsize > mspImgBufHeap->getBufSizeInBytes(0) )
    {
        MY_LOGE("%s@ bitstream size(%zu) > heap buffer size(%zu)", getMagicName(), bitstreamsize, mspImgBufHeap->getBufSizeInBytes(0));
        return MFALSE;
    }
    //
    mBitstreamSize = bitstreamsize;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBuffer::
getPlaneBitsPerPixel(size_t index) const
{
    return  Format::queryPlaneBitsPerPixel(getImgFormat(), index);
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBuffer::
getImgBitsPerPixel() const
{
    return  Format::queryImageBitsPerPixel(getImgFormat());
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBuffer::
getBufOffsetInBytes(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return  mvImgBufInfo[index]->offsetInBytes;
}


/******************************************************************************
 * Buffer physical address; legal only after lock() with HW usage.
 ******************************************************************************/
MINTPTR
BaseImageBuffer::
getBufPA(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    MUINT32 offset = getBufOffsetInBytes(index);
    //
    Mutex::Autolock _l(mLockMtx);
    //
    if  (
        0 == mLockCount
    ||  0 == (mLockUsage & eBUFFER_USAGE_HW_MASK)
    )
    {
        MY_LOGE("This call is legal only after lockBuf() with HW usage - LockCount:%d Usage:%#x", mLockCount, mLockUsage);
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    // Buf PA(i) = Heap PA(i) + Buf Offset(i)
    return  mvImgBufInfo[index]->pa + offset;
}


/******************************************************************************
 * Buffer virtual address; legal only after lock() with SW usage.
 ******************************************************************************/
MINTPTR
BaseImageBuffer::
getBufVA(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    size_t offset = getBufOffsetInBytes(index);
    //
    Mutex::Autolock _l(mLockMtx);
    //
    if  (
        0 == mLockCount
    ||  0 == (mLockUsage & eBUFFER_USAGE_SW_MASK)
    )
    {
        MY_LOGE("This call is legal only after lockBuf() with SW usage - LockCount:%d Usage:%#x", mLockCount, mLockUsage);
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    // Buf VA(i) = Heap VA(i) + Buf Offset(i)
    return  mvImgBufInfo[index]->va + offset;
}


/******************************************************************************
 * Buffer size in bytes; always legal.
 ******************************************************************************/
size_t
BaseImageBuffer::
getBufSizeInBytes(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return  mvImgBufInfo[index]->sizeInBytes;
}


/******************************************************************************
 * Buffer Strides in bytes; always legal.
 ******************************************************************************/
size_t
BaseImageBuffer::
getBufStridesInBytes(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return  mvImgBufInfo[index]->stridesInBytes;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
setExtParam(MSize const& imgSize, size_t offsetInBytes)
{
    MBOOL ret = MFALSE;
    size_t const sizeInBytes       = getBufSizeInBytes(0) + getBufOffsetInBytes(0);
    size_t const bitsPerPixel      = getPlaneBitsPerPixel(0);
    size_t const strideInBytes     = getBufStridesInBytes(0);
    size_t const strideInPixels    = BYTE2PIXEL(bitsPerPixel, strideInBytes);
    size_t const roundUpValue      = (imgSize.w*bitsPerPixel%8 > 0) ? 1 : 0;
    size_t const imgWidthInBytes   = (imgSize.w*bitsPerPixel/8) + roundUpValue;
    size_t const imgSizeInBytes    = ( 0 == offsetInBytes )
                                    ? strideInBytes*imgSize.h
                                    : strideInBytes*(imgSize.h-1) + imgWidthInBytes;

    if ( (size_t)imgSize.w > strideInPixels || imgSize.h > mBufHeight )
    {
        MY_LOGE("invalid image size(%dx%d)>(%dx%d), strideInBytes(%zu)",
                imgSize.w, imgSize.h, strideInPixels, mBufHeight, strideInBytes);
        goto lbExit;
    }
    if ( imgSizeInBytes + offsetInBytes > sizeInBytes )
    {
        MY_LOGE("oversize S(%dx%d):(%zu) + Offset(%zu) > original size(%zu)",
                imgSize.w, imgSize.h, imgSizeInBytes, offsetInBytes, sizeInBytes);
        goto lbExit;
    }
    //
    //
    {
        Mutex::Autolock _l(mLockMtx);
        //
        if ( mImgSize != imgSize || mvImgBufInfo[0]->extOffsetInBytes != offsetInBytes )
        {
            MY_LOGD("update imgSize(%dx%d -> %dx%d), offset(%zu->%zu) @0-plane",
                mImgSize.w, mImgSize.h, imgSize.w, imgSize.h, mvImgBufInfo[0]->extOffsetInBytes, offsetInBytes);
            //
            mImgSize = imgSize;
            for (size_t i = 0; i < getPlaneCount(); i++)
            {
                mvImgBufInfo[i]->extOffsetInBytes = Format::queryPlaneWidthInPixels(getImgFormat(), i, BYTE2PIXEL(getPlaneBitsPerPixel(i), offsetInBytes));
            }
        }
    }
    ret = MTRUE;
lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBuffer::
getExtOffsetInBytes(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return mvImgBufInfo[index]->extOffsetInBytes;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
lockBuf(char const* szCallerName, MINT usage)
{
    Mutex::Autolock _l(mLockMtx);
    //
    lockBufLocked(szCallerName, usage);
    //
    if ( getPlaneCount() == mspImgBufHeap->getPlaneCount() )
    {
        for (size_t i = 0; i < mvImgBufInfo.size(); ++i)
        {
            mvImgBufInfo[i]->pa = mvBufHeapInfo[i]->pa;
            mvImgBufInfo[i]->va = mvBufHeapInfo[i]->va;
        }
    }
    else
    {   // non-BLOB image buffer created from BLOB heap.
        mvImgBufInfo[0]->pa = mvBufHeapInfo[0]->pa;
        mvImgBufInfo[0]->va = mvBufHeapInfo[0]->va;
        for (size_t i = 1; i < mvImgBufInfo.size(); ++i)
        {
            mvImgBufInfo[i]->pa = ( 0 == mvImgBufInfo[0]->pa ) ? 0 : mvImgBufInfo[i-1]->pa + mvImgBufInfo[i-1]->sizeInBytes;
            mvImgBufInfo[i]->va = ( 0 == mvImgBufInfo[0]->va ) ? 0 : mvImgBufInfo[i-1]->va + mvImgBufInfo[i-1]->sizeInBytes;
        }
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
unlockBuf(char const* szCallerName)
{
    Mutex::Autolock _l(mLockMtx);
    //
    unlockBufLocked(szCallerName);
    //
    if ( getPlaneCount() == mspImgBufHeap->getPlaneCount() )
    {
        for (size_t i = 0; i < mvImgBufInfo.size(); ++i)
        {
            mvImgBufInfo[i]->pa = mvBufHeapInfo[i]->pa;
            mvImgBufInfo[i]->va = mvBufHeapInfo[i]->va;
        }
    }
    else
    {   // non-BLOB image buffer created from BLOB heap.
        mvImgBufInfo[0]->pa = mvBufHeapInfo[0]->pa;
        mvImgBufInfo[0]->va = mvBufHeapInfo[0]->va;
        for (size_t i = 1; i < mvImgBufInfo.size(); ++i)
        {
            mvImgBufInfo[i]->pa = ( 0 == mvImgBufInfo[0]->pa ) ? 0 : mvImgBufInfo[i]->pa;
            mvImgBufInfo[i]->va = ( 0 == mvImgBufInfo[0]->va ) ? 0 : mvImgBufInfo[i]->va;
        }
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
lockBufLocked(char const* szCallerName, MINT usage)
{
    MY_LOGF_IF(0<mLockCount, "%s@ Has locked - LockCount:%d", szCallerName, mLockCount);
    //
    if  ( ! mspImgBufHeap->lockBuf(szCallerName, usage) )
    {
        MY_LOGE("%s@ impLockBuf() usage:%#x", szCallerName, usage);
        return  MFALSE;
    }
    //
    //  Check Buffer Info.
    if  ( mspImgBufHeap->getPlaneCount() != mvBufHeapInfo.size() )
    {
        MY_LOGE("%s@ BufInfo.size(%zu) != PlaneCount(%zu)", szCallerName, mvBufHeapInfo.size(), mspImgBufHeap->getPlaneCount());
        return  MFALSE;
    }
    //
    for (size_t i = 0; i < mvBufHeapInfo.size(); i++)
    {
        mvBufHeapInfo[i]->va = ( 0 != (usage & eBUFFER_USAGE_SW_MASK) ) ? mspImgBufHeap->getBufVA(i) : 0;
        mvBufHeapInfo[i]->pa = ( 0 != (usage & eBUFFER_USAGE_HW_MASK) ) ? mspImgBufHeap->getBufPA(i) : 0;
        //
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) && 0 == mvBufHeapInfo[i]->va )
        {
            MY_LOGE("%s@ Bad result at %d-th plane: va=0 with SW usage:%#x", szCallerName, i, usage);
            return  MFALSE;
        }
        //
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) && 0 == mvBufHeapInfo[i]->pa )
        {
            MY_LOGE("%s@ Bad result at %d-th plane: pa=0 with HW usage:%#x", szCallerName, i, usage);
            return  MFALSE;
        }
    }
    //
    mLockUsage = usage;
    mLockCount++;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
unlockBufLocked(char const* szCallerName)
{
    if  ( 0 == mLockCount )
    {
        MY_LOGW("%s@ Never lock", szCallerName);
        return  MFALSE;
    }
    //
    if  ( ! mspImgBufHeap->unlockBuf(szCallerName) )
    {
        MY_LOGE("%s@ impUnlockBuf() usage:#x", szCallerName, mLockUsage);
        return  MFALSE;
    }
    for (size_t i = 0; i < mvBufHeapInfo.size(); i++)
    {
        mvBufHeapInfo[i]->va = 0;
        mvBufHeapInfo[i]->pa = 0;
    }
    //
    mLockUsage = 0;
    mLockCount--;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
saveToFile(char const* filepath)
{
    MBOOL ret = MFALSE;
    int fd = -1;
    //
    if ( 0 == (mLockUsage & eBUFFER_USAGE_SW_READ_MASK) )
    {
        MY_LOGE("mLockUsage(0x%x) can not read VA", mLockUsage);
        goto lbExit;
    }
    //
    MY_LOGD("save to %s", filepath);
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if  ( fd < 0 )
    {
        MY_LOGE("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        MUINT8* pBuf = (MUINT8*)getBufVA(i);
        size_t  size = getBufSizeInBytes(i);
        size_t  written = 0;
        int nw = 0, cnt = 0;
        while ( written < size )
        {
            nw = ::write(fd, pBuf+written, size-written);
            if  (nw < 0)
            {
                MY_LOGE(
                    "fail to write %s, %d-th plane, write-count:%d, written-bytes:%d : %s",
                    filepath, i, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        MY_LOGD("[%d-th plane] write %d bytes to %s", i, size, filepath);
    }
    //
    syncCache(eCACHECTRL_INVALID);
    //
    ret = MTRUE;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBuffer::
loadFromFile(char const* filepath)
{
    MBOOL ret = MFALSE;
    MBOOL isLocked = MFALSE;
    int fd = -1;
    int filesize = 0;
    //
    isLocked = lockBuf(filepath, eBUFFER_USAGE_SW_WRITE_OFTEN);
    if  ( ! isLocked )
    {
        MY_LOGE("lockBuf fail");
        goto lbExit;
    }
    //
    MY_LOGD("load from %s", filepath);
    fd = ::open(filepath, O_RDONLY);
    if  ( fd < 0 )
    {
        MY_LOGE("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    filesize = ::lseek(fd, 0, SEEK_END);
    ::lseek(fd, 0, SEEK_SET);
    //
    for (MUINT i = 0; i < getPlaneCount(); i++)
    {
        MUINT8* pBuf = (MUINT8*)getBufVA(i);
        MUINT   bytesToRead = getBufSizeInBytes(i);
        MUINT   bytesRead = 0;
        int nr = 0, cnt = 0;
        while ( 0 < bytesToRead )
        {
            nr = ::read(fd, pBuf+bytesRead, bytesToRead-bytesRead);
            if  (nr < 0)
            {
                MY_LOGE(
                    "fail to read from %s, %d-th plane, read-count:%d, read-bytes:%d : %s",
                    filepath, i, cnt, bytesRead, ::strerror(errno)
                );
                goto lbExit;
            }
            bytesToRead -= nr;
            bytesRead += nr;
            cnt++;
        }
    }
    //
    ret = MTRUE;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    if  ( isLocked )
    {
        unlockBuf(filepath);
    }
    //
    return  ret;
}

