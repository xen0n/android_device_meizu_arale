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

#define LOG_TAG "MtkCam/BaseHeap"
//
#include "MyUtils.h"
#include <IPlatform.h>
#include <IHalMemory.h>
#include <utils/include/imagebuf/BaseImageBufferHeap.h>
#include "BaseImageBuffer.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::NSImageBufferHeap;
//
#include <linux/ion_drv.h>  // for ION_CMDS, ION_CACHE_SYNC_TYPE
#include <libion_mtk/include/ion.h>
#include <ion/ion.h>
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
 *
 ******************************************************************************/
BaseImageBufferHeap::
~BaseImageBufferHeap()
{
}


/******************************************************************************
 *
 ******************************************************************************/
BaseImageBufferHeap::
BaseImageBufferHeap(char const* szCallerName)
    : IImageBufferHeap()
    //
    , mIonDevFD(-1)
    , mBufMap()
    , mInitMtx()
    , mLockMtx()
    , mLockCount(0)
    , mLockUsage(0)
    , mvBufInfo()
    //
    , mCallerName(szCallerName)
    , mImgSize(0)
    , mImgFormat(0)
    , mPlaneCount(0)
    , mBitstreamSize(0)
    //
    , mEnableLog(MTRUE)
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
BaseImageBufferHeap::
onLastStrongRef(const void* /*id*/)
{
    MY_LOGD_IF(/*getLogCond()*/1, "[%s] this:%p %dx%d format:%#x", mCallerName.string(), this, mImgSize.w, mImgSize.h, mImgFormat);
    //
    Mutex::Autolock _l(mInitMtx);
    uninitLocked();

    if  ( 0 != mLockCount )
    {
        MY_LOGE("Not unlock before release heap - LockCount:%d", mLockCount);
        dumpCallStack(__FUNCTION__);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
onCreate(
    MSize const& imgSize,
    MINT const imgFormat,
    size_t const bitstreamSize,
    MBOOL const enableLog
)
{
    MY_LOGD_IF(/*getLogCond()*/1, "[%s] this:%p %dx%d format:%#x", mCallerName.string(), this, imgSize.w, imgSize.h, imgFormat);
    //
    if ( eImgFmt_JPEG == imgFormat )
    {
        CAM_LOGE("Cannnot create JPEG format heap");
        dumpCallStack(__FUNCTION__);
        return MFALSE;
    }
    if ( ! Format::checkValidFormat(imgFormat) )
    {
        CAM_LOGE("Unsupported Image Format!!");
        dumpCallStack(__FUNCTION__);
        return MFALSE;
    }
    if ( ! imgSize )
    {
        CAM_LOGE("Unvalid Image Size(%dx%d)", imgSize.w, imgSize.h);
        dumpCallStack(__FUNCTION__);
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mInitMtx);
    //
    mImgSize = imgSize;
    mImgFormat = imgFormat;
    mBitstreamSize = bitstreamSize;
    mPlaneCount = Format::queryPlaneCount(imgFormat);
    mEnableLog = enableLog;
    //
    return  initLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBufferHeap::
getPlaneBitsPerPixel(size_t index) const
{
    return  Format::queryPlaneBitsPerPixel(getImgFormat(), index);
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBufferHeap::
getImgBitsPerPixel() const
{
    return  Format::queryImageBitsPerPixel(getImgFormat());
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
setBitstreamSize(size_t const bitstreamsize)
{
    switch  (getImgFormat())
    {
    case eImgFmt_JPEG:
    case eImgFmt_BLOB:
        break;
    default:
        MY_LOGE("%s@ bad format:%#x", getMagicName(), getImgFormat());
        return MFALSE;
        break;
    }
    //
    if ( bitstreamsize > getBufSizeInBytes(0) ) {
        MY_LOGE("%s@ bitstreamSize:%zu > heap buffer size:%zu", getMagicName(), bitstreamsize, getBufSizeInBytes(0));
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLockMtx);
    mBitstreamSize = bitstreamsize;
    return MTRUE;
}


/******************************************************************************
 * Heap ID could be ION fd, PMEM fd, and so on.
 * Legal only after lock().
 ******************************************************************************/
MINT32
BaseImageBufferHeap::
getHeapID(size_t index) const
{
    Mutex::Autolock _l(mLockMtx);
    //
    if  ( 0 >= mLockCount )
    {
        MY_LOGE("This call is legal only after lock()");
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    HeapInfoVect_t const& rvHeapInfo = impGetHeapInfo();
    if  ( index >= rvHeapInfo.size() )
    {
        MY_LOGE("this:%p Invalid index:%zu >= %zu", this, index, rvHeapInfo.size());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  rvHeapInfo[index]->heapID;
}


/******************************************************************************
 * 0 <= Heap ID count <= plane count.
 * Legal only after lock().
 ******************************************************************************/
size_t
BaseImageBufferHeap::
getHeapIDCount() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    if  ( 0 >= mLockCount )
    {
        MY_LOGE("This call is legal only after lock()");
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  impGetHeapInfo().size();
}


/******************************************************************************
 * Buffer physical address; legal only after lock() with HW usage.
 ******************************************************************************/
MINTPTR
BaseImageBufferHeap::
getBufPA(size_t index) const
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
    return  mvBufInfo[index]->pa;
}


/******************************************************************************
 * Buffer virtual address; legal only after lock() with SW usage.
 ******************************************************************************/
MINTPTR
BaseImageBufferHeap::
getBufVA(size_t index) const
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
    return  mvBufInfo[index]->va;
}


/******************************************************************************
 * Buffer size in bytes; always legal.
 ******************************************************************************/
size_t
BaseImageBufferHeap::
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
    return  mvBufInfo[index]->sizeInBytes;
}


/******************************************************************************
 * Buffer Strides in bytes; always legal.
 ******************************************************************************/
size_t
BaseImageBufferHeap::
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
    return  mvBufInfo[index]->stridesInBytes;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
lockBuf(char const* szCallerName, MINT usage)
{
    Mutex::Autolock _l(mLockMtx);
    return  lockBufLocked(szCallerName, usage);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
unlockBuf(char const* szCallerName)
{
    Mutex::Autolock _l(mLockMtx);
    return  unlockBufLocked(szCallerName);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
lockBufLocked(char const* szCallerName, MINT usage)
{
    if ( 0 < mLockCount )
    {
        MINT const readUsage = eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE;
        if ( !(usage & ~readUsage)
            && mLockUsage == usage )
        {
            mLockCount++;
            return  MTRUE;
        }
        else
        {
            MY_LOGE("%s@ count:%d, usage:%#x, can't lock with usage:%#x", szCallerName, mLockCount, mLockUsage, usage);
            return  MFALSE;
        }
    }
    //
    if  ( ! impLockBuf(szCallerName, usage, mvBufInfo) )
    {
        MY_LOGE("%s@ impLockBuf() usage:%#x", szCallerName, usage);
        return  MFALSE;
    }
    //
    //  Check Buffer Info.
    if  ( getPlaneCount() != mvBufInfo.size() )
    {
        MY_LOGE("%s@ BufInfo.size(%zu) != PlaneCount(%zu)", szCallerName, mvBufInfo.size(), getPlaneCount());
        return  MFALSE;
    }
    //
    for (size_t i = 0; i < mvBufInfo.size(); i++)
    {
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) && 0 == mvBufInfo[i]->va )
        {
            MY_LOGE("%s@ Bad result at %zu-th plane: va=0 with SW usage:%#x", szCallerName, i, usage);
            return  MFALSE;
        }
        //
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) && 0 == mvBufInfo[i]->pa )
        {
            MY_LOGE("%s@ Bad result at %zu-th plane: pa=0 with HW usage:%#x", szCallerName, i, usage);
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
BaseImageBufferHeap::
unlockBufLocked(char const* szCallerName)
{
    if  ( 1 < mLockCount )
    {
        mLockCount--;
        MY_LOGD("%s@ still locked (%d)", szCallerName, mLockCount);
        return  MTRUE;
    }
    //
    if  ( 0 == mLockCount )
    {
        MY_LOGW("%s@ Never lock", szCallerName);
        return  MFALSE;
    }
    //
    if  ( ! impUnlockBuf(szCallerName, mLockUsage, mvBufInfo) )
    {
        MY_LOGE("%s@ impUnlockBuf() usage:%#x", szCallerName, mLockUsage);
        return  MFALSE;
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
BaseImageBufferHeap::
initLocked()
{
    MBOOL ret = MFALSE;
    //
    mIonDevFD = ::mt_ion_open("HalMemoryAdapter");
    if  ( 0 > mIonDevFD )
    {
        MY_LOGE("mt_ion_open() return %d", mIonDevFD);
        goto lbExit;
    }
    mBufMap.clear();
    //
    mvBufInfo.clear();
    mvBufInfo.setCapacity(getPlaneCount());
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        mvBufInfo.push_back(new BufInfo);
    }
    //
    if  ( ! impInit(mvBufInfo) )
    {
        MY_LOGE("%s@ impInit()", getMagicName());
        goto lbExit;
    }
    //
    for (size_t i = 0; i < mvBufInfo.size(); i++)
    {
        if  ( mvBufInfo[i]->stridesInBytes <= 0)
        {
            MY_LOGE("%s@ Bad result at %zu-th plane: strides:%zu", getMagicName(), i, mvBufInfo[i]->stridesInBytes);
            goto lbExit;
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret ) {
        uninitLocked();
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
uninitLocked()
{
    if  ( ! impUninit(mvBufInfo) )
    {
        MY_LOGE("%s@ impUninit()", getMagicName());
    }
    mvBufInfo.clear();
    //
    if  ( 0 <= mIonDevFD )
    {
        ::ion_close(mIonDevFD);
        mIonDevFD = -1;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
syncCache(eCacheCtrl const ctrl)
{
    //
    HeapInfoVect_t const& rvHeapInfo = impGetHeapInfo();
    size_t const num = getPlaneCount();
    Vector<HelperParamFlushCache> vInfo;
    vInfo.insertAt(0, num);
    HelperParamFlushCache*const aInfo = vInfo.editArray();
    for (size_t i = 0; i < num; i++)
    {
        aInfo[i].phyAddr    = mvBufInfo[i]->pa;
        aInfo[i].virAddr    = mvBufInfo[i]->va;
        aInfo[i].ionFd      = (rvHeapInfo.size() > 1) ? rvHeapInfo[i]->heapID : rvHeapInfo[0]->heapID;
        aInfo[i].size       = mvBufInfo[i]->sizeInBytes;
    }
    //
    if  ( ! helpFlushCache(ctrl, aInfo, num) )
    {
        MY_LOGE("IHalMemory::syncCache");
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
BaseImageBufferHeap::
createImageBuffer()
{
    size_t bufStridesInBytes[3] = {0, 0, 0};
    for (size_t i = 0; i < getPlaneCount(); ++i)
    {
        bufStridesInBytes[i] = getBufStridesInBytes(i);
    }
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, getImgSize(), getImgFormat(), getBitstreamSize(), bufStridesInBytes);
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
BaseImageBufferHeap::
createImageBuffer_FromBlobHeap(
    size_t      offsetInBytes,
    size_t      sizeInBytes
)
{
    if ( getImgFormat() != eImgFmt_BLOB )
    {
        CAM_LOGE("Heap format(0x%x) is illegal.", getImgFormat());
        return NULL;
    }
    //
    MSize imgSize(sizeInBytes, getImgSize().h);
    size_t bufStridesInBytes[] = { sizeInBytes, 0, 0 };
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, imgSize, getImgFormat(), getBitstreamSize(), bufStridesInBytes, offsetInBytes);
    //
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
BaseImageBufferHeap::
createImageBuffer_FromBlobHeap(
    size_t      offsetInBytes,
    MINT32      imgFormat,
    MSize const&imgSize,
    size_t const bufStridesInBytes[3]
)
{
    if ( getImgFormat() != eImgFmt_BLOB )
    {
        CAM_LOGE("Heap format(0x%x) is illegal.", getImgFormat());
        return NULL;
    }
    //
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, imgSize, imgFormat, getBitstreamSize(), bufStridesInBytes, offsetInBytes);
    //
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
BaseImageBufferHeap::
createImageBuffer_SideBySide(MBOOL isRightSide)
{
    size_t imgWidth = getImgSize().w >> 1;
    size_t imgHeight= getImgSize().h;
    size_t offset   = (isRightSide) ? (imgWidth*getPlaneBitsPerPixel(0))>>3 : 0;
    MSize SBSImgSize(imgWidth, imgHeight);
    size_t bufStridesInBytes[3] = {0, 0, 0};
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        bufStridesInBytes[i] = ( eImgFmt_BLOB == getImgFormat() ) ? getBufStridesInBytes(i)>>1 : getBufStridesInBytes(i);
    }
    //
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, SBSImgSize, getImgFormat(), getBitstreamSize(), bufStridesInBytes, offset);
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpMapPhyAddr(char const* szCallerName, HelperParamMapPA& rParam)
{
    MINT32 const memID      = rParam.ionFd;
    if ( 0 > mBufMap.indexOfKey(memID) )
    {
        if( 0 > memID)
        {
            MY_LOGE("error memID(%d)", memID);
        }
        else
        {
            ion_user_handle_t pIonHandle;
            struct ion_sys_data sys_data;
            struct ion_mm_data mm_data;
            MINT32 err = 0;
            //
            //a. get handle from IonBufFd and increase handle ref count
            if(ion_import(mIonDevFD, memID, &pIonHandle))
            {
                MY_LOGE("ion_import fail, memId(0x%x)", memID);
            }
            //b. config buffer
            mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
            mm_data.config_buffer_param.handle      = pIonHandle;
            mm_data.config_buffer_param.eModuleID   = 1;
            mm_data.config_buffer_param.security    = rParam.security;
            mm_data.config_buffer_param.coherent    = rParam.coherence;
            err = ion_custom_ioctl(mIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
            if(err == (-ION_ERROR_CONFIG_LOCKED))
            {
                MY_LOGE("ion_custom_ioctl Double config after map phy address");
            }
            else if(err != 0)
            {
                MY_LOGE("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
            }
            //c. map physical address
            sys_data.sys_cmd = ION_SYS_GET_PHYS;
            sys_data.get_phys_param.handle = pIonHandle;
            if(ion_custom_ioctl(mIonDevFD, ION_CMD_SYSTEM, &sys_data))
            {
                MY_LOGE("ion_custom_ioctl get_phys_param failed!");
            }
            rParam.phyAddr  = (MINTPTR)(sys_data.get_phys_param.phy_addr);
            mBufMap.add(memID, rParam.phyAddr);
            //d. decrease handle ref count 
            if(ion_free(mIonDevFD, pIonHandle))
            {
                MY_LOGE("ion_free fail");
            }
        }
        MY_LOGI("[%s] mID(0x%x),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d)", szCallerName,
            rParam.ionFd, rParam.size, rParam.virAddr, rParam.phyAddr, rParam.security, rParam.coherence);
    }
    else
    {
        rParam.phyAddr = mBufMap.valueFor(memID);
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpUnmapPhyAddr(char const* szCallerName, HelperParamMapPA const& rParam)
{
    if ( 0 <= mBufMap.indexOfKey(rParam.ionFd) )
    {
        mBufMap.removeItem(rParam.ionFd);
        MY_LOGI("[%s] mID(0x%x),size(0x%x),VA(x%x),PA(0x%x),S/C(%d/%d)", szCallerName,
            rParam.ionFd, rParam.size, rParam.virAddr, rParam.phyAddr, rParam.security, rParam.coherence);
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpFlushCache(
    eCacheCtrl const ctrl,
    HelperParamFlushCache const* paParam,
    size_t const num
)
{
    MY_LOGA_IF(NULL==paParam||0==num, "Bad arguments: %p %zu", paParam, num);
    //
    ION_CACHE_SYNC_TYPE CACHECTRL = ION_CACHE_FLUSH_BY_RANGE;
    switch (ctrl)
    {
        case eCACHECTRL_FLUSH:
            CACHECTRL = ION_CACHE_FLUSH_BY_RANGE;
            break;
        case eCACHECTRL_INVALID:
            CACHECTRL = ION_CACHE_FLUSH_BY_RANGE;
            break;
        default:
            MY_LOGE("ERR cmd(%d)", ctrl);
            break;
    }
    //
    for (size_t i = 0; i < num; i++)
    {
        MY_LOGI("ctrl(%d), num[%d]: mID(0x%x),size(0x%x),VA(x%x),PA(0x%x)", ctrl, i,
            paParam[i].ionFd, paParam[i].size, paParam[i].virAddr, paParam[i].phyAddr);
        MINTPTR const pa    = paParam[i].phyAddr;
        MINTPTR const va    = paParam[i].virAddr;
        MINT32 const memID  = paParam[i].ionFd;
        size_t const size   = paParam[i].size;
        //
        if( 0 > memID)
        {
            MY_LOGE("error memID(%d)", memID);
        }
        else
        {
            //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
            ion_user_handle_t pIonHandle;
            if(ion_import(mIonDevFD, memID, &pIonHandle))
            {
                MY_LOGE("ion_import fail, memId(0x%x)", memID);
            }
            //b. cache sync by range 
            struct ion_sys_data sys_data;
            sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
            sys_data.cache_sync_param.handle = pIonHandle;
            sys_data.cache_sync_param.sync_type = CACHECTRL;
            if(ion_custom_ioctl(mIonDevFD, ION_CMD_SYSTEM, &sys_data))
            {
                MY_LOGE("ION_SYS_CACHE_SYNC fail, memID(0x%x)", memID);
            }
            //c. decrease handle ref count
            if(ion_free(mIonDevFD, pIonHandle))
            {
                MY_LOGE("ion_free fail");
            }
        }
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpCheckBufStrides(
    size_t const planeIndex,
    size_t const planeBufStridesInBytes
) const
{
    size_t const planeImgWidthInPixels     = Format::queryPlaneWidthInPixels(getImgFormat(), planeIndex, getImgSize().w);
    size_t const planeBitsPerPixel         = getPlaneBitsPerPixel(planeIndex);
    size_t const roundUpValue              = ((planeBufStridesInBytes<<3) % planeBitsPerPixel > 0 ) ? 1 : 0;
    size_t const planeBufStridesInPixels   = (planeBufStridesInBytes<<3) / planeBitsPerPixel + roundUpValue;
    //
    if  ( planeBufStridesInPixels < planeImgWidthInPixels )
    {
        MY_LOGE(
            "[%dx%d image @ %zu-th plane] Bad width stride in pixels: given buffer stride:%zu < image stride:%zu. stride in bytes(%zu) bpp(%zu)",
            getImgSize().w, getImgSize().h, planeIndex,
            planeBufStridesInPixels, planeImgWidthInPixels,
            planeBufStridesInBytes, planeBitsPerPixel
        );
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
BaseImageBufferHeap::
helpQueryBufSizeInBytes(
    size_t const planeIndex,
    size_t const planeStridesInBytes
) const
{
    MY_LOGF_IF(planeIndex >= getPlaneCount(), "Bad index:%zu >= PlaneCount:%zu", planeIndex, getPlaneCount());
    //
    size_t const planeImgHeight = Format::queryPlaneHeightInPixels(getImgFormat(), planeIndex, getImgSize().h);
    return  planeStridesInBytes*planeImgHeight;
}

