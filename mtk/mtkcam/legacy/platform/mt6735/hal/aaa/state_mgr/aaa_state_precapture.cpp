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
#define LOG_TAG "aaa_state_precapture"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
#define MTK_LOG_ENABLE 1
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <aaa_hal_raw.h>
#include <aaa_state.h>
#include <aaa_state_mgr.h>
//#include <mtkcam/core/featureio/pipe/aaa/aaa_scheduler.h>
#include <aaa_common_custom.h>

#include <ae_mgr_if.h>
#include <flash_mgr.h>
#include <flash_feature.h>
#include <mcu_drv.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <afo_buf_mgr.h>
#include <awb_mgr_if.h>
#include <aao_buf_mgr.h>
#include <lsc_mgr2.h>
#include <flicker_hal_base.h>
#include <aaa_sensor_mgr.h>
#include <aaa_sensor_buf_mgr.h>
#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
//#include <aaa_common_custom.h>
#include <flash_param.h>
#include <flash_tuning_custom.h>


using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePrecapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatePrecapture::
StatePrecapture(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StatePrecapture", sensorDevId, pStateMgr)
{
}

#if 0
//WRONG FLOW:
//PrecaptureStart shouldn't happen in StatePrecapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    MY_LOG("sendIntent(intent2type<eIntent_PrecaptureStart>) line=%d",__LINE__);
    return  S_3A_OK;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MRESULT err = S_3A_OK;

    MY_LOG("[StatePrecapture::sendIntent]<eIntent_CameraPreviewEnd>");

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB uninit
    if (!IAwbMgr::getInstance().uninit(m_SensorDevId)) {
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
    err = IAfMgr::getInstance().uninit(m_SensorDevId);
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
    if (!IAAOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAAOBufMgr::getInstance().AAStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAAOBufMgr::getInstance().AAStatEnable() fail");
        return E_3A_ERR;
    }
    if (!IAEBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAEBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAEBufMgr::getInstance().AAStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAEBufMgr::getInstance().AAStatEnable() fail");
        return E_3A_ERR;
    }
}
if (ENABLE_AFOBUF & m_pHal3A->m_3ACtrlEnable) {
    // AFO DMAUninit + AFStatEnable
    if (!IAFOBufMgr::getInstance().DMAUninit(m_SensorDevId)) {
        MY_ERR("IAFOBufMgr::getInstance().DMAUninit() fail");
        return E_3A_ERR;
    }
    if (!IAFOBufMgr::getInstance().AFStatEnable(m_SensorDevId, MFALSE)) {
        MY_ERR("IAFOBufMgr::getInstance().AFStatEnable() fail");
        return E_3A_ERR;
    }
}

}

    // State transition: eState_Precapture --> eState_Init
    m_pStateMgr->transitState(eState_Precapture, eState_Init);

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_PrecaptureEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_PrecaptureEnd>) line=%d",__LINE__);
    //IAeMgr::getInstance().setExp(m_SensorDevId, 3000);

    //reset is moved to PreviewEnd

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StatePrecapture::sendIntent]<eIntent_VsyncUpdate> line=%d, frameCnt=%d, EPrecapState=%d"
        , __LINE__
        , m_pStateMgr->getFrameCount()
        , static_cast<int>(m_pStateMgr->getPrecapState()));
/*
    if(m_pHal3A->isReadyToCapture())
    {
        MY_LOG("VsyncUpdate ReadyToCapture=1, skip");
        return S_3A_OK;
    }
*/

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
    // Dequeue AAO DMA buffer
        IAAOBufMgr::getInstance().dequeueHwBuf(m_SensorDevId, rBufInfo);
}
}
    m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
    if (m_pStateMgr->getPrecapState() == ePrecap_AF_BeforeFlash)
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AF_BeforeFlash>());
        if (err != S_3A_OK) MY_ERR("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AF_BeforeFlash) return(%d)\n", err);
    }
    if ((m_pStateMgr->getPrecapState() == ePrecap_AE) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AE>(), &rBufInfo);
        if (err != S_3A_OK) MY_ERR("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AE) return(%d)\n", err);
    }
    //Note: not "else if ..." because after AE finishes, flash should continue to make Precapture faster.
    if ((m_pStateMgr->getPrecapState() == ePrecap_Flash) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_Flash>(), &rBufInfo);
        if (err != S_3A_OK) MY_ERR("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_Flash) return(%d)\n", err);
    }
    if ((m_pStateMgr->getPrecapState() == ePrecap_AF_AfterFlash) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AF_AfterFlash>());
        if (err != S_3A_OK) MY_ERR("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AF_AfterFlash) return(%d)\n", err);
    }


