/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

/**
* @file aaa_hal_common.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __AAA_HAL_COMMON_H__
#define __AAA_HAL_COMMON_H__

namespace NS3A
{

typedef enum
{
    ESensorDevId_Main         = 0x01,
    ESensorDevId_Sub          = 0x02,
    ESensorDevId_MainSecond   = 0x04,
    ESensorDevId_Main3D       = 0x05,
    ESensorDevId_Atv          = 0x08,
}   ESensorDevId_T;

enum ESensorType_T {
    ESensorType_RAW = 0,
    ESensorType_YUV = 1
};
/**
 * @brief 3A commands
 */
enum ECmd_T {
     ECmd_CameraPreviewStart,
     ECmd_CameraPreviewEnd,
     ECmd_CamcorderPreviewStart,
     ECmd_CamcorderPreviewEnd,
     ECmd_PrecaptureStart,
     ECmd_PrecaptureEnd,
     ECmd_CaptureStart,
     ECmd_CaptureEnd,
     ECmd_RecordingStart,
     ECmd_RecordingEnd,
     ECmd_Update,
     // internal command
     ECmd_Init,
     ECmd_Uninit,
     ECmd_AFUpdate, // sync with AF done
     ECmd_AFStart,
     ECmd_AFEnd

};

enum EQueryType_T{
    EQueryType_Init,
    EQueryType_Effect,
    EQueryType_AWB,
    EQueryType_AF,
    EQueryType_Ev,
    EQueryType_Sat,
    EQueryType_Bright,
    EQueryType_Contrast
};
/**
 * @brief ISP tuning profile
 */
enum EIspProfile_T
{
    // Camera1.0/Camera3.0
    EIspProfile_Preview = 0,          // Preview
    EIspProfile_Video,                // Video
    EIspProfile_Capture,              // Capture
    EIspProfile_ZSD_Capture,          // ZSD Capture
    EIspProfile_VSS_Capture,          // VSS Capture
    // Camera1.0
    EIspProfile_PureRAW_Capture,      // Pure RAW Capture
    // N3D
    EIspProfile_N3D_Preview,          // N3D Preview
    EIspProfile_N3D_Video,            // N3D Video
    EIspProfile_N3D_Capture,          // N3D Capture
    // MFB
    EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
    EIspProfile_MFB_Capture_EE_Off_SWNR, // MFB capture with SW NR: EE off
    EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
    EIspProfile_MFB_Blending_All_Off_SWNR, // MFB blending with SW NR: all off
    EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
    EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
    EIspProfile_MFB_PostProc_ANR_EE_SWNR,  // MFB post process with SW NR: capture + ANR + EE
    EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
    EIspProfile_MFB_PostProc_Mixing_SWNR,  // MFB post process with SW NR: mixing + all off
    // vFB
    EIspProfile_VFB_PostProc,        // VFB post process: all off + ANR + CCR + PCA
    // iHDR
    EIspProfile_IHDR_Preview,        // IHDR preview
    EIspProfile_IHDR_Video,          // IHDR video
    // Multi-pass ANR
    EIspProfile_Capture_MultiPass_ANR_1,     // Capture multi pass ANR 1
    EIspProfile_Capture_MultiPass_ANR_2,     // Capture multi pass ANR 2
    EIspProfile_VSS_Capture_MultiPass_ANR_1, // VSS Capture multi Pass ANR 1
    EIspProfile_VSS_Capture_MultiPass_ANR_2, // VSS Capture multi Pass ANR 2
    EIspProfile_MFB_MultiPass_ANR_1, // MFB multi Pass ANR 1
    EIspProfile_MFB_MultiPass_ANR_2, // MFB multi Pass ANR 2
    EIspProfile_Capture_SWNR, // Capture with SW NR
    EIspProfile_VSS_Capture_SWNR, // VSS capture with SW NR
    EIspProfile_PureRAW_Capture_SWNR, //Pure RAW capture with SW NR
    // mHDR
    EIspProfile_MHDR_Preview,         // MHDR preview
    EIspProfile_MHDR_Video,           // MHDR video
    EIspProfile_MHDR_Capture, // TODO: reserve dedicated NVRAM
    // VSS_MFB (ZSD+MFLL)
    EIspProfile_VSS_MFB_Capture_EE_Off,   // MFB capture: EE off
    EIspProfile_VSS_MFB_Capture_EE_Off_SWNR, // MFB capture with SW NR: EE off
    EIspProfile_VSS_MFB_Blending_All_Off, // MFB blending: all off
    EIspProfile_VSS_MFB_Blending_All_Off_SWNR, // MFB blending with SW NR: all off
    EIspProfile_VSS_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
    EIspProfile_VSS_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
    EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR,  // MFB post process with SW NR: capture + ANR + EE
    EIspProfile_VSS_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
    EIspProfile_VSS_MFB_PostProc_Mixing_SWNR,  // MFB post process with SW NR: mixing + all off
    EIspProfile_VSS_MFB_MultiPass_ANR_1, // MFB multi Pass ANR 1
    EIspProfile_VSS_MFB_MultiPass_ANR_2, // MFB multi Pass ANR 2

