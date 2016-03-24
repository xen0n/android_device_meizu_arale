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
* @file aaa_hal_base.h
* @brief Declarations of 3A Hal Base Class and Top Data Structures
*/
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <stdint.h>

#ifndef _AAA_HAL_BASE_H_
#define _AAA_HAL_BASE_H_

typedef intptr_t                MINTPTR;
typedef uintptr_t               MUINTPTR;

class IBaseCamExif;

namespace NS3A
{

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
    E3ACtrl_SetShadingTSFCapByp     = 0x0008,
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
    //AF
    E3ACtrl_PostPDAFtask            = 0x0400,
    E3ACtrl_QueryAFStatus           = 0x0401,
    //Flow control set
    E3ACtrl_Enable3ASetParams       = 0x1000,
    E3ACtrl_SetOperMode             = 0x1001,
    E3ACtrl_KeepOnLastStatus        = 0x1002,
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
    //  NORMAL
    EIspProfile_NormalPreview              = 0,
    EIspProfile_ZsdPreview_CC,
    EIspProfile_ZsdPreview_NCC,
    EIspProfile_NormalCapture,
    EIspProfile_VideoPreview,
    EIspProfile_VideoCapture,
    EIspProfile_NormalCapture_CC,
    EIspProfile_NormalCapture_16M,
    //  MF
    EIspProfile_MFCapPass1,
    EIspProfile_MFCapPass2,
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
    MUINT32 u4FRameRate_x10;     // 10 base frame rate
    MINT32 i4BrightValue_x10;       // 10 base brightness value
    MINT32 i4ExposureValue_x10;       // 10 base exposure value
    MINT32 i4LightValue_x10;          // 10 base lumince value
    MUINT32 u4AEIndex;

    MUINT32 u4ShutterSpeed_us;    // micro second
    MUINT32 u4SensorGain_x1024;  // 1024 base
    MUINT32 u4ISPGain_x1024;       // 1024 base

    MUINT32 u4PreviewShutterSpeed_us;    // micro secondMUINT32 u4ShutterSpeed_us;    // micro second
    MUINT32 u4PreviewSensorGain_x1024;  // 1024 base
    MUINT32 u4PreviewISPGain_x1024;       // 1024 base
    MUINT32 u4RealISOValue;
    MUINT32 u4CapShutterSpeed_us;    // micro second
    MUINT32 u4CapSensorGain_x1024;  // 1024 base
    MUINT32 u4CapISPGain_x1024;       // 1024 base
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
    MUINT32 u4AfeGain;           //!<: sensor gain
    MUINT32 u4IspGain;           //!<: raw gain
    MUINT32 u4RealISO;           //!<: Real ISO speed
    MUINT32  u4FlareOffset;
    MUINT32  u4FlareGain;   // 512 is 1x
    MINT32   i4LightValue_x10;   // 10 base LV value
    MUINT32  u4YuvShading;      //0: off, 1,on
    MUINT32  u4YuvGamma;        //0: off, 1,on
    MUINT32  u4YuvAE;           //0: off, 1,on
    MUINT32  u4YuvShutter;
    MUINT32  u4YuvGain;
    MUINT32  u4YuvShutterRange;
};

struct FeatureParam_T {
     MBOOL   bExposureLockSupported;
     MBOOL   bAutoWhiteBalanceLockSupported;
     MUINT32 u4MaxFocusAreaNum;
     MINT32  i4MaxLensPos;
     MINT32  i4MinLensPos;
     MINT32  i4AFBestPos;
     MINT64  i8BSSVlu;
     MUINT32 u4MaxMeterAreaNum;
     MUINT32 u4FocusLength_100x;
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
    MVOID  *pAFTable;           // Pointer to AF table
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
    MINT32 i4MinFps;
    MINT32 i4MaxFps;
    MUINT32 u4AfMode;
    MUINT32 u4AeMode;
    MUINT32 u4AwbMode;
    MUINT32 u4EffectMode;
    MUINT32 u4AntiBandingMode;
    MUINT32 u4SceneMode;
    MUINT32 u4StrobeMode;
    MINT32  i4ExpIndex;
    MFLOAT  fExpCompStep;
    MBOOL  bIsAELock;
    MBOOL  bIsAWBLock;

    MINT32  i4FullScanStep;
    MINT32  i4MFPos;
    CameraFocusArea_T       rFocusAreas;
    CameraMeteringArea_T    rMeteringAreas;

    // MTK DEFINITION CATEGORY

