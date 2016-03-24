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
#define LOG_TAG "isp_mgr"

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
//#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/imageio/ispio_stddef.h>

#include "isp_mgr.h"
//#include <mtkcam/featureio/tdri_mgr.h>
#include <mtkcam/featureio/tuning_mgr.h>

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define ISP_WRITE_ENABLE_BITS(RegBase, RegName, FieldName, Value)   \
    do {                                                            \
        (RegBase->RegName.Bits.FieldName) = (Value);                \
    } while (0)


#define ISP_WRITE_ENABLE_REG(RegBase, RegName, Value)   \
    do {                                                \
        (RegBase->RegName.Raw) = (Value);               \
    } while (0)

using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define ISP_MGR_LOG_BIT  (1<<0)
#define ISP_MGR_EN_BIT1  (1<<1)
#define ISP_MGR_EN_BIT2  (1<<2)


namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Manager Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_BASE::
dumpRegInfo()
{
    RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

    for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
        MY_LOG("[addr] 0x%8x [value] 0x%8x\n", pRegInfo[i].addr, pRegInfo[i].val);
    }
}

MVOID
ISP_MGR_BASE::
dumpRegs()
{
    RegInfo_T RegInfo[m_u4RegInfoNum];

    readRegs(RegInfo, m_u4RegInfoNum);

    for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
        MY_LOG("[addr] 0x%8x [value] 0x%8x\n", RegInfo[i].addr, RegInfo[i].val);
    }
}

MVOID
ISP_MGR_BASE::
addressErrorCheck(char const*const ptestCastName)
{
    RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

    if (m_u4StartAddr != pRegInfo[0].addr) {
        MY_ERR("[%s] Start address check error: (m_u4StartAddr, pRegInfo[0].addr) = (0x%8x, 0x%8x)", ptestCastName, m_u4StartAddr, pRegInfo[0].addr);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CTL_EN_T&
ISP_MGR_CTL_EN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CTL_EN_T&
ISP_MGR_CTL_EN_T::
get(ISP_NVRAM_CTL_EN_T& rParam)
{
    GET_REG_INFO(CAM_CTL_EN1, en1);
    GET_REG_INFO(CAM_CTL_EN2, en2);

    return  (*this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_OBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
put(ISP_NVRAM_OBC_T const& rParam)
{
    PUT_REG_INFO(CAM_OBC_OFFST0, offst0);
    PUT_REG_INFO(CAM_OBC_OFFST1, offst1);
    PUT_REG_INFO(CAM_OBC_OFFST2, offst2);
    PUT_REG_INFO(CAM_OBC_OFFST3, offst3);
    PUT_REG_INFO(CAM_OBC_GAIN0, gain0);
    PUT_REG_INFO(CAM_OBC_GAIN1, gain1);
    PUT_REG_INFO(CAM_OBC_GAIN2, gain2);
    PUT_REG_INFO(CAM_OBC_GAIN3, gain3);
    return  (*this);
}


template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
get(ISP_NVRAM_OBC_T& rParam)
{
    GET_REG_INFO(CAM_OBC_OFFST0, offst0);
    GET_REG_INFO(CAM_OBC_OFFST1, offst1);
    GET_REG_INFO(CAM_OBC_OFFST2, offst2);
    GET_REG_INFO(CAM_OBC_OFFST3, offst3);
    GET_REG_INFO(CAM_OBC_GAIN0, gain0);
    GET_REG_INFO(CAM_OBC_GAIN1, gain1);
    GET_REG_INFO(CAM_OBC_GAIN2, gain2);
    GET_REG_INFO(CAM_OBC_GAIN3, gain3);
    return  (*this);
}

MBOOL
ISP_MGR_OBC_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL bOBC_EN = isEnable();

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.obc_apply.log", value, "0");
    MINT32 logEn = atoi(value);


    // Merge AE gain
    MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 * m_u4IspAEGain + 256) >> 9;

    reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 = CLAMP(OBGAIN0, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 = CLAMP(OBGAIN1, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 = CLAMP(OBGAIN2, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 = CLAMP(OBGAIN3, 0, 8191);

    MY_LOG_IF(logEn, "[ISP_MGR_OBC_T] OBGAIN 0-3 (%d,%d,%d,%d), offset 0-3 (%d,%d,%d,%d)"
        , reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0
        , reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1
        , reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2
        , reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3
        , reinterpret_cast<ISP_NVRAM_OBC_OFFST0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_OFFST0))->val
        , reinterpret_cast<ISP_NVRAM_OBC_OFFST1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_OFFST1))->val
        , reinterpret_cast<ISP_NVRAM_OBC_OFFST2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_OFFST2))->val
        , reinterpret_cast<ISP_NVRAM_OBC_OFFST3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_OFFST3))->val
        );

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                          eTuningMgrFunc_Obc);
    // TOP
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN1, OB_EN, bOBC_EN);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    return  MTRUE;
}

MBOOL
ISP_MGR_OBC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL bOBC_EN = isEnable();

    // Merge AE gain
    MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 * m_u4IspAEGain + 256) >> 9;

    reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 = CLAMP(OBGAIN0, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 = CLAMP(OBGAIN1, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 = CLAMP(OBGAIN2, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 = CLAMP(OBGAIN3, 0, 8191);

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, OB_EN, bOBC_EN);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BPC_T const& rParam)
{
    PUT_REG_INFO(CAM_BPC_CON, con);
    PUT_REG_INFO(CAM_BPC_CD1_1, cd1_1);
    PUT_REG_INFO(CAM_BPC_CD1_2, cd1_2);
    PUT_REG_INFO(CAM_BPC_CD1_3, cd1_3);
    PUT_REG_INFO(CAM_BPC_CD1_4, cd1_4);
    PUT_REG_INFO(CAM_BPC_CD1_5, cd1_5);
    PUT_REG_INFO(CAM_BPC_CD1_6, cd1_6);
    PUT_REG_INFO(CAM_BPC_CD2_1, cd2_1);
    PUT_REG_INFO(CAM_BPC_CD2_2, cd2_2);
    PUT_REG_INFO(CAM_BPC_CD2_3, cd2_3);
    PUT_REG_INFO(CAM_BPC_CD0, cd0);
    PUT_REG_INFO(CAM_BPC_COR, cor);
    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BPC_T& rParam)
{
    GET_REG_INFO(CAM_BPC_CON, con);
    GET_REG_INFO(CAM_BPC_CD1_1, cd1_1);
    GET_REG_INFO(CAM_BPC_CD1_2, cd1_2);
    GET_REG_INFO(CAM_BPC_CD1_3, cd1_3);
    GET_REG_INFO(CAM_BPC_CD1_4, cd1_4);
    GET_REG_INFO(CAM_BPC_CD1_5, cd1_5);
    GET_REG_INFO(CAM_BPC_CD1_6, cd1_6);
    GET_REG_INFO(CAM_BPC_CD2_1, cd2_1);
    GET_REG_INFO(CAM_BPC_CD2_2, cd2_2);
    GET_REG_INFO(CAM_BPC_CD2_3, cd2_3);
    GET_REG_INFO(CAM_BPC_CD0, cd0);
    GET_REG_INFO(CAM_BPC_COR, cor);

    return  (*this);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_NR1_T const& rParam)
{
    PUT_REG_INFO(CAM_NR1_CON, con);
    PUT_REG_INFO(CAM_NR1_CT_CON, ct_con);

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_NR1_T& rParam)
{
    GET_REG_INFO(CAM_NR1_CON, con);
    GET_REG_INFO(CAM_NR1_CT_CON, ct_con);

    return  (*this);
}

MBOOL
ISP_MGR_BNR_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE & isBPCEnable();
    MBOOL bCT_EN = reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN & isCTEnable();
    MBOOL bBNR_EN = bBPC_EN | bCT_EN;

    reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE = bBPC_EN;
    reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN = bCT_EN;

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_Bnr);

    // TOP
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN1, BNR_EN, bBNR_EN);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    return  MTRUE;
}

