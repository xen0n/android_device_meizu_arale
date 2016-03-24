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

#ifndef _AWB_MGR_H_
#define _AWB_MGR_H_


#include <mtkcam/algorithm/lib3a/awb_algo_if.h>
#include <mtkcam/algorithm/libsync3a/MTKSyncAwb.h>
#include <isp_tuning.h>

using namespace NSIspTuning;

namespace NS3A
{

// AWB statistics info
typedef struct
{
    MINT32 i4SensorWidth[ESensorMode_NUM];   // Sensor width
    MINT32 i4SensorHeight[ESensorMode_NUM];  // Sensor height
    MINT32 i4NumX[ESensorMode_NUM];          // AWB window number (Horizontal)
    MINT32 i4NumY[ESensorMode_NUM];          // AWB window number (Vertical)
    MINT32 i4SizeX[ESensorMode_NUM];         // AWB window size (Horizontal)
    MINT32 i4SizeY[ESensorMode_NUM];         // AWB window size (Vertical)
    MINT32 i4PitchX[ESensorMode_NUM];        // AWB window pitch (Horizontal)
    MINT32 i4PitchY[ESensorMode_NUM];        // AWB window pitch (Vertical)
    MINT32 i4OriginX[ESensorMode_NUM];       // AWB window origin (Horizontal)
    MINT32 i4OriginY[ESensorMode_NUM];       // AWB window origin (Vertical)
    //------------------------------------------------------------------------------------
    MBOOL  bNeedCrop[ESensorMode_NUM];           // if need crop
    MINT32 i4CropOffsetX[ESensorMode_NUM];       // crop offset x
    MINT32 i4CropOffsetY[ESensorMode_NUM];       // crop offset y
    MINT32 i4CropRegionWidth[ESensorMode_NUM];   // crop region width
    MINT32 i4CropRegionHeight[ESensorMode_NUM];  // crop region height
} AWB_WINDOW_CONFIG_T;

class AwbStateMgr;

class AwbMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AwbMgr(AwbMgr const&);
    //  Copy-assignment operator is disallowed.
    AwbMgr& operator=(AwbMgr const&);

public:  ////
    AwbMgr(ESensorDev_T eSensorDev);
    ~AwbMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AwbMgr& getInstance(MINT32 const i4SensorDev = ESensorDev_None);
    static AwbMgr* s_pAwbMgr; // FIXME: REMOVED LATTER
    MBOOL cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MBOOL camcorderPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MBOOL cameraCaptureInit();
    MBOOL cameraPreviewReinit();
    MBOOL uninit();
    inline MBOOL isAWBEnable()
    {
        return m_bEnableAWB;
    }


    MBOOL setAWBMode(MINT32 i4NewAWBMode);


    MINT32 getAWBMode() const;
    MBOOL setSensorMode(MINT32 i4NewSensorMode);
    MINT32 getSensorMode() const;

    MBOOL setStrobeMode(MINT32 i4NewStrobeMode);



    MINT32 getStrobeMode() const;
    MBOOL setFlashAWBData(FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData);
    MBOOL setAWBLock(MBOOL bAWBLock);
    MBOOL enableAWB();
    MBOOL disableAWB();

