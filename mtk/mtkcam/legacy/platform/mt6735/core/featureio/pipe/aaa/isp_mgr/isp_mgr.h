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
#ifndef _ISP_MGR_H_
#define _ISP_MGR_H_

#include <utils/threads.h>
#include <ispdrv_mgr.h>
#include <mtkcam/drv/isp_reg.h>
#include <isp_tuning.h>
#include <mtkcam/drv/isp_drv.h>
#include <ispif.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <shading_tuning_custom.h>
#include <mtkcam/featureio/tuning_mgr.h>

using namespace android;
using namespace NS3A;

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BASE
{
protected:
    typedef ISP_MGR_BASE  MyType;
    typedef ISPREG_INFO_T RegInfo_T;

#define REG_ADDR(reg)\
    ((MUINT32)((MUINTPTR)&getIspReg(ISPDRV_MODE_ISP)->reg) - (MUINTPTR)getIspReg(ISPDRV_MODE_ISP))

#define REG_INFO(reg)\
    (m_rIspRegInfo[ERegInfo_##reg])

#define REG_INFO_ADDR(reg)\
    (REG_INFO(reg).addr)

#define REG_INFO_VALUE(reg)\
    (REG_INFO(reg).val)

#define REG_INFO_VALUE_PTR(reg)\
    (& REG_INFO_VALUE(reg))

#define INIT_REG_INFO_ADDR(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR(reg)

#define INIT_REG_INFO_VALUE(reg, val)\
    REG_INFO_VALUE(reg) = val

#define PUT_REG_INFO(dest, src)\
    REG_INFO_VALUE(dest) = setbits(REG_INFO_VALUE(dest), rParam.src)

#define GET_REG_INFO(src, dest)\
    rParam.dest.val = REG_INFO_VALUE(src)

#define INIT_ISP_DRV_SCENARIO(ispProfile, ispDrvScenario) \
    m_rIspDrvScenario[ispProfile] = ispDrvScenario

protected:
    virtual ~ISP_MGR_BASE() {}
    ISP_MGR_BASE(MVOID*const pRegInfo, MUINT32 const u4RegInfoNum, MUINT32& u4StartAddr, ESensorDev_T const eSensorDev)
     : m_bDebugEnable(MFALSE)
     , m_pRegInfo(pRegInfo)
     , m_u4RegInfoNum(u4RegInfoNum)
     , m_u4StartAddr(u4StartAddr)
     , m_eSensorDev(eSensorDev)
    {
        // isp drv scenario init
        if (m_eSensorDev == ESensorDev_Main) {
            INIT_ISP_DRV_SCENARIO(EIspProfile_Preview, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Video, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_ZSD_Capture, eSoftwareScenario_Main_ZSD_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture, eSoftwareScenario_Main_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture, eSoftwareScenario_Main_Pure_Raw_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_N3D_Preview, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_N3D_Video, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_N3D_Capture, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off, eSoftwareScenario_Main_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Main_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off, eSoftwareScenario_Main_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Main_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_EE_Off, eSoftwareScenario_Main_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE, eSoftwareScenario_Main_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Main_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing, eSoftwareScenario_Main_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Main_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VFB_PostProc, eSoftwareScenario_Main_Vfb_Stream_2);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Preview, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Video, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_1, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_2, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_1, eSoftwareScenario_Main_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_2, eSoftwareScenario_Main_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_1, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_2, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_SWNR, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_SWNR, eSoftwareScenario_Main_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture_SWNR, eSoftwareScenario_Main_Pure_Raw_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Preview, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Video, eSoftwareScenario_Main_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Capture, eSoftwareScenario_Main_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off, eSoftwareScenario_Main_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Main_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off, eSoftwareScenario_Main_VSS_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Main_VSS_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_EE_Off, eSoftwareScenario_Main_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE, eSoftwareScenario_Main_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Main_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing, eSoftwareScenario_Main_VSS_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Main_VSS_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_1, eSoftwareScenario_Main_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_2, eSoftwareScenario_Main_VSS_Capture);
        }
        else if (m_eSensorDev == ESensorDev_Sub) {
            INIT_ISP_DRV_SCENARIO(EIspProfile_Preview, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Video, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_ZSD_Capture, eSoftwareScenario_Sub_ZSD_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture, eSoftwareScenario_Sub_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture, eSoftwareScenario_Sub_Pure_Raw_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off, eSoftwareScenario_Sub_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Sub_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off, eSoftwareScenario_Sub_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Sub_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_EE_Off, eSoftwareScenario_Sub_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE, eSoftwareScenario_Sub_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Sub_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing, eSoftwareScenario_Sub_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Sub_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VFB_PostProc, eSoftwareScenario_Sub_Vfb_Stream_2);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Preview, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Video, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_1, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_2, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_1, eSoftwareScenario_Sub_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_2, eSoftwareScenario_Sub_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_1, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_2, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_SWNR, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_SWNR, eSoftwareScenario_Sub_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture_SWNR, eSoftwareScenario_Sub_Pure_Raw_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Preview, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Video, eSoftwareScenario_Sub_Normal_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Capture, eSoftwareScenario_Sub_Normal_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off, eSoftwareScenario_Sub_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Sub_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off, eSoftwareScenario_Sub_VSS_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Sub_VSS_Mfb_Blending);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_EE_Off, eSoftwareScenario_Sub_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE, eSoftwareScenario_Sub_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Sub_VSS_Mfb_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing, eSoftwareScenario_Sub_VSS_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Sub_VSS_Mfb_Mixing);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_1, eSoftwareScenario_Sub_VSS_Capture);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_2, eSoftwareScenario_Sub_VSS_Capture);
        }
        else { // ESensorDev_MainSecond
            INIT_ISP_DRV_SCENARIO(EIspProfile_Preview, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Video, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_ZSD_Capture, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_EE_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VFB_PostProc, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Preview, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_IHDR_Video, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_1, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_MultiPass_ANR_2, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_1, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_MultiPass_ANR_2, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_1, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MFB_MultiPass_ANR_2, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_Capture_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_Capture_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_PureRAW_Capture_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Preview, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Video, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_MHDR_Capture, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Capture_EE_Off_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_Blending_All_Off_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_EE_Off, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_PostProc_Mixing_SWNR, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_1, eSoftwareScenario_Main2_N3D_Stream);
            INIT_ISP_DRV_SCENARIO(EIspProfile_VSS_MFB_MultiPass_ANR_2, eSoftwareScenario_Main2_N3D_Stream);
        }


    }

