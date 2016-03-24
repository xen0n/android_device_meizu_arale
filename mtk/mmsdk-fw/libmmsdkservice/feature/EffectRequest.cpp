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
#define LOG_TAG "mmsdk/EffectRequest"
//#include <cutils/xlog.h>
#include <cutils/log.h>
#include <common.h>
#include <cutils/properties.h>

//#include "HdrEffectHal.h"
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <mmsdk/EffectRequest.h>

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

// #define FUNCTION_LOG_START          MY_LOGD_IF(1<=mLogLevel, "+");
// #define FUNCTION_LOG_END            MY_LOGD_IF(1<=mLogLevel, "-");
#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, "+");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, "-");
/******************************************************************************
 *
 ******************************************************************************/



using namespace NSCam;
using namespace android;


const sp<EffectParameter>
EffectRequest::
getRequestParameter()
{
    FUNCTION_LOG_START;
    return mpRequestParameter;
    FUNCTION_LOG_END;
}

const sp<EffectResult>
EffectRequest::
getRequestResult()
{
    FUNCTION_LOG_START;

    //parameter = mpRequestResult;
    return mpRequestResult;
    FUNCTION_LOG_END;
}


status_t
EffectRequest::
setRequestParameter(sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    mpRequestParameter = parameter;
    return OK;
    FUNCTION_LOG_END;
}

status_t
EffectRequest::
setRequestResult(sp<EffectResult> result)
{
    FUNCTION_LOG_START;
    mpRequestResult = result;
    return OK;
    FUNCTION_LOG_END;
}


/******************************************************************************
 *
 * @class EffectFrameInfo
 * @brief
 * @details
 *
 *
 ******************************************************************************/
EffectFrameInfo::
EffectFrameInfo(
        const EffectFrameInfo& other) :
        mRequestNo(other.mRequestNo),
        mFrameNo(other.mFrameNo),
        mpOnFrameProcessed(other.mpOnFrameProcessed),
        mpTag(other.mpTag),
        mIsFrameReady(other.mIsFrameReady),
        mFrame(other.mFrame),
        mpFrameParameter(other.mpFrameParameter),
        mpFrameResult(other.mpFrameResult) {
}



bool
EffectFrameInfo::
isFrameBufferReady()
{
    FUNCTION_LOG_START;
    return mIsFrameReady;
    FUNCTION_LOG_END;
}

status_t
EffectFrameInfo::
getFrameBuffer(sp<IImageBuffer>& frame)
{
    FUNCTION_LOG_START;
    if (mFrame != NULL)
    {
        frame = mFrame;
    }
    return OK;
    FUNCTION_LOG_END;
}


sp<EffectParameter>
EffectFrameInfo::
getFrameParameter()
{
    FUNCTION_LOG_START;
    return mpFrameParameter;
    FUNCTION_LOG_END;
}

sp<EffectResult>
EffectFrameInfo::
getFrameResult()
{
    FUNCTION_LOG_START;
    return mpFrameResult;
    FUNCTION_LOG_END;
}

status_t
EffectFrameInfo::
setFrameBuffer(sp<IImageBuffer> frame)
{
    FUNCTION_LOG_START;
    mFrame = frame;
    return OK;
    FUNCTION_LOG_END;
}

status_t
EffectFrameInfo::
setFrameParameter(sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    mpFrameParameter = parameter;
    return OK;
    FUNCTION_LOG_END;
}

status_t
EffectFrameInfo::
setFrameResult(sp<EffectResult> result)
{
    FUNCTION_LOG_START;
    mpFrameResult = result;
    return OK;
    FUNCTION_LOG_END;
}

status_t
EffectFrameInfo::
setFrameReady(bool ready)
{
    FUNCTION_LOG_START;
    mIsFrameReady = ready;
    FUNCTION_LOG_END;
    return OK;
}
