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
#define LOG_TAG "MtkCam/GrallocHeap"
//
#include "MyUtils.h"
#include <utils/include/imagebuf/BaseImageBufferHeap.h>
#include <utils/include/imagebuf/IGrallocImageBufferHeap.h>
#include <ui/gralloc_extra.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
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
 *  Image Buffer Heap (Gralloc).
 ******************************************************************************/
namespace {
class GrallocImageBufferHeap : public IGrallocImageBufferHeap
                             , protected NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class IGrallocImageBufferHeap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGrallocImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Accessors.
    virtual void*                   getGraphicBuffer()  const    { return (void*)&mspGBuffer; };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual char const*             impGetMagicName()                   const   { return magicName(); }

    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return mvHeapInfo; }

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
protected:  ////                    Buffer Info.
                                    struct MyBufInfo : public BufInfo
                                    {
                                    size_t              u4Offset;
                                    size_t              iBoundaryInBytesToAlloc;
                                                        //
                                                        MyBufInfo()
                                                            : BufInfo()
                                                            , u4Offset(0)
                                                            , iBoundaryInBytesToAlloc(0)
                                                        {
                                                        }
                                    };
    typedef Vector<sp<MyBufInfo> >  MyBufInfoVect_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual MBOOL                   doAllocGB(MINT32& heapID, size_t& stridesInBytes);
    virtual MVOID                   doDeallocGB();

    virtual MBOOL                   doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);
    virtual MBOOL                   doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);

    virtual MBOOL                   doFlushCache();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~GrallocImageBufferHeap() {}
                                    GrallocImageBufferHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam, 
                                        AllocExtraParam const& rExtraParam
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Info to Allocate.
    AllocExtraParam                 mExtraParam;
    MINT32                          mImgFormat;                     // image format.
    MSize                           mImgSize;                       // image size.
    size_t                          mBufStridesInBytesToAlloc[3];   // buffer strides in bytes.
    MINT32                          mBufBoundaryInBytesToAlloc[3];  // the address will be a multiple of boundary in bytes, which must be a power of two.

