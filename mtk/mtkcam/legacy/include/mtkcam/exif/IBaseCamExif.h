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
#ifndef _MTK_CAMERA_INC_COMMON_CAMEXIF_IBASECAMEXIF_H_
#define _MTK_CAMERA_INC_COMMON_CAMEXIF_IBASECAMEXIF_H_

#include "BuiltinTypes.h"
#include <string.h>

/*******************************************************************************
*
********************************************************************************/
#define DBG_EXIF_SIZE       (0xFFFF * 4)
#define REFOCUS_EXIF_SIZE   (0xFFFF * 5)    // for Image Refocus jpeg
#define STEREO_EXIF_SIZE    (0xFFFF * 8)    // for JPS

/*******************************************************************************
* Camera EXIF Command
********************************************************************************/
typedef enum DEBUG_EXIF_CMD_S {

    CMD_REGISTER            = 0x1001,
    CMD_SET_DBG_EXIF

} DEBUG_EXIF_CMD_E;


/*******************************************************************************
* V1: 3A EXIF Information
********************************************************************************/
struct EXIF_INFO_T
{
public:     ////    Data member.
    MUINT32 u4FNumber;          // Format: F2.8 = 28
    MUINT32 u4FocalLength;      // Format: FL 3.5 = 350
    MUINT32 u4SceneMode;        // Scene mode   (SCENE_MODE_XXX)
    MUINT32 u4AWBMode;          // White balance mode   (AWB_NODE_XXX)
    MUINT32 u4FlashLightTimeus; // Strobe on/off
    MUINT32 u4AEMeterMode;      // Exposure metering mode   (AE_METERING_MODE_XXX)
    MINT32  i4AEExpBias;        // Exposure index*10
    MUINT32 u4CapExposureTime;  //
    MUINT32 u4AEISOSpeed;       //
    MUINT32 u4RealISOValue;     //

public:     ////    Operations.
    EXIF_INFO_T()  { ::memset(this, 0, sizeof(EXIF_INFO_T)); }

};


/*******************************************************************************
* V3: standard exif information
********************************************************************************/
struct ExifParams {
    MUINT32     u4ImageWidth;       // Image width
    MUINT32     u4ImageHeight;      // Image height
    //
    MUINT32     u4FNumber;          // Format: F2.8 = 28
    MUINT32     u4FocalLength;      // Format: FL 3.5 = 350
    MUINT32     u4AWBMode;          // White balance mode
    MUINT32     u4LightSource;      // Light Source mode
    MUINT32     u4ExpProgram;       // Exposure Program
    MUINT32     u4SceneCapType;     // Scene Capture Type
    MUINT32     u4FlashLightTimeus; // Strobe on/off
    MUINT32     u4AEMeterMode;      // Exposure metering mode
    MINT32      i4AEExpBias;        // Exposure index*10
    MUINT32     u4CapExposureTime;  //
    MUINT32     u4AEISOSpeed;       // AE ISO value
    //
    MUINT32     u4GpsIsOn;
    MUINT32     u4GPSAltitude;
    MUINT8      uGPSLatitude[32];
    MUINT8      uGPSLongitude[32];
    MUINT8      uGPSTimeStamp[32];
    MUINT8      uGPSProcessingMethod[64];   //(values of "GPS", "CELLID", "WLAN" or "MANUAL" by the EXIF spec.)
    //
    MUINT32     u4Orientation;      // 0, 90, 180, 270
    MUINT32     u4ZoomRatio;        // Digital zoom ratio (x100) For example, 100, 114, and 132 refer to 1.00, 1.14, and 1.32 respectively.
    //
    MUINT32     u4Facing;           // 1: front camera, 0: not front
    //
public:     ////    Operations.
    ExifParams()  { ::memset(this, 0, sizeof(ExifParams)); }

};


/*******************************************************************************
*
********************************************************************************/

enum ECapTypeId
{
    eCapTypeId_Standard   = 0,
    eCapTypeId_Landscape  = 1,
    eCapTypeId_Portrait   = 2,
    eCapTypeId_Night      = 3
};

enum EExpProgramId
{
    eExpProgramId_NotDefined    = 0,
    eExpProgramId_Manual        = 1,
    eExpProgramId_Normal        = 2,
    eExpProgramId_Portrait      = 7,
    eExpProgramId_Landscape     = 8
};

enum ELightSourceId
{
    eLightSourceId_Daylight     = 1,
    eLightSourceId_Fluorescent  = 2,
    eLightSourceId_Tungsten     = 3,
    eLightSourceId_Cloudy       = 10,
    eLightSourceId_Shade        = 11,
    eLightSourceId_Other        = 255
};

enum EMeteringModeId
{
    eMeteringMode_Average   = 1,
    eMeteringMode_Center    = 2,
    eMeteringMode_Spot      = 3,
    eMeteringMode_Other     = 255
};

/*******************************************************************************
* (Basic) Camera Exif
********************************************************************************/
class IBaseCamExif
{
public:     ////    Interfaces.

    IBaseCamExif() {};
    virtual ~IBaseCamExif() {};

//=============================================================
//  Interfaces.
//=============================================================

    /*******************************************************************************
    *  set 3A-related EXIF values for CamExif
    *******************************************************************************/
    virtual
    MBOOL
    set3AEXIFInfo(EXIF_INFO_T* p3AEXIFInfo)   = 0;

    /*******************************************************************************
    *  sendCommand
    *******************************************************************************/
    virtual
    MBOOL sendCommand(
                MINT32      cmd,
                MINT32      arg1 = 0,
                MUINTPTR    arg2 = 0,
                MINT32      arg3 = 0)   = 0;


};


#endif  //  _MTK_CAMERA_INC_COMMON_CAMEXIF_IBASECAMEXIF_H_
