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

#ifndef _MTK_HARDWARE_MTKCAM_V3_APP_APPSTREAMMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_V3_APP_APPSTREAMMANAGER_H_
//
#include <utils/BitSet.h>
#include <mtkcam/v3/app/StreamId.h>
#include <mtkcam/v3/app/IAppStreamManager.h>
//
#include <mtkcam/metadata/IMetadataConverter.h>
#include <cutils/properties.h>
//
#include <utils/Timers.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Imp {


/**
 * An implementation of App stream manager.
 */
class AppStreamManager
    : public IAppStreamManager
    , public android::Thread
{
    friend  class IAppStreamManager;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Callback (camera3_capture_result & camera3_notify_msg)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  CallbackParcel
    {
        struct  ImageItem
        {
            android::sp <
                AppImageStreamBuffer
                        >                   buffer;
            android::sp <
                AppImageStreamInfo
                        >                   stream;
        };

        struct  MetaItem
        {
            android::sp <
                IMetaStreamBuffer
                        >                   buffer;
            MUINT32                         bufferNo;   //partial_result
        };

        struct  Error
            : public android::LightRefBase<Error>
        {
            android::sp <
                AppImageStreamInfo
                        >                   stream;
            MINT                            errorCode;
        };

        struct  Shutter
            : public android::LightRefBase<Shutter>
        {
            MUINT64                         timestamp;
        };

        android::Vector<ImageItem>          vInputImageItem;
        android::Vector<ImageItem>          vOutputImageItem;
        android::Vector<MetaItem>           vOutputMetaItem;
        android::Vector<Error>              vError;
        android::sp<Shutter>                shutter;
        MUINT32                             frameNo;
        MBOOL                               valid;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Result Queue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    typedef android::SortedVector<MUINT32>  ResultQueueT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.
    MINT32  const               mOpenId;
    camera3_callback_ops const* mpCallbackOps;

protected:  ////                Data Members.
    android::sp<IMetadataProvider const>
                                mpMetadataProvider;
    size_t                      mAtMostMetaStreamCount;
    android::sp<IMetadataConverter>
                                mpMetadataConverter;
    camera_metadata_t*          mMetadata;

protected:  ////                Data Members (RESULT)
    mutable android::Mutex      mResultQueueLock;
    android::Condition          mResultQueueCond;
    ResultQueueT                mResultQueue;

protected:  ////                Data Members (CONFIG/FRAME)
    class   FrameHandler;
    android::sp<FrameHandler>   mFrameHandler;
    mutable android::Mutex      mFrameHandlerLock;
    android::Condition          mFrameHandlerCond;
    StreamId_T                  mStreamIdToConfig;
    IMetadata                   mLatestSettings;

protected:  ////                LOGD & LOGI on / off
    MINT32                      mLogLevel;

protected:  ////                fps / duration information for debug
    MUINT64                     mAvgTimestampDuration;
    MUINT64                     mAvgCallbackDuration;
    MUINT64                     mAvgTimestampFps;
    MUINT64                     mAvgCallbackFps;
    MUINT32                     mFrameCounter;
    MUINT32                     mMaxFrameCount;
    //                          latest timestamp / callback time
    MUINT64                     mTimestamp;
    MUINT64                     mCallbackTime;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Operations.
                                AppStreamManager(
                                    MINT32 openId,
                                    camera3_callback_ops const* callback_ops,
                                    IMetadataProvider const* pMetadataProvider
                                );

protected:  ////                Operations.
    MERROR                      checkStream(camera3_stream* stream) const;
    MERROR                      checkStreams(camera3_stream_configuration_t *stream_list) const;

    AppImageStreamInfo*         createImageStreamInfo(
                                    StreamId_T suggestedStreamId,
                                    camera3_stream* stream
                                )   const;

    AppMetaStreamInfo*          createMetaStreamInfo(
                                    StreamId_T suggestedStreamId
                                )   const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Request Handler)
    MERROR                      checkRequestLocked(
                                    camera3_capture_request_t const* request
                                )   const;
    AppImageStreamBuffer*       createImageStreamBuffer(
                                    camera3_stream_buffer const* buffer
                                )   const;
    AppMetaStreamBuffer*        createMetaStreamBuffer(
                                    android::sp<IMetaStreamInfo> pStreamInfo,
                                    IMetadata const& rSettings,
                                    MBOOL const repeating
                                )   const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Result Handler)
    MERROR                      enqueResult(MUINT32 const frameNo, MINTPTR const userId);
    MERROR                      dequeResult(ResultQueueT& rvResult);
    MVOID                       handleResult(ResultQueueT const& rvResult);
    MVOID                       performCallback(CallbackParcel const& cbParcel);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                requestExit();

    // Good place to do one-time initializations
    virtual android::status_t   readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAppStreamManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.

    virtual MVOID               destroy();

    virtual MERROR              configureStreams(
                                    camera3_stream_configuration_t *stream_list
                                );

    virtual MERROR              queryConfiguredStreams(
                                    ConfigAppStreams& rStreams
                                )   const;

    virtual MERROR              createRequest(
                                    camera3_capture_request_t* request,
                                    Request& rRequest
                                );

    virtual MERROR              registerRequest(
                                    Request const& rRequest
                                );

    virtual MVOID               updateResult(
                                    MUINT32 const frameNo,
                                    MINTPTR const userId
                                );

    virtual MVOID               updateResult(
                                    MUINT32 const frameNo,
                                    MINTPTR const userId,
                                    android::Vector< android::sp<IMetaStreamBuffer> > resultMeta,
                                    bool hasLastPartial
                                );

    virtual MERROR              waitUntilDrained(nsecs_t const timeout);

};


