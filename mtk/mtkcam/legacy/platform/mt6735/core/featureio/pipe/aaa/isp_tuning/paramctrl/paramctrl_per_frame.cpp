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
#define LOG_TAG "paramctrl_per_frame"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>
#include <mtkcam/algorithm/lib3a/isp_interpolation.h>
#include <ccm_mgr.h>
#include <lsc_mgr2.h>
#include "paramctrl.h"
#include <mtkcam/hwutils/CameraProfile.h>
#include "mtkcam/featureio/vfb_hal_base.h"
#include <mtkcam/featureio/capturenr.h>

using namespace android;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_All(MINT32 const i4FrameID)
{
    MBOOL fgRet = MTRUE;

    MY_LOG_IF(m_bDebugEnable,"[%s] i4FrameID(0x%8x)\n", __FUNCTION__, i4FrameID);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa.pvlog.enable", value, "0");
    int r3ALogEnable = atoi(value);
    AaaTimer localTimer("applyToHwAll", m_eSensorDev, (r3ALogEnable & EN_3A_SCHEDULE_LOG));

    TuningMgr::getInstance().dequeBuffer(m_eSoftwareScenario, i4FrameID);

    fgRet = ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)  //
        &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CFA_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        ;

    TuningMgr::getInstance().enqueBuffer(m_eSoftwareScenario, i4FrameID);

    localTimer.End();

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_All()
{
    MBOOL fgRet = MTRUE;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_CFA_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).reset()
        ;

    if  ( ! fgRet )
    {
        MY_ERR("reset error");
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_RPG()
        &&  prepareHw_PerFrame_PGN()
        &&  prepareHw_PerFrame_CFA()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_GGM()
        &&  prepareHw_PerFrame_ANR()
        &&  prepareHw_PerFrame_CCR()
        &&  prepareHw_PerFrame_PCA()
        &&  prepareHw_PerFrame_EE()
        &&  prepareHw_PerFrame_EFFECT()
        &&  prepareHw_PerFrame_NR3D()
        &&  prepareHw_PerFrame_MFB()
        &&  prepareHw_PerFrame_MIXER3()
        &&  prepareHw_PerFrame_LCE()
        ;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        MY_ERR("prepareHw error");
        goto lbExit;
    }

lbExit:
    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// For dynamic bypass application
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_Partial()
{
    MBOOL fgRet = MTRUE;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).reset()
            ;

    //  Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= ISP_MGR_CCM_T::getInstance(m_eSensorDev).reset();


    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_DynamicBypass_OBC()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_PGN()
            ;

    //Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= prepareHw_PerFrame_CCM();


    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    return  fgRet;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.obc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
        }

        this->setPureOBCInfo(&obc);

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, obc );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_DynamicBypass_OBC()
{
    // Get backup NVRAM parameter
    ISP_NVRAM_OBC_T obc;

    this->getPureOBCInfo(&obc);

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, m_eSensorTG, obc );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.bpc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTBPCEnable())){
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BPC_T bpc = m_IspNvramMgr.getBPC();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BPC(m_rIspCamInfo, m_IspNvramMgr, bpc);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, bpc );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1()
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.nr1.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTCTEnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)) {
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_NR1_T nr1 = m_IspNvramMgr.getNR1();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR1(m_rIspCamInfo, m_IspNvramMgr, nr1);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, nr1 );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC()
{
    MY_LOG_IF(m_bDebugEnable,"%s", __FUNCTION__);

    //////////////////////////////////////
    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;

    // Check to see if it is needed to load LUT.
    MY_LOG_IF(m_bDebugEnable,"[%s] m_pLscMgr OperMode(%d)\n", __FUNCTION__, getOperMode());

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);
    }

    if (!m_pLscMgr->getTsfOnOff())
        m_pLscMgr->updateLsc();

    // debug message
    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    m_IspNvramMgr.setIdx_LSC(m_pLscMgr->getRegIdx());
    //////////////////////////////////////
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_SL2()
{
    // Get default NVRAM parameter
    ISP_NVRAM_SL2_T sl2;

    getIspHWBuf(m_eSensorDev, sl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_SL2(m_rIspCamInfo, m_IspNvramMgr, sl2);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, sl2 );

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RPG()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.rpg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    // Get default NVRAM parameter
    ISP_NVRAM_RPG_T rpg;

    if (isRPGEnable() && (!bDisable)) {// RPG is enabled
        ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, m_eSensorTG, rpg );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RPG(m_rIspCamInfo, m_IspNvramMgr, rpg);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, rpg );
    }
    else {
        ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pgn.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"%s(): isRPGEnable() = %d\n", __FUNCTION__, isRPGEnable());

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