    EIspProfile_NUM
};

enum E3ACaptureMode_T
{
    ECapMode_P2_Cal     = (1<<0),
    ECapMode_P2_Set     = (1<<1)
};

enum E3APreviewMode_T
{
    EPv_Normal    = 0,
    EPv_Video
};

enum ECaptureType_T
{
    ECapType_SingleCapture = 0,
    ECapType_MultiCapture
};

struct FrameOutputParam_T
{
    MUINT32 u4AEIndex;
    MUINT32 u4FRameRate_x10;     // 10 base frame rate
    MUINT32 u4PreviewShutterSpeed_us;    // micro second
    MUINT32 u4PreviewSensorGain_x1024;  // 1024 base
    MUINT32 u4PreviewISPGain_x1024;       // 1024 base
    MUINT32 u4RealISOValue;
    MUINT32 u4CapShutterSpeed_us;    // micro second
    MUINT32 u4CapSensorGain_x1024;  // 1024 base
    MUINT32 u4CapISPGain_x1024;       // 1024 base
    MINT32 i4BrightValue_x10;       // 10 base brightness value
    MINT32 i4ExposureValue_x10;       // 10 base exposure value
    MINT32 i4LightValue_x10;       // 10 base lumince value
    MINT16 i2FlareOffset;              // 12 bit domain
    MINT16 i2FlareGain;                // 9 base gain
};

struct Hal3A_HDROutputParam_T
{
    MUINT32 u4OutputFrameNum;     // Output frame number (2 or 3)
    MUINT32 u4FinalGainDiff[2];   // 1x=1024; [0]: Between short exposure and 0EV; [1]: Between 0EV and long exposure
    MUINT32 u4TargetTone; //Decide the curve to decide target tone
};
/**
 * @brief 3A parameters for capture
 */
struct CaptureParam_T
{
    MUINT32 u4ExposureMode;  //0: exp. time, 1: exp. line
    MUINT32 u4Eposuretime;   //!<: Exposure time in us
    MUINT32 u4AfeGain;       //!<: sensor gain
    MUINT32 u4IspGain;       //!<: raw gain
    MUINT32 u4RealISO;       //!<: Real ISO speed
    MUINT32 u4FlareOffset;
    MUINT32 u4FlareGain;     // 512 is 1x
    MINT32  i4LightValue_x10;// 10 base LV value
    MUINT32 u43ACapMode;     //refer to enum E3ACaptureMode_T
    MINT32  i4YuvEvIdx;
    CaptureParam_T()
        : u43ACapMode(ECapMode_P2_Cal|ECapMode_P2_Set)
        , i4YuvEvIdx(0)
    {}
};

struct FeatureParam_T {
    MBOOL   bExposureLockSupported;
    MBOOL   bAutoWhiteBalanceLockSupported;
    MUINT32 u4MaxFocusAreaNum;
    MUINT32 u4MaxMeterAreaNum;
    MBOOL   bEnableDynamicFrameRate;
    MINT32 i4MaxLensPos;
    MINT32 i4MinLensPos;
    MINT32 i4AFBestPos;
    MUINT32 u4FocusLength_100x;
    MINT32 u4PureRawInterval;
    MUINT32 u4MiniISOGain;
};

struct CameraArea_T {
    MINT32 i4Left;
    MINT32 i4Top;
    MINT32 i4Right;
    MINT32 i4Bottom;
    MINT32 i4Weight;
};

