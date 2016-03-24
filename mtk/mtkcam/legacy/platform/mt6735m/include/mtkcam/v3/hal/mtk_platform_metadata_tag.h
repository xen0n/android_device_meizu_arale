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

#ifndef _MTK_PLATFORM_INCLUDE_MTK_PLATFORM_METADATA_TAG_H_
#define _MTK_PLATFORM_INCLUDE_MTK_PLATFORM_METADATA_TAG_H_

/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_section {
    MTK_HAL_REQUEST,
    MTK_P1NODE,
    MTK_3A_TUNINING,
    MTK_3A_EXIF,
    MTK_EIS
} mtk_platform_metadata_section_t;


/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_section_start {
    MTK_HAL_REQUEST_START      = MTK_HAL_REQUEST       << 16,
    MTK_P1NODE_START           = MTK_P1NODE            << 16,
    MTK_3A_TUNINING_START      = MTK_3A_TUNINING       << 16,
    MTK_3A_EXIF_START          = MTK_3A_EXIF           << 16,
    MTK_EIS_START              = MTK_EIS               << 16,
} mtk_platform_metadata_section_start_t;


/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_tag {
    MTK_HAL_REQUEST_REQUIRE_EXIF      = MTK_HAL_REQUEST_START, //MUINT8
    MTK_HAL_REQUEST_REPEAT,                                    //MUINT8
    MTK_HAL_REQUEST_DUMMY,                                     //MUINT8
    MTK_HAL_REQUEST_SENSOR_SIZE,                               //MSize
    MTK_P1NODE_SCALAR_CROP_REGION     = MTK_P1NODE_START,      //MRect
    MTK_P1NODE_DMA_CROP_REGION,                                //MRect
    MTK_P1NODE_RESIZER_SIZE,                                   //MSize
    MTK_P1NODE_PROCESSOR_MAGICNUM,                             //MINT32
    MTK_PROCESSOR_CAMINFO             = MTK_3A_TUNINING_START, //MUINT8
    MTK_3A_ISP_PROFILE,                                        //MUINT8
    MTK_3A_AE_CAP_PARAM,                                       //IMemory
    MTK_3A_EXIF_METADATA              = MTK_3A_EXIF_START,     //IMetadata
    MTK_EIS_REGION                    = MTK_EIS_START,         //MINT32
} mtk_platform_metadata_tag_t;


/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_3a_exif_metadata_tag {
    MTK_3A_EXIF_FNUMBER,                                       //MINT32
    MTK_3A_EXIF_FOCAL_LENGTH,                                  //MINT32
    MTK_3A_EXIF_SCENE_MODE,                                    //MINT32
    MTK_3A_EXIF_AWB_MODE,                                      //MINT32
    MTK_3A_EXIF_LIGHT_SOURCE,                                  //MINT32
    MTK_3A_EXIF_EXP_PROGRAM,                                   //MINT32
    MTK_3A_EXIF_SCENE_CAP_TYPE,                                //MINT32
    MTK_3A_EXIF_FLASH_LIGHT_TIME_US,                           //MINT32
    MTK_3A_EXIF_AE_METER_MODE,                                 //MINT32
    MTK_3A_EXIF_AE_EXP_BIAS,                                   //MINT32
    MTK_3A_EXIF_CAP_EXPOSURE_TIME,                             //MINT32
    MTK_3A_EXIF_AE_ISO_SPEED,                                  //MINT32
    MTK_3A_EXIF_REAL_ISO_VALUE,                                //MINT32
    MTK_3A_EXIF_DEBUGINFO_BEGIN, // debug info begin
    // key: MINT32
    MTK_3A_EXIF_DBGINFO_AAA_KEY = MTK_3A_EXIF_DEBUGINFO_BEGIN, //MINT32
    MTK_3A_EXIF_DBGINFO_AAA_DATA,
    MTK_3A_EXIF_DBGINFO_SDINFO_KEY,
    MTK_3A_EXIF_DBGINFO_SDINFO_DATA,
    MTK_3A_EXIF_DBGINFO_ISP_KEY,
    MTK_3A_EXIF_DBGINFO_ISP_DATA,
    // data: Memory
    MTK_3A_EXIF_DEBUGINFO_END,   // debug info end
} mtk_platform_3a_exif_metadata_tag_t;

#endif