    MUINT32 u4IsoSpeedMode;
    MUINT32 u4AfLampMode;
    //
    MUINT32 u4BrightnessMode;
    MUINT32 u4HueMode;
    MUINT32 u4SaturationMode;
    MUINT32 u4EdgeMode;
    MUINT32 u4ContrastMode;
    MINT32  i4RotateDegree;
    MUINT32 u4AeMeterMode;

    // NEWLY-ADDED CATEGORY
    MUINT32      u4CamMode;   //Photo, Video, ZSD, ENG mode
    MUINT32      u4ShotMode;

    //flash for engineer mode
    MINT32  i4PreFlashDuty;
    MINT32  i4PreFlashStep;
    MINT32  i4MainFlashDuty;
    MINT32  i4MainFlashStep;

    MBOOL   bIsSupportAndroidService;
    MBOOL   bVideoStabilization;

    Param_T()
        : i4MinFps(5000)
        , i4MaxFps(30000)
        , u4AfMode(0)
        , u4AwbMode(1)
        , u4EffectMode(0)
        , u4AntiBandingMode(0)
        , u4SceneMode(0)
        , u4StrobeMode(0)
        , i4ExpIndex(0)
        , fExpCompStep(0)
        , bIsAELock(MFALSE)
        , bIsAWBLock(MFALSE)
        , i4FullScanStep(1)
        , i4MFPos(0)
        , rFocusAreas()
        , rMeteringAreas()
        , u4AeMode(0)
        , u4IsoSpeedMode(0)
        , u4AfLampMode(0)
        , u4BrightnessMode(0)
        , u4HueMode(0)
        , u4SaturationMode(0)
        , u4EdgeMode(0)
        , u4ContrastMode(0)
        , i4RotateDegree(0)
        , u4AeMeterMode (0)
        , u4CamMode(0)
        , u4ShotMode(0)
        , i4PreFlashDuty(-1)
        , i4PreFlashStep(-1)
        , i4MainFlashDuty(-1)
        , i4MainFlashStep(-1)
        , bIsSupportAndroidService(MTRUE)
        , bVideoStabilization(0)
    {}
};


/*
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
*/
struct Result_T
{
    MINT32      i4FrmId;
    MINT32      i4PrecaptureId; // android.control.aePrecaptureId
    //EAfState_T  eAfState;
    //EAeState_T  eAeState;
    //EAwbState_T eAwbState;
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
                           int32_t _ext3
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
};

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A Hal Base Class
 */
