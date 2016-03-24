/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "CamExif"
//
#include <string.h>
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include "mtkcam/exif/IBaseCamExif.h"
#include <mtkcam/exif/StdExif.h>
//
#include <stdlib.h>
#include <IBaseExif.h>
#include <Exif.h>
//
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
//
#include <limits>
using namespace std;
/**************************  Basic format for Marker **************************
*  [ 0xFF+Marker Number(1 byte) ] + [ DataSize desciptor (2 bytes) ] + [ Data (n bytes) ]
*  DataSize desciptor should be (2+n). For example:
*  FF E1 00 0C means marker APP1(0xFFE1) has 0x000C(equal 12)bytes of data.
*  But the data size '12' includes "Data size" descriptor, it follows only 10 bytes
*  of data after 0x000C.
*  DEFAULT_DATA: Data content
*  DEFAULT_SIZE: DataSize desciptor = DEFAULT_DATA + 0x02
**************************************************************************/
#define DEFAULT_DATA        (0xFF7C)
#define DEFAULT_SIZE        (DEFAULT_DATA + 0x02)   // 0xFF7E: include "Data size" descriptor
#define APPN_SIZE           (DEFAULT_SIZE + 0x02)   // 0xFF80: should be 128-aligned after include 0xFFEn.
#define APPN_COUNT          (4)

/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)


/*******************************************************************************
*
********************************************************************************/
StdExif::
StdExif()
    : mExifParam()
    , mpBaseExif(NULL)
    //
    , mbEnableDbgExif(MFALSE)
    , mApp1Size(0)
    , mDbgAppnSize(0)
    , mMaxThumbSize(0)
    , mpOutputExifBuf(0)
    //
    , mDbgInfo()
    , mMapModuleID()
    , mi4DbgModuleType(0)
    , mpDebugIdMap(NULL)
{
    MY_LOGI("- this:%p", this);
}

