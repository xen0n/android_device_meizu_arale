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
#define LOG_TAG "Hal3Av3"

#include "Hal3AAdapter3.h"

//#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <utils/Atomic.h>
#include <cutils/properties.h>
//#include <camera_feature.h>

#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/v3/hal/dngInfo.h>
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/common/faces.h>
#include <mtkcam/Trace.h>

using namespace android;
using namespace NS3Av3;
using namespace NSCam;

#define GET_PROP(prop, dft, val)\
{\
   char value[PROPERTY_VALUE_MAX] = {'\0'};\
   property_get(prop, value, (dft));\
   (val) = atoi(value);\
}

#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define HAL3A_TEST_OVERRIDE (1)

#define HAL3AADAPTER3_LOG_SET_0 (1<<0)
#define HAL3AADAPTER3_LOG_SET_1 (1<<1)
#define HAL3AADAPTER3_LOG_SET_2 (1<<2)
#define HAL3AADAPTER3_LOG_GET_0 (1<<3)
#define HAL3AADAPTER3_LOG_GET_1 (1<<4)
#define HAL3AADAPTER3_LOG_GET_2 (1<<5)
#define HAL3AADAPTER3_LOG_GET_3 (1<<6)
#define HAL3AADAPTER3_LOG_GET_4 (1<<7)
#define HAL3AADAPTER3_LOG_PF    (1<<8)

#define HAL3A_REQ_PROC_KEY (2)
#define HAL3A_REQ_CAPACITY (HAL3A_REQ_PROC_KEY + 2)

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 Hal3AAdapter3::mu4LogEn = 0;

#if 0
inline static
MINT32 _convertAFMode(MINT32 i4Cam3Mode)
{
    MINT32 i4Cam1Mode;
    switch (i4Cam3Mode)
    {
    case MTK_CONTROL_AF_MODE_AUTO: // AF-Single Shot Mode
        i4Cam1Mode = NSFeature::AF_MODE_AFS;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE: // AF-Continuous Mode
        i4Cam1Mode = NSFeature::AF_MODE_AFC;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO: // AF-Continuous Mode (Video)
        i4Cam1Mode = NSFeature::AF_MODE_AFC_VIDEO;
        break;
    case MTK_CONTROL_AF_MODE_MACRO: // AF Macro Mode
        i4Cam1Mode = NSFeature::AF_MODE_MACRO;
        break;
    case MTK_CONTROL_AF_MODE_OFF: // Focus is set at infinity
        i4Cam1Mode = NSFeature::AF_MODE_INFINITY;
        break;
    default:
        i4Cam1Mode = NSFeature::AF_MODE_AFS;
        break;
    }
    return i4Cam1Mode;
}
#endif