class Hal3ABase {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3ABase() {}
    virtual ~Hal3ABase() {}

private: // disable copy constructor and copy assignment operator
    Hal3ABase(const Hal3ABase&);
    Hal3ABase& operator=(const Hal3ABase&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
   /**
     * @brief Create instance of Hal3ABase
     * @param [in] i4SensorDevId sensor device; please refer to halSensorDev_e in sensor_hal.h
     */
    static Hal3ABase* createInstance(MINT32 const i4SensorDevId);
   /**
     * @brief destroy instance of Hal3ABase
     */
    virtual MVOID destroyInstance() {}
   /**
     * @brief send commands to 3A hal
     * @param [in] eCmd 3A commands; please refer to ECmd_T
     */
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0) {return MTRUE;}
   /**
     * @brief get 3A error code
     */
    virtual MINT32 getErrorCode() const {return 0;}
   /**
     * @brief get 3A parameters
     * @param [out] rParam 3A parameter struct; please refer to Param_T
     */
    virtual MBOOL getParams(Param_T &rParam) const {return MTRUE;}
   /**
     * @brief set 3A parameters
     * @param [in] rNewParam 3A parameter struct; please refer to Param_T
     */
    virtual MBOOL setParams(Param_T const &rNewParam) {return MTRUE;}
   /**
     * @brief get 3A feature parameters
     * @param [out] rFeatureParam feature parameter struct; please refer to FeatureParam_T
     */
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam) {return MTRUE;}
   /**
     * @brief to return whether ready to capture or not
     */
    virtual MBOOL isReadyToCapture() const {return MTRUE;}
   /**
     * @brief execute auto focus process
     */
    virtual MBOOL autoFocus() {return MTRUE;}
   /**
     * @brief cancel auto focus process
     */
    virtual MBOOL cancelAutoFocus() {return MTRUE;}
   /**
     * @brief set zoom parameters of auto focus
     */
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height) {return MTRUE;}
   /**
     * @brief set 3A photo EXIF information
     * @param [in] pIBaseCamExif pointer of Exif base object; please refer to IBaseCamExif in IBaseCamExif.h
     */
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const {return MTRUE;}
   /**
     * @brief set debug information of MTK photo debug parsor
     * @param [in] pIBaseCamExif pointer of Exif base object; please refer to IBaseCamExif in IBaseCamExif.h
     */
    virtual MBOOL setDebugInfo(IBaseCamExif *pIBaseCamExif) const {return MTRUE;}
   /**
     * @brief provide number of delay frames required by 3A mechanism
     * @param [in] eQueryType query type; please refer to EQueryType_T
     */
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const {return 0;}
   /**
     * @brief set callbacks of AF Manager
     */
    virtual MBOOL setCallbacks(I3ACallBack* cb) {return MTRUE;}
   /**
     * @brief set ISP tuning profile
     * @param [in] IspProfile ISP profile; please refer to EIspProfile
     */
    virtual MBOOL setIspProfile(EIspProfile_T IspProfile) {return MTRUE;}
   /**
     * @brief get capture AE parameters information
     * @param [in] index capture index information, the value is 0~2
     * @param [in] i4EVidx increase or decrease capture AE parameters information
     * @param [out] a_rCaptureInfo AE information structure; please refer to Ae_param.h
     */
    virtual MINT32 getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo) {return 0;}
   /**
     * @brief update capture AE parameters
     * @param [in] a_rCaptureInfo capture AE parameters information
     */
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}

    virtual MINT32 getExposureInfo(ExpSettingParam_T &strHDRInputSetting) {return 0;}
    /**
     * @brief get High dynamic range capture information
     * @param [out] a_strHDROutputInfo capture information;
    */
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo) {return 0;}
    /**
     * @brief set AE face detection area and weight information
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     */
    virtual MVOID setFDEnable(MBOOL bEnable) {}
    virtual MBOOL setFDInfo(MVOID* a_sFaces) {return MTRUE;}
    virtual MBOOL setOTInfo(MVOID* a_sOT) {return MTRUE;}
    /**
     * @brief get real time AE parameters information
     * @param [out] a_strFrameOutputInfo previiew AE information;
    */
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo) {return 0;}
    /**
      * @brief to return whether need to fire flash
      */
    virtual MINT32 isNeedFiringFlash() {return 0;}
    /**
     * @brief get ASD info
     * @param [out] a_rASDInfo ASD info;
     */
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo) {return MTRUE;}
    /**
     * @brief get LCE info from AE
     * @param [out] a_rLCEInfo LCE info;
     */
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo) {return MTRUE;}
    /**
     * @brief End continuous shot, EX: turn off flash device
     */
    virtual MVOID endContinuousShotJobs() {}
    /**
     * @brief enable AE limiter control
     */
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter) {return 0;}
    virtual MINT32 getFlashFrameNumBeforeCapFrame() {return 1;}
    virtual MVOID onFireCapFlashIfNeeded() {}
    virtual MVOID setSensorMode(MINT32 i4SensorMode) {}
    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) {return 0;}
    virtual MBOOL setAELock(MBOOL  bIsAELock){return MTRUE;};
    virtual MBOOL setAWBLock(MBOOL  bIsAELock){return MTRUE;};
#if 0
    virtual MBOOL getFocusData (vector<T.B.D> &rData) const = 0;
    // FD
    // EXIF (T.B.D)

#endif
};

#include <sys/time.h>
#include <cutils/log.h>
class aaaTimer {
public:
    inline MINT32 getUsTime()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    aaaTimer(const char* info, MINT32 sensorDevId, MBOOL enable)
        : mInfo(info), mIdx(sensorDevId), mEnable(enable)
    {
        if (mEnable) mStartTime = getUsTime();
    }

    MVOID start(const char* info, MINT32 sensorDevId, MBOOL enable) //used by global/static variables
    {
        mInfo = info;
        mIdx = sensorDevId;
        mEnable = enable;
        if (mEnable) mStartTime = getUsTime();
    }

    MVOID printTime()
    {
        if (!mEnable) return;
        MINT32 endTime = getUsTime();
        ALOGD("[Function: %s, SensorDevId: %d] =====> time(ms): %f\n", mInfo, mIdx, ((double)(endTime - mStartTime)) / 1000);
    }

    ~aaaTimer()
    {
    }

protected:
    const char* mInfo;
    MINT32 mStartTime;
    MINT32 mIdx;
    MBOOL mEnable;
};


}; // namespace NS3A

#endif

