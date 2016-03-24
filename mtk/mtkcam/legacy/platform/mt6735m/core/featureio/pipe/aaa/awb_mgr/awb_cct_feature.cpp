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
#define LOG_TAG "awb_cct_feature"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <af_param.h>
#include <awb_param.h>
#include <mtkcam/algorithm/lib3a/awb_algo_if.h>
//#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <awb_tuning_custom.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <nvbuf_util.h>
#include "awb_mgr.h"

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBEnable()
{
    MY_LOG("[ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN]\n");

    enableAWB();

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBDisable()
{
    MY_LOG("[ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN]\n");

    disableAWB();

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBGetEnableInfo(
    MINT32 *a_pEnableAWB,
    MUINT32 *a_pOutLen
)
{
    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO]\n");

    *a_pEnableAWB = isAWBEnable();

    *a_pOutLen = sizeof(MINT32);

    MY_LOG("AWB Enable = %d\n", *a_pEnableAWB);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::
CCTOPAWBGetAWBGain(
    MVOID *a_pAWBGain,
    MUINT32 *a_pOutLen
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_GAIN]\n");

    *pAWBGain = m_rAWBOutput.rPreviewAWBGain;

    *a_pOutLen = sizeof(AWB_GAIN_T);

    MY_LOG("[RGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4R);
    MY_LOG("[GGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4G);
    MY_LOG("[BGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4B);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSetAWBGain(
    MVOID *a_pAWBGain
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_SET_GAIN]\n");

    m_rAWBOutput.rPreviewAWBGain = *pAWBGain;
    m_rAWBOutput.rCaptureAWBGain = *pAWBGain;
    m_rAWBOutput.rAWBInfo.rCurrentAWBGain = *pAWBGain;

    ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_eSensorDev)).setIspAWBGain (m_rAWBOutput.rPreviewAWBGain);
    ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_eSensorDev)).apply (static_cast<NSIspTuning::EIspProfile_T>(IspTuningMgr::getInstance().getIspProfile(m_eSensorDev)));
    IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo); // update AWB info
    IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    MY_LOG("[RGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4R);
    MY_LOG("[GGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4G);
    MY_LOG("[BGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4B);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBApplyNVRAMParam(
    MVOID *a_pAWBNVRAM
)
{
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2]\n");

    m_pNVRAM_3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL] = pAWBNVRAM[AWB_NVRAM_IDX_NORMAL];
    m_pNVRAM_3A->rAWBNVRAM[AWB_NVRAM_IDX_VHDR] = pAWBNVRAM[AWB_NVRAM_IDX_VHDR];

    m_rAWBInitInput.rAWBNVRAM = pAWBNVRAM[m_i4AWBNvramIdx];

    AWBWindowConfig();
    AWBStatConfig();

    m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                  m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                  m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

    m_pIAwbAlgo->updateAWBParam(m_rAWBInitInput);

    AWBRAWPreGain1Config();

    m_bAWBModeChanged = MTRUE;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetNVRAMParam(
    MVOID *a_pAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if(err!=0)
    {
        MY_ERR("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    pAWBNVRAM[AWB_NVRAM_IDX_NORMAL] = m_pNVRAM_3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL];
    pAWBNVRAM[AWB_NVRAM_IDX_VHDR] = m_pNVRAM_3A->rAWBNVRAM[AWB_NVRAM_IDX_VHDR];

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetNVRAMParam(
    AWB_NVRAM_IDX_T eIdx,
    MVOID *a_pAWBNVRAM
 )
{
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if(err!=0)
    {
        MY_ERR("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    *pAWBNVRAM = m_pNVRAM_3A->rAWBNVRAM[eIdx];

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSaveNVRAMParam(
 )
{
    MRESULT err = S_AWB_OK;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA]\n");

    //m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx] = m_rAWBInitInput.rAWBNVRAM;

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_eSensorDev);
    if (err != 0)
    {
        MY_ERR("NvBufUtil::getInstance().write\n");
        return E_AWB_NVRAM_ERROR;
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBSetAWBMode(
    MINT32 a_AWBMode
)
{
    MY_LOG("[ACDK_CCT_OP_AWB_SET_AWB_MODE]\n");

    //setAWBMode(a_AWBMode);

    LIB3A_AWB_MODE_T eNewAWBMode = static_cast<LIB3A_AWB_MODE_T>(a_AWBMode);

    if (m_eAWBMode != eNewAWBMode)
    {
        m_eAWBMode = eNewAWBMode;
        m_bAWBModeChanged = MTRUE;
        MY_LOG("m_eAWBMode: %d\n", m_eAWBMode);
    }

    MY_LOG("[AWB Mode] = %d\n", a_AWBMode);


    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetAWBMode(
    MINT32 *a_pAWBMode,
    MUINT32 *a_pOutLen
)
{
    MY_LOG("[ACDK_CCT_OP_AWB_GET_AWB_MODE]\n");

    *a_pAWBMode = getAWBMode();

    *a_pOutLen = sizeof(MINT32);

    MY_LOG("[AWB Mode] = %d\n", *a_pAWBMode);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBGetLightProb(
    MVOID *a_pAWBLightProb,
    MUINT32 *a_pOutLen
)
{
    AWB_LIGHT_PROBABILITY_T *pAWBLightProb = reinterpret_cast<AWB_LIGHT_PROBABILITY_T*>(a_pAWBLightProb);

    MY_LOG("[ACDK_CCT_OP_AWB_GET_LIGHT_PROB]\n");

    m_pIAwbAlgo->getLightProb(*pAWBLightProb);

    *a_pOutLen = sizeof(AWB_LIGHT_PROBABILITY_T);

    return S_AWB_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPAWBBypassCalibration(
    MBOOL bBypassCalibration
)
{
    m_bAWBCalibrationBypassed =  bBypassCalibration;

    MY_LOG("[AWB Calibration Bypass] = %d\n", m_bAWBCalibrationBypassed);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBApplyNVRAMParam(
    MVOID *a_pFlashAWBNVRAM
)
{
    FLASH_AWB_NVRAM_T *pFlashAWBNVRAM = reinterpret_cast<FLASH_AWB_NVRAM_T*>(a_pFlashAWBNVRAM);

    MY_LOG("[ACDK_CCT_OP_AWB_APPLY_FLASH_AWB_PARA]\n");

    m_pNVRAM_3A->rFlashAWBNVRAM = *pFlashAWBNVRAM;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBGetNVRAMParam(
    MVOID *a_pFlashAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    FLASH_AWB_NVRAM_T *pFlashAWBNVRAM = reinterpret_cast<FLASH_AWB_NVRAM_T*>(a_pFlashAWBNVRAM);

    MY_LOG("[ACDK_CCT_OP_AWB_GET_FLASH_AWB_PARA]\n");

    // Get default AWB calibration data
    MRESULT err = S_AWB_OK;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A, MTRUE);
    if (err!=0)
    {
        MY_ERR("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

    *pFlashAWBNVRAM = m_pNVRAM_3A->rFlashAWBNVRAM;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AwbMgr::CCTOPFlashAWBSaveNVRAMParam(
 )
{
    MRESULT err = S_AWB_OK;

    MY_LOG("[ACDK_CCT_OP_AWB_SAVE_FLASH_AWB_PARA]\n");

    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_eSensorDev);
    if (err != 0)
    {
        MY_ERR("NvBufUtil::getInstance().write\n");
        return E_AWB_NVRAM_ERROR;
    }

    return err;
}
