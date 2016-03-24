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


#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_H_

#include <hardware/hardware.h>

typedef enum mtk_camera_metadata_section {
    MTK_COLOR_CORRECTION,
    MTK_CONTROL,
    MTK_DEMOSAIC,
    MTK_EDGE,
    MTK_FLASH,
    MTK_FLASH_INFO,
    MTK_HOT_PIXEL,
    MTK_JPEG,
    MTK_LENS,
    MTK_LENS_INFO,
    MTK_NOISE_REDUCTION,
    MTK_QUIRKS,
    MTK_REQUEST,
    MTK_SCALER,
    MTK_SENSOR,
    MTK_SENSOR_INFO,
    MTK_SHADING,
    MTK_STATISTICS,
    MTK_STATISTICS_INFO,
    MTK_TONEMAP,
    MTK_LED,
    MTK_INFO,
    MTK_BLACK_LEVEL,
    MTK_SYNC,
    MTK_REPROCESS,
    MTK_DEPTH,
    MTK_HAL_INFO,
    MTK_IOPIPE_INFO,
    MTK_SECTION_COUNT,

    //vendor tag
    MTK_VENDOR_TAG_SECTION          = 0x8000,
    MTK_FACE_FEATURE                = 0,
    MTK_NR_FEATURE                  = 1,
    MTK_VENDOR_SECTION_COUNT,
} mtk_camera_metadata_section_t;

/**
 * Hierarchy positions in enum space. All vendor extension tags must be
 * defined with tag >= VENDOR_SECTION_START
 */

typedef enum mtk_camera_metadata_section_start {
    MTK_COLOR_CORRECTION_START  = MTK_COLOR_CORRECTION  << 16,
    MTK_CONTROL_START           = MTK_CONTROL           << 16,
    MTK_DEMOSAIC_START          = MTK_DEMOSAIC          << 16,
    MTK_EDGE_START              = MTK_EDGE              << 16,
    MTK_FLASH_START             = MTK_FLASH             << 16,
    MTK_FLASH_INFO_START        = MTK_FLASH_INFO        << 16,
    MTK_HOT_PIXEL_START         = MTK_HOT_PIXEL         << 16,
    MTK_JPEG_START              = MTK_JPEG              << 16,
    MTK_LENS_START              = MTK_LENS              << 16,
    MTK_LENS_INFO_START         = MTK_LENS_INFO         << 16,
    MTK_NOISE_REDUCTION_START   = MTK_NOISE_REDUCTION   << 16,
    MTK_QUIRKS_START            = MTK_QUIRKS            << 16,
    MTK_REQUEST_START           = MTK_REQUEST           << 16,
    MTK_SCALER_START            = MTK_SCALER            << 16,
    MTK_SENSOR_START            = MTK_SENSOR            << 16,
    MTK_SENSOR_INFO_START       = MTK_SENSOR_INFO       << 16,
    MTK_SHADING_START           = MTK_SHADING           << 16,
    MTK_STATISTICS_START        = MTK_STATISTICS        << 16,
    MTK_STATISTICS_INFO_START   = MTK_STATISTICS_INFO   << 16,
    MTK_TONEMAP_START           = MTK_TONEMAP           << 16,
    MTK_LED_START               = MTK_LED               << 16,
    MTK_INFO_START              = MTK_INFO              << 16,
    MTK_BLACK_LEVEL_START       = MTK_BLACK_LEVEL       << 16,
    MTK_SYNC_START              = MTK_SYNC              << 16,
    MTK_REPROCESS_START         = MTK_REPROCESS         << 16,
    MTK_DEPTH_START             = MTK_DEPTH             << 16,
    MTK_HAL_INFO_START          = MTK_HAL_INFO          << 16,
    MTK_IOPIPE_INFO_START       = MTK_IOPIPE_INFO       << 16,

    MTK_VENDOR_TAG_SECTION_START                = MTK_VENDOR_TAG_SECTION     << 16,
    MTK_FACE_FEATURE_START                      = (MTK_FACE_FEATURE + MTK_VENDOR_TAG_SECTION)    << 16,
    MTK_NR_FEATURE_START                        = (MTK_NR_FEATURE + MTK_VENDOR_TAG_SECTION)      << 16,
} mtk_camera_metadata_section_start_t;



/**
 * Main enum for defining camera metadata tags.  New entries must always go
 * before the section _END tag to preserve existing enumeration values.  In
 * addition, the name and type of the tag needs to be added to
 * ""
 */
