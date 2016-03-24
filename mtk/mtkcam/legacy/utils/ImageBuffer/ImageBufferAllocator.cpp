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
#define LOG_TAG "MtkCam/ImgBufAllocator"
//
#include "MyUtils.h"
#include <utils/imagebuf/IIonImageBufferHeap.h>
#include <utils/imagebuf/IGrallocImageBufferHeap.h>
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
 *
 ******************************************************************************/
IImageBufferAllocator*
IImageBufferAllocator::
getInstance()
{
    static IImageBufferAllocator singleton;
    return &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
IImageBufferAllocator::
alloc(
    char const* szCallerName,
    ImgParam const& rImgParam,
    ExtraParam const& rExtraParam
)
{
    IImageBuffer* pImgBuf = NULL;
    //
#if defined(MTK_ION_SUPPORT)
    pImgBuf = alloc_ion(szCallerName, rImgParam, rExtraParam);
#else
// TBD
    CAM_LOGE("No implement");
#endif
    //
    return pImgBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
IImageBufferAllocator::
alloc_ion(
    char const* szCallerName,
    ImgParam const& rImgParam,
    ExtraParam const& rExtraParam
)
{
    IImageBuffer* pImgBuf = NULL;
    //
#if defined(MTK_ION_SUPPORT)
    //
    IIonImageBufferHeap::AllocImgParam_t imgParam = rImgParam;
    IIonImageBufferHeap::AllocExtraParam extraParam;
    extraParam.nocache = rExtraParam.nocache;
    if ( eImgFmt_JPEG == rImgParam.imgFormat )
    {
        if ( 0 == imgParam.bufSize )
        {
            CAM_LOGE("Err imgParam: bufSize should not be 0 for JPEG memory");
            return NULL;
        }
        imgParam.imgFormat = eImgFmt_BLOB;
    }
    //
    sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create(szCallerName, imgParam, extraParam);
    if  ( pHeap == 0 ) {
        CAM_LOGE("NULL Heap");
        return NULL;
    }
    //
    if ( eImgFmt_JPEG == rImgParam.imgFormat )
    {
        pImgBuf = pHeap->createImageBuffer_FromBlobHeap(0, rImgParam.imgFormat, rImgParam.jpgSize, rImgParam.bufStridesInBytes);
    }
    else
    {
        pImgBuf = pHeap->createImageBuffer();
    }
    if  ( pImgBuf == 0 ) {
        CAM_LOGE("createImageBuffer fail");
        return NULL;
    }
    //
    pImgBuf->incStrong(pImgBuf);
#else
    CAM_LOGW("does not support ION!");
#endif
    //
    return pImgBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
IImageBufferAllocator::
alloc_gb(
    char const* szCallerName,
    ImgParam const& rImgParam,
    ExtraParam const& rExtraParam
)
{
    IImageBuffer* pImgBuf = NULL;
    //
    IGrallocImageBufferHeap::AllocImgParam_t imgParam = rImgParam;
    IGrallocImageBufferHeap::AllocExtraParam extraParam(rExtraParam.usage, rExtraParam.nocache);
    if ( eImgFmt_JPEG == rImgParam.imgFormat )
    {
        if ( 0 == imgParam.bufSize )
        {
            CAM_LOGE("Err imgParam: bufSize should not be 0 for JPEG memory");
            return NULL;
        }
        imgParam.imgFormat = eImgFmt_BLOB;
    }
    //
    sp<IImageBufferHeap> pHeap = IGrallocImageBufferHeap::create(szCallerName, imgParam, extraParam);
    if  ( pHeap == 0 ) {
        CAM_LOGE("NULL Heap");
        return NULL;
    }
    //
    if ( eImgFmt_JPEG == rImgParam.imgFormat )
    {
        pImgBuf = pHeap->createImageBuffer_FromBlobHeap(0, rImgParam.imgFormat, rImgParam.jpgSize, rImgParam.bufStridesInBytes);
    }
    else
    {
        pImgBuf = pHeap->createImageBuffer();
    }
    if  ( pImgBuf == 0 ) {
        CAM_LOGE("createImageBuffer fail");
        return NULL;
    }
    //
    pImgBuf->incStrong(pImgBuf);
    //
    return pImgBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
IImageBufferAllocator::
free(IImageBuffer* pImageBuffer)
{
    if (pImageBuffer != NULL) {
        pImageBuffer->decStrong(pImageBuffer);
    }
    else {
        CAM_LOGE("pImageBuffer is NULL");
        dumpCallStack(__FUNCTION__);
    }
}

