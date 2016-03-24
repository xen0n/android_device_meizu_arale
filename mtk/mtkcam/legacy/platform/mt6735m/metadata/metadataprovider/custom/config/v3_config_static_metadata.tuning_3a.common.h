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


STATIC_METADATA2_BEGIN(DEVICE, TUNING_3A, COMMON)
//------------------------------------------------------------------------------
//  android.control
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_AVAILABLE_ABERRATION_MODES)
        CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_ANTIBANDING_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON_AUTO_FLASH, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH, MUINT8)
        //CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES)
        CONFIG_ENTRY_VALUE(5, MINT32)
        CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_ENTRY_VALUE(15, MINT32)
        CONFIG_ENTRY_VALUE(15, MINT32)
        CONFIG_ENTRY_VALUE(30, MINT32)
        CONFIG_ENTRY_VALUE(30, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_COMPENSATION_RANGE)
        CONFIG_ENTRY_VALUE(-3, MINT32)
        CONFIG_ENTRY_VALUE(3, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AE_COMPENSATION_STEP)
        CONFIG_ENTRY_VALUE(MRational(1, 2), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AF_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_AUTO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_MACRO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AVAILABLE_EFFECTS)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_MONO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_NEGATIVE, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_SEPIA, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_WHITEBOARD, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_BLACKBOARD, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_AQUA, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AVAILABLE_SCENE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_SCENE_MODE_DISABLED  , MUINT8) //MTK_CONTROL_SCENE_MODE_UNSUPPORTED
        CONFIG_ENTRY_VALUE(MTK_CONTROL_SCENE_MODE_FACE_PRIORITY, MUINT8)
//        MTK_CONTROL_SCENE_MODE_ACTION,
//        MTK_CONTROL_SCENE_MODE_PORTRAIT,
//        MTK_CONTROL_SCENE_MODE_LANDSCAPE,
//        MTK_CONTROL_SCENE_MODE_NIGHT,
//        MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
//        MTK_CONTROL_SCENE_MODE_THEATRE,
//        MTK_CONTROL_SCENE_MODE_BEACH,
//        MTK_CONTROL_SCENE_MODE_SNOW,
//        MTK_CONTROL_SCENE_MODE_SUNSET,
//        MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
//        MTK_CONTROL_SCENE_MODE_FIREWORKS,
//        MTK_CONTROL_SCENE_MODE_SPORTS,
//        MTK_CONTROL_SCENE_MODE_PARTY,
//        MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
//        MTK_CONTROL_SCENE_MODE_BARCODE,
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AWB_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_INCANDESCENT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_FLUORESCENT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_DAYLIGHT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_TWILIGHT, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_SHADE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_MAX_REGIONS)
        CONFIG_ENTRY_VALUE(1, MINT32)
        CONFIG_ENTRY_VALUE(1, MINT32)
        CONFIG_ENTRY_VALUE(1, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_SCENE_MODE_OVERRIDES)
        //  MTK_CONTROL_SCENE_MODE_UNSUPPORTED
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON_AUTO_FLASH, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_OFF, MUINT8)

        //  MTK_CONTROL_SCENE_MODE_FACE_PRIORITY
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.statistics.info
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT)
        CONFIG_ENTRY_VALUE(64, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_MAX_HISTOGRAM_COUNT)
        CONFIG_ENTRY_VALUE(1000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE)
        CONFIG_ENTRY_VALUE(1000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_SHARPNESS_MAP_SIZE)
        CONFIG_ENTRY_VALUE(MSize(64, 64), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.tonemap
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_MAX_CURVE_POINTS)
        CONFIG_ENTRY_VALUE(101, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_AVAILABLE_TONE_MAP_MODES)
        CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_CONTRAST_CURVE, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
    CONFIG_METADATA_END()
//------------------------------------------------------------------------------
//  android.edge
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_EDGE_AVAILABLE_EDGE_MODES)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_HIGH_QUALITY, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.flash
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_FLASH_INFO_AVAILABLE)
        CONFIG_ENTRY_VALUE(MTK_FLASH_INFO_AVAILABLE_TRUE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.hotPixel
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_AVAILABLE_HOT_PIXEL_MODES)
        CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.noiseReduction
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES)
        CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
STATIC_METADATA_END()


#if 0
static tag_info_t android_color_correction[ANDROID_COLOR_CORRECTION_END -
        ANDROID_COLOR_CORRECTION_START] = {
    [ ANDROID_COLOR_CORRECTION_MODE - ANDROID_COLOR_CORRECTION_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_COLOR_CORRECTION_TRANSFORM - ANDROID_COLOR_CORRECTION_START ] =
    { "transform",                     TYPE_FLOAT  },
};

static tag_info_t android_control[ANDROID_CONTROL_END -
        ANDROID_CONTROL_START] = {

    [ ANDROID_CONTROL_AVAILABLE_EFFECTS - ANDROID_CONTROL_START ] =
    { "availableEffects",              TYPE_BYTE   },
    [ ANDROID_CONTROL_AVAILABLE_SCENE_MODES - ANDROID_CONTROL_START ] =
    { "availableSceneModes",           TYPE_BYTE   },

};

static tag_info_t android_demosaic[ANDROID_DEMOSAIC_END -
        ANDROID_DEMOSAIC_START] = {
    [ ANDROID_DEMOSAIC_MODE - ANDROID_DEMOSAIC_START ] =
    { "mode",                          TYPE_BYTE   },
};

