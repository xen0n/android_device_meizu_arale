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
#define LOG_TAG "mmsdk/EffectHalBase"

#include <cutils/log.h>
#include <utils/Errors.h>
#include <gui/IGraphicBufferProducer.h>
#include <mmsdk/EffectHalBase.h>

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E. , mState=%d, this=%p", __FUNCTION__, mState, this)
#define FUNCTION_LOG_END        ALOGD("[%s] - X. , mState=%d", __FUNCTION__, mState)
#define aaa 1

using namespace NSCam;
using namespace android;

#define AT_STATE(candidate)   (mState & (candidate))


//-----------------------------------------------------------------------------
//public: // may change state
//-----------------------------------------------------------------------------
EffectHalBase::
EffectHalBase() : mState(STATE_UNINIT), mPrepared(0), mUid(0)
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
}


EffectHalBase::
~EffectHalBase()
{
    FUNCTION_LOG_START;
    mpListener = NULL;
    FUNCTION_LOG_END;
}


status_t
EffectHalBase::
init()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_UNINIT))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    ret = initImpl();

    mState = STATE_INIT;
    mUid = 0;

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
uninit()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_INIT))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    //
    ret = uninitImpl();
    mState = STATE_UNINIT;

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
configure()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_INIT))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    if(allParameterConfigured())
    {
        mState = STATE_CONFIGURED;
    }

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
unconfigure()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_CONFIGURED))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    ret = release();
    if(ret == OK)
    {
        mState = STATE_INIT;
    }

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


uint64_t
EffectHalBase::
start()
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    static uint64_t uid = 0;

    // check state machine
    if(!AT_STATE(STATE_CONFIGURED))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    ++uid;
    // action
    ret = startImpl(&uid);
    if(ret == OK)
    {
        mState = STATE_RUNNING;
        mUid = uid;
    }
    ALOGD("[%s]: uid=%" PRIu64 ", mState=%d", __FUNCTION__, mUid, mState);

FUNCTION_END:
    FUNCTION_LOG_END;
    //return ret;
    return mUid;
}


// status_t
// EffectHalBase::
// startDone(const EffectResult &result, const EffectParameter &parameter, android::status_t state)
// {
//     FUNCTION_LOG_START;
//     status_t ret = state;

//     //@todo implement this
//     //set result
//     //set parameter

//     // listener
//     sp<IEffectListener> listener = mpListener.promote();
//     if(listener != 0)
//     {
//         if(ret == OK)
//         {
//             ALOGD("[%s]: uid=%d", __FUNCTION__, mUid);
//             listener->onCompleted(NULL, result, mUid);
//         }
//         else
//         {
//             listener->onFailed(NULL, result);
//         }
//     }

// FUNCTION_END:
//     FUNCTION_LOG_END;
//     return ret;
// }


status_t
EffectHalBase::
abort(EffectParameter const *parameter)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    EffectResult result;
    sp<IEffectListener> listener;

    // check state machine
    if(!AT_STATE(STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    // action
    ret = abortImpl(result, parameter);
    if(ret == OK)
    {
        mState = STATE_CONFIGURED;
    }

    // listener
    listener = mpListener.promote();

    if(listener != 0)
    {
        if(ret == OK)
        {
            listener->onAborted(NULL, result);
        }
        else
        {
            listener->onFailed(NULL, result);
        }
    }


FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}



//-----------------------------------------------------------------------------
//public: // would not change state
//-----------------------------------------------------------------------------
status_t
EffectHalBase::
getNameVersion(EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_UNINIT|STATE_INIT|STATE_CONFIGURED|STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    //
    ret = getNameVersionImpl(nameVersion);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
setEffectListener(const android::wp<IEffectListener> &listener)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_INIT|STATE_CONFIGURED))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    mpListener = listener;

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
setParameter(android::String8 &key, android::String8 &object)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_INIT|STATE_CONFIGURED|STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }
    //@todo make sure paramter could be change is this state.
    //e.g. parameters related with capture session are only allowed in STATE_INIT

    // action
    ALOGD("SET_PARAMETER key=%s, value:%s", key.string(), object.string());
    ret = setParameterImpl(key, object);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

status_t
EffectHalBase::
setParameters(sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_INIT|STATE_CONFIGURED|STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }
    // action
    ret = setParametersImpl(parameter);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}

status_t
EffectHalBase::
getCaptureRequirement(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_CONFIGURED|STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    // action
    ret = getCaptureRequirementImpl(inputParam, requirements);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


//non-blocking
status_t
EffectHalBase::
prepare()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_CONFIGURED))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }
    if(mPrepared)
    {
        ALOGD("skip prepare action since already prepared");
        goto FUNCTION_END;
    }

    //
    ret = prepareImpl();    //@todo use thread to handle this

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


status_t
EffectHalBase::
prepareDone(const EffectResult &result, status_t state)
{
    FUNCTION_LOG_START;
    status_t ret = state;

    // action
    if(ret == OK)
    {
        mPrepared = true;
    }
    ALOGD("prepareDone mPrepared=%d", mPrepared);

    // listener
    sp<IEffectListener> listener = mpListener.promote();
    if(listener != 0)
    {
        if(ret == OK)
        {
            listener->onPrepared(NULL, result);
        }
        else
        {
            listener->onFailed(NULL, result);
        }
    }

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}



status_t
EffectHalBase::
release()
{
    FUNCTION_LOG_START;
    status_t ret = OK;

    // check state machine
    if(!AT_STATE(STATE_CONFIGURED))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }
    if(!mPrepared)
    {
        ALOGD("skip relase action since not prepared");
        goto FUNCTION_END;
    }

    //
    ret = releaseImpl();
    if(ret == OK)
    {
        mPrepared = false;
    }

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}


//non-blocking
status_t
EffectHalBase::
updateEffectRequest(const sp<EffectRequest> request)
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    //EffectResult result;

    // check state machine
    if(!AT_STATE(STATE_RUNNING))
    {
        ret = INVALID_OPERATION;
        ALOGE("can't call this function at state %d", mState);
        goto FUNCTION_END;
    }

    // action
    ret = updateEffectRequestImpl(request);

FUNCTION_END:
    FUNCTION_LOG_END;
    return ret;
}
