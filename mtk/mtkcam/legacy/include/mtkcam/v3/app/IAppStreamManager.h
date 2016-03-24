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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_IAPPSTREAMMANAGER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_IAPPSTREAMMANAGER_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include <mtkcam/v3/utils/streaminfo/MetaStreamInfo.h>
#include "Cam3ImageStreamInfo.h"
#include "AppStreamBuffers.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * An interface of App stream manager.
 */
class IAppStreamManager
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Definitions.
    typedef Cam3ImageStreamInfo AppImageStreamInfo;

    typedef NSCam::v3::Utils::MetaStreamInfo
                                AppMetaStreamInfo;

    typedef NSCam::v3::AppImageStreamBuffer
                                AppImageStreamBuffer;

    typedef NSCam::v3::AppMetaStreamBuffer
                                AppMetaStreamBuffer;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Definitions.

    struct  Request
    {
        /*****
         * Assigned by App Stream Manager (during createRequest).
         */

        /**
         * @param frame number.
         */
        MUINT32                 frameNo;

        /**
         * @param input image stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<AppImageStreamBuffer>
                            >   vInputImageBuffers;

        /**
         * @param output image stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<AppImageStreamBuffer>
                            >   vOutputImageBuffers;

        /**
         * @param input meta stream buffers.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamBuffer>
                            >   vInputMetaBuffers;

        /*****
         * Assigned out of App Stream Manager.
         */

        /**
         * @param output meta stream buffers.
         *  Note that the number of output meta streams equals to the number of
         *  partial meta result callbacks.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamBuffer>
                            >   vOutputMetaBuffers;
    };


    struct  ConfigAppStreams
    {
        /**
         * @param image streams.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamInfo>
                            >   vImageStreams;

        /**
         * @param stream min frame duration.
         */
        android::KeyedVector<
            StreamId_T,
            MINT64
                            >   vMinFrameDuration;

        /**
         * @param stream stall frame duration.
         */
        android::KeyedVector<
            StreamId_T,
            MINT64
                            >   vStallFrameDuration;

        /**
         * @param meta streams.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamInfo>
                            >   vMetaStreams;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.

    /**
     * Create an instance.
     */
    static  IAppStreamManager*  create(
                                    MINT32 openId,
                                    camera3_callback_ops const* callback_ops,
                                    IMetadataProvider const* pMetadataProvider
                                );

    /**
     * Destroy the instance.
     */
    virtual MVOID               destroy()                                   = 0;

    /**
     * Configure streams.
     *
     * @param[in] stream_list
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              configureStreams(
                                    camera3_stream_configuration_t *stream_list
                                )                                           = 0;

    /**
     * Query configured streams.
     * This call is valid only after streams are configured.
     *
     * @param[out] rStreams: contains all configured streams.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              queryConfiguredStreams(
                                    ConfigAppStreams& rStreams
                                )   const                                   = 0;

    /**
     * Create a request based-on camera3_capture_request.
     * This call is valid only after streams are configured.
     *
     * @param[in] request: a given camera3_capture_request.
     *
     * @param[out] rRequest: a newly-created request.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              createRequest(
                                    camera3_capture_request_t* request,
                                    Request& rRequest
                                )                                           = 0;

    /**
     * Register a request, which is created from createRequest.
     * This call is valid only after streams are configured.
     *
     * @param[in] rRequest: a request to register.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR              registerRequest(
                                    Request const& rRequest
                                )                                           = 0;

    /**
     * Update a given result frame.
     *
     * @param[in] frameNo: the frame number to update.
     */
    virtual MVOID               updateResult(
                                    MUINT32 const frameNo,
                                    MINTPTR const userId
                                )                                           = 0;

    /**
     * Update a given result frame.
     *
     * @param[in] frameNo: the frame number to update.
     *
     * @param[in] resultMeta: result partial metadata to update.
     *
     * @param[in] hasLastPartial: contain last partial metadata in result partial metadata vector.
     */
    virtual MVOID               updateResult(
                                    MUINT32 const frameNo,
                                    MINTPTR const userId,
                                    android::Vector< android::sp<IMetaStreamBuffer> > resultMeta,
                                    bool hasLastPartial
                                )                                           = 0;

    /**
     * Wait until all the registered requests have finished returning.
     *
     * @param[in] timeout
     */
    virtual MERROR              waitUntilDrained(nsecs_t const timeout)     = 0;

    virtual MERROR              queryOldestRequestNumber(MUINT32 &ReqNo){return OK;}
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_IAPPSTREAMMANAGER_H_

