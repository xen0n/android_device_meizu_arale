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

#define LOG_TAG "MtkCam/DummyNode"
//
#include <mtkcam/Log.h>
#include <mtkcam/v3/hwnode/DummyNode.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>

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
DummyNode::
DummyNode(InitParams const& rParams)
    : IPipelineNode()
    , mOpenId(rParams.openId)
    , mNodeId(rParams.nodeId)
    , mNodeName(rParams.nodeName)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DummyNode::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return;
    }
    //
    IPipelineDAG::NodeObjSet_T nextNodes;
    MERROR err = pFrame->getPipelineDAG().getOutAdjacentNodes(getNodeId(), nextNodes);
    if  ( ! err && ! nextNodes.empty() )
    {
        for (size_t i = 0; i < nextNodes.size(); i++) {
            sp<IPipelineNode> pNextNode = pPipelineNodeMap->nodeAt(nextNodes[i].val);
            if  ( pNextNode != 0 ) {
                pNextNode->queue(pFrame);
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DummyNode::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    MY_LOGD("frameNo:%"PRIxPTR" nodeId:%"PRIxPTR" +", pFrame->getFrameNo(), getNodeId());
    //
    IPipelineFrame::InfoIOMapSet aIOMapSet;
    IPipelineFrame::ImageInfoIOMapSet const& rImageIOMapSet = aIOMapSet.mImageInfoIOMapSet;
    IPipelineFrame::MetaInfoIOMapSet const&  rMetaIOMapSet = aIOMapSet.mMetaInfoIOMapSet;
    //
    pFrame->queryInfoIOMapSet(getNodeId(), aIOMapSet);
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();


    ////////////////////////////////////////////////////////////////////////////
    //  Ensure buffers available.
    ////////////////////////////////////////////////////////////////////////////

    //  Image
    {
        IPipelineFrame::ImageInfoIOMapSet const& rMapSet = rImageIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            //
            for (size_t j = 0; j < rMapSet[i].vIn.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vIn.keyAt(j);
                MY_LOGD("[Image] IN streamId:%"PRIxPTR, streamId);
                //
                sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                MY_LOGE_IF(pStreamBuffer==0, "frameNo:%d nodeId:%"PRIxPTR" streamId:%"PRIxPTR" has null buffer", pFrame->getFrameNo(), getNodeId(), streamId);
                sp<IFence> acquire_fence = IFence::create(pStreamBuffer->createAcquireFence(getNodeId()));
                MERROR const err = acquire_fence->waitForever(getNodeName());
                MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", pFrame->getFrameNo(), getNodeId(), pStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
                //
                //  Mark this buffer as ACQUIRED by this user.
                rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
                //
                //  Check buffer status.
            }
            //
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                MY_LOGD("[Image] OUT streamId:%"PRIxPTR, streamId);
                //
                sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
                MY_LOGE_IF(pStreamBuffer==0, "frameNo:%d nodeId:%"PRIxPTR" streamId:%"PRIxPTR" has null buffer", pFrame->getFrameNo(), getNodeId(), streamId);
                sp<IFence> acquire_fence = IFence::create(pStreamBuffer->createAcquireFence(getNodeId()));
                MERROR const err = acquire_fence->waitForever(getNodeName());
                MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", pFrame->getFrameNo(), getNodeId(), pStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
                //
                //  Mark this buffer as ACQUIRED by this user.
                rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
                //
                //  Check buffer status.
            }
        }
    }
    //
    //  Meta
    {
        IPipelineFrame::MetaInfoIOMapSet const& rMapSet = rMetaIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            //
            for (size_t j = 0; j < rMapSet[i].vIn.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vIn.keyAt(j);
                MY_LOGD("[Meta] IN streamId:%"PRIxPTR, streamId);
                //
                sp<IMetaStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                sp<IFence> acquire_fence = IFence::create(pStreamBuffer->createAcquireFence(getNodeId()));
                MERROR const err = acquire_fence->waitForever(getNodeName());
                MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", pFrame->getFrameNo(), getNodeId(), pStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
                //
                //  Mark this buffer as ACQUIRED by this user.
                rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
                //
                //  Check buffer status.
            }
            //
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                MY_LOGD("[Meta] OUT streamId:%"PRIxPTR, streamId);
                //
                sp<IMetaStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                sp<IFence> acquire_fence = IFence::create(pStreamBuffer->createAcquireFence(getNodeId()));
                MERROR const err = acquire_fence->waitForever(getNodeName());
                MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", pFrame->getFrameNo(), getNodeId(), pStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
                //
                //  Mark this buffer as ACQUIRED by this user.
                rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
                //
                //  Check buffer status.
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    //  Prepare buffers before using.
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    //  Access buffers.
    ////////////////////////////////////////////////////////////////////////////

    //  Image
    {
        IPipelineFrame::ImageInfoIOMapSet const& rMapSet = rImageIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            //
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                //
                sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
#if 0
                sp<IImageBufferHeap> pImageBufferHeap = pStreamBuffer->tryWriteLock(getNodeName());
                sp<IImageBuffer> pImageBuffer = pImageBufferHeap->createImageBuffer();
                pImageBuffer->loadFromFile("sdcard/camera_dump/disp_640x480_nv21_000.yuv");
                pStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
#endif

                pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
                MY_LOGF_IF(
                    pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR),
                    "[Image] OUT streamId:%"PRIxPTR, streamId
                );
            }
        }
    }

    //  Meta
    {
        IPipelineFrame::MetaInfoIOMapSet const& rMapSet = rMetaIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                //
                sp<IMetaStreamBuffer> pStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
                IMetadata* metadata = pStreamBuffer->tryWriteLock(getNodeName());

                //MTK_SENSOR_TIMESTAMP
                {
                    IMetadata::IEntry entry(MTK_SENSOR_TIMESTAMP);
                    entry.push_back(::systemTime(), Type2Type< MINT64 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AF_MODE
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AF_MODE);
                    entry.push_back(MTK_CONTROL_AF_MODE_OFF, Type2Type< MUINT8 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AWB_MODE
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AWB_MODE);
                    entry.push_back(MTK_CONTROL_AWB_MODE_OFF, Type2Type< MUINT8 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AE_STATE
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AE_STATE);
                    entry.push_back(MTK_CONTROL_AE_STATE_CONVERGED, Type2Type< MUINT8 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AF_STATE
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AF_STATE);
                    entry.push_back(MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED, Type2Type< MUINT8 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AF_STATE
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AWB_STATE);
                    entry.push_back(MTK_CONTROL_AWB_STATE_CONVERGED, Type2Type< MUINT8 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AF_TRIGGER_ID
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AF_TRIGGER_ID);
                    entry.push_back(0, Type2Type< MINT32 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_CONTROL_AF_TRIGGER_ID
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AE_PRECAPTURE_ID);
                    entry.push_back(0, Type2Type< MINT32 >());
                    metadata->update(entry.tag(), entry);
                }

                //MTK_REQUEST_ID
                {
                    IMetadata::IEntry entry(MTK_REQUEST_ID);
                    entry.push_back(pFrame->getFrameNo(), Type2Type< MINT32 >());
                    metadata->update(entry.tag(), entry);
                }

                pStreamBuffer->unlock(getNodeName(), metadata);
                pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
                MY_LOGF_IF(
                    pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR),
                    "[Meta] OUT streamId:%"PRIxPTR, streamId
                );
            }
        }
    }
    ::sleep(5);

    ////////////////////////////////////////////////////////////////////////////
    //  Release buffers after using.
    ////////////////////////////////////////////////////////////////////////////

    //  Image
    {
        IPipelineFrame::ImageInfoIOMapSet const& rMapSet = rImageIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            //
            for (size_t j = 0; j < rMapSet[i].vIn.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vIn.keyAt(j);
                rStreamBufferSet.markUserStatus(
                    streamId, getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                );
            }
            //
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                rStreamBufferSet.markUserStatus(
                    streamId, getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                );
            }
        }
    }
    //
    //  Meta
    {
        IPipelineFrame::MetaInfoIOMapSet const& rMapSet = rMetaIOMapSet;
        for (size_t i = 0; i < rMapSet.size(); i++)
        {
            //
            for (size_t j = 0; j < rMapSet[i].vIn.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vIn.keyAt(j);
                rStreamBufferSet.markUserStatus(
                    streamId, getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                );
            }
            //
            for (size_t j = 0; j < rMapSet[i].vOut.size(); j++)
            {
                StreamId_T const streamId = rMapSet[i].vOut.keyAt(j);
                rStreamBufferSet.markUserStatus(
                    streamId, getNodeId(),
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                );
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    MY_LOGD("[applyRelease] frameNo:%"PRIxPTR" nodeId:%"PRIxPTR" +", pFrame->getFrameNo(), getNodeId());
    rStreamBufferSet.applyRelease(getNodeId());
    MY_LOGD("[applyRelease] frameNo:%"PRIxPTR" nodeId:%"PRIxPTR" -", pFrame->getFrameNo(), getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
queue(
    android::sp<IPipelineFrame> pFrame
)
{
    onProcessFrame(pFrame);
    return OK;
}













#if 0

#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;


/******************************************************************************
 *
 ******************************************************************************/
//
//  [Input]
//      Image/Yuv
//      Meta/Request
//
//  [Output]
//      Meta/Result
//
namespace {
class DummyNode
    : public virtual IPipelineNode
    , protected Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;
    typedef android::Vector<android::sp<IMetaStreamInfo> >  MetaStreamInfoSetT;
    typedef android::Vector<android::sp<IImageStreamInfo> > ImageStreamInfoSetT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    /**
     * Configure Parameters.
     */
    struct  ConfigParams
    {
        /**
         * A pointer to a set of input meta stream info.
         */
        MetaStreamInfoSetT const*   pvInMeta;

        /**
         * A pointer to a set of input image stream info.
         */
        ImageStreamInfoSetT const*  pvInImage;

        /**
         * A pointer to a set of output meta stream info.
         */
        MetaStreamInfoSetT const*   pvOutMeta;

        /**
         * A pointer to a set of output image stream info.
         */
        ImageStreamInfoSetT const*  pvOutImage;
    };

protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    MINT32                          mOpenId;
    NodeId_T                        mNodeId;
    String8                         mNodeName;
    sp<IMetaStreamInfo>             mpOutMetaStreamInfo_Result;
    sp<IMetaStreamInfo>             mpInpMetaStreamInfo_Request;
    sp<IImageStreamInfo>            mpInpImageStreamInfo_Yuv;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;

protected:  ////                    Operations.
    MERROR                          onDequeRequest(
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MVOID                           onFlushFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MVOID                           onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MERROR                          ensureMetaBufferAvailable_(
                                        MINT32 const frameNo,
                                        StreamId_T const streamId,
                                        IStreamBufferSet& rStreamBufferSet,
                                        sp<IMetaStreamBuffer>& rpStreamBuffer
                                    );

    MERROR                          ensureImageBufferAvailable_(
                                        MINT32 const frameNo,
                                        StreamId_T const streamId,
                                        IStreamBufferSet& rStreamBufferSet,
                                        sp<IImageStreamBuffer>& rpStreamBuffer
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    DummyNode();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual MINT32                  getOpenId() const   { return mOpenId; }

    virtual NodeId_T                getNodeId() const   { return mNodeId; }

    virtual char const*             getNodeName() const { return mNodeName.string(); }

public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
init(InitParams const& rParams)
{
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mOpenId  = rParams.openId;
        mNodeId  = rParams.nodeId;
        mNodeName= rParams.nodeName;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
uninit()
{
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mpOutMetaStreamInfo_Result = 0;
        mpInpMetaStreamInfo_Request = 0;
        mpInpImageStreamInfo_Yuv = 0;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
config(ConfigParams const& rParams)
{
    if  ( ! rParams.pvInMeta || 1 != rParams.pvInMeta->size() ) {
        return BAD_VALUE;
    }
    if  ( ! rParams.pvInImage || 1 != rParams.pvInImage->size() ) {
        return BAD_VALUE;
    }
    if  ( ! rParams.pvOutMeta || 1 != rParams.pvOutMeta->size() ) {
        return BAD_VALUE;
    }
    if  ( rParams.pvOutImage && 0 != rParams.pvOutImage->size() ) {
        return BAD_VALUE;
    }
    //
    //
    RWLock::AutoWLock _l(mConfigRWLock);
    //
    MetaStreamInfoSetT const&   rvInMeta = *rParams.pvInMeta;
    ImageStreamInfoSetT const&  rvInImage= *rParams.pvInImage;
    MetaStreamInfoSetT const&   rvOutMeta= *rParams.pvOutMeta;
    //
    mpOutMetaStreamInfo_Result  = rvOutMeta[0];
    mpInpMetaStreamInfo_Request = rvInMeta[0];
    mpInpImageStreamInfo_Yuv    = rvInImage[0];
    return OK;
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
queue(
    android::sp<IPipelineFrame> pFrame
)
{
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Make sure the request with a smaller frame number has a higher priority.
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= pFrame->getFrameNo() - (*it)->getFrameNo() ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }
    mRequestQueue.insert(it, pFrame);
    mRequestQueueCond.broadcast();
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
flush()
{
    MY_LOGW("Not Implement !!!");

    return INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Wait until the queue is not empty or this thread will exit.
    while ( mRequestQueue.empty() && ! exitPending() )
    {
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu, exitPending:%d",
                status, ::strerror(-status), mRequestQueue.size(), exitPending()
            );
        }
    }
    //
    if  ( exitPending() ) {
        MY_LOGW("[exitPending] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
DummyNode::
requestExit()
{
    MY_LOGW("Not Implement !!!");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
DummyNode::
readyToRun()
{
    ::prctl(PR_SET_NAME, (unsigned long)"Cam@DummyNode", 0, 0, 0);
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MY_LOGW("Not Implement !!! Need set thread priority & policy");
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DummyNode::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            OK == onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        onProcessFrame(pFrame);
    }


    MY_LOGW("Not Implement !!!");


    return  true;
}




/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
ensureMetaBufferAvailable_(
    MINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IMetaStreamBuffer>& rpStreamBuffer
)
{
    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGW("[frame:%d node:%d] null stream buffer:%s", frameNo, getNodeId(), rpStreamBuffer->getName());
        return NAME_NOT_FOUND;
    }
    //
    //  Wait acquire_fence.
    sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
    MERROR const err = acquire_fence->waitForever(getNodeName());
    MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
    //
    //
    //  Mark this buffer as ACQUIRED by this user.
    rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    //
    //  Check buffer status.
    if  ( 0 != rpStreamBuffer->getStatus() ) {
        MY_LOGE("[frame:%d node:%d][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyNode::
ensureImageBufferAvailable_(
    MINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IImageStreamBuffer>& rpStreamBuffer
)
{
    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGW("[frame:%d node:%d] null stream buffer:%s", frameNo, getNodeId(), rpStreamBuffer->getName());
        return NAME_NOT_FOUND;
    }
    //
    //  Wait acquire_fence.
    sp<IFence> acquire_fence = IFence::create(rpStreamBuffer->createAcquireFence(getNodeId()));
    MERROR const err = acquire_fence->waitForever(getNodeName());
    MY_LOGE_IF(OK != err, "[frame:%d node:%d][stream buffer:%s] fail to wait acquire_fence:%d[%s] err:%d[%s]", frameNo, getNodeId(), rpStreamBuffer->getName(), acquire_fence->getFd(), acquire_fence->name(), err, ::strerror(-err));
    //
    //  Mark this buffer as ACQUIRED by this user.
    rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::ACQUIRE);
    //
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        //  The producer ahead of this user may fail to render this buffer's content.
        MY_LOGE("[frame:%d node:%d][stream buffer:%s] bad status:%d", frameNo, getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DummyNode::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    StreamId_T const        streamIdOutMetaResult       = mpOutMetaStreamInfo_Result->getStreamId();
    sp<IMetaStreamBuffer>   pOutMetaStreamBufferResult  = NULL;
    IMetadata*              pOutMetadataResult          = NULL;
    //
    StreamId_T const        streamIdInpMetaRequest      = mpInpMetaStreamInfo_Request->getStreamId();
    sp<IMetaStreamBuffer>   pInpMetaStreamBufferRequest = NULL;
    IMetadata*              pInpMetadataRequest         = NULL;
    //
    StreamId_T const        streamIdInpImageYuv         = mpInpImageStreamInfo_Yuv->getStreamId();
    sp<IImageStreamBuffer>  pInpImageStreamBufferYuv    = NULL;
    sp<IImageBufferHeap>    pInpImageBufferHeapYuv      = NULL;
    sp<IImageBuffer>        pInpImageBufferYuv          = NULL;
    //
    IStreamInfoSet const& rStreamInfoSet= pFrame->getStreamInfoSet();
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //
    //
    ////////////////////////////////////////////////////////////////////////////
    //  Ensure buffers available.
    ////////////////////////////////////////////////////////////////////////////
    //  Output Meta Stream: Result
    {
        StreamId_T const streamId = streamIdOutMetaResult;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pOutMetaStreamBufferResult
        );
        //Should check the returned error code!!!
    }
    //
    //  Input Meta Stream: Request
    {
        StreamId_T const streamId = streamIdInpMetaRequest;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInpMetaStreamBufferRequest
        );
        //Should check the returned error code!!!
    }
    //
    //  Input Image Stream: YUV
    {
        StreamId_T const streamId = streamIdInpImageYuv;
        MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInpImageStreamBufferYuv
        );
        //Should check the returned error code!!!
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Prepare buffers before using.
    ////////////////////////////////////////////////////////////////////////////
    {
        //  Output Meta Stream: Result
        {
            pOutMetadataResult = pOutMetaStreamBufferResult->tryWriteLock(getNodeName());
        }
        //
        //  Input Meta Stream: Request
        {
            pInpMetadataRequest = pInpMetaStreamBufferRequest->tryReadLock(getNodeName());
        }
        //
        //  Input Image Stream: YUV
        {
            //  Query the group usage.
            MUINT const groupUsage = pInpImageStreamBufferYuv->queryGroupUsage(getNodeId());
            //
            pInpImageBufferHeapYuv = pInpImageStreamBufferYuv->tryReadLock(getNodeName());
            pInpImageBufferYuv = pInpImageBufferHeapYuv->createImageBuffer();
            pInpImageBufferYuv->lockBuf(getNodeName(), groupUsage);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Access buffers.
    ////////////////////////////////////////////////////////////////////////////
    {
        pOutMetadataResult;
        pInpMetadataRequest;
        pInpImageBufferYuv;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Release buffers after using.
    ////////////////////////////////////////////////////////////////////////////
    {
        //  Output Meta Stream: Result
        {
            StreamId_T const streamId = streamIdOutMetaResult;
            //
            //Buffer Producer must set this status.
            if  ( 1 ) {
                pOutMetaStreamBufferResult->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
            }
            else {
                pOutMetaStreamBufferResult->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
            }
            pOutMetaStreamBufferResult->unlock(getNodeName(), pOutMetadataResult);
            //
            //  Mark this buffer as USED by this user.
            //  Mark this buffer as RELEASE by this user.
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        //
        //  Input Meta Stream: Request
        {
            StreamId_T const streamId = streamIdInpMetaRequest;
            //
            pInpMetaStreamBufferRequest->unlock(getNodeName(), pInpMetadataRequest);
            //
            //  Mark this buffer as USED by this user.
            //  Mark this buffer as RELEASE by this user.
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        //
        //  Input Image Stream: YUV
        {
            StreamId_T const streamId = streamIdInpImageYuv;
            //
            pInpImageBufferYuv->unlockBuf(getNodeName());
            pInpImageStreamBufferYuv->unlock(getNodeName(), pInpImageBufferHeapYuv.get());
            //
            //  Mark this buffer as USED by this user.
            //  Mark this buffer as RELEASE by this user.
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DummyNode::
onFlushFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    IStreamInfoSet const& rStreamInfoSet= pFrame->getStreamInfoSet();
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

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

    //  Meta Stream Buffers
    for (size_t i = 0; i < rStreamInfoSet.getMetaInfoNum(); i++) {
        sp<IStreamInfo> pStreamInfo = rStreamInfoSet.getMetaInfoAt(i);
        StreamId_T const streamId = pStreamInfo->getStreamId();
        //
        //  Mark this buffer as RELEASE by this user.
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::RELEASE);
    }

    //  Image Stream Buffers
    for (size_t i = 0; i < rStreamInfoSet.getImageInfoNum(); i++) {
        sp<IStreamInfo> pStreamInfo = rStreamInfoSet.getImageInfoAt(i);
        StreamId_T const streamId = pStreamInfo->getStreamId();
        //
        //  Mark this buffer as RELEASE by this user.
        rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::RELEASE);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
}
#endif