MBOOL
ISP_MGR_BNR_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE & isBPCEnable();
    MBOOL bCT_EN = reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN & isCTEnable();
    MBOOL bBNR_EN = bBPC_EN | bCT_EN;

    reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE = bBPC_EN;
    reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN = bCT_EN;

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, BNR_EN, bBNR_EN);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return ISP_MGR_LSC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return ISP_MGR_LSC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return ISP_MGR_LSC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return ISP_MGR_LSC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
put(ISP_NVRAM_LSC_T const& rParam)
{
    MY_LOG_IF(0,"%s\n", __FUNCTION__);
    PUT_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    PUT_REG_INFO(CAM_LSCI_XSIZE, xsize);
    PUT_REG_INFO(CAM_LSC_CTL1, ctl1);
    PUT_REG_INFO(CAM_LSC_CTL2, ctl2);
    PUT_REG_INFO(CAM_LSC_CTL3, ctl3);
    PUT_REG_INFO(CAM_LSC_LBLOCK, lblock);
    PUT_REG_INFO(CAM_LSC_RATIO, ratio);
    REG_INFO_VALUE(CAM_LSC_GAIN_TH) = 0x1FF7FDFF;

    return  (*this);
}


template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(ISP_NVRAM_LSC_T& rParam)
{
    MY_LOG_IF(0,"%s\n", __FUNCTION__);
    GET_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    GET_REG_INFO(CAM_LSCI_XSIZE, xsize);
    GET_REG_INFO(CAM_LSC_CTL1, ctl1);
    GET_REG_INFO(CAM_LSC_CTL2, ctl2);
    GET_REG_INFO(CAM_LSC_CTL3, ctl3);
    GET_REG_INFO(CAM_LSC_LBLOCK, lblock);
    GET_REG_INFO(CAM_LSC_RATIO, ratio);
    GET_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(ISP_NVRAM_LSC_CFG_T& rParam)
{
    MY_LOG_IF(0,"%s\n", __FUNCTION__);
    GET_REG_INFO(CAM_LSC_CTL1, ctl1);
    GET_REG_INFO(CAM_LSC_CTL2, ctl2);
    GET_REG_INFO(CAM_LSC_CTL3, ctl3);
    GET_REG_INFO(CAM_LSC_LBLOCK, lblock);
    GET_REG_INFO(CAM_LSC_RATIO, ratio);
    GET_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}

MBOOL
ISP_MGR_LSC_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL fgOnOff = m_fgOnOff;

    MUINT32 u4XNum, u4YNum;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.lsc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    u4XNum = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM;
    u4YNum = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM;

    LSCI_XSIZE = (u4XNum+1)*4*128/8 - 1;
    LSCI_YSIZE = u4YNum;
    LSCI_STRIDE = (LSCI_XSIZE+1);

    REG_INFO_VALUE(CAM_LSCI_OFST_ADDR)  = 0;
    REG_INFO_VALUE(CAM_LSCI_XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE(CAM_LSCI_YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE(CAM_LSCI_STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE(CAM_LSC_CTL1)        = 0x30000000; //0x30000000;
    REG_INFO_VALUE(CAM_LSC_GAIN_TH)     = 0x1FF7FDFF;

    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    // TOP
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Lsc);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN1, LSC_EN, fgOnOff);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_DMA_EN, LSCI_EN, fgOnOff);

    //ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(
        eSwScn,
        static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
        m_u4RegInfoNum);

    MY_LOG_IF(logEn, "[%s] eSwScn(%d), fgOnOff(%d)", __FUNCTION__, eSwScn, fgOnOff);
    if (logEn) dumpRegInfo();

    return  MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    // for p2 ip raw
    MBOOL fgOnOff = m_fgOnOff;

    MUINT32 u4XNum, u4YNum;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.lsc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    u4XNum = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM;
    u4YNum = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM;

    LSCI_XSIZE = (u4XNum+1)*4*128/8 - 1;
    LSCI_YSIZE = u4YNum;
    LSCI_STRIDE = (LSCI_XSIZE+1);

    REG_INFO_VALUE(CAM_LSCI_OFST_ADDR)  = 0;
    REG_INFO_VALUE(CAM_LSCI_XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE(CAM_LSCI_YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE(CAM_LSCI_STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE(CAM_LSC_CTL1)        = 0x30000000; //0x30000000;
    REG_INFO_VALUE(CAM_LSC_GAIN_TH)     = 0x1FF7FDFF;

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, LSC_EN, fgOnOff);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_DMA_EN, LSCI_EN, fgOnOff);

    MY_LOG_IF(logEn, "[%s] P2, fgOnOff(%d)", __FUNCTION__, fgOnOff);
    if (logEn) dumpRegInfo();

    return MTRUE;
}

MVOID
ISP_MGR_LSC_T::
enableLsc(MBOOL enable)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.lsc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    MY_LOG_IF(logEn,"%s %d\n", __FUNCTION__, enable);
    m_fgOnOff = enable;
}

