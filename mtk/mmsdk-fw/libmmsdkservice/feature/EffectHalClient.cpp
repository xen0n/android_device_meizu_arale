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

//#include <cutils/xlog.h>
#define LOG_TAG "mmsdk/EffectHalClient"

#include <cutils/log.h>
#include <utils/Errors.h>

//#include "EffectHalClient.h"
#include <mmsdk/IEffectHal.h>
#include <common.h>
#include <imagebuf/IGraphicImageBufferHeap.h>
#include <gui/Surface.h>

//use property_get
#include <cutils/properties.h>
//#include <device3/Camera3Device.h>


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

// #define FUNCTION_LOG_START          MY_LOGD_IF(1<=mLogLevel, "+");
// #define FUNCTION_LOG_END            MY_LOGD_IF(1<=mLogLevel, "-");
#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");
/******************************************************************************
 *
 ******************************************************************************/



/******************************************************************************
 *
 ******************************************************************************/
// #include <cutils/xlog.h>

using namespace std;
using namespace NSCam;
using namespace android;


const char EffectHalClient::KEY_PICTURE_SIZE[] = "picture-size";
const char EffectHalClient::KEY_PICTURE_FORMAT[] = "picture-format";


/******************************************************************************
 *
 ******************************************************************************/
void
BufferListener::
onFrameAvailable(const BufferItem& item)
{
    MY_LOGD("BufferListener::[%s]: index=%d", __FUNCTION__, mIdx);
    sp<EffectHalClient> listener;
    listener = mFrameAvailableListener.promote();
    listener->onBufferFrameAvailable(mIdx);

    //listener->availableBufferIdx = mIdx;
}


/******************************************************************************
 *
 ******************************************************************************/
EffectHalClient::
EffectHalClient(IEffectHal* effect, String8 name)
{
    mpEffect = effect;
    mMaxBufferQueueSize = 5;
    mSurfaceMap.clear();
    mInputComsumer.clear();
    mListener.clear();
    //mBufferMap.clear();
    mInputBufferInfo.clear();
    mInputSyncMode.clear();
    mInputFrameInfo.clear();

    mOutputBufferInfo.clear();
    availableBufferIdx = 0;
    mpEffectRequest = NULL;
    mSessionUid = 0;
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.effecthal", cLogLevel, "1");
        mLogLevel = ::atoi(cLogLevel);
    }
    mEffectName = name;
}

/******************************************************************************
 *
 ******************************************************************************/
EffectHalClient::
~EffectHalClient()
{
    FUNCTION_LOG_START;
    MY_LOGD("%s: size: I/O Surface(%d,%d), I/O FrameInfo(%d,%d), I/O Buffer(%d,%d), Listener(%d), mOutputEffectParams(%d), mBufferMap(%d), mOutputBuffers(%d)",
        __FUNCTION__, mOutputSurfaces.size(), mInputComsumer.size(),
        mInputFrameInfo.size(), mOutputFrameInfo.size(),
        mInputBufferInfo.size(), mOutputBufferInfo.size(),
        mListener.size(),
        mOutputEffectParams.size(), mBufferMap.size(), mOutputBuffers.size());

    delete mpEffect;
    mEffectListener = NULL;
    mpEffect = NULL;
    mOutputSurfaces.clear();
    mInputComsumer.clear();
    mInputFrameInfo.clear();
    mOutputFrameInfo.clear();
    mInputBufferInfo.clear();
    mOutputBufferInfo.clear();
    mListener.clear();
    mSurfaceMap.clear();
    mInputSyncMode.clear();
    mOutputSyncMode.clear();
    mOutputEffectParams.clear();
    mBufferMap.clear();
    mOutputBuffers.clear();
    mpEffectRequest = NULL;
    FUNCTION_LOG_END;
}

/******************************************************************************
 * static method
 ******************************************************************************/
static void requestCallback(MVOID* tag, String8 status, sp<EffectRequest>& request)
{
    FUNCTION_LOG_START;
    EffectHalClient* pEffectHalClinetObj = (EffectHalClient*)(tag);
    if (status == "onCompleted")
    {
        pEffectHalClinetObj->requestCompleted(request);
    }
    FUNCTION_LOG_END;
}

/******************************************************************************
 * static method
 ******************************************************************************/
static void frameCallback(MVOID* tag, sp<EffectFrameInfo>& frameInfo)
{
    FUNCTION_LOG_START;
    EffectHalClient* pEffectHalClinetObj = (EffectHalClient*)(tag);
    pEffectHalClinetObj->frameProcessored(frameInfo);
    FUNCTION_LOG_END;
}

/******************************************************************************
 * static method
 ******************************************************************************/
static int parse_pair(const char *str, int *first, int *second, char delim,
                      char **endptr = NULL)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);
    // If a delimeter does not immediately follow, give up.
    if (*end != delim)
    {
        ALOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = (int)strtol(end+1, &end, 10);

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }
    return 0;
}


/******************************************************************************
 *public: // may change state
 ******************************************************************************/
status_t
EffectHalClient::
init()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->init();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
uninit()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->uninit();

    MY_LOGD("%s: size: I/O Surface(%d,%d), I/O FrameInfo(%d,%d), I/O Buffer(%d,%d), Listener(%d), mOutputEffectParams(%d), mBufferMap(%d), mOutputBuffers(%d)",
        __FUNCTION__, mOutputSurfaces.size(), mInputComsumer.size(),
        mInputFrameInfo.size(), mOutputFrameInfo.size(),
        mInputBufferInfo.size(), mOutputBufferInfo.size(),
        mListener.size(),
        mOutputEffectParams.size(), mBufferMap.size(), mOutputBuffers.size());

    delete mpEffect;
    mEffectListener = NULL;
    mpEffect = NULL;
    mOutputSurfaces.clear();
    mInputComsumer.clear();
    mInputFrameInfo.clear();
    mOutputFrameInfo.clear();
    mInputBufferInfo.clear();
    mOutputBufferInfo.clear();
    mListener.clear();
    mSurfaceMap.clear();
    mInputSyncMode.clear();
    mOutputSyncMode.clear();
    mOutputEffectParams.clear();
    mBufferMap.clear();
    mOutputBuffers.clear();
    mpEffectRequest = NULL;
    FUNCTION_LOG_END;

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
configure()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->configure();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
unconfigure()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->unconfigure();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
uint64_t
EffectHalClient::
start()
{
    FUNCTION_LOG_START;
    //status_t ret = OK;
    //uint64_t uid;

    mSessionUid = mpEffect->start();

    //test, get output buffer
    mOutputBufferInfo.clear();
    for (size_t idx = 0; idx < mOutputSurfaces.size(); ++idx)
    {
        sp<ANativeWindow> anw = mOutputSurfaces[idx];
        getOutputBuffer(idx, mOutputEffectParams[idx]->getInt("picture-number"),
                        anw, mOutputEffectParams[idx]);
    }

FUNCTION_END:
    FUNCTION_LOG_END;
    //return ret;
    return mSessionUid;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
abort(EffectParameter const *parameter)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->abort();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}



/******************************************************************************
 *public: // would not change state
 ******************************************************************************/
