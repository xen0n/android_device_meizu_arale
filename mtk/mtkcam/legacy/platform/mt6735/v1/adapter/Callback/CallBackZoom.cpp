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

#define LOG_TAG "CallBackZoom"
//
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera/MtkCamera.h>
#include <mtkcam/v1/IParamsManager.h>
//
#include <mtkcam/UITypes.h>
using namespace NSCam;
//
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;
//
#include <vector>
using namespace std;
//
#include <mtkcam/camnode/IspSyncControl.h>
using namespace NSCamNode;
//
#include <CallBackZoom.h>
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), LOG_TAG, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), LOG_TAG, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), LOG_TAG, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), LOG_TAG, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] " fmt, ::gettid(), LOG_TAG, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
/******************************************************************************
*
*******************************************************************************/
ICallBackZoom*
ICallBackZoom::
createInstance()
{
    return new CallBackZoom();
}


/******************************************************************************
*
*******************************************************************************/
MVOID
ICallBackZoom::
destroyInstance()
{
    delete this;
}


/******************************************************************************
*
*******************************************************************************/
CallBackZoom::
CallBackZoom()
    : mLock()
    , mspCamMsgCbInfo(0)
{
    //FUNC_NAME;
}


/******************************************************************************
*
*******************************************************************************/
CallBackZoom::
~CallBackZoom()
{
    //FUNC_NAME;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBackZoom::
doNotifyCb(
    int32_t _msgType,
    int32_t _ext1,
    int32_t _ext2,
    int32_t _ext3)
{
    Mutex::Autolock lock(mLock);
    //
    bool retStop = false;
    //
    if(mspCamMsgCbInfo == 0)
    {
        MY_LOGW("mspCamMsgCbInfo == 0");
        return;
    }
    //
    if(mPrevIdx == _ext1)
    {
        MY_LOGW("same as previous idx, drop!");
        return;
    }
    //
    if( _msgType == eID_NOTIFY_Zoom &&
        _ext1 != mIniIdx)
    {
        retStop = (mRequestStop)||(_ext1 == mTargetIdx);
        MY_LOGD("smoothZoom cb(#%d): (%d, %d)", mCBcnt++, _ext1, retStop);
        mspParamsMgr->set(
                        CameraParameters::KEY_ZOOM,
                        _ext1);
        mpIspSyncCtrl->setCurZoomRatio(mspParamsMgr->getZoomRatioByIndex(_ext1));
        mspCamMsgCbInfo->mNotifyCb(
                            CAMERA_MSG_ZOOM,
                            _ext1,
                            retStop,
                            mspCamMsgCbInfo->mCbCookie);
        mPrevIdx = _ext1;
    }
    //
    if(retStop)
    {
        mCondReqStop.broadcast();
        reset();
    }
}


/******************************************************************************
*
*******************************************************************************/
void
CallBackZoom::
reset()
{
    mspCamMsgCbInfo = 0;
    mspParamsMgr = 0;
    mpIspSyncCtrl = NULL;
    mTargetIdx = mIniIdx = mCBcnt = 0;
    mPrevIdx = -1;
    mRequestStop = false;
}


/******************************************************************************
*
*******************************************************************************/
bool
CallBackZoom::
requestStop()
{
    Mutex::Autolock lock(mLock);
    //
    mRequestStop = true;
    nsecs_t nsTimeoutToWait = 33LL*1000LL*1000LL;//wait 33 msecs.
    status_t status = mCondReqStop.waitRelative(mLock, nsTimeoutToWait);
    //
    return status == OK;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBackZoom::
setZoomTarget(
    int32_t cIdx,
    int32_t tIdx)
{
    mIniIdx = cIdx;
    mTargetIdx = tIdx;
    mPrevIdx = -1;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBackZoom::
setUser(
    sp<CamMsgCbInfo> const &rpCamMsgCbInfo,
    sp<IParamsManager> const &rpParamsMgr,
    IspSyncControl* pIspSyncCtrl)
{
    Mutex::Autolock lock(mLock);
    //
    reset();
    mspCamMsgCbInfo = rpCamMsgCbInfo;
    mspParamsMgr = rpParamsMgr;
    mpIspSyncCtrl = pIspSyncCtrl;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBackZoom::
destroyUser()
{
    Mutex::Autolock lock(mLock);
    reset();
}


