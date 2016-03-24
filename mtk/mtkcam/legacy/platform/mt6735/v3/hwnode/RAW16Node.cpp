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

#define LOG_TAG "MtkCam/Raw16Node"
//
#include <mtkcam/Log.h>
#include <mtkcam/v3/hwnode/RAW16Node.h>
#include <mtkcam/v3/stream/IStreamInfo.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/algorithm/libdngop/MTKDngOp.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

/******************************************************************************
 *
 ******************************************************************************/
#define RAW16THREAD_NAME       ("Cam@Raw16")
#define RAW16THREAD_POLICY     (SCHED_OTHER)
#define RAW16THREAD_PRIORITY   (0)

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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

/******************************************************************************
 *
 ******************************************************************************/
class RAW16NodeImp
    : public BaseNode
    , public RAW16Node
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class UnpackThread
        : public Thread
    {

    public:

                                    UnpackThread(RAW16NodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~UnpackThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

                    RAW16NodeImp*       mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    RAW16NodeImp();

                                    ~RAW16NodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.

    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );

    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           waitForRequestDrained();


    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MBOOL const isInStream
                                    );

    MBOOL                           unpackRaw10(
                                        sp<IImageBuffer>& rpInImageBuffer,
                                        sp<IImageBuffer>& rpOutImageBuffer
                                    );


protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