typedef enum mtk_camera_metadata_tag {
    MTK_COLOR_CORRECTION_MODE           = MTK_COLOR_CORRECTION_START,
    MTK_COLOR_CORRECTION_TRANSFORM,
    MTK_COLOR_CORRECTION_GAINS,
    MTK_COLOR_CORRECTION_ABERRATION_MODE,
    MTK_COLOR_CORRECTION_AVAILABLE_ABERRATION_MODES,
    MTK_COLOR_CORRECTION_END,

    MTK_CONTROL_AE_ANTIBANDING_MODE     = MTK_CONTROL_START,
    MTK_CONTROL_AE_EXPOSURE_COMPENSATION,
    MTK_CONTROL_AE_LOCK,
    MTK_CONTROL_AE_MODE,
    MTK_CONTROL_AE_REGIONS,
    MTK_CONTROL_AE_TARGET_FPS_RANGE,
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER,
    MTK_CONTROL_AF_MODE,
    MTK_CONTROL_AF_REGIONS,
    MTK_CONTROL_AF_TRIGGER,
    MTK_CONTROL_AWB_LOCK,
    MTK_CONTROL_AWB_MODE,
    MTK_CONTROL_AWB_REGIONS,
    MTK_CONTROL_CAPTURE_INTENT,
    MTK_CONTROL_EFFECT_MODE,
    MTK_CONTROL_MODE,
    MTK_CONTROL_SCENE_MODE,
    MTK_CONTROL_VIDEO_STABILIZATION_MODE,
    MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES,
    MTK_CONTROL_AE_AVAILABLE_MODES,
    MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES,
    MTK_CONTROL_AE_COMPENSATION_RANGE,
    MTK_CONTROL_AE_COMPENSATION_STEP,
    MTK_CONTROL_AF_AVAILABLE_MODES,
    MTK_CONTROL_AVAILABLE_EFFECTS,
    MTK_CONTROL_AVAILABLE_SCENE_MODES,
    MTK_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES,
    MTK_CONTROL_AWB_AVAILABLE_MODES,
    MTK_CONTROL_MAX_REGIONS,
    MTK_CONTROL_SCENE_MODE_OVERRIDES,
    MTK_CONTROL_AE_PRECAPTURE_ID,
    MTK_CONTROL_AE_STATE,
    MTK_CONTROL_AF_STATE,
    MTK_CONTROL_AF_TRIGGER_ID,
    MTK_CONTROL_AWB_STATE,
    MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS,
    MTK_CONTROL_AE_LOCK_AVAILABLE,
    MTK_CONTROL_AWB_LOCK_AVAILABLE,
    MTK_CONTROL_AVAILABLE_MODES,
    // Camera1
    MTK_CONTROL_ISP_EDGE,
    MTK_CONTROL_ISP_HUE,
    MTK_CONTROL_ISP_SATURATION,
    MTK_CONTROL_ISP_BRIGHTNESS,
    MTK_CONTROL_ISP_CONTRAST,
    MTK_CONTROL_END,

    MTK_DEMOSAIC_MODE                   = MTK_DEMOSAIC_START,
    MTK_DEMOSAIC_END,

    MTK_EDGE_MODE                       = MTK_EDGE_START,
    MTK_EDGE_STRENGTH,
    MTK_EDGE_AVAILABLE_EDGE_MODES,
    MTK_EDGE_END,

    MTK_FLASH_FIRING_POWER              = MTK_FLASH_START,
    MTK_FLASH_FIRING_TIME,
    MTK_FLASH_MODE,
    MTK_FLASH_COLOR_TEMPERATURE,
    MTK_FLASH_MAX_ENERGY,
    MTK_FLASH_STATE,
    MTK_FLASH_END,

    MTK_FLASH_INFO_AVAILABLE            = MTK_FLASH_INFO_START,
    MTK_FLASH_INFO_CHARGE_DURATION,
    MTK_FLASH_INFO_END,

    MTK_HOT_PIXEL_MODE                  = MTK_HOT_PIXEL_START,
    MTK_HOT_PIXEL_AVAILABLE_HOT_PIXEL_MODES,
    MTK_HOT_PIXEL_END,

    MTK_JPEG_GPS_COORDINATES            = MTK_JPEG_START,
    MTK_JPEG_GPS_PROCESSING_METHOD,
    MTK_JPEG_GPS_TIMESTAMP,
    MTK_JPEG_ORIENTATION,
    MTK_JPEG_QUALITY,
    MTK_JPEG_THUMBNAIL_QUALITY,
    MTK_JPEG_THUMBNAIL_SIZE,
    MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES,
    MTK_JPEG_MAX_SIZE,
    MTK_JPEG_SIZE,
    MTK_JPEG_END,

    MTK_LENS_APERTURE                   = MTK_LENS_START,
    MTK_LENS_FILTER_DENSITY,
    MTK_LENS_FOCAL_LENGTH,
    MTK_LENS_FOCUS_DISTANCE,
    MTK_LENS_OPTICAL_STABILIZATION_MODE,
    MTK_SENSOR_INFO_FACING,
    MTK_LENS_POSE_ROTATION,
    MTK_LENS_POSE_TRANSLATION,
    MTK_LENS_FOCUS_RANGE,
    MTK_LENS_STATE,
    MTK_LENS_INTRINSIC_CALIBRATION,
    MTK_LENS_RADIAL_DISTORTION,
    MTK_LENS_END,

    MTK_LENS_INFO_AVAILABLE_APERTURES   = MTK_LENS_INFO_START,
    MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES,
    MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS,
    MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION,
    MTK_LENS_INFO_HYPERFOCAL_DISTANCE,
    MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE,
    MTK_LENS_INFO_SHADING_MAP_SIZE,
    MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION,
    MTK_LENS_INFO_END,

    MTK_NOISE_REDUCTION_MODE            = MTK_NOISE_REDUCTION_START,
    MTK_NOISE_REDUCTION_STRENGTH,
    MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES,
    MTK_NOISE_REDUCTION_END,

    MTK_QUIRKS_METERING_CROP_REGION     = MTK_QUIRKS_START,
    MTK_QUIRKS_TRIGGER_AF_WITH_AUTO,
    MTK_QUIRKS_USE_ZSL_FORMAT,
    MTK_QUIRKS_USE_PARTIAL_RESULT,
    MTK_QUIRKS_PARTIAL_RESULT,
    MTK_QUIRKS_END,

    MTK_REQUEST_FRAME_COUNT             = MTK_REQUEST_START,
    MTK_REQUEST_ID,
    MTK_REQUEST_INPUT_STREAMS,
    MTK_REQUEST_METADATA_MODE,
    MTK_REQUEST_OUTPUT_STREAMS,
    MTK_REQUEST_TYPE,
    MTK_REQUEST_MAX_NUM_OUTPUT_STREAMS,
    MTK_REQUEST_MAX_NUM_REPROCESS_STREAMS,
    MTK_REQUEST_MAX_NUM_INPUT_STREAMS,
    MTK_REQUEST_PIPELINE_DEPTH,
    MTK_REQUEST_PIPELINE_MAX_DEPTH,
    MTK_REQUEST_PARTIAL_RESULT_COUNT,
    MTK_REQUEST_AVAILABLE_CAPABILITIES,
    MTK_REQUEST_AVAILABLE_REQUEST_KEYS,
    MTK_REQUEST_AVAILABLE_RESULT_KEYS,
    MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS,
    MTK_REQUEST_END,

    MTK_SCALER_CROP_REGION              = MTK_SCALER_START,
    MTK_SCALER_AVAILABLE_FORMATS,
    MTK_SCALER_AVAILABLE_JPEG_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_JPEG_SIZES,
    MTK_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM,
    MTK_SCALER_AVAILABLE_PROCESSED_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_PROCESSED_SIZES,
    MTK_SCALER_AVAILABLE_RAW_MIN_DURATIONS,
    MTK_SCALER_AVAILABLE_RAW_SIZES,
    MTK_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP,
    MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
    MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
    MTK_SCALER_AVAILABLE_STALL_DURATIONS,
    MTK_SCALER_CROPPING_TYPE,
    MTK_SCALER_END,

    MTK_IOPIPE_INFO_AVAILABLE_IN_PORT_INFO
                                        = MTK_IOPIPE_INFO_START,
    MTK_IOPIPE_INFO_AVAILABLE_OUT_PORT_INFO,
    MTK_IOPIPE_INFO_PORT_ID,
    MTK_IOPIPE_INFO_TRANSFORM,
    MTK_IOPIPE_INFO_CROP,
    MTK_IOPIPE_INFO_SCALE_DOWN_RATIO,
    MTK_IOPIPE_INFO_SCALE_UP_RATIO,
    MTK_IOPIPE_INFO_LINEBUFFER,
    MTK_IOPIPE_INFO_AVAILABLE_FORMATS,
    MTK_IOPIPE_INFO_END,

    MTK_SENSOR_EXPOSURE_TIME            = MTK_SENSOR_START,
    MTK_SENSOR_FRAME_DURATION,
    MTK_SENSOR_SENSITIVITY,
    MTK_SENSOR_REFERENCE_ILLUMINANT1,
    MTK_SENSOR_REFERENCE_ILLUMINANT2,
    MTK_SENSOR_CALIBRATION_TRANSFORM1,
    MTK_SENSOR_CALIBRATION_TRANSFORM2,
    MTK_SENSOR_COLOR_TRANSFORM1,
    MTK_SENSOR_COLOR_TRANSFORM2,
    MTK_SENSOR_FORWARD_MATRIX1,
    MTK_SENSOR_FORWARD_MATRIX2,
    MTK_SENSOR_BASE_GAIN_FACTOR,
    MTK_SENSOR_BLACK_LEVEL_PATTERN,
    MTK_SENSOR_MAX_ANALOG_SENSITIVITY,
    MTK_SENSOR_ORIENTATION,
    MTK_SENSOR_PROFILE_HUE_SAT_MAP_DIMENSIONS,
    MTK_SENSOR_TIMESTAMP,
    MTK_SENSOR_TEMPERATURE,
    MTK_SENSOR_NEUTRAL_COLOR_POINT,
    MTK_SENSOR_NOISE_PROFILE,
    MTK_SENSOR_PROFILE_HUE_SAT_MAP,
    MTK_SENSOR_PROFILE_TONE_CURVE,
    MTK_SENSOR_GREEN_SPLIT,
    MTK_SENSOR_TEST_PATTERN_DATA,
    MTK_SENSOR_TEST_PATTERN_MODE,
    MTK_SENSOR_AVAILABLE_TEST_PATTERN_MODES,
    MTK_SENSOR_ROLLING_SHUTTER_SKEW,
    MTK_SENSOR_END,

    MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION = MTK_SENSOR_INFO_START,
    MTK_SENSOR_INFO_SENSITIVITY_RANGE,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT,
    MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE,
    MTK_SENSOR_INFO_MAX_FRAME_DURATION,
    MTK_SENSOR_INFO_PHYSICAL_SIZE,
    MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE,
    MTK_SENSOR_INFO_WHITE_LEVEL,
    MTK_SENSOR_INFO_TIMESTAMP_SOURCE,
    MTK_SENSOR_INFO_LENS_SHADING_APPLIED,
    MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE,
    MTK_SENSOR_INFO_ORIENTATION,
    MTK_SENSOR_INFO_PACKAGE,
    MTK_SENSOR_INFO_DEV,
    MTK_SENSOR_INFO_SCENARIO_ID,
    MTK_SENSOR_INFO_FRAME_RATE,
    MTK_SENSOR_INFO_REAL_OUTPUT_SIZE,
    MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY,
    MTK_SENSOR_INFO_WANTED_ORIENTATION,
    MTK_SENSOR_INFO_END,

    MTK_SHADING_MODE                    = MTK_SHADING_START,
    MTK_SHADING_STRENGTH,
    MTK_SHADING_AVAILABLE_MODES,
    MTK_SHADING_END,

    MTK_STATISTICS_FACE_DETECT_MODE     = MTK_STATISTICS_START,
    MTK_STATISTICS_HISTOGRAM_MODE,
    MTK_STATISTICS_SHARPNESS_MAP_MODE,
    MTK_STATISTICS_HOT_PIXEL_MAP_MODE,
    MTK_STATISTICS_FACE_IDS,
    MTK_STATISTICS_FACE_LANDMARKS,
    MTK_STATISTICS_FACE_RECTANGLES,
    MTK_STATISTICS_FACE_SCORES,
    MTK_STATISTICS_HISTOGRAM,
    MTK_STATISTICS_SHARPNESS_MAP,
    MTK_STATISTICS_LENS_SHADING_CORRECTION_MAP,
    MTK_STATISTICS_LENS_SHADING_MAP,
    MTK_STATISTICS_PREDICTED_COLOR_GAINS,
    MTK_STATISTICS_PREDICTED_COLOR_TRANSFORM,
    MTK_STATISTICS_SCENE_FLICKER,
    MTK_STATISTICS_HOT_PIXEL_MAP,
    MTK_STATISTICS_LENS_SHADING_MAP_MODE,
    MTK_STATISTICS_OBJECT_TRACKING_ENABLE,
    MTK_STATISTICS_OBJECT_TRACKING_INIT,
    MTK_STATISTICS_OBJECT_TRACKING_SCORE,
    MTK_STATISTICS_OBJECT_TRACKING_RECT,
    MTK_STATISTICS_END,

    MTK_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES
                                        = MTK_STATISTICS_INFO_START,
    MTK_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT,
    MTK_STATISTICS_INFO_MAX_FACE_COUNT,
    MTK_STATISTICS_INFO_MAX_HISTOGRAM_COUNT,
    MTK_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE,
    MTK_STATISTICS_INFO_SHARPNESS_MAP_SIZE,
    MTK_STATISTICS_INFO_AVAILABLE_HOT_PIXEL_MAP_MODES,
    MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES,
    MTK_STATISTICS_INFO_END,

    MTK_TONEMAP_CURVE_BLUE              = MTK_TONEMAP_START,
    MTK_TONEMAP_CURVE_GREEN,
    MTK_TONEMAP_CURVE_RED,
    MTK_TONEMAP_MODE,
    MTK_TONEMAP_MAX_CURVE_POINTS,
    MTK_TONEMAP_AVAILABLE_TONE_MAP_MODES,
    MTK_TONEMAP_GAMMA,
    MTK_TONEMAP_PRESET_CURVE,
    MTK_TONEMAP_END,

    MTK_LED_TRANSMIT                    = MTK_LED_START,
    MTK_LED_AVAILABLE_LEDS,
    MTK_LED_END,

    MTK_INFO_SUPPORTED_HARDWARE_LEVEL   = MTK_INFO_START,
    MTK_INFO_END,

    MTK_BLACK_LEVEL_LOCK                = MTK_BLACK_LEVEL_START,
    MTK_BLACK_LEVEL_END,

    MTK_SYNC_FRAME_NUMBER               = MTK_SYNC_START,
    MTK_SYNC_MAX_LATENCY,
    MTK_SYNC_END,

    MTK_REPROCESS_EFFECTIVE_EXPOSURE_FACTOR
                                        = MTK_REPROCESS_START,
    MTK_REPROCESS_MAX_CAPTURE_STALL,
    MTK_REPROCESS_END,

    MTK_DEPTH_MAX_DEPTH_SAMPLES         = MTK_DEPTH_START,
    MTK_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS,
    MTK_DEPTH_AVAILABLE_DEPTH_MIN_FRAME_DURATIONS,
    MTK_DEPTH_AVAILABLE_DEPTH_STALL_DURATIONS,
    MTK_DEPTH_DEPTH_IS_EXCLUSIVE,
    MTK_DEPTH_END,

    MTK_FACE_FEATURE_GESTURE_MODE       = MTK_FACE_FEATURE_START,
    MTK_FACE_FEATURE_GESTURE_RESULT,
    MTK_FACE_FEATURE_SMILE_DETECT_MODE,
    MTK_FACE_FEATURE_SMILE_DETECT_RESULT,
    MTK_FACE_FEATURE_ASD_MODE,
    MTK_FACE_FEATURE_ASD_RESULT,
    MTK_FACE_FEATURE_AVAILABLE_GESTURE_MODES,
    MTK_FACE_FEATURE_AVAILABLE_SMILE_DETECT_MODES,
    MTK_FACE_FEATURE_AVAILABLE_ASD_MODES,
    MTK_FACE_FEATURE_END,

    MTK_NR_FEATURE_3DNR_MODE            = MTK_NR_FEATURE_START,
    MTK_NR_FEATURE_AVAILABLE_3DNR_MODES,
    MTK_NR_FEATURE_END,

    MTK_HAL_VERSION                     = MTK_HAL_INFO_START,

} mtk_camera_metadata_tag_t;

