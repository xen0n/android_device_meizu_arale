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
#define LOG_TAG "isp_mgr_cfa"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CFA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CFA_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
put(ISP_NVRAM_CFA_T const& rParam)
{
    PUT_REG_INFO(CAM_DM_O_BYP, byp);
    PUT_REG_INFO(CAM_DM_O_ED_FLAT, ed_flat);
    PUT_REG_INFO(CAM_DM_O_ED_NYQ, ed_nyq);
    PUT_REG_INFO(CAM_DM_O_ED_STEP, ed_step);
    PUT_REG_INFO(CAM_DM_O_RGB_HF, rgb_hf);
    PUT_REG_INFO(CAM_DM_O_DOT, dot);
    PUT_REG_INFO(CAM_DM_O_F1_ACT, f1_act);
    PUT_REG_INFO(CAM_DM_O_F2_ACT, f2_act);
    PUT_REG_INFO(CAM_DM_O_F3_ACT, f3_act);
    PUT_REG_INFO(CAM_DM_O_F4_ACT, f4_act);
    PUT_REG_INFO(CAM_DM_O_F1_L, f1_l);
    PUT_REG_INFO(CAM_DM_O_F2_L, f2_l);
    PUT_REG_INFO(CAM_DM_O_F3_L, f3_l);
    PUT_REG_INFO(CAM_DM_O_F4_L, f4_l);
    PUT_REG_INFO(CAM_DM_O_HF_RB, hf_rb);
    PUT_REG_INFO(CAM_DM_O_HF_GAIN, hf_gain);
    PUT_REG_INFO(CAM_DM_O_HF_COMP, hf_comp);
    PUT_REG_INFO(CAM_DM_O_HF_CORIN_TH, hf_coring_th);
    PUT_REG_INFO(CAM_DM_O_ACT_LUT, act_lut);
    PUT_REG_INFO(CAM_DM_O_SPARE, spare);
    PUT_REG_INFO(CAM_DM_O_BB, bb);

    return  (*this);
}


template <>
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
get(ISP_NVRAM_CFA_T& rParam)
{
    GET_REG_INFO(CAM_DM_O_BYP, byp);
    GET_REG_INFO(CAM_DM_O_ED_FLAT, ed_flat);
    GET_REG_INFO(CAM_DM_O_ED_NYQ, ed_nyq);
    GET_REG_INFO(CAM_DM_O_ED_STEP, ed_step);
    GET_REG_INFO(CAM_DM_O_RGB_HF, rgb_hf);
    GET_REG_INFO(CAM_DM_O_DOT, dot);
    GET_REG_INFO(CAM_DM_O_F1_ACT, f1_act);
    GET_REG_INFO(CAM_DM_O_F2_ACT, f2_act);
    GET_REG_INFO(CAM_DM_O_F3_ACT, f3_act);
    GET_REG_INFO(CAM_DM_O_F4_ACT, f4_act);
    GET_REG_INFO(CAM_DM_O_F1_L, f1_l);
    GET_REG_INFO(CAM_DM_O_F2_L, f2_l);
    GET_REG_INFO(CAM_DM_O_F3_L, f3_l);
    GET_REG_INFO(CAM_DM_O_F4_L, f4_l);
    GET_REG_INFO(CAM_DM_O_HF_RB, hf_rb);
    GET_REG_INFO(CAM_DM_O_HF_GAIN, hf_gain);
    GET_REG_INFO(CAM_DM_O_HF_COMP, hf_comp);
    GET_REG_INFO(CAM_DM_O_HF_CORIN_TH, hf_coring_th);
    GET_REG_INFO(CAM_DM_O_ACT_LUT, act_lut);
    GET_REG_INFO(CAM_DM_O_SPARE, spare);
    GET_REG_INFO(CAM_DM_O_BB, bb);

    return  (*this);
}

MBOOL
ISP_MGR_CFA_T::
apply(EIspProfile_T eIspProfile)
{

    addressErrorCheck("Before ISP_MGR_CFA_T::apply()");

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_Cfa);

    // TOP ==> don't care
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, CFA_EN, 1);

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_CFA(MTRUE);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    addressErrorCheck("After ISP_MGR_CFA_T::apply()");

    dumpRegInfo("CFA");

    return  MTRUE;
}


}
