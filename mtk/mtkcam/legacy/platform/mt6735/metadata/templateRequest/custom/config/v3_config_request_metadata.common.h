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


REQUEST_METADATA_BEGIN(COMMON)
//------------------------------------------------------------------------------
//  android.color
//------------------------------------------------------------------------------
    //==========================================================================// none
     CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_TRANSFORM)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
     CONFIG_METADATA_END()
    //==========================================================================//New none
    CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_GAINS)
        CONFIG_ENTRY_VALUE( 1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE( 1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE( 1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE( 1.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================//New none
    CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_ABERRATION_MODE)
        CONFIG_ENTRY_VALUE( MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF , MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.control
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_ANTIBANDING_MODE)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO, MUINT8)//!CONTROL_AE_ANTIBANDING_MODE_OFF
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_EXPOSURE_COMPENSATION)//0
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_LOCK)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_LOCK_OFF, MUINT8)//false
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_REGIONS)//
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================//new
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_PRECAPTURE_TRIGGER)//CONTROL_AE_PRECAPTURE_TRIGGER_IDLE
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_REGIONS)//
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================//new
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_TRIGGER)// CONTROL_AF_TRIGGER_IDLE
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_TRIGGER_IDLE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_LOCK)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_LOCK_OFF, MUINT8)//false
    CONFIG_METADATA_END()
    //==========================================================================
    /*CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_OFF, MUINT8)//auto, default: off
    CONFIG_METADATA_END()*/
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_REGIONS)//
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_CONTROL_EFFECT_MODE)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    /*CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
        CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()*/
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_CONTROL_SCENE_MODE)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_SCENE_MODE_DISABLED, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_CONTROL_VIDEO_STABILIZATION_MODE)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.demosaic
//------------------------------------------------------------------------------
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
        CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_FAST, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.flash
//------------------------------------------------------------------------------
    //==========================================================================//none FUTURE
    CONFIG_METADATA_BEGIN(MTK_FLASH_FIRING_POWER)
        CONFIG_ENTRY_VALUE(10, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//none FUTURE
    CONFIG_METADATA_BEGIN(MTK_FLASH_FIRING_TIME)
        CONFIG_ENTRY_VALUE(0, MINT64)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_FLASH_MODE)
        CONFIG_ENTRY_VALUE(MTK_FLASH_MODE_OFF, MUINT8)//off
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.jpeg
//------------------------------------------------------------------------------
    //==========================================================================//new none
    /*CONFIG_METADATA_BEGIN(MTK_JPEG_GPS_COORDINATES)//hidden
        CONFIG_ENTRY_VALUE( , MDOUBLE)
        CONFIG_ENTRY_VALUE( , MDOUBLE)
        CONFIG_ENTRY_VALUE( , MDOUBLE)
    CONFIG_METADATA_END()
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_JPEG_GPS_PROCESSING_METHOD)//hidden
        CONFIG_ENTRY_VALUE( , MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_JPEG_GPS_TIMESTAMP)//hidden
        CONFIG_ENTRY_VALUE( , MINT64)
    CONFIG_METADATA_END() */
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_JPEG_ORIENTATION)//public
        CONFIG_ENTRY_VALUE(0 , MINT32)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_JPEG_QUALITY)
        CONFIG_ENTRY_VALUE(100, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_JPEG_THUMBNAIL_QUALITY)
        CONFIG_ENTRY_VALUE(100, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_JPEG_THUMBNAIL_SIZE)
        CONFIG_ENTRY_VALUE(MSize(176, 128), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.lens
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_APERTURE)//not null
        CONFIG_ENTRY_VALUE(2.8f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_FILTER_DENSITY)//not null
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_FOCAL_LENGTH)//not null
        CONFIG_ENTRY_VALUE(5.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_LENS_FOCUS_DISTANCE)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_OPTICAL_STABILIZATION_MODE)//not null
        CONFIG_ENTRY_VALUE(MTK_LENS_OPTICAL_STABILIZATION_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.noiseReduction
//------------------------------------------------------------------------------
    //==========================================================================//new none FUTURE
    /*CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
        CONFIG_ENTRY_VALUE( , MUINT8)
    CONFIG_METADATA_END()*/
    //==========================================================================
//------------------------------------------------------------------------------
//  android.request
//------------------------------------------------------------------------------
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_FRAME_COUNT)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
#endif
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_REQUEST_ID)
        CONFIG_ENTRY_VALUE(0, MINT32)//hidden
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_REQUEST_METADATA_MODE)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_METADATA_MODE_FULL, MUINT8)//
    CONFIG_METADATA_END()
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_TYPE)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_TYPE_CAPTURE, MUINT8)
    CONFIG_METADATA_END()
#endif
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_MAX_NUM_OUTPUT_STREAMS)
        CONFIG_ENTRY_VALUE(3, MINT32)
    CONFIG_METADATA_END()
#endif
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_MAX_NUM_REPROCESS_STREAMS)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
#endif
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_INPUT_STREAMS)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
#endif
    //==========================================================================
#if 0
    CONFIG_METADATA_BEGIN(MTK_REQUEST_OUTPUT_STREAMS)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
#endif
    //==========================================================================
