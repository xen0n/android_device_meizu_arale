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

#define LOG_TAG "MtkCam/ResourceMgr"
//
#include "MyUtils.h"
#include <utils/Mutex.h>
#include <mtkcam/common.h>
#include <mtkcam/v3/hwnode/P1Node.h>
#include <mtkcam/v3/hwnode/JpegNode.h>
#include <mtkcam/v3/hwpipeline/NodeId.h>
#include <mtkcam/v3/hwpipeline/StreamId.h>
#include "ScenarioControl.h"
#include <mtkcam/v3/pipeline/IPipelineModel.h>
#include "PipelineResourceMgr.h"


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

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

/******************************************************************************
 *
 ******************************************************************************/
static DefaultKeyedVector< MINT32, wp<PipelineResourceMgr> >
                                gMgrMap;
static Mutex                    gLock;
/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineResourceMgr>
IPipelineResourceMgr::
create(MINT32 const openId)
{
    Mutex::Autolock _l(gLock);
    sp<PipelineResourceMgr> pSingleton = gMgrMap.valueFor(openId).promote();
    if ( ! pSingleton.get() ) {
        pSingleton = new PipelineResourceMgr(openId);
        gMgrMap.add(openId, pSingleton);
    } else {
        MY_LOGD("Pipelineresourcemgr singleton got");
    }
    return pSingleton;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<PipelineResourceMgr>
PipelineResourceMgr::
getInstance(MINT32 const openId)
{
    Mutex::Autolock _l(gLock);
    return gMgrMap.valueFor(openId).promote();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineResourceMgr::
PipelineResourceMgr(MINT32 const openId)
    : mLock()
    , mName("ResourceMgr")
    , mOpenId(openId)
    , mvReusableNodes()
    , mvReusableStreams()
    , mvNodeStatus()
    , mvStreamStatus()
    , mvStreamInfo()
    , mHalImageStreamBufferPoolMap()
    , mpScenarioCtrl()
    , mSceneStatus()
    , mSceneCtrlParams()
    , mpNode_P1()
    , mpNode_Jpeg()
    , mvInitParams()
    , mConfigParams_P1()
    , mInitParams_Jpeg()
    , mConfigParams_Jpeg()
{
    MY_LOGD("Create Pipelineresourcemgr instance id: %d", openId);
}

/******************************************************************************
 *
 ******************************************************************************/
void
PipelineResourceMgr::
onLastStrongRef(const void* /*id*/)
{
    Mutex::Autolock _l(mLock);
    onDestroyLocked();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineResourceMgr::
onDestroyLocked()
{
    MY_LOGD("Destroy Pipelineresourcemgr instance onDestroyLocked");
    for ( size_t i = 0; i < mvReusableNodes.size(); i++ ) {
        MUINT const nodeid = mvReusableNodes.keyAt(i);
        switch ( nodeid ) {
            case eNODEID_P1Node:
            {
                mpNode_P1->uninit();
    mpNode_P1   = NULL;
            }
            break;
            case eNODEID_JpegNode:
            {
                mpNode_Jpeg->uninit();
    mpNode_Jpeg = NULL;
            }
            break;
            default:
            {
                MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
                return;
            }
            break;
        }
    }
    mvReusableNodes.clear();
    //
    for ( size_t i = 0; i < mHalImageStreamBufferPoolMap.size(); i++ )
    {
        MUINT32 streamid = mHalImageStreamBufferPoolMap.keyAt(i);
        sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.editValueAt(i);
        pPool->uninitPool(LOG_TAG);
        pPool = NULL;
    }
    mHalImageStreamBufferPoolMap.clear();
    mvReusableStreams.clear();
    //
    mpScenarioCtrl = NULL;
#if 0
    gpSingleton = NULL;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
PipelineResourceMgr::
~PipelineResourceMgr()
{
    MY_LOGD("Destroy Pipelineresourcemgr instance");
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
getReusableNodes(
    Vector<NodeId_T> &vNodeList
)
{
    Mutex::Autolock _l(mLock);
    //
    vNodeList.setCapacity(mvReusableNodes.size());
    for ( size_t i=0; i<mvReusableNodes.size(); i++)
        vNodeList.add(mvReusableNodes.keyAt(i));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
getReusableStreams(
    Vector<StreamId_T> &vStreamList
)
{
    Mutex::Autolock _l(mLock);
    //
    vStreamList.setCapacity(mvReusableStreams.size());
    for ( size_t i=0; i<mvReusableStreams.size(); i++)
        vStreamList.add(mvReusableStreams.keyAt(i));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
template <class NodeT>
MERROR
PipelineResourceMgr::
checkInitParams(
    sp<NodeT> pNode,
    InitParams const& rParams
)
{
    if ( !pNode.get() )
        return MTRUE;
    MBOOL bRet = MFALSE;
    //
    if ( rParams.openId   != pNode->getOpenId() ||
         rParams.nodeId   != pNode->getNodeId() ||
         strcmp(rParams.nodeName, pNode->getNodeName()) )
        bRet = MTRUE;

    if ( bRet ) {
        MY_LOGW_IF(1, "Need to reconfigure of node: openId(%d:%d) nodeId(%d:%d) nodeName(%s:%s)(%p:%p)",
                      rParams.openId, pNode->getOpenId(),
                      rParams.nodeId, pNode->getNodeId(),
                      rParams.nodeName, pNode->getNodeName(),
                      rParams.nodeName, pNode->getNodeName()
                      );
    } else {
        MY_LOGD_IF(0, "No need to reconfigure of node: openId(%d:%d) nodeId(%d:%d) nodeName(%s:%s)(%p:%p)",
                      rParams.openId, pNode->getOpenId(),
                      rParams.nodeId, pNode->getNodeId(),
                      rParams.nodeName, pNode->getNodeName(),
                      rParams.nodeName, pNode->getNodeName()
                      );
    }

    return bRet;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
queryNode(
    NodeId_T nodeid,
    sp<IPipelineNode>& pNode,
    InitParams const& rParams
)
{
    Mutex::Autolock _l(mLock);
    ssize_t index = mvNodeStatus.indexOfKey(nodeid);
    if ( index < 0 ) {
        mvNodeStatus.add(nodeid, NodeStatus::UNINIT);   // or in initialization?
        index = mvNodeStatus.indexOfKey(nodeid);
    }
    //
    MERROR err = OK;
    //
    if ( NodeStatus::UNINIT == mvNodeStatus.valueAt(index) ) {
        // create nodes
        err = createNode(nodeid, rParams);
        err |= updateInitParams(nodeid, rParams);
        mvNodeStatus.replaceValueAt(index, NodeStatus::UNCONFIG);
    }
    else {
        // check params
        MBOOL bReconfigure = MFALSE;
        switch ( nodeid ) {
            case eNODEID_P1Node:
            {
                //P1Node::InitParams *params = static_cast<P1Node::InitParams*>(pInitParams);
                bReconfigure = checkInitParams(mpNode_P1, rParams);
            }
            break;
/*.................................................................................................
    FIX ME: YOU MUST add node if you push node into ResourceMgr
    .................................................................................................*/
/*            case eNODEID_P2Node:
            {
                P2Node::InitParams *params = static_cast<P2Node::InitParams*>(pInitParams);
            }
            break;
            case eNODEID_P2Node_VSS:
            {
                P2Node::InitParams *params = static_cast<P2Node::InitParams*>(pInitParams);
            }
            break;
            case eNODEID_FDNode:
            {
                FdNode::InitParams *params = static_cast<FdNode::InitParams*>(pInitParams);
            }
            break;*/
            case eNODEID_JpegNode:
            {
                //JpegNode::InitParams *params = static_cast<JpegNode::InitParams*>(pInitParams);
                bReconfigure = checkInitParams(mpNode_Jpeg, rParams);
            }
            break;
            default:
            {
                MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
                return -EINVAL;
            }
            break;
        }
        if ( bReconfigure ) {
            err |= updateInitParams(nodeid, rParams);
            // do something?
            mvNodeStatus.replaceValueFor(nodeid, NodeStatus::RECONFIG);
            MY_LOGD("NodeId(%#"PRIxPTR") needs to reconfigure", nodeid);
        }
    }
    //
    switch ( nodeid ) {
        case eNODEID_P1Node:
        {
            pNode = mpNode_P1;
        }
        break;
        case eNODEID_JpegNode:
        {
            pNode = mpNode_Jpeg;
        }
        break;
        default:
        {
            MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
            return -EINVAL;
        }
        break;
    }
    //
    if ( 0 > mvReusableNodes.indexOfKey(nodeid) )
        mvReusableNodes.add(nodeid, pNode);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<PipelineResourceMgr::HalImageStreamBufferPoolT>
PipelineResourceMgr::
queryStream(
    StreamId_T streamid,
    android::sp<IImageStreamInfo>&        pStreamInfo
)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD_IF( pStreamInfo.get(), "streamId:%#"PRIxPTR" %s %p type:%d Size(%d,%d) MaxBufSize:%d MinInitBufSize:%d",
                    pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pStreamInfo.get(),
                    pStreamInfo->getStreamType(), pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h,
                    pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum() );
    //
    ssize_t index = mvStreamStatus.indexOfKey(streamid);
    if ( index < 0 ) {
        mvStreamStatus.add(streamid, StreamStatus::UNINIT);   // or in initialization?
        index = mvStreamStatus.indexOfKey(streamid);
    }
    MY_LOGD("operate on streampool index(%d) size(%d)", index, mvStreamStatus.size());

    //
    MERROR err = OK;
    if ( StreamStatus::UNINIT == mvStreamStatus.valueAt(index) ) {
        MY_LOGD("Create new halimage stream id:%#"PRIxPTR" ", streamid);
        err = createStream(streamid, pStreamInfo);
        if ( OK != err )
            return NULL;
        MY_LOGD("After create pool(%p)", mHalImageStreamBufferPoolMap.valueAt(index).get());
        mvStreamInfo.add(streamid, pStreamInfo);
        mvReusableStreams.add(streamid, pStreamInfo);
        mvStreamStatus.replaceValueAt(index, StreamStatus::INITIALIZED);
        return mHalImageStreamBufferPoolMap.editValueAt(index);
    }
    //
    //if ( StreamStatus::RECONFIG == mvStreamStatus.valueAt(index) )
    // check whole image streaminfo
    MBOOL bReconfigure = MFALSE;
    if ( strcmp( pStreamInfo->getStreamName(),  mvStreamInfo[index]->getStreamName() )      ||
         pStreamInfo->getStreamId()          != mvStreamInfo[index]->getStreamId()          ||
         pStreamInfo->getStreamType()        != mvStreamInfo[index]->getStreamType()        ||
         pStreamInfo->getMaxBufNum()         != mvStreamInfo[index]->getMaxBufNum()         ||
         pStreamInfo->getMinInitBufNum()     != mvStreamInfo[index]->getMinInitBufNum()     ||
         pStreamInfo->getUsageForConsumer()  != mvStreamInfo[index]->getUsageForConsumer()  ||
         pStreamInfo->getUsageForAllocator() != mvStreamInfo[index]->getUsageForAllocator() ||
         pStreamInfo->getImgFormat()         != mvStreamInfo[index]->getImgFormat()         ||
         pStreamInfo->getImgSize()           != mvStreamInfo[index]->getImgSize()           ||
         pStreamInfo->getTransform()         != mvStreamInfo[index]->getTransform()
       ) {
       bReconfigure = MTRUE;
       mvStreamInfo.replaceValueAt(index, pStreamInfo);
    }

    if ( bReconfigure ) {
        MY_LOGW("Need to reconfigure stream here!");
        mpNode_P1->flush();
        MY_LOGW("P1 flush done!");
        sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.editValueFor(streamid);
        pPool->uninitPool(LOG_TAG);
        err = createStream(streamid, pStreamInfo);
        if ( err != OK )
            return NULL;
        mvNodeStatus.replaceValueAt(0, NodeStatus::RECONFIG);
    }

/*
    switch (streamid)
    {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00:
        {
            mpP1_Raw = pStreamInfo;
        }
        break;
        case eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00:
        {
            mpP1_RawResized = pStreamInfo;
        }
        break;
        default:
            return NULL;
        break;
    }
*/
    mvStreamStatus.replaceValueAt(index, StreamStatus::INITIALIZED);
    return mHalImageStreamBufferPoolMap.editValueAt(index);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
acquireHalStreamBuffer(
    android::sp<IImageStreamInfo> const   pStreamInfo,
    android::sp<HalImageStreamBuffer>&    rpStreamBuffer
)   const
{
    Mutex::Autolock _l(mLock);
    StreamId_T const streamId = pStreamInfo->getStreamId();
    sp<HalImageStreamBufferPoolT> pPool = mHalImageStreamBufferPoolMap.valueFor(streamId);
    //
    MY_LOGE_IF(pPool == 0, "NULL HalImageStreamBufferPool - stream:%#"PRIxPTR"(%s)", streamId, pStreamInfo->getStreamName());
    MERROR err = pPool == 0 ? UNKNOWN_ERROR : pPool->acquireFromPool(__FUNCTION__, rpStreamBuffer, ::s2ns(10));
    MY_LOGA_IF(
        OK!=err || rpStreamBuffer==0,
        "[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#"PRIxPTR"(%s)",
        err, ::strerror(-err), rpStreamBuffer.get(), streamId, pStreamInfo->getStreamName()
    );
    //MY_LOGD("acquire %p from pool %p", rpStreamBuffer.get(), pPool.get());
    return err;

}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
createNode(
    NodeId_T nodeid,
    InitParams const& rParams
)
{
    switch (nodeid)
    {
        case eNODEID_P1Node:
        {
            mpNode_P1 = P1Node::createInstance();
            if ( ! mpNode_P1.get() )
                MY_LOGE("i want to create p1!");
            mpNode_P1->init(rParams);
        }
        break;
        case eNODEID_P2Node:
            //
        break;
        case eNODEID_JpegNode:
        {
            mpNode_Jpeg = JpegNode::createInstance();
            if ( ! mpNode_Jpeg.get() )
                MY_LOGE("i want to create jpeg!");
            mpNode_Jpeg->init(rParams);
        }
        break;
        default:
            return -EINVAL;
        break;
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
createStream(
    StreamId_T streamid,
    android::sp<IImageStreamInfo>&        pStreamInfo
)
{
    //
    String8 const name = String8::format("%s:%s", getName(), pStreamInfo->getStreamName());
    //
    IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
    }
    IImageBufferAllocator::ImgParam const imgParam(
        pStreamInfo->getImgFormat(),
        pStreamInfo->getImgSize(),
        bufStridesInBytes, bufBoundaryInBytes,
        bufPlanes.size()
    );
    sp<HalImageStreamBufferPoolT>
    pPool = new HalImageStreamBufferPoolT(
        name.string(),
        HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
    );
    //
    MY_LOGD("init pool");
    if  ( pPool == 0 ) {
        MY_LOGE("Fail to new a image pool:%s", name.string());
        return -EINVAL;
    }
    //
    MERROR err = pPool->initPool(getName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if  ( OK != err ) {
        MY_LOGE("%s: initPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return -EINVAL;
    }
    if  ( OK != pPool->commitPool(getName()) ) {
        MY_LOGE("%s: commitPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return -EINVAL;
    }
    //
    mHalImageStreamBufferPoolMap.add(pStreamInfo->getStreamId(), pPool);
    //return pPool;
    MY_LOGD("add to pool map %d(%p)", mHalImageStreamBufferPoolMap.size(), pPool.get());

    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
updateInitParams(
    NodeId_T nodeid,
    InitParams const& rParams
)
{
    //Mutex::Autolock _l(mLock);
    //
    switch ( nodeid ) {
        case eNODEID_P1Node:
        case eNODEID_P2Node:
        case eNODEID_P2Node_VSS:
        case eNODEID_FDNode:
        case eNODEID_JpegNode:
        {
            ssize_t const index = mvInitParams.indexOfKey(rParams.nodeId);
            if ( index >= 0 ) {
                InitParams param;
                param.openId   = rParams.openId;
                param.nodeId   = rParams.nodeId;
                param.nodeName = rParams.nodeName;
                mvInitParams.replaceValueAt(index, param);
                MY_LOGD("Modify init param of nodeid(%d) openid(%d) nodename(%p:%s)",
                         param.nodeId, param.openId, param.nodeName, param.nodeName);
            } else {
                MY_LOGD("Base new init param of nodeid(%d) openid(%d) nodename(%p:%s)",
                         rParams.nodeId, rParams.openId, rParams.nodeName, rParams.nodeName);
                InitParams param;
                param.openId   = rParams.openId;
                param.nodeId   = rParams.nodeId;
                param.nodeName = rParams.nodeName;
                mvInitParams.add(param.nodeId, param);
                MY_LOGD("ADD new init param of nodeid(%d) openid(%d) nodename(%p:%s)",
                         param.nodeId, param.openId, param.nodeName, param.nodeName);
            }
        }
        break;
        default:
        {
            MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
            return -EINVAL;
        }
        break;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
updateConfigParams_P1(
    NodeId_T nodeid,
    P1Node::ConfigParams* params
)
{
    //Mutex::Autolock _l(mLock);
    if ( !params || nodeid!=eNODEID_P1Node )
        return -EINVAL;
    //
    if ( ! isEqualMetaStreamInfo(mConfigParams_P1.pInAppMeta, params->pInAppMeta) ) {
        MY_LOGD("updata config param InAppMeta");
        mConfigParams_P1.pInAppMeta = params->pInAppMeta;
    }
    if ( ! isEqualMetaStreamInfo(mConfigParams_P1.pInHalMeta, params->pInHalMeta) ) {
        MY_LOGD("updata config param InHalMeta");
        mConfigParams_P1.pInHalMeta = params->pInHalMeta;
    }
    if ( ! isEqualMetaStreamInfo(mConfigParams_P1.pOutAppMeta, params->pOutAppMeta) ) {
        MY_LOGD("updata config param OutAppMeta");
        mConfigParams_P1.pOutAppMeta= params->pOutAppMeta;
    }
    if ( ! isEqualMetaStreamInfo(mConfigParams_P1.pOutHalMeta, params->pOutHalMeta) ) {
        MY_LOGD("updata config param OutHalMeta");
        mConfigParams_P1.pOutHalMeta= params->pOutHalMeta;
    }
    if ( ! isEqualImageStreamInfo(mConfigParams_P1.pOutImage_resizer, params->pOutImage_resizer) ) {
        MY_LOGD("updata config param resizer");
        mConfigParams_P1.pOutImage_resizer = params->pOutImage_resizer;
    }
    MY_LOGD("updata config param full raw-%d", params->pvOutImage_full.size());
    mConfigParams_P1.pvOutImage_full.clear();
    if ( params->pvOutImage_full.size() ) {
        mConfigParams_P1.pvOutImage_full.setCapacity(params->pvOutImage_full.size());
        for ( size_t i=0; i<params->pvOutImage_full.size(); i++ ) {
            mConfigParams_P1.pvOutImage_full.add( params->pvOutImage_full[i] );
        }
    }
    if ( params->pvOutImage_full.size() ) {
        mConfigParams_P1.pStreamPool_full    = params->pStreamPool_full;
        MY_LOGD("updata config param full raw pool");
    } else {
        mConfigParams_P1.pStreamPool_full = NULL;
    }
    if ( params->pOutImage_resizer.get() ) {
        mConfigParams_P1.pStreamPool_resizer = params->pStreamPool_resizer;
        MY_LOGD("updata config param resizer raw pool");
    } else{
        mConfigParams_P1.pStreamPool_resizer = NULL;
    }
    if ( mConfigParams_P1.sensorParams.mode != params->sensorParams.mode ) {
        MY_LOGD("updata sensor param mode");
        mConfigParams_P1.sensorParams.mode = params->sensorParams.mode;
    }
    if ( mConfigParams_P1.sensorParams.size != params->sensorParams.size ) {
        MY_LOGD("updata sensor param size");
        mConfigParams_P1.sensorParams.size = params->sensorParams.size;
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
updateConfigParams_Jpeg(
    NodeId_T nodeid,
    JpegNode::ConfigParams* params
)
{
    //Mutex::Autolock _l(mLock);
    if ( !params || nodeid!=eNODEID_JpegNode )
        return -EINVAL;
    //
    mConfigParams_Jpeg.pInAppMeta = params->pInAppMeta;
    mConfigParams_Jpeg.pInHalMeta = params->pInHalMeta;
    mConfigParams_Jpeg.pOutAppMeta= params->pOutAppMeta;
    mConfigParams_Jpeg.pInYuv_Main = params->pInYuv_Main;
    mConfigParams_Jpeg.pInYuv_Thumbnail = params->pInYuv_Thumbnail;
    mConfigParams_Jpeg.pOutJpeg = params->pOutJpeg;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineResourceMgr::
isEqualImageStreamInfo(
    sp<IImageStreamInfo> pStreamInfo1,
    sp<IImageStreamInfo> pStreamInfo2
)
{
    MBOOL bRet = MTRUE;
    if ( !pStreamInfo1.get() || !pStreamInfo2.get() )
        return MFALSE;
    //
    if ( pStreamInfo1->getStreamId()  != pStreamInfo2->getStreamId() ||
         pStreamInfo1->getImgFormat() != pStreamInfo2->getImgFormat() ||
         pStreamInfo1->getImgSize()   != pStreamInfo2->getImgSize() ||
         pStreamInfo1->getBufPlanes().itemAt(0).rowStrideInBytes   != pStreamInfo2->getBufPlanes().itemAt(0).rowStrideInBytes
    )
        return MFALSE;
    //
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineResourceMgr::
isEqualMetaStreamInfo(
    sp<IMetaStreamInfo> pStreamInfo1,
    sp<IMetaStreamInfo> pStreamInfo2
)
{
    if ( !pStreamInfo1.get() || !pStreamInfo2.get() )
        return MFALSE;
    //
    if ( pStreamInfo1->getStreamId() != pStreamInfo2->getStreamId() )
        return MFALSE;
    //
    return MTRUE;
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
checkConfigParams_P1(
    NodeId_T nodeid,
    P1Node::ConfigParams* params
)
{
    //Mutex::Autolock _l(mLock);
    if ( !params || nodeid!=eNODEID_P1Node )
        return -EINVAL;
    //
    MY_LOGD("full(%d) resizer(%p)", params->pvOutImage_full.size(), params->pOutImage_resizer.get());
    //
    if ( ! isEqualMetaStreamInfo(mConfigParams_P1.pInAppMeta, params->pInAppMeta) ||
         ! isEqualMetaStreamInfo(mConfigParams_P1.pInHalMeta, params->pInHalMeta) ||
         ! isEqualMetaStreamInfo(mConfigParams_P1.pOutAppMeta, params->pOutAppMeta) ||
         ! isEqualMetaStreamInfo(mConfigParams_P1.pOutHalMeta, params->pOutHalMeta) ||
         //
         ! isEqualImageStreamInfo(mConfigParams_P1.pOutImage_resizer, params->pOutImage_resizer) ||
         //isEqualImageStreamInfo(mConfigParams_P1.pvOutImage_full[0], params->pvOutImage_full[0]) &&
         //
         mConfigParams_P1.sensorParams.mode != params->sensorParams.mode ||
         mConfigParams_P1.sensorParams.size != params->sensorParams.size )
        return MTRUE;
    //
    if ( params->pvOutImage_full.size()==0 && mConfigParams_P1.pvOutImage_full.size()>0 )
        return MTRUE;
    //
    for ( size_t i=0; i<params->pvOutImage_full.size(); i++ ) {
        StreamId_T const streamid = params->pvOutImage_full[i]->getStreamId();
        MY_LOGD("check full raw streamid:%d @ %d", streamid, i);
        MBOOL found = MFALSE;
        for ( size_t j=0; j<mConfigParams_P1.pvOutImage_full.size(); j++ ) {
            if ( streamid == mConfigParams_P1.pvOutImage_full[j]->getStreamId() ) {
                MY_LOGD("got previous full raw streamid:%d @ %d", streamid, j);
                found = MTRUE;
                if ( ! isEqualImageStreamInfo(mConfigParams_P1.pvOutImage_full[j], params->pvOutImage_full[i]) )
                    return MTRUE;
            }
        }
        if ( !found )
            return MTRUE;
    }
    //
    return MFALSE;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
checkConfigParams_Jpeg(
    NodeId_T nodeid,
    JpegNode::ConfigParams* params
)
{
    //Mutex::Autolock _l(mLock);
    if ( !params || nodeid!=eNODEID_JpegNode )
        return -EINVAL;
    //
    MBOOL bRet = MFALSE;

    if ( mConfigParams_Jpeg.pInAppMeta.get()         != params->pInAppMeta.get() ||
         mConfigParams_Jpeg.pInHalMeta.get()         != params->pInHalMeta.get() ||
         mConfigParams_Jpeg.pOutAppMeta.get()        != params->pOutAppMeta.get() ||
         mConfigParams_Jpeg.pInYuv_Main.get()        != params->pInYuv_Main.get() ||
         mConfigParams_Jpeg.pInYuv_Thumbnail.get()   != params->pInYuv_Thumbnail.get() ||
         mConfigParams_Jpeg.pOutJpeg.get()           != params->pOutJpeg.get()
    )
        bRet = MTRUE;
    //
    return bRet;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
evaluateNodeReconfigure(
    NodeId_T nodeid,
    MVOID *pConfigParam
)
{
    //Mutex::Autolock _l(mLock);
    ssize_t index = mvNodeStatus.indexOfKey(nodeid);
    if ( index < 0 ) {
        MY_LOGE("Node(%id) must be add to status map", nodeid);
    }
    //
    MERROR err = OK;

    if ( NodeStatus::UNCONFIG == mvNodeStatus.valueAt(index) )
    {
        switch ( nodeid ) {
            case eNODEID_P1Node:
            {
                P1Node::ConfigParams *params = static_cast<P1Node::ConfigParams*>(pConfigParam);
                err = mpNode_P1->config(*params);
                if ( OK == err ) {
                    updateConfigParams_P1(nodeid, params);
                    mvNodeStatus.replaceValueAt(index, NodeStatus::INITIALIZED);
                    MY_LOGD("Config node success: %d", mvNodeStatus.valueAt(index));
                }
                return err;
            }
            break;
        /*.................................................................................................
            FIX ME: YOU MUST add node if you push node into ResourceMgr
            .................................................................................................*/
        /*            case eNODEID_P2Node:
                    {
                        P2Node::InitParams *params = static_cast<P2Node::InitParams*>(pInitParams);
                    }
                    break;
                    case eNODEID_P2Node_VSS:
                    {
                        P2Node::InitParams *params = static_cast<P2Node::InitParams*>(pInitParams);
                    }
                    break;
                    case eNODEID_FDNode:
                    {
                        FdNode::InitParams *params = static_cast<FdNode::InitParams*>(pInitParams);
                    }
                    break;*/
            case eNODEID_JpegNode:
            {
                JpegNode::ConfigParams *params = static_cast<JpegNode::ConfigParams*>(pConfigParam);
                err = mpNode_Jpeg->config(*params);
                if ( OK == err ) {
                    updateConfigParams_Jpeg(nodeid, params);
                    mvNodeStatus.replaceValueAt(index, NodeStatus::INITIALIZED);
                    MY_LOGD("Config node success: %d", mvNodeStatus.valueAt(index));
                }
                return err;

            }
            break;
            default:
            {
                MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
                return -EINVAL;
            }
            break;
        }
    }    else if ( NodeStatus::UNINIT == mvNodeStatus.valueAt(index) ) {
        MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
        return -EINVAL;
    } else {    // RECONFIG or INITIALIZED
        MBOOL bReconfigure = MFALSE;
        switch ( nodeid ) {
            case eNODEID_P1Node:
            {
                P1Node::ConfigParams *params = static_cast<P1Node::ConfigParams*>(pConfigParam);
                bReconfigure = checkConfigParams_P1(nodeid, params);
                if ( bReconfigure ) {
                    updateConfigParams_P1(nodeid, params);
                    MY_LOGD("Update config params of P1");
                }
            }
            break;
            case eNODEID_JpegNode:
            {
                JpegNode::ConfigParams *params = static_cast<JpegNode::ConfigParams*>(pConfigParam);
                MY_LOGW("[Todo] Update config params of JpegNode");
                //bReconfigure = checkConfigParams_Jpeg(nodeid, params);
                if ( bReconfigure ) {
                    updateConfigParams_Jpeg(nodeid, params);
                    MY_LOGD("Update config params of JpegNode");
                }
            }
            break;
            default:
            {
                MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeid);
                return -EINVAL;
            }
            break;
        }
        //
        if ( bReconfigure || mvNodeStatus.valueAt(index)==NodeStatus::RECONFIG || mSceneStatus==SceneStatus::RECONFIG) {
            // FIX ME: different nodes implementation
            if ( mpNode_P1.get() ) {
                if ( ! mpNode_P1.get() )
                    MY_LOGF("Need to reconfigure of p1 node but w/o sp");
                //
                MY_LOGD("Need to reconfigure of p1 node(%p)", mpNode_P1.get());
                // wait drained
                mpNode_P1->flush();
                mpNode_P1->uninit();
                //
                MINT32 const index = mvInitParams.indexOfKey(nodeid);
                InitParams initParams;
                if ( index >= 0 ) {
                    initParams = mvInitParams.valueAt(index);
                }
                for( size_t i=0; i<mHalImageStreamBufferPoolMap.size(); i++ ) {
                    //sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.valueAt(i);
#if 1
                    StreamId_T const streamid = mHalImageStreamBufferPoolMap.keyAt(i);
                    MBOOL bHasStream = MFALSE;
                    if ( mConfigParams_P1.pOutImage_resizer->getStreamId() == streamid )
                        bHasStream = MTRUE;
                    for( size_t i=0; i<mConfigParams_P1.pvOutImage_full.size(); i++ ) {
                        if ( mConfigParams_P1.pvOutImage_full[i]->getStreamId() == streamid )
                            bHasStream = MTRUE;
                    }
                    //
                    MY_LOGD("check stream is in new ConfigParams %d", bHasStream);
                    if ( !bHasStream ) {
                        MY_LOGD("We should remove this pool from resourcemgr (%d)", streamid);
                        sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.editValueFor(streamid);
                        pPool->uninitPool(LOG_TAG);
                        mvStreamStatus.replaceValueFor(streamid, StreamStatus::UNINIT);
                    }
#else
                    StreamId_T const streamid = mHalImageStreamBufferPoolMap.keyAt(i);
                    if ( StreamStatus::RECONFIG== mvStreamStatus.valueAt(i) ) {
                        MY_LOGD("We should remove this pool from resourcemgr (%d)", streamid);
                        sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.editValueFor(streamid);
                        pPool->uninitPool(LOG_TAG);
                        mvStreamStatus.replaceValueFor(streamid, StreamStatus::UNINIT);
                    } else {
                        mvStreamStatus.replaceValueFor(streamid, StreamStatus::RECONFIG);
                    }
#endif
                }
                //
                if ( SceneStatus::RECONFIG == mSceneStatus) {
                    mpScenarioCtrl->setScenario((MVOID*)&mSceneCtrlParams);
                    mSceneStatus = SceneStatus::INITIALIZED;
                }
                //
                mpNode_P1->init(initParams);
                P1Node::ConfigParams *params = static_cast<P1Node::ConfigParams*>(pConfigParam);
                mpNode_P1->config(*params);
                // do something
                mvNodeStatus.replaceValueFor(nodeid, NodeStatus::INITIALIZED);
            } else {
                MY_LOGD("Need to reconfigure of p1 node(%p) but w/o sp", mpNode_P1.get());
            }
        } else {
            MY_LOGD("No need to reconfigure of p1 node");
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
setScenarioCtrl(
    MVOID* pBWCparam
)
{
    Mutex::Autolock _l(mLock);
    //
    if ( ! mpScenarioCtrl.get() )
        mpScenarioCtrl = ScenarioControl::create(mOpenId);
    //
    ScenarioControl::BWC_Scenario_Param *bwcparam = static_cast<ScenarioControl::BWC_Scenario_Param*>(pBWCparam);
    if ( SceneStatus::UNINIT == mSceneStatus && pBWCparam) {
        mSceneCtrlParams.scenario = bwcparam->scenario;
        mSceneCtrlParams.sensorSize = bwcparam->sensorSize;
        mSceneCtrlParams.sensorFps = bwcparam->sensorFps;
        MY_LOGD("Set senario ctrl: scenario(%d) sensorSize(%dx%d) sensorfps(%d)",
                bwcparam->scenario, bwcparam->sensorSize.w, bwcparam->sensorSize.h, bwcparam->sensorFps);
        mpScenarioCtrl->setScenario(pBWCparam);
        mSceneStatus = SceneStatus::INITIALIZED;
    } else if ( mSceneCtrlParams.scenario   != bwcparam->scenario   ||
                mSceneCtrlParams.sensorSize != bwcparam->sensorSize ||
                mSceneCtrlParams.sensorFps  != bwcparam->sensorFps) {
        mSceneCtrlParams.scenario = bwcparam->scenario;
        mSceneCtrlParams.sensorSize = bwcparam->sensorSize;
        mSceneCtrlParams.sensorFps = bwcparam->sensorFps;
        //
        MY_LOGD("Update senario ctrl: scenario(%d) sensorSize(%dx%d) sensorfps(%d)",
                bwcparam->scenario, bwcparam->sensorSize.w, bwcparam->sensorSize.h, bwcparam->sensorFps);
        mSceneStatus = SceneStatus::RECONFIG;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineResourceMgr::
clear()
{

    return OK;
}