/**
 * Frame Handler
 */
class AppStreamManager::FrameHandler
    : public android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  FrameParcel;
    struct  MetaItem;
    struct  MetaItemSet;
    struct  ImageItem;
    struct  ImageItemSet;

    /**
     * IN_FLIGHT    -> PRE_RELEASE
     * IN_FLIGHT    -> VALID
     * IN_FLIGHT    -> ERROR
     * PRE_RELEASE  -> VALID
     * PRE_RELEASE  -> ERROR
     */
    struct  State
    {
        enum T
        {
            IN_FLIGHT,
            PRE_RELEASE,
            VALID,
            ERROR,
        };
    };

    struct  HistoryBit
    {
        enum
        {
            RETURNED,
            ERROR_SENT_FRAME,
            ERROR_SENT_META,
            ERROR_SENT_IMAGE,
        };
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Meta Stream
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  MetaItem
        : public android::RefBase
    {
        FrameParcel*                        pFrame;
        MetaItemSet*                        pItemSet;
        State::T                            state;
        android::BitSet32                   history;//HistoryBit::xxx
        android::sp<IMetaStreamBuffer>      buffer;
                                            //
                                            MetaItem()
                                                : android::RefBase()
                                                , pFrame(NULL)
                                                , pItemSet(NULL)
                                                , state(State::IN_FLIGHT)
                                                , history(0)
                                                , buffer()
                                            {}
    };

    struct  MetaItemSet
        : public android::DefaultKeyedVector<StreamId_T, android::sp<MetaItem> >
    {
        MBOOL                               asInput;
        size_t                              numReturnedStreams;
        size_t                              numValidStreams;
        size_t                              numErrorStreams;
                                            //
                                            MetaItemSet(MBOOL _asInput)
                                                : asInput(_asInput)
                                                , numReturnedStreams(0)
                                                , numValidStreams(0)
                                                , numErrorStreams(0)
                                            {}
    };

    struct  MetaConfigItem
    {
        android::sp<AppMetaStreamInfo>      pStreamInfo;
                                            //
                                            MetaConfigItem()
                                                : pStreamInfo()
                                            {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Image Stream
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  ImageItemFrameQueue
        : public android::List<android::sp<ImageItem> >
    {
                                            ImageItemFrameQueue()
                                            {}
    };

    struct  ImageItem
        : public android::RefBase
    {
        FrameParcel*                        pFrame;
        ImageItemSet*                       pItemSet;
        State::T                            state;
        android::BitSet32                   history;//HistoryBit::xxx
        android::sp<AppImageStreamBuffer>   buffer;
        ImageItemFrameQueue::iterator       iter;
                                            //
                                            ImageItem()
                                                : android::RefBase()
                                                , pFrame(NULL)
                                                , pItemSet(NULL)
                                                , state(State::IN_FLIGHT)
                                                , history(0)
                                                , buffer()
                                                , iter()
                                            {}
    };

    struct  ImageItemSet
        : public android::DefaultKeyedVector<StreamId_T, android::sp<ImageItem> >
    {
        MBOOL                               asInput;
        size_t                              numReturnedStreams;
        size_t                              numValidStreams;
        size_t                              numErrorStreams;
                                            //
                                            ImageItemSet(MBOOL _asInput)
                                                : asInput(_asInput)
                                                , numReturnedStreams(0)
                                                , numValidStreams(0)
                                                , numErrorStreams(0)
                                            {}
    };

    struct  ImageConfigItem
    {
        android::sp<AppImageStreamInfo>     pStreamInfo;
        ImageItemFrameQueue                 vItemFrameQueue;
                                            //
                                            ImageConfigItem()
                                                : pStreamInfo()
                                                , vItemFrameQueue()
                                            {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Frame Parcel & Queue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  FrameParcel
        : public android::RefBase
    {
        ImageItemSet                        vOutputImageItem;
        ImageItemSet                        vInputImageItem;
        MetaItemSet                         vOutputMetaItem;
        MetaItemSet                         vInputMetaItem;
        MUINT32                             frameNo;
        MUINT64                             timestampShutter;
        android::BitSet32                   errors; //HistoryBit::ERROR_SENT_xxx
                                            //
                                            FrameParcel()
                                                : android::RefBase()
                                                //
                                                , vOutputImageItem(MFALSE)
                                                , vInputImageItem(MTRUE)
                                                , vOutputMetaItem(MFALSE)
                                                , vInputMetaItem(MTRUE)
                                                //
                                                , frameNo(0)
                                                , timestampShutter(0)
                                                , errors(0)
                                            {}
    };

    struct  FrameQueue
        : public android::List<android::sp<FrameParcel> >
    {
        MUINT32                             latestResultFrameNo;
                                            FrameQueue()
                                                : latestResultFrameNo(0)
                                            {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.
    android::sp<IMetadataProvider const>
                                mpMetadataProvider;
    size_t                      mAtMostMetaStreamCount;

protected:  ////                Data Members (CONFIG/REQUEST)
    FrameQueue                  mFrameQueue;

    android::KeyedVector<
        StreamId_T,
        ImageConfigItem
                        >       mImageConfigMap;

    android::KeyedVector<
        StreamId_T,
        MetaConfigItem
                        >       mMetaConfigMap;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations: Request
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Operations.
    MERROR                      registerStreamBuffers(
                                    android::KeyedVector<
                                        StreamId_T,
                                        android::sp<AppImageStreamBuffer>
                                                        > const& buffers,
                                    android::sp<FrameParcel> const pFrame,
                                    ImageItemSet*const pItemSet
                                );
    MERROR                      registerStreamBuffers(
                                    android::KeyedVector<
                                        StreamId_T,
                                        android::sp<IMetaStreamBuffer>
                                                        > const& buffers,
                                    android::sp<FrameParcel> const pFrame,
                                    MetaItemSet*const pItemSet
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations: Result
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Operations.

    /**
     * @param[in] frame: a given frame to check.
     *
     * @return
     *      ==0: uncertain
     *      > 0: it is indeed a request error
     *      < 0: it is indeed NOT a request error
     */
    static  MINT                checkRequestError(FrameParcel const& frame);

protected:  ////                Operations.
    MVOID                       prepareErrorFrame(
                                    CallbackParcel& rCbParcel,
                                    android::sp<FrameParcel> const& pFrame
                                );
    MVOID                       prepareErrorMetaIfPossible(
                                    CallbackParcel& rCbParcel,
                                    android::sp<MetaItem> const& pItem
                                );
    MVOID                       prepareErrorImage(
                                    CallbackParcel& rCbParcel,
                                    android::sp<ImageItem> const& pItem
                                );

protected:  ////                Operations.
    MVOID                       prepareShutterNotificationIfPossible(
                                    CallbackParcel& rCbParcel,
                                    android::sp<MetaItem> const& pItem
                                );
    MVOID                       prepareReturnMeta(
                                    CallbackParcel& rCbParcel,
                                    android::sp<MetaItem> const& pItem
                                );
    MBOOL                       isReturnable(
                                    android::sp<MetaItem> const& pItem
                                )   const;

protected:  ////                Operations.
    MVOID                       prepareReturnImage(
                                    CallbackParcel& rCbParcel,
                                    android::sp<ImageItem> const& pItem
                                );
    MBOOL                       isReturnable(
                                    android::sp<ImageItem> const& pItem
                                )   const;

protected:  ////                Operations.
    MBOOL                       isFrameRemovable(
                                    android::sp<FrameParcel> const& pFrame
                                )   const;
    MBOOL                       prepareCallbackIfPossible(
                                    CallbackParcel& rCbParcel,
                                    MetaItemSet& rItemSet
                                );
    MBOOL                       prepareCallbackIfPossible(
                                    CallbackParcel& rCbParcel,
                                    ImageItemSet& rItemSet
                                );

protected:  ////                Operations.
    MVOID                       updateItemSet(MetaItemSet& rItemSet);
    MVOID                       updateItemSet(ImageItemSet& rItemSet);
    MVOID                       update(ResultQueueT const& rvResult);
    MVOID                       update(android::List<CallbackParcel>& rCbList);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.
                                FrameHandler(IMetadataProvider const* pMetadataProvider);

    MBOOL                       isEmptyFrameQueue() const;
    size_t                      getFrameQueueSize() const;

    MVOID                       addConfigStream(AppImageStreamInfo* pStreamInfo);
    MVOID                       addConfigStream(AppMetaStreamInfo* pStreamInfo);
    MERROR                      getConfigStreams(ConfigAppStreams& rStreams) const;
    android::sp<AppMetaStreamInfo> getConfigMetaStream(size_t index) const;

    MERROR                      registerFrame(Request const& rRequest);

    MVOID                       update(
                                    ResultQueueT const& rvResult,
                                    android::List<CallbackParcel>& rCbList
                                );

    MVOID                       dump();
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Imp
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_V3_APP_APPSTREAMMANAGER_H_

