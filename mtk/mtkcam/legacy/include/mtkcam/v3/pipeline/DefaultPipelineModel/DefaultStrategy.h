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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_DEFAULTPIPELINEMODEL_DEFAULTSTRATEGY_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_DEFAULTPIPELINEMODEL_DEFAULTSTRATEGY_H_
//
#include <utils/String8.h>
#include <mtkcam/v3/pipeline/IPipelineNodeMapControl.h>
#include <mtkcam/v3/pipeline/IPipelineBufferSetFrameControl.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Pipeline {
namespace DefaultStrategy {


/******************************************************************************
 *
 ******************************************************************************/
struct  IEvaluator;

typedef Pipeline_NodeId_T                           NodeId_T;

typedef NSCam::v3::Utils::IStreamInfoSetControl     IStreamInfoSetControl;
typedef NSCam::v3::Utils::HalMetaStreamBuffer       HalMetaStreamBuffer;
typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;

typedef HalMetaStreamBuffer::Allocator              HalMetaStreamBufferAllocatorT;

typedef IPipelineFrame::ImageInfoIOMap              ImageInfoIOMap;
typedef IPipelineFrame::MetaInfoIOMap               MetaInfoIOMap;
typedef IPipelineFrame::ImageInfoIOMapSet           ImageInfoIOMapSet;
typedef IPipelineFrame::MetaInfoIOMapSet            MetaInfoIOMapSet;
typedef IPipelineFrame::InfoIOMapSet                InfoIOMapSet;


/**
 *
 */
struct  evaluate_request
{
    struct  Imp;
    struct  Params
    {
    /**
     * @param[in] request number from framework.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    MUINT32                     requestNo;

    /**
     * @param[in] frame number for pipeline.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    MUINT32                     frameNo;

    /**
     * @param[in] A pointer to the callback.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    IPipelineBufferSetFrameControl::IAppCallback*
                                pAppCallback;

    /**
     * @param[in,out] input App image stream buffers, if any.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamBuffer>
                        >*      vIImageBuffers;

    /**
     * @param[in,out] output App image stream buffers.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamBuffer>
                        >*      vOImageBuffers;

    /**
     * @param[in,out] input App meta stream buffers.
     *  The caller must promise the number of buffers and each content.
     *  The callee will update each buffer's users.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        >*      vIMetaBuffers;

    /**
     * @param[out] output App meta stream buffers.
     *  By default, this field is empty. The callee must be in charge of
     *  adding buffers and update each buffer's users.
     *  Note that the number of output meta streams equals to the number of
     *  partial meta result callbacks.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        >*      vOMetaBuffers;

    /**
     * @param[out] A pointer to the frame.
     *  By default, this field is NULL. The callee is responsible for
     *  filling it.
     */
    android::sp<IPipelineFrame>*ppFrame;
    };

    /**
     * Evaluate a request from App.
     *
     * @param[in,out] rParams: the input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    MERROR                      operator()(
                                    Params& rParams,
                                    IEvaluator const* pEvaluator
                                );
};


/**
 *
 */
struct  RequestAppStreamsInfo
{
    /**
     * @param[in] input App image stream info.
     *  The caller must promise its content.
     *  The callee should not modify it.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vIAppImageStreams;

    /**
     * @param[in] output App image stream info.
     *  The caller must promise its content.
     *  The callee should not modify it.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vOAppImageStreams;

    /**
     * @param[in] input App meta stream info.
     *  The caller must promise its content.
     *  The callee should not modify it.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vIAppMetaStreams;

    /**
     * @param[in,out] output App meta stream info.
     *  The caller must promise its initial value.
     *  The callee has an authority to update it based on its initial value.
     *
     * @remark
     *  Note that the number of output meta streams equals to the number of
     *  partial meta result callbacks.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vOAppMetaStreams;
};


/**
 *
 */
struct  evaluate_request_pipeline
{
    typedef IPipelineFrameNodeMapControl    FrameNodeMapT;

    struct  Imp;
    struct  Params
    {
    /**
     * @param[in] input App metadata.
     *  The caller must promise its content.
     *  The callee should not modify it.
     */
    IMetadata const*            pIAppMetaData;

    /**
     * @param[in,out] App stream info.
     *  The caller must promise its initial value.
     *  The callee has an authority to update it based on its initial value.
     */
    RequestAppStreamsInfo*      pAppStreams;

    /**
     * @param[out] a pointer to the resulting stream set.
     *  By default, this field is NULL. The callee is responsible for
     *  filling it.
     */
    android::sp<IStreamInfoSetControl>
                                pStreamInfoSet;

    /**
     * @param[out] a pointer to the resulting pipeline DAG. By default, this
     *  field is NULL. The callee is responsible for filling it.
     */
    android::sp<IPipelineDAG>   pPipelineDAG;

    /**
     * @param[out] a pointer to the resulting pipeline node map. By default,
     *  this field is NULL. The callee is responsible for filling it.
     */
    android::sp<IPipelineNodeMapControl>
                                pPipelineNodeMap;

    /**
     * @param[out] a pointer to the resulting pipeline frame node map.
     *  The caller must ensure that it is pointering to a valid instance.
     *  The callee is responsible for filling its content.
     */
    android::sp<FrameNodeMapT>  pNodeMap;

    };