struct ExpSettingParam_T {
    MINT32 u4AOEMode;
    MUINT32 u4MaxSensorAnalogGain; // 1x=1024
    MUINT32 u4MaxAEExpTimeInUS;    // unit: us
    MUINT32 u4MinAEExpTimeInUS;    // unit: us
    MUINT32 u4ShutterLineTime;     // unit: 1/1000 us
    MUINT32 u4MaxAESensorGain;     // 1x=1024
    MUINT32 u4MinAESensorGain;     // 1x=1024
    MUINT32 u4ExpTimeInUS0EV;      // unit: us
    MUINT32 u4SensorGain0EV;       // 1x=1024
    MUINT8  u1FlareOffset0EV;
    MINT32     i4GainBase0EV;            // AOE application for LE calculation
    MINT32     i4LE_LowAvg;            // AOE application for LE calculation, def: 0 ~ 39 avg
    MINT32     i4SEDeltaEVx100;        // AOE application for SE calculation
    MUINT32 u4Histogram[128];
};

#define MAX_FOCUS_AREAS  9

struct CameraFocusArea_T {
    CameraArea_T rAreas[MAX_FOCUS_AREAS];
    MUINT32 u4Count;
};

#define MAX_METERING_AREAS 9

struct CameraMeteringArea_T {
    CameraArea_T rAreas[MAX_METERING_AREAS];
    MUINT32 u4Count;
};

// 3A ASD info
struct ASDInfo_T {
    MINT32 i4AELv_x10;          // AE Lv
    MBOOL  bAEBacklit;          // AE backlit condition
    MBOOL  bAEStable;           // AE stable
    MINT16 i2AEFaceDiffIndex;   // Face AE difference index with central weighting
    MINT32 i4AWBRgain_X128;     // AWB Rgain
    MINT32 i4AWBBgain_X128;     // AWB Bgain
    MINT32 i4AWBRgain_D65_X128; // AWB Rgain (D65; golden sample)
    MINT32 i4AWBBgain_D65_X128; // AWB Bgain (D65; golden sample)
    MINT32 i4AWBRgain_CWF_X128; // AWB Rgain (CWF; golden sample)
    MINT32 i4AWBBgain_CWF_X128; // AWB Bgain (CWF; golden sample)
    MBOOL  bAWBStable;          // AWB stable
    MINT32 i4AFPos;             // AF position
    MVOID* pAFTable;            // Pointer to AF table
    MINT32 i4AFTableOffset;     // AF table offset
    MINT32 i4AFTableMacroIdx;   // AF table macro index
    MINT32 i4AFTableIdxNum;     // AF table total index number
    MBOOL  bAFStable;           // AF stable
};

// LCE Info
struct LCEInfo_T {
    MINT32 i4NormalAEidx;    // gain >= 4x AE Pline table index at 30fps
    MINT32 i4LowlightAEidx;  // gain max AE Pline table index at 30fps
    MINT32 i4AEidxCur;          // AE current frame Pline table index
    MINT32 i4AEidxNext;        // AE next frame Pline table index
};

// Prepare the exposure information of pre-capture and return to Hal3A
struct ExpInfo_T
{
    //MINT32 i4Magic;
    MINT32 i4ISO;
    MINT32 i4ExpTime;
    MINT32 i4AfeGain;
    MINT32 i4IspGain;
    MINT32 i4Duty;
    MINT32 i4Step;
    MINT32 i4IsFlash;
    MINT32 i4AWB_R;
    MINT32 i4AWB_G;
    MINT32 i4AWB_B;
};

typedef enum
{
    E_AE_PRECAPTURE_IDLE,
    E_AE_PRECAPTURE_START
} EAePreCapture_T;


/**
 * @brief 3A parameters
 */
// 3A parameters
struct Param_T {
    // DEFAULT DEFINITION CATEGORY ( ordered by SDK )
    MINT32  i4MinFps;
    MINT32  i4MaxFps;
    MUINT32 u4AfMode;
    MUINT32 u4AeMode;
    MUINT32 u4AwbMode;
    MUINT32 u4EffectMode;
    MUINT32 u4AntiBandingMode;
    MUINT32 u4SceneMode;
    MUINT32 u4StrobeMode;
    MINT32  i4ExpIndex;
    MFLOAT  fExpCompStep;

    MINT32  i4FullScanStep;
    MINT32  i4MFPos;

    // NEWLY-ADDED CATEGORY
    MUINT32 u4CamMode;   //Factory, ENG, normal
    MUINT32 u4ShotMode;
    MUINT32 u4CapType;  //refer to ECaptureType_T

    // MTK DEFINITION CATEGORY
    MUINT32 u4IsoSpeedMode;
    MUINT32 u4BrightnessMode;
    MUINT32 u4HueMode;
    MUINT32 u4SaturationMode;
    MUINT32 u4EdgeMode;
    MUINT32 u4ContrastMode;
    MUINT32 u4AeMeterMode;
    MINT32  i4RotateDegree;

