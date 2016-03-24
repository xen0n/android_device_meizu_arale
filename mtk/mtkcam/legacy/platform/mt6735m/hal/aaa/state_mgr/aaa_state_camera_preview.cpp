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
#define LOG_TAG "aaa_state_camera_preview"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <hal/aaa/aaa_hal_raw.h>
#include <hal/aaa/state_mgr/aaa_state.h>
#include <hal/aaa/state_mgr/aaa_state_mgr.h>
//#include <mtkcam/core/featureio/pipe/aaa/aaa_scheduler.h>
//#include <aaa_common_custom.h>

#include <hal/aaa/ae_mgr/ae_mgr_if.h>
#include <hal/aaa/flash_mgr/flash_mgr.h>
#include <flash_feature.h>
#include <mcu_drv.h>
#include <hal/aaa/af_mgr/af_mgr.h>
#include <hal/aaa/buf_mgr/afo_buf_mgr.h>
#include <hal/aaa/awb_mgr/awb_mgr_if.h>
#include <hal/aaa/buf_mgr/aao_buf_mgr.h>
#include <hal/aaa/lsc_mgr/lsc_mgr2.h>
#include <hal/aaa/flicker/flicker_hal_base.h>
#include <hal/aaa/sensor_mgr/aaa_sensor_mgr.h>

