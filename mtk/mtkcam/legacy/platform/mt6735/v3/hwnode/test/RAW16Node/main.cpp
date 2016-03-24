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
#define LOG_TAG "RAW16NodeTest"
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
#include <mtkcam/v3/hwnode/RAW16Node.h>
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
    printf("RAW16Node <test>\n");

}

/******************************************************************************
 *
 ******************************************************************************/

namespace {

    enum STREAM_ID{
        STREAM_ID_IN = 1,
        STREAM_ID_OUT
    };

    enum NODE_ID{
        NODE_ID_NODE1 = 1,
        NODE_ID_NODE2,
    };

    class AppSimulator
        : public virtual RefBase
    {
    };

    //
    android::sp<AppSimulator>           mpAppSimulator;
    //
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageRAW10;
    sp<HalImageStreamBuffer::Allocator::StreamBufferPoolT> mpPool_HalImageRAW16;
    //
    IHalSensor* mpSensorHalObj;
    //
    typedef NSCam::v3::Utils::IStreamInfoSetControl       IStreamInfoSetControlT;
    android::sp<IStreamInfoSetControlT> mpStreamInfoSet;
    android::sp<IPipelineNodeMapControl>mpPipelineNodeMap;
    android::sp<IPipelineDAG>           mpPipelineDAG;
    android::sp<RAW16Node>              mpNode1;
    //remember output stream buffer for dump file
    android::sp<HalImageStreamBuffer>       mpHalImageStreamBuffer;
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
    //mpPipelineNodeMap = NULL;
    //mpStreamInfoSet = NULL;
    //mpPipelineDAG = NULL;
    //mpPool_HalImageRAW10->uninitPool("Tester");
    //mpPool_HalImageRAW10 = NULL;
    //mpPool_HalImageRAW16->uninitPool("Tester");
    //mpPool_HalImageRAW16 = NULL;

    //mpAppSimulator = NULL;
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
void
saveToFile(const char *filename, StreamId_T const streamid)
{
    StreamId_T const streamId = streamid;
    sp<HalImageStreamBuffer> pStreamBuffer;
    //
    sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
    //
    //acquireFromPool
    MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
    MERROR err = mpPool_HalImageRAW16->acquireFromPool(
        "Tester", pStreamBuffer, ::s2ns(30)
    );
    MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
    MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");

    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    IImageBuffer*          pImageBuffer = NULL;
    if (pStreamBuffer == NULL) {
        MY_LOGE("pStreamBuffer == NULL");
    }
    pImageBufferHeap = pStreamBuffer->tryReadLock(LOG_TAG);

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
    }

    pImageBuffer = pImageBufferHeap->createImageBuffer();

    if (pImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
    }
    pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
    MY_LOGD("@@@fist byte:%x", *(reinterpret_cast<MINT8*>(pImageBuffer->getBufVA(0))));

    pImageBuffer->saveToFile(filename);



    pImageBuffer->unlockBuf(LOG_TAG);

    pStreamBuffer->unlock(LOG_TAG, pImageBufferHeap.get());


}

/******************************************************************************
 *
 ******************************************************************************/
