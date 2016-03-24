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
#include <ae_param.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>
#include <flash_mgr.h>
#include <dbg_isp_param.h>
#include <isp_mgr.h>
#include <flash_param.h>
#include <isp_tuning_mgr.h>
#include <lsc_mgr2.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include "aaa_state_flow_custom.h"
#include "flash_feature.h"
#include <isp_tuning_mgr.h>

using namespace NS3A;
using namespace NSIspTuning;

 extern int g_isAFLampOnInAfState;
 extern EState_T g_preStateForAe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCameraPreview::
StateCameraPreview()
    : IState("StateCameraPreview")
    , bIsRecording(MFALSE)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_Uninit>");

    // AAO DMA buffer uninit
    BufMgr::getInstance().uninit();

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    IAwbMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flicker uninit
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);


    // State transition: eState_CameraPreview --> eState_Uninit
    transitState(eState_CameraPreview, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MRESULT err;

    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_CameraPreviewStart>");

    // AAO DMA / state enable again
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AAO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AAO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AAStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AAStatEnable(MTRUE) fail\n");
        return err;
    }

    // AFO DMA / state enable again
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AFO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AFO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AFStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AFStatEnable(MTRUE) fail\n");
        return err;
    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MRESULT err;

    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_CameraPreviewEnd>");

    FlashMgr::getInstance().cameraPreviewEnd(m_pHal3A->getSensorDev());
    //FlickerHalBase::getInstance().cameraPreviewEnd(m_pHal3A->getSensorDev());
    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    IAwbMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flicker uninit
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());

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
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);
}

    // State transition: eState_CameraPreview --> eState_Init
    transitState(eState_CameraPreview, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] enter\n");
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_VsyncUpdate>");

    // Update frame count
    updateFrameCount();

    if (getFrameCount() < 0) {// AA statistics is not ready
        // Dequeue AAO DMA buffer
        //BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

        // Enqueue AAO DMA buffer
        //BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

        // Update AAO DMA base address for next frame
        err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));
        //IspDebug::getInstance().dumpIspDebugMessage();
        return err;
    }

    // Dequeue AAO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] deQHwBufAAO done\n");


    // AWB: FIXME: pass LV info
    MINT32 i4SceneLv = IAeMgr::getInstance().getLVvalue(m_pHal3A->getSensorDev(), MFALSE);
    CPTLog(Event_Pipe_3A_AWB, CPTFlagStart);    // Profiling Start.
    AaaTimer localTimer("doPvAWB", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
    IAwbMgr::getInstance().doPvAWB(m_pHal3A->getSensorDev(), getFrameCount(),IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev()), i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    localTimer.printTime();
    CPTLog(Event_Pipe_3A_AWB, CPTFlagEnd);     // Profiling End.
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAWB done\n");

    // AE
    //pass WB gain info
    if(g_preStateForAe==eState_AF && g_isAFLampOnInAfState==1)
    {
          //ALOGD("meter disable");
        IAeMgr::getInstance().setAeMeterAreaEn(m_pHal3A->getSensorDev(), 0);
    }
    AWB_OUTPUT_T _a_rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_pHal3A->getSensorDev(), _a_rAWBOutput);
    CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    if (sm_bHasAEEverBeenStable == MFALSE)
    {
        if (IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev()))
            sm_bHasAEEverBeenStable = MTRUE;
    }
    if (isAELockedDuringCAF())
    {
        if (IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev()) || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
            (sm_bHasAEEverBeenStable == MFALSE)) //guarantee AE can doPvAE at beginning, until IsAEStable()=1
        {
            AaaTimer localTimer("doPvAE", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
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
        AaaTimer localTimer("doPvAE", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
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
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAE done\n");

    IspTuningMgr::GMA_AE_DYNAMIC_INFO dynamicInfo;
    dynamicInfo.bStable = IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev());
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC, (MINTPTR)&dynamicInfo, 0);
    // Enqueue AAO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

    // Update AAO DMA base address for next frame
    err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

    //MTK_SWIP_PROJECT_START
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
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] LscUpdateTSF done\n");
    //MTK_SWIP_PROJECT_END
    if (bIsRecording == MFALSE)
    {
        CPTLog(Event_Pipe_3A_Flicker, CPTFlagStart);    // Profiling Start.
        AaaTimer localTimer("Flicker", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));

            FlickerInput flkIn;
            FlickerOutput flkOut;
            AE_MODE_CFG_T previewInfo;
            IAeMgr::getInstance().getPreviewParams(m_pHal3A->getSensorDev(), previewInfo);
            flkIn.aeExpTime = previewInfo.u4Eposuretime;
            flkIn.afFullStat = IAfMgr::getInstance().getFLKStat(m_pHal3A->getSensorDev());
            //MY_LOG("qq1 bValid %d", flkIn.afFullStat.bValid);
            if(flkIn.afFullStat.bValid==1)
            {
                //if(m_pHal3A->getTGInfo() == CAM_TG_1)
                //{
                FlickerHalBase::getInstance().update(m_pHal3A->getSensorDev(), &flkIn, &flkOut);
                //}
                //else
                //{
                //    int flkResult;
                //    FlickerHalBase::getInstance().getFlickerResult(flkResult);
                //    flkOut.flickerResult = flkResult;

                    //MY_LOG("qq2 CAM_TG_2 %d", flkOut.flickerResult);
                //}

                if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
                {
                    MY_LOG_IF(sm_3APvLogEnable, "setaeflicker 60hz+");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_pHal3A->getSensorDev(), 1);
                }
                else
                {
                    MY_LOG_IF(sm_3APvLogEnable, "setaeflicker 50hz+");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_pHal3A->getSensorDev(), 0);
                }

            }
            else
            {
                int flkResult;
                FlickerHalBase::getInstance().getFlickerResult(m_pHal3A->getSensorDev(), flkResult);
        MY_LOG_IF(sm_3APvLogEnable,"flk=%d, m_SensorDevId=%d", flkResult,m_pHal3A->getSensorDev() );
                flkOut.flickerResult = flkResult;
                if(flkOut.flickerResult == HAL_FLICKER_AUTO_60HZ)
                {
                    MY_LOG_IF(sm_3APvLogEnable, "setaeflicker 60hz-");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_pHal3A->getSensorDev(), 1);
                }
                else
                {
                    MY_LOG_IF(sm_3APvLogEnable, "setaeflicker 50hz-");
                    IAeMgr::getInstance().setAEAutoFlickerMode(m_pHal3A->getSensorDev(), 0);
                }
                //MY_LOG("qq1 skip bValid %d %d %d", flkIn.afFullStat.bValid,__LINE__,flkOut.flickerResult);
                MY_LOG_IF(sm_3APvLogEnable, "skip flicker");
            }


        localTimer.printTime();
        CPTLog(Event_Pipe_3A_Flicker, CPTFlagEnd);    // Profiling End.
    }

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enter\n");
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_AFUpdate>");

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_AFUpdate>] deQHwBufAFO done\n");

    AE_MODE_CFG_T rPreviewInfo;
    CameraArea_T  rWinSize;
    AEMeterArea_T rAeWinSize;
    MUINT8 iYvalue;
    AE2AFInfo_T rAEInfo;
    FrameOutputParam_T AEFrameParam;
    //get AF window from AF, and set to AE meter, then get Y value.
    IAfMgr::getInstance().getAFRefWin(m_pHal3A->getSensorDev(), rWinSize);
    rAeWinSize.i4Left  =rWinSize.i4Left;
    rAeWinSize.i4Right =rWinSize.i4Right;
    rAeWinSize.i4Top   =rWinSize.i4Top;
    rAeWinSize.i4Bottom=rWinSize.i4Bottom;
    rAeWinSize.i4Weight=rWinSize.i4Weight;
    IAeMgr::getInstance().getAEMeteringYvalue(m_pHal3A->getSensorDev(), rAeWinSize, &iYvalue);
    MY_LOG_IF(sm_3APvLogEnable, "AFAEInfo[L]%d[R]%d[T]%d[B]%d\n",rAeWinSize.i4Left,rAeWinSize.i4Right,rAeWinSize.i4Top,rAeWinSize.i4Bottom);

    //get current AE info, and write to AF for reference.
    IAeMgr::getInstance().getAEBlockYvalues(m_pHal3A->getSensorDev(), rAEInfo.aeBlockV, 25);
    IAeMgr::getInstance().getPreviewParams(m_pHal3A->getSensorDev(), rPreviewInfo);
    IAeMgr::getInstance().getRTParams(m_pHal3A->getSensorDev(), AEFrameParam);
    rAEInfo.i4IsAEStable= IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev());
    rAEInfo.i4ISO=rPreviewInfo.u4RealISO;
    rAEInfo.i4SceneLV=IAeMgr::getInstance().getLVvalue(m_pHal3A->getSensorDev(),MTRUE);
    rAEInfo.iYvalue=(MINT64)iYvalue;
    rAEInfo.ishutterValue=AEFrameParam.u4PreviewShutterSpeed_us;

    MUINT16 u2YCnt;
    MUINT8 uAEBlockAreaY[120*90];
    IAeMgr::getInstance().getAEMeteringBlockAreaValue(m_pHal3A->getSensorDev(),rAeWinSize, uAEBlockAreaY, &u2YCnt);
    rAEInfo.i4AEBlockAreaYCnt = u2YCnt;
    rAEInfo.pAEBlockAreaYvalue = uAEBlockAreaY;
    rAEInfo.i4IsFlashFrm=FlashMgr::getInstance().isAFLampOn(m_pHal3A->getSensorDev());

    IAfMgr::getInstance().setAE2AFInfo(m_pHal3A->getSensorDev(), rAEInfo);
    MY_LOG_IF(sm_3APvLogEnable & EN_3A_FLOW_LOG, "AFAEInfo[AEStable]%d[ISO]%d[LV]%d[Y]%d\n",rAEInfo.i4IsAEStable,rAEInfo.i4ISO,rAEInfo.i4SceneLV,(MINT32)rAEInfo.iYvalue);

    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);    // Profiling Start.
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_AFUpdate>] doAF done\n");

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);
    MY_LOG_IF(sm_3APvLogEnable,"[StateCameraPreview::sendIntent<eIntent_AFUpdate>] enQHwBuf done\n");

    return  err;
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

    IAeMgr::getInstance().setAeMeterAreaEn(m_pHal3A->getSensorDev(), 1);
    // State transition: eState_CameraPreview --> eState_Precapture
    transitState(eState_CameraPreview, eState_Precapture);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CaptureStart>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_CaptureStart>");

    transitState(eState_CameraPreview, eState_Capture);

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
    IAeMgr::getInstance().setAeMeterAreaEn(m_pHal3A->getSensorDev(),1);
    // State transition: eState_CameraPreview --> eState_AF
    transitState(eState_CameraPreview, eState_AF);
    FlashMgr::getInstance().notifyAfEnter(m_pHal3A->getSensorDev());

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateCameraPreview::sendIntent]<eIntent_RecordingStart>");
    bIsRecording = MTRUE;
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_SET_GMA_SCENARIO, IspTuningMgr::E_GMA_SCENARIO_VIDEO, 0);

    ALOGD("flash mode=%d LIB3A_FLASH_MODE_AUTO=%d triger=%d",
            (int)FlashMgr::getInstance().getFlashMode(m_pHal3A->getSensorDev()),
            (int)LIB3A_FLASH_MODE_AUTO,
            (int)IAeMgr::getInstance().IsStrobeBVTrigger(m_pHal3A->getSensorDev()));


    //if(AeMgr::getInstance().IsStrobeBVTrigger())
    if(FlashMgr::getInstance().getFlashMode(m_pHal3A->getSensorDev())==LIB3A_FLASH_MODE_AUTO && IAeMgr::getInstance().IsStrobeBVTrigger(m_pHal3A->getSensorDev()))
    {
        ALOGD("video flash on");
        FlashMgr::getInstance().setTorchOnOff(m_pHal3A->getSensorDev(), 1);
    }
    else
    {
        ALOGD("video flash off");
    }

    FlashMgr::getInstance().videoRecordingStart(m_pHal3A->getSensorDev());
    //FlickerHalBase::getInstance().recordingStart(m_pHal3A->getSensorDev());
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_pHal3A->getSensorDev(), 0);

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
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_SET_GMA_SCENARIO, IspTuningMgr::E_GMA_SCENARIO_PREVIEW, 0);

    //if(FlashMgr::getInstance().getFlashMode()==LIB3A_FLASH_MODE_AUTO)
    //if(FlashMgr::getInstance().getFlashMode()!=LIB3A_FLASH_MODE_FORCE_TORCH)
    //     FlashMgr::getInstance().setAFLampOnOff(0);
    FlashMgr::getInstance().videoRecordingEnd(m_pHal3A->getSensorDev());
    //FlickerHalBase::getInstance().recordingEnd(m_pHal3A->getSensorDev());
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_pHal3A->getSensorDev(), 1);
    return  S_3A_OK;
}

