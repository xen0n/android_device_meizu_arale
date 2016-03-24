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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_EFFECT_HAL_BASE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_EFFECT_HAL_BASE_H_
#include <IEffectHal.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace android {
    //class Vector;
    //class sp;
    class IGraphicBufferProducer;
};

namespace NSCam {
//class EffectHalBase : public IEffectHal
/**
 *  @brief EffectHalBase implement the IEffectHal inteface and declare pure virtual functions let feature owner implemenetation.
 *         
 */
class EffectHalBase : public IEffectHal
{
public:
                                EffectHalBase();
    virtual                     ~EffectHalBase();

public: // may change state

    /**
     *  @brief Change state to STATE_INIT and call initImpl() that overriding by derived class.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_UNINIT.
            - This function may change status to STATE_INIT.


     */
    virtual android::status_t   init() final;
    /**
     *  @brief Change state to STATE_UNINIT and call uninitImpl() that overriding by derived class.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_INIT.
            - This function may change status to STATE_UNINIT.
     */
    virtual android::status_t   uninit() final;

    /**
     *  @brief This function will check all capture parameters are setting done via allParameterConfigured() and change statue to STATE_CONFIGURED.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_INIT.
            - This function may change status to STATE_CONFIGURED.
    */
    virtual android::status_t   configure() final;
    /**
     *  @brief Release resource and change status to STATE_INIT.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_CONFIGURED.
            - This function may change status to STATE_INIT.
    */
    virtual android::status_t   unconfigure() final;
    /**
     *  @brief Start this session. Change state to STATE_RUNNING and call startImpl() that overriding by derived class.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_CONFIGURED.
            - This function may change status to STATE_RUNNING.
    */
    virtual uint64_t            start() final;
    /**
     *  @brief Abort this session. This function will change state to STATE_CONFIGURED and call abortImpl() that overriding by derived class.
     *  @details 
            - This function couldn’t be overridden anymore.
            - This function should be called on STATE_RUNNING.
            - This function may change status to STATE_CONFIGURED.
    */
    virtual android::status_t   abort(EffectParameter const *parameter=NULL) final;

public: // would not change state
    virtual android::status_t   getNameVersion(EffectHalVersion &nameVersion) const final;
    virtual android::status_t   setEffectListener(const android::wp<IEffectListener>&) final;
    virtual android::status_t   setParameter(android::String8 &key, android::String8 &object) final;
    virtual android::status_t   setParameters(android::sp<EffectParameter> parameter) final;
    virtual android::status_t   getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const final;
    //non-blocking
    virtual android::status_t   prepare() final;
    virtual android::status_t   release() final;
    //non-blocking
    virtual android::status_t   updateEffectRequest(const android::sp<EffectRequest> request) final;


public: //debug
public: //autotest
private:
    android::wp<IEffectListener>  mpListener;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
private:
    enum State {STATE_UNINIT        = 0x01
                , STATE_INIT        = 0x02
                , STATE_CONFIGURED  = 0x04
                , STATE_RUNNING     = 0x08
                };
    State               mState;
    bool                mPrepared;
    uint64_t            mUid;
    //android::Vector<android::sp<android::IGraphicBufferProducer> >  mOutputSurfaces;


protected:  //call those in sub-class
    android::status_t   prepareDone(const EffectResult& result, android::status_t state);
    // android::status_t   addInputFrameDone(EffectResult result, const android::sp<EffectParameter> parameter, android::status_t state);  //TTT3
    // android::status_t   addOutputFrameDone(EffectResult result, const android::sp<EffectParameter> parameter, android::status_t state); //TTT3
    // android::status_t   startDone(const EffectResult& result, const EffectParameter& parameter, android::status_t state);
    
protected:  //should be implement in sub-class
    virtual bool allParameterConfigured() = 0;
    
    virtual android::status_t   initImpl() = 0;
    virtual android::status_t   uninitImpl() = 0;
    //non-blocking
    virtual android::status_t   prepareImpl() = 0;
    virtual android::status_t   releaseImpl() = 0;

    virtual android::status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const = 0;
    virtual android::status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const = 0;
    virtual android::status_t   setParameterImpl(android::String8 &key, android::String8 &object) = 0;
    virtual android::status_t   setParametersImpl(android::sp<EffectParameter> parameter) = 0;
    virtual android::status_t   startImpl(uint64_t *uid=NULL) = 0;
    virtual android::status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL) = 0;
    //non-blocking
    virtual android::status_t   updateEffectRequestImpl(const android::sp<EffectRequest> request) = 0;
};

}   //namespace NSCam {

#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_EFFECT_HAL_BASE_H_
