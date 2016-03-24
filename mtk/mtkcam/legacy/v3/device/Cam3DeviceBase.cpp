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

#define LOG_TAG "MtkCam/Cam3Device"
//
#include "MyUtils.h"
#include <mtkcam/device/Cam3DeviceBase.h>
//
using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[Cam3DeviceBase::%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
Cam3DeviceBase::
Cam3DeviceBase(
    String8 const&          rDevName,
    int32_t const           i4OpenId
)
    : Cam3Device()
    //
    , mDevOpsLock()
    , mpDeviceManager(NULL)
    , mDevName(rDevName)
    , mi4OpenId(i4OpenId)
    , mpCallbackOps(NULL)
    //
{
    char cLogLevel[PROPERTY_VALUE_MAX] = {0};
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("debug.camera.log.Cam3Device", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
    MY_LOGD("LogLevel:%d", mLogLevel);
}


/******************************************************************************
 *
 ******************************************************************************/
Cam3DeviceBase::
~Cam3DeviceBase()
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Cam3DeviceBase::
getLogLevel() const
{
    return mLogLevel;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam3DeviceBase::
setDeviceManager(ICamDeviceManager* manager)
{
    mpDeviceManager = manager;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam3DeviceBase::
i_closeDevice()
{
    MY_LOGD("");
    i_uninitialize();
    return  mpDeviceManager->close(this);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam3DeviceBase::
i_initialize(
    camera3_callback_ops const* callback_ops
)
{
    Mutex::Autolock _DevOpsLock(mDevOpsLock);
    //
    mpCallbackOps = callback_ops;
    //
    return  onInitializeLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam3DeviceBase::
i_uninitialize()
{
    Mutex::Autolock _DevOpsLock(mDevOpsLock);
    //
    return  onUninitializeLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam3DeviceBase::
i_configure_streams(
    camera3_stream_configuration_t* stream_list
)
{
    Mutex::Autolock _DevOpsLock(mDevOpsLock);
    return  onConfigureStreamsLocked(stream_list);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam3DeviceBase::
i_flush()
{
    Mutex::Autolock _DevOpsLock(mDevOpsLock);
    return  onFlushLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam3DeviceBase::
i_dump(int /*fd*/)
{
    MY_LOGW("[TODO] Cam3DeviceBase::i_dump");
}