status_t
EffectHalClient::
getNameVersion(EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->getNameVersion(nameVersion);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
setEffectListener(const wp<IEffectListener> &listener)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    mEffectListener = listener.promote();

    ret = mpEffect->setEffectListener(this);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
setParameter(String8 &key, String8 &object)
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    MY_LOGD_IF(mLogLevel, "[%s]: key=%s, value:%s", __FUNCTION__,
        key.string(), object.string());
    ret = mpEffect->setParameter(key, object);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
setParameters(sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    MY_LOGD_IF(mLogLevel, "[%s]: height=%s, level:%s", __FUNCTION__,
        parameter->get("picture-height"), parameter->get("fb-smooth-level"));

    ret = mpEffect->setParameters(parameter);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
setStaticMetadata(sp<BasicParameters> staticMetadata)
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    MY_LOGD_IF(mLogLevel, "[%s] maxJpegsize=%d, maxJpegWidth=%d, maxJpegHeight=%d",
            __FUNCTION__, staticMetadata->getInt("maxJpegsize"),
            staticMetadata->getInt("maxJpegWidth"),
            staticMetadata->getInt("maxJpegHeight"));

    mpStaticMetadata = staticMetadata;
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->getCaptureRequirement(inputParam, requirements);

    //set surface-id
    int surfaceId;
    EffectCaptureRequirement*const editedRequirements = requirements.editArray();
    for(int i=0; i<requirements.size(); ++i)
    {
        if (requirements[i].get(KEY_PICTURE_SIZE) && requirements[i].get(KEY_PICTURE_FORMAT) )
        {
            String8 picFormat = String8(requirements[i].get(KEY_PICTURE_SIZE));
            picFormat += requirements[i].get(KEY_PICTURE_FORMAT);
            surfaceId = mSurfaceMap.valueFor(picFormat);
            MY_LOGD_IF(mLogLevel, "[%s]: picFormat=%s, surfaceId=%d",
                __FUNCTION__, picFormat.string(), surfaceId);
            editedRequirements[i].set("surface-id", surfaceId);
        }
        else
        {
            ALOGE("picture-size or picture-format is null");
        }
    }


FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}



/******************************************************************************
 * non-blocking
 ******************************************************************************/
status_t
EffectHalClient::
prepare()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->prepare();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
release()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    ret = mpEffect->release();

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


// status_t
// EffectHalClient::
// addInputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter )
// {
//     FUNCTION_LOG_START;

// FUNCTION_END:
//     FUNCTION_LOG_END;
//     return INVALID_OPERATION;
// }


// status_t
// EffectHalClient::
// addOutputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter )
// {
//     FUNCTION_LOG_START;

// FUNCTION_END:
//     FUNCTION_LOG_END;
//     return INVALID_OPERATION;
// }

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
updateEffectRequest(const sp<EffectRequest> request)
{
    FUNCTION_LOG_START;

FUNCTION_END:
    FUNCTION_LOG_END;
    return INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
addBufferQueue(Vector< sp<IGraphicBufferProducer> > &input, int index, int *height, int *width, int format)
{
    //create bufferqueue and bufferItemConsumer
    status_t res;
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    sp<BufferItemConsumer> itemConsumer;

    BufferQueue::createBufferQueue(&producer, &consumer);

    //get buffer count
    int minUndequeuedBuffers = 0;
    res = producer->query(NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBuffers);
    if (res != OK || minUndequeuedBuffers < 0)
    {
        ALOGE("%s: Could not query min undequeued buffers (error %d, bufCount %d)",
              __FUNCTION__, res, minUndequeuedBuffers);
        return;
    }
    size_t minBufs = static_cast<size_t>(minUndequeuedBuffers);
    mMaxBufferQueueSize = mMaxBufferQueueSize > minBufs ? mMaxBufferQueueSize : minBufs;

    itemConsumer = new BufferItemConsumer(consumer, (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN)
                            |(GRALLOC_USAGE_HW_CAMERA_READ|GRALLOC_USAGE_HW_CAMERA_WRITE), mMaxBufferQueueSize);

    //MY_LOGD_IF(mLogLevel, "[%s]: producer(Binder):%p, minBufs=%d, mMaxBufferQueueSize=%d, itemConsumer=%p",
    //    __FUNCTION__, producer->asBinder().get(), minBufs, mMaxBufferQueueSize, itemConsumer.get());

    MY_LOGD_IF(mLogLevel, "[%s]: producer(Binder):%p, minBufs=%d, mMaxBufferQueueSize=%d, itemConsumer=%p",
        __FUNCTION__, IInterface::asBinder(producer).get(), minBufs, mMaxBufferQueueSize, itemConsumer.get());

    //add EffectHalClient instance to BufferListener and add BufferListener instance to bufferqueue
    sp<BufferItemConsumer::FrameAvailableListener> listener;
    listener = new BufferListener(index, this);
    itemConsumer->setFrameAvailableListener(listener);
    mListener.push_back(listener);

    //set consumer name
    itemConsumer->setName(String8::format("BQ-%s-%d", mEffectName.string(), index));

    //set buffer size
    res = itemConsumer->setDefaultBufferSize(*width, *height);
    if (res != OK)
    {
        ALOGE("%s: Could not set buffer dimensions (w = %d, h = %d)", __FUNCTION__, *width, *height);
        return;
    }
    MY_LOGD_IF(mLogLevel, "[%s]: set buffer dimensions (w = %d, h = %d)", __FUNCTION__, *width, *height);

    //set buffer format
    res = itemConsumer->setDefaultBufferFormat(format);
    if (res != OK)
    {
        ALOGE("%s: Could not set buffer format:(0x%x)", __FUNCTION__, format);
        return;
    }
    MY_LOGD_IF(mLogLevel, "[%s]: set buffer format:(0x%x)", __FUNCTION__, format);

    mInputComsumer.push_back(itemConsumer);
    input.push_back(producer);
    //set default sync mode
    mInputSyncMode.push_back(false);
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
requestCompleted(sp<EffectRequest>& request)
{
    FUNCTION_LOG_START;
    mEffectListener->onCompleted(this, *(request->getRequestResult().get()), mSessionUid);
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
frameProcessored(sp<EffectFrameInfo>& frameInfo)
{
    FUNCTION_LOG_START;

    frameInfo->getFrameParameter()->dump();
    if (frameInfo->getFrameParameter()->getInt("vIn") == 1)
    {
        //input frame processored
        MY_LOGD("%s: Effect-[%s]", __FUNCTION__, mEffectName.string());
        mEffectListener->onInputFrameProcessed(this, frameInfo->getFrameParameter(), *(frameInfo->getFrameResult().get()));
        int64_t timestamp = frameInfo->getFrameParameter()->getInt64("timestamp");
        if(mInputBufferInfo.indexOfKey(timestamp) < 0)
        {
            MY_LOGE("[%s]: imgBuffer not found, timestamp=%" PRId64 " ", __FUNCTION__, timestamp);
            return;
        }
        //BufferItemConsumer::BufferItem imgBuffer = mBufferMap.valueFor(timestamp);
        BufferItem imgBuffer = mInputBufferInfo.valueFor(timestamp)->mBufferItem;
        int idx = frameInfo->getFrameParameter()->getInt("surfaceId");
        if (idx < 0)
        {
            MY_LOGE("[%s]: Error : frameInfo's surfaceId < 0!!", __FUNCTION__);
            return;
        }
        sp<BufferItemConsumer> consumer = mInputComsumer[idx];
        MY_LOGD_IF(mLogLevel, "[%s]: surface-id:%d ", __FUNCTION__,
            idx);
        mInputBufferInfo.removeItem(timestamp);
        consumer->releaseBuffer(imgBuffer);
        // consumer.clear();
        // mInputComsumer.clear();
    }
    else
    {
        //output frame processored
        MY_LOGD("%s: Effect-[%s]", __FUNCTION__, mEffectName.string());
        //get surface index and buffer index
        int idx = frameInfo->getFrameParameter()->getInt("surfaceId");
        int bufferIndex = frameInfo->getFrameParameter()->getInt("bufferIndex");
        MY_LOGD_IF(mLogLevel, "[%s]: surface-id:%d bufferIndex:%d", __FUNCTION__, idx, bufferIndex);

        //set timestamp
        if (idx < 0)
        {
            MY_LOGE("[%s]: Error : frameInfo's surfaceId < 0!!", __FUNCTION__);
            return;
        }
        sp<ANativeWindow> anw = mOutputSurfaces[idx];
        int64_t timestamp = 123456L;
        MY_LOGD_IF(mLogLevel, "[%s]: Set timestamp=%" PRId64 " to %p", __FUNCTION__, timestamp, anw.get());
        status_t res = native_window_set_buffers_timestamp(anw.get(), timestamp);
        if (res != OK)
        {
            ALOGE("[%s]: Error setting timestamp: %s (%d)", __FUNCTION__, strerror(-res), res);
            return;
        }

        //queue buffer to bufferQueue
        if (bufferIndex < 0)
        {
            MY_LOGE("[%s]: Error : frameInfo's bufferIndex < 0!!", __FUNCTION__);
            return;

        }
        sp<GraphicBuffer> buf = mOutputBufferInfo[bufferIndex]->pImageBuffer;

        //debug: output result
        /*sp<IImageBuffer> imageBuffer;
        convertGraphicBufferToIImageBuffer(buf, imageBuffer);

        imageBuffer->lockBuf( String8::format("%s-%d-%d", __FUNCTION__, idx, bufferIndex),
        eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );

        MBOOL ret = imageBuffer->saveToFile(String8::format("/sdcard/output-%s.jpg", mEffectName.string()));
        MY_LOGD_IF(mLogLevel, "[%s] save iamge end ret=%d ", __FUNCTION__, ret);
        imageBuffer->unlockBuf(String8::format("%s-%d-%d", __FUNCTION__, idx, bufferIndex));*/

        res = anw->queueBuffer(anw.get(), buf->getNativeBuffer(), -1);
        mOutputBufferInfo.replaceAt(bufferIndex);
        if (res != OK)
        {
            MY_LOGE("%s: Unable to queue output frame: %s (%d)", __FUNCTION__, strerror(-res), res);
            return;
        }
        mOutputBufferInfo.replaceAt(bufferIndex);
        if (res != OK)
        {
            MY_LOGE("%s: Unable to queue output frame: %s (%d)", __FUNCTION__, strerror(-res), res);
            return;
        }

        buf = NULL;
        //--------------------------------
        // res = native_window_api_disconnect(anw.get(), NATIVE_WINDOW_API_CAMERA);

        // /**
        //  * This is not an error. if client calling process dies, the window will
        //  * also die and all calls to it will return DEAD_OBJECT, thus it's already
        //  * "disconnected"
        //  */
        // if (res == DEAD_OBJECT) {
        //     ALOGW("%s: While disconnecting stream from native window, the"
        //             " native window died from under us", __FUNCTION__);
        // }
        // else if (res != OK) {
        //     ALOGE("%s: Unable to disconnect stream from native window "
        //           "(error %d %s)", __FUNCTION__, res, strerror(-res));
        // }
        //--------------------------------
        //on event
        mEffectListener->onOutputFrameProcessed(this, frameInfo->getFrameParameter(), *(frameInfo->getFrameResult().get()));
    }


    FUNCTION_LOG_END;
    return;
}


/******************************************************************************
 * API for buffer queue
 ******************************************************************************/
status_t
EffectHalClient::
getInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input)
{
    FUNCTION_LOG_START;
    mSurfaceMap.clear();
    mInputComsumer.clear();
    mListener.clear();
    //mBufferMap.clear();
    mInputBufferInfo.clear();
    mInputSyncMode.clear();
    mInputFrameInfo.clear();

    EffectParameter *inputParam = NULL;
    Vector<EffectCaptureRequirement> requirements;
    status_t ret = mpEffect->getCaptureRequirement(inputParam, requirements);
    MY_LOGD_IF(mLogLevel, "[%s]: requirements.size()=%d", __FUNCTION__, requirements.size());


    //init EffectRequest
    if (mpEffectRequest == NULL)
    {
        mpEffectRequest = new EffectRequest(mSessionUid, requestCallback, (void*)this);
    }
    mpEffectRequest->vInputFrameInfo.clear();
    mpEffectRequest->vInputFrameInfo.setCapacity(requirements.size());
    MY_LOGD_IF(mLogLevel, "[%s]: init vInputFrameInfo, size=%d", __FUNCTION__, requirements.size());


    //decided surface numbers according to picture size and format
    for(int i=0; i<requirements.size(); ++i)
    {
        //get format and hight,weight
        if (requirements[i].get(KEY_PICTURE_SIZE) && requirements[i].get(KEY_PICTURE_FORMAT) )
        {
#if (PLATFORM_SDK_VERSION >= 21)
            if (mpStaticMetadata->getInt("isHal3") == 1){  // Only Hal3 need to check
                if (mpStaticMetadata->getInt(String8(requirements[i].get(KEY_PICTURE_FORMAT))) != 1)
                {
                    MY_LOGE("PLATFORM verions is %d, isHal3 = %d", PLATFORM_SDK_VERSION, mpStaticMetadata->getInt("isHal3"));
                    MY_LOGE("[%s]: format:%s is not support on this project", __FUNCTION__, requirements[i].get(KEY_PICTURE_FORMAT));
                    return BAD_VALUE;
                }
                else
                {
                    MY_LOGD("[%s]: format:%s is support on this project, PLATFORM_SDK_VERSION=%d",
                        __FUNCTION__, requirements[i].get(KEY_PICTURE_FORMAT), PLATFORM_SDK_VERSION);
                }
            }
#endif
            String8 picFormat = String8(requirements[i].get(KEY_PICTURE_SIZE));
            //check format is support, hal1 can support any format.
            MY_LOGD("[%s]: PLATFORM_SDK_VERSION=%d, isHal3=%d", __FUNCTION__, PLATFORM_SDK_VERSION, mpStaticMetadata->getInt("isHal3"));

            picFormat += requirements[i].get(KEY_PICTURE_FORMAT);
            MY_LOGD_IF(mLogLevel, "[%s]: picFormat=%s, is exist=%d", __FUNCTION__,
                picFormat.string(), mSurfaceMap.valueFor(picFormat));

            if (mSurfaceMap.valueFor(picFormat) == 0)
            {
                int width, height;
                int _size = mSurfaceMap.size();
                mSurfaceMap.replaceValueFor(picFormat, _size+1);
                parse_pair(requirements[i].get(KEY_PICTURE_SIZE), &height, &width, 'x');
                addBufferQueue(input, _size, &height, &width, requirements[i].getInt(KEY_PICTURE_FORMAT));
            }
        }
        else
        {
            MY_LOGD_IF(mLogLevel, "[%s]: picture-size or picture-format is null", __FUNCTION__);
        }
    }

    MY_LOGD_IF(mLogLevel, "[%s]: mListener size=%d", __FUNCTION__, mListener.size());
    FUNCTION_LOG_END;
    return OK;
}

/******************************************************************************
 * API for buffer queue
 * init surfaces and parameters
 * configure surface
    #.connect to bufferQueue
    #.query width, height and format info. for debug
 *
 ******************************************************************************/
status_t
EffectHalClient::
setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &ouput, Vector<sp<EffectParameter> > &effectParams)
{
    FUNCTION_LOG_START;

    // init surfaces and parameters
    mOutputSurfaces.clear();
    mOutputSurfaces.setCapacity(ouput.size());
    mOutputEffectParams.clear();
    mOutputEffectParams.setCapacity(effectParams.size());

    mOutputEffectParams = effectParams;

    size_t outputFrameCount = 0;

    //configure
    for (size_t idx = 0; idx < ouput.size(); ++idx)
    {
        sp<Surface> s = new Surface(ouput[idx]);
        sp<ANativeWindow> anw = s;
        //sp<ANativeWindow> anw(s);
        mOutputSurfaces.push_back(s);
        mOutputEffectParams.push_back(effectParams[idx]);
        status_t res;
        ANativeWindowBuffer* anb;

        //MY_LOGD_IF(mLogLevel, "[%s]: idx=%d, producer(Binder):%p", __FUNCTION__, idx, ouput[idx]->asBinder().get());
        MY_LOGD_IF(mLogLevel, "[%s]: idx=%d, producer(Binder):%p", __FUNCTION__, idx, IInterface::asBinder(ouput[idx]).get());

        // connect to bufferQueue
        res = native_window_api_connect(anw.get(), NATIVE_WINDOW_API_CAMERA);
        MY_LOGD_IF(mLogLevel, "[%s]: connect=%d, native window is: %p", __FUNCTION__, res, anw.get());
        if (res != OK)
        {
            ALOGE("%s: Unable to connect ANativeWindow: %s (%d)  [%s]", __FUNCTION__, strerror(-res), res, mEffectName.string());
            //for test, do reconnect
            // res = native_window_api_disconnect(anw.get(), NATIVE_WINDOW_API_CAMERA);
            // MY_LOGD_IF(mLogLevel, "[%s]: disconnect=%d, native window is: %p", __FUNCTION__, res, anw.get());
            // res = native_window_api_connect(anw.get(), NATIVE_WINDOW_API_CAMERA);
            // MY_LOGD_IF(mLogLevel, "[%s]: connect retry, result=%d, native window is: %p", __FUNCTION__, res, anw.get());
        }

        // query width, height format and usage info. for debug
        int width, height, format;
        int32_t consumerUsage;
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_WIDTH, &width)) != OK) {
            ALOGE("[%s]: Failed to query Surface width", __FUNCTION__);
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_HEIGHT, &height)) != OK) {
            ALOGE("[%s]: Failed to query Surface height", __FUNCTION__);
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_FORMAT, &format)) != OK) {
            ALOGE("[%s]: Failed to query Surface format", __FUNCTION__);
            return res;
        }
        if ((res = anw->query(anw.get(), NATIVE_WINDOW_CONSUMER_USAGE_BITS, &consumerUsage)) != OK) {
            ALOGE("%s: Failed to query consumer usage", __FUNCTION__);
            return res;
        }
        MY_LOGD_IF(mLogLevel, "[%s]: (w = %d, h = %d), format:(0x%x), consumerUsage=%d",
                    __FUNCTION__, width, height, format, consumerUsage);



        // for test, check camera device allow and disallow usage.
        /*int32_t disallowedFlags = GraphicBuffer::USAGE_HW_VIDEO_ENCODER |
                                  GRALLOC_USAGE_RENDERSCRIPT;
        int32_t allowedFlags = GraphicBuffer::USAGE_SW_READ_MASK |
                               GraphicBuffer::USAGE_HW_TEXTURE |
                               GraphicBuffer::USAGE_HW_COMPOSER;
        bool flexibleConsumer = (consumerUsage & disallowedFlags) == 0 &&
                (consumerUsage & allowedFlags) != 0;
        ALOGD("[%s]: consumerUsage=%08x, disallowedUsage:%08x, allowedUsage:%08x, isvalid=%d)",
            __FUNCTION__, consumerUsage, disallowedFlags, allowedFlags, flexibleConsumer);*/

        // Configure consumer-side ANativeWindow interface
        //for test, 0x60033 copy from Camera3OutputStream.cpp
        //GRALLOC_USAGE_HW_CAMERA_MASK        = 0x00060000
        //GRALLOC_USAGE_SW_WRITE_OFTEN        = 0x00000030
        //GRALLOC_USAGE_SW_READ_OFTEN         = 0x00000003
        int32_t effectUsage = 0x60033;
        res = native_window_set_usage(anw.get(), effectUsage);
        MY_LOGD_IF(mLogLevel, "[%s]: native_window_set_usage :%08x", __FUNCTION__, effectUsage);
        if (res != OK) {
            ALOGE("[%s]: Unable to configure usage %08x", __FUNCTION__, effectUsage);
        }

        res = native_window_set_scaling_mode(anw.get(),
                NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        if (res != OK) {
            ALOGE("[%s]: Unable to configure stream scaling: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
        }

        ssize_t jpegBufferSize = 0;
        if (format == HAL_PIXEL_FORMAT_BLOB)
        {
            // Calculate final jpeg buffer size for the given resolution.
            ssize_t maxJpegBufferSize = mpStaticMetadata->getInt("maxJpegsize");
            ssize_t maxJpegWidth = mpStaticMetadata->getInt("maxJpegWidth");
            ssize_t maxJpegHeight = mpStaticMetadata->getInt("maxJpegHeight");
            if (maxJpegBufferSize == 0)
            {
                if (maxJpegWidth == 0 || maxJpegHeight == 0)
                {
                    ALOGE("[%s]: Error staticMetadat, maxJpegWidth=%d, maxJpegHeight=%d", __FUNCTION__, maxJpegWidth, maxJpegHeight);
                    return -22;
                }
                float scaleFactor = ((float) (width * height)) /
                (maxJpegWidth * maxJpegHeight);

                jpegBufferSize = scaleFactor * (maxJpegBufferSize - kMinJpegBufferSize) +
                    kMinJpegBufferSize;

                MY_LOGD_IF(mLogLevel, "[%s]: jpegBufferSize=%d, kMinJpegBufferSize=%d, scaleFactor=%f",
                __FUNCTION__, jpegBufferSize, kMinJpegBufferSize, scaleFactor);

                if (jpegBufferSize > maxJpegBufferSize)
                {
                    jpegBufferSize = maxJpegBufferSize;
                }
            }
            else
            {
                jpegBufferSize = width*height*1.2;
                if (jpegBufferSize > 5898240)
                {
                    jpegBufferSize = 5898240;
                }
            }

            res = native_window_set_buffers_dimensions(anw.get(), jpegBufferSize, 1);
            MY_LOGD_IF(mLogLevel, "[%s]: native_window_set_buffers_dimensions: (w = %d, h = %d), realH:%d, realW:%d",
                __FUNCTION__, width, height, jpegBufferSize, 1);
        }
        else
        {
            // For buffers of known size
            res = native_window_set_buffers_dimensions(anw.get(), width, height);
            MY_LOGD_IF(mLogLevel, "[%s]: set non jpeg dimension: (w = %d, h = %d), realH:%d, realW:%d",
                __FUNCTION__, width, height, height, width);
        }


        // //check format and set the buffer width and height
        // //if format is blob(same to jpeg), have to get jpeg buffer size, for test set 1105920 directly.
        // // ssize_t jpegBufferSize = width*height*1.2;
        // // if (jpegBufferSize > 5898240)
        // // {
        // //     jpegBufferSize = 5898240;
        // // }
        // if (format == HAL_PIXEL_FORMAT_BLOB) {
        //     //ssize_t jpegBufferSize = Camera3Device::getJpegBufferSize(width, height); //must call with instance
        //     // if (jpegBufferSize <= 0) {
        //     //     ALOGE("Invalid jpeg buffer size %zd", jpegBufferSize);
        //     //     return BAD_VALUE;
        //     // }
        //     // For buffers with bounded size
        //     res = native_window_set_buffers_dimensions(anw.get(), jpegBufferSize, 1);
        //     MY_LOGD_IF(mLogLevel, "[%s]: native_window_set_buffers_dimensions: (w = %d, h = %d), realH:%d, realW:%d",
        //         __FUNCTION__, width, height, jpegBufferSize, 1);

        // } else {
        //     // For buffers of known size
        //     res = native_window_set_buffers_dimensions(anw.get(), width, height);
        //     MY_LOGD_IF(mLogLevel, "[%s]: set non jpeg dimension: (w = %d, h = %d), realH:%d, realW:%d",
        //         __FUNCTION__, width, height, height, width);
        // }

        if (res != OK) {
            ALOGE("%s: Unable to configure stream buffer dimensions"
                    " %d x %d (maxSize %zu)", __FUNCTION__, height, width, jpegBufferSize);
        }


        //set buffer format, can remove if ap is configed.
        res = native_window_set_buffers_format(anw.get(), format);
        MY_LOGD_IF(mLogLevel, "[%s]: native_window_set_buffers_format :format:(0x%x)", __FUNCTION__, format);
        if (res != OK) {
            ALOGE("%s: Unable to configure stream buffer format %#x", __FUNCTION__, format);
            return res;
        }

        int maxConsumerBuffers;
        res = anw->query(anw.get(), NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &maxConsumerBuffers);
        if (res != OK) {
            ALOGE("%s: Unable to query consumer undequeued", __FUNCTION__);
            return res;
        }

        MY_LOGD_IF(mLogLevel, "[%s]: Consumer wants %d buffers, HAL wants %d", __FUNCTION__, maxConsumerBuffers, 5);
        MY_LOGD_IF(mLogLevel, "[%s]: picture-numbers=%d", __FUNCTION__, effectParams[idx]->getInt("picture-number"));

        outputFrameCount += effectParams[idx]->getInt("picture-number");
        //ap have to set this
        res = native_window_set_buffer_count(anw.get(), 10);
        if (res != OK)
        {
            ALOGE("%s: Unable to native_window_set_buffer_count: %s (%d)", __FUNCTION__, strerror(-res), res);
            return OK;
        }
    }

    //new add, use picture-number init vOutputFrameInfo
    //init EffectRequest
    if (mpEffectRequest == NULL)
    {
        mpEffectRequest = new EffectRequest(1, requestCallback, (void*)this);
    }
    mpEffectRequest->vOutputFrameInfo.clear();
    mpEffectRequest->vOutputFrameInfo.setCapacity(outputFrameCount);
    MY_LOGD_IF(mLogLevel, "[%s]: init vOutputFrameInfo, size=%d", __FUNCTION__, outputFrameCount);

    FUNCTION_LOG_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
addInputParameter(int index, sp<EffectParameter> &parameter, int64_t timestamp, bool repeat)
//addInputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat)
{
    MY_LOGD_IF(mLogLevel, "[%s]: index=%d, mInputFrameInfo size=%d",
        __FUNCTION__, index, mInputFrameInfo.size());


    //new
    if(mpEffectRequest->vInputFrameInfo.indexOfKey(timestamp) < 0)
    {
        //init EffectFrameInfo object
        sp<EffectFrameInfo> frame = new EffectFrameInfo(mpEffectRequest->getRequestNo(),
                                        timestamp, frameCallback, (void*)this);
        frame->setFrameParameter(parameter);
        mpEffectRequest->vInputFrameInfo.replaceValueFor(timestamp, frame);
    }
    else
    {
        sp<EffectFrameInfo> frame = mpEffectRequest->vInputFrameInfo.editValueFor(timestamp);
        frame->setFrameParameter(parameter);
        //do update
        mpEffect->updateEffectRequest(mpEffectRequest);
    }
    return OK;
    //new end


    /*int res;
    bool sync;
    sp<FrameInfo> frameInfo = new FrameInfo();
    frameInfo->mIndex = index;
    frameInfo->mTimestamp = timestamp;
    frameInfo->mRepeat = repeat;
    frameInfo->mEffectParam = parameter;

    mInputFrameInfo.replaceValueFor(timestamp, frameInfo);
    //mInputFrameInfo.push_back(frameInfo);
    // size_t frameInfoSize = mInputFrameInfo.size();
    // MY_LOGD_IF(mLogLevel, "mInputFrameInfo replace idx=%d, ts=%d, ready=%d, repeat=%d",
    //             index, mInputFrameInfo[frameInfoSize-1]->mTimestamp,
    //             mInputFrameInfo[frameInfoSize-1]->isReady,
    //             mInputFrameInfo[frameInfoSize-1]->mRepeat);


    MY_LOGD_IF(mLogLevel, "[%s]: mInputFrameInfo mInputFrameInfo idx=%d, ts=%" PRId64 " ", __FUNCTION__,
            mInputFrameInfo.valueFor(timestamp)->mIndex, mInputFrameInfo.valueFor(timestamp)->mTimestamp);

    //check sync mode
    sync = getInputsyncMode(index);
    if (sync)
    {
        //if(mBufferMap.indexOfKey(timestamp) < 0)
        if(mInputBufferInfo.indexOfKey(timestamp) < 0)
        {
            MY_LOGD_IF(mLogLevel, "[%s]: Save buffer info.(surface index=%d)", __FUNCTION__, index);
        }
        else
        {
            //BufferItemConsumer::BufferItem imgBuffer = mBufferMap.valueFor(timestamp);
            BufferItemConsumer::BufferItem imgBuffer = mInputBufferInfo.valueFor(timestamp)->mBufferItem;
            MY_LOGD_IF(mLogLevel, "[%s]: matched: index=%d, ts=%" PRId64 "", __FUNCTION__, index, timestamp);
            //convert to image buffer
            sp<IImageBuffer> imageBuffer;
            convertGraphicBufferToIImageBuffer(imgBuffer.mGraphicBuffer, imageBuffer);

            res = mpEffect->addInputFrame(imageBuffer, parameter);
        }
    }
    else
    {
        //call directly
        sp<IImageBuffer> imageBuffer;
        res = mpEffect->addInputFrame(imageBuffer, parameter);
    }
    return OK;*/
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
addOutputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat)
{
    //@todo implement this
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
setInputsyncMode(int index, bool sync)
{
    //david add
    FUNCTION_LOG_START;
    if (index > mInputSyncMode.size()-1)
    {
        //return outof index;
        return OK;
    }
    MY_LOGD_IF(mLogLevel, "[%s]: index=%d, sync mode=%d", __FUNCTION__, index, int(sync));
    mInputSyncMode.replaceAt(sync, index);
    FUNCTION_LOG_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectHalClient::
getInputsyncMode(int index)
{
    //david add
    MY_LOGD_IF(mLogLevel, "[%s]: index=%d, sync mode=%d", __FUNCTION__,
        index, int(mInputSyncMode.itemAt(index)));
    return mInputSyncMode.itemAt(index);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EffectHalClient::
setOutputsyncMode(int index, bool sync)
{
    //david add
    FUNCTION_LOG_START;
    if (index < mOutputSyncMode.size()-1)
    {
        //return outof index;
        return OK;
    }
    mOutputSyncMode.replaceAt(sync, index);
    FUNCTION_LOG_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EffectHalClient::
getOutputsyncMode(int index)
{
    //david add
    return mOutputSyncMode.itemAt(index);
}

/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
onInputFrameAvailable()
{
    //@todo implement this
}


void
EffectHalClient::
onInputSurfacesChanged(EffectResult partialResult)
{
    //@todo implement this
}

status_t
EffectHalClient::
setInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input)
{
    //@todo implement this
    return OK;
}

status_t
EffectHalClient::
getOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &ouput)
{
    //@todo implement this
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
convertGraphicBufferToIImageBuffer(sp<GraphicBuffer> &buf, sp<IImageBuffer> &imageBuffer)
{
#if '1' == MTKCAM_HAVE_IIMAGE_BUFFER
    sp<IGraphicImageBufferHeap> bufferHeap = IGraphicImageBufferHeap::create(LOG_TAG, buf.get());
    //IGraphicImageBufferHeap *bufferHeap = IGraphicImageBufferHeap::create(LOG_TAG, buf.get());
    imageBuffer = bufferHeap->createImageBuffer();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
getOutputBuffer(int surfaceIdx, int bufferCount, sp<ANativeWindow> anw, sp<EffectParameter> param)
{
    ANativeWindowBuffer* anb;
    sp<IImageBuffer> imageBuffer;
    status_t res;
    static frame_no oFrameNo = 0;
    for(int i=0; i<bufferCount; i++)
    {
        MY_LOGD_IF(mLogLevel, "[%s]: Dequeue buffer from %p", __FUNCTION__, anw.get());
        /************** for test, ap have to done this. ***************/
        // res = native_window_set_usage(anw.get(),
        // GRALLOC_USAGE_SW_WRITE_OFTEN);
        // MY_LOGD_IF(mLogLevel, "set_usage, result: %d", res);

        // int minUndequeuedBuffers;
        // res = anw.get()->query(anw.get(),
        //         NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
        //         &minUndequeuedBuffers);
        // MY_LOGD_IF(mLogLevel, "query, result: %d", res);
        /************** for test, ap have to done this. ***************/
        res = native_window_dequeue_buffer_and_wait(anw.get(), &anb);
        if (res != OK)
        {
            ALOGE("[%s]:  Unable to dequeue buffer: %s (%d) surfaceId:%d bufferIndex:%d",
                __FUNCTION__, strerror(-res), res, surfaceIdx, mOutputBufferInfo.size());
            return;
        }

        //MY_LOGD_IF(mLogLevel, "anb should != NULL, result=%d", int(anb != NULL));
        sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));

        //convert to IImageBuffer
        convertGraphicBufferToIImageBuffer(buf, imageBuffer);
        MY_LOGD_IF(mLogLevel, "ImageBuffer format=(0x%x), dimensions: (w = %d, h = %d)",
                imageBuffer->getImgFormat(), imageBuffer->getImgSize().w, imageBuffer->getImgSize().h);
        MY_LOGD_IF(mLogLevel, "imageBuffer address=%p, GraphicBuffer address=%p, ANativeWindowBuffer=%p",
                imageBuffer.get(), buf.get(), anb);

        //cache buffer
        mOutputBufferInfo.push_back(new OutputBufferInfo(surfaceIdx, buf));

        // new add
        //init EffectFrameInfo object
        sp<EffectFrameInfo> frame = new EffectFrameInfo(mpEffectRequest->getRequestNo(),
                                        oFrameNo, frameCallback, (void*)this);
        //add surface-id and bufferIndex
        param->set("surfaceId", surfaceIdx);
        param->set("bufferIndex", mOutputBufferInfo.size()-1);
        param->set("vOut", 1);
        frame->setFrameParameter(param);
        frame->setFrameBuffer(imageBuffer);
        mpEffectRequest->vOutputFrameInfo.replaceValueFor(oFrameNo, frame);
        MY_LOGD_IF(mLogLevel, "Add output frame, frameNo=%" PRId64 "", oFrameNo);
        oFrameNo++;
        imageBuffer->lockBuf( "addOutputFrame", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );


        /*// int64_t timestamp=1234L;
        // MY_LOGD_IF(mLogLevel, "Set timestamp:%d to %p", timestamp, anw.get());
        // err = native_window_set_buffers_timestamp(anw.get(), timestamp);
        // MY_LOGD_IF(mLogLevel, "Set timestamp result:%d", err);


        //cache buffer
        //mOutputBuffers.push_back(buf);
        mOutputBufferInfo.push_back(new OutputBufferInfo(surfaceIdx, buf));

        //add surface-id and bufferIndex
        param->set("surfaceId", surfaceIdx);
        param->set("bufferIndex", mOutputBufferInfo.size()-1);
        imageBuffer->lockBuf( "addOutputFrame", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );


        res = mpEffect->addOutputFrame(imageBuffer, param);
        //imageBuffer->unlockBuf( "addOutputFrame");

        //MY_LOGD_IF(mLogLevel, "Queue buffer to %p", anw.get());
        //err = anw->queueBuffer(anw.get(), buf->getNativeBuffer(), -1);
        //mOutputBuffers.replaceAt(param->getInt("bufferIndex"));*/
    }
    mpEffect->updateEffectRequest(mpEffectRequest);
}


/******************************************************************************
 *
 ******************************************************************************/
void
EffectHalClient::
onBufferFrameAvailable(int idx)
{

    bool sync;
    status_t res;
    sp<BufferItemConsumer> consumer = mInputComsumer[idx];
    BufferItem imgBuffer;

    //acquire buffer
    res = consumer->acquireBuffer(&imgBuffer, 0);
    MY_LOGD_IF(mLogLevel, "[%s]: [%s] acquireBuffer, result=%d, itemConsumer=%p ", __FUNCTION__, mEffectName.string(), res, consumer.get());
    if (res != OK)
    {
        if (res == BufferItemConsumer::NO_BUFFER_AVAILABLE)
        {
            MY_LOGE("%s: NO_BUFFER_AVAILABLE: %s (%d)", __FUNCTION__, strerror(-res), res);
        }
        return;
    }

    //save buffer according to timestamp
    //mBufferMap.replaceValueFor(imgBuffer.mTimestamp, imgBuffer);
    mInputBufferInfo.replaceValueFor(imgBuffer.mTimestamp, new InputBufferInfo(idx, imgBuffer));

    MY_LOGD_IF(mLogLevel, "handle=%p, dimensions: (w = %d, h = %d), Stride=%d, usage=%d, PixelFormat=%d, timestamp=%" PRId64 "",
            imgBuffer.mGraphicBuffer->handle, imgBuffer.mGraphicBuffer->getWidth(), imgBuffer.mGraphicBuffer->getHeight(),
            imgBuffer.mGraphicBuffer->getStride(), imgBuffer.mGraphicBuffer->getUsage(),
            imgBuffer.mGraphicBuffer->getPixelFormat(), imgBuffer.mTimestamp );

    //convert to image buffer
    sp<IImageBuffer> imageBuffer;
    convertGraphicBufferToIImageBuffer(imgBuffer.mGraphicBuffer, imageBuffer);
    MY_LOGD_IF(mLogLevel, "[%s]: imageBuffer dimensions: (w = %d, h = %d), format:(0x%x), plane(%zu)", __FUNCTION__,
            imageBuffer->getImgSize().w, imageBuffer->getImgSize().h,
            imageBuffer->getImgFormat(), imageBuffer->getPlaneCount());
    MY_LOGD_IF(mLogLevel, "[%s]: eImgFmt_JPEG=(0x%x), eImgFmt_BLOB=(0x%x), eImgFmt_RGBA8888=(0x%x), eImgFmt_YV12=(0x%x)",
            __FUNCTION__, eImgFmt_JPEG, eImgFmt_BLOB, eImgFmt_RGBA8888, eImgFmt_YV12);

    //check sync mode
    sync = getInputsyncMode(idx);
    MY_LOGD_IF(mLogLevel, "[%s]: index=%d, mInputBufferInfo size=%d, sync mode=%d", __FUNCTION__, idx, mInputBufferInfo.size(), int(sync));


    imageBuffer->lockBuf("addInputFrame", eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );


    // MBOOL ret = imageBuffer->saveToFile(String8::format("/sdcard/%s.yuv", mEffectName.string()));
    // MY_LOGD_IF(1, "[%s] save iamge end ret=%d ", __FUNCTION__, ret);


    //new
    if(mpEffectRequest->vInputFrameInfo.indexOfKey(imgBuffer.mTimestamp) < 0)
    {
        //init EffectFrameInfo object
        sp<EffectFrameInfo> frame = new EffectFrameInfo(mpEffectRequest->getRequestNo(),
                                        imgBuffer.mTimestamp, frameCallback, (void*)this);
        frame->setFrameBuffer(imageBuffer);
        sp<EffectParameter> parameter= new EffectParameter();
        parameter->setInt64("timestamp", imgBuffer.mTimestamp);
        parameter->set("surfaceId", idx);
        parameter->set("vIn", 1);
        frame->setFrameParameter(parameter);
        MY_LOGD_IF(mLogLevel, "Add input frame, frameNo=%" PRId64 "", imgBuffer.mTimestamp);
        mpEffectRequest->vInputFrameInfo.replaceValueFor(imgBuffer.mTimestamp, frame);
    }
    else
    {
        sp<EffectFrameInfo> frame = mpEffectRequest->vInputFrameInfo.editValueFor(imgBuffer.mTimestamp);
        frame->setFrameBuffer(imageBuffer);
        sp<EffectParameter> parameter = frame->getFrameParameter();
        parameter->setInt64("timestamp", imgBuffer.mTimestamp);
        parameter->set("surfaceId", idx);
        parameter->set("vIn", 1);
        frame->setFrameParameter(parameter);
        //do update
        mpEffect->updateEffectRequest(mpEffectRequest);
        return;
    }
    if (!sync)
    {
        MY_LOGD_IF(mLogLevel, "[%s]: do updateEffectRequest, vIn size=%d, vOut size=%d, ts=%" PRId64 "",
            __FUNCTION__, mpEffectRequest->vInputFrameInfo.size(), mpEffectRequest->vOutputFrameInfo.size(), imgBuffer.mTimestamp);
        mpEffect->updateEffectRequest(mpEffectRequest);
        return;
    }
    //new end


    // if (sync)
    // {
    //     if(mInputFrameInfo.indexOfKey(imgBuffer.mTimestamp) < 0)
    //     {
    //         MY_LOGD_IF(mLogLevel, "[%s]:Frame parmaeter not ready yet. Save buffer info.(surface index=%d)", __FUNCTION__, idx);
    //     }
    //     else
    //     {
    //         sp<FrameInfo> frameInfo = mInputFrameInfo.valueFor(imgBuffer.mTimestamp);
    //         MY_LOGD_IF(mLogLevel, "[%s]: matched: index=%d, ts=%" PRId64 "", __FUNCTION__, frameInfo->mIndex,
    //                                                                 frameInfo->mTimestamp);
    //         sp<EffectParameter> parameter = frameInfo->mEffectParam;
    //         if (parameter == 0)
    //         {
    //             parameter = new EffectParameter();
    //         }
    //         parameter->setInt64("timestamp", imgBuffer.mTimestamp);
    //         parameter->set("surfaceId", idx);
    //         res = mpEffect->addInputFrame(imageBuffer, frameInfo->mEffectParam);
    //     }
    // }
    // else
    // {
    //     //call directly
    //     sp<EffectParameter> parameter= new EffectParameter();
    //     parameter->setInt64("timestamp", imgBuffer.mTimestamp);
    //     parameter->set("surfaceId", idx);
    //     res = mpEffect->addInputFrame(imageBuffer, parameter);
    // }
    // //imageBuffer->unlockBuf("addInputFrame");
    // return;
}


/******************************************************************************
 * for test
 ******************************************************************************/
void
EffectHalClient::
acquireBufferManual()
{
    MY_LOGD_IF(mLogLevel, "[%s]: index=%d", __FUNCTION__, availableBufferIdx);
    onBufferFrameAvailable(availableBufferIdx);
    availableBufferIdx = 0;
}

/******************************************************************************
 * for test
 * reference from cpuComsumer_test
 ******************************************************************************/
void
EffectHalClient::configureANW(const sp<ANativeWindow>& anw,
        const CpuConsumerTestParams& params,
        int maxBufferSlack) {
    status_t err;
    err = native_window_set_buffers_geometry(anw.get(),
            params.width, params.height, params.format);
    MY_LOGD_IF(mLogLevel, "set_buffers_geometry, result: %d", err);

    err = native_window_set_usage(anw.get(),
            GRALLOC_USAGE_SW_WRITE_OFTEN);
    MY_LOGD_IF(mLogLevel, "set_usage, result: %d", err);

    int minUndequeuedBuffers;
    err = anw.get()->query(anw.get(),
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
            &minUndequeuedBuffers);
    MY_LOGD_IF(mLogLevel, "query, result: %d", err);

    MY_LOGD_IF(mLogLevel, "Setting buffer count to %d",
            maxBufferSlack + 1 + minUndequeuedBuffers);
    err = native_window_set_buffer_count(anw.get(),
            maxBufferSlack + 1 + minUndequeuedBuffers);
    MY_LOGD_IF(mLogLevel, "set_buffer_count, result: %d", err);
}


/******************************************************************************
 * for test
 * reference from cpuComsumer_test
 ******************************************************************************/
void
EffectHalClient::produceOneFrame(const sp<ANativeWindow>& anw,
        const CpuConsumerTestParams& params,
        int64_t timestamp, uint32_t *stride) {
    status_t err;
    ANativeWindowBuffer* anb;
    MY_LOGD_IF(mLogLevel, "Dequeue buffer from %p", anw.get());
    err = native_window_dequeue_buffer_and_wait(anw.get(), &anb);

    MY_LOGD_IF(mLogLevel, "anb should != NULL, result=%d", int(anb != NULL));

    sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));

    *stride = buf->getStride();
    uint8_t* img = NULL;

    MY_LOGD_IF(mLogLevel, "Lock buffer from %p for write", anw.get());
    err = buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&img));

    //do something

    MY_LOGD_IF(mLogLevel, "Unlock buffer from %p", anw.get());
    err = buf->unlock();


    MY_LOGD_IF(mLogLevel, "Set timestamp:%" PRId64 " to %p", timestamp, anw.get());
    err = native_window_set_buffers_timestamp(anw.get(), timestamp);
     MY_LOGD_IF(mLogLevel, "Set timestamp result:%d", err);


    MY_LOGD_IF(mLogLevel, "Queue buffer to %p", anw.get());
    err = anw->queueBuffer(anw.get(), buf->getNativeBuffer(), -1);
}


