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

#define LOG_TAG "MtkCam/Pipeline"
//
#include "../MyUtils.h"
#include <mtkcam/v3/pipeline/DefaultPipelineModel/DefaultPipelineModel.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


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
DefaultPipelineModel::
DefaultPipelineModel(MINT32 const openId, android::String8 const& name)
    : RefBase()
    , mRWLock()
    , mOpenId(openId)
    , mName(name)
    //
    , mConfigContext()
    //
    , mHalImageStreamBufferPoolMap()
    //
    , mflushLock()
    , mInFlush(MFALSE)
{
    mConfigContext.mpStreamInfoSet  = IStreamInfoSetControl::create();
    mConfigContext.mpPipelineDAG    = IPipelineDAG::create();
    mConfigContext.mpPipelineNodeMap= IPipelineNodeMapControl::create();
    //
    {
        char cLogLevel[PROPERTY_VALUE_MAX] = {0};
        ::property_get("debug.camera.log", cLogLevel, "0");
        mLogLevel = ::strtol(cLogLevel, NULL, 0);
        if ( mLogLevel == 0 ) {
            ::property_get("debug.camera.log.HwPipeline", cLogLevel, "0");
            mLogLevel = ::strtol(cLogLevel, NULL, 0);
        }
    }
    {
        char value[PROPERTY_VALUE_MAX] = {0};
        ::property_get("debug.camera.force_fd", value, "0");
        mDebugFdMode = ::strtol(value, NULL, 0);
        mFdEnable = 0;
    }
    MY_LOGD("LogLevel:%d DebugFdMode:%d", mLogLevel, mDebugFdMode);
}


/******************************************************************************
 *
 ******************************************************************************/
