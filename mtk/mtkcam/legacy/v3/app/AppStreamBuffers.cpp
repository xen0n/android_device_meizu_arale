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

#define LOG_TAG "MtkCam/streambuf"
//
#include "MyUtils.h"
#include <mtkcam/v3/app/AppStreamBuffers.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


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
AppImageStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
AppImageStreamBuffer::StreamBufferT*
AppImageStreamBuffer::
Allocator::
operator()(IGraphicImageBufferHeap* pHeap, IStreamInfoT* pStreamInfo)
{
    if  ( pHeap == 0 ) {
        MY_LOGE(
            "NULL IGraphicImageBufferHeap: %s",
            mpStreamInfo->getStreamName()
        );
        return NULL;
    }
    //
    return new StreamBufferT(
        pStreamInfo ? pStreamInfo : mpStreamInfo,
        pHeap
    );
}


/******************************************************************************
 *
 ******************************************************************************/
AppImageStreamBuffer::
AppImageStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    sp<IGraphicImageBufferHeap> pImageBufferHeap,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, *pImageBufferHeap.get(), pUsersManager)
    , mImageBufferHeap(pImageBufferHeap)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IGraphicImageBufferHeap>
AppImageStreamBuffer::
getImageBufferHeap() const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return mImageBufferHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
AppImageStreamBuffer::
getAcquireFence()   const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return mImageBufferHeap->getAcquireFence();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppImageStreamBuffer::
setAcquireFence(MINT fence)
{
    Mutex::Autolock _l(mBufMutex);
    //
    mImageBufferHeap->setAcquireFence(fence);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
AppImageStreamBuffer::
getReleaseFence()   const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return mImageBufferHeap->getReleaseFence();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppImageStreamBuffer::
setReleaseFence(MINT fence)
{
    Mutex::Autolock _l(mBufMutex);
    //
    mImageBufferHeap->setReleaseFence(fence);
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
AppImageStreamBuffer::
enqueUserGraph(
    android::sp<IUserGraph> pUserGraph
)
{
    ssize_t const groupIndex = mUsersManager->enqueUserGraph(pUserGraph);
    if  ( 0 != groupIndex ) {
        return groupIndex;
    }
    //
    MBOOL found = MFALSE;
    sp<IUserGraph> const& pMyUserGraph = pUserGraph;
    for (size_t i = 0; i < pMyUserGraph->size(); i++) {
        if ( IUsersManager::Category::NONE == pMyUserGraph->getCategory(i) )
            continue;
        MY_LOGD_IF(0, "Node:%d is a (%d: producer or consumer)!", i, pMyUserGraph->getCategory(i));
        //
        MERROR err = OK;
        if  ( ! found ) {
            found = MTRUE;
            MY_LOGD_IF(0, "0-indegree user:%d streamid(%#" PRIxPTR ") set acquire fence:%d", i, this->getStreamInfo()->getStreamId(), getAcquireFence());
            err = pMyUserGraph->setAcquireFence(i, getAcquireFence());
        }
        else {
            MINT fence = ::dup(getAcquireFence());
            MY_LOGW("another 0-indegree user:%d; need dup acquire fence:%d->%d", i, getAcquireFence(), fence);
            err = pMyUserGraph->setAcquireFence(i, fence);
        }
        //
        if  ( OK != err ) {
            MY_LOGE("Fail to setAcquireFence(%zu, %d)", i, getAcquireFence());
        }
    }
    return groupIndex;
}

/******************************************************************************
 *
 ******************************************************************************/
AppMetaStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
AppMetaStreamBuffer::StreamBufferT*
AppMetaStreamBuffer::
Allocator::
operator()()
{
    return new StreamBufferT(
        mpStreamInfo
    );
}


/******************************************************************************
 *
 ******************************************************************************/
AppMetaStreamBuffer::StreamBufferT*
AppMetaStreamBuffer::
Allocator::
operator()(NSCam::IMetadata const& metadata)
{
    return new StreamBufferT(
        mpStreamInfo, metadata
    );
}


/******************************************************************************
 *
 ******************************************************************************/
AppMetaStreamBuffer::
AppMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, mMetadata, pUsersManager)
    , mMetadata()
    , mRepeating(MFALSE)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
AppMetaStreamBuffer::
AppMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    NSCam::IMetadata const& metadata,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, mMetadata, pUsersManager)
    , mMetadata(metadata)
    , mRepeating(MFALSE)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppMetaStreamBuffer::
setRepeating(MBOOL const repeating)
{
    android::Mutex::Autolock _l(mBufMutex);
    mRepeating = repeating;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppMetaStreamBuffer::
isRepeating() const
{
    android::Mutex::Autolock _l(mBufMutex);
    return mRepeating;
}

