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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_SYNC_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_SYNC_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {
namespace Sync {


/******************************************************************************
 *  Fence Interface
 ******************************************************************************/
class IFence : public virtual android::RefBase
{
public:     ////                        Definitions.

    static  android::sp<IFence> const   NO_FENCE;

    enum    { TIMEOUT_NEVER = -1 };

public:     ////                        Operations.

    /**
     * Create a new fence object to manage a given fence file descriptor.
     * If a valid fence file descriptor is given, it will be closed when the
     * newly createdly object is destructed.
     */
    static  android::sp<IFence>         create(MINT fenceFd);

    /**
     * Merge two fence objects, creating a new fence object that becomes
     * signaled when both f1 and f2 are signaled (even if f1 or f2 is destroyed
     * before it becomes signaled)
     *
     * @param[in] szFenceName: a name to associated with the new fence.
     *
     * @param[in] f1,f2: fence objects to merge.
     *
     * @return a newly createdly fence.
     */
    static  android::sp<IFence>         merge(
                                            char const* szFenceName,
                                            android::sp<IFence> const& f1,
                                            android::sp<IFence> const& f2
                                        );

    /**
     * Return a duplicate of the fence file descriptor.
     * The caller is responsible for closing the returned file descriptor.
     * On error, -1 will be returned and errno will indicate the problem.
     */
    virtual MINT                        dup() const                         = 0;

    /**
     * Wait with a given timeout for a fence to signal.
     *
     * @param[in] timeoutMs: a timeout in milliseconds.
     *      A timeout of TIMEOUT_NEVER may be used to indicate that the call
     *      should wait indefinitely for the fence to signal.
     *
     * @return
     *      OK if the fence is signaled.
     *      TIMED_OUT if the timeout expires before the fence signals.
     */
    virtual MERROR                      wait(MINT timeoutMs)                = 0;

    /**
     * Wait forever for a fence to signal.
     * Just like wait(TIMEOUT_NEVER), this is a convenience function for waiting
     * forever but issuing an error to the system log and fence state to the
     * kernel log if the wait lasts longer than a warning timeout.
     *
     * @param[in] logname: a timeout in milliseconds.
     *      The logname argument should be a string identifying the caller and
     *      will be included in the log message.
     *
     * @return
     *      OK if the fence is signaled.
     *      TIMED_OUT if the timeout expires before the fence signals.
     */
    virtual MERROR                      waitForever(char const* logname)    = 0;

public:     ////                        Attributes.

    /**
     * Fence name.
     */
    virtual char const*                 name() const                        = 0;

    /**
     * Check to see whether this fence is valid or not.
     */
    virtual MBOOL                       isValid() const                     = 0;

    /**
     * Get fence fd.
     */
    virtual MINT                        getFd() const                       = 0;

    /**
     * Return the system monotonic clock time at which the fence transitioned to
     * the signaled state.
     *
     * @return
     *      -1 if the fence is invalid or if an error occurs.
     *      INT64_MAX if the fence is not signaled.
     *      Otherwise, a timestamp in ns, at which the fence is signaled.
     */
    virtual MINT64                      getSignalTime() const               = 0;

};


/******************************************************************************
 *  Timeline Interface
 ******************************************************************************/
class ITimeline : public virtual android::RefBase
{
public:     ////                        Operations.

    /**
     * Create a new sync timeline object.
     *
     * @param[in] name: a name to associated with the timeline.
     */
    static  android::sp<ITimeline>      create(char const* szTimelineName);

    /**
     * Increase timeline
     *
     * @param[in] count: timline increase count.
     */
    virtual MERROR                      inc(size_t count)                   = 0;

    /**
     * Create a new Fence
     *
     * @param[in] szFenceName: The name of Fence
     * @param[in] value: Timeline FD
     */
    virtual MINT                        createFence(
                                            char const* szFenceName,
                                            size_t value
                                        )                                   = 0;

public:     ////                        Attributes.

    /**
     * Timeline name.
     */
    virtual char const*                 name() const                        = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace Sync
};  // namespace Utils
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_SYNC_H_

