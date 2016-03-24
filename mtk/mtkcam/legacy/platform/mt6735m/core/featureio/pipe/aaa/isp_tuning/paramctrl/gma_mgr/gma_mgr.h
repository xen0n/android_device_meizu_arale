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
#ifndef _GMA_MGR_H_
#define _GMA_MGR_H_

#include <mtkcam/algorithm/libgma/MTKGma.h>
#include <ae_param.h>
#include <ispif.h>
#include <dbg_isp_param.h>

namespace NSIspTuning
{

class IspTuningCustom;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class GmaMgr
{
public:
    static GmaMgr* createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv);
    virtual MVOID destroyInstance() = 0;


private:
    /*
    enum
    {
        CCM_IDX_D65 = 0,
        CCM_IDX_TL84,
        CCM_IDX_CWF,
        CCM_IDX_A,
        CCM_IDX_NUM
    };
    */

private:
/*
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
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
/*
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
*/
private:
    ISP_NVRAM_GGM_T m_rGGMOutput; // GGM output
    ISP_NVRAM_REGISTER_STRUCT& m_rIspNvramReg;
    ESensorDev_T m_eSensorDev;
    MTK_GMA_ENV_INFO_STRUCT* m_pCustomEnv;
    MTKGma* m_pGmaAlgo;
    MTK_GMA_ENV_INFO_STRUCT mEnv;
    MUINT32 mGmaScenario;
    MTK_GMA_PROC_INFO_STRUCT mInInfo;
    MTK_GMA_RESULT_INFO_STRUCT mOutGGM;
    MTK_GMA_EXIF_INFO_STRUCT mOutExif;
    AE_INFO_T mAEInfo;
    MBOOL mbAEStable;
    MINT32 mLogEn;
    MINT32 mAlgoEn;
    


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    GmaMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, MTK_GMA_ENV_INFO_STRUCT* pCustomEnv);

    virtual ~GmaMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    MVOID
    calculateGGM(ISP_NVRAM_GGM_T* pGGMReg, NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo* pGmaExif);

    MVOID start();

    MVOID stop();

    inline MVOID setScenario(MUINT32 scenario)
    {
        mGmaScenario = scenario;
    }
    MVOID setAEInfo(AE_INFO_T const & rAEInfo);

    inline MVOID setAEStable(MBOOL bAEStable)
    {
        mbAEStable = bAEStable;
    }

    MVOID printInfo() const;
    
    /*    
    inline
    ISP_NVRAM_GGM_T&
    getGGM()
    {
        return m_rGGMOutput;
    }


    inline
    ISP_CCM_WEIGHT_T    
    getCCMWeight()
    {
        return m_rCCMWeight;
    }
    */

    inline
    ESensorDev_T
    getSensorDev() const
    {
        return m_eSensorDev;
    }

};

};  //  NSIspTuning
#endif // _GMA_MGR_H_

