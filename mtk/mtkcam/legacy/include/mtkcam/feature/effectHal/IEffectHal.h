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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>
#include <utils/Singleton.h>
#include <utils/Flattenable.h>

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "mtkcam/IImageBuffer.h"
#include <gui/IGraphicBufferProducer.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/BufferItemConsumer.h>
#include <gui/IGraphicBufferProducer.h>


#include <IEffectListener.h>
#include <EffectRequest.h>
//get camera3_jpeg_blob
#include <hardware/camera3.h>

//for test
// #include <gui/CpuConsumer.h>
// #include <gui/Surface.h>
// #include <ui/GraphicBuffer.h>
// #include <utils/String8.h>
// #include <utils/Thread.h>
// #include <utils/Mutex.h>
// #include <utils/Condition.h>


//#include "mtkcam/sdk/libcore/MTKCoreType.h"

/******************************************************************************
 *
 ******************************************************************************/
/*
namespace android {
    class IGraphicBufferProducer;
   class IgraphicBufferConsumer;
    class BufferItemConsumer;
};
*/

namespace NSCam
{
using namespace android;

//@todo remove this
class IEffectHal;
class IEffectHalClient;
class IImageBuffer;
class IEffectListener;

typedef enum
{
    HDR_MODE,
    FB_MODE,
    MFB_MODE,
} EFFECT_SDK_HAL_MODE;


class EffectHalVersion : public LightFlattenable<EffectHalVersion>
{
public:    //LightFlattenable
    inline  bool        isFixedSize() const
    {
        return false;
    }
    size_t              getFlattenedSize() const;
    status_t   flatten(void* buffer, size_t size) const;
    status_t   unflatten(void const* buffer, size_t size);

private:
    static void flattenString8(void*& buffer, size_t& size, const String8& string8);
    static bool unflattenString8(void const*& buffer, size_t& size, String8& outputString8);

public:    //@todo private
    String8     effectName;
    uint32_t    major;
    uint32_t    minor;

};







/**
 *  @brief                      The prototype of MediaTek camera features.
 *  @details                    A common case of call sequence will be
 *  <pre>
 *  getNameVersion() (optional)
 *  init()
 *    setEffectListener()
 *    setParameter() * N
 *    prepare()
 *      setParameter() * N
 *      getCaptureRequirement()
 *      start()
 *        addInputFrame() * N
 *        addOutputFrame() * N
 *        abort() (optional)
 *    release()
 *  uninit()
 *  </pre>
 */
class IEffectHal
{
public:
    virtual     ~IEffectHal(){};

public: // may change state
    /**
     *  @brief                  The first function to initialize IEffectHal object.
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @return                 status_t
     */
    virtual status_t   init() = 0;

    /**
     *  @brief                  The latest function to de-initialize IEffectHal object.
     *
     *  @par When to call:
     *  After calling init()
     */
    virtual status_t   uninit() = 0;

    /**
     *  @brief                  A start call to inform IEffectHal, the client is ready to initial a request.
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling release().
     *
     *  @return                 status_t
     */
    virtual status_t   configure() = 0;

    /**
     *  @brief                  Abort current process.
     *  @details                client call this function to abort IEffectHal current activity.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @return                 status_t
     */
    virtual status_t   unconfigure() = 0;

    /**
     *  @brief                  A start call to inform IEffectHal, the client is ready to add input/output buffer.
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling release().
     *
     *  @return                 session id - a unique id for all IEffectHal::start()
     */
    virtual uint64_t            start() = 0;

    /**
     *  @brief                  Abort current process.
     *  @details                client call this function to abort IEffectHal current activity.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param parameter        for client to config abort behavior.
     *                          EX: For MAV and Panorama
     *                          - parameter["save"] = true
     *
     */
    virtual status_t   abort(EffectParameter const *parameter=NULL) = 0;


public: // would not change state
    /**
     *  @brief                  Get version of IEffectHal object
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @param[out] nameVersion A reference of returned name, major, minor version number
     */
    virtual status_t   getNameVersion(EffectHalVersion &nameVersion) const = 0;

