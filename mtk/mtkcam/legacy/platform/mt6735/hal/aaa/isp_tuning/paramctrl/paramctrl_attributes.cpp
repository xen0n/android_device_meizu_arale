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
#define LOG_TAG "paramctrl_attributes"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <sys/stat.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/common.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>
#include <ccm_mgr.h>
#include <lsc_mgr2.h>
#include <isp_mgr.h>
#include <mtkcam/hal/IHalSensor.h>
#include "paramctrl.h"

using namespace android;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

MVOID
Paramctrl::
enableDynamicTuning(MBOOL const fgEnable)
{
    MY_LOG_IF(m_bDebugEnable, "[+enableDynamicTuning](old, new)=(%d, %d)", m_fgDynamicTuning, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicTuning, fgEnable) )
    {
        m_fgDynamicTuning = fgEnable;
    }
}


MVOID
Paramctrl::
enableDynamicCCM(MBOOL const fgEnable)
{
    MY_LOG_IF(m_bDebugEnable, "[+enableDynamicCCM](old, new)=(%d, %d)", m_fgDynamicCCM, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicCCM, fgEnable) )
    {
        m_fgDynamicCCM = fgEnable;
    }
}

MVOID
Paramctrl::
enableDynamicShading(MBOOL fgEnable)
{
    MY_LOG_IF(m_bDebugEnable, "[+enableDynamicShading](old, new)=(%d, %d)", m_fgDynamicShading, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if (m_eOperMode == EOperMode_Meta)
    {
        fgEnable = MFALSE;
    }

    MY_LOG("[%s] fgEnable(%d) m_eOperMode(%d)", __FUNCTION__, fgEnable, m_eOperMode);
    if ( checkParamChange(m_fgDynamicShading, fgEnable) )
    {
        m_fgDynamicShading = fgEnable;
    }
}


#if 0
MVOID
Paramctrl::
updateShadingNVRAMdata(MBOOL const fgEnable)
{
    MY_LOG("[+updateShadingNVRAMdata](old, new)=(%d, %d)", m_fgShadingNVRAMdataChange, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgShadingNVRAMdataChange, fgEnable) )
    {
        m_fgShadingNVRAMdataChange = fgEnable;
    }

    MY_LOG("[-updateShadingNVRAMdata] return");
    return;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspProfile(EIspProfile_T const eIspProfile)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIspProfile, eIspProfile) )
    {
        MY_LOG("[+setIspProfile](old, new)=(%d, %d)", m_rIspCamInfo.eIspProfile, eIspProfile);
        m_rIspCamInfo.eIspProfile = eIspProfile;
    }

    // check software scenario
    ESoftwareScenario eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;

    switch(m_rIspCamInfo.eIspProfile) {
        case EIspProfile_Preview:
        case EIspProfile_Video:
        // iHDR
        case EIspProfile_IHDR_Preview:
        case EIspProfile_IHDR_Video:
        case EIspProfile_MHDR_Preview:
        case EIspProfile_MHDR_Video:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Normal_Stream;
            break;
        case EIspProfile_Capture:
        case EIspProfile_Capture_SWNR:
        case EIspProfile_Capture_MultiPass_ANR_1:
        case EIspProfile_Capture_MultiPass_ANR_2:
        case EIspProfile_MFB_MultiPass_ANR_1:
        case EIspProfile_MFB_MultiPass_ANR_2:
        case EIspProfile_MHDR_Capture:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Normal_Capture;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Normal_Capture;
            break;
        case EIspProfile_ZSD_Capture:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_ZSD_Capture;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_ZSD_Capture;
            break;
        case EIspProfile_VSS_Capture:
        case EIspProfile_VSS_Capture_SWNR:
        case EIspProfile_VSS_Capture_MultiPass_ANR_1:
        case EIspProfile_VSS_Capture_MultiPass_ANR_2:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_VSS_Capture;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_VSS_Capture;
            break;
        case EIspProfile_PureRAW_Capture:
        case EIspProfile_PureRAW_Capture_SWNR:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Pure_Raw_Stream;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Pure_Raw_Stream;
            break;
        // N3D
        case EIspProfile_N3D_Preview:
        case EIspProfile_N3D_Video:
        case EIspProfile_N3D_Capture:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;
            else
                eSoftwareScenario = eSoftwareScenario_Main2_N3D_Stream;
            break;
        // MFB
        case EIspProfile_MFB_Capture_EE_Off:
        case EIspProfile_MFB_Capture_EE_Off_SWNR:
        case EIspProfile_MFB_PostProc_EE_Off:
        case EIspProfile_MFB_PostProc_ANR_EE:
        case EIspProfile_MFB_PostProc_ANR_EE_SWNR:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Mfb_Capture;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Capture;
            break;
        case EIspProfile_MFB_Blending_All_Off:
        case EIspProfile_MFB_Blending_All_Off_SWNR:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Mfb_Blending;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Blending;
            break;
        case EIspProfile_MFB_PostProc_Mixing:
        case EIspProfile_MFB_PostProc_Mixing_SWNR:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Mfb_Mixing;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Mixing;
            break;
        // vFB
        case EIspProfile_VFB_PostProc:
            if (m_eSensorDev == ESensorDev_Main)
                eSoftwareScenario = eSoftwareScenario_Main_Vfb_Stream_2;
            else
                eSoftwareScenario = eSoftwareScenario_Sub_Vfb_Stream_2;
            break;

        default:
            MY_ERR("Incorect ISP profile\n");
            break;
        }


        if  ( checkParamChange(m_eSoftwareScenario, eSoftwareScenario) )
        {
            MY_LOG("[+setSoftwareScenario](old, new)=(%d, %d)", m_eSoftwareScenario, eSoftwareScenario);
            m_eSoftwareScenario = eSoftwareScenario;
        }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setSceneMode(EIndex_Scene_T const eScene)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIdx_Scene, eScene) )
    {
        MY_LOG("[+setSceneMode] scene(old, new)=(%d, %d)", m_rIspCamInfo.eIdx_Scene, eScene);
        m_rIspCamInfo.eIdx_Scene = eScene;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setEffect(EIndex_Effect_T const eEffect)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIdx_Effect, eEffect) )
    {
        MY_LOG("[+setEffect] effect(old, new)=(%d, %d)", m_rIspCamInfo.eIdx_Effect, eEffect);
        m_rIspCamInfo.eIdx_Effect = eEffect;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setOperMode(EOperMode_T const eOperMode)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_eOperMode, eOperMode) )
    {
        MY_LOG("[+setOperMode](old, new)=(%d, %d)", m_eOperMode, eOperMode);
        m_eOperMode = eOperMode;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setSensorMode(ESensorMode_T const eSensorMode)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eSensorMode, eSensorMode) )
    {
        MY_LOG("[+setSensorMode](old, new)=(%d, %d)", m_rIspCamInfo.eSensorMode, eSensorMode);
        m_rIspCamInfo.eSensorMode = eSensorMode;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setZoomRatio(MINT32 const i4ZoomRatio_x100)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100) )
    {
        MY_LOG("[+setZoomRatio](old, new)=(%d, %d)", m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100);
        m_rIspCamInfo.i4ZoomRatio_x100 = i4ZoomRatio_x100;
    }

    return  MERR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAWBInfo(AWB_INFO_T const &rAWBInfo)
{
    MBOOL bAWBGainChanged = MFALSE;

    Mutex::Autolock lock(m_Lock);
#if 0
   if (checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4R, rAWBInfo.rCurrentAWBGain.i4R) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4G, rAWBInfo.rCurrentAWBGain.i4G) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4B, rAWBInfo.rCurrentAWBGain.i4B)) {
        bAWBGainChanged = MTRUE;
        MY_LOG_IF(m_bDebugEnable, "setAWBInfo(): bAWBGainChanged = MTRUE");
   }
