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

#define LOG_TAG "MtkCam/AppStreamMgr"
//
#include "MyUtils.h"
#include "AppStreamMgr.h"
//
#include <sys/prctl.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Imp;

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
AppStreamMgr::FrameHandler::
FrameHandler(IMetadataProvider const* pMetadataProvider)
    : RefBase()
    , mpMetadataProvider(pMetadataProvider)
{
    IMetadata::IEntry const& entry = mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_REQUEST_PARTIAL_RESULT_COUNT);
    if  ( entry.isEmpty() ) {
        MY_LOGE("no static REQUEST_PARTIAL_RESULT_COUNT");
        mAtMostMetaStreamCount = 1;
    }
    else {
        mAtMostMetaStreamCount = entry.itemAt(0, Type2Type<MINT32>());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
isEmptyFrameQueue() const
{
    return mFrameQueue.empty();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
AppStreamMgr::FrameHandler::
getFrameQueueSize() const
{
    return mFrameQueue.size();
}

MERROR
AppStreamMgr::FrameHandler::
queryOldestRequestNumber(MUINT32 &ReqNo) const
{
    if ( mFrameQueue.empty() ) {
        return 22;
    }
    FrameQueue::const_iterator itFrame = mFrameQueue.begin();
    ReqNo = (*itFrame)->frameNo;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
addConfigStream(AppImageStreamInfo* pStreamInfo)
{
    ImageConfigItem item;
    item.pStreamInfo = pStreamInfo;
    mImageConfigMap.add(pStreamInfo->getStreamId(), item);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
addConfigStream(AppMetaStreamInfo* pStreamInfo)
{
    MetaConfigItem item;
    item.pStreamInfo = pStreamInfo;
    mMetaConfigMap.add(pStreamInfo->getStreamId(), item);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::FrameHandler::
getConfigStreams(ConfigAppStreams& rStreams) const
{
    if  ( mMetaConfigMap.isEmpty() || mImageConfigMap.isEmpty() ) {
        MY_LOGE("Bad mMetaConfigMap.size:%zu, mImageConfigMap.size:%zu", mMetaConfigMap.size(), mImageConfigMap.size());
        return -ENODEV;
    }
    //
    rStreams.vMetaStreams.clear();
    rStreams.vMetaStreams.setCapacity(mMetaConfigMap.size());
    for (size_t i = 0; i < mMetaConfigMap.size(); i++) {
        rStreams.vMetaStreams.add(
            mMetaConfigMap.keyAt(i),
            mMetaConfigMap.valueAt(i).pStreamInfo
        );
    }
    //
    rStreams.vImageStreams.clear();
    rStreams.vImageStreams.setCapacity(mImageConfigMap.size());
    for (size_t i = 0; i < mImageConfigMap.size(); i++) {
        rStreams.vImageStreams.add(
            mImageConfigMap.keyAt(i),
            mImageConfigMap.valueAt(i).pStreamInfo
        );
    }
    //
    IMetadata::IEntry const& entryMinDuration =
        mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    if  ( entryMinDuration.isEmpty() ) {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS");
        return OK;
    }
    IMetadata::IEntry const& entryStallDuration =
        mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STALL_DURATIONS);
    if  ( entryStallDuration.isEmpty() ) {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_STALL_DURATIONS");
        return OK;
    }
    //
    rStreams.vMinFrameDuration.clear();
    rStreams.vStallFrameDuration.clear();
    rStreams.vMinFrameDuration.setCapacity(mImageConfigMap.size());
    rStreams.vStallFrameDuration.setCapacity(mImageConfigMap.size());
    for (size_t i = 0; i < mImageConfigMap.size(); i++) {
        for (size_t j = 0; j < entryMinDuration.count(); j+=4) {
            if (entryMinDuration.itemAt(j    , Type2Type<MINT64>()) == mImageConfigMap.valueAt(i).pStreamInfo->getImgFormatToAlloc() &&
                entryMinDuration.itemAt(j + 1, Type2Type<MINT64>()) == mImageConfigMap.valueAt(i).pStreamInfo->getImgSize().w &&
                entryMinDuration.itemAt(j + 2, Type2Type<MINT64>()) == mImageConfigMap.valueAt(i).pStreamInfo->getImgSize().h) {
                    rStreams.vMinFrameDuration.add(
                        mImageConfigMap.keyAt(i),
                        entryMinDuration.itemAt(j + 3, Type2Type<MINT64>())
                    );
                    rStreams.vStallFrameDuration.add(
                        mImageConfigMap.keyAt(i),
                        entryStallDuration.itemAt(j + 3, Type2Type<MINT64>())
                    );
                MY_LOGI("format:%" PRId64 " size:(%" PRId64 ", %" PRId64 ") min_duration:%" PRId64 ", stall_duration:%" PRId64,
                    entryMinDuration.itemAt(j, Type2Type<MINT64>())
                  , entryMinDuration.itemAt(j + 1, Type2Type<MINT64>())
                  , entryMinDuration.itemAt(j + 2, Type2Type<MINT64>())
                  , entryMinDuration.itemAt(j + 3, Type2Type<MINT64>())
                  , entryStallDuration.itemAt(j + 3, Type2Type<MINT64>()));
                break;
            }
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<AppStreamMgr::AppMetaStreamInfo>
AppStreamMgr::FrameHandler::
getConfigMetaStream(size_t index) const
{
    return mMetaConfigMap[index].pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::FrameHandler::
registerFrame(
    Request const& rRequest
)
{
    sp<FrameParcel> pFrame = new FrameParcel;
    mFrameQueue.push_back(pFrame);
    pFrame->frameNo = rRequest.frameNo;
    //
    //  Request::vInputImageBuffers
    //  Request::vOutputImageBuffers
    {
        registerStreamBuffers(rRequest.vOutputImageBuffers, pFrame, &pFrame->vOutputImageItem);
        registerStreamBuffers(rRequest.vInputImageBuffers,  pFrame, &pFrame->vInputImageItem);
    }
    //
    //  Request::vInputMetaBuffers (Needn't register)
    //  Request::vOutputMetaBuffers
    {
        //registerStreamBuffers(rRequest.vOutputMetaBuffers, pFrame, &pFrame->vOutputMetaItem);
        registerStreamBuffers(rRequest.vInputMetaBuffers,  pFrame, &pFrame->vInputMetaItem);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::FrameHandler::
registerStreamBuffers(
    android::KeyedVector<
        StreamId_T,
        android::sp<AppImageStreamBuffer>
                        > const& buffers,
    android::sp<FrameParcel> const pFrame,
    ImageItemSet*const pItemSet
)
{
    //  Request::v[Input|Output]ImageBuffers
    //  -> FrameParcel
    //  -> mImageConfigMap::vItemFrameQueue
    //
    for (size_t i = 0; i < buffers.size(); i++)
    {
        sp<AppImageStreamBuffer> const pBuffer = buffers[i];
        //
        StreamId_T const streamId = pBuffer->getStreamInfo()->getStreamId();
        //
        ssize_t const index = mImageConfigMap.indexOfKey(streamId);
        if  ( 0 > index ) {
            MY_LOGE("[frameNo:%u] bad streamId:%#" PRIxPTR, pFrame->frameNo, streamId);
            return NAME_NOT_FOUND;
        }
        ImageItemFrameQueue& rItemFrameQueue = mImageConfigMap.editValueAt(index).vItemFrameQueue;
        //
        sp<ImageItem> pItem = new ImageItem;
        //
        rItemFrameQueue.push_back(pItem);
        //
        pItem->pFrame = pFrame.get();
        pItem->pItemSet = pItemSet;
        pItem->buffer = pBuffer;
        pItem->iter = --rItemFrameQueue.end();
        //
        pItemSet->add(streamId, pItem);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::FrameHandler::
registerStreamBuffers(
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        > const& buffers,
    android::sp<FrameParcel> const pFrame,
    MetaItemSet*const pItemSet
)
{
    //  Request::v[Input|Output]MetaBuffers
    //  -> FrameParcel
    //
    for (size_t i = 0; i < buffers.size(); i++)
    {
        sp<IMetaStreamBuffer> const pBuffer = buffers[i];
        //
        StreamId_T const streamId = pBuffer->getStreamInfo()->getStreamId();
        //
        sp<MetaItem> pItem = new MetaItem;
        pItem->pFrame = pFrame.get();
        pItem->pItemSet = pItemSet;
        pItem->buffer = pBuffer;
        //
        pItemSet->add(streamId, pItem);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
AppStreamMgr::FrameHandler::
checkRequestError(FrameParcel const& frame)
{
    /**
     * @return
     *      ==0: uncertain
     *      > 0: it is indeed a request error
     *      < 0: it is indeed NOT a request error
     */
    //  It's impossible to be a request error if:
    //  1) any valid output image streams exist, or
    //  2) all valid output meta streams exist
    //
    //[NOT a request error]
    //
    if  (
            frame.vOutputImageItem.numValidStreams > 0
        ||  (frame.vOutputMetaItem.numValidStreams == frame.vOutputMetaItem.size()
        &&   frame.vOutputMetaItem.hasLastPartial)
        )
    {
        return -1;
    }
    //
    //[A request error]
    //
    if  (
            frame.vOutputImageItem.numErrorStreams == frame.vOutputImageItem.size()
        &&  frame.vOutputMetaItem.numErrorStreams > 0
        )
    {
        return 1;
    }
    //
    //[uncertain]
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareErrorFrame(
    CallbackParcel& rCbParcel,
    android::sp<FrameParcel> const& pFrame
)
{
    rCbParcel.valid = MTRUE;
    //
    {
        CallbackParcel::Error error;
        error.errorCode = CAMERA3_MSG_ERROR_REQUEST;
        //
        rCbParcel.vError.add(error);
        //
    }
    //
    //Note:
    //FrameParcel::vInputImageItem
    //We're not sure whether input image streams are returned or not.
    //
    //FrameParcel::vOutputImageItem
    for (size_t i = 0; i < pFrame->vOutputImageItem.size(); i++) {
        prepareReturnImage(rCbParcel, pFrame->vOutputImageItem.valueAt(i));
    }
    //
    pFrame->errors.markBit(HistoryBit::ERROR_SENT_FRAME);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareErrorMetaIfPossible(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)
{
    sp<FrameParcel> const pFrame = pItem->pFrame;
    if  ( ! pFrame->errors.hasBit(HistoryBit::ERROR_SENT_META) ) {
        pFrame->errors.markBit(HistoryBit::ERROR_SENT_META);
        //
        CallbackParcel::Error error;
        error.errorCode = CAMERA3_MSG_ERROR_RESULT;
        //
        rCbParcel.vError.add(error);
        rCbParcel.valid = MTRUE;
    }
    //
    //  Actually, this item will be set to NULL, and it is not needed for
    //  the following statements.
    //
    pItem->history.markBit(HistoryBit::ERROR_SENT_META);
    //
    if  ( 0 == pFrame->timestampShutter ) {
        MY_LOGW("[frameNo:%u] CAMERA3_MSG_ERROR_RESULT with shutter timestamp = 0", pFrame->frameNo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareErrorImage(
    CallbackParcel& rCbParcel,
    android::sp<ImageItem> const& pItem
)
{
    rCbParcel.valid = MTRUE;
    //
    {
        StreamId_T const streamId = pItem->buffer->getStreamInfo()->getStreamId();
        ImageConfigItem const& rConfigItem = mImageConfigMap.valueFor(streamId);
        //
        CallbackParcel::Error error;
        error.errorCode = CAMERA3_MSG_ERROR_BUFFER;
        error.stream = rConfigItem.pStreamInfo;
        //
        rCbParcel.vError.add(error);
        MY_LOGW_IF(1, "(Error Status) streamId:%#" PRIxPTR "(%s)", streamId, rConfigItem.pStreamInfo->getStreamName());
    }
    //
    //  Actually, this item will be set to NULL, and it is not needed for
    //  the following statements.
    //
    pItem->history.markBit(HistoryBit::ERROR_SENT_IMAGE);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareShutterNotificationIfPossible(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)
{
    sp<FrameParcel> const pFrame = pItem->pFrame;
    if  ( 0 == pFrame->timestampShutter ) {
        IMetadata* pMetadata = pItem->buffer->tryReadLock(LOG_TAG);
        IMetadata::IEntry const entry = pMetadata->entryFor(MTK_SENSOR_TIMESTAMP);
        pItem->buffer->unlock(LOG_TAG, pMetadata);
        //
        if  ( ! entry.isEmpty() && entry.tag() == MTK_SENSOR_TIMESTAMP ) {
            MINT64 const timestamp = entry.itemAt(0, Type2Type<MINT64>());
            //
            pFrame->timestampShutter = timestamp;
            //
            rCbParcel.shutter = new CallbackParcel::Shutter;
            rCbParcel.shutter->timestamp = timestamp;
            rCbParcel.valid = MTRUE;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareReturnMeta(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)
{
    rCbParcel.valid = MTRUE;
    //
    {
        pItem->history.markBit(HistoryBit::RETURNED);
        pItem->pItemSet->numReturnedStreams++;
        //
        Vector<CallbackParcel::MetaItem>* pvCbItem = &rCbParcel.vOutputMetaItem;
        CallbackParcel::MetaItem& rCbItem = pvCbItem->editItemAt(pvCbItem->add());
        rCbItem.buffer = pItem->buffer;

        if  ( pItem->bufferNo == mAtMostMetaStreamCount ) {
            rCbItem.bufferNo = mAtMostMetaStreamCount;
            //
#warning "[FIXME] hardcode: REQUEST_PIPELINE_DEPTH=4"
            IMetadata::IEntry entry(MTK_REQUEST_PIPELINE_DEPTH);
            entry.push_back(4, Type2Type<MUINT8>());
            //
            IMetadata* pMetadata = rCbItem.buffer->tryWriteLock(LOG_TAG);
            pMetadata->update(MTK_REQUEST_PIPELINE_DEPTH, entry);
            rCbItem.buffer->unlock(LOG_TAG, pMetadata);
        }
        else {
            rCbItem.bufferNo = pItem->pItemSet->numReturnedStreams;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
isReturnable(
    android::sp<MetaItem> const& pItem
)   const
{
    if (pItem->bufferNo == mAtMostMetaStreamCount) {
        //the final meta result to return must keep the order submitted.

        FrameQueue::const_iterator itFrame = mFrameQueue.begin();
        while (1)
        {
            sp<FrameParcel> const& pFrame = *itFrame;
            if  ( pFrame == pItem->pFrame ) {
                break;
            }
            //
            if ( pFrame->vOutputMetaItem.isEmpty() ) {
                MY_LOGW("[%d/%d] vOutputMetaItem:%d", pFrame->frameNo, pItem->pFrame->frameNo, pFrame->vOutputMetaItem.size());
                dump();
                return  MFALSE;
            }
            android::sp<MetaItem> const& pMetaItem = pFrame->vOutputMetaItem.valueAt(pFrame->vOutputMetaItem.size() - 1);
            if( (  pFrame->vOutputMetaItem.hasLastPartial && pMetaItem != NULL ) ||
                ( !pFrame->vOutputMetaItem.hasLastPartial && pMetaItem == NULL ))
            {
                MY_LOGW(
                    "Block to return the final meta of frameNo:%u since frameNo:%u (%zu|%zu) partial:%d isNULL:%d",
                    pItem->pFrame->frameNo,
                    pFrame->frameNo,
                    pFrame->vOutputMetaItem.numReturnedStreams,
                    pFrame->vOutputMetaItem.size(),
                    pFrame->vOutputMetaItem.hasLastPartial,
                    pMetaItem == NULL
                );
                return  MFALSE;
            }
            //
            ++itFrame;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
prepareReturnImage(
    CallbackParcel& rCbParcel,
    android::sp<ImageItem> const& pItem
)
{
    rCbParcel.valid = MTRUE;
    //
    {
        pItem->history.markBit(HistoryBit::RETURNED);
        pItem->pItemSet->numReturnedStreams++;
        //
        StreamId_T const streamId = pItem->buffer->getStreamInfo()->getStreamId();
        ImageConfigItem& rConfigItem = mImageConfigMap.editValueFor(streamId);
        rConfigItem.vItemFrameQueue.erase(pItem->iter);
        //
        Vector<CallbackParcel::ImageItem>* pvCbItem = ( pItem->pItemSet->asInput )
                                                    ? &rCbParcel.vInputImageItem
                                                    : &rCbParcel.vOutputImageItem
                                                    ;
        CallbackParcel::ImageItem& rCbItem = pvCbItem->editItemAt(pvCbItem->add());
        rCbItem.buffer = pItem->buffer;
        rCbItem.stream = rConfigItem.pStreamInfo;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
isReturnable(
    android::sp<ImageItem> const& pItem
)   const
{
    StreamId_T const streamId = pItem->buffer->getStreamInfo()->getStreamId();
    ImageItemFrameQueue const& rItemFrameQueue = mImageConfigMap.valueFor(streamId).vItemFrameQueue;
    //
    ImageItemFrameQueue::const_iterator it = rItemFrameQueue.begin();
    for (; it != pItem->iter; it++) {
        if  ( State::IN_FLIGHT == (*it)->state ) {
            return MFALSE;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
isFrameRemovable(
    android::sp<FrameParcel> const& pFrame
)   const
{
    //Not all output image streams have been returned.
    if  ( pFrame->vOutputImageItem.size() != pFrame->vOutputImageItem.numReturnedStreams ) {
        return MFALSE;
    }
    //
    //Not all input image streams have been returned.
    if  ( pFrame->vInputImageItem.size() != pFrame->vInputImageItem.numReturnedStreams ) {
        return MFALSE;
    }
    //
    //
    if  ( pFrame->errors.hasBit(HistoryBit::ERROR_SENT_FRAME) ) {
        //frame error was sent.
        return MTRUE;
    }
    else
    if  ( pFrame->errors.hasBit(HistoryBit::ERROR_SENT_META) ) {
        //meta error was sent.
        if  ( 0 == pFrame->timestampShutter ) {
            MY_LOGW("[frameNo:%u] shutter not sent with meta error", pFrame->frameNo);
        }
    }
    else {
        //Not all meta streams have been returned.
        android::sp<MetaItem> const& pItem = pFrame->vOutputMetaItem.valueAt(pFrame->vOutputMetaItem.size() - 1);
        if( !pFrame->vOutputMetaItem.hasLastPartial || pItem != NULL ) {
            return MFALSE;
        }
        //
        //No shutter timestamp has been sent.
        if  ( 0 == pFrame->timestampShutter ) {
            MY_LOGW("[frameNo:%u] shutter not sent @ no meta error", pFrame->frameNo);
            return MFALSE;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
prepareCallbackIfPossible(
    CallbackParcel& rCbParcel,
    MetaItemSet& rItemSet
)
{
    MBOOL anyUpdate = MFALSE;
    //
    for ( size_t i = 0; i < rItemSet.size(); i++ )
    {
        sp<MetaItem> pItem = rItemSet[i];
        if  ( pItem == 0 ) {
            continue;
        }
        //
        sp<FrameParcel> const pFrame = pItem->pFrame;
        //
        switch  ( pItem->state )
        {
        case State::VALID:{
            //Valid Buffer but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                if  ( isReturnable(pItem) ) {
                    prepareShutterNotificationIfPossible(rCbParcel, pItem);
                    prepareReturnMeta(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::ERROR:{
            //Error Buffer but Not Error Sent Yet
            if  ( ! pItem->history.hasBit(HistoryBit::ERROR_SENT_META) ) {
                if  ( checkRequestError(*pFrame) < 0 ) {
                    //Not a request error
                    prepareErrorMetaIfPossible(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
                else {
                    MY_LOGD("frameNo:%u Result Error Pending", pFrame->frameNo);
                }
            }
            }break;
            //
        default:
            break;
        }
        //
        MBOOL const needRelease =
              ( pItem->buffer->haveAllUsersReleased() == OK )
            &&( pItem->history.hasBit(HistoryBit::RETURNED)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_FRAME)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_META)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE)
              ) ;
        if  ( needRelease ) {
            rItemSet.editValueAt(i) = NULL;
        }
    }
    //
    return anyUpdate;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
AppStreamMgr::FrameHandler::
prepareCallbackIfPossible(
    CallbackParcel& rCbParcel,
    ImageItemSet& rItemSet
)
{
    MBOOL anyUpdate = MFALSE;
    //
    for ( size_t i = 0; i < rItemSet.size(); i++ )
    {
        sp<ImageItem> pItem = rItemSet[i];
        if  ( pItem == 0 ) {
            continue;
        }
        //
        sp<FrameParcel> const pFrame = pItem->pFrame;
        //
        switch  ( pItem->state )
        {
        case State::PRE_RELEASE:{
            //Pre-Release but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                if  ( isReturnable(pItem) ) {
                    prepareReturnImage(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::VALID:{
            //Valid Buffer but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                if  ( isReturnable(pItem) ) {
                    prepareReturnImage(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::ERROR:{
            //Error Buffer but Not Error Sent Yet
            if  ( ! pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE) ) {
                if  ( checkRequestError(*pFrame) < 0 ) {
                    //Not a request error
                    prepareErrorImage(rCbParcel, pItem);
                    if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                        prepareReturnImage(rCbParcel, pItem);
                    }
                    anyUpdate = MTRUE;
                }
                else {
                    MY_LOGV("frameNo:%u Buffer Error Pending, streamId:%#" PRIxPTR, pFrame->frameNo, pItem->buffer->getStreamInfo()->getStreamId());
                }
            }
            }break;
            //
        default:
            break;
        }
        //
        MBOOL const needRelease =
              ( pItem->buffer->haveAllUsersReleased() == OK )
            &&( pItem->history.hasBit(HistoryBit::RETURNED)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_FRAME)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_META)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE)
              ) ;
        if  ( needRelease ) {
            rItemSet.editValueAt(i) = NULL;
        }
    }
    //
    return anyUpdate;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
updateItemSet(MetaItemSet& rItemSet)
{
    for (size_t i = 0; i < rItemSet.size(); i++)
    {
        StreamId_T const streamId = rItemSet.keyAt(i);
        sp<MetaItem> pItem = rItemSet.valueAt(i);
        if  ( pItem == 0 ) {
            MY_LOGV("Meta streamId:%#" PRIxPTR " NULL MetaItem", streamId);
            continue;
        }
        //
        if  ( State::VALID != pItem->state && State::ERROR != pItem->state )
        {
            sp<IMetaStreamBuffer> pStreamBuffer = pItem->buffer;
            //
            if  (
                    pStreamBuffer->getStreamInfo()->getStreamType() != eSTREAMTYPE_META_IN
                &&  pStreamBuffer->haveAllProducerUsersReleased() == OK
                )
            {
                MBOOL const isError = pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR);
                if  ( isError ) {
                    // RELEASE & ERROR BUFFER
                    pItem->state = State::ERROR;
                    pItem->pItemSet->numErrorStreams++;
                    CAM_LOGW(
                        "[Meta Stream Buffer] Error happens..."
                        " - frameNo:%u streamId:%#" PRIxPTR " %s",
                        pItem->pFrame->frameNo, streamId, pStreamBuffer->getName()
                    );
                }
                else {
                    // RELEASE & VALID BUFFER
                    pItem->state = State::VALID;
                    pItem->pItemSet->numValidStreams++;
                }
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
updateItemSet(ImageItemSet& rItemSet)
{
    struct ReleaseHandler
    {
        MVOID
        operator()(
            StreamId_T const        streamId,
            ImageItem* const        pItem,
            AppImageStreamBuffer*   pStreamBuffer
        )
        {
            MBOOL const isError = pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR);
            if  ( isError ) {
                CAM_LOGW(
                    "[Image Stream Buffer] Error happens and all users release"
                    " - frameNo:%u streamId:%#" PRIxPTR " %s state:%s->ERROR",
                    pItem->pFrame->frameNo, streamId, pStreamBuffer->getName(), (( State::IN_FLIGHT == pItem->state ) ? "IN-FLIGHT" : "PRE-RELEASE")
                );
                //We should:
                //  RF = ( ACQUIRE ) ? -1 : AF
                //For simplity, however, no matter if acquire_fence was
                //waited on or not, we just:
                //  RF = AF
                MINT AF = pStreamBuffer->createAcquireFence();
                pStreamBuffer->setReleaseFence(AF);
                pStreamBuffer->setAcquireFence(-1);
                //
                pItem->state = State::ERROR;
                pItem->pItemSet->numErrorStreams++;
            }
            else {
                pStreamBuffer->setReleaseFence(-1);
                pStreamBuffer->setAcquireFence(-1);
                //
                pItem->state = State::VALID;
                pItem->pItemSet->numValidStreams++;
            }
        }
    };
    //
    struct PreReleaseHandler
    {
        MVOID
        operator()(
            StreamId_T const        streamId,
            ImageItem* const        pItem,
            AppImageStreamBuffer*   pStreamBuffer
        )
        {
            //Here the buffer status is uncertain, but its default should be OK.
            MINT RF = pStreamBuffer->createReleaseFence();
            MY_LOGW_IF(
                -1 == RF,
                "[Image Stream Buffer] pre-release but release_fence=-1 !! "
                " - frameNo:%u streamId:%#" PRIxPTR " %s state:IN-FLIGHT->PRE-RELEASE",
                pItem->pFrame->frameNo, streamId, pStreamBuffer->getName()
            );
            pStreamBuffer->setReleaseFence(RF);
            pStreamBuffer->setAcquireFence(-1);
            //
            pItem->state = State::PRE_RELEASE;
        }
    };
    //
    for (size_t i = 0; i < rItemSet.size(); i++)
    {
        StreamId_T const streamId = rItemSet.keyAt(i);
        sp<ImageItem> pItem = rItemSet.valueAt(i);
        if  ( pItem == 0 ) {
            MY_LOGV("Image streamId:%#" PRIxPTR " NULL ImageItem", streamId);
            continue;
        }
        //
        switch  (pItem->state)
        {
        case State::IN_FLIGHT:{
            MUINT32 allUsersStatus = pItem->buffer->getAllUsersStatus();
            //
            //  IN_FLIGHT && all users release ==> VALID/ERROR
            if ( allUsersStatus == IUsersManager::UserStatus::RELEASE )
            {
                ReleaseHandler()(streamId, pItem.get(), pItem->buffer.get());
            }
            //
            //  IN-IN_FLIGHT && all users release or pre-release ==> PRE_RELEASE
            else
            if ( allUsersStatus == IUsersManager::UserStatus::PRE_RELEASE )
            {
                PreReleaseHandler()(streamId, pItem.get(), pItem->buffer.get());
            }
            }break;
            //
        case State::PRE_RELEASE:{
            //  PRE_RELEASE && all users release ==> VALID/ERROR
            if  ( OK == pItem->buffer->haveAllUsersReleased() )
            {
                ReleaseHandler()(streamId, pItem.get(), pItem->buffer.get());
            }
            }break;
            //
        default:
            break;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
update(ResultQueueT const& rvResult)
{
    if  ( mFrameQueue.empty() ) {
        MY_LOGD("Empty FrameQueue:%p %p", &mFrameQueue, this);
        return;
    }
    //
    FrameQueue::iterator itFrame = mFrameQueue.begin();
    for (size_t iResult = 0; iResult < rvResult.size(); iResult++)
    {
        MUINT32 const frameNo = rvResult.valueAt(iResult)->frameNo;
        for (; itFrame != mFrameQueue.end(); itFrame++) {
            //
            sp<FrameParcel>& pFrame = *itFrame;
            if  ( frameNo != pFrame->frameNo ) {
                continue;
            }
            // put output meta into vOutputMetaItem
            sp<MetaItem> pMetaItem = NULL; // last partial metadata
            MetaItemSet* pItemSet = &pFrame->vOutputMetaItem;
            Vector< sp<IMetaStreamBuffer> >::iterator it = rvResult.valueAt(iResult)->buffer.begin();
            for(; it != rvResult.valueAt(iResult)->buffer.end(); it++)
            {
                sp<IMetaStreamBuffer> const pBuffer = *it;
                //
                StreamId_T const streamId = pBuffer->getStreamInfo()->getStreamId();
                //
                sp<MetaItem> pItem = new MetaItem;
                pItem->pFrame = pFrame.get();
                pItem->pItemSet = pItemSet;
                pItem->buffer = pBuffer;
                pItem->bufferNo = pItemSet->size() + 1;
                pMetaItem = pItem;
                //
                pItemSet->add(streamId, pItem);
            }

            if  ( pMetaItem == 0 ) {
                MY_LOGV("frameNo:%u NULL MetaItem", frameNo);
            } else if(rvResult.valueAt(iResult)->lastPartial) {
                pMetaItem->bufferNo = mAtMostMetaStreamCount;
                pItemSet->hasLastPartial = true;
            }
            //
            updateItemSet(pFrame->vOutputMetaItem);
            updateItemSet(pFrame->vOutputImageItem);
            updateItemSet(pFrame->vInputImageItem);
            break;
        }
        //
        if  ( itFrame == mFrameQueue.end() ) {
            MY_LOGW("frameNo:%u is not in FrameQueue; its first frameNo:%u", frameNo, (*mFrameQueue.begin())->frameNo);
            itFrame = mFrameQueue.begin();
        }
    }
    //
    MUINT32 const latestResultFrameNo = rvResult.valueAt(rvResult.size() - 1)->frameNo;
    if  (0 < (MINT32)(latestResultFrameNo - mFrameQueue.latestResultFrameNo) ) {
        mFrameQueue.latestResultFrameNo = latestResultFrameNo;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
update(List<CallbackParcel>& rCbList)
{
    FrameQueue::iterator itFrame = mFrameQueue.begin();
    while ( itFrame != mFrameQueue.end() )
    {
        MUINT32 const frameNo = (*itFrame)->frameNo;
        if  ( 0 < (MINT32)(frameNo - mFrameQueue.latestResultFrameNo) ) {
            MY_LOGV("stop updating frame => frameNo: this(%u) > latest(%u) ", frameNo, mFrameQueue.latestResultFrameNo);
            break;
        }
        //
        CallbackParcel cbParcel;
        cbParcel.valid = MFALSE;
        cbParcel.frameNo = frameNo;
        //
        if  ( checkRequestError(**itFrame) > 0 ) {
            //It's a request error
            //Here we're still not sure that the input image stream is returned or not.
            MY_LOGD("frameNo:%u Request Error", (*itFrame)->frameNo);
            prepareErrorFrame(cbParcel, *itFrame);
        }
        else {
            prepareCallbackIfPossible(cbParcel, (*itFrame)->vOutputMetaItem);
            prepareCallbackIfPossible(cbParcel, (*itFrame)->vOutputImageItem);
            prepareCallbackIfPossible(cbParcel, (*itFrame)->vInputImageItem);
        }
        //
        if  ( cbParcel.valid ) {
            rCbList.push_back(cbParcel);
        }
        //
        if  ( isFrameRemovable(*itFrame) ) {
            //remove this frame from the frame queue.
            itFrame = mFrameQueue.erase(itFrame);
        }
        else {
            //next iteration
            itFrame++;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
update(ResultQueueT const& rvResult, List<CallbackParcel>& rCbList)
{
    update(rvResult);
    update(rCbList);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::FrameHandler::
dump() const
{
    FrameQueue::const_iterator itFrame = mFrameQueue.begin();
    for (; itFrame != mFrameQueue.end(); itFrame++)
    {
        MUINT32 const frameNo = (*itFrame)->frameNo;
        MY_LOGD("frameNo:%u shutter:%" PRId64 " errors:%#x", (*itFrame)->frameNo, (*itFrame)->timestampShutter, (*itFrame)->errors);
        //
        //  Input Image
        {
            ImageItemSet const& rItems = (*itFrame)->vInputImageItem;
            MY_LOGD("\t Input Image");
            MY_LOGD("\t\t return#:%zu valid#:%zu error#:%zu", rItems.numReturnedStreams, rItems.numValidStreams, rItems.numErrorStreams);
            for (size_t i = 0; i < rItems.size(); i++) {
                StreamId_T const streamId = rItems.keyAt(i);
                ImageItem* pItem = rItems.valueAt(i).get();
                if  ( pItem ) {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " "
                        "state:%#x history:%#x buffer:%p %s",
                        streamId, pItem->state, pItem->history, pItem->buffer.get(),
                        (pItem->buffer != 0 ? pItem->buffer->getName() : "")
                    );
                }
                else {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " ", streamId);
                }
            }
        }
        //
        //  Output Image
        {
            ImageItemSet const& rItems = (*itFrame)->vOutputImageItem;
            MY_LOGD("\t Output Image");
            MY_LOGD("\t\t return#:%zu valid#:%zu error#:%zu", rItems.numReturnedStreams, rItems.numValidStreams, rItems.numErrorStreams);
            for (size_t i = 0; i < rItems.size(); i++) {
                StreamId_T const streamId = rItems.keyAt(i);
                ImageItem* pItem = rItems.valueAt(i).get();
                if  ( pItem ) {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " "
                        "state:%#x history:%#x buffer:%p %s",
                        streamId, pItem->state, pItem->history, pItem->buffer.get(),
                        (pItem->buffer != 0 ? pItem->buffer->getName() : "")
                    );
                }
                else {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " ", streamId);
                }
            }
        }
        //
        //  Output Meta
        {
            MetaItemSet const& rItems = (*itFrame)->vOutputMetaItem;
            MY_LOGD("\t Output Meta");
            MY_LOGD("\t\t return#:%zu valid#:%zu error#:%zu", rItems.numReturnedStreams, rItems.numValidStreams, rItems.numErrorStreams);
            for (size_t i = 0; i < rItems.size(); i++) {
                StreamId_T const streamId = rItems.keyAt(i);
                MetaItem* pItem = rItems.valueAt(i).get();
                if  ( pItem ) {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " "
                        "state:%#x history:%#x buffer:%p %s",
                        streamId, pItem->state, pItem->history, pItem->buffer.get(),
                        (pItem->buffer != 0 ? pItem->buffer->getName() : "")
                    );
                }
                else {
                    MY_LOGD("\t\t streamId:%#" PRIxPTR " ", streamId);
                }
            }
        }
    }
}

