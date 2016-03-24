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
#define LOG_TAG "paramctrl_exif"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <camera_feature.h>
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
#include <ccm_mgr.h>
#include "paramctrl.h"

using namespace android;
using namespace NSFeature;
using namespace NSIspTuning;
using namespace NSIspExifDebug;


namespace
{


//  Tag <- Isp Index.
inline
MVOID
setIspIdx(
    IspDebugTag (&rTags)[TagID_Total_Num],
    IspDebugTagID const eTagID,
    MUINT32 const u4Idx
)
{
    rTags[eTagID].u4ID  = getIspTag(eTagID);
    rTags[eTagID].u4Val = u4Idx;
}

//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    ESensorDev_T eSensorDev,
    MBOOL fgReadFromHW,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    //MY_LOG("fgReadFromHW = %d", fgReadFromHW);

    if (fgReadFromHW) {
        getIspReg(eSensorDev, param);
    }
    else {
        getIspHWBuf(eSensorDev, param);
    }

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //MY_LOG("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}

//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    ESensorDev_T eSensorDev,
    ESensorTG_T eSensorTG,
    MBOOL fgReadFromHW,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    if (fgReadFromHW) {
        getIspReg(eSensorDev, eSensorTG, param);
    }
    else {
        getIspHWBuf(eSensorDev, eSensorTG, param);
    }

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //MY_LOG("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}


};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, MBOOL const fgReadFromHW) const
{
    Mutex::Autolock lock(m_Lock);

    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rDebugInfo.hdr  = g_rIspExifDebugInfoHdr;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = m_rIspExifDebugInfo.debugInfo.tags;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);


    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // CAM_CTL_EN_P1
    MY_LOG_IF(m_bDebugEnable,"CAM_CTL_EN_P1:");
    setIspTags<ISP_NVRAM_CTL_EN_P1_T, CAM_CTL_EN_P1_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // CAM_CTL_EN_P1_D
    MY_LOG_IF(m_bDebugEnable,"CAM_CTL_EN_P1_D:");
    setIspTags<ISP_NVRAM_CTL_EN_P1_D_T, CAM_CTL_EN_P1_D_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // CAM_CTL_EN_P2
    MY_LOG_IF(m_bDebugEnable,"CAM_CTL_EN_P2:");
    setIspTags<ISP_NVRAM_CTL_EN_P2_T, CAM_CTL_EN_P2_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // OBC
    MY_LOG_IF(m_bDebugEnable,"OBC:");
    setIspIdx(rTags, IDX_OBC, m_IspNvramMgr.getIdx_OBC());
    setIspTags<ISP_NVRAM_OBC_T, CAM_OBC_Begin>(m_eSensorDev, m_eSensorTG, fgReadFromHW, rTags);

    // BPC
    MY_LOG_IF(m_bDebugEnable,"BPC:");
    setIspIdx(rTags, IDX_BPC, m_IspNvramMgr.getIdx_BPC());
    setIspTags<ISP_NVRAM_BPC_T, CAM_BPC_Begin>(m_eSensorDev, m_eSensorTG, fgReadFromHW, rTags);

    // RMM
    MY_LOG_IF(m_bDebugEnable,"RMM:");
    setIspTags<ISP_NVRAM_BPC_RMM_T, CAM_RMM_Begin>(m_eSensorDev, m_eSensorTG, fgReadFromHW, rTags);

    // NR1
    MY_LOG_IF(m_bDebugEnable,"NR1:");
    setIspIdx(rTags, IDX_NR1, m_IspNvramMgr.getIdx_NR1());
    setIspTags<ISP_NVRAM_NR1_T, CAM_NR1_Begin>(m_eSensorDev, m_eSensorTG, fgReadFromHW, rTags);

    // LSC
    MY_LOG_IF(m_bDebugEnable,"LSC:");
    setIspIdx(rTags, IDX_LSC, m_IspNvramMgr.getIdx_LSC());
    setIspTags<ISP_NVRAM_LSC_T, CAM_LSC_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // SL2
    MY_LOG_IF(m_bDebugEnable,"SL2:");
    setIspIdx(rTags, IDX_SL2, m_IspNvramMgr.getIdx_SL2());
    setIspTags<ISP_NVRAM_SL2_T, CAM_SL2_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // RPG
    MY_LOG_IF(m_bDebugEnable,"RPG:");
    setIspTags<ISP_NVRAM_RPG_T, CAM_RPG_Begin>(m_eSensorDev, m_eSensorTG, fgReadFromHW, rTags);

    // PGN
    MY_LOG_IF(m_bDebugEnable,"PGN:");
    setIspTags<ISP_NVRAM_PGN_T, CAM_PGN_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // CFA
    MY_LOG_IF(m_bDebugEnable,"CFA:");
    setIspIdx(rTags, IDX_CFA, m_IspNvramMgr.getIdx_CFA());
    setIspIdx(rTags, CFA_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO);
    setIspIdx(rTags, CFA_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO);
    setIspIdx(rTags, CFA_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx);
    setIspIdx(rTags, CFA_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);
    setIspTags<ISP_NVRAM_CFA_T, CAM_CFA_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // G2G
    MY_LOG_IF(m_bDebugEnable,"G2G:");
    setIspTags<ISP_NVRAM_CCM_T, CAM_G2G_Begin>(m_eSensorDev, fgReadFromHW, rTags);
    ISP_CCM_WEIGHT_T CCM_Weight = m_pCcmMgr->getCCMWeight();
    setIspIdx(rTags, SMOOTH_CCM, m_rIspParam.bInvokeSmoothCCM);
    setIspIdx(rTags, CCM_Weight_Strobe, CCM_Weight.i4Strobe);
    setIspIdx(rTags, CCM_Weight_A, CCM_Weight.i4A);
    setIspIdx(rTags, CCM_Weight_TL84, CCM_Weight.i4TL84);
    setIspIdx(rTags, CCM_Weight_CWF, CCM_Weight.i4CWF);
    setIspIdx(rTags, CCM_Weight_D65, CCM_Weight.i4D65);
    setIspIdx(rTags, CCM_Weight_RSV1, CCM_Weight.i4RSV1);
    setIspIdx(rTags, CCM_Weight_RSV2, CCM_Weight.i4RSV2);
    setIspIdx(rTags, CCM_Weight_RSV3, CCM_Weight.i4RSV3);

    // GGM
    setIspIdx(rTags, IDX_GGM, m_IspNvramMgr.getIdx_GGM());

    m_rIspExifDebugInfo.GGMTableInfo.u4TableSize = 288;
    ISP_NVRAM_GGM_T& rGGM = *(reinterpret_cast<ISP_NVRAM_GGM_T*>(m_rIspExifDebugInfo.GGMTableInfo.GGM));
    ISP_MGR_GGM_T::getInstance(getSensorDev()).get(rGGM);

    // G2C
    MY_LOG_IF(m_bDebugEnable,"G2C:");
    setIspTags<ISP_NVRAM_G2C_T, CAM_G2C_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // ANR
    MY_LOG_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, m_IspNvramMgr.getIdx_ANR());
    setIspIdx(rTags, ANR_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO);
    setIspIdx(rTags, ANR_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO);
    setIspIdx(rTags, ANR_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx);
    setIspIdx(rTags, ANR_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);
    setIspTags<ISP_NVRAM_ANR_T, CAM_ANR_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // CCR
    MY_LOG_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, m_IspNvramMgr.getIdx_CCR());
    setIspTags<ISP_NVRAM_CCR_T, CAM_CCR_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // PCA
    MY_LOG_IF(m_bDebugEnable,"PCA:");
    setIspIdx(rTags, IDX_PCA, m_pPcaMgr->getIdx());
    setIspIdx(rTags, PCA_SLIDER, m_pPcaMgr->getSliderValue());
    setIspTags<ISP_NVRAM_PCA_T, CAM_PCA_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    m_rIspExifDebugInfo.PCATableInfo.u4TableSize = 180;
    ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).getLut(m_rIspExifDebugInfo.PCATableInfo.PCA);

    //  EE
    MY_LOG_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, m_IspNvramMgr.getIdx_EE());
    setIspIdx(rTags, EE_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4EEUpperISO);
    setIspIdx(rTags, EE_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4EELowerISO);
    setIspIdx(rTags, EE_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx);
    setIspIdx(rTags, EE_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);
    setIspTags<ISP_NVRAM_EE_T, CAM_EE_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // SE
    MY_LOG_IF(m_bDebugEnable,"SE:");
    setIspTags<ISP_NVRAM_SE_T, CAM_SE_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // NR3D
    MY_LOG_IF(m_bDebugEnable,"NR3D:");
    setIspIdx(rTags, IDX_NR3D, m_IspNvramMgr.getIdx_NR3D());
    setIspTags<ISP_NVRAM_NR3D_T, CAM_NR3D_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // MFB
    MY_LOG_IF(m_bDebugEnable,"MFB:");
    setIspIdx(rTags, IDX_MFB, m_IspNvramMgr.getIdx_MFB());
    setIspTags<ISP_NVRAM_MFB_T, CAM_MFB_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // MIXER3
    MY_LOG_IF(m_bDebugEnable,"MIXER3:");
    setIspTags<ISP_NVRAM_MIXER3_T, CAM_MIXER3_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // LCE
    MY_LOG_IF(m_bDebugEnable,"LCE:");
    setIspIdx(rTags, IDX_LCE, m_IspNvramMgr.getIdx_LCE());
    setIspTags<ISP_NVRAM_LCE_T, CAM_LCE_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    ////////////////////////////////////////////////////////////////////////////
    //  (2.2) ISPComm
    ////////////////////////////////////////////////////////////////////////////
    for (MUINT32 i = 0; i < sizeof(ISP_NVRAM_COMMON_STRUCT)/sizeof(MUINT32); i++)
    {
        MUINT32 const u4TagID = COMM_Begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = m_rIspComm.CommReg[i];
    }

    //  (2.3) RAWIspCamInfo
    MY_LOG_IF(m_bDebugEnable,"RAWIspCamInfo:");
    setIspIdx(rTags, IspProfile, m_rIspCamInfo.eIspProfile);
    setIspIdx(rTags, SensorMode, m_rIspCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx, m_rIspCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, m_rIspCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, m_rIspCamInfo.eIdx_ISO);
    setIspIdx(rTags, ShadingIdx, m_rIspCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, ZoomRatio_x100, m_rIspCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, LightValue_x10, m_rIspCamInfo.i4LightValue_x10);
    setIspIdx(rTags, SwnrEncEnableIsoThreshold, m_u4SwnrEncEnableIsoThreshold);
    //
    //  (2.4) EffectMode
    MY_LOG_IF(m_bDebugEnable,"EffectMode:");
    setIspIdx(rTags, EffectMode, getEffect());
    //
    //  (2.5) UserSelectLevel
    MY_LOG_IF(m_bDebugEnable,"UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, getIspUsrSelectLevel().eIdx_Edge);
    setIspIdx(rTags, HueIdx, getIspUsrSelectLevel().eIdx_Hue);
    setIspIdx(rTags, SatIdx, getIspUsrSelectLevel().eIdx_Sat);
    setIspIdx(rTags, BrightIdx, getIspUsrSelectLevel().eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, getIspUsrSelectLevel().eIdx_Contrast);
    //
    //  (2.6) Output
    rDebugInfo.debugInfo = m_rIspExifDebugInfo.debugInfo;
    rDebugInfo.GGMTableInfo = m_rIspExifDebugInfo.GGMTableInfo;
    rDebugInfo.PCATableInfo = m_rIspExifDebugInfo.PCATableInfo;

    //  (3) Reset to the default.
    ::memset(&m_rIspExifDebugInfo, 0, sizeof(m_rIspExifDebugInfo));

    return  MERR_OK;
}