    /**
     *  @brief                  Client register listener object by this function.
     *
     *  @par When to call:
     *                          At the start of IEffectHal instance has been created.
     *
     *  @param[in] listener     Point to client's listener object
     */
    virtual status_t   setEffectListener(const wp<IEffectListener>& listener) = 0;


    /**
     *  @brief                  The usage is similar to Android CameraParameters. The client use this api to set IEffectHal's parameter.
     *  @details                EX:setParameter(ZoomRatio, "320"); setParameter(Transform, "90");
     *
     *  @par When to call:
     *                          After calling init(), but before calling start().
     *
     *  @param[in] parameterKey the key name for the parameter
     *  @param[in] object       The address that point to the string value of the parameter
     *  @return                 status_t
     */
    virtual status_t   setParameter(String8 &key, String8 &object) = 0;    //@todo find better solution
    virtual status_t   setParameters(const sp<EffectParameter> parameter) =0;

    /**
     *  @brief                  Get the requirement for following capture request.
     *
     *  @par When to call:
     *                          After calling init(), but before calling unint().
     *
     *  @param[out] requirement filled Capture requirement. EX: (HDR)Target exp_time, gain for bright/dark frame
     *  @return                 status_t
     */
    virtual status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const = 0;

    /**
     *  @brief                  [non-blocking] The function to allocate necessary resource, initialize default setting of IEffectHal object.
     *
     *  @par When to call:
     *                          After calling init(), but before calling uninit()
     *
     *  @return                 status_t
     */
    //non-blocking
    virtual status_t   prepare() = 0;

    /**
     *  @brief                  Release the resource allocated by IEffectHal::prepare().
     *
     *  @par When to call:
     *                          After calling prepare(), but before calling uninit().
     */
    virtual status_t   release() = 0;

    /**
     *  @brief                  [non-blocking] Client call this function to feed input frame into IEffectHal for further process.
     *  @details                EX: (HDR) Client feed bright and dark frames by call this function twice.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param[in] frame        Point to input frame buffer. Set this as NULL if frame is unnecessary.
     *  @param[in] parameter    For client to attach information with buffer.
     *                          EX:
     *                          - parameter["key"] = "long-exposure"
     *                          - parameter["id"] = 1
     *  @return                 status_t
     */
    //non-blocking
    //virtual status_t   addInputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL) = 0;

    /**
     *  @brief                  [non-blocking] Client call this function to feed output frame into IEffectHal for further process.
     *  @details                EX: (HDR) HDR IEffectHal save processed frame in frame.
     *
     *  @par When to call:
     *                          After calling start(), but before EffectListener::onAborted() or EffectListener::onCompleted() has been triggered.
     *
     *  @param[in] frame        Point to output frame buffer. Set this as NULL if frame is unnecessary.
     *  @param[in] parameter    For client to attach information with buffer.
     *                          EX:
     *                          - parameter["key"] = "long-exposure"
     *                          - parameter["id"] = 1
     */
    //non-blocking
    //virtual status_t   addOutputFrame(const sp<IImageBuffer> frame, const sp<EffectParameter> parameter=NULL) = 0;

    virtual status_t   updateEffectRequest(const sp<EffectRequest> request) = 0;
public: //debug
public: //autotest
};


//-----------------------------------------------------------------------------
//IEffectFactory
//-----------------------------------------------------------------------------
class IEffectFactory : public IInterface
{
public:
    DECLARE_META_INTERFACE(EffectFactory);

public:
    /**
     *  @brief                  Create IEffectHal by name and version.
     *  @param[in] nameVersion  Specific effect name and verison
     *  @param[out] effectHal   Point to created IEffectHal instance.
     *  @return                 status_t
     */
    //virtual status_t   createEffectHal(EffectHalVersion nameVersion, sp<IEffectHal> &effectHal) = 0;

    /**
     *  @brief                  Create IEffectHalClient by name and version.
     *  @param[in] nameVersion  Specific effect name and verison
     *  @param[out] effectHalClient   Point to created IEffectHalClient instance.
     *  @return                 status_t
     */
    virtual status_t   createEffectHalClient(EffectHalVersion nameVersion, sp<IEffectHalClient> &effectHalClient) = 0;