protected:
    MBOOL              m_bDebugEnable;
    MVOID*const        m_pRegInfo;
    MUINT32 const      m_u4RegInfoNum;
    MUINT32&           m_u4StartAddr;
    ESensorDev_T const m_eSensorDev;
    MINT32             m_rIspDrvScenario[EIspProfile_NUM];

//==============================================================================
protected:
    template <class _ISP_XXX_T>
    inline
    MUINT32
    setbits(MUINT32 const dest, _ISP_XXX_T const src)
    {
        MUINT32 const u4Mask = _ISP_XXX_T::MASK;
        //  (1) clear bits + (2) set bits
        return  ((dest & ~u4Mask) | (src.val & u4Mask));
    }

    inline
    isp_reg_t*
    getIspReg(MINT32 i4IspDrvMode) const
    {
        return reinterpret_cast<isp_reg_t*> (
            IspDrvMgr::getInstance().getIspReg(static_cast<ISPDRV_MODE_T>(i4IspDrvMode))
        );
    }

    inline
    MBOOL
    readRegs(MINT32 i4IspDrvMode, RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum) const
    {
        return  IspDrvMgr::getInstance().readRegs(static_cast<ISPDRV_MODE_T>(i4IspDrvMode), pRegInfo, u4RegInfoNum);
    }

    inline
    MBOOL
    writeRegs(CAM_MODULE_ENUM eCamModule, MINT32 i4IspDrvMode, RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum)
    {
        return  IspDrvMgr::getInstance().writeRegs(eCamModule, static_cast<ISPDRV_MODE_T>(i4IspDrvMode), pRegInfo, u4RegInfoNum);
    }

    inline
    MBOOL
    IsDebugEnabled()
    {
         return m_bDebugEnable;
    }

public: // Interfaces
    virtual
    MBOOL
    reset()
    {
        addressErrorCheck("Before reset()");
        MBOOL err = readRegs(ISPDRV_MODE_ISP, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        addressErrorCheck("After reset()");
        return err;
    }

    virtual
    MVOID
    dumpRegInfo(char const*const pModuleName);

    virtual
    MVOID
    dumpRegs();

    virtual
    MVOID
    addressErrorCheck(char const*const ptestCastName);

} ISP_MGR_BASE_T;

#include "isp_mgr_ctl.h"
#include "isp_mgr_obc.h"
#include "isp_mgr_bnr.h"
#include "isp_mgr_rpg.h"
#include "isp_mgr_pgn.h"
#include "isp_mgr_cfa.h"
#include "isp_mgr_ccm.h"
#include "isp_mgr_ggm.h"
#include "isp_mgr_g2c.h"
#include "isp_mgr_nbc.h"
#include "isp_mgr_pca.h"
#include "isp_mgr_seee.h"
#include "isp_mgr_nr3d.h"
#include "isp_mgr_mfb.h"
#include "isp_mgr_mixer3.h"
#include "isp_mgr_awb_stat.h"
#include "isp_mgr_lsc.h"
#include "isp_mgr_sl2.h"
#include "isp_mgr_flk.h"
#include "isp_mgr_lce.h"
#include "isp_mgr_rmg.h"
#include "isp_mgr_ae_stat.h"




class IspDebug
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IspDebug(IspDebug const&);
    //  Copy-assignment operator is disallowed.
    IspDebug& operator=(IspDebug const&);

public:  ////
    IspDebug();
    ~IspDebug() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IspDebug& getInstance();
    MBOOL init();
    MBOOL uninit();
    MBOOL dumpIspDebugMessage();
    MVOID dumpIspDebugMessage_DMA();
    MVOID dumpIspDebugMessage_AWB();
    MVOID dumpIspDebugMessage_OBC();
    MVOID dumpIspDebugMessage_LSC();
    MVOID dumpIspDebugMessage_BNR();
    MVOID dumpIspDebugMessage_RPG();
    MVOID dumpIspDebugMessage_PGN();
    MVOID dumpIspDebugMessage_CFA();
    MVOID dumpIspDebugMessage_CCM();
    MVOID dumpIspDebugMessage_GGM();
    MVOID dumpIspDebugMessage_G2C();
    MVOID dumpIspDebugMessage_NBC();
    MVOID dumpIspDebugMessage_PCA();
    MVOID dumpIspDebugMessage_SEEE();
    MVOID dumpIspDebugMessage_NR3D();
    MVOID dumpIspDebugMessage_AE();
    MVOID dumpIspDebugMessage_MFB();
    MVOID dumpIspDebugMessage_MIX3();

    MUINT32 readLsciAddr(MUINT32 u4TgInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member: please follow the order of member initialization list in constructor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IspDrv*     m_pIspDrv;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL       m_bDebugEnable;
};

};  //  namespace NSIspTuning

#endif // _ISP_MGR_H_