    MBOOL setAWBStatCropRegion(MINT32 i4SensorMode, MINT32 i4CropOffsetX, MINT32 i4CropOffsetY, MINT32 i4CropRegionWidth, MINT32 i4CropRegionHeight);
    MBOOL doPvAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW=1);
    MBOOL doAFAWB(MVOID *pAWBStatBuf, MBOOL bApplyToHW=1);
    MBOOL doPreCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW=1);
    MBOOL doCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW=1);
    MBOOL applyAWB(AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput);
    MBOOL getDebugInfo(AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData);
    MINT32 getAWBCCT();
    MBOOL getASDInfo(AWB_ASD_INFO_T &a_rAWBASDInfo);
    MBOOL getAWBOutput(AWB_OUTPUT_T &a_rAWBOutput);
    MRESULT getAWBParentStat(AWB_PARENT_BLK_STAT_T &a_rAWBParentState,  MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY);







    inline MVOID setAFLV(MINT32 i4AFLV)
    {
         m_i4AFLV = i4AFLV;
    }

    inline MINT32 getAFLV()
    {
         return m_i4AFLV;
    }

    MBOOL getInitInputParam(SYNC_CTC_INIT_PARAM &CCTInitParam, NORMAL_GAIN_INIT_INPUT_PARAM &NoramlGainInitParam);
    MBOOL queryHBinInfo();
    MBOOL setTGInfo(MINT32 const i4TGInfo);
    MBOOL setAWBNvramIdx(MINT32 const i4AWBNvramIdx);
    MBOOL SaveAwbMgrInfo(const char *);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MRESULT CCTOPAWBEnable();
    MRESULT CCTOPAWBDisable();
    MRESULT CCTOPAWBGetEnableInfo(MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetAWBGain(MVOID *a_pAWBGain, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBSetAWBGain(MVOID *a_pAWBGain);
    MRESULT CCTOPAWBApplyNVRAMParam(MVOID *a_pAWBNVRAM);
    MRESULT CCTOPAWBGetNVRAMParam(MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetNVRAMParam(AWB_NVRAM_IDX_T eIdx, MVOID *a_pAWBNVRAM);
    MRESULT CCTOPAWBSaveNVRAMParam();
    MRESULT CCTOPAWBSetAWBMode(MINT32 a_AWBMode);
    MRESULT CCTOPAWBGetAWBMode(MINT32 *a_pAWBMode, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetLightProb(MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBBypassCalibration(MBOOL bBypassCalibration);
    MRESULT CCTOPFlashAWBApplyNVRAMParam(MVOID *a_pFlashAWBNVRAM);
    MRESULT CCTOPFlashAWBGetNVRAMParam(MVOID *a_pFlashAWBNVRAM, MUINT32 *a_pOutLen);
    MRESULT CCTOPFlashAWBSaveNVRAMParam();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL getSensorResolution();
    MBOOL getNvramData();
    MBOOL AWBInit(Param_T &rParam);
    MBOOL AWBWindowConfig();
    MBOOL AWBStatConfig();
    MBOOL AWBRAWPreGain1Config();
    MBOOL getEEPROMData();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IAwbAlgo* m_pIAwbAlgo;
    AwbStateMgr* m_pAwbStateMgr;
    LIB3A_AWB_MODE_T m_eAWBMode;
    MINT32 m_i4StrobeMode;
    MINT32 m_i4AWBNvramIdx;
    MBOOL m_bEnableAWB;
    MBOOL m_bAWBLock;
    MBOOL m_bAdbAWBLock;
    MBOOL m_bOneShotAWB;
    MBOOL m_bAWBModeChanged;
    MBOOL m_bStrobeModeChanged;
    MBOOL m_bColdBoot;
    MBOOL m_bAWBNvramIdxChanged;
    MINT32 m_eSensorDev;
    ESensorTG_T m_eSensorTG;
    ESensorMode_T m_eSensorMode;
    MINT32      m_i4SensorIdx;
    MBOOL m_bDebugEnable;
    MBOOL m_bInitState;
    MINT32 m_i4AFLV;
    MBOOL m_bSkipOneFrame;
    MBOOL m_bHBIN2Enable;
    MBOOL m_bAWBCalibrationBypassed;
    NVRAM_CAMERA_3A_STRUCT* m_pNVRAM_3A;
    MUINT32 m_flashAwbWeight;
    MINT32 m_flashDuty;
    MINT32 m_flashStep;
    AWB_STAT_PARAM_T m_rAWBStatParam;
    AWB_WINDOW_CONFIG_T m_rAWBWindowConfig;
    AWB_GAIN_T m_rAWBRAWPreGain1;
    AWB_INIT_INPUT_T m_rAWBInitInput;
    AWB_OUTPUT_T m_rAWBOutput;
    AWB_STAT_CONFIG_T m_rAWBStatCfg[AWB_STROBE_MODE_NUM][ESensorMode_NUM][LIB3A_AWB_MODE_NUM];
    char* mEngFileName;

};

};  //  namespace NS3A
#endif // _AWB_MGR_H_

