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
#define LOG_TAG "P2NodeTest"
//
#include <mtkcam/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/v3/pipeline/IPipelineDAG.h>
#include <mtkcam/v3/pipeline/IPipelineNode.h>
#include <mtkcam/v3/pipeline/IPipelineNodeMapControl.h>
#include <mtkcam/v3/pipeline/IPipelineFrameControl.h>
//
#include <mtkcam/v3/utils/streambuf/StreamBufferPool.h>
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include <mtkcam/v3/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/v3/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/v3/hwnode/P2Node.h>
//
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");
/******************************************************************************
 *
 ******************************************************************************/
void help()
{
    printf("P2 Node <test>\n");
}

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    enum STREAM_ID{
        STREAM_ID_IN_BAYER = 1,
        STREAM_ID_OUT_YUV_0,
        STREAM_ID_OUT_YUV_1,
        STREAM_ID_APPREQUEST,
        STREAM_ID_HALREQUEST,
        STREAM_ID_APPMETADATA
    };

    enum NODE_ID{
        NODE_ID_NODE1 = 1,
        NODE_ID_NODE2,
    };

#if (0)
    class FakeNode : public IPipelineNode
    {
        public:     ////                    Attributes.

                                            FakeNode()
                                                : IPipelineNode()
                                                {}

            /**
             * @return
             *      An index to indicate which camera device to open.
             */
            virtual MINT32                  getOpenId() const { return 0; }

            /**
             * @return
             *      A unique node id.
             */
            virtual NodeId_T                getNodeId() const { return NODE_ID_NODE2; }

            /**
             * @return
             *      A null-terminated string for the node name.
             */
            virtual char const*             getNodeName() const { return "fake"; }

            /**
             * Query the minimal usage of a given input stream.
             *
             * @param[in] streamId: input stream id.
             *
             * @return
             *      the minimal usage if the stream is configured; otherwise 0.
             */
            virtual MUINT                   getInputStreamMinimalUsage(
                                                StreamId_T streamId
                                            ) const  { return 0; }

            /**
             * Query the minimal usage of a given output stream.
             *
             * @param[in] streamId: output stream id.
             *
             * @return
             *      the minimal usage if the stream is configured; otherwise 0.
             */
            virtual MUINT                   getOutputStreamMinimalUsage(
                                                StreamId_T streamId
                                            ) const    { return 0; }

        public:     ////                    Operations.
            /**
             * Given input and output request stream sets, determine additional minimal
             * needed stream sets in order to handle the request.
             *
             * @param[in] vInRequest: a vector of input request streams.
             *
             * @param[in] vOutRequest: a vector of output request streams.
             *
             * @param[out] rvInResult: a vector of additional input streams.
             *
             * @param[out] rvOutResult: a vector of additional output streams.
             *
             * @return
             *      0 indicates success; otherwise failure.
             */
            virtual MERROR                  evaluateRequestStreams(
                                                android::SortedVector<StreamId_T>const& vInRequest,
                                                android::SortedVector<StreamId_T>const& vOutRequest,
                                                android::SortedVector<StreamId_T>& rvInResult,
                                                android::SortedVector<StreamId_T>& rvOutResult
                                            ) const     { return INVALID_OPERATION; }

        public:     ////                    Operations.

            /**
             *
             */
            virtual MERROR                  init() { return OK; }

            /**
             *
             */
            virtual MERROR                  uninit() { return OK; }

            /**
             *
             */
            virtual MERROR                  flush() { return OK; }

            /**
             *
             */
            virtual MERROR                  queue(
                                                android::sp<IPipelineFrame> pFrame
                                            )
                                            {
                                                MY_LOGD("[FakeNode] FrameNo: %u", pFrame->getFrameNo());
                                                return onHandleflush(pFrame);
                                            }


        protected:

            virtual MERROR                  onHandleflush(android::sp<IPipelineFrame>& pFrame) {
                                                IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
                                                //  Meta Stream Buffers
                                                //  Mark this buffer as RELEASE by this user.
                                                rStreamBufferSet.markUserStatus(STREAM_ID_HALMETADATA1, NODE_ID_NODE2, IUsersManager::UserStatus::RELEASE);

                                                //  Image Stream Buffers
                                                //  Mark this buffer as RELEASE by this user.
                                                rStreamBufferSet.markUserStatus(STREAM_ID_RAW1, NODE_ID_NODE2, IUsersManager::UserStatus::RELEASE);
                                                //
                                                rStreamBufferSet.applyRelease(NODE_ID_NODE2);

                                                return OK;
                                            }
    };
