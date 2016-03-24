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


STATIC_METADATA2_BEGIN(DEVICE, LENS, COMMON)
//------------------------------------------------------------------------------
//  android.lens.info
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_AVAILABLE_APERTURES)
        CONFIG_ENTRY_VALUE(2.8f, MFLOAT)
    CONFIG_METADATA_END()

    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES)
        CONFIG_ENTRY_VALUE(0.0f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS)
        CONFIG_ENTRY_VALUE(3.30f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION)
        CONFIG_ENTRY_VALUE(MTK_LENS_OPTICAL_STABILIZATION_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_HYPERFOCAL_DISTANCE)
        CONFIG_ENTRY_VALUE((1.0/5.0), MFLOAT)     // 5 m hyperfocal distance
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE)
        CONFIG_ENTRY_VALUE((1.0/0.05), MFLOAT)    // 5 cm min focus distance
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION)
        CONFIG_ENTRY_VALUE(MTK_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_UNCALIBRATED, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    /**
    CONFIG_METADATA_BEGIN(MTK_LENS_OPTICAL_AXIS_ANGLE)
        CONFIG_ENTRY_VALUE(1.0, MFLOAT)
        CONFIG_ENTRY_VALUE(2.0, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_POSITION)
            CONFIG_ENTRY_VALUE(1.0, MFLOAT)
            CONFIG_ENTRY_VALUE(2.0, MFLOAT)
            CONFIG_ENTRY_VALUE(5.0, MFLOAT)
    CONFIG_METADATA_END()
    **/
    //==========================================================================
//    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_SHADING_MAP,
//        1.f, 1.f, 1.f
//    )
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_LENS_INFO_SHADING_MAP_SIZE)
        CONFIG_ENTRY_VALUE(MSize(1, 1), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
STATIC_METADATA_END()