void prepareConfig()
{
    printf("prepareConfig + \n");
    //
    //params.type = P2Node::PASS2_STREAM;
    //
    mpStreamInfoSet = IStreamInfoSetControl::create();
    mpPipelineNodeMap = IPipelineNodeMapControl::create();
    mpPipelineDAG = IPipelineDAG::create();

    //

    //android::Vector<android::sp<IImageStreamInfo> > pvHalImageRaw;
    //sp<IMetaStreamInfo>  pHalMetaPlatform = 0;
    sp<ImageStreamInfo>  pHalImageIn = 0;
    sp<ImageStreamInfo>  pHalImageOut = 0;


    printf("create raw10 image buffer\n");
    //Image src: raw10
    {
        StreamId_T const streamId = STREAM_ID_IN;
        MSize const imgSize(4176, 3088);//
        MINT const format = eImgFmt_BAYER10;
        MUINT const usage = eBUFFER_USAGE_SW_MASK;
        IImageStreamInfo::BufPlanes_t bufPlanes;
        IImageStreamInfo::BufPlane bufPlane;
        //
        bufPlane.rowStrideInBytes = imgSize.w * 10 / 8;
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * imgSize.h;
        bufPlanes.push_back(bufPlane);
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image RAW10",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            1, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageIn = pStreamInfo;
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
        MY_LOGD("format=%#x, imgSize=%dx%d, stride=%d\n",format, imgSize.w, imgSize.h, bufStridesInBytes[0]);
        mpPool_HalImageRAW10 = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImageRAW10->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_ImageRAW10 init fail");
        }
    }
    //Hal:Image: RAW16
    {
        StreamId_T const streamId = STREAM_ID_OUT;
        MSize const imgSize(4176, 3088);
        MINT const format = eImgFmt_RAW16;
        MUINT const usage = eBUFFER_USAGE_SW_MASK;
        IImageStreamInfo::BufPlanes_t bufPlanes;
        //[++]
        IImageStreamInfo::BufPlane bufPlane;
        bufPlane.rowStrideInBytes = imgSize.w * 2;
        bufPlane.sizeInBytes = bufPlane.rowStrideInBytes * imgSize.h;

        bufPlanes.push_back(bufPlane);
        //
        sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
            "Hal:Image: RAW16",
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            1, 1,
            usage, format, imgSize, bufPlanes
        );
        pHalImageOut = pStreamInfo;
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
        //MY_LOGE("Jpeg format=%#x, imgSize=%dx%d\n",format, imgSize.w, imgSize.h);
        mpPool_HalImageRAW16 = new HalImageStreamBuffer::Allocator::StreamBufferPoolT(
            pStreamInfo->getStreamName(),
            HalImageStreamBuffer::Allocator(pStreamInfo.get(), allocImgParam)
        );
        MERROR err = mpPool_HalImageRAW16->initPool("Tester", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        if  ( err ) {
            MY_LOGE("mpPool_HalImageRAW16 init fail");
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////////
    printf("add stream to streamInfoset\n");

    mpStreamInfoSet->editHalImage().addStream(pHalImageIn);
    mpStreamInfoSet->editHalImage().addStream(pHalImageOut);


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
        //      Hal:Image:RAW10
        //  [output]
        //      Hal:Image:RAW16
        rpInpStreams->editHalImage().addStream(pHalImageIn);
        //
        rpOutStreams->editHalImage().addStream(pHalImageOut);
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
        printf("setNodeValue%d\n", i);
    }

    mpAppSimulator = new AppSimulator;

    printf("prepareConfig - \n");
}


/******************************************************************************
 *
 ******************************************************************************/