MBOOL
ISP_MGR_LSC_T::
isEnable(void)
{
    return m_fgOnOff;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL ISP_MGR_PGN_T::setIspAWBGain(AWB_GAIN_T& rIspAWBGain)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.pgn_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    MY_LOG_IF(logEn, "ISP_MGR_PGN_T setIspAWBGain = %d %d %d\n", rIspAWBGain.i4R, rIspAWBGain.i4G, rIspAWBGain.i4B);
        m_rIspAWBGain = rIspAWBGain;
        return setIspPregain();
}

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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.pgn_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    // ISP pregain
    m_rIspPregain.i4R = (m_rIspAWBGain.i4R * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4G = (m_rIspAWBGain.i4G * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4B = (m_rIspAWBGain.i4B * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG_IF(logEn, "m_i4FlareGain = %d\n", m_i4FlareGain);
    MY_LOG_IF(logEn, "m_rIspPregain.i4R = %d; m_rIspAWBGain.i4R = %d\n", m_rIspPregain.i4R , m_rIspAWBGain.i4R);
    MY_LOG_IF(logEn, "m_rIspPregain.i4G = %d; m_rIspAWBGain.i4G = %d\n", m_rIspPregain.i4G , m_rIspAWBGain.i4G);
    MY_LOG_IF(logEn, "m_rIspPregain.i4B = %d; m_rIspAWBGain.i4B = %d\n", m_rIspPregain.i4B , m_rIspAWBGain.i4B);

    // ISP flare offset
    m_i4IspFlareOffset = (m_i4FlareOffset * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG_IF(logEn, "m_i4IspFlareOffset = %d; m_i4FlareOffset = %d\n", m_i4IspFlareOffset , m_i4FlareOffset);

    if ((m_rIspPregain.i4R == 0) ||
        (m_rIspPregain.i4G == 0) ||
        (m_rIspPregain.i4B == 0)) {
        MY_ERR("setIspPregain(): R = %d, G = %d, B = %d\n", m_rIspPregain.i4R, m_rIspPregain.i4G, m_rIspPregain.i4B);
        return MFALSE;
    }

    // CAM_PGN_GAIN01
    reinterpret_cast<ISP_CAM_PGN_GAIN01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN01))->PGN_CH0_GAIN = m_rIspPregain.i4B;
    reinterpret_cast<ISP_CAM_PGN_GAIN01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN01))->PGN_CH1_GAIN = m_rIspPregain.i4G;

    // CAM_PGN_GAIN23
    reinterpret_cast<ISP_CAM_PGN_GAIN23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN23))->PGN_CH2_GAIN = m_rIspPregain.i4G;
    reinterpret_cast<ISP_CAM_PGN_GAIN23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN23))->PGN_CH3_GAIN = m_rIspPregain.i4R;

    // CAM_PGN_OFFS01
    MUINT32 OFFS = (m_i4IspFlareOffset >= 0) ? static_cast<MUINT32>(m_i4IspFlareOffset) : static_cast<MUINT32>(4096 + m_i4IspFlareOffset);
    reinterpret_cast<ISP_CAM_PGN_OFFS01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS01))->PGN_CH0_OFFS = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFFS01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS01))->PGN_CH1_OFFS = OFFS;

    // CAM_PGN_OFFS23
    reinterpret_cast<ISP_CAM_PGN_OFFS23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS23))->PGN_CH2_OFFS = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFFS23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS23))->PGN_CH3_OFFS = OFFS;

    return MTRUE;
}

template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
put(ISP_NVRAM_PGN_T const& rParam)
{
    PUT_REG_INFO(CAM_PGN_SATU01, satu01);
    PUT_REG_INFO(CAM_PGN_SATU23, satu23);
    PUT_REG_INFO(CAM_PGN_GAIN01, gain01);
    PUT_REG_INFO(CAM_PGN_GAIN23, gain23);
    PUT_REG_INFO(CAM_PGN_OFFS01, offs01);
    PUT_REG_INFO(CAM_PGN_OFFS23, offs23);

    return  (*this);
}


template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
get(ISP_NVRAM_PGN_T& rParam)
{
    GET_REG_INFO(CAM_PGN_SATU01, satu01);
    GET_REG_INFO(CAM_PGN_SATU23, satu23);
    GET_REG_INFO(CAM_PGN_GAIN01, gain01);
    GET_REG_INFO(CAM_PGN_GAIN23, gain23);
    GET_REG_INFO(CAM_PGN_OFFS01, offs01);
    GET_REG_INFO(CAM_PGN_OFFS23, offs23);

    return  (*this);
}


MBOOL
ISP_MGR_PGN_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, PGN_EN, 1);

    return  MTRUE;
}

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

MBOOL
ISP_MGR_SL2_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
#define ISP_SL2_LOG(cq) \
    MY_LOG_IF(ENABLE_MY_LOG,                                        \
        "%s "#cq" bSL2_EN = %d\n"                                   \
        "    SL2_EN_SET = %d\n"                                     \
        "    CAM_SL2_CEN     = 0x%08x\n"                            \
        "    CAM_SL2_MAX0_RR = 0x%08x\n"                            \
        "    CAM_SL2_MAX1_RR = 0x%08x\n"                            \
        "    CAM_SL2_MAX2_RR = 0x%08x\n",                           \
        __FUNCTION__, bSL2_EN,                                      \
        ISP_READ_BITS(getIspReg(cq), CAM_CTL_EN1_SET, SL2_EN_SET),  \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_CEN),                   \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX0_RR),               \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX1_RR),               \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX2_RR))

    MBOOL bSL2_EN = isEnable();

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, SL2_EN, bSL2_EN);

    //ISP_SL2_LOG(ISPDRV_MODE_CQ0);

    return  MTRUE;
}


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
    PUT_REG_INFO(CAM_CFA_BYPASS, bypass);
    PUT_REG_INFO(CAM_CFA_ED_F, ed_f);
    PUT_REG_INFO(CAM_CFA_ED_NYQ, ed_nyq);
    PUT_REG_INFO(CAM_CFA_ED_STEP, ed_step);
    PUT_REG_INFO(CAM_CFA_RGB_HF, rgb_hf);
    PUT_REG_INFO(CAM_CFA_BW, bw);
    PUT_REG_INFO(CAM_CFA_F1_ACT, f1_act);
    PUT_REG_INFO(CAM_CFA_F2_ACT, f2_act);
    PUT_REG_INFO(CAM_CFA_F3_ACT, f3_act);
    PUT_REG_INFO(CAM_CFA_F4_ACT, f4_act);
    PUT_REG_INFO(CAM_CFA_F1_L, f1_l);
    PUT_REG_INFO(CAM_CFA_F2_L, f2_l);
    PUT_REG_INFO(CAM_CFA_F3_L, f3_l);
    PUT_REG_INFO(CAM_CFA_F4_L, f4_l);
    PUT_REG_INFO(CAM_CFA_HF_RB, hf_rb);
    PUT_REG_INFO(CAM_CFA_HF_GAIN, hf_gain);
    PUT_REG_INFO(CAM_CFA_HF_COMP, hf_comp);
    PUT_REG_INFO(CAM_CFA_HF_CORING_TH, hf_coring_th);
    PUT_REG_INFO(CAM_CFA_ACT_LUT, act_lut);
    PUT_REG_INFO(CAM_CFA_SPARE, spare);
    PUT_REG_INFO(CAM_CFA_BB, bb);

    return  (*this);
}