/**
 * Enumeration definitions for the various entries that need them
 */

// MTK_COLOR_CORRECTION_MODE
typedef enum mtk_camera_metadata_enum_android_color_correction_mode {
    MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX,
    MTK_COLOR_CORRECTION_MODE_FAST,
    MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_color_correction_mode_t;

// MTK_COLOR_CORRECTION_ABERRATION_MODE
typedef enum mtk_camera_metadata_enum_android_color_correction_aberration_mode {
    MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF,
    MTK_COLOR_CORRECTION_ABERRATION_MODE_FAST,
    MTK_COLOR_CORRECTION_ABERRATION_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_color_correction_aberration_mode_t;

// MTK_CONTROL_AE_ANTIBANDING_MODE
typedef enum mtk_camera_metadata_enum_android_control_ae_antibanding_mode {
    MTK_CONTROL_AE_ANTIBANDING_MODE_OFF,
    MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ,
    MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ,
    MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO,
} mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t;

// MTK_CONTROL_AE_LOCK
typedef enum mtk_camera_metadata_enum_android_control_ae_lock {
    MTK_CONTROL_AE_LOCK_OFF,
    MTK_CONTROL_AE_LOCK_ON,
} mtk_camera_metadata_enum_android_control_ae_lock_t;

// MTK_CONTROL_AE_MODE
typedef enum mtk_camera_metadata_enum_android_control_ae_mode {
    MTK_CONTROL_AE_MODE_OFF,
    MTK_CONTROL_AE_MODE_ON,
    MTK_CONTROL_AE_MODE_ON_AUTO_FLASH,
    MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH,
    MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE,
} mtk_camera_metadata_enum_android_control_ae_mode_t;

// MTK_CONTROL_AE_PRECAPTURE_TRIGGER
typedef enum mtk_camera_metadata_enum_android_control_ae_precapture_trigger {
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE,
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START,
    MTK_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL,
} mtk_camera_metadata_enum_android_control_ae_precapture_trigger_t;

// MTK_CONTROL_AF_MODE
typedef enum mtk_camera_metadata_enum_android_control_af_mode {
    MTK_CONTROL_AF_MODE_OFF,
    MTK_CONTROL_AF_MODE_AUTO,
    MTK_CONTROL_AF_MODE_MACRO,
    MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO,
    MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE,
    MTK_CONTROL_AF_MODE_EDOF,
} mtk_camera_metadata_enum_android_control_af_mode_t;

// MTK_CONTROL_AF_TRIGGER
typedef enum mtk_camera_metadata_enum_android_control_af_trigger {
    MTK_CONTROL_AF_TRIGGER_IDLE,
    MTK_CONTROL_AF_TRIGGER_START,
    MTK_CONTROL_AF_TRIGGER_CANCEL,
} mtk_camera_metadata_enum_android_control_af_trigger_t;

// MTK_CONTROL_AWB_LOCK
typedef enum mtk_camera_metadata_enum_android_control_awb_lock {
    MTK_CONTROL_AWB_LOCK_OFF,
    MTK_CONTROL_AWB_LOCK_ON,
} mtk_camera_metadata_enum_android_control_awb_lock_t;

// MTK_CONTROL_AWB_MODE
typedef enum mtk_camera_metadata_enum_android_control_awb_mode {
    MTK_CONTROL_AWB_MODE_OFF,
    MTK_CONTROL_AWB_MODE_AUTO,
    MTK_CONTROL_AWB_MODE_INCANDESCENT,
    MTK_CONTROL_AWB_MODE_FLUORESCENT,
    MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT,
    MTK_CONTROL_AWB_MODE_DAYLIGHT,
    MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT,
    MTK_CONTROL_AWB_MODE_TWILIGHT,
    MTK_CONTROL_AWB_MODE_SHADE,
    MTK_CONTROL_AWB_MODE_GRAYWORLD, //deprecated
} mtk_camera_metadata_enum_android_control_awb_mode_t;

// MTK_CONTROL_CAPTURE_INTENT
typedef enum mtk_camera_metadata_enum_android_control_capture_intent {
    MTK_CONTROL_CAPTURE_INTENT_CUSTOM,
    MTK_CONTROL_CAPTURE_INTENT_PREVIEW,
    MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE,
    MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD,
    MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT,
    MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG,
    MTK_CONTROL_CAPTURE_INTENT_MANUAL,
} mtk_camera_metadata_enum_android_control_capture_intent_t;

// MTK_CONTROL_EFFECT_MODE
typedef enum mtk_camera_metadata_enum_android_control_effect_mode {
    MTK_CONTROL_EFFECT_MODE_OFF,
    MTK_CONTROL_EFFECT_MODE_MONO,
    MTK_CONTROL_EFFECT_MODE_NEGATIVE,
    MTK_CONTROL_EFFECT_MODE_SOLARIZE,
    MTK_CONTROL_EFFECT_MODE_SEPIA,
    MTK_CONTROL_EFFECT_MODE_POSTERIZE,
    MTK_CONTROL_EFFECT_MODE_WHITEBOARD,
    MTK_CONTROL_EFFECT_MODE_BLACKBOARD,
    MTK_CONTROL_EFFECT_MODE_AQUA,  //,
    MTK_CONTROL_EFFECT_MODE_SEPIAGREEN,
    MTK_CONTROL_EFFECT_MODE_SEPIABLUE,
    MTK_CONTROL_EFFECT_MODE_NASHVILLE ,        //LOMO
    MTK_CONTROL_EFFECT_MODE_HEFE ,
    MTK_CONTROL_EFFECT_MODE_VALENCIA ,
    MTK_CONTROL_EFFECT_MODE_XPROII ,
    MTK_CONTROL_EFFECT_MODE_LOFI ,
    MTK_CONTROL_EFFECT_MODE_SIERRA ,
    MTK_CONTROL_EFFECT_MODE_KELVIN ,
    MTK_CONTROL_EFFECT_MODE_WALDEN,
    MTK_CONTROL_EFFECT_MODE_F1977 ,        //LOMO
    MTK_CONTROL_EFFECT_MODE_NUM
} mtk_camera_metadata_enum_android_control_effect_mode_t;

// MTK_CONTROL_MODE
typedef enum mtk_camera_metadata_enum_android_control_mode {
    MTK_CONTROL_MODE_OFF,
    MTK_CONTROL_MODE_AUTO,
    MTK_CONTROL_MODE_USE_SCENE_MODE,
    MTK_CONTROL_MODE_OFF_KEEP_STATE,
} mtk_camera_metadata_enum_android_control_mode_t;

// MTK_CONTROL_SCENE_MODE
typedef enum mtk_camera_metadata_enum_android_control_scene_mode {
    MTK_CONTROL_SCENE_MODE_DISABLED                       = 0,
    MTK_CONTROL_SCENE_MODE_UNSUPPORTED                    = MTK_CONTROL_SCENE_MODE_DISABLED,
    MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
    MTK_CONTROL_SCENE_MODE_ACTION,
    MTK_CONTROL_SCENE_MODE_PORTRAIT,
    MTK_CONTROL_SCENE_MODE_LANDSCAPE,
    MTK_CONTROL_SCENE_MODE_NIGHT,
    MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
    MTK_CONTROL_SCENE_MODE_THEATRE,
    MTK_CONTROL_SCENE_MODE_BEACH,
    MTK_CONTROL_SCENE_MODE_SNOW,
    MTK_CONTROL_SCENE_MODE_SUNSET,
    MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
    MTK_CONTROL_SCENE_MODE_FIREWORKS,
    MTK_CONTROL_SCENE_MODE_SPORTS,
    MTK_CONTROL_SCENE_MODE_PARTY,
    MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
    MTK_CONTROL_SCENE_MODE_BARCODE,
    MTK_CONTROL_SCENE_MODE_HIGH_SPEED_VIDEO,
    MTK_CONTROL_SCENE_MODE_HDR,
    MTK_CONTROL_SCENE_MODE_FACE_PRIORITY_LOW_LIGHT,
    // Camera1
    MTK_CONTROL_SCENE_MODE_NORMAL,
    MTK_CONTROL_SCENE_MODE_NUM
} mtk_camera_metadata_enum_android_control_scene_mode_t;

// MTK_CONTROL_VIDEO_STABILIZATION_MODE
typedef enum mtk_camera_metadata_enum_android_control_video_stabilization_mode {
    MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF,
    MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON,
} mtk_camera_metadata_enum_android_control_video_stabilization_mode_t;

// MTK_CONTROL_AE_STATE
typedef enum mtk_camera_metadata_enum_android_control_ae_state {
    MTK_CONTROL_AE_STATE_INACTIVE,
    MTK_CONTROL_AE_STATE_SEARCHING,
    MTK_CONTROL_AE_STATE_CONVERGED,
    MTK_CONTROL_AE_STATE_LOCKED,
    MTK_CONTROL_AE_STATE_FLASH_REQUIRED,
    MTK_CONTROL_AE_STATE_PRECAPTURE,
} mtk_camera_metadata_enum_android_control_ae_state_t;

// MTK_CONTROL_AF_STATE
typedef enum mtk_camera_metadata_enum_android_control_af_state {
    MTK_CONTROL_AF_STATE_INACTIVE,
    MTK_CONTROL_AF_STATE_PASSIVE_SCAN,
    MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED,
    MTK_CONTROL_AF_STATE_ACTIVE_SCAN,
    MTK_CONTROL_AF_STATE_FOCUSED_LOCKED,
    MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED,
    MTK_CONTROL_AF_STATE_PASSIVE_UNFOCUSED,
} mtk_camera_metadata_enum_android_control_af_state_t;

// MTK_CONTROL_AWB_STATE
typedef enum mtk_camera_metadata_enum_android_control_awb_state {
    MTK_CONTROL_AWB_STATE_INACTIVE,
    MTK_CONTROL_AWB_STATE_SEARCHING,
    MTK_CONTROL_AWB_STATE_CONVERGED,
    MTK_CONTROL_AWB_STATE_LOCKED,
} mtk_camera_metadata_enum_android_control_awb_state_t;

// MTK_CONTROL_AE_LOCK_AVAILABLE
typedef enum mtk_camera_metadata_enum_android_control_ae_lock_available {
    MTK_CONTROL_AE_LOCK_AVAILABLE_FALSE,
    MTK_CONTROL_AE_LOCK_AVAILABLE_TRUE,
} mtk_camera_metadata_enum_android_control_ae_lock_available_t;

// MTK_CONTROL_AWB_LOCK_AVAILABLE
typedef enum mtk_camera_metadata_enum_android_control_awb_lock_available {
    MTK_CONTROL_AWB_LOCK_AVAILABLE_FALSE,
    MTK_CONTROL_AWB_LOCK_AVAILABLE_TRUE,
} mtk_camera_metadata_enum_android_control_awb_lock_available_t;


// MTK_CONTROL_ISP_EDGE
typedef enum mtk_camera_metadata_enum_android_control_isp_edge {
    MTK_CONTROL_ISP_EDGE_LOW,
    MTK_CONTROL_ISP_EDGE_MIDDLE,
    MTK_CONTROL_ISP_EDGE_HIGH
} mtk_camera_metadata_enum_android_control_isp_edge_t;

// MTK_CONTROL_ISP_HUE
typedef enum mtk_camera_metadata_enum_android_control_isp_hue {
    MTK_CONTROL_ISP_HUE_LOW,
    MTK_CONTROL_ISP_HUE_MIDDLE,
    MTK_CONTROL_ISP_HUE_HIGH
} mtk_camera_metadata_enum_android_control_isp_hue_t;

// MTK_CONTROL_ISP_SATURATION
typedef enum mtk_camera_metadata_enum_android_control_isp_saturation {
    MTK_CONTROL_ISP_SATURATION_LOW,
    MTK_CONTROL_ISP_SATURATION_MIDDLE,
    MTK_CONTROL_ISP_SATURATION_HIGH
} mtk_camera_metadata_enum_android_control_isp_saturation_t;

// MTK_CONTROL_ISP_BRIGHTNESS
typedef enum mtk_camera_metadata_enum_android_control_isp_brightness {
    MTK_CONTROL_ISP_BRIGHTNESS_LOW,
    MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE,
    MTK_CONTROL_ISP_BRIGHTNESS_HIGH
} mtk_camera_metadata_enum_android_control_isp_brightness_t;

// MTK_CONTROL_ISP_CONTRAST
typedef enum mtk_camera_metadata_enum_android_control_isp_contrast {
    MTK_CONTROL_ISP_CONTRAST_LOW,
    MTK_CONTROL_ISP_CONTRAST_MIDDLE,
    MTK_CONTROL_ISP_CONTRAST_HIGH
} mtk_camera_metadata_enum_android_control_isp_contrast_t;

// MTK_DEMOSAIC_MODE
typedef enum mtk_camera_metadata_enum_android_demosaic_mode {
    MTK_DEMOSAIC_MODE_FAST,
    MTK_DEMOSAIC_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_demosaic_mode_t;

// MTK_EDGE_MODE
typedef enum mtk_camera_metadata_enum_android_edge_mode {
    MTK_EDGE_MODE_OFF,
    MTK_EDGE_MODE_FAST,
    MTK_EDGE_MODE_HIGH_QUALITY,
    MTK_EDGE_MODE_ZERO_SHUTTER_LAG,
} mtk_camera_metadata_enum_android_edge_mode_t;

// MTK_FLASH_MODE
typedef enum mtk_camera_metadata_enum_android_flash_mode {
    MTK_FLASH_MODE_OFF,
    MTK_FLASH_MODE_SINGLE,
    MTK_FLASH_MODE_TORCH,
} mtk_camera_metadata_enum_android_flash_mode_t;

// MTK_FLASH_STATE
typedef enum mtk_camera_metadata_enum_android_flash_state {
    MTK_FLASH_STATE_UNAVAILABLE,
    MTK_FLASH_STATE_CHARGING,
    MTK_FLASH_STATE_READY,
    MTK_FLASH_STATE_FIRED,
    MTK_FLASH_STATE_PARTIAL,
} mtk_camera_metadata_enum_android_flash_state_t;

// MTK_FLASH_INFO_AVAILABLE
typedef enum mtk_camera_metadata_enum_android_flash_info_available {
    MTK_FLASH_INFO_AVAILABLE_FALSE,
    MTK_FLASH_INFO_AVAILABLE_TRUE,
} mtk_camera_metadata_enum_android_flash_info_available_t;

// MTK_HOT_PIXEL_MODE
typedef enum mtk_camera_metadata_enum_android_hot_pixel_mode {
    MTK_HOT_PIXEL_MODE_OFF,
    MTK_HOT_PIXEL_MODE_FAST,
    MTK_HOT_PIXEL_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_hot_pixel_mode_t;

// MTK_LENS_OPTICAL_STABILIZATION_MODE
typedef enum mtk_camera_metadata_enum_android_lens_optical_stabilization_mode {
    MTK_LENS_OPTICAL_STABILIZATION_MODE_OFF,
    MTK_LENS_OPTICAL_STABILIZATION_MODE_ON,
} mtk_camera_metadata_enum_android_lens_optical_stabilization_mode_t;

// MTK_LENS_FACING
typedef enum mtk_camera_metadata_enum_android_lens_facing {
    MTK_LENS_FACING_FRONT,
    MTK_LENS_FACING_BACK,
    MTK_LENS_FACING_EXTERNAL,
} mtk_camera_metadata_enum_android_lens_facing_t;

// MTK_LENS_STATE
typedef enum mtk_camera_metadata_enum_android_lens_state {
    MTK_LENS_STATE_STATIONARY,
} mtk_camera_metadata_enum_android_lens_state_t;

// MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION
typedef enum mtk_camera_metadata_enum_android_lens_info_focus_distance_calibration {
    MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_UNCALIBRATED,
    MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_APPROXIMATE,
    MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_CALIBRATED,
} mtk_camera_metadata_enum_android_lens_info_focus_distance_calibration_t;

// MTK_NOISE_REDUCTION_MODE
typedef enum mtk_camera_metadata_enum_android_noise_reduction_mode {
    MTK_NOISE_REDUCTION_MODE_OFF,
    MTK_NOISE_REDUCTION_MODE_FAST,
    MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY,
    MTK_NOISE_REDUCTION_MODE_MINIMAL,
    MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG,
} mtk_camera_metadata_enum_android_noise_reduction_mode_t;

// MTK_QUIRKS_PARTIAL_RESULT
typedef enum mtk_metadata_enum_android_quirks_partial_result {
    MTK_QUIRKS_PARTIAL_RESULT_FINAL,
    MTK_QUIRKS_PARTIAL_RESULT_PARTIAL,
} mtk_metadata_enum_android_quirks_partial_result_t;

// MTK_REQUEST_METADATA_MODE
typedef enum mtk_camera_metadata_enum_android_request_metadata_mode {
    MTK_REQUEST_METADATA_MODE_NONE,
    MTK_REQUEST_METADATA_MODE_FULL,
} mtk_camera_metadata_enum_android_request_metadata_mode_t;

// MTK_REQUEST_TYPE
typedef enum mtk_camera_metadata_enum_android_request_type {
    MTK_REQUEST_TYPE_CAPTURE,
    MTK_REQUEST_TYPE_REPROCESS,
} mtk_camera_metadata_enum_android_request_type_t;

// MTK_REQUEST_AVAILABLE_CAPABILITIES
typedef enum mtk_camera_metadata_enum_android_request_available_capabilities {
    MTK_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_SENSOR,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_POST_PROCESSING,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_PRIVATE_REPROCESSING,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_READ_SENSOR_SETTINGS,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_BURST_CAPTURE,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_YUV_REPROCESSING,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_DEPTH_OUTPUT,
    MTK_REQUEST_AVAILABLE_CAPABILITIES_CONSTRAINED_HIGH_SPEED_VIDEO,
} mtk_camera_metadata_enum_android_request_available_capabilities_t;

// MTK_SCALER_AVAILABLE_FORMATS
typedef enum mtk_camera_metadata_enum_android_scaler_available_formats {
    MTK_SCALER_AVAILABLE_FORMATS_RAW16                      = 0x20,
    MTK_SCALER_AVAILABLE_FORMATS_RAW_OPAQUE                 = 0x24,
    MTK_SCALER_AVAILABLE_FORMATS_YV12                       = 0x32315659,
    MTK_SCALER_AVAILABLE_FORMATS_YCrCb_420_SP               = 0x11,
    MTK_SCALER_AVAILABLE_FORMATS_IMPLEMENTATION_DEFINED     = 0x22,
    MTK_SCALER_AVAILABLE_FORMATS_YCbCr_420_888              = 0x23,
    MTK_SCALER_AVAILABLE_FORMATS_BLOB                       = 0x21,
} mtk_camera_metadata_enum_android_scaler_available_formats_t;

// MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
typedef enum mtk_camera_metadata_enum_android_scaler_available_stream_configurations {
    MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT,
    MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT,
} mtk_camera_metadata_enum_android_scaler_available_stream_configurations_t;

// MTK_SCALER_CROPPING_TYPE
typedef enum mtk_camera_metadata_enum_android_scaler_cropping_type {
    MTK_SCALER_CROPPING_TYPE_CENTER_ONLY,
    MTK_SCALER_CROPPING_TYPE_FREEFORM,
} mkt_camera_metadata_enum_android_scaler_cropping_type_t;

// MTK_IOPIPE_INFO_CROP
typedef enum mtk_camera_metadata_enum_android_iopipe_info_crop {
    MTK_IOPIPE_INFO_CROP_NOT_SUPPORT,
    MTK_IOPIPE_INFO_CROP_SYMMETRIC,
    MTK_IOPIPE_INFO_CROP_ASYMMETRIC,
} mtk_camera_metadata_enum_android_iopipe_info_crop_t;

// MTK_SENSOR_REFERENCE_ILLUMINANT1
typedef enum mtk_camera_metadata_enum_android_sensor_reference_illuminant1 {
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT               =  1,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FLUORESCENT            =  2,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_TUNGSTEN               =  3,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FLASH                  =  4,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_FINE_WEATHER           =  9,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_CLOUDY_WEATHER         = 10,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_SHADE                  = 11,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT_FLUORESCENT   = 12,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_DAY_WHITE_FLUORESCENT  = 13,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_COOL_WHITE_FLUORESCENT = 14,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_WHITE_FLUORESCENT      = 15,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_A             = 17,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_B             = 18,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_STANDARD_C             = 19,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D55                    = 20,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D65                    = 21,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D75                    = 22,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_D50                    = 23,
    MTK_SENSOR_REFERENCE_ILLUMINANT1_ISO_STUDIO_TUNGSTEN    = 24,
} mtk_camera_metadata_enum_android_sensor_reference_illuminant1_t;

// MTK_SENSOR_TEST_PATTERN_MODE
typedef enum mtk_camera_metadata_enum_android_sensor_test_pattern_mode {
    MTK_SENSOR_TEST_PATTERN_MODE_OFF,
    MTK_SENSOR_TEST_PATTERN_MODE_SOLID_COLOR,
    MTK_SENSOR_TEST_PATTERN_MODE_COLOR_BARS,
    MTK_SENSOR_TEST_PATTERN_MODE_COLOR_BARS_FADE_TO_GRAY,
    MTK_SENSOR_TEST_PATTERN_MODE_PN9,
    MTK_SENSOR_TEST_PATTERN_MODE_CUSTOM1                    = 256,
} mkt_camera_metadata_enum_android_sensor_test_pattern_mode_t;

// MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT
typedef enum mtk_camera_metadata_enum_android_sensor_info_color_filter_arrangement {
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_RGGB,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GRBG,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GBRG,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR,
    MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_RGB,
} mtk_camera_metadata_enum_android_sensor_info_color_filter_arrangement_t;

// MTK_SENSOR_INFO_TIMESTAMP_SOURCE
typedef enum mtk_camera_metadata_enum_android_sensor_info_timestamp_source {
    MTK_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN,
    MTK_SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME,
} mtk_camera_metadata_enum_android_sensor_info_timestamp_source_t;

// MTK_SENSOR_INFO_LENS_SHADING_APPLIED
typedef enum mtk_camera_metadata_enum_android_sensor_info_lens_shading_applied {
    MTK_SENSOR_INFO_LENS_SHADING_APPLIED_FALSE,
    MTK_SENSOR_INFO_LENS_SHADING_APPLIED_TRUE,
} mtk_camera_metadata_enum_android_sensor_info_lens_shading_applied_t;

// MTK_SENSOR_INFO_SCENARIO_ID
typedef enum mtk_camera_metadata_enum_android_sensor_info_scenario_id {
    MTK_SENSOR_INFO_SCENARIO_ID_ZSD,
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_PREVIEW,
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_CAPTURE,
    MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_VIDEO,
    /**************************************************************************
     * All unnamed scenario id for a specific sensor must be started with
     * values >= MTK_SENSOR_INFO_SCENARIO_ID_UNNAMED_START.
     **************************************************************************/
    MTK_SENSOR_INFO_SCENARIO_ID_UNNAMED_START = 0x100,
} mtk_camera_metadata_enum_android_sensor_info_scenario_id_t;

// MTK_SHADING_MODE
typedef enum mtk_camera_metadata_enum_android_shading_mode {
    MTK_SHADING_MODE_OFF,
    MTK_SHADING_MODE_FAST,
    MTK_SHADING_MODE_HIGH_QUALITY,
} mtk_camera_metadata_enum_android_shading_mode_t;

// MTK_STATISTICS_FACE_DETECT_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_face_detect_mode {
    MTK_STATISTICS_FACE_DETECT_MODE_OFF,
    MTK_STATISTICS_FACE_DETECT_MODE_SIMPLE,
    MTK_STATISTICS_FACE_DETECT_MODE_FULL,
} mtk_camera_metadata_enum_android_statistics_face_detect_mode_t;

// MTK_STATISTICS_HISTOGRAM_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_histogram_mode {
    MTK_STATISTICS_HISTOGRAM_MODE_OFF,
    MTK_STATISTICS_HISTOGRAM_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_histogram_mode_t;

// MTK_STATISTICS_SHARPNESS_MAP_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_sharpness_map_mode {
    MTK_STATISTICS_SHARPNESS_MAP_MODE_OFF,
    MTK_STATISTICS_SHARPNESS_MAP_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_sharpness_map_mode_t;

// MTK_STATISTICS_HOT_PIXEL_MAP_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_hot_pixel_map_mode {
    MTK_STATISTICS_HOT_PIXEL_MAP_MODE_OFF,
    MTK_STATISTICS_HOT_PIXEL_MAP_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_hot_pixel_map_mode_t;

// MTK_STATISTICS_SCENE_FLICKER
typedef enum mtk_camera_metadata_enum_android_statistics_scene_flicker {
    MTK_STATISTICS_SCENE_FLICKER_NONE,
    MTK_STATISTICS_SCENE_FLICKER_50HZ,
    MTK_STATISTICS_SCENE_FLICKER_60HZ,
} mtk_camera_metadata_enum_android_statistics_scene_flicker_t;

// MTK_STATISTICS_LENS_SHADING_MAP_MODE
typedef enum mtk_camera_metadata_enum_android_statistics_lens_shading_map_mode {
    MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF,
    MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON,
} mtk_camera_metadata_enum_android_statistics_lens_shading_map_mode_t;

// MTK_TONEMAP_MODE
typedef enum mtk_camera_metadata_enum_android_tonemap_mode {
    MTK_TONEMAP_MODE_CONTRAST_CURVE,
    MTK_TONEMAP_MODE_FAST,
    MTK_TONEMAP_MODE_HIGH_QUALITY,
    MTK_TONEMAP_MODE_GAMMA_VALUE,
    MTK_TONEMAP_MODE_PRESET_CURVE,
} mtk_camera_metadata_enum_android_tonemap_mode_t;

// MTK_TONEMAP_PRESET_CURVE
typedef enum mtk_camera_metadata_enum_android_tonemap_preset_curve {
    MTK_TONEMAP_PRESET_CURVE_SRGB,
    MTK_TONEMAP_PRESET_CURVE_REC709,
} mtk_camera_metadata_enum_android_tonemap_preset_curve_t;

// MTK_LED_TRANSMIT
typedef enum mtk_camera_metadata_enum_android_led_transmit {
    MTK_LED_TRANSMIT_OFF,
    MTK_LED_TRANSMIT_ON,
} mtk_camera_metadata_enum_android_led_transmit_t;

// MTK_LED_AVAILABLE_LEDS
typedef enum mtk_camera_metadata_enum_android_led_available_leds {
    MTK_LED_AVAILABLE_LEDS_TRANSMIT,
} mtk_camera_metadata_enum_android_led_available_leds_t;

// MTK_INFO_SUPPORTED_HARDWARE_LEVEL
typedef enum mtk_camera_metadata_enum_android_info_supported_hardware_level {
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED,
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_FULL,
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY,
    MTK_INFO_SUPPORTED_HARDWARE_LEVEL_HIGH_RESOLUTION,
} mtk_camera_metadata_enum_android_info_supported_hardware_level_t;

// MTK_BLACK_LEVEL_LOCK
typedef enum mtk_camera_metadata_enum_android_black_level_lock {
    MTK_BLACK_LEVEL_LOCK_OFF,
    MTK_BLACK_LEVEL_LOCK_ON,
} mtk_camera_metadata_enum_android_black_level_lock_t;

// MTK_SYNC_FRAME_NUMBER
typedef enum mtk_camera_metadata_enum_android_sync_frame_number {
    MTK_SYNC_FRAME_NUMBER_CONVERGING                        = -1,
    MTK_SYNC_FRAME_NUMBER_UNKNOWN                           = -2,
} mtk_camera_metadata_enum_android_sync_frame_number_t;

// MTK_SYNC_MAX_LATENCY
typedef enum mtk_camera_metadata_enum_android_sync_max_latency {
    MTK_SYNC_MAX_LATENCY_PER_FRAME_CONTROL                  =  0,
    MTK_SYNC_MAX_LATENCY_UNKNOWN                            = -1,
} mtk_camera_metadata_enum_android_sync_max_latency_t;

// MTK_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS
typedef enum mtk_camera_metadata_enum_android_depth_available_depth_stream_configurations {
    MTK_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS_OUTPUT,
    MTK_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS_INPUT,
} mtk_camera_metadata_enum_android_depth_available_depth_stream_configurations_t;

// MTK_DEPTH_DEPTH_IS_EXCLUSIVE
typedef enum mtk_camera_metadata_enum_android_depth_depth_is_exclusive {
    MTK_DEPTH_DEPTH_IS_EXCLUSIVE_FALSE,
    MTK_DEPTH_DEPTH_IS_EXCLUSIVE_TRUE,
} mtk_camera_metadata_enum_android_depth_depth_is_exclusive_t;

// MTK_HAL_VERSION
typedef enum mtk_camera_metadata_num_hal_version {
    MTK_HAL_VERSION_1_0 = HARDWARE_DEVICE_API_VERSION(1, 0),
    MTK_HAL_VERSION_3_0 = HARDWARE_DEVICE_API_VERSION(3, 0),
    MTK_HAL_VERSION_3_1 = HARDWARE_DEVICE_API_VERSION(3, 1),
    MTK_HAL_VERSION_3_2 = HARDWARE_DEVICE_API_VERSION(3, 2),
    MTK_HAL_VERSION_3_3 = HARDWARE_DEVICE_API_VERSION(3, 3),
} mtk_camera_metadata_num_hal_version_t;

// MTK GESTURE SHOT FEATURE
typedef enum mtk_camera_metadata_enum_gesture_shot_mode {
    MTK_FACE_FEATURE_GESTURE_MODE_OFF = 0,
    MTK_FACE_FEATURE_GESTURE_MODE_SIMPLE,
} mtk_camera_metadata_enum_gesture_shot_mode_t;

// MTK SMILE SHOT FEATURE
typedef enum mtk_camera_metadata_enum_smile_detect_mode {
    MTK_FACE_FEATURE_SMILE_DETECT_MODE_OFF = 0,
    MTK_FACE_FEATURE_SMILE_DETECT_MODE_SIMPLE,
} mtk_camera_metadata_enum_smile_detect_mode_t;

// MTK AUTO SCENE DETECTION FEATURE
typedef enum mtk_camera_metadata_enum_asd_mode {
    MTK_FACE_FEATURE_ASD_MODE_OFF = 0,
    MTK_FACE_FEATURE_ASD_MODE_SIMPLE,
    MTK_FACE_FEATURE_ASD_MODE_FULL,
} mtk_camera_metadata_enum_asd_mode_t;

// MTK 3DNR
typedef enum mtk_camera_metadata_enum_3dnr_mode {
    MTK_NR_FEATURE_3DNR_MODE_OFF = 0,
    MTK_NR_FEATURE_3DNR_MODE_ON,
} mtk_camera_metadata_enum_3dnr_mode_t;
#endif