if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_LSC & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    //FIXME: move all TSF parameter mapping operations and print log into a common function
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
        LscMgr2::E_TSF_CMD_RUN, &rAwbInfo, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

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
}
}

    //Note: not "else if ...", if flash finishes this frame, then we announce ReadyToCapture to Middleware
    if ((m_pStateMgr->getPrecapState() == ePrecap_Num) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        // at this moment, all operation of Precapture must have finished. Then wait for Capture command
        if(FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId)) FlashMgr::getInstance().setCapPara(m_SensorDevId);
        m_pStateMgr->resetPrecapState();
        //m_pHal3A->notifyReadyToCapture();

        //PrecaptureEnd
        FlashMgr::getInstance().endPrecapture(m_SensorDevId);
        IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);
        //#warning "FIXME: switch back to StatePreview"

        m_pHal3A->mpCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_READY2CAP, 0, 0, 0);
        m_pStateMgr->transitState(eState_Precapture, eState_CameraPreview);
        MY_LOG("VsyncUpdate notifyReadyToCapture, resetPrecapState");

        IAeMgr::getInstance().setAEState2Converge(m_SensorDevId);
    }
    return  err;
}

MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AE>, MVOID* pBufInfo)
{
    MY_LOG("StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AE)");

    MRESULT err = S_3A_OK;
    MBOOL bIsStrobeFired = MFALSE;
    BufInfo_T* pBuf = reinterpret_cast<BufInfo_T*>(pBufInfo);
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;

if (!(
(ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) &&
(ENABLE_AE & m_pHal3A->m_3ACtrlEnable) &&
(ENABLE_AWB & m_pHal3A->m_3ACtrlEnable)
))
{m_pStateMgr->proceedPrecapState();return err;}

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // Update frame count
    m_pStateMgr->updateFrameCount();
    // One-shot AE or strobe AE
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    IAeMgr::getInstance().doPreCapAE(m_SensorDevId, m_pStateMgr->getFrameCount(), bIsStrobeFired, reinterpret_cast<MVOID *>(pBuf->virtAddr), 0, MTRUE, 0);
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
    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);

    // workaround for iVHDR
    MUINT32 u4AFSGG1Gain;
    IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
    IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);

    /*NeedUpdate*///CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);     // Profiling End.
}
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    if(bAEStable) // AE is stable
    {
        IAeMgr::getInstance().doBackAEInfo(m_SensorDevId);

        // One-shot AWB without strobe
        IAwbMgr::getInstance().doPreCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->virtAddr));

        // both one-shot AE and one-shot AWB are done
        m_pStateMgr->proceedPrecapState();
    }
}
}
    return err;
}


MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_Flash>, MVOID* pBufInfo)
{
    MY_LOG("StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_Flash)");
    int isFlashCalibration=0;
    MRESULT err = S_3A_OK;
    BufInfo_T* pBuf = reinterpret_cast<BufInfo_T*>(pBufInfo);
    FlashExePara para;
    FlashExeRep rep;
    FLASH_AWB_PASS_FLASH_INFO_T passFlashAwbData;
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    para.staBuf =reinterpret_cast<MVOID *>(pBuf->virtAddr);

if (!(
(ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) &&
(ENABLE_AE & m_pHal3A->m_3ACtrlEnable) &&
(ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) &&
(ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable)
))
{m_pStateMgr->proceedPrecapState();return err;}

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    // flash
    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Strobe, CPTFlagStart);     // Profiling start.

    AWB_STAT_PARAM_T rAWBStatParam;

    switch (m_SensorDevId)
    {
    case ESensorDev_Main: //  Main Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        break;
    default:
        MY_ERR("m_eSensorDev = %d", m_SensorDevId);
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    }
    para.staX = rAWBStatParam.i4WindowNumX;
    para.staY = rAWBStatParam.i4WindowNumY;
    para.isBurst = 0;
    AWB_OUTPUT_T awb_out;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, awb_out);
    para.NonPreferencePrvAWBGain = awb_out.rPreviewFullWBGain;
    para.PreferencePrvAWBGain = awb_out.rPreviewAWBGain;
    para.i4AWBCCT = awb_out.rAWBInfo.i4CCT;
    FlashMgr::getInstance().doPfOneFrame(m_SensorDevId, &para, &rep);

    if(rep.isFlashCalibrationMode==1)
        LscMgr2::getInstance(static_cast<ESensorDev_T>(m_SensorDevId))->setTsfOnOff(0);

    isFlashCalibration = rep.isFlashCalibrationMode;


    /*NeedUpdate*///CPTLog(Event_Pipe_3A_Strobe, CPTFlagEnd);     // Profiling End.

    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
}
}

    //FIXME @@ YawbANU}(precaptureUAWB)

