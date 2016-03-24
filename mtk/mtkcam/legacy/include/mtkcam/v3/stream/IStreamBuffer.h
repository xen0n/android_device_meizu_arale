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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFER_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/List.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/Sync.h>
#include <mtkcam/metadata/IMetadata.h>
#include "IUsersManager.h"
#include "IStreamInfo.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * Camera stream buffer status.
 */
struct STREAM_BUFFER_STATUS
{
    typedef STREAM_BUFFER_STATUS    T;
    enum
    {
        ERROR                       = (1U << 0),/*!< The buffer may contain invalid data. */
        WRITE                       = (1U << 1),/*!< The buffer's content has been touched. */
        WRITE_OK                    = T::WRITE,
        WRITE_ERROR                 = T::WRITE | T::ERROR,
    };
};


/**
 * An interface of stream buffer.
 */
class IStreamBuffer
    : public virtual IUsersManager
{
public:     ////                            Attributes.
    virtual char const*                     getName() const                 = 0;
    virtual MUINT32                         getStatus() const               = 0;
    virtual MBOOL                           hasStatus(MUINT32 mask) const   = 0;
    virtual MVOID                           markStatus(MUINT32 mask)        = 0;
    virtual MVOID                           clearStatus()                   = 0;

};


/**
 * An interface template of stream buffer.
 *
 * @param <_IStreamInfoT_> the type of stream info interface.
 *
 * @param <_IBufferT_> the type of buffer interface.
 */
template<
    class _IStreamInfoT_,
    class _IBufferT_
>
class TIStreamBuffer
    : public virtual IStreamBuffer
{
public:     ////                            Definitions.
    typedef _IStreamInfoT_                  IStreamInfoT;
    typedef _IBufferT_                      IBufferT;

public:     ////                            Attributes.
    virtual IStreamInfoT const*             getStreamInfo() const           = 0;

public:     ////                            Operations.
    /**
     * Release the buffer and unlock its use.
     *
     * @remark Make sure that the caller name must be the same as that passed
     *  during tryReadLock or tryWriteLock.
     */
    virtual MVOID                           unlock(
                                                char const* szCallName,
                                                IBufferT* pBuffer
                                            )                               = 0;

    /**
     * A reader must try to lock the buffer for reading.
     *
     * @remark The same caller name must be passed to unlock.
     */
    virtual IBufferT*                       tryReadLock(
                                                char const* szCallName
                                            )                               = 0;

    /**
     * A writer must try to lock the buffer for writing.
     *
     * @remark The same caller name must be passed to unlock.
     */
    virtual IBufferT*                       tryWriteLock(
                                                char const* szCallName
                                            )                               = 0;
};


/**
 * An interface of metadata stream buffer.
 */
class IMetaStreamBuffer
    : public TIStreamBuffer<IMetaStreamInfo, IMetadata>
{
public:     ////                            Definitions.

public:     ////                            Attributes.

    /**
     * MTRUE indicates that the meta settnigs are identical to the most-recently
     * submitted meta settnigs; otherwise MFALSE.
     */
    virtual MBOOL                           isRepeating() const             = 0;

public:     ////                            Operations.

};


/**
 * An interface of image stream buffer.
 */
class IImageStreamBuffer
    : public TIStreamBuffer<IImageStreamInfo, IImageBufferHeap>
{
public:     ////                            Definitions.

public:     ////                            Attributes.

public:     ////                            Operations.

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFER_H_