inline static
CameraArea_T _normalizeArea(const CameraArea_T& rArea, const MRect& activeSize)
{
    CameraArea_T rOut;
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    rOut.i4Left = (2000 * (rArea.i4Left - activeSize.p.x) / activeSize.s.w) - 1000;
    rOut.i4Right = (2000 * (rArea.i4Right - activeSize.p.x) / activeSize.s.w) - 1000;
    rOut.i4Top = (2000 * (rArea.i4Top - activeSize.p.y) / activeSize.s.h) - 1000;
    rOut.i4Bottom = (2000 * (rArea.i4Bottom - activeSize.p.y) / activeSize.s.h) - 1000;
    rOut.i4Weight = rArea.i4Weight;
    #if 0
    MY_LOG("[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    #endif
    return rOut;
}

inline static
MINT32 _convertResultToMeta(MUINT32 u4SensorDev, const Result_T& rResult, const IMetadata& staticMeta, MetaSet_T& rMetaResult)
{
    MBOOL fgLogPf = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE: MFALSE;
    MBOOL fgLog = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_GET_3) ? MTRUE : MFALSE;
    MBOOL fgLogMap = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_GET_4) ? MTRUE : MFALSE;

    // convert result into metadata
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_SCENE_MODE, rResult.u1SceneMode);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AWB_STATE, rResult.u1AwbState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AE_STATE, rResult.u1AeState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AF_STATE, rResult.u1AfState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_STATE, rResult.u1LensState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_FLASH_STATE, rResult.u1FlashState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_STATISTICS_SCENE_FLICKER, rResult.u1SceneFlk);
    MY_LOG_IF(fgLogPf, "[%s] #(%d) AWB(%d) AE(%d) AF(%d) Lens(%d) Flash(%d) Flk(%d)", __FUNCTION__,
        rResult.i4FrmId, rResult.u1AwbState, rResult.u1AeState, rResult.u1AfState, rResult.u1LensState, rResult.u1FlashState, rResult.u1SceneFlk);

    // sensor
    DngInfo* pDngInfo = DngInfo::getInstance(u4SensorDev);
    IMetadata rMetaDngDynNoiseProfile = pDngInfo->getDynamicNoiseProfile(rResult.i4SensorSensitivity);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_EXPOSURE_TIME, rResult.i8SensorExposureTime);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_FRAME_DURATION, rResult.i8SensorFrameDuration);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_SENSITIVITY, rResult.i4SensorSensitivity);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_ROLLING_SHUTTER_SKEW, (MINT64)rResult.i8SensorRollingShutterSkew);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_GREEN_SPLIT, (MFLOAT) 0.0f);
    rMetaResult.appMeta += rMetaDngDynNoiseProfile;
    MY_LOG_IF(fgLogPf, "[%s] ExpTime(%lld) FrmDuration(%lld) ISO(%d) RSS(%lld)", __FUNCTION__,
        rResult.i8SensorExposureTime, rResult.i8SensorFrameDuration, rResult.i4SensorSensitivity, rResult.i8SensorRollingShutterSkew);

    // lens
    MFLOAT fFNum, fFocusLength;
    if (QUERY_ENTRY_SINGLE(staticMeta, MTK_LENS_INFO_AVAILABLE_APERTURES, fFNum))
    {
        UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_APERTURE, fFNum);
    }
    if (QUERY_ENTRY_SINGLE(staticMeta, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, fFocusLength))
    {
        UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_FOCAL_LENGTH, fFocusLength);
    }
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_FOCUS_DISTANCE, rResult.fLensFocusDistance);
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_LENS_FOCUS_RANGE, rResult.fLensFocusRange, 2);
    MY_LOG_IF(fgLog, "[%s] FocusDist(%f), FocusRange(%f, %f)", __FUNCTION__, rResult.fLensFocusDistance, rResult.fLensFocusRange[0], rResult.fLensFocusRange[1]);

    // wb
    MRational NeutralColorPt[3];
    for (MINT32 k = 0; k < 3; k++)
    {
        NeutralColorPt[k].denominator = rResult.i4AwbGain[k];
        NeutralColorPt[k].numerator = rResult.i4AwbGainScaleUint;
    }
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, (MUINT8) MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF);
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_COLOR_CORRECTION_GAINS, rResult.fColorCorrectGain, 4);
    MY_LOG_IF(fgLog, "[%s] Awb Gain(%3.6f, %3.6f, %3.6f, %3.6f)", __FUNCTION__,
            rResult.fColorCorrectGain[0], rResult.fColorCorrectGain[1], rResult.fColorCorrectGain[2], rResult.fColorCorrectGain[3]);

    // color correction matrix
    if (rResult.vecColorCorrectMat.size())
    {
        const MFLOAT* pfMat = rResult.vecColorCorrectMat.array();
        IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
        for (MINT32 k = 0; k < 9; k++)
        {
            MRational rMat;
            MFLOAT fVal = *pfMat++;
            rMat.numerator = fVal*512;
            rMat.denominator = 512;
            entry.push_back(rMat, Type2Type<MRational>());
            MY_LOG_IF(fgLog, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
        }
    rMetaResult.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
    }

    // shading
    MINT32 i4Size = rResult.vecShadingMap.size();
    if (i4Size)
    {
        UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_STATISTICS_LENS_SHADING_MAP, rResult.vecShadingMap.array(), i4Size);
    }

    // tonemap
    i4Size = rResult.vecTonemapCurveBlue.size();
    if (i4Size)
    {
        const MFLOAT* pCurve = rResult.vecTonemapCurveBlue.array();
        UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_TONEMAP_CURVE_BLUE, pCurve, i4Size);

        MY_LOG_IF(fgLogMap, "[%s] B size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
    }
    i4Size = rResult.vecTonemapCurveGreen.size();
    if (i4Size)
    {
        const MFLOAT* pCurve = rResult.vecTonemapCurveGreen.array();
        UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_TONEMAP_CURVE_GREEN, pCurve, i4Size);
        MY_LOG_IF(fgLogMap, "[%s] G size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
    }
    i4Size = rResult.vecTonemapCurveRed.size();
    if (i4Size)
    {
        const MFLOAT* pCurve = rResult.vecTonemapCurveRed.array();
        UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_TONEMAP_CURVE_RED, pCurve, i4Size);
        MY_LOG_IF(fgLogMap, "[%s] R size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
    }

    // tuning
    UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, rResult.rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));

    // Exif
    if (rResult.vecExifInfo.size())
    {
        IMetadata metaExif;
        const EXIF_3A_INFO_T& rExifInfo = rResult.vecExifInfo[0];
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FNUMBER,              fFNum*10/*rExifInfo.u4FNumber*/);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);
        //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_MODE,           rExifInfo.u4SceneMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AWB_MODE,             rExifInfo.u4AWBMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_LIGHT_SOURCE,         rExifInfo.u4LightSource);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_EXP_PROGRAM,          rExifInfo.u4ExpProgram);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_CAP_TYPE,       rExifInfo.u4SceneCapType);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  rExifInfo.u4FlashLightTimeus);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_METER_MODE,        rExifInfo.u4AEMeterMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_EXP_BIAS,          rExifInfo.i4AEExpBias);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    rExifInfo.u4CapExposureTime);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_ISO_SPEED,         rExifInfo.u4AEISOSpeed);
        //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_REAL_ISO_VALUE,       rExifInfo.u4RealISOValue);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
    }

    // log of control
    MINT32 rRgn[5];
    MUINT8 u1Mode;
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_CONTROL_AE_REGIONS, rRgn, 5))
    {
        MY_LOG_IF(fgLog, "[%s] MTK_CONTROL_AE_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_CONTROL_AF_REGIONS, rRgn, 5))
    {
        MY_LOG_IF(fgLog, "[%s] MTK_CONTROL_AF_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }
    if (QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, u1Mode))
    {
        MY_LOG_IF(fgLog, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    if (QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_NOISE_REDUCTION_MODE, u1Mode))
    {
        MY_LOG_IF(fgLog, "[%s] MTK_NOISE_REDUCTION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    double noise_profile[8];
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_SENSOR_NOISE_PROFILE, noise_profile, 8))
    {
        MY_LOG_IF(fgLog, "[%s] MTK_SENSOR_NOISE_PROFILE (%f,%f,%f,%f,%f,%f,%f,%f)", __FUNCTION__,
            noise_profile[0], noise_profile[1], noise_profile[2], noise_profile[3],
            noise_profile[4], noise_profile[5], noise_profile[6], noise_profile[7]);
    }

    return 0;
}

inline static
MVOID _updateMetadata(const IMetadata& src, IMetadata& dest)
{
    for (MUINT32 i = 0; i < src.count(); i++)
    {
        const IMetadata::IEntry& entry = src.entryAt(i);
        dest.update(entry.tag(), entry);
    }
}

inline static
MVOID _printStaticMetadata(const IMetadata& src)
{
    MUINT32 i;
    MBOOL fgLogPf = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE: MFALSE;
    for (i = 0; i < src.count(); i++)
    {
        MUINT32 j;
        const IMetadata::IEntry& entry = src.entryAt(i);
        MY_LOG_IF(fgLogPf, "[%s] Tag(0x%08x)", __FUNCTION__, entry.tag());
        switch (entry.tag())
        {
        case MTK_CONTROL_MAX_REGIONS:
        case MTK_SENSOR_INFO_SENSITIVITY_RANGE:
        case MTK_SENSOR_MAX_ANALOG_SENSITIVITY:
        case MTK_SENSOR_INFO_ORIENTATION:
        case MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES:
        case MTK_CONTROL_AE_COMPENSATION_RANGE:
            // MINT32
            for (j = 0; j < entry.count(); j++)
            {
                MINT32 val = entry.itemAt(j, Type2Type<MINT32>());
                MY_LOG_IF(fgLogPf, "[%s] val(%d)", __FUNCTION__, val);
            }
            break;
        case MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE:
        case MTK_SENSOR_INFO_MAX_FRAME_DURATION:
            // MINT64
            for (j = 0; j < entry.count(); j++)
            {
                MINT64 val = entry.itemAt(j, Type2Type<MINT64>());
                MY_LOG_IF(fgLogPf, "[%s] val(%lld)", __FUNCTION__, val);
            }
            break;
        case MTK_SENSOR_INFO_PHYSICAL_SIZE:
        case MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE:
        //case MTK_LENS_OPTICAL_AXIS_ANGLE:
        //case MTK_LENS_POSITION:
        case MTK_LENS_INFO_AVAILABLE_APERTURES:
        case MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES:
        case MTK_LENS_INFO_HYPERFOCAL_DISTANCE:
        case MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS:
            // MFLOAT
            for (j = 0; j < entry.count(); j++)
            {
                MFLOAT val = entry.itemAt(j, Type2Type<MFLOAT>());
                MY_LOG_IF(fgLogPf, "[%s] val(%f)", __FUNCTION__, val);
            }
            break;
        case MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES:
        case MTK_CONTROL_AE_AVAILABLE_MODES:
        case MTK_CONTROL_AF_AVAILABLE_MODES:
        case MTK_CONTROL_AWB_AVAILABLE_MODES:
        case MTK_CONTROL_AVAILABLE_EFFECTS:
        case MTK_CONTROL_AVAILABLE_SCENE_MODES:
        case MTK_CONTROL_SCENE_MODE_OVERRIDES:
        case MTK_SENSOR_INFO_FACING:
        case MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION:
        case MTK_INFO_SUPPORTED_HARDWARE_LEVEL:
            // MUINT8
            for (j = 0; j < entry.count(); j++)
            {
                MUINT8 val = entry.itemAt(j, Type2Type<MUINT8>());
                MY_LOG_IF(fgLogPf, "[%s] val(%d)", __FUNCTION__, val);
            }
            break;
        case MTK_CONTROL_AE_COMPENSATION_STEP:
        case MTK_SENSOR_BASE_GAIN_FACTOR:
            // MRational
            for (j = 0; j < entry.count(); j++)
            {
                MRational val = entry.itemAt(j, Type2Type<MRational>());
                MY_LOG_IF(fgLogPf, "[%s] val(%d/%d)", __FUNCTION__, val.numerator, val.denominator);
            }
            break;
        }
    }
}

#if HAL3A_TEST_OVERRIDE
inline static
MVOID _test(MUINT32 u4Flag, Param_T& rParam)
{
#define _TEST_MANUAL_SENSOR (1<<0)
#define _TEST_MANUAL_LENS   (1<<1)
#define _TEST_MANUAL_WB     (1<<2)
#define _TEST_EDGE_MODE     (1<<3)
#define _TEST_NR_MODE       (1<<4)
#define _TEST_SHADING       (1<<5)
#define _TEST_TORCH         (1<<6)
#define _TEST_FLK           (1<<7)
#define _TEST_AALOCK        (1<<8)
#define _TEST_BLK_LOCK      (1<<9)
#define _TEST_TONEMAP       (1<<10)
#define _TEST_TONEMAP2      (1<<11)
#define _TEST_CAP_SINGLE    (1<<14)
#define _TEST_EFFECT_MODE   (1<<15)
#define _TEST_SCENE_MODE    (1<<16)

    MUINT32 u4Magic = rParam.u4MagicNum;

    if (u4Flag & _TEST_MANUAL_SENSOR)
    {
        MUINT32 u4Sensor = 0;
        GET_PROP("debug.hal3av3.sensor", "0", u4Sensor);
        rParam.u4AeMode = MTK_CONTROL_AE_MODE_OFF;
        if (u4Sensor == 0)
        {
            rParam.i8ExposureTime = 3000000L + 3000000L * (u4Magic % 10);
            rParam.i4Sensitivity = 100;
            rParam.i8FrameDuration = 33000000L;
        }
        else if (u4Sensor == 1)
        {
            rParam.i8ExposureTime = 15000000L;
            rParam.i4Sensitivity = 100 + 100*(u4Magic%10);
            rParam.i8FrameDuration = 33000000L;
        }
        else if (u4Sensor == 2)
        {
            MINT32 iso = 100;
            GET_PROP("debug.hal3av3.iso", "100", iso);
            rParam.i8ExposureTime = 15000000L;
            rParam.i4Sensitivity = iso;
            rParam.i8FrameDuration = 33000000L;
        }
        else if (u4Sensor == 3)
        {
            if (u4Magic & 0x1)
            {
                rParam.i8ExposureTime = 15000000L;
                rParam.i4Sensitivity = 100;
            }
            else
            {
                MINT32 iso = 100;
                GET_PROP("debug.hal3av3.iso", "200", iso);
                rParam.i8ExposureTime = 7500000L;
                rParam.i4Sensitivity = iso;
            }
            rParam.i8FrameDuration = 33000000L;
        }
        else
        {
            rParam.i8ExposureTime = 3000000L;
            rParam.i4Sensitivity = 400;
            rParam.i8FrameDuration = 3000000L + 3000000L * (u4Magic % 10);
        }
    }

    if (u4Flag & _TEST_MANUAL_LENS)
    {
        static MFLOAT fDist[] = {50, 100, 150, 200, 400, 800, 1200, 2000, 3000};
        rParam.u4AfMode = MTK_CONTROL_AF_MODE_OFF;
        rParam.fFocusDistance = (MFLOAT) 1000.0f / fDist[u4Magic%9];
    }

    if (u4Flag & _TEST_MANUAL_WB)
    {
        MUINT32 u4Wb = 0;
        MUINT32 u4Channel = 0;
        GET_PROP("debug.hal3av3.wb", "0", u4Wb);
        rParam.u4AwbMode = MTRUE ? MTK_CONTROL_AWB_MODE_OFF : rParam.u4AwbMode;
        rParam.u1ColorCorrectMode = MTRUE ? MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX : rParam.u1ColorCorrectMode;
        u4Channel = u4Wb & 0x3;
        rParam.fColorCorrectGain[u4Channel] = (u4Magic % 2) ? 1.0f : 2.0f;
        rParam.fColorCorrectGain[(u4Channel+1)&0x3] = 1.0f;
        rParam.fColorCorrectGain[(u4Channel+2)&0x3] = 1.0f;
        rParam.fColorCorrectGain[(u4Channel+3)&0x3] = 1.0f;
        rParam.fColorCorrectMat[0] = 1.0f;
        rParam.fColorCorrectMat[1] = 0.0f;
        rParam.fColorCorrectMat[2] = 0.0f;
        rParam.fColorCorrectMat[3] = 0.0f;
        rParam.fColorCorrectMat[4] = 1.0f;
        rParam.fColorCorrectMat[5] = 0.0f;
        rParam.fColorCorrectMat[6] = 0.0f;
        rParam.fColorCorrectMat[7] = 0.0f;
        rParam.fColorCorrectMat[8] = 1.0f;
    }

    if (u4Flag & _TEST_EDGE_MODE)
    {
        MUINT32 u4Edge = 0;
        GET_PROP("debug.hal3av3.edge", "0", u4Edge);
        rParam.u1EdgeMode = u4Edge;
    }

    if (u4Flag & _TEST_NR_MODE)
    {
        MUINT32 u4NR = 0;
        GET_PROP("debug.hal3av3.nr", "0", u4NR);
        rParam.u1NRMode = u4NR;
    }

    if (u4Flag & _TEST_SHADING)
    {
        MUINT32 u4Shading = 0;
        GET_PROP("debug.hal3av3.shading", "0", u4Shading);
        rParam.u1ShadingMode = u4Shading;
    }

    if (u4Flag & _TEST_TORCH)
    {
        rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
        rParam.u4StrobeMode = MTK_FLASH_MODE_TORCH;
    }

    if (u4Flag & _TEST_FLK)
    {
        MUINT32 u4Flk = 0;
        GET_PROP("debug.hal3av3.flk", "0", u4Flk);
        rParam.u4AntiBandingMode = u4Flk;
    }

    if (u4Flag & _TEST_AALOCK)
    {
        MUINT32 u4AALock = 0;
        GET_PROP("debug.hal3av3.aalock", "0", u4AALock);
        rParam.bIsAELock = (u4AALock&0x1) ? MTRUE : MFALSE;
        rParam.bIsAWBLock = (u4AALock&0x2) ? MTRUE : MFALSE;
    }

    if (u4Flag & _TEST_BLK_LOCK)
    {
        MUINT32 u4Lock = 0;
        GET_PROP("debug.hal3av3.blklock", "0", u4Lock);
        rParam.u1BlackLvlLock = u4Lock ? MTRUE : MFALSE;
    }

    if (u4Flag & _TEST_TONEMAP)
    {
        const MINT32 i4Cnt = 32;
        static MFLOAT fCurve15[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0667f, 0.2920f, 0.1333f, 0.4002f, 0.2000f, 0.4812f,
            0.2667f, 0.5484f, 0.3333f, 0.6069f, 0.4000f, 0.6594f, 0.4667f, 0.7072f,
            0.5333f, 0.7515f, 0.6000f, 0.7928f, 0.6667f, 0.8317f, 0.7333f, 0.8685f,
            0.8000f, 0.9035f, 0.8667f, 0.9370f, 0.9333f, 0.9691f, 1.0000f, 1.0000f
        };

        static MFLOAT fCurve0[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0625f, 0.0625f, 0.1250f, 0.1250f, 0.2500f, 0.2500f,
            0.3125f, 0.3125f, 0.3750f, 0.3750f, 0.4375f, 0.4375f, 0.5000f, 0.5000f,
            0.5625f, 0.5625f, 0.6250f, 0.6250f, 0.6875f, 0.6875f, 0.7500f, 0.7500f,
            0.8125f, 0.8125f, 0.8750f, 0.8750f, 0.9375f, 0.9375f, 1.0000f, 1.0000f
        };

        MFLOAT fCurve[i4Cnt];

        MUINT32 u4Tonemap = 0;
        MUINT32 u4Idx = 0;
        GET_PROP("debug.hal3av3.tonemap", "0", u4Tonemap);
        if (u4Tonemap == 16)
        {
            u4Idx = u4Magic % 16;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }
        else if (u4Tonemap >= 17)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }
        else
        {
            u4Idx = u4Tonemap;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }

        if (u4Tonemap == MTK_TONEMAP_MODE_CONTRAST_CURVE)
        {
            MFLOAT fScale = (MFLOAT) u4Idx / 15.0f;
            for (MUINT32 i = 0; i < i4Cnt; i++)
            {
                fCurve[i] = ((1.0f-fScale)*fCurve0[i] + fScale*fCurve15[i]);
            }
            rParam.vecTonemapCurveBlue.resize(i4Cnt);
            rParam.vecTonemapCurveGreen.resize(i4Cnt);
            rParam.vecTonemapCurveRed.resize(i4Cnt);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
        }
    }

    if (u4Flag & _TEST_TONEMAP2)
    {
        MFLOAT fCurve[] =
        {
            0.000000,0.000000,
            0.032258,0.064516,
            0.064516,0.129032,
            0.096774,0.193548,
            0.129032,0.258065,
            0.161290,0.322581,
            0.193548,0.387097,
            0.225806,0.451613,
            0.258065,0.516129,
            0.290323,0.580645,
            0.322581,0.645161,
            0.354839,0.709677,
            0.387097,0.774194,
            0.419355,0.838710,
            0.451613,0.903226,
            0.483871,0.967742,
            0.516129,1.000000,
            0.548387,1.000000,
            0.580645,1.000000,
            0.612903,1.000000,
            0.645161,1.000000,
            0.677419,1.000000,
            0.709677,1.000000,
            0.741935,1.000000,
            0.774194,1.000000,
            0.806452,1.000000,
            0.838710,1.000000,
            0.870968,1.000000,
            0.903226,1.000000,
            0.935484,1.000000,
            0.967742,1.000000,
            1.000000,1.000000
        };

        MUINT32 u4Tonemap = 0;

        GET_PROP("debug.hal3av3.tonemap2", "0", u4Tonemap);

        if (u4Tonemap == 0)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 1)
        {
            MFLOAT fLinearCurve[] = {0.0f, 1.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 2)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 3)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 4)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 5)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 6)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 7)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 8)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 9)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
        rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        rParam.vecTonemapCurveBlue.resize(4);
        rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 10)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
        rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 11)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }
    }

    if (u4Flag & _TEST_CAP_SINGLE)
    {
        //rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH;
        MUINT32 u4Flash = 0;
        GET_PROP("debug.hal3av3.flash", "0", u4Flash);
        switch (u4Flash)
        {
        case 0:
            rParam.u4StrobeMode = MTK_FLASH_MODE_OFF;
            break;
        case 1:
            //if ((u4Magic % 5) == 0)
            {
                rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
                rParam.u4StrobeMode = MTK_FLASH_MODE_SINGLE;
            }
            break;
        case 2:
            {
                rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
                rParam.u4StrobeMode = MTK_FLASH_MODE_TORCH;
            }
            break;
        }
    }

    if (u4Flag & _TEST_SCENE_MODE)
    {
        MUINT32 u4Scene = 0;
        GET_PROP("debug.hal3av3.scene", "0", u4Scene);
        rParam.u1ControlMode = MTK_CONTROL_MODE_USE_SCENE_MODE;
        rParam.u4SceneMode = u4Scene;
    }

    if (u4Flag & _TEST_EFFECT_MODE)
    {
        MUINT32 u4Efct = 0;
        GET_PROP("debug.hal3av3.effect", "0", u4Efct);
        rParam.u4EffectMode = u4Efct;
    }
}
#endif

