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
#define LOG_TAG "aaa_state_camera_preview_n3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <mtkcam/core/featureio/pipe/aaa/aaa_hal.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr_n3d/aaa_state_n3d.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state_mgr.h>
#include <mtkcam/core/featureio/pipe/aaa/aaa_scheduler.h>

#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <flash_feature.h>
#include <af_mgr_if.h>
#include <afo_buf_mgr.h>
#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>
#include <lsc_mgr2.h>
#include <mtkcam/featureio/flicker_hal_base.h>
#include <aaa_sensor_mgr.h>

#include <mtkcam/featureio/ISync3A.h>

using namespace NS3Av3;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCameraPreviewN3d::
StateCameraPreviewN3d(MINT32 sensorDevId, StateMgr* pStateMgr)
    : StateCameraPreview(sensorDevId, pStateMgr)
{
    MY_LOG("[%s] sensorDevId(%d)", __FUNCTION__, sensorDevId);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
    if (ISync3AMgr::getInstance()->isActive()&&
        ISync3A::getInstance()->isSyncEnable())
    {
        MY_LOG("[%s] Sync 2A: Sensor(%d), Sync OFF", __FUNCTION__, m_SensorDevId);
        ISync3A::getInstance()->syncEnable(MFALSE);
    }
}
    return StateCameraPreview::sendIntent(intent2type<eIntent_CameraPreviewEnd>());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MINT32 i4SceneLv = 80;
    MBOOL bAEStable = MTRUE;
    MINT32 i4ActiveAeItem = 0;
    MINT32 i4SyncOpt = 0;
    MINT32 i4SyncFrmCount;

    // Update frame count
    m_pStateMgr->updateFrameCount();
    MY_LOG_IF(1
            , "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] enter, frameCnt=%d\n"
            , m_pStateMgr->getFrameCount());

    //BufInfo_T rBufInfo;
    BufInfo_T & rBufInfo = *(BufInfo_T*)(m_pStateMgr->mpAAOBuf);
    if (m_pStateMgr->getFrameCount() < 0) {// AAO statistics is not ready

        m_pHal3A->m_b3APvInitOK = MFALSE;
if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
        // Dequeue AAO DMA buffer
        IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);

        // Enqueue AAO DMA buffer
        IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);

        // Update AAO DMA base address for next frame
        IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);
}
}

        return S_3A_OK;
    }
    m_pHal3A->m_b3APvInitOK = MTRUE;

    i4SyncFrmCount = ISync3A::getInstance()->getFrameCount();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AAO DMA buffer
    if (m_pScheduler->jobAssignAndTimerStart(E_Job_AAO))
        IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);
    m_pScheduler->jobTimerEnd(E_Job_AAO);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] deQHwBufAAO done\n");
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, (m_pHal3A->get3APreviewMode() == EPv_Normal) ? MFALSE : MTRUE);
    bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
}
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB
    if (m_pScheduler->jobAssignAndTimerStart(E_Job_Awb))
    {
        i4SyncOpt |= ISync3A::E_SYNC3A_DO_AWB;
        IAwbMgr::getInstance().doPvAWB(m_SensorDevId, i4SyncFrmCount, bAEStable, i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr), MFALSE);
    }
    m_pScheduler->jobTimerEnd(E_Job_Awb);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAWB done\n");
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // AE
    i4ActiveAeItem = m_pScheduler->jobAssignAndTimerStart(E_Job_AeFlare);
    i4SyncOpt |= ((i4ActiveAeItem&E_AE_AE_CALC) ? ISync3A::E_SYNC3A_DO_AE : 0);
    //CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    IAeMgr::getInstance().doPvAE(m_SensorDevId, i4SyncFrmCount, reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
        i4ActiveAeItem&(E_AE_AE_CALC|E_AE_FLARE), 1, 0);
    m_pScheduler->jobTimerEnd(E_Job_AeFlare);
    //CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);     // Profiling End.
}
#if 1
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

    LscMgr2::E_TSF_CMD_T eTsfCmd = (0 == m_pStateMgr->getFrameCount()) ? LscMgr2::E_TSF_CMD_BATCH : LscMgr2::E_TSF_CMD_RUN;
    if (m_pScheduler->jobAssignAndTimerStart(E_Job_Lsc))
        LscMgr2::getInstance(static_cast<ESensorDev_T>(m_SensorDevId))->updateTsf(
            eTsfCmd, &rAwbInfo, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    m_pScheduler->jobTimerEnd(E_Job_Lsc);

    MY_LOG("lv(%d),cct(%d),rgain(%d),bgain(%d),ggain(%d),fluoidx(%d), dayflouidx(%d)",
            rAwbInfo.m_i4LV,
            rAwbInfo.m_u4CCT,
            rAwbInfo.m_RGAIN,
            rAwbInfo.m_GGAIN,
            rAwbInfo.m_BGAIN,
            rAwbInfo.m_FLUO_IDX,
            rAwbInfo.m_DAY_FLUO_IDX
            );
}
#endif
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Enqueue AAO DMA buffer
    if (m_pScheduler->jobAssignAndTimerStart(E_Job_IspValidate))
        IAAOBufMgr::getInstance().enqueueHwBuf(m_SensorDevId, rBufInfo);
    m_pScheduler->jobTimerEnd(E_Job_IspValidate);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] enQHwBufAAO done\n");

    // Update AAO DMA base address for next frame
    if (m_pScheduler->jobAssignAndTimerStart(E_Job_IspValidate, MFALSE))
        IAAOBufMgr::getInstance().updateDMABaseAddr(m_SensorDevId);
    m_pScheduler->jobTimerEnd(E_Job_IspValidate);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] updateDMAAddr done\n");
}

