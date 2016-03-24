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
#define LOG_TAG "aaa_state"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <hal/aaa/aaa_hal_raw.h>
#include <hal/aaa/state_mgr/aaa_state.h>
#include <hal/aaa/state_mgr/aaa_state_mgr.h>
#include <hal/aaa/ae_mgr/ae_mgr_if.h>
#include <mcu_drv.h>
#include <hal/aaa/af_mgr/af_mgr.h>
#include <hal/aaa/flash_mgr/flash_mgr.h>
#include <hal/aaa/buf_mgr/afo_buf_mgr.h>
#include <hal/aaa/awb_mgr/awb_mgr_if.h>
#include <hal/aaa/buf_mgr/aao_buf_mgr.h>
#include <hal/aaa/flicker/flicker_hal_base.h>
#include <hal/aaa/sensor_mgr/aaa_sensor_mgr.h>

using namespace NS3A;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IState
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



IState::
IState(char const*const pcszName, MINT32 sensorDevId, StateMgr* pStateMgr)
    : m_pHal3A(Hal3ARaw::getInstance(sensorDevId))
    , m_pcszName(pcszName)
    , m_SensorDevId(sensorDevId)
    , m_pStateMgr(pStateMgr)
    //, m_pScheduler(m_pHal3A->mpScheduler)
{
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateUninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateUninit::
StateUninit(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateUninit", sensorDevId, pStateMgr)
{
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Init
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateUninit::
sendIntent(intent2type<eIntent_Init>)
{
    MY_LOG("[StateUninit::sendIntent]<eIntent_Init>");

    // AAO DMA buffer init
    MINT32 i4SensorIdx = m_pHal3A->getSensorOpenIdx();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AAO DMA buffer init
    if (!IAAOBufMgr::getInstance().init(i4SensorIdx)) {
        MY_ERR("IAAOBufMgr::getInstance().init() fail");
        return E_3A_ERR;
    }

}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMA buffer init
    if (!AFOBufMgr::getInstance().init(m_SensorDevId, i4SensorIdx)) {
        MY_ERR("IAFOBufMgr::getInstance().init() fail");
        return E_3A_ERR;
    }
}
}

    // State transition: eState_Uninit --> eState_Init
    m_pStateMgr->transitState(eState_Uninit, eState_Init);

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateInit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateInit::
StateInit(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateInit", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInit::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("[StateInit::sendIntent]<eIntent_Uninit>");


if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AAO DMA buffer uninit
    if (!IAAOBufMgr::getInstance().uninit()) {
        MY_ERR("IAAOBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMA buffer uninit
    if (!AFOBufMgr::getInstance().uninit()) {
        MY_ERR("IAFOBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }
}
}

    // State transition: eState_Init --> eState_Uninit
    m_pStateMgr->transitState(eState_Init, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInit::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MY_LOG("[StateInit::sendIntent]<eIntent_CameraPreviewStart>");

    MRESULT err = S_3A_OK;
    MBOOL bRet = MTRUE;
    // Get parameters
    Param_T rParam;
    m_pStateMgr->m_bHasAEEverBeenStable = MFALSE;
    m_pHal3A->getParams(rParam);
    MINT32 i4SensorIdx = m_pHal3A->getSensorOpenIdx();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AAO DMAInit + AAStatEnable
    if (!IAAOBufMgr::getInstance().DMAInit()) {
        MY_ERR("IAAOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }
    if (!IAAOBufMgr::getInstance().AAStatEnable(MTRUE)) {
        MY_ERR("IAAOBufMgr::getInstance().AAStatEnable() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAInit + AFStatEnable
    if (!AFOBufMgr::getInstance().DMAInit()) {
        MY_ERR("IAFOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }
    if (!AFOBufMgr::getInstance().AFStatEnable(MTRUE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    // Flash init + cameraPreviewStart
    if (!FlashMgr::getInstance().init(m_SensorDevId, i4SensorIdx)) {
        MY_ERR("FlashMgr::getInstance().init() fail\n");
        return E_3A_ERR;
    }
    if (!FlashMgr::getInstance().cameraPreviewStart(m_SensorDevId)) {
        MY_ERR("FlashMgr::getInstance().PreviewStart() fail");
        return E_3A_ERR;
    }
}
    IAwbMgr::getInstance().setStrobeMode(m_SensorDevId,
        (FlashMgr::getInstance().isAFLampOn(m_SensorDevId)) ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);

if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB init
    err = IAwbMgr::getInstance().cameraPreviewInit(m_SensorDevId, m_SensorDevId, i4SensorIdx);
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().PreviewInit() fail");
        return E_3A_ERR;
    }
}
    IAeMgr::getInstance().setStrobeMode(m_SensorDevId,
        (FlashMgr::getInstance().isAFLampOn(m_SensorDevId)) ? MTRUE : MFALSE);

if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // AE init
    err = IAeMgr::getInstance().cameraPreviewInit(m_SensorDevId, i4SensorIdx, rParam);
    if (FAILED(err)) {
        MY_ERR("IAeMgr::getInstance().PreviewInit() fail, PvMode = %d\n", m_pHal3A->get3APreviewMode());
        return err;
    }
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF init
    err = AfMgr::getInstance().init(m_SensorDevId, i4SensorIdx);
    if (FAILED(err)) {
        MY_ERR("AfMgr::getInstance().init() fail\n");
        return err;
    }
}

if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    MY_LOG("FlickerHalBase sensor index: %d", i4SensorIdx);
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_SensorDevId, 1);
    FlickerHalBase::getInstance().open(m_SensorDevId, i4SensorIdx, m_pHal3A->getTGInfo());
}
}

    // Reset frame count to -2
    m_pStateMgr->resetFrameCount();

    // State transition: eState_Init --> eState_CameraPreview
    m_pStateMgr->transitState(eState_Init, eState_CameraPreview);

    return  S_3A_OK;
}

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateInit::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    return  S_3A_OK;
}
#endif