#endif

   m_rIspCamInfo.rAWBInfo = rAWBInfo;

    // Dynamic CCM
    if (isDynamicCCM())
    {
        if (m_rIspParam.bInvokeSmoothCCM) { // smooth CCM
            MY_LOG_IF(m_bDebugEnable, "Smooth CCM");
            MY_LOG_IF(m_bDebugEnable, "is_to_invoke_smooth_ccm_with_preference_gain = %d", m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo));
            m_pCcmMgr->calculateCCM(rAWBInfo, m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo));
        }
        else { // dynamic CCM
            MY_LOG_IF(m_bDebugEnable, "Dynamic CCM");

            // Evaluate CCM index
            EIndex_CCM_T const eIdx_CCM_old = m_rIspCamInfo.eIdx_CCM;
            EIndex_CCM_T const eIdx_CCM_new = m_pIspTuningCustom->evaluate_CCM_index(m_rIspCamInfo);

            if ( checkParamChange(eIdx_CCM_old, eIdx_CCM_new) )
            {
                m_rIspCamInfo.eIdx_CCM = eIdx_CCM_new;
                m_pCcmMgr->setIdx(m_rIspCamInfo.eIdx_CCM);
                MY_LOG("[setAWBInfo][ParamChangeCount:%d]" "CCM index(old, new) =(%d, %d)", getParamChangeCount(), eIdx_CCM_old, eIdx_CCM_new);
            }
        }
    }

    // Evaluate PCA LUT index
    EIndex_PCA_LUT_T const eIdx_PCA_LUT_old = m_rIspCamInfo.eIdx_PCA_LUT;
    EIndex_PCA_LUT_T eIdx_PCA_LUT_new;

    if ((m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Capture)) {
        eIdx_PCA_LUT_new = m_pIspTuningCustom->evaluate_PCA_LUT_index_for_IHDR(m_rIspCamInfo);
    } else {
        eIdx_PCA_LUT_new = m_pIspTuningCustom->evaluate_PCA_LUT_index(m_rIspCamInfo);
    }

    if  ( checkParamChange(eIdx_PCA_LUT_old, eIdx_PCA_LUT_new) )
    {
        m_rIspCamInfo.eIdx_PCA_LUT = eIdx_PCA_LUT_new;
        MY_LOG("[setAWBInfo][ParamChangeCount:%d]" "PCA LUT index(old, new) =(%d, %d)", getParamChangeCount(), eIdx_PCA_LUT_old, eIdx_PCA_LUT_new);
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAWBGain(AWB_GAIN_T& rNewIspAWBGain)
{
#if 0
    MY_LOG_IF(m_bDebugEnable,"%s(): rNewIspAWBGain.i4R = %d, rNewIspAWBGain.i4G = %d, rNewIspAWBGain.i4B = %d\n",
              __FUNCTION__, rNewIspAWBGain.i4R, rNewIspAWBGain.i4G, rNewIspAWBGain.i4B);

    AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();

   if (checkParamChange(rCurrentIspAWBGain.i4R, rNewIspAWBGain.i4R) ||
       checkParamChange(rCurrentIspAWBGain.i4G, rNewIspAWBGain.i4G) ||
       checkParamChange(rCurrentIspAWBGain.i4B, rNewIspAWBGain.i4B)) {
       ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAWBGain(rNewIspAWBGain);
       ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rNewIspAWBGain);
   }
#endif

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{
    AWB_GAIN_T rCurrentIspAEPreGain2 = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).getIspAEPreGain2();

    if (checkParamChange(rCurrentIspAEPreGain2.i4R, rNewIspAEPreGain2.i4R) ||
        checkParamChange(rCurrentIspAEPreGain2.i4G, rNewIspAEPreGain2.i4G) ||
        checkParamChange(rCurrentIspAEPreGain2.i4B, rNewIspAEPreGain2.i4B)) {
        ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAEInfo(AE_INFO_T const &rAEInfo)
{
    MY_LOG_IF(m_bDebugEnable, "setAEInfo()");

    Mutex::Autolock lock(m_Lock);

    EIndex_ISO_T const eIdx_ISO = m_pIspTuningCustom->map_ISO_value_to_index(rAEInfo.u4RealISOValue);

    // ISO value
    if (checkParamChange(m_rIspCamInfo.u4ISOValue, rAEInfo.u4RealISOValue)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.u4ISOValue](old, new)=(%d, %d)", m_rIspCamInfo.u4ISOValue, rAEInfo.u4RealISOValue);
        m_rIspCamInfo.u4ISOValue = rAEInfo.u4RealISOValue;
    }

    // ISO index
    if (checkParamChange(m_rIspCamInfo.eIdx_ISO, eIdx_ISO)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.eIdx_ISO](old, new)=(%d, %d)", m_rIspCamInfo.eIdx_ISO, eIdx_ISO);
        m_rIspCamInfo.eIdx_ISO = eIdx_ISO;
    }

    // AE Stable Cnt : need to before LV
    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        //MY_LOG("[+m_rIspCamInfo.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)\n", m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        m_rIspCamInfo.rAEInfo.u4AEStableCnt = rAEInfo.u4AEStableCnt;
    }

    // LV
    if (checkParamChange(m_rIspCamInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.i4LightValue_x10](old, new)=(%d, %d)", m_rIspCamInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10);
        m_rIspCamInfo.i4LightValue_x10 = rAEInfo.i4LightValue_x10;

        MY_LOG_IF(m_bDebugEnable, "LV value(%d)",rAEInfo.i4LightValue_x10);
    }

    // ISP gain
    if (checkParamChange(m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4IspGain](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain);
        m_rIspCamInfo.rAEInfo.u4IspGain = rAEInfo.u4IspGain;
    }

    //check Flare offset
    if(checkParamChange(m_rIspCamInfo.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i2FlareOffset](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset);
        m_rIspCamInfo.rAEInfo.i2FlareOffset = rAEInfo.i2FlareOffset;
    }

    //check Gamma Index
    if(checkParamChange(m_rIspCamInfo.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i4GammaIdx](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx);
        m_rIspCamInfo.rAEInfo.i4GammaIdx = rAEInfo.i4GammaIdx;
    }

    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE) || checkParamChange(m_rIspCamInfo.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4SWHDR_SE](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE);
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i4LESE_Ratio](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio);
    }

    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO)) {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4MaxISO](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO);
        m_rIspCamInfo.rAEInfo.u4MaxISO = rAEInfo.u4MaxISO;
        // TBD
    }

    m_rIspCamInfo.rAEInfo = rAEInfo;
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setISPAEGain(MBOOL bEnableWorkAround, MUINT32 u4NewIspAEGain)
{
    MUINT32 u4CurrentIspGain;

    if(bEnableWorkAround == MTRUE) {   // enable iVHDR work around
        //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEGain(u4NewIspAEGain);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEGain(512);
        }
    } else {
        ISP_MGR_OBC_T::getInstance(m_eSensorDev).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEGain(512);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEGain(u4NewIspAEGain);
        }
    }
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspFlareGainOffset(MINT16 i2FlareGain, MINT16 i2FlareOffset)
{
    MY_LOG_IF(m_bDebugEnable, "setIspFlareGainOffset(gain, offset)=(%d, %d)", i2FlareGain, i2FlareOffset);

    ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspFlare(i2FlareGain, i2FlareOffset);
    ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspFlare(i2FlareGain, i2FlareOffset);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAFInfo(AF_INFO_T const &rAFInfo)
{
    MY_LOG_IF(m_bDebugEnable, "setAFInfo()");

    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.rAFInfo = rAFInfo;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFlashInfo(FLASH_INFO_T const &rFlashInfo)
{
    MY_LOG_IF(m_bDebugEnable, "setFlashInfo()");

    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.rFlashInfo = rFlashInfo;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setIndex_Shading(MINT32 const i4IDX)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] idx %d", __FUNCTION__, i4IDX);

    Mutex::Autolock lock(m_Lock);

    if (m_pLscMgr) {
        m_pLscMgr->setCTIdx(i4IDX);
    } else {
        MY_LOG_IF(m_bDebugEnable, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
getIndex_Shading(MVOID*const pCmdArg)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] idx %d", __FUNCTION__);

    Mutex::Autolock lock(m_Lock);

    if (m_pLscMgr) {
        *(MINT8*)pCmdArg = m_pLscMgr->getCTIdx();
    } else {
        MY_LOG_IF(m_bDebugEnable, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    return  MERR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Since OBC mixed with AE info, so it is required to backup OBC info
// for dynamic bypass
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MERROR_ENUM
Paramctrl::
setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo)
{
    memcpy(&m_backup_OBCInfo, pOBCInfo, sizeof(ISP_NVRAM_OBC_T));

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getPureOBCInfo(ISP_NVRAM_OBC_T *pOBCInfo)
{
    memcpy(pOBCInfo, &m_backup_OBCInfo, sizeof(ISP_NVRAM_OBC_T));

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTGInfo(MINT32 const i4TGInfo)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] i4TGInfo = %d", __FUNCTION__, i4TGInfo);

    Mutex::Autolock lock(m_Lock);

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    default:
        MY_ERR("i4TGInfo = %d", i4TGInfo);
        return MERR_BAD_PARAM;
    }

    m_pLscMgr->setTgInfo(m_eSensorTG);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setCamMode(MINT32 const i4CamMode)
{
    if (m_i4CamMode != i4CamMode) {
        m_i4CamMode = i4CamMode;
        MY_LOG("m_i4CamMode = %d", m_i4CamMode);

        switch (m_i4CamMode)
        {
        case eAppMode_EngMode:
             char fileName[] = "/sdcard/isp/isp_nvram_anr.table";
             char fileName2[] = "/sdcard/isp/isp_nvram_ccr.table";
             mkdir("/sdcard/isp", S_IRWXU | S_IRWXG | S_IRWXO);
             FILE *fp = fopen(fileName, "w");
             if (!fp) {
                MY_ERR("fopen fail: /sdcard/isp/isp_nvram_anr.table");
             }
             else {
                 fwrite(reinterpret_cast<void *>(&m_rIspParam.ISPRegs.ANR[0]), 1, sizeof(ISP_NVRAM_ANR_T)*NVRAM_ANR_TBL_NUM, fp);
                 fclose(fp);
             }

             fp = fopen(fileName2, "w");
             if (!fp) {
                MY_ERR("fopen fail: /sdcard/isp/isp_nvram_ccr.table");
             }
             else {
                 fwrite(reinterpret_cast<void *>(&m_rIspParam.ISPRegs.CCR[0]), 1, sizeof(ISP_NVRAM_CCR_T)*NVRAM_CCR_TBL_NUM, fp);
                 fclose(fp);
             }
             break;
        }
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                            MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                            MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    ISP_CCM_T rCCM;

    m_pCcmMgr->getCCM(rCCM);

    M11 = static_cast<MFLOAT>(rCCM.M11) / 512;
    M12 = static_cast<MFLOAT>(rCCM.M12) / 512;
    M13 = static_cast<MFLOAT>(rCCM.M13) / 512;
    M21 = static_cast<MFLOAT>(rCCM.M21) / 512;
    M22 = static_cast<MFLOAT>(rCCM.M22) / 512;
    M23 = static_cast<MFLOAT>(rCCM.M23) / 512;
    M31 = static_cast<MFLOAT>(rCCM.M31) / 512;
    M32 = static_cast<MFLOAT>(rCCM.M32) / 512;
    M33 = static_cast<MFLOAT>(rCCM.M33) / 512;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                            MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                            MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    ISP_CCM_T rCCM;

    rCCM.M11 = static_cast<MINT32>((M11 * 512) + 0.5);
    rCCM.M12 = static_cast<MINT32>((M12 * 512) + 0.5);
    rCCM.M13 = static_cast<MINT32>((M13 * 512) + 0.5);
    rCCM.M21 = static_cast<MINT32>((M21 * 512) + 0.5);
    rCCM.M22 = static_cast<MINT32>((M22 * 512) + 0.5);
    rCCM.M23 = static_cast<MINT32>((M23 * 512) + 0.5);
    rCCM.M31 = static_cast<MINT32>((M31 * 512) + 0.5);
    rCCM.M32 = static_cast<MINT32>((M32 * 512) + 0.5);
    rCCM.M33 = static_cast<MINT32>((M33 * 512) + 0.5);

    m_pCcmMgr->setColorCorrectionTransform(rCCM);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setColorCorrectionMode(MINT32 i4ColorCorrectionMode)
{
    MY_LOG("%s() %d\n", __FUNCTION__, i4ColorCorrectionMode);

    switch (i4ColorCorrectionMode)
    {
    case MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX;
         break;
    case MTK_COLOR_CORRECTION_MODE_FAST:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_FAST;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_FAST;
         break;
    case MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY;
         break;
    default:
         MY_ERR("Incorrect color correction mode = %d", i4ColorCorrectionMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setEdgeMode(MINT32 i4EdgeMode)
{
    MY_LOG("%s()\n", __FUNCTION__);

    switch (i4EdgeMode)
    {
    case MTK_EDGE_MODE_OFF:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_OFF;
         break;
    case MTK_EDGE_MODE_FAST:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_FAST;
         break;
    case MTK_EDGE_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_HIGH_QUALITY;
         break;
    default:
         MY_ERR("Incorrect edge mode = %d", i4EdgeMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setNoiseReductionMode(MINT32 i4NRMode)
{
    MY_LOG("%s()\n", __FUNCTION__);

    switch (i4NRMode)
    {
    case MTK_NOISE_REDUCTION_MODE_OFF:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_OFF;
         break;
    case MTK_NOISE_REDUCTION_MODE_FAST:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_FAST;
         break;
    case MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY;
         break;
    default:
         MY_ERR("Incorrect noise reduction mode = %d", i4NRMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setToneMapMode(MINT32 i4ToneMapMode)
{
    MY_LOG("%s()\n", __FUNCTION__);

    switch (i4ToneMapMode)
    {
    case MTK_TONEMAP_MODE_CONTRAST_CURVE:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
         break;
    case MTK_TONEMAP_MODE_FAST:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_FAST;
         break;
    case MTK_TONEMAP_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_HIGH_QUALITY;
         break;
    default:
         MY_ERR("Incorrect tone map mode = %d", i4ToneMapMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
convertPtPairsToGMA(const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    return m_pGgmMgr->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
}