if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    if (bIsRecording == MFALSE)
    {
        if (m_pScheduler->jobAssignAndTimerStart(E_Job_Flicker))
        {
            FlickerInput flkIn;
            FlickerOutput flkOut;
            AE_MODE_CFG_T previewInfo;
            IAeMgr::getInstance().getPreviewParams(m_SensorDevId, previewInfo);
            flkIn.aeExpTime = previewInfo.u4Eposuretime;
            flkIn.afFullStat = IAfMgr::getInstance().getFLKStat(m_SensorDevId);
            if(flkIn.afFullStat.bValid==1)
            {
                FlickerHalBase::getInstance().update(m_SensorDevId, &flkIn, &flkOut);
                if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
                {
                    MY_LOG("setaeflicker 60hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 1);
                }
                else
                {
                    MY_LOG("setaeflicker 50hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 0);
                }
            }
            else
                MY_LOG("skip flicker");

        }
        m_pScheduler->jobTimerEnd(E_Job_Flicker);

    }
}

    if (ISync3A::getInstance()->isSyncEnable())
    {
        // 2A sync: independent AE/AWB
        MY_LOG("[%s] Sync 2A: Sensor(%d) =============", __FUNCTION__, m_SensorDevId);
        MINT32 i4Sync = ISync3AMgr::getSync3A()->sync(m_SensorDevId, i4SyncOpt);
        // apply
        MY_LOG("[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);
    }

    // AAOUpdate is already true if AE needs calculation.
    IAeMgr::getInstance().doPvAE(m_SensorDevId, i4SyncFrmCount, reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
        (i4ActiveAeItem&E_AE_AE_APPLY), (i4ActiveAeItem&(E_AE_AE_CALC|E_AE_FLARE))?0:1);

}


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
    if (ISync3AMgr::getInstance()->isActive()&&
        ISync3AMgr::getSync3A()->isSyncEnable())
    {
        MY_LOG("[%s] Sync 2A: Sensor(%d), Sync OFF", __FUNCTION__, m_SensorDevId);
        ISync3AMgr::getSync3A()->syncEnable(MFALSE);
        ISync3AMgr::getSync3A(ISync3AMgr::E_SYNC3AMGR_CAPTURE)->syncEnable(MTRUE);
    }
}
    return StateCameraPreview::sendIntent(intent2type<eIntent_PrecaptureStart>());
}