template <>
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
get(ISP_NVRAM_CFA_T& rParam)
{
    GET_REG_INFO(CAM_CFA_BYPASS, bypass);
    GET_REG_INFO(CAM_CFA_ED_F, ed_f);
    GET_REG_INFO(CAM_CFA_ED_NYQ, ed_nyq);
    GET_REG_INFO(CAM_CFA_ED_STEP, ed_step);
    GET_REG_INFO(CAM_CFA_RGB_HF, rgb_hf);
    GET_REG_INFO(CAM_CFA_BW, bw);
    GET_REG_INFO(CAM_CFA_F1_ACT, f1_act);
    GET_REG_INFO(CAM_CFA_F2_ACT, f2_act);
    GET_REG_INFO(CAM_CFA_F3_ACT, f3_act);
    GET_REG_INFO(CAM_CFA_F4_ACT, f4_act);
    GET_REG_INFO(CAM_CFA_F1_L, f1_l);
    GET_REG_INFO(CAM_CFA_F2_L, f2_l);
    GET_REG_INFO(CAM_CFA_F3_L, f3_l);
    GET_REG_INFO(CAM_CFA_F4_L, f4_l);
    GET_REG_INFO(CAM_CFA_HF_RB, hf_rb);
    GET_REG_INFO(CAM_CFA_HF_GAIN, hf_gain);
    GET_REG_INFO(CAM_CFA_HF_COMP, hf_comp);
    GET_REG_INFO(CAM_CFA_HF_CORING_TH, hf_coring_th);
    GET_REG_INFO(CAM_CFA_ACT_LUT, act_lut);
    GET_REG_INFO(CAM_CFA_SPARE, spare);
    GET_REG_INFO(CAM_CFA_BB, bb);

    return  (*this);
}

MBOOL
ISP_MGR_CFA_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, CFA_EN, 1);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_T const& rParam)
{
    PUT_REG_INFO(CAM_G2G_CONV0A, conv0a);
    PUT_REG_INFO(CAM_G2G_CONV0B, conv0b);
    PUT_REG_INFO(CAM_G2G_CONV1A, conv1a);
    PUT_REG_INFO(CAM_G2G_CONV1B, conv1b);
    PUT_REG_INFO(CAM_G2G_CONV2A, conv2a);
    PUT_REG_INFO(CAM_G2G_CONV2B, conv2b);

    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam)
{
    GET_REG_INFO(CAM_G2G_CONV0A, conv0a);
    GET_REG_INFO(CAM_G2G_CONV0B, conv0b);
    GET_REG_INFO(CAM_G2G_CONV1A, conv1a);
    GET_REG_INFO(CAM_G2G_CONV1B, conv1b);
    GET_REG_INFO(CAM_G2G_CONV2A, conv2a);
    GET_REG_INFO(CAM_G2G_CONV2B, conv2b);

    return  (*this);
}

MBOOL
ISP_MGR_CCM_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2G_CONV0A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0A))->G2G_CNV_00 = 256;
        reinterpret_cast<ISP_CAM_G2G_CONV0A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0A))->G2G_CNV_01 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV0B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0B))->G2G_CNV_02 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV1A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1A))->G2G_CNV_10 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV1A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1A))->G2G_CNV_11 = 256;
        reinterpret_cast<ISP_CAM_G2G_CONV1B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1B))->G2G_CNV_12 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2A))->G2G_CNV_20 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2A))->G2G_CNV_21 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2B))->G2G_CNV_22 = 256;
    }

    // CAM_G2G_CTRL
    ISP_WRITE_ENABLE_BITS(pReg, CAM_G2G_ACC, G2G_ACC, 8); // Q1.2.8

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, G2G_EN, isEnable());

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_GGM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
put(ISP_NVRAM_GGM_T const& rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        m_rIspRegInfo_GGM_RB[i].val = rParam.rb_gmt.set[i];
        m_rIspRegInfo_GGM_G[i].val = rParam.g_gmt.set[i];
        //MY_LOG("m_rIspRegInfo_GGM_RB[%d].val = 0x%8x", i, m_rIspRegInfo_GGM_RB[i].val);
        //MY_LOG("m_rIspRegInfo_GGM_G[%d].val = 0x%8x", i, m_rIspRegInfo_GGM_G[i].val);
    }

    return  (*this);
}


template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
get(ISP_NVRAM_GGM_T& rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        rParam.rb_gmt.set[i] = m_rIspRegInfo_GGM_RB[i].val;
        rParam.g_gmt.set[i] = m_rIspRegInfo_GGM_G[i].val;
    }

    return  (*this);
}

