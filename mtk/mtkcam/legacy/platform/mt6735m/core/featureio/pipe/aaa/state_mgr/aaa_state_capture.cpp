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
#define LOG_TAG "aaa_state_capture"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <isp_tuning.h>
#include <awb_mgr_if.h>
#include <buf_mgr.h>
#include "aaa_state.h"
#include <mtkcam/common.h>
using namespace NSCam;
#include <camera_custom_AEPlinetable.h>
#include <ae_param.h>
#include <ae_mgr.h>
#include <flash_mgr.h>
//#include <mtkcam/hal/sensor_hal.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <af_mgr_if.h>
#include <isp_tuning.h>
#include <dbg_isp_param.h>
#include <flash_param.h>
#include <isp_tuning_mgr.h>
#include <lsc_mgr2.h>
#include <aaa_sensor_mgr.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.


using namespace NS3A;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCapture::
StateCapture()
    : IState("StateCapture")
{
    MY_LOG("IState(StateCapture)  line=%d", __LINE__);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CaptureStart>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CaptureStart>)  line=%d", __LINE__);

/*
    FlashMgr::getInstance().endPrecapture();
    if(FlashMgr::getInstance().isBurstShotMode()==1)
    {
        FlashMgr::getInstance().changeBurstEngLevel();
    }
    */


    // AE: update capture parameter
    IAeMgr::getInstance().doCapAE(m_pHal3A->getSensorDev());

    IAwbMgr::getInstance().cameraCaptureInit(m_pHal3A->getSensorDev());

    /*if ((IspTuningMgr::getInstance().getOperMode() == EOperMode_Normal) ||
        (IspTuningMgr::getInstance().getSensorMode() == ESensorMode_Capture))*/ {

        // AAO DMA / state enable again
        MRESULT err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AAO>());
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

    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CaptureEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>)  line=%d", __LINE__);
    BufInfo_T rBufInfo;
    MRESULT err;
    MINT32 iSensorDev = m_pHal3A->getSensorDev();
    //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "FlhChkFire_E");
    //
    IspTuningMgr::getInstance().setFlashDynamicInfo(iSensorDev, FlashMgr::getInstance().isFlashOnCapture(iSensorDev)); //prepare status for CaptureEnd PCA/CCM

    /*if ((IspTuningMgr::getInstance().getOperMode() == EOperMode_Normal) ||
        (IspTuningMgr::getInstance().getSensorMode() == ESensorMode_Capture))*/ {

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrdQAA");
        // Dequeue AAO DMA buffer
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AWBDoCapAWB");
        // One-shot AWB
        MINT32 i4SceneLV = IAeMgr::getInstance().getLVvalue(m_pHal3A->getSensorDev(), MFALSE);

        IAwbMgr::getInstance().doCapAWB(iSensorDev, i4SceneLV, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

        MY_LOG("IAwbMgr::getInstance().doCapAWB() END");
        FlashMgr::getInstance().capCheckAndFireFlash_End(m_pHal3A->getSensorDev());

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AEDoCapFlare");
       //Capture Flare compensate
       //pass WB gain info
       AWB_OUTPUT_T _a_rAWBOutput;
       IAwbMgr::getInstance().getAWBOutput(iSensorDev, _a_rAWBOutput);
       IAeMgr::getInstance().doCapFlare(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),FlashMgr::getInstance().isFlashOnCapture(iSensorDev) );

       MY_LOG("AeMgr::getInstance().doCapFlare() END");

        #if 1
        // F858
        LscMgr2::TSF_AWB_INFO rAwbInfo;
        rAwbInfo.m_i4LV     = i4SceneLV; //AeMgr::getInstance().getLVvalue();
        rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(iSensorDev);
        rAwbInfo.m_RGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        rAwbInfo.m_GGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        rAwbInfo.m_BGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        rAwbInfo.m_FLUO_IDX = _a_rAWBOutput.rAWBInfo.i4FluorescentIndex;
        rAwbInfo.m_DAY_FLUO_IDX = _a_rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
        LscMgr2::getInstance(static_cast<ESensorDev_T>(iSensorDev))->updateTsf(
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
        //MTK_SWIP_PROJECT_END

        MY_LOG("LscMgr::getInstance()->updateTSFinput() END");
        #endif

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgreQAA");
        // Enqueue AAO DMA buffer
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrUpAAAddr");
        // Update AAO DMA address
        BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

        //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AFCalBestShot");
        // --- best shot select ---
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);
        IAfMgr::getInstance().calBestShotValue(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);
        // --- best shot select ---

        MY_LOG("AfMgr::getInstance().calBestShotValue() END");

    }
    //else
    //{
    //    FlashMgr::getInstance().capCheckAndFireFlash_End();
    //}



    //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrAAUninit");
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

    //CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrAFUninit");
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

    IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(), MFALSE);
    IspTuningMgr::getInstance().validate(m_pHal3A->getSensorDev(), MTRUE);


    MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>) END");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MY_LOG("sendIntent(intent2type<eIntent_VsyncUpdate>)  line=%d", __LINE__);


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG("sendIntent(intent2type<eIntent_AFUpdate>)  line=%d", __LINE__);


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CameraPreviewEnd>)  line=%d", __LINE__);

    // Flicker uninit
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MY_LOG("sendIntent(intent2type<eIntent_CameraPreviewStart>)  line=%d", __LINE__);
    MRESULT err;

    // Get parameters
    Param_T rParam;
    m_pHal3A->getParams(rParam);

    // AE init
    err = IAeMgr::getInstance().cameraPreviewReinit(m_pHal3A->getSensorDev());
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().cameraPreviewReinit() fail\n");
        return err;
    }
    sm_bHasAEEverBeenStable = MFALSE;

    // AF init

    // AWB init
    err = IAwbMgr::getInstance().cameraPreviewReinit(m_pHal3A->getSensorDev());
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().cameraPreviewReinit() fail\n");
        return err;
    }

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

    IAfMgr::getInstance().setAF_IN_HSIZE(m_pHal3A->getSensorDev());
    //TODO: IAfMgr::getInstance().setFlkWinConfig(m_pHal3A->getSensorDev());

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

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_SET_GMA_SCENARIO, IspTuningMgr::E_GMA_SCENARIO_PREVIEW, 0);
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_NOTIFY_START, 0, 0);
    // Reset frame count to -2
    resetFrameCount();

    IspTuningMgr::getInstance().validatePerFrame(m_pHal3A->getSensorDev(), MTRUE);


    FlashMgr::getInstance().cameraPreviewStart(m_pHal3A->getSensorDev());
    //FlickerHalBase::getInstance().cameraPreviewStart(m_pHal3A->getSensorDev());
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_pHal3A->getSensorDev(), 1);
    FlickerHalBase::getInstance().open(m_pHal3A->getSensorDev(), 0, 0);
    //LscMgr::getInstance()->notifyPreflash(MFALSE);

    // State transition: eState_Capture --> eState_CameraPreview
    transitState(eState_Capture, eState_CameraPreview);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("sendIntent(intent2type<eIntent_AFEnd>)  line=%d", __LINE__);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("sendIntent(intent2type<eIntent_Uninit>)  line=%d", __LINE__);
    FlashMgr::getInstance().turnOffFlashDevice(m_pHal3A->getSensorDev());

    // AAO DMA buffer uninit
    BufMgr::getInstance().uninit();

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    IAwbMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);
    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // State transition: eState_Capture --> eState_Uninit
    transitState(eState_Capture, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart: for CTS only
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MRESULT err;
    FlashMgr::getInstance().turnOffFlashDevice(m_pHal3A->getSensorDev());

    MY_LOG("[StateCapture::sendIntent]<eIntent_CamcorderPreviewStart>");

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    IAwbMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Get parameters
    Param_T rParam;
    m_pHal3A->getParams(rParam);
    MINT32 i4SensorDev = m_pHal3A->getSensorDev();

    // AE init
    err = IAeMgr::getInstance().camcorderPreviewInit(m_pHal3A->getSensorDev(), i4SensorDev, rParam);
    if (FAILED(err)) {
        MY_ERR("AebMgr::getInstance().camcorderPreviewInit() fail\n");
        return err;
    }
    sm_bHasAEEverBeenStable = MFALSE;

    // AF init
    err = IAfMgr::getInstance().init(m_pHal3A->getSensorDev(),1);
    if (FAILED(err)) {
        MY_ERR("AfMgr::getInstance().init() fail\n");
        return err;
    }

    IAfMgr::getInstance().setAF_IN_HSIZE(m_pHal3A->getSensorDev());
    //TODO:IAfMgr::getInstance().setFlkWinConfig(m_pHal3A->getSensorDev());

    // AWB init
    err = IAwbMgr::getInstance().camcorderPreviewInit(m_pHal3A->getSensorDev(), i4SensorDev, rParam);
    if (FAILED(err)) {
        MY_ERR("IAwbMgr::getInstance().camcorderPreviewInit() fail\n");
        return err;
    }

    // Flash init
    err = FlashMgr::getInstance().init(m_pHal3A->getSensorDev(), i4SensorDev);
    if (FAILED(err)) {
        MY_ERR("FlashMgr::getInstance().init(i4SensorDev) fail\n");
        return err;
    }

    // Flicker init
    AAASensorMgr::getInstance().setFlickerFrameRateActive(m_pHal3A->getSensorDev(), 1);
    FlickerHalBase::getInstance().open(m_pHal3A->getSensorDev(), 0, 0);

    // AAO DMA / state enable
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

    // AFO DMA / state enable
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

    // Reset frame count to -2
    resetFrameCount();

    FlashMgr::getInstance().videoPreviewStart(m_pHal3A->getSensorDev());

    // State transition: eState_Capture --> eState_CamcorderPreview
    transitState(eState_Capture, eState_CamcorderPreview);

    return  S_3A_OK;
}



