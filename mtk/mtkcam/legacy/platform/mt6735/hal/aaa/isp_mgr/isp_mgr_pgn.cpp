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
#define LOG_TAG "isp_mgr_pgn"

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_PGN_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_PGN_T::
setIspPregain()
{
    // ISP pregain
    m_rIspPregain.i4R = (m_rIspAWBGain.i4R * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4G = (m_rIspAWBGain.i4G * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4B = (m_rIspAWBGain.i4B * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG_IF(IsDebugEnabled(), "m_i4FlareGain = %d\n", m_i4FlareGain);
    MY_LOG_IF(IsDebugEnabled(), "m_rIspPregain.i4R = %d; m_rIspAWBGain.i4R = %d\n", m_rIspPregain.i4R , m_rIspAWBGain.i4R);
    MY_LOG_IF(IsDebugEnabled(), "m_rIspPregain.i4G = %d; m_rIspAWBGain.i4G = %d\n", m_rIspPregain.i4G , m_rIspAWBGain.i4G);
    MY_LOG_IF(IsDebugEnabled(), "m_rIspPregain.i4B = %d; m_rIspAWBGain.i4B = %d\n", m_rIspPregain.i4B , m_rIspAWBGain.i4B);

    // ISP flare offset
    m_i4IspFlareOffset = (m_i4FlareOffset * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG_IF(IsDebugEnabled(), "m_i4IspFlareOffset = %d; m_i4FlareOffset = %d\n", m_i4IspFlareOffset , m_i4FlareOffset);

    if ((m_rIspPregain.i4R == 0) ||
        (m_rIspPregain.i4G == 0) ||
        (m_rIspPregain.i4B == 0)) {
        MY_ERR("setIspPregain(): R = %d, G = %d, B = %d\n", m_rIspPregain.i4R, m_rIspPregain.i4G, m_rIspPregain.i4B);
        return MFALSE;
    }

    // CAM_PGN_GAIN_1
    reinterpret_cast<ISP_CAM_PGN_GAIN_1_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN_1))->PGN_GAIN_B = m_rIspPregain.i4B;
    reinterpret_cast<ISP_CAM_PGN_GAIN_1_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN_1))->PGN_GAIN_GB = m_rIspPregain.i4G;

    // CAM_PGN_GAIN_2
    reinterpret_cast<ISP_CAM_PGN_GAIN_2_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN_2))->PGN_GAIN_GR = m_rIspPregain.i4G;
    reinterpret_cast<ISP_CAM_PGN_GAIN_2_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN_2))->PGN_GAIN_R = m_rIspPregain.i4R;

    // CAM_PGN_OFST_1
    MUINT32 OFFS = (m_i4IspFlareOffset >= 0) ? static_cast<MUINT32>(m_i4IspFlareOffset) : static_cast<MUINT32>(4096 + m_i4IspFlareOffset);
    reinterpret_cast<ISP_CAM_PGN_OFST_1_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFST_1))->PGN_OFST_B = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFST_1_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFST_1))->PGN_OFST_GB = OFFS;

    // CAM_PGN_OFST_2
    reinterpret_cast<ISP_CAM_PGN_OFST_2_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFST_2))->PGN_OFST_GR = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFST_2_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFST_2))->PGN_OFST_R = OFFS;

    return MTRUE;
}

template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
put(ISP_NVRAM_PGN_T const& rParam)
{
    PUT_REG_INFO(CAM_PGN_SATU_1, satu_1);
    PUT_REG_INFO(CAM_PGN_SATU_2, satu_2);
    PUT_REG_INFO(CAM_PGN_GAIN_1, gain_1);
    PUT_REG_INFO(CAM_PGN_GAIN_2, gain_2);
    PUT_REG_INFO(CAM_PGN_OFST_1, ofst_1);
    PUT_REG_INFO(CAM_PGN_OFST_2, ofst_2);

    return  (*this);
}


template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
get(ISP_NVRAM_PGN_T& rParam)
{
    GET_REG_INFO(CAM_PGN_SATU_1, satu_1);
    GET_REG_INFO(CAM_PGN_SATU_2, satu_2);
    GET_REG_INFO(CAM_PGN_GAIN_1, gain_1);
    GET_REG_INFO(CAM_PGN_GAIN_2, gain_2);
    GET_REG_INFO(CAM_PGN_OFST_1, ofst_1);
    GET_REG_INFO(CAM_PGN_OFST_2, ofst_2);

    return  (*this);
}


MBOOL
ISP_MGR_PGN_T::
apply(EIspProfile_T eIspProfile)
{
    MY_LOG_IF(IsDebugEnabled(),"%s(): isEnable() = %d\n", __FUNCTION__, isEnable());

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_Pgn);

    // TOP
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, PGN_EN, isEnable());

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_PGN(isEnable());

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    dumpRegInfo("PGN");

    return  MTRUE;
}

MBOOL
ISP_MGR_PGN_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MY_LOG_IF(IsDebugEnabled(),"%s(): isEnable() = %d\n", __FUNCTION__, isEnable());

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P2, PGN_EN, isEnable());

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_PGN(isEnable());

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("PGN");

    return  MTRUE;
}

}
