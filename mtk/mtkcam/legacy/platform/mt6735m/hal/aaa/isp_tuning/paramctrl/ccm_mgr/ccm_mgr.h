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
#ifndef _CCM_MGR_H_
#define _CCM_MGR_H_

//#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>
#include <dynamic_ccm.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>

namespace NSIspTuningv3
{

class NSIspTuning::IspTuningCustom;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CcmMgr
{
public:
    static CcmMgr* createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, NSIspTuning::IspTuningCustom* pIspTuningCustom);
    virtual MVOID destroyInstance() = 0;


private:
    enum
    {
        CCM_IDX_D65 = 0,
        CCM_IDX_TL84,
        CCM_IDX_CWF,
        CCM_IDX_A,
        CCM_IDX_NUM
    };

private:

    inline
    MVOID
    setIfChange(MINT32 i4Idx)
    {
        if  ( i4Idx != m_i4Idx )
        {
            m_i4Idx = i4Idx;
            m_rCCMOutput = m_rCCMInput[m_i4Idx];
        }
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    MINT32
    getIdx() const
    {
        return m_i4Idx;
    }

    inline
    MBOOL
    setIdx(MINT32 const i4Idx)
    {
        if  (( CCM_IDX_NUM <= i4Idx ) || ( 0 > i4Idx ))
            return  MFALSE;
        setIfChange(i4Idx);
        return  MTRUE;
    }

private:
    MINT32 m_i4Idx; //  CCM index
    ISP_NVRAM_CCM_T (&m_rCCMInput)[CCM_IDX_NUM];
    ISP_NVRAM_CCM_T m_rCCMOutput; // CCM output
    ESensorDev_T m_eSensorDev;
    ISP_NVRAM_MULTI_CCM_STRUCT& m_rISPMultiCCM;
    IspTuningCustom*    m_pIspTuningCustom;
    ISP_CCM_WEIGHT_T m_rCCMWeight;
    mtk_camera_metadata_enum_android_color_correction_mode_t m_eColorCorrectionMode;
    ISP_NVRAM_CCM_T m_rColorCorrectionTransform;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    CcmMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, IspTuningCustom* pIspTuningCustom)
        : m_i4Idx       (CCM_IDX_D65)
        , m_rCCMInput   (rIspNvramReg.CCM)
        , m_rCCMOutput  (m_rCCMInput[m_i4Idx])
        , m_eSensorDev  (eSensorDev)
        , m_rISPMultiCCM (rISPMultiCCM)
        , m_pIspTuningCustom (pIspTuningCustom)
        , m_rCCMWeight ()
        , m_eColorCorrectionMode (MTK_COLOR_CORRECTION_MODE_FAST)
        , m_rColorCorrectionTransform()
    {

    }

    virtual ~CcmMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    MVOID
    calculateCCM(AWB_INFO_T const& rAWBInfo, MBOOL bWithPreferenceGain, MINT32 i4FlashOnOff/*1=on; 0=off*/, MINT32 i4SmoothMethod);

    ISP_NVRAM_CCM_T&
    getCCM();

    inline
    ISP_CCM_WEIGHT_T
    getCCMWeight()
    {
        return m_rCCMWeight;
    }

    inline
    ESensorDev_T
    getSensorDev() const
    {
        return m_eSensorDev;
    }

    MVOID getCCM(ISP_CCM_T& rCCM);

    MVOID
    setColorCorrectionTransform(ISP_CCM_T& rCCM);

    inline
    MVOID
    setColorCorrectionMode(mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode)
    {
        m_eColorCorrectionMode = eColorCorrectionMode;
    }

};

};  //  NSIspTuningv3
#endif // _CCM_MGR_H_

