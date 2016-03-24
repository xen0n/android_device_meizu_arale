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
* @file Hal3AAdapter1.h
* @brief Declarations of Implementation of 3A Hal Class
*/

#ifndef __HAL_3A_ADAPTER_1_H__
#define __HAL_3A_ADAPTER_1_H__

#include <mtkcam/featureio/IHal3A.h>
#include <mtkcam/featureio/aaa_hal_if.h>

#include <utils/Mutex.h>

namespace NS3A
{
using namespace android;

class Hal3AAdapter1 : public IHal3A
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AAdapter1(MINT32 const i4SensorIdx);
    virtual ~Hal3AAdapter1(){}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // interfaces for metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of IMetadata
     */
    virtual MINT32 set(const List<IMetadata>& controls){return -1;}

    /**
     * @brief Get dynamic result with specified frame ID via Hal3AAdapter1
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     */
    virtual MINT32 get(MINT32 frmId, IMetadata& result){return -1;}

    /**
     * @brief Get delay frames via Hal3AAdapter1
     * @param [out] delay_info in terms of metadata with MTK defined tags.
     */
    virtual MINT32 getDelay(IMetadata& delay_info) const {return -1;}

    virtual MINT32 getDelay(MUINT32 tag) const {return 0;}

    virtual const IMetadata* queryStaticInfo() const {return NULL;}

