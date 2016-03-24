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
#define LOG_TAG "MtkCam/DummyHeap"
//
#include "MyUtils.h"
#include <utils/imagebuf/BaseImageBufferHeap.h>
#include <utils/imagebuf/IDummyImageBufferHeap.h>
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
 *  Image Buffer Heap (Dummy).
 ******************************************************************************/
namespace {
class DummyImageBufferHeap : public IDummyImageBufferHeap
                             , protected NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class IDummyImageBufferHeap;

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
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~DummyImageBufferHeap() {}
                                    DummyImageBufferHeap(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam,
                                        PortBufInfo_dummy const& rPortBufInfo
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Info to Allocate.
    size_t                          mBufStridesInBytesToAlloc[3];   // buffer strides in bytes.

protected:  ////                    Info of Allocated Result.
    PortBufInfo_dummy               mPortBufInfo;   //
    HeapInfoVect_t                  mvHeapInfo;     //
    BufInfoVect_t                   mvBufInfo;      //

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
#define GET_BUF_VA(plane, va, index)                   (plane >= (index+1)) ? va : 0
#define GET_BUF_ID(plane, memID, index)                (plane >= (index+1)) ? memID : 0

/******************************************************************************
 *
 ******************************************************************************/
IDummyImageBufferHeap*
IDummyImageBufferHeap::
create(
    char const* szCallerName,
    ImgParam_t const& rImgParam,
    PortBufInfo_dummy const& rPortBufInfo,
    MBOOL const enableLog
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
    DummyImageBufferHeap* pHeap = NULL;
    pHeap = new DummyImageBufferHeap(szCallerName, rImgParam, rPortBufInfo);
    if  ( ! pHeap )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize, enableLog) )
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
DummyImageBufferHeap::
DummyImageBufferHeap(
    char const* szCallerName,
    ImgParam_t const& rImgParam,
    PortBufInfo_dummy const& rPortBufInfo
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mPortBufInfo(rPortBufInfo)
    , mvHeapInfo()
    , mvBufInfo()
    //
{
    ::memcpy(mBufStridesInBytesToAlloc, rImgParam.bufStridesInBytes, sizeof(mBufStridesInBytesToAlloc));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DummyImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    MBOOL ret = MFALSE;
    MUINT32 planesSizeInBytes = 0;    // for calculating n-plane va
    //
    MY_LOGD_IF(getLogCond(), "plane(%d), memID(0x%x/0x%x/0x%x), va(0x%x/0x%x/0x%x)",
            getPlaneCount(), 
            GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[0], 0),
            GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[1], 1),
            GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[2], 2),
            GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[0], 0),
            GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[1], 1),
            GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[2], 2)
            );
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        if  ( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) )
        {
            goto lbExit;
        }
        //
        {
            sp<HeapInfo> pHeapInfo = new HeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            pHeapInfo->heapID = mPortBufInfo.memID[i];
            //
            sp<BufInfo> pBufInfo = new BufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes = mBufStridesInBytesToAlloc[i];
            pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]);
            pBufInfo->va = mPortBufInfo.virtAddr[i];
            pBufInfo->pa = mPortBufInfo.phyAddr[i];
            //
            planesSizeInBytes += pBufInfo->sizeInBytes;
            //
            rvBufInfo[i]->stridesInBytes = pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;
        }
    }
    //
    ret = MTRUE;
lbExit:
    //MY_LOGD_IF(getLogCond(), "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DummyImageBufferHeap::
impUninit(BufInfoVect_t const& rvBufInfo)
{
    //
    //MY_LOGD_IF(getLogCond(), "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DummyImageBufferHeap::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    MUINT32 planesSizeInBytes = 0;    // for calculating n-plane pa
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  SW Access.
        pBufInfo->va = ( 0 != (usage & eBUFFER_USAGE_SW_MASK) ) ? mvBufInfo[i]->va : 0;
        //
        //  HW Access.
        pBufInfo->pa = ( 0 != (usage & eBUFFER_USAGE_HW_MASK) ) ? mvBufInfo[i]->pa : 0;
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DummyImageBufferHeap::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
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
    }
    //
    return  MTRUE;
}

