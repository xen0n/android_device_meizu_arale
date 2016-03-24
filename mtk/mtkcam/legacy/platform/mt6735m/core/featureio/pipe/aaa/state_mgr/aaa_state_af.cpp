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
#define LOG_TAG "aaa_state_af"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <aaa_hal.h>
#include "aaa_state.h"
#include <buf_mgr.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <isp_tuning.h>
#include <awb_mgr.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <camera_custom_AEPlinetable.h>
#include <ae_param.h>
#include <ae_mgr.h>
//#include <mtkcam/hal/sensor_hal.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv_common/isp_reg.h>
#include <af_mgr_if.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include <flash_mgr.h>
#include <flash_param.h>
#include "flash_tuning_custom.h"
#include <isp_tuning_mgr.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include "aaa_state_flow_custom.h"
#include <kd_camera_feature.h>

using namespace NS3A;

extern EState_T g_ePrevState;

static sem_t           g_sem;
EState_T g_nextState;

int g_isAFLampOnInAfState;
int g_isBackRestore;
int g_is1stFrame;
extern int g_isPostPrecapAF;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateAF::
StateAF()
    : IState("StateAF")
{
     sem_init(&g_sem, 0, 1);
     g_nextState = eState_Invalid;
     g_isAFLampOnInAfState = 0;
     g_is1stFrame=1;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_AFStart>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_AFEnd>");

    // State transition: eState_AF --> g_ePrevState
    if(g_nextState!=eState_Invalid)
    {
        transitState(eState_AF, g_nextState);
        FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
        g_nextState=eState_Invalid;
        g_is1stFrame=1;
    }
    else
    {
        transitState(eState_AF, g_ePrevState);
        FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
        g_is1stFrame=1;
        if(g_isBackRestore==1)
        {
            g_isBackRestore=0;
            FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
            IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MFALSE);

        }
        //if((FlashMgr::getInstance().isAFLampOn()==1) && (FlashMgr::getInstance().getFlashMode() != FLASHLIGHT_TORCH) )
            //FlashMgr::getInstance().setAFLampOnOff(0);
    }

    g_nextState = eState_Invalid;

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_Uninit>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_Uninit>");

    // AAO DMA buffer uninit
    BufMgr::getInstance().uninit();

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    AwbMgr::getInstance().uninit();

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev()); //MINT32 i4SensorDev,MINT32 isInitMCU=1

    // Flash uninit
    FlashMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // Flicker uninit
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_pHal3A->getSensorDev(), IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);

    // State transition: eState_AF --> eState_Uninit
    transitState(eState_AF, eState_Uninit);
    g_is1stFrame=1;

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_CameraPreviewStart>");
     exitPreview();
     FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
     IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MFALSE);
     g_nextState = eState_Invalid;
    return  S_3A_OK;
}

MRESULT
StateAF::
sendIntent(intent2type<eIntent_RecordingEnd>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_RecordingEnd>");
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_CamcorderPreviewStart>");
    exitPreview();

    g_nextState = eState_Invalid;
    return  S_3A_OK;
}

