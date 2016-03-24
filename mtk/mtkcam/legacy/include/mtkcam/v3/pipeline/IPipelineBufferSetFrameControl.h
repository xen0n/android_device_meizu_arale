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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <mtkcam/v3/stream/IStreamBufferSet.h>
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include "IPipelineNode.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * An interface of pipeline stream buffer provider.
 */
struct IPipelineStreamBufferProvider
    : public virtual android::RefBase
{
public:     ////                Definitions.
    typedef NSCam::v3::Utils::HalImageStreamBuffer  HalImageStreamBuffer;

public:     ////                Operations.
    virtual MERROR              acquireHalStreamBuffer(
                                    android::sp<IImageStreamInfo> const pStreamInfo,
                                    android::sp<HalImageStreamBuffer>& rpStreamBuffer
                                )   const                                   = 0;

};


/**
 * An interface of pipeline frame node map.
 */
class IPipelineFrameNodeMapControl
    : public virtual android::RefBase
{
public:     ////
    typedef IPipelineFrameNodeMapControl        ThisT;
    typedef IPipelineFrame::NodeId_T            NodeId_T;
    typedef IPipelineFrame::InfoIOMapSet        InfoIOMapSet;
    typedef IStreamInfoSet                      IStreamInfoSetT;
    typedef android::sp<IStreamInfoSetT>        IStreamInfoSetPtr;
    typedef android::sp<IStreamInfoSetT const>  IStreamInfoSetPtr_CONST;

public:     ////                Definitions.
    /**
     *
     */
    struct  INode
        : public virtual android::RefBase
    {
        virtual NodeId_T        getNodeId() const                           = 0;

        virtual IStreamInfoSetPtr_CONST
                                getIStreams() const                         = 0;
        virtual MVOID           setIStreams(IStreamInfoSetPtr p)            = 0;

        virtual IStreamInfoSetPtr_CONST
                                getOStreams() const                         = 0;
        virtual MVOID           setOStreams(IStreamInfoSetPtr p)            = 0;

        virtual InfoIOMapSet const&
                                getInfoIOMapSet() const                     = 0;
        virtual InfoIOMapSet&   editInfoIOMapSet()                          = 0;
    };

public:     ////                Operations.
    static  ThisT*              create();

    virtual ssize_t             setCapacity(size_t size)                    = 0;
    virtual MVOID               clear()                                     = 0;
    virtual ssize_t             addNode(NodeId_T const nodeId)              = 0;

public:     ////                Operations.
    virtual MBOOL               isEmpty() const                             = 0;
    virtual size_t              size() const                                = 0;

    virtual INode*              getNodeFor(NodeId_T const nodeId) const     = 0;
    virtual INode*              getNodeAt(size_t index) const               = 0;

};


/**
 * An interface of pipeline buffer set control.
 */
class IPipelineBufferSetControl
    : public IStreamBufferSet
{
public:     ////                Definitions.
    typedef NSCam::v3::Utils::HalImageStreamBuffer  HalImageStreamBuffer;
    typedef NSCam::v3::Utils::HalMetaStreamBuffer   HalMetaStreamBuffer;

public:     ////                Definitions.
    /**
     * Stream Buffer Map Interface.
     */
    template <class _StreamBuffer_>
    class IMap
        : public virtual android::RefBase
    {
    public:     ////            Definitions.
        typedef _StreamBuffer_  StreamBufferT;
        typedef typename StreamBufferT::IStreamInfoT
                                IStreamInfoT;

    public:     ////            Operations.
        virtual ssize_t         add(
                                    android::sp<IStreamInfoT> pStreamInfo,
                                    android::sp<IUsersManager> pUsersManager
                                )                                           = 0;

        virtual ssize_t         add(
                                    android::sp<StreamBufferT> value
                                )                                           = 0;

        virtual ssize_t         setCapacity(size_t size)                    = 0;

        virtual bool            isEmpty() const                             = 0;

        virtual size_t          size() const                                = 0;

        virtual ssize_t         indexOfKey(StreamId_T const key) const      = 0;

        virtual StreamId_T      keyAt(size_t index) const                   = 0;

        virtual IUsersManager*  usersManagerAt(size_t index) const          = 0;

        virtual IStreamInfoT*   streamInfoAt(size_t index) const            = 0;

    };

public:     ////                Operations.
    virtual android::sp<IMap<IImageStreamBuffer> >
                                editMap_AppImage()                          = 0;

    virtual android::sp<IMap<IMetaStreamBuffer> >
                                editMap_AppMeta()                           = 0;

    virtual android::sp<IMap<HalImageStreamBuffer> >
                                editMap_HalImage()                          = 0;

    virtual android::sp<IMap<HalMetaStreamBuffer> >
                                editMap_HalMeta()                           = 0;

public:     ////                Operations.
    virtual MUINT32             getFrameNo() const                          = 0;

};


/**
 * An interface of (in-flight) pipeline frame control.
 */
class IPipelineBufferSetFrameControl
    : public IPipelineFrame
    , public IPipelineBufferSetControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef IPipelineBufferSetFrameControl          ThisT;

public:     ////
    typedef NSCam::v3::Utils::HalImageStreamBuffer  HalImageStreamBuffer;
    typedef NSCam::v3::Utils::HalMetaStreamBuffer   HalMetaStreamBuffer;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Callback.
    /**
     *
     */
    class IAppCallback
        : public virtual android::RefBase
    {
    public:     ////            Operations.
        virtual MVOID           updateFrame(
                                    MUINT32 const requestNo,
                                    MINTPTR const userId,
                                    ssize_t const /*nOutMetaLeft*/,
                                    android::Vector<android::sp<IMetaStreamBuffer> > /*vOutMeta*/
                                )                                           = 0;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Creation.
    static  ThisT*              create(
                                    MUINT32 requestNo,
                                    MUINT32 frameNo,
                                    android::wp<IAppCallback>const& pAppCallback,
                                    IPipelineStreamBufferProvider const* pBufferProvider,
                                    android::wp<IPipelineNodeCallback> pNodeCallback = NULL
                                );

public:     ////                Configuration.
    virtual MERROR              startConfiguration()                        = 0;
    virtual MERROR              finishConfiguration()                       = 0;

    virtual MERROR              setNodeMap(
                                    android::sp<IPipelineFrameNodeMapControl> value
                                )                                           = 0;

    virtual MERROR              setPipelineNodeMap(
                                    android::sp<IPipelineNodeMap const> value
                                )                                           = 0;

    virtual MERROR              setPipelineDAG(
                                    android::sp<IPipelineDAG const> value
                                )                                           = 0;

    virtual MERROR              setStreamInfoSet(
                                    android::sp<IStreamInfoSet const> value
                                )                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_

