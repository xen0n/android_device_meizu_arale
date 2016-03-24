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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <camera/MtkCamera.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <inc/CamUtils.h>
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
//
#include "inc/ZipImageCallbackThread.h"
using namespace NSMtkEngCamAdapter;
//
#include <sys/prctl.h>
//


/******************************************************************************
*
*******************************************************************************/

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[ZipImageCallbackThread] " fmt, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[ZipImageCallbackThread] " fmt, ##arg)
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
*******************************************************************************/
ZipImageCallbackThread::
ZipImageCallbackThread(
        IStateManager* pStateMgr,
        sp<CamMsgCbInfo> pCamMsgCbInfo
        )
    : Thread()
    , mpSelf(this)
    , mpszThreadName("ZipImageCallbackThread")
    , mpStateMgr(pStateMgr)
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mu4ShotMode(0)
    , mNextState(IState::eState_Idle)
    , mIsExtCallback(MFALSE)
{
    MY_LOGD("this %p create", this);
}


/******************************************************************************
*
*******************************************************************************/
ZipImageCallbackThread::
~ZipImageCallbackThread()
{
    MY_LOGD("this %p destroy", this);
}


/******************************************************************************
*
*******************************************************************************/
status_t
ZipImageCallbackThread::
readyToRun()
{
    ::prctl(PR_SET_NAME, mpszThreadName, 0, 0, 0);
    //
    int const expect_policy     = SCHED_OTHER;
    int const expect_priority   = NICE_CAMERA_ZIP_IMAGE_CB;
    int policy = 0, priority = 0;
    setThreadPriority(expect_policy, expect_priority);
    getThreadPriority(policy, priority);
    //
    MY_LOGD(
            "policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%x, 0x%x)"
            , expect_policy, policy, expect_priority, priority
            );
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
void
ZipImageCallbackThread::
setShotMode(uint32_t const u4ShotMode, const IState::ENState nextState)
{
    mu4ShotMode = u4ShotMode;
    mNextState = nextState;
    mIsExtCallback = MTRUE; // force mIsExtCallback to MTRUE in engineer mode
}


/******************************************************************************
*
*******************************************************************************/
void
ZipImageCallbackThread::
addCallbackData(callback_data* pData)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("type(%d)",pData->type);
    mqTodoCallback.push(*pData);
    mCondCallback.signal();
}


/******************************************************************************
*
*******************************************************************************/
bool
ZipImageCallbackThread::
getCallbackData(callback_data* pData)
{
    Mutex::Autolock _l(mLock);
    if( mqTodoCallback.size() == 0 ) {
        MY_LOGD("wait for callback");
        mCondCallback.wait(mLock);
    }

    if( mqTodoCallback.size() == 0 ) {
        MY_LOGD("no callback data");
        return false;
    }

    *pData = mqTodoCallback.front();
    mqTodoCallback.pop();

    // fake image to escape wating
    if( pData->pImage == NULL && (pData->type == ZipImageCallbackThread::callback_type_image))
    {
        return false;
    }

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ZipImageCallbackThread::
threadLoop()
{
    MY_LOGD("+");
    callback_data one;

    if  ( !getCallbackData(&one) )
    {
        MY_LOGW("couldn't get callback data, finish");
        mpSelf = 0;
        return false;
    }

    MY_LOGD("type(%d)",one.type);
    switch(one.type)
    {
        case callback_type_image:
        {
            if  ( one.isFinalImage )
            {
                MY_LOGD("the final image: wait done before callback");
                IStateManager::StateObserver stateWaiter(mpStateMgr);
                mpStateMgr->registerOneShotObserver(&stateWaiter);
                //
                if  ( OK != stateWaiter.waitState(mNextState) )
                {
                    MY_LOGW(
                            "do nothing due to fail to wait - Index:%d ShotMode:%d",
                            one.u4CallbackIndex, mu4ShotMode
                            );
                    return  false;
                }
            }

            if( mIsExtCallback )
            {
                MY_LOGD("MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE - Index:%d ShotMode:%d",
                        one.u4CallbackIndex, mu4ShotMode);
                mpCamMsgCbInfo->mDataCb(
                        MTK_CAMERA_MSG_EXT_DATA,
                        one.pImage,
                        0,
                        NULL,
                        mpCamMsgCbInfo->mCbCookie
                        );
                one.pImage->release(one.pImage);

                if  ( one.isFinalImage )
                {
                    mpCamMsgCbInfo->mNotifyCb(
                            MTK_CAMERA_MSG_EXT_NOTIFY,
                            MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE,
                            0,
                            mpCamMsgCbInfo->mCbCookie
                            );
                }
            }
            else
            {
                MY_LOGD("CAMERA_MSG_COMPRESSED_IMAGE - Index:%d ShotMode:%d",
                        one.u4CallbackIndex, mu4ShotMode);
                mpCamMsgCbInfo->mDataCb(
                        CAMERA_MSG_COMPRESSED_IMAGE,
                        one.pImage,
                        0,
                        NULL,
                        mpCamMsgCbInfo->mCbCookie
                        );
                one.pImage->release(one.pImage);
            }

            if  ( one.isFinalImage )
            {
                sp<ICam1DeviceCallback> pCam1DeviceCb = mpCamMsgCbInfo->mCam1DeviceCb.promote();
                if  ( pCam1DeviceCb != 0 ) {
                    pCam1DeviceCb->onCam1Device_NotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE, 0);
                }
            }

            if  (   one.isFinalImage &&
                    (   eShotMode_ContinuousShot == mu4ShotMode ||
                        eShotMode_ContinuousShotCc == mu4ShotMode) )
             {
                #if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
                MY_LOGD("Continuous shot end msg callback, total shot number is %d", one.u4CallbackIndex);
                mpCamMsgCbInfo->mNotifyCb(
                        MTK_CAMERA_MSG_EXT_NOTIFY,
                        MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END,
                        one.u4CallbackIndex,
                        mpCamMsgCbInfo->mCbCookie
                        );
                #endif
            }

            if( one.isFinalImage )
                mpSelf = 0;

            MY_LOGD("-");
            return  !one.isFinalImage;  // returns false, the thread will exit upon return.
        }
        case callback_type_shutter:
        {
            MY_LOGD("(%d)[ShutterThread] +", ::gettid());
            if(one.bMtkExt)
            {
                one.spCamMsgCbInfo->mNotifyCb(
                                        MTK_CAMERA_MSG_EXT_NOTIFY,
                                        MTK_CAMERA_MSG_EXT_NOTIFY_SHUTTER,
                                        one.i4PlayShutterSound,
                                        one.spCamMsgCbInfo->mCbCookie);
            }
            else
            {
                one.spCamMsgCbInfo->mNotifyCb(
                                        CAMERA_MSG_SHUTTER,
                                        0,
                                        0,
                                        one.spCamMsgCbInfo->mCbCookie);
            }
            MY_LOGD("(%d)[ShutterThread] -", ::gettid());
            break;
        }
        default:
        {
            break;
        }
    }
    //
    return true;
}
