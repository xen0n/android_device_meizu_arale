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

#define LOG_TAG "MtkCam/camtest"
//
#include <utils/StrongPointer.h>
using namespace android;
//
//
#include <utils/Format.h>
using namespace NSCam::Utils;
//
#include "CamLog.h"
#include "TestUtils.h"
//
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  test heap with a blob format
 ******************************************************************************/
bool
test_heap_blob(
    char const*const szHeapMagicName,
    sp<IImageBufferHeap>& pHeap,
    MUINT32 size
)
{
    CAM_LOGD("++++++++++++++++ test_heap_blob");
    //
    MY_CHECK(0==::strcmp(pHeap->getMagicName(), szHeapMagicName), "%s", pHeap->getMagicName());
    MY_CHECK(1==pHeap->getStrongCount(), "StrongCount:%d", pHeap->getStrongCount());
    MY_CHECK(eImgFmt_BLOB==pHeap->getImgFormat(), "Format:%#x", pHeap->getImgFormat());
    MY_CHECK(MSize(size, 1)==pHeap->getImgSize(), "Size:%dx%d", pHeap->getImgSize().w, pHeap->getImgSize().h);
    MY_CHECK(1==pHeap->getPlaneCount(), "PlaneCount:%d", pHeap->getPlaneCount());
    MY_CHECK(8==pHeap->getImgBitsPerPixel(), "BitsPerPixel:%d", pHeap->getImgBitsPerPixel());
    MY_CHECK(8==pHeap->getPlaneBitsPerPixel(0), "PlaneBitsPerPixel(0):%d", pHeap->getPlaneBitsPerPixel(0));
    MY_CHECK(size==pHeap->getBufSizeInBytes(0), "BufSizeInBytes(0):%d", pHeap->getBufSizeInBytes(0));
    MY_CHECK(size==pHeap->getBufStridesInBytes(0), "BufStridesInBytes(0):%d", pHeap->getBufStridesInBytes(0));
    //
    MY_CHECK(pHeap->syncCache(eCACHECTRL_FLUSH), "syncCache()");
    //
    int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0==pHeap->getBufPA(0), "BufPA(0)=%p", pHeap->getBufPA(0));
        MY_CHECK(0!=pHeap->getBufVA(0), "BufVA(0)=%p", pHeap->getBufVA(0));
        if  ( pHeap->getBufVA(0) )
        {
            ::memset((MUINT8*)pHeap->getBufVA(0), 0xAA, pHeap->getBufSizeInBytes(0));
        }
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0!=pHeap->getBufPA(0), "BufPA(0)=%p", pHeap->getBufPA(0));
        MY_CHECK(0!=pHeap->getBufVA(0), "BufVA(0)=%p", pHeap->getBufVA(0));
        if  ( pHeap->getBufVA(0) )
        {
            MUINT8* pb = (MUINT8*)pHeap->getBufVA(0);
            for (MUINT32 i = 0; i < pHeap->getBufSizeInBytes(0); i++)
            {
                if  ( 0xAA != pb[i] )
                {
                    CAM_LOGE("buf[%d]:%#x != 0xAA", i, pb[i]);
                    break;
                }
            }
        }
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    usage = 0;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0<pHeap->getHeapIDCount(), "HeapIDCount:%d", pHeap->getHeapIDCount());
        for (unsigned int i = 0; i < pHeap->getHeapIDCount(); i++)
        {
            MY_CHECK(0<=pHeap->getHeapID(i), "HeapID(%d):%d", i, pHeap->getHeapID(i));
        }
        //
        MY_CHECK(0==pHeap->getBufPA(0), "BufPA(0)=%p", pHeap->getBufPA(0));
        MY_CHECK(0==pHeap->getBufVA(0), "BufVA(0)=%p", pHeap->getBufVA(0));
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    //
    CAM_LOGD("---------------- test_heap_blob");
    return  true;
}

/******************************************************************************
 *  test image buffer with a blob format
 ******************************************************************************/