private:   ////                     Threads
    sp<UnpackThread>                mpUnpackThread;

};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<RAW16Node>
RAW16Node::
createInstance()
{
    MY_LOGD("createInstance");
    return new RAW16NodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
RAW16NodeImp::
RAW16NodeImp()
    : BaseNode()
    , RAW16Node()
    //
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpUnpackThread(NULL)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
RAW16NodeImp::
~RAW16NodeImp()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %d, name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpUnpackThread = new UnpackThread(this);
    if( mpUnpackThread->run(RAW16THREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    MY_LOGD("");
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpUnpackThread->requestExit();
    // join
    mpUnpackThread->join();
    //
    mpUnpackThread = NULL;
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
flush()
{
    FUNC_START;
    //
    // 1. clear requests
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //
        Que_T::iterator it = mRequestQueue.begin();
        while ( it != mRequestQueue.end() ) {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
        }
    }
    //
    // 2. wait enque thread
    waitForRequestDrained();
    //

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);

    //  Make sure the request with a smaller frame number has a higher priority.
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }

    mRequestQueue.insert(it, pFrame);

    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }
    //
    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RAW16NodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;//IOMapSet.mImageInfoIOMapSet.begin()

    MBOOL            bUnpackSuccess;

    if(
            OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
            || IOMapSet.mImageInfoIOMapSet.size() != 1
       ) {
        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %d/%d",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
                );
        return;
    }

    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
    StreamId_T const streamInId = imageIOMap.vIn.keyAt(0);
    StreamId_T const streamOutId = imageIOMap.vOut.keyAt(0);

    //
    {
        IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
        //handle input stream
        sp<IImageStreamBuffer> pInImageStreamBuffer = NULL;
        sp<IImageBuffer>       pInImageBuffer       = NULL;

        MERROR const err = getImageBufferAndLock(
                pFrame,
                streamInId,
                pInImageStreamBuffer,
                pInImageBuffer,
                MTRUE
                );

        if( err != OK ) {
            MY_LOGE("getImageBufferAndLock err = %d", err);
            return;
        }

        //handle output stream
        sp<IImageStreamBuffer> pOutImageStreamBuffer = NULL;
        sp<IImageBuffer>       pOutImageBuffer       = NULL;

        MERROR const err1 = getImageBufferAndLock(
                pFrame,
                streamOutId,
                pOutImageStreamBuffer,
                pOutImageBuffer,
                MFALSE
                );

        if( err1 != OK ) {
            MY_LOGE("getImageBufferAndLock err = %d", err);
            return;
        }
        //
        MINT const bufferInFormat = pInImageBuffer->getImgFormat();
        if( eImgFmt_BAYER10 != bufferInFormat )
            MY_LOGE("Not Support This Format:0x%x", pInImageBuffer->getImgFormat());

        bUnpackSuccess = unpackRaw10(pInImageBuffer, pOutImageBuffer);

        //unlock inputstream
        pInImageBuffer->unlockBuf(getNodeName());
        pInImageStreamBuffer->unlock(getNodeName(), pInImageBuffer->getImageBufferHeap());

        streamBufferSet.markUserStatus(
                pInImageStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );

        //unlock outputstream
        pOutImageBuffer->unlockBuf(getNodeName());
        pOutImageStreamBuffer->unlock(getNodeName(), pOutImageBuffer->getImageBufferHeap());

        pOutImageStreamBuffer->markStatus(
                bUnpackSuccess ?
                STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                );

        streamBufferSet.markUserStatus(
                pOutImageStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
                //
        // release
        streamBufferSet.applyRelease(getNodeId());
    }
    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RAW16NodeImp::
waitForRequestDrained()
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    MBOOL const isInStream
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
        return err;
    //
    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());

    if(isInStream){
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    MY_LOGD("@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD("stream buffer: (%p) %p, heap: %p, buffer: %p, usage: %p",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RAW16NodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (RAW16THREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, RAW16THREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, RAW16THREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = RAW16THREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, RAW16THREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), RAW16THREAD_POLICY, RAW16THREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
RAW16NodeImp::UnpackThread::
requestExit()
{
    //TODO: refine this
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
RAW16NodeImp::UnpackThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RAW16NodeImp::UnpackThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            !exitPending()
        &&  OK == mpNodeImp->onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        mpNodeImp->onProcessFrame(pFrame);

        return true;
    }

    MY_LOGD("exit unpack thread");
    return  false;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RAW16NodeImp::
unpackRaw10(
    sp<IImageBuffer>& rpInImageBuffer,
    sp<IImageBuffer>& rpOutImageBuffer
)
{
    MTKDngOp *MyDngop = NULL;
    DngOpResultInfo MyDngopResultInfo;
    DngOpImageInfo MyDngopImgInfo;
    //upack parameter
    MyDngop = MyDngop->createInstance(DRV_DNGOP_UNPACK_OBJ_SW);

    MyDngopImgInfo.Width = rpInImageBuffer->getImgSize().w;
    MyDngopImgInfo.Height = rpInImageBuffer->getImgSize().h;
    MyDngopImgInfo.Stride_src = rpInImageBuffer->getBufStridesInBytes(0);
    MyDngopImgInfo.srcAddr = reinterpret_cast<void*>(rpInImageBuffer->getBufVA(0));
    MyDngopImgInfo.BIT_NUM = 10;
    MyDngopImgInfo.Bit_Depth = 10;
    MY_LOGD("unpack image:(wxh) = (%zux%zu), srcStride = %zu, srcAddr = %p",
            MyDngopImgInfo.Width, MyDngopImgInfo.Height, MyDngopImgInfo.Stride_src, MyDngopImgInfo.srcAddr);

    MyDngopImgInfo.Stride_dst = rpOutImageBuffer->getBufStridesInBytes(0);
    MyDngopImgInfo.Buff_size = (MyDngopImgInfo.Stride_dst * MyDngopImgInfo.Height);

    MyDngopResultInfo.ResultAddr = reinterpret_cast<void*>(rpOutImageBuffer->getBufVA(0));
    MY_LOGD("dstStride = %zu, buffsize = %zu, ResultAddr = %p",
            MyDngopImgInfo.Stride_dst, MyDngopImgInfo.Buff_size, (MUINT16*)MyDngopResultInfo.ResultAddr);
    //unpack
    MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);
    MY_LOGD("@@fist byte:%x", *(reinterpret_cast<MINT8*>(rpOutImageBuffer->getBufVA(0))));

    //destroy(unpack end)
    MyDngop->destroyInstance(MyDngop);

    MY_LOGE_IF(S_DNGOP_OK != MyDngopResultInfo.RetCode,
            "ErrPattern: %d, RetCode: %d",
            MyDngopResultInfo.ErrPattern,
            MyDngopResultInfo.RetCode
            );
    return (S_DNGOP_OK == MyDngopResultInfo.RetCode) ? MTRUE : MFALSE;
}