#if ISP_P1_UPDATE
    if (isRPGEnable() || (bDisable)) {// RPG is enabled
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else
#endif
    {
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, pgn );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PGN(m_rIspCamInfo, m_IspNvramMgr, pgn);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, pgn );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CFA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.cfa.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        return MTRUE;
    }

    // Get default NVRAM parameter
    ISP_NVRAM_CFA_T cfa = m_IspNvramMgr.getCFA();

    if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
        if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
             if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA()+1;
                m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA();

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                          m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                          m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                          cfa);  // Output
             }
         }
         else {
             if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA();
                m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA()-1;

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                cfa);  // Output
             }
         }
    }
    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CFA(m_rIspCamInfo, m_IspNvramMgr, cfa);
    }

    // FG mode protection
    if (isRPGEnable() && ((m_rIspCamInfo.eIspProfile == EIspProfile_Preview)      ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_Video)        ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)    ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
        cfa.byp.bits.DM_FG_MODE = 1;
    }
    else {
        cfa.byp.bits.DM_FG_MODE = 0;
    }


    if ((!ISP_MGR_CFA_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
    {
        cfa = m_IspNvramMgr.getCFA(NVRAM_CFA_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, cfa );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccm );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ggm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_NVRAM_GGM_T ggm;

    if ((bDisable) ||
        (!ISP_MGR_GGM_T::getInstance(m_eSensorDev).isCCTEnable())                  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)            ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)             ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)             ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)             ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
        )
    {
        ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        if ((m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
            (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
            (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
            (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video)) {
            ggm = m_IspNvramMgr.getIHDRGGM(m_rIspCamInfo.rAEInfo.i4GammaIdx);
            MY_LOG_IF(m_bDebugEnable,"%s: m_rIspCamInfo.rAEInfo.i4GammaIdx = %d", __FUNCTION__, m_rIspCamInfo.rAEInfo.i4GammaIdx);
        }
        else {
            ggm = m_IspNvramMgr.getGGM();
        }

        // Invoke callback for customers to modify.
        if ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_GGM(m_rIspCamInfo,  m_IspNvramMgr, ggm);
        }
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->userSetting_EFFECT_GGM(m_rIspCamInfo, m_eIdx_Effect, ggm);
    }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ggm );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.anr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(m_rIspCamInfo);

    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR))
    #else
    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR))
    #endif
    {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MFALSE);
    }
    else if ((m_rIspCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR)    ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR)   ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)   ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR)    ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR)   ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)
            )
        {

        if (m_rIspCamInfo.u4ISOValue >= m_u4SwnrEncEnableIsoThreshold) {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MTRUE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }
        else {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MFALSE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(m_rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }
    else {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(m_rIspCamInfo, m_IspNvramMgr, anr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ccr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)
       )
    {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCR(m_rIspCamInfo, m_IspNvramMgr, ccr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccr );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pca.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    if (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }

    if ((bDisable) ||
        (!ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).isCCTEnable())  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2))
    {
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MTRUE);
    }


    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
        MY_LOG_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
    }
    else {
        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {   // Dynamic Tuning: Enable
            m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));
        }

        // Check to see if it is needed to load LUT.
        switch  (getOperMode())
        {
        case EOperMode_Normal:
        case EOperMode_PureRaw:
            fgIsToLoadLut = m_pPcaMgr->isChanged();   // Load if changed.
            break;
        default:
            fgIsToLoadLut = MTRUE;                  // Force to load.
            break;
        }

        if (fgIsToLoadLut) {
            m_pPcaMgr->loadLut();
            m_pPcaMgr->loadConfig();
        }
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ee.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_SEEE_T::getInstance(m_eSensorDev).isCCTEnable())             ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off)          ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off)          ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE()+1;
                    m_rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE();
                    m_rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_EE(m_rIspCamInfo, m_IspNvramMgr, ee);

            if (m_IspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(m_rIspCamInfo, m_IspUsrSelectLevel.eIdx_Edge, ee);
            }
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ee);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.g2c.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_G2C_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        //}

        ISP_NVRAM_G2C_T g2c;
        ISP_NVRAM_G2C_SHADE_T g2c_shade;
        ISP_NVRAM_SE_T se;
        ISP_NVRAM_GGM_T ggm;

        // Get ISP HW buffer
        getIspHWBuf(m_eSensorDev, g2c);
        getIspHWBuf(m_eSensorDev, g2c_shade);
        getIspHWBuf(m_eSensorDev, se);
//return to GGM setting        getIspHWBuf(m_eSensorDev, ggm);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_eIdx_Effect, m_IspUsrSelectLevel, g2c, g2c_shade, se, ggm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, g2c);
        putIspHWBuf(m_eSensorDev, g2c_shade);
        putIspHWBuf(m_eSensorDev, se);
//return to GGM setting        putIspHWBuf(m_eSensorDev, ggm);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D()
{
    // Get default NVRAM parameter
    ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR3D(m_rIspCamInfo, m_IspNvramMgr, nr3d);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, nr3d );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB()
{
    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MFB(m_rIspCamInfo, m_IspNvramMgr, mfb);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mfb );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MIXER3()
{
#if 0
    // Get default NVRAM parameter
    ISP_NVRAM_MIXER3_T mixer3 = m_rIspParam.ISPMfbMixer;
#else
    // Since the original design can't apply mixer3 by ISO, and the nvram of mfb mix has never been used.
    // We steal the nvram of mfb mix and apply it to mixer3.
    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB();
    ISP_NVRAM_MIXER3_T mixer3;
    //
    mixer3.ctrl_0.bits.MIX3_WT_SEL = 1;
    mixer3.ctrl_0.bits.MIX3_B0 = mfb.ll_con6.bits.BLD_LL_MX_B0;
    mixer3.ctrl_0.bits.MIX3_B1 = mfb.ll_con6.bits.BLD_LL_MX_B1;
    mixer3.ctrl_0.bits.MIX3_DT = mfb.ll_con5.bits.BLD_LL_MX_DT;
    //
    mixer3.ctrl_1.bits.MIX3_M0 = mfb.ll_con5.bits.BLD_LL_MX_M0;
    mixer3.ctrl_1.bits.MIX3_M1 = mfb.ll_con5.bits.BLD_LL_MX_M1;
#endif

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MIXER3(m_rIspCamInfo, m_IspNvramMgr, mixer3);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mixer3 );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE()
{
    // Get default NVRAM parameter
    ISP_NVRAM_LCE_T lce = m_IspNvramMgr.getLCE();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_LCE(m_rIspCamInfo, m_IspNvramMgr, lce);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, lce);

    return  MTRUE;
}