#include <cutils/properties.h>

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCameraPreview::
StateCameraPreview(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateCameraPreview", sensorDevId, pStateMgr)
{

    char cLogLevel[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    m_3ALogEnable = atoi(cLogLevel);
    if ( m_3ALogEnable == 0 ) {
        ::property_get("debug.camera.log.hal3a", cLogLevel, "0");
        m_3ALogEnable = atoi(cLogLevel);
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_Uninit>)
{
    //ERROR HANDLING:
    //in preview state, eIntent_Uninit wont be called in normal cases,
    //its only for flow protection
    // = CameraPreviewEnd then Uninit
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_Uninit>");

    MRESULT err = S_3A_OK;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

    // ----- CameraPreviewEnd operations go here -----
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB uninit
    err = IAwbMgr::getInstance().uninit(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // AE uninit
    err = IAeMgr::getInstance().uninit(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAeMgr::getInstance().uninit() fail\n");
        return err;
    }
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF uninit
    err = AfMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("IAfMgr::getInstance().uninit() fail\n");
        return err;
    }
}
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    //Flash uninit
    // Flash cameraPreviewEnd + uninit
    if (!FlashMgr::getInstance().cameraPreviewEnd(m_SensorDevId)) {
        MY_ERR("FlashMgr::getInstance().cameraPreviewEnd() fail\n");
        return E_3A_ERR;
    }
    if (!FlashMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("FlashMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }
}
if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    FlickerHalBase::getInstance().close(m_SensorDevId);
}
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AAO DMAUninit + AAStatEnable
    if (!IAAOBufMgr::getInstance().DMAUninit()) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAAOBufMgr::getInstance().AAStatEnable(MFALSE)) {
        MY_ERR("IAAOBufMgr::getInstance().AAStatEnable() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAUninit + AFStatEnable
    if (!AFOBufMgr::getInstance().DMAUninit()) {
        MY_ERR("IAFOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!AFOBufMgr::getInstance().AFStatEnable(MFALSE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

    // ----- Uninit operations go here -----
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

    // State transition: eState_CameraPreview --> eState_Uninit
    m_pStateMgr->transitState(eState_CameraPreview, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MRESULT err = S_3A_OK;

    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_CameraPreviewEnd>");

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB uninit
    err = IAwbMgr::getInstance().uninit(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // AE uninit
    err = IAeMgr::getInstance().uninit(m_SensorDevId);
    if (FAILED(err)) {
        MY_ERR("IAeMgr::getInstance().uninit() fail\n");
        return err;
    }
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    // AF uninit
    err = AfMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("IAfMgr::getInstance().uninit() fail\n");
        return err;
    }
}
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    //Flash uninit
    // Flash cameraPreviewEnd + uninit
    if (!FlashMgr::getInstance().cameraPreviewEnd(m_SensorDevId)) {
        MY_ERR("FlashMgr::getInstance().cameraPreviewEnd() fail\n");
        return E_3A_ERR;
    }
    if (!FlashMgr::getInstance().uninit(m_SensorDevId)) {
        MY_ERR("FlashMgr::getInstance().uninit() fail\n");
        return E_3A_ERR;
    }
}
if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    FlickerHalBase::getInstance().close(m_SensorDevId);
}
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AAO DMAUninit + AAStatEnable
    if (!IAAOBufMgr::getInstance().DMAUninit()) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAAOBufMgr::getInstance().AAStatEnable(MFALSE)) {
        MY_ERR("IAAOBufMgr::getInstance().AAStatEnable() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAUninit + AFStatEnable
    if (!AFOBufMgr::getInstance().DMAUninit()) {
        MY_ERR("IAFOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!AFOBufMgr::getInstance().AFStatEnable(MFALSE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

}

    // State transition: eState_CameraPreview --> eState_Init
    m_pStateMgr->transitState(eState_CameraPreview, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;

    // Update frame count
    m_pStateMgr->updateFrameCount();
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG
            , "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] enter, frameCnt=%d\n"
            , m_pStateMgr->getFrameCount());

    //BufInfo_T rBufInfo;
    BufInfo_T & rBufInfo = *(BufInfo_T*)(m_pStateMgr->mpAAOBuf);
    if (m_pStateMgr->getFrameCount() < 0) {// AAO statistics is not ready

        m_pHal3A->m_b3APvInitOK = MFALSE;
if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
        // Dequeue AAO DMA buffer
        IAAOBufMgr::getInstance().dequeueHwBuf(rBufInfo);

        // Enqueue AAO DMA buffer
        IAAOBufMgr::getInstance().enqueueHwBuf(rBufInfo);

        // Update AAO DMA base address for next frame
        IAAOBufMgr::getInstance().updateDMABaseAddr(IAAOBufMgr::getInstance().getNextHwBuf());
}
}

        return S_3A_OK;
    }
    m_pHal3A->m_b3APvInitOK = MTRUE;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AAO DMA buffer
    IAAOBufMgr::getInstance().dequeueHwBuf(rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] deQHwBufAAO done\n");
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
}

if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // AE
    //CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    if (m_pStateMgr->m_bHasAEEverBeenStable == MFALSE)
    {
        if (IAeMgr::getInstance().IsAEStable(m_SensorDevId)==MTRUE) m_pStateMgr->m_bHasAEEverBeenStable = MTRUE;
    }
    if(0/*CUST_LOCK_AE_DURING_CAF()*/)
    {
        if (AfMgr::getInstance().isFocusFinish() || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
            (m_pStateMgr->m_bHasAEEverBeenStable == MFALSE)) //guarantee AE can doPvAE at beginning, until IsAEStable()=1
        {
             IAeMgr::getInstance().setAFAELock(m_SensorDevId, MFALSE);
        }
        else
        {
             IAeMgr::getInstance().setAFAELock(m_SensorDevId, MTRUE);
        }
    }
    else //always do AE, no matter whether lens are moving or not
    {
        IAeMgr::getInstance().setAFAELock(m_SensorDevId, MFALSE);
    }

    IAeMgr::getInstance().doPvAE(m_SensorDevId, m_pStateMgr->getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr), MFALSE, 1, MFALSE);
    if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
    {
    #if USE_AE_THD
        m_pHal3A->mbPostAESenThd = MTRUE;
        m_pHal3A->postToAESenThread();
    #else
        //MINT32 i4Start = m_pHal3A->getCurrentHwId();
        IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
        //MINT32 i4End = m_pHal3A->getCurrentHwId();
        //MY_LOG_IF((i4Start!=i4End), "[I2C](%d,%d)", i4Start, i4End);
    #endif
    }

    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAE done\n");

    //CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);     // Profiling End.
}
#if 1

if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB
    IAwbMgr::getInstance().doPvAWB(m_SensorDevId, m_pStateMgr->getFrameCount(), bAEStable, i4AoeCompLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAWB done\n");
}

if (ENABLE_LSC & m_pHal3A->m_3ACtrlEnable) {
    // TSF
    AaaTimer TSFPrepTimer("TSFPrep", m_SensorDevId, (m_pHal3A->m_3ALogEnable & EN_3A_SCHEDULE_LOG));
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
    TSFPrepTimer.End();
    LscMgr2::getInstance(static_cast<ESensorDev_T>(m_SensorDevId))->updateTsf(
            eTsfCmd, &rAwbInfo, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

    MY_LOG_IF(m_3ALogEnable, "lv(%d),cct(%d),rgain(%d),bgain(%d),ggain(%d),fluoidx(%d), dayflouidx(%d)",
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
    IAAOBufMgr::getInstance().enqueueHwBuf(rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] enQHwBufAAO done\n");

    // Update AAO DMA base address for next frame
    IAAOBufMgr::getInstance().updateDMABaseAddr(IAAOBufMgr::getInstance().getNextHwBuf());
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] updateDMAAddr done\n");
}

if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
#if 1
    if (m_pStateMgr->mbIsRecording == MFALSE)
    {
        {
            FlickerInput flkIn;
            FlickerOutput flkOut;
            AE_MODE_CFG_T previewInfo;
            IAeMgr::getInstance().getPreviewParams(m_SensorDevId, previewInfo);
            flkIn.aeExpTime = previewInfo.u4Eposuretime;
            flkIn.afFullStat = AfMgr::getInstance().getFLKStat();
            //MY_LOG("qq1 bValid %d", flkIn.afFullStat.bValid);
            if(flkIn.afFullStat.bValid==1)
            {
                if(m_pHal3A->getTGInfo() == CAM_TG_1)
                {
                FlickerHalBase::getInstance().update(m_SensorDevId, &flkIn, &flkOut);
                    //MY_LOG("qq2 CAM_TG_1 %d", flkOut.flickerResult);
                }
                else
                {
                    int flkResult;
                    FlickerHalBase::getInstance().getFlickerResult(flkResult);
                    flkOut.flickerResult = flkResult;

                    //MY_LOG("qq2 CAM_TG_2 %d", flkOut.flickerResult);
                }

                if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
                {
                    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "setaeflicker 60hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 1);
                }
                else
                {
                    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "setaeflicker 50hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 0);
                }

            }
            else
            {
                int flkResult;
                FlickerHalBase::getInstance().getFlickerResult(flkResult);
                flkOut.flickerResult = flkResult;
                if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
                {
                    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "setaeflicker 60hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 1);
            }
            else
                {
                    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "setaeflicker 50hz");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_SensorDevId, 0);
                }
                //MY_LOG("qq1 skip bValid %d %d %d", flkIn.afFullStat.bValid,__LINE__,flkOut.flickerResult);
                MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "skip flicker");
            }

        }

    }