    /**
     * Evaluate a REQUEST pipeline based on the CONFIG pipeline.
     *
     * @param[in,out] rParams: input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    MERROR                      operator()(
                                    Params& rParams,
                                    IEvaluator const* pEvaluator
                                );
};


/**
 *
 */
struct  evaluate_node_io_streams_params
{
    /**
     * @param[in] input App metadata.
     *  The caller must promise its content.
     *  The callee should not modify it.
     */
    IMetadata const*            pIAppMetaData;

    /**
     * @param[in] Output App image streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vOAppImage;

    /**
     * @param[in] Input App image streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vIAppImage;

    /**
     * @param[in,out] Output App meta streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vOAppMeta;

    /**
     * @param[in,out] Input App meta streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vIAppMeta;

    /**
     * @param[in,out] Output Hal image streams.
     *  For example, P1 may be enforced to output two streams due to the
     *  limitation from drivers or hardwares.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vOHalImage;

    /**
     * @param[in,out] Input Hal image streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >       vIHalImage;

    /**
     * @param[in,out] Output Hal meta streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vOHalMeta;

    /**
     * @param[in,out] Input Hal meta streams.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >       vIHalMeta;

    /**
     * @param[out] a pointer to I/O map set.
     *  The caller must ensure that it is pointering to a valid instance.
     *  The callee is responsible for filling its content.
     */
    InfoIOMapSet*               pIOMapSet;
};


/**
 *
 */
struct  refine_meta_stream_buffer_params
{
    /**
     * @param[in] Request App image stream info set.
     *  The caller must ensure its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >const* pAppImageStreams;

    /**
     * @param[in] Request App meta stream info set.
     *  The caller must ensure its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >const* pAppMetaStreams;

    /**
     * @param[in] Request Hal image stream info set.
     *  The caller must ensure its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamInfo>
                        >const* pHalImageStreams;

    /**
     * @param[in] Request Hal meta stream info set.
     *  The caller must ensure its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamInfo>
                        >const* pHalMetaStreams;

    /**
     * @param[out] App meta streams.
     *  The caller must ensure its instance.
     *  The callee could update its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        >       vAppMetaBuffer;

    /**
     * @param[out] Hal meta streams.
     *  The caller must ensure its instance.
     *  The callee could update its content.
     */
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        >       vHalMetaBuffer;
};


/**
 *
 */
struct  evaluate_request_buffer_users
{
    struct  Imp;
    struct  Params
    {
    /**
     * @param[in] A REQUEST evaluator.
     *  The caller must promise the instance.
     *  The callee can not modify it.
     */
    IEvaluator const*           pEvaluator;

    /**
     * @param[in] A REQUEST pipeline DAG.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    IPipelineDAG const*         pPipelineDAG;

    /**
     * @param[in] A REQUEST pipeline frame node map.
     *  The caller must promise its content.
     *  The callee can not modify it.
     */
    IPipelineFrameNodeMapControl const*
                                pNodeMap;

    /**
     * @param[in,out] A buffer set of this request.
     *  The caller must prepare all instances of buffer's users manager.
     *  The callee is responsible for evaluating each buffer's users.
     */
    IPipelineBufferSetControl*  pBufferSet;
    };

    /**
     * Evaluate each buffer's users.
     *
     * @param[in,out] rParams: the input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    MERROR                      operator()(
                                    Params& rParams
                                );
};


/**
 *
 */
struct IEvaluator
{
public:     ////                Operations.
    virtual                     ~IEvaluator() {}

    virtual IPipelineDAG*       getConfigPipelineDAG() const                = 0;

    virtual IPipelineNodeMapControl*
                                getConfigPipelineNodeMap() const            = 0;

    virtual IPipelineStreamBufferProvider const*
                                getStreamBufferProvider() const             = 0;

    virtual IPipelineNodeCallback*
                                getPipelineNodeCallback() const             = 0;

    /**
     * Refine REQUEST App stream.
     *
     * @param[in,out] rParams: input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              refineRequestAppParams(
                                    evaluate_request_pipeline::Params& rParams
                                )   const                                   = 0;

    /**
     * For a specific pipeline node, given partial I/O request stream sets,
     * evaluate the minimal needed I/O stream sets in order for the node to
     * handle the request.
     *
     * @param[in] nodeId: node Id.
     *
     * @param[in,out] rParams: input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              evaluateNodeIOStreams(
                                    NodeId_T const nodeId,
                                    evaluate_node_io_streams_params& rParams
                                )   const                                   = 0;

    /**
     * Query the minimal usage of a given stream for a specific node.
     *
     * @param[in] nodeId: node id.
     *
     * @param[in] pStreamInfo: stream info.
     *
     * @remark
     *  This function is valid only after configured.
     *
     * @return
     *      the minimal usage if this node can handle this the stream;
     *      otherwise 0.
     */
    virtual MUINT               queryMinimalUsage(
                                    NodeId_T const nodeId,
                                    IImageStreamInfo const*const pStreamInfo
                                )   const                                   = 0;

    /**
     * Refine REQUEST meta stream buffers.
     *
     * @param[in,out] rParams: input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              refineRequestMetaStreamBuffers(
                                    refine_meta_stream_buffer_params& rParams
                                )   const                                   = 0;

public:     ////                Operations.
    virtual MVOID               dump(
                                    android::Vector<android::String8>& rLogs,
                                    InfoIOMapSet const& rIOMapSet
                                )   const                                   = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace DefaultStrategy
};  //namespace Pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_DEFAULTPIPELINEMODEL_DEFAULTSTRATEGY_H_

