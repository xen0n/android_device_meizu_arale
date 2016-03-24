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

#define LOG_TAG "MtkCam/BaseNode"
//
#include <mtkcam/Log.h>
#include "BaseNode.h"
#include <cutils/properties.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;


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
#define FUNCTION_IN             MY_LOGD_IF(1<mLogLevel, "%d:%#"PRIxPTR":%s +", getOpenId(), getNodeId(), getNodeName());
#define FUNCTION_OUT            MY_LOGD_IF(1<mLogLevel, "%d:%#"PRIxPTR":%s -", getOpenId(), getNodeId(), getNodeName());


/******************************************************************************
 *
 ******************************************************************************/
BaseNode::
BaseNode()
    : mOpenId(-1L)
    , mNodeId(NodeId_T(-1))
    , mNodeName()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.basenode", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
BaseNode::
getOpenId() const
{
    return mOpenId;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineNode::NodeId_T
BaseNode::
getNodeId() const
{
    return mNodeId;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
BaseNode::
getNodeName() const
{
    return mNodeName.string();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
ensureMetaBufferAvailable_(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IMetaStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
)
{
    FUNCTION_IN

    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGE("[frame:%u node:%#"PRIxPTR"]", frameNo, getNodeId());
        return NAME_NOT_FOUND;
    }
    //
    //  Wait acquire_fence.
    sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
    MERROR const err = acquire_fence->waitForever(getNodeName());
    MY_LOGE_IF(OK != err, "[frame:%u node:%#"PRIxPTR"][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
    //
    //
    //  Mark this buffer as ACQUIRED by this user.
    if (acquire) {
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    }
    //
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        MY_LOGE("[frame:%u node:%#"PRIxPTR"][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }

    //
    FUNCTION_OUT

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
ensureImageBufferAvailable_(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
)
{
    FUNCTION_IN

    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGW("[frame:%u node:%#"PRIxPTR"]", frameNo, getNodeId());
        return NAME_NOT_FOUND;
    }
    //
    //  Wait acquire_fence.
    sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
    MERROR const err = acquire_fence->waitForever(getNodeName());
    MY_LOGE_IF(OK != err, "[frame:%u node:%#"PRIxPTR"][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
    //
    //  Mark this buffer as ACQUIRED by this user.
    if (acquire) {
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    }
    //
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        //  The producer ahead of this user may fail to render this buffer's content.
        MY_LOGE("[frame:%u node:%#"PRIxPTR"][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }

    FUNCTION_OUT
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseNode::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNCTION_IN

    sp<IPipelineNodeCallback> cb = pFrame->getPipelineNodeCallback();
        if (cb != NULL) {
            cb->onDispatchFrame(pFrame, getNodeId());
        }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BaseNode::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    FUNCTION_IN;
    //
    MERROR err = OK;

    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Note:
    //  1. Don't mark ACQUIRE if never waiting on its acquire fence.
    //  2. Don't mark IN_FLIGHT (so we know the producer has not touched the buffer, and its content is ERROR).
    //  3. (Producer) users Needn't set its buffer status to ERROR.

    ////////////////////////////////////////////////////////////////////////////
    //  Mark buffers as RELEASE.
    ////////////////////////////////////////////////////////////////////////////

    sp<IStreamInfoSet const> pIStreams, pOStreams;
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    err = pFrame->queryIOStreamInfoSet(getNodeId(), pIStreams, pOStreams);
    if  ( OK != err ) {
        MY_LOGE("nodeId:%#"PRIxPTR" frameNo:%#u queryIOStreamInfoSet", getNodeId(), pFrame->getFrameNo());
    }
    //
    if  ( IStreamInfoSet const* pStreams = pIStreams.get() ) {
        {// I:Meta
            sp<IStreamInfoSet::IMap<IMetaStreamInfo> > pMap = pStreams->getMetaInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
        {// I:Image
            sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
    }
    else {
        MY_LOGE("nodeId:%#"PRIxPTR" frameNo:%#u NULL IStreams", getNodeId(), pFrame->getFrameNo());
    }
    //
    if  ( IStreamInfoSet const* pStreams = pOStreams.get() ) {
        {// O:Meta
            sp<IStreamInfoSet::IMap<IMetaStreamInfo> > pMap = pStreams->getMetaInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
        {// O:Image
            sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
            for (size_t i = 0; i < pMap->size(); i++) {
                StreamId_T const streamId = pMap->valueAt(i)->getStreamId();
                //  Mark this buffer as RELEASE by this user.
                sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                if  ( pStreamBuffer != 0 ) {
                    pStreamBuffer->markUserStatus(getNodeId(), IUsersManager::UserStatus::RELEASE);
                }
            }
        }
    }
    else {
        MY_LOGE("nodeId:%#"PRIxPTR" frameNo:%#u NULL OStreams", getNodeId(), pFrame->getFrameNo());
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
    //
    FUNCTION_OUT;
    return OK;
}

