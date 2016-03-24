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
#define LOG_TAG "isp_mgr_ae_stat"

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
// AE Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_STAT_CONFIG_T&
ISP_MGR_AE_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
config(MINT32 i4SensorIndex, AE_STAT_PARAM_T& rAEStatConfig)
{
    addressErrorCheck("Before ISP_MGR_AE_STAT_CONFIG_T::apply()");

    if (m_eSensorTG == ESensorTG_1) {
         // CAM_AE_HST_CTL
#if 1      // disable first for verify AAO statistic output
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
#else
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST0_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST1_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST2_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST3_EN = 0;
#endif
        // CAM_AE_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;
        // CAM_AE_LMT2_0
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_0))->AE_LIMIT2_R = 0xFFF;
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_0))->AE_LIMIT2_G = 0xFFF;
        // CAM_AE_LMT2_1
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_1))->AE_LIMIT2_B = 0xFFF;
        // CAM_AE_RC_CNV_0
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_0))->RC_CNV00 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_0))->RC_CNV01 = 0x000;
        // CAM_AE_RC_CNV_1
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_1))->RC_CNV02 = 0x000;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_1))->RC_CNV10 = 0x000;
        // CAM_AE_RC_CNV_2
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_2))->RC_CNV11 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_2))->RC_CNV12 = 0x000;
        // CAM_AE_RC_CNV_3
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_3))->RC_CNV20 = 0x000;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_3))->RC_CNV21 = 0x000;
        // CAM_AE_RC_CNV_4
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_4))->RC_CNV22 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_4))->AE_RC_ACC = 0x09;
        // CAM_AE_YGAMMA_0
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR1 = 0x10;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR2 = 0x20;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR3 = 0x40;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR4 = 0x60;
        // CAM_AE_YGAMMA_1
        reinterpret_cast<ISP_CAM_AE_YGAMMA_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_1))->Y_GMR5 = 0x80;
        // CAM_AE_HST_SET
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_BIN = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_BIN = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_BIN = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_BIN = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_COLOR = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_COLOR = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_COLOR = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_COLOR = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;
        // CAM_AE_HST0_RNG
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;
        // CAM_AE_HST1_RNG
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;
        // CAM_AE_HST2_RNG
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;
        // CAM_AE_HST3_RNG
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

        m_bTG1Init = MTRUE;
        apply_TG1(i4SensorIndex);
    } else {
         // CAM_AE_D_HST_CTL
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
        // CAM_AE_D_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_D_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;
        // CAM_AE_D_LMT2_0
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_0))->AE_LIMIT2_R = 0xFFF;
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_0))->AE_LIMIT2_G = 0xFFF;
        // CAM_AE_D_LMT2_1
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_1))->AE_LIMIT2_B = 0xFFF;
        // CAM_AE_D_RC_CNV_0
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_0))->RC_CNV00 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_0))->RC_CNV01 = 0x000;
        // CAM_AE_D_RC_CNV_1
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_1))->RC_CNV02 = 0x000;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_1))->RC_CNV10 = 0x000;
        // CAM_AE_D_RC_CNV_2
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_2))->RC_CNV11 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_2))->RC_CNV12 = 0x000;
        // CAM_AE_D_RC_CNV_3
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_3))->RC_CNV20 = 0x000;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_3))->RC_CNV21 = 0x000;
        // CAM_AE_D_RC_CNV_4
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_4))->RC_CNV22 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_4))->AE_RC_ACC = 0x09;
        // CAM_AE_D_YGAMMA_0
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR1 = 0x10;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR2 = 0x20;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR3 = 0x40;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR4 = 0x60;
        // CAM_AE_D_YGAMMA_1
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_1))->Y_GMR5 = 0x80;
        // CAM_AE_D_HST_SET
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST0_BIN = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST1_BIN = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST2_BIN = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST3_BIN = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST0_COLOR = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST1_COLOR = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST2_COLOR = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST3_COLOR = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;
        // CAM_AE_D_HST0_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_X_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_X_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_Y_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_Y_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;
        // CAM_AE_D_HST1_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_X_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_X_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_Y_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_Y_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;
        // CAM_AE_D_HST2_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_X_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_X_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_Y_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_Y_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;
        // CAM_AE_D_HST3_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_X_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_X_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_Y_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_Y_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

        m_bTG2Init = MTRUE;
        apply_TG2(i4SensorIndex);
    }

    addressErrorCheck("After ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply_TG1(MINT32 i4SensorIndex)
{
    MY_LOG("%s(): m_eSensorDev = %d, i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIndex);

    MUINTPTR handle;

    if(m_bTG1Init == MFALSE) {
        MY_LOG("%s(): AE don't configure TG1\n", __FUNCTION__);
        return MTRUE;
    }

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(i4SensorIndex,"isp_mgr_ae_stat_tg1");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AE, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_ae_stat_tg1")))) {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }
    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_CTL, m_rIspRegInfo[ERegInfo_CAM_AE_HST_CTL].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_0, m_rIspRegInfo[ERegInfo_CAM_AE_GAIN2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_1, m_rIspRegInfo[ERegInfo_CAM_AE_GAIN2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_LMT2_0, m_rIspRegInfo[ERegInfo_CAM_AE_LMT2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_LMT2_1, m_rIspRegInfo[ERegInfo_CAM_AE_LMT2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_0, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_1, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_2, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_3, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_3].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_4, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_4].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_0, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_1, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_SET, m_rIspRegInfo[ERegInfo_CAM_AE_HST_SET].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST0_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST0_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST1_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST1_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST2_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST2_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST3_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST3_RNG].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_ae_stat_tg1")), MNULL)) {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_ae_stat_tg1");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply_TG2(MINT32 i4SensorIndex)
{
    MY_LOG("%s(): m_eSensorDev = %d, i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIndex);

    MUINTPTR handle;

    if(m_bTG2Init == MFALSE) {
        MY_LOG("%s(): AE don't configure TG2\n", __FUNCTION__);
        return MTRUE;
    }

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(i4SensorIndex,"isp_mgr_ae_stat_tg2");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AE_D, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_ae_stat_tg2")))) {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST_CTL, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST_CTL].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_GAIN2_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_GAIN2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_GAIN2_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_GAIN2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_LMT2_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_LMT2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_LMT2_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_LMT2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_2, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_3, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_3].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_4, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_4].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_YGAMMA_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_YGAMMA_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_YGAMMA_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_YGAMMA_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST_SET, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST_SET].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST0_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST0_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST1_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST1_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST2_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST2_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST3_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST3_RNG].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_ae_stat_tg2")), MNULL)) {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_ae_stat_tg2");

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rIspAWBGain)
{
    m_rIspAEPreGain2 = rIspAWBGain;

    if (m_eSensorTG == ESensorTG_1) {
        // CAM_AE_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;

        apply_TG1(i4SensorIndex);
    } else {
            // CAM_AE_D_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_D_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;

        apply_TG2(i4SensorIndex);
    }

    return MTRUE;
}

}

