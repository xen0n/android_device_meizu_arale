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
#include <mmutils/include/IGrallocHelper.h>
#include <mtkcam/metadata/mtk_metadata_types.h>
#include <mtkcam/metadata/client/TagMap.h>
#include "MyUtils.h"
#include "AppStreamMgr.h"
//
#include <sys/prctl.h>
#include <cutils/properties.h>
#include <mtkcam/Trace.h>
//
using namespace android;
using namespace mmutils;
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
namespace
{
class MetadataConverterFactory
{
private:
    sp<IMetadataConverter>  mMetadataConverter;
    Mutex                   mMetadataConverterLock;

public:
    sp<IMetadataConverter>
    getMetadataConverter()
    {
        Mutex::Autolock _l(mMetadataConverterLock);
        if  ( mMetadataConverter == 0 ) {
            mMetadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
            MY_LOGE_IF(mMetadataConverter==0, "IMetadataConverter::createInstance");
        }
        //
        return mMetadataConverter;
    }

}   gMetadataConverterFactory;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::
destroy()
{
    requestExit();
    join();
    //
    if ( mMetadata ) {
        ::free_camera_metadata(mMetadata);
        mMetadata = NULL;
    }
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
AppStreamMgr::
readyToRun()
{
    ::prctl(PR_SET_NAME,(unsigned long)"Camera@AppMgr", 0, 0, 0);

#warning "[TODO] should set thread priority & policy"

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
AppStreamMgr::
requestExit()
{
    MY_LOGD("+");
    //
    {
        Mutex::Autolock _l(mResultQueueLock);
        Thread::requestExit();
        mResultQueueCond.broadcast();
    }
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
AppStreamMgr::
threadLoop()
{
    ResultQueueT vResult;
    MERROR err = dequeResult(vResult);
    if  ( OK == err && ! vResult.isEmpty() )
    {
        handleResult(vResult);
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::
AppStreamMgr(
    MINT32 openId,
    camera3_callback_ops const* callback_ops,
    IMetadataProvider const* pMetadataProvider
)
    : mOpenId(openId)
    , mpCallbackOps(callback_ops)
    //
    , mpMetadataProvider(pMetadataProvider)
    , mpMetadataConverter(gMetadataConverterFactory.getMetadataConverter())
    , mMetadata(NULL)
    //
    , mResultQueueLock()
    , mResultQueueCond()
    , mResultQueue()
    //
    , mFrameHandler(new FrameHandler(pMetadataProvider))
    , mFrameHandlerLock()
    , mFrameHandlerCond()
    , mStreamIdToConfig(eSTREAMID_BEGIN_OF_APP)
    , mLatestSettings()
    //
    , mAvgTimestampDuration(0)
    , mAvgCallbackDuration(0)
    , mAvgTimestampFps(0)
    , mAvgCallbackFps(0)
    , mFrameCounter(0)
    , mMaxFrameCount(33)
    , mTimestamp(0)
    , mCallbackTime(0)
{
    IMetadata::IEntry const& entry = mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_REQUEST_PARTIAL_RESULT_COUNT);
    if  ( entry.isEmpty() ) {
        MY_LOGE("no static REQUEST_PARTIAL_RESULT_COUNT");
        mAtMostMetaStreamCount = 1;
    }
    else {
        mAtMostMetaStreamCount = entry.itemAt(0, Type2Type<MINT32>());
    }
    //
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("debug.camera.log.AppStreamMgr", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
waitUntilDrained(nsecs_t const timeout)
{
    nsecs_t const startTime = ::systemTime();
    //
    Mutex::Autolock _l(mFrameHandlerLock);
    //
    while ( ! mFrameHandler->isEmptyFrameQueue() )
    {
        nsecs_t const elapsedInterval = (::systemTime() - startTime);
        nsecs_t const timeoutToWait = (timeout > elapsedInterval)
                                    ? (timeout - elapsedInterval)
                                    :   0
                                    ;
        MERROR err = mFrameHandlerCond.waitRelative(mFrameHandlerLock, timeoutToWait);
        if  ( OK != err ) {
            MY_LOGW(
                "FrameQueue#:%zu timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
                mFrameHandler->getFrameQueueSize(), timeout, (::systemTime() - startTime), err, ::strerror(-err)
            );
            mFrameHandler->dump();
            return err;
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
queryOldestRequestNumber(MUINT32 &ReqNo)
{
    return mFrameHandler->queryOldestRequestNumber(ReqNo);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
checkStream(camera3_stream* stream) const
{
    if  ( ! stream ) {
        MY_LOGE("NULL stream");
        return -EINVAL;
    }
    //
    /**
     * Return values:
     *
     *  0:      On successful stream configuration
     *
     * -EINVAL: If the requested stream configuration is invalid. Some examples
     *          of invalid stream configurations include:
     *
     *          - Including streams with unsupported formats, or an unsupported
     *            size for that format.
     */

    //
    IMetadata::IEntry const& entryScaler =
      mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);

    if ( entryScaler.isEmpty() )
    {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS");
        return -EINVAL;
    }

    // android.scaler.availableStreamConfigurations
    // int32 x n x 4
    MUINT i;
    for ( i = 0; i < entryScaler.count(); i += 4 )
    {
        if ( entryScaler.itemAt(i, Type2Type< MINT32 >()) != stream->format )
        {
            continue;
        }
        MUINT32 scalerWidth  = entryScaler.itemAt(i + 1, Type2Type< MINT32 >());
        MUINT32 scalerHeight = entryScaler.itemAt(i + 2, Type2Type< MINT32 >());

        if ( stream->width == scalerWidth && stream->height == scalerHeight )
        {
            return OK;
        }
    }

    MY_LOGE("unsupported size w:%d h:%d for format %d", stream->width, stream->height, stream->format);
    return -EINVAL;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
checkStreams(camera3_stream_configuration_t *stream_list) const
{
    if  ( ! stream_list ) {
        MY_LOGE("NULL stream_list");
        return  -EINVAL;
    }
    //
    if  ( ! stream_list->streams ) {
        MY_LOGE("NULL stream_list->streams");
        return  -EINVAL;
    }
    //
    if  ( 0 == stream_list->num_streams ) {
        MY_LOGE("stream_list->num_streams = 0");
        return  -EINVAL;
    }
    //
    //
    KeyedVector<int, size_t> typeNum;
    typeNum.add(CAMERA3_STREAM_OUTPUT,          0);
    typeNum.add(CAMERA3_STREAM_INPUT,           0);
    typeNum.add(CAMERA3_STREAM_BIDIRECTIONAL,   0);
    for (size_t i = 0; i < stream_list->num_streams; i++) {
        camera3_stream* stream = stream_list->streams[i];
        //
        MERROR err = checkStream(stream);
        if  ( OK != err ) {
            MY_LOGE("streams[%zu] has a bad status: %d(%s)", i, err, ::strerror(-err));
            return err;
        }
        //
        typeNum.editValueFor(stream->stream_type)++;
    }

    /**
     * At most one input-capable stream may be defined (INPUT or BIDIRECTIONAL)
     * in a single configuration.
     *
     * At least one output-capable stream must be defined (OUTPUT or
     * BIDIRECTIONAL).
     */
    /*
     *
     * Return values:
     *
     *  0:      On successful stream configuration
     *
     * -EINVAL: If the requested stream configuration is invalid. Some examples
     *          of invalid stream configurations include:
     *
     *          - Including more than 1 input-capable stream (INPUT or
     *            BIDIRECTIONAL)
     *
     *          - Not including any output-capable streams (OUTPUT or
     *            BIDIRECTIONAL)
     *
     *          - Including too many output streams of a certain format.
     */
    size_t const num_stream_O  = typeNum[CAMERA3_STREAM_OUTPUT];
    size_t const num_stream_I  = typeNum[CAMERA3_STREAM_INPUT];
    size_t const num_stream_IO = typeNum[CAMERA3_STREAM_BIDIRECTIONAL];

    if  (
            (num_stream_I + num_stream_IO) > 1
        ||  (num_stream_O + num_stream_IO) == 0
        )
    {
        MY_LOGE(
            "bad stream count: (out, in, in-out)=(%zu, %zu, %zu)",
            num_stream_O, num_stream_I, num_stream_IO
        );
        return  -EINVAL;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
configureStreams(
    camera3_stream_configuration_t *stream_list
)
{
    MERROR err = OK;
    //
    err = checkStreams(stream_list);
    if  ( OK != err ) {
        return err;
    }
    //
    Mutex::Autolock _l(mFrameHandlerLock);
    //
    {
        StreamId_T streamId = mStreamIdToConfig++;
        mFrameHandler->addConfigStream(createMetaStreamInfo(streamId));
    }
    //
    for (size_t i = 0; i < stream_list->num_streams; i++)
    {
        StreamId_T streamId = mStreamIdToConfig++;
        mFrameHandler->addConfigStream(createImageStreamInfo(streamId, stream_list->streams[i]));
    }
    //
    //  An emtpy settings buffer cannot be used as the first submitted request
    //  after a configure_streams() call.
    mLatestSettings.clear();
    //
    {
        status_t status = run();
        if  ( OK != status ) {
            MY_LOGE("Fail to run the thread - status:%d(%s)", status, ::strerror(-status));
            return -ENODEV;
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::AppImageStreamInfo*
AppStreamMgr::
createImageStreamInfo(
    StreamId_T suggestedStreamId,
    camera3_stream* stream
)   const
{
    MERROR err = OK;
    //
    MINT  formatToAllocate  = stream->format;
    MUINT usageForConsumer  = stream->usage;
    MUINT usageForAllocator = usageForConsumer
                            |(GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN)
                            |(GRALLOC_USAGE_HW_CAMERA_READ|GRALLOC_USAGE_HW_CAMERA_WRITE)
//                            |(GRALLOC_USAGE_HW_TEXTURE|GRALLOC_USAGE_HW_RENDER)
                            ;
    //
    //
    IGrallocHelper* pGrallocHelper = IGrallocHelper::singleton();
    //
    GrallocStaticInfo   grallocStaticInfo;
    GrallocRequest      grallocRequest;
    grallocRequest.usage    = usageForAllocator;
    grallocRequest.format   = formatToAllocate;
    if  ( HAL_PIXEL_FORMAT_BLOB == formatToAllocate ) {
        IMetadata::IEntry const& entry = mpMetadataProvider->geMtktStaticCharacteristics().entryFor(MTK_JPEG_MAX_SIZE);
        if  ( entry.isEmpty() ) {
            MY_LOGE("no static JPEG_MAX_SIZE");
            grallocRequest.widthInPixels = stream->width * stream->height * 2;
        }
        else {
            grallocRequest.widthInPixels = entry.itemAt(0, Type2Type<MINT32>());
        }
        grallocRequest.heightInPixels   = 1;
    }
    else {
        grallocRequest.widthInPixels    = stream->width;
        grallocRequest.heightInPixels   = stream->height;
    }
    //
    err = pGrallocHelper->query(&grallocRequest, &grallocStaticInfo);
    if  ( OK != err ) {
        MY_LOGE("IGrallocHelper::query - err:%d(%s)", err, ::strerror(-err));
        return NULL;
    }
    //
    String8 const s8FormatToAllocate = pGrallocHelper->queryPixelFormatName(formatToAllocate);
    String8 const s8FormatAllocated  = pGrallocHelper->queryPixelFormatName(grallocStaticInfo.format);
    String8 const s8UsageForConsumer = pGrallocHelper->queryGrallocUsageName(usageForConsumer);
    String8 const s8UsageForAllocator= pGrallocHelper->queryGrallocUsageName(usageForAllocator);
    //
    StreamId_T streamId = suggestedStreamId;
    String8 s8StreamName("Image:App:");
    //
    if  ( 0 != (usageForConsumer & GRALLOC_USAGE_HW_VIDEO_ENCODER) )
    {
        s8StreamName += pGrallocHelper->queryGrallocUsageName(GRALLOC_USAGE_HW_VIDEO_ENCODER);
    }
    else
    {
        switch  (grallocStaticInfo.format)
        {
        case HAL_PIXEL_FORMAT_BLOB:
            s8StreamName += "JPEG-BLOB";
            break;
            //
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        case HAL_PIXEL_FORMAT_RAW16:
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            s8StreamName += s8FormatAllocated;
            break;
        default:
            MY_LOGE("Unsupported format:0x%x(%s)", grallocStaticInfo.format, s8FormatAllocated.string());
            return NULL;
        }
    }
    //
    String8 s8Planes;
    IImageStreamInfo::BufPlanes_t bufPlanes;
    bufPlanes.resize(grallocStaticInfo.planes.size());
    for (size_t i = 0; i < bufPlanes.size(); i++)
    {
        IImageStreamInfo::BufPlane& plane = bufPlanes.editItemAt(i);
        plane.sizeInBytes      = grallocStaticInfo.planes[i].sizeInBytes;
        plane.rowStrideInBytes = grallocStaticInfo.planes[i].rowStrideInBytes;
        //
        s8Planes += String8::format(" %zu/%zu", plane.rowStrideInBytes, plane.sizeInBytes);
    }
    //
    AppImageStreamInfo* pStream = NULL;
    pStream = new AppImageStreamInfo(
        stream,
        s8StreamName.string(),
        streamId,
        usageForConsumer,
        usageForAllocator,
        formatToAllocate,
        grallocStaticInfo.format,
        bufPlanes,
        0
    );
    //
    MY_LOGD_IF(
        1,
        "[%#" PRIxPTR " %s] stream:%p->%p %dx%d "
        "formatToAllocate:%#x(%s) formatAllocated:%#x(%s) "
        "Consumer-usage:%#x(%s) Allocator-usage:%#x(%s) "
        "rowStrideInBytes/sizeInBytes:%s",
        pStream->getStreamId(), pStream->getStreamName(), stream, pStream,
        pStream->getImgSize().w, pStream->getImgSize().h,
        formatToAllocate, s8FormatToAllocate.string(),
        grallocStaticInfo.format, s8FormatAllocated.string(),
        usageForConsumer, s8UsageForConsumer.string(),
        usageForAllocator, s8UsageForAllocator.string(),
        s8Planes.string()
    );
    return pStream;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::AppMetaStreamInfo*
AppStreamMgr::
createMetaStreamInfo(
    StreamId_T suggestedStreamId
)   const
{
    return new AppMetaStreamInfo(
        "Meta:App:Control",
        suggestedStreamId,
        eSTREAMTYPE_META_IN,
        0
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
queryConfiguredStreams(
    ConfigAppStreams& rStreams
)   const
{
    Mutex::Autolock _l(mFrameHandlerLock);
    return mFrameHandler->getConfigStreams(rStreams);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
checkRequestLocked(camera3_capture_request_t const* request) const
{
    if  ( NULL == request ) {
        MY_LOGE("NULL request");
        return  -EINVAL;
    }
    //
    //  there are 0 output buffers
    if  ( NULL == request->output_buffers || 0 == request->num_output_buffers ) {
        MY_LOGE("[frameNo:%u] output_buffers:%p num_output_buffers:%d", request->frame_number, request->output_buffers, request->num_output_buffers);
        return  -EINVAL;
    }
    //
    //  the settings are NULL when not allowed
    if  ( NULL == request->settings && mLatestSettings.isEmpty() ) {
        MY_LOGE("[frameNo:%u] NULL request settings; however most-recently submitted request is also NULL after configure_stream", request->frame_number);
        return  -EINVAL;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
createRequest(
    camera3_capture_request_t* request,
    Request& rRequest
)
{
    Mutex::Autolock _l(mFrameHandlerLock);
    //
    MERROR err = checkRequestLocked(request);
    if  ( OK != err ) {
        return err;
    }
    //
    rRequest.frameNo = request->frame_number;
    //
    //  vInputImageBuffers <- camera3_capture_request_t::input_buffer
    if  ( camera3_stream_buffer const* p_stream_buffer = request->input_buffer )
    {
        sp<AppImageStreamBuffer> pStreamBuffer = createImageStreamBuffer(p_stream_buffer);
        //
        rRequest.vInputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
    }
    //
    //  vOutputImageBuffers <- camera3_capture_request_t::output_buffers
    for (size_t i = 0; i < request->num_output_buffers; i++)
    {
        camera3_stream_buffer const* p_stream_buffer = &request->output_buffers[i];
        //
        sp<AppImageStreamBuffer> pStreamBuffer = createImageStreamBuffer(p_stream_buffer);
        //
        rRequest.vOutputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
    }
    //
    //  vInputMetaBuffers <- camera3_capture_request_t::settings
    {
        sp<IMetaStreamInfo> pStreamInfo = mFrameHandler->getConfigMetaStream(0);
        MBOOL isRepeating = MFALSE;
        //
        if  ( request->settings ) {
            isRepeating = MFALSE;
            mLatestSettings.clear();
            if  ( ! mpMetadataConverter->convert(request->settings, mLatestSettings) ) {
                MY_LOGE("frameNo:%u IMetadataConverter->convert", request->frame_number);
                return -ENODEV;
            }
            // to debug
            {
                if(mLogLevel >= 2){
                    mpMetadataConverter->dumpAll(mLatestSettings, request->frame_number);
                } else if(mLogLevel >= 1){
                    mpMetadataConverter->dump(mLatestSettings, request->frame_number);
                }
            }
        }
        else {
            /**
             * As a special case, a NULL settings buffer indicates that the
             * settings are identical to the most-recently submitted capture request. A
             * NULL buffer cannot be used as the first submitted request after a
             * configure_streams() call.
             */
            isRepeating = MTRUE;
            MY_LOGD_IF(
                mLogLevel >= 1,
                "frameNo:%u NULL settings -> most-recently submitted capture request", request->frame_number
            );
        }
        //
        sp<AppMetaStreamBuffer> pStreamBuffer = createMetaStreamBuffer(pStreamInfo, mLatestSettings, isRepeating);
        //
        rRequest.vInputMetaBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);

        if (!isRepeating) {

            camera_metadata_ro_entry e1;
            if (OK == find_camera_metadata_ro_entry(request->settings,
                    ANDROID_CONTROL_AF_TRIGGER, &e1) && *e1.data.u8 == ANDROID_CONTROL_AF_TRIGGER_START){
                CAM_TRACE_FMT_BEGIN("AF_state: %d", *e1.data.u8);
                MY_LOGD_IF(mLogLevel >= 1, "AF_state: %d", *e1.data.u8);
                CAM_TRACE_END();
            }

            camera_metadata_ro_entry e2;
            if (OK == find_camera_metadata_ro_entry(request->settings,
                    ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER, &e2) && *e2.data.u8 == ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_START){
                CAM_TRACE_FMT_BEGIN("ae precap: %d", *e2.data.u8);
                MY_LOGD_IF(mLogLevel >= 1, "ae precapture trigger: %d", *e2.data.u8);
                CAM_TRACE_END();
            }

            camera_metadata_ro_entry e4;
            if (OK == find_camera_metadata_ro_entry(request->settings,
                    ANDROID_CONTROL_CAPTURE_INTENT, &e4)) {
                CAM_TRACE_FMT_BEGIN("capture intent: %d", *e4.data.u8);
                MY_LOGD_IF(mLogLevel >= 1, "capture intent: %d", *e4.data.u8);
                CAM_TRACE_END();
            }
        }
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::AppImageStreamBuffer*
AppStreamMgr::
createImageStreamBuffer(
    camera3_stream_buffer const* buffer
)   const
{
    MY_LOGV(
        "stream:%p buffer:%p status:%d acquire_fence:%d release_fence:%d",
        buffer->stream, buffer->buffer, buffer->status, buffer->acquire_fence, buffer->release_fence
    );
    sp<IImageStreamInfo> pStreamInfo = AppImageStreamInfo::cast(buffer->stream);
    //
    sp<IGraphicImageBufferHeap>
    pImageBufferHeap =
    IGraphicImageBufferHeap::create(
        pStreamInfo->getStreamName(),
        buffer
    );
    //
    AppImageStreamBuffer*
    pStreamBuffer =
    AppImageStreamBuffer::Allocator(pStreamInfo.get())(pImageBufferHeap.get());
    //
    return pStreamBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::AppMetaStreamBuffer*
AppStreamMgr::
createMetaStreamBuffer(
    android::sp<IMetaStreamInfo> pStreamInfo,
    IMetadata const& rSettings,
    MBOOL const repeating
)   const
{
    AppMetaStreamBuffer*
    pStreamBuffer =
    AppMetaStreamBuffer::Allocator(pStreamInfo.get())(rSettings);
    //
    pStreamBuffer->setRepeating(repeating);
    //
    return pStreamBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
registerRequest(
    Request const& rRequest
)
{
    Mutex::Autolock _l(mFrameHandlerLock);
    return mFrameHandler->registerFrame(rRequest);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::
updateResult(
    MUINT32 const /*frameNo*/,
    MINTPTR const /*userId*/
)
{
    //enqueResult(frameNo, userId);
    MY_LOGE("Should not happen.");
}

MVOID
AppStreamMgr::
updateResult(
    MUINT32 const frameNo,
    MINTPTR const userId,
    Vector< sp<IMetaStreamBuffer> > resultMeta,
    bool hasLastPartial
)
{
    //mFrameHandler->updateResult(frameNo, resultMeta, hasLastPartial);
    enqueResult(frameNo, userId, resultMeta, hasLastPartial);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
enqueResult(
    MUINT32 const frameNo,
    MINTPTR const userId,
    Vector< sp<IMetaStreamBuffer> > resultMeta,
    bool hasLastPartial
)
{
    NSCam::Utils::CamProfile profile(__FUNCTION__, "AppStreamManager");
    Mutex::Autolock _l(mResultQueueLock);
    //
    if  ( exitPending() ) {
        MY_LOGW("Dead ResultQueue");
        return DEAD_OBJECT;
    }
    profile.print_overtime(1, "Mutex: frameNo:%u userId:%#" PRIxPTR, frameNo, userId);
    //
    ssize_t const index = mResultQueue.indexOfKey(frameNo);
    profile.print_overtime(1, "indexOf ResultQueue#:%zu frameNo:%u userId:%#" PRIxPTR, mResultQueue.size(), frameNo, userId);
    if  ( 0 <= index ) {
        MY_LOGW("frameNo:%u existed @ index:%zd", frameNo, index);
        //
        sp<ResultItem> pItem = mResultQueue.editValueAt(index);
        pItem->lastPartial   = hasLastPartial;
        pItem->buffer.appendVector(resultMeta);
        mResultQueueCond.broadcast();
    }
    else {
        //mResultQueue.add(frameNo);
        sp<ResultItem> pItem = new ResultItem;
        pItem->frameNo     = frameNo;
        pItem->buffer      = resultMeta;
        pItem->lastPartial = hasLastPartial;
        //
        mResultQueue.add(frameNo, pItem);
        mResultQueueCond.broadcast();
    }
    //
    profile.print_overtime(1, "- frameNo:%u userId:%#" PRIxPTR, frameNo, userId);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
AppStreamMgr::
dequeResult(
    ResultQueueT& rvResult
)
{
    MERROR err = OK;
    //
    Mutex::Autolock _l(mResultQueueLock);
    //
    while ( ! exitPending() && mResultQueue.isEmpty() )
    {
        err = mResultQueueCond.wait(mResultQueueLock);
        MY_LOGW_IF(
            OK != err,
            "exitPending:%d ResultQueue#:%zu err:%d(%s)",
            exitPending(), mResultQueue.size(), err, ::strerror(-err)
        );
    }
    //
    if  ( mResultQueue.isEmpty() )
    {
        MY_LOGD_IF(
            mLogLevel >= 1,
            "empty queue"
        );
        rvResult.clear();
        err = NOT_ENOUGH_DATA;
    }
    else
    {
        //  If the queue is not empty, deque all items from the queue.
        rvResult = mResultQueue;
        mResultQueue.clear();
        err = OK;
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::
handleResult(
    ResultQueueT const& rvResult
)
{
    List<CallbackParcel> cbList;
    //
    {
        Mutex::Autolock _l(mFrameHandlerLock);
        mFrameHandler->update(rvResult, cbList);
    }
    //
    {
        List<CallbackParcel>::iterator it = cbList.begin();
        while ( ! cbList.empty() ) {
            performCallback(*it);
            it = cbList.erase(it);
        }
    }
    //
    {
        Mutex::Autolock _l(mFrameHandlerLock);
        if  ( mFrameHandler->isEmptyFrameQueue() ) {
            mFrameHandlerCond.broadcast();
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
AppStreamMgr::
performCallback(CallbackParcel const& cbParcel)
{
    uint32_t const frame_number = cbParcel.frameNo;
    //
    String8 str = String8::format("frameNo:%u", frame_number);
    {
        if  ( cbParcel.shutter != 0 ) {
            str += String8::format(" shutter:%" PRId64, cbParcel.shutter->timestamp);
        }
        if  ( ! cbParcel.vError.isEmpty() ) {
            str += String8::format(" Error#:%zu", cbParcel.vError.size());
        }
        if  ( ! cbParcel.vOutputMetaItem.isEmpty() ) {
            str += String8::format(" O:Meta#:%zu", cbParcel.vOutputMetaItem.size());
        }
        if  ( ! cbParcel.vOutputImageItem.isEmpty() ) {
            str += String8::format(" O:Image#:%zu", cbParcel.vOutputImageItem.size());
        }
        if  ( ! cbParcel.vInputImageItem.isEmpty() ) {
            str += String8::format(" I:Image#:%zu", cbParcel.vInputImageItem.size());
        }
        MY_LOGD_IF(
            mLogLevel >= 1,
            "+ %s", str.string()
        );
    }
    //
    //CallbackParcel::shutter
    if  ( cbParcel.shutter != 0 )
    {
        // to debug
        {
            if(cbParcel.shutter->timestamp < mTimestamp ) {
                MY_LOGE(" #(%d), now shutter:%" PRId64 " last shutter:%" PRId64, frame_number, cbParcel.shutter->timestamp, mTimestamp);

            }
            mAvgTimestampDuration += cbParcel.shutter->timestamp - mTimestamp;
            mTimestamp = cbParcel.shutter->timestamp;
            if (mAvgTimestampFps == 0) { mAvgTimestampFps = cbParcel.shutter->timestamp; }
            if(mFrameCounter >= mMaxFrameCount) { mAvgTimestampFps = cbParcel.shutter->timestamp - mAvgTimestampFps; }
        }
        //
        camera3_notify_msg msg;
        msg.type = CAMERA3_MSG_SHUTTER;
        msg.message.shutter.frame_number = frame_number;
        msg.message.shutter.timestamp = cbParcel.shutter->timestamp;
        mpCallbackOps->notify(mpCallbackOps, &msg);
    }
    //
    //CallbackParcel::vOutputMetaItem
    for (size_t i = 0; i < cbParcel.vOutputMetaItem.size(); i++) {
        CallbackParcel::MetaItem const& rCbMetaItem = cbParcel.vOutputMetaItem[i];
        //
        IMetadata* pMetadata = rCbMetaItem.buffer->tryReadLock(LOG_TAG);
        {
            MBOOL ret = MTRUE;
#if 0
            size_t entryCount = 0, dataCount = 0;
            ret = mpMetadataConverter->get_data_count(*pMetadata, entryCount, dataCount);
            MY_LOGF_IF(!ret||(0==entryCount&&0==dataCount), "IMetadataConverter::get_data_count() - entryCount:%zu dataCount:%zu ret:%d", entryCount, dataCount, ret);
            //
            metadata = ::allocate_camera_metadata(entryCount, dataCount);
            MY_LOGF_IF(!metadata, "allocate_camera_metadata - entryCount:%zu dataCount:%zu", entryCount, dataCount);
            //
#endif
            ret = mpMetadataConverter->convertWithoutAllocate(*pMetadata, mMetadata);
            // to debug
            {
                if(mLogLevel >= 3){
                    mpMetadataConverter->dumpAll(*pMetadata, frame_number);
                } else if(mLogLevel >= 2){
                    mpMetadataConverter->dump(*pMetadata, frame_number);
                }
            }

            MY_LOGF_IF(!ret || !mMetadata, "fail to convert metadata:%p ret:%d", mMetadata, ret);
        }
        rCbMetaItem.buffer->unlock(LOG_TAG, pMetadata);
        //
        //
        camera3_capture_result const result =
        {
            frame_number        : frame_number,
            result              : mMetadata,
            num_output_buffers  : 0,
            output_buffers      : NULL,
            input_buffer        : NULL,
            partial_result      : rCbMetaItem.bufferNo,
        };
        // to debug
        {
            if (rCbMetaItem.bufferNo == mAtMostMetaStreamCount) {
                struct timespec ms;
                MUINT64 ms64 = static_cast<MUINT64>(systemTime(SYSTEM_TIME_PROCESS));
                //
                mAvgCallbackDuration += ms64 - mCallbackTime;
                mCallbackTime = ms64;
                //
                if (mFrameCounter == 0) { mAvgCallbackFps = ms64; }
                //
                mFrameCounter++;
                //
                if(mFrameCounter > mMaxFrameCount) { mAvgCallbackFps = ms64 - mAvgCallbackFps; }
            }
        }
        str += String8::format(" %s(partial#:%d)", rCbMetaItem.buffer->getName(), result.partial_result);
        mpCallbackOps->process_capture_result(mpCallbackOps, &result);
    }
    //
    //CallbackParcel::vError
    for (size_t i = 0; i < cbParcel.vError.size(); i++) {
        CallbackParcel::Error const& rError = cbParcel.vError[i];
        camera3_notify_msg msg;
        msg.type = CAMERA3_MSG_ERROR;
        msg.message.error.frame_number = frame_number;
        msg.message.error.error_stream = (rError.stream != 0) ? rError.stream->get_camera3_stream() : NULL;
        msg.message.error.error_code = rError.errorCode;
        str += String8::format(" error_code:%d", msg.message.error.error_code);
        mpCallbackOps->notify(mpCallbackOps, &msg);
    }
    //
    //CallbackParcel::vOutputImageItem
    //CallbackParcel::vInputImageItem
    if  ( ! cbParcel.vOutputImageItem.isEmpty() || ! cbParcel.vInputImageItem.empty() )
    {
        //Output
        Vector<camera3_stream_buffer_t> vOutBuffers;
        vOutBuffers.resize(cbParcel.vOutputImageItem.size());
        for (size_t i = 0; i < cbParcel.vOutputImageItem.size(); i++) {
            CallbackParcel::ImageItem const& rCbImageItem = cbParcel.vOutputImageItem[i];
            sp<IGraphicImageBufferHeap> pImageBufferHeap = rCbImageItem.buffer->getImageBufferHeap();
            //
            camera3_stream_buffer_t& rDst = vOutBuffers.editItemAt(i);
            rDst.stream = rCbImageItem.stream->get_camera3_stream();
            rDst.buffer = pImageBufferHeap->getBufferHandlePtr();
            rDst.status = rCbImageItem.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR)
                            ? CAMERA3_BUFFER_STATUS_ERROR
                            : CAMERA3_BUFFER_STATUS_OK
                            ;
            rDst.acquire_fence = rCbImageItem.buffer->getAcquireFence();
            rDst.release_fence = rCbImageItem.buffer->getReleaseFence();
            str += String8::format(" %s", rCbImageItem.buffer->getName());
#if 0
            if  ( HAL_PIXEL_FORMAT_YV12 == pImageBufferHeap->getImgFormat() )
            {
                sp<IImageBuffer> pBuffer = pImageBufferHeap->createImageBuffer();
                String8 s8DumpPath = String8::format("sdcard/DCIM/Camera/dump_%dx%d_yv12_%03d_.yuv", pBuffer->getImgSize().w, pBuffer->getImgSize().h, frame_number);
                pBuffer->saveToFile(s8DumpPath.string());
            }
#endif
            if  (
                    HAL_PIXEL_FORMAT_BLOB == rDst.stream->format
                &&  CAMERA3_BUFFER_STATUS_OK == rDst.status
                )
            {
                GrallocStaticInfo staticInfo;
                IGrallocHelper::singleton()->query(*rDst.buffer, &staticInfo, NULL);
                //
                if  ( pImageBufferHeap->lockBuf(LOG_TAG, GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_SW_READ_OFTEN) ) {

                    MINTPTR jpegBuf = pImageBufferHeap->getBufVA(0);
                    size_t jpegDataSize = pImageBufferHeap->getBitstreamSize();
                    size_t jpegBufSize = staticInfo.widthInPixels;

                    camera3_jpeg_blob* pTransport =
                    reinterpret_cast<camera3_jpeg_blob*>(
                        jpegBuf + jpegBufSize - sizeof(camera3_jpeg_blob)
                    );
                    pTransport->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
                    pTransport->jpeg_size = jpegDataSize;

                    pImageBufferHeap->unlockBuf(LOG_TAG);
                    MY_LOGD("jpegBuf:%#" PRIxPTR " bufsize:%d datasize:%d", jpegBuf, staticInfo.widthInPixels, jpegDataSize);
                }
                else {
                    MY_LOGE("Fail to lock jpeg");
                }
            }
        }
        //
        //Input
        Vector<camera3_stream_buffer_t> vInpBuffers;
        vInpBuffers.resize(cbParcel.vInputImageItem.size());
        for (size_t i = 0; i < cbParcel.vInputImageItem.size(); i++) {
            CallbackParcel::ImageItem const& rCbImageItem = cbParcel.vInputImageItem[i];
            sp<IGraphicImageBufferHeap> pImageBufferHeap = rCbImageItem.buffer->getImageBufferHeap();
            //
            camera3_stream_buffer_t& rDst = vInpBuffers.editItemAt(i);
            rDst.stream = rCbImageItem.stream->get_camera3_stream();
            rDst.buffer = pImageBufferHeap->getBufferHandlePtr();
            rDst.status = CAMERA3_BUFFER_STATUS_OK;
            rDst.acquire_fence = rCbImageItem.buffer->getAcquireFence();
            rDst.release_fence = rCbImageItem.buffer->getReleaseFence();
            str += String8::format(" %s", rCbImageItem.buffer->getName());
        }
        //
        camera3_capture_result const result =
        {
            frame_number        : frame_number,
            result              : NULL,
            num_output_buffers  : vOutBuffers.size(),
            output_buffers      : vOutBuffers.array(),
            input_buffer        : vInpBuffers.size() ? vInpBuffers.array() : NULL,
            partial_result      : 0,
        };
        mpCallbackOps->process_capture_result(mpCallbackOps, &result);
    }
    //
    // to debug
    {
        if(mFrameCounter > mMaxFrameCount) {
            mAvgTimestampDuration = mAvgTimestampDuration / mFrameCounter;
            String8 strDebugInfo = String8::format(" AvgTimestampDuration = %"PRId64, mAvgTimestampDuration);
            //
            mAvgCallbackDuration = (mAvgCallbackDuration / mFrameCounter);
            strDebugInfo += String8::format(" AvgCallbackDuration = %"PRId64, mAvgCallbackDuration);
            //
            if ( mAvgTimestampFps > 0 ) {
                mAvgTimestampFps = (mFrameCounter*10000000000) / mAvgTimestampFps;
                strDebugInfo += String8::format(" AvgTimestampFps = %"PRId64, mAvgTimestampFps);
            }
            if ( mAvgCallbackFps > 0 ) {
                mAvgCallbackFps = (mFrameCounter*10000000000) / mAvgCallbackFps;
                strDebugInfo += String8::format(" AvgCallbackFps = %"PRId64, mAvgCallbackFps);
            }
            //
            mAvgTimestampFps = 0;
            mAvgCallbackFps = 0;
            mAvgTimestampDuration = 0;
            mAvgCallbackDuration = 0;
            //
            mFrameCounter = 0;
            //
            MY_LOGD_IF(
                1,
                "%s", strDebugInfo.string()
            );
        }
    }
    //
    MY_LOGD_IF(
        mLogLevel >= 1,
        "- %s", str.string()
    );
}