MBOOL
ISP_MGR_GGM_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{

    MBOOL bGAMMA_EN = isEnable();
    reinterpret_cast<ISP_CAM_GGM_CTRL_T*>(REG_INFO_VALUE_PTR(CAM_GGM_CTRL))->GAMMA_EN = bGAMMA_EN;

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo), m_u4RegInfoNum, pReg);
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_RB), GGM_LUT_SIZE, pReg);
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_G), GGM_LUT_SIZE, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN1, GGM_EN, bGAMMA_EN);

    return MTRUE;
}

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
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV00 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV01 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_CNV02 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_YOFFSET11 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV10 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV11 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_CNV12 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_UOFFSET10 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV20 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV21 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_CNV22 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_VOFFSET10 = 0;
    }

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN2, G2C_EN, isEnable());

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_T const& rParam)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.nbc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    PUT_REG_INFO(CAM_ANR_CON1, con1);
    PUT_REG_INFO(CAM_ANR_CON2, con2);
    PUT_REG_INFO(CAM_ANR_CON3, con3);
    PUT_REG_INFO(CAM_ANR_YAD1, yad1);
    PUT_REG_INFO(CAM_ANR_YAD2, yad2);
    PUT_REG_INFO(CAM_ANR_4LUT1, lut1);
    PUT_REG_INFO(CAM_ANR_4LUT2, lut2);
    PUT_REG_INFO(CAM_ANR_4LUT3, lut3);
    PUT_REG_INFO(CAM_ANR_PTY, pty);
    PUT_REG_INFO(CAM_ANR_CAD, cad);
    PUT_REG_INFO(CAM_ANR_PTC, ptc);
    PUT_REG_INFO(CAM_ANR_LCE1, lce1);
    PUT_REG_INFO(CAM_ANR_LCE2, lce2);
    PUT_REG_INFO(CAM_ANR_HP1, hp1);
    PUT_REG_INFO(CAM_ANR_HP2, hp2);
    PUT_REG_INFO(CAM_ANR_HP3, hp3);
    PUT_REG_INFO(CAM_ANR_ACTY, acty);
    PUT_REG_INFO(CAM_ANR_ACTC, actc);

    m_bANRENCBackup = rParam.con1.val & 0x1;
    m_bANRENYBackup = (rParam.con1.val & 0x2)>>1;
    MY_LOG_IF(logEn, "ISP_MGR_NBC_T::put() ANR CON1 = %d\n", (rParam.con1.val));

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_T& rParam)
{
    GET_REG_INFO(CAM_ANR_CON1, con1);
    GET_REG_INFO(CAM_ANR_CON2, con2);
    GET_REG_INFO(CAM_ANR_CON3, con3);
    GET_REG_INFO(CAM_ANR_YAD1, yad1);
    GET_REG_INFO(CAM_ANR_YAD2, yad2);
    GET_REG_INFO(CAM_ANR_4LUT1, lut1);
    GET_REG_INFO(CAM_ANR_4LUT2, lut2);
    GET_REG_INFO(CAM_ANR_4LUT3, lut3);
    GET_REG_INFO(CAM_ANR_PTY, pty);
    GET_REG_INFO(CAM_ANR_CAD, cad);
    GET_REG_INFO(CAM_ANR_PTC, ptc);
    GET_REG_INFO(CAM_ANR_LCE1, lce1);
    GET_REG_INFO(CAM_ANR_LCE2, lce2);
    GET_REG_INFO(CAM_ANR_HP1, hp1);
    GET_REG_INFO(CAM_ANR_HP2, hp2);
    GET_REG_INFO(CAM_ANR_HP3, hp3);
    GET_REG_INFO(CAM_ANR_ACTY, acty);
    GET_REG_INFO(CAM_ANR_ACTC, actc);

    return  (*this);
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_CCR_T const& rParam)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.nbc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    PUT_REG_INFO(CAM_CCR_CON, con);
    PUT_REG_INFO(CAM_CCR_YLUT, ylut);
    PUT_REG_INFO(CAM_CCR_UVLUT, uvlut);
    PUT_REG_INFO(CAM_CCR_YLUT2, ylut2);

    m_bCCREnBackup = rParam.con.val & 0x1;
    MY_LOG_IF(logEn, "ISP_MGR_NBC_T::put() m_bCCREnBackup = %d\n", m_bCCREnBackup);

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
get(ISP_NVRAM_CCR_T& rParam)
{
    GET_REG_INFO(CAM_CCR_CON, con);
    GET_REG_INFO(CAM_CCR_YLUT, ylut);
    GET_REG_INFO(CAM_CCR_UVLUT, uvlut);
    GET_REG_INFO(CAM_CCR_YLUT2, ylut2);

    return  (*this);
}

MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.nbc_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    MBOOL bANR_ENY = /*reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY*/m_bANRENYBackup & isANREnable();
    MBOOL bANR_ENC = /*reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC*/m_bANRENCBackup & isANREnable();
    MBOOL bCCR_ENC = m_bCCREnBackup & isCCREnable();
    MBOOL bNBC_EN = bANR_ENY|bANR_ENC|bCCR_ENC;
    MINT32 i4ANR_FLT_MODE = reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_FLT_MODE;
    MINT32 i4ANR_SCALE_MODE = reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_SCALE_MODE;

    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY = bANR_ENY;
    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC = bANR_ENC;
    reinterpret_cast<ISP_CAM_CCR_CON_T*>(REG_INFO_VALUE_PTR(CAM_CCR_CON))->CCR_EN = bCCR_ENC;

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN2, NBC_EN, bNBC_EN);

    MY_LOG_IF(logEn, "bNBC_EN(%d)", bNBC_EN);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_PCA_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
put(ISP_NVRAM_PCA_T const& rParam)
{
    PUT_REG_INFO(CAM_PCA_CON1, con1);
    PUT_REG_INFO(CAM_PCA_CON2, con2);

    return  (*this);
}


template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
get(ISP_NVRAM_PCA_T& rParam)
{
    GET_REG_INFO(CAM_PCA_CON1, con1);
    GET_REG_INFO(CAM_PCA_CON2, con2);

    return  (*this);
}

MBOOL
ISP_MGR_PCA_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL bPCA_EN = isEnable();

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_PCA_LUT), PCA_BIN_NUM, pReg);
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN2, PCA_EN, bPCA_EN);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SEEE (SE + EE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_SEEE_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_EE_T const& rParam)
{
    PUT_REG_INFO(CAM_SEEE_SRK_CTRL, srk_ctrl);
    PUT_REG_INFO(CAM_SEEE_CLIP_CTRL, clip_ctrl);
    PUT_REG_INFO(CAM_SEEE_HP_CTRL1, hp_ctrl1);
    PUT_REG_INFO(CAM_SEEE_HP_CTRL2, hp_ctrl2);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL1, ed_ctrl1);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL2, ed_ctrl2);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL3, ed_ctrl3);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL4, ed_ctrl4);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL5, ed_ctrl5);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL6, ed_ctrl6);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL7, ed_ctrl7);
    PUT_REG_INFO(CAM_SEEE_EE_LINK1, ee_link1);
    PUT_REG_INFO(CAM_SEEE_EE_LINK2, ee_link2);
    PUT_REG_INFO(CAM_SEEE_EE_LINK3, ee_link3);
    PUT_REG_INFO(CAM_SEEE_EE_LINK4, ee_link4);
    PUT_REG_INFO(CAM_SEEE_EE_LINK5, ee_link5);

    return  (*this);
}


