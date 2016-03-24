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

#ifndef _MTK_HARDWARE_MTKCAM_UTILS_IMAGEBUFFER_TEST_TESTUTILS_H_
#define _MTK_HARDWARE_MTKCAM_UTILS_IMAGEBUFFER_TEST_TESTUTILS_H_
//
#include <common.h>
#include <utils/include/common.h>
//
#include <utils/StrongPointer.h>
using namespace android;
//
//
namespace NSCam {
/******************************************************************************
 *
 ******************************************************************************/
#define MY_CHECK(cond, ...) \
        do { \
            if ((cond)) { CAM_LOGD("<PASS>"__VA_ARGS__); } \
            else        { CAM_LOGE("<FAIL>"__VA_ARGS__); } \
        }while(0)

#define OBJ_CHECK(obj)                  \
        do {                            \
            if ((obj==0)) {             \
                CAM_LOGE("NULL OBJ");   \
                return;                 \
            }                           \
        }while(0)


/**
 *  test heap with a blob format
 */
bool
test_heap_blob(
    char const*const szHeapMagicName, 
    sp<IImageBufferHeap>& pHeap, 
    MUINT32 size
);


/**
 *  test image buffer with a blob format
 */
bool
test_imgbuf_blob(
    char const*const szHeapMagicName, 
    IImageBuffer* pImgBuf, 
    MUINT32 size,
    bool const isSBS = false,
    bool const isROI = false
);


/**
 *  test heap with a format of yuv420 3p
 */
bool
test_heap_yuv420_3p(
    char const*const szHeapMagicName, 
    sp<IImageBufferHeap>& pHeap, 
    IImageBufferAllocator::ImgParam const& imgParam,
    bool const isGB = false
);


/**
 *  test image buffer with a format of yuv420 3p
 */
bool
test_imgbuf_yuv420_3p(
    char const*const szHeapMagicName, 
    IImageBuffer* pImgBuf, 
    IImageBufferAllocator::ImgParam const& imgParam,
    bool const isSBS = false,
    bool const isROI = false
);


/**
 *  test image buffer with a format of jpeg 1p
 */
bool
test_imgbuf_jpeg_1p(
    char const*const szHeapMagicName, 
    IImageBuffer* pImgBuf, 
    IImageBufferAllocator::ImgParam const& imgParam
);


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_UTILS_IMAGEBUFFER_TEST_TESTUTILS_H_

