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

/**
* @file aaa_result.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __AAA_RESULT_H__
#define __AAA_RESULT_H__

#include <isp_tuning.h>
//#include <camera_feature.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_cam_info.h>
#include <utils/Vector.h>

namespace NS3Av3
{

struct EXIF_3A_INFO_T
{
    MUINT32     u4FNumber;                  // Format: F2.8 = 28
    MUINT32     u4FocalLength;              // Format: FL 3.5 = 350
//    MUINT32     u4SceneMode;              // Scene mode   (SCENE_MODE_XXX)
    MUINT32     u4AWBMode;                  // White balance mode          update!!
    MUINT32     u4LightSource;              // Light Source mode             new!!
    MUINT32     u4ExpProgram;               // Exposure Program              new!!
    MUINT32     u4SceneCapType;             // Scene Capture Type            new!!
    MUINT32     u4FlashLightTimeus;         // Strobe on/off
    MUINT32     u4AEMeterMode;              // Exposure metering mode   update!!
    MINT32      i4AEExpBias;                // Exposure index*10
    MUINT32     u4CapExposureTime;          //
    MUINT32     u4AEISOSpeed;               // AE ISO value                       update!!
//    MUINT32     u4RealISOValue;           // ISO value

    EXIF_3A_INFO_T()  { ::memset(this, 0, sizeof(EXIF_3A_INFO_T)); }
};

struct Result_T
{
    MINT32      i4FrmId;
    MUINT8      u1SceneMode;
    // state
    MUINT8      u1AfState;
    MUINT8      u1AeState;
    MUINT8      u1AwbState;
    MUINT8      u1LensState;
    MUINT8      u1FlashState;
    MUINT8      u1SceneFlk;
    // lens
    MFLOAT      fLensAperture;
    MFLOAT      fLensFilterDensity;
    MFLOAT      fLensFocalLength;
    MFLOAT      fLensFocusDistance;
    MFLOAT      fLensFocusRange[2];
    // sensor
    MINT64      i8SensorRollingShutterSkew;
    MINT64      i8SensorExposureTime;
    MINT64      i8SensorFrameDuration;
    MINT32      i4SensorSensitivity;
    // color correction
    MINT32      i4AwbGain[3];
    MINT32      i4AwbGainScaleUint;
    MFLOAT      fColorCorrectGain[4];
    //MFLOAT      fColorCorrectMat[9];
    android::Vector<MFLOAT> vecColorCorrectMat;
    // shading map
    android::Vector<MFLOAT> vecShadingMap;
    // tonemap
    android::Vector<MFLOAT> vecTonemapCurveBlue;
    android::Vector<MFLOAT> vecTonemapCurveGreen;
    android::Vector<MFLOAT> vecTonemapCurveRed;
    // cam info
    NSIspTuning::RAWIspCamInfo_U rCamInfo;
    // exif
    android::Vector<EXIF_3A_INFO_T> vecExifInfo;
};

};

#endif //__AAA_RESULT_H__