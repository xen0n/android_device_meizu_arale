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
#define LOG_TAG "isp_mgr_lce"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>

#include "isp_mgr.h"
#include <mtkcam/featureio/tuning_mgr.h>
#include "mtkcam/featureio/vfb_hal_base.h"

/*********************************************************************
*
*********************************************************************/
#undef __func__
#define __func__ __FUNCTION__

#define LCE_DEBUG 0
#define LCE_ADJUST_AFTER_AE_CNT 3
#define LCE_ADJUST_MOVE_STEP 1
#define LCE_ADJUST_METHOD 0

#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

static MINT32 g_lceDebug = 0;

/*********************************************************************
*
*********************************************************************/
namespace NSIspTuning
{

/*********************************************************************
*
*********************************************************************/
ISP_MGR_LCE_T &ISP_MGR_LCE_T::getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
        }
    case ESensorDev_MainSecond:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
        }
    case ESensorDev_Sub:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_LCE_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
        }
    default:
        MY_ERR("eSensorDev(%d),eSensorTG(%d)", eSensorDev, eSensorTG);
        return  ISP_MGR_LCE_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

/*********************************************************************
*
*********************************************************************/
template <>
ISP_MGR_LCE_T &
ISP_MGR_LCE_T::
put(ISP_NVRAM_LCE_T const &rParam)
{
    //MY_LOG("rParam.qua.val = x%08x", rParam.qua.val);

    PUT_REG_INFO(CAM_LCE_QUA, qua);

    //MY_LOG("m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val = 0x%8x", m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val);

    mLceWeakest  = m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val & 0x01F;
    mLceStrogest = (m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val & 0x3E0) >> 5;
    mLvLowBound  = (m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val & 0x1FFC00) >> 10;
    mLvUpBound   = (m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val & 0xFFE00000) >> 21;

#if (LCE_DEBUG)
    MY_LOG("[%s] QUA(0x%08x),lceIdx(%u,%u),LV(%u,%u)",__func__,m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val,mLceWeakest,mLceStrogest,mLvLowBound,mLvUpBound);
#else
    MY_LOG_IF(g_lceDebug,"[%s] QUA(0x%08x),lceIdx(%u,%u),LV(%u,%u)",__func__,m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val,mLceWeakest,mLceStrogest,mLvLowBound,mLvUpBound);
#endif

    return  (*this);
}

