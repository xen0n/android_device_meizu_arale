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

#include <isp_tuning_mgr.h>
#include <isp_mgr.h>
#include <nvram_drv_mgr.h>
#include <nvram_drv.h>
#include <mtkcam/hal/IHalSensor.h>
#include "awb_mgr.h"

using namespace NSCam;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
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
MINT32
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
MINT32
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
MINT32
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
MINT32
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
    IspTuningMgr::getInstance().setAWBInfo(m_rAWBOutput.rAWBInfo); // update AWB info
    IspTuningMgr::getInstance().forceValidate();


    MY_LOG("[RGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4R);
    MY_LOG("[GGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4G);
    MY_LOG("[BGain] = %d\n", m_rAWBOutput.rPreviewAWBGain.i4B);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AwbMgr::CCTOPAWBApplyNVRAMParam(
    MVOID *a_pAWBNVRAM
)
{
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2]\n");

    m_rAWBInitInput.rAWBNVRAM = *pAWBNVRAM;

    //m_pIAwbAlgo->updateAWBParam(m_rAWBInitInput, m_rAWBStatCfg);

    m_bAWBModeChanged = MTRUE;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AwbMgr::CCTOPAWBGetNVRAMParam(
    MVOID *a_pAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    AWB_NVRAM_T *pAWBNVRAM = reinterpret_cast<AWB_NVRAM_T*>(a_pAWBNVRAM);

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");

    getNvramData();

    //*pAWBNVRAM = m_pNVRAM_3A->rAWBNVRAM;

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AwbMgr::CCTOPAWBSaveNVRAMParam(
 )
{
    CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum = DUAL_CAMERA_NONE_SENSOR;
    MRESULT err = S_AWB_OK;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA]\n");

    NvramDrvBase* pNvramDrvObj = NvramDrvBase::createInstance();

//ME14@@    NSNvram::BufIF<NVRAM_CAMERA_3A_STRUCT>*const pBufIF_3A = pNvramDrvObj->getBufIF< NVRAM_CAMERA_3A_STRUCT>();


    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    SensorStaticInfo rSensorStaticInfo;
    MUINT32 u4SensorID;

    switch  ( m_eSensorDev )
    {
    case ESensorDev_Main:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        eSensorEnum = DUAL_CAMERA_MAIN_SENSOR;
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        eSensorEnum = DUAL_CAMERA_SUB_SENSOR;
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        eSensorEnum = DUAL_CAMERA_MAIN_2_SENSOR;
        break;
    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", m_eSensorDev);
        goto lbExit;
    }

    u4SensorID = rSensorStaticInfo.sensorDevID;

    MY_LOG("u4SensorID = %d\n", u4SensorID);

//ME14@@    m_pNVRAM_3A = pBufIF_3A->getRefBuf(eSensorEnum, u4SensorID);

//    m_pNVRAM_3A->rAWBNVRAM = m_rAWBInitInput.rAWBNVRAM;

//ME14@@    pBufIF_3A->flush(eSensorEnum, u4SensorID);

lbExit:

    if ( pNvramDrvObj )
        pNvramDrvObj->destroyInstance();

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
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
MINT32
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
MINT32
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

