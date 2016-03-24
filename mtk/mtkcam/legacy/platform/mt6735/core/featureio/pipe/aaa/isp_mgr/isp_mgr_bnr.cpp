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
#define LOG_TAG "isp_mgr_bnr"

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
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BPC_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_BPC_CON, con);
        PUT_REG_INFO(CAM_BPC_TH1, th1);
        PUT_REG_INFO(CAM_BPC_TH2, th2);
        PUT_REG_INFO(CAM_BPC_TH3, th3);
        PUT_REG_INFO(CAM_BPC_TH4, th4);
        PUT_REG_INFO(CAM_BPC_DTC, dtc);
        PUT_REG_INFO(CAM_BPC_COR, cor);
        PUT_REG_INFO(CAM_BPC_TBLI1, tbli1);
        PUT_REG_INFO(CAM_BPC_TBLI2, tbli2);
        PUT_REG_INFO(CAM_BPC_TH1_C, th1_c);
        PUT_REG_INFO(CAM_BPC_TH2_C, th2_c);
        PUT_REG_INFO(CAM_BPC_TH3_C, th3_c);
    }
    else {
        PUT_REG_INFO(CAM_BPC_D_CON, con);
        PUT_REG_INFO(CAM_BPC_D_TH1, th1);
        PUT_REG_INFO(CAM_BPC_D_TH2, th2);
        PUT_REG_INFO(CAM_BPC_D_TH3, th3);
        PUT_REG_INFO(CAM_BPC_D_TH4, th4);
        PUT_REG_INFO(CAM_BPC_D_DTC, dtc);
        PUT_REG_INFO(CAM_BPC_D_COR, cor);
        PUT_REG_INFO(CAM_BPC_D_TBLI1, tbli1);
        PUT_REG_INFO(CAM_BPC_D_TBLI2, tbli2);
        PUT_REG_INFO(CAM_BPC_D_TH1_C, th1_c);
        PUT_REG_INFO(CAM_BPC_D_TH2_C, th2_c);
        PUT_REG_INFO(CAM_BPC_D_TH3_C, th3_c);
    }

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BPC_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_BPC_CON, con);
        GET_REG_INFO(CAM_BPC_TH1, th1);
        GET_REG_INFO(CAM_BPC_TH2, th2);
        GET_REG_INFO(CAM_BPC_TH3, th3);
        GET_REG_INFO(CAM_BPC_TH4, th4);
        GET_REG_INFO(CAM_BPC_DTC, dtc);
        GET_REG_INFO(CAM_BPC_COR, cor);
        GET_REG_INFO(CAM_BPC_TBLI1, tbli1);
        GET_REG_INFO(CAM_BPC_TBLI2, tbli2);
        GET_REG_INFO(CAM_BPC_TH1_C, th1_c);
        GET_REG_INFO(CAM_BPC_TH2_C, th2_c);
        GET_REG_INFO(CAM_BPC_TH3_C, th3_c);
    }
    else {
        GET_REG_INFO(CAM_BPC_D_CON, con);
        GET_REG_INFO(CAM_BPC_D_TH1, th1);
        GET_REG_INFO(CAM_BPC_D_TH2, th2);
        GET_REG_INFO(CAM_BPC_D_TH3, th3);
        GET_REG_INFO(CAM_BPC_D_TH4, th4);
        GET_REG_INFO(CAM_BPC_D_DTC, dtc);
        GET_REG_INFO(CAM_BPC_D_COR, cor);
        GET_REG_INFO(CAM_BPC_D_TBLI1, tbli1);
        GET_REG_INFO(CAM_BPC_D_TBLI2, tbli2);
        GET_REG_INFO(CAM_BPC_D_TH1_C, th1_c);
        GET_REG_INFO(CAM_BPC_D_TH2_C, th2_c);
        GET_REG_INFO(CAM_BPC_D_TH3_C, th3_c);
    }

    return  (*this);
}