    /**
     *  @brief                  If specific effect supported, return all supported versions. Otherwise return empty array.
     *  @param[in] effectName   effect name string
     *  @param[out] versions    A vector lists all supported versions or empty for not support.
     *  @return                 status_t
     */
    virtual status_t   getSupportedVersion(String8 effectName, Vector<EffectHalVersion> &versions) = 0;

#if 0   //@todo implement this - wait chengtian
    /**
     *  @brief                  Query multiple IEffectHal supported version.
     *  @param[in] effectNames  A vector lists effect name, EX: ["HdrEffect", "FbEffect"]
     *  @param[out] versions    A vector lists all supported name and corresponding version information
     *  @return                 status_t
     */
    virtual status_t   getSupportedVersion(Vector<String8> effectNames, Vector<Vector<EffectHalVersion> > &versions) = 0;
#endif

public:
    const static String8 EFFECT_HDR;     ///< "Hdr"
    const static String8 EFFECT_FB;      ///< "Fb"
    const static String8 EFFECT_MFLL;    ///< "Mfll"
    const static String8 EFFECT_PANORAMA;///< "Panorama"
    const static String8 EFFECT_MAV;     ///< "Mav"
};


class BnEffectFactory : public BnInterface<IEffectFactory>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


class EffectFactory : public BnEffectFactory
{
public:
    //EffectFactory(sp<IEffectFactory> const &client);
    //virtual     ~EffectFactory(){};
    EffectFactory();
    virtual     ~EffectFactory();

public:
    //virtual status_t   createEffectHal(EffectHalVersion nameVersion, sp<IEffectHal> &effectHal);
    virtual status_t   createEffectHalClient(EffectHalVersion nameVersion, sp<IEffectHalClient> &effectHalClient);
    virtual status_t   getSupportedVersion(String8 effectName, Vector<EffectHalVersion> &versions);
#if 0   //@todo implement this - wait chengtian
    virtual status_t   getSupportedVersion(Vector<String8> effectNames, Vector<Vector<EffectHalVersion> > &versions);
#endif
};
const String8 EFFECT_HDR       = String8("Hdr");
const String8 EFFECT_FB        = String8("Fb");
const String8 EFFECT_MFLL      = String8("Mfll");
const String8 EFFECT_PANORAMA  = String8("Panorama");
const String8 EFFECT_MAV       = String8("Mav");



//-----------------------------------------------------------------------------
//IEffectHalClient
//-----------------------------------------------------------------------------
//class IEffectHalClient : public IInterface, public IEffectHal
class IEffectHalClient : public IInterface, public IEffectHal
{
public:
    DECLARE_META_INTERFACE(EffectHalClient);

//-----------------------------------------------------------------------------
//API for IEffectHal
//-----------------------------------------------------------------------------
public: //ctor, DefaultKeyedVector
    //IEffectHalClient(sp<IEffectHal> &effect) = 0;
public: // may change state
    virtual status_t   init() = 0;
    virtual status_t   uninit() = 0;
    virtual status_t   configure() = 0;
    virtual status_t   unconfigure() = 0;
    virtual uint64_t   start() = 0;
    virtual status_t   abort(EffectParameter const *parameter=NULL) = 0;

public: // would not change state
    virtual status_t   getNameVersion(EffectHalVersion &nameVersion) const = 0;
    virtual status_t   setEffectListener(const wp<IEffectListener>& listener) = 0;
    virtual status_t   setParameter(String8 &key, String8 &object) = 0;
    virtual status_t   setParameters(const sp<EffectParameter> parameter) = 0;
    virtual status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const = 0;
    //non-blocking
    virtual status_t   prepare() = 0;
    virtual status_t   release() = 0;
    //non-blocking
    virtual status_t   updateEffectRequest(const sp<EffectRequest> request) = 0;
//-----------------------------------------------------------------------------
//API for buffer queue
//-----------------------------------------------------------------------------
public:
    //
    virtual status_t   getInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input) = 0;
    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &input, Vector<sp<EffectParameter> > &effectParams) = 0;

    //
    virtual status_t   addInputParameter(int index, sp<EffectParameter> &parameter, int64_t timestamp, bool repeat=false) = 0;
    virtual status_t   addOutputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat=false) = 0;

    //
    virtual status_t   setInputsyncMode(int index, bool sync) = 0;
    virtual bool       getInputsyncMode(int index) = 0;
    virtual status_t   setOutputsyncMode(int index, bool sync) = 0;
    virtual bool       getOutputsyncMode(int index) = 0;

    virtual void       dequeueAndQueueBuf(int64_t timestamp) = 0;
    virtual void       setStaticMetadata(sp <BasicParameters> staticMetadata) = 0;
