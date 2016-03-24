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

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/hal/aaa_hal_base.h>
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
#include <gma_mgr.h>
#include <lsc_mgr2.h>
#include <dbg_isp_param.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <cutils/properties.h>

using namespace android;
using namespace NSIspTuning;

#define CTRL_BIT_OBC 1
#define CTRL_BIT_BNR 2

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_All()
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.paramctrl.enable", value, "0");
    m_bDebugEnable = atoi(value);

    MBOOL fgRet = MTRUE;

    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_apply.disable", value, "0");
    MINT32 m_disable = atoi(value);

    MY_LOG_IF(m_bDebugEnable,"[applyToHw_PerFrame_All] m_disable(%d)\n", m_disable);

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagStart); // Profiling Start.

    fgRet =
            ((m_disable & CTRL_BIT_OBC) ? MTRUE : ISP_MGR_OBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile))
        &&  ((m_disable & CTRL_BIT_BNR) ? MTRUE : ISP_MGR_BNR_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile))
        &&  ISP_MGR_LSC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_SL2_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PGN_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PCA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        ;

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagEnd);   // Profiling End.

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_All()
{
    MY_LOG_IF(m_bDebugEnable,"[prepareHw_PerFrame_All()] enter\n");
    MBOOL fgRet = MTRUE;

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_BNR_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).reset()
        //&&  ISP_MGR_PCA_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).reset()
            ;

    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (2) default
    prepareHw_PerFrame_Default();

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_PGN()
        &&  prepareHw_PerFrame_CFA()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_GGM()
        &&  prepareHw_PerFrame_ANR()
        &&  prepareHw_PerFrame_CCR()
        &&  prepareHw_PerFrame_PCA()
        &&  prepareHw_PerFrame_EE()
        &&  prepareHw_PerFrame_EFFECT()
            ;

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_All()] exit\n");
    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// For dynamic bypass application
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_Partial()
{
    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_Partial()] enter\n");
    MBOOL fgRet = MTRUE;

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).reset()
            ;

    //  Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= ISP_MGR_CCM_T::getInstance(getSensorDev()).reset();


    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (2) default
    //prepareHw_PerFrame_Default();

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_DynamicBypass_OBC()
        &&  prepareHw_PerFrame_PGN()
            ;

    //Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= prepareHw_PerFrame_CCM();


    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_Partial()] exit\n");
    return  fgRet;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