/*******************************************************************************
*
********************************************************************************/
StdExif::
~StdExif()
{
    MY_LOGI("- this:%p", this);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
init(
    ExifParams const&   rExifParams,
    MBOOL const         enableDbgExif
)
{
    mExifParam = rExifParams;
    MY_LOGI("mpExifParam(0x%x), ImageSize(%dx%d), FNumber(%d/10), FocalLegth(%d/1000), AWBMode(%d), Strobe(%d)",
        &mExifParam, mExifParam.u4ImageWidth, mExifParam.u4ImageHeight, mExifParam.u4FNumber,
        mExifParam.u4FocalLength, mExifParam.u4AWBMode, mExifParam.u4FlashLightTimeus);
    MY_LOGI("AEMeterMode(%d), AEExpBias(%d), CapExposureTime(%d), AEISOSpeed(%d), LightSource(%d)",
        mExifParam.u4AEMeterMode, mExifParam.i4AEExpBias, mExifParam.u4CapExposureTime,
        mExifParam.u4AEISOSpeed, mExifParam.u4LightSource);
    MY_LOGI("ExpProgram(%d), SceneCapType(%d), Orientation(%d), ZoomRatio(%d), Facing(%d)",
        mExifParam.u4ExpProgram, mExifParam.u4SceneCapType,
        mExifParam.u4Orientation, mExifParam.u4ZoomRatio, mExifParam.u4Facing);
    MY_LOGI("GPS(%d), Altitude(%d), Latitude(%s), Longitude(%s), TimeStamp(%s), ProcessingMethod(%s)",
        mExifParam.u4GpsIsOn, mExifParam.u4GPSAltitude, mExifParam.uGPSLatitude,
        mExifParam.uGPSLongitude, mExifParam.uGPSTimeStamp, mExifParam.uGPSProcessingMethod);
    //
    // Exif Utilitis
    mpBaseExif = new ExifUtils();
    if ( !(mpBaseExif->init(mExifParam.u4GpsIsOn)) )
    {
        MY_LOGE("mpBaseExif->init() fail");
        return MFALSE;
    }
    //
    mpDebugIdMap = new ExifIdMap();
    //
    mbEnableDbgExif = enableDbgExif;
    mApp1Size       = mpBaseExif->exifApp1SizeGet();
    mDbgAppnSize    = isEnableDbgExif() ? (APPN_SIZE*APPN_COUNT) : 0;
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
uninit()
{
    //
    if ( mpBaseExif != NULL )
    {
        if ( !(mpBaseExif->uninit()) )
        {
            MY_LOGE("mpBaseExif->uninit() fail");
        }
        delete mpBaseExif;
        mpBaseExif = NULL;
    }
    //
    if ( mpDebugIdMap != NULL )
    {
        delete mpDebugIdMap;
        mpDebugIdMap = NULL;
    }
    //
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
reset(
    ExifParams const&   rExifParams,
    MBOOL const         enableDbgExif
)
{
    uninit();
    init(rExifParams, enableDbgExif);

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
size_t
StdExif::
getHeaderSize() const
{
    size_t size = getStdExifSize() + getThumbnailSize() + getDbgExifSize();
    MY_LOGI("header size(%d)  getDbgExifSize(%d)", size, getDbgExifSize());
    return size;
}

/*******************************************************************************
*
********************************************************************************/
void
StdExif::
setMaxThumbnail(
    size_t const        thumbnailSize
)
{
    mMaxThumbSize   = thumbnailSize;
    MY_LOGI("max thumbnail size(%d)", mMaxThumbSize);
}

/*******************************************************************************
*
********************************************************************************/
void
StdExif::
updateStdExif(exifAPP1Info_s* exifApp1Info)
{
    ::memset(exifApp1Info, 0, sizeof(exifAPP1Info_s));

    /*********************************************************************************
                                           GPS
    **********************************************************************************/
    if  (mExifParam.u4GpsIsOn == 1) {
        float latitude = atof((char*)mExifParam.uGPSLatitude);
        float longitude = atof((char*)mExifParam.uGPSLongitude);
        long long timestamp = atol((char*)mExifParam.uGPSTimeStamp);
        char const*pgpsProcessingMethod = (char*)mExifParam.uGPSProcessingMethod;
        //
        // Set GPS Info
        if (latitude >= 0) {
            strcpy((char *)exifApp1Info->gpsLatitudeRef, "N");
        }
        else {
            strcpy((char *)exifApp1Info->gpsLatitudeRef, "S");
            latitude *= -1;     // make it positive
        }
        if (longitude >= 0) {
            strcpy((char *)exifApp1Info->gpsLongitudeRef, "E");
        }
        else {
            strcpy((char *)exifApp1Info->gpsLongitudeRef, "W");
            longitude *= -1;    // make it positive
        }
        exifApp1Info->gpsIsOn = 1;
        // Altitude
        exifApp1Info->gpsAltitude[0] = mExifParam.u4GPSAltitude;
        exifApp1Info->gpsAltitude[1] = 1;
        // Latitude
        exifApp1Info->gpsLatitude[0] = (int) latitude;
        exifApp1Info->gpsLatitude[1] = 1;
        latitude -= exifApp1Info->gpsLatitude[0];
        latitude *= 60;
        exifApp1Info->gpsLatitude[2] = (int) latitude;
        exifApp1Info->gpsLatitude[3] = 1;
        latitude -= exifApp1Info->gpsLatitude[2];
        latitude *= 60;
        latitude *= 10000;
        exifApp1Info->gpsLatitude[4] = (int) latitude;
        exifApp1Info->gpsLatitude[5] = 10000;
        // Longtitude
        exifApp1Info->gpsLongitude[0] = (int) longitude;
        exifApp1Info->gpsLongitude[1] = 1;
        longitude -= exifApp1Info->gpsLongitude[0];
        longitude *= 60;
        exifApp1Info->gpsLongitude[2] = (int) longitude;
        exifApp1Info->gpsLongitude[3] = 1;
        longitude -= exifApp1Info->gpsLongitude[2];
        longitude *= 60;
        longitude *= 10000;
        exifApp1Info->gpsLongitude[4] = (int) longitude;
        exifApp1Info->gpsLongitude[5] = 10000;

        // Timestamp
        if ( timestamp >= 0 )
        {
            time_t tim = (time_t) timestamp;
            struct tm *ptime = gmtime(&tim);
            exifApp1Info->gpsTimeStamp[0] = ptime->tm_hour;
            exifApp1Info->gpsTimeStamp[1] = 1;
            exifApp1Info->gpsTimeStamp[2] = ptime->tm_min;
            exifApp1Info->gpsTimeStamp[3] = 1;
            exifApp1Info->gpsTimeStamp[4] = ptime->tm_sec;
            exifApp1Info->gpsTimeStamp[5] = 1;
            sprintf((char *)exifApp1Info->gpsDateStamp, "%04d:%02d:%02d", ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday);
        }
        else
        {
            MY_LOGE("wrong timestamp(%lld)", timestamp);
        }
        // ProcessingMethod
        const char exifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 }; // ASCII
        int len1, len2, maxLen;
        len1 = sizeof(exifAsciiPrefix);
        memcpy(exifApp1Info->gpsProcessingMethod, exifAsciiPrefix, len1);
        maxLen = sizeof(exifApp1Info->gpsProcessingMethod) - len1;
        len2 = strlen(pgpsProcessingMethod);
        if (len2 > maxLen) {
            len2 = maxLen;
        }
        memcpy(&exifApp1Info->gpsProcessingMethod[len1], pgpsProcessingMethod, len2);
    }

    /*********************************************************************************
                                           common
    **********************************************************************************/
    // software information
    memset(exifApp1Info->strSoftware, 0, 32);
    strcpy((char *)exifApp1Info->strSoftware, "MediaTek Camera Application");

    // get datetime
    struct tm *tm;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if((tm = localtime(&tv.tv_sec)) != NULL)
    {
        strftime((char *)exifApp1Info->strDateTime, 20, "%Y:%m:%d %H:%M:%S", tm);
        snprintf((char *)exifApp1Info->strSubSecTime, 3, "%02ld\n", tv.tv_usec);
        MY_LOGI("strDateTime(%s), strSubSecTime(%s)", exifApp1Info->strDateTime, exifApp1Info->strSubSecTime);
    }

    // [digital zoom ratio]
    exifApp1Info->digitalZoomRatio[0] = (unsigned int)mExifParam.u4ZoomRatio;
    exifApp1Info->digitalZoomRatio[1] = 100;
    // [orientation]
    exifApp1Info->orientation = (unsigned short)determineExifOrientation(
                                    mExifParam.u4Orientation,
                                    mExifParam.u4Facing
                                );

    /*********************************************************************************
                                           3A
    **********************************************************************************/
    // [f number]
    exifApp1Info->fnumber[0] = (unsigned int)mExifParam.u4FNumber;
    exifApp1Info->fnumber[1] = 10;

    // [focal length]
    exifApp1Info->focalLength[0] = (unsigned int)mExifParam.u4FocalLength;
    exifApp1Info->focalLength[1] = 1000;

    // [iso speed]
    exifApp1Info->isoSpeedRatings = (unsigned short)mExifParam.u4AEISOSpeed;

    // [exposure time]
    if(mExifParam.u4CapExposureTime == 0){
        //YUV sensor
        exifApp1Info->exposureTime[0] = 0;
        exifApp1Info->exposureTime[1] = 0;
    }
    else{
        // RAW sensor
        if (mExifParam.u4CapExposureTime > 1000000) { //1 sec
            exifApp1Info->exposureTime[0] = mExifParam.u4CapExposureTime / 100000;
            exifApp1Info->exposureTime[1] = 10;
        }
        else{ // us
            exifApp1Info->exposureTime[0] = mExifParam.u4CapExposureTime;
            exifApp1Info->exposureTime[1] = 1000000;
        }
    }

    // [flashlight]
    exifApp1Info->flash = (0 != mExifParam.u4FlashLightTimeus) ? 1 : 0;

    // [white balance mode]
    exifApp1Info->whiteBalanceMode = mExifParam.u4AWBMode;

    // [light source]
    exifApp1Info->lightSource = mExifParam.u4LightSource;

    // [metering mode]
    exifApp1Info->meteringMode = mExifParam.u4AEMeterMode;

    // [exposure program] , [scene mode]
    exifApp1Info->exposureProgram  = mExifParam.u4ExpProgram;
    exifApp1Info->sceneCaptureType = mExifParam.u4SceneCapType;

    // [Ev offset]
    exifApp1Info->exposureBiasValue[0] = (unsigned int)mExifParam.i4AEExpBias;
    exifApp1Info->exposureBiasValue[1] = 10;

    /*********************************************************************************
                                           update customized exif
    **********************************************************************************/
    {
        char make[PROPERTY_VALUE_MAX] = {'\0'};
        char model[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("ro.product.manufacturer", make, "0");
        property_get("ro.product.model", model, "0");
        MY_LOGI("property: make(%s), model(%s)", make, model);
        // [Make]
        if ( ::strcmp(make, "0") != 0 ) {
            ::memset(exifApp1Info->strMake, 0, 32);
            ::strncpy((char*)exifApp1Info->strMake, (const char*)make, 31);
        }
        // [Model]
        if ( ::strcmp(model, "0") != 0 ) {
            ::memset(exifApp1Info->strModel, 0, 32);
            ::strncpy((char*)exifApp1Info->strModel, (const char*)model, 31);
        }
    }

    /*********************************************************************************
                                           MISC
    **********************************************************************************/
    // [flashPixVer]
    memcpy(exifApp1Info->strFlashPixVer, "0100 ", 5);
    // [exposure mode]
    exifApp1Info->exposureMode = 0;  // 0 means Auto exposure

}

/*******************************************************************************
*
********************************************************************************/
void
StdExif::
updateDbgExif()
{
    MUINT32 u4AppnCAM       = 5;
    MUINT32 u4AppnAAA       = 6;
    MUINT32 u4AppnISP       = 7;
    MUINT32 u4AppnSHAD_TBL  = 8;
    MUINT8* pDst            = (MUINT8*)getBufAddr() + getStdExifSize() + getThumbnailSize();

    //// CAM debug info
    appendCamDebugInfo(u4AppnCAM, &pDst);

    //// AAA debug info
    appendDebugInfo(ID_AAA, u4AppnAAA, &pDst);

    //// ISP debug info
    appendDebugInfo(ID_ISP, u4AppnISP, &pDst);

    //// SHAD_ARRAY debug info
    appendDebugInfo(ID_SHAD_TABLE, u4AppnSHAD_TBL, &pDst);

}

/*******************************************************************************
*
********************************************************************************/
status_t
StdExif::
make(
    MUINTPTR const  outputExifBuf,
    size_t& rOutputExifSize
)
{
    int ret = 0;
    mpOutputExifBuf = outputExifBuf;
    // set 0 first for error return
    rOutputExifSize = 0;
    MY_LOGI("out buffer(0x%x)", getBufAddr());

    unsigned int u4OutputExifSize = 0;
    exifAPP1Info_s exifApp1Info;
    exifImageInfo_s exifImgInfo;

    //  (1) Fill exifApp1Info
    updateStdExif(&exifApp1Info);

    //  (2) Fill exifImgInfo
    ::memset(&exifImgInfo, 0, sizeof(exifImageInfo_t));
    exifImgInfo.bufAddr     = getBufAddr();
    exifImgInfo.mainWidth   = mExifParam.u4ImageWidth;
    exifImgInfo.mainHeight  = mExifParam.u4ImageHeight;
    exifImgInfo.thumbSize   = getThumbnailSize();

    ret = mpBaseExif->exifApp1Make(&exifImgInfo, &exifApp1Info, &u4OutputExifSize);
    rOutputExifSize = (size_t)u4OutputExifSize;

    // (3) Append debug exif
    if ( isEnableDbgExif() )
    {
        updateDbgExif();
    }

    return (status_t)ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
StdExif::
determineExifOrientation(
    MUINT32 const   u4DeviceOrientation,
    MBOOL const     bIsFacing,
    MBOOL const     bIsFacingFlip
)
{
    MINT32  result = -1;

    if  ( bIsFacing && bIsFacingFlip )
    {
        //  Front Camera with Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 8;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 6;
            break;
        default:
            result = 0;
            break;
        }
    }
    else
    {   //  Rear Camera or Front Camera without Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 6;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 8;
            break;
        default:
            result = 0;
            break;
        }
    }

    return  result;
}

/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_CMN_INFO_S &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_CMN_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
void
StdExif::
getCommonDebugInfo(DEBUG_CMN_INFO_S &a_rDbgCommonInfo)
{
    MBOOL  ret = MTRUE;
    MUINT32 u4ID = 0;

    // Cam Debug Info
    memset(&a_rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));

    // Cam Debug Version
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_VERSION, (MUINT32)CMN_DEBUG_TAG_VERSION);

    // Shot mode
//    setDebugTag(a_rDbgCommonInfo, CMN_TAG_SHOT_MODE, mCamDbgParam.u4ShotMode);

    // Camera mode: Normal, Engineer
//    setDebugTag(a_rDbgCommonInfo, CMN_TAG_CAM_MODE, mCamDbgParam.u4CamMode);

    ret = sendCommand(CMD_REGISTER, DEBUG_CAM_CMN_MID, reinterpret_cast<MUINTPTR>(&u4ID));
    if (!ret) {
        MY_LOGE("ERROR: CMD_REGISTER");
    }

    ret = sendCommand(CMD_SET_DBG_EXIF, u4ID, (MUINTPTR) &a_rDbgCommonInfo, sizeof(DEBUG_CMN_INFO_S));
    if (!ret) {
        MY_LOGE("ERROR: ID_CMN");
    }

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
getCamDebugInfo(
    MUINT8* const pDbgInfo,
    MUINT32 const rDbgSize,
    MINT32 const dbgModuleID
)
{
    MUINT8* pDbgModuleInfo = NULL;
    MUINT32 dbgModuleSize = 0;
    if (mi4DbgModuleType & dbgModuleID) {
        MUINT32 moduleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("Get: ID_%s(0x%04x), Size(%d), Addr(%p)",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID,
            mDbgInfo[moduleIndex].u4BufSize, (MUINTPTR)&mDbgInfo[moduleIndex].puDbgBuf);
        //
        pDbgModuleInfo = mDbgInfo[moduleIndex].puDbgBuf;
        dbgModuleSize = mDbgInfo[moduleIndex].u4BufSize;
        //
    }
    else {
        MY_LOGW("ID_%s(0x%04x) did not exist.", mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID);
        return MFALSE;
    }
    //
    if ( dbgModuleSize > rDbgSize) {
        MY_LOGE("ID_%s(0x%04x) ERROR size! (%d)>(%d)",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID, dbgModuleSize, rDbgSize);
        return MFALSE;
    }
    else {
        memcpy(pDbgInfo, pDbgModuleInfo, rDbgSize);
    }
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
appendCamDebugInfo(
    MUINT32 const dbgAppn,          //  [I] APPn for CAM module
    MUINT8** const ppuAppnBuf       //  [O] Pointer to APPn Buffer
)
{
    MBOOL  ret = MFALSE;

    //// CAM debug info: Common, MF, N3D, Sensor, Shading
    {
        // COMMON
        DEBUG_CMN_INFO_T rDbgCommonInfo;
        memset(&rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));
        getCommonDebugInfo(rDbgCommonInfo);

        //// CAM
        MUINT32 app5ReturnSize = 0;
        MUINT32 dbgCamHeaderSize = 0;
        MUINT8* pDbgCamInfo = NULL;
        MUINT32 dbgCamSize = 0;
        DEBUG_CAM_INFO_T rDbgCamInfo;
        memset(&rDbgCamInfo, 0, sizeof(DEBUG_CAM_INFO_T));
        dbgCamHeaderSize = sizeof(rDbgCamInfo.hdr);

        rDbgCamInfo.hdr.u4KeyID                 = DEBUG_CAM_KEYID;
        rDbgCamInfo.hdr.u4ModuleCount           = MODULE_NUM(DEBUF_CAM_TOT_MODULE_NUM, DEBUF_CAM_TAG_MODULE_NUM);
        rDbgCamInfo.hdr.u4DbgCMNInfoOffset      = dbgCamHeaderSize;
        rDbgCamInfo.hdr.u4DbgMFInfoOffset       = rDbgCamInfo.hdr.u4DbgCMNInfoOffset + sizeof(DEBUG_CMN_INFO_T);
        rDbgCamInfo.hdr.u4DbgN3DInfoOffset      = rDbgCamInfo.hdr.u4DbgMFInfoOffset + sizeof(DEBUG_MF_INFO_T);
        rDbgCamInfo.hdr.u4DbgSENSORInfoOffset   = rDbgCamInfo.hdr.u4DbgN3DInfoOffset + sizeof(DEBUG_N3D_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVEAInfoOffset = rDbgCamInfo.hdr.u4DbgSENSORInfoOffset + sizeof(DEBUG_SENSOR_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVEBInfoOffset = rDbgCamInfo.hdr.u4DbgRESERVEAInfoOffset + sizeof(DEBUG_RESERVEA_INFO_T);
        rDbgCamInfo.hdr.u4DbgRESERVECInfoOffset = rDbgCamInfo.hdr.u4DbgRESERVEBInfoOffset + sizeof(DEBUG_RESERVEB_INFO_T);
        //
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgCMNInfo,      sizeof(DEBUG_CMN_INFO_T),       ID_CMN);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgMFInfo,       sizeof(DEBUG_MF_INFO_T),        ID_MF);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgN3DInfo,      sizeof(DEBUG_N3D_INFO_T),       ID_N3D);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgSENSORInfo,   sizeof(DEBUG_SENSOR_INFO_T),    ID_SENSOR);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVEAInfo, sizeof(DEBUG_RESERVEA_INFO_T),  ID_RESERVE1);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVEBInfo, sizeof(DEBUG_RESERVEB_INFO_T),  ID_RESERVE2);
        getCamDebugInfo((MUINT8*)&rDbgCamInfo.rDbgRESERVECInfo, sizeof(DEBUG_RESERVEC_INFO_T),  ID_RESERVE3);
        //
        pDbgCamInfo = (MUINT8*) &rDbgCamInfo;
        dbgCamSize  = sizeof(DEBUG_CAM_INFO_T) + 2; // Data(n bytes) + Data size(2 bytes)

        if ( pDbgCamInfo && ((dbgCamSize >= 0x02) && (dbgCamSize <= DEFAULT_SIZE)) )
        {
            mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgCamInfo, dbgCamSize, &app5ReturnSize, DEFAULT_SIZE);
            //
            *ppuAppnBuf += app5ReturnSize;
        }
        else
        {
            MY_LOGE("dbgCamSize(%d) < 0", dbgCamSize);
            goto lbExit;
        }
    }

    ret = MTRUE;