#endif
}
}


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFUpdate>)
{

    AaaTimer AFUpdateTimer("AFUpdate", m_SensorDevId, (m_pHal3A->m_3ALogEnable & EN_3A_SCHEDULE_LOG));


    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enter\n");
    MRESULT err = S_3A_OK;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {

    //BufInfo_T rBufInfo;
    BufInfo_T & rBufInfo = *(BufInfo_T*)(m_pStateMgr->mpAFOBuf);

    //MY_LOG("[StateCameraPreview::sendIntent]<eIntent_AFUpdate>");
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AFO DMA buffer
    AFOBufMgr::getInstance().dequeueHwBuf(rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] deQHwBufAFO done\n");
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
/*
    AE_MODE_CFG_T rPreviewInfo;
    CameraArea_T  rWinSize;
    AEMeterArea_T rAeWinSize;
    MUINT8 iYvalue;
    AE2AFInfo_T rAEInfo;
    FrameOutputParam_T AEFrameParam;
    //get AF window from AF, and set to AE meter, then get Y value.
    AfMgr::getInstance().getAFRefWin(rWinSize);
    rAeWinSize.i4Left  =rWinSize.i4Left;
    rAeWinSize.i4Right =rWinSize.i4Right;
    rAeWinSize.i4Top   =rWinSize.i4Top;
    rAeWinSize.i4Bottom=rWinSize.i4Bottom;
    rAeWinSize.i4Weight=rWinSize.i4Weight;
    AeMgr::getInstance().getAEMeteringYvalue(rAeWinSize, &iYvalue);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "AFAEInfo[L]%d[R]%d[T]%d[B]%d\n",rAeWinSize.i4Left,rAeWinSize.i4Right,rAeWinSize.i4Top,rAeWinSize.i4Bottom);

    //get current AE info, and write to AF for reference.
    AeMgr::getInstance().getAEBlockYvalues(rAEInfo.aeBlockV, 25);
    AeMgr::getInstance().getPreviewParams(rPreviewInfo);
    AeMgr::getInstance().getRTParams(AEFrameParam);
    rAEInfo.i4IsAEStable= AeMgr::getInstance().IsAEStable();
    rAEInfo.i4ISO=rPreviewInfo.u4RealISO;
    rAEInfo.i4SceneLV=AeMgr::getInstance().getLVvalue(MTRUE);
    rAEInfo.iYvalue=(MINT64)iYvalue;
    rAEInfo.ishutterValue=AEFrameParam.u4PreviewShutterSpeed_us;

    MUINT16 u2YCnt;
    AeMgr::getInstance().getAEMeteringBlockAreaValue(rAeWinSize, m_pStateMgr->m_uAEBlockAreaY, &u2YCnt);
    rAEInfo.i4AEBlockAreaYCnt = u2YCnt;
    rAEInfo.pAEBlockAreaYvalue=m_pStateMgr->m_uAEBlockAreaY;
    rAEInfo.i4IsFlashFrm=FlashMgr::getInstance().isAFLampOn(m_SensorDevId);

    AfMgr::getInstance().setAE2AFInfo(rAEInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "AFAEInfo[AEStable]%d[ISO]%d[LV]%d[Y]%d\n",rAEInfo.i4IsAEStable,rAEInfo.i4ISO,rAEInfo.i4SceneLV,(MINT32)rAEInfo.iYvalue);
*/
    AaaTimer doAFTimer("doAF", m_SensorDevId, (m_pHal3A->m_3ALogEnable & EN_3A_SCHEDULE_LOG));
    AfMgr::getInstance().doAF(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    doAFTimer.End();

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);    // Profiling Start.
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] doAF done\n");
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Enqueue AFO DMA buffer
    AFOBufMgr::getInstance().enqueueHwBuf(rBufInfo);
    MY_LOG_IF(m_pHal3A->m_3ALogEnable & EN_3A_FLOW_LOG, "[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enQHwBuf done\n");
}
}

    AFUpdateTimer.End();
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_PrecaptureStart>");

    // Init
