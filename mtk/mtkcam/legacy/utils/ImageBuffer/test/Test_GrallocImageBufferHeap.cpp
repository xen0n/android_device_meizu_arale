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
//
#include "CamLog.h"
#include "TestUtils.h"
//
#include <utils/imagebuf/IGrallocImageBufferHeap.h>
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
void
Test_GrallocImageBufferHeap()
{
    CAM_LOGD("++++++++++++++++ Test_GrallocImageBufferHeap ++++++++++++++++");
    //
    //  Heap Blob
    {
        int size = 2048;
        IImageBufferAllocator::ImgParam imgParam(size, 0);
        IGrallocImageBufferHeap::AllocExtraParam extraParam(eBUFFER_USAGE_HW_TEXTURE | eBUFFER_USAGE_SW_WRITE_RARELY | eBUFFER_USAGE_HW_RENDER);
        sp<IImageBufferHeap> pHeap = IGrallocImageBufferHeap::create("Test_GrallocImageBufferHeap", imgParam, extraParam);
        MY_CHECK(pHeap == 0, "Gralloc does not support blob format.");
    }
    //
    //  Heap YUV420 3P
    {
        size_t bufStridesInBytes[3] = {2000, 1920, 200};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        //
        IImageBufferAllocator::ImgParam imgParam(
            eImgFmt_YV12, MSize(200, 200),
            bufStridesInBytes, bufBoundaryInBytes, 3
        );
        IGrallocImageBufferHeap::AllocExtraParam extraParam(eBUFFER_USAGE_HW_TEXTURE | eBUFFER_USAGE_SW_WRITE_RARELY | eBUFFER_USAGE_HW_RENDER);
        sp<IImageBufferHeap> pHeap = IGrallocImageBufferHeap::create("Test_GrallocImageBufferHeap", imgParam, extraParam);
        OBJ_CHECK(pHeap);
        //
        test_heap_yuv420_3p(IGrallocImageBufferHeap::magicName(), pHeap, imgParam, true);
    }
    //
    CAM_LOGD("---------------- Test_GrallocImageBufferHeap ----------------");
}