/*******************************************************************************
* implementations
********************************************************************************/
Hal3AAdapter3*
Hal3AAdapter3::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    MINT32 i4LogLevel = 0;
    GET_PROP("debug.camera.log", "0", i4LogLevel);
    GET_PROP("debug.hal3av3.log", "0", mu4LogEn);
    if (i4LogLevel) mu4LogEn |= HAL3AADAPTER3_LOG_PF;

    MY_LOG_IF(i4LogLevel, "[%s] i4SensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);
    switch (i4SensorIdx)
    {
    case 0:
        {
            static Hal3AAdapter3 _singleton(0);
            _singleton.init();
            return &_singleton;
        }
    case 1:
        {
            static Hal3AAdapter3 _singleton(1);
            _singleton.init();
            return &_singleton;
        }
    case 2:
        {
            static Hal3AAdapter3 _singleton(2);
            _singleton.init();
            return &_singleton;
        }
    case 3:
        {
            static Hal3AAdapter3 _singleton(3);
            _singleton.init();
            return &_singleton;
        }
    default:
        return NULL;
    }
}

Hal3AAdapter3::
Hal3AAdapter3(MINT32 const i4SensorIdx)
    : mpHal3aObj(NULL)
    , mi4FrmId(-1)
    , mi4SensorIdx(i4SensorIdx)
    , mi4User(0)
    , mu4Counter(0)
    , mLock()
    , mu1Start(0)
    , mu4MetaResultQueueCapacity(6)
{
/*
    if (! buildStaticInfo(mMetaStaticInfo))
    {
        CAM_LOGE("[%s] Static Info load error.", __FUNCTION__);
    }
*/
    MY_LOG_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF),
        "[%s] mi4SensorIdx(0x%04x)", __FUNCTION__, i4SensorIdx);
}

