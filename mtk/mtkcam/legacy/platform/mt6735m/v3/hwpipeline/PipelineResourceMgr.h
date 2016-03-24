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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINERESOURCEMGR_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINERESOURCEMGR_H_
//
#include "MyUtils.h"
//
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include <mtkcam/v3/pipeline/IPipelineResourceMgr.h>
#include <mtkcam/v3/pipeline/IScenarioControl.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
/**
 * An implementation of pipeline resource manager.
 */
class IPipelineModel;
class PipelineResourceMgr
    : public IPipelineResourceMgr
{
public:     ////                    Definitions.

    typedef Pipeline_NodeId_T       NodeId_T;

    typedef NSCam::v3::Utils::      HalMetaStreamBuffer
                                    HalMetaStreamBuffer;

    typedef NSCam::v3::Utils::      HalImageStreamBuffer
                                    HalImageStreamBuffer;

    typedef HalMetaStreamBuffer::   Allocator
                                    HalMetaStreamBufferAllocatorT;

    typedef HalImageStreamBuffer::  Allocator
                                    HalImageStreamBufferAllocatorT;

    typedef HalMetaStreamBufferAllocatorT::StreamBufferPoolT
                                    HalMetaStreamBufferPoolT;

    typedef HalImageStreamBufferAllocatorT::StreamBufferPoolT
                                    HalImageStreamBufferPoolT;

    typedef android::DefaultKeyedVector <StreamId_T, android::sp<HalMetaStreamBufferPoolT> >
                                    HalMetaStreamBufferPoolMapT;

    typedef android::DefaultKeyedVector<StreamId_T, android::sp<HalImageStreamBufferPoolT> >
                                    HalImageStreamBufferPoolMapT;

    typedef NSCam::v3::Utils::IStreamBufferPool<IImageStreamBuffer>
                                    IImageStreamBufferPoolT;

    typedef android::KeyedVector< NodeId_T, android::sp< IPipelineNode > >
                                    NodeList;

    typedef android::KeyedVector< NodeId_T, android::sp< MVOID > >
                                    NodeParamList;

    typedef android::KeyedVector< StreamId_T, android::sp< IStreamInfo > >
                                    StreamList;

    typedef IPipelineNode::InitParams
                                    InitParams;

    struct NodeStatus
    {
        enum
        {
            UNINIT                  = 0,
            UNCONFIG                = 1,
            RECONFIG                = 2,
            INITIALIZED             = 3,
        };
    };

    struct StreamStatus
    {
        enum
        {
            UNINIT                  = 0,
            UNCONFIG                = 1,
            RECONFIG                = 2,
            INITIALIZED             = 3,
        };
    };

    struct SceneStatus {
        enum
        {
            UNINIT                  = 0,
            UNCONFIG                = 1,
            RECONFIG                = 2,
            INITIALIZED             = 3,
        };
    };

    struct NodesInitParams
        : public android::DefaultKeyedVector< NodeId_T, InitParams >
    {
    };

public:
    static
    android::sp<PipelineResourceMgr>    getInstance(MINT32 const openId);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //                       Instantiation
                                        ~PipelineResourceMgr();

    virtual MVOID                       onDestroyLocked();

                                        PipelineResourceMgr(MINT32 const openId);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipelineResourceMgr Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //                       Operations

    MERROR                              getReusableNodes(
                                            android::Vector<NodeId_T> &vNodeList
                                        );

    MERROR                              getReusableStreams(
                                            android::Vector<StreamId_T> &vStreamList
                                        );

    MERROR                              queryNode(
                                            NodeId_T nodeid,
                                            android::sp< IPipelineNode >& pNode,
                                            InitParams const& rParams
                                        );

    android::sp<HalImageStreamBufferPoolT>
                                        queryStream(
                                            StreamId_T streamid,
                                            android::sp<IImageStreamInfo>&        pStreamInfo
                                        );

    MERROR                              acquireHalStreamBuffer(
                                            android::sp<IImageStreamInfo> const   pStreamInfo,
                                            android::sp<HalImageStreamBuffer>&    rpStreamBuffer
                                        )   const;


    MERROR                              evaluateNodeReconfigure(
                                            NodeId_T nodeid,
                                            MVOID *pConfigParam
                                        );

    android::sp<IScenarioControl>       getScenarioCtrl(MINT32 const openId);

    MERROR                              setScenarioCtrl(
                                            MVOID* pBWCparam
                                        );

    MVOID                               registerOwnerOfNode(
                                            NodeId_T nodeid,
                                            IPipelineModel* pPipeline
                                        ) {}

    MVOID                               unregisterOwnerOfNode(
                                            NodeId_T nodeid
                                        ) {}

    MBOOL                               isOwnerOfNode(
                                            NodeId_T nodeid,
                                            IPipelineModel* pPipeline
                                        ) { return MFALSE; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MERROR                              createNode(
                                            NodeId_T nodeid,
                                            InitParams const& rParams
                                        );

    MERROR                              createStream(
                                            StreamId_T streamid,
                                            android::sp<IImageStreamInfo>&        pStreamInfo
                                        );

    template <class NodeT>
    MERROR                              checkInitParams(
                                            android::sp<NodeT> pNode,
                                            InitParams const& rParams
                                        );

    MERROR                              updateInitParams(
                                            NodeId_T nodeid,
                                            InitParams const& rParams
                                        );

    MERROR                              updateConfigParams_P1(
                                            NodeId_T nodeid,
                                            P1Node::ConfigParams* params
                                        );

    MERROR                              checkConfigParams_P1(
                                            NodeId_T nodeid,
                                            P1Node::ConfigParams* params
                                        );

    MERROR                              updateConfigParams_Jpeg(
                                            NodeId_T nodeid,
                                            JpegNode::ConfigParams* params
                                        );

    MERROR                              checkConfigParams_Jpeg(
                                            NodeId_T nodeid,
                                            JpegNode::ConfigParams* params
                                        );

    MBOOL                               isEqualImageStreamInfo(
                                            android::sp<IImageStreamInfo> pStreamInfo1,
                                            android::sp<IImageStreamInfo> pStreamInfo2
                                        );

    MBOOL                               isEqualMetaStreamInfo(
                                            android::sp<IMetaStreamInfo> pStreamInfo1,
                                            android::sp<IMetaStreamInfo> pStreamInfo2
                                        );


    char const*                         getName() const     { return mName.string(); }

    MERROR                              clear();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual void                        onLastStrongRef(const void* id);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //                       Data Members

    mutable android::Mutex              mLock;
    android::String8 const              mName;
    MINT32 const                        mOpenId;
    NodeList                            mvReusableNodes;
//    NodeParamList                       mvNodesParams;
    StreamList                          mvReusableStreams;
    // node related
    android::KeyedVector<NodeId_T, MUINT32>
                                        mvNodeStatus;

    android::KeyedVector<NodeId_T, MUINT32>
                                        mvStreamStatus;
    android::KeyedVector< StreamId_T, android::sp<IImageStreamInfo> >
                                        mvStreamInfo;

    HalImageStreamBufferPoolMapT        mHalImageStreamBufferPoolMap;

    //
    android::sp<IScenarioControl>       mpScenarioCtrl;
    MUINT32                             mSceneStatus;
    ScenarioControl::BWC_Scenario_Param mSceneCtrlParams;

    // Nodes
    android::sp<P1Node>                 mpNode_P1;
    android::sp<JpegNode>               mpNode_Jpeg;
    //
    NodesInitParams                     mvInitParams;
    P1Node::ConfigParams                mConfigParams_P1;
    JpegNode::InitParams                mInitParams_Jpeg;
    JpegNode::ConfigParams              mConfigParams_Jpeg;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINERESOURCEMGR_H_