//------------------------------------------------------------------------------
//  android.scaler
//------------------------------------------------------------------------------
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_SCALER_CROP_REGION)//public
        CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MPoint(640, 480)) , MRect)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.sensor
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_EXPOSURE_TIME)
        CONFIG_ENTRY_VALUE(10 * 1000000LL, MINT64)//not null
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_FRAME_DURATION)
        CONFIG_ENTRY_VALUE(33333333LL, MINT64)//not null
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_SENSITIVITY)
        CONFIG_ENTRY_VALUE(100, MINT32)//not null
    CONFIG_METADATA_END()
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_SENSOR_TEST_PATTERN_DATA)
        CONFIG_ENTRY_VALUE(0 , MINT32)
        CONFIG_ENTRY_VALUE(0 , MINT32)
        CONFIG_ENTRY_VALUE(0 , MINT32)
        CONFIG_ENTRY_VALUE(0 , MINT32)
    CONFIG_METADATA_END()
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_SENSOR_TEST_PATTERN_MODE)
        CONFIG_ENTRY_VALUE( MTK_SENSOR_TEST_PATTERN_MODE_COLOR_BARS , MINT32)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  android.shading
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
        CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//new FUTURE
    CONFIG_METADATA_BEGIN(MTK_SHADING_STRENGTH)
        CONFIG_ENTRY_VALUE(10, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  android.statistics
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_FACE_DETECT_MODE)
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_FACE_DETECT_MODE_OFF, MUINT8)//off
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_HISTOGRAM_MODE)
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_HISTOGRAM_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_SHARPNESS_MAP_MODE)
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_SHARPNESS_MAP_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//new none
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_HOT_PIXEL_MAP_MODE)
        CONFIG_ENTRY_VALUE( MTK_STATISTICS_HOT_PIXEL_MAP_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================//new
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_LENS_SHADING_MAP_MODE)//default: STATISTICS_LENS_SHADING_MAP_MODE_OFF
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.tonemap
//------------------------------------------------------------------------------
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_CURVE_BLUE)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_CURVE_GREEN)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================//none
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_CURVE_RED)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
        CONFIG_ENTRY_VALUE(1.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.led
//------------------------------------------------------------------------------
    //==========================================================================//new
    /*CONFIG_METADATA_BEGIN(MTK_LED_TRANSMIT)
        CONFIG_ENTRY_VALUE( , MUINT8)
    CONFIG_METADATA_END()*/
    //==========================================================================
//------------------------------------------------------------------------------
//  android.blackLevel
//------------------------------------------------------------------------------
    //==========================================================================//new
    CONFIG_METADATA_BEGIN(MTK_BLACK_LEVEL_LOCK)
        CONFIG_ENTRY_VALUE(MTK_BLACK_LEVEL_LOCK_OFF, MUINT8)//false
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  request template types
//------------------------------------------------------------------------------
    //==========================================================================
    switch  (requestType)
    {
    //..........................................................................
    case CAMERA3_TEMPLATE_PREVIEW:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//dont care
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//!TONEMAP_MODE_CONTRAST_CURVE
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_ON
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//max>20 (available)
            CONFIG_ENTRY_VALUE(5, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//CONTROL_AF_MODE_AUTO (available)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPLATE_PREVIEW
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_PREVIEW, MUINT8)
        CONFIG_METADATA_END()
        break;
    //..........................................................................
    case CAMERA3_TEMPLATE_STILL_CAPTURE:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//!COLOR_CORRECTION_MODE_TRANSFORM_MATRIX
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//EDGE_MODE_HIGH_QUALITY
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//NOISE_REDUCTION_MODE_HIGH_QUALITY
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//NOISE_REDUCTION_MODE_HIGH_QUALITY
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_ON
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//no max request (available)
            CONFIG_ENTRY_VALUE(5, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//CONTROL_AF_MODE_AUTO (available)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPLATE_STILL_CAPTURE
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE, MUINT8)
        CONFIG_METADATA_END()
        break;
    //..........................................................................
    case CAMERA3_TEMPLATE_VIDEO_RECORD:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//dont care
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//!TONEMAP_MODE_CONTRAST_CURVE
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_ON
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//min=max
            CONFIG_ENTRY_VALUE(30, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//(available) (default: auto)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPLATE_RECORD
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD, MUINT8)
        CONFIG_METADATA_END()
        break;

    //..........................................................................
    case CAMERA3_TEMPLATE_VIDEO_SNAPSHOT:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//dont care
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//!TONEMAP_MODE_CONTRAST_CURVE
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_ON
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//min=max
            CONFIG_ENTRY_VALUE(30, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//(available) (default: auto)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPLATE_VIDEO_SNAPSHOT
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT, MUINT8)
        CONFIG_METADATA_END()
        break;
    //..........................................................................
    case CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//dont care
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//!TONEMAP_MODE_CONTRAST_CURVE
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_ON
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//max > 20
            CONFIG_ENTRY_VALUE(5, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//(available) (default: CONTROL_AF_MODE_AUTO)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPALTE_ZERO_SHUTTER_LAG
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG, MUINT8)
        CONFIG_METADATA_END()
        break;
    //..........................................................................
    default:
        CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_MODE)//dont care
            CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_DEMOSAIC_MODE)
            CONFIG_ENTRY_VALUE(MTK_DEMOSAIC_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_EDGE_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_MODE)
            CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_MODE)//not null
            CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_STRENGTH)
            CONFIG_ENTRY_VALUE(5, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_SHADING_MODE)
            CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_TONEMAP_MODE)//!TONEMAP_MODE_CONTRAST_CURVE
            CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_MODE)//CONTROL_MODE_AUTO
            CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_OFF, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_MODE)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_OFF, MUINT8)//auto, default: off
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_MODE)//CONTROL_AE_MODE_OFF
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_OFF, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_TARGET_FPS_RANGE)//no request for max
            CONFIG_ENTRY_VALUE(5, MINT32)
            CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_MODE)//(pre:auto)
            CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_OFF, MUINT8)
        CONFIG_METADATA_END()
        CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_INTENT)//TEMPLATE_MANUAL
            CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_INTENT_MANUAL, MUINT8)
        CONFIG_METADATA_END()
        break;
    }
//------------------------------------------------------------------------------
REQUEST_METADATA_END()

