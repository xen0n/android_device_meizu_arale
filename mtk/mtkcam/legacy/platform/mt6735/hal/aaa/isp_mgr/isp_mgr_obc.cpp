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
#define LOG_TAG "isp_mgr_obc"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_OBC_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_OBC_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_OBC_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_OBC_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_OBC_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_OBC_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_OBC_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
put(ISP_NVRAM_OBC_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_OBC_OFFST0, offst0);
        PUT_REG_INFO(CAM_OBC_OFFST1, offst1);
        PUT_REG_INFO(CAM_OBC_OFFST2, offst2);
        PUT_REG_INFO(CAM_OBC_OFFST3, offst3);
        PUT_REG_INFO(CAM_OBC_GAIN0, gain0);
        PUT_REG_INFO(CAM_OBC_GAIN1, gain1);
        PUT_REG_INFO(CAM_OBC_GAIN2, gain2);
        PUT_REG_INFO(CAM_OBC_GAIN3, gain3);
    }
    else {
        PUT_REG_INFO(CAM_OBC_D_OFFST0, offst0);
        PUT_REG_INFO(CAM_OBC_D_OFFST1, offst1);
        PUT_REG_INFO(CAM_OBC_D_OFFST2, offst2);
        PUT_REG_INFO(CAM_OBC_D_OFFST3, offst3);
        PUT_REG_INFO(CAM_OBC_D_GAIN0, gain0);
        PUT_REG_INFO(CAM_OBC_D_GAIN1, gain1);
        PUT_REG_INFO(CAM_OBC_D_GAIN2, gain2);
        PUT_REG_INFO(CAM_OBC_D_GAIN3, gain3);
    }

    return  (*this);
}


template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
get(ISP_NVRAM_OBC_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_OBC_OFFST0, offst0);
        GET_REG_INFO(CAM_OBC_OFFST1, offst1);
        GET_REG_INFO(CAM_OBC_OFFST2, offst2);
        GET_REG_INFO(CAM_OBC_OFFST3, offst3);
        GET_REG_INFO(CAM_OBC_GAIN0, gain0);
        GET_REG_INFO(CAM_OBC_GAIN1, gain1);
        GET_REG_INFO(CAM_OBC_GAIN2, gain2);
        GET_REG_INFO(CAM_OBC_GAIN3, gain3);
    }
    else {
        GET_REG_INFO(CAM_OBC_D_OFFST0, offst0);
        GET_REG_INFO(CAM_OBC_D_OFFST1, offst1);
        GET_REG_INFO(CAM_OBC_D_OFFST2, offst2);
        GET_REG_INFO(CAM_OBC_D_OFFST3, offst3);
        GET_REG_INFO(CAM_OBC_D_GAIN0, gain0);
        GET_REG_INFO(CAM_OBC_D_GAIN1, gain1);
        GET_REG_INFO(CAM_OBC_D_GAIN2, gain2);
        GET_REG_INFO(CAM_OBC_D_GAIN3, gain3);
    }

    return  (*this);
}

MBOOL
ISP_MGR_OBC_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL bOBC_EN = isEnable();

    if (m_eSensorTG == ESensorTG_1) {
        // Merge AE gain
        MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBC_GAIN_B * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBC_GAIN_GR * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBC_GAIN_GB * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBC_GAIN_R * m_u4IspAEGain + 256) >> 9;

        reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBC_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBC_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBC_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBC_GAIN_R = CLAMP(OBGAIN3, 0, 8191);

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Obc);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1, OB_EN, bOBC_EN);

        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_OB(bOBC_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }
    else {
        // Merge AE gain
        MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN0))->OBC_D_GAIN_B * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN1))->OBC_D_GAIN_GR * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN2))->OBC_D_GAIN_GB * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN3))->OBC_D_GAIN_R * m_u4IspAEGain + 256) >> 9;

        reinterpret_cast<ISP_CAM_OBC_D_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN0))->OBC_D_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN1))->OBC_D_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN2))->OBC_D_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN3))->OBC_D_GAIN_R = CLAMP(OBGAIN3, 0, 8191);

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Obc_d);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1_D, OB_D_EN, bOBC_EN);

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_OB(bOBC_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }

    dumpRegInfo("OBC");

    return  MTRUE;
}

MBOOL
ISP_MGR_OBC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL bOBC_EN = isEnable();

    if (m_eSensorTG == ESensorTG_1) {
        // Merge AE gain
        MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBC_GAIN_B * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBC_GAIN_GR * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBC_GAIN_GB * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBC_GAIN_R * m_u4IspAEGain + 256) >> 9;

        reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBC_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBC_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBC_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBC_GAIN_R = CLAMP(OBGAIN3, 0, 8191);

        // TOP
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1, OB_EN, bOBC_EN);

        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_OB(bOBC_EN);

        // Register setting
        writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    }
    else {
        // Merge AE gain
        MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN0))->OBC_D_GAIN_B * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN1))->OBC_D_GAIN_GR * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN2))->OBC_D_GAIN_GB * m_u4IspAEGain + 256) >> 9;
        MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_D_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN3))->OBC_D_GAIN_R * m_u4IspAEGain + 256) >> 9;

        reinterpret_cast<ISP_CAM_OBC_D_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN0))->OBC_D_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN1))->OBC_D_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN2))->OBC_D_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
        reinterpret_cast<ISP_CAM_OBC_D_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_D_GAIN3))->OBC_D_GAIN_R = CLAMP(OBGAIN3, 0, 8191);

        // TOP
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1_D, OB_D_EN, bOBC_EN);

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_OB(bOBC_EN);

        // Register setting
        writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    }

    dumpRegInfo("OBC");

    return  MTRUE;
}

}
