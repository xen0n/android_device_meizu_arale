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
//#include <mtkcam/v3/hal/aaa_hal_common.h>
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
#include <ccm_mgr.h>
#include <lsc_mgr2.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <mtkcam/featureio/tuning_mgr.h>

using namespace android;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_All(MINT32 const i4FrameID)
{
    MBOOL fgRet = MTRUE;

    MY_LOG_IF(m_bDebugEnable, "[%s] + i4FrameID(%d)", __FUNCTION__, i4FrameID);

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagStart); // Profiling Start.

    TuningMgr::getInstance().dequeBuffer(m_eSoftwareScenario, i4FrameID);

    fgRet = ISP_MGR_OBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_BNR_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LSC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_SL2_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_PGN_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_PCA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
      //  &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        ;

    TuningMgr::getInstance().enqueBuffer(m_eSoftwareScenario, i4FrameID);

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagEnd);   // Profiling End.

    MY_LOG_IF(m_bDebugEnable, "[%s] - i4FrameID(%d)", __FUNCTION__, i4FrameID);

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_P2(MINT32 flowType, const RAWIspCamInfo& rIspCamInfo, void* pRegBuf)
{
    MBOOL fgRet = MTRUE;

    MY_LOG_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagStart); // Profiling Start.

    isp_reg_t* pReg = reinterpret_cast<isp_reg_t*>(pRegBuf);

    if (flowType == 1)
    {
        MY_LOG("[%s] PureRaw Flow", __FUNCTION__);
        fgRet = ISP_MGR_OBC_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_BNR_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_LSC_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg);
    }

    fgRet = ISP_MGR_SL2_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PGN_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).apply(rIspCamInfo.eIspProfile, pReg)
        ;

    CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_APPLY, CPTFlagEnd);   // Profiling End.

    MY_LOG_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);

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
//        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).reset()
//        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).reset()
//        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).reset()
//        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).reset()
//        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).reset()
        //&&  ISP_MGR_PCA_T::getInstance(getSensorDev()).reset()
//        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).reset()
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
//        &&  prepareHw_PerFrame_PGN()
//        &&  prepareHw_PerFrame_CFA()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_GGM()
//        &&  prepareHw_PerFrame_ANR()
//        &&  prepareHw_PerFrame_CCR()
//        &&  prepareHw_PerFrame_PCA()
//        &&  prepareHw_PerFrame_EE()
//        &&  prepareHw_PerFrame_EFFECT()
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

MBOOL
Paramctrl::
prepareHw_PerFrame_All(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);

    MBOOL fgRet = MTRUE
//            &&  prepareHw_PerFrame_NR1(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_PGN(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_CFA(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_CCM(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_GGM(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_ANR(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_CCR(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_PCA(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_EE(rIspCamInfo, rIdx)
            &&  prepareHw_PerFrame_EFFECT(rIspCamInfo, rIdx)
                ;

    MY_LOG_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);

    return fgRet;
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

