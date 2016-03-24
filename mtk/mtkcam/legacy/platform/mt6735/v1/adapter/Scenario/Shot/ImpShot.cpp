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

#define LOG_TAG "MtkCam/Shot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <Shot/IShot.h>
//
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camnode/ICamBufHandler.h>
//
#include "inc/ImpShot.h"
//
#include <cutils/properties.h>
//
#include <mtkcam/featureio/capturenr.h>
#include <camera_custom_capture_nr.h>
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
using namespace android;
using namespace NSShot;


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


////////////////////////////////////////////////////////////////////////////////
//  IShot
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 *
 ******************************************************************************/
IShot::
IShot(sp<ImpShot> pImpShot)
    : mpImpShot(pImpShot)
{
}


/******************************************************************************
 *
 ******************************************************************************/
IShot::
~IShot()
{
    MY_LOGD("");
    mpImpShot->onDestroy();
    mpImpShot = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
IShot::
getShotName() const
{
    return  mpImpShot->getShotName();
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
IShot::
getShotMode() const
{
    return  mpImpShot->getShotMode();
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
IShot::
getOpenId() const
{
    return  mpImpShot->getOpenId();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
IShot::
setCallback(sp<IShotCallback> pShotCallback)
{
    return  mpImpShot->setCallback(pShotCallback);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
IShot::
sendCommand(
    ECommand const  cmd,
    MUINTPTR const  arg1 /*= 0*/,
    uint32_t const  arg2 /*= 0*/,
    uint32_t const  arg3
)
{
    return  mpImpShot->sendCommand(cmd, arg1, arg2, arg3);
}


////////////////////////////////////////////////////////////////////////////////
//  ImpShot
////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 *
 ******************************************************************************/
ImpShot::
ImpShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ms8ShotName(String8(pszShotName))
    , mu4ShotMode(u4ShotMode)
    , mi4OpenId(i4OpenId)
    , mpShotCallback(NULL)
    //
    , mShotParam()
    , mJpegParam()
    //
    , mpPrvBufHandler(NULL)
    , mpCapBufMgr(NULL)
    , mpCapBufMgr_Main2(NULL)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ImpShot::
~ImpShot()
{
    MY_LOGD("+");
    if  ( mpShotCallback != 0 ) {
        MY_LOGD("mpShotCallback.get(%p), mpShotCallback->getStrongCount(%d)", mpShotCallback.get(), mpShotCallback->getStrongCount());
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setCallback(sp<IShotCallback>& rpShotCallback)
{
    MY_LOGD("+ rpShotCallback(%p), rpShotCallback->getStrongCount(%d)", rpShotCallback.get(), rpShotCallback->getStrongCount());
    mpShotCallback = rpShotCallback;
    return  (mpShotCallback != 0);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setShotParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to ShotParam");
        return  false;
    }
    //
    if  ( size != sizeof(ShotParam) )
    {
        MY_LOGE("size[%d] != sizeof(ShotParam)[%d]; please fully build source codes", size, sizeof(ShotParam));
        return  false;
    }
    //
    mShotParam = *reinterpret_cast<ShotParam const*>(pParam);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setJpegParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to JpegParam");
        return  false;
    }
    //
    if  ( size != sizeof(JpegParam) )
    {
        MY_LOGE("size[%d] != sizeof(JpegParam)[%d]; please fully build source codes", size, sizeof(JpegParam));
        return  false;
    }
    //
    mJpegParam = *reinterpret_cast<JpegParam const*>(pParam);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setPrvBufHandler(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to prv buf handler");
        return  false;
    }
    //
    mpPrvBufHandler = const_cast<ICamBufHandler*>(reinterpret_cast<ICamBufHandler const*>(pParam));
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
setCapBufMgr(void const* pParam_0, void const* pParam_1, size_t const size)
{
    if  ( ! pParam_0 )
    {
        MY_LOGE("Null pointer to CapBufMgr");
        return  false;
    }
    //
    mpCapBufMgr = const_cast<NSCamShot::CapBufMgr*>(reinterpret_cast<NSCamShot::CapBufMgr const*>(pParam_0));
    if  ( pParam_1 != NULL )
    {
        mpCapBufMgr_Main2 = const_cast<NSCamShot::CapBufMgr*>(reinterpret_cast<NSCamShot::CapBufMgr const*>(pParam_1));
    }
    else
    {
        MY_LOGW("Null pointer to CapBufMgr_Main2");
    }
    return  true;
}



/******************************************************************************
 *
 ******************************************************************************/
bool
ImpShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    switch  (cmd)
    {
    //  This command is to set shot-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ShotParam (i.e. ShotParam const*)
    //      arg2
    //          [I] sizeof(ShotParam)
    case eCmd_setShotParam:
        return  setShotParam(reinterpret_cast<void const*>(arg1), arg2);

    //  This command is to set jpeg-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to JpegParam (i.e. JpegParam const*)
    //      arg2
    //          [I] sizeof(JpegParam)
    case eCmd_setJpegParam:
        return  setJpegParam(reinterpret_cast<void const*>(arg1), arg2);

    //  This command is to set preview buffer handler
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ICamBufHandler
    //      arg2
    //          [I] sizeof(ICamBufHandler)
    case eCmd_setPrvBufHdl:
        return  setPrvBufHandler(reinterpret_cast<void const*>(arg1), arg2);

    //  This command is to set CapBufMgr
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to CapBufMgr
    //      arg2
    //          [I] sizeof(CapBufMgr)
    case eCmd_setCapBufMgr:
        return  setCapBufMgr(reinterpret_cast<void const*>(arg1), reinterpret_cast<void const*>(arg3), arg2);

    default:
        break;
    }
    MY_LOGW("Do nothing (cmd, arg1, arg2, arg3)=(%x, %d, %d, %d)", cmd, arg1, arg2, arg3);
    return  false;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImpShot::
makeExifHeader(MUINT32 const u4CamMode,
                   MUINT8* const puThumbBuf,
                   MUINT32 const u4ThumbSize,
                   MUINT8* puExifBuf,
                   MUINT32 &u4FinalExifSize,
                   IDbgInfoContainer* pDbg,
                   MUINT32 u4ImgIndex,
                   MUINT32 u4GroupId,
                   MUINT32 u4FocusValH,
                   MUINT32 u4FocusValL)
{
    //
    MY_LOGD("+ (CamMode, ThumbBuf, ThumbSize, ExifBuf, pDbg) = (%d, %p, %d, %p, %p)",
            u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf, pDbg);

    if (u4ThumbSize > 63 * 1024)
    {
        MY_LOGW("The thumbnail size is large than 63K, the exif header will be broken");
    }
    bool ret = true;
    uint32_t u4App1HeaderSize = 0;
    uint32_t u4AppnHeaderSize = 0;

    uint32_t exifHeaderSize = 0;
    CamExif rCamExif;
    CamExifParam rExifParam;
    CamDbgParam rDbgParam;

    // ExifParam (for Gps)
    if (! mJpegParam.ms8GpsLatitude.isEmpty() && !mJpegParam.ms8GpsLongitude.isEmpty())
    {
        rExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLatitude), mJpegParam.ms8GpsLatitude.string(), mJpegParam.ms8GpsLatitude.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLongitude), mJpegParam.ms8GpsLongitude.string(), mJpegParam.ms8GpsLongitude.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSTimeStamp), mJpegParam.ms8GpsTimestamp.string(), mJpegParam.ms8GpsTimestamp.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSProcessingMethod), mJpegParam.ms8GpsMethod.string(), mJpegParam.ms8GpsMethod.length());
        rExifParam.u4GPSAltitude = ::atoi(mJpegParam.ms8GpsAltitude.string());
    }
    // the bitstream already rotated. rotation should be 0
    rExifParam.u4Orientation = 0;
    rExifParam.u4ZoomRatio = mShotParam.mu4ZoomRatio;
    //
    rExifParam.u4Facing = NSCam::IHalSensorList::get()->queryFacingDirection(getOpenId());
    //
    rExifParam.u4ImgIndex = u4ImgIndex;
    rExifParam.u4GroupId = u4GroupId;
    //
    rExifParam.u4FocusH = u4FocusValH;
    rExifParam.u4FocusL = u4FocusValL;
    //
    rExifParam.u4RefocusPos = 1;    // TODO:
    if ( !mShotParam.ms8StereoFileName.isEmpty() )
    {
        ::strncpy(reinterpret_cast<char*>(rExifParam.uJpsFileName), mShotParam.ms8StereoFileName.string(), mShotParam.ms8StereoFileName.length());
    }
    //
    //! CamDbgParam (for camMode, shotMode)
    rDbgParam.u4CamMode = u4CamMode;
    rDbgParam.u4ShotMode = getShotMode();
    //
    rCamExif.init(rExifParam,  rDbgParam);
    //
    IHal3A* p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,
                                            getOpenId(),
                                            LOG_TAG);
    if( p3AHal == NULL ) {
        MY_LOGE("p3AHal == NULL");
    } else {
        p3AHal->set3AEXIFInfo(&rCamExif);
        p3AHal->destroyInstance(LOG_TAG);
        p3AHal = NULL;
    }

    // the bitstream already rotated. it need to swap the width/height
    if( mShotParam.mu4Transform == NSCam::eTransform_ROT_90 ||
            mShotParam.mu4Transform == NSCam::eTransform_ROT_270 )
    {
        rCamExif.makeExifApp1(mShotParam.mi4PictureHeight,  mShotParam.mi4PictureWidth, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    else
    {
        rCamExif.makeExifApp1(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    // copy thumbnail image after APP1
    MUINT8 *pdest = puExifBuf + u4App1HeaderSize;
    ::memcpy(pdest, puThumbBuf, u4ThumbSize) ;
    //
    if( pDbg != NULL )
    {
        pDbg->setToExif(&rCamExif);
    }
    //
    // Sensor Debug Info
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        ret = false;
    }
    //
    NSCam::IHalSensor* pSensorHalObj = pHalSensorList->createSensor( ms8ShotName.string(), getOpenId());
    if(pSensorHalObj == NULL)
    {
       MY_LOGE("pSensorHalObj is NULL");
       ret = false;
    } else {
        pSensorHalObj->setDebugInfo(&rCamExif);
        pSensorHalObj->destroyInstance(ms8ShotName.string());
        pSensorHalObj = NULL;
    }
    //
    pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize;
    //
    rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
    rCamExif.uninit();

    u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize;

    MY_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)",
                          u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize);

    if ( DBG_EXIF_SIZE < u4FinalExifSize )
    {
        MY_LOGE("u4FinalExifSize(%d) >= exif header size(%d)", u4FinalExifSize, DBG_EXIF_SIZE);
        ret = false;
    }

    return ret;
}


/******************************************************************************
* for Stereo EXIF <pDbg_0 for 1st camera, pDbg_1 for 2nd(minor) camera>
*******************************************************************************/
bool
ImpShot::
makeExifHeader(MUINT32 const u4CamMode,
                   MUINT8* const puThumbBuf,
                   MUINT32 const u4ThumbSize,
                   MUINT8* puExifBuf,
                   MUINT32 &u4FinalExifSize,
                   IDbgInfoContainer* pDbg_0,
                   IDbgInfoContainer* pDbg_1
)
{
    //
    MY_LOGD("+ (CamMode, ThumbBuf, ThumbSize, ExifBuf, pDbg_0, pDbg_1) = (%d, %p, %d, %p, %p, %p)",
            u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf, pDbg_0, pDbg_1);

    if (u4ThumbSize > 63 * 1024)
    {
        MY_LOGW("The thumbnail size is large than 63K, the exif header will be broken");
    }
    bool ret = true;
    uint32_t u4App1HeaderSize = 0;
    uint32_t u4AppnHeaderSize = 0;

    uint32_t exifHeaderSize = 0;
    CamExif rCamExif;
    CamExifParam rExifParam;
    CamDbgParam rDbgParam;

    // ExifParam (for Gps)
    if (! mJpegParam.ms8GpsLatitude.isEmpty() && !mJpegParam.ms8GpsLongitude.isEmpty())
    {
        rExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLatitude), mJpegParam.ms8GpsLatitude.string(), mJpegParam.ms8GpsLatitude.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLongitude), mJpegParam.ms8GpsLongitude.string(), mJpegParam.ms8GpsLongitude.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSTimeStamp), mJpegParam.ms8GpsTimestamp.string(), mJpegParam.ms8GpsTimestamp.length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSProcessingMethod), mJpegParam.ms8GpsMethod.string(), mJpegParam.ms8GpsMethod.length());
        rExifParam.u4GPSAltitude = ::atoi(mJpegParam.ms8GpsAltitude.string());
    }
    // the bitstream already rotated. rotation should be 0
    rExifParam.u4Orientation = 0;
    rExifParam.u4ZoomRatio = mShotParam.mu4ZoomRatio;
    //
    rExifParam.u4Facing = NSCam::IHalSensorList::get()->queryFacingDirection(getOpenId());
    //
    //! CamDbgParam (for camMode, shotMode)
    rDbgParam.u4CamMode = u4CamMode;
    rDbgParam.u4ShotMode = getShotMode();
    //
    rCamExif.init(rExifParam,  rDbgParam);
    //
    IHal3A* p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,
                                            getOpenId(),
                                            LOG_TAG);
    if( p3AHal == NULL ) {
        MY_LOGE("p3AHal == NULL");
    } else {
        p3AHal->set3AEXIFInfo(&rCamExif);
        p3AHal->destroyInstance(LOG_TAG);
        p3AHal = NULL;
    }

    // the bitstream already rotated. it need to swap the width/height
    if( mShotParam.mu4Transform == NSCam::eTransform_ROT_90 ||
            mShotParam.mu4Transform == NSCam::eTransform_ROT_270 )
    {
        rCamExif.makeExifApp1(mShotParam.mi4PictureHeight,  mShotParam.mi4PictureWidth, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    else
    {
        rCamExif.makeExifApp1(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    // copy thumbnail image after APP1
    MUINT8 *pdest = puExifBuf + u4App1HeaderSize;
    ::memcpy(pdest, puThumbBuf, u4ThumbSize) ;
    //
    if( pDbg_0 != NULL )
    {
        pDbg_0->setToExif(&rCamExif);
    }
    if( pDbg_1 != NULL )
    {
        pDbg_1->setToExif(&rCamExif);
    }
    //
    // Sensor Debug Info
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        ret = false;
    }
    //
    NSCam::IHalSensor* pSensorHalObj = pHalSensorList->createSensor( ms8ShotName.string(), getOpenId());
    if(pSensorHalObj == NULL)
    {
       MY_LOGE("pSensorHalObj is NULL");
       ret = false;
    } else {
        pSensorHalObj->setDebugInfo(&rCamExif);
        pSensorHalObj->destroyInstance(ms8ShotName.string());
        pSensorHalObj = NULL;
    }
    //
    pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize;
    //
    rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
    rCamExif.uninit();

    u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize;

    MY_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)",
                          u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize);

    if ( DBG_EXIF_SIZE < u4FinalExifSize )
    {
        MY_LOGE("u4FinalExifSize(%d) >= exif header size(%d)", u4FinalExifSize, DBG_EXIF_SIZE);
        ret = false;
    }

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MUINT32
ImpShot::
queryCapNRType(
        MUINT32 currentIso,
        MBOOL usemfll
        ) const
{
    MUINT32 NRType = NSCamShot::ECamShot_NRTYPE_NONE; //default no NR
    Capture_NR_Th_t NR_thres;
    //
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    if( pHalSensorList->queryType(getOpenId()) == NSCam::NSSensorType::eYUV ) {
        // yuv sensor not support multi-pass NR
        return NSCamShot::ECamShot_NRTYPE_NONE;
    }
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(getOpenId());
    // get threshold from custom folder
    get_capture_nr_th(
            sensorDev,
            getShotMode(),
            usemfll,
            &NR_thres);

    MY_LOGD("iso %d, mfll %d, th %d/%d", currentIso, usemfll, NR_thres.hwth, NR_thres.swth);

    if( currentIso >= NR_thres.hwth )
        NRType = NSCamShot::ECamShot_NRTYPE_HWNR;

    if( currentIso >= NR_thres.swth )
        NRType = NSCamShot::ECamShot_NRTYPE_SWNR;

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.shot.forcenr", value, "-1");
        MINT32 force = atoi(value);
        if(force!=-1) {
            MY_LOGD("force nr %d", force);
            NRType = force;
        }
    }

    if( MTRUE /*NRType == NSCamShot::ECamShot_NRTYPE_SWNR*/ ) //always set for engshot
    {
        // udpate iso, single/mfll info to swnr
        SwNRParam* pParam = NULL;
        MINT32 perfLevel = -1;
        pParam = SwNRParam::getInstance(getOpenId());
        if( !pParam )
        {
            MY_LOGE("cannot get SwNRParam");
            goto lbExit;
        }
        if( NRType == NSCamShot::ECamShot_NRTYPE_SWNR ) {
            perfLevel = get_performance_level(
                    sensorDev,
                    getShotMode(),
                    usemfll,
                    CamManager::getInstance()->isMultiDevice());
            //MY_LOGD("query perf level %d", perfLevel);
        }
        //
        pParam->setParam(currentIso, usemfll, perfLevel);
    }

lbExit:
    return NRType;
}


/******************************************************************************
*
*******************************************************************************/
MUINT32
ImpShot::
getPreviewIso() const
{
    MUINT32 iso = 0;
    FrameOutputParam_T rPrv3AParam;
    IHal3A* p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,
                                            getOpenId(),
                                            LOG_TAG);
    if( p3AHal == NULL ) {
        MY_LOGE("p3AHal == NULL");
        goto lbExit;
    }
    p3AHal->getRTParams(rPrv3AParam);
    p3AHal->destroyInstance(LOG_TAG);

    iso = rPrv3AParam.u4RealISOValue;
lbExit:
    return iso;
}


/******************************************************************************
*
*******************************************************************************/
MUINT32
ImpShot::
getCaptureIso() const
{
    MUINT32 iso = 0;
    CaptureParam_T rCap3AParam;
    IHal3A* p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,
                                            getOpenId(),
                                            LOG_TAG);
    if( p3AHal == NULL ) {
        MY_LOGE("p3AHal == NULL");
        goto lbExit;
    }
    p3AHal->getCaptureParams(rCap3AParam);
    p3AHal->destroyInstance(LOG_TAG);

    iso = rCap3AParam.u4RealISO;
lbExit:
    return iso;
}


