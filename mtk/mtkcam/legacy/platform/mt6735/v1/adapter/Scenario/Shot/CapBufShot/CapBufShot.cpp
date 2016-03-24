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

#define LOG_TAG "MtkCam/CapBufShot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
//
#include <mtkcam/exif/IBaseCamExif.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "CapBufShot.h"
//
using namespace android;
using namespace NSShot;

#define SHUTTER_TIMING (NSCamShot::ECamShot_NOTIFY_MSG_EOF)
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
sp<IShot>
createInstance_CapBufShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<CapBufShot>     pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new CapBufShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new CapBufShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
CapBufShot::
onCreate()
{
#warning "[TODO] CapBufShot::onCreate()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
CapBufShot::
onDestroy()
{
#warning "[TODO] CapBufShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
CapBufShot::
CapBufShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
    FUNC_NAME;
}


/******************************************************************************
 *
 ******************************************************************************/
CapBufShot::
~CapBufShot()
{
    FUNC_NAME;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CapBufShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    AutoCPTLog cptlog(Event_Shot_sendCmd, cmd, arg1);
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CapBufShot::
onCmd_reset()
{
#warning "[TODO] CapBufShot::onCmd_reset()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CapBufShot::
onCmd_capture()
{
    AutoCPTLog cptlog(Event_Shot_capture);
    MBOOL ret = MTRUE;
    NSCamShot::ISingleShot *pSingleShot =
        NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "CapBufShot");
    //
    MUINT32 nrtype = queryCapNRType( getPreviewIso() );
    //
    pSingleShot->init();
    //
    pSingleShot->enableNotifyMsg( SHUTTER_TIMING );
    //
    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

    pSingleShot->enableDataMsg(
            NSCamShot::ECamShot_DATA_MSG_JPEG
#if 0
            | ((ePostViewFmt != eImgFmt_UNKNOWN) ?
             NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
#endif
            );

    // shot param
    NSCamShot::ShotParam rShotParam(
            eImgFmt_YUY2,                    //yuv format
            mShotParam.mi4PictureWidth,      //picutre width
            mShotParam.mi4PictureHeight,     //picture height
            mShotParam.mu4Transform,         //picture transform
            ePostViewFmt,                    //postview format
            mShotParam.mi4PostviewWidth,     //postview width
            mShotParam.mi4PostviewHeight,    //postview height
            0,                               //postview transform
            mShotParam.mu4ZoomRatio          //zoom
            );

    // jpeg param
    NSCamShot::JpegParam rJpegParam(
            NSCamShot::ThumbnailParam(
                mJpegParam.mi4JpegThumbWidth,
                mJpegParam.mi4JpegThumbHeight,
                mJpegParam.mu4JpegThumbQuality,
                MTRUE),
            mJpegParam.mu4JpegQuality,         //Quality
            MFALSE                             //isSOI
            );

    // sensor param
    NSCamShot::SensorParam rSensorParam(
            getOpenId(),                             //sensor idx
            SENSOR_SCENARIO_ID_NORMAL_CAPTURE,       //Scenaio
            10,                                      //bit depth
            MFALSE,                                  //bypass delay
            MFALSE                                   //bypass scenario
            );
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = pSingleShot->setShotParam(rShotParam);
    //
    ret = pSingleShot->setJpegParam(rJpegParam);
    //
    if(mu4ShotMode == NSCam::eShotMode_ZsdShot)
    {
        CAM_LOGD("ZSD Shot");
        ret = pSingleShot->sendCommand(
                                NSCamShot::ECamShot_CMD_SET_CAPTURE_STYLE,
                                NSCamShot::ECamShot_CMD_STYLE_NORMAL,
                                0,
                                0);
    }
    else
    if(mu4ShotMode == NSCam::eShotMode_VideoSnapShot)
    {
        CAM_LOGD("VSS Shot");
        ret = pSingleShot->sendCommand(
                                NSCamShot::ECamShot_CMD_SET_CAPTURE_STYLE,
                                NSCamShot::ECamShot_CMD_STYLE_VSS,
                                0,
                                0);
    }
    else
    {
        CAM_LOGE("Un-supported Shot");
    }
    //
    ret = pSingleShot->sendCommand( NSCamShot::ECamShot_CMD_SET_NRTYPE, nrtype, 0, 0 );
    //
    IImageBuffer* pImageBuffer;
    mpCapBufMgr->dequeBuf(pImageBuffer);
    ret = pSingleShot->startOne(rSensorParam, pImageBuffer);
    mpCapBufMgr->enqueBuf(pImageBuffer);
    //
    ret = pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();


    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CapBufShot::
onCmd_cancel()
{
    AutoCPTLog cptlog(Event_Shot_cancel);
#warning "[TODO] CapBufShot::onCmd_cancel()"
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CapBufShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    AutoCPTLog cptlog(Event_Shot_handleNotifyCb);
    CapBufShot *pCapBufShot = reinterpret_cast <CapBufShot *>(user);
    if (NULL != pCapBufShot)
    {
        if ( SHUTTER_TIMING == msg.msgType)
        {
            pCapBufShot->mpShotCallback->onCB_Shutter(true,
                                                      0
                                                     );
        }
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CapBufShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    CapBufShot *pCapBufShot = reinterpret_cast<CapBufShot *>(user);
    if (NULL != pCapBufShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            //pCapBufShot->handlePostViewData( msg.puData, msg.u4Size);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pCapBufShot->handleJpegData(
                    (IImageBuffer*)msg.pBuffer,
                    (IImageBuffer*)msg.ext1,
                    (IDbgInfoContainer*)msg.ext2
                    );
        }
    }

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
CapBufShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
#if 0
    AutoCPTLog cptlog(Event_Shot_handlePVData);
    MY_LOGD("+ (puBuf, size) = (%p, %d)", puBuf, u4Size);
    mpShotCallback->onCB_PostviewDisplay(0,
                                         u4Size,
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        );

    MY_LOGD("-");
#endif
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
CapBufShot::
handleJpegData(IImageBuffer* pJpeg, IImageBuffer* pThumb, IDbgInfoContainer* pDbg)
{
    AutoCPTLog cptlog(Event_Shot_handleJpegData);
    MUINT8* puJpegBuf = (MUINT8*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();
    MUINT8* puThumbBuf = NULL;
    MUINT32 u4ThumbSize = 0;
    if( pThumb != NULL )
    {
        puThumbBuf = (MUINT8*)pThumb->getBufVA(0);
        u4ThumbSize = pThumb->getBitstreamSize();
    }

    MY_LOGD("+ (puJpgBuf, jpgSize, puThumbBuf, thumbSize, dbg) = (%p, %d, %p, %d, %p)",
            puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize, pDbg);

    MUINT8 *puExifHeaderBuf = new MUINT8[ DBG_EXIF_SIZE ];
    MUINT32 u4ExifHeaderSize = 0;

    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "makeExifHeader");
    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize, pDbg);
    MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "onCB_CompressedImage");
    mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize,
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index
                                         true                     //final image
                                         );
    MY_LOGD("-");

    delete [] puExifHeaderBuf;

    return MTRUE;

}