template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_EE_T& rParam)
{
    GET_REG_INFO(CAM_SEEE_SRK_CTRL, srk_ctrl);
    GET_REG_INFO(CAM_SEEE_CLIP_CTRL, clip_ctrl);
    GET_REG_INFO(CAM_SEEE_HP_CTRL1, hp_ctrl1);
    GET_REG_INFO(CAM_SEEE_HP_CTRL2, hp_ctrl2);
    GET_REG_INFO(CAM_SEEE_ED_CTRL1, ed_ctrl1);
    GET_REG_INFO(CAM_SEEE_ED_CTRL2, ed_ctrl2);
    GET_REG_INFO(CAM_SEEE_ED_CTRL3, ed_ctrl3);
    GET_REG_INFO(CAM_SEEE_ED_CTRL4, ed_ctrl4);
    GET_REG_INFO(CAM_SEEE_ED_CTRL5, ed_ctrl5);
    GET_REG_INFO(CAM_SEEE_ED_CTRL6, ed_ctrl6);
    GET_REG_INFO(CAM_SEEE_ED_CTRL7, ed_ctrl7);
    GET_REG_INFO(CAM_SEEE_EE_LINK1, ee_link1);
    GET_REG_INFO(CAM_SEEE_EE_LINK2, ee_link2);
    GET_REG_INFO(CAM_SEEE_EE_LINK3, ee_link3);
    GET_REG_INFO(CAM_SEEE_EE_LINK4, ee_link4);
    GET_REG_INFO(CAM_SEEE_EE_LINK5, ee_link5);
    return  (*this);
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_SE_T const& rParam)
{
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL, edge_ctrl);
    PUT_REG_INFO(CAM_SEEE_Y_CTRL, y_ctrl);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL1, edge_ctrl1);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL2, edge_ctrl2);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL3, edge_ctrl3);
    PUT_REG_INFO(CAM_SEEE_SPECIAL_CTRL, special_ctrl);
    PUT_REG_INFO(CAM_SEEE_CORE_CTRL1, core_ctrl1);
    PUT_REG_INFO(CAM_SEEE_CORE_CTRL2, core_ctrl2);

    return  (*this);
}


template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_SE_T& rParam)
{
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL, edge_ctrl);
    GET_REG_INFO(CAM_SEEE_Y_CTRL, y_ctrl);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL1, edge_ctrl1);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL2, edge_ctrl2);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL3, edge_ctrl3);
    GET_REG_INFO(CAM_SEEE_SPECIAL_CTRL, special_ctrl);
    GET_REG_INFO(CAM_SEEE_CORE_CTRL1, core_ctrl1);
    GET_REG_INFO(CAM_SEEE_CORE_CTRL2, core_ctrl2);

    return  (*this);
}


MBOOL
ISP_MGR_SEEE_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.ee_apply.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_MGR_LOG_BIT) ? MTRUE : MFALSE);

    MBOOL bSEEE_EN = isEnable();

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN2, SEEE_EN, bSEEE_EN);

    MY_LOG_IF(logEn, "bSEEE_EN(%d)", bSEEE_EN);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AWB_STAT_CONFIG_T&
ISP_MGR_AWB_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}