/******************************************************************************
 * for test
 * reference from cpuComsumer_test
 ******************************************************************************/
void
EffectHalClient::
dequeueAndQueueBuf(int64_t timestamp)
{
    status_t err;
    sp<Surface> s = new Surface(mProducer);
    sp<ANativeWindow> anw(s);

    CpuConsumerTestParams params;
    params.height = 1080;
    params.width = 720;
    params.maxLockedBuffers = 5;
    params.format = HAL_PIXEL_FORMAT_Y8;

    int res;
    res = native_window_api_connect(anw.get(), NATIVE_WINDOW_API_CAMERA);
    MY_LOGD_IF(mLogLevel, "connect=%d, native window is: %p", res, anw.get());

    configureANW(anw, params, 5);
    // Produce

    //const int64_t time = 12345678L;
    uint32_t stride;
    produceOneFrame(anw, params, timestamp, &stride);
}



/******************************************************************************
 * API for IEffectListener
 ******************************************************************************/
void
EffectHalClient::
onPrepared(const IEffectHalClient* effectClient, const EffectResult& result) const
{
    FUNCTION_LOG_START;
    mEffectListener->onPrepared(effectClient, result);
    FUNCTION_LOG_END;
}

void
EffectHalClient::
onInputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult)
{
    FUNCTION_LOG_START;
    mEffectListener->onInputFrameProcessed(this, parameter, partialResult);
    if (parameter == 0)
    {
        MY_LOGE("[%s]: EffectParameter is null", __FUNCTION__);
        return;
    }

    int64_t timestamp = parameter->getInt64("timestamp");
    //if(mBufferMap.indexOfKey(timestamp) < 0)
    if(mInputBufferInfo.indexOfKey(timestamp) < 0)
    {
        MY_LOGE("[%s]: imgBuffer not found, timestamp=%" PRId64 " ", __FUNCTION__, timestamp);
        return;
    }
    //BufferItemConsumer::BufferItem imgBuffer = mBufferMap.valueFor(timestamp);
    BufferItem imgBuffer = mInputBufferInfo.valueFor(timestamp)->mBufferItem;
    int idx = parameter->getInt("surfaceId");
    if (idx < 0)
    {
        MY_LOGE("[%s]: Error : parameter's surfaceId < 0!!", __FUNCTION__);
        return;
    }
    sp<BufferItemConsumer> consumer = mInputComsumer[idx];
    MY_LOGD_IF(mLogLevel, "[%s]: surface-id:%d ", __FUNCTION__, parameter->getInt("surfaceId"));
    consumer->releaseBuffer(imgBuffer);

    //mInputBufferInfo.removeItem(timestamp);
    //res = mConsumer->releaseBuffer(bufferItem, releaseFence);
    //mRecordingConsumer->releaseBuffer(imgBuffer);
    FUNCTION_LOG_END;
}