MBOOL
Paramctrl::
prepareHw_PerFrame_Partial(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    return MTRUE;
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
    m_rIspCamInfo.rMtkCCM = m_pCcmMgr->getCCM();
    #if 0
    // Get default NVRAM parameter
    ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ccm );
    #endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM()
{
    if (m_rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        m_rIspCamInfo.rMtkGGM = m_pGgmMgr->getGGM();
        m_rIspCamInfo.fgToneMapInvert = m_pGgmMgr->isNeedInvert();
    }
    else
    {
        ISP_NVRAM_GGM_T ggm;
        // Get default NVRAM parameter
        ggm = m_IspNvramMgr.getGGM();
        m_rIspCamInfo.fgToneMapInvert = MFALSE;

        // Invoke callback for customers to modify.
        if ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_GGM(m_rIspCamInfo,  m_IspNvramMgr, ggm);
        }

        m_rIspCamInfo.rMtkGGM = ggm;
        m_pGgmMgr->updateGGM(ggm);
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
    // Get default NVRAM parameter
    ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_ANR] anr_index = %d\n", m_IspNvramMgr.getIdx_ANR());

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

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_EE(m_rIspCamInfo, m_IspNvramMgr, ee);

        if (m_rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
        {
            // User setting
            m_pIspTuningCustom->userSetting_EE(m_rIspCamInfo, m_rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);

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
        m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_rIspCamInfo.eIdx_Effect, m_rIspCamInfo.rIspUsrSelectLevel, g2c, se, ggm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), g2c);
    putIspHWBuf(getSensorDev(), se);
    putIspHWBuf(getSensorDev(), ggm);

    return  MTRUE;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    MUINT8 u1Idx = rIdx.getIdx_NR1();
    ISP_NVRAM_NR1_T nr1 = m_IspNvramMgr.getNR1(u1Idx);

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_NR1] nr1_index = %d\n", u1Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR1(rIspCamInfo, m_IspNvramMgr, nr1);
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
prepareHw_PerFrame_SL2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    ISP_NVRAM_SL2_T sl2;

    getIspHWBuf(getSensorDev(), sl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_SL2(rIspCamInfo, m_IspNvramMgr, sl2);
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
prepareHw_PerFrame_PGN(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    AWB_GAIN_T rCurrentAWBGain = rIspCamInfo.rAWBInfo.rCurrentAWBGain;

    ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rCurrentAWBGain);

    getIspHWBuf(getSensorDev(), pgn );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_PGN(rIspCamInfo, m_IspNvramMgr, pgn);
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
prepareHw_PerFrame_CFA(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    MUINT8 u1Idx = rIdx.getIdx_CFA();
    ISP_NVRAM_CFA_T cfa = m_IspNvramMgr.getCFA(u1Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CFA(rIspCamInfo, m_IspNvramMgr, cfa);
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
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    //ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();
    ISP_NVRAM_CCM_T ccm = rIspCamInfo.rMtkCCM;

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCM(rIspCamInfo, m_IspNvramMgr, ccm);
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
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    putIspHWBuf(getSensorDev(), rIspCamInfo.rMtkGGM);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    MUINT8 u1Idx = rIdx.getIdx_ANR();
    ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(u1Idx);

    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_ANR] anr_index = %d\n", u1Idx);

    // check noise reduction mode
    if (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) {
        ISP_MGR_NBC_T::getInstance(getSensorDev()).setANREnable(MFALSE);
    }
    else {
        ISP_MGR_NBC_T::getInstance(getSensorDev()).setANREnable(MTRUE);
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
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
prepareHw_PerFrame_CCR(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    MUINT8 u1Idx = rIdx.getIdx_CCR();
    ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR(u1Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCR(rIspCamInfo, m_IspNvramMgr, ccr);
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
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        m_pPcaMgr->setIdx(static_cast<MUINT32>(rIspCamInfo.eIdx_PCA_LUT));
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
prepareHw_PerFrame_EE(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    MUINT8 u1Idx = rIdx.getIdx_EE();
    ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE(u1Idx);

    // check edge mode
    if (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF) {
        ISP_MGR_SEEE_T::getInstance(getSensorDev()).setEnable(MFALSE);
    }
    else {
        ISP_MGR_SEEE_T::getInstance(getSensorDev()).setEnable(MTRUE);
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_EE(rIspCamInfo, m_IspNvramMgr, ee);

        if (m_rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
        {
            // User setting
            m_pIspTuningCustom->userSetting_EE(m_rIspCamInfo, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);

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
prepareHw_PerFrame_EFFECT(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        m_pIspTuningCustom->userSetting_EFFECT(rIspCamInfo, rIspCamInfo.eIdx_Effect, rIspCamInfo.rIspUsrSelectLevel, g2c, se, ggm);
    }

#define TO_INT(a, bit) ((MINT32)((MINT32) (a)) << (32-(bit)) >> (32-(bit)))
#define MASK_VAL(a, bit) ((MUINT32)((MUINT32)(a)) & ((1U<<(bit))-1))
#define MIN_(a, b) ((a) < (b) ? (a) : (b))
#define MAX_(a, b) ((a) >= (b) ? (a) : (b))
#define CLIP(a, lo, hi) MIN_(MAX_(a, lo), hi)

    MINT32 G2C_A_CNV_00 = TO_INT(g2c.conv_0a.bits.G2C_CNV00, 11);
    MINT32 G2C_A_CNV_01 = TO_INT(g2c.conv_0a.bits.G2C_CNV01, 11);
    MINT32 G2C_A_CNV_02 = TO_INT(g2c.conv_0b.bits.G2C_CNV02, 11);
    MINT32 G2C_A_CNV_10 = TO_INT(g2c.conv_1a.bits.G2C_CNV10, 11);
    MINT32 G2C_A_CNV_11 = TO_INT(g2c.conv_1a.bits.G2C_CNV11, 11);
    MINT32 G2C_A_CNV_12 = TO_INT(g2c.conv_1b.bits.G2C_CNV12, 11);
    MINT32 G2C_A_CNV_20 = TO_INT(g2c.conv_2a.bits.G2C_CNV20, 11);
    MINT32 G2C_A_CNV_21 = TO_INT(g2c.conv_2a.bits.G2C_CNV21, 11);
    MINT32 G2C_A_CNV_22 = TO_INT(g2c.conv_2b.bits.G2C_CNV22, 11);
    MINT32 G2C_A_Y_OFST = TO_INT(g2c.conv_0b.bits.G2C_YOFFSET11, 11);
    MINT32 G2C_A_U_OFST = TO_INT(g2c.conv_1b.bits.G2C_UOFFSET10, 10);
    MINT32 G2C_A_V_OFST = TO_INT(g2c.conv_2b.bits.G2C_VOFFSET10, 10);

    MINT32 i4MonoFlgR = (rIspCamInfo.fgToneMapInvert & 0x1) ? 1 : 0;
    MINT32 i4MonoFlgG = (rIspCamInfo.fgToneMapInvert & 0x2) ? 1 : 0;
    MINT32 i4MonoFlgB = (rIspCamInfo.fgToneMapInvert & 0x4) ? 1 : 0;

    // invert g2c
    G2C_A_Y_OFST += (G2C_A_CNV_00*i4MonoFlgR + G2C_A_CNV_01*i4MonoFlgG + G2C_A_CNV_02*i4MonoFlgB)*1023/512;
    G2C_A_U_OFST += (G2C_A_CNV_10*i4MonoFlgR + G2C_A_CNV_11*i4MonoFlgG + G2C_A_CNV_12*i4MonoFlgB)*1023/512;
    G2C_A_V_OFST += (G2C_A_CNV_20*i4MonoFlgR + G2C_A_CNV_21*i4MonoFlgG + G2C_A_CNV_22*i4MonoFlgB)*1023/512;

    G2C_A_Y_OFST = CLIP(G2C_A_Y_OFST, -1024, 1023);
    G2C_A_U_OFST = CLIP(G2C_A_U_OFST, -512, 511);
    G2C_A_V_OFST = CLIP(G2C_A_V_OFST, -512, 511);

    MY_LOG_IF(m_bDebugEnable, "[%s] Inv(0x%x) YUV(%d, %d, %d)", __FUNCTION__, rIspCamInfo.fgToneMapInvert, G2C_A_Y_OFST, G2C_A_U_OFST, G2C_A_V_OFST);

    if (i4MonoFlgR)
    {
        G2C_A_CNV_00 = -G2C_A_CNV_00;
        G2C_A_CNV_10 = -G2C_A_CNV_10;
        G2C_A_CNV_20 = -G2C_A_CNV_20;
    }
    if (i4MonoFlgG)
    {
        G2C_A_CNV_01 = -G2C_A_CNV_01;
        G2C_A_CNV_11 = -G2C_A_CNV_11;
        G2C_A_CNV_21 = -G2C_A_CNV_21;
    }
    if (i4MonoFlgB)
    {
        G2C_A_CNV_02 = -G2C_A_CNV_02;
        G2C_A_CNV_12 = -G2C_A_CNV_12;
        G2C_A_CNV_22 = -G2C_A_CNV_22;
    }
    g2c.conv_0b.bits.G2C_YOFFSET11 = MASK_VAL(G2C_A_Y_OFST, 11);
    g2c.conv_1b.bits.G2C_UOFFSET10 = MASK_VAL(G2C_A_U_OFST, 10);
    g2c.conv_2b.bits.G2C_VOFFSET10 = MASK_VAL(G2C_A_V_OFST, 10);

    g2c.conv_0a.bits.G2C_CNV00 = MASK_VAL(G2C_A_CNV_00, 11);
    g2c.conv_0a.bits.G2C_CNV01 = MASK_VAL(G2C_A_CNV_01, 11);
    g2c.conv_0b.bits.G2C_CNV02 = MASK_VAL(G2C_A_CNV_02, 11);

    g2c.conv_1a.bits.G2C_CNV10 = MASK_VAL(G2C_A_CNV_10, 11);
    g2c.conv_1a.bits.G2C_CNV11 = MASK_VAL(G2C_A_CNV_11, 11);
    g2c.conv_1b.bits.G2C_CNV12 = MASK_VAL(G2C_A_CNV_12, 11);

    g2c.conv_2a.bits.G2C_CNV20 = MASK_VAL(G2C_A_CNV_20, 11);
    g2c.conv_2a.bits.G2C_CNV21 = MASK_VAL(G2C_A_CNV_21, 11);
    g2c.conv_2b.bits.G2C_CNV22 = MASK_VAL(G2C_A_CNV_22, 11);

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), g2c);
    putIspHWBuf(getSensorDev(), se);
    putIspHWBuf(getSensorDev(), ggm);

    return  MTRUE;
}