MVOID
Hal3AAdapter3::
destroyInstance(const char* strUser)
{
    MY_LOG_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] mi4SensorIdx(%d) %s", __FUNCTION__, mi4SensorIdx, strUser);
    uninit();
}

MINT32
Hal3AAdapter3::
start(MINT32 i4StartNum)
{
    MY_LOG("[%s] mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)",
            __FUNCTION__, mi4SensorIdx, mpHal3aObj);
#if 0
    mMetaResultQueue.clear();
    mParams = Param_T();
    setupStaticInfo();
#endif
    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);

    MY_LOG("[%s] Update 1", __FUNCTION__);
    NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None);
    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    _3AProf.i4MagicNum = 2;
    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    mu1Start = 1;
    return 0;
}

MINT32
Hal3AAdapter3::
stop()
{
    if (mu1Start)
    {
        MY_LOG("[%s] mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)",
                __FUNCTION__, mi4SensorIdx, mpHal3aObj);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewEnd);
        return 0;
    }
    else
    {
        MY_ERR("[%s] Not started yet! mi4SensorIdx(0x%04x), mpHal3aObj(0x%08x)",
                    __FUNCTION__, mi4SensorIdx, mpHal3aObj);
        return 1;
    }
}


MBOOL
Hal3AAdapter3::
init()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {

    }
    else
    {
        mu1CapIntent = -1; //MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mu4Counter = 0;
        mu1Start = 0;
        mu1FdEnable = 0;
        mpHal3aObj = Hal3AIf::createInstance(mi4SensorIdx);
        mu4SensorDev = mpHal3aObj->getSensorDev();
        MY_LOG("[%s] mi4SensorIdx(0x%04x), mu4SensorDev(0x%04x), mpHal3aObj(0x%08x)",
            __FUNCTION__, mi4SensorIdx, mu4SensorDev, mpHal3aObj);

        //sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(mi4SensorIdx);
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4SensorIdx);
        if (pMetadataProvider != NULL)
        {
            mMetaStaticInfo = pMetadataProvider->geMtktStaticCharacteristics();
            //setupStaticInfo();
            // DEBUG
            // _printStaticMetadata(mMetaStaticInfo);
        }
        else
        {
            MY_ERR("[%s] Fail to get static metadata: mi4SensorIdx(0x%04x)",
                __FUNCTION__, mi4SensorIdx);
        }

#if 1
        mMetaResultQueue.clear();
        mParams = Param_T();
        setupStaticInfo();
        mpHal3aObj->setParams(mParams);
#endif
        mpHal3aObj->attachCb(this);
    }

    android_atomic_inc(&mi4User);

    return MTRUE;
}

MBOOL
Hal3AAdapter3::
uninit()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            m_ScnModeOvrd.clear();
            mpHal3aObj->detachCb(this);
            mpHal3aObj->destroyInstance();
            mpHal3aObj = NULL;
            mMetaResultQueue.clear();
            MY_LOG("[%s] mi4SensorIdx(0x%04x)", __FUNCTION__, mi4SensorIdx);
        }
        else    // There are still some users.
        {
            //CAM_LOGD(m_bDebugEnable,"Still %d users \n", mi4User);
        }
    }

    return MTRUE;
}

#if 0
const IMetadata*
Hal3AAdapter3::
queryStaticInfo() const
{
    return &mMetaStaticInfo;
}
#endif

MINT32
Hal3AAdapter3::
buildSceneModeOverride()
{
    // override
    const IMetadata::IEntry& entryAvailableScene = mMetaStaticInfo.entryFor(MTK_CONTROL_AVAILABLE_SCENE_MODES);
    if (entryAvailableScene.isEmpty())
    {
        MY_ERR("[%s] MTK_CONTROL_AVAILABLE_SCENE_MODES are not defined", __FUNCTION__);
    }
    else
    {
        const IMetadata::IEntry& entryScnOvrd = mMetaStaticInfo.entryFor(MTK_CONTROL_SCENE_MODE_OVERRIDES);
        if (entryScnOvrd.isEmpty())
        {
            MY_ERR("[%s] MTK_CONTROL_SCENE_MODE_OVERRIDES are not defined", __FUNCTION__);
        }
        else
        {
            MUINT32 i;
            MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
            for (i = 0; i < entryAvailableScene.count(); i++)
            {
                MUINT8 u1ScnMode = entryAvailableScene.itemAt(i, Type2Type< MUINT8 >());
                if (entryScnOvrd.count() >= 3*i)
                {
                    // override
                    Mode3A_T rMode3A;
                    rMode3A.u1AeMode  = entryScnOvrd.itemAt(3*i,   Type2Type< MUINT8 >());
                    rMode3A.u1AwbMode = entryScnOvrd.itemAt(3*i+1, Type2Type< MUINT8 >());
                    rMode3A.u1AfMode  = entryScnOvrd.itemAt(3*i+2, Type2Type< MUINT8 >());
                    m_ScnModeOvrd.add(u1ScnMode, rMode3A);
                    MY_LOG_IF(fgLog, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, i, rMode3A.u1AeMode, rMode3A.u1AwbMode, rMode3A.u1AfMode);
                }
            }
        }
    }

    return 0;
}

MINT32
Hal3AAdapter3::
setupStaticInfo()
{
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;

    buildSceneModeOverride();

    // AE Comp Step
    MRational rStep;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_CONTROL_AE_COMPENSATION_STEP, rStep))
    {
        mParams.fExpCompStep = (MFLOAT) rStep.numerator / rStep.denominator;
        MY_LOG_IF(fgLog, "[%s] ExpCompStep(%3.3f), (%d/%d)", __FUNCTION__, mParams.fExpCompStep, rStep.numerator, rStep.denominator);
    }

    MINT32 rRgn[3];
    if (GET_ENTRY_ARRAY(mMetaStaticInfo, MTK_CONTROL_MAX_REGIONS, rRgn, 3))
    {
        m_rStaticInfo.i4MaxRegionAe  = rRgn[0];
        m_rStaticInfo.i4MaxRegionAwb = rRgn[1];
        m_rStaticInfo.i4MaxRegionAf  = rRgn[2];
        MY_LOG_IF(fgLog, "[%s] Max Regions AE(%d) AWB(%d) AF(%d)", __FUNCTION__, rRgn[0], rRgn[1], rRgn[2]);
    }

    // active array size for normalizing
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArraySize))
    {
        MY_LOG_IF(fgLog, "[%s] ActiveRgn(%d,%d,%d,%d)", __FUNCTION__, mActiveArraySize.p.x, mActiveArraySize.p.y, mActiveArraySize.s.w, mActiveArraySize.s.h);
    }
    return 0;
}