MRESULT StateAF::exitPreview()
{
    ::sem_wait(&g_sem);
    MY_LOG("StateAF::exitPreview line=%d",__LINE__);
    MRESULT err;

    // AE uninit
    IAeMgr::getInstance().uninit(m_pHal3A->getSensorDev());

    // AWB uninit
    AwbMgr::getInstance().uninit();

    // AF uninit
    IAfMgr::getInstance().uninit(m_pHal3A->getSensorDev()); //MINT32 i4SensorDev,MINT32 isInitMCU=1


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

    // State transition: eState_AF --> eState_Init
    transitState(eState_AF, eState_Init);
    FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
    g_is1stFrame=1;
    ::sem_post(&g_sem);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_CameraPreviewEnd> line=%d",__LINE__);
    exitPreview();
    FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
    FlickerHalBase::getInstance().close(m_pHal3A->getSensorDev());
    IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MFALSE);
    g_nextState = eState_Invalid;
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_CamcorderPreviewEnd>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_CamcorderPreviewEnd> line=%d", __LINE__);
    exitPreview();
    FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
    IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MFALSE);
    g_nextState = eState_Invalid;
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    ::sem_wait(&g_sem);
    MRESULT err = S_3A_OK;

    MY_LOG("[StateAF::sendIntent]<eIntent_VsyncUpdate> line=%d", __LINE__);

    switch (getAFState())
    {
    case eAFState_None:
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_None>());
        break;
    case eAFState_PreAF:
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_PreAF>());
        break;
    case eAFState_AF:
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_AF>());
        break;
    case eAFState_PostAF:
        err = sendAFIntent(intent2type<eIntent_VsyncUpdate>(), state2type<eAFState_PostAF>());
        break;
    default:
        break;
    }
    ::sem_post(&g_sem);

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    ::sem_wait(&g_sem);
    MRESULT err = S_3A_OK;

    MY_LOG("[StateAF::sendIntent]<eIntent_AFUpdate>");

    switch (getAFState())
    {
    case eAFState_None:
        err = sendAFIntent(intent2type<eIntent_AFUpdate>(), state2type<eAFState_None>());
        break;
    case eAFState_PreAF:
        err = sendAFIntent(intent2type<eIntent_AFUpdate>(), state2type<eAFState_PreAF>());
        break;
    case eAFState_AF:
        err = sendAFIntent(intent2type<eIntent_AFUpdate>(), state2type<eAFState_AF>());
        break;
    case eAFState_PostAF:
        err = sendAFIntent(intent2type<eIntent_AFUpdate>(), state2type<eAFState_PostAF>());
        break;
    default:
        break;
    }
    ::sem_post(&g_sem);
    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    MY_LOG("[StateAF::sendIntent]<eIntent_PrecaptureStart>");

    // Init

    // State transition: eState_AF --> eState_Precapture
    //transitState(eState_AF, eState_Precapture);
    g_nextState = eState_Precapture;
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PreAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PreAF>)
{
    //MY_LOG("[StateAF::sendAFIntent]<eIntent_VsyncUpdate>,<eAFState_PreAF>");
    ALOGD("[StateAF::sendAFIntent]<eIntent_VsyncUpdate>,<eAFState_PreAF>");
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;
    // Update frame count
    updateFrameCount();
    //static int is1stFrameToCheckAFLamp=1;
    static int skipFrames=0;
    static int isAFLampOn=0;

    if(IAeMgr::getInstance().IsDoAEInPreAF(m_pHal3A->getSensorDev()) == MTRUE) {   // do AE/AWB before AF start

        //FlashMgr::getInstance().capCheckAndFireFlash();
        Param_T param;
        m_pHal3A->getParams(param);
        if(g_is1stFrame==1)
        {
            skipFrames=0;
            g_is1stFrame=0;

            ALOGD("eAFState_PreAF-g_is1stFrame line=%d",__LINE__);
            if(FlashMgr::getInstance().isAFLampOn(m_pHal3A->getSensorDev())==1 || g_isPostPrecapAF==1)
            {
                ALOGD("eAFState_PreAF-isAFLampOn=0 (flash on|video)");
                g_isBackRestore=0;

        }
            else
        {
            isAFLampOn = cust_isNeedAFLamp(    FlashMgr::getInstance().getFlashMode(m_pHal3A->getSensorDev()),
                            FlashMgr::getInstance().getAfLampMode(m_pHal3A->getSensorDev()),
                            IAeMgr::getInstance().IsStrobeBVTrigger(m_pHal3A->getSensorDev()));
            ALOGD("eAFState_PreAF-cust_isNeedAFLamp ononff:%d flashM:%d AfLampM:%d triger:%d",
                            isAFLampOn,
                            FlashMgr::getInstance().getFlashMode(m_pHal3A->getSensorDev()),
                            FlashMgr::getInstance().getAfLampMode(m_pHal3A->getSensorDev()),
                            IAeMgr::getInstance().IsStrobeBVTrigger(m_pHal3A->getSensorDev()));
            if(isAFLampOn==1)
            {
                ALOGD("eAFState_PreAF-isAFLampOn=1");
                skipFrames=2;
                IAeMgr::getInstance().doBackAEInfo(m_pHal3A->getSensorDev());
                FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 1);
                IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MTRUE);
                //AwbMgr::getInstance().backupAWB();
                    g_isBackRestore=1;
                }
                else
                {
                    ALOGD("eAFState_PreAF-isAFLampOn=0");
                    g_isBackRestore=0;
                }
            }
        }

        // Dequeue AAO DMA buffer
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

        if(g_isBackRestore==1)
        {
        // one-shot AWB
            if (isAFLampOn==0)
                AwbMgr::getInstance().setStrobeMode(AWB_STROBE_MODE_OFF);
            else
                AwbMgr::getInstance().setStrobeMode(AWB_STROBE_MODE_ON);

            AwbMgr::getInstance().doAFAWB(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
        }

        if (skipFrames>0)
        {
            skipFrames--;
        }
        else
        {
            //skipFrames=0;

#if 0
        if (getFrameCount() < 0) {// AA statistics is not ready
            // Update AAO DMA base address for next frame
            err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));
            return err;
        }

        IspTuningMgr::GMA_AE_DYNAMIC_INFO dynamicInfo;
        dynamicInfo.bStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
        IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC, (MINTPTR)&dynamicInfo, 0);

