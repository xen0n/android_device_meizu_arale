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

#define LOG_TAG "MtkCam/pipeline"
//
#include "MyUtils.h"
#include "PipelineBufferSetFrameControlImp.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::NSPipelineBufferSetFrameControlImp;

#define MAIN_CLASS_NAME PipelineBufferSetFrameControlImp

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
//
#define MY_LOG1(...)                MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOG2(...)                MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)


/******************************************************************************
 *
 ******************************************************************************/
IPipelineBufferSetFrameControl*
IPipelineBufferSetFrameControl::
create(
    MUINT32 requestNo,
    MUINT32 frameNo,
    android::wp<IAppCallback>const& pAppCallback,
    IPipelineStreamBufferProvider const* pBufferProvider,
    android::wp<IPipelineNodeCallback> pNodeCallback
)
{
    if( pNodeCallback == NULL ) {
        MY_LOGE("IPipelineNodeCallback should not be NULL!");
        return NULL;
    }
    return new MAIN_CLASS_NAME(requestNo, frameNo, pAppCallback, pBufferProvider, pNodeCallback);
}

/******************************************************************************
 *
 ******************************************************************************/
MAIN_CLASS_NAME::
MAIN_CLASS_NAME(
    MUINT32 requestNo,
    MUINT32 frameNo,
    android::wp<IAppCallback>const& pAppCallback,
    IPipelineStreamBufferProvider const* pBufferProvider,
    android::wp<IPipelineNodeCallback> pNodeCallback
)
    : mFrameNo(frameNo)
    , mRequestNo(requestNo)
    , mRWLock()
    , mpAppCallback(pAppCallback)
    , mListeners()
    , mTimestampFrameCreated(::elapsedRealtimeNano())
    , mTimestampFrameDone(0)
    //
    , mBufferProvider(const_cast<IPipelineStreamBufferProvider*>(pBufferProvider))
    , mpPipelineCallback(pNodeCallback)
    , mpStreamInfoSet(0)
    , mpNodeMap(0)
    , mpPipelineNodeMap(0)
    , mpPipelineDAG(0)
    //
    , mItemMapLock()
    , mNodeStatusMap()
    , mpReleasedCollector(new ReleasedCollector)
    , mpItemMap_AppImage(new ItemMap_AppImageT(mpReleasedCollector.get()))
    , mpItemMap_AppMeta (new ItemMap_AppMetaT (mpReleasedCollector.get()))
    , mpItemMap_HalImage(new ItemMap_HalImageT(mpReleasedCollector.get()))
    , mpItemMap_HalMeta (new ItemMap_HalMetaT (mpReleasedCollector.get()))
    //
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.pipeline", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
MAIN_CLASS_NAME::
onLastStrongRef(const void* /*id*/)
{
    if  (
            (0 != mpItemMap_AppImage->mNonReleasedNum)
        ||  (0 != mpItemMap_AppMeta->mNonReleasedNum)
        ||  (0 != mpItemMap_HalImage->mNonReleasedNum)
        ||  (0 != mpItemMap_HalMeta->mNonReleasedNum)
        )
    {
        MY_LOGW(
            "buffers are not released: #(AppImage, AppMeta, HalImage, HalMeta)=(%zd %zd %zd %zd)",
            mpItemMap_AppImage->mNonReleasedNum,
            mpItemMap_AppMeta->mNonReleasedNum,
            mpItemMap_HalImage->mNonReleasedNum,
            mpItemMap_HalMeta->mNonReleasedNum
        );

        struct Log
        {
            static  MVOID
            dump(IMyMap& rMap)
            {
                for (size_t i = 0; i < rMap.size(); i++) {
                    if  ( rMap.itemAt(i) != 0 ) {
                        MY_LOGW("Non-released stream:%#" PRIxPTR "(%s)", rMap.keyAt(i), rMap.itemAt(i)->getStreamInfo()->getStreamName());
                    }
                }
            }
        };
        Log::dump(*mpItemMap_AppImage);
        Log::dump(*mpItemMap_AppMeta);
        Log::dump(*mpItemMap_HalImage);
        Log::dump(*mpItemMap_HalMeta);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
attachListener(
    wp<IPipelineFrameListener>const& pListener,
    MVOID* pCookie
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    mListeners.push_back(MyListener(pListener, pCookie));
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeMap const>
MAIN_CLASS_NAME::
getPipelineNodeMap() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeMap const> p = mpPipelineNodeMap.promote();
    //
    MY_LOGE_IF(
        mpPipelineNodeMap==0 || p==0,
        "requestNo:%u frameNo:%u Bad PipelineNodeMap: wp:%p promote:%p - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        getRequestNo(), getFrameNo(), mpPipelineNodeMap.unsafe_get(), p.get(),
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    //
    return p;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG const&
MAIN_CLASS_NAME::
getPipelineDAG() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpPipelineDAG==0,
        "requestNo:%u frameNo:%u NULL PipelineDAG - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        getRequestNo(), getFrameNo(), mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return *mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamInfoSet const&
MAIN_CLASS_NAME::
getStreamInfoSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpStreamInfoSet==0,
        "requestNo:%u frameNo:%u NULL StreamInfoSet - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        getRequestNo(), getFrameNo(), mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return *mpStreamInfoSet;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamBufferSet&
MAIN_CLASS_NAME::
getStreamBufferSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    return *const_cast<MAIN_CLASS_NAME*>(this);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeCallback>
MAIN_CLASS_NAME::
getPipelineNodeCallback() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeCallback> p = mpPipelineCallback.promote();
    //
    MY_LOGE_IF(
        mpPipelineCallback==0 || p==0,
        "requestNo:%u frameNo:%u Bad PipelineNodeCallback: wp:%p promote:%p - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        getRequestNo(), getFrameNo(), mpPipelineCallback.unsafe_get(), p.get(),
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return p;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setNodeMap(
    android::sp<IPipelineFrameNodeMapControl> value
)
{
    if  ( value == 0) {
        MY_LOGE("requestNo:%u frameNo:%u - NULL value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    if  ( value->isEmpty() ) {
        MY_LOGE("requestNo:%u frameNo:%u - Empty value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpNodeMap = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setPipelineNodeMap(
    android::sp<IPipelineNodeMap const> value
)
{
    if  ( value == 0) {
        MY_LOGE("requestNo:%u frameNo:%u - NULL value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    if  ( value->isEmpty() ) {
        MY_LOGE("requestNo:%u frameNo:%u - Empty value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineNodeMap = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setPipelineDAG(android::sp<IPipelineDAG const> value)
{
    if  ( value == 0) {
        MY_LOGE("requestNo:%u frameNo:%u - NULL value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineDAG = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setStreamInfoSet(android::sp<IStreamInfoSet const> value)
{
    if  ( value == 0) {
        MY_LOGE("requestNo:%u frameNo:%u - NULL value", getRequestNo(), getFrameNo());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpStreamInfoSet = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
queryIOStreamInfoSet(
    NodeId_T const& nodeId,
    sp<IStreamInfoSet const>& rIn,
    sp<IStreamInfoSet const>& rOut
) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( mpNodeMap == 0 ) {
        MY_LOGE("requestNo:%u frameNo:%u NULL node map", getRequestNo(), getFrameNo());
        rIn = 0;
        rOut = 0;
        return NO_INIT;
    }
    //
    sp<IPipelineFrameNodeMapControl::INode> pNode = mpNodeMap->getNodeFor(nodeId);
    if  ( pNode == 0 ) {
        MY_LOGE("requestNo:%u frameNo:%u nodeId:%#" PRIxPTR " not found", getRequestNo(), getFrameNo(), nodeId);
        rIn = 0;
        rOut = 0;
        return NAME_NOT_FOUND;
    }
    //
    rIn = pNode->getIStreams();
    rOut= pNode->getOStreams();
    //
    if  ( rIn == 0 || rOut == 0 ) {
        MY_LOGE("requestNo:%u frameNo:%u nodeId:%#" PRIxPTR " IStreams:%p OStreams:%p", getRequestNo(), getFrameNo(), nodeId, rIn.get(), rOut.get());
        return NO_INIT;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
queryInfoIOMapSet(
    NodeId_T const& nodeId,
    InfoIOMapSet& rIOMapSet
) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( mpNodeMap == 0 ) {
        MY_LOGE("requestNo:%u frameNo:%u NULL node map", getRequestNo(), getFrameNo());
        return NO_INIT;
    }
    //
    sp<IPipelineFrameNodeMapControl::INode> pNode = mpNodeMap->getNodeFor(nodeId);
    if  ( pNode == 0 ) {
        MY_LOGE("requestNo:%u frameNo:%u nodeId:%#" PRIxPTR " not found", getRequestNo(), getFrameNo(), nodeId);
        return NAME_NOT_FOUND;
    }
    //
    rIOMapSet = pNode->getInfoIOMapSet();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
startConfiguration()
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
finishConfiguration()
{
    RWLock::AutoWLock _lRWLock(mRWLock);
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    if  ( mpNodeMap == 0 || mpNodeMap->isEmpty() ) {
        MY_LOGE("Empty NodeMap: %p", mpNodeMap.get());
        return NO_INIT;
    }
    //
    if  ( mpStreamInfoSet == 0 )
    {
        MY_LOGE("StreamInfoSet:%p", mpStreamInfoSet.get());
        return NO_INIT;
    }
    //
    if  ( mpPipelineDAG == 0 || mpPipelineNodeMap == 0 )
    {
        MY_LOGE("PipelineDAG:%p PipelineNodeMap:%p", mpPipelineDAG.get(), mpPipelineNodeMap.unsafe_get());
        return NO_INIT;
    }
    //
    mpReleasedCollector->finishConfiguration(*mpItemMap_AppImage, *mpItemMap_AppMeta);
    //
    mNodeStatusMap.setCapacity(mpNodeMap->size());
    for (size_t i = 0; i < mpNodeMap->size(); i++)
    {
        sp<NodeStatus> pNodeStatus = new NodeStatus;
        //
        IPipelineFrameNodeMapControl::INode* pNode = mpNodeMap->getNodeAt(i);
        NodeId_T const nodeId = pNode->getNodeId();
        {
            sp<IStreamInfoSet const> pStreams = pNode->getIStreams();
            //I:Meta
            for (size_t j = 0; j < pStreams->getMetaInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getMetaInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mISetMeta.push_back(pIO);
                pIO->mMapItem = getMetaMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No I meta item for streamId:%#" PRIxPTR, streamId);
            }
            //I:Image
            for (size_t j = 0; j < pStreams->getImageInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getImageInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mISetImage.push_back(pIO);
                pIO->mMapItem = getImageMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No I image item for streamId:%#" PRIxPTR, streamId);
            }
        }
        {
            sp<IStreamInfoSet const> pStreams = pNode->getOStreams();
            //O:Meta
            for (size_t j = 0; j < pStreams->getMetaInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getMetaInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mOSetMeta.push_back(pIO);
                pIO->mMapItem = getMetaMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No O meta item for streamId:%#" PRIxPTR, streamId);
            }
            //O:Image
            for (size_t j = 0; j < pStreams->getImageInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getImageInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mOSetImage.push_back(pIO);
                pIO->mMapItem = getImageMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No O image item for streamId:%#" PRIxPTR, streamId);
            }
        }
        //
        if  (
                ! pNodeStatus->mISetMeta.empty()
            ||  ! pNodeStatus->mOSetMeta.empty()
            ||  ! pNodeStatus->mISetImage.empty()
            ||  ! pNodeStatus->mOSetImage.empty()
            )
        {
            mNodeStatusMap.add(nodeId, pNodeStatus);
            mNodeStatusMap.mInFlightNodeCount++;
            //
            MY_LOG1(
                "nodeId:%#" PRIxPTR " Image:I/O#=%zu/%zu Meta:I/O#=%zu/%zu",
                nodeId,
                pNodeStatus->mISetImage.size(), pNodeStatus->mOSetImage.size(),
                pNodeStatus->mISetMeta.size(), pNodeStatus->mOSetMeta.size()
            );
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IUsersManager>
MAIN_CLASS_NAME::
findSubjectUsersLocked(
    StreamId_T streamId
)   const
{
#define _IMPLEMENT_(_map_) \
    { \
        ssize_t const index = _map_->indexOfKey(streamId); \
        if  ( 0 <= index ) { \
            return _map_->usersManagerAt(index); \
        } \
    }

    _IMPLEMENT_(mpItemMap_AppImage);
    _IMPLEMENT_(mpItemMap_AppMeta);
    _IMPLEMENT_(mpItemMap_HalImage);
    _IMPLEMENT_(mpItemMap_HalMeta);

#undef  _IMPLEMENT_

    MY_LOGW("[requestNo:%u frameNo:%u] streamId:%#" PRIxPTR " not found", getRequestNo(), getFrameNo(), streamId);
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class ItemMapT>
sp<typename ItemMapT::IStreamBufferT>
MAIN_CLASS_NAME::
getBufferLocked(
    StreamId_T streamId,
    ItemMapT const& rMap
)   const
{
    if  ( 0 == rMap.mNonReleasedNum ) {
        MY_LOGW_IF(
            0,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] "
            "mNonReleasedNum==0",
            getRequestNo(), getFrameNo(), streamId
        );
        return NULL;
    }
    //
    sp<typename ItemMapT::ItemT> pItem = rMap.getItemFor(streamId);
    if  ( pItem == 0 ) {
        MY_LOGW_IF(
            0,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] "
            "cannot find from map",
            getRequestNo(), getFrameNo(), streamId
        );
        return NULL;
    }
    //
    if  ( ! pItem->mBitStatus.hasBit(eBUF_STATUS_ACQUIRE) ) {
        MY_LOGF_IF(pItem->mBuffer!=0, "[frameNo:%u streamId:%#" PRIxPTR "] Non-null buffer but non-acquired status:%#x", getRequestNo(), getFrameNo(), streamId, pItem->mBitStatus.value);
        sp<IPipelineStreamBufferProvider> pBufferProvider = mBufferProvider.promote();
        if  ( pBufferProvider == 0 ) {
            MY_LOGE(
                "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] Fail to promote buffer provider:%p",
                getRequestNo(), getFrameNo(), streamId, mBufferProvider.unsafe_get()
            );
            return NULL;
        }
        //
        struct Helper
        {
            static  MERROR  acquireStreamBuffer(IPipelineStreamBufferProvider* pBufferProvider, ItemMap_HalImageT::MapValueT pItem)
                            {
                                MERROR err = pBufferProvider->acquireHalStreamBuffer(pItem->mStreamInfo, pItem->mBuffer);
                                if  ( OK == err && pItem->mBuffer != 0 ) {
                                    pItem->mBuffer->setUsersManager(pItem->mUsersManager);
                                    pItem->mBitStatus.markBit(eBUF_STATUS_ACQUIRE);
                                }
                                return err;
                            }

            static  MERROR  acquireStreamBuffer(IPipelineStreamBufferProvider*, ItemMap_HalMetaT::MapValueT)    { return INVALID_OPERATION; }
            static  MERROR  acquireStreamBuffer(IPipelineStreamBufferProvider*, ItemMap_AppMetaT::MapValueT)    { return INVALID_OPERATION; }
            static  MERROR  acquireStreamBuffer(IPipelineStreamBufferProvider*, ItemMap_AppImageT::MapValueT)   { return INVALID_OPERATION; }
        };
        CAM_TRACE_BEGIN("acquireStreamBuffer");
        NSCam::Utils::CamProfile profile(__FUNCTION__, "acquireStreamBuffer");
        MERROR err = Helper::acquireStreamBuffer(pBufferProvider.get(), pItem);
        profile.print_overtime(10, "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "]", getRequestNo(), getFrameNo(), streamId);
        CAM_TRACE_END();
        if  ( OK != err || pItem->mBuffer == 0 ) {
            MY_LOGE(
                "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] mBuffer:%p err:%d(%s)",
                getRequestNo(), getFrameNo(), streamId, pItem->mBuffer.get(), err, ::strerror(-err)
            );
            pItem->mBuffer = NULL;
            return NULL;
        }
    }
    //
    if  ( pItem->mBuffer == 0 ) {
        MY_LOGW(
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] "
            "mBitStatus(%#x) pValue->mBuffer == 0",
            getRequestNo(), getFrameNo(), streamId, pItem->mBitStatus.value
        );
        return NULL;
    }
    //
    return pItem->mBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class ItemMapT>
sp<typename ItemMapT::IStreamBufferT>
MAIN_CLASS_NAME::
getBufferLocked(
    StreamId_T streamId,
    UserId_T userId,
    ItemMapT const& rMap
)   const
{
    sp<typename ItemMapT::IStreamBufferT>
    pBuffer = getBufferLocked(streamId, rMap);
    //
    if  ( pBuffer == 0 ) {
        MY_LOGW_IF(
            0,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] NULL buffer",
            getRequestNo(), getFrameNo(), streamId, userId
        );
        return NULL;
    }

    /**
     * The buffer is NOT available if all users have released this buffer
     * (so as to be marked as released).
     */
    if  ( OK == pBuffer->haveAllUsersReleased() ) {
        MY_LOGW_IF(
            1,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "all users released this buffer",
            getRequestNo(), getFrameNo(), streamId, userId
        );
        return NULL;
    }

    /**
     * For a specific stream buffer (associated with a stream Id), a user (with
     * a unique user Id) could successfully acquire the buffer from this buffer
     * set only if all users ahead of this user have pre-released or released
     * the buffer.
     */
    if  ( OK != pBuffer->haveAllUsersReleasedOrPreReleased(userId) ) {
        MY_LOGW_IF(
            1,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "not all of prior users release or pre-release this buffer",
            getRequestNo(), getFrameNo(), streamId, userId
        );
        return NULL;
    }

    return pBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
MAIN_CLASS_NAME::
getMetaStreamBuffer(StreamId_T streamId) const
{
    sp<IMetaStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, *mpItemMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, *mpItemMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamBuffer>
MAIN_CLASS_NAME::
getImageStreamBuffer(StreamId_T streamId) const
{
    sp<IImageStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, *mpItemMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, *mpItemMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
MAIN_CLASS_NAME::
getMetaBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IMetaStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamBuffer>
MAIN_CLASS_NAME::
getImageBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IImageStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
MAIN_CLASS_NAME::
markUserStatus(
    StreamId_T const streamId,
    UserId_T userId,
    MUINT32 eStatus
)
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->markUserStatus(userId, eStatus);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setUserReleaseFence(
    StreamId_T const streamId,
    UserId_T userId,
    MINT releaseFence
)
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->setUserReleaseFence(userId, releaseFence);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
MAIN_CLASS_NAME::
queryGroupUsage(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return 0;
    }
    //
    return pSubjectUsers->queryGroupUsage(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
MAIN_CLASS_NAME::
createAcquireFence(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return -1;
    }
    //
    return pSubjectUsers->createAcquireFence(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getMapItemLocked(
    StreamId_T streamId,
    IMyMap const& rItemMap
)   const
{
    android::sp<IMyMap::IItem>const& pItem = rItemMap.itemFor(streamId);
    if  ( pItem == 0 ) {
        MY_LOGW_IF(
            0,
            "[requestNo:%u frameNo:%u streamId:%#" PRIxPTR "] "
            "cannot find from map",
            getRequestNo(), getFrameNo(), streamId
        );
        return NULL;
    }
    return pItem;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getMetaMapItemLocked(StreamId_T streamId) const
{
    sp<IMyMap::IItem> p;
    //
    p = getMapItemLocked(streamId, *mpItemMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getMapItemLocked(streamId, *mpItemMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getImageMapItemLocked(StreamId_T streamId) const
{
    sp<IMyMap::IItem> p;
    //
    p = getMapItemLocked(streamId, *mpItemMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getMapItemLocked(streamId, *mpItemMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
struct MAIN_CLASS_NAME::NodeStatusUpdater
{
public:     ////                    Definitions.
    typedef NodeStatus::IOSet       IOSet;

public:     ////                    Data Members.
    MUINT32 const                   mFrameNo;
    MINT32                          mLogLevel;

public:
    NodeStatusUpdater(MUINT32 frameNo, MINT32 logLevel)
        : mFrameNo(frameNo)
        , mLogLevel(logLevel)
    {
    }

    MBOOL
    run(
        NodeId_T const nodeId,
        NodeStatusMap& rNodeStatusMap,
        android::BitSet32& rNodeStatusUpdated
    )
    {
        MBOOL isAnyUpdate = MFALSE;
        //
        ssize_t const index = rNodeStatusMap.indexOfKey(nodeId);
        if  ( index < 0 ) {
            MY_LOGE("frameNo:%u nodeId:%#" PRIxPTR " not found", mFrameNo, nodeId);
            return MFALSE;
        }
        //
        sp<NodeStatus> pNodeStatus = rNodeStatusMap.valueAt(index);
        if  ( pNodeStatus == 0 ) {
            MY_LOGE("frameNo:%u nodeId:%#" PRIxPTR " NULL buffer", mFrameNo, nodeId);
            return MFALSE;
        }
        //
        // O Image
        if  ( updateNodeStatus(nodeId, pNodeStatus->mOSetImage) ) {
            isAnyUpdate = MTRUE;
            rNodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED);
            MY_LOG2("frameNo:%u nodeId:%#" PRIxPTR " O Image Buffers Released", mFrameNo, nodeId);
        }
        // I Image
        if  ( updateNodeStatus(nodeId, pNodeStatus->mISetImage) ) {
            isAnyUpdate = MTRUE;
            MY_LOG2("frameNo:%u nodeId:%#" PRIxPTR " I Image Buffers Released", mFrameNo, nodeId);
        }
        // O Meta
        if  ( updateNodeStatus(nodeId, pNodeStatus->mOSetMeta) ) {
            isAnyUpdate = MTRUE;
            rNodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED);
            MY_LOG2("frameNo:%u nodeId:%#" PRIxPTR " O Meta Buffers Released", mFrameNo, nodeId);
        }
        // I Meta
        if  ( updateNodeStatus(nodeId, pNodeStatus->mISetMeta) ) {
            isAnyUpdate = MTRUE;
            MY_LOG2("frameNo:%u nodeId:%#" PRIxPTR " I Meta Buffers Released", mFrameNo, nodeId);
        }

        //
        // Is it a new node with all buffers released?
        if  (
                isAnyUpdate
            &&  pNodeStatus->mOSetImage.empty()
            &&  pNodeStatus->mISetImage.empty()
            &&  pNodeStatus->mOSetMeta.empty()
            &&  pNodeStatus->mISetMeta.empty()
            )
        {
            rNodeStatusMap.mInFlightNodeCount--;
        }
        //
        return isAnyUpdate;
    }

protected:

    MBOOL
    updateNodeStatus(NodeId_T const nodeId, IOSet& rIOSet)
    {
        if  ( rIOSet.mNotified ) {
            return MFALSE;
        }
        //
        IOSet::iterator it = rIOSet.begin();
        for (; it != rIOSet.end();) {
            sp<IMyMap::IItem> pMapItem = (*it)->mMapItem;
            //
            if  ( OK == pMapItem->getUsersManager()->haveAllProducerUsersReleased() ) {
                pMapItem->handleProducersReleased();
            }
            //
            //  Check to see if this user "nodeId" has released.
            MUINT32 const status = pMapItem->getUsersManager()->getUserStatus(nodeId);
            if  ( 0 == (status & IUsersManager::UserStatus::RELEASE) ) {
                ++it;
                continue;
            }
            //
            it = rIOSet.erase(it);   //remove if released
            //
            if  ( OK == pMapItem->getUsersManager()->haveAllUsersReleased() ) {
                pMapItem->handleAllUsersReleased();
            }
        }
        //
        if  ( rIOSet.empty() ) {
            rIOSet.mNotified = MTRUE;
            return MTRUE;
        }
        //
        return MFALSE;
    }

}; // end struct MAIN_CLASS_NAME::NodeStatusUpdater


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
handleReleasedBuffers(UserId_T userId, sp<IAppCallback> pAppCallback)
{
    typedef ReleasedCollector::HalImageSetT HalImageSetT;
    typedef ReleasedCollector::HalMetaSetT  HalMetaSetT;
    typedef ReleasedCollector::AppMetaSetT  AppMetaSetT;

    HalImageSetT aHalImageSet;
    HalMetaSetT  aHalMetaSet;
    AppMetaSetT  aAppMetaSetO;
    ssize_t      aAppMetaNumO;
    {
        android::Mutex::Autolock _l(mpReleasedCollector->mLock);
        //
        aHalImageSet = mpReleasedCollector->mHalImageSet_AllUsersReleased;
        mpReleasedCollector->mHalImageSet_AllUsersReleased.clear();
        aHalMetaSet = mpReleasedCollector->mHalMetaSet_AllUsersReleased;
        mpReleasedCollector->mHalMetaSet_AllUsersReleased.clear();
        //
        aAppMetaSetO = mpReleasedCollector->mAppMetaSetO_ProducersReleased;
        mpReleasedCollector->mAppMetaSetO_ProducersReleased.clear();
        aAppMetaNumO = mpReleasedCollector->mAppMetaNumO_ProducersInFlight;
    }
    //
    //  Callback to App.
    {
        if  ( pAppCallback == 0 ) {
            MY_LOGW("Cannot promote AppCallback for requestNo:%u frameNo:%u, userId:%#" PRIxPTR, getRequestNo(), getFrameNo(), userId);
        }
        else {
            MY_LOG1("requestNo:%u frameNo:%u userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", getRequestNo(), getFrameNo(), userId, aAppMetaNumO, aAppMetaSetO.size());
            pAppCallback->updateFrame(getRequestNo(), userId, aAppMetaNumO, aAppMetaSetO);
        }
        aAppMetaSetO.clear();
    }
    //
    //  Release to Hal.
    {
        HalImageSetT::iterator it = aHalImageSet.begin();
        for (; it != aHalImageSet.end(); it++) {
            if  ( (*it) != 0 ) {
                (*it)->releaseBuffer();
            }
        }
        aHalImageSet.clear();
    }
    {
        HalMetaSetT::iterator it = aHalMetaSet.begin();
        for (; it != aHalMetaSet.end(); it++) {
            if  ( (*it) != 0 ) {
                (*it)->releaseBuffer();
            }
        }
        aHalMetaSet.clear();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
applyRelease(UserId_T userId)
{
    NodeId_T const nodeId = userId;
    sp<IAppCallback> pAppCallback;
    List<MyListener> listeners;
    BitSet32 nodeStatusUpdated;
    NodeStatusUpdater updater(getFrameNo(), mLogLevel);
    //
    String8 const logTag = String8::format("requestNo:%u frameNo:%u nodeId:%#" PRIxPTR, getRequestNo(), getFrameNo(), nodeId);
    MY_LOG1("%s +", logTag.string());
    //
    {
        RWLock::AutoWLock _lRWLock(mRWLock);
        Mutex::Autolock _lMapLock(mItemMapLock);
        //
        //  Update
        MBOOL isAnyUpdate = updater.run(nodeId, mNodeStatusMap, nodeStatusUpdated);
        //
        // Is the entire frame released?
        if  ( isAnyUpdate && 0 == mNodeStatusMap.mInFlightNodeCount )
        {
            nodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_FRAME_RELEASED);
            //
            mTimestampFrameDone = ::elapsedRealtimeNano();
            //
#if 1
//          mpPipelineNodeMap = 0;
//          mpPipelineDAG = 0;
            mpStreamInfoSet = 0;
#endif
            MY_LOG1(
                "Done requestNo:%u frameNo:%u @ nodeId:%#" PRIxPTR " - timestamp:%" PRIu64 "=%" PRIu64 "-%" PRIu64,
                getRequestNo(), getFrameNo(), nodeId,
                (mTimestampFrameDone-mTimestampFrameCreated),
                mTimestampFrameDone, mTimestampFrameCreated
            );
        }
        //
        if  ( ! nodeStatusUpdated.isEmpty() ) {
            listeners = mListeners;
        }
        //
        pAppCallback = mpAppCallback.promote();
    }
    //
    //
    handleReleasedBuffers(userId, pAppCallback);
    //
    //  Callback to listeners if needed.
    if  ( ! nodeStatusUpdated.isEmpty() )
    {
        NSCam::Utils::CamProfile profile(__FUNCTION__, logTag.string());
        //
        List<MyListener>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it) {
            sp<MyListener::IListener> p = it->mpListener.promote();
            if  ( p == 0 ) {
                continue;
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED) ) {
                MY_LOG2("%s O Meta Buffers Released", logTag.string());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED) ) {
                MY_LOG2("%s O Image Buffers Released", logTag.string());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_FRAME_RELEASED) ) {
                MY_LOG2("%s Frame Done", logTag.string());
                p->onPipelineFrame(
                    getFrameNo(),
                    IPipelineFrameListener::eMSG_FRAME_RELEASED,
                    it->mpCookie
                );
            }
        }
        //
        profile.print_overtime(3, "notify listeners (nodeStatusUpdated:%#x)", nodeStatusUpdated.value);
    }
    //
    MY_LOG1("%s -", logTag.string());
}