MINT32
Hal3AAdapter3::
set(const List<MetaSet_T>& requestQ)
{
#define OVERRIDE_AE  (1<<0)
#define OVERRIDE_AWB (1<<1)
#define OVERRIDE_AF  (1<<2)

    CAM_TRACE_CALL();

    MINT32 i;
    MINT32 i4FrmId = 0;
    MINT32 i4FrmIdCur = 0;
    MINT32 i4AfTrigId = 0;
    MINT32 i4AePreCapId = 0;
    MINT64 i8MinFrmDuration = 0;
    MUINT8 u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    MUINT8 u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    MUINT8 u1Override3A = (OVERRIDE_AE|OVERRIDE_AWB|OVERRIDE_AF);

    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
    MBOOL fgLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MBOOL fgLogEn1 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_1) ? MTRUE : MFALSE;
    MBOOL fgLogEn2 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_2) ? MTRUE : MFALSE;
    MUINT32 u4ManualMode = 0;

    MY_LOG_IF(fgLog, "[%s] mi4SensorIdx(%d), mu4Counter(%d), queueSize(%d)", __FUNCTION__, mi4SensorIdx, mu4Counter++, requestQ.size());

    mLock.lock();

    MetaSetList_T::const_iterator it = requestQ.begin();

    for (i = 0; i < getCapacity() && it != requestQ.end(); i++, it++)
    {
        MINT32 j, cnt;
        MRect rSclCropRect;
        MINT32 rSclCrop[4];
        MBOOL fgCrop = MFALSE;
        const IMetadata& metadata = it->appMeta;

        cnt = metadata.count();
        MY_LOG_IF(fgLogEn0, "[%s] metadata[%d], count(%d)", __FUNCTION__, i, cnt);

        if (i == HAL3A_REQ_PROC_KEY)
        {
            MBOOL fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
            mParams.u4MagicNum = i4FrmId;
            if (fgOK)
            {
                mMetaResult = *it;
                MY_LOG_IF(fgLog, "[%s] magic(%d)", __FUNCTION__, i4FrmId);
            }
            else
            {
                MY_ERR("fail to get magic(%d)", i4FrmId);
            }

            QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_MIN_FRM_DURATION, i8MinFrmDuration);

            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, mParams.u1IsGetExif);
            if (fgOK)
            {
                MY_LOG_IF(fgLog, "[%s] Request EXIF(%d)", __FUNCTION__, mParams.u1IsGetExif);
            }

            fgCrop = QUERY_ENTRY_SINGLE(metadata, MTK_SCALER_CROP_REGION, rSclCropRect);
            if (fgCrop)
            {
                rSclCrop[0] = rSclCropRect.p.x;
                rSclCrop[1] = rSclCropRect.p.y;
                rSclCrop[2] = rSclCropRect.p.x + rSclCropRect.s.w;
                rSclCrop[3] = rSclCropRect.p.y + rSclCropRect.s.h;
                MY_LOG_IF(fgLogEn0, "[%s] SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3]);
            }

            MUINT8 u1Dummy = 0;
            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_DUMMY, u1Dummy);
            MY_LOG_IF(fgOK && u1Dummy, "[%s] Request dummy", __FUNCTION__);
            HAL3A_SETBIT(mParams.u4HalFlag, HAL_FLG_DUMMY2, (u1Dummy ? HAL_FLG_DUMMY2 : 0));
        }
        else if (i == 0)
        {
            MBOOL fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmIdCur);
            if (fgOK)
            {
                MY_LOG_IF(fgLog, "[%s] current magic(%d)", __FUNCTION__, i4FrmIdCur);
            }
            else
            {
                MY_ERR("fail to get current magic(%d)", i4FrmIdCur);
            }

            MUINT8 u1Dummy = 0;
            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_DUMMY, u1Dummy);
            MY_LOG_IF(fgOK && u1Dummy, "[%s] current is dummy", __FUNCTION__);
            HAL3A_SETBIT(mParams.u4HalFlag, HAL_FLG_DUMMY, (u1Dummy ? HAL_FLG_DUMMY : 0));
            continue;
        }
        else
        {
            continue;
        }

        //
        for (j = 0; j < cnt; j++)
        {
            IMetadata::IEntry const& entry = metadata.entryAt(j);
            MUINT32 tag = entry.tag();
#if 0
            // for control only, ignore; for control+dynamic, not ignore.
            MBOOL fgControlOnly = MFALSE;

            // Only update 3A setting when delay matches.
            if (i != getDelay(tag)) continue;
#endif

            // convert metadata tag into 3A settings.
            switch (tag)
            {
            case MTK_CONTROL_MODE:  // dynamic
                {
                    MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1ControlMode = u1ControlMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_MODE(%d)", __FUNCTION__, u1ControlMode);
                }
                break;
            case MTK_CONTROL_CAPTURE_INTENT:
                {
                    u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1CaptureIntent = u1CapIntent;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_CAPTURE_INTENT(%d)", __FUNCTION__, u1CapIntent);
                }
                break;

            // AWB
            case MTK_CONTROL_AWB_LOCK:
                {
                    MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.bIsAWBLock = (bLock==MTK_CONTROL_AWB_LOCK_ON) ? MTRUE : MFALSE;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_LOCK(%d)", __FUNCTION__, bLock);
                }
                break;
            case MTK_CONTROL_AWB_MODE:  // dynamic
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AwbMode = u1Mode;
                    u1Override3A &= (~OVERRIDE_AWB);
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_MODE(%d)", __FUNCTION__, u1Mode);
                }
                break;
            case MTK_CONTROL_AWB_REGIONS:
                {
                    if (m_rStaticInfo.i4MaxRegionAwb == 0)
                    {
                        mMetaResult.appMeta.remove(MTK_CONTROL_AWB_REGIONS);
                    }
                    else
                    {
                        IMetadata::IEntry entryNew(MTK_CONTROL_AWB_REGIONS);
                        MINT32 numRgns = entry.count() / 5;
                        MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS(%d)", __FUNCTION__, numRgns);
                        for (MINT32 k = 0; k < numRgns; k++)
                        {
                            CameraArea_T rArea;
                            rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                            rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                            rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                            rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                            rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                            if (fgCrop)
                            {
                                rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                            }
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                        }
                        if (fgCrop)
                        {
                            mMetaResult.appMeta.update(MTK_CONTROL_AWB_REGIONS, entryNew);
                        }
                    }
                }
                break;

            // AE
            case MTK_CONTROL_AE_ANTIBANDING_MODE:
                {
                    MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AntiBandingMode = i4Mode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_ANTIBANDING_MODE(%d)", __FUNCTION__, i4Mode);
                }
                break;
            case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                {
                    MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                    mParams.i4ExpIndex = i4ExpIdx;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d)", __FUNCTION__, i4ExpIdx);
                }
                break;
            case MTK_CONTROL_AE_LOCK:
                {
                    MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.bIsAELock = (bLock==MTK_CONTROL_AE_LOCK_ON) ? MTRUE : MFALSE;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_LOCK(%d)", __FUNCTION__, bLock);
                }
                break;
            case MTK_CONTROL_AE_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AeMode = u1Mode;
                    u1Override3A &= (~OVERRIDE_AE);
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_MODE(%d)", __FUNCTION__, u1Mode);
                }
                break;
            case MTK_CONTROL_AE_REGIONS:    // dynamic
                {
                    if (m_rStaticInfo.i4MaxRegionAe == 0)
                    {
                        mMetaResult.appMeta.remove(MTK_CONTROL_AE_REGIONS);
                    }
                    else
                    {
                        IMetadata::IEntry entryNew(MTK_CONTROL_AE_REGIONS);
                        MINT32 numRgns = entry.count() / 5;
                        mParams.rMeteringAreas.u4Count = numRgns;
                        MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS(%d)", __FUNCTION__, numRgns);
                        for (MINT32 k = 0; k < numRgns; k++)
                        {
                            CameraArea_T& rArea = mParams.rMeteringAreas.rAreas[k];
                            rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                            rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                            rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                            rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                            rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                            CameraArea_T rTmpArea = rArea;
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                            if (fgCrop)
                            {
                                 rTmpArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                 rTmpArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                 rTmpArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                 rTmpArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                 entryNew.push_back(rTmpArea.i4Left,   Type2Type<MINT32>());
                                 entryNew.push_back(rTmpArea.i4Top,    Type2Type<MINT32>());
                                 entryNew.push_back(rTmpArea.i4Right,  Type2Type<MINT32>());
                                 entryNew.push_back(rTmpArea.i4Bottom, Type2Type<MINT32>());
                                 entryNew.push_back(rTmpArea.i4Weight, Type2Type<MINT32>());
                                if (rArea.i4Left!=0 || rArea.i4Top!=0 || rArea.i4Right!=0 || rArea.i4Bottom!=0)
                                {
                                      rArea = _normalizeArea(rTmpArea, mActiveArraySize);
                                }
                            }
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                        }
                        if (fgCrop)
                        {
                            mMetaResult.appMeta.update(MTK_CONTROL_AE_REGIONS, entryNew);
                        }
                    }
                }
                break;
            case MTK_CONTROL_AE_TARGET_FPS_RANGE:
                {
                    MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
                    MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
                    mParams.i4MinFps = i4MinFps*1000;
                    mParams.i4MaxFps = i4MaxFps*1000;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_TARGET_FPS_RANGE(%d,%d)", __FUNCTION__, i4MinFps, i4MaxFps);
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
                {
                    u1AePrecapTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1PrecapTrig = u1AePrecapTrig;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d)", __FUNCTION__, u1AePrecapTrig);
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_ID:
                {
                    i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_ID(%d)", __FUNCTION__, i4AePreCapId);
                }
                break;

            case MTK_FLASH_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4StrobeMode = u1Mode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_FLASH_MODE(%d)", __FUNCTION__, u1Mode);
                }
                break;

            // Sensor
            case MTK_SENSOR_EXPOSURE_TIME:
                {
                    MINT64 i8AeExposureTime = entry.itemAt(0, Type2Type< MINT64 >());
                    mParams.i8ExposureTime = i8AeExposureTime;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_SENSOR_EXPOSURE_TIME(%lld)", __FUNCTION__, i8AeExposureTime);
                }
                break;
            case MTK_SENSOR_SENSITIVITY:
                {
                    MINT32 i4AeSensitivity = entry.itemAt(0, Type2Type< MINT32 >());
                    mParams.i4Sensitivity = i4AeSensitivity;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_SENSOR_SENSITIVITY(%d)", __FUNCTION__, i4AeSensitivity);
                }
                break;
            case MTK_SENSOR_FRAME_DURATION:
                {
                    MINT64 i8FrameDuration = entry.itemAt(0, Type2Type< MINT64 >());
                    mParams.i8FrameDuration = i8FrameDuration;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_SENSOR_FRAME_DURATION(%lld)", __FUNCTION__, i8FrameDuration);
                }
                break;
            case MTK_BLACK_LEVEL_LOCK:
                {
                    MUINT8 u1BlackLvlLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1BlackLvlLock = u1BlackLvlLock;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_BLACK_LEVEL_LOCK(%d)", __FUNCTION__, u1BlackLvlLock);
                }
                break;

            // AF
            case MTK_CONTROL_AF_MODE:
                {
                    MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4AfMode = u1AfMode; //_convertAFMode(u1AfMode);
                    u1Override3A &= (~OVERRIDE_AF);
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_MODE(%d)", __FUNCTION__, u1AfMode);
                }
                break;
            case MTK_CONTROL_AF_REGIONS:    // dynamic
                {
                    if (m_rStaticInfo.i4MaxRegionAf == 0)
                    {
                        mMetaResult.appMeta.remove(MTK_CONTROL_AF_REGIONS);
                    }
                    else
                    {
                        IMetadata::IEntry entryNew(MTK_CONTROL_AF_REGIONS);
                        MINT32 numRgns = entry.count() / 5;
                        mParams.rFocusAreas.u4Count = numRgns;
                        MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS(%d)", __FUNCTION__, numRgns);
                        for (MINT32 k = 0; k < numRgns; k++)
                        {
                            CameraArea_T& rArea = mParams.rFocusAreas.rAreas[k];
                            rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                            rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                            rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                            rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                            rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                            if (fgCrop)
                            {
                                rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                rArea = _normalizeArea(rArea, mActiveArraySize);
                            }
                            MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                        }
                        if (fgCrop)
                        {
                            mMetaResult.appMeta.update(MTK_CONTROL_AF_REGIONS, entryNew);
                        }
                    }
                }
                break;
            case MTK_CONTROL_AF_TRIGGER:
                {
                    u1AfTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER(%d)", __FUNCTION__, u1AfTrig);
                }
                break;
            case MTK_CONTROL_AF_TRIGGER_ID:
                {
                    i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER_ID(%d)", __FUNCTION__, i4AfTrigId);
                }
                break;

            // Lens
            case MTK_LENS_FOCUS_DISTANCE:
                {
                    MFLOAT fFocusDist = entry.itemAt(0, Type2Type< MFLOAT >());
                    mParams.fFocusDistance = fFocusDist;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_LENS_FOCUS_DISTANCE(%3.6f)", __FUNCTION__, fFocusDist);
                }
                break;

            // ISP
            case MTK_CONTROL_EFFECT_MODE:
                {
                    MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4EffectMode = u1EffectMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_EFFECT_MODE(%d)", __FUNCTION__, u1EffectMode);
                }
                break;
            case MTK_CONTROL_SCENE_MODE:
                {
                    MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u4SceneMode = u1SceneMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_CONTROL_SCENE_MODE(%d)", __FUNCTION__, u1SceneMode);
                }
                break;
            case MTK_EDGE_MODE:
                {
                    MUINT8 u1EdgeMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1EdgeMode = u1EdgeMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_EDGE_MODE(%d)", __FUNCTION__, u1EdgeMode);
                }
                break;
            case MTK_NOISE_REDUCTION_MODE:
                {
                    MUINT8 u1NRMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    mParams.u1NRMode = u1NRMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_NOISE_REDUCTION_MODE(%d)", __FUNCTION__, u1NRMode);
                }
                break;

            // Color correction
            case MTK_COLOR_CORRECTION_MODE:
                {
                    MUINT8 u1ColorCrctMode = entry.itemAt(0, Type2Type<MUINT8>());
                    mParams.u1ColorCorrectMode = u1ColorCrctMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_MODE(%d)", __FUNCTION__, u1ColorCrctMode);
                }
                break;
            case MTK_COLOR_CORRECTION_GAINS:
                {
                    // [R G_even G_odd B]
                    MFLOAT fGains[4];
                    mParams.fColorCorrectGain[0] = fGains[0] = entry.itemAt(0, Type2Type<MFLOAT>());
                    mParams.fColorCorrectGain[1] = fGains[1] = entry.itemAt(1, Type2Type<MFLOAT>());
                    mParams.fColorCorrectGain[2] = fGains[2] = entry.itemAt(2, Type2Type<MFLOAT>());
                    mParams.fColorCorrectGain[3] = fGains[3] = entry.itemAt(3, Type2Type<MFLOAT>());
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_GAINS(%f, %f, %f, %f)", __FUNCTION__, fGains[0], fGains[1], fGains[2], fGains[3]);
                }
                break;
            case MTK_COLOR_CORRECTION_TRANSFORM:
                {
                    for (MINT32 k = 0; k < 9; k++)
                    {
                        MRational rMat;
                        MFLOAT fVal = 0.0f;
                        rMat = entry.itemAt(k, Type2Type<MRational>());
                        mParams.fColorCorrectMat[k] = fVal = rMat.denominator ? (MFLOAT)rMat.numerator / rMat.denominator : 0.0f;
                        MY_LOG_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_TRANSFORM rMat[%d]=(%d/%d)=(%3.6f)", __FUNCTION__, k, rMat.numerator, rMat.denominator, fVal);
                    }
                }
                break;
            case MTK_COLOR_CORRECTION_ABERRATION_MODE:
                {
                    MUINT8 u1ColorAberrationMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1ColorAberrationMode);
                }
                break;