bool
test_imgbuf_blob(
    char const*const szHeapMagicName,
    IImageBuffer* pImgBuf,
    MUINT32 size,
    bool const isSBS,
    bool const isROI
)
{
    CAM_LOGD("++++++++++++++++ test_imgbuf_blob");
    //
    MY_CHECK(0==::strcmp(pImgBuf->getMagicName(), szHeapMagicName), "%s", pImgBuf->getMagicName());
    MY_CHECK(1==pImgBuf->getStrongCount(), "StrongCount:%d", pImgBuf->getStrongCount());
    MY_CHECK(eImgFmt_BLOB==pImgBuf->getImgFormat(), "Format:%#x", pImgBuf->getImgFormat());
    MY_CHECK(1==pImgBuf->getPlaneCount(), "PlaneCount:%d", pImgBuf->getPlaneCount());
    MY_CHECK(8==pImgBuf->getImgBitsPerPixel(), "BitsPerPixel:%d", pImgBuf->getImgBitsPerPixel());
    MY_CHECK(8==pImgBuf->getPlaneBitsPerPixel(0), "PlaneBitsPerPixel(0):%d", pImgBuf->getPlaneBitsPerPixel(0));
    if ( !isSBS && !isROI )
    {
        MY_CHECK(size==pImgBuf->getBufSizeInBytes(0), "BufSizeInBytes(0):%d", pImgBuf->getBufSizeInBytes(0));
        MY_CHECK(MSize(size, 1)==pImgBuf->getImgSize(), "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
        MY_CHECK(size==pImgBuf->getBufStridesInBytes(0), "BufStridesInBytes(0):%d", pImgBuf->getBufStridesInBytes(0));
    }
    else if ( isSBS )
    {
        MY_CHECK(size>>1==pImgBuf->getBufSizeInBytes(0), "BufSizeInBytes(0):%d", pImgBuf->getBufSizeInBytes(0));
        MY_CHECK(MSize(size>>1, 1)==pImgBuf->getImgSize(), "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
        MY_CHECK(size>>1==pImgBuf->getBufStridesInBytes(0), "BufStridesInBytes(0):%d", pImgBuf->getBufStridesInBytes(0));
    }
    //
    MY_CHECK(pImgBuf->syncCache(eCACHECTRL_FLUSH), "syncCache()");
    //
    int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0==pImgBuf->getBufPA(0), "BufPA(0)=%p", pImgBuf->getBufPA(0));
        MY_CHECK(0!=pImgBuf->getBufVA(0), "BufVA(0)=%p", pImgBuf->getBufVA(0));
        if  ( pImgBuf->getBufVA(0) )
        {
            ::memset((MUINT8*)pImgBuf->getBufVA(0), 0xAA, pImgBuf->getBufSizeInBytes(0));
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0!=pImgBuf->getBufPA(0), "BufPA(0)=%p", pImgBuf->getBufPA(0));
        MY_CHECK(0!=pImgBuf->getBufVA(0), "BufVA(0)=%p", pImgBuf->getBufVA(0));
        if  ( pImgBuf->getBufVA(0) )
        {
            MUINT8* pb = (MUINT8*)pImgBuf->getBufVA(0);
            for (MUINT32 i = 0; i < pImgBuf->getBufSizeInBytes(0); i++)
            {
                if  ( 0xAA != pb[i] )
                {
                    CAM_LOGE("buf[%d]:%#x != 0xAA", i, pb[i]);
                    break;
                }
            }
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = 0;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0==pImgBuf->getBufPA(0), "BufPA(0)=%p", pImgBuf->getBufPA(0));
        MY_CHECK(0==pImgBuf->getBufVA(0), "BufVA(0)=%p", pImgBuf->getBufVA(0));
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    //
    CAM_LOGD("---------------- test_imgbuf_blob");
    return  true;
}


/******************************************************************************
 *  test heap with a format of yuv420 3p
 ******************************************************************************/
bool
test_heap_yuv420_3p(
    char const*const szHeapMagicName,
    sp<IImageBufferHeap>& pHeap,
    IImageBufferAllocator::ImgParam const& imgParam,
    bool const isGB
)
{
    CAM_LOGD("++++++++++++++++ test_heap_yuv420_3p");
    //
    MY_CHECK(0==::strcmp(pHeap->getMagicName(), szHeapMagicName), "%s", pHeap->getMagicName());
    MY_CHECK(1==pHeap->getStrongCount(), "StrongCount:%d", pHeap->getStrongCount());
    MY_CHECK(imgParam.imgFormat==pHeap->getImgFormat(), "Format:%#x", pHeap->getImgFormat());
    MY_CHECK(imgParam.imgSize==pHeap->getImgSize(), "Size:%dx%d", pHeap->getImgSize().w, pHeap->getImgSize().h);
    MY_CHECK(3==pHeap->getPlaneCount(), "PlaneCount:%d", pHeap->getPlaneCount());
    MY_CHECK(12==pHeap->getImgBitsPerPixel(), "BitsPerPixel:%d", pHeap->getImgBitsPerPixel());
    //
    MY_CHECK(8==pHeap->getPlaneBitsPerPixel(0), "PlaneBitsPerPixel(0):%d", pHeap->getPlaneBitsPerPixel(0));
    MY_CHECK(8==pHeap->getPlaneBitsPerPixel(1), "PlaneBitsPerPixel(1):%d", pHeap->getPlaneBitsPerPixel(1));
    MY_CHECK(8==pHeap->getPlaneBitsPerPixel(2), "PlaneBitsPerPixel(2):%d", pHeap->getPlaneBitsPerPixel(2));
    //
    for (unsigned int i = 0; i < pHeap->getPlaneCount(); i++)
    {
        MUINT32 const imgHeightInPixels = Format::queryPlaneHeightInPixels(pHeap->getImgFormat(), i, (size_t)pHeap->getImgSize().h);
        MUINT32 const bufSizeInBytes = imgParam.bufStridesInBytes[i]*imgHeightInPixels;
        //
        if ( !isGB )
        {
            MY_CHECK(bufSizeInBytes==pHeap->getBufSizeInBytes(i), "BufSizeInBytes(%d):%d, %dx%d",
                i, pHeap->getBufSizeInBytes(i), imgParam.bufStridesInBytes[i], imgHeightInPixels);
            MY_CHECK(imgParam.bufStridesInBytes[i]==pHeap->getBufStridesInBytes(i), "BufStridesInBytes(%d):%d", i, pHeap->getBufStridesInBytes(i));
        }
    }
    //
    MY_CHECK(pHeap->syncCache(eCACHECTRL_FLUSH), "syncCache()");
    //
    int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pHeap->getPlaneCount(); i++)
        {
            MY_CHECK(0==pHeap->getBufPA(i), "BufPA(%d)=%p", i, pHeap->getBufPA(i));
            MY_CHECK(0!=pHeap->getBufVA(i), "BufVA(%d)=%p", i, pHeap->getBufVA(i));
            if  ( pHeap->getBufVA(i) )
            {
                ::memset((MUINT8*)pHeap->getBufVA(i), 0xAA, pHeap->getBufSizeInBytes(i));
            }
        }
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
    MY_CHECK(pHeap->lockBuf("Test_lockTwice", usage), "lockBuf, usage:%#x", usage);
    MY_CHECK(pHeap->unlockBuf("Test_lockTwice"), "unlockBuf");
    usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(!pHeap->lockBuf("Test_lockFail", usage), "should lock fail, usage:%#x", usage);
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pHeap->getPlaneCount(); i++)
        {
            MY_CHECK(0!=pHeap->getBufPA(i), "BufPA(%d)=%p", i, pHeap->getBufPA(i));
            MY_CHECK(0!=pHeap->getBufVA(i), "BufVA(%d)=%p", i, pHeap->getBufVA(i));
            if  ( pHeap->getBufVA(i) )
            {
                MUINT8* pb = (MUINT8*)pHeap->getBufVA(i);
                for (MUINT32 c = 0; c < pHeap->getBufSizeInBytes(i); c++)
                {
                    if  ( 0xAA != pb[c] )
                    {
                        CAM_LOGE("buf[%d]:%#x != 0xAA", c, pb[c]);
                        break;
                    }
                }
            }
        }
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    usage = 0;
    MY_CHECK(pHeap->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        MY_CHECK(0<pHeap->getHeapIDCount(), "HeapIDCount:%d", pHeap->getHeapIDCount());
        for (unsigned int i = 0; i < pHeap->getHeapIDCount(); i++)
        {
            MY_CHECK(0<=pHeap->getHeapID(i), "HeapID(%d):%d", i, pHeap->getHeapID(i));
        }
        //
        for (unsigned int i = 0; i < pHeap->getPlaneCount(); i++)
        {
            MY_CHECK(0==pHeap->getBufPA(i), "BufPA(%d)=%p", i, pHeap->getBufPA(i));
            MY_CHECK(0==pHeap->getBufVA(i), "BufVA(%d)=%p", i, pHeap->getBufVA(i));
        }
    MY_CHECK(pHeap->unlockBuf("Test"), "unlockBuf");
    //
    CAM_LOGD("---------------- test_heap_yuv420_3p");
    return  true;
}


