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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPOOL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPOOL_H_
//
#include <utils/Timers.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <mtkcam/common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {


/**
 * @class IStreamBufferPool
 *
 * @param <_IBufferT_> the type of buffer interface.
 *  This type must have operations of incStrong and decStrong.
 */
template <class _IBufferT_>
class IStreamBufferPool : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Definitions.
    typedef _IBufferT_                  IBufferT;
    typedef android::sp<IBufferT>       SP_IBufferT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                        Disallowed.
    //  Copy constructor and Copy assignment are disallowed.
                                        IStreamBufferPool(IStreamBufferPool const&);
    IStreamBufferPool&                  operator=(IStreamBufferPool const&);

protected:  ////                        Instantiation.
                                        IStreamBufferPool() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Pool/Buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.

    /**
     * Pool name.
     */
    virtual char const*                 poolName() const                    = 0;

    /**
     * Dump information for debug.
     */
    virtual MVOID                       dumpPool() const                    = 0;

    /**
     * Initialize the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] maxNumberOfBuffers: maximum number of buffers which can be
     *  allocated from this pool.
     *
     * @param[in] minNumberOfInitialCommittedBuffers: minimum number of buffers
     *  which are initially committed.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      initPool(
                                            char const* szCallerName,
                                            size_t maxNumberOfBuffers,
                                            size_t minNumberOfInitialCommittedBuffers
                                        )                                   = 0;

    /**
     * Uninitialize the pool and free all buffers.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     */
    virtual MVOID                       uninitPool(
                                            char const* szCallerName
                                        )                                   = 0;

    /**
     * Commit all buffers in the pool.
     * This is a non-blocking call and will enforce to allocate buffers up to
     * the max. count in background.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      commitPool(
                                            char const* szCallerName
                                        )                                   = 0;

    /**
     * Try to acquire a buffer from the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[out] rpBuffer: a reference to a newly acquired buffer.
     *
     * @param[in] nsTimeout: a timeout in nanoseconds.
     *  timeout=0: this call will try acquiring a buffer and return immediately.
     *  timeout>0: this call will block to return until a buffer is acquired,
     *             the timeout expires, or an error occurs.
     *  timeout<0: this call will block to return until a buffer is acquired, or
     *             an error occurs.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      acquireFromPool(
                                            char const* szCallerName,
                                            SP_IBufferT& rpBuffer,
                                            nsecs_t nsTimeout
                                        )                                   = 0;

    /**
     * Release a buffer to the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] pBuffer: a buffer to release.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      releaseToPool(
                                            char const* szCallerName,
                                            SP_IBufferT pBuffer
                                        )                                   = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPOOL_H_

