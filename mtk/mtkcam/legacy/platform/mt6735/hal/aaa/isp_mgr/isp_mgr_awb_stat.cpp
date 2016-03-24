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
#define LOG_TAG "isp_mgr_awb_stat"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_reg.h>
#include "isp_mgr.h"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AWB_STAT_CONFIG_T&
ISP_MGR_AWB_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
config(AWB_STAT_CONFIG_T& rAWBStatConfig, MBOOL bHBIN2Enable, MINT32 i4SensorIndex)
{
    addressErrorCheck("Before ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    m_i4SensorIndex = i4SensorIndex;

    if (m_eSensorTG == ESensorTG_1) {
        // CAM_AWB_WIN_ORG
        reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_W_HORG = (bHBIN2Enable) ? (rAWBStatConfig.i4WindowOriginX/2) : rAWBStatConfig.i4WindowOriginX;
        reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_W_VORG = rAWBStatConfig.i4WindowOriginY;
        // CAM_AWB_WIN_SIZE
        reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_W_HSIZE = (bHBIN2Enable) ? ((rAWBStatConfig.i4WindowPitchX/4)*2) : rAWBStatConfig.i4WindowSizeX;
        reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_W_VSIZE = rAWBStatConfig.i4WindowSizeY;
        // CAM_AWB_WIN_PIT
        reinterpret_cast<ISP_CAM_AWB_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PIT))->AWB_W_HPIT = (bHBIN2Enable) ? (rAWBStatConfig.i4WindowPitchX/2) : rAWBStatConfig.i4WindowPitchX;
        reinterpret_cast<ISP_CAM_AWB_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PIT))->AWB_W_VPIT = rAWBStatConfig.i4WindowPitchY;
        // CAM_AWB_WIN_NUM
        reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_W_HNUM = rAWBStatConfig.i4WindowNumX;
        reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_W_VNUM = rAWBStatConfig.i4WindowNumY;
        // CAM_AWB_GAIN1_0
        reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_R = rAWBStatConfig.i4PreGainR;
        reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_G = rAWBStatConfig.i4PreGainG;
        // CAM_AWB_GAIN1_1
        reinterpret_cast<ISP_CAM_AWB_GAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_1))->AWB_GAIN1_B = rAWBStatConfig.i4PreGainB;
        // CAM_AWB_LMT1_0
        reinterpret_cast<ISP_CAM_AWB_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_0))->AWB_LMT1_R = rAWBStatConfig.i4PreGainLimitR;
        reinterpret_cast<ISP_CAM_AWB_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_0))->AWB_LMT1_G = rAWBStatConfig.i4PreGainLimitG;
        // CAM_AWB_LMT1_1
        reinterpret_cast<ISP_CAM_AWB_LMT1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_1))->AWB_LMT1_B = rAWBStatConfig.i4PreGainLimitB;
        // CAM_AWB_LOW_THR
        reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;
        reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;
        reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;
        // CAM_AWB_HI_THR
        reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;
        reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;
        reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;
        // CAM_AWB_PIXEL_CNT0
        reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT0))->AWB_PIXEL_CNT0 = (bHBIN2Enable) ? (rAWBStatConfig.i4PixelCountR * 2) : rAWBStatConfig.i4PixelCountR;
        // CAM_AWB_PIXEL_CNT1
        reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT1))->AWB_PIXEL_CNT1 = (bHBIN2Enable) ? (rAWBStatConfig.i4PixelCountG * 2) : rAWBStatConfig.i4PixelCountG;
        // CAM_AWB_PIXEL_CNT2
        reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT2_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT2))->AWB_PIXEL_CNT2 = (bHBIN2Enable) ? (rAWBStatConfig.i4PixelCountB * 2) : rAWBStatConfig.i4PixelCountB;
        // CAM_AWB_ERR_THR
        reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;
        reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_SFT = rAWBStatConfig.i4ErrorShiftBits;

        // CAM_AWB_ROT
        reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_C = (rAWBStatConfig.i4Cos >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Cos) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Cos);
        reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_S = (rAWBStatConfig.i4Sin >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Sin) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Sin);

        #define AWB_LIGHT_AREA_CFG(TYPE, REG, FIELD, BOUND)\
        if (BOUND >= 0)\
            reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = BOUND;\
        else\
            reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = (1 << 14) + BOUND;\


        // CAM_AWB_L0
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_LOW, rAWBStatConfig.i4AWBXY_WINL[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_UP, rAWBStatConfig.i4AWBXY_WINR[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_UP, rAWBStatConfig.i4AWBXY_WINU[0])

        // CAM_AWB_L1
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_LOW, rAWBStatConfig.i4AWBXY_WINL[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_UP, rAWBStatConfig.i4AWBXY_WINR[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_UP, rAWBStatConfig.i4AWBXY_WINU[1])

        // CAM_AWB_L2
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_LOW, rAWBStatConfig.i4AWBXY_WINL[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_UP, rAWBStatConfig.i4AWBXY_WINR[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_UP, rAWBStatConfig.i4AWBXY_WINU[2])

        // CAM_AWB_L3
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_LOW, rAWBStatConfig.i4AWBXY_WINL[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_UP, rAWBStatConfig.i4AWBXY_WINR[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_UP, rAWBStatConfig.i4AWBXY_WINU[3])

        // CAM_AWB_L4
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_LOW, rAWBStatConfig.i4AWBXY_WINL[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_UP, rAWBStatConfig.i4AWBXY_WINR[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_UP, rAWBStatConfig.i4AWBXY_WINU[4])

        // CAM_AWB_L5
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_LOW, rAWBStatConfig.i4AWBXY_WINL[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_UP, rAWBStatConfig.i4AWBXY_WINR[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_UP, rAWBStatConfig.i4AWBXY_WINU[5])

        // CAM_AWB_L6
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_LOW, rAWBStatConfig.i4AWBXY_WINL[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_UP, rAWBStatConfig.i4AWBXY_WINR[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_UP, rAWBStatConfig.i4AWBXY_WINU[6])

        // CAM_AWB_L7
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_LOW, rAWBStatConfig.i4AWBXY_WINL[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_UP, rAWBStatConfig.i4AWBXY_WINR[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_UP, rAWBStatConfig.i4AWBXY_WINU[7])

        // CAM_AWB_L8
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_LOW, rAWBStatConfig.i4AWBXY_WINL[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_UP, rAWBStatConfig.i4AWBXY_WINR[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_UP, rAWBStatConfig.i4AWBXY_WINU[8])

        // CAM_AWB_L9
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_LOW, rAWBStatConfig.i4AWBXY_WINL[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_UP, rAWBStatConfig.i4AWBXY_WINR[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_UP, rAWBStatConfig.i4AWBXY_WINU[9])
    }
    else {
        // CAM_AWB_D_WIN_ORG
        reinterpret_cast<ISP_CAM_AWB_D_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_ORG))->AWB_W_HORG = rAWBStatConfig.i4WindowOriginX;
        reinterpret_cast<ISP_CAM_AWB_D_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_ORG))->AWB_W_VORG = rAWBStatConfig.i4WindowOriginY;
        // CAM_AWB_D_WIN_SIZE
        reinterpret_cast<ISP_CAM_AWB_D_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_SIZE))->AWB_W_HSIZE = rAWBStatConfig.i4WindowSizeX;
        reinterpret_cast<ISP_CAM_AWB_D_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_SIZE))->AWB_W_VSIZE = rAWBStatConfig.i4WindowSizeY;
        // CAM_AWB_D_WIN_PIT
        reinterpret_cast<ISP_CAM_AWB_D_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_PIT))->AWB_W_HPIT = rAWBStatConfig.i4WindowPitchX;
        reinterpret_cast<ISP_CAM_AWB_D_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_PIT))->AWB_W_VPIT = rAWBStatConfig.i4WindowPitchY;
        // CAM_AWB_D_WIN_NUM
        reinterpret_cast<ISP_CAM_AWB_D_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_NUM))->AWB_W_HNUM = rAWBStatConfig.i4WindowNumX;
        reinterpret_cast<ISP_CAM_AWB_D_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_WIN_NUM))->AWB_W_VNUM = rAWBStatConfig.i4WindowNumY;
        // CAM_AWB_D_GAIN1_0
        reinterpret_cast<ISP_CAM_AWB_D_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_GAIN1_0))->AWB_GAIN1_R = rAWBStatConfig.i4PreGainR;
        reinterpret_cast<ISP_CAM_AWB_D_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_GAIN1_0))->AWB_GAIN1_G = rAWBStatConfig.i4PreGainG;
        // CAM_AWB_D_GAIN1_1
        reinterpret_cast<ISP_CAM_AWB_D_GAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_GAIN1_1))->AWB_GAIN1_B = rAWBStatConfig.i4PreGainB;
        // CAM_AWB_D_LMT1_0
        reinterpret_cast<ISP_CAM_AWB_D_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LMT1_0))->AWB_LMT1_R = rAWBStatConfig.i4PreGainLimitR;
        reinterpret_cast<ISP_CAM_AWB_D_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LMT1_0))->AWB_LMT1_G = rAWBStatConfig.i4PreGainLimitG;
        // CAM_AWB_D_LMT1_1
        reinterpret_cast<ISP_CAM_AWB_D_LMT1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LMT1_1))->AWB_LMT1_B = rAWBStatConfig.i4PreGainLimitB;
        // CAM_AWB_D_LOW_THR
        reinterpret_cast<ISP_CAM_AWB_D_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LOW_THR))->AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;
        reinterpret_cast<ISP_CAM_AWB_D_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LOW_THR))->AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;
        reinterpret_cast<ISP_CAM_AWB_D_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_LOW_THR))->AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;
        // CAM_AWB_D_HI_THR
        reinterpret_cast<ISP_CAM_AWB_D_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_HI_THR))->AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;
        reinterpret_cast<ISP_CAM_AWB_D_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_HI_THR))->AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;
        reinterpret_cast<ISP_CAM_AWB_D_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_HI_THR))->AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;
        // CAM_AWB_D_PIXEL_CNT0
        reinterpret_cast<ISP_CAM_AWB_D_PIXEL_CNT0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_PIXEL_CNT0))->AWB_PIXEL_CNT0 = rAWBStatConfig.i4PixelCountR;
        // CAM_AWB_D_PIXEL_CNT1
        reinterpret_cast<ISP_CAM_AWB_D_PIXEL_CNT1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_PIXEL_CNT1))->AWB_PIXEL_CNT1 = rAWBStatConfig.i4PixelCountG;
        // CAM_AWB_D_PIXEL_CNT2
        reinterpret_cast<ISP_CAM_AWB_D_PIXEL_CNT2_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_PIXEL_CNT2))->AWB_PIXEL_CNT2 = rAWBStatConfig.i4PixelCountB;
        // CAM_AWB_D_ERR_THR
        reinterpret_cast<ISP_CAM_AWB_D_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_ERR_THR))->AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;
        reinterpret_cast<ISP_CAM_AWB_D_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_ERR_THR))->AWB_ERR_SFT = rAWBStatConfig.i4ErrorShiftBits;

        // CAM_AWB_D_ROT
        reinterpret_cast<ISP_CAM_AWB_D_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_ROT))->AWB_C = (rAWBStatConfig.i4Cos >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Cos) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Cos);
        reinterpret_cast<ISP_CAM_AWB_D_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_D_ROT))->AWB_S = (rAWBStatConfig.i4Sin >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Sin) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Sin);

        #define AWB_LIGHT_AREA_CFG(TYPE, REG, FIELD, BOUND)\
        if (BOUND >= 0)\
            reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = BOUND;\
        else\
            reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = (1 << 14) + BOUND;\


        // CAM_AWB_D_L0
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L0_X_T, CAM_AWB_D_L0_X, AWB_L0_X_LOW, rAWBStatConfig.i4AWBXY_WINL[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L0_X_T, CAM_AWB_D_L0_X, AWB_L0_X_UP, rAWBStatConfig.i4AWBXY_WINR[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L0_Y_T, CAM_AWB_D_L0_Y, AWB_L0_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[0])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L0_Y_T, CAM_AWB_D_L0_Y, AWB_L0_Y_UP, rAWBStatConfig.i4AWBXY_WINU[0])

        // CAM_AWB_D_L1
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L1_X_T, CAM_AWB_D_L1_X, AWB_L1_X_LOW, rAWBStatConfig.i4AWBXY_WINL[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L1_X_T, CAM_AWB_D_L1_X, AWB_L1_X_UP, rAWBStatConfig.i4AWBXY_WINR[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L1_Y_T, CAM_AWB_D_L1_Y, AWB_L1_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[1])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L1_Y_T, CAM_AWB_D_L1_Y, AWB_L1_Y_UP, rAWBStatConfig.i4AWBXY_WINU[1])

        // CAM_AWB_D_L2
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L2_X_T, CAM_AWB_D_L2_X, AWB_L2_X_LOW, rAWBStatConfig.i4AWBXY_WINL[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L2_X_T, CAM_AWB_D_L2_X, AWB_L2_X_UP, rAWBStatConfig.i4AWBXY_WINR[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L2_Y_T, CAM_AWB_D_L2_Y, AWB_L2_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[2])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L2_Y_T, CAM_AWB_D_L2_Y, AWB_L2_Y_UP, rAWBStatConfig.i4AWBXY_WINU[2])

        // CAM_AWB_D_L3
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L3_X_T, CAM_AWB_D_L3_X, AWB_L3_X_LOW, rAWBStatConfig.i4AWBXY_WINL[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L3_X_T, CAM_AWB_D_L3_X, AWB_L3_X_UP, rAWBStatConfig.i4AWBXY_WINR[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L3_Y_T, CAM_AWB_D_L3_Y, AWB_L3_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[3])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L3_Y_T, CAM_AWB_D_L3_Y, AWB_L3_Y_UP, rAWBStatConfig.i4AWBXY_WINU[3])

        // CAM_AWB_D_L4
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L4_X_T, CAM_AWB_D_L4_X, AWB_L4_X_LOW, rAWBStatConfig.i4AWBXY_WINL[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L4_X_T, CAM_AWB_D_L4_X, AWB_L4_X_UP, rAWBStatConfig.i4AWBXY_WINR[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L4_Y_T, CAM_AWB_D_L4_Y, AWB_L4_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[4])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L4_Y_T, CAM_AWB_D_L4_Y, AWB_L4_Y_UP, rAWBStatConfig.i4AWBXY_WINU[4])

        // CAM_AWB_D_L5
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L5_X_T, CAM_AWB_D_L5_X, AWB_L5_X_LOW, rAWBStatConfig.i4AWBXY_WINL[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L5_X_T, CAM_AWB_D_L5_X, AWB_L5_X_UP, rAWBStatConfig.i4AWBXY_WINR[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L5_Y_T, CAM_AWB_D_L5_Y, AWB_L5_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[5])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L5_Y_T, CAM_AWB_D_L5_Y, AWB_L5_Y_UP, rAWBStatConfig.i4AWBXY_WINU[5])

        // CAM_AWB_D_L6
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L6_X_T, CAM_AWB_D_L6_X, AWB_L6_X_LOW, rAWBStatConfig.i4AWBXY_WINL[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L6_X_T, CAM_AWB_D_L6_X, AWB_L6_X_UP, rAWBStatConfig.i4AWBXY_WINR[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L6_Y_T, CAM_AWB_D_L6_Y, AWB_L6_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[6])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L6_Y_T, CAM_AWB_D_L6_Y, AWB_L6_Y_UP, rAWBStatConfig.i4AWBXY_WINU[6])

        // CAM_AWB_D_L7
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L7_X_T, CAM_AWB_D_L7_X, AWB_L7_X_LOW, rAWBStatConfig.i4AWBXY_WINL[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L7_X_T, CAM_AWB_D_L7_X, AWB_L7_X_UP, rAWBStatConfig.i4AWBXY_WINR[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L7_Y_T, CAM_AWB_D_L7_Y, AWB_L7_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[7])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L7_Y_T, CAM_AWB_D_L7_Y, AWB_L7_Y_UP, rAWBStatConfig.i4AWBXY_WINU[7])

        // CAM_AWB_D_L8
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L8_X_T, CAM_AWB_D_L8_X, AWB_L8_X_LOW, rAWBStatConfig.i4AWBXY_WINL[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L8_X_T, CAM_AWB_D_L8_X, AWB_L8_X_UP, rAWBStatConfig.i4AWBXY_WINR[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L8_Y_T, CAM_AWB_D_L8_Y, AWB_L8_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[8])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L8_Y_T, CAM_AWB_D_L8_Y, AWB_L8_Y_UP, rAWBStatConfig.i4AWBXY_WINU[8])

        // CAM_AWB_D_L9
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L9_X_T, CAM_AWB_D_L9_X, AWB_L9_X_LOW, rAWBStatConfig.i4AWBXY_WINL[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L9_X_T, CAM_AWB_D_L9_X, AWB_L9_X_UP, rAWBStatConfig.i4AWBXY_WINR[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L9_Y_T, CAM_AWB_D_L9_Y, AWB_L9_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[9])
        AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_D_L9_Y_T, CAM_AWB_D_L9_Y, AWB_L9_Y_UP, rAWBStatConfig.i4AWBXY_WINU[9])
    }

    apply();

    addressErrorCheck("After ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply()
{
    if (m_eSensorTG == ESensorTG_1) {
        return apply_TG1();
    }
    else {
        return apply_TG2();
    }
}


MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply_TG1()
{
    MY_LOG_IF(IsDebugEnabled(), "%s(): m_eSensorDev = %d, m_i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIndex);
    dumpRegInfo("AWB_STAT_TG1");

    MUINTPTR handle;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIndex,"isp_mgr_awb_stat_tg1");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1"))))
    {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_ORG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_SIZE].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_PIT, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_PIT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_NUM].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_GAIN1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_GAIN1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_LMT1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_LMT1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LOW_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_LOW_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_HI_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_HI_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT0, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT1, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT2, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ERR_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_ERR_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ROT, m_rIspRegInfo[ERegInfo_CAM_AWB_ROT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L0_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L0_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L1_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L1_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L1_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L1_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L2_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L2_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L2_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L2_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L3_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L3_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L3_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L3_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L4_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L4_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L4_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L4_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L5_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L5_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L5_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L5_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L6_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L6_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L6_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L6_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L7_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L7_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L7_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L7_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L8_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L8_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L8_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L8_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L9_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L9_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L9_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L9_Y].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1")), MNULL))
    {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_awb_stat_tg1");

    return MTRUE;
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply_TG2()
{
    MY_LOG_IF(IsDebugEnabled(),"%s(): m_eSensorDev = %d, m_i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIndex);
    dumpRegInfo("AWB_STAT_TG2");

    MUINTPTR handle;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIndex,"isp_mgr_awb_stat_tg2");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB_D, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg2"))))
    {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_ORG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_SIZE].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_PIT, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_PIT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_NUM].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_GAIN1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_GAIN1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_GAIN1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_GAIN1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LMT1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LMT1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LMT1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LMT1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LOW_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LOW_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_HI_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_HI_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT2, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_ERR_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_ERR_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_ROT, m_rIspRegInfo[ERegInfo_CAM_AWB_D_ROT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L0_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L0_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L0_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L0_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L1_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L1_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L1_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L1_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L2_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L2_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L2_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L2_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L3_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L3_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L3_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L3_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L4_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L4_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L4_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L4_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L5_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L5_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L5_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L5_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L6_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L6_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L6_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L6_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L7_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L7_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L7_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L7_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L8_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L8_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L8_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L8_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L9_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L9_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L9_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L9_Y].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_awb_stat_tg2")), MNULL))
    {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_awb_stat_tg2");

    return MTRUE;
}

}