public: //callback
    virtual void       onInputFrameAvailable() = 0;
    virtual void       onInputSurfacesChanged(EffectResult partialResult) = 0;

};





class BnEffectHalClient : public BnInterface<IEffectHalClient>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


class EffectHalClient :
    public BnEffectHalClient, public EffectListener//, public BufferItemConsumer::FrameAvailableListener
{
//-----------------------------------------------------------------------------
//API for IEffectHal
//-----------------------------------------------------------------------------

public:
    struct FrameInfo : public LightRefBase<FrameInfo>
    {
        //frame index
        int                     mIndex;
        bool                    mSyncMode;
        int64_t                 mTimestamp;
        bool                    mRepeat;
        bool                    isReady;
        sp<EffectParameter>     mEffectParam;

        //
        FrameInfo(
            int                 _idx = 0,
            bool                _sm = 0,
            int64_t             _ts = 0,
            bool                _re = 0,
            bool                _is = 0
        )
            : mIndex(_idx)
            , mSyncMode(_sm)
            , mTimestamp(_ts)
            , mRepeat(_re)
            , isReady(_is)
            , mEffectParam(NULL)
        {
        }
    };
    typedef DefaultKeyedVector<int64_t, sp<FrameInfo> > FrameInfoVect_t;


    struct InputBufferInfo : public LightRefBase<InputBufferInfo>
    {
        //surface id
        int                                 mSurfaceId;
        //get from BufferQueue(acquireBuffer)
        BufferItemConsumer::BufferItem      mBufferItem;
        InputBufferInfo(
            int                             _surfaceId,
            BufferItemConsumer::BufferItem  _bufferItem
        )
            : mSurfaceId(_surfaceId)
            , mBufferItem(_bufferItem)
        {
        }
    };
    //key is timestamp, value is InputBufferInfo
    typedef DefaultKeyedVector<int64_t, sp<InputBufferInfo> > InputBufferVect_t;

    struct OutputBufferInfo : public LightRefBase<OutputBufferInfo>
    {
        //surface id
        int                                 mSurfaceId;
        sp<GraphicBuffer>                   pImageBuffer;
        OutputBufferInfo(
            int                     _surfaceId = 0,
            sp<GraphicBuffer>       _imageBuffer = NULL
        )
            : mSurfaceId(_surfaceId)
            , pImageBuffer(_imageBuffer)
        {
        }
    };
    typedef Vector<sp<OutputBufferInfo> > OutputBufferVect_t;


    /*public:
        class BufferListener: public BufferItemConsumer::FrameAvailableListener
        {
        public:
            BufferListener(int idx, const wp<BufferListener>& listener) :
                    mIdx(idx),
                    mFrameAvailableListener(listener) {
            }

            virtual void            onFrameAvailable();
        private:
            int                     mIdx;
            //sp<EffectHalClient> mFrameAvailableListener;
            wp<BufferListener> mFrameAvailableListener;
        };*/

public: //ctor, dtor
    //TTT5 EffectHalClient(sp<IEffectHal> &effect);
    EffectHalClient(IEffectHal* effect, String8 name);
    virtual     ~EffectHalClient();

public: // may change state
    virtual status_t   init();
    virtual status_t   uninit();
    virtual status_t   configure();
    virtual status_t   unconfigure();
    virtual uint64_t   start();
    virtual status_t   abort(EffectParameter const *parameter=NULL);

public: // would not change state
    virtual status_t   getNameVersion(EffectHalVersion &nameVersion) const;
    virtual status_t   setEffectListener(const wp<IEffectListener>& listener);
    virtual status_t   setParameter(String8 &key, String8 &object);
    virtual status_t   setParameters(const sp<EffectParameter> parameter);
    virtual status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
    //non-blocking
    virtual status_t   prepare();
    virtual status_t   release();
    //non-blocking
    virtual status_t   updateEffectRequest(const sp<EffectRequest> request);
    virtual void       frameProcessored(sp<EffectFrameInfo>& frameInfo);
    virtual void       requestCompleted(sp<EffectRequest>& request);

//-----------------------------------------------------------------------------
//API for buffer queue
//-----------------------------------------------------------------------------
public:
    //
    virtual status_t   getInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input);

    //
    //virtual status_t   addInputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat=false);
    virtual status_t   addInputParameter(int index, sp<EffectParameter> &parameter, int64_t timestamp, bool repeat=false);
    virtual status_t   addOutputParameter(int index, EffectParameter &parameter, int64_t timestamp, bool repeat=false);

    //
    virtual status_t   setInputsyncMode(int index, bool sync);
    virtual bool       getInputsyncMode(int index);
    virtual status_t   setOutputsyncMode(int index, bool sync);
    virtual bool       getOutputsyncMode(int index);

    virtual void       convertGraphicBufferToIImageBuffer(sp<GraphicBuffer> &buf, sp<IImageBuffer> &imageBuffer);
    virtual void       addBufferQueue(Vector< sp<IGraphicBufferProducer> > &input, int index, int *height, int *width, int format);
    virtual void       getOutputBuffer(int surfaceIdx, int bufferCount, sp<ANativeWindow> anw, sp<EffectParameter> param);

    virtual void       setStaticMetadata(sp <BasicParameters> staticMetadata);
    /**************test***************/
    virtual void       dequeueAndQueueBuf(int64_t timestamp);
    virtual void       acquireBufferManual();
    int availableBufferIdx;
    /**************test***************/