lbExit:
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
appendDebugInfo(
    MINT32 const dbgModuleID,       //  [I] debug module ID
    MINT32 const dbgAppn,           //  [I] APPn
    MUINT8** const ppuAppnBuf       //  [O] Pointer to APPn Buffer
)
{
    MBOOL  ret = MTRUE;
    MUINT32 appnReturnSize      = 0;

    if (mi4DbgModuleType & dbgModuleID)
    {
        MUINT32 dbgModuleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("Get: ID_%s(0x%04x), Index(%d), Size(%d), Addr(%p), APP%d",
            mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID,
            dbgModuleIndex, mDbgInfo[dbgModuleIndex].u4BufSize, (MUINTPTR)mDbgInfo[dbgModuleIndex].puDbgBuf, dbgAppn);
        //
        MUINT8* pDbgModuleInfo  = mDbgInfo[dbgModuleIndex].puDbgBuf;
        MUINT32 dbgModuleSize   = mDbgInfo[dbgModuleIndex].u4BufSize + 2; // Data(n bytes) + Data size(2 bytes)
        //
        if ( (dbgModuleSize >= 0x02) && (dbgModuleSize <= DEFAULT_SIZE) )
        {
            mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgModuleInfo, dbgModuleSize, &appnReturnSize, DEFAULT_SIZE);
            //
            *ppuAppnBuf += appnReturnSize;
        }
        else
        {
            MY_LOGW("dbgModuleSize(%d)", dbgModuleSize);
            ret = MFALSE;
        }
    }
    else
    {
        MY_LOGW("ID_%s(0x%04x) did not exist", mpDebugIdMap->stringFor(dbgModuleID).string(), dbgModuleID);
        MUINT8* pDbgModuleInfo = new MUINT8[DEFAULT_DATA];
        ::memset(pDbgModuleInfo, 1, sizeof(MUINT8)*DEFAULT_DATA);
        mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgModuleInfo, DEFAULT_DATA, &appnReturnSize, DEFAULT_SIZE);
        *ppuAppnBuf += appnReturnSize;
        delete []pDbgModuleInfo;
        ret = MFALSE;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