prepareHw_PerFrame_Default()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC()
{
    // Get default NVRAM parameter
    ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC();

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_OBC] obc_index = %d\n", m_IspNvramMgr.getIdx_OBC());

    this->setPureOBCInfo(&obc);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.obc_apply.ofst", value, "-1");
    MINT32 ob_val = atoi(value);
    property_get("debug.obc_apply.gain", value, "-1");
    MINT32 ob_g = atoi(value);

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_OBC] ob_val = %d\n", ob_val);

    if (ob_val != -1)
    {
        obc.offst0.val = obc.offst1.val = obc.offst2.val = obc.offst3.val = ob_val;
    }
    if (ob_g != -1)
    {
        obc.gain0.val = obc.gain1.val = obc.gain2.val = obc.gain3.val = ob_g;
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), obc );

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
    putIspHWBuf(getSensorDev(), obc );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC()
{
    // Get default NVRAM parameter
    ISP_NVRAM_BPC_T bpc = m_IspNvramMgr.getBPC();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_BPC(m_rIspCamInfo, m_IspNvramMgr, bpc);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), bpc );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1()
{
    // Get default NVRAM parameter
    ISP_NVRAM_NR1_T nr1 = m_IspNvramMgr.getNR1();

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_NR1] nr1_index = %d\n", m_IspNvramMgr.getIdx_NR1());

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR1(m_rIspCamInfo, m_IspNvramMgr, nr1);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), nr1 );

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
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.lsc_mgr.ratio", value, "-1");
    MINT32 i4Rto = atoi(value);



    // Check to see if it is needed to load LUT.
    MY_LOG_IF(m_bDebugEnable,"[%s] m_pLscMgr OperMode(%d)\n", __FUNCTION__, getOperMode());

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);

        if (i4Rto == -1)
        {
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(m_rIspCamInfo);
            MY_LOG("[%s] (flash, iso, rto) = (%d, %d, %d)", __FUNCTION__,
                m_rIspCamInfo.rFlashInfo.isFlash,
                m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto);
    }
    }
    else
    {
        if (i4Rto == -1) i4Rto = 32;
    }    

    m_pLscMgr->setLscRatio(i4Rto);


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

    getIspHWBuf(getSensorDev(), sl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_SL2(m_rIspCamInfo, m_IspNvramMgr, sl2);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), sl2 );

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN()
{
    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    getIspHWBuf(getSensorDev(), pgn );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_PGN(m_rIspCamInfo, m_IspNvramMgr, pgn);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), pgn );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CFA()
{
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

    if ((getOperMode() == EOperMode_Meta) && (!ISP_MGR_CFA_T::getInstance(getSensorDev()).isEnable())) // CCT usage: fix CFA index
    {
        cfa = m_IspNvramMgr.getCFA(NVRAM_CFA_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), cfa );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM()
{
    // Get default NVRAM parameter
    ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ccm );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM()
{
    // Get default NVRAM parameter
    ISP_NVRAM_GGM_T ggm = m_IspNvramMgr.getGGM();
    m_pGmaMgr->calculateGGM(&ggm, &m_GmaExifInfo);

    // Invoke callback for customers to modify.
    if ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_GGM(m_rIspCamInfo,  m_IspNvramMgr, ggm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ggm );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR()
{
    // Get default NVRAM parameter
    ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_ANR] anr_index = %d\n", m_IspNvramMgr.getIdx_ANR());

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
    putIspHWBuf(getSensorDev(), anr );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR()
{
    // Get default NVRAM parameter
    ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCR(m_rIspCamInfo, m_IspNvramMgr, ccr);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ccr );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA()
{
    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    // (2) Invoke callback for customers to modify.
    if  (isDynamicTuning())
    {   // Dynamic Tuning: Enable
        if (m_pIspTuningCustom->get_CCM_smooth_method(m_rIspCamInfo) == 0 /*0: CCM (without flash info)*/)
            m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));
        else //1: enable flash CCM
        {
            EIndex_PCA_LUT eIdx_flash_PCA = m_rIspCamInfo.eIdx_PCA_LUT;
            if (m_i4FlashOnOff)
                switch (m_rIspCamInfo.eIdx_PCA_LUT)
                {
                    //here we only convert normal PCA to flash PCA, keep IHDR PCA unchanged
                    case eIDX_PCA_LOW:
                        eIdx_flash_PCA = eIDX_PCA_LOW_3;
                        break;
                    case eIDX_PCA_MIDDLE:
                        eIdx_flash_PCA = eIDX_PCA_MIDDLE_3;
                        break;
                    case eIDX_PCA_HIGH:
                        eIdx_flash_PCA = eIDX_PCA_HIGH_3;
                        break;
                    default:
                        MY_LOG_IF(1, "[prepareHw_PerFrame_PCA] eIdx_PCA_LUT(%d) unchanged\n", m_rIspCamInfo.eIdx_PCA_LUT);
                }

            MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_PCA] eIdx_flash_PCA(%d), m_i4FlashOnOff(%d), smooth_method(%d)\n"
                , eIdx_flash_PCA
                , m_i4FlashOnOff
                , m_pIspTuningCustom->get_CCM_smooth_method(m_rIspCamInfo));
            m_pPcaMgr->setIdx(static_cast<MUINT32>(eIdx_flash_PCA));
        }
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

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE()
{
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
    putIspHWBuf(getSensorDev(), ee);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT()
{
    ISP_NVRAM_G2C_T g2c;
    ISP_NVRAM_SE_T se;
    ISP_NVRAM_GGM_T ggm;

    // Get ISP HW buffer
    getIspHWBuf(getSensorDev(), g2c);
    getIspHWBuf(getSensorDev(), se);
    getIspHWBuf(getSensorDev(), ggm);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_eIdx_Effect, m_IspUsrSelectLevel, g2c, se, ggm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), g2c);
    putIspHWBuf(getSensorDev(), se);
    putIspHWBuf(getSensorDev(), ggm);

    return  MTRUE;
}


