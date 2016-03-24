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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERSETCONTROL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERSETCONTROL_H_
//
#include <mtkcam/v3/stream/IStreamBufferSet.h>
#include "StreamBuffers.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
class IStreamBufferSetControl
    : public IStreamBufferSet
{
public:     ////                Definitions.
    /**
     *
     */
    class IAppCallback
        : public virtual android::RefBase
    {
    public:     ////            Operations.
        virtual MVOID           updateFrame(
                                    MUINT32 const frameNo,
                                    MINTPTR const userId
                                )                                           = 0;
    };

    /**
     *
     */
    class IListener
        : public virtual android::RefBase
    {
    public:
        /**
         * Invoked when the buffer set is updated.
         *
         * @param[in] pCookie: the listener's cookie.
         */
        virtual MVOID           onStreamBufferSet_Updated(
                                    MVOID*  pCookie
                                )                                           = 0;
    };

    /**
     *
     */
    template <class _StreamBuffer_>
    class IMap
        : public virtual android::RefBase
    {
    public:     ////            Definitions.
        typedef _StreamBuffer_  StreamBufferT;

    public:     ////            Operations.
        virtual ssize_t         add(
                                    android::sp<StreamBufferT> pBuffer
                                )                                           = 0;

        virtual ssize_t         setCapacity(size_t size)                    = 0;

        virtual bool            isEmpty() const                             = 0;

        virtual size_t          size() const                                = 0;

        virtual ssize_t         indexOfKey(StreamId_T const key) const      = 0;

        virtual StreamId_T      keyAt(size_t index) const                   = 0;

        virtual android::sp<StreamBufferT>const&
                                valueAt(size_t index) const                 = 0;

        virtual android::sp<StreamBufferT>const&
                                valueFor(StreamId_T const key) const        = 0;

    };

public:     ////                Operations.
    virtual android::sp<IMap<IImageStreamBuffer> >
                                editMap_AppImage()                          = 0;

    virtual android::sp<IMap<IMetaStreamBuffer> >
                                editMap_AppMeta()                           = 0;

    virtual android::sp<IMap<HalImageStreamBuffer> >
                                editMap_HalImage()                          = 0;

    virtual android::sp<IMap<HalMetaStreamBuffer> >
                                editMap_HalMeta()                           = 0;

public:     ////                Operations.
    static  android::sp<IStreamBufferSetControl>
                                create(
                                    MUINT32 frameNo,
                                    android::wp<IAppCallback>const& pAppCallback
                                );

    virtual MERROR              attachListener(
                                    android::wp<IListener>const& pListener,
                                    MVOID* pCookie
                                )                                           = 0;

    virtual MUINT32             getFrameNo() const                          = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERSETCONTROL_H_

