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

#define LOG_TAG "MtkCam/camtest/Alloc"
//
//
#include "CamLog.h"
#include "TestUtils.h"
//
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
void
Test_ImageBufferAllocator()
{
    CAM_LOGD("++++++++++++++++ Test_ImageBufferAllocator ++++++++++++++++");
    //
    //
    size_t bufStridesInBytes[3] = {2000, 1920, 960};
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParam(
        eImgFmt_YV12, MSize(1920, 1080),
        bufStridesInBytes, bufBoundaryInBytes, 3
    );
    //
    MUINT32 size = 2048;
    IImageBufferAllocator::ImgParam imgParam_blob(size, 0);
    //
    MUINT32 buffer_size = 1280*720;
    IImageBufferAllocator::ImgParam imgParam_jpeg(
        MSize(1280, 720), buffer_size, 0
    );
    //
    //// raw pointer
    {
        //
        IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
        //
        //// blob
        IImageBuffer* pImgBuf_blob = allocator->alloc("Test_Allocator_raw_blob", imgParam_blob);
        OBJ_CHECK(pImgBuf_blob);
        test_imgbuf_blob(pImgBuf_blob->getMagicName(), pImgBuf_blob, size);
        allocator->free(pImgBuf_blob);
        //
        //// yuv 3p
        IImageBuffer* pImgBuf = allocator->alloc("Test_Allocator_raw_yuv3p", imgParam);
        OBJ_CHECK(pImgBuf);
        test_imgbuf_yuv420_3p(pImgBuf->getMagicName(), pImgBuf, imgParam);
        allocator->free(pImgBuf);
        //
        //// jpeg 1p
        IImageBuffer* pImgBuf_jpeg = allocator->alloc("Test_Allocator_raw_jpeg1p", imgParam_jpeg);
        OBJ_CHECK(pImgBuf_jpeg);
        test_imgbuf_jpeg_1p(pImgBuf_jpeg->getMagicName(), pImgBuf_jpeg, imgParam_jpeg);
        //
        MY_CHECK(buffer_size==pImgBuf_jpeg->getBitstreamSize(), "%d", pImgBuf_jpeg->getBitstreamSize());
        MY_CHECK(!pImgBuf_jpeg->setBitstreamSize(7200000), "can not setBitstreamSize()");
        MY_CHECK(buffer_size==pImgBuf_jpeg->getBitstreamSize(), "%d", pImgBuf_jpeg->getBitstreamSize());
        MY_CHECK(pImgBuf_jpeg->setBitstreamSize(720000), "can setBitstreamSize()");
        MY_CHECK(720000==pImgBuf_jpeg->getBitstreamSize(), "%d", pImgBuf_jpeg->getBitstreamSize());
        allocator->free(pImgBuf_jpeg);
        //
    }
    //
    //// strong pointer
    {
        //
        IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
        //
        //// blob
        sp<IImageBuffer> spImgBuf_blob = allocator->alloc("Test_Allocator_sp_blob", imgParam_blob);
        OBJ_CHECK(spImgBuf_blob);
        allocator->free(spImgBuf_blob.get());
        test_imgbuf_blob(spImgBuf_blob->getMagicName(), spImgBuf_blob.get(), size);
        //
        //// yuv 3p
        sp<IImageBuffer> spImgBuf = allocator->alloc("Test_Allocator_sp_yuv3p", imgParam);
        OBJ_CHECK(spImgBuf);
        allocator->free(spImgBuf.get());
        test_imgbuf_yuv420_3p(spImgBuf->getMagicName(), spImgBuf.get(), imgParam);
        //
    }

    //
    CAM_LOGD("---------------- Test_ImageBufferAllocator ----------------");
}