public: //callback
    virtual void       onInputFrameAvailable();
    virtual void       onInputSurfacesChanged(EffectResult partialResult);
    virtual void       onBufferFrameAvailable(int index);



//-----------------------------------------------------------------------------
//API for effectListener
//-----------------------------------------------------------------------------
    virtual void       onPrepared(const IEffectHalClient* effect, const EffectResult& result) const;
    virtual void       onInputFrameProcessed(const IEffectHalClient* effect, const sp<EffectParameter> parameter, EffectResult partialResult);
    virtual void       onOutputFrameProcessed(const IEffectHalClient* effect, const sp<EffectParameter> parameter, EffectResult partialResult);
    virtual void       onCompleted(const IEffectHalClient* effect, const EffectResult& partialResult, uint64_t uid) const;
    virtual void       onAborted(const IEffectHalClient* effect, const EffectResult& result);
    virtual void       onFailed(const IEffectHalClient* effect, const EffectResult& result) const;

    // The dimensions for captured pictures in pixels (width x height).
    // Example value: "1024x768". Read/write.
    static const char KEY_PICTURE_SIZE[];
    // The image format for captured pictures. See CAMERA_MSG_COMPRESSED_IMAGE
    // in frameworks/base/include/camera/Camera.h.
    // Example value: "jpeg" or PIXEL_FORMAT_XXX constants. Read/write.
    static const char KEY_PICTURE_FORMAT[];

    //for test
    struct CpuConsumerTestParams
    {
        uint32_t       width;
        uint32_t       height;
        int            maxLockedBuffers;
        PixelFormat    format;
    };

    virtual void       produceOneFrame(const sp<ANativeWindow>& anw, const CpuConsumerTestParams& params, int64_t timestamp, uint32_t *stride);
    virtual void       configureANW(const sp<ANativeWindow>& anw, const CpuConsumerTestParams& params, int maxBufferSlack);

protected:
    virtual status_t   setInputSurfaces(Vector< sp<IGraphicBufferProducer> > &input);
    virtual status_t   setOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &ouput, Vector<sp<EffectParameter> > &effectParams);
    virtual status_t   getOutputSurfaces(Vector< sp<IGraphicBufferProducer> > &ouput);

protected:
    MINT32                      mLogLevel;

