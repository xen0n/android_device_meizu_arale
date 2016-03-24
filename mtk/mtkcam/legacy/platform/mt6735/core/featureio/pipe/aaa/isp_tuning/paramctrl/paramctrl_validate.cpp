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
#define LOG_TAG "paramctrl_validate"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include "paramctrl.h"
#include <mtkcam/hwutils/CameraProfile.h>

using namespace android;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validate(MINT32 const i4FrameID, MBOOL const fgForce)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MY_LOG_IF(m_bDebugEnable,"%s(): i4FrameID = %d\n", __FUNCTION__, i4FrameID);

    // flush turning setting
    TuningMgr::getInstance().flushSetting(m_eSoftwareScenario);


    MBOOL const fgRet = ( MERR_OK == (err = validateFrameless()) )
                    &&  ( MERR_OK == (err = validatePerFrame(i4FrameID, fgForce)) )
                        ;

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validateFrameless()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MY_LOG("[+validateFrameless]");

    Mutex::Autolock lock(m_Lock);

    //  (1)
    if  ( ! prepareHw_Frameless_All() )
    {
        err = MERR_PREPARE_HW;
        goto lbExit;
    }

    //  (2)
    if  ( ! applyToHw_Frameless_All() )
    {
        err = MERR_APPLY_TO_HW;
        goto lbExit;
    }

    //  (3) Force validatePerFrame() to run.
    m_u4ParamChangeCount++;

    err = MERR_OK;

lbExit:
#if ENABLE_MY_ERR
    if  ( MERR_OK != err )
    {
        MY_ERR("[-validateFrameless]err(%X)", err);
    }
#endif

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrame(MINT32 const i4FrameID, MBOOL const fgForce)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MY_LOG_IF(m_bDebugEnable, "[validatePerFrame] i4FrameID = %d", i4FrameID);

    Mutex::Autolock lock(m_Lock);

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        MY_LOG("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    //  (0) Make sure it's really needed to apply.
    if  ( 0 == getParamChangeCount()  //  no params change
          && !fgForce)                //  not force to apply
    {
        err = MERR_OK;
        TuningMgr::getInstance().byPassSetting(m_eSoftwareScenario, i4FrameID);
        goto lbExit;
    }

    MY_LOG_IF(m_bDebugEnable, "[validatePerFrame](ParamChangeCount, fgForce)=(%d, %d)", getParamChangeCount(), fgForce);

    //  (1) reset to 0 since all params have been applied.
    resetParamChangeCount();

    //  (2) Do something.
    err = do_validatePerFrame(i4FrameID);
    if  (MERR_OK != err)
    {
        MY_ERR("[validatePerFrame]do_validatePerFrame returns err(%d)", err);
        goto lbExit;
    }

    err = MERR_OK;

lbExit:

    return  err;
}

MVOID*
Paramctrl::getDefaultISPIndex(MUINT32 eIspProfile, MUINT32 eSensorMode, MUINT32 eIdx_ISO)
{
    MY_LOG_IF(1, "eIspProfile = %d, eSensorMode = %d, eIdx_ISO = %d", eIspProfile, eSensorMode, eIdx_ISO);

    INDEX_T const* pDefaultIndex = m_pIspTuningCustom->getDefaultIndex(
        static_cast<EIspProfile_T>(eIspProfile), static_cast<ESensorMode_T>(eSensorMode), EIndex_Scene_T(0), static_cast<EIndex_ISO_T>(eIdx_ISO) );

    return reinterpret_cast<MVOID*>(const_cast<INDEX_T *>(pDefaultIndex));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrame(MINT32 const i4FrameID)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy;
    MINT32 time_ms;

    MY_LOG_IF(m_bDebugEnable, "[do_validatePerFrame]");

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, CPTFlagStart); // Profiling Start.
    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: nvram index", m_eSensorDev, m_bProfileEnable);

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {

        IndexMgr idxmgr;

        MY_LOG_IF(m_bDebugEnable, "eIspProfile = %d, eSensorMode = %d, eIdx_Scene = %d, eIdx_ISO = %d",
                  m_rIspCamInfo.eIspProfile, m_rIspCamInfo.eSensorMode, m_rIspCamInfo.eIdx_Scene, m_rIspCamInfo.eIdx_ISO);

        // Refine camera info.
        m_pIspTuningCustom->refine_CamInfo(m_rIspCamInfo);

        //  a) Get default index setting.
        INDEX_T const*const pDefaultIndex = m_pIspTuningCustom->getDefaultIndex(
            m_rIspCamInfo.eIspProfile, m_rIspCamInfo.eSensorMode, m_rIspCamInfo.eIdx_Scene, m_rIspCamInfo.eIdx_ISO
        );
        if  ( ! pDefaultIndex )
        {
            MY_ERR("[ERROR][validatePerFrame]pDefaultIndex==NULL");
            err = MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND;
            goto lbExit;
        }
        idxmgr = *pDefaultIndex;

        MY_LOG_IF(m_bDebugEnable, "[BEFORE][evaluate_nvram_index]");

        if (m_bDebugEnable) {
            idxmgr.dump();
        }

        //  b) Customize the index setting.
        m_pIspTuningCustom->evaluate_nvram_index(m_rIspCamInfo, idxmgr);

        MY_LOG_IF(m_bDebugEnable, "[AFTER][evaluate_nvram_index]");

        if (m_bDebugEnable) {
            idxmgr.dump();
        }

        //  c) Restore customized index set to member.
        if (!isDynamicBypass()) m_IspNvramMgr = idxmgr;
        //m_IspNvramMgr = idxmgr;

    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: prepareHw_PerFrame_All()", m_eSensorDev, m_bProfileEnable);
    //  (2) Apply Per-Frame Parameters.
    (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial())
                                 : (prepare_rdy = prepareHw_PerFrame_All());
    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: applyToHw_PerFrame_All()", m_eSensorDev, m_bProfileEnable);

    if(!prepare_rdy || ! applyToHw_PerFrame_All(i4FrameID))  //  Apply the ispmgr's buffer to H/W.
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }

#if 0
    if  (
            ! prepareHw_PerFrame_All()          //  Prepare param members to the ispmgr's buffer.
        ||  ! applyToHw_PerFrame_All()          //  Apply the ispmgr's buffer to H/W.
        )
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }
#endif

lbExit:
    if  ( MERR_OK != err )
    {
        MY_ERR("[-do_validatePerFrame]err(%X)", err);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    return  err;
}




#if 0

MERROR_ENUM
ParamctrlComm::
setEnable_Meta_Gamma(MBOOL const fgForceEnable)
{
    MY_LOG(
        "[+setEnable_Meta_Gamma] (fgForceEnable, m_fgForceEnable_Meta_Gamma)=(%d, %d)"
        , fgForceEnable, m_fgForceEnable_Meta_Gamma
    );

    Mutex::Autolock lock(m_Lock);

    checkParamChange(m_fgForceEnable_Meta_Gamma, fgForceEnable);

    m_fgForceEnable_Meta_Gamma = fgForceEnable;

    return  MERR_OK;
}

#endif