    /**
     * @brief Get capacity of metadata list via IHal3A
     * @return
     * - MINT32 value of capacity.
     */
    virtual MINT32 getCapacity() const {return 0;}

public:
     //
    /**
      * @brief Create instance of Hal3AAdapter1
      * @param [in] i4SensorIdx.
      */
    static Hal3AAdapter1* createInstance(MINT32 const i4SensorIdx, const char* strUser);
    /**
      * @brief destroy instance of Hal3AAdapter1
      */
    virtual MVOID destroyInstance(const char* strUser);
    /**
     * @brief Set magic number for frame sync
     * @param [in] i4FrmId magic number
     */
    virtual MVOID setFrameId(MINT32 i4FrmId);
    /**
     * @brief Get magic number
     * @return
     * - MINT32 magic number value.
     */
    virtual MINT32 getFrameId() const;
    /**
      * @brief send commands to 3A hal
      * @param [in] eCmd 3A commands; please refer to ECmd_T
      */
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0);
    /**
      * @brief get 3A error code
      */
    virtual MINT32 getErrorCode() const {return 0;}
    /**
      * @brief get 3A parameters
      * @param [out] rParam 3A parameter struct; please refer to Param_T
      */
    virtual MBOOL getParams(Param_T &rParam) const;
    /**
      * @brief set 3A parameters
      * @param [in] rNewParam 3A parameter struct; please refer to Param_T
      */
    virtual MBOOL setParams(Param_T const &rNewParam);
    /**
      * @brief get 3A feature parameters
      * @param [out] rFeatureParam feature parameter struct; please refer to FeatureParam_T
      */
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam);
    /**
      * @brief to return whether ready to capture or not
      */
    virtual MBOOL isReadyToCapture() const;
    /**
      * @brief execute auto focus process
      */
    virtual MBOOL autoFocus();
    /**
      * @brief cancel auto focus process
      */
    virtual MBOOL cancelAutoFocus();
    /**
      * @brief set zoom parameters of auto focus
      */
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    /**
      * @brief set 3A photo EXIF information
      * @param [in] pIBaseCamExif pointer of Exif base object; please refer to IBaseCamExif in IBaseCamExif.h
      */
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;
    /**
      * @brief set debug information of MTK photo debug parsor
      * @param [in] pIBaseCamExif pointer of Exif base object; please refer to IBaseCamExif in IBaseCamExif.h
      */
    virtual MBOOL setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const;
    /**
      * @brief provide number of delay frames required by 3A mechanism
      * @param [in] eQueryType query type; please refer to EQueryType_T
      */
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;
    /**
     * @brief add callbacks for 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 addCallbacks(I3ACallBack* cb);
    /**
     * @brief remove callbacks in 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 removeCallbacks(I3ACallBack* cb);
    /**
      * @brief set ISP tuning profile
      * @param [in] IspProfile ISP profile; please refer to EIspProfile
      * @param [in] i4MagicNum magic number
      * @param [in] fgEnableRPG TURE: enable RPG (RAW Pre-gain before RRZ); FALSE: disable RPG
      * @param [in] fgNeedValidate TURE: need to validate ISP tuning parameter; FALSE: no need to validate ISP tuning parameter
      */
    virtual MBOOL setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr = NULL);
    /**
      * @brief get exposure information
      * @param [out] strHDRInputSetting exposure information (limitation & historgram)
      */
    virtual MINT32 getExposureInfo(ExpSettingParam_T &strHDRInputSetting);
    /**
      * @brief get capture AE parameters information
      * @param [out] a_rCaptureInfo AE information structure; please refer to Ae_param.h
      */
    virtual MINT32 getCaptureParams(CaptureParam_T &a_rCaptureInfo);
    /**
      * @brief update capture AE parameters
      * @param [in] a_rCaptureInfo capture AE parameters information
      */
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);
     /**
      * @brief get High dynamic range capture information
      * @param [out] a_strHDROutputInfo capture information;
     */
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo);
     /**
      * @brief set AE face detection area and weight information
      * @param [in] a_sFaces face detection information; please refer to Faces.h
      */
    virtual MBOOL setFDInfo(MVOID* a_sFaces);
      /**
       * @brief set AE OT detection area and weight information
       * @param [in] a_sOT OT detection information; please refer to OT.h
       */
     virtual MBOOL setOTInfo(MVOID* a_sOT);
     /**
      * @brief get real time AE parameters information
      * @param [out] a_strFrameOutputInfo previiew AE information;
     */
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo);
     /**
       * @brief to return whether need to fire flash
       */
    virtual MINT32 isNeedFiringFlash(MBOOL bEnCal);
     /**
      * @brief get ASD info
      * @param [out] a_rASDInfo ASD info;
      */
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo);
     /**
      * @brief modify Pline table limitation
      * @param [in] bEnable enable or disable this function;
      * @param [in] bEquivalent equivalent with original value;
      * @param [in] u4IncreaseISO_x100 increase the ISO value (100 = 1x);
      * @param [in] u4IncreaseShutter_x100 increase the shutter value (100 = 1x);
      */
     virtual MINT32 modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100);
     /**
      * @brief get LCE info from AE
      * @param [out] a_rLCEInfo LCE info;
      */
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo);
     /**
      * @brief End continuous shot, EX: turn off flash device
      */
    virtual MVOID endContinuousShotJobs();
     /**
      * @brief enable AE limiter control
      */
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter);
    /**
     * @brief set face detection on/off flag
     * @param [in] bEnable
     */
    virtual MVOID setFDEnable(MBOOL bEnable);

    /**
     * @brief set sensor mode
     * @param [in] eSensorMode
     */
    virtual MVOID setSensorMode(MINT32 i4SensorMode);

    /**
     * @brief set 3A preview mode
     * @param [in] PvMode, refer to E3APreviewMode_T
     */
    virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode);

    /**
     * @brief set AE target mode
     * @param [in] AeTargetMode, refer to eAETARGETMODE in ae_param.h
     */
    virtual MINT32 SetAETargetMode(MUINT32 AeTargetMode);
    /**
     * @brief enter capture process, called before first ECmd_CaptureStart (single capture or multi capture)
     */
    virtual MVOID enterCaptureProcess();
    /**
     * @brief exit capture process, called after last ECmd_CaptureEnd (single capture or multi capture)
     */
    virtual MVOID exitCaptureProcess();
    /**
     * @brief Middleware query number (M) of frames per cycle for slow motion
     */
    virtual MUINT32 queryFramesPerCycle(MUINT32 fps);

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    MINT32 enableFlashQuickCalibration(MINT32 bEn);
    MINT32 getFlashQuickCalibrationResult();

    virtual MVOID setAELock(MBOOL  bIsAELock);

    virtual MVOID setAWBLock(MBOOL  bIsAWBLock);

#if 0
    virtual MBOOL getFocusData (vector<T.B.D> &rData) const = 0;
    // FD
    // EXIF (T.B.D)

#endif

protected:
    MBOOL init(const char* strUser);
    MBOOL uninit(const char* strUser);

    Hal3AIf*                mpHal3aObj;
    MINT32                  mi4FrmId;
    MINT32                  mi4SensorIdx;
    MINT32                  mi4User;
    Mutex                   mLock;
    Param_T                 mParam;
    Param_T                 mParamCvt;
};

};

#endif //__HAL_3A_ADAPTER_3_H__