private:
    //TTT5 sp<IEffectHal>                                                         mpEffect;
    IEffectHal*                         mpEffect;

    //Vector< sp<IGraphicBufferProducer> > mOutputSurfaces;
    Vector< sp<Surface> >               mOutputSurfaces;
    Vector< sp<BufferItemConsumer> >    mInputComsumer;
    // Vector< sp<IGraphicBufferProducer> > mInputProducer;

    //for parameters
    FrameInfoVect_t                     mInputFrameInfo;
    FrameInfoVect_t                     mOutputFrameInfo;
    //for buffers
    InputBufferVect_t                   mInputBufferInfo;
    OutputBufferVect_t                  mOutputBufferInfo;

    //listener
    Vector< sp<BufferItemConsumer::FrameAvailableListener> > mListener;

    //key is {picture-size}{picture-format}, value is surface-id
    //When AP call getInputSurface, fill the mSurfaceMap according to getCaptureRequirement result.
    //When AP call getCaptureRequest, add surface-id to Vector<EffectCaptureRequirement> according to {picture-size}{picture-format} string.
    DefaultKeyedVector<String8, int> mSurfaceMap;

    Vector< bool >                  mInputSyncMode;
    Vector< bool >                  mOutputSyncMode;

    Vector<sp<EffectParameter> >    mOutputEffectParams;
    sp<IEffectListener>             mEffectListener;

    size_t                          mMaxBufferQueueSize;
    uint64_t                        mSessionUid;

    //=================================================================================
    //key is timestamp, value is bufferItem get from BufferQueue(acquireBuffer)
    DefaultKeyedVector<int64_t, BufferItemConsumer::BufferItem> mBufferMap;

    //replace by mBufferMap
    // Vector< BufferItemConsumer::BufferItem > mInputBuffer;
    // Vector< BufferItemConsumer::BufferItem > mOutputBuffer;
    Vector<sp<GraphicBuffer> >      mOutputBuffers;
    //=================================================================================

    sp<EffectRequest>               mpEffectRequest;


    //     //for test
    //sp<Surface> mSTC;
    //ANativeWindow *mANW;
    sp<IGraphicBufferProducer> mProducer;
    sp <BasicParameters> mpStaticMetadata;

    // minimal jpeg buffer size: 256KB + blob header <=FeatureAPI change to 256KB
    static const ssize_t       kMinJpegBufferSize = 256 * 1024 + sizeof(camera3_jpeg_blob);
    String8                         mEffectName;
};


/**
 *  @brief BufferListener exists to connect the
 *          EffectHalClient object and the consumer object.
 */
class BufferListener: public BufferItemConsumer::FrameAvailableListener
{
public:
    BufferListener(int idx, const wp<EffectHalClient>& listener) :
        mIdx(idx),
        mFrameAvailableListener(listener)
    {
    }
    virtual void onFrameAvailable(const BufferItem& item);
private:
    int                     mIdx;
    //sp<EffectHalClient> mFrameAvailableListener;
    wp<EffectHalClient> mFrameAvailableListener;
};

//-----------------------------------------------------------------------------
//IFeatureManager
//-----------------------------------------------------------------------------
class IFeatureManager : public IInterface
{
public:
    DECLARE_META_INTERFACE(FeatureManager);

public:
    virtual status_t           setParameter(String8 key, String8 value) = 0;
    virtual String8            getParameter(String8 key) = 0;
    virtual status_t           setUp(EffectHalVersion const &nameVersion) = 0;
    virtual status_t           tearDown(EffectHalVersion const &nameVersion) = 0;

    //
    virtual status_t           getEffectFactory(sp<IEffectFactory> & effectFactory) = 0;
};


class BnFeatureManager : public BnInterface<IFeatureManager>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


class FeatureManager : public BnFeatureManager
{
public:
    //          FeatureManager(sp<IFeatureManager> const &client);
    FeatureManager();
    virtual     ~FeatureManager();

public:
    status_t           setParameter(String8 key, String8 value);
    String8            getParameter(String8 key);
    status_t           setUp(EffectHalVersion const &nameVersion);
    status_t           tearDown(EffectHalVersion const &nameVersion);
    status_t           getEffectFactory(sp<IEffectFactory> & effectFactory);

private:
    status_t           createEffectFactory();

private:
    sp<IEffectFactory> mpEffectFactory;
};

};  //namespace NSCam {


#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTHAL_H_