static tag_info_t android_edge[ANDROID_EDGE_END -
        ANDROID_EDGE_START] = {
    [ ANDROID_EDGE_MODE - ANDROID_EDGE_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_EDGE_STRENGTH - ANDROID_EDGE_START ] =
    { "strength",                      TYPE_BYTE   },
};

static tag_info_t android_flash[ANDROID_FLASH_END -
        ANDROID_FLASH_START] = {
    [ ANDROID_FLASH_FIRING_POWER - ANDROID_FLASH_START ] =
    { "firingPower",                   TYPE_BYTE   },
    [ ANDROID_FLASH_FIRING_TIME - ANDROID_FLASH_START ] =
    { "firingTime",                    TYPE_INT64  },
    [ ANDROID_FLASH_MODE - ANDROID_FLASH_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_FLASH_COLOR_TEMPERATURE - ANDROID_FLASH_START ] =
    { "colorTemperature",              TYPE_BYTE   },
    [ ANDROID_FLASH_MAX_ENERGY - ANDROID_FLASH_START ] =
    { "maxEnergy",                     TYPE_BYTE   },
    [ ANDROID_FLASH_STATE - ANDROID_FLASH_START ] =
    { "state",                         TYPE_BYTE   },
};

static tag_info_t android_flash_info[ANDROID_FLASH_INFO_END -
        ANDROID_FLASH_INFO_START] = {
    [ ANDROID_FLASH_INFO_AVAILABLE - ANDROID_FLASH_INFO_START ] =
    { "available",                     TYPE_BYTE   },
    [ ANDROID_FLASH_INFO_CHARGE_DURATION - ANDROID_FLASH_INFO_START ] =
    { "chargeDuration",                TYPE_INT64  },
};

static tag_info_t android_geometric[ANDROID_GEOMETRIC_END -
        ANDROID_GEOMETRIC_START] = {
    [ ANDROID_GEOMETRIC_MODE - ANDROID_GEOMETRIC_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_GEOMETRIC_STRENGTH - ANDROID_GEOMETRIC_START ] =
    { "strength",                      TYPE_BYTE   },
};

static tag_info_t android_hot_pixel[ANDROID_HOT_PIXEL_END -
        ANDROID_HOT_PIXEL_START] = {
    [ ANDROID_HOT_PIXEL_MODE - ANDROID_HOT_PIXEL_START ] =
    { "mode",                          TYPE_BYTE   },
};

static tag_info_t android_hot_pixel_info[ANDROID_HOT_PIXEL_INFO_END -
        ANDROID_HOT_PIXEL_INFO_START] = {
    [ ANDROID_HOT_PIXEL_INFO_MAP - ANDROID_HOT_PIXEL_INFO_START ] =
    { "map",                           TYPE_INT32  },
};

static tag_info_t android_noise_reduction[ANDROID_NOISE_REDUCTION_END -
        ANDROID_NOISE_REDUCTION_START] = {
    [ ANDROID_NOISE_REDUCTION_MODE - ANDROID_NOISE_REDUCTION_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_NOISE_REDUCTION_STRENGTH - ANDROID_NOISE_REDUCTION_START ] =
    { "strength",                      TYPE_BYTE   },
};

static tag_info_t android_shading[ANDROID_SHADING_END -
        ANDROID_SHADING_START] = {
    [ ANDROID_SHADING_MODE - ANDROID_SHADING_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_SHADING_STRENGTH - ANDROID_SHADING_START ] =
    { "strength",                      TYPE_BYTE   },
};

static tag_info_t android_statistics_info[ANDROID_STATISTICS_INFO_END -
        ANDROID_STATISTICS_INFO_START] = {
    [ ANDROID_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES - ANDROID_STATISTICS_INFO_START ] =
    { "availableFaceDetectModes",      TYPE_BYTE   },
    [ ANDROID_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT - ANDROID_STATISTICS_INFO_START ] =
    { "histogramBucketCount",          TYPE_INT32  },
    [ ANDROID_STATISTICS_INFO_MAX_FACE_COUNT - ANDROID_STATISTICS_INFO_START ] =
    { "maxFaceCount",                  TYPE_INT32  },
    [ ANDROID_STATISTICS_INFO_MAX_HISTOGRAM_COUNT - ANDROID_STATISTICS_INFO_START ] =
    { "maxHistogramCount",             TYPE_INT32  },
    [ ANDROID_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE - ANDROID_STATISTICS_INFO_START ] =
    { "maxSharpnessMapValue",          TYPE_INT32  },
    [ ANDROID_STATISTICS_INFO_SHARPNESS_MAP_SIZE - ANDROID_STATISTICS_INFO_START ] =
    { "sharpnessMapSize",              TYPE_INT32  },
};

static tag_info_t android_tonemap[ANDROID_TONEMAP_END -
        ANDROID_TONEMAP_START] = {
    [ ANDROID_TONEMAP_CURVE_BLUE - ANDROID_TONEMAP_START ] =
    { "curveBlue",                     TYPE_FLOAT  },
    [ ANDROID_TONEMAP_CURVE_GREEN - ANDROID_TONEMAP_START ] =
    { "curveGreen",                    TYPE_FLOAT  },
    [ ANDROID_TONEMAP_CURVE_RED - ANDROID_TONEMAP_START ] =
    { "curveRed",                      TYPE_FLOAT  },
    [ ANDROID_TONEMAP_MODE - ANDROID_TONEMAP_START ] =
    { "mode",                          TYPE_BYTE   },
    [ ANDROID_TONEMAP_MAX_CURVE_POINTS - ANDROID_TONEMAP_START ] =
    { "maxCurvePoints",                TYPE_INT32  },
};
#endif

