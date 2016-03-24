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
#define LOG_TAG "aaa_state_recording"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <aaa_hal.h>
#include "aaa_state.h"
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <isp_tuning.h>
#include <awb_mgr_if.h>
#include <buf_mgr.h>
//#include <mtkcam/hal/sensor_hal.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv_common/isp_reg.h>
#include <af_mgr_if.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_param.h>
#include <ae_mgr.h>
#include <flash_mgr.h>
#include <lsc_mgr2.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include "aaa_state_flow_custom.h"

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateRecording
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateRecording::
StateRecording()
    : IState("StateRecording")
{
}


MRESULT StateRecording::exitPreview()
{
    MY_LOG("StateRecording::exitPreview line=%d",__LINE__);
    MRESULT err;

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    IAwbMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AAO DMA / state disable again
    err = BufMgr::getInstance().AAStatEnable(MFALSE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AAStatEnable(MFALSE) fail\n");
        return err;
    }

    err = BufMgr::getInstance().DMAUninit(camdma2type<ECamDMA_AAO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAunInit(ECamDMA_AAO) fail\n");
        return err;
    }

    // AFO DMA / state disable again
    err = BufMgr::getInstance().AFStatEnable(MFALSE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AFStatEnable(MFALSE) fail\n");
        return err;
    }

    err = BufMgr::getInstance().DMAUninit(camdma2type<ECamDMA_AFO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAunInit(ECamDMA_AFO) fail\n");
        return err;
    }


    transitState(eState_Recording, eState_Init);
    return  S_3A_OK;
}


MRESULT
StateRecording::
sendIntent(intent2type<eIntent_CamcorderPreviewEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_CamcorderPreviewEnd> line=%d", __LINE__);
    exitPreview();
    FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);

    // Flicker uninit
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());
    return  S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingStart>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingEndd>");

    // State transition: eState_Recording --> eState_CamcorderPreview
    transitState(eState_Recording, eState_CamcorderPreview);

    //if(FlashMgr::getInstance().getFlashMode()==LIB3A_FLASH_MODE_AUTO)
    //if(FlashMgr::getInstance().getFlashMode()!=LIB3A_FLASH_MODE_FORCE_TORCH)
    //    FlashMgr::getInstance().setAFLampOnOff(0);
    FlashMgr::getInstance().videoRecordingEnd(m_pHal3A->getSensorDev());
    //FlickerHalBase::getInstance().recordingEnd(m_pHal3A->getSensorDev());

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StateRecording::sendIntent]<eIntent_VsyncUpdate>");

    // Update frame count
    updateFrameCount();

    // Dequeue AAO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

    //MTK_SWP_PROJECT_START
    // F858
    AWB_OUTPUT_T rAWBOutput;
    MINT32 i4SensorDev = m_pHal3A->getSensorDev();
    IAwbMgr::getInstance().getAWBOutput(i4SensorDev, rAWBOutput);
    LscMgr2::TSF_AWB_INFO rAwbInfo;
    rAwbInfo.m_i4LV     = IAeMgr::getInstance().getLVvalue(i4SensorDev,MTRUE);
    rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(i4SensorDev);
    rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
    rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
    LscMgr2::E_TSF_CMD_T eTsfCmd = (0 == getFrameCount()) ? LscMgr2::E_TSF_CMD_BATCH : LscMgr2::E_TSF_CMD_RUN;
    LscMgr2::getInstance(static_cast<ESensorDev_T>(i4SensorDev))->updateTsf(
            eTsfCmd, &rAwbInfo, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

    MY_LOG("lv %d, cct %d, rgain %d, bgain %d, ggain %d, fluo idx %d, day flou idx %d\n",
            rAwbInfo.m_i4LV,
            rAwbInfo.m_u4CCT,
            rAwbInfo.m_RGAIN,
            rAwbInfo.m_GGAIN,
            rAwbInfo.m_BGAIN,
            rAwbInfo.m_FLUO_IDX,
            rAwbInfo.m_DAY_FLUO_IDX
            );

    //MTK_SWP_PROJECT_END

    // AWB
    MINT32 i4SceneLv = IAeMgr::getInstance().getLVvalue(m_pHal3A->getSensorDev(), MTRUE);
    CPTLog(Event_Pipe_3A_AWB, CPTFlagStart);    // Profiling Start.
    AaaTimer localTimer("doVideoAWB", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
    IAwbMgr::getInstance().doPvAWB(m_pHal3A->getSensorDev(), getFrameCount(),IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev()), i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    localTimer.printTime();
    CPTLog(Event_Pipe_3A_AWB, CPTFlagEnd);     // Profiling End.

    // AE
    AWB_OUTPUT_T _a_rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(i4SensorDev, _a_rAWBOutput);
    CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    if (sm_bHasAEEverBeenStable == MFALSE)
    {
        if (IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev())) sm_bHasAEEverBeenStable = MTRUE;
    }
    if (isAELockedDuringCAF())
    {
        if (IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev()) || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
        (sm_bHasAEEverBeenStable == MFALSE)) //guarantee AE can doPvAE at beginning, until IsAEStable()=1
        {
            AaaTimer localTimer("doPvAE", i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
            MBOOL isNeedUpdateI2C;
            IAeMgr::getInstance().doPvAEmonitor(m_pHal3A->getSensorDev(), getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
                0, 1, 0, isNeedUpdateI2C);
            IAeMgr::getInstance().doPvAE(m_pHal3A->getSensorDev(), getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
                0, 1, 0);
            if(isNeedUpdateI2C)
            {
                MY_LOG_IF(1, "updateSensorbyI2C start\n");
                IAeMgr::getInstance().updateSensorbyI2C(m_pHal3A->getSensorDev());
                MY_LOG_IF(1, "updateSensorbyI2C end\n");
            }
            localTimer.printTime();
        }
    }
    else //always do AE, no matter whether lens are moving or not
    {
        AaaTimer localTimer("doPvAE", i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
        MBOOL isNeedUpdateI2C;
        IAeMgr::getInstance().doPvAEmonitor(m_pHal3A->getSensorDev(), getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            0, 1, 0, isNeedUpdateI2C);
        IAeMgr::getInstance().doPvAE(m_pHal3A->getSensorDev(), getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            0, 1, 0);
        if(isNeedUpdateI2C)
        {
            MY_LOG_IF(1, "updateSensorbyI2C start\n");
            IAeMgr::getInstance().updateSensorbyI2C(m_pHal3A->getSensorDev());
            MY_LOG_IF(1, "updateSensorbyI2C end\n");
        }
        localTimer.printTime();
    }

    CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);     // Profiling End.

    // Enqueue AAO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

    // Update AAO DMA base address for next frame
    err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFUpdate>");

    BufInfo_T rBufInfo;

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);

    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);     // Profiling End.

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFStart>");

    // Init
    if(IAeMgr::getInstance().IsDoAEInPreAF(m_pHal3A->getSensorDev()) == MTRUE)   {
        MY_LOG("Enter PreAF state");
        transitAFState(eAFState_PreAF);
    }
    else   {
        MY_LOG("Enter AF state");
        IAfMgr::getInstance().triggerAF(m_pHal3A->getSensorDev());
        transitAFState(eAFState_AF);
    }

    // State transition: eState_CameraPreview --> eState_AF
    //transitState(eState_CameraPreview, eState_AF);

    transitState(eState_Recording, eState_AF);
    FlashMgr::getInstance().notifyAfEnter(m_pHal3A->getSensorDev());



    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}

