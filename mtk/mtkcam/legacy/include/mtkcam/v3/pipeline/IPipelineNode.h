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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODE_H_
//
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <mtkcam/common.h>
#include "../stream/IStreamBufferSet.h"
#include "../stream/IStreamInfo.h"
#include "IPipelineDAG.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineNodeMap;
class IPipelineFrame;
class IPipelineNode;
class IPipelineDAG;


/**
 * Type of Camera Pipeline Node Id.
 */
typedef MINTPTR                     Pipeline_NodeId_T;


/**
 * An interface of Pipeline node map (key:NodeId_T, value:NodePtr_T).
 */
class IPipelineNodeMap
    : public virtual android::RefBase
{
public:     ////                    Definitions.
    typedef Pipeline_NodeId_T       NodeId_T;
    typedef IPipelineNode           NodeT;
    typedef android::sp<NodeT>      NodePtrT;

public:     ////                    Operations.
    virtual MBOOL                   isEmpty() const                         = 0;
    virtual size_t                  size() const                            = 0;

    virtual NodePtrT                nodeFor(NodeId_T const& id) const       = 0;
    virtual NodePtrT                nodeAt(size_t index) const              = 0;

};


/**
 * An interface of (in-flight) pipeline frame listener.
 */
class IPipelineFrameListener
    : public virtual android::RefBase
{
public:     ////                    Definitions.
    typedef Pipeline_NodeId_T       NodeId_T;

    enum
    {
        /** This frame is released */
        eMSG_FRAME_RELEASED,

        /** All output meta buffers released */
        eMSG_ALL_OUT_META_BUFFERS_RELEASED,

        /** All output image buffers released */
        eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED,

    };

    /**
     * Invoked when something happens.
     *
     * @param[in] frameNo: frame number.
     *
     * @param[in] message: a message to indicate what happen.
     *
     * @param[in] pCookie: the listener's cookie.
     */
    virtual MVOID                   onPipelineFrame(
                                        MUINT32 const frameNo,
                                        MUINT32 const message,
                                        MVOID*const pCookie
                                    )                                       = 0;

    /**
     * Invoked when something happens.
     *
     * @param[in] frameNo: frame number.
     *
     * @param[in] nodeId: node ID.
     *
     * @param[in] message: a message to indicate what happen.
     *
     * @param[in] pCookie: the listener's cookie.
     */
    virtual MVOID                   onPipelineFrame(
                                        MUINT32 const frameNo,
                                        NodeId_T const nodeId,
                                        MUINT32 const message,
                                        MVOID*const pCookie
                                    )                                       = 0;

};


struct IPipelineNodeCallback;
/**
 * An interface of (in-flight) pipeline frame.
 */
class IPipelineFrame
    : public virtual android::RefBase
{
public:     ////                    Definitions.
    typedef Pipeline_NodeId_T       NodeId_T;

    struct ImageInfoIOMap
    {
        android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamInfo> > vIn;
        android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamInfo> > vOut;
    };

    struct MetaInfoIOMap
    {
        android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamInfo> >  vIn;
        android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamInfo> >  vOut;
    };

    struct ImageInfoIOMapSet
        : public android::Vector<ImageInfoIOMap>
    {
    };

    struct MetaInfoIOMapSet
        : public android::Vector<MetaInfoIOMap>
    {
    };

    struct InfoIOMapSet
    {
        typedef IPipelineFrame::ImageInfoIOMapSet   ImageInfoIOMapSet;
        typedef IPipelineFrame::MetaInfoIOMapSet    MetaInfoIOMapSet;
        ImageInfoIOMapSet           mImageInfoIOMapSet;
        MetaInfoIOMapSet            mMetaInfoIOMapSet;
    };

public:     ////                    Operations.

    virtual MUINT32                 getFrameNo() const                      = 0;
    virtual MUINT32                 getRequestNo() const                    = 0;

    virtual android::sp<IPipelineNodeMap const>
                                    getPipelineNodeMap() const              = 0;
    virtual IPipelineDAG const&     getPipelineDAG() const                  = 0;
    virtual IStreamBufferSet&       getStreamBufferSet() const              = 0;
    virtual IStreamInfoSet const&   getStreamInfoSet() const                = 0;
    /**
     * Note: getPipelineNodeCallback() const
     * actually, IPipelineNodeCallback is stord as wp. Calling this
     * function, this module will help to promote IPipelineNodeCallback from wp
     * to sp.
     */
    virtual android::sp<IPipelineNodeCallback>
                                    getPipelineNodeCallback() const         = 0;

    virtual MERROR                  queryIOStreamInfoSet(
                                        NodeId_T const& nodeId,
                                        android::sp<IStreamInfoSet const>& rIn,
                                        android::sp<IStreamInfoSet const>& rOut
                                    ) const                                 = 0;

    virtual MERROR                  queryInfoIOMapSet(
                                        NodeId_T const& nodeId,
                                        InfoIOMapSet& rIOMapSet
                                    ) const                                 = 0;

    /**
     * Attach a pipeline frame listener.
     *
     * @param[in] pListener: the listener to attach.
     *
     * @param[in] pCookie: the listener's cookie.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  attachListener(
                                        android::wp<IPipelineFrameListener>const& pListener,
                                        MVOID*const pCookie
                                    )                                       = 0;

};


/**
 * An interface of pipeline node.
 */

class IPipelineNode
    : public virtual android::RefBase
{
public:     ////                    Definitions.
    typedef Pipeline_NodeId_T       NodeId_T;

    /**
     * Initialization Parameters.
     */
    struct  InitParams
    {
        /**
         * An index to indicate which camera device to open.
         */
        MINT32                      openId;

        /**
         * A unique value for the node id.
         */
        NodeId_T                    nodeId;

        /**
         * A pointer to a null-terminated string for the node name.
         */
        char const*                 nodeName;

    };

public:     ////                    Attributes.
    /**
     * @return
     *      An index to indicate which camera device to open.
     */
    virtual MINT32                  getOpenId() const                       = 0;

    /**
     * @return
     *      A unique node id.
     */
    virtual NodeId_T                getNodeId() const                       = 0;

    /**
     * @return
     *      A null-terminated string for the node name.
     */
    virtual char const*             getNodeName() const                     = 0;

public:     ////                    Operations.

    /**
     *
     */
    virtual MERROR                  uninit()                                = 0;

    /**
     *
     */
    virtual MERROR                  flush()                                 = 0;

    /**
     *
     */
    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &pFrame
                                    )                                       = 0;

    /**
     *
     */
    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    )                                       = 0;
};


/**
 * An interface of callback function from node to pipeline
 */
struct IPipelineNodeCallback : public virtual android::RefBase
{
    virtual MVOID                   onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        Pipeline_NodeId_T nodeId
                                    )                                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODE_H_

