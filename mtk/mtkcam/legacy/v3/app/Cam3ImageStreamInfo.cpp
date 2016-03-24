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

#define LOG_TAG "MtkCam/StreamInfo"
//
#include "MyUtils.h"
#include <mtkcam/v3/app/Cam3ImageStreamInfo.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
Cam3ImageStreamInfo*
Cam3ImageStreamInfo::
cast(camera3_stream* stream)
{
    MY_LOGW_IF(!stream||!stream->priv, "camera3_stream stream:%p", stream);
    return static_cast<ThisT*>(stream->priv);
}


/******************************************************************************
 *
 ******************************************************************************/
Cam3ImageStreamInfo const*
Cam3ImageStreamInfo::
cast(camera3_stream const* stream)
{
    MY_LOGW_IF(!stream||!stream->priv, "camera3_stream:%p", stream);
    return static_cast<ThisT const*>(stream->priv);
}


/******************************************************************************
 *
 ******************************************************************************/
Cam3ImageStreamInfo::
Cam3ImageStreamInfo(
    camera3_stream*     pCamera3Stream,
    char const*         streamName,
    StreamId_T          streamId,
    MUINT               usageForConsumer,
    MUINT               usageForAllocator,
    MINT                imgFormatToAlloc,
    MINT                imgFormatInFact,
    BufPlanes_t const&  bufPlanes,
    size_t              maxBufNum,
    MUINT32             transform
)
    : IImageStreamInfo()
    , mpCamera3Stream(pCamera3Stream)
    , mStreamName(streamName)
    , mStreamId(streamId)
    , mStreamType(pCamera3Stream->stream_type)
    , mUsageForConsumer(usageForConsumer)
    , mImgFormat(imgFormatInFact)
    , mImgFormatToAlloc(imgFormatToAlloc)
    , mImgSize(pCamera3Stream->width, pCamera3Stream->height)
    , mvbufPlanes(bufPlanes)
    , mTransform(transform)
{
    switch  (pCamera3Stream->stream_type)
    {
    case CAMERA3_STREAM_OUTPUT:
        mStreamType = eSTREAMTYPE_IMAGE_OUT;
        break;
    case CAMERA3_STREAM_INPUT:
        mStreamType = eSTREAMTYPE_IMAGE_IN;
        break;
    case CAMERA3_STREAM_BIDIRECTIONAL:
        mStreamType = eSTREAMTYPE_IMAGE_INOUT;
        break;
    default:
        break;
    }
    //
    //
    pCamera3Stream->priv = this;
    pCamera3Stream->max_buffers = maxBufNum;
    pCamera3Stream->usage = usageForAllocator;
    MY_LOGV("camera3_stream:%p this:%p", pCamera3Stream, this);
}


/******************************************************************************
 *
 ******************************************************************************/
camera3_stream*
Cam3ImageStreamInfo::
get_camera3_stream() const
{
    return mpCamera3Stream;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
Cam3ImageStreamInfo::
getImgFormatToAlloc() const
{
    return mImgFormatToAlloc;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
Cam3ImageStreamInfo::
getUsageForConsumer() const
{
    return mUsageForConsumer;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
Cam3ImageStreamInfo::
getUsageForAllocator() const
{
    return mpCamera3Stream->usage;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
Cam3ImageStreamInfo::
getImgFormat() const
{
    return mImgFormat;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
Cam3ImageStreamInfo::
getImgSize() const
{
    return mImgSize;
}


/******************************************************************************
 *
 ******************************************************************************/
Cam3ImageStreamInfo::BufPlanes_t const&
Cam3ImageStreamInfo::
getBufPlanes() const
{
    return mvbufPlanes;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
Cam3ImageStreamInfo::
getTransform() const
{
    return mTransform;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Cam3ImageStreamInfo::
setTransform(MUINT32 transform)
{
        mTransform = transform;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
Cam3ImageStreamInfo::
getStreamName() const
{
    return mStreamName.string();
}


/******************************************************************************
 *
 ******************************************************************************/
StreamId_T
Cam3ImageStreamInfo::
getStreamId() const
{
    return mStreamId;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
Cam3ImageStreamInfo::
getStreamType() const
{
    return mStreamType;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
Cam3ImageStreamInfo::
getMaxBufNum() const
{
    return mpCamera3Stream->max_buffers;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cam3ImageStreamInfo::
setMaxBufNum(size_t count)
{
    mpCamera3Stream->max_buffers = count;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
Cam3ImageStreamInfo::
getMinInitBufNum() const
{
    return 0;
}