#endif

            // AE
            CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
            MBOOL isNeedUpdateI2C;
            IAeMgr::getInstance().doAFAEmonitor(m_pHal3A->getSensorDev(),getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
                0, 1, 0, isNeedUpdateI2C);
            IAeMgr::getInstance().doAFAE(m_pHal3A->getSensorDev(),getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
                0, 1, 0);
            if(isNeedUpdateI2C)
            {
                MY_LOG_IF(1, "updateSensorbyI2C start\n");
                IAeMgr::getInstance().updateSensorbyI2C(m_pHal3A->getSensorDev());
                MY_LOG_IF(1, "updateSensorbyI2C end\n");
            }
            CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);    // Profiling Start.

            if(IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev()) == MTRUE)
            {
                MY_LOG("[StateAF::sendAFIntent]Pre AF transfer to AF state\n");

                IAfMgr::getInstance().triggerAF(m_pHal3A->getSensorDev()); //MINT32 i4SensorDev
                transitAFState(eAFState_AF);
                g_is1stFrame=1;
            }
        }

        // Enqueue AAO DMA buffer
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

        // Update AAO DMA base address for next frame
        err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));
    }else {
        // change to next state directly
        IAfMgr::getInstance().triggerAF(m_pHal3A->getSensorDev()); //MINT32 i4SensorDev
        transitAFState(eAFState_AF);
        g_is1stFrame=1; //set to 1. Then, in the first frame of AE (next AF),  check if af needs enable.
    }
    return  S_3A_OK;
}

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PreAF>)
{
    MY_LOG("[StateAF::sendAFIntent]<eIntent_AFUpdate>,<eAFState_PreAF>");

    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);

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

    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr)); //MINT32 i4SensorDev, MVOID *pAFStatBuf
    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagEnd);    // Profiling Start.

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);

    // FIXME: state transition
    // transitAFState(eAFState_AF);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_AF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>)
{
#define AfLampOnSkipFrame 2//6
    static int skipFrames=0;
    BufInfo_T rBufInfo;

    MY_LOG("[StateAF::sendAFIntent]<eIntent_VsyncUpdate>,<eAFState_AF>");
    if (IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev()))
    {

/*
        if(skipFrames>0)
        {
            FlashMgr::getInstance().setAFLampOnOff(0);
            AeMgr::getInstance().setExpTest(1);
            skipFrames--;
             return S_3A_OK;
        }
        else
        {
            // FIXME: state transition
            if (AeMgr::getInstance().IsDoAEInPreAF() == MTRUE)
            {
                transitAFState(eAFState_None);
                transitState(eState_AF, g_ePrevState);
            }
            else {
                transitAFState(eAFState_PostAF);
            }
        }
        */


        if(skipFrames>0)
        {
            if(skipFrames == AfLampOnSkipFrame)
            IAeMgr::getInstance().doRestoreAEInfo(m_pHal3A->getSensorDev(), MFALSE);

            if((FlashMgr::getInstance().getFlashMode(m_pHal3A->getSensorDev()) != FLASHLIGHT_TORCH) && (skipFrames == AfLampOnSkipFrame-1/*to advance by 1 frame*//*2*/))
            {
//#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
                if (isSupportSetMaxFrameRate())
            {
                ALOGD("af sync support");
                usleep(33000);
                }
//#else
                else
                ALOGD("af sync NOT support");
//#endif
                FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
                IAeMgr::getInstance().setStrobeMode(m_pHal3A->getSensorDev(),MFALSE);
              }


            int aeFrm;
            aeFrm = AfLampOnSkipFrame-skipFrames+1/*to advance by 1 frame*/;
            if(aeFrm>=0)
                IAeMgr::getInstance().setRestore(m_pHal3A->getSensorDev(),aeFrm);

            if(g_isBackRestore==1)
            {

                BufInfo_T rBufInfo;
                BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

            //    if(skipFrames == AfLampOnSkipFrame-2)
            //    AwbMgr::getInstance().restoreAWB();
            //if(skipFrames <= AfLampOnSkipFrame)
            {
            // one-shot AWB
            //    MINT32 i4SceneLV = AeMgr::getInstance().getLVvalue();
                    AwbMgr::getInstance().setStrobeMode(AWB_STROBE_MODE_OFF);
                    AwbMgr::getInstance().doAFAWB(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
            }

            // if(skipFrames==2)
            //     AeMgr::getInstance().setExp(10000);
            //


            //int y;
            //y= aaToMean(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
            //ALOGD("skipFrames=\t%d\taeyy=\t%d",skipFrames, y);
                BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);
            // Update AAO DMA base address for next frame
                BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));
            }

            skipFrames--;
            //AeMgr::getInstance().setExpTest();
            //AeMgr::getInstance().setCnt(0);

        }
        else
        {
            if (IAeMgr::getInstance().IsDoAEInPreAF(m_pHal3A->getSensorDev()) == MTRUE)
            {
                transitAFState(eAFState_None);
                IAfMgr::getInstance().SingleAF_CallbackNotify(m_pHal3A->getSensorDev());
                m_pHal3A->bAfNeedSingleNotify = MTRUE;

                if(g_nextState!=eState_Invalid)
                {
                    transitState(eState_AF, g_nextState);
                    FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
                    g_nextState=eState_Invalid;
                    g_is1stFrame=1;
                }
                else
                {
                    transitState(eState_AF, g_ePrevState);
                    FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
                    g_is1stFrame=1;
                }
            }
            else
            {
                transitAFState(eAFState_PostAF);
                IAfMgr::getInstance().SingleAF_CallbackNotify(m_pHal3A->getSensorDev());
                m_pHal3A->bAfNeedSingleNotify = MTRUE;
            }
        }
    }
    else {

        // Dequeue AAO DMA buffer
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);
        MBOOL isNeedUpdateI2C;
        IAeMgr::getInstance().setAFAELock(m_pHal3A->getSensorDev(), MTRUE);
        IAeMgr::getInstance().doAFAEmonitor(m_pHal3A->getSensorDev(),getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            0, 1, 0, isNeedUpdateI2C);
        IAeMgr::getInstance().setAFAELock(m_pHal3A->getSensorDev(), MFALSE);
        // Enqueue AAO DMA buffer
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);
        // Update AAO DMA base address for next frame
        BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

        if(g_isBackRestore==0)
        {
            g_isAFLampOnInAfState=0;
            skipFrames = 0;
        }
        else
        {
            skipFrames = AfLampOnSkipFrame;
            g_isAFLampOnInAfState=1;
        }
    }

    return  S_3A_OK;
}

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_AF>)
{
    MY_LOG("[StateAF::sendAFIntent]<eIntent_AFUpdate>,<eAFState_AF>");

    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    if (IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev()))
        return S_3A_OK;

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);

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

    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr)); //MINT32 i4SensorDev, MVOID *pAFStatBuf
    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagEnd);    // Profiling Start.

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);