#define logReg( name, v ) {MY_LOG( "Reg(%s) = %d ", name, (int)v );}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
config(AWB_STAT_CONFIG_T& rAWBStatConfig, MINT32 i4SensorIndex)
{
    m_i4SensorIndex = i4SensorIndex;

#if 1
    //MY_LOG("rAWBStatConfig.i4WindowOriginX=%d",(int)rAWBStatConfig.i4WindowOriginX);

    logReg( "rAWBStatConfig.i4WindowOriginX", rAWBStatConfig.i4WindowOriginX );

    logReg("rAWBStatConfig.i4WindowOriginY", rAWBStatConfig.i4WindowOriginY);
    logReg("rAWBStaConfig.i4WindowSizeX",rAWBStatConfig.i4WindowSizeX);
    logReg("rAWBStaConfig.i4WindowSizeY",rAWBStatConfig.i4WindowSizeY);

    logReg("rAWBStaConfig.i4WindowPitchX",rAWBStatConfig.i4WindowPitchX);
    logReg("rAWBStaConfig.i4WindowPitchY",rAWBStatConfig.i4WindowPitchY);

    logReg("rAWBStaConfig.i4WindowNumX",rAWBStatConfig.i4WindowNumX);
    logReg("rAWBStaConfig.i4WindowNumY",rAWBStatConfig.i4WindowNumY);

    logReg("rAWBStaConfig.i4PreGainR",rAWBStatConfig.i4PreGainR);
    logReg("rAWBStaConfig.i4PreGainG",rAWBStatConfig.i4PreGainG);
    logReg("rAWBStaConfig.i4PreGainB",rAWBStatConfig.i4PreGainB);

    logReg("rAWBStaConfig.i4PreGainLimitR",rAWBStatConfig.i4PreGainLimitR);
    logReg("rAWBStaConfig.i4PreGainLimitG",rAWBStatConfig.i4PreGainLimitG);
    logReg("rAWBStaConfig.i4PreGainLimitB",rAWBStatConfig.i4PreGainLimitB);

    logReg("rAWBStaConfig.i4LowThresholdR",rAWBStatConfig.i4LowThresholdR);
    logReg("rAWBStaConfig.i4LowThresholdG",rAWBStatConfig.i4LowThresholdG);
    logReg("rAWBStaConfig.i4LowThresholdB",rAWBStatConfig.i4LowThresholdB);

    logReg("rAWBStaConfig.i4HighThresholdR",rAWBStatConfig.i4HighThresholdR);
    logReg("rAWBStaConfig.i4HighThresholdG",rAWBStatConfig.i4HighThresholdG);
    logReg("rAWBStaConfig.i4HighThresholdB",rAWBStatConfig.i4HighThresholdB);

    logReg("rAWBStaConfig.i4PixelCountR",rAWBStatConfig.i4PixelCountR);
    logReg("rAWBStaConfig.i4PixelCountG",rAWBStatConfig.i4PixelCountG);
    logReg("rAWBStaConfig.i4PixelCountB",rAWBStatConfig.i4PixelCountB);

    logReg("rAWBStaConfig.i4ErrorThreshold",rAWBStatConfig.i4ErrorThreshold);
    logReg("rAWBStaConfig.i4Cos",rAWBStatConfig.i4Cos);
    logReg("rAWBStaConfig.i4Sin",rAWBStatConfig.i4Sin);





#endif


    // CAM_AWB_WIN_ORG
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_WIN_ORG_X = rAWBStatConfig.i4WindowOriginX;
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_WIN_ORG_Y = rAWBStatConfig.i4WindowOriginY;
    // CAM_AWB_WIN_SIZE
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_WIN_SIZE_X = rAWBStatConfig.i4WindowSizeX;
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_WIN_SIZE_Y = rAWBStatConfig.i4WindowSizeY;
    // CAM_AWB_WIN_PITCH
    reinterpret_cast<ISP_CAM_AWB_WIN_PITCH_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PITCH))->AWB_WIN_PIT_X = rAWBStatConfig.i4WindowPitchX;
    reinterpret_cast<ISP_CAM_AWB_WIN_PITCH_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PITCH))->AWB_WIN_PIT_Y = rAWBStatConfig.i4WindowPitchY;
    // CAM_AWB_WIN_NUM
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_WIN_NUM_X = rAWBStatConfig.i4WindowNumX;
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_WIN_NUM_Y = rAWBStatConfig.i4WindowNumY;
    MY_LOG("[ISP_MGR_AWB_STAT_CONFIG_T] i4WindowNumX:%d i4WindowNumY:%d\n", rAWBStatConfig.i4WindowNumX, rAWBStatConfig.i4WindowNumY);

    // CAM_AWB_RAWPREGAIN1_0
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_0))->RAWPREGAIN1_R = rAWBStatConfig.i4PreGainR;
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_0))->RAWPREGAIN1_G = rAWBStatConfig.i4PreGainG;
    // CAM_AWB_RAWPREGAIN1_1
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_1))->RAWPREGAIN1_B = rAWBStatConfig.i4PreGainB;
    // CAM_AWB_RAWLIMIT1_0
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_0))->RAWLIMIT1_R = rAWBStatConfig.i4PreGainLimitR;
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_0))->RAWLIMIT1_G = rAWBStatConfig.i4PreGainLimitG;
    // CAM_AWB_RAWLIMIT1_1
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_1))->RAWLIMIT1_B = rAWBStatConfig.i4PreGainLimitB;
    // CAM_AWB_LOW_THR
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;
    // CAM_AWB_HI_THR
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;
    // CAM_AWB_PIXEL_CNT0
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT0))->PIXEL_CNT0 = rAWBStatConfig.i4PixelCountR;
    // CAM_AWB_PIXEL_CNT1
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT1))->PIXEL_CNT1 = rAWBStatConfig.i4PixelCountG;
    // CAM_AWB_PIXEL_CNT2
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT2_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT2))->PIXEL_CNT2 = rAWBStatConfig.i4PixelCountB;
    // CAM_AWB_ERR_THR
    reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;
    // CAM_AWB_ROT
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_C = rAWBStatConfig.i4Cos;
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_S = rAWBStatConfig.i4Sin;

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

    apply();

    return MTRUE;
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply()
{
    //MY_LOG_IF(IsDebugEnabled(), "%s(): m_i4SensorIndex = %d\n", __FUNCTION__, m_i4SensorIndex);
#if (CAM3_3ATESTLVL <= CAM3_3AUT)
#warning "FIXME"
    return MTRUE;
#endif
    MUINTPTR handle;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIndex,"isp_mgr_awb_stat_config");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_GET_MODULE_HANDLE,
                                     NSImageio_FrmB::NSIspio_FrmB::EModule_AWB, (MUINTPTR)&handle, (MUINTPTR)(&("isp_mgr_awb_stat_config"))))
    {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
#if 1
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_ORG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_SIZE].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_PITCH, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_PITCH].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_NUM].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_RAWPREGAIN1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_RAWPREGAIN1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_RAWPREGAIN1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_RAWPREGAIN1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_RAWLIMIT1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_RAWLIMIT1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_RAWLIMIT1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_RAWLIMIT1_1].val);
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
#endif

    MY_LOG("[ISP_MGR_AWB_STAT_CONFIG_T] i4WindowNumX i4WindowNumY:0x%x\n", m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_NUM].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MUINTPTR)(&("isp_mgr_awb_stat_tg1")), MNULL))
    {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_awb_stat_config");

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_RAWPREGAIN2_T&
ISP_MGR_AE_RAWPREGAIN2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_RAWPREGAIN2_T::
setRAWPregain2(AWB_GAIN_T& rAWBRAWPregain2)
{
    m_rIspAEPreGain2 = rAWBRAWPregain2;

    // CAM_AE_RAWPREGAIN2_0
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_0))->RAWPREGAIN2_R = rAWBRAWPregain2.i4R;
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_0))->RAWPREGAIN2_G = rAWBRAWPregain2.i4G;

    // CAM_AE_RAWPREGAIN2_1
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_1))->RAWPREGAIN2_B = rAWBRAWPregain2.i4B;

    apply();

    return MTRUE;
}

MBOOL
ISP_MGR_AE_RAWPREGAIN2_T::
apply()
{

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics and Histogram Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_STAT_HIST_CONFIG_T&
ISP_MGR_AE_STAT_HIST_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_STAT_HIST_CONFIG_T::
config(AE_STAT_PARAM_T& rAEStatConfig, MINT32 i4SensorIndex)
{
    MY_LOG("setAEHistConfig SensorIdx:%d Hist0:%d %d %d %d %d %d %d, Hist1:%d %d %d %d %d %d %d, Hist2:%d %d %d %d %d %d %d, Hist3:%d %d %d %d %d %d %d\n", m_i4SensorIndex,
    rAEStatConfig.rAEHistWinCFG[0].bAEHistEn, rAEStatConfig.rAEHistWinCFG[0].uAEHistBin, rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow,
    rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[1].bAEHistEn,
    rAEStatConfig.rAEHistWinCFG[1].uAEHistBin, rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi,
    rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[2].bAEHistEn, rAEStatConfig.rAEHistWinCFG[2].uAEHistBin,
    rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow,
    rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[3].bAEHistEn, rAEStatConfig.rAEHistWinCFG[3].uAEHistBin, rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt,
    rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi);

    m_i4SensorIndex = i4SensorIndex;

    // CAM_AE_HST_CTL
    reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
    reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
    reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
    reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
     // CAM_AE_RAWLIMIT2_0
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_0))->AE_LIMIT2_R = 0xFFF;
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_0))->AE_LIMIT2_G = 0xFFF;
    // CAM_AE_RAWLIMIT2_1
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_1))->AE_LIMIT2_B = 0xFFF;
    // CAM_AE_MATRIX_COEF0
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF0))->RC_CNV00 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF0))->RC_CNV01 = 0x000;
    // CAM_AE_MATRIX_COEF1
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF1))->RC_CNV02 = 0x000;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF1))->RC_CNV10 = 0x000;
    // CAM_AE_MATRIX_COEF2
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF2))->RC_CNV11 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF2))->RC_CNV12 = 0x000;
    // CAM_AE_MATRIX_COEF3
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF3))->RC_CNV20 = 0x000;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF3))->RC_CNV21 = 0x000;
    // CAM_AE_MATRIX_COEF4
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF4))->RC_CNV22 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF4))->AE_RC_ACC = 0x09;
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

    apply();
    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_HIST_CONFIG_T::
