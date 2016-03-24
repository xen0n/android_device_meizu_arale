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
#define LOG_TAG "isp_mgr_lsc"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_reg.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_LSC_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_LSC_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_LSC_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_LSC_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_LSC_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_LSC_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_LSC_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
put(ISP_NVRAM_LSC_T const& rParam)
{
    MY_LOG_IF(ENABLE_MY_LOG, "[%s\n", __FUNCTION__);

//    PUT_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    PUT_REG_INFO(CAM_LSC_CTL1, ctl1);
    PUT_REG_INFO(CAM_LSC_CTL2, ctl2);
    PUT_REG_INFO(CAM_LSC_CTL3, ctl3);
    PUT_REG_INFO(CAM_LSC_LBLOCK, lblock);
    PUT_REG_INFO(CAM_LSC_RATIO, ratio);
//    PUT_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}

ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
putAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(ENABLE_MY_LOG, "[%s] eSensorDev(%d), m_eSensorTG(%d), u4BaseAddr(0x%08x)", __FUNCTION__, m_eSensorDev, m_eSensorTG, u4BaseAddr);
    //MY_LOG_IF(ENABLE_MY_LOG, "[%s] u4BaseAddr(0x%08x)\n", __FUNCTION__, u4BaseAddr);

    REG_INFO_VALUE(CAM_LSCI_BASE_ADDR) = u4BaseAddr;

    return  (*this);
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(ISP_NVRAM_LSC_T& rParam)
{
    MY_LOG_IF(ENABLE_MY_LOG, "[%s]\n", __FUNCTION__);

//    GET_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    GET_REG_INFO(CAM_LSC_CTL1, ctl1);
    GET_REG_INFO(CAM_LSC_CTL2, ctl2);
    GET_REG_INFO(CAM_LSC_CTL3, ctl3);
    GET_REG_INFO(CAM_LSC_LBLOCK, lblock);
    GET_REG_INFO(CAM_LSC_RATIO, ratio);
//    GET_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}

MUINT32
ISP_MGR_LSC_T::
getAddr()
{
    MUINT32 u4Addr = REG_INFO_VALUE(CAM_LSCI_BASE_ADDR);
    MY_LOG_IF(ENABLE_MY_LOG, "[%s] u4BaseAddr(0x%08x)\n", __FUNCTION__, u4Addr);
    return u4Addr;
}

MBOOL
ISP_MGR_LSC_T::
reset()
{
    return MTRUE;
}


#define LSC_DIRECT_ACCESS 0
#define EN_WRITE_REGS     0
MBOOL
ISP_MGR_LSC_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL fgOnOff = m_fgOnOff;

    MUINT32 u4XNum, u4YNum, u4Wd, u4Ht;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE;

    u4XNum = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->LSC_SDBLK_XNUM;
    u4YNum = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->LSC_SDBLK_YNUM;
    u4Wd = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->LSC_SDBLK_WIDTH;
    u4Ht = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->LSC_SDBLK_HEIGHT;

    LSCI_XSIZE = (u4XNum+1)*4*128/8 - 1;
    LSCI_YSIZE = u4YNum;
    LSCI_STRIDE = (LSCI_XSIZE+1);

    REG_INFO_VALUE(CAM_LSCI_OFST_ADDR)  = 0;
    REG_INFO_VALUE(CAM_LSCI_XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE(CAM_LSCI_YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE(CAM_LSCI_STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE(CAM_LSC_CTL1)        = 0x30000000;
//    REG_INFO_VALUE(CAM_LSC_GAIN_TH)     = 0x03F00000;

    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    // TOP
    if (m_eSensorTG == ESensorTG_1)
    {
        TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Lsc);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_OFST_ADDR, LSCI_OFFSET);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_XSIZE, LSCI_XSIZE);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_YSIZE, LSCI_YSIZE);
        //TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_LSCI_STRIDE, STRIDE, LSCI_STRIDE);
        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1, LSC_EN, fgOnOff);
        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1_DMA, LSCI_EN, fgOnOff);

        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);
    }
    else
    {
        TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Lsc_d);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_D_OFST_ADDR, LSCI_OFFSET);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_D_XSIZE, LSCI_XSIZE);
        //TUNING_MGR_WRITE_REG(eSwScn, CAM_LSCI_D_YSIZE, LSCI_YSIZE);
        //TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_LSCI_D_STRIDE, STRIDE, LSCI_STRIDE);
        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1_D, LSC_D_EN, fgOnOff);
        TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, fgOnOff);

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);
    }

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(
        eSwScn,
        static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
        m_u4RegInfoNum);

    dumpRegInfo("LSC");

    return  MTRUE;
}

MBOOL
ISP_MGR_LSC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL fgOnOff = m_fgOnOff;

    MUINT32 u4XNum, u4YNum, u4Wd, u4Ht;
    MUINT32 LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE;

    u4XNum = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->LSC_SDBLK_XNUM;
    u4YNum = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->LSC_SDBLK_YNUM;
    u4Wd = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->LSC_SDBLK_WIDTH;
    u4Ht = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->LSC_SDBLK_HEIGHT;

    LSCI_XSIZE = (u4XNum+1)*4*128/8 - 1;
    LSCI_YSIZE = u4YNum;
    LSCI_STRIDE = (LSCI_XSIZE+1);

    REG_INFO_VALUE(CAM_LSCI_OFST_ADDR)  = 0;
    REG_INFO_VALUE(CAM_LSCI_XSIZE)      = LSCI_XSIZE;
    REG_INFO_VALUE(CAM_LSCI_YSIZE)      = LSCI_YSIZE;
    REG_INFO_VALUE(CAM_LSCI_STRIDE)     = LSCI_STRIDE;
    REG_INFO_VALUE(CAM_LSC_CTL1)        = 0x30000000;
//    REG_INFO_VALUE(CAM_LSC_GAIN_TH)     = 0x03F00000;

    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    // TOP
    if (m_eSensorTG == ESensorTG_1)
    {
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1, LSC_EN, fgOnOff);
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1_DMA, LSCI_EN, fgOnOff);

        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);
    }
    else
    {
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1_D, LSC_D_EN, fgOnOff);
        ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P1_DMA_D, LSCI_D_EN, fgOnOff);

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);
    }

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("LSC");

    return  MTRUE;
}

MVOID
ISP_MGR_LSC_T::
enableLsc(MBOOL enable)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s %d\n", __FUNCTION__, enable);
    m_fgOnOff = enable;
}

MBOOL
ISP_MGR_LSC_T::
isEnable(void)
{
    return m_fgOnOff;
}

}
