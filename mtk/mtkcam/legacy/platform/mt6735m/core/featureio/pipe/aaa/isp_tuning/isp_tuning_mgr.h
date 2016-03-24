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

#include <ae_param.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>

namespace NSIspTuning
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
    MBOOL init(MINT32 const i4SensorDev/*, MINT32 const i4SensorIdx*/);

    /**
     * @brief uninit function
     * @param [in] i4SensorDev sensor device
     */
    MBOOL uninit(MINT32 const i4SensorDev);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Attributes

    /**
     * @brief set ISP profile
     * @param [in] i4SensorDev sensor device
     * @param [in] i4IspProfile ISP profile
     */
    MBOOL setIspProfile(MINT32 const i4SensorDev, MINT32 const i4IspProfile);

    /**
     * @brief get ISP profile
     * @param [in] i4SensorDev sensor device
     * @return ISP profile
     */
    MINT32 getIspProfile(MINT32 const i4SensorDev) {return m_i4IspProfile;}

    /**
     * @brief set scene mode
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Scene scene mode
     */
    MBOOL setSceneMode(MINT32 const i4SensorDev, MUINT32 const u4Scene);

    /**
     * @brief set effect mode
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Effect effect mode
     */
    MBOOL setEffect(MINT32 const i4SensorDev, MUINT32 const u4Effect);

    /**
     * @brief set operation mode
     * @param [in] i4SensorDev sensor device
     * @param [in] i4OperMode operation mode
     */
    MBOOL setOperMode(MINT32 const i4SensorDev, MINT32 const i4OperMode);

    /**
     * @brief get operation mode
     * @param [in] i4SensorDev sensor device
     * @return operation mode
     */
    MINT32 getOperMode(MINT32 const i4SensorDev);

    /**
     * @brief set dynamic bypass
     * @param [in] i4SensorDev sensor device
     * @param [in] fgEnable TURE: enable dynamic bypass, FALSE: disable dynamic bypass
     */
    MBOOL setDynamicBypass(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief set dynamic CCM
     * @param [in] i4SensorDev sensor device
     * @param [in] bdynamic_ccm TURE: enable dynamic CCM, FALSE: disable dynamic CCM
     */
    MBOOL setDynamicCCM(MINT32 const i4SensorDev, MBOOL bdynamic_ccm);

    /**
     * @brief get dynamic bypass status
     * @param [in] i4SensorDev sensor device
     * @return dynamic bypass status
     */
    MINT32 getDynamicBypass(MINT32 const i4SensorDev);

    /**
     * @brief get dynamic CCM status
     * @param [in] i4SensorDev sensor device
     * @return dynamic CCM status
     */
    MINT32 getDynamicCCM(MINT32 const i4SensorDev);

    /**
     * @brief get current Gamma G table
     * @param [in] i4SensorDev sensor device
     * @param [out] pIspGamma Gamma G array (144 G values)
     * @param [out] pEnable GMM module enable status
     * @return dynamic CCM status
     */
    MBOOL getIspGamma(MINT32 const i4SensorDev, MUINT32* pIspGamma, MBOOL* pEnable);

    /**
     * @brief set sensor mode
     * @param [in] i4SensorDev sensor device
     * @param [in] i4SensorMode sensor mode
     */
    MBOOL setSensorMode(MINT32 const i4SensorDev, MINT32 const i4SensorMode);

    /**
     * @brief get sensor mode
     * @param [in] i4SensorDev sensor device
     * @return sensor mode
     */
    MINT32 getSensorMode(MINT32 const i4SensorDev);

    /**
     * @brief set zoom ratio
     * @param [in] i4SensorDev sensor device
     * @param [in] i4ZoomRatio_x100 zoom ratio
     */
    MBOOL setZoomRatio(MINT32 const i4SensorDev, MINT32 const i4ZoomRatio_x100);

    /**
     * @brief set AWB info
     * @param [in] i4SensorDev sensor device
     * @param [in] rAWBInfo AWB info
     */
    MBOOL setAWBInfo(MINT32 const i4SensorDev, AWB_INFO_T const &rAWBInfo);

    /**
     * @brief set AE info
     * @param [in] i4SensorDev sensor device
     * @param [in] rAEInfo AE info
     */
    MBOOL setAEInfo(MINT32 const i4SensorDev, AE_INFO_T const &rAEInfo);

    /**
     * @brief set AF info
     * @param [in] i4SensorDev sensor device
     * @param [in] rAFInfo AF info
     */
    MBOOL setAFInfo(MINT32 const i4SensorDev, AF_INFO_T const &rAFInfo);

    /**
     * @brief set flash info
     * @param [in] i4SensorDev sensor device
     * @param [in] rFlashInfo flash info
     */
    MBOOL setFlashInfo(MINT32 const i4SensorDev, FLASH_INFO_T const &rFlashInfo);

    /**
     * @brief set flash dynamic info
     * @param [in] i4SensorDev sensor device
     * @param [in] bIsFlashOnOff flash on/off status
     */
    MBOOL setFlashDynamicInfo(MINT32 const i4SensorDev, MINT32 const i4FlashOnOff/*1=on; 0=off*/);

    /**
     * @brief enable dynamic tuning
     * @param [in] i4SensorDev sensor device
     * @param [in] fgEnable TURE: enable dynamic tuning, FALSE: disable dynamic tuning
     */
    MBOOL enableDynamicTuning(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief enable dynamic shading
     * @param [in] fgEnable TURE: enable dynamic shading, FALSE: disable dynamic shading
     */
    MBOOL enableDynamicShading(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief set shading index
     * @param [in] i4SensorDev sensor device
     * @param [in] i4IDX shading index
     */
    MBOOL setIndex_Shading(MINT32 const i4SensorDev, MINT32 const i4IDX);

    /**
     * @brief get shading index
     * @param [in] i4SensorDev sensor device
     * @param [in] pCmdArg pointer to command argument
     */
    MBOOL getIndex_Shading(MINT32 const i4SensorDev, MVOID*const pCmdArg);

    /**
     * @brief set OBC info
     * @param [in] i4SensorDev sensor device
     * @param [in] pOBCInfo pointer to OBC info
     */
    MBOOL setPureOBCInfo(MINT32 const i4SensorDev, const ISP_NVRAM_OBC_T *pOBCInfo);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // ISP End-User-Defined Tuning Index.
    /**
     * @brief set sharpness level
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Edge(MINT32 const i4SensorDev, MUINT32 const u4Index);

    /**
     * @brief set hue level
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Hue(MINT32 const i4SensorDev, MUINT32 const u4Index);

    /**
     * @brief set saturation level
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Sat(MINT32 const i4SensorDev, MUINT32 const u4Index);

    /**
     * @brief set brightness level
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Bright(MINT32 const i4SensorDev, MUINT32 const u4Index);

    /**
     * @brief set contrast level
     * @param [in] i4SensorDev sensor device
     * @param [in] u4Index index
     */
    MBOOL setIspUserIdx_Contrast(MINT32 const i4SensorDev, MUINT32 const u4Index);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Operations.

    /**
     * @brief validate ISP tuning parameter
     * @param [in] i4SensorDev sensor device
     * @param [in] i4FrameID frame ID
     * @param [in] fgForce TRUE: force validation
     */
    MBOOL validate(MINT32 const i4SensorDev, MBOOL const fgForce = MFALSE);

    /**
     * @brief validate frame-based ISP tuning parameter
     * @param [in] i4SensorDev sensor device
     * @param [in] i4FrameID frame ID
     * @param [in] fgForce TRUE: force validation
     */
    MBOOL validatePerFrame(MINT32 const i4SensorDev, MBOOL const fgForce = MFALSE);

    /**
     * @brief get ISP debug info
     * @param [in] i4SensorDev sensor device
     * @param [in] rIspExifDebugInfo ISP debug info
     */
    MBOOL getDebugInfo(MINT32 const i4SensorDev, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo);


    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rIspAWBGain RRZ AWB gain
     */
    MBOOL setAWBGain(MINT32 const i4SensorDev, AWB_GAIN_T& rIspAWBGain);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rIspAEGain RRZ AE gain
     */
    MBOOL setISPAEGain(MINT32 const i4SensorDev, MBOOL bEnableWorkAround, MUINT32 rIspGain);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rNewIspAEPreGain2 RRZ AE pre gain2
     */
    MBOOL setIspAEPreGain2(MINT32 const i4SensorDev, MINT32 i4SensorIndex, AWB_GAIN_T &rNewIspAEPreGain2);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] i2FlareGain RRZ flare gain
     * @param [in] i2FlareOffset RRZ flare offset
     */

    MBOOL setIspFlareGainOffset(MINT32 const i4SensorDev, MINT16 i2FlareGain, MINT16 i2FlareOffset);

    /**
     * @brief set TG info
     * @param [in] i4SensorDev sensor device
     * @param [in] i4TGInfo TG info
     */
    MBOOL setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo);

    /**
     * @brief force ISP validation
     * @param [in] i4SensorDev sensor device
     */
    MBOOL forceValidate(MINT32 const i4SensorDev);

    /**
     * @brief set camera mode
     * @param [in] i4CamMode camera mode
     */
    MBOOL setCamMode(MINT32 const i4SensorDev, MINT32 const i4CamMode);

    /**
     * @brief get ISP index set
     * @param [in] ISP profile
     * @param [in] sensor mode
     * @param [in] ISO
     * @return index set pointer
     */
    MVOID* getDefaultISPIndex(MINT32 const i4SensorDev, MUINT32 eIspProfile, MUINT32 eSensorMode, MUINT32 eIdx_ISO);

    enum E_ISPTUNING_CTRL
    {
        E_ISPTUNING_CTRL_BEGIN         = 0x0000,
        //basic flow control
        E_ISPTUNING_NOTIFY_START       = 0x0001,
        E_ISPTUNING_NOTIFY_STOP        = 0x0002,

        //GMA
        E_ISPTUNING_SET_GMA_SCENARIO   = 0x1000, //preview/capture/video
        E_ISPTUNING_SET_GMA_AE_DYNAMIC = 0x1001,
        E_ISPTUNING_CTRL_END
    };
    enum E_GMA_SCENARIO
    {
        E_GMA_SCENARIO_PREVIEW,
        E_GMA_SCENARIO_CAPTURE,
        E_GMA_SCENARIO_VIDEO
    };
    struct GMA_AE_DYNAMIC_INFO
    {
        MBOOL bStable;
        GMA_AE_DYNAMIC_INFO()
            : bStable(MFALSE)
        {
        }
    };

    MVOID sendIspTuningIOCtrl(MINT32 const i4SensorDev, E_ISPTUNING_CTRL const ctrl, MINTPTR arg1, MINTPTR arg2);

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

};  //  namespace NSIspTuning
#endif // _ISP_TUNING_MGR_H_