if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    IAeMgr::getInstance().setAeMeterAreaEn(m_SensorDevId, 1);
}
}
    m_pStateMgr->resetPrecapState(); //reset Precap state
    // State transition: eState_CameraPreview --> eState_Precapture
    m_pStateMgr->transitState(eState_CameraPreview, eState_Precapture);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CaptureStart>)
{
    // Update frame count
    m_pStateMgr->updateFrameCount();
    MY_LOG("sendIntent(intent2type<eIntent_CaptureStart>) line(%d), frame(%d)",__LINE__, m_pStateMgr->getFrameCount());

    MINT32 i4AEEnable, i4AWBEnable;
    MUINT32 u4Length;
    MINT32 bIsFlashOn = MFALSE;
    //BufInfo_T rBufInfo;
    BufInfo_T & rBufInfo = *(BufInfo_T*)(m_pStateMgr->mpAAOBuf);

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    FlashMgr::getInstance().capCheckAndFireFlash_Start(m_SensorDevId);
    bIsFlashOn = FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId);
}
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB: update AWB statistics config
    IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF));
    // AWB init for capture
    //AwbMgr::getInstance().cameraCaptureInit();
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    IAeMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? MTRUE : MFALSE));
    // AE: update capture parameter
    IAeMgr::getInstance().doCapAE(m_SensorDevId);
}

    // Dequeue AAO DMA buffer
    IAAOBufMgr::getInstance().dequeueHwBuf(rBufInfo);

    // Enqueue AAO DMA buffer
    IAAOBufMgr::getInstance().enqueueHwBuf(rBufInfo);

    // Update AAO DMA base address for next frame
    IAAOBufMgr::getInstance().updateDMABaseAddr(IAAOBufMgr::getInstance().getNextHwBuf());


    //IAfMgr::getInstance().setBestShotConfig(m_SensorDevId);
