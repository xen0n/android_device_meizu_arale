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

#define LOG_TAG "MtkCam/HwPipeline"
//
#include "MyUtils.h"
#include <mtkcam/v3/pipeline/DefaultPipelineModel/DefaultPipelineModel.h>
//
#include <mtkcam/v3/hwpipeline/PipelineModel_Default.h>
#include <mtkcam/v3/hwpipeline/InFlightRequest.h>
#include <mtkcam/v3/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/v3/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/v3/hwnode/P1Node.h>
#include <mtkcam/v3/hwnode/P2Node.h>
#include <mtkcam/v3/hwnode/FDNode.h>
#include <mtkcam/v3/hwnode/JpegNode.h>
//
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
//
#include "ScenarioControl.h"
#include "PipelineResourceMgr.h"
//

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using NSCam::v3::Utils::MetaStreamInfo;
using NSCam::v3::Utils::ImageStreamInfo;

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
#define SUPPORT_RECONFIGURE         (0)

#define ALIGN_2(x)     (((x) + 1) & (~1))

static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}
// utility functions
static
sp<ImageStreamInfo> createImageStreamInfo(
                        char const*         streamName,
                        StreamId_T          streamId,
                        MUINT32             streamType,
                        size_t              maxBufNum,
                        size_t              minInitBufNum,
                        MUINT               usageForAllocator,
                        MINT                imgFormat,
                        MSize const&        imgSize
                    );

/******************************************************************************
 *
 ******************************************************************************/
namespace {
class PipelineDefaultImp
    : public PipelineModel_Default
    , public DefaultPipelineModel
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    static MUINT32 const            mMaxNumNodes = 5;

    struct ConfigProfile
    {
        // node to config
        MBOOL                       mbConfigP1;
        MBOOL                       mbConfigP2;
        MBOOL                       mbConfigP2VSS;
        MBOOL                       mbConfigFD;
        MBOOL                       mbConfigJpeg;
        //
        MSize                       mSensorSize;
        //  p1 related setting
        MSize                       mFullSize;
        MINT                        mFullFormat;
        MSize                       mResizedSize;
        MINT                        mResizedFormat;
        //
                                    ConfigProfile()
                                        : mbConfigP1(MFALSE)
                                        , mbConfigP2(MFALSE)
                                        , mbConfigP2VSS(MFALSE)
                                        , mbConfigFD(MFALSE)
                                        , mbConfigJpeg(MFALSE)
                                        //
                                    {}
    };

    struct SensorParam
    {
        MUINT                       mode;
        MSize                       size;
        MUINT                       fps;
        MUINT                       id;
    };

    enum {
        eWait_None,
        eWait_1080pJpeg_Request,
        eWait_Reprocessing_Request
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.

    ConfigProfile                   mConfigProfile;

    // Nodes
    android::sp<IPipelineNode>             mpNode_P1;
    android::sp<P2Node>             mpNode_P2;
    android::sp<P2Node>             mpNode_P2_VSS;
    android::sp<FdNode>             mpNode_FD;
    android::sp<JpegNode>           mpNode_Jpeg;
    //
    // meta: hal
    android::sp<IMetaStreamInfo>    mpHalMeta_Control;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP2;
    // meta: app
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP1;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP2;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicFD;
    android::sp<IMetaStreamInfo>    mpAppMeta_DynamicJpeg;
    android::sp<IMetaStreamInfo>    mpAppMeta_Control;
    // image: hal
    android::sp<IImageStreamInfo>   mpHalImage_P1_Raw;
    android::sp<IImageStreamInfo>   mpHalImage_P1_ResizerRaw;
    android::sp<IImageStreamInfo>   mpHalImage_FD_YUV;
    android::sp<IImageStreamInfo>   mpHalImage_Jpeg_YUV;
    android::sp<IImageStreamInfo>   mpHalImage_Thumbnail_YUV;
    // image: app
    android::KeyedVector <
            StreamId_T, android::sp<IImageStreamInfo>
                    >               mvAppYuvImage;
    android::sp<IImageStreamInfo>   mpAppImage_Jpeg;

    // raw/yuv stream mapping
    android::Vector<StreamId_T>     mvYuvStreams_Fullraw;
    android::Vector<StreamId_T>     mvYuvStreams_Resizedraw;

protected:
    MBOOL                           mReconfig;
    MBOOL                           mHasVRConsumer;
    PipeConfigParams                mConfig_pipe_param;

protected:
    android::sp<InFlightRequest>    mInFlightRequest;

protected:
    sp<PipelineResourceMgr>         mpResourceMgr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNodeCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        NodeId_T nodeId
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  submitRequestFrame(
                                        android::sp<IPipelineFrame> pFrame
                                    );

    virtual MERROR                  submitRequest(AppRequest& request);

    virtual MVOID                   endFlush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual void                    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DefaultPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MERROR                  evaluateNodeIOStreams(
                                        NodeId_T const nodeId,
                                        evaluate_node_io_streams_params& rParams
                                    )   const;

    virtual MERROR                  refineRequestMetaStreamBuffers(
                                        refine_meta_stream_buffer_params& rParams
                                    )   const;