    //EAePreCapture_T eAePreCapture;
    //MBOOL   bHistogramMode;
    MBOOL   bIsAELock;
    MBOOL   bIsAWBLock;

    //flash for engineer mode
    MINT32  i4PreFlashDuty;
    MINT32  i4PreFlashStep;
    MINT32  i4MainFlashDuty;
    MINT32  i4MainFlashStep;

    CameraFocusArea_T       rFocusAreas;
    CameraMeteringArea_T    rMeteringAreas;

    MBOOL   bIsSupportAndroidService;
    MBOOL   bRecordingHint;

    Param_T()
        : i4MinFps(5000)
        , i4MaxFps(30000)
        , u4AfMode(0)
        , u4AeMode(1)
        , u4AwbMode(1)
        , u4EffectMode(0)
        , u4AntiBandingMode(3)
        , u4SceneMode(0)
        , u4StrobeMode(0)
        , i4ExpIndex(0)
        , fExpCompStep(5)
        , i4FullScanStep(1)
        , i4MFPos(0)
        , u4CapType(ECapType_SingleCapture)
        , u4IsoSpeedMode(0)
        , u4BrightnessMode(1)
        , u4HueMode(1)
        , u4SaturationMode(1)
        , u4EdgeMode(1)
        , u4ContrastMode(1)
        , u4AeMeterMode (0)
        , i4RotateDegree(0)
        , bIsAELock(MFALSE)
        , bIsAWBLock(MFALSE)
        , i4PreFlashDuty(-1)
        , i4PreFlashStep(-1)
        , i4MainFlashDuty(-1)
        , i4MainFlashStep(-1)
        , rFocusAreas()
        , rMeteringAreas()
        , bIsSupportAndroidService(MTRUE)
        , bRecordingHint(MFALSE)
    {}
};
#if 0
typedef enum
{
    E_AF_INACTIVE,
    E_AF_PASSIVE_SCAN,
    E_AF_PASSIVE_FOCUSED,
    E_AF_ACTIVE_SCAN,
    E_AF_FOCUSED_LOCKED,
    E_AF_NOT_FOCUSED_LOCKED,
    E_AF_PASSIVE_UNFOCUSED,
} EAfState_T;

typedef enum
{
    E_AE_INACTIVE,
    E_AE_SEARCHING,
    E_AE_CONVERGED,
    E_AE_LOCKED,
    E_AE_FLASH_REQUIRED,
    E_AE_PRECAPTURE
} EAeState_T;

typedef enum
{
    E_AWB_INACTIVE,
    E_AWB_SEARCHING,
    E_AWB_CONVERGED,
    E_AWB_LOCKED
} EAwbState_T;
#endif
// max frames to queue DAF information
#define DAF_TBL_QLEN 32
#define DIST_TBL_QLEN 16
typedef struct
{
    MUINT32 frm_mun;
    MUINT8  is_learning;
    MUINT8  is_querying;
    MUINT8  af_valid;
    MUINT16 af_dac_index;
    MUINT16 af_confidence;
    MUINT16 af_win_start_x;
    MUINT16 af_win_start_y;
    MUINT16 af_win_end_x;
    MUINT16 af_win_end_y;
    MUINT16 daf_dac_index;
    MUINT16 daf_confidence;
    MUINT16 daf_distance;
} DAF_VEC_STRUCT;

typedef struct
{
    MUINT8   is_daf_run;
    MUINT32  is_query_happen;
    MUINT32  curr_p1_frm_num;
    MUINT32  curr_p2_frm_num;
    MUINT16  af_dac_min;
    MUINT16  af_dac_max;
    MUINT16  af_dac_start;
    MUINT32 dac[DIST_TBL_QLEN];
    MUINT32 dist[DIST_TBL_QLEN];

    DAF_VEC_STRUCT daf_vec[DAF_TBL_QLEN];
}DAF_TBL_STRUCT;

struct Result_T
{
    MINT32      i4FrmId;
    MINT32      i4PrecaptureId; // android.control.aePrecaptureId
    //EAfState_T  eAfState;
    //EAeState_T  eAeState;
    //EAwbState_T eAwbState;
};

struct ParamIspProfile_T
{
    EIspProfile_T eIspProfile;
    MINT32        i4MagicNum;
    MINT32        iEnableRPG;
    MINT32        iValidateOpt;