protected:  ////                    Info of Allocated Result.
    HeapInfoVect_t                  mvHeapInfo;     //
    MyBufInfoVect_t                 mvBufInfo;      //
    sp<GraphicBuffer>               mspGBuffer;

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IGrallocImageBufferHeap*
IGrallocImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam
)
{
    MUINT const planeCount = Format::queryPlaneCount(rImgParam.imgFormat);
#if 0
    for (MUINT i = 0; i < planeCount; i++)
    {
        CAM_LOGW_IF(
            0!=(rImgParam.bufBoundaryInBytes[i]%L1_CACHE_BYTES), 
            "BoundaryInBytes[%d]=%d is not a multiple of %d", 
            i, rImgParam.bufBoundaryInBytes[i], L1_CACHE_BYTES
        );
    }
#endif
    //
    GrallocImageBufferHeap* pHeap = NULL;
    pHeap = new GrallocImageBufferHeap(szCallerName, rImgParam, rExtraParam);
    if  ( ! pHeap )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize) )
    {
        CAM_LOGE("onCreate");
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
GrallocImageBufferHeap::
GrallocImageBufferHeap(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mExtraParam(rExtraParam)
    //
    , mImgFormat(rImgParam.imgFormat)
    , mImgSize(rImgParam.imgSize)
    , mvHeapInfo()
    , mvBufInfo()
    , mspGBuffer(NULL)
    //
{
    MY_LOGD("");
    ::memcpy(mBufStridesInBytesToAlloc, rImgParam.bufStridesInBytes, sizeof(mBufStridesInBytesToAlloc));
    ::memcpy(mBufBoundaryInBytesToAlloc, rImgParam.bufBoundaryInBytes, sizeof(mBufBoundaryInBytesToAlloc));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    MBOOL ret = MFALSE;
    MINT32 heapID = -1;
    MUINT32 planesSizeInBytes = 0;
    size_t stridesInBytes = mBufStridesInBytesToAlloc[0];
    //
    //  Allocate memory and setup mBufHeapInfo & rBufHeapInfo.
    if  ( ! helpCheckBufStrides(0, mBufStridesInBytesToAlloc[0]) )
    {
        goto lbExit;
    }
    if  ( ! doAllocGB(heapID, stridesInBytes) )
    {
        MY_LOGE("doAllocGB");
        goto lbExit;
    }
    else
    {   // TODO: multi-plane stride
        mBufStridesInBytesToAlloc[0] = stridesInBytes;
    }
    //
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        if  ( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) )
        {
            goto lbExit;
        }
        //
        sp<HeapInfo> pHeapInfo = new HeapInfo;
        mvHeapInfo.push_back(pHeapInfo);
        pHeapInfo->heapID = heapID;
        //
        sp<MyBufInfo> pBufInfo = new MyBufInfo;
        mvBufInfo.push_back(pBufInfo);
        pBufInfo->stridesInBytes = mBufStridesInBytesToAlloc[i];
        pBufInfo->iBoundaryInBytesToAlloc = mBufBoundaryInBytesToAlloc[i];
        pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]);
        pBufInfo->u4Offset = planesSizeInBytes;
        //
        planesSizeInBytes += pBufInfo->sizeInBytes;
        //
        rvBufInfo[i]->stridesInBytes = pBufInfo->stridesInBytes;
        rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        doDeallocGB();
        mvHeapInfo.clear();
        mvBufInfo.clear();
    }
    MY_LOGD_IF(1, "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impUninit(BufInfoVect_t const& rvBufInfo)
{
    doDeallocGB();
    mvHeapInfo.clear();
    mvBufInfo.clear();
    //
    MY_LOGD_IF(1, "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doAllocGB(MINT32& heapID, size_t& stridesInBytes)
{
    //
    status_t err = NO_ERROR;
    MINT32 width  = mImgSize.w;
    MINT32 height = mImgSize.h;
    //
    mspGBuffer = new GraphicBuffer(width, height, mImgFormat, mExtraParam.usage);
    //
    size_t stride = (size_t)mspGBuffer->getStride();
    if ( stride != (size_t)width )
    {
        size_t const planeBitsPerPixel         = getPlaneBitsPerPixel(0);
        size_t const planeBufStridesInBytes    = stride * planeBitsPerPixel >> 3;
        //
        MY_LOGD("update stride(%zu)->(%zu)", stridesInBytes, planeBufStridesInBytes);
        stridesInBytes = planeBufStridesInBytes;
    }
    //
    err = ::gralloc_extra_query(mspGBuffer->handle, GRALLOC_EXTRA_GET_ION_FD, &heapID);
    if (NO_ERROR != err)
    {
        MY_LOGE("getIonFd(): status[%d]", -err);
        goto lbExit;
    }
    //
    return  MTRUE;
lbExit:
    return  MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
GrallocImageBufferHeap::
doDeallocGB()
{
    if ( mspGBuffer != NULL )
    {
        mspGBuffer = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impLockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    MBOOL ret = MFALSE;
    //
    char* buf = NULL;
    mspGBuffer->lock(usage, (void**)(&buf));
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo   = mvHeapInfo[i];
        sp<MyBufInfo> pMyBufInfo = mvBufInfo[i];
        sp<BufInfo> pBufInfo     = rvBufInfo[i];
        //
        //  SW Access.
        pBufInfo->va = ( 0 != (usage & eBUFFER_USAGE_SW_MASK) ) ? (pMyBufInfo->u4Offset + (MINTPTR)buf) : 0;
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( ! doMapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("%s@ doMapPhyAddr at %d-th plane", szCallerName, i);
                goto lbExit;
            }
            if ( ( i > 0 ) 
                && ( mvHeapInfo[i]->heapID != -1 )  // ion fd
                && ( mvHeapInfo[i]->heapID == mvHeapInfo[i-1]->heapID ) // continuous memory
                )
            {
                pBufInfo->pa += pMyBufInfo->u4Offset;
            }
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        impUnlockBuf(szCallerName, usage, rvBufInfo);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
impUnlockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo   = mvHeapInfo[i];
        sp<BufInfo> pBufInfo     = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
                doUnmapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo);
                pBufInfo->pa = 0;
            }
            else {
                MY_LOGW("%s@ skip PA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  ( 0 != pBufInfo->va ) {
                pBufInfo->va = 0;
            }
            else {
                MY_LOGW("%s@ skip VA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
    }
    mspGBuffer->unlock();
    //
#if 0
    //  SW Write + Cacheable Memory => Flush Cache.
    if  ( 0!=(usage & eBUFFER_USAGE_SW_WRITE_MASK) && 0==mExtraParam.nocache )
    {
        doFlushCache();
    }
#endif
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = 0;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpMapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpMapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = param.phyAddr;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = rBufInfo.pa;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpUnmapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpUnmapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = 0;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GrallocImageBufferHeap::
doFlushCache()
{
#if 0
    Vector<HelperParamFlushCache> vParam;
    vParam.insertAt(0, mvHeapInfo.size());
    HelperParamFlushCache*const aParam = vParam.editArray();
    for (MUINT i = 0; i < vParam.size(); i++)
    {
        aParam[i].phyAddr = mvBufInfo[i]->pa;
        aParam[i].virAddr = mvBufInfo[i]->va;
        aParam[i].ionFd   = mvHeapInfo[i]->heapID;
        aParam[i].size    = mvBufInfo[i]->sizeInBytes;
    }
    if  ( ! helpFlushCache(aParam, vParam.size()) )
    {
        MY_LOGE("helpFlushCache");
        return  MFALSE;
    }
#endif
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/

