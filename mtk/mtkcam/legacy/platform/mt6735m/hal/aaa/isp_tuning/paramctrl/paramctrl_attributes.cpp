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
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
//#include <mtkcam/v3/hal/aaa_hal_common.h>
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
#include <dbg_isp_param.h>
#include <lsc_mgr2.h>
#include <isp_mgr.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <cutils/properties.h>

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
enableDynamicShading(MBOOL const fgEnable)
{
    MY_LOG_IF(m_bDebugEnable, "[+enableDynamicShading](old, new)=(%d, %d)", m_fgDynamicShading, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicShading, fgEnable) )
    {
        m_fgDynamicShading = fgEnable;
    }
}

MBOOL
Paramctrl::
getIspGamma(MUINT32* pIspGamma, MBOOL* pEnable) const
{
    ISP_NVRAM_GGM_T ggm;
    ISP_MGR_GGM_T::getInstance(m_eSensorDev).get(ggm);
    for (int i=0; i<GGM_LUT_SIZE; i++)
    {
        pIspGamma[i] = ggm.g_gmt.lut[i].G_GAMMA;
        if (!m_bDebugEnable) continue;

        if ((i%10 == 0) && (i != 140))
            MY_LOG("[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                i, i+9,
                pIspGamma[i], pIspGamma[i+1], pIspGamma[i+2], pIspGamma[i+3], pIspGamma[i+4],
                pIspGamma[i+5], pIspGamma[i+6], pIspGamma[i+7], pIspGamma[i+8], pIspGamma[i+9]);
        else if (i == 140)
            MY_LOG("[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d)",
                i, i+3,
                pIspGamma[i], pIspGamma[i+1], pIspGamma[i+2], pIspGamma[i+3]);

    }
    *pEnable = ISP_MGR_GGM_T::getInstance(m_eSensorDev).isEnable();

    return MTRUE;
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
setIspProfile(MINT32 const i4IspProfile)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_i4IspProfile/*m_rIspCamInfo.eIspProfile*/, i4IspProfile) )
    {
        MY_LOG("[+setIspProfile](old, new)=(%d, %d)", m_i4IspProfile, i4IspProfile);
        m_i4IspProfile = i4IspProfile;
    }

    // check software scenario
    ESoftwareScenario eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;

    if (m_eSensorDev == ESensorDev_Main)
        eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;
    else
        eSoftwareScenario = eSoftwareScenario_Sub_Normal_Stream;

    if ( checkParamChange(m_eSoftwareScenario, eSoftwareScenario) )
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

   char value[PROPERTY_VALUE_MAX] = {'\0'};
   property_get("debug.ccm.switch", value, "-1");
   MINT32 adb_switch = atoi(value);

   if (checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4R, rAWBInfo.rCurrentAWBGain.i4R) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4G, rAWBInfo.rCurrentAWBGain.i4G) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4B, rAWBInfo.rCurrentAWBGain.i4B)) {
        bAWBGainChanged = MTRUE;
        MY_LOG_IF(m_bDebugEnable, "setAWBInfo(): bAWBGainChanged = MTRUE");
   }

   m_rIspCamInfo.rAWBInfo = rAWBInfo;

    // Dynamic CCM
    if ( isDynamicCCM() &&
         bAWBGainChanged)
    {
        MINT32 ccm_switch = m_rIspParam.bInvokeSmoothCCM;
        if (adb_switch != -1) ccm_switch = adb_switch;

        if (ccm_switch) { // smooth CCM
            MBOOL bInvokeSmoothCCMwPrefGain = m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo);
            MINT32 i4SmoothMethod = m_pIspTuningCustom->get_CCM_smooth_method(m_rIspCamInfo);

            MY_LOG_IF(m_bDebugEnable, "[Smooth CCM] bInvokeSmoothCCMwPrefGain(%d), m_i4FlashOnOff(%d), i4SmoothMethod(%d), ccm_switch(%d)"
                , bInvokeSmoothCCMwPrefGain
                , m_i4FlashOnOff
                , i4SmoothMethod
                , ccm_switch);
            m_pCcmMgr->calculateCCM(rAWBInfo
                                  , bInvokeSmoothCCMwPrefGain
                                  , m_i4FlashOnOff
                                  , i4SmoothMethod);
        }
        else { // dynamic CCM
            MY_LOG_IF(m_bDebugEnable, "[Dynamic CCM] ccm_switch(%d)", ccm_switch);

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
    EIndex_PCA_LUT_T const eIdx_PCA_LUT_new = m_pIspTuningCustom->evaluate_PCA_LUT_index(m_rIspCamInfo);

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
    MY_LOG_IF(m_bDebugEnable,"%s(): rNewIspAWBGain.i4R = %d, rNewIspAWBGain.i4G = %d, rNewIspAWBGain.i4B = %d\n",
              __FUNCTION__, rNewIspAWBGain.i4R, rNewIspAWBGain.i4G, rNewIspAWBGain.i4B);

    AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();

   if (checkParamChange(rCurrentIspAWBGain.i4R, rNewIspAWBGain.i4R) ||
       checkParamChange(rCurrentIspAWBGain.i4G, rNewIspAWBGain.i4G) ||
       checkParamChange(rCurrentIspAWBGain.i4B, rNewIspAWBGain.i4B)) {
       //ISP_TEMP_MARK_OUT ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAWBGain(rNewIspAWBGain);
       ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rNewIspAWBGain);
       //ISP_TEMP_MARK_OUT ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setRmmGain(rNewIspAWBGain);
   }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{
/* ISP_TEMP_MARK_OUT
    AWB_GAIN_T rCurrentIspAEPreGain2 = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).getIspAEPreGain2();

    if (checkParamChange(rCurrentIspAEPreGain2.i4R, rNewIspAEPreGain2.i4R) ||
        checkParamChange(rCurrentIspAEPreGain2.i4G, rNewIspAEPreGain2.i4G) ||
        checkParamChange(rCurrentIspAEPreGain2.i4B, rNewIspAEPreGain2.i4B)) {
        ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
    }
*/
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
    if (checkParamChange(m_rIspCamInfo.u4ISOValue, rAEInfo.u4RealISOValue))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.u4ISOValue](old, new)=(%d, %d)", m_rIspCamInfo.u4ISOValue, rAEInfo.u4RealISOValue);
        m_rIspCamInfo.u4ISOValue = rAEInfo.u4RealISOValue;
    }

    // ISO index
    if (checkParamChange(m_rIspCamInfo.eIdx_ISO, eIdx_ISO))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.eIdx_ISO](old, new)=(%d, %d)", m_rIspCamInfo.eIdx_ISO, eIdx_ISO);
        m_rIspCamInfo.eIdx_ISO = eIdx_ISO;
    }

    // LV
    if (checkParamChange(m_rIspCamInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.i4LightValue_x10](old, new)=(%d, %d)", m_rIspCamInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10);
        m_rIspCamInfo.i4LightValue_x10 = rAEInfo.i4LightValue_x10;
    }

    // ISP gain
    if (checkParamChange(m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain))
    {
        MY_LOG_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4IspGain](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain);
        m_rIspCamInfo.rAEInfo.u4IspGain = rAEInfo.u4IspGain;
    }

    //check Flare offset
    checkParamChange(m_rIspCamInfo.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset);

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
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev/*ISP_TEMP_MARK_OUT, m_eSensorTG*/).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev/*ISP_TEMP_MARK_OUT, m_eSensorTG*/).setIspAEGain(u4NewIspAEGain);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev/*ISP_TEMP_MARK_OUT, m_eSensorTG*/).setIspAEGain(512);
        }
    } else {
        ISP_MGR_OBC_T::getInstance(m_eSensorDev).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev/*ISP_TEMP_MARK_OUT, m_eSensorTG*/).setIspAEGain(512);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev/*ISP_TEMP_MARK_OUT, m_eSensorTG*/).setIspAEGain(u4NewIspAEGain);
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

    //ISP_TEMP_MARK_OUT: ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspFlare(i2FlareGain, i2FlareOffset);
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFlashDynamicInfo(MINT32 const i4FlashOnOff)
{
    MY_LOG_IF(m_bDebugEnable, "setFlashDynamicInfo(%d)", i4FlashOnOff);

    Mutex::Autolock lock(m_Lock);

    m_i4FlashOnOff = i4FlashOnOff;

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
getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                            MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                            MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    ISP_CCM_T rCCM;

    m_pCcmMgr->getCCM(rCCM);

    M11 = static_cast<MFLOAT>(rCCM.M11) / 256;
    M12 = static_cast<MFLOAT>(rCCM.M12) / 256;
    M13 = static_cast<MFLOAT>(rCCM.M13) / 256;
    M21 = static_cast<MFLOAT>(rCCM.M21) / 256;
    M22 = static_cast<MFLOAT>(rCCM.M22) / 256;
    M23 = static_cast<MFLOAT>(rCCM.M23) / 256;
    M31 = static_cast<MFLOAT>(rCCM.M31) / 256;
    M32 = static_cast<MFLOAT>(rCCM.M32) / 256;
    M33 = static_cast<MFLOAT>(rCCM.M33) / 256;

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

    rCCM.M11 = static_cast<MINT32>((M11 * 256) + 0.5);
    rCCM.M12 = static_cast<MINT32>((M12 * 256) + 0.5);
    rCCM.M13 = static_cast<MINT32>((M13 * 256) + 0.5);
    rCCM.M21 = static_cast<MINT32>((M21 * 256) + 0.5);
    rCCM.M22 = static_cast<MINT32>((M22 * 256) + 0.5);
    rCCM.M23 = static_cast<MINT32>((M23 * 256) + 0.5);
    rCCM.M31 = static_cast<MINT32>((M31 * 256) + 0.5);
    rCCM.M32 = static_cast<MINT32>((M32 * 256) + 0.5);
    rCCM.M33 = static_cast<MINT32>((M33 * 256) + 0.5);

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
    //MY_LOG("%s() %d\n", __FUNCTION__, i4ColorCorrectionMode);

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
    //MY_LOG("%s()\n", __FUNCTION__);

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
    //MY_LOG("%s()\n", __FUNCTION__);

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
    //MY_LOG("%s(%d)\n", __FUNCTION__, i4ToneMapMode);

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