#if 1
            // Tonemap
            case MTK_TONEMAP_MODE:
                {
                    MUINT8 u1TonemapMode = entry.itemAt(0, Type2Type<MUINT8>());
                    mParams.u1TonemapMode = u1TonemapMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_TONEMAP_MODE(%d)", __FUNCTION__, u1TonemapMode);
                }
                break;
            case MTK_TONEMAP_CURVE_BLUE:
                {
                    MUINT32 u4Cnt = entry.count();
                    mParams.vecTonemapCurveBlue.resize(u4Cnt);
                    MFLOAT* fVec = mParams.vecTonemapCurveBlue.editArray();
                    for (MUINT32 i = 0; i < u4Cnt; i++)
                    {
                        *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                    }
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_BLUE(%d)", __FUNCTION__, u4Cnt);
                }
                break;
            case MTK_TONEMAP_CURVE_GREEN:
                {
                    MUINT32 u4Cnt = entry.count();
                    mParams.vecTonemapCurveGreen.resize(u4Cnt);
                    MFLOAT* fVec = mParams.vecTonemapCurveGreen.editArray();
                    for (MUINT32 i = 0; i < u4Cnt; i++)
                    {
                        *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                    }
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_GREEN(%d)", __FUNCTION__, u4Cnt);
                }
                break;
            case MTK_TONEMAP_CURVE_RED:
                {
                    MUINT32 u4Cnt = entry.count();
                    mParams.vecTonemapCurveRed.resize(u4Cnt);
                    MFLOAT* fVec = mParams.vecTonemapCurveRed.editArray();
                    for (MUINT32 i = 0; i < u4Cnt; i++)
                    {
                        *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                    }
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_RED(%d)", __FUNCTION__, u4Cnt);
                }
                break;
