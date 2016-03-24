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


STATIC_METADATA2_BEGIN(DEVICE, SENSOR, COMMON)
//------------------------------------------------------------------------------
//  android.sensor
//------------------------------------------------------------------------------
    //==========================================================================
    /*CONFIG_METADATA_BEGIN(MTK_SENSOR_BLACK_LEVEL_PATTERN)
        CONFIG_ENTRY_VALUE(NULL, MINT32)
        CONFIG_ENTRY_VALUE(NULL, MINT32)
        CONFIG_ENTRY_VALUE(NULL, MINT32)
        CONFIG_ENTRY_VALUE(NULL, MINT32)
    CONFIG_METADATA_END()*/
    //==========================================================================
//------------------------------------------------------------------------------
//  android.sensor.info
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION)
        CONFIG_ENTRY_VALUE(MRect(MPoint(0, 0), MSize(3200, 2400)), MRect)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_SENSITIVITY_RANGE)
        CONFIG_ENTRY_VALUE(100, MINT32)
        CONFIG_ENTRY_VALUE(200, MINT32)
        CONFIG_ENTRY_VALUE(400, MINT32)
        CONFIG_ENTRY_VALUE(800, MINT32)
        CONFIG_ENTRY_VALUE(1600, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE)// 1 us - 30 sec
        CONFIG_ENTRY_VALUE(1000L, MINT64)
        CONFIG_ENTRY_VALUE(30000000000L, MINT64)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_MAX_FRAME_DURATION)// 30 sec
        CONFIG_ENTRY_VALUE(30000000000L, MINT64)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PHYSICAL_SIZE) // mm
        CONFIG_ENTRY_VALUE(3.20f, MFLOAT)
        CONFIG_ENTRY_VALUE(2.40f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE)
        CONFIG_ENTRY_VALUE(MSize(3200, 2400), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_WHITE_LEVEL)
        CONFIG_ENTRY_VALUE(4000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
STATIC_METADATA_END()