    enum
    {
        EParamValidate_None     = 0,
        EParamValidate_All      = 1,
        EParamValidate_P2Only   = 2
    };

    ParamIspProfile_T()
        : eIspProfile(EIspProfile_Preview)
        , i4MagicNum(0)
        , iEnableRPG(0)
        , iValidateOpt(1)
    {}

    ParamIspProfile_T(
        EIspProfile_T eIspProfile_,
        MINT32        i4MagicNum_,
        MINT32        iEnableRPG_,
        MINT32        iValidateOpt_)
        : eIspProfile(eIspProfile_)
        , i4MagicNum(i4MagicNum_)
        , iEnableRPG(iEnableRPG_)
        , iValidateOpt(iValidateOpt_)
    {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class I3ACallBack {

public:

    virtual             ~I3ACallBack() {}

public:

    virtual void        doNotifyCb (
                           int32_t _msgType,
                           int32_t _ext1,
                           int32_t _ext2,
                           int32_t _ext3,
                           MINTPTR _ext4 = 0
                        ) = 0;

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        ) = 0;
public:

    enum ECallBack_T
    {
        eID_NOTIFY_AF_FOCUSED,
        eID_NOTIFY_AF_MOVING,
        eID_DATA_AF_FOCUSED,
        eID_NOTIFY_3APROC_FINISH,
        eID_NOTIFY_VSYNC_DONE,
        eID_NOTIFY_READY2CAP,
        eID_NOTIFY_STEREO_WARNING,
        eID_NOTIFY_STEREO_DISTANCE,
        eID_NOTIFY_COLLECT_FILE_DONE,
        eID_MSGTYPE_NUM
    };

    enum E3APROC_FINISH_BIT
    {
        e3AProcOK           = 0,
        e3APvInitReady    = 1,
        e3AProcNum
    };
};

enum E3ACtrl_T
{
    E3ACtrl_Begin = 0,
    //shading
    E3ACtrl_SetShadingOnOff         = 0x0001,
    E3ACtrl_SetShadingStrength      = 0x0002,
    E3ACtrl_SetShadingSdblkCfg      = 0x0003,
    E3ACtrl_SetShadingTSFOnOff      = 0x0004,
    E3ACtrl_SetShadingDynamic       = 0x0005,
    E3ACtrl_SetShadingColorTemp     = 0x0006,
    E3ACtrl_SetShadingByp123        = 0x0007,
    //AWB
    E3ACtrl_SetAwbBypCalibration    = 0x0100,
    //AE
    E3ACtrl_SetExposureParam        = 0x0200,
    E3ACtrl_GetExposureParam        = 0x0201,
    E3ACtrl_GetAEPLineTable         = 0x0202,
    E3ACtrl_EnableDisableAE         = 0x0203,
    E3ACtrl_SetIsAEMultiCapture     = 0x0204,
    E3ACtrl_GetAECapDelay           = 0x0205,
    E3ACtrl_GetSensorDelayFrame  = 0x0206,
    E3ACtrl_SetSensorDirectly         = 0x0207,
    E3ACtrl_SetAEContinueShot     = 0x0208,
    E3ACtrl_GetEvCapture            = 0x0209,
    //ISP
    E3ACtrl_GetIspGamma             = 0x0300,
    E3ACtrl_SetIspTuningISO         = 0x0301,
    //AF
    E3ACtrl_PostPDAFtask            = 0x0400,
    E3ACtrl_QueryAFStatus           = 0x0401,
    //Flow control set
    E3ACtrl_Enable3ASetParams       = 0x1000,
    E3ACtrl_SetOperMode             = 0x1001,
    //Flow control get
    E3ACtrl_GetOperMode             = 0x2001,
    //Stereo
    E3ACtrl_GetStereo3DWarning      = 0x8000,
    E3ACtrl_GetDAFTBL               = 0x8001,
    // For engineer collect module info file
    ECmd_Set3ACollectInfoFile       = 0x9000,
    //
    E3ACtrl_Num
};

enum E3ACollectInfo_T
{
    E3ACollectInfo_NONE             = 0x0000,
    E3ACollectInfo_AE               = 0x0001,
    E3ACollectInfo_AF               = 0x0002,
    E3ACollectInfo_AWB              = 0x0003,
    E3ACollectInfo_FLASH            = 0x0004,
    E3ACollectInfo_Num
};

typedef struct
{
    MINT32  i4Type;
    char*    path;
}CollectCmd_T;


}

#endif //__AAA_HAL_COMMON_H__