void
DefaultPipelineModel::
onLastStrongRef(const void* /*id*/)
{
    RWLock::AutoWLock _l(mRWLock);
    onDestroyLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultPipelineModel::
onDestroyLocked()
{
    MY_LOGD("openId:%d", mOpenId);

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
        pNode = NULL;
    }

    //
    for (size_t i = 0; i < mHalImageStreamBufferPoolMap.size(); i++)
    {
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
DefaultPipelineModel::Context const&
DefaultPipelineModel::
getConfigContext() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mConfigContext;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineNodeMapControl*
DefaultPipelineModel::
getConfigPipelineNodeMap() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mConfigContext.mpPipelineNodeMap.get();
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG*
DefaultPipelineModel::
getConfigPipelineDAG() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mConfigContext.mpPipelineDAG.get();
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineStreamBufferProvider const*
DefaultPipelineModel::
getStreamBufferProvider() const
{
    return this;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineNodeCallback*
DefaultPipelineModel::
getPipelineNodeCallback() const
{
    IPipelineNodeCallback const* pCb = this;
    return const_cast<IPipelineNodeCallback*>(pCb);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultPipelineModel::
acquireHalStreamBuffer(
    android::sp<IImageStreamInfo> const pStreamInfo,
    android::sp<HalImageStreamBuffer>& rpStreamBuffer
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    StreamId_T const streamId = pStreamInfo->getStreamId();
    sp<HalImageStreamBufferPoolT> pPool = mHalImageStreamBufferPoolMap.valueFor(streamId);
    MY_LOGE_IF(pPool == 0, "NULL HalImageStreamBufferPool - stream:%#"PRIxPTR"(%s)", streamId, pStreamInfo->getStreamName());
    MERROR err = pPool == 0 ? UNKNOWN_ERROR : pPool->acquireFromPool(__FUNCTION__, rpStreamBuffer, ::s2ns(10));
    MY_LOGA_IF(
        OK!=err || rpStreamBuffer==0,
        "[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#"PRIxPTR"(%s)",
        err, ::strerror(-err), rpStreamBuffer.get(), streamId, pStreamInfo->getStreamName()
    );
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<DefaultPipelineModel::HalImageStreamBufferPoolT>
DefaultPipelineModel::
addHalStreamBufferPoolLocked(
    android::sp<IImageStreamInfo> pStreamInfo
)
{
    if  ( pStreamInfo == 0 ) {
        MY_LOGE("NULL IImageStreamInfo");
        return NULL;
    }
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
    if  ( pPool == 0 ) {
        MY_LOGE("Fail to new a image pool:%s", name.string());
        return NULL;
    }
    //
    MERROR err = pPool->initPool(getName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if  ( OK != err ) {
        MY_LOGE("%s: initPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return NULL;
    }
    if  ( OK != pPool->commitPool(getName()) ) {
        MY_LOGE("%s: commitPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return NULL;
    }

    //
    mHalImageStreamBufferPoolMap.add(pStreamInfo->getStreamId(), pPool);
    return pPool;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<DefaultPipelineModel::HalImageStreamBufferPoolT>
DefaultPipelineModel::
replaceHalStreamBufferPoolLocked(
    android::sp<IImageStreamInfo> pStreamInfo,
    android::sp<HalImageStreamBufferPoolT> pPool
)
{
    if  ( pStreamInfo == 0 ) {
        MY_LOGE("NULL IImageStreamInfo");
        return NULL;
    }
    //
    if(mHalImageStreamBufferPoolMap.indexOfKey(pStreamInfo->getStreamId()) >= 0)
        mHalImageStreamBufferPoolMap.replaceValueFor(pStreamInfo->getStreamId(), pPool);
    else
        mHalImageStreamBufferPoolMap.add(pStreamInfo->getStreamId(), pPool);
    return pPool;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultPipelineModel::
submitRequestFrame(
    android::sp<IPipelineFrame> pFrame
)
{
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return DEAD_OBJECT;
    }
    //
    IPipelineDAG::NodeObj_T const nodeObj = pFrame->getPipelineDAG().getRootNode();
    sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(nodeObj.val);
    if  ( pNode == 0 ) {
        MY_LOGE("[frameNo:%d] Bad root node", pFrame->getFrameNo());
        return DEAD_OBJECT;
    }
    //
    MERROR err = OK;
    {
        RWLock::AutoRLock _l(mflushLock);
        if  ( mInFlush ) {
            err = pNode->flush(pFrame);
        }
        else {
            err = pNode->queue(pFrame);
        }
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultPipelineModel::
submitRequest(AppRequest& request)
{
    MERROR err = OK;
    //
    evaluate_app_request_params params;
    params.pAppRequest = &request;
    err = evaluateAppRequest(params);
    if  ( OK != err ) {
        MY_LOGE("evaluate err:%d(%s)", err, ::strerror(-err));
        return err;
    }
    //
    sp<IPipelineFrame> pFrame = params.pFrame;
    MY_LOGE_IF(pFrame==0, "requestNo:%u - NULL PipelineFrame", request.requestNo);
    MY_LOGD_IF(
        mLogLevel >= 1,
        "requestNo:%u Image:I|O=%zu|%zu Meta:I|O=%zu|%zu",
        params.pAppRequest->requestNo,
        params.pAppRequest->vIImageBuffers.size(),
        params.pAppRequest->vOImageBuffers.size(),
        params.pAppRequest->vIMetaBuffers.size(),
        params.vOMetaBuffers.size()
    );
    //
    return submitRequestFrame(pFrame);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultPipelineModel::
setPipelineFrameNumberGenerator(
    android::sp<IPipelineFrameNumberGenerator> aPipelineFrameNumberGenerator
)
{
    RWLock::AutoWLock _l(mFrameNumberGeneratorLock);
    mpPipelineFrameNumberGenerator = aPipelineFrameNumberGenerator;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineFrameNumberGenerator>
DefaultPipelineModel::
getPipelineFrameNumberGenerator() const
{
    RWLock::AutoRLock _l(mFrameNumberGeneratorLock);
    return mpPipelineFrameNumberGenerator;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultPipelineModel::
beginFlush()
{
    {
        RWLock::AutoWLock _l(mflushLock);
        mInFlush = MTRUE;
    }

    RWLock::AutoRLock _l(mRWLock);
    //
    //  flush each node according to the topological sort list.
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
        pNode->flush();
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultPipelineModel::
endFlush()
{
    RWLock::AutoWLock _l(mflushLock);
    mInFlush = MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultPipelineModel::
dump(
    android::Vector<android::String8>& rLogs,
    InfoIOMapSet const& rIOMapSet
) const
{
    rLogs.resize(rIOMapSet.mImageInfoIOMapSet.size() + rIOMapSet.mMetaInfoIOMapSet.size());
    android::Vector<android::String8>::iterator it = rLogs.begin();
    {
        typedef IImageStreamInfo IStreamInfoT;
        typedef InfoIOMapSet::ImageInfoIOMapSet MapSetT;
        MapSetT const& rSet = rIOMapSet.mImageInfoIOMapSet;
        for (size_t j = 0; j < rSet.size(); j++) {
            String8& str = *it++;
            //
            for (size_t k = 0; k < rSet[j].vIn.size(); k++) {
                IStreamInfoT* p = rSet[j].vIn[k].get();
                str += String8::format("%#"PRIxPTR":%s ", p->getStreamId(), p->getStreamName());
            }
            str += " -> ";
            for (size_t k = 0; k < rSet[j].vOut.size(); k++) {
                IStreamInfoT* p = rSet[j].vOut[k].get();
                str += String8::format("%#"PRIxPTR":%s ", p->getStreamId(), p->getStreamName());
            }
        }
    }
    //
    {
        typedef IMetaStreamInfo IStreamInfoT;
        typedef InfoIOMapSet::MetaInfoIOMapSet MapSetT;
        MapSetT const& rSet = rIOMapSet.mMetaInfoIOMapSet;
        for (size_t j = 0; j < rSet.size(); j++) {
            String8& str = *it++;
            //
            for (size_t k = 0; k < rSet[j].vIn.size(); k++) {
                IStreamInfoT* p = rSet[j].vIn[k].get();
                str += String8::format("%#"PRIxPTR":%s ", p->getStreamId(), p->getStreamName());
            }
            str += " -> ";
            for (size_t k = 0; k < rSet[j].vOut.size(); k++) {
                IStreamInfoT* p = rSet[j].vOut[k].get();
                str += String8::format("%#"PRIxPTR":%s ", p->getStreamId(), p->getStreamName());
            }
        }
    }
}