void
EffectHalClient::
onOutputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult)
{
    FUNCTION_LOG_START;
    status_t res;


    if (parameter == 0)
    {
        MY_LOGE("[%s]: EffectParameter is null", __FUNCTION__);
        return;
    }

    //get surface index and buffer index
    int idx = parameter->getInt("surfaceId");
    int bufferIndex = parameter->getInt("bufferIndex");
    MY_LOGD_IF(mLogLevel, "[%s]: surface-id:%d bufferIndex:%d", __FUNCTION__, idx, bufferIndex);

    if (idx < 0 || bufferIndex < 0)
    {
        MY_LOGE("[%s]: Error : Invalid surfaceID or bufferIndex!!", __FUNCTION__);
        return;
    }
    //set timestamp
    sp<ANativeWindow> anw = mOutputSurfaces[idx];
    int64_t timestamp = 123456L;
    MY_LOGD_IF(mLogLevel, "[%s]: Set timestamp=%" PRId64 " to %p", __FUNCTION__, timestamp, anw.get());
    res = native_window_set_buffers_timestamp(anw.get(), timestamp);
    if (res != OK)
    {
        ALOGE("[%s]: Error setting timestamp: %s (%d)", __FUNCTION__, strerror(-res), res);
        return;
    }

    //for test, convert to image buffer and dump it.
    //sp<GraphicBuffer> buf = mOutputBuffers[bufferIndex];
    sp<GraphicBuffer> buf = mOutputBufferInfo[bufferIndex]->pImageBuffer;
    MY_LOGD_IF(mLogLevel, "[%s]: Queue buffer to %p, GraphicBuffer address:%p, ANativeWindowBuffer address:%p",
            __FUNCTION__, anw.get(), buf.get(), buf->getNativeBuffer());


    sp<IImageBuffer> imageBuffer;
    convertGraphicBufferToIImageBuffer(buf, imageBuffer);
    MY_LOGD_IF(mLogLevel, "[%s]: ImageBuffer format=(0x%x), dimensions: (w = %d, h = %d)", __FUNCTION__,
            imageBuffer->getImgFormat(), imageBuffer->getImgSize().w, imageBuffer->getImgSize().h);
    MY_LOGD_IF(mLogLevel, "imageBuffer address=%p, GraphicBuffer address=%p", imageBuffer.get(), buf.get());

    //save debug image
    /*imageBuffer->lockBuf( String8::format("%s-%d-%d", __FUNCTION__, idx, bufferIndex),
        eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );

    MBOOL ret = imageBuffer->saveToFile(String8::format("/sdcard/output-%s.jpg", mEffectName.string()));
    MY_LOGD_IF(mLogLevel, "[%s] save iamge end ret=%d ", __FUNCTION__, ret);
    imageBuffer->unlockBuf(String8::format("%s-%d-%d", __FUNCTION__, idx, bufferIndex));*/


    //queue buffer to bufferQueue
    res = anw->queueBuffer(anw.get(), buf->getNativeBuffer(), -1);
    mOutputBufferInfo.replaceAt(bufferIndex);
    if (res != OK)
    {
        MY_LOGE("%s: Unable to queue output frame: %s (%d)", __FUNCTION__, strerror(-res), res);
        return;
    }

    buf = NULL;
    //--------------------------------
    res = native_window_api_disconnect(anw.get(), NATIVE_WINDOW_API_CAMERA);

    /**
     * This is not an error. if client calling process dies, the window will
     * also die and all calls to it will return DEAD_OBJECT, thus it's already
     * "disconnected"
     */
    if (res == DEAD_OBJECT) {
        ALOGW("%s: While disconnecting stream from native window, the"
                " native window died from under us", __FUNCTION__);
    }
    else if (res != OK) {
        ALOGE("%s: Unable to disconnect stream from native window "
              "(error %d %s)", __FUNCTION__, res, strerror(-res));
    }
    //--------------------------------


    //on event
    mEffectListener->onOutputFrameProcessed(this, parameter, partialResult);
    FUNCTION_LOG_END;
}