/*********************************************************************
*
*********************************************************************/
template <>
ISP_MGR_LCE_T &
ISP_MGR_LCE_T::
get(ISP_NVRAM_LCE_T &rParam)
{
//    GET_REG_INFO(CAM_LCE_QUA, qua);
    //MY_LOG("m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val = 0x%8x", m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val);

    MUINT32 qua = 0 | mLceWeakest | (mLceStrogest << 5) | (mLcsFinalIdx << 10) | (mLvLowBound << 15) | (mLvUpBound << 23);
    rParam.qua.val = qua;

    MY_LOG_IF(g_lceDebug,"rParam.qua.val(0x%08x)",rParam.qua.val);
    return  (*this);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::setLceEnable(MBOOL bEnable)
{
    m_bEnable = bEnable;
    MY_LOG("[%s]en(%d),dev(%d),TG(%d)",__func__,m_bEnable,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::setLcsoSize(MUINT32 width, MUINT32 height)
{
    mLcsOutWidth  = width;
    mLcsOutHeight = height;
    MY_LOG("[%s](W/H)=(%u,%u),dev(%d),TG(%d)",__func__,mLcsOutWidth,mLcsOutHeight,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::setLceiAddr(MUINT32 addr)
{
    mLceiBaseAddr = addr;
    MY_LOG("[%s]addr(0x%08x),dev(%d),TG(%d)",__func__,mLceiBaseAddr,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::setAeStableCnt(MUINT32 cnt)
{
    mAeStableCnt = cnt;
    MY_LOG_IF(g_lceDebug,"[%s] mAeStableCnt(%u)",__func__,mAeStableCnt);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::setLceCurLv(MINT32 aCurLv)
{
#if LCE_ADJUST_METHOD

    if(mAeStableCnt > LCE_ADJUST_AFTER_AE_CNT)
    {
        MY_LOG("[%s] aCurLv(%d),bound(%d,%d),cnt(%u)",__func__,aCurLv,mLvLowBound,mLvUpBound,mAeStableCnt);

        if(aCurLv >= mLvUpBound)
        {
            mLcsFinalIdx = mLceStrogest;
        }
        else if(aCurLv <= mLvLowBound)
        {
            mLcsFinalIdx = mLceWeakest;
        }
        else
        {
            MFLOAT tempIdx = (((MFLOAT)aCurLv-(MFLOAT)mLvLowBound)/((MFLOAT)mLvUpBound-(MFLOAT)mLvLowBound))*((MFLOAT)mLceStrogest-(MFLOAT)mLceWeakest)+(MFLOAT)mLceWeakest;
            mLcsFinalIdx = 0.5 + tempIdx;
        }

        //> boundary check

        if(mLcsFinalIdx > mLceStrogest)
        {
            MY_LOG("[%s] mLcsFinalIdx(%u->%u)",__func__,mLcsFinalIdx,mLceStrogest);
            mLcsFinalIdx = mLceStrogest;
        }
        else if(mLcsFinalIdx < mLceWeakest)
        {
            MY_LOG("[%s] mLcsFinalIdx(%u->%u)",__func__,mLcsFinalIdx,mLceWeakest);
            mLcsFinalIdx = mLceWeakest;
        }
        else
        {
            MY_LOG("[%s] mLcsFinalIdx(%u)",__func__,mLcsFinalIdx);
        }
    }

#else

    MY_LOG("[%s] aCurLv(%d),bound(%d,%d)",__func__,aCurLv,mLvLowBound,mLvUpBound);

    if(aCurLv >= mLvUpBound)
    {
        mLcsFinalIdx = mLceStrogest;
    }
    else if(aCurLv <= mLvLowBound)
    {
        mLcsFinalIdx = mLceWeakest;
    }
    else
    {
        MFLOAT tempIdx = (((MFLOAT)aCurLv-(MFLOAT)mLvLowBound)/((MFLOAT)mLvUpBound-(MFLOAT)mLvLowBound))*((MFLOAT)mLceStrogest-(MFLOAT)mLceWeakest)+(MFLOAT)mLceWeakest;
        mLcsFinalIdx = 0.5 + tempIdx;
    }

    //> boundary check

    if(mLcsFinalIdx > mLceStrogest)
    {
        MY_LOG("[%s] mLcsFinalIdx(%u->%u)",__func__,mLcsFinalIdx,mLceStrogest);
        mLcsFinalIdx = mLceStrogest;
    }
    else if(mLcsFinalIdx < mLceWeakest)
    {
        MY_LOG("[%s] mLcsFinalIdx(%u->%u)",__func__,mLcsFinalIdx,mLceWeakest);
        mLcsFinalIdx = mLceWeakest;
    }
    else
    {
        MY_LOG("[%s] mLcsFinalIdx(%u)",__func__,mLcsFinalIdx);
    }

#endif
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_LCE_T::updateLceIdx()
{
    if(mLcsIdx > mLcsFinalIdx)
    {
        mLcsIdx -= LCE_ADJUST_MOVE_STEP;
    }
    else if(mLcsIdx < mLcsFinalIdx)
    {
        mLcsIdx += LCE_ADJUST_MOVE_STEP;
    }
    MY_LOG("mLcsIdx(%u)",mLcsIdx);
}

/*********************************************************************
*
*********************************************************************/
MBOOL ISP_MGR_LCE_T::apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_LCE_T::apply()");

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    g_lceDebug = atoi(value);

    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

#if (LCE_DEBUG)
    MY_LOG("dev(%d),TG(%d),ispProf(%d),eSwScn(%u),m_bEnable(%u)",m_eSensorDev,m_eSensorTG,eIspProfile,eSwScn,m_bEnable);
#else
    MY_LOG_IF(g_lceDebug, "dev(%d),TG(%d),ispProf(%d),eSwScn(%u),m_bEnable(%u)",m_eSensorDev,m_eSensorTG,eIspProfile,eSwScn,m_bEnable);
#endif

    //====== Update LCE Tuning Engien ======

    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Lce);

    //====== Register Setting ======

    //> TOP

    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, LCE_EN, m_bEnable);

    //> prepar register value

    //CAM_LCE_SLM_SIZE

    MUINT32 lcmSlmSize = 0;

    lcmSlmSize = lcmSlmSize | (mLcsOutHeight << 8) | mLcsOutWidth;
    m_rIspRegInfo[ERegInfo_CAM_LCE_SLM_SIZE].val = lcmSlmSize;

    //> LCE tuning

#if 0
    m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val   = mLceQuaReg[8];
    m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_1].val = mLceDgc1Reg[8];
    m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_2].val = mLceDgc2Reg[8];
#else

    #if LCE_ADJUST_METHOD
        updateLceIdx();
        m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val   = mLceQuaReg[mLcsIdx];
        m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_1].val = mLceDgc1Reg[mLcsIdx];
        m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_2].val = mLceDgc2Reg[mLcsIdx];
    #else
        m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val   = mLceQuaReg[mLcsFinalIdx];
        m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_1].val = mLceDgc1Reg[mLcsFinalIdx];
        m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_2].val = mLceDgc2Reg[mLcsFinalIdx];
    #endif
#endif

#if (LCE_DEBUG)

    MY_LOG("(mLcsOutWidth/mLcsOutHeight)=(%u,%u),CAM_LCE_SLM_SIZE(0x%08x)",mLcsOutWidth,mLcsOutHeight,m_rIspRegInfo[ERegInfo_CAM_LCE_SLM_SIZE].val);
    MY_LOG("CAM_LCE_QUA(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val);
    MY_LOG("CAM_LCE_DGC_1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_1].val);
    MY_LOG("CAM_LCE_DGC_2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_2].val);
    MY_LOG("mLcsFinalIdx(%u),mLcsIdx(%u)",mLcsFinalIdx,mLcsIdx);

#else

    MY_LOG_IF(g_lceDebug, "(LcsOutW/LcsOutH)=(%u,%u),CAM_LCE_SLM_SIZE(0x%08x)",mLcsOutWidth,mLcsOutHeight,m_rIspRegInfo[ERegInfo_CAM_LCE_SLM_SIZE].val);
    MY_LOG_IF(g_lceDebug, "CAM_LCE_QUA(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_QUA].val);
    MY_LOG_IF(g_lceDebug, "CAM_LCE_DGC_1(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_1].val);
    MY_LOG_IF(g_lceDebug, "CAM_LCE_DGC_2(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_LCE_DGC_2].val);
    MY_LOG_IF(g_lceDebug, "mLcsFinalIdx(%u),mLcsIdx(%u)",mLcsFinalIdx,mLcsIdx);

#endif

    //> Write to TuningMgr
    TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);

    addressErrorCheck("After ISP_MGR_LCE_T::apply()");
    dumpRegInfo("LCE");

    return  MTRUE;
}


}