#endif
            // Shading
            case MTK_SHADING_MODE:
                {
                    MUINT8 u1ShadingMode = entry.itemAt(0, Type2Type<MUINT8>());
                    mParams.u1ShadingMode = u1ShadingMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_SHADING_MODE(%d)", __FUNCTION__, u1ShadingMode);
                }
                break;
            case MTK_STATISTICS_LENS_SHADING_MAP_MODE:
                {
                    MUINT8 u1ShadingMapMode = entry.itemAt(0, Type2Type<MUINT8>());
                    mParams.u1ShadingMapMode = u1ShadingMapMode;
                    MY_LOG_IF(fgLogEn1, "[%s] MTK_STATISTICS_LENS_SHADING_MAP_MODE(%d)", __FUNCTION__, u1ShadingMapMode);
                }
                break;

            default:
                break;
            }
#if 0
            if (!fgControlOnly && (HAL3A_REQ_PROC_KEY == i))
            {
                // update result for control/dynamic, including MTK_REQUEST_FRAME_COUNT
                mMetaResult.appMeta.update(tag, entry);
            }
#endif
        }
    }

    mParams.u1CaptureIntent = u1CapIntent;

    // push request to result queue
    mMetaResultQueue.push_back(mMetaResult);
    // remove if full
    if (mMetaResultQueue.size() > mu4MetaResultQueueCapacity)
    {
        MY_LOG_IF(fgLogEn1, "[%s] Queue fulls (%d). Remove front", __FUNCTION__, mMetaResultQueue.size());
        mMetaResultQueue.erase(mMetaResultQueue.begin());
    }
    mLock.unlock();

#if HAL3A_TEST_OVERRIDE
    GET_PROP("debug.hal3av3.manual", "0", u4ManualMode);
    _test(u4ManualMode, mParams);
#endif

    // control mode and scene mode
    switch (mParams.u1ControlMode)
    {
    case MTK_CONTROL_MODE_OFF:
        mParams.u4AeMode  = MTK_CONTROL_AE_MODE_OFF;
        mParams.u4AwbMode = MTK_CONTROL_AWB_MODE_OFF;
        mParams.u4AfMode  = MTK_CONTROL_AF_MODE_OFF;
        mParams.u4EffectMode = MTK_CONTROL_EFFECT_MODE_OFF;
        break;
    case MTK_CONTROL_MODE_USE_SCENE_MODE:
        switch (mParams.u4SceneMode)
        {
        case MTK_CONTROL_SCENE_MODE_DISABLED:
        case MTK_CONTROL_SCENE_MODE_FACE_PRIORITY:
            break;
        default:
            {
                if (!m_ScnModeOvrd.isEmpty())
                {
                    Mode3A_T rOverride = m_ScnModeOvrd.valueFor((MUINT8)mParams.u4SceneMode);
                    mParams.u4AeMode  = (u1Override3A & OVERRIDE_AE) ? rOverride.u1AeMode : mParams.u4AeMode;
                    mParams.u4AwbMode = (u1Override3A & OVERRIDE_AWB) ? rOverride.u1AwbMode : mParams.u4AwbMode;
                    mParams.u4AfMode  = (u1Override3A & OVERRIDE_AF) ? rOverride.u1AfMode : mParams.u4AfMode; //_convertAFMode(rOverride.u1AfMode);
                    MY_LOG_IF(fgLogEn2, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, mParams.u4SceneMode, mParams.u4AeMode, mParams.u4AwbMode, mParams.u4AfMode);
                }
                else
                {
                    MY_ERR("[%s] Scene mode(%d) overrides Fails", __FUNCTION__, mParams.u4SceneMode);
                }
            }
            break;
        }
        break;
    default:
        break;
    }

    // set m_Params
    mpHal3aObj->setFrameId(i4FrmId);

    // face priority
    if (mParams.u4SceneMode == MTK_CONTROL_SCENE_MODE_FACE_PRIORITY)
    {
        if (mu1FdEnable != MTRUE)
        {
            mpHal3aObj->setFDEnable(MTRUE);
            mu1FdEnable = MTRUE;
        }
    }
    else
    {
        if (mu1FdEnable != MFALSE)
        {
            MtkCameraFaceMetadata rFaceMeta;
            MtkCameraFace rFace;
            rFaceMeta.number_of_faces = 0;
            rFaceMeta.faces = &rFace;
            rFaceMeta.posInfo = NULL;
            ::memset(&rFace, 0, sizeof(MtkCameraFace));
            mpHal3aObj->setFDInfo(&rFaceMeta);
            mpHal3aObj->setFDEnable(MFALSE);
            mu1FdEnable = MFALSE;
        }
    }

    // AF trigger cancel
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
    {
        mpHal3aObj->cancelAutoFocus();
    }

    // workaround for p1 frame rate
    if (i8MinFrmDuration > 0)
    {
        MINT64 i4MaxFps = (1000000000LL*1000LL) / i8MinFrmDuration;
        mParams.i4MaxFps = mParams.i4MaxFps >= i4MaxFps ? i4MaxFps : mParams.i4MaxFps;
        if( mParams.i4MinFps > mParams.i4MaxFps )
            mParams.i4MinFps = mParams.i4MaxFps;
        mParams.i8FrameDuration = mParams.i8FrameDuration < i8MinFrmDuration ? i8MinFrmDuration : mParams.i8FrameDuration;
    }

    // set m_Params
    mpHal3aObj->setParams(mParams);

    // AF trigger start
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        mpHal3aObj->autoFocus();
    }

    if (u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START &&
        mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
    {
        MY_LOG_IF(fgLog, "[%s] Precapture Trigger @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureStart);
    }

    /*if (u1CapIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
    {
        CAM_LOGD("[%s] Capture Start @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        NS3Av3::ParamIspProfile_T _3AProf(NS3Av3::EIspProfile_Capture, i4FrmId, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_CaptureStart, reinterpret_cast<MINT32>(&_3AProf));
    }
    else*/
    {
        NSIspTuning::EIspProfile_T prof = NSIspTuning::EIspProfile_Preview;
    #if 0
        switch (u1CapIntent)
        {
        case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            prof = NSIspTuning::EIspProfile_Preview;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
            prof = NSIspTuning::EIspProfile_Capture;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
            prof = NSIspTuning::EIspProfile_VSS_Capture;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            prof = NSIspTuning::EIspProfile_Video;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
            prof = NSIspTuning::EIspProfile_ZSD_Capture;
            break;
        }
    #endif
        MY_LOG_IF(fgLog,"[%s] Update @ i4FrmId(%d), dummy(%d)", __FUNCTION__, i4FrmId, (mParams.u4HalFlag & HAL_FLG_DUMMY2) ? 1 : 0);
        NS3Av3::ParamIspProfile_T _3AProf(prof, i4FrmId, i4FrmIdCur, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    }

    return 0;
}

MINT32
Hal3AAdapter3::
get(MUINT32 frmId, MetaSet_T& result)
{
    Result_T rResult;
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;

    Mutex::Autolock lock(mLock);

    MY_LOG_IF(fgLog, "[%s] mi4SensorIdx(%d) R(%d)", __FUNCTION__, mi4SensorIdx, frmId);

    // 1. get result (dynamic) of x from 3AMgr
    if (-1 == mpHal3aObj->getResult(frmId, rResult))
    {
        CAM_LOGW("[%s] Fail to get R[%d], current result will be obtained.", __FUNCTION__, frmId);
    }

    // 2. copy result at x to MW
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
        if (frmId == u4Id)
        {
            _updateMetadata(it->appMeta, result.appMeta);
            _updateMetadata(it->halMeta, result.halMeta);
            _convertResultToMeta(mu4SensorDev, rResult, mMetaStaticInfo, result);
            break;
        }
    }

    // 3. fail to get the specified result, use current.
    if (it == mMetaResultQueue.end())
    {
        MINT32 i4Id = 0;
        QUERY_ENTRY_SINGLE(mMetaResult.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4Id);
        CAM_LOGW("[%s] fail to get the specified results, use current. i4Id(%d)", __FUNCTION__, i4Id);
        _updateMetadata(mMetaResult.appMeta, result.appMeta);
        _updateMetadata(mMetaResult.halMeta, result.halMeta);
        _convertResultToMeta(mu4SensorDev, rResult, mMetaStaticInfo, result);
    }

    return 0;
}

MINT32
Hal3AAdapter3::
getCur(MUINT32 frmId, MetaSet_T& result)
{
    Result_T rResult;
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;

    Mutex::Autolock lock(mLock);

    MY_LOG_IF(fgLog, "[%s] mi4SensorIdx(%d) R(%d)", __FUNCTION__, mi4SensorIdx, frmId);

    // 1. get result (dynamic) of x from 3AMgr
    if (-1 == mpHal3aObj->getResultCur(frmId, rResult))
    {
        CAM_LOGW("[%s] Fail to get R[%d], current result will be obtained.", __FUNCTION__, frmId);
    }

    // 2. copy result at x to MW
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
        if (frmId == u4Id)
        {
            _updateMetadata(it->appMeta, result.appMeta);
            _updateMetadata(it->halMeta, result.halMeta);
            _convertResultToMeta(mu4SensorDev, rResult, mMetaStaticInfo, result);
            break;
        }
    }

    // 3. fail to get the specified result, use current.
    if (it == mMetaResultQueue.end())
    {
        MINT32 i4Id = 0;
        QUERY_ENTRY_SINGLE(mMetaResult.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4Id);
        CAM_LOGW("[%s] fail to get the specified results, use current. i4Id(%d)", __FUNCTION__, i4Id);
        _updateMetadata(mMetaResult.appMeta, result.appMeta);
        _updateMetadata(mMetaResult.halMeta, result.halMeta);
        _convertResultToMeta(mu4SensorDev, rResult, mMetaStaticInfo, result);
    }

    return 0;
}

