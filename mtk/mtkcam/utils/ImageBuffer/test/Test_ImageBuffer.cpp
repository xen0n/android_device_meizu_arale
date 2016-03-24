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

#define LOG_TAG "MtkCam/camtest/ImgBuf"
//
//
#include "CamLog.h"
#include "TestUtils.h"
//
#include "../BaseImageBuffer.h"
#include <utils/include/imagebuf/IIonImageBufferHeap.h>
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
void
Test_ImageBuffer()
{
    CAM_LOGD("++++++++++++++++ Test_ImageBuffer ++++++++++++++++");
    /*****************
        *        Heap Blob
        ******************/
    {
        MUINT32 size = 2048;
        IImageBufferAllocator::ImgParam imgParam(size, 0);
        CAM_LOGD("    >>>>>>>> TestCase: BLOB Heap");
        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create("Test_ImgBuf_ION", imgParam);
        OBJ_CHECK(pHeap);
        test_heap_blob(IIonImageBufferHeap::magicName(), pHeap, size);
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: BLOB ImgBuf from BLOB Heap w/o offset");
            sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer_FromBlobHeap(0, size);
            OBJ_CHECK(pImgBuf);
            test_imgbuf_blob(pHeap->getMagicName(), pImgBuf.get(), size);
        }
        //
        {
            MUINT32 offset = 800;
            //// Image Buffer: with non-zero offset ////
            CAM_LOGD("    >>>>>>>> TestCase: Error check (oversize)");
            sp<IImageBuffer> pImgBuf_x = pHeap->createImageBuffer_FromBlobHeap(offset, size);
            MY_CHECK(pImgBuf_x == 0, "can not create ImgBuf");
            //
            CAM_LOGD("    >>>>>>>> TestCase: BLOB ImgBuf from BLOB Heap with offset");
            sp<IImageBuffer> pImgBuf_o = pHeap->createImageBuffer_FromBlobHeap(offset, size-offset);
            OBJ_CHECK(pImgBuf_o);
            test_imgbuf_blob(pHeap->getMagicName(), pImgBuf_o.get(), size-offset);
            //
            CAM_LOGD("    >>>>>>>> TestCase: setExtParam/getExtOffsetInBytes");
            MSize imgSize_fail(50, 5);
            MSize imgSize_pass(100, 1);
            MY_CHECK(!pImgBuf_o->setExtParam(imgSize_fail), 
                "setExtParam: size(%dx%d), getImgSize(%dx%d)", imgSize_fail.w, imgSize_fail.h, pImgBuf_o->getImgSize().w, pImgBuf_o->getImgSize().h);
            MUINT32 offset_fail = 2000;
            MUINT32 offset_pass = 100;
            MY_CHECK(pImgBuf_o->setExtParam(imgSize_pass, offset_pass), 
                "setExtParam: size(%dx%d), getImgSize(%dx%d), offset(%d), getExtOffsetInBytes(%d)@0-plane", 
                imgSize_pass.w, imgSize_pass.h, pImgBuf_o->getImgSize().w, pImgBuf_o->getImgSize().h, offset_pass, pImgBuf_o->getExtOffsetInBytes(0));
            MY_CHECK(!pImgBuf_o->setExtParam(imgSize_pass, offset_fail), "setExtParam: size(%dx%d) offset(%d)", imgSize_pass.w, imgSize_pass.h, offset_fail);
        }
        //
        MSize imgSize(80, 8);
        size_t bufStridesInBytes_fail[3] = {1000, 50, 50};
        size_t bufStridesInBytes_fail_2[3] = {100, 50, 10};
        size_t bufStridesInBytes_pass[3] = {100, 50, 50};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam2(
            eImgFmt_YV12, imgSize, 
            bufStridesInBytes_pass, bufBoundaryInBytes, 3
        );
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: Error check (oversize)");
            sp<IImageBuffer> pImgBuf_x = pHeap->createImageBuffer_FromBlobHeap(0, eImgFmt_YV12, imgSize, bufStridesInBytes_fail);
            MY_CHECK(pImgBuf_x == 0, "can not create ImgBuf");
            //
            CAM_LOGD("    >>>>>>>> TestCase: Error check (image size > stride)");
            pImgBuf_x = pHeap->createImageBuffer_FromBlobHeap(0, eImgFmt_YV12, imgSize, bufStridesInBytes_fail_2);
            MY_CHECK(pImgBuf_x == 0, "can not create ImgBuf");
            //
            CAM_LOGD("    >>>>>>>> TestCase: YV12 ImgBuf from BLOB Heap w/o offset");
            sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer_FromBlobHeap(0, eImgFmt_YV12, imgSize, bufStridesInBytes_pass);
            OBJ_CHECK(pImgBuf);
            test_imgbuf_yuv420_3p(pHeap->getMagicName(), pImgBuf.get(), imgParam2);
            //
            CAM_LOGD("    >>>>>>>> TestCase: setExtParam/getExtOffsetInBytes");
            MSize imgSize_fail(10, 10);
            MY_CHECK(!pImgBuf->setExtParam(imgSize_fail), "setExtParam: size(%dx%d)", imgSize_fail.w, imgSize_fail.h);
            MSize imgSize_fail2(150, 5);
            MY_CHECK(!pImgBuf->setExtParam(imgSize_fail2), "setExtParam: size(%dx%d)", imgSize_fail2.w, imgSize_fail2.h);
            MSize imgSize_pass(50, 5);
            MY_CHECK(pImgBuf->setExtParam(imgSize_pass) && imgSize_pass == pImgBuf->getImgSize(), 
                "setExtParam: size(%dx%d), getImgSize(%dx%d)", imgSize_pass.w, imgSize_pass.h, pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
            MUINT32 offset_fail = 500;
            MUINT32 offset_pass = 100;
            MY_CHECK(pImgBuf->setExtParam(imgSize_pass, offset_pass), 
                "setExtParam: size(%dx%d), getImgSize(%dx%d), offset(%d), getExtOffsetInBytes(%d)@0-plane", 
                imgSize_pass.w, imgSize_pass.h, pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, offset_pass, pImgBuf->getExtOffsetInBytes(0));
            MY_CHECK(!pImgBuf->setExtParam(imgSize_pass, offset_fail), "setExtParam: size(%dx%d) offset(%d)", imgSize_pass.w, imgSize_pass.h, offset_fail);
        }
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: BLOB ImgBuf");
            sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer();
            OBJ_CHECK(pImgBuf);
            test_imgbuf_blob(pHeap->getMagicName(), pImgBuf.get(), size);
        }
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase BLOB ImgBuf SBS: Right");
            sp<IImageBuffer> pImgBuf_R = pHeap->createImageBuffer_SideBySide(true); // is Right hand side
            OBJ_CHECK(pImgBuf_R);
            test_imgbuf_blob(pHeap->getMagicName(), pImgBuf_R.get(), size, true);
        }
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase BLOB ImgBuf SBS: Left");
            sp<IImageBuffer> pImgBuf_L = pHeap->createImageBuffer_SideBySide(false); // is Left hand side
            OBJ_CHECK(pImgBuf_L);
            test_imgbuf_blob(pHeap->getMagicName(), pImgBuf_L.get(), size, true);
        }
    }
    /***********************
        *        Heap YUV420 3P
        ***********************/
    {
        size_t bufStridesInBytes[3] = {2000, 1920, 960};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam(
            eImgFmt_YV12, MSize(1920, 1080), 
            bufStridesInBytes, bufBoundaryInBytes, 3
        );
        //
        CAM_LOGD("    >>>>>>>> TestCase: YUV420 3P Heap");
        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create("Test_ImgBuf_ION", imgParam);
        OBJ_CHECK(pHeap);
        test_heap_yuv420_3p(IIonImageBufferHeap::magicName(), pHeap, imgParam);
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: Error Check: BLOB ImgBuf from non-BLOB Heap");
            sp<IImageBuffer> pBlobBuf = pHeap->createImageBuffer_FromBlobHeap(0, 1080);
            MY_CHECK(pBlobBuf == NULL, "pBlobBuf is legal only if heap format is blob.");
        }
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: ImageBuffer with its ROI equal to the image resolution of this heap.");
            sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer();
            OBJ_CHECK(pImgBuf);
            test_imgbuf_yuv420_3p(pHeap->getMagicName(), pImgBuf.get(), imgParam);
        }
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: ImageBuffer SBS: Right");
            sp<IImageBuffer> pImgBuf_R = pHeap->createImageBuffer_SideBySide(true); // is Right hand side
            OBJ_CHECK(pImgBuf_R);
            test_imgbuf_yuv420_3p(pHeap->getMagicName(), pImgBuf_R.get(), imgParam, true);
            //
            CAM_LOGD("    >>>>>>>> TestCase: ImageBuffer SBS: Left");
            sp<IImageBuffer> pImgBuf_L = pHeap->createImageBuffer_SideBySide(false); // is Left hand side
            OBJ_CHECK(pImgBuf_L);
            test_imgbuf_yuv420_3p(pHeap->getMagicName(), pImgBuf_L.get(), imgParam, true);
        }
        //
        //
    }
    /***********************
        *        Heap RAW 1P
        ***********************/
    {
        size_t bufStridesInBytes[3] = {3688, 0, 0};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam(
            eImgFmt_BAYER10, MSize(2952, 100), 
            bufStridesInBytes, bufBoundaryInBytes, 1
        );
        //
        CAM_LOGD("    >>>>>>>> TestCase: RAW 1P Heap");
        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create("Test_ImgBuf_ION", imgParam);
        OBJ_CHECK(pHeap);
        //
        //
        {
            CAM_LOGD("    >>>>>>>> TestCase: Error Check: Invalid stride and image size");
            sp<IImageBuffer> pBuf = pHeap->createImageBuffer();
            MY_CHECK(pBuf == NULL, "can not create ImgBuf");
        }
    }
    //
    CAM_LOGD("---------------- Test_ImageBuffer ----------------");
}