#if 0   //RMM don't use NVRAM
template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BPC_RMM_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_BPC_RMM1, rmm1);
        PUT_REG_INFO(CAM_BPC_RMM2, rmm2);
        PUT_REG_INFO(CAM_BPC_RMM_REVG_1, rmm_revg_1);
        PUT_REG_INFO(CAM_BPC_RMM_REVG_2, rmm_revg_2);
        PUT_REG_INFO(CAM_BPC_RMM_LEOS, rmm_leos);
        PUT_REG_INFO(CAM_BPC_RMM_GCNT, rmm_gcnt);
    }
    else {
        PUT_REG_INFO(CAM_BPC_D_RMM1, rmm1);
        PUT_REG_INFO(CAM_BPC_D_RMM2, rmm2);
        PUT_REG_INFO(CAM_BPC_D_RMM_REVG_1, rmm_revg_1);
        PUT_REG_INFO(CAM_BPC_D_RMM_REVG_2, rmm_revg_2);
        PUT_REG_INFO(CAM_BPC_D_RMM_LEOS, rmm_leos);
        PUT_REG_INFO(CAM_BPC_D_RMM_GCNT, rmm_gcnt);
    }


    return  (*this);
}
#endif

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BPC_RMM_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_BPC_RMM1, rmm1);
        GET_REG_INFO(CAM_BPC_RMM2, rmm2);
        GET_REG_INFO(CAM_BPC_RMM_REVG_1, rmm_revg_1);
        GET_REG_INFO(CAM_BPC_RMM_REVG_2, rmm_revg_2);
        GET_REG_INFO(CAM_BPC_RMM_LEOS, rmm_leos);
        GET_REG_INFO(CAM_BPC_RMM_GCNT, rmm_gcnt);
    }
    else {
        GET_REG_INFO(CAM_BPC_D_RMM1, rmm1);
        GET_REG_INFO(CAM_BPC_D_RMM2, rmm2);
        GET_REG_INFO(CAM_BPC_D_RMM_REVG_1, rmm_revg_1);
        GET_REG_INFO(CAM_BPC_D_RMM_REVG_2, rmm_revg_2);
        GET_REG_INFO(CAM_BPC_D_RMM_LEOS, rmm_leos);
        GET_REG_INFO(CAM_BPC_D_RMM_GCNT, rmm_gcnt);
    }

    return  (*this);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_NR1_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_NR1_CON, con);
        PUT_REG_INFO(CAM_NR1_CT_CON, ct_con);
    }
    else {
        PUT_REG_INFO(CAM_NR1_D_CON, con);
        PUT_REG_INFO(CAM_NR1_D_CT_CON, ct_con);
    }

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_NR1_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_NR1_CON, con);
        GET_REG_INFO(CAM_NR1_CT_CON, ct_con);
    }
    else {
        GET_REG_INFO(CAM_NR1_D_CON, con);
        GET_REG_INFO(CAM_NR1_D_CT_CON, ct_con);
    }

    return  (*this);
}

