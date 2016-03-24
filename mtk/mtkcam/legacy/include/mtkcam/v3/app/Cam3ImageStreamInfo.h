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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_CAM3IMAGESTREAMINFO_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_CAM3IMAGESTREAMINFO_H_
//
#include <utils/String8.h>
#include <utils/RWLock.h>
//
#include <hardware/camera3.h>
#include <mtkcam/v3/stream/IStreamInfo.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * camera3 image stream info.
 */
class Cam3ImageStreamInfo : public IImageStreamInfo
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    typedef Cam3ImageStreamInfo     ThisT;

protected:  ////                    Data Members.

    camera3_stream*                 mpCamera3Stream;    /**<
                                                         * camera3_stream::usage: usage for buffer allocator.
                                                         */
    android::String8                mStreamName;
    StreamId_T                      mStreamId;
    MUINT32                         mStreamType;

    MUINT                           mUsageForConsumer;  /**< usage for buffer consumer. */
    MINT                            mImgFormat;         /**< image format in reality. */
    MINT                            mImgFormatToAlloc;  /**< image format for buffer allocation.
                                                         * It must equal to camera3_stream::format.
                                                         */
    MSize                           mImgSize;           /**< image size, in pixels.
                                                         * It must equal to width and height in camera3_stream.
                                                         */
    BufPlanes_t                     mvbufPlanes;

    MUINT32                         mTransform;
    mutable android::RWLock         mLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    /**
     *  @param[in] pCamera3Stream: pointer to a camera3_stream, where
     *      all fields except priv are specified.
     */
                                    Cam3ImageStreamInfo(
                                        camera3_stream*     pCamera3Stream,
                                        char const*         streamName,
                                        StreamId_T          streamId,
                                        MUINT               usageForConsumer,
                                        MUINT               usageForAllocator,
                                        MINT                imgFormatToAlloc,
                                        MINT                imgFormatInFact,
                                        BufPlanes_t const&  bufPlanes,
                                        size_t              maxBufNum = 1,
                                        MUINT32             transform = 0
                                    );

public:     ////                    Attributes.

    static  ThisT*                  cast(camera3_stream* stream);
    static  ThisT const*            cast(camera3_stream const* stream);

    virtual camera3_stream*         get_camera3_stream() const;

    /*
     * image format to alloc.
     *
     * @remark It must equal to camera3_stream::format.
     */
    virtual MINT                    getImgFormatToAlloc() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    /**
     * Usage for buffer consumer.
     */
    virtual MUINT                   getUsageForConsumer() const;

    /**
     * Usage for buffer allocator.
     *
     * @remark It must equal to camera3_stream::usage.
     */
    virtual MUINT                   getUsageForAllocator() const;

    virtual MINT                    getImgFormat() const;
    virtual MSize                   getImgSize() const;
    virtual BufPlanes_t const&      getBufPlanes() const;
    virtual MUINT32                 getTransform() const;
    virtual MERROR                  setTransform(MUINT32 transform);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual char const*             getStreamName() const;
    virtual StreamId_T              getStreamId() const;
    virtual MUINT32                 getStreamType() const;
    virtual size_t                  getMaxBufNum() const;
    virtual MVOID                   setMaxBufNum(size_t count);
    virtual size_t                  getMinInitBufNum() const;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_APP_CAM3IMAGESTREAMINFO_H_

