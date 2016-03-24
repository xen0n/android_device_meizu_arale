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
#define LOG_TAG "aaa_state_camcorder_preview"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif
#define MTK_LOG_ENABLE 1
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <aaa_hal.h>
#include "aaa_state.h"
#include "aaa_state_mgr.h"

#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>
#include <aaa_sensor_buf_mgr.h>

using namespace NS3A;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCamcorderPreview::
StateCamcorderPreview(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateCamcorderPreview", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_Uninit>");

    // AAO DMA buffer uninit
    if (!IAAOBufMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }

    // AAE DMA buffer uninit
    if (!IAEBufMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAEBufMgr::getInstance().uninit() fail");
        return E_3A_ERR;
    }

    // AWB uninit
    if (!IAwbMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAwbMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }

    // State transition: eState_CamcorderPreview --> eState_Uninit
    m_pStateMgr->transitState(eState_CamcorderPreview, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MRESULT err;

    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_CamcorderPreviewStart>");

    // AAO DMA init
    if (!IAAOBufMgr::getInstance().DMAInit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_CamcorderPreviewEnd>)
{
    MRESULT err;

    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_CamcorderPreviewEnd>");

    // AAO DMA uninit
    if (!IAAOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }

    // AWB uninit
    if (!IAwbMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("IAwbMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }

    // State transition: eState_CamcorderPreview --> eState_Init
    m_pStateMgr->transitState(eState_CamcorderPreview, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_VsyncUpdate>");

    // Update frame count
    m_pStateMgr->updateFrameCount();

    BufInfo_T rBufInfo;
    if (m_pStateMgr->getFrameCount() < 0) {// AAO statistics is not ready

        // Dequeue AAO DMA buffer
        IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);

        // Enqueue AAO DMA buffer
        IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);

        // Update AAO DMA base address for next frame
        IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);

        return S_3A_OK;
    }

    // Dequeue AAO DMA buffer
    IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);

    // AWB
    MINT32 i4SceneLv = 80;//FIXME: IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    MBOOL bAEStable = MTRUE;//FIXME: IAeMgr::getInstance().IsAEStable(m_SensorDevId)
    IAwbMgr::getInstance().doPvAWB(m_SensorDevId, m_pStateMgr->getFrameCount(), bAEStable, i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

    // Enqueue AAO DMA buffer
    IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);

    // Update AAO DMA base address for next frame
    IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MRESULT err = S_3A_OK;
#if ENABLE_3A_CODE_BODY /*NeedUpdate*/
    BufInfo_T rBufInfo;

    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_AFUpdate>");

    // Dequeue AFO DMA buffer
    IAFOBufMgr::getInstance().dequeueHwBuf(1/*sensordev*/, rBufInfo);

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);    // Profiling Start.

    // Enqueue AFO DMA buffer
    IAFOBufMgr::getInstance().enqueueHwBuf(1/*sensordev*/, rBufInfo);
#endif /*NeedUpdate*/
    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_RecordingStart>");

    // Init

#if ENABLE_3A_CODE_BODY /*NeedUpdate*/
    ALOGD("flash mode=%d LIB3A_FLASH_MODE_AUTO=%d triger=%d",
        (int)FlashMgr::getInstance().getFlashMode(m_SensorDevId),
        (int)LIB3A_FLASH_MODE_AUTO,
        (int)IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));



#ifdef MTK_VIDEO_AUTO_FLASH_SUPPORT

    //if(AeMgr::getInstance().IsStrobeBVTrigger())
    if(FlashMgr::getInstance().getFlashMode(m_SensorDevId)==LIB3A_FLASH_MODE_AUTO && IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId))
    {
        ALOGD("video flash on");
        FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 1);
    }
    else
    {
        ALOGD("video flash off");
    }

#else

#endif
    FlashMgr::getInstance().videoRecordingStart(m_SensorDevId);
#endif /*NeedUpdate*/
    // State transition: eState_CamcorderPreview --> eState_Recording
    m_pStateMgr->transitState(eState_CamcorderPreview, eState_Recording);

    /*NeedUpdate*///FlickerHalBase::getInstance().recordingStart(m_SensorDevId);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_AFStart>");
#if ENABLE_3A_CODE_BODY /*NeedUpdate*/
    // Init
    if(IAeMgr::getInstance().IsDoAEInPreAF(m_SensorDevId) == MTRUE)   {
        transitAFState(eAFState_PreAF);
    }
    else   {
        IAfMgr::getInstance().triggerAF(m_SensorDevId);
        transitAFState(eAFState_AF);
    }
#endif /*NeedUpdate*/

    // State transition: eState_CamcorderPreview --> eState_AF
    m_pStateMgr->transitState(eState_CamcorderPreview, eState_AF);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart: for CTS only
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCamcorderPreview::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    MY_LOG("[StateCamcorderPreview::sendIntent]<eIntent_PrecaptureStart>");

    // State transition: eState_CamcorderPreview --> eState_Precapture
    m_pStateMgr->transitState(eState_CamcorderPreview, eState_Precapture);

    return  S_3A_OK;
}