/******************************************************************************
 *  test image buffer with a format of yuv420 3p
 ******************************************************************************/
bool
test_imgbuf_yuv420_3p(
    char const*const szHeapMagicName,
    IImageBuffer* pImgBuf,
    IImageBufferAllocator::ImgParam const& imgParam,
    bool const isSBS,
    bool const isROI
)
{
    CAM_LOGD("++++++++++++++++ test_imgbuf_yuv420_3p");
    //
    MY_CHECK(0==::strcmp(pImgBuf->getMagicName(), szHeapMagicName), "%s", pImgBuf->getMagicName());
    MY_CHECK(1==pImgBuf->getStrongCount(), "StrongCount:%d", pImgBuf->getStrongCount());
    MY_CHECK(imgParam.imgFormat==pImgBuf->getImgFormat(), "Format:%#x", pImgBuf->getImgFormat());
    if ( !isSBS && !isROI )
    {
        MY_CHECK(imgParam.imgSize==pImgBuf->getImgSize(), "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
    }
    else if ( isSBS )
    {
        MY_CHECK(imgParam.imgSize.w==pImgBuf->getImgSize().w << 1, "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
        MY_CHECK(imgParam.imgSize.h==pImgBuf->getImgSize().h, "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
    }
    MY_CHECK(3==pImgBuf->getPlaneCount(), "PlaneCount:%d", pImgBuf->getPlaneCount());
    MY_CHECK(12==pImgBuf->getImgBitsPerPixel(), "BitsPerPixel:%d", pImgBuf->getImgBitsPerPixel());
    //
    MY_CHECK(8==pImgBuf->getPlaneBitsPerPixel(0), "PlaneBitsPerPixel(0):%d", pImgBuf->getPlaneBitsPerPixel(0));
    MY_CHECK(8==pImgBuf->getPlaneBitsPerPixel(1), "PlaneBitsPerPixel(1):%d", pImgBuf->getPlaneBitsPerPixel(1));
    MY_CHECK(8==pImgBuf->getPlaneBitsPerPixel(2), "PlaneBitsPerPixel(2):%d", pImgBuf->getPlaneBitsPerPixel(2));
    //
    CAM_LOGD("getImgSize(%dx%d), getTimestamp(%llu)",
        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, pImgBuf->getTimestamp());

    for (unsigned int plane = 0; plane < pImgBuf->getPlaneCount(); ++plane)
    {
        CAM_LOGD("[plane %d] BitsPerPixel(%d), BufOffsetInBytes(%d), BufSizeInBytes(%d), BufStridesInBytes(%d)",
            plane, pImgBuf->getPlaneBitsPerPixel(plane),
            pImgBuf->getBufOffsetInBytes(plane),
            pImgBuf->getBufSizeInBytes(plane),
            pImgBuf->getBufStridesInBytes(plane)
        );
    }
    //
    MY_CHECK(pImgBuf->syncCache(eCACHECTRL_FLUSH), "syncCache()");
    //
    int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0==pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0!=pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
            if  ( pImgBuf->getBufVA(i) )
            {
                ::memset((MUINT8*)pImgBuf->getBufVA(i), 0xAA, pImgBuf->getBufSizeInBytes(i));
            }
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0!=pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0!=pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
            if  ( pImgBuf->getBufVA(i) )
            {
                MUINT8* pb = (MUINT8*)pImgBuf->getBufVA(i);
                for (MUINT32 c = 0; c < pImgBuf->getBufSizeInBytes(i); c++)
                {
                    if  ( 0xAA != pb[c] )
                    {
                        CAM_LOGE("buf[%d]:%#x != 0xAA", c, pb[c]);
                        break;
                    }
                }
            }
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = 0;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        //
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0==pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0==pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
        }
    CAM_LOGD("   Test: FDCount(%d)", pImgBuf->getFDCount());
    for (unsigned int i = 0; i < pImgBuf->getFDCount(); ++i)
        CAM_LOGD("   Test: [%d] FD(%d)", i, pImgBuf->getFD(i));
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    MY_CHECK(0==pImgBuf->getFDCount(), "getFDCount is legal only after lockBuf()");
    //
    //
    CAM_LOGD("---------------- test_imgbuf_yuv420_3p");
    return  true;
}


/******************************************************************************
 *  test image buffer with a format of yuv420 3p
 ******************************************************************************/
bool
test_imgbuf_jpeg_1p(
    char const*const szHeapMagicName,
    IImageBuffer* pImgBuf,
    IImageBufferAllocator::ImgParam const& imgParam
)
{
    CAM_LOGD("++++++++++++++++ test_imgbuf_jpeg_1p");
    //
    MY_CHECK(0==::strcmp(pImgBuf->getMagicName(), szHeapMagicName), "%s", pImgBuf->getMagicName());
    MY_CHECK(1==pImgBuf->getStrongCount(), "StrongCount:%d", pImgBuf->getStrongCount());
    MY_CHECK(imgParam.imgFormat==pImgBuf->getImgFormat(), "Format:%#x", pImgBuf->getImgFormat());
    MY_CHECK(imgParam.jpgSize==pImgBuf->getImgSize(), "Size:%dx%d", pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
    MY_CHECK(1==pImgBuf->getPlaneCount(), "PlaneCount:%d", pImgBuf->getPlaneCount());
    MY_CHECK(8==pImgBuf->getImgBitsPerPixel(), "BitsPerPixel:%d", pImgBuf->getImgBitsPerPixel());
    //
    MY_CHECK(8==pImgBuf->getPlaneBitsPerPixel(0), "PlaneBitsPerPixel(0):%d", pImgBuf->getPlaneBitsPerPixel(0));
    //
    CAM_LOGD("getImgSize(%dx%d), getTimestamp(%llu)",
        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, pImgBuf->getTimestamp());

    for (unsigned int plane = 0; plane < pImgBuf->getPlaneCount(); ++plane)
    {
        CAM_LOGD("[plane %d] BitsPerPixel(%d), BufOffsetInBytes(%d), BufSizeInBytes(%d), BufStridesInBytes(%d)",
            plane, pImgBuf->getPlaneBitsPerPixel(plane),
            pImgBuf->getBufOffsetInBytes(plane),
            pImgBuf->getBufSizeInBytes(plane),
            pImgBuf->getBufStridesInBytes(plane)
        );
    }
    //
    MY_CHECK(pImgBuf->syncCache(eCACHECTRL_FLUSH), "syncCache()");
    //
    int usage = eBUFFER_USAGE_SW_WRITE_OFTEN;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0==pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0!=pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
            if  ( pImgBuf->getBufVA(i) )
            {
                ::memset((MUINT8*)pImgBuf->getBufVA(i), 0xAA, pImgBuf->getBufSizeInBytes(i));
            }
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0!=pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0!=pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
            if  ( pImgBuf->getBufVA(i) )
            {
                MUINT8* pb = (MUINT8*)pImgBuf->getBufVA(i);
                for (MUINT32 c = 0; c < pImgBuf->getBufSizeInBytes(i); c++)
                {
                    if  ( 0xAA != pb[c] )
                    {
                        CAM_LOGE("buf[%d]:%#x != 0xAA", c, pb[c]);
                        break;
                    }
                }
            }
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    usage = 0;
    MY_CHECK(pImgBuf->lockBuf("Test", usage), "lockBuf, usage:%#x", usage);
        //
        for (unsigned int i = 0; i < pImgBuf->getPlaneCount(); i++)
        {
            MY_CHECK(0==pImgBuf->getBufPA(i), "BufPA(%d)=%p", i, pImgBuf->getBufPA(i));
            MY_CHECK(0==pImgBuf->getBufVA(i), "BufVA(%d)=%p", i, pImgBuf->getBufVA(i));
        }
    MY_CHECK(pImgBuf->unlockBuf("Test"), "unlockBuf");
    //
    //
    CAM_LOGD("---------------- test_imgbuf_jpeg_1p");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam

