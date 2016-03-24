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

#define LOG_TAG "FDNodeTest"
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
#include <mtkcam/v3/hwnode/FDNode.h>
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
    printf("FD Node <test>\n");
}

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    enum STREAM_ID{
        STREAM_ID_YV12 = 1,
        STREAM_ID_INMETADATA,
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
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImage;
    sp<HalMetaStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalMetadata;
    //
    typedef NSCam::v3::Utils::IStreamInfoSetControl       IStreamInfoSetControlT;
    android::sp<IStreamInfoSetControlT> mpStreamInfoSet;
    android::sp<IPipelineNodeMapControl>mpPipelineNodeMap;
    android::sp<IPipelineDAG>           mpPipelineDAG;
    android::sp<FdNode>                 mpNode1;
    static int gFdId = 0;

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
    mpPool_HalImage->uninitPool("Tester");
    mpPool_HalImage = NULL;
    mpPool_HalMetadata->uninitPool("Tester");
    mpPool_HalMetadata = NULL;
    mpAppSimulator = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void prepareConfig(FdNode::ConfigParams &params)
{
    printf("prepareConfig + \n");

    //
    mpStreamInfoSet = IStreamInfoSetControl::create();
    mpPipelineNodeMap = IPipelineNodeMapControl::create();
    mpPipelineDAG = IPipelineDAG::create();

    //

    //android::Vector<android::sp<IImageStreamInfo> > pvHalImageRaw;
    //sp<IMetaStreamInfo>  pHalMetaPlatform = 0;
    sp<IMetaStreamInfo>  pAppMetaResult = 0;
    sp<IMetaStreamInfo>  pAppMetaRequest = 0;
    sp<ImageStreamInfo>  pHalImageFd;


    printf("create FD image buffer\n");
    //Hal:Image:FD
    {
        StreamId_T const streamId = STREAM_ID_YV12;
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
            "Hal:Image:FD",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, imgSize, bufPlanes
        );
        //pHalImageFd.push_back(pStreamInfo);
        pHalImageFd = pStreamInfo;
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

        mpPool_HalImage = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImage->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImage init fail");
        }
    }

    //App:Meta:FD Out
    printf("create FD out app metadata\n");
    {
        sp<IMetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App FD out Metadata",
            STREAM_ID_APPMETADATA,
            eSTREAMTYPE_META_OUT,
            5, 1
        );
        pAppMetaResult = pStreamInfo;
    }


    //App:Meta:FD In
    printf("create FD in app metadata\n");
    {
        sp<MetaStreamInfo>
        pStreamInfo = new MetaStreamInfo(
            "App FD in Request",
            STREAM_ID_INMETADATA,
            eSTREAMTYPE_META_IN,
            5, 1
        );
        pAppMetaRequest = pStreamInfo;
    }

    params.pInAppMeta = pAppMetaRequest;
    params.pOutAppMeta = pAppMetaResult;
    params.vInImage = pHalImageFd;
    //params.pStreamPool_full = mpPool_HalImage;


    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    printf("add stream to streamInfoset\n");

    mpStreamInfoSet->editHalImage().addStream(pHalImageFd);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaResult);
    mpStreamInfoSet->editAppMeta().addStream(pAppMetaRequest);

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
        //      Hal:Image:FD
        //  [output]
        //      App:Meta:Result
        //
        rpOutStreams->editAppMeta().addStream(pAppMetaRequest);
        rpInpStreams->editHalImage().addStream(pHalImageFd);
        //
        rpOutStreams->editAppMeta().addStream(pAppMetaResult);
        //
        //
#if (0)
        FdNode::ConfigParams cfgParams;
        if(mpNode1 != NULL)
        {
            printf("mpNode1 != NULL \n");
            mpNode1->config(cfgParams);
        }
        else
            printf("mpNode1 = NULL \n");
#endif

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

    //FD Node
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
    printf("Test1 \n");
    //
    //
    {
        printf("Test11 \n");
        //YV12
        StreamId_T const streamId = STREAM_ID_YV12;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        printf("Test12 \n");
        sp<HalImageStreamBuffer> pStreamBuffer;

        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        printf("[acquireFromPool] + %s \n", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImage->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        printf("[acquireFromPool] + %s , err:%d %s\n", pStreamInfo->getStreamName(), err, ::strerror(err));
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");

        if(OK!=err)
            printf("OK!=err\n");
        if(pStreamBuffer==0)
            printf("pStreamBuffer==0 \n");
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

        printf("Test3 \n");
        //User Group2
#if (0)
        {
            sp<IUsersManager::IUserGraph> pUserGraph = pStreamBuffer->createGraph();
            IUsersManager::User user;
            //
            user.mUserId = NODE_ID_NODE2;
            user.mCategory = IUsersManager::Category::CONSUMER;
            user.mUsage = pStreamInfo->getUsageForAllocator();
            pUserGraph->addUser(user);
            userGroupIndex = pStreamBuffer->enqueUserGraph(pUserGraph.get());
        }
#endif
        //
        //
        pBufferSetControl->editMap_HalImage()->add(pStreamBuffer);
        printf("Test13 \n");
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
        MERROR err = mpPool_HalMetadata->acquireFromPool(
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
    //
    {
        //APP/Meta/Request
        StreamId_T const streamId = STREAM_ID_INMETADATA;
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
    mpNode1 = FdNode::createInstance();

    //init
    {
         struct  FdNode::InitParams params;
         params.openId = gFdId;
         params.nodeName = "FDTester";
         params.nodeId = NODE_ID_NODE1;
         mpNode1->init(params);
    };

    //----------test 1 ------------//

    //config
    {
       FdNode::ConfigParams params;
       prepareConfig(params);
       mpNode1->config(params);
    }

    //request
    {
        android::sp<IPipelineFrameControl> pFrame;
        prepareRequest(pFrame);
        mpNode1->queue(pFrame);
    }

    sleep(10);

    //flush
    printf("flush\n");
    mpNode1->flush();

    //uninit
    printf("uninit\n");
    mpNode1->uninit();

    MY_LOGD("end of test");
    return 0;
}

