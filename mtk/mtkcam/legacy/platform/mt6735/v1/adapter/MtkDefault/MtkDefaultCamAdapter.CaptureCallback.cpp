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
using namespace MtkCamUtils;
using namespace NSCam::Utils;
//
#include <inc/ImgBufProvidersManager.h>
#include <mtkcam/v1/IParamsManager.h>

//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkDefaultCamAdapter.h"
using namespace NSMtkDefaultCamAdapter;
//
#include <mtkcam/drv/hwutils.h>
//
#include <sys/prctl.h>
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
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
//
//  Callback of Error (CAMERA_MSG_ERROR)
//
//  Arguments:
//      ext1
//          [I] extend argument 1.
//
//      ext2
//          [I] extend argument 2.
//
bool
CamAdapter::
onCB_Error(
    int32_t ext1,
    int32_t ext2
)
{
    MY_LOGW("CAMERA_MSG_ERROR %d %d", ext1, ext2);
    mpCamMsgCbInfo->mNotifyCb(CAMERA_MSG_ERROR, ext1, ext2, mpCamMsgCbInfo->mCbCookie);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Shutter (CAMERA_MSG_SHUTTER)
//
//      Invoking this callback may play a shutter sound.
//
//  Arguments:
//      bPlayShutterSound
//          [I] Play a shutter sound if ture; otherwise play no sound.
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one shutter callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
bool
CamAdapter::
onCB_Shutter(
    bool const bPlayShutterSound,
    uint32_t const u4CallbackIndex
)
{
    if  ( msgTypeEnabled(CAMERA_MSG_SHUTTER) )
    {
        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            return false;
        }
        //
        pThread->doCallback(
                bPlayShutterSound ?
                ZipImageCallbackThread::callback_type_shutter :
                ZipImageCallbackThread::callback_type_shutter_nosound
                );
        pThread = NULL;
    }
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Postview for Display
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      u4PostviewSize
//          [I] Postview buffer size in bytes.
//
//      puPostviewBuf
//          [I] Postview buffer with its size = u4PostviewSize
//
bool
CamAdapter::
onCB_PostviewDisplay(
    int64_t const   i8Timestamp,
    uint32_t const  u4PostviewSize,
    uint8_t const*  puPostviewBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);
#if 1
    // when do ZSD+HDR/MFLL, this funciton will cause preview fress after take shots,
    // cause by the display buffer need to rerturn in order, and due to this function during ZSD extention mode
    // the return order will not in order
    if (mShotMode == eShotMode_ZsdHdrShot || mShotMode == eShotMode_ZsdMfllShot)
    {
        return MTRUE;
    }
    //
    if  ( ! u4PostviewSize || ! puPostviewBuf )
    {
        MY_LOGW("Bad callback: size/buf=%d/%p", u4PostviewSize, puPostviewBuf);
        return  false;
    }
    //
    sp<IImgBufProvider> pImgBufPvdr = mpImgBufProvidersMgr->getProvider(IImgBufProvider::eID_DISPLAY);
    if  ( pImgBufPvdr == 0 )
    {
        MY_LOGW("Bad IImgBufProvider");
        return  false;
    }
    //
    ImgBufQueNode node;
    if  ( ! pImgBufPvdr->dequeProvider(node) )
    {
        MY_LOGW("dequeProvider fail");
        return  false;
    }
    //
    sp<IImgBuf> pImgBuf = node.getImgBuf();
    if  ( u4PostviewSize != pImgBuf->getBufSize() )
    {
        MY_LOGW(
            "callback size(%d) != display:[%d %s %dx%d]",
            u4PostviewSize, pImgBuf->getBufSize(), pImgBuf->getImgFormat().string(),
            pImgBuf->getImgWidth(), pImgBuf->getImgHeight()
        );
        node.setStatus(ImgBufQueNode::eSTATUS_CANCEL);
    }
    else
    {
        ::memcpy(pImgBuf->getVirAddr(), puPostviewBuf, u4PostviewSize);
        globalcacheFlushAll();
        MY_LOGD_IF(1, "- globalcacheFlushAll()");
        //
        pImgBuf->setTimestamp(i8Timestamp);
        node.setStatus(ImgBufQueNode::eSTATUS_DONE);
    }
    //
    if  ( ! pImgBufPvdr->enqueProvider(node) )
    {
        MY_LOGW("enqueProvider fail");
        return  false;
    }
    //
#endif
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      u4PostviewSize
//          [I] Postview buffer size in bytes.
//
//      puPostviewBuf
//          [I] Postview buffer with its size = u4PostviewSize
//
bool
CamAdapter::
onCB_PostviewClient(
    int64_t const   i8Timestamp,
    uint32_t const  u4PostviewSize,
    uint8_t const*  puPostviewBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);

    MY_LOGW("Not implement yet");

    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Raw Image (CAMERA_MSG_RAW_IMAGE/CAMERA_MSG_RAW_IMAGE_NOTIFY)
//
//  Arguments:
//      i8Timestamp
//          [I] Raw image timestamp
//
//      u4RawImgSize
//          [I] Raw image buffer size in bytes.
//
//      puRawImgBuf
//          [I] Raw image buffer with its size = u4RawImgSize
//
bool
CamAdapter::
onCB_RawImage(
    int64_t const   i8Timestamp,
    uint32_t const  u4RawImgSize,
    uint8_t const*  puRawImgBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4RawImgSize, puRawImgBuf);
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) )
    {
        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            return false;
        }
        //
        pThread->doCallback(ZipImageCallbackThread::callback_type_raw_notify);
        pThread = NULL;
        return  true;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) )
    {
        camera_memory* pmem = NULL;
        if  ( ! u4RawImgSize || ! puRawImgBuf )
        {
            MY_LOGD("raw dummy callback");
            pmem = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, NULL);
        }
        else
        {
            pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4RawImgSize, 1, NULL);
            if  ( pmem )
            {
                ::memcpy(pmem->data, puRawImgBuf, u4RawImgSize);
            }
        }

        if  ( pmem )
        {
            sp<ZipImageCallbackThread> pThread = NULL;
            //
            pThread = mpZipCallbackThread.promote();
            if  ( pThread.get() == 0 )
            {
                MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
                return false;
            }
            //
            ZipImageCallbackThread::callback_data callbackData =
            {
                ZipImageCallbackThread::callback_type_raw,
                pmem,
                0,
                false
            };
            pThread->addCallbackData(&callbackData);
            //
            pThread->doCallback(ZipImageCallbackThread::callback_type_raw);
            pThread = NULL;
        }
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
//
//      [Compressed Image] = [Header] + [Bitstream],
//      where
//          Header may be jpeg exif (including thumbnail)
//
//  Arguments:
//      i8Timestamp
//          [I] Compressed image timestamp
//
//      u4BitstreamSize
//          [I] Bitstream buffer size in bytes.
//
//      puBitstreamBuf
//          [I] Bitstream buffer with its size = u4BitstreamSize
//
//      u4HeaderSize
//          [I] Header size in bytes; header may be jpeg exif.
//
//      puHeaderBuf
//          [I] Header buffer with its size = u4HeaderSize
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one compressed callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
//      fgIsFinalImage
//          [I] booliean value to indicate whether it is the final image.
//              true if this is the final image callback; otherwise false.
//              For single captures, this value must be true.
//
bool
CamAdapter::
onCB_CompressedImage(
    int64_t const   i8Timestamp,
    uint32_t const  u4BitstreamSize,
    uint8_t const*  puBitstreamBuf,
    uint32_t const  u4HeaderSize,
    uint8_t const*  puHeaderBuf,
    uint32_t const  u4CallbackIndex,
    bool            fgIsFinalImage,
    uint32_t const  msgType
)
{
    bool ret = true;
    camera_memory* image = NULL;
    uint8_t* pImage = NULL;
    uint32_t const u4DataSize = u4HeaderSize + u4BitstreamSize + sizeof(uint32_t)*(1+1);
    sp<ZipImageCallbackThread> pThread = NULL;
    //
    MY_LOGD(
        "timestamp(%lld), bitstream:size/buf=%d/%p, header:size/buf=%d/%p, index(%d), IsFinalImage(%d)",
        i8Timestamp, u4BitstreamSize, puBitstreamBuf, u4HeaderSize, puHeaderBuf, u4CallbackIndex, fgIsFinalImage
    );
    //
    if  ( ! msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) )
    {
        MY_LOGW("msgTypeEnabled=%#x", msgTypeEnabled(0xFFFFFFFF));
        goto lbExit;
    }
    //
    pThread = mpZipCallbackThread.promote();
    if  ( pThread.get() == 0 )
    {
        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
        ret = false;
        goto lbExit;
    }
    //
    if ( pThread->isExtCallback() )
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4DataSize, 1, NULL);
        if  ( image )
        {
            uint32_t*const pCBData = reinterpret_cast<uint32_t*>(image->data);
            pCBData[0] = msgType;
            pCBData[1] = u4CallbackIndex;
            pImage = reinterpret_cast<uint8_t*>(&pCBData[2]);
        }
    }
    else
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4HeaderSize + u4BitstreamSize, 1, NULL);
        if  ( image )
        {
            pImage = reinterpret_cast<uint8_t*>(image->data);
        }
    }

    if  ( ! image )
    {
        MY_LOGW("mRequestMemory fail");
        ret = false;
        goto lbExit;
    }
    //
    if  ( image )
    {
        if  ( 0 != u4HeaderSize && 0 != puHeaderBuf )
        {
            ::memcpy(pImage, puHeaderBuf, u4HeaderSize);
            pImage += u4HeaderSize;
        }
        if  ( 0 != u4BitstreamSize && 0 != puBitstreamBuf )
        {
            ::memcpy(pImage, puBitstreamBuf, u4BitstreamSize);
        }
        //
        ZipImageCallbackThread::callback_data callbackData =
        {
            ZipImageCallbackThread::callback_type_jpeg,
            image,
            u4CallbackIndex,
            fgIsFinalImage
        };
        pThread->addCallbackData(&callbackData);
        //
        pThread->doCallback(ZipImageCallbackThread::callback_type_jpeg);
        pThread = 0;
    }
    //
lbExit:
    //
    return  ret;
}