#if 0
    if (IAfMgr::getInstance().isFocusFinish(1))   {
        FlashMgr::getInstance().setAFLampOnOff(m_pHal3A->getSensorDev(), 0);
        AeMgr::getInstance().setStrobeMode(MFALSE);
        if(AeMgr::getInstance().IsDoAEInPreAF() == MTRUE)   {

            MY_LOG("SingleAF Callback Notify %d", g_ePrevState);
            //AF callback notify
            IAfMgr::getInstance().SingleAF_CallbackNotify(1);
            transitAFState(eAFState_None);
            transitState(eState_AF, g_ePrevState);
            FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
        }
        else   {
            transitAFState(eAFState_PostAF);
        }
    }
#else
    MY_LOG("IAfMgr::getInstance().isFocusFinish() = %d", IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev()));

    if (IAfMgr::getInstance().isFocusFinish(m_pHal3A->getSensorDev())) {
        MY_LOG("SingleAF Callback Notify %d", g_ePrevState);
        //AF callback notify
        IAfMgr::getInstance().SingleAF_CallbackNotify(m_pHal3A->getSensorDev()); // move to <eIntent_VsyncUpdate>,<eAFState_AF> when AF finish.
    }
#endif

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_PostAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>)
{
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StateAF::sendAFIntent]<eIntent_VsyncUpdate>,<eAFState_PostAF>");

        if(IAeMgr::getInstance().IsDoAEInPreAF(m_pHal3A->getSensorDev()) == MFALSE)
        {    // do AE/AWB after AF done

        // Update frame count
        updateFrameCount();

        if (getFrameCount() < 0) {// AA statistics is not ready
            // Update AAO DMA base address for next frame
            err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));
            return err;
        }

        // Dequeue AAO DMA buffer
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

        // AE
        CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
        MBOOL isNeedUpdateI2C;
        IAeMgr::getInstance().doAFAEmonitor(m_pHal3A->getSensorDev(),getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            0, 1, 0, isNeedUpdateI2C);
        IAeMgr::getInstance().doAFAE(m_pHal3A->getSensorDev(),getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr),
            0, 1, 0);
        if(isNeedUpdateI2C)
        {
            MY_LOG_IF(1, "updateSensorbyI2C start\n");
            IAeMgr::getInstance().updateSensorbyI2C(m_pHal3A->getSensorDev());
            MY_LOG_IF(1, "updateSensorbyI2C end\n");
        }
        CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);    // Profiling Start.

        //if(AeMgr::getInstance().IsAEStable() == MTRUE) {
        //    MINT32 i4SceneLV = AeMgr::getInstance().getLVvalue();
        //   AwbMgr::getInstance().doPreCapAWB(i4SceneLV, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
        //}

        // Enqueue AAO DMA buffer
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

        // Update AAO DMA base address for next frame
        err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

        if(IAeMgr::getInstance().IsAEStable(m_pHal3A->getSensorDev()) == MTRUE){
            MY_LOG("[StateAF::sendAFIntent]Post AF: finish\n");
            //AF callback notify
            IAfMgr::getInstance().SingleAF_CallbackNotify(m_pHal3A->getSensorDev());
            m_pHal3A->bAfNeedSingleNotify = MTRUE;
            transitAFState(eAFState_None);
            if(g_nextState!=eState_Invalid)
            {
                transitState(eState_AF, g_nextState);
                FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
                g_nextState=eState_Invalid;
                g_is1stFrame=1;
            }
            else
            {
            transitState(eState_AF, g_ePrevState);
            FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
                g_is1stFrame=1;
            }
        }
    }else {
        // change to next state directly
        transitAFState(eAFState_None);
        if(g_nextState!=eState_Invalid)
        {
            transitState(eState_AF, g_nextState);
            FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
            g_nextState=eState_Invalid;
            g_is1stFrame=1;
        }
        else
        {
        transitState(eState_AF, g_ePrevState);
        FlashMgr::getInstance().notifyAfExit(m_pHal3A->getSensorDev());
            g_is1stFrame=1;
        }
    }

    return  S_3A_OK;
}

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PostAF>)
{
    MY_LOG("[StateAF::sendAFIntent]<eIntent_AFUpdate>,<eAFState_PostAF>");

    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);

    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagStart);    // Profiling Start.
    IAfMgr::getInstance().doAF(m_pHal3A->getSensorDev(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr)); //MINT32 i4SensorDev, MVOID *pAFStatBuf
    CPTLog(Event_Pipe_3A_Single_AF, CPTFlagEnd);    // Profiling Start.

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eAFState_None
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_None>)
{
    MY_LOG("[StateAF::sendAFIntent]<eIntent_VsyncUpdate>,<eAFState_None>");
    return  S_3A_OK;
}

MRESULT
StateAF::
sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_None>)
{
    MY_LOG("[StateAF::sendAFIntent]<eIntent_AFUpdate>,<eAFState_None>");
    return  S_3A_OK;
}

