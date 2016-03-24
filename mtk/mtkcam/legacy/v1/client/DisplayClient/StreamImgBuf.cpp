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

#define LOG_TAG "MtkCam/DisplayClient"
//
#include "CamUtils.h"
#include "StreamImgBuf.h"
using namespace android;
using namespace NSDisplayClient;
using namespace NSCam::Utils;
//
#if ('1'==MTKCAM_HAVE_GRALLOC_EXTRA)
#include <ui/gralloc_extra.h>
#endif
#include "system/graphics.h"
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define GET_IIMGBUF_IMG_STRIDE_IN_BYTE_FOR_DISPLAY_ROTATION(pImgInfo, plane)  \
            ((Format::queryPlaneCount(Format::queryImageFormat(pImgInfo->ms8ImgFormat)) >= (plane+1)) ? \
            ((getImgWidthStride(plane+3)*Format::queryPlaneBitsPerPixel(Format::queryImageFormat(pImgInfo->ms8ImgFormat),plane)))>>3 : 0)
//
#define GET_IIMGBUF_IMG_STRIDE_IN_BYTE(pImgInfo, plane)   \
            ((Format::queryPlaneCount(Format::queryImageFormat(pImgInfo->ms8ImgFormat)) >= (plane+1)) ? \
            ((getImgWidthStride(plane)*Format::queryPlaneBitsPerPixel(Format::queryImageFormat(pImgInfo->ms8ImgFormat),plane)))>>3 : 0)

/******************************************************************************
 *
 ******************************************************************************/
ImgInfo::
ImgInfo(
    uint32_t const u4ImgWidth,
    uint32_t const u4ImgHeight,
    char const*const ImgFormat,
    int32_t const i4ImgFormat,
    char const*const pImgName,
    uint32_t const u4Orientation
)
    : ms8ImgName(pImgName)
    , ms8ImgFormat(ImgFormat)
    , mi4ImgFormat(i4ImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4BitsPerPixel(Format::queryImageBitsPerPixel(i4ImgFormat))
    , mi4Orientation(u4Orientation)
{
    CAM_LOGD(
        "[%s](%s@%dx%d@%d-bit)",
        ms8ImgName.string(), ms8ImgFormat.string(),
        mu4ImgWidth, mu4ImgHeight, mu4BitsPerPixel
    );
}


/******************************************************************************
 *
 ******************************************************************************/
StreamImgBuf::
StreamImgBuf(
    sp<ImgInfo const>const& rpImgInfo,
    int32_t const   i4Stride,
    void*const      pBufBase,
    buffer_handle_t*pBufHndl,
    int const       fdIon,
    int64_t const   timestamp
)
    : IImgBuf()
    , mpImgInfo(rpImgInfo)
    , mi8Timestamp(timestamp)
    , mfdIon(fdIon)
    , mpBufBase(pBufBase)
    , mpBufHndl(pBufHndl)
    , mpANWBuffer(0)
    , mi4Stride(i4Stride)
    , mBufSize(0)
    , mbNeedRotation(false)
{
#ifndef container_of
#define container_of(ptr, type, member) \
    (type *)((char*)(ptr) - offsetof(type, member))
#endif

    mpANWBuffer = container_of(pBufHndl, ANativeWindowBuffer, handle);

    MY_LOGE_IF(mpANWBuffer->stride != (int)getImgWidthStride(0),"mismatch stride: %d %d", mpANWBuffer->stride, mi4Stride);
    MY_LOGE_IF(mpANWBuffer->width  != (int)getImgWidth(),       "mismatch width: %d %d", mpANWBuffer->width, getImgWidth());
    MY_LOGE_IF(mpANWBuffer->height != (int)getImgHeight(),      "mismatch height: %d %d", mpANWBuffer->height, getImgHeight());
#if 1

    if(rpImgInfo->mi4Orientation & HAL_TRANSFORM_ROT_90)    //It also included rot 270 case, because rot 270 = rot90|flipH|flipV )
    {
        //If rotated, the fromat must be YUYV
        //For (Rotated) YUYV Format
        mBufSize =  getImgWidth() * (GET_IIMGBUF_IMG_STRIDE_IN_BYTE_FOR_DISPLAY_ROTATION(rpImgInfo,0));
    }
    else
    {
        //If not rotated, the fromat must be YV12
        //For YV12 Format
        size_t const y_size = getImgHeight() * (GET_IIMGBUF_IMG_STRIDE_IN_BYTE(rpImgInfo,0));
        size_t const vu_size= (getImgHeight()>>1) * ((GET_IIMGBUF_IMG_STRIDE_IN_BYTE(rpImgInfo,1)) + (GET_IIMGBUF_IMG_STRIDE_IN_BYTE(rpImgInfo,2)));
        mBufSize = y_size + vu_size;
    }
#endif
#if 0
    dump();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
StreamImgBuf::
~StreamImgBuf()
{
#if 0
    MY_LOGD(
        "%dx%d ion:%d, pBuf/pBufHndl=%p/%p/%p",
        getImgWidth(), getImgHeight(),
        mfdIon, mpBufBase, mpBufHndl, *mpBufHndl
    );
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
StreamImgBuf::
getImgWidthStride(uint_t const uPlaneIndex) const
{
    if(mpImgInfo->mi4Orientation == 0 || uPlaneIndex <= 2)
    {
        return  (0 == uPlaneIndex)
            ?   mi4Stride
            :   ((~15) & (15 + (mi4Stride>>1)))
                ;
    }
    else
    {
        int stride = mi4Stride;
#if ('1'==MTKCAM_HAVE_GRALLOC_EXTRA)
        status_t err = 0;
        #if 0 //TBD
        err = ::gralloc_extra_query(*mpBufHndl, GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE, &stride);
        #endif
        if(err)
            MY_LOGE("gralloc_extra_query GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE err = %d",err);
#endif
        return ((3 == uPlaneIndex) ? stride : 0);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
StreamImgBuf::
dump() const
{
    MY_LOGD(
        "[%s](%s@%dx%d@%d-bit@%d), ion:%d, pBuf/pBufHndl=%p/%p/%p, Stride:%d, Timestamp:%lld",
        getBufName(), getImgFormat().string(), getImgWidth(), getImgHeight(),
        getBitsPerPixel(), getBufSize(),
        mfdIon,
        mpBufBase, mpBufHndl, *mpBufHndl, mi4Stride, mi8Timestamp
    );
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
StreamImgBuf::
getOrientation()
{
    return mpImgInfo->mi4Orientation;
}

/******************************************************************************
 *
 ******************************************************************************/
void
StreamImgBuf::
setNeedDisplayRotation(bool bNeed)
{
    mbNeedRotation = bNeed;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
StreamImgBuf::
getNeedDisplayRotation()
{
    return mbNeedRotation;
}

