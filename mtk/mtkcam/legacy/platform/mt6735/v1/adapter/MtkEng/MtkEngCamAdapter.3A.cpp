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
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkEngCamAdapter.h>
using namespace NSMtkEngCamAdapter;
//
#include <cutils/properties.h>
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
init3A()
{
    status_t ret = OK;
    //
    MY_LOGD("+");
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    if(mpCallback3a == NULL)
    {
        mpCallback3a = ICallBack3A::createInstance();
        if(mpCallback3a == NULL)
        {
            MY_LOGE("mpCallback3a == NULL");
            return INVALID_OPERATION;
        }
    }
    //
    mpCallback3a->setUser(mpCamMsgCbInfo);
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }

    if ( ! pHal3a->addCallbacks(mpCallback3a) )
    {
        MY_LOGE("setCallbacks fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    //
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
uninit3A()
{
    MY_LOGD("");
    #if '1'==MTKCAM_HAVE_3A_HAL
    if(mpCallback3a)
    {
        IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());
        if (pHal3a)
        {
            pHal3a->removeCallbacks(mpCallback3a);
            pHal3a->destroyInstance(getName());
        }
        //
        mpCallback3a->destroyUser();
        mpCallback3a->destroyInstance();
        mpCallback3a = NULL;
    }
    #endif
}


/******************************************************************************
*  This method is only valid when preview is active
*******************************************************************************/
status_t
CamAdapter::
autoFocus()
{
    MY_LOGD("+");
    //
    status_t ret = OK;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }

    if ( ! pHal3a->autoFocus() )
    {
        MY_LOGE("autoFocus fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return ret;
}


/******************************************************************************
* Cancels any auto-focus function in progress.
* Whether or not auto-focus is currently in progress
*******************************************************************************/
status_t
CamAdapter::
cancelAutoFocus()
{
    MY_LOGD("+");
    status_t ret = OK;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }

    if ( ! pHal3a->cancelAutoFocus() )
    {
        MY_LOGE("cancelAutoFocus fail");
        // do not return error since 3a may not have received autofocus
        //ret = INVALID_OPERATION;
        //goto lbExit;
    }
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return ret;

}


/******************************************************************************
*
*
*******************************************************************************/
void
CamAdapter::enableAFMove(bool flag)
{
    #if '1'==MTKCAM_HAVE_3A_HAL
    if(mpCallback3a)
    {
        mpCallback3a->enableAFMove(flag);
    }
    #endif
}