MVOID
ISP_MGR_BNR_T::
updateRMM()
{
    //====== Dynamic Debug ======

    MUINT32 rmmDebug = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    rmmDebug = atoi(value);

    //====== Prepare Register Value ======

    if(m_u4AeExpRatio < 1.0)
    {
        MY_LOG("m_u4AeExpRatio < 1.0");
        m_u4AeExpRatio = 1.0;
    }
    MUINT32 ihdrGain = 0.5 + (4 * 64 / m_u4AeExpRatio);    //m_u4AeExpRatio  = LE/SE

    MUINT32 rmm1 = 0x114000;  //default : LEBLD=1(fixed),SEBLD=1(fixed),SEDIR_SL=4(fixed)
    MUINT32 rmm2 = 0x00241;  //default : HORI_ADDWT=31(fixed),MOTH_RATIO=2(fixed),MOBLD_FT=4(fixed),MO_MODE=1(fixed)
    MUINT32 rmmOscTh = 1020 * ihdrGain / 64;
    MUINT32 rmmRevg1 = 0 | (m_i4RMM_Ggain << 16) | m_i4RMM_Rgain;
    MUINT32 rmmRevg2 = 0 | (m_i4RMM_Ggain << 16) | m_i4RMM_Bgain;
    MUINT32 rmmLeosGray = ((1023 * ihdrGain) + 32) >> 6;

    MY_LOG_IF(rmmDebug, "rmm1(0x%08x),rmm2(0x%08x)",rmm1,rmm2);
    MY_LOG_IF(rmmDebug, "RMM_Gain:(R,G,B)=(0x%08x,0x%08x,0x%08x)",m_i4RMM_Rgain,m_i4RMM_Ggain,m_i4RMM_Bgain);
    MY_LOG_IF(rmmDebug, "rmmRevg1(0x%08x),rmmRevg2(0x%08x)",rmmRevg1,rmmRevg2);
    MY_LOG_IF(rmmDebug, "rmmOscTh(0x%08x),rmmLeosGray(0x%08x)",rmmOscTh,rmmLeosGray);
    MY_LOG_IF(rmmDebug, "aeExpRatio(%f),ihdrGain(%u)",m_u4AeExpRatio,ihdrGain);

    //====== Update Register Value ======

    if(m_eSensorTG == ESensorTG_1)
    {
        m_rIspRegInfo[ERegInfo_CAM_BPC_RMM1].val       = (rmm1 & 0x777000) | rmmOscTh;
        m_rIspRegInfo[ERegInfo_CAM_BPC_RMM2].val       = rmm2;
        m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_REVG_1].val = rmmRevg1;
        m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_REVG_2].val = rmmRevg2;
        m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_LEOS].val   = rmmLeosGray;

        MY_LOG_IF(rmmDebug, "CAM_BPC_RMM1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_RMM1].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_RMM2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_RMM2].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_RMM_REVG_1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_REVG_1].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_RMM_REVG_2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_REVG_2].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_RMM_LEOS(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_RMM_LEOS].val);
    }
    else if(m_eSensorTG == ESensorTG_2)
    {
        m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM1].val       = (rmm1 & 0x777000) | rmmOscTh;
        m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM2].val       = rmm2;
        m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_REVG_1].val = rmmRevg1;
        m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_REVG_2].val = rmmRevg2;
        m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_LEOS].val   = rmmLeosGray;

        MY_LOG_IF(rmmDebug, "CAM_BPC_D_RMM1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM1].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_D_RMM2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM2].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_D_RMM_REVG_1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_REVG_1].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_D_RMM_REVG_2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_REVG_2].val);
        MY_LOG_IF(rmmDebug, "CAM_BPC_D_RMM_LEOS(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_BPC_D_RMM_LEOS].val);
    }
}


MBOOL
ISP_MGR_BNR_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_BNR_T::apply()");

    if (m_eSensorTG == ESensorTG_1) {
        MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_EN & isBPCEnable();
        MBOOL bCT_EN = reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN & isCTEnable();
        MBOOL bBNR_EN = bBPC_EN | bCT_EN;
        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);

        reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_EN = bBPC_EN;
        reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN = bCT_EN;



        //update RMM
        updateRMM();

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Bnr);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1, BNR_EN, bBNR_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }
    else {
        MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BPC_D_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_D_CON))->BPC_EN & isBPCEnable();
        MBOOL bCT_EN = reinterpret_cast<ISP_CAM_NR1_D_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_D_CON))->NR1_CT_EN & isCTEnable();
        MBOOL bBNR_EN = bBPC_EN | bCT_EN;

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);
        reinterpret_cast<ISP_CAM_BPC_D_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_D_CON))->BPC_EN = bBPC_EN;
        reinterpret_cast<ISP_CAM_NR1_D_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_D_CON))->NR1_CT_EN = bCT_EN;

        //update RMM
        updateRMM();

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Bnr_d);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1_D, BNR_D_EN, bBNR_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }

    dumpRegInfo("BNR");

    addressErrorCheck("After ISP_MGR_BNR_T::apply()");

    return  MTRUE;
}

}