#endif



    class AppSimulator
        : public virtual RefBase
    {
    };

    //
    android::sp<AppSimulator>           mpAppSimulator;
    //
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageRaw;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageYuv0;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageYuv1;
    sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalMetadataRequest;
    //sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_AppMetadataRequest;
    //sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_AppMetadataResult;
    //
    IHalSensor* mpSensorHalObj;
    //
    typedef NSCam::v3::Utils::IStreamInfoSetControl       IStreamInfoSetControlT;
    android::sp<IStreamInfoSetControlT> mpStreamInfoSet;
    android::sp<IPipelineNodeMapControl>mpPipelineNodeMap;
    android::sp<IPipelineDAG>           mpPipelineDAG;
    android::sp<P2Node>                 mpNode1;
    //
    static int gSensorId = 0;

}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineNodeMapControl>
getPipelineNodeMapControl()
{
    return mpPipelineNodeMap;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IStreamInfoSet>
getStreamInfoSet()
{
    return mpStreamInfoSet;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineNodeMap>
getPipelineNodeMap()
{
    return mpPipelineNodeMap;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineDAG>
getPipelineDAG()
{
    return mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
void clear_global_var()
{
    mpPipelineNodeMap = NULL;
    mpStreamInfoSet = NULL;
    mpPipelineDAG = NULL;
    mpPool_HalImageRaw->uninitPool("Tester");
    mpPool_HalImageRaw = NULL;
    mpPool_HalImageYuv0->uninitPool("Tester");
    mpPool_HalImageYuv0 = NULL;
    mpPool_HalImageYuv1->uninitPool("Tester");
    mpPool_HalImageYuv1 = NULL;
    mpPool_HalMetadataRequest->uninitPool("Tester");
    mpPool_HalMetadataRequest = NULL;
    mpAppSimulator = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void prepareSensor()
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();
    mpSensorHalObj = pHalSensorList->createSensor("tester", gSensorId);
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);
}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor()
{
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
    mpSensorHalObj->destroyInstance("tester");
    mpSensorHalObj = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void prepareConfig(P2Node::ConfigParams &params)
{
    printf("prepareConfig + \n");
    //
    //
    mpStreamInfoSet = IStreamInfoSetControl::create();
    mpPipelineNodeMap = IPipelineNodeMapControl::create();
    mpPipelineDAG = IPipelineDAG::create();

    //

    //android::Vector<android::sp<IImageStreamInfo> > pvHalImageRaw;
    //sp<IMetaStreamInfo>  pHalMetaPlatform = 0;
    sp<IMetaStreamInfo>  pAppMetaResult   = 0;
    sp<IMetaStreamInfo>  pAppMetaRequest  = 0;
    sp<IMetaStreamInfo>  pHalMetaRequest  = 0;
    sp<ImageStreamInfo>  pHalImageInRaw   = 0;
    sp<ImageStreamInfo>  pHalImageOutYuv0 = 0;
    sp<ImageStreamInfo>  pHalImageOutYuv1 = 0;


    printf("create P2 image buffer\n");
    //Hal:Image: src
    {
        StreamId_T const streamId = STREAM_ID_IN_BAYER;
        MSize const imgSize(640, 480);
        MINT const format = eImgFmt_BAYER10;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
        IImageStreamInfo::BufPlane bufPlane;
        //
        bufPlane.rowStrideInBytes = imgSize.w * 10 / 8;
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * imgSize.h;
        bufPlanes.push_back(bufPlane);
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:P2",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageInRaw = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImageRaw = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImageRaw->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImageRaw init fail");
        }
    }
    //Hal:Image: dst0
    {
        StreamId_T const streamId = STREAM_ID_OUT_YUV_0;
        MSize const imgSize(640, 480);
        MINT const format = eImgFmt_YV12;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:YUV0",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutYuv0 = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImageYuv0 = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImageYuv0->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImageYuv0 init fail");
        }
    }

    //Hal:Image: dst1
    {
        StreamId_T const streamId = STREAM_ID_OUT_YUV_1;
        MSize const imgSize(640, 480);
        MINT const format = eImgFmt_YV12;
        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                            eBUFFER_USAGE_HW_CAMERA_READWRITE
                            ;
        IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                         \
        do{                                                                         \
            IImageStreamInfo::BufPlane bufPlane= { (height) * (stride), (stride) }; \
            planes.push_back(bufPlane);                                             \
        }while(0)
        if( format == eImgFmt_YV12 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w/2);
        }
        else if( format == eImgFmt_NV21 ) {
            addBufPlane(bufPlanes, (unsigned int)imgSize.h, (unsigned int)imgSize.w);
            addBufPlane(bufPlanes, (unsigned int)imgSize.h/2, (unsigned int)imgSize.w);
        }
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image:YUV1",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOutYuv1 = pStreamInfo;
        //
        //
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }
        IIonImageBufferHeap::AllocImgParam_t const allocImgParam(
            format, imgSize,
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
        );

        mpPool_HalImageYuv1 = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImageYuv1->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImageYuv1 init fail");
        }
    }
    //App:Meta:P2 Out
    printf("create P2 out app metadata\n");
    {
        sp<IMetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App P2 out Metadata",
            STREAM_ID_APPMETADATA,
            eSTREAMTYPE_META_OUT,
            5, 1
        );
        pAppMetaResult = pStreamInfo;
    }


    //App:Meta:P2 In
    printf("create P2 in app metadata\n");
    {
        sp<MetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App P2 in Request",
            STREAM_ID_APPREQUEST,
            eSTREAMTYPE_META_IN,
            5, 1
        );
        pAppMetaRequest = pStreamInfo;
    }

    //Hal:Meta:P2 In
    printf("create P2 in app metadata\n");
    {
        sp<MetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "Hal P2 in Request",
            STREAM_ID_HALREQUEST,
            eSTREAMTYPE_META_IN,
            5, 1
        );
        pHalMetaRequest = pStreamInfo;
    }

    params.pInAppMeta  = pAppMetaRequest;
    params.pInHalMeta  = pHalMetaRequest;
    params.pOutAppMeta = pAppMetaResult;
    params.pvInFullRaw.push_back(pHalImageInRaw);
    params.pInResizedRaw = NULL;
    params.vOutImage.push_back(pHalImageOutYuv0);
    params.vOutImage.push_back(pHalImageOutYuv1);


    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    printf("add stream to streamInfoset\n");

    mpStreamInfoSet->editHalImage().addStream(pHalImageInRaw);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutYuv0);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOutYuv1);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaResult);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaRequest);
    mpStreamInfoSet->editHalMeta().addStream(pHalMetaRequest);

    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    //
    //
    printf("add stream to pipelineNodeMap\n");
    mpPipelineNodeMap->setCapacity(1);
    //
    {
        mpPipelineDAG->addNode(NODE_ID_NODE1);
        ssize_t const tmpNodeIndex = mpPipelineNodeMap->add(NODE_ID_NODE1, mpNode1);
        //
        sp<IStreamInfoSetControl> const&
        rpInpStreams = mpPipelineNodeMap->getNodeAt(tmpNodeIndex)->editInStreams();
        sp<IStreamInfoSetControl> const&
        rpOutStreams = mpPipelineNodeMap->getNodeAt(tmpNodeIndex)->editOutStreams();
        //
        //  [input]
        //      App:Meta:Request
        //      Hal:Image:P2
        //  [output]
        //      App:Meta:Result
        //
        rpInpStreams->editAppMeta().addStream(pAppMetaRequest);
        rpInpStreams->editHalMeta().addStream(pHalMetaRequest);
        rpInpStreams->editHalImage().addStream(pHalImageInRaw);
        //
        rpOutStreams->editAppMeta().addStream(pAppMetaResult);
        rpOutStreams->editHalImage().addStream(pHalImageOutYuv0);
        rpOutStreams->editHalImage().addStream(pHalImageOutYuv1);
        //
        // TODO: re-config flow
        //P2Node::ConfigParams cfgParams;
        //mpNode1->config(cfgParams);
    }
    //
