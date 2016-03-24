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
#define LOG_TAG "isp_mgr_g2c"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_G2C_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
put(ISP_NVRAM_G2C_T const& rParam)
{
    PUT_REG_INFO(CAM_G2C_CONV_0A, conv_0a);
    PUT_REG_INFO(CAM_G2C_CONV_0B, conv_0b);
    PUT_REG_INFO(CAM_G2C_CONV_1A, conv_1a);
    PUT_REG_INFO(CAM_G2C_CONV_1B, conv_1b);
    PUT_REG_INFO(CAM_G2C_CONV_2A, conv_2a);
    PUT_REG_INFO(CAM_G2C_CONV_2B, conv_2b);

    return  (*this);
}

template <>
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
get(ISP_NVRAM_G2C_T& rParam)
{
    GET_REG_INFO(CAM_G2C_CONV_0A, conv_0a);
    GET_REG_INFO(CAM_G2C_CONV_0B, conv_0b);
    GET_REG_INFO(CAM_G2C_CONV_1A, conv_1a);
    GET_REG_INFO(CAM_G2C_CONV_1B, conv_1b);
    GET_REG_INFO(CAM_G2C_CONV_2A, conv_2a);
    GET_REG_INFO(CAM_G2C_CONV_2B, conv_2b);

    return  (*this);
}

MBOOL
ISP_MGR_G2C_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_G2C_T::apply()");

    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV_00 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV_01 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_CNV_02 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_Y_OFST = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV_10 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV_11 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_CNV_12 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_U_OFST = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV_20 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV_21 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_CNV_22 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_V_OFST = 0;
    }

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_G2C(isEnable());

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_G2c_Conv);

    // TOP
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, G2C_EN, isEnable());

    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    addressErrorCheck("After ISP_MGR_G2C_T::apply()");

    dumpRegInfo("G2C");

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C_SHADE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_G2C_SHADE_T&
ISP_MGR_G2C_SHADE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_G2C_SHADE_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_G2C_SHADE_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_G2C_SHADE_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_G2C_SHADE_DEV<ESensorDev_Main>::getInstance();
    }
}


template <>
ISP_MGR_G2C_SHADE_T&
ISP_MGR_G2C_SHADE_T::
put(ISP_NVRAM_G2C_SHADE_T const& rParam)
{
    PUT_REG_INFO(CAM_G2C_SHADE_CON_1, con_1);
    PUT_REG_INFO(CAM_G2C_SHADE_CON_2, con_2);
    PUT_REG_INFO(CAM_G2C_SHADE_CON_3, con_3);
    PUT_REG_INFO(CAM_G2C_SHADE_TAR, tar);
    PUT_REG_INFO(CAM_G2C_SHADE_SP, sp);

    return  (*this);
}

template <>
ISP_MGR_G2C_SHADE_T&
ISP_MGR_G2C_SHADE_T::
get(ISP_NVRAM_G2C_SHADE_T& rParam)
{
    GET_REG_INFO(CAM_G2C_SHADE_CON_1, con_1);
    GET_REG_INFO(CAM_G2C_SHADE_CON_2, con_2);
    GET_REG_INFO(CAM_G2C_SHADE_CON_3, con_3);
    GET_REG_INFO(CAM_G2C_SHADE_TAR, tar);
    GET_REG_INFO(CAM_G2C_SHADE_SP, sp);

    return  (*this);
}

MBOOL
ISP_MGR_G2C_SHADE_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_G2C_SHADE_T::apply()");

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_G2c_Shade);

    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    addressErrorCheck("After ISP_MGR_G2C_SHADE_T::apply()");

    dumpRegInfo("G2C_SHADE");

    return MTRUE;
}


}