void
prepareRequest(android::sp<IPipelineFrameControl> &pFrame, const char *filename)
{
    printf("prepare request\n");

    pFrame = IPipelineFrameControl::create(0);
    pFrame->setPipelineNodeMap(getPipelineNodeMapControl());
    pFrame->setPipelineDAG(getPipelineDAG());
    pFrame->setStreamInfoSet(getStreamInfoSet());

    //RAW16 Node
    {
        IPipelineNode::NodeId_T const nodeId = NODE_ID_NODE1;
        //
        IPipelineFrame::InfoIOMapSet aInfoIOMapSet;
        IPipelineFrame::ImageInfoIOMapSet& rImageInfoIOMapSet = aInfoIOMapSet.mImageInfoIOMapSet;
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
        StreamId_T const streamId = STREAM_ID_IN;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImageRAW10->acquireFromPool(
            "Tester", pStreamBuffer, ::s2ns(30)
        );
        MY_LOGD("[acquireFromPool] - %s %p err:%d", pStreamInfo->getStreamName(), pStreamBuffer.get(), err);
        MY_LOGE_IF(OK!=err || pStreamBuffer==0, "pStreamBuffer==0");
        //write raw10 to src buffer
        {

            sp<IImageBufferHeap>   pImageBufferHeap = NULL;
            IImageBuffer*          pImageBuffer = NULL;
            pImageBufferHeap = pStreamBuffer->tryWriteLock(LOG_TAG);

            if (pImageBufferHeap == NULL) {
                MY_LOGE("pImageBufferHeap == NULL");
            }
            pImageBuffer = pImageBufferHeap->createImageBuffer();
            if (pImageBuffer == NULL) {
                MY_LOGE("pImageBuffer == NULL");
            }
            //pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
            printf("load image:%s\n", filename);
            pImageBuffer->loadFromFile(filename);
            printf("@@BufSize = %zu\n",
                    pImageBuffer->getBufSizeInBytes(0));

            //MY_LOGD("@@%x", *(reinterpret_cast<MINT8*>(pImageBuffer->getBufVA(0))));
            //pImageBuffer->saveToFile("/data/raw16_result.raw");
            //printf("@@BufSize = %zu",
            //        pImageBuffer->getBufSizeInBytes(0));
            pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
            //pImageBuffer->getBufVA(0);
            //pImageBuffer->saveToFile("/data/raw16_result.raw");
            MY_LOGD("@@fist byte:%x", *(reinterpret_cast<MINT8*>(pImageBuffer->getBufVA(0))));
            pImageBuffer->unlockBuf(LOG_TAG);
            pStreamBuffer->unlock(LOG_TAG, pImageBufferHeap.get());
            //saveToFile("/data/raw16_result.raw", STREAM_ID_IN);
        }

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

    {
        //
        StreamId_T const streamId = STREAM_ID_OUT;
        //
        sp<IImageStreamInfo> pStreamInfo = getStreamInfoSet()->getImageInfoFor(streamId);
        sp<HalImageStreamBuffer> pStreamBuffer;
        //
        //acquireFromPool
        MY_LOGD("[acquireFromPool] + %s ", pStreamInfo->getStreamName());
        MERROR err = mpPool_HalImageRAW16->acquireFromPool(
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
        pBufferSetControl->editMap_HalImage()->add(pStreamBuffer);
        //mpHalImageStreamBuffer = pStreamBuffer;

    }
    //
    pFrame->setStreamBufferSet(pBufferSetControl);
}


/******************************************************************************
 *
 ******************************************************************************/
int main(/*int argc, char** argv*/)
{
    printf("%s\n","start test");

    //printf("Usage: test_raw16node <filename> <out_filename>\n");
    //image size 4176x3088
    //filename = argv[1];
    //out_filename = argv[2];
    const char *filename = "/sdcard/DCIM/Camera/testRaw123.raw";
    const char *out_filename = "/sdcard/result16.raw";

    printf("src=%s dst=%s\n", filename, out_filename);
    mpNode1 = RAW16Node::createInstance();

    //
    MUINT32 frameNo = 0;
    //

    //init
    {
         struct  RAW16Node::InitParams params;
         params.openId = gSensorId;
         params.nodeName = "Raw16Tester";
         params.nodeId = NODE_ID_NODE1;
         mpNode1->init(params);
    };

    //----------test 1 ------------//

    //config
    {
        RAW16Node::ConfigParams params;
        prepareConfig();
        mpNode1->config(params);
    }
    //request
    {
        android::sp<IPipelineFrameControl> pFrame;
        prepareRequest(pFrame, filename);
        mpNode1->queue(pFrame);
    }

    printf("save to file\n");
    saveToFile(out_filename, STREAM_ID_OUT);
    usleep(100000);
    //save to  file

    //flush
    printf("flush\n");
    mpNode1->flush();

    //uninit
    printf("uninit\n");
    mpNode1->uninit();

    mpPool_HalImageRAW16 = NULL;
    printf("end of test\n");
    MY_LOGD("end of test");
    return 0;
}

