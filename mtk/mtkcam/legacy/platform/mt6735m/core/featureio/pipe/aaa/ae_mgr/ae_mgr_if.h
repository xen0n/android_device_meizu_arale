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

#ifndef _AE_MGR_IF_H_
#define _AE_MGR_IF_H_

#include <ae_feature.h>
#include <ae_param.h>
#include "camera_custom_hdr.h"
#include "camera_custom_hdr.h"
#include <mtkcam/Modes.h>





/**
 * @brief AE state motion define
 */
typedef enum
{
    AE_INIT_STATE = -1,                       /*!< AE init state */
    AE_AUTO_FRAMERATE_STATE = 0,  /*!< AE dynamic frame rate state */
    AE_MANUAL_FRAMERATE_STATE,    /*!< AE fix frame rate state */
    AE_TWO_SHUTTER_INIT_STATE,    /*!< AE fix frame rate state */
    AE_TWO_SHUTTER_FRAME_STATE,    /*!< AE fix frame rate state */
    AE_AF_STATE,                                  /*!< AE auto focus state */
    AE_PRE_CAPTURE_STATE,                /*!< AE pre-capture state */
    AE_CAPTURE_STATE,                        /*!< AE start capture state */
    AE_POST_CAPTURE_STATE,             /*!< AE post capture state */
    AE_REINIT_STATE,                           /*!< AE re-init state */
    AE_AF_RESTORE_STATE                   /*!< AE AF restore state */
} AE_STATE_T;


/**
 * @brief AE exposure mode define
 */
typedef enum {
    eAE_EXPO_TIME = 0,     /*!< AE exposure by time */
    eAE_EXPO_LINE             /*!< AE exposure by line */
}eAE_EXPO_BASE;

typedef struct {
    strAEPLineInfomation AEPLineInfomation;
    strAEPLineMapping AEPLineMapping[31];
    strAFPlineInfo normalAFPlineInfo;    // normal mode
    strAFPlineInfo ZSDAFPlineInfo;
    strAEPLineGainList AEPLineGainList;
}AE_EASYTUNING_PLINE_INFO_T;

