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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTLISTENER_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTLISTENER_H_

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <BasicParameters.h>
// #include <mmsdk/IEffectHal.h>

namespace NSCam
{
using namespace android;

class IEffectHalClient;






//-----------------------------------------------------------------------------
//IEffectFactory
//-----------------------------------------------------------------------------
/**
 *  @brief IEffectListener is the interface through which the feature owner notifies
 *         the register(AP, IEffectHalClient) of events that the register may wish to react to.
 */
class IEffectListener : public IInterface
{
public:
    DECLARE_META_INTERFACE(EffectListener);


public:
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::prepare().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       Prepared result
     */
    //TTT4 virtual void    onPrepared(const sp<IEffectHal> effect, const EffectResult& result) const = 0;
    virtual void    onPrepared(const IEffectHalClient* effectClient, const EffectResult& result) const = 0;
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::addInputFrame() operation.
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] parameter    the parameter feed in IEffectHal::addInputFrame()
     *  @param[in] partialResult partial Result
     */
    //TTT4 virtual void    onInputFrameProcessed(const sp<IEffectHal> effect, const sp<EffectParameter> parameter, EffectResult partialResult) const = 0;
    virtual void    onInputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult) = 0;
    /**
     *  @brief                  IEffectHal object call while complete frame processed, and update the content of frame that feed by IEffectHal::addOutputFrame().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] parameter    the parameter feed in IEffectHal::addOutputFrame()
     *  @param[in] partialResult partial Result
     */
    //TTT4 virtual void    onOutputFrameProcessed(const sp<IEffectHal> effect, const sp<EffectParameter> parameter, EffectResult partialResult) const = 0;
    virtual void    onOutputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult) = 0;
    /**
     *  @brief                  IEffectHal object call for an end of a session.
     *  @details                EX: (HDR) Call after complete blending frames into single frame.
     *                          If client add two frames by IEffectHal::addInputFrame() , the expected
     *                          call back sequence will be onInputFrameProcessed() + onInputFrameProcessed() +
     *                          onOutputFrameProcessed() + onCompleted()
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] partialResult partial Result
     *  @param[in] uid          The session id set by IEffectHal::start()
     */
    //TTT4 virtual void    onCompleted(const sp<IEffectHal> effect, const EffectResult& partialResult, uint64_t uid) const = 0;
    virtual void    onCompleted(const IEffectHalClient* effectClient, const EffectResult& partialResult, uint64_t uid) const = 0;
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::abort().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       Abort result.
     */
    //TTT4 virtual void    onAborted(const sp<IEffectHal> effect, const EffectResult& result) const = 0;
    virtual void    onAborted(const IEffectHalClient* effectClient, const EffectResult& result) = 0;
    /**
     *  @brief                  Might be called if anything wrong in IEffectHal
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       A result represents error type.
     *  @param[in] errorData    The address that point to errorData
     */
    //TTT4 virtual void    onFailed(const sp<IEffectHal> effect, const EffectResult& result) const = 0;
    virtual void    onFailed(const IEffectHalClient* effectClient, const EffectResult& result) const = 0;
};


class BnEffectListener : public BnInterface<IEffectListener>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};


/**
 *  @brief IEffectListener is the interface through which the feature owner notifies
 *         the register(AP, IEffectHalClient) of events that the register may wish to react to.
 */
class EffectListener : public BnEffectListener
{
//protected:
public:
    virtual         ~EffectListener() {};
public:
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::prepare().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       Prepared result
     */
    //TTT4 virtual void    onPrepared(const sp<IEffectHal> effect, const EffectResult& result) const {};
    virtual void    onPrepared(const IEffectHalClient* effectClient, const EffectResult& result) const {};
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::addInputFrame() operation.
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] parameter    the parameter feed in IEffectHal::addInputFrame()
     *  @param[in] partialResult partial Result
     */
    //TTT4 virtual void    onInputFrameProcessed(const sp<IEffectHal> effect, const sp<EffectParameter> parameter, EffectResult partialResult) const {};
    virtual void    onInputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult) {};
    /**
     *  @brief                  IEffectHal object call while complete frame processed, and update the content of frame that feed by IEffectHal::addOutputFrame().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] parameter    the parameter feed in IEffectHal::addOutputFrame()
     *  @param[in] partialResult partial Result
     */
    //TTT4 virtual void    onOutputFrameProcessed(const sp<IEffectHal> effect, const sp<EffectParameter> parameter, EffectResult partialResult) const {};
    virtual void    onOutputFrameProcessed(const IEffectHalClient* effectClient, const sp<EffectParameter> parameter, EffectResult partialResult) {};
    /**
     *  @brief                  IEffectHal object call for an end of a session.
     *  @details                EX: (HDR) Call after complete blending frames into single frame.
     *                          If client add two frames by IEffectHal::addInputFrame() , the expected
     *                          call back sequence will be onInputFrameProcessed() + onInputFrameProcessed() +
     *                          onOutputFrameProcessed() + onCompleted()
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] partialResult partial Result
     *  @param[in] uid          The session id set by IEffectHal::start()
     */
    //TTT4 virtual void    onCompleted(const sp<IEffectHal> effect, const EffectResult& partialResult, uint64_t uid) const {};
    virtual void    onCompleted(const IEffectHalClient* effectClient, const EffectResult& partialResult, uint64_t uid) const {};
    /**
     *  @brief                  IEffectHal object call while complete IEffectHal::abort().
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       Abort result.
     */
    //TTT4 virtual void    onAborted(const sp<IEffectHal> effect, const EffectResult& result) const {};
    virtual void    onAborted(const IEffectHalClient* effectClient, const EffectResult& result) {};
    /**
     *  @brief                  Might be called if anything wrong in IEffectHal
     *  @param[in] effect       effectClient points to the specific IEffectHalClient object or is NULL
     *  @param[in] result       A result represents error type.
     */
    //TTT4 virtual void    onFailed(const sp<IEffectHal> effect, const EffectResult& result) const {};
    virtual void    onFailed(const IEffectHalClient* effectClient, const EffectResult& result) const {};
};
} //end NSCam

#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_IEFFECTLISTENER_H_