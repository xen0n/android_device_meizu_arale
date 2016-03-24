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


STATIC_METADATA_BEGIN(PROJECT, CAMERA, COMMON)
//------------------------------------------------------------------------------
// android.sync
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SYNC_MAX_LATENCY)
        CONFIG_ENTRY_VALUE(MTK_SYNC_MAX_LATENCY_PER_FRAME_CONTROL, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.sensor
//------------------------------------------------------------------------------
    //==========================================================================
    switch  (rInfo.getDeviceId())
    {
    case 0:
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ORIENTATION)
            CONFIG_ENTRY_VALUE(90, MINT32)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_WANTED_ORIENTATION)
            CONFIG_ENTRY_VALUE(90, MINT32)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_FACING)
            CONFIG_ENTRY_VALUE(MTK_LENS_FACING_BACK, MUINT8)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_HAL_VERSION)
            CONFIG_ENTRY_VALUE(MTK_HAL_VERSION_1_0, MINT32)
        CONFIG_METADATA_END()
        //======================================================================

        break;

    case 1:
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ORIENTATION)
            CONFIG_ENTRY_VALUE(270, MINT32)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_WANTED_ORIENTATION)
            CONFIG_ENTRY_VALUE(270, MINT32)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_FACING)
            CONFIG_ENTRY_VALUE(MTK_LENS_FACING_FRONT, MUINT8)
        CONFIG_METADATA_END()
        //======================================================================
        CONFIG_METADATA_BEGIN(MTK_HAL_VERSION)
            CONFIG_ENTRY_VALUE(MTK_HAL_VERSION_1_0, MINT32)
        CONFIG_METADATA_END()
        //======================================================================
        break;

    default:
        break;
    }
    //==========================================================================
//------------------------------------------------------------------------------
STATIC_METADATA_END()

