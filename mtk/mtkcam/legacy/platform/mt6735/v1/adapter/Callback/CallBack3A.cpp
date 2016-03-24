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

#define LOG_TAG "CallBack3A"
//
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera/MtkCamera.h>
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <CallBack3A.h>
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
ICallBack3A*
ICallBack3A::
createInstance()
{
    return new CallBack3A();
}


/******************************************************************************
*
*******************************************************************************/
MVOID
ICallBack3A::
destroyInstance()
{
    delete this;
}


/******************************************************************************
*
*******************************************************************************/
CallBack3A::
CallBack3A()
    : mspCamMsgCbInfo(0)
    , mLock()
{
    //FUNC_NAME;
}


/******************************************************************************
*
*******************************************************************************/
CallBack3A::
~CallBack3A()
{
    //FUNC_NAME;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBack3A::
doNotifyCb(
    int32_t _msgType,
    int32_t _ext1,
    int32_t _ext2,
    int32_t _ext3,
    MINTPTR _ext4
)
{
    MY_LOGD_IF(0, "msgType(%d): +", _msgType);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mspCamMsgCbInfo == 0)
    {
        MY_LOGW("mspCamMsgCbInfo == 0");
        return;
    }
    //
    if( _msgType == I3ACallBack::eID_NOTIFY_AF_FOCUSED &&
        (mspCamMsgCbInfo->mMsgEnabled & (CAMERA_MSG_FOCUS)))
    {
        mspCamMsgCbInfo->mNotifyCb(CAMERA_MSG_FOCUS, _ext1, _ext2, mspCamMsgCbInfo->mCbCookie);
    }
    //
    if( _msgType == I3ACallBack::eID_NOTIFY_AF_MOVING &&
        (mspCamMsgCbInfo->mMsgEnabled & CAMERA_MSG_FOCUS_MOVE) &&
        mAFMoveCb)
    {
        mspCamMsgCbInfo->mNotifyCb(CAMERA_MSG_FOCUS_MOVE, _ext1, _ext2, mspCamMsgCbInfo->mCbCookie);
    }
    //
    //FUNC_END;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBack3A::
doDataCb(
    int32_t  _msgType,
    void*    _data,
    uint32_t  _size
)
{
    MY_LOGD_IF(0, "msgType(%d): +", _msgType);

    Mutex::Autolock lock(mLock);

    if( mspCamMsgCbInfo == 0)
    {
        MY_LOGW("mspCamMsgCbInfo == 0");
        return;
    }

    if ( _msgType == I3ACallBack::eID_DATA_AF_FOCUSED)
    {
        camera_memory* mem = mspCamMsgCbInfo->mRequestMemory(-1, _size + sizeof(int32_t), 1, NULL);
        if  ( mem )
        {
            if (mem->data && mem->size >= _size)
            {
                int32_t* pSrc = reinterpret_cast<int32_t*>(_data);
                int32_t* pDst = reinterpret_cast<int32_t*>(mem->data);

                pDst[0] = MTK_CAMERA_MSG_EXT_DATA_AF;
                for (uint32_t i = 0; i < _size/4; i++)
                {
                    pDst[i+1] = pSrc[i];
                }

                mspCamMsgCbInfo->mDataCb(MTK_CAMERA_MSG_EXT_DATA, mem, 0, NULL, mspCamMsgCbInfo->mCbCookie);
                mem->release(mem);
            }
        }
    }
    else
    {
        MY_LOGW("undefined");
    }
    //
    FUNC_END;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBack3A::
setUser(sp<CamMsgCbInfo> const &rpCamMsgCbInfo)
{
    Mutex::Autolock lock(mLock);
    mspCamMsgCbInfo = rpCamMsgCbInfo;
}


/******************************************************************************
*
*******************************************************************************/
void
CallBack3A::
destroyUser()
{
    Mutex::Autolock lock(mLock);
    if ( mspCamMsgCbInfo != 0 )
    {
        mspCamMsgCbInfo = 0;
    }
}



/******************************************************************************
*
*******************************************************************************/
void
CallBack3A::
enableAFMove(bool flag)
{
    Mutex::Autolock lock(mLock);
    mAFMoveCb = flag;
}

