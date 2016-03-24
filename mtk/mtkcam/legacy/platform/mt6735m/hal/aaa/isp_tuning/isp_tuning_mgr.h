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
 * @file isp_tuning_mgr.h
 * @brief ISP tuning manager
 */

#ifndef _ISP_TUNING_MGR_H_
#define _ISP_TUNING_MGR_H_

#include <Local.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_cam_info.h>
#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>

namespace NSIspTuningv3
{

class IParamctrl;

/*******************************************************************************
*
*******************************************************************************/
class IspTuningMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    IspTuningMgr(IspTuningMgr const&);
    //  Copy-assignment operator is disallowed.
    IspTuningMgr& operator=(IspTuningMgr const&);

public:  ////
    IspTuningMgr();
    ~IspTuningMgr();

    /**
     * @brief get instance
     */
    static IspTuningMgr& getInstance();

    /**
     * @brief init function
     * @param [in] i4SensorDev sensor device
     */
    MBOOL init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx);

    /**
     * @brief uninit function
     */
    MBOOL uninit();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Attributes

    /**
     * @brief set ISP profile
     * @param [in] i4IspProfile ISP profile
     */
    MBOOL setIspProfile(MINT32 const i4IspProfile);

    /**
     * @brief get ISP profile
     * @return ISP profile
     */
    MINT32 getIspProfile() {return m_i4IspProfile;}

    /**
     * @brief set scene mode
     * @param [in] u4Scene scene mode
     */
    MBOOL setSceneMode(MUINT32 const u4Scene);

    /**
     * @brief set effect mode
     * @param [in] u4Effect effect mode
     */
    MBOOL setEffect(MUINT32 const u4Effect);

    /**
     * @brief set operation mode
     * @param [in] i4OperMode operation mode
     */
    MBOOL setOperMode(MINT32 const i4OperMode);

    /**
     * @brief get operation mode
     * @return operation mode
     */
    MINT32 getOperMode();

    /**
     * @brief set dynamic bypass
     * @param [in] fgEnable TURE: enable dynamic bypass, FALSE: disable dynamic bypass
     */
    MBOOL setDynamicBypass(MBOOL const fgEnable);

    /**
     * @brief set dynamic CCM
     * @param [in] bdynamic_ccm TURE: enable dynamic CCM, FALSE: disable dynamic CCM
     */
    MBOOL setDynamicCCM(MBOOL bdynamic_ccm);

    /**
     * @brief get dynamic bypass status
     * @return dynamic bypass status
     */
    MINT32 getDynamicBypass();

    /**
     * @brief get dynamic CCM status
     * @return dynamic CCM status
     */
    MINT32 getDynamicCCM();

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode sensor mode
     */
    MBOOL setSensorMode(MINT32 const i4SensorMode);

    /**
     * @brief get sensor mode
     * @return sensor mode
     */
    MINT32 getSensorMode();

    /**
     * @brief set zoom ratio
     * @param [in] i4ZoomRatio_x100 zoom ratio
     */
    MBOOL setZoomRatio(MINT32 const i4ZoomRatio_x100);

    /**
     * @brief set AWB info
     * @param [in] rAWBInfo AWB info
     */
    MBOOL setAWBInfo(AWB_INFO_T const &rAWBInfo);

    /**
     * @brief set AE info
     * @param [in] rAEInfo AE info
     */
    MBOOL setAEInfo(AE_INFO_T const &rAEInfo);

    /**
     * @brief set AF info
     * @param [in] rAFInfo AF info
     */
    MBOOL setAFInfo(AF_INFO_T const &rAFInfo);

    /**
     * @brief set flash info
     * @param [in] rFlashInfo flash info
     */
    MBOOL setFlashInfo(FLASH_INFO_T const &rFlashInfo);

    /**
     * @brief enable dynamic tuning
     * @param [in] fgEnable TURE: enable dynamic tuning, FALSE: disable dynamic tuning
     */
    MBOOL enableDynamicTuning(MBOOL const fgEnable);

    /**
     * @brief enable dynamic shading
     * @param [in] fgEnable TURE: enable dynamic shading, FALSE: disable dynamic shading
     */
    MBOOL enableDynamicShading(MBOOL const fgEnable);

    /**
     * @brief set shading index
     * @param [in] i4IDX shading index
     */
    MBOOL setIndex_Shading(MINT32 const i4IDX);

    MBOOL getIndex_Shading(MVOID*const pCmdArg);
    MBOOL setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo);

    MBOOL getCamInfo(RAWIspCamInfo& rCamInfo) const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // ISP End-User-Defined Tuning Index.
    /**
     * @brief set sharpness level
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Edge(MUINT32 const u4Index);

    /**
     * @brief set hue level
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Hue(MUINT32 const u4Index);

    /**
     * @brief set saturation level
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Sat(MUINT32 const u4Index);

    /**
     * @brief set brightness level
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Bright(MUINT32 const u4Index);

    /**
     * @brief set contrast level
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Contrast(MUINT32 const u4Index);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Operations.

    /**
     * @brief validate ISP tuning parameter
     * @param [in] i4FrameID frame ID
     * @param [in] fgForce TRUE: force validation
     */
    MBOOL validate(MINT32 const i4FrameID, MBOOL const fgForce = MFALSE);

    /**
     * @brief validate frame-based ISP tuning parameter
     * @param [in] i4FrameID frame ID
     * @param [in] fgForce TRUE: force validation
     */
    MBOOL validatePerFrame(MINT32 const i4FrameID, MBOOL const fgForce = MFALSE);

    /**
     * @brief validate frame-based ISP tuning parameter
     * @param [in] flowType 0 processed raw, 1 for ip pure raw
     * @param [in] rIspCamInfo
     * @param [out] pRegBuf: register buffer
     */
    MBOOL validatePerFrameP2(MINT32 flowType, const RAWIspCamInfo& rIspCamInfo, void* pRegBuf);

    /**
     * @brief get ISP debug info
     * @param [in] rIspExifDebugInfo ISP debug info
     */
    MBOOL getDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rIspAWBGain RRZ AWB gain
     */
    MBOOL setAWBGain(AWB_GAIN_T& rIspAWBGain);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rNewIspAEPreGain2 RRZ AE pre gain2
     */
    MBOOL setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T &rNewIspAEPreGain2);
    /**
     * @brief force ISP validation
     * @param [in] i4SensorDev sensor device
     */
    MBOOL forceValidate();

    /**
     * @brief get color correction transform
     * @param [out] rCCM CCM
     */
    MBOOL getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                      MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                      MFLOAT& M31, MFLOAT& M32, MFLOAT& M33);

    /**
     * @brief set color correction transform
     * @param [in] rCCM CCM
     */
    MBOOL setColorCorrectionTransform(MFLOAT M11, MFLOAT M12, MFLOAT M13,
                                      MFLOAT M21, MFLOAT M22, MFLOAT M23,
                                      MFLOAT M31, MFLOAT M32, MFLOAT M33);

    /**
     * @brief set color correction mode
     * @param [in] i4ColorCorrectionMode color correction mode
     */
    MBOOL setColorCorrectionMode(MINT32 i4ColorCorrectionMode);

    /**
     * @brief set edge mode
     * @param [in] i4EdgeMode edge mode
     */
    MBOOL setEdgeMode(MINT32 i4EdgeMode);

    /**
     * @brief set noise reduction mode
     * @param [in] i4NRMode noise reduction mode
     */
    MBOOL setNoiseReductionMode(MINT32 i4NRMode);

    /**
     * @brief set tone map mode
     * @param [in] i4ToneMapMode tone map mode
     */
    MBOOL setToneMapMode(MINT32 i4ToneMapMode);

    /**
     * @brief get tone mapping curve (R channel)
     * @param [in] p_in_red control point input
     * @param [in] p_out_red control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum);

     /**
     * @brief get tone mapping curve (G channel)
     * @param [in] p_in_green control point input
     * @param [in] p_out_green control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum);

    /**
     * @brief get tone mapping curve (B channel)
     * @param [in] p_in_blue control point input
     * @param [in] p_out_blue control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum);

    /**
     * @brief set tone mapping curve (R channel)
     * @param [in] p_in_red control point input
     * @param [in] p_out_red control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum);

     /**
     * @brief get tone mapping curve (G channel)
     * @param [in] p_in_green control point input
     * @param [in] p_out_green control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum);

    /**
     * @brief get tone mapping curve (B channel)
     * @param [in] p_in_blue control point input
     * @param [in] p_out_blue control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum);

    /**
     * @brief convert tone mapping curve (point pairs) into gamma array [144]
     * @param [in] inPtPairs control point input
     * @param [in] u4NumOfPts number of points
     * @param [out] outGMA array to gamma
     */
    MBOOL convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IParamctrl* m_pParamctrl_Main;
    IParamctrl* m_pParamctrl_Sub;
    IParamctrl* m_pParamctrl_Main2;
    MINT32      m_i4SensorDev;
    MBOOL       m_bDebugEnable;
    MINT32      m_i4IspProfile;
    MINT32      m_i4SensorIdx;
};

};  //  namespace NSIspTuningv3
#endif // _ISP_TUNING_MGR_H_