MINT32
Hal3AAdapter3::
setIsp(MINT32 flowType, const MetaSet_T& control, void* pRegBuf, MetaSet_T* pResult)
{
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;

    MY_LOG_IF(fgLog, "[%s]+ flowType(%d), pRegBuf(%p)", __FUNCTION__, flowType, pRegBuf);
    NSIspTuning::RAWIspCamInfo_U rCamInfo;
    if (GET_ENTRY_ARRAY(control.halMeta, MTK_PROCESSOR_CAMINFO, rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U)))
    {
        NSIspTuning::RAWIspCamInfo rRawInfo;
        UtilConvertCamInfo(rCamInfo, rRawInfo);
        MUINT8 u1IspProfile = NSIspTuning::EIspProfile_Preview;
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_PROFILE, u1IspProfile))
        {
            rRawInfo.eIspProfile = (NSIspTuning::EIspProfile_T)u1IspProfile;
        }
        MUINT8 u1PGN = MFALSE;
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_PGN_ENABLE, u1PGN))
        {
            rRawInfo.fgRPGEnable = !u1PGN;
        }
        mpHal3aObj->setIspPass2(flowType, rRawInfo, pRegBuf);
        MUINT8 u1Exif = MFALSE;
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, u1Exif) && pResult)
        {
            IMetadata exifMeta;
            if (QUERY_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, exifMeta))
            {
                Vector<MINT32> keys;
                Vector< Vector<MUINT8> > data;
                if (!mpHal3aObj->getDebugInfo(keys, data))
                {
                    MY_ERR("Fail to get debug info");
                }
                else
                {
                    MY_LOG_IF(fgLog, "[%s] get debug info OK", __FUNCTION__);

                    for (MUINT32 i = 0; i < keys.size(); i++)
                    {
                        UPDATE_ENTRY_SINGLE(exifMeta, MTK_3A_EXIF_DEBUGINFO_BEGIN+i*2, keys[i]);
                        IMetadata::Memory data_i;
                        data_i.resize(data[i].size());
                        ::memcpy(data_i.editArray(), data[i].array(), data_i.size());
                        UPDATE_ENTRY_SINGLE(exifMeta, MTK_3A_EXIF_DEBUGINFO_BEGIN+i*2+1, data_i);
                    }
                    UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, exifMeta);
                }
            }
        }
    }
    MY_LOG_IF(fgLog, "[%s]-", __FUNCTION__);
    return 0;
}

MINT32
Hal3AAdapter3::
attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    MY_LOG_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] eId(%d), pCb(%p)", __FUNCTION__, eId, pCb);
    return m_CbSet[eId].addCallback(pCb);
}

MINT32
Hal3AAdapter3::
detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    MY_LOG_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] eId(%d), pCb(%p)", __FUNCTION__, eId, pCb);
    return m_CbSet[eId].removeCallback(pCb);
}

MINT32
Hal3AAdapter3::
getDelay(IMetadata& delay_info) const
{
    return 0;
}

MINT32
Hal3AAdapter3::
getDelay(MUINT32 tag) const
{
    // must be >= HAL3A_MIN_PIPE_LATENCY
    MINT32 i4Delay = 2;
    #if 0
    // temp
    switch (tag)
    {
    case MTK_REQUEST_FRAME_COUNT:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AE_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_MODE:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_REGIONS:
        i4Delay = 3;
        break;
    //case MTK_CONTROL_AF_APERTURE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCALLENGTH:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCUSDISTANCE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_OIS:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_SHARPNESSMAPMODE:
    //    i4Delay = 1;
    //    break;

    case MTK_CONTROL_AWB_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AWB_MODE:
        i4Delay = 3;
        break;
    }
    #endif
    return i4Delay;
}

MINT32
Hal3AAdapter3::
getCapacity() const
{
    return HAL3A_REQ_CAPACITY;
}

MVOID
Hal3AAdapter3::
setSensorMode(MINT32 i4SensorMode)
{
    mpHal3aObj->setSensorMode(i4SensorMode);
}

MVOID
Hal3AAdapter3::
notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg)
{
    //CAM_LOGD("[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    mpHal3aObj->notifyP1Done(u4MagicNum, pvArg);
}

MVOID
Hal3AAdapter3::
setFDEnable(MBOOL fgEnable)
{
    mpHal3aObj->setFDEnable(fgEnable);
}

MBOOL
Hal3AAdapter3::
setFDInfo(MVOID* prFaces)
{
    return mpHal3aObj->setFDInfo(prFaces);
}

MINT32
Hal3AAdapter3::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2)
{
    return mpHal3aObj->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
}

void
Hal3AAdapter3::
doNotifyCb(MINT32 _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3)
{
    switch (_msgType)
    {
    case eID_NOTIFY_3APROC_FINISH:
        MY_LOG_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3);
        m_CbSet[IHal3ACb::eID_NOTIFY_3APROC_FINISH].doNotifyCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_READY2CAP:
        m_CbSet[IHal3ACb::eID_NOTIFY_READY2CAP].doNotifyCb(IHal3ACb::eID_NOTIFY_READY2CAP, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_CURR_RESULT:
        m_CbSet[IHal3ACb::eID_NOTIFY_CURR_RESULT].doNotifyCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, _ext1, _ext2, _ext3);
        break;
    default:
        break;
    }
}

void
Hal3AAdapter3::
doDataCb(int32_t _msgType, void* _data, uint32_t _size)
{}

//====================
Hal3ACbSet::
Hal3ACbSet()
    : m_Mutex()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

Hal3ACbSet::
~Hal3ACbSet()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

void
Hal3ACbSet::
doNotifyCb (
    MINT32  _msgType,
    MINTPTR _ext1,
    MINTPTR _ext2,
    MINTPTR _ext3
)
{
    Mutex::Autolock autoLock(m_Mutex);

    //CAM_LOGD("[Hal3ACbSet::%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3);
    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        IHal3ACb* pCb = *it;
        //CAM_LOGD("[%s] pCb(%p)", __FUNCTION__, pCb);
        if (pCb)
        {
            pCb->doNotifyCb(_msgType, _ext1, _ext2, _ext3);
        }
    }
}

MINT32
Hal3ACbSet::
addCallback(IHal3ACb* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;

    if (cb == NULL)
    {
        MY_ERR("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
            break;
    }

    if (i4Cnt == (MINT32) m_CallBacks.size())
    {
        // not exist, add
        m_CallBacks.push_back(cb);
        return m_CallBacks.size();
    }
    else
    {
        // already exists
        return -m_CallBacks.size();
    }
}

MINT32
Hal3ACbSet::
removeCallback(IHal3ACb* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;
    MINT32 i4Size = m_CallBacks.size();

    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
        {
            m_CallBacks.erase(it);
            // successfully removed
            return m_CallBacks.size();
        }
    }

    // cannot be found
    return -i4Size;
}

