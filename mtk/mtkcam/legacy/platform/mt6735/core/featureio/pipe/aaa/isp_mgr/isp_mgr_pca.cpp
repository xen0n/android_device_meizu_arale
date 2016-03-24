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
#define LOG_TAG "isp_mgr_pca"

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
// PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
getInstance(ESensorDev_T const eSensorDev, EPCAMode_T const ePCAMode)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (ePCAMode == EPCAMode_180BIN)
            return  ISP_MGR_PCA_DEV<ESensorDev_Main, EPCAMode_180BIN>::getInstance();
        else
            return  ISP_MGR_PCA_DEV<ESensorDev_Main, EPCAMode_360BIN>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (ePCAMode == EPCAMode_180BIN)
            return  ISP_MGR_PCA_DEV<ESensorDev_MainSecond, EPCAMode_180BIN>::getInstance();
        else
            return  ISP_MGR_PCA_DEV<ESensorDev_MainSecond, EPCAMode_360BIN>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (ePCAMode == EPCAMode_180BIN)
            return  ISP_MGR_PCA_DEV<ESensorDev_Sub, EPCAMode_180BIN>::getInstance();
        else
            return  ISP_MGR_PCA_DEV<ESensorDev_Sub, EPCAMode_360BIN>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, ePCAMode = %d", eSensorDev, ePCAMode);
        return  ISP_MGR_PCA_DEV<ESensorDev_Main, EPCAMode_180BIN>::getInstance();
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
apply(EIspProfile_T eIspProfile)
{

    addressErrorCheck("Before ISP_MGR_PCA_T::apply()");

    MBOOL bPCA_EN = isEnable();

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_PCA(bPCA_EN);

    // PCA_LUT_360
    reinterpret_cast<ISP_CAM_PCA_CON1_T*>(REG_INFO_VALUE_PTR(CAM_PCA_CON1))->PCA_LUT_360 = (m_ePCAMode == EPCAMode_180BIN) ? 0 : 1;
    reinterpret_cast<ISP_CAM_PCA_CON2_T*>(REG_INFO_VALUE_PTR(CAM_PCA_CON2))->PCA_Y2YLUT_EN = (m_ePCAMode == EPCAMode_180BIN) ? 0 : 1;

    // TOP
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, PCA_EN, bPCA_EN);


    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_Pca_Con);

    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                 m_u4RegInfoNum);

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                           eTuningMgrFunc_Pca_Tbl);

    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>((MVOID*)m_rIspRegInfo_PCA_LUT),
                                                 (m_ePCAMode == EPCAMode_180BIN) ? PCA_BIN_NUM : PCA_BIN_NUM*2);

    addressErrorCheck("After ISP_MGR_PCA_T::apply()");

    return  MTRUE;
}


}
