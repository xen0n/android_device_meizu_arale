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
/**
 * @file ae_mgr.h
 * @brief AE manager
 */

#ifndef _AE_MGR_H_
#define _AE_MGR_H_

#include <isp_tuning.h>
#include <ae_mgr_if.h>
//#include <ae_algo_if.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_ivhdr.h"

namespace NS3A
{

class IAeAlgo;

/*******************************************************************************
*
*******************************************************************************/
/**
 * @brief AE manager
 */

class AeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AeMgr(AeMgr const&);
    //  Copy-assignment operator is disallowed.
    AeMgr& operator=(AeMgr const&);

public:  ////
    AeMgr(ESensorDev_T eSensorDev);
    ~AeMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AeMgr& getInstance(MINT32 const i4SensorDev);
    static AeMgr* s_pAeMgr; // FIXME: REMOVED LATTER
    MRESULT cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MRESULT camcorderPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MRESULT cameraPreviewReinit();
    MRESULT uninit();
    void setAeMeterAreaEn(int en);
    void setExp(int exp);
    void setAfe(int afe);
    void setIsp(int isp);
    void setPfPara(int exp, int afe, int isp);
    void setRestore(int frm);
    MRESULT setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea);
    MRESULT setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep);
    MRESULT setAEMeteringMode(MUINT32 u4NewAEMeteringMode);
    MINT32 getAEMeterMode() const;
    MRESULT setAEISOSpeed(MUINT32 i4NewAEISOSpeed);
    MINT32 getAEISOSpeedMode() const;
    MRESULT setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    MRESULT setAEFlickerMode(MUINT32 u4NewAEFLKMode);
    MRESULT setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode);
    MRESULT setAECamMode(MUINT32 u4NewAECamMode);
    MRESULT setAEShotMode(MUINT32 u4NewAEShotMode);
    MRESULT setAELimiterMode(MBOOL bAELimter);
    MRESULT setSceneMode(MUINT32 u4NewScene);
    MINT32 getAEScene() const;
    MRESULT setAEMode(MUINT32 u4NewAEmode);
    MINT32 getAEMode() const;
    MINT32 getAEState() const;
    MBOOL setSensorMode(MINT32 i4NewSensorMode);
    MBOOL updateAEScenarioMode(EIspProfile_T eIspProfile);
    MINT32 getSensorMode() const;
    MRESULT setAELock();
    MRESULT setAPAELock(MBOOL bAPAELock);
    MRESULT setAFAELock(MBOOL bAFAELock);
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT enableAE();
    MRESULT disableAE();
    MRESULT doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe);
    MRESULT doAFAEmonitor(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C);
    MRESULT doAFAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT doPreCapAE(MINT32 i4FrameCount, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT doCapAE();
    MRESULT doBackAEInfo();
    MRESULT doRestoreAEInfo(MBOOL bRestorePrvOnly);
    MRESULT doPvAEmonitor(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C);
    MRESULT doPvAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo);
    MINT32 getLVvalue(MBOOL isStrobeOn);
    MINT32 getAOECompLVvalue(MBOOL isStrobeOn);
    MINT32 getBVvalue();
    MINT32 getCaptureLVvalue();
    MUINT32 getAEMaxMeterAreaNum();
    MINT32 getEVCompensateIndex();
    MRESULT getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo);
    MRESULT getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo);
    MBOOL IsDoAEInPreAF();
    MBOOL IsAEStable();
    MBOOL IsStrobeBVTrigger();
    MRESULT getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo);
    MRESULT updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext);
    MRESULT getExposureInfo(ExpSettingParam_T &strHDRInputSetting);
    MRESULT getCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    MRESULT getAEMeteringBlockAreaValue(AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt);
    MRESULT getRTParams(FrameOutputParam_T &a_strFrameInfo);
    MRESULT setFDenable(MBOOL bFDenable);
    MRESULT setFDInfo(MVOID* a_sFaces);
    MRESULT setOTInfo(MVOID* a_sOT);
    MRESULT setStrobeMode(MBOOL bIsStrobeOn);
    MRESULT setAERotateDegree(MINT32 i4RotateDegree);
    MBOOL getAECondition(MUINT32 i4AECondition);
    MRESULT getLCEPlineInfo(LCEInfo_T &a_rLCEInfo);
    MINT16 getAEFaceDiffIndex();
    MRESULT updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    MRESULT getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    MRESULT UpdateSensorISPParams(AE_STATE_T eNewAEState);
    MBOOL getAENvramData(AE_NVRAM_T &rAENVRAM);
    MRESULT getNvramData(MINT32 i4SensorDev, MINT32 isForce = 0);
    MRESULT getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size);
    MRESULT SetAETargetMode(eAETARGETMODE eAETargetMode);
    MRESULT getAESensorActiveCycle(MINT32* i4ActiveCycle);
    MRESULT setVideoDynamicFrameRate(MBOOL bVdoDynamicFps);
    MRESULT modifyAEPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100);
    MRESULT getAESGG1Gain(MUINT32 *pSGG1Gain);
    MRESULT enableAEOneShotControl(MBOOL bAEControl);
    MRESULT getAEPlineTable(eAETableID eTableID, strAETable &a_AEPlineTable);
    MBOOL IsNeedUpdateSensor();
    MRESULT updateSensorbyI2C();
    MINT32 get3ACaptureDelayFrame();
    MRESULT IsMultiCapture(MBOOL bMultiCap);
    MRESULT IsAEContinueShot(MBOOL bCShot);
    MRESULT updateCaptureShutterValue();
    MRESULT setSensorDirectly(CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting);
    MBOOL WillUpdateSensorbyI2C();
    MBOOL isLVChangeTooMuch();
    MRESULT switchCapureDiffEVState(MINT8 iDiffEV, strAEOutput &aeoutput);
    MBOOL SaveAEMgrInfo(const char * fname);

    // CCT feature APIs.
    MINT32 CCTOPAEEnable();
    MINT32 CCTOPAEDisable();
    MINT32 CCTOPAEGetEnableInfo(MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetSceneMode(MINT32 a_AEScene);
    MINT32 CCTOPAEGetAEScene(MINT32 *a_pAEScene, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetMeteringMode(MINT32 a_AEMeteringMode);
    MINT32 CCTOPAEApplyExpParam(MVOID *a_pAEExpParam);
    MINT32 CCTOPAESetFlickerMode(MINT32 a_AEFlickerMode);
    MINT32 CCTOPAEGetExpParam(MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetFlickerMode(MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetMeteringMode(MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEApplyNVRAMParam(MVOID *a_pAENVRAM);
    MINT32 CCTOPAEGetNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESaveNVRAMParam();
    MINT32 CCTOPAEGetCurrentEV(MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    MINT32 CCTOPAELockExpSetting();
    MINT32 CCTOPAEUnLockExpSetting();
    MINT32 CCTOPAEGetIspOB(MUINT32 *a_pIspOB, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspOB(MUINT32 a_IspOB);
    MINT32 CCTOPAEGetIspRAWGain(MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspRAWGain(MUINT32 a_IspRAWGain);
    MINT32 CCTOPAESetSensorExpTime(MUINT32 a_ExpTime);
    MINT32 CCTOPAESetSensorExpLine(MUINT32 a_ExpLine) const;
    MINT32 CCTOPAESetSensorGain(MUINT32 a_SensorGain) const;
    MINT32 CCTOPAESetCaptureMode(MUINT32 a_CaptureMode);
    MINT32 CCTOSetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOGetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOPAEGetFlareOffset(MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen);
    MINT32 CCTOPSetAETargetValue(MUINT32 u4AETargetValue);
    MINT32 CCTOPAEApplyPlineNVRAM(MVOID *a_pAEPlineNVRAM);
    MINT32 CCTOPAEGetPlineNVRAM(MVOID *a_pAEPlineNVRAM,MUINT32 *a_pOutLen);
    MINT32 CCTOPAESavePlineNVRAM();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MRESULT getSensorResolution();

    MRESULT AEInit(Param_T &rParam);
    MRESULT copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo);
    MRESULT prepareCapParams();
    MRESULT PreviewAEInit(MINT32 m_i4SensorIdx, Param_T &rParam);
    MRESULT updatePreviewParamsByiVHDR(strAEOutput *sAEInfo);
    MRESULT ModifyCaptureParamsBySensorMode(MINT32 i4newSensorMode, MINT32 i4oldSensorMode);
    MRESULT startSensorPerFrameControl();
    MRESULT monitorAndReschedule( MBOOL bAAASchedule, MUINT32 u4AvgYcur, MUINT32 u4AvgYStable, MBOOL *bAECalc, MBOOL *bAEApply);
    MBOOL AEStatisticChange( MUINT32 u4AvgYcur, MUINT32 u4AvgYpre, MUINT32 u4thres);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IAeAlgo* m_pIAeAlgo;
    AE_CCT_CFG_T m_AeMgrCCTConfig;
    EZOOM_WINDOW_T m_eZoomWinInfo;
    AEMeteringArea_T m_eAEMeterArea;
    AEMeteringArea_T m_eAEMeterAreaForDPwhileZoom;
    AEMeteringArea_T m_eAEFDArea;
    ESensorDev_T m_eSensorDev;
    ESensorTG_T m_eSensorTG;
    ESensorMode_T m_eSensorMode;
    MINT32      m_i4SensorIdx;
    MINT32 m_BVvalue;
    MINT32 m_AOECompBVvalue;
    MINT32 m_BVvalueWOStrobe;
    MINT32 m_i4EVvalue;
    MINT32 m_i4WaitVDNum;
    MINT32 m_i4RotateDegree;
    MINT32 m_i4TimeOutCnt;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4SensorGainDelayFrames;
    MINT32 m_i4SensorGainDelayFramesWOShutter;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4SensorCaptureDelayFrame;
    MINT32 m_i4TotalCaptureDelayFrame;
    MINT32 m_i4AEidxCurrent;  // current AE idx
    MINT32 m_i4AEidxNext;   // next AE idx
    MINT16 m_i2AEFaceDiffIndex;
    MUINT32 m_u4PreExposureTime;
    MUINT32 m_u4PreSensorGain;
    MUINT32 m_u4PreIspGain;
    MUINT32 m_u4SmoothIspGain;
    MUINT32 m_u4AECondition;
    MUINT32 m_u4DynamicFrameCnt;
    MUINT32 m_u4AFSGG1Gain;

    MBOOL m_bOneShotAEBeforeLock;
    MBOOL m_bAESceneChanged;
    MBOOL m_bAELock;
    MBOOL m_bAPAELock;
    MBOOL m_bAFAELock;
    MBOOL m_bEnableAE;
    MBOOL m_bVideoDynamic;
    MBOOL m_bRealISOSpeed;
    MBOOL m_bAElimitor;
    MBOOL m_bAEMonitorStable;
    MBOOL m_bAEStable;
    MBOOL m_bAEReadyCapture;
    MBOOL m_bLockExposureSetting;
    MBOOL m_bStrobeOn;
    MBOOL m_bAEMgrDebugEnable;
    MBOOL m_bRestoreAE;
    MBOOL m_bAECaptureUpdate;
    MBOOL m_bOtherIPRestoreAE;
    LIB3A_AE_SCENE_T m_eAEScene;     // change AE Pline
    LIB3A_AE_MODE_T m_eAEMode;
    mtk_camera_metadata_enum_android_control_ae_state_t m_ePreAEState;
    mtk_camera_metadata_enum_android_control_ae_state_t m_eAEState;
    MFLOAT  m_fEVCompStep;
    MINT32  m_i4EVIndex;
    LIB3A_AE_METERING_MODE_T    m_eAEMeterMode;
    MUINT32 m_u4AEISOSpeed;   // change AE Pline
    LIB3A_AE_FLICKER_MODE_T    m_eAEFlickerMode;    // change AE Pline
    MINT32    m_i4AEMaxFps;
    MINT32    m_i4AEMinFps;
    LIB3A_AE_FLICKER_AUTO_MODE_T    m_eAEAutoFlickerMode;   // change AE Pline
    NS3A::EAppMode m_eCamMode;
    LIB3A_AECAM_MODE_T m_eAECamMode;
    NS3A::EShotMode m_eShotMode;
    strAETable m_CurrentPreviewTable;
    strAETable m_CurrentCaptureTable;
    strAETable m_CurrentStrobetureTable;
    LIB3A_AE_EVCOMP_T m_eAEEVcomp;
    AE_MODE_CFG_T mPreviewMode;
    AE_MODE_CFG_T mCaptureMode;
    Hal3A_HDROutputParam_T m_strHDROutputInfo;
    AE_STATE_T m_AEState;
    MBOOL m_bIsAutoFlare;
    MBOOL m_bFrameUpdate;
    MBOOL m_bAdbAEEnable;
    MBOOL m_bAdbAELock;
    MBOOL m_bAdbAELog;
    MBOOL m_bAdbAEPreviewUpdate;
    MBOOL m_bAdbAEDisableSmooth;
    MINT32 m_i4AdbAEISPDisable;
    MUINT32 m_u4AdbAEShutterTime;
    MUINT32 m_u4AdbAESensorGain;
    MUINT32 m_u4AdbAEISPGain;
    MBOOL m_bAdbAEPerFrameCtrl;
    MUINT32 m_u4IndexTestCnt;
    MINT32 m_i4ObjectTrackNum;
    MINT32 const* m_pIsAEActive;
    MINT32 m_i4AECycleNum;
    MINT32 m_i4GammaIdx;   // next gamma idx
    MINT32 m_i4LESE_Ratio;    // LE/SE ratio
    MUINT32 m_u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
    MUINT32 m_u4MaxShutter;
    MUINT32 m_u4MaxISO;
    MUINT32 m_u4StableCnt;
    MBOOL m_bCalc;
    MBOOL m_bApply;
    IVHDRExpSettingOutputParam_T m_strIVHDROutputSetting;

    int m_isAeMeterAreaEn;
    AE_PLINETABLE_T* m_pAEPlineTable;
    NVRAM_CAMERA_3A_STRUCT* m_p3ANVRAM;
    eAETARGETMODE m_eAETargetMode;
    CameraMeteringArea_T m_backupMeterArea;
    AE_INITIAL_INPUT_T m_rAEInitInput;
    AE_OUTPUT_T m_rAEOutput;
    AE_STAT_PARAM_T m_rAEStatCfg;
    SENSOR_RES_INFO_T m_rSensorResolution[2]; // [0]: for TG1 (main/sub), [1]: for TG2(main_2)
    AE_PLINE_LIMITATION_T m_rAEPLineLimitation;
    MUINT32 m_u4PrevExposureTime;
    MUINT32 m_u4PrevSensorGain;
    MUINT32 m_u4PrevIspGain;
    LIB3A_AECAM_MODE_T m_ePrevAECamMode;
    MUINT32 m_3ALogEnable;
    MBOOL m_bAEOneShotControl;
    MBOOL bUpdateSensorAWBGain;
    SENSOR_VC_INFO_T m_rSensorVCInfo;
    MBOOL bApplyAEPlineTableByCCT;
    MBOOL m_bSetShutterValue;
    MBOOL m_bSetGainValue;
    MBOOL m_bSetFrameRateValue;
    MUINT32 m_u4UpdateShutterValue;
    MUINT32 m_u4UpdateGainValue;
    MUINT32 m_u4UpdateFrameRate_x10;
    MBOOL m_bTouchAEAreaChage;
    MBOOL m_bMultiCap;
    MBOOL m_bCShot;
    MUINT32 m_u4VsyncCnt;
    MUINT32 m_u4WOFDCnt;
    MBOOL m_bFDenable;
    MBOOL m_bFaceAEAreaChage;
    MUINT32 m_u4AEScheduleCnt;
    MUINT32 m_u4StableYValue;
    MUINT32 m_u4AEExitStableCnt;
    MBOOL m_bWillUpdateSensorbyI2C;
    char* mEngFileName;
    MUINT32 m_u4LEHDRshutter;
    MUINT32 m_u4SEHDRshutter;
};

};  //  namespace NS3A
#endif // _AE_MGR_H_

