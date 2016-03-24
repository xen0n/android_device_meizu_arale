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
#define LOG_TAG "isp_mgr_rmg"

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

/*********************************************************************
*
*********************************************************************/
#undef __func__
#define __func__ __FUNCTION__

#define RMG_DEBUG 0

#define CLAMP(x,min,max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

static MINT32 g_rmgDebug = 0;

/*********************************************************************
*
*********************************************************************/
namespace NSIspTuning
{

/*********************************************************************
*
*********************************************************************/
ISP_MGR_RMG_T &ISP_MGR_RMG_T::getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
        }
    case ESensorDev_MainSecond:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
        }
    case ESensorDev_Sub:
        if (eSensorTG == ESensorTG_1)
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        }
        else
        {
            return  ISP_MGR_RMG_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
        }
    default:
        MY_ERR("sensorDev(%d),sensorTG(%d)", eSensorDev, eSensorTG);
        return  ISP_MGR_RMG_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

#if 0   // RMG do not use NVRAM
/*********************************************************************
*
*********************************************************************/
template <>
ISP_MGR_RNG_T &
ISP_MGR_RNG_T::
put(ISP_NVRAM_SL2_T const &rParam)
{
    PUT_REG_INFO(CAM_SL2_CEN, cen);
    PUT_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    PUT_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    PUT_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);
    return  (*this);
}

/*********************************************************************
*
*********************************************************************/
template <>
ISP_MGR_RNG_T &
ISP_MGR_RNG_T::
get(ISP_NVRAM_SL2_T &rParam)
{
    GET_REG_INFO(CAM_SL2_CEN, cen);
    GET_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    GET_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    GET_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);
    return  (*this);
}

#endif

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_RMG_T::setRmgEnable(MBOOL bEnable)
{
    m_bEnable = bEnable;
    MY_LOG("[%s] en(%d),dev(%d),TG(%d)",__func__,m_bEnable,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_RMG_T::setRmgAeExpRatio(MINT32 aeRatio)
{
    MY_LOG("[%s] aeRatio(%d)",__func__,aeRatio);
    mAeExpRatio = aeRatio/100.0;

    if(mAeExpRatio < 1.0)
    {
        MY_LOG("mAeExpRatio < 1.0");
        mAeExpRatio = 1.0;
    }

    MY_LOG("[%s] mAeExpRatio(%f),dev(%d),TG(%d)",__func__,mAeExpRatio,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MVOID ISP_MGR_RMG_T::setRmgLeFirst(MUINT32 leFirst)
{
    mLeFirst = leFirst;    // 0 : first two rows are SE,  1 : first two rows are LE
    MY_LOG("[%s] mLeFirst(%u),dev(%d),TG(%d)",__func__,mLeFirst,m_eSensorDev,m_eSensorTG);
}

/*********************************************************************
*
*********************************************************************/
MBOOL ISP_MGR_RMG_T::apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_RMG_T::apply()");

    //====== Dynamic Debug ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    g_rmgDebug = atoi(value);

    //====== Prepare Register Value ======

    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);
    MUINT32 ihdrGain = 0.5 + (4 * 64 / mAeExpRatio);    //mAeExpRatio = LE/SE
    MUINT32 ihdrMode = (m_bEnable == MTRUE) ? 1 : 0;

#if (RMG_DEBUG)
    MY_LOG("[%s] dev(%d),TG(%d)",__func__,m_eSensorDev,m_eSensorTG);
    MY_LOG("ispProf(%d),eSwScn(%u),m_bEnable(%u)",eIspProfile,eSwScn,m_bEnable);
    MY_LOG("mAeExpRatio(%f),mLeFirst(%u),ihdrGain(%u),ihdrMode(%u)",mAeExpRatio,mLeFirst,ihdrGain,ihdrMode);
#else
    MY_LOG_IF(g_rmgDebug, "[%s] dev(%d),TG(%d)",__func__,m_eSensorDev,m_eSensorTG);
    MY_LOG_IF(g_rmgDebug, "ispProf(%d),eSwScn(%u),m_bEnable(%u)",eIspProfile,eSwScn,m_bEnable);
    MY_LOG_IF(g_rmgDebug, "mAeExpRatio(%f),mLeFirst(%u),ihdrGain(%u),ihdrMode(%u)",mAeExpRatio,mLeFirst,ihdrGain,ihdrMode);
#endif

    if(m_eSensorTG == ESensorTG_1)
    {
        //====== Update LCE Tuning Engien ======

        TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Rmg);

        //====== Register Setting ======

        //> TOP

        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1, RMG_EN, m_bEnable);

        //> prepar register value

        //CAM_CTL_IHDR

        MUINT32 camCtlIHdr = 0 | ((mLeFirst&0x1) << 13) | (ihdrMode << 12) | ihdrGain;

        m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR].val = camCtlIHdr;

    #if (RMG_DEBUG)
        MY_LOG("CAM_CTL_IHDR(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR].val);
    #else
        MY_LOG_IF(g_rmgDebug,"CAM_CTL_IHDR(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR].val);
    #endif

        //> Write to TuningMgr
        TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);
    }
    else if(m_eSensorTG == ESensorTG_2)
    {
        //====== Update LCE Tuning Engien ======

        TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Rmg_d);

        //====== Register Setting ======

        //> TOP

        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1_D, RMG_D_EN, m_bEnable);

        //> prepar register value

        //CAM_CTL_IHDR_D

        MUINT32 camCtlIHdrD = 0 | ((mLeFirst&0x1) << 13) | (ihdrMode << 12) | ihdrGain;

        m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR_D].val = camCtlIHdrD;

    #if (RMG_DEBUG)
        MY_LOG("CAM_CTL_IHDR_D(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR_D].val);
    #else
        MY_LOG_IF(g_rmgDebug,"CAM_CTL_IHDR_D(0x%08x)",m_rIspRegInfo[ERegInfo_CAM_CTL_IHDR_D].val);
    #endif

        //> Write to TuningMgr
        TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);
    }

    addressErrorCheck("After ISP_MGR_RMG_T::apply()");
    dumpRegInfo("RMG");

    return  MTRUE;
}


}
