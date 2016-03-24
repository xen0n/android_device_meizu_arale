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
#define LOG_TAG "isp_mgr_sl2"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>

#include "isp_mgr.h"
#include <mtkcam/featureio/tuning_mgr.h>

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
put(ISP_NVRAM_SL2_T const& rParam)
{
    PUT_REG_INFO(CAM_SL2_CEN, cen);
    PUT_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    PUT_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    PUT_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);
    return  (*this);
}


template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2_T& rParam)
{
    GET_REG_INFO(CAM_SL2_CEN, cen);
    GET_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    GET_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    GET_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);
    return  (*this);
}

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

MBOOL
ISP_MGR_SL2_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_SL2_T::apply()");

    MBOOL bSL2_EN = isEnable();
    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    MINT32 i4Flg = 0;
    GET_PROP("debug.sl2.en", "7", i4Flg);
    MUINT32 u4SL2_EN = (bSL2_EN&&(i4Flg&1)) ? 1 : 0;
    MUINT32 u4SL2b_EN = (bSL2_EN&&(i4Flg&2)) ? 1 : 0;
    MUINT32 u4SL2c_EN = (bSL2_EN&&(i4Flg&4)) ? 1 : 0;

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_SL2(bSL2_EN);

    #if 1
    if (eIspProfile == EIspProfile_VFB_PostProc ||
            eIspProfile == EIspProfile_Capture_MultiPass_ANR_1 ||
            eIspProfile == EIspProfile_Capture_MultiPass_ANR_2 ||
            eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
            eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
            eIspProfile == EIspProfile_MFB_MultiPass_ANR_1          ||
            eIspProfile == EIspProfile_MFB_MultiPass_ANR_2          ||
            eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR    ||
            eIspProfile == EIspProfile_MFB_Blending_All_Off         ||
            eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR     ||
            eIspProfile == EIspProfile_MFB_PostProc_Mixing          ||
            eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1          ||
            eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2          ||
            eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR    ||
            eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off         ||
            eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR     ||
            eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
    )
    #else
    if (eIspProfile == EIspProfile_VFB_PostProc ||
        eIspProfile == EIspProfile_Capture_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_Capture_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_MFB_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_MFB_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR ||
        eIspProfile == EIspProfile_MFB_Blending_All_Off ||
        eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR ||
        eIspProfile == EIspProfile_MFB_PostProc_Mixing )
    #endif
    {
        u4SL2_EN = 0;
    }

    REG_INFO_VALUE(CAM_SL2_HRZ) = 2048;
    REG_INFO_VALUE(CAM_SL2_XOFF) = 0;
    REG_INFO_VALUE(CAM_SL2_YOFF) = 0;

    // TOP
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Sl2);
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Sl2b);
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Sl2c);

    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, SL2_EN, u4SL2_EN);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, SL2B_EN, u4SL2b_EN);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, SL2C_EN, u4SL2c_EN);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);

    MY_LOG_IF(1/*ENABLE_MY_LOG*/,
        "[%s] eSwScn(%d), bSL2_EN(%d), u4SL2_EN(%d), u4SL2b_EN(%d), u4SL2c_EN(%d), CAM_SL2_CEN(0x%08x), CAM_SL2_MAX0_RR(0x%08x), CAM_SL2_MAX1_RR(0x%08x), CAM_SL2_MAX2_RR(0x%08x)",
        __FUNCTION__,
        eSwScn, bSL2_EN, u4SL2_EN, u4SL2b_EN, u4SL2c_EN,
        REG_INFO_VALUE(CAM_SL2_CEN),
        REG_INFO_VALUE(CAM_SL2_MAX0_RR),
        REG_INFO_VALUE(CAM_SL2_MAX1_RR),
        REG_INFO_VALUE(CAM_SL2_MAX2_RR));

    dumpRegInfo("SL2");

    addressErrorCheck("After ISP_MGR_SL2_T::apply()");

    return  MTRUE;
}


}