apply()
{
#if (CAM3_3ATESTLVL <= CAM3_3AUT)
#warning "FIXME"
    return MTRUE;
#endif

    MUINTPTR handle;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIndex,"isp_mgr_ae_stat_tg1");

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio_FrmB::NSIspio_FrmB::EModule_AE, (MUINTPTR)&handle, (MUINTPTR)(&("isp_mgr_ae_stat_tg1")))) {
        //Error Handling
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }
    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_CTL, m_rIspRegInfo[ERegInfo_CAM_AE_HST_CTL].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RAWLIMIT2_0, m_rIspRegInfo[ERegInfo_CAM_AE_RAWLIMIT2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RAWLIMIT2_1, m_rIspRegInfo[ERegInfo_CAM_AE_RAWLIMIT2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_MATRIX_COEF0, m_rIspRegInfo[ERegInfo_CAM_AE_MATRIX_COEF0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_MATRIX_COEF1, m_rIspRegInfo[ERegInfo_CAM_AE_MATRIX_COEF1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_MATRIX_COEF2, m_rIspRegInfo[ERegInfo_CAM_AE_MATRIX_COEF2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_MATRIX_COEF3, m_rIspRegInfo[ERegInfo_CAM_AE_MATRIX_COEF3].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_MATRIX_COEF4, m_rIspRegInfo[ERegInfo_CAM_AE_MATRIX_COEF4].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_0, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_1, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_SET, m_rIspRegInfo[ERegInfo_CAM_AE_HST_SET].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST0_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST0_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST1_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST1_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST2_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST2_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST3_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST3_RNG].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
        //Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MUINTPTR)(&("isp_mgr_ae_stat_tg1")), MNULL)) {
        // Error Handling
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_ae_stat_tg1");
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Flicker
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_FLK_CONFIG_T&
ISP_MGR_FLK_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    }
}


MBOOL
ISP_MGR_FLK_CONFIG_T::
apply()
{
#if 0
    ISPDRV_MODE_T drv_mode;
    IspDrv* m_pIspDrv = IspDrv::createInstance();

    drv_mode=ISPDRV_MODE_CQ0;

        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_DMA_ESFKO);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_DMA_FLKI);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_ISP_FLK);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_TOP_CTL_01);

#if 1
        ISP_BITS(getIspReg(drv_mode),CAM_FLK_CON,FLK_MODE)=reinterpret_cast<ISP_CAM_FLK_CON*>(REG_INFO_VALUE_PTR(CAM_FLK_CON))->FLK_MODE;
        ISP_BITS(getIspReg(drv_mode), CAM_CTL_EN1, FLK_EN) =reinterpret_cast<ISP_CAM_FLK_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->FLK_EN;
        ISP_BITS(getIspReg(drv_mode),CAM_CTL_EN1_SET,FLK_EN_SET)=reinterpret_cast<ISP_CAM_CTL_EN1_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1_SET))->FLK_EN_SET;
        ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_EN,ESFKO_EN)=reinterpret_cast<ISP_CAM_CTL_DMA_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN))->ESFKO_EN;
        ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_EN_SET,ESFKO_EN_SET)=reinterpret_cast<ISP_CAM_CTL_DMA_EN_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN_SET))->ESFKO_EN_SET;
        ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_INT,ESFKO_DONE_EN)=reinterpret_cast<ISP_CAM_CTL_DMA_INT*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN ;
        ISP_BITS(getIspReg(drv_mode),CAM_CTL_INT_EN,FLK_DON_EN)=reinterpret_cast<ISP_CAM_CTL_INT_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_INT_EN))->FLK_DON_EN ;
#endif

    m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_ESFKO);
    m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_FLKI);
    m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_ISP_FLK);
    m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_TOP_CTL_01);

    //FLICKER_LOG("[apply 22]: ,FLK_MODE:%d ,FLK_EN:%d ,FLK_EN_SET:%d,ESFKO_DMA_EN:%d\n",  ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_FLK_CON,FLK_MODE),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1,FLK_EN),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1_SET,FLK_EN_SET),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_DMA_EN,ESFKO_EN));
    //MY_LOG_IF(ENABLE_MY_LOG,"[apply 23]: ,FLK_MODE:%d ,FLK_EN:%d ,FLK_EN_SET:%d,ESFKO_DMA_EN:%d\n",  ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_FLK_CON,FLK_MODE), (int) ISP_REG(getIspReg(ISPDRV_MODE_ISP), CAM_FLK_CON),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1,FLK_EN),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1_SET,FLK_EN_SET),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_DMA_EN,ESFKO_EN));

#endif

    return  MTRUE;//writeRegs(CAM_ISP_FLK, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
}


MVOID
ISP_MGR_FLK_CONFIG_T::
enableFlk(MBOOL enable)
{
#if 0
        reinterpret_cast<ISP_CAM_FLK_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->FLK_EN = enable;
        reinterpret_cast<ISP_CAM_CTL_EN1_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1_SET))->FLK_EN_SET = enable;
        reinterpret_cast<ISP_CAM_CTL_DMA_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN))->ESFKO_EN = enable;
        reinterpret_cast<ISP_CAM_CTL_DMA_EN_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN_SET))->ESFKO_EN_SET = enable;
        reinterpret_cast<ISP_CAM_FLK_CON*>(REG_INFO_VALUE_PTR(CAM_FLK_CON))->FLK_MODE = 0;
        reinterpret_cast<ISP_CAM_CTL_DMA_INT*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN = enable;
        reinterpret_cast<ISP_CAM_CTL_INT_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_INT_EN))->FLK_DON_EN = enable;
        //reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN = enable;
        apply();
#endif
}
MVOID
ISP_MGR_FLK_CONFIG_T::
SetFLKWin(MINT32 offsetX,MINT32 offsetY , MINT32 sizeX ,MINT32 sizeY)
{

}
MVOID
ISP_MGR_FLK_CONFIG_T::
SetFKO_DMA_Addr(MINT32 address,MINT32 size)
{

}
};