if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {


    if(rep.isCurFlashOn==0)
        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_OFF);
    else
        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_ON);

    IAwbMgr::getInstance().doPreCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->virtAddr));

}



if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {

    if(isFlashCalibration==1)
        FlashMgr::getInstance().cctCaliFixAwb2(m_SensorDevId);

}
}

}


    if(rep.isEnd==1)
    {
        MY_LOG("Flash-AE precapture metering process is done");

        if (isFlashAWBv2Enabled()) {
            passFlashAwbData.flashDuty = rep.nextDuty;
            passFlashAwbData.flashStep = rep.nextStep;
            passFlashAwbData.flashAwbWeight = rep.flashAwbWeight;
            IAwbMgr::getInstance().setFlashAWBData(m_SensorDevId, passFlashAwbData);
        }

        m_pStateMgr->proceedPrecapState();
    }

    return err;


}

MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_BeforeFlash>)
{
    MY_LOG("StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AF_BeforeFlash)");
    m_pStateMgr->proceedPrecapState(); return S_3A_OK;

    MRESULT err = S_3A_OK;
/*
    MY_LOG("[sendPrecapIntent] CUST_ENABLE_PRECAPTURE_AF(%d), CUST_PRECAPTURE_AF_AFTER_PREFLASH(%d)\n"
        , CUST_ENABLE_PRECAPTURE_AF()
        , CUST_PRECAPTURE_AF_AFTER_PREFLASH());
    if(CUST_ENABLE_PRECAPTURE_AF() && !CUST_PRECAPTURE_AF_AFTER_PREFLASH())
    {
        MY_LOG("[sendPrecapIntent] To execute Precapture AF before Flash\n");
        int isDoAF;
        int hasHw;
        FlashMgr::getInstance().hasFlashHw(m_SensorDevId, hasHw);
        isDoAF = cust_isNeedDoPrecapAF_v2(
                MFALSE,
                IAfMgr::getInstance().isFocused(m_SensorDevId),
                FlashMgr::getInstance().getFlashMode(m_SensorDevId),
                FlashMgr::getInstance().getAfLampMode(m_SensorDevId),
                IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));
        if(isDoAF==1 && hasHw==1)
        {
            ALOGD("[sendPrecapIntent]isDoAF=1 line=%d",__LINE__);
            IAfMgr::getInstance().setAFMode(m_SensorDevId, AF_MODE_AFS);

            //refer to StateCameraPreview::sendIntent(intent2type<eIntent_AFStart>)
            m_pStateMgr->setNextState(eState_Precapture); //reset 3A Next state
            m_pStateMgr->resetAFState(); //only single entrance point: EAFState_T=0
            m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
            m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF = 1;
            // State transition: eState_CameraPreview --> eState_AF
            m_pStateMgr->transitState(eState_Precapture, eState_AF);


            FlashMgr::getInstance().notifyAfEnter(m_SensorDevId);
            IAfMgr::getInstance().autoFocus(m_SensorDevId);

            m_pStateMgr->proceedPrecapState();

            return S_3A_OK;
        }
        else
        {
            ALOGD("[sendPrecapIntent]isDoAF=0 || no flash hw");
        }
    }
    else
*/
    {
        MY_LOG("[sendPrecapIntent] NOT To execute Precapture AF before Flash\n");
    }
    m_pStateMgr->proceedPrecapState();

    return err;


}


MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_AfterFlash>)
{
    MY_LOG("StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AF_AfterFlash)");
    m_pStateMgr->proceedPrecapState(); return S_3A_OK;

    MRESULT err = S_3A_OK;
/*
    MY_LOG("[sendPrecapIntent] CUST_ENABLE_PRECAPTURE_AF(%d), CUST_PRECAPTURE_AF_AFTER_PREFLASH(%d)\n"
        , CUST_ENABLE_PRECAPTURE_AF()
        , CUST_PRECAPTURE_AF_AFTER_PREFLASH());
    if(CUST_ENABLE_PRECAPTURE_AF() && CUST_PRECAPTURE_AF_AFTER_PREFLASH())
    {
        MY_LOG("[sendPrecapIntent] To execute Precapture AF after Flash\n");
        int isDoAF;
        int hasHw;
        FlashMgr::getInstance().hasFlashHw(m_SensorDevId, hasHw);
        isDoAF = cust_isNeedDoPrecapAF_v2(
                MFALSE,
                IAfMgr::getInstance().isFocused(m_SensorDevId),
                FlashMgr::getInstance().getFlashMode(m_SensorDevId),
                FlashMgr::getInstance().getAfLampMode(m_SensorDevId),
                IAeMgr::getInstance().IsStrobeBVTrigger(m_SensorDevId));
        if(isDoAF==1 && hasHw==1)
        {
            ALOGD("[sendPrecapIntent]isDoAF=1 line=%d",__LINE__);
            if(FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId)) FlashMgr::getInstance().setPfParaToAe(m_SensorDevId);
            IAfMgr::getInstance().setAFMode(m_SensorDevId, AF_MODE_AFS);

            //refer to StateCameraPreview::sendIntent(intent2type<eIntent_AFStart>)
            m_pStateMgr->setNextState(eState_Precapture); //reset 3A Next state
            m_pStateMgr->resetAFState(); //only single entrance point: EAFState_T=0
            m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
            m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF = 1;
            // State transition: eState_CameraPreview --> eState_AF
            m_pStateMgr->transitState(eState_Precapture, eState_AF);


            FlashMgr::getInstance().notifyAfEnter(m_SensorDevId);
            IAfMgr::getInstance().autoFocus(m_SensorDevId);

            m_pStateMgr->proceedPrecapState();

            return S_3A_OK;
        }
        else
        {
            ALOGD("[sendPrecapIntent]isDoAF=0 || no flash hw");
        }
    }
    else
*/
    {
        MY_LOG("[sendPrecapIntent] NOT To execute Precapture AF after Flash\n");
    }
    m_pStateMgr->proceedPrecapState();

    return err;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG("sendIntent(intent2type<eIntent_AFUpdate>) line=%d",__LINE__);

    return  S_3A_OK;
}

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_CaptureStart>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CaptureStart>) line =%d",__LINE__);
    MINT32 i4AEEnable, i4AWBEnable;
    MUINT32 u4Length;
    MINT32 bIsFlashOn = MFALSE;

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
    FlashMgr::getInstance().capCheckAndFireFlash_Start(m_SensorDevId);
    bIsFlashOn = FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId);
}
if (ENABLE_AWB & m_pHal3A->m_3ACtrlEnable) {
    // AWB: update AWB statistics config
    IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF));
    // AWB init for capture
    IAwbMgr::getInstance().cameraCaptureInit(m_SensorDevId);
}
if (ENABLE_AE & m_pHal3A->m_3ACtrlEnable) {
    IAeMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? MTRUE : MFALSE));
    // AE: update capture parameter
    IAeMgr::getInstance().doCapAE(m_SensorDevId);
}

    //IAfMgr::getInstance().setBestShotConfig(m_SensorDevId);

    // if the ae/awb don't enable, don't need to enable the AAO
    IAeMgr::getInstance().CCTOPAEGetEnableInfo(m_SensorDevId, &i4AEEnable, &u4Length);
    IAwbMgr::getInstance().CCTOPAWBGetEnableInfo(m_SensorDevId, &i4AWBEnable, &u4Length);

    // Get operation mode and sensor mode for CCT and EM
    if((i4AWBEnable != MFALSE) && (i4AEEnable != MFALSE))
    {
if (ENABLE_AAOBUF & m_pHal3A->m_3ACtrlEnable) {
        // AAO DMAInit + AAStatEnable
    if (!IAAOBufMgr::getInstance().DMAInit(m_SensorDevId)) {
        MY_ERR("IAAOBufMgr::getInstance().DMAInit() fail");
        return E_3A_ERR;
    }
        if (!IAAOBufMgr::getInstance().AAStatEnable(m_SensorDevId, MTRUE)) {
            MY_ERR("IAAOBufMgr::getInstance().AAStatEnable() fail");
            return E_3A_ERR;
        }
}

    }
}
    // State transition: eState_Precapture --> eState_Capture
    m_pStateMgr->transitState(eState_Precapture, eState_Capture);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CameraPreviewEnd>) line=%d",__LINE__);

    //reset is moved from PrecaptureEnd
    m_pHal3A->resetReadyToCapture();

if (ENABLE_3A_GENERAL & m_pHal3A->m_3ACtrlEnable) {
if (ENABLE_FLASH & m_pHal3A->m_3ACtrlEnable) {
        FlashMgr::getInstance().endPrecapture(m_SensorDevId);
}
}
    IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_AFEnd>) line=%d",__LINE__);

    return  S_3A_OK;
}
#endif

