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
* @file aaa_hal_cmd.h
* @brief 3A command enumeration
*/
#ifndef __AAA_HAL_CMD_H__
#define __AAA_HAL_CMD_H__

namespace NS3Av3
{
enum E3ACaptureMode_T
{
    ECapMode_P2_Cal     = (1<<0),
    ECapMode_P2_Set     = (1<<1)
};

enum E3ACtrl_T
{
    E3ACtrl_Begin = 0,
    // shading
    E3ACtrl_SetShadingMode          = 0x0101,
    E3ACtrl_SetShadingStrength      = 0x0102,
    E3ACtrl_SetShadingMapMode       = 0x0103,
    E3ACtrl_SetShadingMapSize       = 0x0104,
    //AWB
    E3ACtrl_SetAwbBypCalibration    = 0x0100,
    //AE set
    E3ACtrl_SetExposureParam        = 0x0200,
    E3ACtrl_SetHalHdr,
    E3ACtrl_EnableDisableAE,
    //AE get
    E3ACtrl_GetAEPLineTable         = 0x0280,
    E3ACtrl_GetExposureInfo,
    E3ACtrl_GetExposureParam,
    E3ACtrl_GetHdrCapInfo,
    //ISP
    E3ACtrl_ConvertToIspGamma       = 0x0300,
    //
    //3A misc set
    E3ACtrl_Enable3ASetParams       = 0x0400,
    //3A misc get
    E3ACtrl_GetAsdInfo              = 0x0480,
    //
    E3ACtrl_GetAfPos                = 0x0500,
    //
    E3ACtrl_Num
};
struct Hal3A_HDROutputParam_T
{
    MUINT32 u4OutputFrameNum;     // Output frame number (2 or 3)
    MUINT32 u4FinalGainDiff[2];   // 1x=1024; [0]: Between short exposure and 0EV; [1]: Between 0EV and long exposure
    MUINT32 u4TargetTone; //Decide the curve to decide target tone
};
/**
 * @brief 3A parameters for capture
 */
struct CaptureParam_T
{
    MUINT32 u4ExposureMode;  //0: exp. time, 1: exp. line
    MUINT32 u4Eposuretime;   //!<: Exposure time in us
    MUINT32 u4AfeGain;       //!<: sensor gain
    MUINT32 u4IspGain;       //!<: raw gain
    MUINT32 u4RealISO;       //!<: Real ISO speed
    MUINT32 u4FlareOffset;
    MUINT32 u4FlareGain;     // 512 is 1x
    MINT32  i4LightValue_x10;// 10 base LV value
    MUINT32 u43ACapMode;     //refer to enum E3ACaptureMode_T
    CaptureParam_T()
        : u43ACapMode(ECapMode_P2_Cal|ECapMode_P2_Set)
    {}
};

struct E3ACtrl_ConvertToIspGamma_ARG_T
{
    MUINT32 u4NumPts;
    const MFLOAT* pPtPairs;
    MINT32* pOutGMA;

    E3ACtrl_ConvertToIspGamma_ARG_T()
        : pPtPairs(NULL), u4NumPts(0), pOutGMA(NULL) {}
    E3ACtrl_ConvertToIspGamma_ARG_T(const MFLOAT* _pPtPairs, MUINT32 _u4NumPts, MINT32* _pOutGMA)
        : pPtPairs(_pPtPairs), u4NumPts(_u4NumPts), pOutGMA(_pOutGMA) {}
};
};
#endif  //__AAA_HAL_CMD_H__