/*
    // if the ae/awb don't enable, don't need to enable the AAO
    AeMgr::getInstance().CCTOPAEGetEnableInfo(&i4AEEnable, &u4Length);
    AwbMgr::getInstance().CCTOPAWBGetEnableInfo(&i4AWBEnable, &u4Length);

    // Get operation mode and sensor mode for CCT and EM
    if((i4AWBEnable != MFALSE) && (i4AEEnable != MFALSE))
    {
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
    }
*/
}
    // State transition: eState_Precapture --> eState_Capture
    //m_pStateMgr->transitState(eState_Precapture, eState_Capture);

    return  S_3A_OK;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_AFStart>");

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
        IAeMgr::getInstance().setAeMeterAreaEn(m_SensorDevId, 1);
}
}
    m_pStateMgr->setNextState(eState_Invalid); //reset 3A Next state
    m_pStateMgr->resetAFState(); //only single entrance point: EAFState_T=0
    m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
    FlashMgr::getInstance().notifyAfEnter(m_SensorDevId);
    // State transition: eState_CameraPreview --> eState_AF
    m_pStateMgr->transitState(eState_CameraPreview, eState_AF);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}
#if 0

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_RecordingStart>");
    bIsRecording = MTRUE;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    MY_LOG("flash mode=%d LIB3A_FLASH_MODE_AUTO=%d triger=%d"
            ,(int)FlashMgr::getInstance().getFlashMode(m_SensorDevId)
            ,(int)LIB3A_FLASH_MODE_AUTO
            ,(int)IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));

    if ( FlashMgr::getInstance().getFlashMode(m_SensorDevId) == LIB3A_FLASH_MODE_AUTO &&
         IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId))
    {
        MY_LOG("flash auto then turn on for recording");
        IAeMgr::getInstance().setStrobeMode(m_SensorDevId, MTRUE);
        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_ON);
        FlashMgr::getInstance().setTorchOnOff(m_SensorDevId, 1);
    }
    else
    {
        //keep flash status unchanged
        ALOGD("flash status unchanged");
    }

    FlashMgr::getInstance().videoRecordingStart(m_SensorDevId);
}
}
if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_SensorDevId, 0);
    //FlickerHalBase::getInstance().recordingStart(m_SensorDevId);
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    IAfMgr::getInstance().setCamMode(m_SensorDevId, eAppMode_VideoMode);
}
}
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_RecordingEnd>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_RecordingEnd>");
    bIsRecording = MFALSE;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    FlashMgr::getInstance().videoRecordingEnd(m_SensorDevId);

    //flash may turn off now
    IAwbMgr::getInstance().setStrobeMode(m_SensorDevId,
        (FlashMgr::getInstance().isAFLampOn(m_SensorDevId)) ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF);
    IAeMgr::getInstance().setStrobeMode(m_SensorDevId,
        (FlashMgr::getInstance().isAFLampOn(m_SensorDevId)) ? MTRUE : MFALSE);
}
if (ENABLE_FLICKER & m_pHal3A->m_3ACtrlEnable) {
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_SensorDevId, 1);
    //FlickerHalBase::getInstance().recordingEnd(m_SensorDevId);
}
if (ENABLE_AF & m_pHal3A->m_3ACtrlEnable) {
    IAfMgr::getInstance().setCamMode(m_SensorDevId, eAppMode_DefaultMode);
}

}

    return  S_3A_OK;
}
#endif
