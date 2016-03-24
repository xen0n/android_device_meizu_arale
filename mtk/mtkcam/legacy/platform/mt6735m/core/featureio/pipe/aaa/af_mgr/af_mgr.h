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
 * @file af_mgr.h
 * @brief AF manager, do focusing for raw sensor.
 */
#ifndef _AF_MGR_H_
#define _AF_MGR_H_
#include <isp_tuning.h>
#include <af_feature.h>
class NvramDrvBase;
using namespace android;
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

namespace NS3A
{

//--- pre-tuned scenechange parameters start ---
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

/* new part ; format chgT|chgN|stbT|stbN */
// thres unit: scale 100
const static MINT32 SCENE_GYRO_LEVEL[3]               = { 40051012,  40031010,  20031007};
const static MINT32 SCENE_GYRO_VID_LEVEL[3]           = { 40071015,  40051013,  20051010};// video mode cnt > normal mode cnt for stable
const static MINT32 SCENE_ACCE_LEVEL[3]               = { 80155012,  80125010,  60125007};
const static MINT32 SCENE_ACCE_VID_LEVEL[3]           = { 80155015,  80125013,  60125010};// video mode cnt > normal mode cnt for stable
// thres unit: % ; chgT and chgN from org part
const static MINT32 SCENE_AEBLOCK_CHG_THR_DIFF        = 10;    // to add based on GS chgT
const static MINT32 SCENE_AEBLOCK_CHG_CNT_DIFF        = 5;    // to add based on GS chgN
const static MINT32 SCENE_GS_LEVEL[3]                = {      512,       510,       507};
const static MINT32 SCENE_GS_VID_LEVEL[3]            = {      515,       513,       510};// video mode cnt > normal mode cnt for stable
const static MINT32 SCENE_AEBLOCK_LEVEL[3]           = {     1012,      1010,      1007};
const static MINT32 SCENE_AEBLOCK_VID_LEVEL[3]       = {     1015,      1013,      1010};// video mode cnt > normal mode cnt for stable

/* org part */
const static MINT32 SCENE_FV_CHG_THR_LEVEL_DIFF[3]    = { 0,  0, -10};    // chgT lower to sensitive
const static MINT32 SCENE_FV_CHG_CNT_LEVEL_DIFF[3]    = { 3,  0,   0};    // chgN bigger to stable
const static MINT32 SCENE_FV_STB_THR_LEVEL_DIFF[3]    = { 0,  0,   0};    // stbT should fix
const static MINT32 SCENE_FV_STB_CNT_LEVEL_DIFF[3]    = { 5,  0,  -5};    // stbN bigger to stable
const static MINT32 SCENE_GS_CHG_THR_LEVEL_DIFF[3]    = { 0,  0,  -5};    // chgT lower to sensitive
const static MINT32 SCENE_GS_CHG_CNT_LEVEL_DIFF[3]    = { 3,  0,   0};    // chgN bigger to stable
//--- pre-tuned scenechange parameters end ---

class IAfAlgo;
class IPdAlgo;
#define MINTPTR MINT32

/**
 * @brief AF manager class
 */
class AfMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AfMgr(AfMgr const&);
    //  Copy-assignment operator is disallowed.
    AfMgr& operator=(AfMgr const&);

public:  ////
    AfMgr(ESensorDev_T eSensorDev);
    ~AfMgr();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief Get AF manager instance.
     */
    static AfMgr& getInstance(MINT32 const i4SensorDev);
    static AfMgr* s_pAfMgr; // FIXME: REMOVED LATTER
    /**
     * @brief Initial AF manager functions.
     */
    MRESULT init(MINT32 i4SensorIdx,MINT32 isInitMCU);
    MBOOL sendAFNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    /**
     * @brief Uninitial AF manager functions.
     */
    MRESULT uninit(MINT32 isInitMCU);

    /**
     * @brief get AF Ref Win when af.
     * @param [in] AF win.
     */
    MVOID getAFRefWin(CameraArea_T &rWinSize);

    /**
     * @brief set AE to AF Info when ae statistic result done.
     * @param [in] statistic result.
     */
    MVOID setAE2AFInfo(AE2AFInfo_T rAEInfo);