#if (0)
    {
        //
        mpPipelineDAG->addNode(NODE_ID_NODE2);
        ssize_t const tmpNodeIndex = mpPipelineNodeMap->add(NODE_ID_NODE2, mpNode2);
    }
    //
    mpPipelineDAG->addEdge(mpNode1->getNodeId(), mpNode2->getNodeId());
#endif
    mpPipelineDAG->setRootNode(mpNode1->getNodeId());
    for (size_t i = 0; i < mpPipelineNodeMap->size(); i++)
    {
        mpPipelineDAG->setNodeValue(mpPipelineNodeMap->nodeAt(i)->getNodeId(), i);
    }

    mpAppSimulator = new AppSimulator;

    printf("prepareConfig - \n");
}


/******************************************************************************
 *
 ******************************************************************************/
void
prepareRequest(android::sp<IPipelineFrameControl> &pFrame)
{
    printf("prepare request\n");

    pFrame = IPipelineFrameControl::create(0);
    pFrame->setPipelineNodeMap(getPipelineNodeMapControl());
    pFrame->setPipelineDAG(getPipelineDAG());
    pFrame->setStreamInfoSet(getStreamInfoSet());

    //P2 Node
    {
        IPipelineNode::NodeId_T const nodeId = NODE_ID_NODE1;
        //
        IPipelineFrame::InfoIOMapSet aInfoIOMapSet;
        IPipelineFrame::ImageInfoIOMapSet& rImageInfoIOMapSet = aInfoIOMapSet.mImageInfoIOMapSet;
        IPipelineFrame::MetaInfoIOMapSet&  rMetaInfoIOMapSet  = aInfoIOMapSet.mMetaInfoIOMapSet;
        //
        //
        sp<IPipelineNodeMapControl::INode> pNodeExt = getPipelineNodeMapControl()->getNodeFor(nodeId);
        sp<IStreamInfoSet const> pInStream = pNodeExt->getInStreams();
        sp<IStreamInfoSet const> pOutStream= pNodeExt->getOutStreams();
        //
        //  Image
        {
            IPipelineFrame::ImageInfoIOMap& rMap =
            rImageInfoIOMapSet.editItemAt(rImageInfoIOMapSet.add());
            //
            //Input
            for (size_t i = 0; i < pInStream->getImageInfoNum(); i++)
            {
                sp<IImageStreamInfo> p = pInStream->getImageInfoAt(i);
                rMap.vIn.add(p->getStreamId(), p);
            }
            //
            //Output
            for (size_t i = 0; i < pOutStream->getImageInfoNum(); i++)
            {
                sp<IImageStreamInfo> p = pOutStream->getImageInfoAt(i);
                rMap.vOut.add(p->getStreamId(), p);
            }
        }
        //
        //  Meta
        {
            IPipelineFrame::MetaInfoIOMap& rMap =
            rMetaInfoIOMapSet.editItemAt(rMetaInfoIOMapSet.add());
            //
            //Input
            for (size_t i = 0; i < pInStream->getMetaInfoNum(); i++)
            {
                sp<IMetaStreamInfo> p = pInStream->getMetaInfoAt(i);
                rMap.vIn.add(p->getStreamId(), p);
            }
            //
            //Output
            for (size_t i = 0; i < pOutStream->getMetaInfoNum(); i++)
            {
                sp<IMetaStreamInfo> p = pOutStream->getMetaInfoAt(i);
                rMap.vOut.add(p->getStreamId(), p);
            }
        }
        //
        //
        pFrame->addInfoIOMapSet(nodeId, aInfoIOMapSet);
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    //  pFrame->setStreamBufferSet(...);
    //
    ////////////////////////////////////////////////////////////////////////////

    //IAppPipeline::AppCallbackParams aAppCallbackParams;
    //aAppCallbackParams.mpBuffersCallback = pAppSimulator;
    sp<IStreamBufferSetControl> pBufferSetControl = IStreamBufferSetControl::create(
        0, NULL
    );
    //
    //
    {
        //
        StreamId_T const streamId = STREAM_ID_IN_BAYER;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImageRaw->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    {
        //
        StreamId_T const streamId = STREAM_ID_OUT_YUV_0;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImageYuv0->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_HalImage()->add(pStreamBuffer);
    }
    //
    {
        //
        StreamId_T const streamId = STREAM_ID_OUT_YUV_1;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImageYuv1->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_HalImage()->add(pStreamBuffer);
    }
    //
#if (0)
    {
        //HAL/Meta/PLATFORM
        StreamId_T const streamId = STREAM_ID_HALMETADATA1;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<HalMetaStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalMetadataRequest->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            user.mUsage = 0;
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //User Group2
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE2;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = 0;
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_HalMeta()->add(pStreamBuffer);
    }
#endif
    //
    {
        //App:Meta:Result
        StreamId_T const streamId = STREAM_ID_APPMETADATA;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        //alloc without default value
        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())();
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            pUserGraph->addUser(user);
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    {
        //APP/Meta/Request
        StreamId_T const streamId = STREAM_ID_APPREQUEST;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        IMetadata appRequest;
        {
            IMetadata::IEntry entry1(MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
            entry1.push_back(16, Type2Type< MFLOAT >());
            appRequest.update(MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, entry1);
        }

        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())(appRequest);
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::CONSUMER;
            pUserGraph->addUser(user);
            //
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    {
        //HAL/Meta/Request
        StreamId_T const streamId = STREAM_ID_HALREQUEST;
        //
        sp<IMetaStreamInfo> pStreamInfo = getStreamInfoSet()->getMetaInfoFor(streamId);
        sp<IMetaStreamBuffer> pStreamBuffer;
        //
        typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator StreamBufferAllocatorT;
        pStreamBuffer = StreamBufferAllocatorT(pStreamInfo.get())();
        MY_LOGE_IF(pStreamBuffer==0, "pStreamBuffer==0");
        //
        ssize_t userGroupIndex = 0;
        //User Group1
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE1;
            user.mCategory = IUsersManager::Category::PRODUCER;
            pUserGraph->addUser(user);
            //
            //
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
        //
        pBufferSetControl->editMap_AppMeta()->add(pStreamBuffer);
    }
    //
    pFrame->setStreamBufferSet(pBufferSetControl);
}


/******************************************************************************
 *
 ******************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    MY_LOGD("start test");


    MUINT32 frameNo = 0;
    //
    //sensor
    prepareSensor();
    //
    mpNode1 = P2Node::createInstance(P2Node::PASS2_STREAM);

    //init
    {
         struct  P2Node::InitParams params;
         params.openId = gSensorId;
         params.nodeName = "P2Tester";
         params.nodeId = NODE_ID_NODE1;
         mpNode1->init(params);
    };

    //----------test 1 ------------//

    //config
    {
       P2Node::ConfigParams params;
       prepareConfig(params);
       mpNode1->config(params);
    }

    //request
    {
        android::sp<IPipelineFrameControl> pFrame;
        prepareRequest(pFrame);
        mpNode1->queue(pFrame);
    }

    usleep(10000);
    //flush
    printf("flush\n");
    mpNode1->flush();

    //uninit
    printf("uninit\n");
    mpNode1->uninit();

    closeSensor();

    MY_LOGD("end of test");
    return 0;
}