    virtual MUINT                   queryMinimalUsage(
                                        NodeId_T const nodeId,
                                        IImageStreamInfo const*const pStreamInfo
                                    )   const;

public:     ////                    Attributes.
    virtual MERROR                  acquireHalStreamBuffer(
                                        android::sp<IImageStreamInfo> const pStreamInfo,
                                        android::sp<HalImageStreamBuffer>& rpStreamBuffer
                                    )   const;

protected:  ////                    Attributes.
    virtual android::sp<IMetaStreamInfo>
                                    getMetaStreamInfoLocked_FaceDetectResult() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipelineModel_Default Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MERROR                  configure(
                                        PipeConfigParams const& rConfigParams
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    ~PipelineDefaultImp();
                                    PipelineDefaultImp(
                                        MINT32 const openId,
                                        android::String8 const& name
                                    );

    MVOID                           onDestroyLocked();

protected:  ////                    Operations.
    MERROR                          initNodes();

private:    ////                    Operations.
    MERROR                          calcPass1OutInfos(
                                        PipeConfigParams const& rConfigParams,
                                        SensorParam const& sensorParam,
                                        MUINT32 const raw_bitDepth,
                                        ConfigProfile & rCfgProfile
                                    )   const;

    MERROR                          evalConfigProfileLocked(
                                        PipeConfigParams const& rConfigParams,
                                        SensorParam const& sensorParam
                                    );

    MBOOL                           hasVRConsumer(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          setupAppStreamsLocked(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          setupHalStreamsLocked(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          sortYuvStreamsLocked();

    MERROR                          configNodeLocked_p1node(
                                        PipeConfigParams const& rConfigParams,
                                        SensorParam const& sensorParam,
                                        sp<IImageStreamBufferPoolT> pPoolRaw,
                                        sp<IImageStreamBufferPoolT> pPoolRawResizer
                                    );

    MERROR                          configNodeLocked_p2node(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          configNodeLocked_p2node_VSS(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          configNodeLocked_fdnode(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          configNodeLocked_jpegnode(
                                        PipeConfigParams const& rConfigParams
                                    );

    MERROR                          evaluateIOStreamsLocked_p1node(
                                        evaluate_node_io_streams_params& rParams
                                    ) const;

    MERROR                          evaluateIOStreamsLocked_p2node(
                                        evaluate_node_io_streams_params& rParams
                                    ) const;

    MERROR                          evaluateIOStreamsLocked_p2node_VSS(
                                        evaluate_node_io_streams_params& rParams
                                    ) const;

    MERROR                          evaluateIOStreamsLocked_fdnode(
                                        evaluate_node_io_streams_params& rParams
                                    ) const;

    MERROR                          evaluateIOStreamsLocked_jpegnode(
                                        evaluate_node_io_streams_params& rParams
                                    ) const;

    MSize                           calcThumbnailYuvSize(
                                        MSize const rPicSize,
                                        MSize const rThumbnailsize
                                    ) const;

    MERROR                          createStreamInfoLocked_Thumbnail_YUV(
                                        IMetadata const* pMetadata,
                                        android::sp<IImageStreamInfo>& rpStreamInfo
                                    ) const;

    MERROR                          createHalStreamBufferLocked_Thumbnail_YUV(
                                        android::sp<IImageStreamInfo> const pStreamInfo,
                                        android::sp<HalImageStreamBuffer>& rpStreamBuffer
                                    ) const;

    MERROR                          waitUntilDrainedThenReconfigure(
                                        AppRequest& request
                                    );

    MERROR                          decideSensor(
                                        PipeConfigParams const& rParams,
                                        MBOOL skipJpeg,
                                        MUINT sensorId,
                                        SensorParam& rSensorParams
                                    ) const;

    MBOOL                           skipStream(
                                        MBOOL skipJpeg,
                                        IImageStreamInfo* pStreamInfo
                                    ) const;

    MUINT32                         checkRequest(
                                        KeyedVector<StreamId_T, sp<IImageStreamBuffer> >const& vOImageBuffers
                                    );

    MERROR                          reConfigure();

    MVOID                           clearVar();

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
PipelineModel_Default*
PipelineModel_Default::
create(MINT32 const openId)
{
    String8 const name = String8::format("%s:%d", magicName(), openId);
    PipelineModel_Default* pPipelineModel = new PipelineDefaultImp(openId, name);
    if  ( ! pPipelineModel ) {
        MY_LOGE("fail to new an instance");
        return NULL;
    }
    //
    return pPipelineModel;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineDefaultImp::
PipelineDefaultImp(MINT32 const openId, android::String8 const& name)
    : DefaultPipelineModel(openId, name)
    //
    , mpNode_P1()
    , mpNode_P2()
    , mpNode_P2_VSS()
    , mpNode_FD()
    , mpNode_Jpeg()
    //
    , mpHalMeta_DynamicP1()
    , mpAppMeta_DynamicP1()
    , mpAppMeta_DynamicP2()
    , mpAppMeta_DynamicFD()
    , mpAppMeta_DynamicJpeg()
    , mpAppMeta_Control()
    //
    , mpHalImage_P1_Raw()
    , mpHalImage_P1_ResizerRaw()
    , mpHalImage_FD_YUV()
    , mpHalImage_Jpeg_YUV()
    , mpHalImage_Thumbnail_YUV()
    //
    , mvAppYuvImage()
    , mpAppImage_Jpeg()
    //
    , mReconfig(MFALSE)
    , mHasVRConsumer(MFALSE)
    , mInFlightRequest(new InFlightRequest())
    , mpResourceMgr(PipelineResourceMgr::getInstance(openId))
{
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineDefaultImp::
~PipelineDefaultImp()
{
    MY_LOGD("default pipeline destroyed");
}

/******************************************************************************
 *
 ******************************************************************************/
void
PipelineDefaultImp::
onLastStrongRef(const void* /*id*/)
{
    RWLock::AutoWLock _l(mRWLock);
    onDestroyLocked();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
onDestroyLocked()
{
    MY_LOGD("platform openId:%d", mOpenId);

    //  uninit each node according to the topological sort list.
    sp<IPipelineNodeMap> pPipelineNodeMap = mConfigContext.mpPipelineNodeMap;
    Vector<IPipelineDAG::NodeObj_T> const& rToposort = mConfigContext.mpPipelineDAG->getToposort();
    Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
    for (; it != rToposort.end(); it++)
    {
        sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(it->val);
        if  ( pNode == 0 ) {
            MY_LOGE("NULL node (id:%"PRIxPTR")", it->id);
            continue;
        }
        // query from resourcemgr
        if ( mpResourceMgr.get() ) {
            Vector<NodeId_T> vNodeList;
            mpResourceMgr->getReusableNodes(vNodeList);
            for ( size_t i=0; i<vNodeList.size(); i++ ) {
                //MY_LOGD("check reusable nodes (%d/%d) from resourcemgr...id:%"PRIxPTR, i, vNodeList.size(), it->id);
                if ( vNodeList[i] == it->id ) {
                    MY_LOGD("skip flush node (id:%"PRIxPTR")", it->id);
                    goto lbSkipUninit;
                }
            }
        }
        //
        pNode->uninit();
        pNode = NULL;
lbSkipUninit:
        MY_LOGV("check next one...");
    }
    //
    for (size_t i = 0; i < mHalImageStreamBufferPoolMap.size(); i++)
    {
        MUINT32 streamid = mHalImageStreamBufferPoolMap.keyAt(i);
        sp<HalImageStreamBufferPoolT>& pPool = mHalImageStreamBufferPoolMap.editValueAt(i);
        pPool->uninitPool(LOG_TAG);
        pPool = NULL;
    }
    mHalImageStreamBufferPoolMap.clear();

    mConfigContext.mpStreamInfoSet = NULL;
    mConfigContext.mpPipelineDAG = NULL;
    mConfigContext.mpPipelineNodeMap = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
initNodes(
)
{
    MERROR err = OK;
    RWLock::AutoWLock _l(mRWLock);
    //
    IPipelineNodeMapControl*pPipelineNodeMap = mConfigContext.mpPipelineNodeMap.get();
    IPipelineDAG* const     pPipelineDAG = mConfigContext.mpPipelineDAG.get();
    //
    //  If one of desired nodes has been created previously,
    //  we needn't create another instance and initialize it.
    //  Notes:
    //  Eveny node Id should belong to the global namespace, not to a specific
    //  pipeline's namespace, so that we can easily reuse node's instances.
    //
#warning"[FIXME] Eveny node Id should belong to the global namespace..."
    //
    pPipelineNodeMap->setCapacity(mMaxNumNodes);
    //
    Vector<NodeId_T> vNodeList;
    mpResourceMgr->getReusableNodes(vNodeList);
    for ( size_t i=0; i<vNodeList.size(); i++ ) {
        MY_LOGD_IF(0, "reusable from resource mgr: Nodeid(%d)", vNodeList[i]);
    }
    Vector<StreamId_T> vStreamList;
    mpResourceMgr->getReusableStreams(vStreamList);
    for ( size_t i=0; i<vStreamList.size(); i++ ) {
        MY_LOGD_IF(0, "reusable from resource mgr: Streamid(%d)", vStreamList[i]);
    }
    MY_LOGD("ResourceMgr has reusable node(%d) stream(%d)", vNodeList.size(), vStreamList.size());
    //
    if ( mConfigProfile.mbConfigP1 )
    {
        NodeId_T const nodeId = eNODEID_P1Node;
        typedef P1Node NodeT;
        //
        NodeT::InitParams params;
        params.openId = mOpenId;
        params.nodeId = nodeId;
        params.nodeName = "nodeP1";
        //
        //mpNode_P1 = NULL;
        mpResourceMgr->queryNode(nodeId, mpNode_P1, params);
        //sp<NodeT> pNode = mpNode_P1 = P1Node::createInstance();
        //pNode->init(params);
        //
        pPipelineDAG->addNode(nodeId);
        pPipelineNodeMap->add(nodeId, mpNode_P1);
    }
    //
    if ( mConfigProfile.mbConfigP2 )
    {
        NodeId_T const nodeId = eNODEID_P2Node;
        typedef P2Node NodeT;
        //
        NodeT::InitParams params;
        params.openId = mOpenId;
        params.nodeId = nodeId;
        params.nodeName = "nodeP2";
        //
        mpNode_P2 = NULL;
        sp<NodeT> pNode = mpNode_P2 = P2Node::createInstance(P2Node::PASS2_STREAM);
        pNode->init(params);
        //
        pPipelineDAG->addNode(nodeId);
        pPipelineNodeMap->add(nodeId, pNode);
    }
    //
    if ( mConfigProfile.mbConfigP2VSS )
    {
        NodeId_T const nodeId = eNODEID_P2Node_VSS;
        typedef P2Node NodeT;
        //
        NodeT::InitParams params;
        params.openId = mOpenId;
        params.nodeId = nodeId;
        params.nodeName = "nodeP2_VSS";
        //
        mpNode_P2_VSS = NULL;
        sp<NodeT> pNode = mpNode_P2_VSS = P2Node::createInstance(P2Node::PASS2_TIMESHARING);
        pNode->init(params);
        //
        pPipelineDAG->addNode(nodeId);
        pPipelineNodeMap->add(nodeId, pNode);
    }
    //
    if ( mConfigProfile.mbConfigFD )
    {
        NodeId_T const nodeId = eNODEID_FDNode;
        typedef FdNode NodeT;
        //
        NodeT::InitParams params;
        params.openId = mOpenId;
        params.nodeId = nodeId;
        params.nodeName = "nodeFD";
        //
        mpNode_FD = NULL;
        sp<NodeT> pNode = mpNode_FD = FdNode::createInstance();
        pNode->init(params);
        //
        pPipelineDAG->addNode(nodeId);
        pPipelineNodeMap->add(nodeId, pNode);
    }
    //
    if ( mConfigProfile.mbConfigJpeg )
    {
        NodeId_T const nodeId = eNODEID_JpegNode;
        typedef JpegNode NodeT;
        //
        NodeT::InitParams params;
        params.openId = mOpenId;
        params.nodeId = nodeId;
        params.nodeName = "nodeJpeg";
        //
        mpNode_Jpeg = NULL;
        sp<NodeT> pNode = mpNode_Jpeg = JpegNode::createInstance();
        pNode->init(params);
        //
        pPipelineDAG->addNode(nodeId);
        pPipelineNodeMap->add(nodeId, pNode);
    }
    //
    if( mpNode_P1.get() && mpNode_P2.get() )
        pPipelineDAG->addEdge(mpNode_P1->getNodeId(), mpNode_P2->getNodeId());
    if( mpNode_P1.get() && mpNode_P2_VSS.get() )
        pPipelineDAG->addEdge(mpNode_P1->getNodeId(), mpNode_P2_VSS->getNodeId());
    if( mpNode_P2.get() && mpNode_FD.get() )
        pPipelineDAG->addEdge(mpNode_P2->getNodeId(), mpNode_FD->getNodeId());
    if ( !mConfigProfile.mbConfigP2VSS )
    {
        if( mpNode_P2.get() && mpNode_Jpeg.get() )
            pPipelineDAG->addEdge(mpNode_P2->getNodeId(), mpNode_Jpeg->getNodeId());
    }
    else
    {
        if( mpNode_P2_VSS.get() && mpNode_Jpeg.get() )
            pPipelineDAG->addEdge(mpNode_P2_VSS->getNodeId(), mpNode_Jpeg->getNodeId());
    }
    //
    if( mpNode_P1.get() ) {
        pPipelineDAG->setRootNode(mpNode_P1->getNodeId());
    }
    else {
        MY_LOGE("no p1node as root, not supported yet");
    }
    //
    for (size_t i = 0; i < pPipelineNodeMap->size(); i++) {
        MY_LOGD("SetNodeValue of %d(id:%#"PRIxPTR")/%d", i, pPipelineNodeMap->nodeAt(i)->getNodeId(), pPipelineNodeMap->size());
        pPipelineDAG->setNodeValue(pPipelineNodeMap->nodeAt(i)->getNodeId(), i);
    }
    //
    if  ( pPipelineDAG->getToposort().isEmpty() ) {
        MY_LOGE("It seemes that the connection of nodes cannot form a DAG...");
        err = UNKNOWN_ERROR;
    }
    //
    //  dump nodes.
    {
        for (size_t i = 0; i < pPipelineNodeMap->size(); i++) {
            IPipelineNodeMap::NodePtrT const& pNode = pPipelineNodeMap->nodeAt(i);
            MY_LOGD("%#"PRIxPTR":%s", pNode->getNodeId(), pNode->getNodeName());
        }
        //
        Vector<String8> logs;
        pPipelineDAG->dump(logs);
        for (size_t i = 0; i < logs.size(); i++) {
            MY_LOGD("%s", logs[i].string());
        }
    }
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
/**
 * Given:
 *      App input meta streams
 *      App in/out image streams
 *
 * Action:
 *      Determine CONFIG stream set
 *      Determine I/O streams of each node
 *      Prepare Hal stream pools
 *      Configure each node (with their streams)
 *
 */
MERROR
PipelineDefaultImp::
configure(
    PipeConfigParams const& rConfigParams
)
{
    MERROR err = OK;
    RWLock::AutoWLock _l(mRWLock);
    //
    sp<IImageStreamBufferPoolT> pPoolRaw = NULL;
    sp<IImageStreamBufferPoolT> pPoolRawResizer = NULL;
    //
    ////////////////////////////////////////////////////////////////////////////
    // Flow
    //     1. evaluate the configure profile
    //     2. setup streams
    //     3. create nodes
    //     4. allocate pools
    //     5. config nodes
    ////////////////////////////////////////////////////////////////////////////
    //
    //clearVar();
    // 0. decide sensor
    mConfig_pipe_param = rConfigParams;
    SensorParam sensorParam;
    MBOOL skipJpeg = MTRUE; //sVRConsumer(rConfigParams);
    decideSensor(rConfigParams, skipJpeg, getOpenId(), sensorParam);
    //
    // 1. evaluate the profile
    {
        err = evalConfigProfileLocked(rConfigParams, sensorParam);
        if  ( OK != err ) {
            MY_LOGE("evalConfigProfileLocked - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    {
        ScenarioControl::BWC_Scenario_Param bwcparam;
        bwcparam.scenario   =
            mHasVRConsumer ? ScenarioControl::Scenario_VideoRecording :
            ScenarioControl::Scenario_NormalStreaming;
        bwcparam.sensorSize = sensorParam.size;
        bwcparam.sensorFps  = sensorParam.fps;
        err = mpResourceMgr->setScenarioCtrl(&bwcparam);
    }
    // 2. setup streams
    {
        //
        err = setupAppStreamsLocked(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("setupAppStreamsLocked - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
        //
        err = setupHalStreamsLocked(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("setupHalStreamsLocked - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
        //
        err = sortYuvStreamsLocked();
        if  ( OK != err ) {
            MY_LOGE("sortYuvStreamsLocked - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    // 3. create nodes
    {
        err = initNodes();
        if  ( OK != err ) {
            MY_LOGE("initNodes - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    // 4. allocate hal pools
    {
        MY_LOGD("LANJUICE START");
        if( mpHalImage_P1_Raw.get() )
            pPoolRaw = mpResourceMgr->queryStream(mpHalImage_P1_Raw->getStreamId(), mpHalImage_P1_Raw);
            //pPoolRaw = addHalStreamBufferPoolLocked(mpHalImage_P1_Raw);
        if( mpHalImage_P1_ResizerRaw.get() )
            pPoolRawResizer = mpResourceMgr->queryStream(mpHalImage_P1_ResizerRaw->getStreamId(), mpHalImage_P1_ResizerRaw);
            //pPoolRawResizer = addHalStreamBufferPoolLocked(mpHalImage_P1_ResizerRaw);
        if( mpHalImage_FD_YUV.get() )
            addHalStreamBufferPoolLocked(mpHalImage_FD_YUV);
        if( mpHalImage_Jpeg_YUV.get() )
            addHalStreamBufferPoolLocked(mpHalImage_Jpeg_YUV);
        MY_LOGD("allocate hal pools done: %p - %p", pPoolRaw.get(), pPoolRawResizer.get() );
    }
    // 5. config nodes
    if( mpNode_P1.get() ) {
        err = configNodeLocked_p1node(rConfigParams, sensorParam, pPoolRaw, pPoolRawResizer);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p1node - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    MY_LOGD("config p1 node done");
    //
    if( mpNode_P2.get() ) {
        err = configNodeLocked_p2node(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p2node - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    if( mpNode_P2_VSS.get() ) {
        err = configNodeLocked_p2node_VSS(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p2node_VSS - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    if( mpNode_FD.get() ) {
        err = configNodeLocked_fdnode(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_fdnode - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    if( mpNode_Jpeg.get() ) {
        err = configNodeLocked_jpegnode(rConfigParams);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_jpegnode - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
submitRequestFrame(
    android::sp<IPipelineFrame> pFrame
)
{
    mInFlightRequest->registerRequest(pFrame);
    //
    return DefaultPipelineModel::submitRequestFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
submitRequest(AppRequest& request)
{
#if SUPPORT_RECONFIGURE
    waitUntilDrainedThenReconfigure(request);
#endif
    //
    return DefaultPipelineModel::submitRequest(request);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
onDispatchFrame(android::sp<IPipelineFrame> const& pFrame, NodeId_T nodeId)
{
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == NULL || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return;
    }
    //
    IPipelineDAG::NodeObjSet_T nextNodes;
    MERROR err = pFrame->getPipelineDAG().getOutAdjacentNodes(nodeId, nextNodes);
    if  ( ! err && ! nextNodes.empty() )
    {
        for (size_t i = 0; i < nextNodes.size(); i++) {
            sp<IPipelineNode> pNextNode = pPipelineNodeMap->nodeAt(nextNodes[i].val);
            if  ( pNextNode != NULL ) {
                RWLock::AutoRLock _l(mflushLock);
                if (mInFlush == MTRUE) {
                    pNextNode->flush(pFrame);
                } else {
                    pNextNode->queue(pFrame);
                }
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
endFlush()
{
    RWLock::AutoWLock _l(mflushLock);
    mInFlush = MFALSE;
    mInFlightRequest->clear();
}


/******************************************************************************
 *
 ******************************************************************************/
//if request contains jpeg 1080p stream and previous no, wait until all previous request done
     // same in the case of the previous request contains jpeg 1080p stream but current request no
     // - wait
     // - re-configure() pipeline
     // - re-configure() P1 (may need to flush)
//if reprocessing to P2, wait until all previous request has finished P1 stage.
MERROR
PipelineDefaultImp::
waitUntilDrainedThenReconfigure(
    AppRequest& request
)
{
    CAM_TRACE_CALL();

    MUINT32 requestType = checkRequest(request.vOImageBuffers);
    MY_LOGV("requestType: %d", requestType);
    switch (requestType)
    {
        case eWait_1080pJpeg_Request:
        {
            mInFlightRequest->waitUntilNodeDrained(eNODEID_P1Node);
            if( mpNode_P1.get() ) {
                mpNode_P1->flush(); //to guarantee P1 has stopped in case that P1 node is using buffer.
            }
            mInFlightRequest->waitUntilDrained();

            #if 0
            //  uninit each node according to the topological sort list.
            sp<IPipelineNodeMap> pPipelineNodeMap = mConfigContext.mpPipelineNodeMap;
            Vector<IPipelineDAG::NodeObj_T> const& rToposort = mConfigContext.mpPipelineDAG->getToposort();
            Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
            for (; it != rToposort.end(); it++)
            {
                sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(it->val);
                if  ( pNode == 0 ) {
                    MY_LOGE("NULL node (id:%"PRIxPTR")", it->id);
                    continue;
                }
                //
                pNode->uninit();
            }
            mConfigContext.mpPipelineDAG = NULL;
            mConfigContext.mpPipelineNodeMap = NULL;
            #endif

            reConfigure();
        }
        break;
        case eWait_Reprocessing_Request:
            //TODO
            break;
        default:
            return OK;
        break;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
PipelineDefaultImp::
checkRequest(
    KeyedVector<StreamId_T, sp<IImageStreamBuffer> >const& vOImageBuffers
)
{
    //TODO ==> reprocessing

    if (mHasVRConsumer){
        return eWait_None;
    }

    MBOOL reconfig = MFALSE;
    KeyedVector<StreamId_T, sp<IImageStreamBuffer> >const& imageBuffers = vOImageBuffers;
    for(size_t i = 0; i < imageBuffers.size(); i++) {
        sp<IImageStreamBuffer> stream = imageBuffers.valueAt(i);
        if(stream->getStreamInfo()->getImgFormat() == HAL_PIXEL_FORMAT_BLOB &&
            stream->getStreamInfo()->getImgSize().size() >= 1920*1080)
        {
            reconfig = MTRUE;
            break;
        }
    }

    if (mReconfig != reconfig) {
       mReconfig = reconfig;
       return eWait_1080pJpeg_Request;
    }

    return eWait_None;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
reConfigure()
{
    CAM_TRACE_CALL();

    MY_LOGD("+");
    MERROR err = OK;
    RWLock::AutoWLock _l(mRWLock);

    clearVar();

    //(1)  decide sensor
    SensorParam sensorParam;
    decideSensor(mConfig_pipe_param, !mReconfig, getOpenId(), sensorParam);

    // update current sensor size
    mConfigProfile.mSensorSize = sensorParam.size;

    //
    {
        ScenarioControl::BWC_Scenario_Param bwcparam;
        bwcparam.scenario   =
            mHasVRConsumer ? ScenarioControl::Scenario_VideoRecording :
            ScenarioControl::Scenario_NormalStreaming;
        bwcparam.sensorSize = sensorParam.size;
        bwcparam.sensorFps  = sensorParam.fps;
        err = mpResourceMgr->setScenarioCtrl(&bwcparam);
    }

    //(2) light version of evaluate the profile
    {
        if( OK != calcPass1OutInfos(mConfig_pipe_param, sensorParam, 10, mConfigProfile) ) {
            MY_LOGE("calcPass1OutInfos failed");
            return BAD_VALUE;
        }
    }

    //(3) light version of setup stream: only P1 full and P1 resizer stream
    {
        IStreamInfoSetControl* pStreamInfoSet = mConfigContext.mpStreamInfoSet.get();

        //(.1)  p1: fullsize
        if (
                mConfigProfile.mbConfigP1 &&
                !! mConfigProfile.mFullSize
           )
        {
            MSize const& size = mConfigProfile.mFullSize;
            MINT const format = mConfigProfile.mFullFormat;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                eBUFFER_USAGE_HW_CAMERA_READWRITE
                ;
            sp<IImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                        "Hal:Image:P1:Fullraw",
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                        eSTREAMTYPE_IMAGE_INOUT,
                        8, 8,
                        usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_P1_Raw = pStreamInfo;
            pStreamInfoSet->editHalImage().addStream(pStreamInfo);
        }

        //(.2)  p1: resize
        if (
                mConfigProfile.mbConfigP1 &&
                !! mConfigProfile.mResizedSize
           )
        {
            MSize const& size = mConfigProfile.mResizedSize;
            MINT const format = mConfigProfile.mResizedFormat;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                eBUFFER_USAGE_HW_CAMERA_READWRITE
                ;
            //
            sp<IImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                        "Hal:Image:P1:Resizeraw",
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                        eSTREAMTYPE_IMAGE_INOUT,
                        8, 8,
                        usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_P1_ResizerRaw = pStreamInfo;
            pStreamInfoSet->editHalImage().addStream(pStreamInfo);
        }

        //(.3) update YUV I/O Combination
        err = sortYuvStreamsLocked();
        if  ( OK != err ) {
            MY_LOGE("sortYuvStreamsLocked - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    CAM_TRACE_BEGIN("raw-alloc");

    //(4) light version of  allocate hal pools: only P1 full and P1 resizer pool
    sp<IImageStreamBufferPoolT> pPoolRaw = NULL;
    sp<IImageStreamBufferPoolT> pPoolRawResizer = NULL;
    {
        if( mpHalImage_P1_Raw.get() ) {
            if(mHalImageStreamBufferPoolMap.indexOfKey(mpHalImage_P1_Raw->getStreamId()) >= 0)
                mHalImageStreamBufferPoolMap.valueFor(mpHalImage_P1_Raw->getStreamId())->uninitPool(LOG_TAG); //notice LOG_TAG
            pPoolRaw = addHalStreamBufferPoolLocked(mpHalImage_P1_Raw);
        }
        if( mpHalImage_P1_ResizerRaw.get() ) {
            if(mHalImageStreamBufferPoolMap.indexOfKey(mpHalImage_P1_ResizerRaw->getStreamId()) >= 0)
                mHalImageStreamBufferPoolMap.valueFor(mpHalImage_P1_ResizerRaw->getStreamId())->uninitPool(LOG_TAG); //notice LOG_TAG
            pPoolRawResizer = addHalStreamBufferPoolLocked(mpHalImage_P1_ResizerRaw);
        }
    }
    CAM_TRACE_END();


    #if 0
    mConfigContext.mpPipelineDAG    = IPipelineDAG::create();
    mConfigContext.mpPipelineNodeMap= IPipelineNodeMapControl::create();
    initNodes();
    #endif

    CAM_TRACE_BEGIN("config");
    // 5. full version of config nodes
    if( mpNode_P1.get() ) {
        err = configNodeLocked_p1node(mConfig_pipe_param, sensorParam, pPoolRaw, pPoolRawResizer);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p1node - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    #if 0
    if( mpNode_P2.get() ) {
        err = configNodeLocked_p2node(mConfig_pipe_param);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p2node - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //
    if( mpNode_P2_VSS.get() ) {
        err = configNodeLocked_p2node_VSS(mConfig_pipe_param);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_p2node_VSS - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }

    //In fact, FD node don't need to reconfigure..
    if( mpNode_FD.get() ) {
        err = configNodeLocked_fdnode(mConfig_pipe_param);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_fdnode - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    //In fact, Jpeg node don't need to reconfigure..
    if( mpNode_Jpeg.get() ) {
        err = configNodeLocked_jpegnode(mConfig_pipe_param);
        if  ( OK != err ) {
            MY_LOGE("configNodeLocked_jpegnode - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    #endif
    CAM_TRACE_END();
    //-
    MY_LOGD("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineDefaultImp::
skipStream(
    MBOOL skipJpeg,
    IImageStreamInfo* pStreamInfo
) const
{
    if  (
            skipJpeg
        &&  pStreamInfo->getImgFormat() == HAL_PIXEL_FORMAT_BLOB
        &&  pStreamInfo->getImgSize().size() >= 1920*1080
        )
    {
 //&& limited mode
        return MTRUE;
    }

    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
decideSensor(
    PipeConfigParams const& rParams,
    MBOOL skipJpeg,
    MUINT sensorId,
    SensorParam& rSensorParams
)   const
{
    struct Log
    {
        static String8
        skippedStream(IImageStreamInfo* pStreamInfo)
        {
            return String8::format(
                "skipped stream - format:0x%x type:%x size:%dx%d",
                pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
            );
        }

        static String8
        candidateStream(IImageStreamInfo* pStreamInfo)
        {
            return String8::format(
                "candidate stream - format:0x%x type:%x size:%dx%d",
                pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
            );
        }
    };

    //get max stream size
    MSize maxStreamSize;
    if  ( IImageStreamInfo* pStreamInfo = rParams.pImage_Raw.get() ) {
        if  ( pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN ) {
            MY_LOGD("%s", Log::skippedStream(pStreamInfo).string());
        }
        else {
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            maxStreamSize = pStreamInfo->getImgSize();
        }
    }
    //
    if  ( IImageStreamInfo* pStreamInfo = rParams.pImage_Jpeg_Stall.get() ) {
#if SUPPORT_RECONFIGURE
        if  (skipStream(skipJpeg, pStreamInfo)) {
            MY_LOGD("%s", Log::skippedStream(pStreamInfo).string());
        }
        else {
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size()) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }
#else
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size()) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
#endif
    }
    //
    bool hasEncoderConsumer = false;
    for (size_t i = 0; i < rParams.vImage_Yuv_NonStall.size(); i++) {
        if  ( IImageStreamInfo* pStreamInfo = rParams.vImage_Yuv_NonStall[i].get()) {
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size()) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
            //
            if  (( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                    == GRALLOC_USAGE_HW_VIDEO_ENCODER )
            {
                hasEncoderConsumer = true;
            }
        }
    }
    //
    MUINT32 sensorDev = IHalSensorList::get()->querySensorDevIdx(sensorId);
    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    IHalSensorList::get()->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

    //assume preview size <= capture size
    int const sizeofmode = 2;
    MSize sensorSize[sizeofmode] = {
                             MSize(sensorStaticInfo.previewWidth, sensorStaticInfo.previewHeight),
                             MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight)
                          };

    MUINT sensorFps_10x[sizeofmode] = {
                             sensorStaticInfo.previewFrameRate,
                             sensorStaticInfo.captureFrameRate
                          };

    MUINT sensorMode[sizeofmode] = {
                             SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
                             SENSOR_SCENARIO_ID_NORMAL_CAPTURE
                          };

    MY_LOGD("[sensor] prv(%d, %d)@%d, cap(%d, %d)@%d, vdo(%d, %d)@%d",
            sensorSize[0].w, sensorSize[0].h, sensorFps_10x[0],
            sensorSize[1].w, sensorSize[1].h, sensorFps_10x[1],
            sensorStaticInfo.videoWidth, sensorStaticInfo.videoHeight, sensorStaticInfo.videoFrameRate/10
           );

    if( hasEncoderConsumer ) {
        // use video mode
        rSensorParams.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        rSensorParams.size = MSize(sensorStaticInfo.videoWidth, sensorStaticInfo.videoHeight);
        rSensorParams.fps  = sensorStaticInfo.videoFrameRate/10;
        //
        MY_LOGD("[decided] mode: %d, size(%d, %d)@%d",
                rSensorParams.mode, rSensorParams.size.w, rSensorParams.size.h, rSensorParams.fps);
        return OK;
    }
    //
    //policy: find the smallest size that is "larger" than max of stream size (not the smallest difference)
    bool hit = false;
    int index = 0;
    for (int i = 0; i < sizeofmode; i++) {
        if (maxStreamSize.size() <= sensorSize[i].size()) {
            rSensorParams.mode = sensorMode[i];
            rSensorParams.size = sensorSize[i];
            rSensorParams.fps  = sensorFps_10x[i] / 10;
            hit = true;
            index = i;
            break;
        }
    }

    //if cannot find any, let it be the largest sensor size.
    if (!hit) {
        rSensorParams.mode = sensorMode[sizeofmode-1];
        rSensorParams.size = sensorSize[sizeofmode-1];
        rSensorParams.fps  = sensorFps_10x[sizeofmode-1] / 10;
    }

    // if frame rate is lower than expectation, may change to another sensor mode.
    // As we may not know the suitable frame rate in configuration stage, by far we
    // put stream size in the first priority. In other words, we choose the mode that
    // exactly match max_stream_size, otherwise, we would choose the mode that could
    // offer up to expected fps. [Temp Sol.]
    else {
        //assume expected fps = 30; (or query from metadata ?)
        MUINT expected_fps = 30;
        while (rSensorParams.fps < expected_fps &&
               maxStreamSize != rSensorParams.size  &&
               index > 0) {
            index--;
            rSensorParams.mode = sensorMode[index];
            rSensorParams.size = sensorSize[index];
            rSensorParams.fps  = sensorFps_10x[index] / 10;
        }
    }

    MY_LOGD("[decided] mode: %d, size(%d, %d)@%d",
        rSensorParams.mode, rSensorParams.size.w, rSensorParams.size.h, rSensorParams.fps);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateNodeIOStreams(
    NodeId_T const nodeId,
    evaluate_node_io_streams_params& rParams
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MERROR err = OK;
    MY_LOGV("node:Id: %d", nodeId);
    //
    switch  (nodeId)
    {
        case eNODEID_P1Node:
            err = evaluateIOStreamsLocked_p1node(rParams);
            break;
            //
        case eNODEID_P2Node:
            err = evaluateIOStreamsLocked_p2node(rParams);
            break;
            //
        case eNODEID_P2Node_VSS:
            err = evaluateIOStreamsLocked_p2node_VSS(rParams);
            break;
            //
        case eNODEID_FDNode:
            err = evaluateIOStreamsLocked_fdnode(rParams);
            break;
            //
        case eNODEID_JpegNode:
            err = evaluateIOStreamsLocked_jpegnode(rParams);
            break;
            //
        default:
            MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeId);
            return -EINVAL;
    }
    //
    MY_LOGE_IF( err != OK, "evaluateNodeIOStreams failed, nodeId %#"PRIxPTR, nodeId);
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
refineRequestMetaStreamBuffers(
    refine_meta_stream_buffer_params& rParams
)   const
{
    // modify in hal metadata
    if  ( mpHalMeta_Control.get() ) {
        // get metadata
        ssize_t const idx_meta = rParams.vHalMetaBuffer.indexOfKey(mpHalMeta_Control->getStreamId());
        if( idx_meta < 0 ) {
            MY_LOGW("cannot get streamBuffer of %#"PRIxPTR"(%s)",
                    mpHalMeta_Control->getStreamId(),
                    mpHalMeta_Control->getStreamName());
            return NAME_NOT_FOUND;
        }

        sp<IMetaStreamBuffer> const pMetaStreamBuffer = rParams.vHalMetaBuffer.valueAt(idx_meta);
        IMetadata* pMetadata                          = pMetaStreamBuffer->tryWriteLock(getName());

        if( pMetadata == NULL ) {
            MY_LOGE("pMetadata == NULL");
            return BAD_VALUE;
        }
        // update sensor size
        {
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mConfigProfile.mSensorSize, Type2Type< MSize >());
            pMetadata->update(entry.tag(), entry);
        }

        // update require exif if jpeg stream exists
        if  ( mpAppImage_Jpeg.get() &&
                0 <= rParams.pAppImageStreams->indexOfKey(mpAppImage_Jpeg->getStreamId()) )
        {
            MY_LOGD_IF(1, "set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
            entry.push_back(1, Type2Type<MUINT8>());
            pMetadata->update(entry.tag(), entry);
        }

        pMetaStreamBuffer->unlock(getName(), pMetadata);

    } else {
        MY_LOGE("no hal control meta");
        return BAD_VALUE;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
PipelineDefaultImp::
queryMinimalUsage(
    NodeId_T const nodeId,
    IImageStreamInfo const*const pStreamInfo
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if ( pStreamInfo == NULL ) {
        MY_LOGE("Null pStreamInfo");
        return 0;
    }
    StreamId_T streamId = pStreamInfo->getStreamId();
#warning "[FIXME] queryMinimalUsage"
    //
    switch  (nodeId)
    {
    case eNODEID_P1Node:{
        //
        //  [input]
        //      App:Meta:Control
        //  [output]
        //      App:Meta:Dynamic01
        //      Hal:Meta:Dynamic01
        //
        if(
                isStream(mpHalImage_P1_Raw, streamId) ||
                isStream(mpHalImage_P1_ResizerRaw, streamId)
          )
        {
            //return (eBUFFER_USAGE_HW_CAMERA_WRITE);
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        }
        }break;
        //
    case eNODEID_P2Node:
    case eNODEID_P2Node_VSS:{
        //
        //  [input]
        //      App:Meta:Control
        //      Hal:Meta:Dynamic01
        //  [output]
        //      App:Image:?
        //
        if(
                isStream(mpHalImage_P1_Raw, streamId) ||
                isStream(mpHalImage_P1_ResizerRaw, streamId)
          )
        {
            //return (eBUFFER_USAGE_HW_CAMERA_READ);
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        }
        //
        ssize_t index = mvAppYuvImage.indexOfKey(streamId);
        if  ( 0 <= index ) {
            //return (eBUFFER_USAGE_HW_CAMERA_WRITE);
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        }
        //
        if(
                isStream(mpHalImage_FD_YUV, streamId)        ||
                isStream(mpHalImage_Jpeg_YUV, streamId)      ||
                isStream(mpHalImage_Thumbnail_YUV, streamId)
          )
        {
            //return (eBUFFER_USAGE_HW_CAMERA_WRITE);
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        }
        }break;
    case eNODEID_FDNode:{
        //
        //  [input]
        //      App:Meta:Control
        //      Hal:Image:
        //
        //  [output]
        //      Hal:Meta:Dynamic03
        //
        if(
                isStream(mpHalImage_FD_YUV, streamId)
          )
        {
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        }
        }break;
    case eNODEID_JpegNode:{
        //
        //  [input]
        //      App:Meta:Control
        //      Hal:Image:YUV_Jpeg
        //      Hal:Image:YUV_Thumbnail
        //
        //  [output]
        //      Hal:Meta:Dynamic03
        //      App:Image:Jpeg
        if(
                isStream(mpHalImage_Jpeg_YUV, streamId)      ||
                isStream(mpHalImage_Thumbnail_YUV, streamId)
          )
        {
            // may hw or sw encode
            return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
            //return (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        }
        //
        if(
                isStream(mpAppImage_Jpeg, streamId)
          )
        {
            return (eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        }
        }break;
        //
    default:
        MY_LOGE("Bad nodeId:%#"PRIxPTR, nodeId);
        break;
    }
    //
    MY_LOGW("cannot find nodeId %#"PRIxPTR" streamId %#"PRIxPTR"(%s)",
            nodeId,
            pStreamInfo->getStreamId(), pStreamInfo->getStreamName());
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
acquireHalStreamBuffer(
    android::sp<IImageStreamInfo> const pStreamInfo,
    android::sp<HalImageStreamBuffer>& rpStreamBuffer
)   const
{
    {
        RWLock::AutoRLock _l(mRWLock);
        // full raw
        if ( mpHalImage_P1_Raw.get() && mpHalImage_P1_Raw->getStreamId() == pStreamInfo->getStreamId() ) {
            return mpResourceMgr->acquireHalStreamBuffer(pStreamInfo, rpStreamBuffer);
        }
        // resized raw
        if ( mpHalImage_P1_ResizerRaw.get() && mpHalImage_P1_ResizerRaw->getStreamId() == pStreamInfo->getStreamId() ) {
            return mpResourceMgr->acquireHalStreamBuffer(pStreamInfo, rpStreamBuffer);
        }
        //Thumbnail_YUV
        if  ( mpHalImage_Thumbnail_YUV.get() && mpHalImage_Thumbnail_YUV->getStreamId() == pStreamInfo->getStreamId() ) {
            return createHalStreamBufferLocked_Thumbnail_YUV(pStreamInfo, rpStreamBuffer);
        }
    }
    return DefaultPipelineModel::acquireHalStreamBuffer(pStreamInfo, rpStreamBuffer);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamInfo>
PipelineDefaultImp::
getMetaStreamInfoLocked_FaceDetectResult() const
{
    return mpAppMeta_DynamicFD;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
calcPass1OutInfos(
    PipeConfigParams const& rConfigParams,
    SensorParam const& sensorParam,
    MUINT32 const raw_bitDepth,
    ConfigProfile & rCfgProfile
)   const
{
    MSize const sensorSize = sensorParam.size;
    // query sensor infos
    IHalSensorList* pSensorList = IHalSensorList::get();
    if( pSensorList == NULL ) {
        MY_LOGE("pSensorList == NULL");
        return DEAD_OBJECT;
    }
    //
    MUINT32 sensorDev = pSensorList->querySensorDevIdx(getOpenId());
    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    pSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    //
    if( sensorStaticInfo.sensorType == SENSOR_TYPE_RAW ) {
        MINT fmt = 0;
        switch( raw_bitDepth )
        {
#define CASE_BIT(x)     case x: fmt = eImgFmt_BAYER##x; break
            CASE_BIT(8);
            CASE_BIT(10);
            CASE_BIT(12);
#undef CASE_BIT
            default:
                MY_LOGE("not support bitdepth %d", raw_bitDepth);
                return BAD_VALUE;
        }
#if 0
        // always use imgo
        rCfgProfile.mFullSize = sensorSize;
        rCfgProfile.mFullFormat = fmt;
        // disable img2o
        rCfgProfile.mResizedSize = MSize(0, 0);
        rCfgProfile.mResizedFormat = fmt;
#else
        MBOOL const hasJpeg = rConfigParams.pImage_Jpeg_Stall.get() != NULL;
        //
        if ( hasJpeg ) {
            // config full-raw if jpeg stream is configured
            rCfgProfile.mFullSize = sensorSize;
            rCfgProfile.mFullFormat = fmt;
        }
        else {
            rCfgProfile.mFullSize = MSize(0, 0); // disable
        }
        //
        // check if need to config img2o: try to limit throughput usage
        MSize maxYuvStreamSize;
        {
            for( size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++ ) {
                MSize const streamSize = rConfigParams.vImage_Yuv_NonStall[i]->getImgSize();
                if( maxYuvStreamSize.size() < streamSize.size() )
                    maxYuvStreamSize = streamSize;
            }
            MY_LOGD_IF( !!maxYuvStreamSize, "max yuv stream size %dx%d",
                    maxYuvStreamSize.w, maxYuvStreamSize.h);
        }
        //
        MUINT32 const fps = sensorParam.fps;
#define ISP_P2_THROUGHPUT_TH_1            (150*1000000) // MPixel/s
#define ISP_P2_THROUGHPUT_TH_2            (200*1000000)
#define P1_SCALE_RATIO_10X                (7)
        MUINT32 const throughput_thres = hasJpeg ? ISP_P2_THROUGHPUT_TH_1 : ISP_P2_THROUGHPUT_TH_2;
#warning"[FIXME] should not always enable img2o"
        if( 1
            //    (fps * sensorSize.size() > throughput_thres) ||
            //    ! rCfgProfile.mFullSize
          )
        {
            MINT32 const width_min = ALIGN_2(sensorSize.w * P1_SCALE_RATIO_10X / 10); //[TODO] not always times  P1_SCALE_RATIO_10X. It depends.
            MINT32 width = maxYuvStreamSize.w;

//#warning"[FIXME] should enable img2o resize"
            // make sure img2o width:  sensor.w * 0.7 <= width <= sensor.w
            if( width < width_min )
                width = width_min;
            else if( width > sensorSize.w )
                width = sensorSize.w;

            rCfgProfile.mResizedSize = MSize(width, sensorSize.h);
            rCfgProfile.mResizedFormat = fmt;
            MY_LOGD("%dx%d@%d: use img2o %dx%d",
                    sensorSize.w, sensorSize.h, fps,
                    rCfgProfile.mResizedSize.w,
                    rCfgProfile.mResizedSize.h
                    );
        }
#undef ISP_P2_THROUGHPUT_TH_1
#undef ISP_P2_THROUGHPUT_TH_2
#undef P1_SCALE_RATIO_10X
#endif
    }
    else if( sensorStaticInfo.sensorType == SENSOR_TYPE_YUV ) {
        MINT fmt = 0;
        switch( sensorStaticInfo.sensorFormatOrder )
        {
#define CASE_ORDER(order, eImgfmt) case SENSOR_FORMAT_ORDER_##order: fmt = eImgfmt; break
            CASE_ORDER(UYVY, eImgFmt_UYVY);
            CASE_ORDER(VYUY, eImgFmt_VYUY);
            CASE_ORDER(YUYV, eImgFmt_YUY2);
            CASE_ORDER(YVYU, eImgFmt_YVYU);
#undef CASE_ORDER
            default:
                MY_LOGE("not support order %d",  sensorStaticInfo.sensorFormatOrder );
                return BAD_VALUE;
        }
        rCfgProfile.mFullSize = sensorSize;
        rCfgProfile.mFullFormat = fmt;
        //
        rCfgProfile.mResizedSize = MSize(0, 0); // disable
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evalConfigProfileLocked(
    PipeConfigParams const& rConfigParams,
    SensorParam const& sensorParam
)
{
    mConfigProfile.mbConfigP1    = MTRUE;
    mConfigProfile.mbConfigP2    = MTRUE;

    mConfigProfile.mbConfigP2VSS = MFALSE;

    if(hasVRConsumer(rConfigParams)){
        mConfigProfile.mbConfigP2VSS = MTRUE;
    }

    mConfigProfile.mbConfigFD   = MTRUE;
    mConfigProfile.mbConfigJpeg = rConfigParams.pImage_Jpeg_Stall.get() ? MTRUE : MFALSE;

    if( OK != calcPass1OutInfos(rConfigParams, sensorParam, 10, mConfigProfile) ) {
        MY_LOGE("calcPass1OutInfos failed");
        return BAD_VALUE;
    }

    // update current sensor size
    mConfigProfile.mSensorSize = sensorParam.size;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineDefaultImp::
hasVRConsumer(
    PipeConfigParams const& rConfigParams
)
{
    if( rConfigParams.pImage_Jpeg_Stall.get() )
    {
        for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  (( rConfigParams.vImage_Yuv_NonStall[i]->getUsageForConsumer()
                        & GRALLOC_USAGE_HW_VIDEO_ENCODER ) == GRALLOC_USAGE_HW_VIDEO_ENCODER )
            {
                MY_LOGD("has video En consumer");
                mHasVRConsumer = MTRUE;
                break;
            }
        }
    }

    return mHasVRConsumer;
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
setupAppStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    IStreamInfoSetControl*  pStreamInfoSet = mConfigContext.mpStreamInfoSet.get();

    //App:Meta:Control
    {
        sp<IMetaStreamInfo> pStreamInfo;
        if  ( (pStreamInfo = rConfigParams.pMeta_Control) != 0 )
        {
            mpAppMeta_Control = pStreamInfo;
            pStreamInfo->setMaxBufNum(10);
            pStreamInfoSet->editAppMeta().addStream(pStreamInfo);
        }
    }

    //App:dynamic
    if ( mConfigProfile.mbConfigP1 )
    {
        //App:Meta:01
        //   pass1 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP1",
                    eSTREAMID_META_APP_DYNAMIC_01,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicP1 = pStreamInfo;
        pStreamInfoSet->editAppMeta().addStream(pStreamInfo);
    }

    if ( mConfigProfile.mbConfigP2 )
    {
        //App:Meta:P2
        //   pass2 result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:DynamicP2",
                    eSTREAMID_META_APP_DYNAMIC_02,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicP2 = pStreamInfo;
        pStreamInfoSet->editAppMeta().addStream(pStreamInfo);
    }


    if ( mConfigProfile.mbConfigFD )
    {
        //App:Meta:FD
        //   FD result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:FD",
                    eSTREAMID_META_APP_DYNAMIC_FD,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicFD = pStreamInfo;
        pStreamInfoSet->editAppMeta().addStream(pStreamInfo);
    }

    if ( mConfigProfile.mbConfigJpeg )
    {
        //App:Meta:Jpeg
        //   Jpeg result meta
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "App:Meta:Jpeg",
                    eSTREAMID_META_APP_DYNAMIC_JPEG,
                    eSTREAMTYPE_META_OUT,
                    10, 1
                    );
        mpAppMeta_DynamicJpeg = pStreamInfo;
        pStreamInfoSet->editAppMeta().addStream(pStreamInfo);
    }


    //App:Image
    {
        sp<IImageStreamInfo> pStreamInfo;
        //
        //App:Image:Raw
        if  ( (pStreamInfo = rConfigParams.pImage_Raw) != 0 ) {
            MY_LOGF("Not supported");
            pStreamInfo->setMaxBufNum(10);
            pStreamInfoSet->editAppImage().addStream(pStreamInfo);
        }
        //
        //App:Image:Jpeg:Stall
        if  ( (pStreamInfo = rConfigParams.pImage_Jpeg_Stall) != 0 ) {
            mpAppImage_Jpeg = pStreamInfo;
            pStreamInfo->setMaxBufNum(1);
            pStreamInfoSet->editAppImage().addStream(pStreamInfo);
        }
        //
        //App:Image:Yuv:NotStall
        for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( (pStreamInfo = rConfigParams.vImage_Yuv_NonStall[i]) != 0 ) {
                mvAppYuvImage.add(pStreamInfo->getStreamId(), pStreamInfo);
                pStreamInfo->setMaxBufNum(10);
                pStreamInfoSet->editAppImage().addStream(pStreamInfo);
            }
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
setupHalStreamsLocked(
    PipeConfigParams const& rConfigParams
)
{
    IStreamInfoSetControl*  pStreamInfoSet = mConfigContext.mpStreamInfoSet.get();

    //Hal:Meta
    //
    {
        //Hal:Meta:Control
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:Control",
                    eSTREAMID_META_PIPE_CONTROL,
                    eSTREAMTYPE_META_IN,
                    10, 1
                    );
        mpHalMeta_Control = pStreamInfo;
        pStreamInfoSet->editHalMeta().addStream(pStreamInfo);
    }
    //
    if ( mConfigProfile.mbConfigP1 )
    {
        //Hal:Meta:01
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P1:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_01,
                    eSTREAMTYPE_META_INOUT,
                    10, 1
                    );
        mpHalMeta_DynamicP1 = pStreamInfo;
        pStreamInfoSet->editHalMeta().addStream(pStreamInfo);
    }
    //
    if ( mConfigProfile.mbConfigP2 )
    {
        //Hal:Meta:01
        sp<IMetaStreamInfo> pStreamInfo =
            new MetaStreamInfo(
                    "Hal:Meta:P2:Dynamic",
                    eSTREAMID_META_PIPE_DYNAMIC_02,
                    eSTREAMTYPE_META_INOUT,
                    10, 1
                    );
        mpHalMeta_DynamicP2 = pStreamInfo;
        pStreamInfoSet->editHalMeta().addStream(pStreamInfo);
    }
    //
    //Hal:Image
    if (
            mConfigProfile.mbConfigP1 &&
            !! mConfigProfile.mFullSize
       )
    {
        // p1: fullsize
        MSize const& size = mConfigProfile.mFullSize;
        MINT const format = mConfigProfile.mFullFormat;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
            eBUFFER_USAGE_HW_CAMERA_READWRITE
            ;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Hal:Image:P1:Fullraw",
                    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    6, 6,
                    usage, format, size
            );
        if( pStreamInfo == NULL ) {
            return BAD_VALUE;
        }
        //
        mpHalImage_P1_Raw = pStreamInfo;
        pStreamInfoSet->editHalImage().addStream(pStreamInfo);
    }

    if (
            mConfigProfile.mbConfigP1 &&
            !! mConfigProfile.mResizedSize
       )
    {
        // p1: resize
        MSize const& size = mConfigProfile.mResizedSize;
        MINT const format = mConfigProfile.mResizedFormat;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
            eBUFFER_USAGE_HW_CAMERA_READWRITE
            ;
        //
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Hal:Image:P1:Resizeraw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    6, 6,
                    usage, format, size
            );
        if( pStreamInfo == NULL ) {
            return BAD_VALUE;
        }
        //
        mpHalImage_P1_ResizerRaw = pStreamInfo;
        pStreamInfoSet->editHalImage().addStream(pStreamInfo);
    }

    //Hal:Image:FD
    if ( mConfigProfile.mbConfigFD )
    {
        MSize const size(640, 480); //FIXME: hard-code here?
        MINT const format = eImgFmt_YV12;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
            eBUFFER_USAGE_HW_CAMERA_READWRITE
            ;

        sp<ImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                "Hal:Image:FD",
                eSTREAMID_IMAGE_FD,
                eSTREAMTYPE_IMAGE_INOUT,
                10, 1,
                usage, format, size
            );
        if( pStreamInfo == NULL ) {
            return BAD_VALUE;
        }
        //
        mpHalImage_FD_YUV = pStreamInfo;
        pStreamInfoSet->editHalImage().addStream(pStreamInfo);
    }

    //Hal:Image:YUY2 for jpeg & thumbnail
    if ( mConfigProfile.mbConfigJpeg )
    {
        //Hal:Image:YUY2 for jpeg
        {
            MSize const& size = rConfigParams.pImage_Jpeg_Stall->getImgSize();
            MINT const format = mConfigProfile.mbConfigP2VSS ? eImgFmt_YV12 : eImgFmt_YUY2;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                eBUFFER_USAGE_HW_CAMERA_READWRITE
                ;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvJpeg",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_Jpeg_YUV = pStreamInfo;
            pStreamInfoSet->editHalImage().addStream(pStreamInfo);
        }
        //
        //Hal:Image:YUY2 for thumbnail
        {
            MSize const size(-1L, -1L); //unknown now
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                eBUFFER_USAGE_HW_CAMERA_READWRITE
                ;
            sp<ImageStreamInfo>
                pStreamInfo = createImageStreamInfo(
                    "Hal:Image:YuvThumbnail",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size
                );
            if( pStreamInfo == NULL ) {
                return BAD_VALUE;
            }
            //
            mpHalImage_Thumbnail_YUV = pStreamInfo;
            pStreamInfoSet->editHalImage().addStream(pStreamInfo);
            MY_LOGD("streamId:%#"PRIxPTR" %s %p", pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pStreamInfo.get());
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
sortYuvStreamsLocked()
{
    //clear var
    mvYuvStreams_Fullraw.clear();
    mvYuvStreams_Resizedraw.clear();

    //TODO: do sort?
#define PUSH_TO_VECTOR( vector, pStreamInfo )               \
    do{                                                     \
        if( pStreamInfo != NULL )                           \
            vector.push_back( pStreamInfo->getStreamId() ); \
    }while(0)
#define PUSH_TO_FULL( pStreamInfo )    PUSH_TO_VECTOR(mvYuvStreams_Fullraw, pStreamInfo)
#define PUSH_TO_RESIZE( pStreamInfo )  PUSH_TO_VECTOR(mvYuvStreams_Resizedraw, pStreamInfo)
    // if use fullraw & resizedraw
    if ( mpHalImage_P1_Raw.get() && mpHalImage_P1_ResizerRaw.get() ) {
        MSize const resize_size = mpHalImage_P1_ResizerRaw->getImgSize();
        // app yuv streams
        for( size_t i = 0; i < mvAppYuvImage.size(); i++ ) {
            IImageStreamInfo const*const pStreamInfo = mvAppYuvImage.valueAt(i).get();
            MSize const streamSize = pStreamInfo->getImgSize();
            if( streamSize.w <= resize_size.w && streamSize.h <= resize_size.h )
                PUSH_TO_RESIZE(pStreamInfo);
            else
                PUSH_TO_FULL(pStreamInfo);
        }
        // fd -> resized raw
        PUSH_TO_RESIZE(mpHalImage_FD_YUV);

        // jpeg & thumbnail -> fullraw
        PUSH_TO_FULL(mpHalImage_Jpeg_YUV);
        PUSH_TO_FULL(mpHalImage_Thumbnail_YUV);
    }
    else if ( mpHalImage_P1_Raw.get() ) {
        // all yuv streams comes from full-raw
        // app yuv streams
        for( size_t i = 0; i < mvAppYuvImage.size(); i++ ) {
            IImageStreamInfo const*const pStreamInfo = mvAppYuvImage.valueAt(i).get();
            PUSH_TO_FULL(pStreamInfo);
        }
        // fd
        PUSH_TO_FULL(mpHalImage_FD_YUV);
        // jpeg & thumbnail yuv
        PUSH_TO_FULL(mpHalImage_Jpeg_YUV);
        PUSH_TO_FULL(mpHalImage_Thumbnail_YUV);
    }
    else if ( mpHalImage_P1_ResizerRaw.get() ) {
        // all yuv streams comes from resized-raw
        // app yuv streams
        for( size_t i = 0; i < mvAppYuvImage.size(); i++ ) {
            IImageStreamInfo const*const pStreamInfo = mvAppYuvImage.valueAt(i).get();
            PUSH_TO_RESIZE(pStreamInfo);
        }
        // fd
        PUSH_TO_RESIZE(mpHalImage_FD_YUV);
        // jpeg & thumbnail yuv
        PUSH_TO_RESIZE(mpHalImage_Jpeg_YUV);
        PUSH_TO_RESIZE(mpHalImage_Thumbnail_YUV);
    }
    else {
        MY_LOGE("both fullraw & resizedraw == NULL");
        return BAD_VALUE;
    }
    //
#if 1
    // dump raw stream dispatch rule
    for( size_t i = 0; i < mvYuvStreams_Fullraw.size(); i++ ) {
        MY_LOGD("raw streamId:%#"PRIxPTR" -> yuv streamId:%#"PRIxPTR,
                mpHalImage_P1_Raw->getStreamId(), mvYuvStreams_Fullraw[i]);
    }
    for( size_t i = 0; i < mvYuvStreams_Resizedraw.size(); i++ ) {
        MY_LOGD("raw streamId:%#"PRIxPTR" -> yuv streamId:%#"PRIxPTR,
                mpHalImage_P1_ResizerRaw->getStreamId(), mvYuvStreams_Resizedraw[i]);
    }
#endif

#undef PUSH_TO_FULL
#undef PUSH_TO_RESIZE
#undef PUSH_TO_VECTOR
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
// used by following configNodeLocked_XXX(...)
#define DECLARE_INOUT_STREAM(nodeId)                                                   \
    IPipelineNodeMapControl*pPipelineNodeMap = mConfigContext.mpPipelineNodeMap.get(); \
    sp<IPipelineNodeMapControl::INode> pINode = pPipelineNodeMap->getNodeFor(nodeId);  \
    sp<IStreamInfoSetControl> const& rpIStreams = pINode->editInStreams();             \
    sp<IStreamInfoSetControl> const& rpOStreams = pINode->editOutStreams();

#define ADD_STREAM( streams, scope, type, pStreamInfo)           \
    do {                                                         \
        if( pStreamInfo.get() )                                  \
            streams->edit##scope##type().addStream(pStreamInfo); \
    } while(0)
#define ADD_IN_STREAM(scope, type, pStreamInfo)      \
    ADD_STREAM(rpIStreams, scope, type, pStreamInfo)
#define ADD_OUT_STREAM(scope, type, pStreamInfo)     \
    ADD_STREAM(rpOStreams, scope, type, pStreamInfo)
/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
configNodeLocked_p1node(
    PipeConfigParams const& rConfigParams,
    SensorParam const& sensorParam,
    sp<IImageStreamBufferPoolT> pPoolRaw,
    sp<IImageStreamBufferPoolT> pPoolRawResizer
    )
{
    NodeId_T const nodeId = eNODEID_P1Node;
    DECLARE_INOUT_STREAM(nodeId)
    //
    //  [input]
    //      App:Meta:Control
    //  [output]
    //      App:Meta:DynamicP1
    //      Hal:Meta:DynamicP1
    //      Hal:Image:RAW
    //      Hal:Image:ResizerRaw
    //
    ADD_IN_STREAM(App, Meta, rConfigParams.pMeta_Control);
    ADD_IN_STREAM(Hal, Meta, mpHalMeta_Control);
    //
    ADD_OUT_STREAM(App, Meta, mpAppMeta_DynamicP1);
    ADD_OUT_STREAM(Hal, Meta, mpHalMeta_DynamicP1);
    ADD_OUT_STREAM(Hal, Image, mpHalImage_P1_ResizerRaw);
    ADD_OUT_STREAM(Hal, Image, mpHalImage_P1_Raw);
    //
    // config node
    {
        P1Node::ConfigParams cfgParams;
        //
        cfgParams.pInAppMeta = rConfigParams.pMeta_Control;
        cfgParams.pInHalMeta = mpHalMeta_Control;
        cfgParams.pOutAppMeta = mpAppMeta_DynamicP1;
        cfgParams.pOutHalMeta = mpHalMeta_DynamicP1;
        if( mpHalImage_P1_ResizerRaw.get() )
            cfgParams.pOutImage_resizer = mpHalImage_P1_ResizerRaw;
        if( mpHalImage_P1_Raw.get() )
            cfgParams.pvOutImage_full.push_back(mpHalImage_P1_Raw);
        cfgParams.sensorParams.mode = sensorParam.mode;
        cfgParams.sensorParams.size = sensorParam.size;
        //
        cfgParams.pStreamPool_resizer = pPoolRawResizer;
        cfgParams.pStreamPool_full    = pPoolRaw;
        //
//        MERROR err = mpNode_P1->config(cfgParams);
        MERROR err = mpResourceMgr->evaluateNodeReconfigure(nodeId, &cfgParams);
        if  ( OK != err ) {
            MY_LOGE("mpNode_P1->config - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
configNodeLocked_p2node(
    PipeConfigParams const& rConfigParams
)
{
    NodeId_T const nodeId = eNODEID_P2Node;
    //
    DECLARE_INOUT_STREAM(nodeId)
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Meta:DynamicP1
    //      Hal:Image:RAW
    //  [output]
    //      App:Meta:DynamicP2
    //      App:Image:YUVs
    //      Hal:Image:YUVs
    //
    ADD_IN_STREAM(App, Meta, rConfigParams.pMeta_Control);
    ADD_IN_STREAM(Hal, Meta, mpHalMeta_DynamicP1);
    //
    ADD_IN_STREAM(Hal, Image, mpHalImage_P1_ResizerRaw);
    ADD_IN_STREAM(Hal, Image, mpHalImage_P1_Raw);
    //
    ADD_OUT_STREAM(Hal, Image, mpHalImage_FD_YUV);
    //
    if ( ! mConfigProfile.mbConfigP2VSS ) {
        ADD_OUT_STREAM(Hal, Meta, mpHalMeta_DynamicP2);
        ADD_OUT_STREAM(Hal, Image, mpHalImage_Jpeg_YUV);
        ADD_OUT_STREAM(Hal, Image, mpHalImage_Thumbnail_YUV);
    }
    //
    ADD_OUT_STREAM(App, Meta, mpAppMeta_DynamicP2);
    //
    for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
        ADD_OUT_STREAM(App, Image, rConfigParams.vImage_Yuv_NonStall[i]);
    }
    //
    // config node
    {
        P2Node::ConfigParams cfgParams;
        //
        cfgParams.pInAppMeta    = rConfigParams.pMeta_Control;
        cfgParams.pInHalMeta    = mpHalMeta_DynamicP1;
        cfgParams.pOutAppMeta   = mpAppMeta_DynamicP2;
        cfgParams.pOutHalMeta   = (!mConfigProfile.mbConfigP2VSS) ? mpHalMeta_DynamicP2 : NULL;
        //
        if( mpHalImage_P1_Raw.get() )
            cfgParams.pvInFullRaw.push_back(mpHalImage_P1_Raw);
        cfgParams.pInResizedRaw = mpHalImage_P1_ResizerRaw;
        //
        for (size_t i = 0; i < rConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( rConfigParams.vImage_Yuv_NonStall[i] != 0 ) {
                cfgParams.vOutImage.push_back(rConfigParams.vImage_Yuv_NonStall[i]);
            }
        }
        if( mpHalImage_FD_YUV.get() )
            cfgParams.vOutImage.push_back(mpHalImage_FD_YUV);
        //
        // config yuvs for jpeg if is not configured as recording
        if( ! mConfigProfile.mbConfigP2VSS ) {
            if( mpHalImage_Jpeg_YUV.get() )
                cfgParams.vOutImage.push_back(mpHalImage_Jpeg_YUV);
            if( mpHalImage_Thumbnail_YUV.get() )
                cfgParams.vOutImage.push_back(mpHalImage_Thumbnail_YUV);
        }
        //
        MERROR err = mpNode_P2->config(cfgParams);
        if  ( OK != err ) {
            MY_LOGE("mpNode_P2->config - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
configNodeLocked_p2node_VSS(
    PipeConfigParams const& rConfigParams
)
{
    NodeId_T const nodeId = eNODEID_P2Node_VSS;
    //
    DECLARE_INOUT_STREAM(nodeId)
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Meta:Dynamic01
    //      Hal:Image:RAW
    //  [output]
    //      App:Meta:Dynamic02(X)
    //      App:Image:YUVs(X)
    //      Hal:Image:YUVs
    //
    ADD_IN_STREAM(App, Meta, rConfigParams.pMeta_Control);
    ADD_IN_STREAM(Hal, Meta, mpHalMeta_DynamicP1);
    //
    ADD_IN_STREAM(Hal, Image, mpHalImage_P1_Raw);
    //
    ADD_OUT_STREAM(Hal, Meta, mpHalMeta_DynamicP2);
    ADD_OUT_STREAM(Hal, Image, mpHalImage_Jpeg_YUV);
    ADD_OUT_STREAM(Hal, Image, mpHalImage_Thumbnail_YUV);
    //
    // config node
    {
        P2Node::ConfigParams cfgParams;
        //
        cfgParams.pInAppMeta    = rConfigParams.pMeta_Control;
        cfgParams.pInHalMeta    = mpHalMeta_DynamicP1;
        cfgParams.pOutAppMeta   = NULL;
        cfgParams.pOutHalMeta   = mpHalMeta_DynamicP2;
        //
        if( mpHalImage_P1_Raw.get() )
            cfgParams.pvInFullRaw.push_back(mpHalImage_P1_Raw);
        cfgParams.pInResizedRaw = NULL;
        //
        // config yuvs for jpeg
        if( mpHalImage_Jpeg_YUV.get() )
            cfgParams.vOutImage.push_back(mpHalImage_Jpeg_YUV);
        if( mpHalImage_Thumbnail_YUV.get() )
            cfgParams.vOutImage.push_back(mpHalImage_Thumbnail_YUV);
        //
        MERROR err = mpNode_P2_VSS->config(cfgParams);
        if  ( OK != err ) {
            MY_LOGE("mpNode_P2_VSS->config - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
configNodeLocked_fdnode(
    PipeConfigParams const& rConfigParams
    )
{
    NodeId_T const nodeId = eNODEID_FDNode;
    DECLARE_INOUT_STREAM(nodeId)
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Image:YUVs
    //  [output]
    //      App:Meta:Dynamic03
    //
    ADD_IN_STREAM(App, Meta, rConfigParams.pMeta_Control);
    ADD_OUT_STREAM(App, Meta, mpAppMeta_DynamicFD);
    ADD_IN_STREAM(Hal, Image, mpHalImage_FD_YUV);
    //
    // config node
    {
        FdNode::ConfigParams cfgParams;
        cfgParams.pInAppMeta = rConfigParams.pMeta_Control;
        cfgParams.pOutAppMeta = mpAppMeta_DynamicFD;
        cfgParams.vInImage = mpHalImage_FD_YUV;
        //
        MERROR err = mpNode_FD->config(cfgParams);
        if  ( OK != err ) {
            MY_LOGE("mpNode_FD->config - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
configNodeLocked_jpegnode(
    PipeConfigParams const& rConfigParams
    )
{
    NodeId_T const nodeId = eNODEID_JpegNode;
    DECLARE_INOUT_STREAM(nodeId)
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Image:YUVs
    //  [output]
    //      App:Meta:Dynamic03
    //
    ADD_IN_STREAM(App, Meta, rConfigParams.pMeta_Control);
    ADD_IN_STREAM(Hal, Meta, mpHalMeta_DynamicP2);
    ADD_IN_STREAM(Hal, Image, mpHalImage_Jpeg_YUV);
    ADD_IN_STREAM(Hal, Image, mpHalImage_Thumbnail_YUV);
    //
    ADD_OUT_STREAM(App, Meta, mpAppMeta_DynamicJpeg);
    ADD_OUT_STREAM(App, Image, mpAppImage_Jpeg);
    //
    // config node
    {
        JpegNode::ConfigParams cfgParams;
        cfgParams.pInAppMeta        = rConfigParams.pMeta_Control;
        cfgParams.pInHalMeta        = mpHalMeta_DynamicP2;
        cfgParams.pOutAppMeta       = mpAppMeta_DynamicJpeg;
        cfgParams.pInYuv_Main       = mpHalImage_Jpeg_YUV;
        cfgParams.pInYuv_Thumbnail  = mpHalImage_Thumbnail_YUV;
        cfgParams.pOutJpeg          = mpAppImage_Jpeg;
        //
        MERROR err = mpNode_Jpeg->config(cfgParams);
        if  ( OK != err ) {
            MY_LOGE("mpNode_Jpeg->config - err:%d(%s)", err, ::strerror(-err));
            return err;
        }
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
#undef ADD_OUT_STREAM
#undef ADD_IN_STREAM
#undef ADD_STREAM
#undef DECLARE_INOUT_STREAM
/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateIOStreamsLocked_p1node(
    evaluate_node_io_streams_params& rParams
) const
{
    //
    //  [input]
    //      App:Meta:Control
    //  [output]
    //      App:Meta:Dynamic01
    //      Hal:Meta:Dynamic01
    //      Hal:Image:RAW
    //      Hal:Image:ResizerRaw
    //
    if  ( ! mpNode_P1.get() )
    {
        MY_LOGE("node is not created");
        return -EINVAL;
    }
    //
    if  (
            ! rParams.vOHalImage.isEmpty()
            &&  ! rParams.vOAppImage.isEmpty()
        )
    {
        MY_LOGE(
                "Unexpected output streams, "
                "HalImage#:%zu AppImage#:%zu",
                rParams.vOHalImage.size(), rParams.vOAppImage.size()
               );
        return -EINVAL;
    }
    //
    rParams.vIAppMeta.add(mpAppMeta_Control->getStreamId()   , mpAppMeta_Control);
    rParams.vIHalMeta.add(mpHalMeta_Control->getStreamId()   , mpHalMeta_Control);
    rParams.vOAppMeta.add(mpAppMeta_DynamicP1->getStreamId() , mpAppMeta_DynamicP1);
    rParams.vOHalMeta.add(mpHalMeta_DynamicP1->getStreamId() , mpHalMeta_DynamicP1);
    if( mpHalImage_P1_ResizerRaw.get() )
        rParams.vOHalImage.add(mpHalImage_P1_ResizerRaw->getStreamId() , mpHalImage_P1_ResizerRaw);
    if( mpHalImage_P1_Raw.get() )
        rParams.vOHalImage.add(mpHalImage_P1_Raw->getStreamId()        , mpHalImage_P1_Raw);
    //
    //InfoIOMapSet::ImageInfoIOMapSet
    {
        ImageInfoIOMap map;
        //map.vOut.add
        if( mpHalImage_P1_ResizerRaw.get() )
            map.vOut.add(mpHalImage_P1_ResizerRaw->getStreamId() , mpHalImage_P1_ResizerRaw);
        if( mpHalImage_P1_Raw.get() )
            map.vOut.add(mpHalImage_P1_Raw->getStreamId()        , mpHalImage_P1_Raw);
        rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
    }
    //
    //InfoIOMapSet::MetaInfoIOMapSet
    {
        MetaInfoIOMap map;
        map.vIn .add(mpAppMeta_Control->getStreamId()   , mpAppMeta_Control);
        map.vIn .add(mpHalMeta_Control->getStreamId()   , mpHalMeta_Control);
        map.vOut.add(mpAppMeta_DynamicP1->getStreamId() , mpAppMeta_DynamicP1);
        map.vOut.add(mpHalMeta_DynamicP1->getStreamId() , mpHalMeta_DynamicP1);
        rParams.pIOMapSet->mMetaInfoIOMapSet.push_back(map);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateIOStreamsLocked_p2node(
    evaluate_node_io_streams_params& rParams
) const
{
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Meta:DynamicP1
    //      Hal:Image:RAW
    //  [output]
    //      App:Meta:DynamicP2
    //      App:Image:YUVs
    //      Hal:Image:YUVs
    //
    if  ( ! mpNode_P2.get() )
    {
        MY_LOGE("node is not created");
        return -EINVAL;
    }
    //
    if  (
            ! rParams.vOAppMeta.isEmpty()
        )
    {
        MY_LOGE(
                "Unexpected output streams, "
                "HalImage#:%zu HalMeta#:%zu AppMeta#:%zu",
                rParams.vOHalImage.size(),
                rParams.vOHalMeta.size(),
                rParams.vOAppMeta.size()
               );
        return -EINVAL;
    }
    for (size_t i = 0; i < rParams.vOAppImage.size(); i++)
    {
        if  ( 0 > mvAppYuvImage.indexOfKey(rParams.vOAppImage.keyAt(i)) ) {
            MY_LOGE("Unexpected App yuv image output streams");
            return -EINVAL;
        }
    }
    //
    rParams.vIAppMeta.add(mpAppMeta_Control->getStreamId()  , mpAppMeta_Control);
    rParams.vIHalMeta.add(mpHalMeta_DynamicP1->getStreamId(), mpHalMeta_DynamicP1);
    rParams.vOAppMeta.add(mpAppMeta_DynamicP2->getStreamId(), mpAppMeta_DynamicP2);
    if ( ! mConfigProfile.mbConfigP2VSS )
        rParams.vOHalMeta.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);
    //
    if( mvYuvStreams_Fullraw.size() > 0 )
    {
        ImageInfoIOMap map;
        for( size_t idx = 0; idx < mvYuvStreams_Fullraw.size(); idx++) {
            StreamId_T const streamId        = mvYuvStreams_Fullraw[idx];
            ssize_t index                    = -1;
            //
            // skip jpeg related yuv streams if configured as vss flow
            if ( mConfigProfile.mbConfigP2VSS )
            {
                if (
                        isStream(mpHalImage_Jpeg_YUV, streamId) ||
                        isStream(mpHalImage_Thumbnail_YUV, streamId)
                   )
                    continue;
            }
            // app streams
            index = rParams.vOAppImage.indexOfKey(streamId);
            if  ( 0 <= index ) {
                map.vOut.add(streamId, rParams.vOAppImage.valueAt(index));
            }
            // hal streams
            index = rParams.vOHalImage.indexOfKey(streamId);
            if  ( 0 <= index ) {
                map.vOut.add(streamId, rParams.vOHalImage.valueAt(index));
            }
        }
        //
        if( map.vOut.size() > 0 ) {
            sp<IImageStreamInfo> pInStreamInfo = mpHalImage_P1_Raw;
            rParams.vIHalImage.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            map.vIn.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            //
            rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
        }
    }

    if( mvYuvStreams_Resizedraw.size() > 0 )
    {
        ImageInfoIOMap map;
        for( size_t idx = 0; idx < mvYuvStreams_Resizedraw.size(); idx++) {
            StreamId_T const streamId        = mvYuvStreams_Resizedraw[idx];
            ssize_t index                    = -1;
            //
            // skip jpeg related yuv streams if configured as vss flow
            if ( mConfigProfile.mbConfigP2VSS )
            {
                if (
                        isStream(mpHalImage_Jpeg_YUV, streamId) ||
                        isStream(mpHalImage_Thumbnail_YUV, streamId)
                   )
                    continue;
            }
            // app streams
            index = rParams.vOAppImage.indexOfKey(streamId);
            if  ( 0 <= index ) {
                map.vOut.add(streamId, rParams.vOAppImage.valueAt(index));
            }
            // hal streams
            index = rParams.vOHalImage.indexOfKey(streamId);
            if  ( 0 <= index ) {
                map.vOut.add(streamId, rParams.vOHalImage.valueAt(index));
            }
        }
        //
        if( map.vOut.size() > 0 ) {
            sp<IImageStreamInfo> pInStreamInfo = mpHalImage_P1_ResizerRaw;
            rParams.vIHalImage.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            map.vIn.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            //
            rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
        }
    }
    //
    //InfoIOMapSet::MetaInfoIOMapSet
    {
        MetaInfoIOMap map;
        map.vIn.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
        map.vIn.add(mpHalMeta_DynamicP1->getStreamId(), mpHalMeta_DynamicP1);
        map.vOut.add(mpAppMeta_DynamicP2->getStreamId(), mpAppMeta_DynamicP2);
        if ( ! mConfigProfile.mbConfigP2VSS )
            map.vOut.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);
        rParams.pIOMapSet->mMetaInfoIOMapSet.push_back(map);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateIOStreamsLocked_p2node_VSS(
    evaluate_node_io_streams_params& rParams
) const
{
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Meta:DynamicP1
    //      Hal:Image:RAW
    //  [output]
    //      Hal:Image:YUVs
    //
    if  ( ! mpNode_P2_VSS.get() )
    {
        MY_LOGE("node is not created");
        return -EINVAL;
    }
    //
    if  (
            ! rParams.vOAppMeta.isEmpty()
        )
    {
        MY_LOGE(
                "Unexpected output streams, "
                "HalImage#:%zu HalMeta#:%zu AppMeta#:%zu",
                rParams.vOHalImage.size(),
                rParams.vOHalMeta.size(),
                rParams.vOAppMeta.size()
               );
        return -EINVAL;
    }
    for (size_t i = 0; i < rParams.vOAppImage.size(); i++)
    {
        if  ( 0 > mvAppYuvImage.indexOfKey(rParams.vOAppImage.keyAt(i)) ) {
            MY_LOGE("Unexpected App image output streams");
            return -EINVAL;
        }
    }
    //
    rParams.vIAppMeta.add(mpAppMeta_Control->getStreamId()   , mpAppMeta_Control);
    rParams.vIHalMeta.add(mpHalMeta_DynamicP1->getStreamId() , mpHalMeta_DynamicP1);
    rParams.vOHalMeta.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);

    if( mvYuvStreams_Fullraw.size() > 0 )
    {
        ImageInfoIOMap map;
        for( size_t idx = 0; idx < mvYuvStreams_Fullraw.size(); idx++) {
            StreamId_T const streamId        = mvYuvStreams_Fullraw[idx];
            ssize_t index                    = -1;
            //
            // skip not jpeg related yuv streams if configured as vss flow
            if (
                    ! isStream(mpHalImage_Jpeg_YUV, streamId) &&
                    ! isStream(mpHalImage_Thumbnail_YUV, streamId)
               )
                continue;
            // hal streams
            index = rParams.vOHalImage.indexOfKey(streamId);
            if  ( 0 <= index )
                map.vOut.add(streamId, rParams.vOHalImage.valueAt(index));
        }
        //
        if( map.vOut.size() > 0 ) {
            sp<IImageStreamInfo> pInStreamInfo = mpHalImage_P1_Raw;
            rParams.vIHalImage.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            map.vIn.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            //
            rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
        }
    }

    if( mvYuvStreams_Resizedraw.size() > 0 )
    {
        ImageInfoIOMap map;
        for( size_t idx = 0; idx < mvYuvStreams_Resizedraw.size(); idx++) {
            StreamId_T const streamId        = mvYuvStreams_Resizedraw[idx];
            ssize_t index                    = -1;
            //
            // skip not jpeg related yuv streams if configured as vss flow
            if (
                    ! isStream(mpHalImage_Jpeg_YUV, streamId) &&
                    ! isStream(mpHalImage_Thumbnail_YUV, streamId)
               )
                continue;
            // hal streams
            index = rParams.vOHalImage.indexOfKey(streamId);
            if  ( 0 <= index )
                map.vOut.add(streamId, rParams.vOHalImage.valueAt(index));
        }
        //
        if( map.vOut.size() > 0 ) {
            sp<IImageStreamInfo> pInStreamInfo = mpHalImage_P1_ResizerRaw;
            rParams.vIHalImage.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            map.vIn.add(pInStreamInfo->getStreamId(), pInStreamInfo);
            //
            rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
        }
    }
    //
    //InfoIOMapSet::MetaInfoIOMapSet
    {
        MetaInfoIOMap map;
        map.vIn.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
        map.vIn.add(mpHalMeta_DynamicP1->getStreamId(), mpHalMeta_DynamicP1);
        map.vOut.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);
        rParams.pIOMapSet->mMetaInfoIOMapSet.push_back(map);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateIOStreamsLocked_fdnode(
    evaluate_node_io_streams_params& rParams
) const
{
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Image:YUV
    //  [output]
    //      App:Meta:Dynamic03
    //
    //
    if  ( ! mpNode_FD.get() )
    {
        MY_LOGE("node is not created");
        return -EINVAL;
    }
    //
    rParams.vIAppMeta.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
    //rParams.vOAppMeta.add(mpAppMeta_DynamicFD->getStreamId(), mpAppMeta_DynamicFD);
    if( mpHalImage_FD_YUV.get() )
        rParams.vIHalImage.add(mpHalImage_FD_YUV->getStreamId(), mpHalImage_FD_YUV);
    //
    //
    //InfoIOMapSet::ImageInfoIOMapSet
    {
        ImageInfoIOMap map;
        map.vIn.add(mpHalImage_FD_YUV->getStreamId(), mpHalImage_FD_YUV);
        rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
    }
    //
    //InfoIOMapSet::MetaInfoIOMapSet
    {
        MetaInfoIOMap map;
        map.vIn.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
        map.vOut.add(mpAppMeta_DynamicFD->getStreamId(), mpAppMeta_DynamicFD);
        rParams.pIOMapSet->mMetaInfoIOMapSet.push_back(map);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
evaluateIOStreamsLocked_jpegnode(
    evaluate_node_io_streams_params& rParams
) const
{
    //
    //  [input]
    //      App:Meta:Control
    //      Hal:Image:Jpeg_YUV
    //      Hal:Image:Thumbnail_YUV
    //  [output]
    //      App:Meta:DynamicJpeg
    //      App:Image:Jpeg
    //
    //
    if  ( ! mpNode_Jpeg.get() )
    {
        MY_LOGE("node is not created");
        return -EINVAL;
    }
    //
    if  ( 0 > rParams.vOAppImage.indexOfKey(mpAppImage_Jpeg->getStreamId()) ) {
        MY_LOGE("Unexpected App image output streams");
        return -EINVAL;
    }
    //
    rParams.vIAppMeta.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
    rParams.vIHalMeta.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);
    rParams.vOAppMeta.add(mpAppMeta_DynamicJpeg->getStreamId(), mpAppMeta_DynamicJpeg);
    //
    //Hal:Image:Jpeg_YUV
    rParams.vIHalImage.add(mpHalImage_Jpeg_YUV->getStreamId(), mpHalImage_Jpeg_YUV);
    //Hal:Image:Thumbnail_YUV
    sp<IImageStreamInfo> pHalImage_Thumbnail_YUV;
    createStreamInfoLocked_Thumbnail_YUV(rParams.pIAppMetaData, pHalImage_Thumbnail_YUV);
    if  ( pHalImage_Thumbnail_YUV.get() ) {
        rParams.vIHalImage.add(pHalImage_Thumbnail_YUV->getStreamId(), pHalImage_Thumbnail_YUV);
    }
    //
    //
    //InfoIOMapSet::ImageInfoIOMapSet
    {
        ImageInfoIOMap map;
        map.vIn.add(mpHalImage_Jpeg_YUV->getStreamId(), mpHalImage_Jpeg_YUV);
        if  ( pHalImage_Thumbnail_YUV.get() ) {
            map.vIn.add(pHalImage_Thumbnail_YUV->getStreamId(), pHalImage_Thumbnail_YUV);
        }
        map.vOut.add(mpAppImage_Jpeg->getStreamId(), mpAppImage_Jpeg);
        rParams.pIOMapSet->mImageInfoIOMapSet.push_back(map);
    }
    //
    //InfoIOMapSet::MetaInfoIOMapSet
    {
        MetaInfoIOMap map;
        map.vIn.add(mpAppMeta_Control->getStreamId(), mpAppMeta_Control);
        map.vIn.add(mpHalMeta_DynamicP2->getStreamId(), mpHalMeta_DynamicP2);
        map.vOut.add(mpAppMeta_DynamicJpeg->getStreamId(), mpAppMeta_DynamicJpeg);
        rParams.pIOMapSet->mMetaInfoIOMapSet.push_back(map);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
PipelineDefaultImp::
calcThumbnailYuvSize(
    MSize const rPicSize,
    MSize const rThumbnailsize
) const
{
#define align2(x) (((x) + 1) & (~0x1))
    MSize size;
    MUINT32 const val0 = rPicSize.w * rThumbnailsize.h;
    MUINT32 const val1 = rPicSize.h * rThumbnailsize.w;
    if( val0 > val1 ) {
        size.w = align2(val0/rPicSize.h);
        size.h = rThumbnailsize.h;
    }
    else if( val0 < val1 ) {
        size.w = rThumbnailsize.w;
        size.h = align2(val1/rPicSize.w);
    }
    else {
        size = rThumbnailsize;
    }
#undef align2
    MY_LOGD_IF(1, "thumb %dx%d, pic %dx%d -> yuv for thumb %dx%d",
            rThumbnailsize.w, rThumbnailsize.h,
            rPicSize.w, rPicSize.h,
            size.w, size.h
            );
    return size;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
createStreamInfoLocked_Thumbnail_YUV(
    IMetadata const* pMetadata,
    android::sp<IImageStreamInfo>& rpStreamInfo
) const
{
    if  ( mpHalImage_Thumbnail_YUV == 0 ) {
        MY_LOGW("No config stream: Thumbnail_YUV");
        return NO_INIT;
    }
    //
    IMetadata::IEntry const& entryThumbnailSize = pMetadata->entryFor(MTK_JPEG_THUMBNAIL_SIZE);
    if  ( entryThumbnailSize.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_THUMBNAIL_SIZE");
        return NAME_NOT_FOUND;
    }
    MSize const& thumbnailSize = entryThumbnailSize.itemAt(0, Type2Type<MSize>());
    if  ( ! thumbnailSize ) {
        MY_LOGW("Bad thumbnail size: %dx%d", thumbnailSize.w, thumbnailSize.h);
        return NOT_ENOUGH_DATA;
    }
    //
    MSize const yuvthumbnailsize = calcThumbnailYuvSize(
                                        mpHalImage_Jpeg_YUV->getImgSize(),
                                        thumbnailSize
                                        );
    //
    //
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return NAME_NOT_FOUND;
    }
    MINT32 const jpegOrientation = entryJpegOrientation.itemAt(0, Type2Type<MINT32>());
    //
    MINT const format = mpHalImage_Thumbnail_YUV->getImgFormat();
    IImageStreamInfo::BufPlanes_t bufPlanes;
    switch (format)
    {
    case eImgFmt_YUY2:{
        IImageStreamInfo::BufPlane bufPlane;
        bufPlane.rowStrideInBytes = (yuvthumbnailsize.w << 1);
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * yuvthumbnailsize.h;
        bufPlanes.push_back(bufPlane);
        }break;
    default:
        MY_LOGE("not supported format: %#x", format);
        break;
    }
    //
    rpStreamInfo = new ImageStreamInfo(
        mpHalImage_Thumbnail_YUV->getStreamName(),
        mpHalImage_Thumbnail_YUV->getStreamId(),
        mpHalImage_Thumbnail_YUV->getStreamType(),
        mpHalImage_Thumbnail_YUV->getMaxBufNum(),
        mpHalImage_Thumbnail_YUV->getMinInitBufNum(),
        mpHalImage_Thumbnail_YUV->getUsageForAllocator(),
        format,
        yuvthumbnailsize,
        bufPlanes
    );
    if  ( rpStreamInfo == 0 ) {
        MY_LOGE(
            "fail to new ImageStreamInfo: %s %#"PRIxPTR,
            mpHalImage_Thumbnail_YUV->getStreamName(),
            mpHalImage_Thumbnail_YUV->getStreamId()
        );
        return NO_MEMORY;
    }
    //
    MY_LOGD_IF(
        1,
        "streamId:%#"PRIxPTR" %s %p %p yuvthumbnailsize:%dx%d jpegOrientation:%d",
        rpStreamInfo->getStreamId(),
        rpStreamInfo->getStreamName(),
        rpStreamInfo.get(),
        mpHalImage_Thumbnail_YUV.get(),
        yuvthumbnailsize.w, yuvthumbnailsize.h, jpegOrientation
    );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineDefaultImp::
createHalStreamBufferLocked_Thumbnail_YUV(
    android::sp<IImageStreamInfo> const pStreamInfo,
    android::sp<HalImageStreamBuffer>& rpStreamBuffer
) const
{
    String8 const str = String8::format(
        "%s StreamId:%#"PRIxPTR" %dx%d %p %p",
        pStreamInfo->getStreamName(),
        pStreamInfo->getStreamId(),
        pStreamInfo->getImgSize().w,
        pStreamInfo->getImgSize().h,
        pStreamInfo.get(),
        mpHalImage_Thumbnail_YUV.get()
    );
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
    //
    rpStreamBuffer = HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)();
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGE("Fail to allocate - %s", str.string());
        return NO_MEMORY;
    }
    //
    MY_LOGD_IF(1, "%s", str.string());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineDefaultImp::
clearVar()
{
    mInFlightRequest->clear();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
            {
                size_t raw_stride = ALIGN_2(((imgSize.w + 8/*hw constraint*/)* 10 + 7)>> 3);
                addBufPlane(bufPlanes , imgSize.h      , raw_stride);
            }
            break;
        case eImgFmt_YV12:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
            break;
        default:
            MY_LOGE("format not support yet %p", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#"PRIxPTR,
                streamName, streamId);
    }

    return pStreamInfo;
}