namespace NS3A
{
/*******************************************************************************
*
*******************************************************************************/
/**
 * @brief AE manager interface
 */

class IAeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IAeMgr(IAeMgr const&);
    //  Copy-assignment operator is disallowed.
    IAeMgr& operator=(IAeMgr const&);

public:  ////
    IAeMgr() {}
    ~IAeMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief AE get instance
     */
    static IAeMgr& getInstance();
    /**
     * @brief camera preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_if.h
     */
    MRESULT cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam);
    /**
     * @brief camera video init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_if.h
     */
    MRESULT camcorderPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam);
    /**
     * @brief camera preview re-initial
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT cameraPreviewReinit(MINT32 i4SensorDev);
    /**
     * @brief uninit
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT uninit(MINT32 i4SensorDev);
    /**
     * @brief check if AE lock is supported or not
     * @return always return TRUE
     */
    inline MBOOL isAELockSupported()
    {
        return MTRUE;
    }
    /**
     * @brief backup AE meter information before capture and resotre before return to preview
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] en "1" is backup and "0" is restore
     */
    void setAeMeterAreaEn(MINT32 i4SensorDev, int en);

    /**
     * @brief send the exposure time to sensor driver
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] exp exposure time value (micro-second)
     */
    void setExp(MINT32 i4SensorDev, int exp);
    /**
     * @brief send the sensor gain to sensor driver
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] afe sensor gain value (1x = 1024)
     */
    void setAfe(MINT32 i4SensorDev, int afe);
    /**
     * @brief send the isp gain to ISP register
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] isp gain value (1x = 1024)
     */
    void setIsp(MINT32 i4SensorDev, int isp);
    /**
     * @brief Restore the sensor frame rate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] frm delay frame rate
     */
    void setRestore(MINT32 i4SensorDev, int frm);
    /**
     * @brief set AE meter area window and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] sNewAEMeteringArea meter area; please refer to aaa_hal_if.h
     */
    MRESULT setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea);
    /**
     * @brief set AE EV compensation index and step
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewEVIndex EV index value;
     * @param [in] fStep Step value; The EV compensation value is equal i4NewEVIndex*fStep
     */
    MRESULT setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep);
    /**
     * @brief set AE metering mode for AE control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEMeteringMode metering mode value; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode);
    /**
     * @brief get AE metering mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE metering mode; please refer to Kd_camera_feature_enum.h
     */
    MINT32 getAEMeterMode(MINT32 i4SensorDev);
    /**
     * @brief set AE ISO speed
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewAEISOSpeed ISO value; "0" means "Auto"
     */
    MRESULT setAEISOSpeed(MINT32 i4SensorDev, MUINT32 i4NewAEISOSpeed);
    /**
     * @brief get AE ISO speed
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE ISO value
     */
    MINT32 getAEISOSpeedMode(MINT32 i4SensorDev);
    /**
     * @brief set AE min / max frame rate value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewAEMinFps minimun frame rate value
     * @param [in] i4NewAEMaxFps maximun frame rate value
     */
    MRESULT setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    /**
     * @brief set Anti-banding mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEFLKMode flicker mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode);
    /**
     * @brief set flicker detection result mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEAutoFLKMode flicker detection result; please refer to Ae_feature.h
     */
    MRESULT setAEAutoFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEAutoFLKMode);
    /**
     * @brief set camera mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAECamMode camera mode; please refer to CamDefs.h
     */
    MRESULT setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode);
    /**
     * @brief set capture mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEShotMode capture mode; please refer to CamDefs.h
     */
    MRESULT setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode);
    /**
     * @brief set limiter control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAELimter enable or disable AE limiter
     */
    MRESULT setAELimiterMode(MINT32 i4SensorDev, MBOOL bAELimter);
    /**
     * @brief set Scene Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewScene Scene mode; please refer to Kd_camera_feature_enum.h
     */
    MRESULT setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewAEScene);
    /**
     * @brief get AE scene
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE scene value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEScene(MINT32 i4SensorDev);
    /**
     * @brief set AE Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4NewAEmode AE mode; please refer to camera_custom_AEPlinetable.h
     */
    MRESULT setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEMode);
    /**
     * @brief get AE mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE mode value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEMode(MINT32 i4SensorDev);
    /**
     * @brief get AE state
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE state value; please refer to Mtk_metadata_tag.h
     */
    MINT32 getAEState(MINT32 i4SensorDev);
    /**
     * @brief set Sensor Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4NewSensorMode sensor mode; please refer to camera_custom_AEPlinetable.h
     */
    MRESULT setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode);
    /**
     * @brief update AE scenario Mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eIspProfile isp profile mode; please refer to aaa_hal_common.h
     */
    MBOOL updateAEScenarioMode(MINT32 i4SensorDev, EIspProfile_T eIspProfile);
    /**
     * @brief get Sensor mode
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the sensor mode value; please refer to Isp_tuning.h
     */
    MINT32 getSensorMode(MINT32 i4SensorDev);
    /**
     * @brief AP lock or unlock AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAPAELock lock AE (MTRUE) or unlock AE (MFALSE)
     */
    MRESULT setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock);
    /**
     * @brief AF lock or unlock AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAFAELock lock AE (MTRUE) or unlock AE (MFALSE)
     */
    MRESULT setAFAELock(MINT32 i4SensorDev, MBOOL bAFAELock);
    /**
     * @brief set Zoom window information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] u4XOffset horizontal offset value
     * @param [in] u4YOffset vertical offset value
     * @param [in] u4Width horizontal width value
     * @param [in] u4Height vertical height value
     */
    MRESULT setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    /**
     * @brief enable AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT enableAE(MINT32 i4SensorDev);
    /**
     * @brief disable AE
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT disableAE(MINT32 i4SensorDev);
    /**
     * @brief capture dynamic flare calculate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] bIsStrobe On or OFF strobe
     */
    MRESULT doCapFlare(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe);
    /**
     * @brief Auto focus AE calculate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doAFAEmonitor(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C);
    /**
     * @brief Auto focus AE calculate
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doAFAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief AE calculate the capture parameters
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bIsStrobeFired On or OFF strobe
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doPreCapAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief send the capture paramters to sensor and isp
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT doCapAE(MINT32 i4SensorDev);
    /**
     * @brief Backup the AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT doBackAEInfo(MINT32 i4SensorDev);
    /**
     * @brief Restore the AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     */
    MRESULT doRestoreAEInfo(MINT32 i4SensorDev, MBOOL bRestorePrvOnly);
    /**
     * @brief Calculate the preview AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doPvAEmonitor(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C);
    /**
     * @brief Calculate the preview AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] pAEStatBuf AE statistic buffer pointer
     * @param [in] i4ActiveAEItem select AE algorithm or flare to execute
     * @param [in] u4AAOUpdate AAO buffer update or not
     */
    MRESULT doPvAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    /**
     * @brief get the AE debug parser information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAEDebugInfo debug information; please refer to Dbg_ae_param.h
     * @param [out] rAEPlineDebugInfo debug information; please refer to Dbg_ae_param.h
     */
    MRESULT getDebugInfo(MINT32 i4SensorDev, AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo);
    /**
     * @brief get AE luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE luminance value
     */
    MINT32 getLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn);
    /**
     * @brief get over exposure area AE luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE luminance value
     */
    MINT32 getAOECompLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn);
    /**
     * @brief get AE brightness value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE brightness value
     */
    MINT32 getBVvalue(MINT32 i4SensorDev);
    /**
     * @brief get AE capture luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE capture luminance value
     */
    MINT32 getCaptureLVvalue(MINT32 i4SensorDev);
    /**
     * @brief get AE maximun meter area number
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE support max area value
     */
    MUINT32 getAEMaxMeterAreaNum(MINT32 i4SensorDev);
    /**
     * @brief get AE EV compensation index
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the AE EV index value
     */
    MINT32 getEVCompensateIndex(MINT32 i4SensorDev);
    /**
     * @brief get AE preview, capture and strobe Pline table information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_PrvAEPlineTable current preview AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_CapAEPlineTable current capture AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_StrobeAEPlineTable current strobe AE Pline table pointer; please refer to camera_custom_AEPlinetable.h
     */
    MRESULT getCurrentPlineTable(MINT32 i4SensorDev, strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo);
    /**
     * @brief get Sensor device information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rDeviceInfo sensor information structure; please refer to Camera_custom_nvram.h
     */
    MRESULT getSensorDeviceInfo(MINT32 i4SensorDev, AE_DEVICES_INFO_T &a_rDeviceInfo);
    /**
     * @brief AE need lock before AF or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return AE need lock before AF (MTRUE) or AE lock after AF lock (MFALSE)
     */
    MBOOL IsDoAEInPreAF(MINT32 i4SensorDev);
    /**
     * @brief AE converge stable or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return AE stable (MTRUE) or AE converge continue (MFALSE)
     */
    MBOOL IsAEStable(MINT32 i4SensorDev);
    /**
     * @brief the strobe trigger threshold is bigger than brightness value or not
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return Strobe trigger threshold is bigger than brightness value (MTRUE) or strobe trigger threshold is smaller than brightness value (MFALSE)
     */
    MBOOL IsStrobeBVTrigger(MINT32 i4SensorDev);
    /**
     * @brief get preview AE parameters information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rPreviewInfo AE information structure; please refer to Ae_param.h
    */
    MRESULT getPreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo);
    /**
     * @brief get capture AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] strHDRInputSetting AE information
    */
    MRESULT getExposureInfo(MINT32 i4SensorDev, ExpSettingParam_T &strHDRInputSetting);
    /**
     * @brief get capture AE parameters information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] index capture index information, the value is 0~2
     * @param [in] i4EVidx increase or decrease capture AE parameters information
     * @param [out] a_rCaptureInfo AE information structure; please refer to Ae_param.h
    */
    MRESULT getCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo);
    /**
     * @brief update preview AE parameters
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_rPreviewInfo preview AE parameters information
     * @param [in] i4AEidxNext preview AE next index step
    */
    MRESULT updatePreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext);
    /**
     * @brief update capture AE parameters
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_rCaptureInfo capture AE parameters information
    */
    MRESULT updateCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo);
    /**
     * @brief get AE meter area luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
    */
    MRESULT getAEMeteringYvalue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    /**
     * @brief get AE meter block area luminance value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rWinSize meter area information; please refer to Ae_param.h
     * @param [out] iYvalue luminance value
     * @param [out] u2YCnt total counter value
    */
    MRESULT getAEMeteringBlockAreaValue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt);
    /**
     * @brief get High dynamic range capture information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] strHDROutputInfo capture information; please refer to aaa_hal_if.h
    */
    MRESULT getHDRCapInfo(MINT32 i4SensorDev, Hal3A_HDROutputParam_T & strHDROutputInfo);
    /**
     * @brief get real time AE parameters information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_strFrameInfo previiew AE information; please refer to aaa_hal_if.h
    */
    MRESULT getRTParams(MINT32 i4SensorDev, FrameOutputParam_T &a_strFrameInfo);
    /**
     * @brief set AE face detection area and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     */
    MRESULT setFDenable(MINT32 i4SensorDev, MBOOL bFDenable);
    /**
     * @brief set AE face detection area and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     */
    MRESULT setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces);
    /**
     * @brief set AE object detection area and weight information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] a_sOT object detection information; please refer to Faces.h
     */
    MRESULT setOTInfo(MINT32 i4SensorDev, MVOID* a_sOT);
    /**
     * @brief set strobe on or off infomation
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bIsStrobeOn strobe on (MTRUE) or strobe off (MFALSE)
     */
    MRESULT setStrobeMode(MINT32 i4SensorDev, MBOOL bIsStrobeOn);
    /**
     * @brief set phone rotate degree
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4RotateDegree rotate degree. The value is 0, 90, 180 or 270 only.
     */
    MRESULT setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree);
    /**
     * @brief get AE algorithm condition result
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] i4AECondition AE condition value. please refer to Ae_param.h
     */
    MBOOL getAECondition(MINT32 i4SensorDev, MUINT32 i4AECondition);
    /**
     * @brief get LCE AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] a_rLCEInfo LCE AE information; please refer to aaa_hal_if.h
    */
    MRESULT getLCEPlineInfo(MINT32 i4SensorDev, LCEInfo_T &a_rLCEInfo);
    /**
     * @brief get Face AE information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @return the luminance value change
    */
    MINT16 getAEFaceDiffIndex(MINT32 i4SensorDev);
    /**
     * @brief update the sensor delay information
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] i4SutterDelay sensor shutter delay information
     * @param [in] i4SensorGainDelay sensor gain delay information
     * @param [in] i4IspGainDelay isp gain delay information
    */
    MRESULT updateSensorDelayInfo(MINT32 i4SensorDev, MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    /**
     * @brief get brightness value by frame
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] bFrameUpdate frame update (MTRUE) or no update (MFALSE)
     * @param [out] iYvalue luminance value
    */
    MRESULT getBrightnessValue(MINT32 i4SensorDev, MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    /**
     * @brief get AE NVRAM data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] rAENVRAM AE NVRAM structure
    */
    MINT32 getAENvramData(MINT32 i4SensorDev, AE_NVRAM_T &rAENVRAM);
    /**
     * @brief get NVRAM data
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
    */
    MRESULT getNvramData(MINT32 i4SensorDev);
    /**
     * @brief update sensor and isp parameters
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eNewAEState AE new state
    */
    MRESULT UpdateSensorISPParams(MINT32 i4SensorDev, AE_STATE_T eNewAEState);
    /**
     * @brief get AE block value
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] pYvalues pointer Y value
     * @param [in] size block numbers
    */
    MRESULT getAEBlockYvalues(MINT32 i4SensorDev, MUINT8 *pYvalues, MUINT8 size);
    /**
     * @brief set AE target mode and setting
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] eAETargetMode AE target mode
     * @param [in] strAOEInputParm input params
    */
    MRESULT SetAETargetMode(MINT32 i4SensorDev, eAETARGETMODE eAETargetMode);
    /**
     * @brief get AE sensor active cycle
     * @param [out] i4ActiveCycle pointer cycle value
    */
    MRESULT getAESensorActiveCycle(MINT32 i4SensorDev, MINT32* i4ActiveCycle);
    /**
     * @brief set Video dynamic frame rate
     * @param [in] bVdoDynamicFps enable or disable video dynamic frame rate
    */
    MRESULT setVideoDynamicFrameRate(MINT32 i4SensorDev, MBOOL bVdoDynamicFps);
    /**
     * @brief modify the AE Pline table limitation
     * @param [in] bEnable enable or disable this limitation
     * @param [in] bEquivalent equivalent the orginal table or not
     * @param [in] u4IncreaseISO_x100 increase the ISO ratio 100 mean 1x
     * @param [in] u4IncreaseShutter_x100 increase the shutter ratio 100 mean 1x
    */
    MRESULT modifyAEPlineTableLimitation(MINT32 i4SensorDev, MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100);
    /**
     * @brief provide the SGG1Gain for AF used
     * @param [out] pSGG1Gain output the SGG1 gain value
    */
    MRESULT getAESGG1Gain(MINT32 i4SensorDev, MUINT32 *pSGG1Gain);
    /**
     * @brief enable AE one shot control
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] bAEControl One Shot control (MTRUE) or Normal control (MFALSE)
     */
    MRESULT enableAEOneShotControl(MINT32 i4SensorDev, MBOOL bAEControl);
    /**
     * @brief get AE Pline table
     * @param [in] eTableID input the table ID; please refer to camera_custom_AEPlinetable.h
     * @param [out] a_AEPlineTable output the Pline table pointer
    */
    MRESULT getAEPlineTable(MINT32 i4SensorDev, eAETableID eTableID, strAETable &a_AEPlineTable);
    /**
     * @brief is Need Update sensor by I2C
    */
    MBOOL IsNeedUpdateSensor(MINT32 i4SensorDev);
    /**
     * @brief Update sensor by I2C
    */
    MRESULT updateSensorbyI2C(MINT32 i4SensorDev);
    /**
     * @brief get 3A Capture delay frame
    */
    MINT32 get3ACaptureDelayFrame(MINT32 i4SensorDev);
    /**
     * @brief is multi capture mode
     * @param [in] bMultiCap input the multi capture mode (MTRUE) or not (FALSE)
    */
    MRESULT IsMultiCapture(MINT32 i4SensorDev,MBOOL bMultiCap);
    /**
     * @brief is AE continue shot
     * @param [in] bCShot input the continue capture mode (MTRUE) or not (FALSE)
    */
    MRESULT IsAEContinueShot(MINT32 i4SensorDev, MBOOL bCShot);
    /**
     * @brief Update capture shutter by Vsync update
    */
    MRESULT updateCaptureShutterValue(MINT32 i4SensorDev);
    /**
     * @brief is set sensor directly
     * @param [in] capture parameters
     * @param [in] bDirectlySetting input the setting to sensor directly (MTRUE) or not (FALSE)
    */
    MRESULT setSensorDirectly(MINT32 i4SensorDev, CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting);
    /**
     * @brief is check the brightness change
     * @param [out] output the change over or not
    */
    MBOOL isLVChangeTooMuch(MINT32 i4SensorDev);

    /**
         * @brief is set sensor frame rate when EIS on
         * @param [in] EIS is on or off
     */
    MRESULT setEISon(MINT32 i4SensorDev, MBOOL bISEISon);

    // CCT feature APIs.
    /**
     * @brief Enable AE
    */
    MINT32 CCTOPAEEnable(MINT32 i4SensorDev);
    /**
     * @brief Disable AE
    */
    MINT32 CCTOPAEDisable(MINT32 i4SensorDev);
    /**
     * @brief get AE enable/disable information
     * @param [out] a_pEnableAE pointer for enable/disable information
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    /**
     * @brief set AE scene mode information
     * @param [out] a_AEScene value
    */
    MINT32 CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene);
    /**
     * @brief get AE scene mode information
     * @param [out] a_pAEScene pointer for change scene mode information
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetAEScene(MINT32 i4SensorDev, MINT32 *a_pAEScene, MUINT32 *a_pOutLen);
    /**
     * @brief set AE metering mode information
     * @param [in] a_AEMeteringMode AE metering mode
    */
    MINT32 CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode);
    /**
     * @brief apply AE exposure relative parameters
     * @param [in] a_pAEExpParam pointer to input exposure relative parameters
    */
    MINT32 CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief set AE flicker mode information
     * @param [in] a_AEFlickerMode AE flicker mode value Auto,60Hz,50Hz,OFF
    */
    MINT32 CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode);
    /**
     * @brief get AE exposure relative parameters
     * @param [in] a_pAEExpParamIn pointer don't input anything
     * @param [out] a_pAEExpParamOut pointer to output exposure relative parameters
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    /**
     * @brief get AE flicker mode information
     * @param [out] a_pAEFlickerMode pointer for flicker mode
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetFlickerMode(MINT32 i4SensorDev, MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    /**
     * @brief get AE metering mode information
     * @param [out] a_pAEMEteringMode pointer for output metering mode value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetMeteringMode(MINT32 i4SensorDev, MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    /**
     * @brief apply AE NVRAM paramters
     * @param [in] a_pAENVRAM pointer for NVRAM data
    */
    MINT32 CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM);
    /**
     * @brief get AE NVRAM parameter value
     * @param [in] a_pAENVRAM pointer to output NVRAM data
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    /**
     * @brief save AE NVRAM parameters to NVRAM
    */
    MINT32 CCTOPAESaveNVRAMParam(MINT32 i4SensorDev);
    /**
     * @brief get current EV value
     * @param [out] a_pAECurrentEV pointer to output EV value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetCurrentEV(MINT32 i4SensorDev, MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    /**
     * @brief lock AE exposure setting
    */
    MINT32 CCTOPAELockExpSetting(MINT32 i4SensorDev);
    /**
     * @brief unlock AE exposure setting
    */
    MINT32 CCTOPAEUnLockExpSetting(MINT32 i4SensorDev);
    /**
     * @brief get AE OB value
     * @param [out] a_pIspOB pointer to get OB value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetIspOB(MINT32 i4SensorDev, MUINT32 *a_pIspOB, MUINT32 *a_pOutLen);
    /**
     * @brief set AE OB value
     * @param [in] a_IspOB OB value setting
    */
    MINT32 CCTOPAESetIspOB(MINT32 i4SensorDev, MUINT32 a_IspOB);
    /**
     * @brief get AE ISP gain value
     * @param [out] a_pIspRawGain pointer to output ISP gain value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetIspRAWGain(MINT32 i4SensorDev, MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen);
    /**
     * @brief set AE ISP gain value
     * @param [in] a_IspRAWGain set ISP gain value
    */
    MINT32 CCTOPAESetIspRAWGain(MINT32 i4SensorDev, MUINT32 a_IspRAWGain);
    /**
     * @brief set AE exposure time value
     * @param [in] a_ExpTime set exposure time value
    */
    MINT32 CCTOPAESetSensorExpTime(MINT32 i4SensorDev, MUINT32 a_ExpTime);
    /**
     * @brief set AE exposure line value
     * @param [in] a_ExpLine set exposure line value
    */
    MINT32 CCTOPAESetSensorExpLine(MINT32 i4SensorDev, MUINT32 a_ExpLine);
    /**
     * @brief set AE sensor gain value
     * @param [in] a_SensorGain set sensor gain value
    */
    MINT32 CCTOPAESetSensorGain(MINT32 i4SensorDev, MUINT32 a_SensorGain);
    /**
     * @brief set AE capture mode value
     * @param [in] a_CaptureMode set capture mode
    */
    MINT32 CCTOPAESetCaptureMode(MINT32 i4SensorDev, MUINT32 a_CaptureMode);
    /**
     * @brief set AE capture parameters value
     * @param [in] a_pAEExpParam pointer to set capture parameters
    */
    MINT32 CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief get AE capture parameters value
     * @param [in] a_pAEExpParam pointer to output capture parameters
    */
    MINT32 CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam);
    /**
     * @brief get AE flare offset value
     * @param [in] a_FlareThres flare threshold value
     * @param [out] a_pAEFlareOffset pointer to output flare offset value
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetFlareOffset(MINT32 i4SensorDev, MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen);
    /**
     * @brief set AE target value
     * @param [in] u4AETargetValue set target value for AE reference
    */
    MINT32 CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue);
    /**
     * @brief apply AE Pline NVRAM paramters
     * @param [in] a_pAEPlineNVRAM pointer for AE Pline NVRAM data
    */
    MINT32 CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM);
    /**
     * @brief get AE Pline parameter value
     * @param [in] a_pAEPlineNVRAM pointer to output AE Pline NVRAM data
     * @param [out] a_pOutLen pointer for output size information
    */
    MINT32 CCTOPAEGetPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM,MUINT32 *a_pOutLen);
    /**
     * @brief save AE Pline parameters to NVRAM
    */
    MINT32 CCTOPAESavePlineNVRAM(MINT32 i4SensorDev);
};

};  //  namespace NS3A
#endif // _AE_MGR_IF_H_