StdExif::
sendCommand(
    MINT32      cmd,
    MINT32      parg1,
    MUINTPTR    parg2,
    MINT32      parg3
)
{
    MBOOL ret = MTRUE;

    //1.Special command
    //MY_LOGI("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd)
    {
        case CMD_REGISTER:
        {
            MUINT32 registerName = (MUINT32) parg1;
            //
            switch (registerName) {
                case AAA_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_AAA;
                    break;
                case ISP_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_ISP;
                    break;
                case DEBUG_EIS_MID:
                    *(MINT32 *) parg2 = ID_EIS;
                    break;
                case DEBUG_CAM_CMN_MID:
                    *(MINT32 *) parg2 = ID_CMN;
                    break;
                case DEBUG_CAM_MF_MID:
                    *(MINT32 *) parg2 = ID_MF;
                    break;
                case DEBUG_CAM_N3D_MID:
                    *(MINT32 *) parg2 = ID_N3D;
                    break;
                case DEBUG_CAM_SENSOR_MID:
                    *(MINT32 *) parg2 = ID_SENSOR;
                    break;
                case DEBUG_CAM_RESERVE1_MID:
                    *(MINT32 *) parg2 = ID_RESERVE1;
                    break;
                case DEBUG_CAM_RESERVE2_MID:
                    *(MINT32 *) parg2 = ID_RESERVE2;
                    break;
                case DEBUG_CAM_RESERVE3_MID:
                    *(MINT32 *) parg2 = ID_RESERVE3;
                    break;
                case DEBUG_SHAD_TABLE_KEYID:
                    *(MINT32 *) parg2 = ID_SHAD_TABLE;
                    break;
                default:
                    *(MINT32 *) parg2 = ID_ERROR;
                    MY_LOGE("registerID: 0x%x", registerName);
                    ret = MFALSE;
                    break;
            }
        }
            break;
        case CMD_SET_DBG_EXIF:
        {
            MUINT32 u4ID    = (MUINT32) parg1;
            MUINTPTR u4Addr  = (MUINTPTR) parg2;
            MUINT32 u4Size  = (MUINT32) parg3;
            //
            if (mi4DbgModuleType & u4ID)
            {
                MY_LOGW("ID_%s(0x%04x) already exists", mpDebugIdMap->stringFor(u4ID).string(), u4ID);
                ret = MFALSE;
            }
            else
            {
                if (u4ID != ID_ERROR)
                {
                    //
                    DbgInfo tmpDbgInfo;
                    tmpDbgInfo.u4BufSize    = u4Size;
                    tmpDbgInfo.puDbgBuf     = new MUINT8[ u4Size ];
                    memset(tmpDbgInfo.puDbgBuf, 0, u4Size);
                    memcpy(tmpDbgInfo.puDbgBuf, (void*)u4Addr, u4Size);
                    //
                    mMapModuleID.add(u4ID, mDbgInfo.size());
                    mDbgInfo.push_back(tmpDbgInfo);
                    mi4DbgModuleType |= u4ID;
                    //
                    MY_LOGI("Set: ID_%s(0x%04x), Size(%d), Addr(%p)",
                        mpDebugIdMap->stringFor(u4ID).string(), u4ID, tmpDbgInfo.u4BufSize, (MUINTPTR)&tmpDbgInfo.puDbgBuf);
                }
                else
                {
                    MY_LOGE("ID_ERROR");
                    ret = MFALSE;
                }
            }
        }
            break;
        default:
            MY_LOGE("unsupport cmd: 0x%x", cmd);
            ret = MFALSE;
            break;
    }
    //
    return ret;

}

