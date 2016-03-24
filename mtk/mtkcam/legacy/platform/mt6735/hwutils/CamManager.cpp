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

#define LOG_TAG "MtkCam/Utils/CamMgr"
//
#include <mtkcam/Log.h>
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;


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
//
static CamManager singleton;
//
/******************************************************************************
 *
 ******************************************************************************/
CamManager*
CamManager::
getInstance()
{
    return &singleton;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
incDevice()
{
    Mutex::Autolock _l(mLockMtx);
    //
    mDeviceCnt++;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
decDevice()
{
    Mutex::Autolock _l(mLockMtx);
    //
    mDeviceCnt--;
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
getDeviceCount() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    return mDeviceCnt;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setFrameRate(uint32_t const id, uint32_t const frameRate)
{
    Mutex::Autolock _l(mLockFps);
    //
    if (id == 0) {
        mFrameRate0 = frameRate;
    }
    else if (id == 1) {
        mFrameRate1 = frameRate;
    }
    else {
        MY_LOGE("id(%d), frameRate(%d)", id, frameRate);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
CamManager::
getFrameRate(uint32_t const id) const
{
    Mutex::Autolock _l(mLockFps);
    //
    uint32_t frameRate = (id == 0) ? mFrameRate0 : mFrameRate1;
    return (mDeviceCnt > 1) ? frameRate : 0;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
isMultiDevice() const
{
    Mutex::Autolock _l(mLockFps);
    //
    return (mDeviceCnt > 1) ? true : false;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setRecordingHint(bool const isRecord)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbRecord = isRecord;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setAvailableHint(bool const isAvailable)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbAvailable = isAvailable;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CamManager::
setStereoHint(bool const isStereo)
{
    Mutex::Autolock _l(mLockMtx);
    //
    mbStereo = isStereo;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamManager::
getPermission() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    MY_LOGD("mDeviceCnt(%d), mbRecord(%d), mbAvailable(%d), mbStereo(%d), 0:fps(%d); 1:fps(%d)",
            mDeviceCnt, mbRecord, mbAvailable, mbStereo, getFrameRate(0), getFrameRate(1));
    return !mbRecord && mbAvailable && !mbStereo;
}