void
EffectHalClient::
onCompleted(const IEffectHalClient* effectClient, const EffectResult& partialResult, uint64_t uid) const
{
    FUNCTION_LOG_START;
    mEffectListener->onCompleted(effectClient, partialResult, uid);
    FUNCTION_LOG_END;
}

void
EffectHalClient::
onAborted(const IEffectHalClient* effectClient, const EffectResult& result)
{
    FUNCTION_LOG_START;

    //free input buffers
    for(size_t i=0; i<mInputBufferInfo.size(); i++)
    {
        if (mInputBufferInfo.valueAt(i) == NULL)
        {
            MY_LOGD_IF(mLogLevel, "input buffer already free, idx=%d", i);
            continue;
        }
        MY_LOGD_IF(mLogLevel, "free input buffer, idx=%d, surfaceId=%d", i, mInputBufferInfo.valueAt(i)->mSurfaceId);
        sp<BufferItemConsumer> consumer = mInputComsumer[mInputBufferInfo.valueAt(i)->mSurfaceId];
        consumer->releaseBuffer(mInputBufferInfo.valueAt(i)->mBufferItem);
    }
    mInputBufferInfo.clear();

    //free output buffers
    for(size_t i=0; i<mOutputBufferInfo.size(); i++)
    {
        if (mOutputBufferInfo[i] == NULL)
        {
            MY_LOGD_IF(mLogLevel, "output buffer already free, idx=%d", i);
            continue;
        }
        MY_LOGD_IF(mLogLevel, "free output buffer, idx=%d, surfaceId=%d", i, mOutputBufferInfo[i]->mSurfaceId);
        sp<ANativeWindow> anw = mOutputSurfaces[mOutputBufferInfo[i]->mSurfaceId];
        sp<GraphicBuffer> buf = mOutputBufferInfo[i]->pImageBuffer;
        status_t res = anw->cancelBuffer(anw.get(), buf->getNativeBuffer(), -1);
        if (res != OK)
        {
            MY_LOGE("%s: Unable to cancelBuffer mSurfaceId:%d, bufferIdx:%d, %s (%d)",
                __FUNCTION__, mOutputBufferInfo[i]->mSurfaceId, i, strerror(-res), res);
        }
    }
    mOutputBufferInfo.clear();


/*
    MY_LOGD("%s: clean all variable, size: I/O Surface(%d,%d), I/O FrameInfo(%d,%d), I/O Buffer(%d,%d), Listener(%d), mOutputEffectParams(%d), mBufferMap(%d), mOutputBuffers(%d)",
        __FUNCTION__, mOutputSurfaces.size(), mInputComsumer.size(),
        mInputFrameInfo.size(), mOutputFrameInfo.size(),
        mInputBufferInfo.size(), mOutputBufferInfo.size(),
        mListener.size(),
        mOutputEffectParams.size(), mBufferMap.size(), mOutputBuffers.size());

    //mpEffect = NULL;
    mOutputSurfaces.clear();
    mInputComsumer.clear();
    mInputFrameInfo.clear();
    mOutputFrameInfo.clear();
    mInputBufferInfo.clear();
    mOutputBufferInfo.clear();
    mListener.clear();
    mSurfaceMap.clear();
    mInputSyncMode.clear();
    mOutputSyncMode.clear();
    mOutputEffectParams.clear();
    mBufferMap.clear();
    mOutputBuffers.clear();
    mpEffectRequest = NULL;*/


    //init EffectRequest
    sp<EffectRequest> newRequest = new EffectRequest(mSessionUid, requestCallback, (void*)this);
    newRequest->vInputFrameInfo.clear();
    newRequest->vInputFrameInfo.setCapacity(mpEffectRequest->vInputFrameInfo.size());
    newRequest->vOutputFrameInfo.clear();
    newRequest->vOutputFrameInfo.setCapacity(mpEffectRequest->vOutputFrameInfo.size());
    mpEffectRequest = newRequest;


    mEffectListener->onAborted(effectClient, result);
    FUNCTION_LOG_END;
}

void
EffectHalClient::
onFailed(const IEffectHalClient* effectClient, const EffectResult& result) const
{
    FUNCTION_LOG_START;
    mEffectListener->onFailed(effectClient, result);
    FUNCTION_LOG_END;
}
