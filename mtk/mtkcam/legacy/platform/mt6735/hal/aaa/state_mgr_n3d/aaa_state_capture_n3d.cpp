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
#define LOG_TAG "aaa_state_capture_n3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <mtkcam/core/featureio/pipe/aaa/aaa_hal.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr_n3d/aaa_state_n3d.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state_mgr.h>
#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <af_mgr_if.h>
#include <afo_buf_mgr.h>
#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>
#include <lsc_mgr2.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/featureio/flicker_hal_base.h>
#include <aaa_sensor_mgr.h>
#include <mtkcam/featureio/ISync3A.h>


using namespace NS3Av3;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCaptureN3d::
StateCaptureN3d(MINT32 sensorDevId, StateMgr* pStateMgr)
    : StateCapture(sensorDevId, pStateMgr)
{
    MY_LOG("[%s] sensorDevId(%d)", __FUNCTION__, sensorDevId);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCaptureN3d::
sendIntent(intent2type<eIntent_CaptureEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>)  line=%d", __LINE__);
    BufInfo_T rBufInfo;
    MRESULT err;
    MINT32 i4SceneLV = 80;
    MINT32 i4SyncOpt = 0;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AAO DMA buffer
    IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    i4SceneLV = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
}
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // One-shot AWB
    i4SyncOpt |= ISync3A::E_SYNC3A_DO_AWB;
    IAwbMgr::getInstance().doCapAWB(m_SensorDevId, i4SceneLV, reinterpret_cast<MVOID *>(rBufInfo.virtAddr), MFALSE);

    MY_LOG("IAwbMgr::getInstance().doCapAWB() END");
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    if (m_pHal3A->get3ACapMode() == (ECapMode_P2_Cal|ECapMode_P2_Set))
    {
        //Capture Flare compensate
        IAeMgr::getInstance().doCapFlare(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId) );
        MY_LOG("IAeMgr::getInstance().doCapFlare() END");
    }
}
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    FlashMgr::getInstance().capCheckAndFireFlash_End(m_SensorDevId);
}

if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_LSC & m_pHal3A->m_3ACtrlEnable) {
    // TSF
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
    LscMgr2::TSF_AWB_INFO rAwbInfo;
    rAwbInfo.m_i4LV        = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_SensorDevId);
    rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
    rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;

    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_SensorDevId))->updateTsf(
        LscMgr2::E_TSF_CMD_BATCH_CAP, &rAwbInfo, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

    MY_LOG("lv %d, cct %d, rgain %d, bgain %d, ggain %d, fluo idx %d, day flou idx %d\n",
            rAwbInfo.m_i4LV,
            rAwbInfo.m_u4CCT,
            rAwbInfo.m_RGAIN,
            rAwbInfo.m_GGAIN,
            rAwbInfo.m_BGAIN,
            rAwbInfo.m_FLUO_IDX,
            rAwbInfo.m_DAY_FLUO_IDX
            );
}
}
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Enqueue AAO DMA buffer
    IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);

    // Update AAO DMA address
    IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);
    // AAStatEnable + AAO DMAUninit
    if (!IAAOBufMgr::getInstance().AAStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAAOBufMgr::getInstance().AAStatEnable(MFALSE) fail");
        return E_3A_ERR;
    }
    if (!IAAOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    IAeMgr::getInstance().setStrobeMode(m_SensorDevId, MFALSE);
}

    // 2A sync: independent AE/AWB
    MY_LOG("[%s]CapEnd Sync 2A: Sensor(%d) =============", __FUNCTION__, m_SensorDevId);
    MINT32 i4Sync = ISync3AMgr::getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->sync(m_SensorDevId, i4SyncOpt);
    MY_LOG("[%s]CapEnd Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);

}
    MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>) END");

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCaptureN3d::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MY_LOG("[%s]", __FUNCTION__);
    if (ISync3AMgr::getInstance()->isActive()&&
        !ISync3AMgr::getSync3A()->isSyncEnable())
    {
        MY_LOG("[%s] Sync 2A: Sensor(%d), Sync ON", __FUNCTION__, m_SensorDevId);
        ISync3AMgr::getSync3A()->syncEnable(MTRUE);
    }
    return StateCapture::sendIntent(intent2type<eIntent_CameraPreviewStart>());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart: for CTS only
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCaptureN3d::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MY_LOG("[%s]", __FUNCTION__);
    if (ISync3AMgr::getInstance()->isActive()&&
        !ISync3AMgr::getSync3A()->isSyncEnable())
    {
        MY_LOG("[%s] Sync 2A: Sensor(%d), Sync ON", __FUNCTION__, m_SensorDevId);
        ISync3AMgr::getSync3A()->syncEnable(MTRUE);
    }
    return StateCapture::sendIntent(intent2type<eIntent_CamcorderPreviewStart>());
}



