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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFERSET_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFERSET_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class IStreamBufferSet
    : public virtual android::RefBase
{
public:     ////                            Definitions.
    typedef IUsersManager::UserId_T         UserId_T;

public:     ////                            Operations.
#if 0
    /**
     * Create a subject's acquire fence associated with a user.
     * This user must wait on this fence before attempting to use the subject.
     *
     * @param[in] streamId: A stream Id.
     *
     * @param[in] userId: A specified user Id.
     *
     * @return
     *      A bad fence indicates this subject has not been initialized or need
     *      not to wait before using it.
     */
    virtual MINT                            createAcquireFence(
                                                StreamId_T const streamId,
                                                UserId_T userId
                                            )   const                       = 0;

    /**
     * Set a specified user's release fence.
     * The specified user must be enqueued before this call.
     *
     * @param[in] streamId: A stream Id.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @param[in] releaseFence: A release fence to register.
     *      The callee takes the ownership of the fence file descriptor and is
     *      charge of closing it.
     *      If a release fence associated with this user is specified during
     *      enqueUserList(), the old release fence will be replaced with the
     *      specified release fence after this call.
     *
     * @return
     *      0 indicates success; otherwise failure.
     *      NAME_NOT_FOUND indicates a bad user Id.
     */
    virtual MERROR                          setUserReleaseFence(
                                                StreamId_T const streamId,
                                                UserId_T userId,
                                                MINT releaseFence
                                            )                               = 0;

    /**
     * Query a specific user's group usage.
     *
     * @param[in] streamId: A stream Id.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @return
     *      A group usage associated with this user.
     */
    virtual MUINT                           queryGroupUsage(
                                                StreamId_T const streamId,
                                                UserId_T userId
                                            )   const                       = 0;
#endif

public:     ////                            Operations.
    /**
     * This call marks a specified user's status.
     *
     * @param[in] streamId: A stream Id.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @param[in] eStatus: user status.
     *      ACQUIRE     : This user has waited on the subject's acquire fence.
     *      PRE_RELEASE : This user is ready to pre-release the subject and will
     *                    still use it after its pre-release until a release
     *                    fence is signalled.
     *      USED        : This user has used the subject.
     *      RELEASE     : This user is ready to release the subject and will not
     *                    use it after its release.
     *
     * @return
     *      the current status mask.
     */
    virtual MUINT32                         markUserStatus(
                                                StreamId_T const streamId,
                                                UserId_T userId,
                                                MUINT32 eStatus
                                            )                               = 0;

public:     ////                            Operations.
    /**
     * Apply to pre-release.
     * After this call, all of PRE_RELEASE-marked buffers are pre-released by
     * this user.
     *
     * @param[in] userId: A specified unique user Id.
     *
     */
    virtual MVOID                           applyPreRelease(
                                                UserId_T userId
                                            )                               = 0;

    /**
     * Apply to release.
     * After this call, all of RELEASE-marked buffers are released by this user.
     *
     * @param[in] userId: A specified unique user Id.
     *
     */
    virtual MVOID                           applyRelease(
                                                UserId_T userId
                                            )                               = 0;

public:     ////                            Operations.

    /**
     * For a specific stream buffer (associated with a stream Id), a user (with
     * a unique user Id) could successfully acquire the buffer from this buffer
     * set only if all users ahead of this user have pre-released or released
     * the buffer.
     *
     * @param[in] streamId: A specified unique stream Id.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @return
     *      A pointer to the buffer associated with the given stream Id.
     */
    virtual android::sp<IMetaStreamBuffer>  getMetaBuffer(
                                                StreamId_T streamId,
                                                UserId_T userId
                                            )   const                       = 0;

    virtual android::sp<IImageStreamBuffer> getImageBuffer(
                                                StreamId_T streamId,
                                                UserId_T userId
                                            )   const                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_ISTREAMBUFFERSET_H_

