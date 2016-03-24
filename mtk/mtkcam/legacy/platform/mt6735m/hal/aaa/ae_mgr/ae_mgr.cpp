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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <mtkcam/common.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <camera_custom_AEPlinetable.h>
//#include <CamDefs.h>
#include <mtkcam/common/faces.h>
#include <isp_tuning.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
//#include <sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include <kd_camera_feature.h>
#include <mtkcam/hal/IHalSensor.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
//#include <aaa_scheduler.h>
#include <aaa_common_custom.h>
#include "aaa_sensor_buf_mgr.h"
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <aaa_scheduling_custom.h>

#define ISP_GAIN_DELAY_OFFSET 2

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2

using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

AeMgr* AeMgr::s_pAeMgr = MNULL;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AeMgrDev : public AeMgr
{
public:
    static    AeMgr&    getInstance() {
        static AeMgrDev<eSensorDev> singleton;
        AeMgr::s_pAeMgr = &singleton;
        return singleton;
    }

    AeMgrDev() : AeMgr(eSensorDev) {}
    virtual ~AeMgrDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr &AeMgr::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev) {
        case ESensorDev_Main: //  Main Sensor
            return  AeMgrDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond: //  Main Second Sensor
            return  AeMgrDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_Sub: //  Sub Sensor
            return  AeMgrDev<ESensorDev_Sub>::getInstance();
        default:
            MY_LOG("i4SensorDev = %d", i4SensorDev);
            if (AeMgr::s_pAeMgr) {
                return  *AeMgr::s_pAeMgr;
            } else {
                return  AeMgrDev<ESensorDev_Main>::getInstance();
            }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
AeMgr(ESensorDev_T eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_1)
    , m_eSensorMode(ESensorMode_Preview)
    , m_i4SensorIdx(0)
    , m_BVvalue(0)
    , m_AOECompBVvalue(0)
    , m_BVvalueWOStrobe(0)
    , m_i4EVvalue(0)
    , m_i4WaitVDNum(0)
    , m_i4RotateDegree(0)
    , m_i4TimeOutCnt(0)
    , m_i4ShutterDelayFrames(2)
    , m_i4SensorGainDelayFrames(2)
    , m_i4SensorGainDelayFramesWOShutter(1)
    , m_i4IspGainDelayFrames(0)
    , m_i4SensorCaptureDelayFrame(0)
    , m_i4TotalCaptureDelayFrame(3)
    , m_i4AEidxCurrent(0)
    , m_i4AEidxNext(0)
    , m_i2AEFaceDiffIndex(0)
    , m_u4PreExposureTime(0)
    , m_u4PreSensorGain(0)
    , m_u4PreIspGain(0)
    , m_u4SmoothIspGain(0)
    , m_u4AECondition(0)
    , m_u4DynamicFrameCnt(0)
    , m_u4AFSGG1Gain(16)
    , m_bOneShotAEBeforeLock(MFALSE)
    , m_bAESceneChanged(MFALSE)
    , m_bAELock(MFALSE)
    , m_bAPAELock(MFALSE)
    , m_bAFAELock(MFALSE)
    , m_bVideoDynamic(MFALSE)
    , m_bRealISOSpeed(MFALSE)
    , m_bAElimitor(MFALSE)
    , m_bAEStable(MFALSE)
    , m_bAEMonitorStable(MFALSE)
    , m_bAEReadyCapture(MFALSE)
    , m_bLockExposureSetting(MFALSE)
    , m_bStrobeOn(MFALSE)
    , m_bAEMgrDebugEnable(MFALSE)
    , m_bRestoreAE(MFALSE)
    , m_bAECaptureUpdate(MFALSE)
    , m_bOtherIPRestoreAE(MFALSE)
    , m_eAEScene(LIB3A_AE_SCENE_AUTO)
    , m_eAEMode(LIB3A_AE_MODE_OFF)
    , m_ePreAEState(MTK_CONTROL_AE_STATE_INACTIVE)
    , m_eAEState(MTK_CONTROL_AE_STATE_INACTIVE)
    , m_fEVCompStep(1)
    , m_i4EVIndex(0)
    , m_eAEMeterMode(LIB3A_AE_METERING_MODE_CENTER_WEIGHT)
    , m_u4AEISOSpeed(0)
    , m_eAEFlickerMode(LIB3A_AE_FLICKER_MODE_50HZ)
    , m_i4AEMaxFps(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_i4AEMinFps(LIB3A_AE_FRAMERATE_MODE_05FPS)
    , m_eAEAutoFlickerMode(LIB3A_AE_FLICKER_AUTO_MODE_50HZ)
    , m_eCamMode(eAppMode_PhotoMode)
    , m_eAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_eShotMode(eShotMode_NormalShot)
    , m_eAEEVcomp(LIB3A_AE_EV_COMP_00)
    , m_AEState(AE_INIT_STATE)
    , m_bIsAutoFlare(MTRUE)
    , m_bFrameUpdate(MFALSE)
    , m_bAdbAEEnable(MFALSE)
    , m_bAdbAELock(MFALSE)
    , m_bAdbAELog(MFALSE)
    , m_bAdbAEPreviewUpdate(MFALSE)
    , m_bAdbAEDisableSmooth(MFALSE)
    , m_i4AdbAEISPDisable(0)
    , m_u4AdbAEShutterTime(0)
    , m_u4AdbAESensorGain(0)
    , m_u4AdbAEISPGain(0)
    , m_bAdbAEPerFrameCtrl(0)
    , m_u4IndexTestCnt(0)
    , m_i4ObjectTrackNum(0)
    , m_pIsAEActive(MNULL)
    , m_i4AECycleNum(0)
    , m_i4GammaIdx(0)
    , m_i4LESE_Ratio(1)
    , m_u4SWHDR_SE(0)
    , m_u4MaxShutter(100000)
    , m_u4MaxISO(800)
    , m_u4StableCnt(0)
    , m_pAEPlineTable(NULL)
    , m_p3ANVRAM(NULL)
    , m_eAETargetMode(AE_MODE_NORMAL)
    , m_u4PrevExposureTime(0)
    , m_u4PrevSensorGain(0)
    , m_u4PrevIspGain(0)
    , m_ePrevAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_3ALogEnable(0)
    , m_bAEOneShotControl(MFALSE)
    , bUpdateSensorAWBGain(MFALSE)
    , bApplyAEPlineTableByCCT(MFALSE)
    , m_bSetShutterValue(MFALSE)
    , m_bSetGainValue(MFALSE)
    , m_bSetFrameRateValue(MFALSE)
    , m_u4UpdateShutterValue(0)
    , m_u4UpdateGainValue(0)
    , m_u4UpdateFrameRate_x10(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_bTouchAEAreaChage(MFALSE)
    , m_bMultiCap(MFALSE)
    , m_bCShot(MFALSE)
    , m_bHDRshot(MFALSE)
    , m_u4VsyncCnt(0)
    , m_bLockBlackLevel(MFALSE)
    , m_bFaceAEAreaChage(MFALSE)
    , m_u4AEScheduleCnt(0)
    , m_u4StableYValue(0)
    , m_pSensorGainThread(NULL)
{
    memset(&m_AeMgrCCTConfig, 0, sizeof(AE_CCT_CFG_T));
    memset(&m_eZoomWinInfo, 0, sizeof(EZOOM_WINDOW_T));
    memset(&m_eAEMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_eAEFDArea, 0, sizeof(AEMeteringArea_T));
    memset(&m_CurrentPreviewTable, 0, sizeof(strAETable));
    memset(&m_CurrentCaptureTable, 0, sizeof(strAETable));
    memset(&m_CurrentStrobetureTable, 0, sizeof(strAETable));
    memset(&mPreviewMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&mCaptureMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));
    memset(&m_strIVHDROutputSetting, 0, sizeof(IVHDRExpSettingOutputParam_T));
    memset(&m_backupMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_rAEInitInput, 0, sizeof(AE_INITIAL_INPUT_T));
    memset(&m_rAEOutput, 0, sizeof(AE_OUTPUT_T));
    memset(&m_rAEStatCfg, 0, sizeof(AE_STAT_PARAM_T));
    memset(&m_rSensorResolution[0], 0, 2*sizeof(SENSOR_RES_INFO_T));
    memset(&m_rAEPLineLimitation, 0, sizeof(AE_PLINE_LIMITATION_T));
    memset(&m_rSensorVCInfo, 0, sizeof(SENSOR_VC_INFO_T));
    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));
    memset(&m_rSensorCurrentInfo, 0, sizeof(AE_SENSOR_PARAM_T));

    m_backupMeterArea.u4Count = 1;
    m_isAeMeterAreaEn=1;
    m_rAEPLineLimitation.u4IncreaseISO_x100 = 100;
    m_rAEPLineLimitation.u4IncreaseShutter_x100 = 100;
    MY_LOG("[AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
~AeMgr()
{
    MY_LOG("[~AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::PreviewAEInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    MRESULT err;
    MINT32 i4SutterDelay, i4SensorGainDelay, i4IspGainDelay;

    // set sensor type
    m_i4SensorIdx = i4SensorIdx;

    // set sensor initial
    err = AAASensorMgr::getInstance().init(m_i4SensorIdx);
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().init fail i4SensorDev:%d\n", m_i4SensorIdx);
        return err;
    }

    AAASensorMgr::getInstance().getSensorSyncinfo(m_eSensorDev, &i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, &m_i4SensorCaptureDelayFrame);
    AAASensorMgr::getInstance().getSensorMaxFrameRate(m_eSensorDev, m_rSensorMaxFrmRate);

    if (m_pSensorGainThread == NULL && i4SutterDelay != i4SensorGainDelay)
    {
        m_pSensorGainThread = ThreadSensorGain::createInstance(m_eSensorDev);
        i4SensorGainDelay = i4SutterDelay;
    }

    if(i4IspGainDelay >= m_i4SensorCaptureDelayFrame) {
        m_i4TotalCaptureDelayFrame = i4IspGainDelay + 1;
    } else {
        m_i4TotalCaptureDelayFrame = m_i4SensorCaptureDelayFrame;
    }

    MY_LOG("Long capture delay frame :%d %d %d\n", m_i4TotalCaptureDelayFrame, m_i4SensorCaptureDelayFrame, i4IspGainDelay);

    if((i4SutterDelay <= 5) && (i4SensorGainDelay <= 5) && (i4IspGainDelay <= 5)) {
        m_i4ShutterDelayFrames = i4SutterDelay;
        m_i4SensorGainDelayFrames = i4SensorGainDelay;
        if(i4IspGainDelay >= 2) {
            m_i4IspGainDelayFrames = i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 2 delay frame
        } else {
            m_i4IspGainDelayFrames = 0;
        }

        MY_LOG("Delay info is shutter :%d sensor gain:%d isp gain:%d Sensor Info:%d %d %d\n", m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    } else {
        MY_LOG("Delay info is incorrectly :%d %d %d\n", i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
        m_i4ShutterDelayFrames = 0;
        m_i4SensorGainDelayFrames = 0;
        m_i4IspGainDelayFrames = 0; // for CQ0 2 delay frame
    }

    // Get sensor resolution
    err = getSensorResolution();
    if (FAILED(err)) {
        MY_ERR("getSensorResolution() fail\n");
        return err;
    }

    // Get NVRAM data
    err = getNvramData(m_eSensorDev);
    if (FAILED(err)) {
        MY_ERR("getNvramData() fail\n");
        return err;
    }

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        err = AAASensorMgr::getInstance().getSensorVCinfo(m_eSensorDev, &m_rSensorVCInfo);
        if (FAILED(err)) {
            MY_ERR("getSensorVCinfo() fail\n");
            return err;
        }
    }

    // Init AE
    err = AEInit(rParam);
    if (FAILED(err)) {
        MY_ERR("AEInit() fail\n");
        return err;
    }

    m_i4SensorGainDelayFramesWOShutter = m_rAEInitInput.rAEPARAM.strAEParasetting.uAESensorGainDelayCycleWOShutter;

    MY_LOG("[PreviewAEInit] m_eSensorDev:%d m_eSensorTG:%d m_i4SensorIdx:%d\n", m_eSensorDev, m_eSensorTG, m_i4SensorIdx);
    // AE statistics and histogram config
//    err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).config(m_i4SensorIdx, m_rAEStatCfg);
     err = ISP_MGR_AE_STAT_HIST_CONFIG_T::getInstance((ESensorDev_T)m_eSensorDev).config(m_rAEStatCfg, m_i4SensorIdx);
    if (FAILED(err)) {
        MY_ERR("AE state hist config() fail\n");
        return err;
    }

    m_bAEStable = MFALSE;
    m_bAEMonitorStable = m_bAEStable;
    mPreviewMode = m_rAEOutput.rPreviewMode;
    if((m_eAETargetMode == AE_MODE_AOE_TARGET) || ((m_eAETargetMode == AE_MODE_MVHDR_TARGET))) {
        m_strIVHDROutputSetting.u4LEExpTimeInUS = m_strIVHDROutputSetting.u4SEExpTimeInUS;   // workaround for iVHDR initial
        m_strIVHDROutputSetting.u4LE_SERatio_x100 = 100;                                 // workaround for iVHDR initial
        UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
    } else {
        UpdateSensorISPParams(AE_INIT_STATE);
    }
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    MY_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d CamMode:%d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode, m_eAECamMode);

    m_bRealISOSpeed = 0;
    m_i4WaitVDNum = 0;
    m_bSetShutterValue = MFALSE;
    m_bSetGainValue = MFALSE;
    m_bSetFrameRateValue = MFALSE;
    m_u4AEScheduleCnt = 0;
    m_i4EVIndex = 0;
    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;

    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));

    switch(m_eCamMode) {
        case eAppMode_EngMode:        //  Engineer Mode
            m_bRealISOSpeed = 1;
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        case eAppMode_ZsdMode:        //  ZSD Mode
            m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
            break;
        case eAppMode_VideoMode:     //  Video Mode
            m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
            break;
        case eAppMode_StereoMode:        //  S3D Mode
            // TBD
            // m_eAECamMode = LIB3A_AECAM_MODE_S3D;
            // break;
        case eAppMode_PhotoMode:     //  Photo Mode
        case eAppMode_DefaultMode:   //  Default Mode
        case eAppMode_AtvMode:         //  ATV Mode
        case eAppMode_VtMode:           //  VT Mode
        case eAppMode_FactoryMode:
        default:
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        }


    PreviewAEInit(i4SensorIdx, rParam);
    m_AEState = AE_AUTO_FRAMERATE_STATE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.ae_mgr.enable", value, "0");
    m_bAdbAEEnable = atoi(value);
    if(m_bAdbAEEnable == 1) {
        MY_LOG("[%s()] Enable AE adb control\n", __FUNCTION__);
    }

    property_get("debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);

    m_u4IndexTestCnt = 0;
    property_get("debug.ae.perframeCtrl.enable", value, "0");
    m_bAdbAEPerFrameCtrl = atoi(value);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::camcorderPreviewInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    MRESULT err;

    MY_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d CamMode:%d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode, m_eAECamMode);

    // the same with preview initial
    m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
    PreviewAEInit(i4SensorIdx, rParam);

    m_AEState = AE_MANUAL_FRAMERATE_STATE;

    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// for come back to preview/video condition use
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewReinit()
{
    MRESULT err;
    strAEInput rAEInput;
    strAEOutput rAEOutput;

    m_i4WaitVDNum = 0;

    if(m_eCamMode == eAppMode_EngMode) {  //  Engineer Mode
        m_bRealISOSpeed = 1;
        m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
    } else if(m_eCamMode == eAppMode_ZsdMode) {  //  ZSD Mode
        m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
        m_bRealISOSpeed = 0;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
        m_pIAeAlgo->setAECamMode(m_eAECamMode);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    MY_LOG("[cameraPreviewReinit] Original m_eAECamMode:%d Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_eAECamMode, m_bRealISOSpeed, m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
                m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.u2FrameRate,
                m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u4RealISO);

    if(m_bEnableAE) {
        if(m_eAECamMode != LIB3A_AECAM_MODE_VIDEO) {
            rAEInput.eAeState = AE_STATE_INIT;
            rAEInput.pAESatisticBuffer = NULL;
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            } else {
                MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
            }
            MY_LOG("[cameraPreviewReinit] init m_eAECamMode:%d Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_eAECamMode, m_bRealISOSpeed, m_rAEOutput.rPreviewMode.u4ExposureMode, rAEOutput.EvSetting.u4Eposuretime,
                rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u2FrameRate,
                rAEOutput.i2FlareGain, rAEOutput.i2FlareOffset, rAEOutput.u4ISO);
        }

        rAEInput.eAeState = AE_STATE_AELOCK;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }

        MY_LOG("[cameraPreviewReinit] Lock Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_bRealISOSpeed, m_rAEOutput.rPreviewMode.u4ExposureMode, rAEOutput.EvSetting.u4Eposuretime,
                rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u2FrameRate,
                rAEOutput.i2FlareGain, rAEOutput.i2FlareOffset, rAEOutput.u4ISO);

        copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);

        MY_LOG("[cameraPreviewReinit] Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                    m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
                    m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.u2FrameRate,
                    m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u4RealISO);
    } else {
        MY_LOG("[cameraPreviewReinit] AE disable\n");
    }

    UpdateSensorISPParams(AE_REINIT_STATE);

    m_AEState = AE_AUTO_FRAMERATE_STATE;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::uninit()
{
    MRESULT err;

    if (m_pSensorGainThread)
    {
        m_pSensorGainThread->destroyInstance();
        m_pSensorGainThread = NULL;
    }

    err = AAASensorMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().uninit fail\n");
        return err;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->destroyInstance();
        m_pIAeAlgo = NULL;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void AeMgr::setAeMeterAreaEn(int en)
{
    ALOGD("setAeMeterAreaEn en=%d, m_isAeMeterAreaEn=%d",en, m_isAeMeterAreaEn);
    m_isAeMeterAreaEn=en;
    setAEMeteringArea(&m_backupMeterArea);

}
MRESULT AeMgr::setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea)
{

    if(CUST_ENABLE_TOUCH_AE() == MFALSE) {
        MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_TOUCH_AE());
        return S_AE_OK;
    }

        MY_LOG_IF(m_3ALogEnable,"setAEMeteringArea m_isAeMeterAreaEn=%d",m_isAeMeterAreaEn);


        CameraMeteringArea_T meterArea;
        memcpy(&m_backupMeterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));
        memcpy(&meterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));




        if(m_isAeMeterAreaEn==0)
        {
            memset(&meterArea, 0, sizeof(CameraMeteringArea_T));
            meterArea.u4Count = 1;
        }


    MUINT32 i;
    MBOOL bAreaChage = MFALSE;
    MUINT32 u4AreaCnt;
    AEMeteringArea_T *sAEMeteringArea = (AEMeteringArea_T* )&meterArea;

    if (sAEMeteringArea->u4Count <= 0) {
//        MY_LOG("No AE Metering area cnt: %d\n", sAEMeteringArea->u4Count);
        return S_AE_OK;
    } else if (sAEMeteringArea->u4Count > MAX_METERING_AREAS) {
        MY_ERR("The AE Metering area cnt error: %d\n", sAEMeteringArea->u4Count);
        return E_AE_UNSUPPORT_MODE;
    }

    u4AreaCnt = sAEMeteringArea->u4Count;

    for(i=0; i<u4AreaCnt; i++) {
        if (sAEMeteringArea->rAreas[i].i4Left   < -1000)  {sAEMeteringArea->rAreas[i].i4Left   = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Right  < -1000)  {sAEMeteringArea->rAreas[i].i4Right  = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Top    < -1000)  {sAEMeteringArea->rAreas[i].i4Top    = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Bottom < -1000)  {sAEMeteringArea->rAreas[i].i4Bottom = -1000;}

        if (sAEMeteringArea->rAreas[i].i4Left   > 1000)  {sAEMeteringArea->rAreas[i].i4Left   = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Right  > 1000)  {sAEMeteringArea->rAreas[i].i4Right  = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Top    > 1000)  {sAEMeteringArea->rAreas[i].i4Top    = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Bottom > 1000)  {sAEMeteringArea->rAreas[i].i4Bottom = 1000;}

        if((sAEMeteringArea->rAreas[i].i4Left != m_eAEMeterArea.rAreas[i].i4Left) || (sAEMeteringArea->rAreas[i].i4Right != m_eAEMeterArea.rAreas[i].i4Right) ||
            (sAEMeteringArea->rAreas[i].i4Top != m_eAEMeterArea.rAreas[i].i4Top) || (sAEMeteringArea->rAreas[i].i4Bottom != m_eAEMeterArea.rAreas[i].i4Bottom)) {
            MY_LOG("New AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n", i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
            MY_LOG("Original AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n", i, m_eAEMeterArea.rAreas[i].i4Left, m_eAEMeterArea.rAreas[i].i4Right, m_eAEMeterArea.rAreas[i].i4Top, m_eAEMeterArea.rAreas[i].i4Bottom, m_eAEMeterArea.rAreas[i].i4Weight);
            m_eAEMeterArea.rAreas[i].i4Left = sAEMeteringArea->rAreas[i].i4Left;
            m_eAEMeterArea.rAreas[i].i4Right = sAEMeteringArea->rAreas[i].i4Right;
            m_eAEMeterArea.rAreas[i].i4Top = sAEMeteringArea->rAreas[i].i4Top;
            m_eAEMeterArea.rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom;
            m_eAEMeterArea.rAreas[i].i4Weight = sAEMeteringArea->rAreas[i].i4Weight;
            bAreaChage = MTRUE;
        }
    }
    if(bAreaChage == MTRUE) {
        m_bTouchAEAreaChage = MTRUE;
        m_eAEMeterArea.u4Count = u4AreaCnt;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMeteringArea(&m_eAEMeterArea);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d TouchAEAreaChage:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, m_bTouchAEAreaChage);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFDInfo(MVOID* a_sFaces)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MINT8 ifaceIdx = 0;

    if(CUST_ENABLE_FACE_AE() == MTRUE) {
        if(pFaces == NULL) {
            MY_LOG("[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
                return S_AF_OK;
        }

        if((pFaces->faces) == NULL) {
            MY_LOG("[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
            return S_AF_OK;
        }

#if 0
        if((m_eAEFDArea.i4Left != pFaces->faces->rect[0]) || (m_eAEFDArea.i4Right != pFaces->faces->rect[2]) || (m_eAEFDArea.i4Top != pFaces->faces->rect[1]) ||
                  (m_eAEFDArea.i4Bottom != pFaces->faces->rect[3]) || (m_eAEFDArea.i4Weight != pFaces->number_of_faces)) {
            m_eAEFDArea.i4Left = pFaces->faces->rect[0];
            m_eAEFDArea.i4Right = pFaces->faces->rect[2];
            m_eAEFDArea.i4Top = pFaces->faces->rect[1];
            m_eAEFDArea.i4Bottom = pFaces->faces->rect[3];
            m_eAEFDArea.i4Weight = pFaces->number_of_faces;
            m_bFaceAEAreaChage = MTRUE;
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
            } else {
                MY_LOG("The AE algo class is NULL (2)\n");
            }
        }
#else
        memset(&m_eAEFDArea, 0, sizeof(AEMeteringArea_T));
        if(pFaces->number_of_faces > MAX_AE_METER_AREAS) {
            m_eAEFDArea.u4Count = MAX_AE_METER_AREAS;
        } else {
            m_eAEFDArea.u4Count = pFaces->number_of_faces;
        }

        for(ifaceIdx=0; ifaceIdx < m_eAEFDArea.u4Count; ifaceIdx++) {
            m_eAEFDArea.rAreas[ifaceIdx].i4Left = pFaces->faces[ifaceIdx].rect[0];
            m_eAEFDArea.rAreas[ifaceIdx].i4Right = pFaces->faces[ifaceIdx].rect[2];
            m_eAEFDArea.rAreas[ifaceIdx].i4Top = pFaces->faces[ifaceIdx].rect[1];
            m_eAEFDArea.rAreas[ifaceIdx].i4Bottom = pFaces->faces[ifaceIdx].rect[3];
            m_eAEFDArea.rAreas[ifaceIdx].i4Weight = 1;
            m_bFaceAEAreaChage = MTRUE;
        }

        MY_LOG("[%s()] i4SensorDev:%d line:%d Face Number:%d ", __FUNCTION__, m_eSensorDev, __LINE__, m_eAEFDArea.u4Count);
#endif

    } else {
        MY_LOG("[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_FACE_AE());
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setOTInfo(MVOID* a_sOT)
{
    MtkCameraFaceMetadata *pOTWindow = (MtkCameraFaceMetadata *)a_sOT;
    MBOOL bEnableObjectTracking;

    if( m_i4ObjectTrackNum != pOTWindow->number_of_faces) {
        MY_LOG("[setOTInfo] Object tracking:%d %d\n", pOTWindow->number_of_faces, m_i4ObjectTrackNum);
        m_i4ObjectTrackNum = pOTWindow->number_of_faces;
        if(pOTWindow->number_of_faces > 0) { // Object tracking enable
            bEnableObjectTracking = MTRUE;
        } else {
            bEnableObjectTracking = MFALSE;
        }

        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEObjectTracking(bEnableObjectTracking);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
        } else {
            MY_LOG("The AE algo class is NULL (2)\n");
        }
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep)
{
MINT32 i4EVValue, i4EVStep;

    if (m_i4EVIndex != i4NewEVIndex) {
        m_i4EVIndex = i4NewEVIndex;
        m_fEVCompStep = fStep;
        i4EVStep = (MINT32) (100 * m_fEVCompStep);
        i4EVValue = i4NewEVIndex * i4EVStep;

        if(i4EVValue < -350) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n40; }
        else if(i4EVValue < -300) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n35; }
        else if(i4EVValue < -250) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n30; }
        else if(i4EVValue < -200) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n25; }
        else if(i4EVValue < -170) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n20; }
        else if(i4EVValue < -160) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n17; }
        else if(i4EVValue < -140) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n15; }
        else if(i4EVValue < -120) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n13; }
        else if(i4EVValue < -90) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n10; }
        else if(i4EVValue < -60) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n07; }
        else if(i4EVValue < -40) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n05; }
        else if(i4EVValue < -10) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n03; }
        else if(i4EVValue == 0) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;   }
        else if(i4EVValue < 40) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_03;   }
        else if(i4EVValue < 60) {     m_eAEEVcomp = LIB3A_AE_EV_COMP_05;  }
        else if(i4EVValue < 90) {     m_eAEEVcomp = LIB3A_AE_EV_COMP_07;  }
        else if(i4EVValue < 110) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_10;   }
        else if(i4EVValue < 140) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_13;   }
        else if(i4EVValue < 160) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_15;   }
        else if(i4EVValue < 180) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_17;   }
        else if(i4EVValue < 210) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_20;   }
        else if(i4EVValue < 260) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_25;   }
        else if(i4EVValue < 310) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_30;   }
        else if(i4EVValue < 360) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_35;   }
        else { m_eAEEVcomp = LIB3A_AE_EV_COMP_40;  }

        MY_LOG("m_i4EVIndex: %d EVComp:%d fEVCompStep:%f PreAEState:%d m_bAELock:%d\n", m_i4EVIndex, m_eAEEVcomp, m_fEVCompStep, m_ePreAEState, m_bAELock);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setEVCompensate(m_eAEEVcomp);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            if(m_bAELock) {
                m_i4WaitVDNum = (m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET+1);
            }
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getEVCompensateIndex()
{
MINT32 iEVIndex;

    switch(m_eAEEVcomp){
        case LIB3A_AE_EV_COMP_03: { iEVIndex = 3;   break; }
        case LIB3A_AE_EV_COMP_05: { iEVIndex = 5;   break; }
        case LIB3A_AE_EV_COMP_07: { iEVIndex = 7;   break; }
        case LIB3A_AE_EV_COMP_10: { iEVIndex = 10;  break; }
        case LIB3A_AE_EV_COMP_13: { iEVIndex = 13;  break; }
        case LIB3A_AE_EV_COMP_15: { iEVIndex = 15;  break; }
        case LIB3A_AE_EV_COMP_17: { iEVIndex = 17;  break; }
        case LIB3A_AE_EV_COMP_20: { iEVIndex = 20;  break; }
        case LIB3A_AE_EV_COMP_25: { iEVIndex = 25;  break; }
        case LIB3A_AE_EV_COMP_30: { iEVIndex = 30;  break; }
        case LIB3A_AE_EV_COMP_35: { iEVIndex = 35;  break; }
        case LIB3A_AE_EV_COMP_40: { iEVIndex = 40;  break; }
        case LIB3A_AE_EV_COMP_n03: { iEVIndex = -3;   break; }
        case LIB3A_AE_EV_COMP_n05: { iEVIndex = -5;   break; }
        case LIB3A_AE_EV_COMP_n07: { iEVIndex = -7;   break; }
        case LIB3A_AE_EV_COMP_n10: { iEVIndex = -10;  break; }
        case LIB3A_AE_EV_COMP_n13: { iEVIndex = -13;  break; }
        case LIB3A_AE_EV_COMP_n15: { iEVIndex = -15;  break; }
        case LIB3A_AE_EV_COMP_n17: { iEVIndex = -17;  break; }
        case LIB3A_AE_EV_COMP_n20: { iEVIndex = -20;  break; }
        case LIB3A_AE_EV_COMP_n25: { iEVIndex = -25;  break; }
        case LIB3A_AE_EV_COMP_n30: { iEVIndex = -30;  break; }
        case LIB3A_AE_EV_COMP_n35: { iEVIndex = -35;  break; }
        case LIB3A_AE_EV_COMP_n40: { iEVIndex = -40;  break; }
        default:
        case LIB3A_AE_EV_COMP_00:
            iEVIndex = 0;
            break;
    }
    return iEVIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMeteringMode(MUINT32 u4NewAEMeteringMode)
{
    AE_METERING_T eNewAEMeteringMode = static_cast<AE_METERING_T>(u4NewAEMeteringMode);
    LIB3A_AE_METERING_MODE_T eAEMeteringMode;

    if ((eNewAEMeteringMode < AE_METERING_BEGIN) || (eNewAEMeteringMode >= NUM_OF_AE_METER)) {
        MY_ERR("Unsupport AE Metering Mode: %d\n", eNewAEMeteringMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEMeteringMode) {
        case AE_METERING_MODE_SOPT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_SOPT;
            break;
        case AE_METERING_MODE_AVERAGE:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_AVERAGE;
            break;
        case AE_METERING_MODE_CENTER_WEIGHT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
        default:
            MY_LOG("The AE metering mode enum value is incorrectly:%d\n", eNewAEMeteringMode);
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
    }

    if (m_eAEMeterMode != eAEMeteringMode) {
        m_eAEMeterMode = eAEMeteringMode;
        MY_LOG("m_eAEMeterMode: %d\n", m_eAEMeterMode);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMeteringMode(m_eAEMeterMode);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMeterMode() const
{
    return static_cast<MINT32>(m_eAEMeterMode);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEISOSpeed(MUINT32 u4NewAEISOSpeed)
{
    MUINT32 eAEISOSpeed;

    if (u4NewAEISOSpeed > LIB3A_AE_ISO_SPEED_MAX) {
        MY_ERR("Unsupport AE ISO Speed: %d\n", u4NewAEISOSpeed);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(u4NewAEISOSpeed) {
        case 0:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_AUTO;
            break;
        case 50:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_50;
            break;
        case 100:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_100;
            break;
        case 150:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_150;
            break;
        case 200:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_200;
            break;
        case 300:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_300;
            break;
        case 400:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_400;
            break;
        case 600:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_600;
            break;
        case 800:
             eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_800;
           break;
        case 1200:
             eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_1200;
           break;
        case 1600:
            eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_1600;
            break;
        case 2400:
             eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_2400;
           break;
        case 3200:
             eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_3200;
           break;
        default:
            if(m_bRealISOSpeed == MTRUE) {   //
                MY_LOG("The Real ISO speed:%d m_bRealISOSpeed:%d \n", u4NewAEISOSpeed, m_bRealISOSpeed);
                eAEISOSpeed = u4NewAEISOSpeed;
            } else {
                MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);
                eAEISOSpeed = (MUINT32) LIB3A_AE_ISO_SPEED_AUTO;
            }
            break;
    }

    if (m_u4AEISOSpeed != eAEISOSpeed) {
        MY_LOG("m_u4AEISOSpeed: %d old:%d\n", eAEISOSpeed, m_u4AEISOSpeed);
        m_u4AEISOSpeed = eAEISOSpeed;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setIsoSpeed(m_u4AEISOSpeed);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;

        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEISOSpeedMode() const
{
    return static_cast<MINT32>(m_u4AEISOSpeed);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 i4NewMinFPS, i4NewMaxFPS, i4SensorMaxFPS;

    i4NewMinFPS = i4NewAEMinFps / 100;
    i4NewMaxFPS = i4NewAEMaxFps / 100;
    i4SensorMaxFPS = m_rSensorMaxFrmRate[m_eSensorMode];

    if (i4NewMaxFPS > i4SensorMaxFPS)
    {
        MY_LOG("[%s] Clipping: i4NewMaxFPS(%d), i4SensorMaxFPS(%d)", __FUNCTION__, i4NewMaxFPS, i4SensorMaxFPS);
        i4NewMaxFPS = i4SensorMaxFPS;
    }

    if ((i4NewMinFPS < LIB3A_AE_FRAMERATE_MODE_05FPS) || (i4NewMaxFPS > LIB3A_AE_FRAMERATE_MODE_MAX)) {
        MY_LOG("Unsupport AE frame rate range value: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        return S_AE_OK;
    } else if(i4NewMinFPS > i4NewMaxFPS) {
        MY_ERR("Unsupport AE frame rate: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        return E_AE_UNSUPPORT_MODE;
    }

    if ((m_i4AEMinFps != i4NewMinFPS) || (m_i4AEMaxFps != i4NewMaxFPS)) {
        m_i4AEMinFps = i4NewMinFPS;
        m_i4AEMaxFps = i4NewMaxFPS;
        MY_LOG("m_i4AEMinFps: %d m_i4AEMaxFps:%d\n", m_i4AEMinFps, m_i4AEMaxFps);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMinMaxFrameRate(m_i4AEMinFps, m_i4AEMaxFps);
            m_pIAeAlgo->setAECamMode(m_eAECamMode);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            m_i4WaitVDNum = (m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET+1);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEFlickerMode(MUINT32 u4NewAEFLKMode)
{
    mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t eNewAEFLKMode = static_cast<mtk_camera_metadata_enum_android_control_ae_antibanding_mode_t>(u4NewAEFLKMode);
    LIB3A_AE_FLICKER_MODE_T eAEFLKMode;

    switch(eNewAEFLKMode) {
        case MTK_CONTROL_AE_ANTIBANDING_MODE_OFF:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_OFF;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_60HZ;
            break;
        case MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_AUTO;
            break;
        default:
            MY_LOG("The flicker enum value is incorrectly:%d\n", eNewAEFLKMode);
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
    }

    if (m_eAEFlickerMode != eAEFLKMode) {
        MY_LOG("AEFlickerMode: %d old:%d\n", eAEFLKMode, m_eAEFlickerMode);
        m_eAEFlickerMode = eAEFLKMode;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEFlickerMode(m_eAEFlickerMode);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;

        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode)
{
    LIB3A_AE_FLICKER_AUTO_MODE_T eNewAEAutoFLKMode = static_cast<LIB3A_AE_FLICKER_AUTO_MODE_T>(u4NewAEAutoFLKMode);

    if ((eNewAEAutoFLKMode <= LIB3A_AE_FLICKER_AUTO_MODE_UNSUPPORTED) || (eNewAEAutoFLKMode >= LIB3A_AE_FLICKER_AUTO_MODE_MAX)) {
        MY_ERR("Unsupport AE auto flicker mode: %d\n", eNewAEAutoFLKMode);
        return E_AE_UNSUPPORT_MODE;
    }

    if (m_eAEAutoFlickerMode != eNewAEAutoFLKMode) {
        m_eAEAutoFlickerMode = eNewAEAutoFLKMode;
        MY_LOG("m_eAEAutoFlickerMode: %d\n", m_eAEAutoFlickerMode);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEFlickerAutoMode(m_eAEAutoFlickerMode);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;

        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAECamMode(MUINT32 u4NewAECamMode)
{
    EAppMode eNewAECamMode = static_cast<EAppMode>(u4NewAECamMode);

    if (m_eCamMode != eNewAECamMode) {
        m_eCamMode = eNewAECamMode;
        MY_LOG("m_eCamMode:%d AECamMode:%d \n", m_eCamMode, m_eAECamMode);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEShotMode(MUINT32 u4NewAEShotMode)
{
    EShotMode eNewAEShotMode = static_cast<EShotMode>(u4NewAEShotMode);

    if (m_eShotMode != eNewAEShotMode) {
        m_eShotMode = eNewAEShotMode;
        m_bAElimitor = MFALSE;
        MY_LOG("m_eAppShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELimiterMode(MBOOL bAELimter)
{
    m_bAElimitor = bAELimter;
    MY_LOG("ShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setSceneMode(MUINT32 u4NewAEScene)
{
    mtk_camera_metadata_enum_android_control_scene_mode_t eNewAEScene = static_cast<mtk_camera_metadata_enum_android_control_scene_mode_t>(u4NewAEScene);
    LIB3A_AE_SCENE_T eAEScene;

    if (eNewAEScene < MTK_CONTROL_SCENE_MODE_DISABLED) {
        MY_ERR("Unsupport AE mode: %d\n", eNewAEScene);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEScene) {
        case MTK_CONTROL_SCENE_MODE_FACE_PRIORITY:
        case MTK_CONTROL_SCENE_MODE_BARCODE:
            eAEScene = LIB3A_AE_SCENE_AUTO;
            break;
        case MTK_CONTROL_SCENE_MODE_ACTION:
            eAEScene = LIB3A_AE_SCENE_ACTION;
            break;
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            eAEScene = LIB3A_AE_SCENE_PORTRAIT;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            eAEScene = LIB3A_AE_SCENE_LANDSCAPE;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT:
            eAEScene = LIB3A_AE_SCENE_NIGHT;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
            eAEScene = LIB3A_AE_SCENE_NIGHT_PORTRAIT;
            break;
        case MTK_CONTROL_SCENE_MODE_THEATRE:
            eAEScene = LIB3A_AE_SCENE_THEATRE;
            break;
        case MTK_CONTROL_SCENE_MODE_BEACH:
            eAEScene = LIB3A_AE_SCENE_BEACH;
            break;
        case MTK_CONTROL_SCENE_MODE_SNOW:
            eAEScene = LIB3A_AE_SCENE_SNOW;
            break;
        case MTK_CONTROL_SCENE_MODE_SUNSET:
            eAEScene = LIB3A_AE_SCENE_SUNSET;
            break;
        case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
            eAEScene = LIB3A_AE_SCENE_STEADYPHOTO;
            break;
        case MTK_CONTROL_SCENE_MODE_FIREWORKS:
            eAEScene = LIB3A_AE_SCENE_FIREWORKS;
            break;
        case MTK_CONTROL_SCENE_MODE_SPORTS:
            eAEScene = LIB3A_AE_SCENE_SPORTS;
            break;
        case MTK_CONTROL_SCENE_MODE_PARTY:
            eAEScene = LIB3A_AE_SCENE_PARTY;
            break;
        case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
            eAEScene = LIB3A_AE_SCENE_CANDLELIGHT;
            break;
        default:
            MY_LOG_IF(m_3ALogEnable,"The Scene mode is not correctly: %d\n", eNewAEScene);
            eAEScene = LIB3A_AE_SCENE_AUTO;
            break;
    }

    if (m_eAEScene != eAEScene) {
        MY_LOG("m_eAEScene: %d old:%d\n", eAEScene, m_eAEScene);
        m_eAEScene = eAEScene;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEScene(m_eAEScene);
            m_pIAeAlgo->getAEMaxISO(m_u4MaxShutter, m_u4MaxISO);
            if(m_bAELock == MFALSE) {

            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
                }
            MY_LOG("m_u4MaxShutter:%d m_u4MaxISO:%d\n", m_u4MaxShutter, m_u4MaxISO);
        } else {
            m_u4MaxShutter = 100000;
            m_u4MaxISO = 800;
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d MaxShutter:%d MaxISO:%d", __FUNCTION__, m_eSensorDev, __LINE__, m_u4MaxShutter, m_u4MaxISO);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEScene() const
{
    return static_cast<MINT32>(m_eAEScene);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMode(MUINT32 u4NewAEMode)
{
    mtk_camera_metadata_enum_android_control_ae_mode_t eNewAEMode = static_cast<mtk_camera_metadata_enum_android_control_ae_mode_t>(u4NewAEMode);
    LIB3A_AE_MODE_T eAEMode;

    MY_LOG_IF(m_3ALogEnable,"%s() : %d %d AEState:%d %d\n", __FUNCTION__, m_eAEMode, eNewAEMode, m_eAEState, m_ePreAEState);

    switch(eNewAEMode) {
        case MTK_CONTROL_AE_MODE_OFF:
            m_eAEMode = LIB3A_AE_MODE_OFF;
            m_eAEState = MTK_CONTROL_AE_STATE_INACTIVE;
            break;
        case MTK_CONTROL_AE_MODE_ON:
            m_eAEMode = LIB3A_AE_MODE_ON;
            break;
        case MTK_CONTROL_AE_MODE_ON_AUTO_FLASH:
            m_eAEMode = LIB3A_AE_MODE_ON_AUTO_FLASH;
            break;
        case MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
            m_eAEMode = LIB3A_AE_MODE_ON_ALWAYS_FLASH;
            break;
        case MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
            m_eAEMode = LIB3A_AE_MODE_ON_AUTO_FLASH_REDEYE;
            break;
        default:
            break;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMode() const
{
    return static_cast<MINT32>(m_eAEMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEState() const
{
    return static_cast<MINT32>(m_eAEState);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setSensorMode(MINT32 i4NewSensorMode)
{
    ESensorMode_T eNewSensorMode;
    MY_LOG("[%s()] m_eSensorDev: %d i4NewSensorMode: %d\n", __FUNCTION__, m_eSensorDev, i4NewSensorMode);

    switch (i4NewSensorMode) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            eNewSensorMode = ESensorMode_Preview;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            eNewSensorMode = ESensorMode_Capture;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            eNewSensorMode = ESensorMode_Video;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            eNewSensorMode = ESensorMode_SlimVideo1;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            eNewSensorMode = ESensorMode_SlimVideo2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            eNewSensorMode = ESensorMode_Custom1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            eNewSensorMode = ESensorMode_Custom2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            eNewSensorMode = ESensorMode_Custom3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            eNewSensorMode = ESensorMode_Custom4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            eNewSensorMode = ESensorMode_Custom5;
            break;
        default:
            MY_ERR("E_AE_UNSUPPORT_SENSOR_MODE: %d\n", i4NewSensorMode);
            return MFALSE;
    }

    if((m_bAEReadyCapture == MTRUE) && (m_bAECaptureUpdate == MFALSE)) {
        ModifyCaptureParamsBySensorMode(eNewSensorMode, m_eSensorMode);
    }

    if (m_eSensorMode != eNewSensorMode) {
        MY_LOG("m_eSensorMode: %d %d\n", eNewSensorMode, m_eSensorMode);
        m_eSensorMode = eNewSensorMode;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAESensorMode(m_eSensorMode);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::updateAEScenarioMode(EIspProfile_T eIspProfile)
{
    if(m_eSensorMode == ESensorMode_Preview) {
        MY_LOG("[%s()] Change AE Pline table SensorMode:%d", __FUNCTION__, m_eSensorMode);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAESensorMode(ESensorMode_Video);
            m_pIAeAlgo->setIsoSpeed(m_u4AEISOSpeed);
            m_bAEMonitorStable = MFALSE;
            m_u4AEScheduleCnt = 0;
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getSensorMode() const
{
    return static_cast<MINT32>(m_eSensorMode);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELock()
{
    MY_LOG("[AeMgr::setAELock] m_bAELock: %d m_bAPAELock:%d m_bAFAELock:%d AEState:%d %d\n", m_bAELock, m_bAPAELock, m_bAFAELock, m_eAEState, m_ePreAEState);
    if((m_bAPAELock == MFALSE) && (m_bAFAELock == MFALSE)) {    // AE unlock
        m_bAELock = MFALSE;
        m_bAEMonitorStable = MFALSE;
        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        m_u4AEScheduleCnt = 0;
    } else {    // AE lock
        m_bAELock = MTRUE;
        m_bOneShotAEBeforeLock = MTRUE;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->lockAE(m_bAELock);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAPAELock(MBOOL bAPAELock)
{
    if (m_bAPAELock != bAPAELock) {
        MY_LOG("[AeMgr::setAPAELock] m_bAPAELock: %d %d\n", m_bAPAELock, bAPAELock);
        m_bAPAELock = bAPAELock;
        setAELock();
        if(m_bAPAELock == MFALSE) {    // AE unlock
            m_eAEState = m_ePreAEState;
        } else {    // AE lock
            m_ePreAEState = m_eAEState;
            m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAFAELock(MBOOL bAFAELock)
{
    if (m_bAFAELock != bAFAELock) {
        MY_LOG("[AeMgr::setAFAELock] m_bAFAELock: %d %d\n", m_bAFAELock, bAFAELock);
        m_bAFAELock = bAFAELock;
        setAELock();
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if((m_eZoomWinInfo.u4XOffset != u4XOffset) || (m_eZoomWinInfo.u4XWidth != u4Width) ||
       (m_eZoomWinInfo.u4YOffset != u4YOffset) || (m_eZoomWinInfo.u4YHeight != u4Height)) {
        MY_LOG("[AeMgr::setZoomWinInfo] New WinX:%d %d New WinY:%d %d Old WinX:%d %d Old WinY:%d %d\n", u4XOffset, u4Width, u4YOffset, u4Height,
           m_eZoomWinInfo.u4XOffset, m_eZoomWinInfo.u4XWidth, m_eZoomWinInfo.u4YOffset, m_eZoomWinInfo.u4YHeight);
        m_eZoomWinInfo.bZoomChange = MTRUE;
        m_eZoomWinInfo.u4XOffset = u4XOffset;
        m_eZoomWinInfo.u4XWidth = u4Width;
        m_eZoomWinInfo.u4YOffset = u4YOffset;
        m_eZoomWinInfo.u4YHeight = u4Height;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getAEMaxMeterAreaNum()
{
    return MAX_METERING_AREAS;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAE()
{
    m_bEnableAE = MTRUE;

    MY_LOG("enableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::disableAE()
{
    m_bEnableAE = MFALSE;

    MY_LOG("disableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPvAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;
MUINT32 u4DFpsThres = 0;
int bRestore=0;
MVOID *pAEStatisticBuf;
MUINT8 *pAEStaticBuf;
BufInfo_T rBufInfo;
mVHDRInputParam_T rInput;
mVHDROutputParam_T rOutput;
MBOOL bCalculateAE = MFALSE;
MBOOL bApplyAE = MFALSE;
MINT32 i4ActiveItem;

    m_bAEReadyCapture = MFALSE;  // reset capture flag
    m_bAECaptureUpdate = MFALSE;
    m_i4TimeOutCnt = MFALSE;  // reset timeout counter
    m_bFrameUpdate = MTRUE;

    if(m_bAdbAEEnable == MTRUE) {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.ae_mgr.lock", value, "0");
        m_bAdbAELock = atoi(value);
        property_get("debug.ae_mgr.log", value, "0");
        m_bAdbAELog = atoi(value);
        property_get("debug.ae_mgr.disableISP", value, "0");
        m_i4AdbAEISPDisable = atoi(value);    // 0 : No disable, 1 : Disable ISP gain to shutter, 2 : Disable ISP gain to sensor gain
        property_get("debug.ae_mgr.preview.update", value, "0");
        m_bAdbAEPreviewUpdate = atoi(value);
        property_get("debug.ae_mgr.disable.smooth", value, "0");
        m_bAdbAEDisableSmooth = atoi(value);
    }

    if(0) {
        pAEStaticBuf = (MUINT8 *)pAEStatBuf;
          MY_LOG("[%s] AE: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", __FUNCTION__,
        *(pAEStaticBuf + 120*4+0) ,*(pAEStaticBuf + 120*4+1) ,*(pAEStaticBuf + 120*4+2),*(pAEStaticBuf + 120*4+3) ,*(pAEStaticBuf + 120*4+4) ,*(pAEStaticBuf + 120*4+5) ,
        *(pAEStaticBuf + 120*4+6) ,*(pAEStaticBuf + 120*4+7) ,*(pAEStaticBuf + 120*4+8) ,*(pAEStaticBuf + 120*4+9) ,*(pAEStaticBuf + 120*4+10) ,*(pAEStaticBuf + 120*4+11) ,
        *(pAEStaticBuf + 120*4+12) ,*(pAEStaticBuf + 120*4+13) ,*(pAEStaticBuf + 120*4+14) ,*(pAEStaticBuf + 120*4+15) ,*(pAEStaticBuf + 120*4+16) ,*(pAEStaticBuf + 120*4+17) ,
        *(pAEStaticBuf + 120*4+18) ,*(pAEStaticBuf + 120*4+19) ,*(pAEStaticBuf + 120*4+20) ,*(pAEStaticBuf + 120*4+21) ,*(pAEStaticBuf + 120*4+22) ,*(pAEStaticBuf + 120*4+23));
    }

//    MY_LOG("%s() : AEMode:%d AEState:%d i4ActiveAEItem:%d u4AAOUpdate:%d\n", __FUNCTION__, m_eAEMode, m_eAEState, i4ActiveAEItem, u4AAOUpdate);
    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        // Dequeue AE DMA buffer
        IAEBufMgr::getInstance().dequeueHwBuf(m_eSensorDev, rBufInfo);
        rInput.u4SensorID = 0; //TBD
        rInput.u4OBValue = 0;
        rInput.u4ISPGain = m_rAEOutput.rPreviewMode.u4IspGain;
        rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
        rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
        if(m_rSensorVCInfo.u2VCModeSelect == 0) {
            rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        } else {
            if(m_strIVHDROutputSetting.u4SEExpTimeInUS != 0) {
                rInput.u2ShutterRatio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            } else {
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d u4SEExpTimeInUS:%d\n", __FUNCTION__, m_eSensorDev, m_strIVHDROutputSetting.u4SEExpTimeInUS);
            }
        }
        rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        rInput.pDataPointer = reinterpret_cast<MVOID *>(rBufInfo.virtAddr);
        decodemVHDRStatistic(rInput, rOutput);
        bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
        pAEStatisticBuf = reinterpret_cast<MVOID *>(rOutput.pDataPointer);
    } else {
        pAEStatisticBuf = pAEStatBuf;
    }

    if(m_pIAeAlgo != NULL) {
        if(pAEStatisticBuf != NULL) {
           {
//            AaaTimer localTimer("AESatistic", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
          AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
          m_pIAeAlgo->setAESatisticBufferAddr(pAEStatisticBuf, rCurrentIspAWBGain);
//          localTimer.End();
            }
            if((m_bIsAutoFlare == MTRUE) && (i4ActiveAEItem & E_AE_FLARE)){
//                AaaTimer localTimer("DoPreFlare", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                m_pIAeAlgo->DoPreFlare(pAEStatisticBuf);
                UpdateFlare2ISP();
//                localTimer.End();
            }
        } else {
            MY_LOG("[%s()] The AAO buffer pointer is NULL  i4SensorDev = %d line:%d u4AAOUpdate:%d i4ActiveAEItem:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, u4AAOUpdate, i4ActiveAEItem);
        }
        // monitor AE state
        monitorAndReschedule(bAAASchedule, m_pIAeAlgo->getBrightnessAverageValue(), m_u4StableYValue, &bCalculateAE, &bApplyAE);
        if(bAAASchedule == MFALSE) {
            i4ActiveItem = 0;
        } else {
            i4ActiveItem = i4ActiveAEItem;
        }
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    // per-frame control test
    if(m_bAdbAEPerFrameCtrl) {
        startSensorPerFrameControl();
        return S_AE_OK;
    }

//    m_pIsAEActive = getAEActiveCycle();
    if(m_eAEMode == LIB3A_AE_MODE_OFF) {
        UpdateSensorISPParams(AE_SENSOR_PER_FRAME_STATE);
    } else if(m_bEnableAE) {
        if ((i4ActiveItem & E_AE_AE_CALC) || (bCalculateAE == MTRUE)) {
            if(m_bAELock || m_bAdbAELock) {
                if(m_bOneShotAEBeforeLock == MTRUE) {
                    rAEInput.eAeState = AE_STATE_AELOCK;

                    m_bOneShotAEBeforeLock = MFALSE;
                } else {
                    rAEInput.eAeState = AE_STATE_AELOCK;
                }
            }  else if(m_bAEOneShotControl == MTRUE) {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                MY_LOG("[%s()]  AE_STATE_ONE_SHOT", __FUNCTION__);
            } else if(m_bTouchAEAreaChage == MTRUE) {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                MY_LOG("[%s()] Enable one shot for touch AE enable:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, m_bTouchAEAreaChage);
            } else {
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
            }

            if(m_eZoomWinInfo.bZoomChange == MTRUE) {
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg);
                } else {
                    MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                }

                m_eZoomWinInfo.bZoomChange = MFALSE;

                // Update AE histogram window config
//                ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev, m_eSensorTG).config(m_i4SensorIdx, m_rAEStatCfg);
                ISP_MGR_AE_STAT_HIST_CONFIG_T::getInstance((ESensorDev_T)m_eSensorDev).config(m_rAEStatCfg, m_i4SensorIdx);
            } else {
                rAEInput.pAESatisticBuffer = pAEStatisticBuf;
                rAEInput.eAeTargetMode = m_eAETargetMode;
                if(m_pIAeAlgo != NULL) {
                    if(m_bRestoreAE == MFALSE) {
//                        AaaTimer localTimer("handleAE", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
//                        localTimer.End();
                        if(m_bFaceAEAreaChage == MTRUE) {   // Face AE window change
                            m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
                            m_bFaceAEAreaChage = MFALSE;
                        }
                        copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);

//                        if((m_eAECamMode == LIB3A_AECAM_MODE_ZSD) || (m_eSensorMode == ESensorMode_Capture)){   // copy the information to
//                            m_rAEOutput.rCaptureMode[0] = m_rAEOutput.rPreviewMode;
//                            copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[0], &rAEOutput);
//                        }
                        if (m_bHDRshot == MTRUE){
                            m_rAEOutput.rPreviewMode = m_rAEOutput.rCaptureMode[0];
                            rAEOutput.bAEStable = MFALSE;
                        }

                        mPreviewMode = m_rAEOutput.rPreviewMode;
                        m_i4WaitVDNum = 0; // reset the delay frame
                        if((rAEInput.eAeState == AE_STATE_NORMAL_PREVIEW) || (rAEInput.eAeState == AE_STATE_ONE_SHOT)) {
                            m_bAEStable = rAEOutput.bAEStable;
                            m_bAEMonitorStable = m_bAEStable;
                        }
                    } else {
                        bRestore=1;
                        m_bRestoreAE = MFALSE;
                        MY_LOG("Restore AE, skip AE one time\n");
                    }
                } else {
                    MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                }


                if(m_bAEStable == MTRUE) {
                    if(m_bAPAELock == MFALSE) {
                        m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                    } else {
                        m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
                    }
                    m_u4StableYValue = m_pIAeAlgo->getBrightnessAverageValue();
                    if(m_bTouchAEAreaChage == MTRUE) {
                        m_bTouchAEAreaChage = MFALSE;
                        MY_LOG("[%s()] Disable one shot for AE stable:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                } else {
                    m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                }
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doPvAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
                    return S_AE_OK;
                }
                if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)){   // apply AE
                    if((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) {
                        UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                    } else {
                        if(bRestore == 0) {
                            UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                        } else {
                            UpdateSensorISPParams(AE_AF_RESTORE_STATE);
                        }
                    }
                }
            }
        } else if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)) {   // apply AE
            if(m_pIAeAlgo != NULL) {
                MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d avgY:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue());
                // continue update for preview or AF state
                if((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) {
                    UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                } else {
                    UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                }
            }
        } else {
            MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d AE idle command:%d\n", __FUNCTION__, m_eSensorDev, i4ActiveItem);
        }
    }else {
        MY_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }

    if(m_bAdbAELog == MTRUE) {   // enable adb log
        MUINT8 uYvalue[AE_BLOCK_NO][AE_BLOCK_NO];

        MY_LOG("[%s()] i4SensorDev:%d Shutter:%d Sensor Gain:%d ISP Gain:%d\n", __FUNCTION__, m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain);

        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO*AE_BLOCK_NO);
            MY_LOG("i4SensorDev:%d block0:%d %d %d %d %d\n", m_eSensorDev, uYvalue[0][0], uYvalue[0][1], uYvalue[0][2], uYvalue[0][3], uYvalue[0][4]);
            MY_LOG("i4SensorDev:%d block1:%d %d %d %d %d\n", m_eSensorDev, uYvalue[1][0], uYvalue[1][1], uYvalue[1][2], uYvalue[1][3], uYvalue[1][4]);
            MY_LOG("i4SensorDev:%d block2:%d %d %d %d %d\n", m_eSensorDev, uYvalue[2][0], uYvalue[2][1], uYvalue[2][2], uYvalue[2][3], uYvalue[2][4]);
            MY_LOG("i4SensorDev:%d block3:%d %d %d %d %d\n", m_eSensorDev, uYvalue[3][0], uYvalue[3][1], uYvalue[3][2], uYvalue[3][3], uYvalue[3][4]);
            MY_LOG("i4SensorDev:%d block4:%d %d %d %d %d\n", m_eSensorDev, uYvalue[4][0], uYvalue[4][1], uYvalue[4][2], uYvalue[4][3], uYvalue[4][4]);

        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        // Enqueue AE DMA buffer
        IAEBufMgr::getInstance().enqueueHwBuf(m_eSensorDev, rBufInfo);

        // Update AE DMA base address for next frame
        IAEBufMgr::getInstance().updateDMABaseAddr(m_eSensorDev);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;
MBOOL bCalculateAE = MFALSE;
MBOOL bApplyAE = MFALSE;
MINT32 i4ActiveItem;

    MY_LOG_IF(m_3ALogEnable,"[doAFAE]:%d %d frameCnt:%d ActiveAE:%d eAETargetMode:%d bTouchAEAreaChage:%d\n", m_i4TimeOutCnt, m_i4WaitVDNum, i4FrameCount, i4ActiveAEItem, m_eAETargetMode, m_bTouchAEAreaChage);

    if(m_i4TimeOutCnt > 18) {
        MY_LOG("[doAFAE] Time out happen\n");
        if(m_bLockExposureSetting == MTRUE) {
            MY_LOG("[doAFAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
            return S_AE_OK;
        }
        //if (i4ActiveAEItem & E_AE_AE_APPLY) {   // apply AE
            m_rAEOutput.rAFMode = mPreviewMode;
            UpdateSensorISPParams(AE_AF_STATE);
        //}
        return S_AE_OK;
    } else {
        m_i4TimeOutCnt++;
    }

    if(m_bEnableAE && (m_bAdbAELock == MFALSE)) {
        if((m_pIAeAlgo != NULL) && (pAEStatBuf != NULL) && ((u4AAOUpdate == 1) || (i4ActiveAEItem & E_AE_FLARE) || (i4ActiveAEItem & E_AE_AE_CALC))) {
          AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
          m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf, rCurrentIspAWBGain);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d u4AAOUpdate:%d i4ActiveAEItem:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, u4AAOUpdate, i4ActiveAEItem);
        }

        // monitor AE state
        monitorAndReschedule(bAAASchedule, m_pIAeAlgo->getBrightnessAverageValue(), m_u4StableYValue, &bCalculateAE, &bApplyAE);
        if(bAAASchedule == MFALSE) {
            i4ActiveItem = 0;
        } else {
            i4ActiveItem = i4ActiveAEItem;
        }
        m_bAEStable = MFALSE;
        m_bAEMonitorStable = m_bAEStable;
        m_bTouchAEAreaChage = MFALSE;

        if ((i4ActiveItem & E_AE_AE_CALC)  || (bCalculateAE == MTRUE)) {
            if((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) {
                m_bAEStable = MTRUE;
                m_bAEMonitorStable = m_bAEStable;
            } else {
                MY_LOG("[doAFAE] AE_STATE_ONE_SHOT\n");
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                rAEInput.pAESatisticBuffer = pAEStatBuf;
                rAEInput.eAeTargetMode = m_eAETargetMode;
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                } else {
                    MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
                mPreviewMode = m_rAEOutput.rPreviewMode;
                m_i4WaitVDNum = 0; // reset the delay frame

                // AE is stable, change to AF state
                if(rAEOutput.bAEStable == MTRUE) {
                    rAEInput.eAeState = AE_STATE_AFASSIST;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    } else {
                        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                    copyAEInfo2mgr(&m_rAEOutput.rAFMode, &rAEOutput);
                    m_AEState = AE_AF_STATE;
                }

                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
                    return S_AE_OK;
                }
                // Using preview state to do AE before AE stable
                if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)) {   // apply AE
                    if(m_AEState == AE_AF_STATE) {
                        UpdateSensorISPParams(AE_AF_STATE);
                    } else {
                        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                    }
                }
            }
        } else if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)) {   // apply AE
            if(m_pIAeAlgo != NULL) {
                MY_LOG("[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d avgY:%d State:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue(), m_AEState);
            }
            // continue update for preview or AF state
            if((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) {
                UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
            } else if(m_AEState == AE_AF_STATE) {
                UpdateSensorISPParams(AE_AF_STATE);
            } else {
                UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
            }
        }
    } else {
        m_bAEStable = MTRUE;
        m_bAEMonitorStable = m_bAEStable;
        MY_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPreCapAE(MINT32 i4FrameCount, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;
MVOID *pAEStatisticBuf;
BufInfo_T rBufInfo;
mVHDRInputParam_T rInput;
mVHDROutputParam_T rOutput;
MBOOL bCalculateAE = MFALSE;
MBOOL bApplyAE = MFALSE;
MINT32 i4ActiveItem;

    if (m_bEnableAE && (m_bAdbAELock == MFALSE))
    {
        if(m_bAEReadyCapture == MFALSE) {
            MY_LOG("[%s()] i4SensorDev:%d Ready:%d isStrobe:%d TimeOut:%d\n", __FUNCTION__, m_eSensorDev, m_bAEReadyCapture, bIsStrobeFired, m_i4TimeOutCnt);
            if(((m_i4TimeOutCnt > 18) && (m_bStrobeOn == MTRUE)) || ((m_i4TimeOutCnt > 8) && (m_bStrobeOn == MFALSE))){
                MY_LOG("[%s()] Time out happen\n", __FUNCTION__);
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                    return S_AE_OK;
                }
                m_rAEOutput.rAFMode = m_rAEOutput.rPreviewMode;
                UpdateSensorISPParams(AE_AF_STATE);
                return S_AE_OK;
            } else {
                m_i4TimeOutCnt++;
            }

            //    MY_LOG("%s() : AEMode:%d AEState:%d i4ActiveAEItem:%d u4AAOUpdate:%d\n", __FUNCTION__, m_eAEMode, m_eAEState, i4ActiveAEItem, u4AAOUpdate);
            if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
                // Dequeue AE DMA buffer
                IAEBufMgr::getInstance().dequeueHwBuf(m_eSensorDev, rBufInfo);
                rInput.u4SensorID = 0; //TBD
                rInput.u4OBValue = 0;
                rInput.u4ISPGain = m_rAEOutput.rPreviewMode.u4IspGain;
                rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
                rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                rInput.pDataPointer = reinterpret_cast<MVOID *>(rBufInfo.virtAddr);
                decodemVHDRStatistic(rInput, rOutput);
                bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
                pAEStatisticBuf = reinterpret_cast<MVOID *>(rOutput.pDataPointer);
            } else {
                pAEStatisticBuf = pAEStatBuf;
            }

            if(m_bAEStable == MFALSE) {
                if((m_pIAeAlgo != NULL) && (pAEStatBuf != NULL) && ((u4AAOUpdate == 1) || (i4ActiveAEItem & E_AE_FLARE) || (i4ActiveAEItem & E_AE_AE_CALC))) {
                  AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
                    m_pIAeAlgo->setAESatisticBufferAddr(pAEStatisticBuf, rCurrentIspAWBGain);
                } else {
                    MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d u4AAOUpdate:%d i4ActiveAEItem:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, u4AAOUpdate, i4ActiveAEItem);
                }

                // monitor AE state
                monitorAndReschedule(bAAASchedule, m_pIAeAlgo->getBrightnessAverageValue(), m_u4StableYValue, &bCalculateAE, &bApplyAE);
                if(bAAASchedule == MFALSE) {
                    i4ActiveItem = 0;
                } else {
                    i4ActiveItem = i4ActiveAEItem;
                }

                if ((i4ActiveItem & E_AE_AE_CALC) ||(bCalculateAE == MTRUE)){
                    MY_LOG_IF(m_3ALogEnable,"[%s()] AE_STATE_ONE_SHOT\n", __FUNCTION__);
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    rAEInput.pAESatisticBuffer = pAEStatisticBuf;
                    rAEInput.eAeTargetMode = m_eAETargetMode;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    } else {
                        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
                    m_i4WaitVDNum = 0; // reset the delay frame
                    mPreviewMode = m_rAEOutput.rPreviewMode;

                    if(m_bLockExposureSetting == MTRUE) {
                         m_bAEStable = MTRUE;
                         m_bAEMonitorStable = m_bAEStable;
                        MY_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                        return S_AE_OK;
                    }
                    // AE is stable, update capture info
                    if(rAEOutput.bAEStable == MTRUE) {
                        UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                        doBackAEInfo();   // do back up AE for Precapture AF state.
                    } else if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)) {
                        // Using preview state to do AE before AE stable
                        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                    }
                } else if ((i4ActiveItem & E_AE_AE_APPLY) || (bApplyAE == MTRUE)) {   // apply AE
                    MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d State:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_AEState);
                    // continue update for preview or AF state
                    UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
               }
            } else {
                MY_LOG_IF(m_3ALogEnable,"[%s()] AE stable already\n", __FUNCTION__);
                UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                doBackAEInfo();   // do back up AE for Precapture AF state.
            }

            if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
             // Enqueue AE DMA buffer
                IAEBufMgr::getInstance().enqueueHwBuf(m_eSensorDev, rBufInfo);

             // Update AE DMA base address for next frame
                IAEBufMgr::getInstance().updateDMABaseAddr(m_eSensorDev);
            }

        } else {
            MY_LOG("[%s()] i4SensorDev:%d Do Nothing Ready:%d isStrobe:%d\n", __FUNCTION__, m_eSensorDev, m_bAEReadyCapture, bIsStrobeFired);
        }
    } else {
        m_bAEStable = MTRUE;
        m_bAEMonitorStable = m_bAEStable;
        MY_LOG("[%s()] AE don't enable Enable:%d\n", __FUNCTION__, m_bEnableAE);
    }

    m_eAEState = MTK_CONTROL_AE_STATE_PRECAPTURE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEState2Converge()
{
    MY_LOG("[%s] Chage to converge state, Old state:%d\n", __FUNCTION__, m_eAEState);
    m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapAE()
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;

#if 0
    if(m_bLockExposureSetting == MTRUE) {
        MY_LOG("[doCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
        return S_AE_OK;
    }
#endif

    MY_LOG("[%s()] i4SensorDev:%d CamMode:%d TargetMode:%d\n", __FUNCTION__, m_eSensorDev, m_eCamMode, m_eAETargetMode);

    if((m_bIsAutoFlare == FALSE) && (m_eSensorMode == ESensorMode_Capture)) {
            rAEInput.eAeState = AE_STATE_POST_CAPTURE;
            rAEInput.pAESatisticBuffer = NULL;
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            mCaptureMode.i2FlareOffset=rAEOutput.i2FlareOffset;
            mCaptureMode.i2FlareGain=rAEOutput.i2FlareGain;
            MY_LOG("[%s()] i4SensorDev:%d Update flare offset :%d Flare Gain::%d\n", __FUNCTION__, m_eSensorDev, mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain);
    }

    if((m_eCamMode == eAppMode_EngMode) && ((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET))) {   // engineer mode and target mode
        UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
    } else {
        UpdateSensorISPParams(AE_CAPTURE_STATE);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int g_defaultFrameRate;
static int g_rExp;
static int g_rAfe;
static int g_rIsp;

static int g_rExpOn2;
static int g_rAfeOn2;
static int g_rIspOn2;

static int g_rIspAFLampOff;
static int g_rFrm;
static AE_MODE_CFG_T g_lastAFInfo;

int mapAFLampOffIsp2(int rollFrmTime, int exp, int afe, int isp, int expOn, int afeOn, int ispOn, int* expOn2=0, int* afeOn2=0, int* ispOn2=0)
{
    ALOGD("AFLampExp, expOn: %d %d %d %d %d %d",exp, afe, isp, expOn, afeOn, ispOn);
    double rDif;
    rDif = (double)exp*afe*isp/expOn/afeOn/ispOn;

    double ispIncRat;
    if(rDif<1.2)
        ispIncRat=1.1;
    else if(rDif<1.5)
        ispIncRat=1.15;
    else if(rDif<2)
        ispIncRat=1.18;
    else
        ispIncRat=1.22;


    int rollFrameTime;

    int exp2;
    int afe2;
    int isp2;

    if(expOn2!=0)
        *expOn2 = expOn;
    if(afeOn2!=0)
        *afeOn2 = afeOn;

    double r;
    r = (rDif*exp)/ (rDif*(exp-rollFrmTime/2)+rollFrmTime/2);

    r = r*1.03;

    ALOGD("AFLampExpp ispIncRat=%lf r=%lf", ispIncRat, r);

    r=1;

    return ispOn*r;
}

int mapAFLampOffIsp(int exp, int afe, int isp, int expOn, int afeOn, int ispOn)
{
    ALOGD("AFLampExp, expOn: %d %d %d %d %d %d",exp, afe, isp, expOn, afeOn, ispOn);
    double rDif;
    rDif = (double)exp*afe*isp/expOn/afeOn/ispOn;

    double ispIncRat;
    if(rDif<1.2)
        ispIncRat=1.1;
    else if(rDif<1.5)
        ispIncRat=1.15;
    else if(rDif<2)
        ispIncRat=1.18;
    else
        ispIncRat=1.22;

    return ispOn*ispIncRat;
}

MRESULT AeMgr::doBackAEInfo()
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    if(m_bEnableAE) {
        MY_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);

        rAEInput.eAeState = AE_STATE_BACKUP_PREVIEW;
        rAEInput.pAESatisticBuffer = NULL;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    } else {
        MY_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AeMgr::setExp(int exp)
{
    MY_LOG("setExp(%d)",exp);
    AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, exp);
}
void AeMgr::setAfe(int afe)
{
    MY_LOG("setAfe(%d)",afe);
    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, afe);
}

void AeMgr::setIsp(int isp)
{
    MY_LOG("setIsp(%d)",isp);
    AE_INFO_T rAEInfo2ISP;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
    }
    rAEInfo2ISP.i4GammaIdx = m_i4GammaIdx;
    rAEInfo2ISP.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
    rAEInfo2ISP.u4MaxISO = m_u4MaxISO*m_rAEPLineLimitation.u4IncreaseISO_x100/100;
    rAEInfo2ISP.u4AEStableCnt = m_u4StableCnt;
    rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
    rAEInfo2ISP.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
    rAEInfo2ISP.u4IspGain = isp;
    rAEInfo2ISP.u4RealISOValue = 100*((rAEInfo2ISP.u4AfeGain*isp)>>10)/1024;

//    IspTuningMgr::getInstance().setAEInfo((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
//    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, isp>>1);
    IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorDev).setIspAEGain(rAEInfo2ISP.u4IspGain>>1);
}

void AeMgr::setRestore(int frm)
{
    MY_LOG("setRestore ====\nsetRestore line=%d frmm=%d delayFrm(shutter, afe, isp)=%d %d %d\n", __LINE__, frm, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);
    m_bOtherIPRestoreAE = MTRUE;
    int expSetFrm;
    int afeSetFrm;
    int ispSetFrm;

#if 0
    int maxFrmDelay=0;
    maxFrmDelay = m_i4ShutterDelayFrames;
    if(maxFrmDelay<m_i4SensorGainDelayFrames)
        maxFrmDelay=m_i4SensorGainDelayFrames;
    if(maxFrmDelay<m_i4IspGainDelayFrames)
        maxFrmDelay=m_i4IspGainDelayFrames;
    expSetFrm = maxFrmDelay-m_i4ShutterDelayFrames;
    afeSetFrm = maxFrmDelay-m_i4SensorGainDelayFrames;
    ispSetFrm = maxFrmDelay-m_i4IspGainDelayFrames;
#else
    int minFrmDelay=0;
    minFrmDelay = m_i4ShutterDelayFrames;
    if(minFrmDelay>m_i4SensorGainDelayFrames)
        minFrmDelay=m_i4SensorGainDelayFrames;
    if(minFrmDelay>m_i4IspGainDelayFrames)
        minFrmDelay=m_i4IspGainDelayFrames;
    expSetFrm = m_i4ShutterDelayFrames-minFrmDelay;
    afeSetFrm = m_i4SensorGainDelayFrames-minFrmDelay;
    ispSetFrm = m_i4IspGainDelayFrames-minFrmDelay;
#endif

    MY_LOG("setRestore exp,afe,isp setFrame %d %d %d\n", expSetFrm, afeSetFrm, ispSetFrm );

    if(frm==expSetFrm) //on exp
    {
    }
    else if(frm==1+expSetFrm) //off exp
    {
        MY_LOG("setExp frm1");
        setExp(g_rExp);

        int frmRate;

        if(g_rExp<33000)
            frmRate = 1000000*10/(40000+33000);
        else
            frmRate = 1000000*10/(40000+g_rExp);

        frmRate = 1000000*10/(40000+g_rExp);
        g_defaultFrameRate = AAASensorMgr::getInstance().getPreviewDefaultFrameRate((ESensorDev_T)m_eSensorDev, m_eSensorMode);
#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, frmRate, m_eSensorMode);
        MY_LOG("setRestore line=%d setFrameRate=%d (extention)\n", __LINE__, frmRate);
#else
#endif



    }
    else if(frm==2+expSetFrm) //next of off exp
    {
#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, g_defaultFrameRate, m_eSensorMode);
        MY_LOG("setRestore line=%d setFrameRate=%d (default)\n", __LINE__, g_defaultFrameRate);
#else
#endif
        MY_LOG("setExp frm2");
        setExp(g_rExp);

    }




    if(frm==afeSetFrm) //on afe
    {
    }
    else if(frm==1+afeSetFrm) //off afe
    {
        MY_LOG("setAfe frm1");
        setAfe(g_rAfe);
    }

    if(frm==ispSetFrm) //on isp
    {
        MY_LOG("setIsp frm0");
        setIsp(g_rIspAFLampOff);
    }
    else if(frm==1+ispSetFrm) //off isp
    {
        MY_LOG("setIsp frm1");
        setIsp(g_rIsp);
    }


}


MRESULT AeMgr::doRestoreAEInfo(MBOOL bRestorePrvOnly)
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    if(m_bEnableAE) {
        MY_LOG("[%s()] i4SensorDev:%d bRestorePrvOnly:%d\n", __FUNCTION__, m_eSensorDev, bRestorePrvOnly);
    rAEInput.eAeState = AE_STATE_RESTORE_PREVIEW;
    rAEInput.pAESatisticBuffer = NULL;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
    MY_LOG_IF(m_3ALogEnable,"[getPreviewParams3] Exp. time: %d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
               m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain,
               m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.u2FrameRate, m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u4RealISO);

    g_rExp = m_rAEOutput.rPreviewMode.u4Eposuretime;
    g_rAfe = m_rAEOutput.rPreviewMode.u4AfeGain;
    g_rIsp = m_rAEOutput.rPreviewMode.u4IspGain;

    mPreviewMode = m_rAEOutput.rPreviewMode;

     m_u4PreExposureTime = m_rAEOutput.rPreviewMode.u4Eposuretime;
     m_u4PreSensorGain = m_rAEOutput.rPreviewMode.u4AfeGain;

    if(bRestorePrvOnly == MFALSE) {
        prepareCapParams();

        int rollFrmTime;
        if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD)
            rollFrmTime = 66000;
        else
            rollFrmTime = 33000;

        g_rIspAFLampOff = mapAFLampOffIsp2(rollFrmTime, g_rExp, g_rAfe, g_rIsp,
                                     m_rAEOutput.rAFMode.u4Eposuretime, m_rAEOutput.rAFMode.u4AfeGain, m_rAEOutput.rAFMode.u4IspGain);
    }

    //m_i4WaitVDNum = m_i4IspGainDelayFrames + 2;
    m_bAEMonitorStable = MFALSE;  // AE monitor - Calculate apply
    m_u4AEScheduleCnt = 0; // AE monitor - Calculate apply
    m_bRestoreAE = MTRUE; // restore AE
    }else {
        MY_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    AE_MODE_CFG_T a_rCaptureInfo;

    if((m_bIsAutoFlare == FALSE) || (m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)){
        MY_LOG("[%s()] i4SensorDev:%d Using predict flare:m_bIsAutoFlare:%d m_eAETargetMode:%d\n", __FUNCTION__, m_eSensorDev, m_bIsAutoFlare, m_eAETargetMode);
        return S_AE_OK;
    }

    if(m_pIAeAlgo != NULL) {
        AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
        m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf, rCurrentIspAWBGain);
        m_pIAeAlgo->CalculateCaptureFlare(pAEStatBuf,bIsStrobe);

        rAEInput.eAeState = AE_STATE_POST_CAPTURE;
        rAEInput.pAESatisticBuffer = NULL;
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        mCaptureMode.i2FlareOffset=rAEOutput.i2FlareOffset;
        mCaptureMode.i2FlareGain=rAEOutput.i2FlareGain;
    }

    if(m_bStrobeOn == TRUE) {
        if(m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableStrobeThres == MFALSE) {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM.rCCTConfig.u4StrobeFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN  - m_rAEInitInput.rAENVRAM.rCCTConfig.u4StrobeFlareOffset));
        }
    } else if(m_eSensorMode == ESensorMode_Video) {
        if(m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableVideoThres == MFALSE) {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM.rCCTConfig.u4VideoFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN  - m_rAEInitInput.rAENVRAM.rCCTConfig.u4VideoFlareOffset));
        }
    } else {
        if(m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableCaptureThres == MFALSE) {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM.rCCTConfig.u4CaptureFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN  - m_rAEInitInput.rAENVRAM.rCCTConfig.u4CaptureFlareOffset));
        }
    }
    MY_LOG("[%s()] i4SensorDev:%d i2FlareOffset:%d i2FlareGain:%d Sensor Mode:%d Strobe:%d Enable:%d %d %d avgY:%d\n", __FUNCTION__, m_eSensorDev,
                   mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain, m_eSensorMode, m_bStrobeOn, m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableCaptureThres,
                   m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableVideoThres, m_rAEInitInput.rAENVRAM.rCCTConfig.bEnableStrobeThres, m_pIAeAlgo->getBrightnessAverageValue());

    UpdateSensorISPParams(AE_POST_CAPTURE_STATE);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getLVvalue(MBOOL isStrobeOn)
{
    if(isStrobeOn == MTRUE) {
        return (m_BVvalue + 50);
    } else {
        return (m_BVvalueWOStrobe + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAOECompLVvalue(MBOOL isStrobeOn)
{

    if(isStrobeOn == MTRUE) {
        return (m_BVvalue + 50);
    } else {
        return (m_AOECompBVvalue + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getCaptureLVvalue()
{
    MINT32 i4LVValue_10x;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->CalculateCaptureLV(&i4LVValue_10x);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return (i4LVValue_10x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo)
{
MINT32 u4AEPlineInfoSize, u4AEPlineMappingSize, u4AFTableSize, u4GainTableSize;
AE_EASYTUNING_PLINE_INFO_T rEasytuningTable;

    if(m_pIAeAlgo != NULL) {
        MY_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
        rAEPlineDebugInfo.u4Size = sizeof(AE_PLINE_DEBUG_INFO_T);
        rAEPlineDebugInfo.u4IsTrue = MTRUE;
        m_pIAeAlgo->getDebugInfo(rAEDebugInfo);
        u4AEPlineInfoSize = sizeof(m_pAEPlineTable->AEPlineInfo);
        u4AEPlineMappingSize = sizeof(strAESceneMapping);
        u4AFTableSize = sizeof(m_rAEInitInput.rAEPARAM.strAFPLine);
        u4GainTableSize = sizeof(m_pAEPlineTable->AEGainList);
        MY_LOG("[getDebugInfo] TotalSize:%d AEPlineInfoSize:%d Mapping:%d AFTable:%d u4GainTableSize:%d\n", rAEPlineDebugInfo.u4Size, u4AEPlineInfoSize, u4AEPlineMappingSize, u4AFTableSize, u4GainTableSize);
        memcpy(&(rEasytuningTable.AEPLineInfomation), &(m_pAEPlineTable->AEPlineInfo), u4AEPlineInfoSize);
        memcpy(&(rEasytuningTable.AEPLineMapping), &(m_pAEPlineTable->sAEScenePLineMapping), u4AEPlineMappingSize);
        memcpy(&(rEasytuningTable.normalAFPlineInfo), &(m_rAEInitInput.rAEPARAM.strAFPLine), u4AFTableSize);
        memcpy(&(rEasytuningTable.ZSDAFPlineInfo), &(m_rAEInitInput.rAEPARAM.strAFZSDPLine), u4AFTableSize);
        memcpy(&(rEasytuningTable.AEPLineGainList), &(m_pAEPlineTable->AEGainList), u4GainTableSize);
        if(sizeof(rAEPlineDebugInfo.PlineInfo) >= sizeof(rEasytuningTable)) {
            memcpy(&rAEPlineDebugInfo.PlineInfo[0], &rEasytuningTable, sizeof(rEasytuningTable));
        } else {
            memcpy(&rAEPlineDebugInfo.PlineInfo[0], &rEasytuningTable, sizeof(rAEPlineDebugInfo.PlineInfo));
        }
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAENvramData(AE_NVRAM_T &rAENVRAM)
{
    MY_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    if(m_p3ANVRAM == NULL) {
        MY_ERR("Nvram 3A pointer NULL\n");
    } else {
        rAENVRAM = m_p3ANVRAM->rAENVRAM;
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo)
{
MUINT32 u4Shutter;
MUINT32 u4SensorGain;
MUINT32 u4ISPGain;
MUINT32 u4FinalGain;
MUINT32 u4PreviewBaseGain=1024;
MUINT32 u4PreviewBaseISO=100;
MUINT32 u4ISOValue;
AE_EXP_GAIN_MODIFY_T rSensorInputData, rSensorOutputData;

    sAEOutputInfo->u4CWValue = sAEInfo->u4CWValue; // for N3D sync

    if(((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) && (m_bAEReadyCapture == MFALSE)){
        updatePreviewParamsByiVHDR(sAEInfo);
        sAEOutputInfo->u4Eposuretime = m_strIVHDROutputSetting.u4SEExpTimeInUS;
        sAEOutputInfo->u4AfeGain = m_strIVHDROutputSetting.u4SESensorGain;
        sAEOutputInfo->u4IspGain = m_strIVHDROutputSetting.u4SEISPGain;
        m_i4LESE_Ratio = sAEInfo->i4LESE_Ratio;   // for HDR LE/SE ratio
    } else {
        u4Shutter = sAEInfo->EvSetting.u4Eposuretime;
        u4SensorGain = sAEInfo->EvSetting.u4AfeGain;
        u4ISPGain = sAEInfo->EvSetting.u4IspGain;

        if(m_bAdbAEPreviewUpdate) {
            char value[PROPERTY_VALUE_MAX] = {'\0'};

            property_get("debug.ae_mgr.shutter", value, "0");
            m_u4AdbAEShutterTime = atoi(value);
            property_get("debug.ae_mgr.sensorgain", value, "0");
            m_u4AdbAESensorGain = atoi(value);
            property_get("debug.ae_mgr.ispgain", value, "0");
            m_u4AdbAEISPGain = atoi(value);
            u4Shutter = m_u4AdbAEShutterTime;
            u4SensorGain = m_u4AdbAESensorGain;
            u4ISPGain = m_u4AdbAEISPGain;
            MY_LOG("[%s()] i4SensorDev:%d Apply New Shutter:%d Sensor Gain:%d ISP Gain:%d m_bAdbAEPreviewUpdate:%d\n", __FUNCTION__, m_eSensorDev, m_u4AdbAEShutterTime, m_u4AdbAESensorGain, m_u4AdbAEISPGain, m_bAdbAEPreviewUpdate);
        }
        // Copy Sensor information to output structure
        if(m_i4AdbAEISPDisable) {
            MY_LOG("[%s()] i4SensorDev:%d Old Shutter:%d Sensor Gain:%d ISP Gain:%d m_i4AdbAEISPDisable:%d\n", __FUNCTION__, m_eSensorDev, u4Shutter, u4SensorGain, u4ISPGain, m_i4AdbAEISPDisable);
            if(m_i4AdbAEISPDisable == 1) {  // Disable ISP gain to shutter
                sAEOutputInfo->u4Eposuretime = (u4Shutter*u4ISPGain) >> 10;
                sAEOutputInfo->u4AfeGain = u4SensorGain;
            } else {   // Disable ISP gain to sensor gain
                sAEOutputInfo->u4Eposuretime = u4Shutter;
                sAEOutputInfo->u4AfeGain = (u4SensorGain*u4ISPGain) >> 10;
            }
            sAEOutputInfo->u4IspGain = 1024;
            MY_LOG("[%s()] i4SensorDev:%d Modify Shutter:%d Sensor Gain:%d ISP Gain:%d\n", __FUNCTION__, m_eSensorDev, u4Shutter, u4SensorGain, u4ISPGain);
        } else {
            sAEOutputInfo->u4Eposuretime = u4Shutter;
            sAEOutputInfo->u4AfeGain = u4SensorGain;
            sAEOutputInfo->u4IspGain = u4ISPGain;
        }
        m_i4LESE_Ratio = 1;   // No used for normal
    }

    // for 3DNR, no used for capture mode
    if((m_rAEPLineLimitation.bEnable == MTRUE) && (m_bAEReadyCapture == MFALSE)){
        if(m_rAEPLineLimitation.bEquivalent == MTRUE) {
            if(m_rAEPLineLimitation.u4IncreaseISO_x100 > 100) {
                if(sAEOutputInfo->u4Eposuretime > (100*m_u4MaxShutter / m_rAEPLineLimitation.u4IncreaseISO_x100)) {
                    rSensorInputData.u4SensorExpTime = 100*m_u4MaxShutter / m_rAEPLineLimitation.u4IncreaseISO_x100;
                    rSensorInputData.u4SensorGain = sAEOutputInfo->u4Eposuretime * sAEOutputInfo->u4AfeGain / rSensorInputData.u4SensorExpTime;
                    rSensorInputData.u4IspGain = sAEOutputInfo->u4IspGain;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
                    } else {
                        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                    MY_LOG("[%s()] i4SensorDev:%d New Shutter:%d %d New Sesnor Gain:%d %d ISP Gain:%d %d\n", __FUNCTION__, m_eSensorDev,
                               rSensorOutputData.u4SensorExpTime, sAEOutputInfo->u4Eposuretime, rSensorOutputData.u4SensorGain,  sAEOutputInfo->u4AfeGain, rSensorOutputData.u4IspGain, sAEOutputInfo->u4IspGain);
                    sAEOutputInfo->u4Eposuretime = rSensorOutputData.u4SensorExpTime;
                    sAEOutputInfo->u4AfeGain = rSensorOutputData.u4SensorGain;
                    sAEOutputInfo->u4IspGain = rSensorOutputData.u4IspGain;
                }
            } else if(m_rAEPLineLimitation.u4IncreaseISO_x100 < 100) {
                if(m_p3ANVRAM != NULL) {
                    u4PreviewBaseISO=m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
                } else {
                    MY_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
                }
                u4FinalGain = (sAEOutputInfo->u4AfeGain*sAEOutputInfo->u4IspGain)>>10 ;
                u4ISOValue = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
                MY_LOG("[%s()] u4ISOValue:%d m_u4MaxISO:%d %d Ratio:%d\n", __FUNCTION__, u4ISOValue, m_u4MaxISO, m_rAEPLineLimitation.u4IncreaseISO_x100);

                if(u4ISOValue > (m_u4MaxISO * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100)) {
                    rSensorInputData.u4SensorGain = sAEOutputInfo->u4AfeGain * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100;
                    rSensorInputData.u4SensorExpTime = sAEOutputInfo->u4Eposuretime * sAEOutputInfo->u4AfeGain / rSensorInputData.u4SensorGain;
                    rSensorInputData.u4IspGain = sAEOutputInfo->u4IspGain;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
                    } else {
                        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                    MY_LOG("[%s()] i4SensorDev:%d New Shutter:%d %d New Sesnor Gain:%d %d ISP Gain:%d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
                               rSensorOutputData.u4SensorExpTime, sAEOutputInfo->u4Eposuretime, rSensorOutputData.u4SensorGain,  sAEOutputInfo->u4AfeGain, rSensorOutputData.u4IspGain, sAEOutputInfo->u4IspGain, m_rAEPLineLimitation.u4IncreaseISO_x100);
                    sAEOutputInfo->u4Eposuretime = rSensorOutputData.u4SensorExpTime;
                    sAEOutputInfo->u4AfeGain = rSensorOutputData.u4SensorGain;
                    sAEOutputInfo->u4IspGain = rSensorOutputData.u4IspGain;
                }
            }
        } else {
            MY_LOG("[%s()] i4SensorDev:%d No rquivalent, Please use AE Pline table:%d ISO:%d Shutter:%d\n", __FUNCTION__, m_eSensorDev, m_rAEPLineLimitation.bEquivalent, m_rAEPLineLimitation.bEnable, m_rAEPLineLimitation.u4IncreaseISO_x100, m_rAEPLineLimitation.u4IncreaseShutter_x100);
        }
    }

    if(sAEOutputInfo->u4IspGain < 1024) {
        MY_LOG("[%s()] i4SensorDev:%d ISP gain too small:%d\n", __FUNCTION__, m_eSensorDev, sAEOutputInfo->u4IspGain);
        sAEOutputInfo->u4IspGain = 1024;
    }
    sAEOutputInfo->u2FrameRate = sAEInfo->u2FrameRate;
    sAEOutputInfo->u4RealISO = sAEInfo->u4ISO;

//    if(m_bIsAutoFlare == TRUE) {
        sAEOutputInfo->i2FlareOffset = sAEInfo->i2FlareOffset;
        sAEOutputInfo->i2FlareGain = sAEInfo->i2FlareGain;
//    } else {     // flare disable, don't update.
//        MY_LOG("[%s()] i4SensorDev:%d i2FlareOffset:%d i2FlareGain:%d\n", __FUNCTION__, m_eSensorDev, sAEOutputInfo->i2FlareOffset, sAEOutputInfo->i2FlareGain);
//    }

    m_BVvalue = sAEInfo->Bv;
    m_AOECompBVvalue = sAEInfo->AoeCompBv;

    if(m_bStrobeOn == MFALSE) {
        m_BVvalueWOStrobe = sAEInfo->Bv;
    }

    m_i4EVvalue = sAEInfo->i4EV;
    m_u4AECondition = sAEInfo->u4AECondition;
    m_i4AEidxCurrent = sAEInfo->i4AEidxCurrent;
    m_i4AEidxNext = sAEInfo->i4AEidxNext;
    m_i2AEFaceDiffIndex = sAEInfo->i2FaceDiffIndex;
    m_i4GammaIdx = sAEInfo->i4gammaidx;

    #ifndef ABS
        #define ABS(x)                                                                  (((x) > 0) ? (x) : (-1*(x)))
    #endif

    m_u4SWHDR_SE = ABS(sAEInfo->i4_swHDR_SEx100);      //for HDR SE ,  -x EV , compare with converge AE

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorResolution()
{
    MRESULT err = S_AE_OK;

    if ((m_eSensorDev == ESensorDev_Main) || (m_eSensorDev == ESensorDev_Sub)) {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[0]);
    } else if(m_eSensorDev == ESensorDev_MainSecond) {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[1]);
    } else {
        MY_ERR("Error sensor device\n");
    }

    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().getSensorWidthHeight fail\n");
        return err;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "nvbuf_util.h"
MRESULT AeMgr::getNvramData(MINT32 i4SensorDev)
{
    MY_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, i4SensorDev);

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SensorDev, (void*&)m_p3ANVRAM, 0);
    if(err!=0)
        MY_ERR("Nvram 3A pointer NULL err=%d\n",err);

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_AE_PLINETABLE, i4SensorDev, (void*&)m_pAEPlineTable, 0);
    if(err!=0)
        MY_ERR("Nvram AE Pline table pointer NULL err=%d\n",err);



    /*
    if (FAILED(NvramDrvMgr::getInstance().init(i4SensorDev))) {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AE_NVRAM_DATA;
    }

    NvramDrvMgr::getInstance().getRefBuf(m_p3ANVRAM);
    if(m_p3ANVRAM == NULL) {
         MY_ERR("Nvram 3A pointer NULL\n");
    }

    NvramDrvMgr::getInstance().getRefBuf(m_pAEPlineTable);
    if(m_pAEPlineTable == NULL) {
         MY_ERR("Nvram AE Pline table pointer NULL\n");
    }*/

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::AEInit(Param_T &rParam)
{
    strAEOutput rAEOutput;
    MINT32 i, j;


    MY_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);

    if(m_p3ANVRAM != NULL) {
        m_rAEInitInput.rAENVRAM = m_p3ANVRAM->rAENVRAM;
    } else {
         MY_ERR("Nvram 3A pointer is NULL\n");
    }

    // Query TG info
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;

    switch(m_eSensorDev) {
        case ESensorDev_Main:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            m_rAEInitInput.rAEPARAM = getAEParam<ESensorDev_Main>();
            m_pIsAEActive = getAEActiveCycle<ESensorDev_Main>();
            m_bEnableAE = isAEEnabled<ESensorDev_Main>();
            m_i4AECycleNum = getAECycleNum<ESensorDev_Main>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_MAIN;
            break;
        case ESensorDev_Sub:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
            m_rAEInitInput.rAEPARAM = getAEParam<ESensorDev_Sub>();
            m_pIsAEActive = getAEActiveCycle<ESensorDev_Sub>();
            m_bEnableAE = isAEEnabled<ESensorDev_Sub>();
            m_i4AECycleNum = getAECycleNum<ESensorDev_Sub>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_SUB;
            break;
        case ESensorDev_MainSecond:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            m_rAEInitInput.rAEPARAM = getAEParam<ESensorDev_MainSecond>();
            m_pIsAEActive = getAEActiveCycle<ESensorDev_MainSecond>();
            m_bEnableAE = isAEEnabled<ESensorDev_MainSecond>();
            m_i4AECycleNum = getAECycleNum<ESensorDev_MainSecond>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_MAIN2;
            break;
        default:    //  Shouldn't happen.
            MY_ERR("Invalid sensor device: %d", m_eSensorDev);
    }

    if(pIHalSensor) {
        pIHalSensor->destroyInstance("ae_mgr");
    }

    MY_LOG("TG = %d, pixel mode = %d\n", rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch(rSensorDynamicInfo.TgInfo){
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        default:
            MY_ERR("rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
            return MFALSE;
    }

    if(bApplyAEPlineTableByCCT == MTRUE) {   // update by CCT
        MY_ERR("AE Pline table update by CCT\n");
    } else if(m_pAEPlineTable != NULL) {
        m_rAEInitInput.rAEPlineTable = m_pAEPlineTable;

        // sensor table mapping
        for(j=0; j < MAX_PLINE_MAP_TABLE; j++) {
            for(i = 0; i < LIB3A_SENSOR_MODE_MAX; i++) {
                m_rAEInitInput.rAEPlineMapTable[j].eAEScene = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].eAEScene;
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[i] = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].ePLineID[i];
            }
            /*MY_LOG("[%s()] %d Scene:%d Table:%d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__, j, m_rAEInitInput.rAEPlineMapTable[j].eAEScene,
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[0], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[1], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[2],
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[3], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[4], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[5],
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[6], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[7], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[8],
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[9], m_rAEInitInput.rAEPlineMapTable[j].ePLineID[10]);*/
        }
    } else {
         MY_ERR("Nvram AE Pline table pointer is NULL\n");
    }

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        m_rAEInitInput.i4AEMaxBlockWidth = m_rSensorVCInfo.u2VCStatWidth;
        m_rAEInitInput.i4AEMaxBlockHeight = m_rSensorVCInfo.u2VCStatHeight;
    } else {
        m_rAEInitInput.i4AEMaxBlockWidth = m_rAEInitInput.rAEPARAM.strAEParasetting.u4AEWinodwNumX;
        m_rAEInitInput.i4AEMaxBlockHeight = m_rAEInitInput.rAEPARAM.strAEParasetting.u4AEWinodwNumY;
    }

    // ezoom info default is sensor resolution
    m_eZoomWinInfo.u4XOffset = 0;
    m_eZoomWinInfo.u4YOffset = 0;
    m_eZoomWinInfo.u4XWidth = m_rAEInitInput.rAEPARAM.strAEParasetting.u4AEWinodwNumX;
    m_eZoomWinInfo.u4YHeight = m_rAEInitInput.rAEPARAM.strAEParasetting.u4AEWinodwNumY;

    m_rAEInitInput.rEZoomWin = m_eZoomWinInfo;
    m_rAEInitInput.eAEMeteringMode = m_eAEMeterMode;
    m_rAEInitInput.eAEScene = m_eAEScene;
    m_rAEInitInput.eAECamMode = m_eAECamMode;
    m_rAEInitInput.eAEFlickerMode = m_eAEFlickerMode;
    m_rAEInitInput.eAEAutoFlickerMode = m_eAEAutoFlickerMode;
    m_rAEInitInput.eAEEVcomp = m_eAEEVcomp;
    m_rAEInitInput.u4AEISOSpeed = m_u4AEISOSpeed;
    m_rAEInitInput.i4AEMaxFps = m_i4AEMaxFps;
    m_rAEInitInput.i4AEMinFps = m_i4AEMinFps;
    m_rAEInitInput.i4SensorMode = m_eSensorMode;
    m_rAEInitInput.eAETargetMode = m_eAETargetMode;
    MY_LOG("[%s()] i4SensorDev:%d SensorIdx:%d AE max block width:%d heigh:%d AE meter:%d CamMode:%d AEScene:%d Flicker :%d %d EVcomp:%d ISO:%d %d MinFps:%d MaxFps:%d Limiter:%d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_rAEInitInput.i4AEMaxBlockWidth, m_rAEInitInput.i4AEMaxBlockHeight, m_eAEMeterMode, m_eAECamMode, m_eAEScene, m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eAEEVcomp, m_u4AEISOSpeed, m_bRealISOSpeed, m_i4AEMinFps, m_i4AEMaxFps, m_bAElimitor);
#if USE_OPEN_SOURCE_AE
    m_pIAeAlgo = NS3A::IAeAlgo::createInstance<NS3A::EAAAOpt_OpenSource>(m_eSensorDev);
#else
    m_pIAeAlgo = NS3A::IAeAlgo::createInstance<NS3A::EAAAOpt_MTK>(m_eSensorDev);
#endif
    if (!m_pIAeAlgo) {
        MY_ERR("AeAlgo::createInstance() fail \n");
        return E_AE_ALGO_INIT_ERR;
    }

    m_pIAeAlgo->lockAE(m_bAELock);
    m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
    m_pIAeAlgo->setAEVideoDynamicEnable(m_bVideoDynamic);
//    m_pIAeAlgo->setAELowLightTargetValue(m_rAEInitInput.rAENVRAM.rCCTConfig.u4AETarget, m_rAEInitInput.rAENVRAM.rCCTConfig.u4InDoorEV - 50, m_rAEInitInput.rAENVRAM.rCCTConfig.u4InDoorEV);   // set AE lowlight target 47 and low light threshold LV5
    m_pIAeAlgo->initAE(&m_rAEInitInput, &rAEOutput, &m_rAEStatCfg);
    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
    m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);  // update limiter
    m_pIAeAlgo->setAEObjectTracking(MFALSE);;
    m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
    m_pIAeAlgo->getAEMaxISO(m_u4MaxShutter, m_u4MaxISO);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.ae_mgr.enable", value, "0");
    m_bAEMgrDebugEnable = atoi(value);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareCapParams()
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;

    MY_LOG("[%s()] i4SensorDev:%d m_eShotMode:%d\n", __FUNCTION__, m_eSensorDev, m_eShotMode);
    rAEInput.eAeState = AE_STATE_CAPTURE;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorIdx = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    m_bAEReadyCapture = MTRUE;  // capture ready flag
    copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[0], &rAEOutput);
    copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[1], &rAEOutput);
    copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[2], &rAEOutput);
    mCaptureMode = m_rAEOutput.rCaptureMode[0];

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
        a_PrvAEPlineTable =  m_CurrentPreviewTable;
        a_CapAEPlineTable = m_CurrentCaptureTable;
        a_StrobeAEPlineTable = m_CurrentStrobetureTable;
        MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD) {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAEPARAM.strStrobeZSDPLine;
    } else {
        a_StrobeAEPlineInfo = m_rAEInitInput.rAEPARAM.strStrobePLine;
    }

    MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d Strobe enable:%d AECamMode:%d\n", __FUNCTION__, m_eSensorDev, a_StrobeAEPlineInfo.bAFPlineEnable, m_eAECamMode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    if(m_p3ANVRAM != NULL) {
        a_rDeviceInfo = m_p3ANVRAM->rAENVRAM.rDevicesInfo;
    } else {
        MY_LOG("[%s()] i4SensorDev:%d NVRAM Data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsDoAEInPreAF()
{
    MY_LOG("[%s()] i4SensorDev:%d DoAEbeforeAF:%d\n", __FUNCTION__, m_eSensorDev, m_rAEInitInput.rAEPARAM.strAEParasetting.bPreAFLockAE);
    return m_rAEInitInput.rAEPARAM.strAEParasetting.bPreAFLockAE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsAEStable()
{
    if (m_bAEMgrDebugEnable) {
        MY_LOG("[%s()] i4SensorDev:%d m_bAEStable:%d\n", __FUNCTION__, m_eSensorDev, m_bAEStable);
    }
    return m_bAEStable;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getBVvalue()
{
    return (m_BVvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsStrobeBVTrigger()
{
    MBOOL bStrobeBVTrigger;
    MINT32 i4Bv = 0;
    strAETable strCurrentPreviewTable;
    strAETable strCurrentCaptureTable;
    strAETable strCurrentStrobeTable;
    strAFPlineInfo strobeAEPlineInfo;

    memset(&strCurrentCaptureTable, 0, sizeof(strAETable));
    getCurrentPlineTable(strCurrentPreviewTable, strCurrentCaptureTable, strCurrentStrobeTable, strobeAEPlineInfo);

    if(m_rAEInitInput.rAEPARAM.strAEParasetting.bEV0TriggerStrobe == MTRUE) {         // The strobe trigger by the EV 0 index
        i4Bv = m_BVvalueWOStrobe;
    } else {
        if(m_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]) {
            if(m_pIAeAlgo != NULL) {
                i4Bv = m_BVvalueWOStrobe - m_pIAeAlgo->getSenstivityDeltaIndex(1024 *1024/ m_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]);
            } else {
                i4Bv = m_BVvalueWOStrobe;
                MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
            }
        }
    }

    bStrobeBVTrigger = (i4Bv < strCurrentCaptureTable.i4StrobeTrigerBV)?MTRUE:MFALSE;

    MY_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d bStrobeBVTrigger:%d BV:%d %d\n", __FUNCTION__, m_eSensorDev, bStrobeBVTrigger, i4Bv, strCurrentCaptureTable.i4StrobeTrigerBV);

    return bStrobeBVTrigger;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setStrobeMode(MBOOL bIsStrobeOn)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setStrobeMode(bIsStrobeOn);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    m_bStrobeOn = bIsStrobeOn;
    // Update flare again
    if(m_bStrobeOn == MTRUE) {   // capture on, get capture parameters for flare again.
//        prepareCapParams();
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo)
{
    a_rPreviewInfo = m_rAEOutput.rPreviewMode;
    MY_LOG_IF(m_3ALogEnable,"[getPreviewParams] Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", a_rPreviewInfo.u4Eposuretime, a_rPreviewInfo.u4AfeGain,
                   a_rPreviewInfo.u4IspGain, a_rPreviewInfo.u2FrameRate, a_rPreviewInfo.i2FlareGain, a_rPreviewInfo.i2FlareOffset, a_rPreviewInfo.u4RealISO);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext)
{
    mPreviewMode = a_rPreviewInfo;
    m_rAEOutput.rAFMode = mPreviewMode;
    m_i4WaitVDNum = 0; // reset the delay frame

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->updateAEIndex(i4AEidxNext);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    MY_LOG("[%s()] m_i4SensorDev:%d i4AEidxNext:%d Exp. mode = %d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        __FUNCTION__, m_eSensorDev, i4AEidxNext, mPreviewMode.u4ExposureMode, mPreviewMode.u4Eposuretime,
        mPreviewMode.u4AfeGain, mPreviewMode.u4IspGain, mPreviewMode.u2FrameRate, mPreviewMode.i2FlareGain, mPreviewMode.i2FlareOffset, mPreviewMode.u4RealISO);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
    MUINT8 i;
    MUINT32 rAEHistogram[AE_HISTOGRAM_BIN] = {0};
    SW_HDR_AE_INFO_T    rSWHdrInfo;
    strAETable strCurrentCaptureTable;
    MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
    MUINT32 u4BinningIndex = 0;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;
    MUINT32 u4FinalGain;

    strHDRInputSetting.u4MaxSensorAnalogGain = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MaxAEExpTimeInUS = 500000; // 0.5sec
    strHDRInputSetting.u4MinAEExpTimeInUS = 500;  // 500us
    if(m_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit < 10000) {
        strHDRInputSetting.u4ShutterLineTime = 1000*m_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit;
    } else {
        strHDRInputSetting.u4ShutterLineTime = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit;
    }

    strHDRInputSetting.u4MaxAESensorGain = 8*m_rAEInitInput.rAENVRAM.rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MinAESensorGain = m_rAEInitInput.rAENVRAM.rDevicesInfo.u4MinGain;

    u4OldSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video2PreRatio;
    u4NewSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Cap2PreRatio;

    if(u4OldSensitivityRatio != 0) {
        u4SensitivityRatio = 1024 *u4NewSensitivityRatio  / u4OldSensitivityRatio;
    } else {
        u4SensitivityRatio = 1024;
        MY_LOG("[%s] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev, u4OldSensitivityRatio);
    }

    if(u4SensitivityRatio <= 300) {
        u4BinningIndex = 20;
    } else if(u4SensitivityRatio <= 450) {
        u4BinningIndex = 15;
    } else if(u4SensitivityRatio <= 768) {
        u4BinningIndex = 10;
    } else {
        u4BinningIndex = 0;
    }

    m_pIAeAlgo->getOnePlineTable(AETABLE_CAPTURE_AUTO, strCurrentCaptureTable);

    if((m_i4AEidxNext + u4BinningIndex) < strCurrentCaptureTable.u4TotalIndex) {
        m_rAEOutput.rCaptureMode[0].u4Eposuretime = strCurrentCaptureTable.pCurrentTable->sPlineTable[m_i4AEidxNext + u4BinningIndex].u4Eposuretime;
        m_rAEOutput.rCaptureMode[0].u4AfeGain = strCurrentCaptureTable.pCurrentTable->sPlineTable[m_i4AEidxNext + u4BinningIndex].u4AfeGain;
        m_rAEOutput.rCaptureMode[0].u4IspGain = strCurrentCaptureTable.pCurrentTable->sPlineTable[m_i4AEidxNext + u4BinningIndex].u4IspGain;
        u4FinalGain = (m_rAEOutput.rCaptureMode[0].u4AfeGain*m_rAEOutput.rCaptureMode[0].u4IspGain)>>10;
        m_rAEOutput.rCaptureMode[0].u4RealISO = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
        MY_LOG("[%s()] m_i4SensorDev:%d Modify Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d Index:%d Binning:%d\n", __FUNCTION__, m_eSensorDev,
              m_rAEOutput.rCaptureMode[0].u4Eposuretime, m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain, m_rAEOutput.rCaptureMode[0].u2FrameRate,
              m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset, m_rAEOutput.rCaptureMode[0].u4RealISO, m_i4AEidxNext, u4BinningIndex);
    }

    strHDRInputSetting.u4ExpTimeInUS0EV = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
    strHDRInputSetting.u4SensorGain0EV = (m_rAEOutput.rCaptureMode[0].u4AfeGain)*(m_rAEOutput.rCaptureMode[0].u4IspGain) >>10;
    strHDRInputSetting.u4ISOValue = m_rAEOutput.rCaptureMode[0].u4RealISO;
    strHDRInputSetting.u1FlareOffset0EV = m_rAEOutput.rCaptureMode[0].i2FlareOffset;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEHistogram(rAEHistogram);
        m_pIAeAlgo->getAESWHdrInfo(rSWHdrInfo);

        if(rSWHdrInfo.eAeTargetMode == AE_MODE_AOE_TARGET) {
            strHDRInputSetting.u4AOEMode = 1;
            strHDRInputSetting.i4GainBase0EV = rSWHdrInfo.i4GainBase;
            strHDRInputSetting.i4LE_LowAvg = rSWHdrInfo.i4LE_LowAvg;
            strHDRInputSetting.i4SEDeltaEVx100 = rSWHdrInfo.i4SE_DeltaEV_x100;
        }
        else if((rSWHdrInfo.eAeTargetMode == AE_MODE_MVHDR_TARGET)) {
            mVHDR_SWHDR_InputParam_T rInput;
            mVHDR_SWHDR_OutputParam_T rOutput;

            if(m_rSensorVCInfo.u2VCModeSelect == 0)
                rInput.i4Ratio = m_rSensorVCInfo.u2VCShutterRatio;
            else
                rInput.i4Ratio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            rInput.LEMax = rSWHdrInfo.i4LE_LowAvg;
            rInput.SEMax = rSWHdrInfo.i4SE_DeltaEV_x100;

            getMVHDR_AEInfo(rInput, rOutput);

            MY_LOG("[%s] Ratio:%d, LEMax:%d, SEMax:%d, SEDeltaEV:%d\n", __FUNCTION__, rInput.i4Ratio, rInput.LEMax, rInput.SEMax, rOutput.i4SEDeltaEVx100);

            strHDRInputSetting.u4AOEMode = 1;
            strHDRInputSetting.i4GainBase0EV = rSWHdrInfo.i4GainBase;
            strHDRInputSetting.i4LE_LowAvg = 0;
            strHDRInputSetting.i4SEDeltaEVx100 = rOutput.i4SEDeltaEVx100;
        }
        else {
            strHDRInputSetting.u4AOEMode = 0;
            strHDRInputSetting.i4GainBase0EV = 0;
            strHDRInputSetting.i4LE_LowAvg = 0;
            strHDRInputSetting.i4SEDeltaEVx100 = 0;
        }

        MY_LOG("[%s()] u4AOEMode = %d, i4GainBase0EV = %d, i4LE_LowAvg = %d, i4SEDeltaEVx100 = %d\n", __FUNCTION__,
                                                                                            strHDRInputSetting.u4AOEMode, strHDRInputSetting.i4GainBase0EV,
                                                                                            strHDRInputSetting.i4LE_LowAvg,  strHDRInputSetting.i4SEDeltaEVx100);

    } else {
        MY_LOG("The AE algo class is NULL (26)\n");
    }

    for (i = 0; i < AE_HISTOGRAM_BIN; i++) {
        strHDRInputSetting.u4Histogram[i] = rAEHistogram[i];
    }

    MY_LOG("[%s()] Input MaxSensorAnalogGain:%d MaxExpTime:%d MinExpTime:%d LineTime:%d MaxSensorGain:%d ExpTime:%d SensorGain:%d FlareOffset:%d\n", __FUNCTION__,
        strHDRInputSetting.u4MaxSensorAnalogGain, strHDRInputSetting.u4MaxAEExpTimeInUS, strHDRInputSetting.u4MinAEExpTimeInUS, strHDRInputSetting.u4ShutterLineTime,
        strHDRInputSetting.u4MaxAESensorGain, strHDRInputSetting.u4ExpTimeInUS0EV, strHDRInputSetting.u4SensorGain0EV, strHDRInputSetting.u1FlareOffset0EV);

    for (i = 0; i < AE_HISTOGRAM_BIN; i+=8) {
        MY_LOG("[%s()] Input Histogram%d~%d:%d %d %d %d %d %d %d %d\n", __FUNCTION__, i, i+7, strHDRInputSetting.u4Histogram[i],
           strHDRInputSetting.u4Histogram[i+1], strHDRInputSetting.u4Histogram[i+2], strHDRInputSetting.u4Histogram[i+3], strHDRInputSetting.u4Histogram[i+4],
           strHDRInputSetting.u4Histogram[i+5], strHDRInputSetting.u4Histogram[i+6], strHDRInputSetting.u4Histogram[i+7]);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCaptureParams(MINT8 index, MINT32 i4EVidx, AE_MODE_CFG_T &a_rCaptureInfo)
{
    strAEOutput rAEOutput;

    a_rCaptureInfo = m_rAEOutput.rCaptureMode[index];

    MY_LOG("[%s()] m_i4SensorDev:%d Capture Exp. mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", __FUNCTION__, m_eSensorDev, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime, a_rCaptureInfo.u4AfeGain,
                   a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u2FrameRate, a_rCaptureInfo.i2FlareGain, a_rCaptureInfo.i2FlareOffset, a_rCaptureInfo.u4RealISO);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;
    AE_EXP_GAIN_MODIFY_T    rSensorInputData, rSensorOutputData;

    if(m_p3ANVRAM != NULL) {
        u4PreviewBaseISO=m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
    } else {
        MY_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    mCaptureMode = a_rCaptureInfo;
    u4FinalGain = (mCaptureMode.u4AfeGain*mCaptureMode.u4IspGain)>>10;
    mCaptureMode.u4RealISO = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
    MY_LOG("[%s()] m_i4SensorDev:%d Exp. mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        __FUNCTION__, m_eSensorDev, mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
        mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);

    if(m_bEnableAE) {
        rSensorInputData.u4SensorExpTime = mCaptureMode.u4Eposuretime;
        rSensorInputData.u4SensorGain = mCaptureMode.u4AfeGain;
        rSensorInputData.u4IspGain = mCaptureMode.u4IspGain;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->switchSensorExposureGain(MTRUE, rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        mCaptureMode.u4Eposuretime = rSensorOutputData.u4SensorExpTime;
        mCaptureMode.u4AfeGain = rSensorOutputData.u4SensorGain;
        mCaptureMode.u4IspGain = rSensorOutputData.u4IspGain;
        u4FinalGain = (mCaptureMode.u4AfeGain*mCaptureMode.u4IspGain)>>10;
        mCaptureMode.u4RealISO = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
        MY_LOG("[%s()] m_i4SensorDev:%d Modify Exp. mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        __FUNCTION__, m_eSensorDev, mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
        mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);
    }

    m_rAEOutput.rCaptureMode[0] = mCaptureMode;
    m_bAECaptureUpdate = MTRUE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setSensorDirectly(CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting)
{
    AE_MODE_CFG_T rCaptureInfo;

    MY_LOG("[%s()] m_i4SensorDev:%d Mode:%d Shutter:%d Sensor Gain:%d ISP Gain:%d ISO:%d Flare Offset:%d %d\n", __FUNCTION__, m_eSensorDev, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime,
    a_rCaptureInfo.u4AfeGain, a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u4RealISO,     a_rCaptureInfo.u4FlareGain, a_rCaptureInfo.u4FlareOffset);

    rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
    rCaptureInfo.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    rCaptureInfo.u4AfeGain = a_rCaptureInfo.u4AfeGain;
    rCaptureInfo.u4IspGain = a_rCaptureInfo.u4IspGain;
    rCaptureInfo.u4RealISO = a_rCaptureInfo.u4RealISO;
    rCaptureInfo.i2FlareGain = (MINT16)a_rCaptureInfo.u4FlareGain;
    rCaptureInfo.i2FlareOffset = (MINT16)a_rCaptureInfo.u4FlareOffset;

    updateCaptureParams(rCaptureInfo);

    if(bDirectlySetting) {
        if((m_eCamMode == eAppMode_EngMode) && ((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET))) {   // engineer mode and target mode
            UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
        } else {
            UpdateSensorISPParams(AE_CAPTURE_STATE);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    MUINT8 iValue;
    AEMeterArea_T sAEMeteringArea = rWinSize;

    if (sAEMeteringArea.i4Left   < -1000)  {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000)  {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000)  {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000)  {sAEMeteringArea.i4Bottom = -1000;}

    if (sAEMeteringArea.i4Left   > 1000)  {sAEMeteringArea.i4Left   = 1000;}
    if (sAEMeteringArea.i4Right  > 1000)  {sAEMeteringArea.i4Right  = 1000;}
    if (sAEMeteringArea.i4Top    > 1000)  {sAEMeteringArea.i4Top    = 1000;}
    if (sAEMeteringArea.i4Bottom > 1000)  {sAEMeteringArea.i4Bottom = 1000;}

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEMeteringAreaValue(sAEMeteringArea, &iValue);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    *uYvalue = iValue;

//    MY_LOG("[getMeteringYvalue] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringBlockAreaValue(AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    AEMeterArea_T sAEMeteringArea = rWinSize;

    if (sAEMeteringArea.i4Left   < -1000)  {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000)  {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000)  {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000)  {sAEMeteringArea.i4Bottom = -1000;}

    if (sAEMeteringArea.i4Left   > 1000)  {sAEMeteringArea.i4Left   = 1000;}
    if (sAEMeteringArea.i4Right  > 1000)  {sAEMeteringArea.i4Right  = 1000;}
    if (sAEMeteringArea.i4Top    > 1000)  {sAEMeteringArea.i4Top    = 1000;}
    if (sAEMeteringArea.i4Bottom > 1000)  {sAEMeteringArea.i4Bottom = 1000;}

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEMeteringBlockAreaValue(sAEMeteringArea, uYvalue, u2YCnt);
    } else {
        MY_LOG("The AE algo class is NULL (25)\n");
    }

//    MY_LOG("[getMeteringYvalue] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEBlockYvalues(pYvalues, size);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getRTParams(FrameOutputParam_T &a_strFrameInfo)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;

    a_strFrameInfo.u4AEIndex = m_i4AEidxNext;
    a_strFrameInfo.u4PreviewShutterSpeed_us = m_rAEOutput.rPreviewMode.u4Eposuretime;
    a_strFrameInfo.u4PreviewSensorGain_x1024 = m_rAEOutput.rPreviewMode.u4AfeGain;
    a_strFrameInfo.u4PreviewISPGain_x1024 = m_rAEOutput.rPreviewMode.u4IspGain;
    u4FinalGain = (a_strFrameInfo.u4PreviewSensorGain_x1024*a_strFrameInfo.u4PreviewISPGain_x1024)>>10;
    if(m_p3ANVRAM != NULL) {
        a_strFrameInfo.u4RealISOValue = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain*u4FinalGain/u4PreviewBaseGain;
    } else {
        a_strFrameInfo.u4RealISOValue = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
    }

    if(m_eCamMode == eAppMode_EngMode){
        rAEInput.eAeState = AE_STATE_CAPTURE;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[0], &rAEOutput);
        a_strFrameInfo.u4CapShutterSpeed_us = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
        a_strFrameInfo.u4CapSensorGain_x1024 = m_rAEOutput.rCaptureMode[0].u4AfeGain;
        a_strFrameInfo.u4CapISPGain_x1024 = m_rAEOutput.rCaptureMode[0].u4IspGain;
    } else {
        a_strFrameInfo.u4CapShutterSpeed_us = m_rAEOutput.rPreviewMode.u4Eposuretime;
        a_strFrameInfo.u4CapSensorGain_x1024 = m_rAEOutput.rPreviewMode.u4AfeGain;
        a_strFrameInfo.u4CapISPGain_x1024 = m_rAEOutput.rPreviewMode.u4IspGain;
    }
    a_strFrameInfo.u4FRameRate_x10 = m_rAEOutput.rPreviewMode.u2FrameRate;
    a_strFrameInfo.i4BrightValue_x10 = m_BVvalue;
    a_strFrameInfo.i4ExposureValue_x10 = m_i4EVvalue;
    a_strFrameInfo.i4LightValue_x10 = (m_BVvalue + 50);
    a_strFrameInfo.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
    a_strFrameInfo.i2FlareGain = m_rAEOutput.rPreviewMode.i2FlareGain;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAECondition(MUINT32 i4AECondition)
{
    if(i4AECondition & m_u4AECondition) {
        return MTRUE;
    } else {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getLCEPlineInfo(LCEInfo_T &a_rLCEInfo)
{
MUINT32 u4LCEStartIdx = 0, u4LCEEndIdx = 0;


    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAELCEIndexInfo(&u4LCEStartIdx, &u4LCEEndIdx);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    a_rLCEInfo.i4AEidxCur = m_i4AEidxCurrent;
    a_rLCEInfo.i4AEidxNext = m_i4AEidxNext;
    a_rLCEInfo.i4NormalAEidx = (MINT32) u4LCEStartIdx;
    a_rLCEInfo.i4LowlightAEidx = (MINT32) u4LCEEndIdx;

    if (m_bAEMgrDebugEnable) {
        MY_LOG("[%s()] m_i4SensorDev:%d i4AEidxCur:%d i4AEidxNext:%d i4NormalAEidx:%d i4LowlightAEidx:%d\n", __FUNCTION__, m_eSensorDev, a_rLCEInfo.i4AEidxCur, a_rLCEInfo.i4AEidxNext, a_rLCEInfo.i4NormalAEidx, a_rLCEInfo.i4LowlightAEidx);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAERotateDegree(MINT32 i4RotateDegree)
{
    if(m_i4RotateDegree == i4RotateDegree) {  // the same degree
        return S_AE_OK;
    }

    MY_LOG("[%s()] m_i4SensorDev:%d old:%d\n", __FUNCTION__, m_eSensorDev, i4RotateDegree, m_i4RotateDegree);
    m_i4RotateDegree = i4RotateDegree;

    if(m_pIAeAlgo != NULL) {
        if((i4RotateDegree == 90) || (i4RotateDegree == 270)){
            m_pIAeAlgo->setAERotateWeighting(MTRUE);
        } else {
            m_pIAeAlgo->setAERotateWeighting(MFALSE);
        }
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateSensorISPParams(AE_STATE_T eNewAEState)
{
    MRESULT err;
    AE_INFO_T rAEInfo2ISP;
    MUINT32 u4IndexRatio;
    MUINT32 u4Index = 0;
    MUINT32 u4ISOValue = 100;
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;
    MBOOL bCaptureState = MFALSE;
    MUINT16 u2FrameRate;
    MUINT32 u4ISPGain = 1024;
    MUINT32 u4MinIsoGain = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
    MUINT32 u4MaxGain = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MaxGain;

    m_AEState = eNewAEState;

    switch(eNewAEState)
    {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
            // sensor initial and send shutter / gain default value
            MY_LOG("[%s()] i4SensorDev:%d Exp Mode: %d Shutter:%d Sensor Gain:%d Isp Gain:%d Flare:%d %d Frame Rate:%d\n",
             __FUNCTION__, m_eSensorDev, m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
             m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u2FrameRate);

            (m_rAEOutput.rPreviewMode.u4ExposureMode == eAE_EXPO_TIME) ? (err = AAASensorMgr::getInstance().setPreviewParams((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain))
                                                           : (err = AAASensorMgr::getInstance().setPreviewLineBaseParams((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain));

            if (FAILED(err)) {
                MY_ERR("AAASensorMgr::getInstance().setPreviewParams fail\n");
                return err;
            }
            m_u4PrevExposureTime = m_rAEOutput.rPreviewMode.u4Eposuretime;
            m_u4PrevSensorGain = m_rAEOutput.rPreviewMode.u4AfeGain;
            m_u4PrevIspGain = m_rAEOutput.rPreviewMode.u4IspGain;

            m_bAEStable = MFALSE;
            updateAEInfo2ISP(AE_INIT_STATE, 0x00);
            m_u4StableCnt = 0 ;
            m_i4WaitVDNum = 0;
            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            break;
        case AE_AUTO_FRAMERATE_STATE:
        case AE_MANUAL_FRAMERATE_STATE:
                if(m_bAEStable == MFALSE) {
                    m_u4StableCnt = 0;
                } else {
                    if(m_u4StableCnt < 0xFFFF) {
                        m_u4StableCnt++;
                    }
                }

            MY_LOG_IF(m_3ALogEnable,"[%s:s] i4SensorDev:%d VDNum %d, Prev %d/%d/%d, Output %d/%d/%d Smooth:%d m_eAEState:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                    m_u4PreExposureTime,
                    m_u4PreSensorGain,
                    m_u4PreIspGain,
                    mPreviewMode.u4Eposuretime,
                    mPreviewMode.u4AfeGain,
                    mPreviewMode.u4IspGain,
                    m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableAESmoothGain, m_eAEState);

            if ((m_u4PrevExposureTime == 0) || (m_u4PrevSensorGain == 0) || (m_u4PrevIspGain == 0)) {
                m_u4PrevExposureTime = mPreviewMode.u4Eposuretime;
                m_u4PrevSensorGain = mPreviewMode.u4AfeGain;
                m_u4PrevIspGain = mPreviewMode.u4IspGain;
            }
#if 1 // edwin version
            if(m_i4WaitVDNum <= (m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET)) {   // restart
                m_u4PreExposureTime = m_u4PrevExposureTime;
                m_u4PreSensorGain = m_u4PrevSensorGain;
                m_u4PreIspGain = m_u4PrevIspGain;

                m_bSetFrameRateValue = MTRUE;
                m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;

                if(m_i4WaitVDNum == m_i4ShutterDelayFrames) {
/*                    if((m_i4AEMinFps == m_i4AEMaxFps) || (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_30FPS)) {    // frame rate control
                        m_bSetFrameRateValue = MTRUE;
                        m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                  }*/
                    m_bSetShutterValue = MTRUE;
                    m_u4UpdateShutterValue = mPreviewMode.u4Eposuretime;
                    m_u4PrevExposureTime = mPreviewMode.u4Eposuretime;
                }
                if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                    m_bSetGainValue = MTRUE;
                    m_u4UpdateGainValue = mPreviewMode.u4AfeGain;
                    m_u4PrevSensorGain = mPreviewMode.u4AfeGain;
                }

#if 0 //enable smooth
                if((m_bAdbAEDisableSmooth == MFALSE) && (m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableAESmoothGain == MTRUE)){
                    if((m_i4WaitVDNum < m_i4IspGainDelayFrames) && (m_ePrevAECamMode == m_eAECamMode) && (m_bOtherIPRestoreAE == MFALSE) && (m_bEnableAE == MTRUE)) {
                        MUINT32 w1;
                        UINT32 CurTotalGain;

#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#endif

                        w1 = 12;
                        CurTotalGain = (mPreviewMode.u4Eposuretime * mPreviewMode.u4AfeGain) /
                            m_u4PreExposureTime *
                            mPreviewMode.u4IspGain /
                            m_u4PreSensorGain;

                        m_u4SmoothIspGain = ((32-w1)*m_u4PrevIspGain + w1*CurTotalGain)>>5;
                        m_u4SmoothIspGain = max(min(m_u4SmoothIspGain, 10*1024), 1024);
                    if(m_u4SmoothIspGain != mPreviewMode.u4IspGain) {
                        IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, m_u4SmoothIspGain>>1);
                        updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_u4SmoothIspGain);
//                MY_LOG("[%s] total/prev/smooth/output %d/%d/%d/%d", __FUNCTION__,
//                        CurTotalGain,
//                        m_u4PreIspGain,
//                        m_u4SmoothIspGain,
//                        m_rAEOutput.rPreviewMode.u4IspGain);
                      }
                    } else {    // reset condition
                        m_u4PreExposureTime =mPreviewMode.u4Eposuretime;
                        m_u4PreSensorGain = mPreviewMode.u4AfeGain;
                        m_u4PrevIspGain = m_u4PreIspGain = mPreviewMode.u4IspGain;

                        if (m_i4WaitVDNum > m_i4IspGainDelayFrames && m_bAEStable == TRUE) { // a complete cycle
                            m_ePrevAECamMode = m_eAECamMode;
                            m_bOtherIPRestoreAE = MFALSE;
                        }
                    }
                }
#endif
                MY_LOG_IF(m_3ALogEnable,"[%s:e] i4SensorDev:%d VDNum %d, Delay %d/%d/%d, Prev %d/%d/%d, Output %d/%d/%d State:%d FrameRate:%d %d %d Flare Offset:%d", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                    m_i4ShutterDelayFrames,
                    m_i4SensorGainDelayFrames,
                    m_i4IspGainDelayFrames,
                    m_u4PreExposureTime,
                    m_u4PreSensorGain,
                    m_u4PreIspGain,
                    mPreviewMode.u4Eposuretime,
                    mPreviewMode.u4AfeGain,
                    mPreviewMode.u4IspGain,
                    m_AEState, mPreviewMode.u2FrameRate, m_i4AEMinFps, m_i4AEMaxFps,
                    mPreviewMode.i2FlareOffset, mPreviewMode.i2FlareGain);

                if(m_i4WaitVDNum == m_i4IspGainDelayFrames) {
                    m_AEState = eNewAEState;
                    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
                }
            }else {
                //MY_LOG("[%s] i4SensorDev:%d m_i4WaitVDNum:%d \n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum);
            }
            m_i4WaitVDNum++;

            u2FrameRate = m_rAEOutput.rPreviewMode.u2FrameRate;

            if(u2FrameRate > m_i4AEMaxFps) {
                MY_LOG_IF(m_3ALogEnable,"[updateAEInfo2ISP] Frame rate too large:%d %d\n", u2FrameRate, m_i4AEMaxFps);
                u2FrameRate = m_i4AEMaxFps;
            }

            if(u2FrameRate != 0) {
                m_rSensorCurrentInfo.u8FrameDuration = 10000000000L / u2FrameRate;
            } else {
                m_rSensorCurrentInfo.u8FrameDuration = 33000000L;
                MY_LOG("No Frame rate value, using default frame rate value (33ms)\n");
            }
#endif
            break;
        case AE_AF_STATE:
            // if the AF setting is the same with preview, skip the re-setting
            if((mPreviewMode.u4Eposuretime != m_rAEOutput.rAFMode.u4Eposuretime) || (mPreviewMode.u4AfeGain != m_rAEOutput.rAFMode.u4AfeGain) ||
                (mPreviewMode.u4IspGain != m_rAEOutput.rAFMode.u4IspGain)) {
                if(m_i4WaitVDNum == m_i4ShutterDelayFrames) {
                    m_bSetShutterValue = MTRUE;
                    m_u4UpdateShutterValue = m_rAEOutput.rAFMode.u4Eposuretime;
                }
                if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                    m_bSetGainValue = MTRUE;
                    m_u4UpdateGainValue = m_rAEOutput.rAFMode.u4AfeGain;
                }
                if(m_i4WaitVDNum >= m_i4IspGainDelayFrames) {
                    updateAEInfo2ISP(AE_AF_STATE, 0x00);
                    m_u4PrevExposureTime = 0;
                    m_u4PrevSensorGain = 0;
                    m_u4PrevIspGain = 0;
                    MY_LOG("[doAFAE] ISP Gain:%d\n", m_rAEOutput.rAFMode.u4IspGain);
                    m_bAEStable = MTRUE;
                    m_bAEMonitorStable = m_bAEStable;
                    prepareCapParams();
                }
                //m_i4WaitVDNum ++;
                MY_LOG_IF(m_3ALogEnable,"[doAFAE] Shutter:%d Sensor Gain:%d\n", m_rAEOutput.rAFMode.u4Eposuretime, m_rAEOutput.rAFMode.u4AfeGain);
            }else {
                 m_bAEStable = MTRUE;
                 m_u4AEScheduleCnt = 0; // for AE monitor
                 m_i4WaitVDNum = m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET+1;
                 prepareCapParams();
                 MY_LOG_IF(m_3ALogEnable,"[doAFAE] AE Stable\n");
            }
            m_i4WaitVDNum++;
            MY_LOG("[doAFAE] AF SensorDev:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d m_i4WaitVDNum:%d\n", m_eSensorDev, m_rAEOutput.rAFMode.u4Eposuretime,
                 m_rAEOutput.rAFMode.u4AfeGain, m_rAEOutput.rAFMode.u4IspGain, m_rAEOutput.rAFMode.u2FrameRate,
                 m_rAEOutput.rAFMode.i2FlareGain, m_rAEOutput.rAFMode.i2FlareOffset, m_rAEOutput.rAFMode.u4RealISO, m_i4WaitVDNum);
            MY_LOG("[doAFAE] Capture i4SensorDev:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", m_eSensorDev, m_rAEOutput.rCaptureMode[0].u4Eposuretime,
                 m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain, m_rAEOutput.rCaptureMode[0].u2FrameRate,
                 m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset, m_rAEOutput.rCaptureMode[0].u4RealISO);
            break;
        case AE_PRE_CAPTURE_STATE:
            m_bAEStable = MTRUE;
            m_bAEMonitorStable = m_bAEStable;
            prepareCapParams();
            MY_LOG("[doPreCapAE] State:%d SensorDev:%d Exp mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                    eNewAEState, m_eSensorDev, m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
                    m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.u2FrameRate,
                    m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u4RealISO);
            MY_LOG("[doPreCapAE] AF Exp mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                 m_rAEOutput.rAFMode.u4ExposureMode, m_rAEOutput.rAFMode.u4Eposuretime,
                 m_rAEOutput.rAFMode.u4AfeGain, m_rAEOutput.rAFMode.u4IspGain, m_rAEOutput.rAFMode.u2FrameRate,
                 m_rAEOutput.rAFMode.i2FlareGain, m_rAEOutput.rAFMode.i2FlareOffset, m_rAEOutput.rAFMode.u4RealISO);
            MY_LOG("[doPreCapAE] Capture Exp mode: %d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                 m_rAEOutput.rCaptureMode[0].u4ExposureMode, m_rAEOutput.rCaptureMode[0].u4Eposuretime,
                 m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain, m_rAEOutput.rCaptureMode[0].u2FrameRate,
                 m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset, m_rAEOutput.rCaptureMode[0].u4RealISO);
            break;
        case AE_CAPTURE_STATE:
            if( (m_bEnableAE == MTRUE) && (m_bMultiCap == MFALSE) &&  (mCaptureMode.u4Eposuretime > m_rAEInitInput.rAEPARAM.strAEParasetting.u4LongCaptureThres) && (m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableLongCaptureCtrl == MTRUE)) {
                (mCaptureMode.u4ExposureMode == eAE_EXPO_TIME) ? (AAASensorMgr::getInstance().setCaptureParams((ESensorDev_T)m_eSensorDev, 30000, mCaptureMode.u4AfeGain))
                                               : (AAASensorMgr::getInstance().setCaptureLineBaseParams((ESensorDev_T)m_eSensorDev, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain));
                m_u4VsyncCnt = m_i4TotalCaptureDelayFrame;
                MY_LOG("[%s()] Enable long time capture performance %d Shutter:%d Cnt:%d\n", __FUNCTION__, m_bMultiCap, mCaptureMode.u4Eposuretime, m_u4VsyncCnt);
            } else {
                (mCaptureMode.u4ExposureMode == eAE_EXPO_TIME) ? (AAASensorMgr::getInstance().setCaptureParams((ESensorDev_T)m_eSensorDev, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain))
                                               : (AAASensorMgr::getInstance().setCaptureLineBaseParams((ESensorDev_T)m_eSensorDev, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain));
                m_u4VsyncCnt = m_i4SensorCaptureDelayFrame;
                MY_LOG("[doCapAE] SensorDev:%d Exp. Mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", m_eSensorDev, mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
                mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);
            }

            // Update to isp tuning
            updateAEInfo2ISP(AE_CAPTURE_STATE, 0x00);
            break;
        case AE_POST_CAPTURE_STATE:
            MY_LOG("[PostCapAE] SensorDev:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", m_eSensorDev, mCaptureMode.u4Eposuretime,
                mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);

            updateAEInfo2ISP(AE_POST_CAPTURE_STATE, 0x00);
            break;
        case AE_SENSOR_PER_FRAME_CAPTURE_STATE:
//            bCaptureState = MTRUE;
        case AE_SENSOR_PER_FRAME_STATE:
            {
                if(m_SensorQueueCtrl.uOutputIndex >= AE_SENSOR_MAX_QUEUE) {
                     m_SensorQueueCtrl.uOutputIndex = 0;
                }

                if(m_SensorQueueCtrl.uInputIndex != m_SensorQueueCtrl.uOutputIndex) {
                    // frame rate control
                    m_bSetFrameRateValue = MTRUE;
                    m_u4UpdateFrameRate_x10 = 10000000 / m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4FrameDuration;

                    if(m_u4UpdateFrameRate_x10 > m_i4AEMaxFps) {
                        MY_LOG("[AE_SENSOR_PER_FRAME_STATE] Frame rate too large:%d %d\n", m_u4UpdateFrameRate_x10, m_i4AEMaxFps);
                        m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                    }

                    m_bSetShutterValue = MTRUE;
                    if(m_i4ShutterDelayFrames == m_i4SensorGainDelayFrames) {
                        m_bSetGainValue = MTRUE;
                    }

                    m_u4UpdateShutterValue = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime;
                    u4ISOValue = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;
                    u4ISPGain = 1024;
                    if(m_p3ANVRAM != NULL) {
                        m_u4UpdateGainValue = (u4ISOValue * 1024) / u4MinIsoGain;
                        if (m_u4UpdateGainValue > u4MaxGain) {
                            u4ISPGain = 1024 * m_u4UpdateGainValue / u4MaxGain;
                            m_u4UpdateGainValue = u4MaxGain;
                        }
                    } else {
                        MY_LOG("[%s] NVRAM is NULL\n", __FUNCTION__);
                        m_u4UpdateGainValue = 1024;
                    }

                    updateAEInfo2ISP(AE_SENSOR_PER_FRAME_STATE, u4ISPGain);
                    m_SensorQueueCtrl.uOutputIndex++;
                } else {
                    MY_LOG("[%s] VDNum %d, No data input %d %d\n", __FUNCTION__, m_i4WaitVDNum, m_SensorQueueCtrl.uInputIndex, m_SensorQueueCtrl.uOutputIndex);
                }

                if(m_i4ShutterDelayFrames < m_i4SensorGainDelayFrames) {
                    if((m_SensorQueueCtrl.uOutputIndex-1) >= (m_i4SensorGainDelayFrames - m_i4ShutterDelayFrames)) {
                        u4Index = (m_SensorQueueCtrl.uOutputIndex-1) - m_i4SensorGainDelayFrames + m_i4ShutterDelayFrames;
                    } else {
                        u4Index = AE_SENSOR_MAX_QUEUE + (m_SensorQueueCtrl.uOutputIndex-1) - m_i4SensorGainDelayFrames + m_i4ShutterDelayFrames;
                    }

                    if(u4Index >= AE_SENSOR_MAX_QUEUE) {
                        MY_LOG("[%s] u4Index incorrectly:%d delay:%d %d %d\n", __FUNCTION__, u4Index, m_SensorQueueCtrl.uOutputIndex-1, m_i4SensorGainDelayFrames, m_i4ShutterDelayFrames);
                        u4Index = m_SensorQueueCtrl.uOutputIndex-1;
                    }

                    if (m_SensorQueueCtrl.rSensorParamQueue[u4Index].u4Sensitivity != 0x00) {
                        u4ISOValue = m_SensorQueueCtrl.rSensorParamQueue[u4Index].u4Sensitivity;
                    } else {
                        u4FinalGain = (mPreviewMode.u4AfeGain*mPreviewMode.u4IspGain)>>10;
                        u4ISOValue = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain*u4FinalGain/u4PreviewBaseGain;
                    }

                    if(m_p3ANVRAM != NULL) {
                        m_u4UpdateGainValue = (u4ISOValue * 1024) / u4MinIsoGain;
                        if (m_u4UpdateGainValue > u4MaxGain) {
                            m_u4UpdateGainValue = u4MaxGain;
                        }
                    } else {
                        MY_LOG("[%s] NVRAM is NULL\n", __FUNCTION__);
                        m_u4UpdateGainValue = 1024;
                    }

                    m_bSetGainValue = MTRUE;
                 }
             }
             break;
        case AE_TWO_SHUTTER_INIT_STATE:
            AAASensorMgr::getInstance().set2ShutterParams((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LEExpTimeInUS, m_strIVHDROutputSetting.u4SEExpTimeInUS, m_strIVHDROutputSetting.u4LESensorGain);
            if(bUpdateSensorAWBGain) {
                  strSensorAWBGain rSensorAWBGain;
                  AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
                rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
                rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;

                AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
            }

            m_u4StableCnt = 0 ;
            updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);

            MY_LOG("[%s] i4SensorDev:%d Delay %d/%d/%d, iVHDR Output %d/%d/%d/%d Flare gain:%d offset:%d State:%d FrameRate:%d %d %d\n", __FUNCTION__, m_eSensorDev,
                m_i4ShutterDelayFrames,
                m_i4SensorGainDelayFrames,
                m_i4IspGainDelayFrames,
                m_strIVHDROutputSetting.u4LEExpTimeInUS,
                m_strIVHDROutputSetting.u4SEExpTimeInUS,
                m_strIVHDROutputSetting.u4LESensorGain,
                m_strIVHDROutputSetting.u4LEISPGain,
                m_rAEOutput.rPreviewMode.i2FlareGain,
                m_rAEOutput.rPreviewMode.i2FlareOffset,
                m_AEState, m_rAEOutput.rPreviewMode.u2FrameRate, m_i4AEMinFps, m_i4AEMaxFps);
            break;
        case AE_TWO_SHUTTER_FRAME_STATE:
            if(m_i4WaitVDNum == m_i4ShutterDelayFrames) {
                if((m_i4AEMinFps == m_i4AEMaxFps) || (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_30FPS)) {      // frame rate control
                    m_bSetFrameRateValue = MTRUE;
                    m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                }
                m_bSetShutterValue = MTRUE;
                m_bSetGainValue = MTRUE;
                if(bUpdateSensorAWBGain) {
                    strSensorAWBGain rSensorAWBGain;
                    AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
                rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
                rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;

                    AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
                }
            }

            if(m_i4WaitVDNum == m_i4IspGainDelayFrames) {   // restart
                m_u4StableCnt = 0 ;
                if(m_bAEStable == MFALSE) {
                    m_u4StableCnt = 0 ;
                } else {
                    if(m_u4StableCnt < 0xFFFF) {
                        m_u4StableCnt++;
                    }
                }
                updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
            }
            MY_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d VDNum %d, Delay %d/%d/%d, iVHDR Output %d/%d/%d/%d Flare gain:%d offset:%d State:%d FrameRate:%d %d %d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                m_i4ShutterDelayFrames,
                m_i4SensorGainDelayFrames,
                m_i4IspGainDelayFrames,
                m_strIVHDROutputSetting.u4LEExpTimeInUS,
                m_strIVHDROutputSetting.u4SEExpTimeInUS,
                m_strIVHDROutputSetting.u4LESensorGain,
                m_strIVHDROutputSetting.u4LEISPGain,
                m_rAEOutput.rPreviewMode.i2FlareGain,
                m_rAEOutput.rPreviewMode.i2FlareOffset,
                m_AEState, m_rAEOutput.rPreviewMode.u2FrameRate, m_i4AEMinFps, m_i4AEMaxFps);
            m_i4WaitVDNum++;
            break;
        case AE_AF_RESTORE_STATE:
        default:
            break;
    }
    return S_AE_OK;
}

MRESULT AeMgr::UpdateFlare2ISP()
{
    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 AeMgr::getAEFaceDiffIndex()
{
 return m_i2AEFaceDiffIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    m_i4ShutterDelayFrames = *i4IspGainDelay - *i4SutterDelay;
    m_i4SensorGainDelayFrames = *i4IspGainDelay - *i4SensorGainDelay;
    if(*i4IspGainDelay > 2) {
        m_i4IspGainDelayFrames = *i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 1 delay frame
    } else {
        m_i4IspGainDelayFrames = 0; // for CQ0 1 delay frame
    }

    MY_LOG("[%s] i4SensorDev:%d m_i4ShutterDelayFrames:%d m_i4SensorGainDelayFrames:%d Isp gain:%d %d %d %d\n", __FUNCTION__, m_eSensorDev,
        m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
MUINT8 uYvalue[5][5];

    if(m_bFrameUpdate == MTRUE) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO*AE_BLOCK_NO);
            * i4Yvalue = uYvalue[2][2];
        } else {
            MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }

        *bFrameUpdate = m_bFrameUpdate;
        m_bFrameUpdate = MFALSE;
    } else {
        * i4Yvalue = 0;
        *bFrameUpdate = MFALSE;
    }

    MY_LOG("[%s] i4SensorDev:%d Yvalue:%d FrameUpdate:%d\n", __FUNCTION__, m_eSensorDev, * i4Yvalue, *bFrameUpdate);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::SetAETargetMode(eAETARGETMODE eAETargetMode)
{
    if(m_eAETargetMode != eAETargetMode) {
       MY_LOG("[%s] i4SensorDev:%d TargetMode:%d %d\n", __FUNCTION__, m_eSensorDev, m_eAETargetMode, eAETargetMode);
       m_eAETargetMode = eAETargetMode;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updatePreviewParamsByiVHDR(strAEOutput *sAEInfo)
{
    IVHDRExpSettingInputParam_T strIVHDRInputSetting;
    IVHDRExpSettingOutputParam_T strIVHDROutputSetting;
    mVHDR_TRANSFER_Param_T strInputTransferParam;
    AE_EXP_GAIN_MODIFY_T    rSensorInputData, rSensorOutputData;

    if((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_60HZ) || ((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_AUTO) && (m_eAEAutoFlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ))) {
        strIVHDRInputSetting.bIs60HZ = MTRUE;
    } else {
        strIVHDRInputSetting.bIs60HZ = MFALSE;
    }

    strIVHDRInputSetting.u4ShutterTime = sAEInfo->EvSetting.u4Eposuretime;
    strIVHDRInputSetting.u4SensorGain = sAEInfo->EvSetting.u4AfeGain*sAEInfo->EvSetting.u4IspGain >> 10;
    if(m_p3ANVRAM != NULL) {
        strIVHDRInputSetting.u41xGainISO = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
        strIVHDRInputSetting.u4SaturationGain = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MinGain;
    } else {
        strIVHDRInputSetting.u41xGainISO = 100;
        strIVHDRInputSetting.u4SaturationGain = 1216;
        MY_LOG("[updatePreviewParamsByiVHDR] NVRAM data is NULL\n");
    }

    getIVHDRExpSetting(strIVHDRInputSetting, strIVHDROutputSetting);

    MY_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d iVHDR 60Hz:%d Shutter:%d Gain:%d %d %d 1xGainISO:%d SatGain:%d Workaround:%d SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
                    strIVHDRInputSetting.bIs60HZ,
                    strIVHDRInputSetting.u4ShutterTime,
                    strIVHDRInputSetting.u4SensorGain,
                    sAEInfo->EvSetting.u4AfeGain,
                    sAEInfo->EvSetting.u4IspGain,
                    strIVHDRInputSetting.u41xGainISO,
                    strIVHDRInputSetting.u4SaturationGain,
                    strIVHDROutputSetting.bEnableWorkaround,
                    strIVHDROutputSetting.u4SEExpTimeInUS,
                    strIVHDROutputSetting.u4SESensorGain,
                    strIVHDROutputSetting.u4SEISPGain,
                    strIVHDROutputSetting.u4LEExpTimeInUS,
                    strIVHDROutputSetting.u4LESensorGain,
                    strIVHDROutputSetting.u4LEISPGain,
                    strIVHDROutputSetting.u4LE_SERatio_x100);

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {   // mVHDR mode used
        strInputTransferParam.bSEInput = isSESetting();
        strInputTransferParam.u2SelectMode = m_rSensorVCInfo.u2VCModeSelect;
         getmVHDRExpSetting(strInputTransferParam, strIVHDROutputSetting);
        MY_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d mVHDR SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
                    strIVHDROutputSetting.u4SEExpTimeInUS,
                    strIVHDROutputSetting.u4SESensorGain,
                    strIVHDROutputSetting.u4SEISPGain,
                    strIVHDROutputSetting.u4LEExpTimeInUS,
                    strIVHDROutputSetting.u4LESensorGain,
                    strIVHDROutputSetting.u4LEISPGain,
                    strIVHDROutputSetting.u4LE_SERatio_x100);
    }

    // LE time
    rSensorInputData.u4SensorExpTime = strIVHDROutputSetting.u4LEExpTimeInUS*101/100;
    rSensorInputData.u4SensorGain = strIVHDROutputSetting.u4LESensorGain*100/101;
    rSensorInputData.u4IspGain = strIVHDROutputSetting.u4LEISPGain;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    m_strIVHDROutputSetting.u4LEExpTimeInUS = rSensorOutputData.u4SensorExpTime;
    m_strIVHDROutputSetting.u4LESensorGain = rSensorOutputData.u4SensorGain;
    m_strIVHDROutputSetting.u4LEISPGain = rSensorOutputData.u4IspGain;

    // SE time
    m_strIVHDROutputSetting.u4SEExpTimeInUS =  strIVHDROutputSetting.u4SEExpTimeInUS*strIVHDROutputSetting.u4SESensorGain / m_strIVHDROutputSetting.u4LESensorGain * strIVHDROutputSetting.u4SEISPGain / m_strIVHDROutputSetting.u4LEISPGain;
    if(m_strIVHDROutputSetting.u4SEExpTimeInUS > m_strIVHDROutputSetting.u4LEExpTimeInUS) {
        m_strIVHDROutputSetting.u4SEExpTimeInUS = m_strIVHDROutputSetting.u4LEExpTimeInUS;
    }
    m_strIVHDROutputSetting.u4SESensorGain = m_strIVHDROutputSetting.u4LESensorGain;
    m_strIVHDROutputSetting.u4SEISPGain = m_strIVHDROutputSetting.u4LEISPGain;

    m_strIVHDROutputSetting.u4LE_SERatio_x100 = 100*m_strIVHDROutputSetting.u4LEExpTimeInUS / m_strIVHDROutputSetting.u4SEExpTimeInUS;
    m_strIVHDROutputSetting.bEnableWorkaround = strIVHDROutputSetting.bEnableWorkaround;

    if(m_strIVHDROutputSetting.bEnableWorkaround == MTRUE) {
        m_u4AFSGG1Gain = 16*m_strIVHDROutputSetting.u4SEISPGain/1024;
    } else {
        m_u4AFSGG1Gain = 16;
    }

    if(m_strIVHDROutputSetting.u4LE_SERatio_x100 > 800) {
        MY_LOG("[%s] i4SensorDev:%d Ratio:%d\n", __FUNCTION__, m_eSensorDev, m_strIVHDROutputSetting.u4LE_SERatio_x100);
        m_strIVHDROutputSetting.u4LE_SERatio_x100 = 800;
    }

    MY_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d iVHDR workound:%d New SE:%d %d %d LE:%d %d %d Ratio:%d SGG1Gain:%d\n", __FUNCTION__, m_eSensorDev,
                    m_strIVHDROutputSetting.bEnableWorkaround,
                    m_strIVHDROutputSetting.u4SEExpTimeInUS,
                    m_strIVHDROutputSetting.u4SESensorGain,
                    m_strIVHDROutputSetting.u4SEISPGain,
                    m_strIVHDROutputSetting.u4LEExpTimeInUS,
                    m_strIVHDROutputSetting.u4LESensorGain,
                    m_strIVHDROutputSetting.u4LEISPGain,
                    m_strIVHDROutputSetting.u4LE_SERatio_x100,
                    m_u4AFSGG1Gain);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAESensorActiveCycle(MINT32* i4ActiveCycle)
{
    * i4ActiveCycle = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1;
    MY_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d Cycle:%d\n", __FUNCTION__, m_eSensorDev, *i4ActiveCycle);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setVideoDynamicFrameRate(MBOOL bVdoDynamicFps)
{
    if(m_bVideoDynamic != bVdoDynamicFps) {
       MY_LOG("[%s] i4SensorDev:%d TargetMode:%d %d Target:%d %d\n", __FUNCTION__, m_eSensorDev, m_bVideoDynamic, bVdoDynamicFps);
       m_bVideoDynamic = bVdoDynamicFps;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::modifyAEPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    if((m_rAEPLineLimitation.bEnable != bEnable) || (m_rAEPLineLimitation.bEquivalent != bEquivalent) || (m_rAEPLineLimitation.u4IncreaseISO_x100 != u4IncreaseISO_x100) || (m_rAEPLineLimitation.u4IncreaseShutter_x100 != u4IncreaseShutter_x100)) {
        MY_LOG("[%s] i4SensorDev:%d Enable:%d %d Equivalent:%d %d IncreaseISO:%d %d IncreaseShutter: %d %d\n", __FUNCTION__, m_eSensorDev,
                      m_rAEPLineLimitation.bEnable, bEnable,
                      m_rAEPLineLimitation.bEquivalent, bEquivalent,
                      m_rAEPLineLimitation.u4IncreaseISO_x100, u4IncreaseISO_x100,
                      m_rAEPLineLimitation.u4IncreaseShutter_x100, u4IncreaseShutter_x100);
        m_rAEPLineLimitation.bEnable = bEnable;
        m_rAEPLineLimitation.bEquivalent = bEquivalent;
        m_rAEPLineLimitation.u4IncreaseISO_x100 = u4IncreaseISO_x100;
        m_rAEPLineLimitation.u4IncreaseShutter_x100 = u4IncreaseShutter_x100;
    }
    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MRESULT AeMgr::ModifyCaptureParamsBySensorMode(MINT32 i4newSensorMode, MINT32 i4oldSensorMode)
{
    MINT32 err;
    MUINT32 u4AFEGain = 0, u4IspGain = 1024, u4BinningRatio = 1;
    MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    MBOOL bStrobeOn;

    if(m_p3ANVRAM == NULL) {
        MY_LOG("[%s] i4SensorDev:%d Nvram 3A pointer is NULL\n", __FUNCTION__, m_eSensorDev);
        return S_AE_OK;
    }

    // Update new sensor mode senstivity ratio
    switch(i4newSensorMode) {
        case ESensorMode_Capture:
            u4NewSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Cap2PreRatio;
            break;
        case ESensorMode_Video:
            u4NewSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video2PreRatio;
            break;
        case ESensorMode_SlimVideo1:
            u4NewSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video12PreRatio;
            break;
        case ESensorMode_SlimVideo2:
            u4NewSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video22PreRatio;
            break;
        case ESensorMode_Preview:
        default:
            u4NewSensitivityRatio = 1024;
            break;
    }

    // Update old sensor mode senstivity ratio
    switch(i4oldSensorMode) {
        case ESensorMode_Capture:
            u4OldSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Cap2PreRatio;
            break;
        case ESensorMode_Video:
            u4OldSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video2PreRatio;
            break;
        case ESensorMode_SlimVideo1:
            u4OldSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video12PreRatio;
            break;
        case ESensorMode_SlimVideo2:
            u4OldSensitivityRatio = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4Video22PreRatio;
            break;
        case ESensorMode_Preview:
        default:
            u4OldSensitivityRatio = 1024;
            break;
    }

    if(u4OldSensitivityRatio != 0) {
        u4SensitivityRatio = 1024 *u4NewSensitivityRatio  / u4OldSensitivityRatio;
    } else {
        u4SensitivityRatio = 1024;
        MY_LOG("[%s] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev, u4OldSensitivityRatio);
    }

    if(u4SensitivityRatio <= 300) {
        u4BinningRatio = 4;
    } else if(u4SensitivityRatio <= 450) {
        u4BinningRatio = 3;
    } else if(u4SensitivityRatio <= 768) {
        u4BinningRatio = 2;
    } else {
        u4BinningRatio = 1;
    }

    if(i4newSensorMode != ESensorMode_Capture) {
        mCaptureMode.u4Eposuretime = mCaptureMode.u4Eposuretime/u4BinningRatio;
        mCaptureMode.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
        mCaptureMode.i2FlareGain = m_rAEOutput.rPreviewMode.i2FlareGain;
        MY_LOG("[%s] i4SensorDev:%d New Capture Shutter:%d Binning:%d Flare:%d %d\n", __FUNCTION__, m_eSensorDev, mCaptureMode.u4Eposuretime, u4BinningRatio,
                                mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAESGG1Gain(MUINT32 *pSGG1Gain)
{
    *pSGG1Gain = m_u4AFSGG1Gain;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEOneShotControl(MBOOL bAEControl)
{
    if(m_bAEOneShotControl != bAEControl) {
        MY_LOG("[%s] i4SensorDev:%d One Shot control old:%d new:%d\n", __FUNCTION__, m_eSensorDev, m_bAEOneShotControl, bAEControl);
         m_bAEOneShotControl = bAEControl;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEPlineTable(eAETableID eTableID, strAETable &a_AEPlineTable)
{
    strAETable strAEPlineTable;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getOnePlineTable(eTableID, strAEPlineTable);
        a_AEPlineTable =  strAEPlineTable;
        MY_LOG("[%s()] i4SensorDev:%d PreId:%d CapId:%d GetID:%d\n", __FUNCTION__, m_eSensorDev, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, eTableID);
    } else {
        MY_LOG("[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::startSensorPerFrameControl()
{
    if(m_u4IndexTestCnt < 300) {
        if(m_u4IndexTestCnt%(m_i4IspGainDelayFrames+1) == 0) {
            m_rAEOutput.rPreviewMode.u4Eposuretime = 10000*2;
            if(m_i4ShutterDelayFrames == m_i4SensorGainDelayFrames) {
                m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
             } else {
                m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
            }
        } else if(m_u4IndexTestCnt%(m_i4IspGainDelayFrames+1) == 1) {
            m_rAEOutput.rPreviewMode.u4Eposuretime = 10000;
            if(m_i4ShutterDelayFrames == m_i4SensorGainDelayFrames) {
                m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
            } else {
                m_rAEOutput.rPreviewMode.u4AfeGain = 1024*2;
            }
        }
        m_rAEOutput.rPreviewMode.u4IspGain = 1024;
        m_u4IndexTestCnt++;
    } else {
        m_rAEOutput.rPreviewMode.u4Eposuretime = 1000;
        m_rAEOutput.rPreviewMode.u4AfeGain = 1024;
        m_rAEOutput.rPreviewMode.u4IspGain = 1024;
    }
    AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime);
    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4AfeGain);
//    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, m_rAEOutput.rPreviewMode.u4IspGain>>1);
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorDev).setIspAEGain(m_rAEOutput.rPreviewMode.u4IspGain>>1);
    MY_LOG("[%s()] u4Index:%d avgY:%d Shutter:%d Gain:%d %d\n", __FUNCTION__ , m_u4IndexTestCnt, m_pIAeAlgo->getBrightnessAverageValue(), m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsNeedUpdateSensor()
{
    return (m_bSetShutterValue | m_bSetGainValue | m_bSetFrameRateValue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorbyI2C()
{
    MINT32 err = S_AE_OK;

    if(m_bSetFrameRateValue) {     // update frame rate
        m_bSetFrameRateValue = MFALSE;
//        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        err = AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, m_u4UpdateFrameRate_x10, m_eSensorMode);
//        localTimer.End();
        if (FAILED(err)) {
            MY_ERR("AAASensorMgr::getInstance().setPreviewMaxFrameRate fail\n");
        }
    }

    if((m_eAETargetMode == AE_MODE_AOE_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)) {   // mVHDR/iVHDR sensor control
        if(m_bSetShutterValue || m_bSetGainValue) {    // update shutter value
            m_bSetShutterValue = MFALSE;
            m_bSetGainValue = MFALSE;
            AAASensorMgr::getInstance().set2ShutterParams((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LEExpTimeInUS, m_strIVHDROutputSetting.u4SEExpTimeInUS, m_strIVHDROutputSetting.u4LESensorGain);
        }
    } else {   // normal control
        if(m_bSetShutterValue) {    // update shutter value
            m_bSetShutterValue = MFALSE;
//            AaaTimer localTimer("SetSensorShutter", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            err = AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, m_u4UpdateShutterValue);
//            localTimer.End();
            if (FAILED(err)) {
                MY_ERR("AAASensorMgr::getInstance().setSensorExpTime fail\n");
            }
        }
        if(m_bSetGainValue) {    // update sensor gain value
            m_bSetGainValue = MFALSE;
            if (m_pSensorGainThread)
            {
                ThreadSensorGain::Cmd_T rCmd(0, 0, m_u4UpdateGainValue);
                m_pSensorGainThread->postCmd(&rCmd);
            }
            else
            {
            err = AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, m_u4UpdateGainValue);
            }
            if (FAILED(err)) {
                MY_ERR("AAASensorMgr::getInstance().setSensorGain fail\n");
            }
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateAEInfo2ISP(AE_STATE_T eNewAEState, MUINT32 u4ISPGain)
{
AE_INFO_T rAEInfo2ISP;
MUINT32 u4FinalGain;
MUINT32 u4PreviewBaseGain=1024;
MUINT32 u4PreviewBaseISO=100;
MINT16 i2FlareGain;
MUINT16 u2FrameRate = 300;

    // Update to isp tuning
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP, LIB3A_SENSOR_MODE_PRVIEW);
    } else {
        MY_LOG("The AE algo class is NULL (updateAEInfo2ISP)\n");
    }

    rAEInfo2ISP.i4GammaIdx = m_i4GammaIdx;
    rAEInfo2ISP.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
    rAEInfo2ISP.u4MaxISO = m_u4MaxISO*m_rAEPLineLimitation.u4IncreaseISO_x100/100;
    rAEInfo2ISP.u4AEStableCnt = m_u4StableCnt;

    switch(eNewAEState) {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
        case AE_AUTO_FRAMERATE_STATE:
        case AE_MANUAL_FRAMERATE_STATE:
            rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
            rAEInfo2ISP.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
            rAEInfo2ISP.i2FlareOffset  =m_rAEOutput.rPreviewMode.i2FlareOffset;

            if(u4ISPGain != 0x00) {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                MY_LOG("[updateAEInfo2ISP] Smooth ISP Gain:%d\n", rAEInfo2ISP.u4IspGain);
            } else {
                rAEInfo2ISP.u4IspGain = m_rAEOutput.rPreviewMode.u4IspGain;
            }

            if(m_p3ANVRAM != NULL) {
                u4PreviewBaseISO=m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
            } else {
                MY_LOG("[updateAEInfo2ISP] NVRAM data is NULL\n");
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain*rAEInfo2ISP.u4IspGain)>>10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO*u4FinalGain+(u4PreviewBaseGain-1))/u4PreviewBaseGain;
            u2FrameRate = m_rAEOutput.rPreviewMode.u2FrameRate;
            break;
        case AE_AF_STATE:
            rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rAFMode.u4Eposuretime;
            rAEInfo2ISP.u4AfeGain = m_rAEOutput.rAFMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = m_rAEOutput.rAFMode.u4IspGain;
            rAEInfo2ISP.i2FlareOffset  =m_rAEOutput.rAFMode.i2FlareOffset;
            rAEInfo2ISP.u4RealISOValue = m_rAEOutput.rAFMode.u4RealISO;
            u2FrameRate = m_rAEOutput.rAFMode.u2FrameRate;
            break;
        case AE_CAPTURE_STATE:
        case AE_POST_CAPTURE_STATE:
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP, LIB3A_SENSOR_MODE_CAPTURE);
            } else {
                MY_LOG("The AE algo class is NULL (updateAEInfo2ISP)\n");
            }

            rAEInfo2ISP.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rAEInfo2ISP.i2FlareOffset  =mCaptureMode.i2FlareOffset;
            rAEInfo2ISP.u4AfeGain = mCaptureMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = mCaptureMode.u4IspGain;
            rAEInfo2ISP.u4RealISOValue = mCaptureMode.u4RealISO;
            u2FrameRate = mCaptureMode.u2FrameRate;
            break;
        case AE_SENSOR_PER_FRAME_STATE :
            rAEInfo2ISP.u4Eposuretime = m_u4UpdateShutterValue;
            rAEInfo2ISP.u4AfeGain = m_u4UpdateGainValue;
            rAEInfo2ISP.i2FlareOffset  = 0;

            if(u4ISPGain != 0x00) {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                MY_LOG("[updateAEInfo2ISP] Smooth ISP Gain:%d avgY:%d\n", rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            } else {
                rAEInfo2ISP.u4IspGain = 1024;
                MY_LOG("[updateAEInfo2ISP] No Smooth ISP Gain:%d avgY:%d\n", rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }

            if(m_p3ANVRAM != NULL) {
                u4PreviewBaseISO = m_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
            } else {
                MY_LOG("[updateAEInfo2ISP] NVRAM data is NULL\n");
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain*rAEInfo2ISP.u4IspGain)>>10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO*u4FinalGain+(u4PreviewBaseGain-1))/u4PreviewBaseGain;
            u2FrameRate = m_u4UpdateFrameRate_x10;
            break;
        case AE_PRE_CAPTURE_STATE:
        case AE_AF_RESTORE_STATE:
        case AE_SENSOR_PER_FRAME_CAPTURE_STATE:
            MY_LOG("[updateAEInfo2ISP] Nothing to do State:%d\n", eNewAEState);
            return S_AE_OK;
    }

    if(u2FrameRate > m_i4AEMaxFps) {
        MY_LOG_IF(m_3ALogEnable,"[updateAEInfo2ISP] Frame rate is too large:%d %d\n", u2FrameRate, m_i4AEMaxFps);
        u2FrameRate = m_i4AEMaxFps;
    }

    if(u2FrameRate != 0) {
        m_rSensorCurrentInfo.u8FrameDuration = 10000000000L / u2FrameRate;
    } else {
        m_rSensorCurrentInfo.u8FrameDuration = 33000000L;
        MY_LOG("No Frame rate value, using default frame rate value (33ms)\n");
    }

    m_rSensorCurrentInfo.u8ExposureTime = rAEInfo2ISP.u4Eposuretime * 1000L;
    m_rSensorCurrentInfo.u4Sensitivity = rAEInfo2ISP.u4RealISOValue;
    i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - rAEInfo2ISP.i2FlareOffset);

//    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, rAEInfo2ISP.u4IspGain>>1);
//    IspTuningMgr::getInstance().setIspFlareGainOffset((ESensorDev_T)m_eSensorDev, i2FlareGain, (-1*rAEInfo2ISP.i2FlareOffset));
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorDev).setIspAEGain(rAEInfo2ISP.u4IspGain>>1);
    ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_eSensorDev).setIspFlare(i2FlareGain, (-1*rAEInfo2ISP.i2FlareOffset));

//    IspTuningMgr::getInstance().setAEInfo((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
    IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
    MY_LOG_IF(m_3ALogEnable,"[updateAEInfo2ISP] State:%d Shutter:%d Gain:%d %d %d Flare offset:%d %d FrameDuration:%lld \n", eNewAEState, rAEInfo2ISP.u4Eposuretime, rAEInfo2ISP.u4AfeGain,
        rAEInfo2ISP.u4IspGain, rAEInfo2ISP.u4RealISOValue, rAEInfo2ISP.i2FlareOffset, i2FlareGain, m_rSensorCurrentInfo.u8FrameDuration);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::get3ACaptureDelayFrame()
{
MINT32 i4CaptureDelayFrame = m_i4SensorCaptureDelayFrame;
//MINT8 idx;


    if((m_bEnableAE == MTRUE) && (mCaptureMode.u4Eposuretime > m_rAEInitInput.rAEPARAM.strAEParasetting.u4LongCaptureThres) && (m_bMultiCap == MFALSE) && (m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableLongCaptureCtrl == MTRUE)) {


        AAASensorMgr::getInstance().getSensorModeDelay(m_eSensorMode, &i4CaptureDelayFrame);

        if (i4CaptureDelayFrame < 3){

        i4CaptureDelayFrame = m_i4TotalCaptureDelayFrame;

        } else {

        m_i4TotalCaptureDelayFrame = i4CaptureDelayFrame;
        m_u4VsyncCnt = m_i4TotalCaptureDelayFrame;

        }
    }

    MY_LOG("[%s()] i4SensorDev:%d i4CaptureDelayFrame:%d %d %d Shutter:%d MultiCap:%d Thres:%d Enable:%d\n", __FUNCTION__, m_eSensorDev, i4CaptureDelayFrame, m_i4SensorCaptureDelayFrame,
        m_i4TotalCaptureDelayFrame, mCaptureMode.u4Eposuretime, m_bMultiCap, m_rAEInitInput.rAEPARAM.strAEParasetting.u4LongCaptureThres, m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableLongCaptureCtrl);
    return i4CaptureDelayFrame;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsMultiCapture(MBOOL bMultiCap)
{
    MY_LOG("[%s()] i4SensorDev:%d bMultiCap:%d\n", __FUNCTION__, m_eSensorDev, bMultiCap);
    m_bMultiCap = bMultiCap;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsAEContinueShot(MBOOL bCShot)
{
    MY_LOG("[%s()] i4SensorDev:%d bCShot:%d\n", __FUNCTION__, m_eSensorDev, bCShot);
    m_bCShot = bCShot;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsHDRShot(MBOOL bHDRShot)
{
   if ((bHDRShot == 1) || (bHDRShot!=m_bHDRshot)){
        MY_LOG("[%s()] i4SensorDev:%d bHDRShot:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bHDRshot ,bHDRShot);
        m_bHDRshot = bHDRShot;
        m_bAEMonitorStable = MFALSE;
        m_u4AEScheduleCnt = 0;
   }
   return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureShutterValue()
{
    if((m_bEnableAE == MTRUE) && (m_bMultiCap == MFALSE) && (mCaptureMode.u4Eposuretime > m_rAEInitInput.rAEPARAM.strAEParasetting.u4LongCaptureThres) && (m_rAEInitInput.rAEPARAM.strAEParasetting.bEnableLongCaptureCtrl == MTRUE)) {
        m_u4VsyncCnt--;
        if(m_u4VsyncCnt == 2) {
            setExp(mCaptureMode.u4Eposuretime);
        } else if((m_u4VsyncCnt == 1) && (m_bCShot == MFALSE)) {   // if m_bQuick2Preview is TRUE mean the capture is continue shot
            setExp(30000);
        }
    }
    MY_LOG("[%s()] i4SensorDev:%d m_u4VsyncCnt:%d m_bMultiCap:%d m_bCShot:%d\n", __FUNCTION__, m_eSensorDev, m_u4VsyncCnt, m_bMultiCap, m_bCShot);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::monitorAndReschedule( MBOOL bAAASchedule, MUINT32 u4AvgYcur, MUINT32 u4AvgYStable, MBOOL *bAECalc, MBOOL *bAEApply )
{
    MBOOL bStatChange;
    MBOOL bReschedule;
    MBOOL bAEEXit = MFALSE;
    MINT32 i4framePerAECycle = 3;
    MUINT16 u2AEStatThrs = m_rAEInitInput.rAEPARAM.strAEParasetting.u2AEStatThrs;
    MUINT8 AEScheduleFrame;

    getAESensorActiveCycle(&i4framePerAECycle);

    if(bAAASchedule){
        m_u4AEScheduleCnt = 0;
        *bAECalc  = MFALSE;
        *bAEApply = MFALSE;
        return 0;
    }

    // Detect AE statistic change and Reset and Follow Schedule
    bStatChange = AEStatisticChange(u4AvgYcur, u4AvgYStable, u2AEStatThrs);

    bReschedule = (m_bAEMonitorStable && (bStatChange || m_bFaceAEAreaChage || m_bTouchAEAreaChage));

    /*if(bReschedule){
        m_u4AEScheduleCnt = 0;
    } else {
        if (m_bAEStable){ // m_bAEStable==1, bStatChange==0
            bAEEXit = MTRUE;
        }
    }
    */


    if (m_bAEMonitorStable){

        if( (bReschedule)&&(m_i4WaitVDNum > (m_i4IspGainDelayFrames+1) ) ){       // (touchAE||Face||statchange) when stable
            m_u4AEScheduleCnt = 0;
        }else if ((bReschedule)&&(m_i4WaitVDNum <= (m_i4IspGainDelayFrames+1) ) ){ // (touchAE||Face||statchange) when stable
            m_u4AEScheduleCnt = m_i4WaitVDNum;
        }else {                    // without (touchAE||Face||statchange) when stable
            bAEEXit = MTRUE;
        }
    }



    // Set AECalc & AEApply
    AEScheduleFrame = (m_u4AEScheduleCnt) % (i4framePerAECycle);

    if(bAEEXit == MTRUE) {
        *bAECalc  = MFALSE;
        *bAEApply = MFALSE;
    } else if ((AEScheduleFrame == 0)) {
        *bAECalc = MTRUE;
        *bAEApply = MTRUE;
    } else {
        *bAECalc = MFALSE;
        *bAEApply = MTRUE;
    }

    MY_LOG_IF(m_3ALogEnable,"[%s()] Calc:%d Apply:%d Cnt:%d Frame:%d ReSchedule:%d Exit:%d bStatChange:%d bAAASchedule:%d frameDelay:%d Stable:%d AEMonitorStable:%d VdCnt:%d ISPdelay:%d FaceArea:%d TouchArea:%d u4AvgYcur:%d u4AvgYpre:%d\n",
              __FUNCTION__, *bAECalc, *bAEApply, m_u4AEScheduleCnt, AEScheduleFrame, bReschedule, bAEEXit, bStatChange, bAAASchedule, i4framePerAECycle,
              m_bAEStable, m_bAEMonitorStable,m_i4WaitVDNum, m_i4IspGainDelayFrames, m_bFaceAEAreaChage, m_bTouchAEAreaChage,u4AvgYcur, u4AvgYStable);

    // Update next AE Schedule Count
    m_u4AEScheduleCnt ++;
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::AEStatisticChange( MUINT32 u4AvgYcur, MUINT32 u4AvgYpre, MUINT32 u4thres)
{
    MBOOL bStatChange=0;

    if ((u4AvgYcur<(u4AvgYpre-u4AvgYpre*u4thres/100))||(u4AvgYcur>(u4AvgYpre+u4AvgYpre*u4thres/100))){
        bStatChange = 1;
//        if(m_bAEStable) {
            //MY_LOG("[%s()] u4AvgYcur:%d u4AvgYpre:%d u4thres:%d\n", __FUNCTION__ , u4AvgYcur, u4AvgYpre, u4thres);
//        }
    }

    return bStatChange;
}