    /**
     * @brief Do AF task when af statistic result done.
     * @param [in] pAFStatBuf Buffer of statistic result.
     */
    MRESULT doAF(MVOID *pAFStatBuf);
    MVOID doPDBuffer(MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride);
    MINT32 PDPureRawInterval();
    MRESULT doPDTask();

    /**
     * @brief Trigger do focusing.
     */
    MRESULT triggerAF();

    /**
     * @brief Set AF mode.
     * @param [in] a_eAFMode Set AF mode for single/continous/Fullscan/MF; Please refer AF_MODE_T in kd_camera_feature_enum.h
     */
    //EAfState_T getAFState();
    MRESULT setAFMode(MINT32 a_eAFMode);

    /**
     * @brief Set AF area
     * @param [in] a_sAFArea AF window information.
     */
    MRESULT setAFArea(CameraFocusArea_T a_sAFArea);

    /**
     * @brief Set camera mode
     * @param [in] a_eCamMode Camera modes; please refer EAppMode in CamDefs.h
     */
    MRESULT setCamMode(MINT32 a_eCamMode);

      /**
     * @brief Set camera zoom information.
     * @param [in] u4XOffset Zoom window X offset
     * @param [in] u4YOffset Zoom window Y offset
     * @param [in] u4Width  Zoom window Width;
     * @param [in] u4Height Zoom window Height;
     */
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);

    /**
     * @brief Set face detecion result information.
     * @param [in] a_sFaces Face detection result; Please refer MtkCameraFaceMetadata in faces.h
     */
    MRESULT setFDInfo(MVOID* a_sFaces);

    /**
     * @brief Set object tracking result information.
     * @param [in] a_sObtinfo object tracking result;
     */
    MRESULT setOTInfo(MVOID* a_sObtinfo);

    /**
     * @brief Get AF maximum area number
     * @return AF maximum area number
     */
    MINT32  getAFMaxAreaNum();

    /**
     * @brief Get maximum lens position value, it gets AF MCU internal position value.
     * @return AF maximum lens Position value, the macro boundary position value in MCU.
     */
    MINT32  getMaxLensPos();

    /**
     * @brief Get minimum. lens position value, it gets AF MCU internal position value.
     * @return AF minimum lens position value, the infinite boundary position value in MCU.
     */
    MINT32  getMinLensPos();

    /**
     * @brief Get AF best position value
     * @return Latest time AF searching result position value.
     */
    MINT32  getAFBestPos();

    /**
     * @brief Get AF current position value.
     * @return AF current position value.
     */
    MINT32  getAFPos();

    /**
     * @brief Check if AF is stable.
     * @return AF stable value; if 1, AF is stable.; if 0, AF is not stable.
     */
    MINT32  getAFStable();

    /**
     * @brief Get AF table Offset.
     * @return AF table offset value; It usually means the infinite position value.;Please refer i4Offset in lens_para_<MCU_Name>.cpp
    */
    MINT32  getAFTableOffset();

    /**
     * @brief Get AF table macro index.
     * @return AF table macro index; An index for AF table, it denotes the macro position of AF table.;Please refer i4MacroNum in lens_para_<MCU_Name>.cpp
     */
    MINT32  getAFTableMacroIdx();

    /**
     * @brief Get AF table index number.
     * @return AF table index number;It means that usable AF table length, and it's define to be 30.;Please refer AF_TABLE_NUM in camera_custom_nvram.h
     */
    MINT32  getAFTableIdxNum();

    /**
     * @brief Get AF Table.
     * @return AF table pointer;AF table is contains 30 entries. Please refer lens_para_<MCU_Name>.cpp
     */
    MVOID*  getAFTable();

    /**
     * @brief Get AF full statistic result. This result is generated for auto-flicker used, and it does not work while AF.
     * @return AF full statistic result; Please refer AF_FULL_STAT_T in af_param.h
     */
    AF_FULL_STAT_T getFLKStat();

    /**
     * @brief Enable AF function.
     * @param [in] a_i4En Enable switch. Value 1 means enable, 0 disable.
     */
    MRESULT enableAF(MINT32 a_i4En);

    /**
     * @brief check if AF is enable function.
     * @param [out] result. Value TRUE means available.
     */
    MBOOL isAFavailable();

    /**
    * @brief Set manual focus position. When AF mode is MF, use it to set lens position.
     * @param [in] a_i4Pos Lens position. Usually value in 0~1023.
    */
    MRESULT setMFPos(MINT32 a_i4Pos);

    /**
     * @brief Set full scan step. When full scan, this value sets the step interval between each scanning. i.e.  Current position 100, next 103, thus step is 3.
     * @param [in] a_i4Step Steps for scan interval.
     */
    MRESULT setFullScanstep(MINT32 a_i4Step);

    /**
     * @brief Check if AF is finished
     * @return AF finish status;  Value 1 means finished, 0 not finished.
     */
    MINT32  isFocusFinish();

    /**
     * @brief Check if AF is focused success. This check can not be done before AF finished.
     * @return AF focused status;  Value 1 means success, 0 fail.
     */
    MINT32  isFocused();

    /**
     * @brief Check the AF mode of last focusing. This check can not be done before AF finished.
     * @return last focusing mode;  Value 1 means TAF, 0 CAF.
     */
    MINT32  isLastFocusModeTAF();

    /**
     * @brief Set AF statistic input image size. It checks ISP to get sensor input W/H, and set parameters for AF window reference. AF sets window according to this W/H.
     */
    MVOID setAF_IN_HSIZE();

    /**
     * @brief Set auto-flicker window configure. Auto-flicker needs AF statistic result, and this function set the statistic window for it.
     */
    MVOID setFlkWinConfig();

    /**
     * @brief Get debug information. For internal debug information.
     * @param [in] rAFDebugInfo debug information data pointer.;Please refer AF_DEBUG_INFO_T in dbg_af_param.h
     */
    MRESULT getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo);

    /**
     * @brief Set callback pointer to AF manger. When AF is doing or done, AF manger must report AF status. This pointer receives the reporting way.
     * @param [in] cb Callback function pointer.
     */
    MBOOL   setCallbacks(I3ACallBack* cb);

    /**
     * @brief Single AF Callback Notify. Reporting AF status when single AF done or AF timeout.
     */
    MRESULT SingleAF_CallbackNotify();

    /**
     * @brief Set best shot configure. When continous shot, camera get AF statistic result for each picture. This function configures the floating window for it.
     */
    MRESULT setBestShotConfig();

    /**
     * @brief Calculate best shot value. When continous shot, camera must do this function to calculate AF statistic result.
     * @param [in] pAFStatBuf Buffer of statistic result.
     */
    MRESULT calBestShotValue(MVOID *pAFStatBuf);

    /**
     * @brief Get best shot value. After calculating best shot AF statistic result, use it to get value.
     * @return Best shot AF statistic result.
     */
    MINT64  getBestShotValue();

    /**
     * @brief Set SGG Pre-gain. For AE control gain used.
     * @param [in] i4SGG_PGAIN SGG Pre-gain.
     */
    MVOID setSGGPGN(MINT32 i4SGG_PGAIN);

    /**
     * @brief Print AF ConfigLog, part 1. this print log only in engining version.
     */
    MVOID   printAFConfigLog0();

    /**
     * @brief Print AF ConfigLog, part 2. this print log only in engining version.
     */
    MVOID   printAFConfigLog1();

    /**
     * @brief Auto focus function, tell AF manger it is auto focus status now. This function doesn't trigger AF.
     */
    MVOID   autoFocus();

    /**
     * @brief Do callback for single AF trigger, to report AF status.
     */
    MVOID   DoCallback();

    /**
     * @brief Cancel auto focus, tell AF manger it is not auto focus status now. This function doesn't trigger AF.
     */
    MVOID   cancelAutoFocus();

    /**
     * @brief Timeout handle, when AF statistic is timeout or no response, this timeout function will be called and do something.
     */
    MVOID   TimeOutHandle();
    /**
     * @brief get the information if android is runing. If android not run, the service depends on it should
     */
    MVOID   setAndroidServiceState(MBOOL a_state);

    // Depth AF API
    MINT32 getDAFtbl(MVOID ** ptbl);
    MVOID  setCurFrmNum(MUINT32 frm_num);
    MINT32 queryAFStatusByMagicNum(MUINT32 i4MagicNum);
    //Camera 3.0
    MVOID   setAperture(MINT32 lens_aperture);
    MINT32  getAperture();
    MVOID   setFocalLength(MINT32 lens_focalLength);
    MINT32  getFocalLength();
    MVOID   setFocusDistance(MINT32 lens_focusDistance);
    MINT32  getFocusDistance();
    MVOID   setOpticalStabilizationMode (MINT32 ois_OnOff);
    MINT32  getOpticalStabilizationMode();
    MINT32  getFocusRange();
    MINT32  getLensState();
    MVOID   setSharpnessMapMode(MINT32 SharpMapOnOff);
    MINT32  getSharpnessMapMode();
    MINT32  getMaxSharpnessMapValue();
    MINT32  getSharpnessMapSize();
    MVOID getSharpnessMap(MVOID* a_sAFFullStat);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Set AF initial configure settings. This function apply AF initial settings to HW.
     * @param [in] a_sAFConfig Containing AF initial settings. Please refer AF_CONFIG_T in af_param.h.
     */
    MVOID setAFConfig(AF_CONFIG_T a_sAFConfig);

    /**
     * @brief Set AF window threshold. This function apply AF statistic filter settings to HW.
     * @param [in] a_sAFConfig Containing AF statistic filter settings. Please refer AF_CONFIG_T in af_param.h.
     */
    MVOID setAFWinTH(AF_CONFIG_T a_sAFConfig);

    /**
     * @brief Apply GMR settings to HW.
     * @param [in] a_sAFConfig Containing GMR settings. Please refer AF_CONFIG_T in af_param.h.
     */
    MVOID setGMR(AF_CONFIG_T a_sAFConfig);

    /**
     * @brief Apply AF window settings to HW. Final window setting will be check and apply to HW here.
     * @param [in] a_sAFArea AF window configure settings. Please refer AF_CONFIG_T in af_param.h.
     */
    MVOID setAFWinConfig(AF_AREA_T a_sAFArea);

    /**
     * @brief Apply zoom info settings to HW. Final window setting will be check and apply to HW here.
     * @param [in] None.
     */
    MVOID applyZoomInfo();

    /**
     * @brief This function deal with pAFStatBuf when doing continuous focus, and extract and calculate for usable statistic result.
     * @param [in] pAFStatBuf Buffer of statistic result.
     * @return AF statistic result. Please refer AF_STAT_T in af_param.h.
     */
    AF_STAT_T Trans4WintoOneStat(MVOID *pAFStatBuf);

    /**
     * @brief This function deal with pAFStatBuf when doing single focus, and extract and calculate for usable statistic result.
     * @param [in] pAFStatBuf Buffer of statistic result.
     * @return AF statistic result. Please refer AF_STAT_T in af_param.h.
     */
    AF_STAT_T TransAFtoOneStat(MVOID *pAFStatBuf);

    /**
     * @brief This function deal with pAFStatBuf for auto-flicker, and extract and calculate for usable statistic result.
     * @param [in] pAFStatBuf Buffer of statistic result.
     * @return AF statistic result. Please refer AF_FULL_STAT_T in af_param.h.
     */
    AF_FULL_STAT_T TransToFullStat(MVOID *pAFStatBuf);

    /**
     * @brief This function is used in AF manger internal, for check & limit variable boundary.
     * @param [in] a_i4Min Value minimum value.
     * @param [in] a_i4Vlu The variable for check and limit.
     * @param [in] a_i4Max Value maximum value.
     * @return limited result by a_i4Min & a_i4Max.
     */
    MINT32 Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max);

    /**
     * @brief This function is used to get lens information in MCU driver.
     * @param [in] a_rLensInfo MCU lens information data pointer. Please refer LENS_INFO_T in af_param.h.
     */
    MRESULT getLensInfo(LENS_INFO_T &a_rLensInfo);

    /**
     * @brief This function is used for AF factory calibration. It executes:  1.Read sensor One Time Programmable(OTP) memory. 2. Calculate and apply the factory data to AF table.
     */
    MRESULT readOTP();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    CCT feature
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MRESULT CCTMCUNameinit(MINT32 i4SensorIdx);
    MRESULT CCTMCUNameuninit();
    MINT32 CCTOPAFOpeartion();
    MINT32 CCTOPMFOpeartion(MINT32 a_i4MFpos);
    MINT32 CCTOPAFGetAFInfo(MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetBestPos(MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFCaliOperation(MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFSetFocusRange(MVOID *a_pFocusRange);
    MINT32 CCTOPAFGetFocusRange(MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFApplyNVRAMParam(MVOID *a_pAFNVRAM);
    MINT32 CCTOPAFSaveNVRAMParam();
    MINT32 CCTOPAFGetFV(MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFEnable();
    MINT32 CCTOPAFDisable();
    MINT32 CCTOPAFGetEnableInfo(MVOID *a_pEnableAF, MUINT32 *a_pOutLen);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile int  m_Users;
    volatile int  m_CCTUsers;
    mutable Mutex m_Lock;
    SensorHal*  m_pSensorHal;
    MCUDrv*     m_pMcuDrv;
    IspDrv*     m_pIspDrv;
    IspDrv*     m_pVirtIspDrvCQ0;
    isp_reg_t*  m_pIspReg;
    IAfAlgo*    m_pIAfAlgo;
    MINT32      m_i4CurrSensorDev;
    MINT32      m_i4CurrSensorId;
    MINT32      m_i4CurrLensId;
    AF_INPUT_T    m_sAFInput;
    AF_OUTPUT_T   m_sAFOutput;
    AF_PARAM_T    m_sAFParam;
    AF_CONFIG_T   m_sAFConfig;
    NVRAM_LENS_PARA_STRUCT  m_NVRAM_LENS;
    LIB3A_AF_MODE_T     m_eLIB3A_AFMode;
    CameraFocusArea_T   m_CameraFocusArea;
    AF_AREA_T           m_FDArea;
    MINT32 m_i4AF_in_Hsize;
    MINT32 m_i4AF_in_Vsize;
    MINT32 m_i4EnableAF;
    MINT32 m_OisDisable;
    MINT32 m_bMotorDisable;
    MINT32 m_MotorManual;
    MINT32 m_i4MFPos;
    I3ACallBack*  m_pAFCallBack;
    MINT32 m_i4AFPreStatus;
    MINT32 m_bDebugEnable;
    MINT32 m_i4AF_TH[2];
    MINT32 m_i4GMR[7];
    MINT32 m_i4AutoFocus;
    MINT32 m_i4LastFocusModeTAF;
    MINT32 m_i4AutoFocuscb;
    AF_FULL_STAT_T m_sAFFullStat;
    MINT64 m_i8PreVStat[36];
    MINT32 m_i4Factor;
    AREA_T m_sEZoom;
    MINT32 m_tcaf_mode;
    MINT32      m_i4SensorIdx;
    MINT32 m_flkwin_syncflag;
    MINT32 m_Pix_Id;
    MBOOL m_AndroidServiceState;
    MUINT8 m_aeBlockV[25];
    MINT32 m_i4FLKValid;
    MBOOL  gSetRectFirst;
    AREA_T gFDRect;
    AREA_T m_RawZoomInfo;
    MBOOL  m_NonInitState;
    MINT32 m_eCurrAFMode;
    MINT32 m_bDebugAFwinW;
    MINT32 m_bDebugAFwinH;
    MUINT32 m_AFStatusTbl[16][2];

    // AF v1.2
    // g/gyro sensor listener handler and data
    MBOOL gAcceValid;
    MBOOL gGyroValid;
    MINT32 m_AEsetPGN;

    // DAF
    AREA_T  m_LastAFArea;
    MUINT32 m_cur_frm_num;
    MUINT32 m_prv_frm_num;
    MUINT32 m_bpdctrl_flg;
    MUINT32 m_HybridAFMode;
    MUINT32 m_next_query_FrmNum;
    MUINT32 m_daf_distance;


};

};  //  namespace NS3A
#endif // _AF_MGR_H_

