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
* @file IHal3A.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __IHAL_3A_H__
#define __IHAL_3A_H__

#include <mtkcam/common.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/metadata/IMetadata.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <utils/List.h>
#include <mtkcam/exif/IDbgInfoContainer.h>

class IBaseCamExif;

namespace NS3A
{

using namespace NSCam;
using namespace android;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief Interface of 3A Hal Class
 */
class IHal3A {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    IHal3A(){}
    virtual ~IHal3A();

private: // disable copy constructor and copy assignment operator
    IHal3A(const IHal3A&);
    IHal3A& operator=(const IHal3A&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // interfaces for metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of IMetadata
     */
    virtual MINT32 set(const List<IMetadata>& controls) = 0;

    /**
     * @brief Get dynamic result with specified frame ID via IHal3A
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     */
    virtual MINT32 get(MINT32 frmId, IMetadata& result) = 0;

    /**
     * @brief Get delay frames via IHal3A
     * @param [out] delay_info in terms of metadata with MTK defined tags.
     */
    virtual MINT32 getDelay(IMetadata& delay_info) const = 0;

    /**
     * @brief Get delay frames via IHal3A
     * @param [in] tag belongs to control+dynamic
     * @return
     * - MINT32 delay frame.
     */
    virtual MINT32 getDelay(MUINT32 tag) const = 0;

    virtual const IMetadata* queryStaticInfo() const = 0;

    /**
     * @brief Get capacity of metadata list via IHal3A
     * @return
     * - MINT32 value of capacity.
     */
    virtual MINT32 getCapacity() const = 0;


public:
    enum E_VER
    {
        E_Camera_1 = 0,
        E_Camera_3 = 1
    };
    //
    /**
     * @brief Create instance of IHal3A
     * @param [in] eVersion, E_Camera_1, E_Camera_3
     * @param [in] i4SensorIdx
     */
    static IHal3A* createInstance(E_VER eVersion, MINT32 const i4SensorIdx, const char* strUser);
    /**
     * @brief destroy instance of IHal3A
     */
    virtual MVOID destroyInstance(const char* strUser) {}
    /**
     * @brief send commands to 3A hal
     * @param [in] eCmd 3A commands; please refer to ECmd_T
     */
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0) {return MTRUE;}
    /**
     * @brief Set magic number for frame sync
     * @param [in] i4FrmId magic number
     */
    virtual MVOID setFrameId(MINT32 i4FrmId) = 0;
    /**
     * @brief Get magic number
     * @return
     * - MINT32 magic number value.
     */
    virtual MINT32 getFrameId() const = 0;
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
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const = 0;
    /**
     * @brief set debug information of MTK photo debug parsor
     * @param [in] pIBaseCamExif pointer of Exif base object; please refer to IBaseCamExif in IBaseCamExif.h
     * @param [in] fgReadFromHW flag indicate if reading ISP setting from HW is required
     */
    virtual MBOOL setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const = 0;
    /**
     * @brief provide number of delay frames required by 3A mechanism
     * @param [in] eQueryType query type; please refer to EQueryType_T
     */
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const {return 0;}
    /**
     * @brief add callbacks for 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 addCallbacks(I3ACallBack* cb) {return 0;}
    /**
     * @brief remove callbacks in 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 removeCallbacks(I3ACallBack* cb) {return 0;}
    /**
     * @brief set ISP tuning profile
     * @param [in] IspProfile ISP profile; please refer to EIspProfile
     * @param [in] i4MagicNum magic number
     * @param [in] fgEnableRPG TURE: enable RPG (RAW Pre-gain before RRZ); FALSE: disable RPG
     * @param [in] fgNeedValidate TURE: need to validate ISP tuning parameter; FALSE: no need to validate ISP tuning parameter
     */
    virtual MBOOL setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr = NULL) {return MTRUE;}
    /**
     * @brief get capture AE information
     * @param [out] strHDRInputSetting AE information
     */
    virtual MINT32 getExposureInfo(ExpSettingParam_T &strHDRInputSetting) {return 0;}
    /**
     * @brief get capture AE parameters information
     * @param [out] a_rCaptureInfo AE information structure; please refer to Ae_param.h
     */
    virtual MINT32 getCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    /**
     * @brief update capture AE parameters
     * @param [in] a_rCaptureInfo capture AE parameters information
     */
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    /**
     * @brief get real time AE parameters information
     * @param [out] a_strFrameOutputInfo previiew AE information;
     */
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo) {return 0;}
    /**
      * @brief to return whether need to fire flash
      */
    virtual MINT32 isNeedFiringFlash(MBOOL bEnCal = MFALSE) {return 0;}
    /**
     * @brief get ASD info
     * @param [out] a_rASDInfo ASD info;
     */
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo) {return MTRUE;}
    /**
      * @brief modify Pline table limitation
      * @param [in] bEnable enable or disable this function;
      * @param [in] bEquivalent equivalent with original value;
      * @param [in] u4IncreaseISO_x100 increase the ISO value (100 = 1x);
      * @param [in] u4IncreaseShutter_x100 increase the shutter value (100 = 1x);
      */
    virtual MINT32 modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100) {return 0;}
    /**
     * @brief get LCE info from AE
     * @param [out] a_rLCEInfo LCE info;
     */
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo) {return MTRUE;}
    /**
     * @brief enable AE limiter control
     */
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter) {return 0;}
    /**
     * @brief set AE face detection area and weight information
     * @param [in] a_sFaces face detection information; please refer to Faces.h
     */
    virtual MBOOL setFDInfo(MVOID* a_sFaces) {return MTRUE;}
    /**
     * @brief set AE OT detection area and weight information
     * @param [in] a_sOT OT detection information; please refer to Faces.h
     */
    virtual MBOOL setOTInfo(MVOID* a_sOT) {return MTRUE;}

    /**
     * @brief set face detection on/off flag
     * @param [in] bEnable
     */
    virtual MVOID setFDEnable(MBOOL bEnable) {}

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     */
    virtual MVOID setSensorMode(MINT32 i4SensorMode) {}

    /**
     * @brief set 3A preview mode
     * @param [in] PvMode, refer to E3APreviewMode_T
     */
    virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode) {}

    /**
     * @brief set AE target mode
     * @param [in] AeTargetMode, refer to eAETARGETMODE in ae_param.h
     */
    virtual MINT32 SetAETargetMode(MUINT32 AeTargetMode){ return 0;}
    /**
     * @brief enter capture process, called before first ECmd_CaptureStart (single capture or multi capture)
     */
    virtual MVOID enterCaptureProcess() {}
    /**
     * @brief exit capture process, called after last ECmd_CaptureEnd (single capture or multi capture)
     */
    virtual MVOID exitCaptureProcess() {}
    /**
     * @brief Middleware query number (M) of frames per cycle for slow motion
     */
    virtual MUINT32 queryFramesPerCycle(MUINT32 fps) { return 1; }

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) {return 0;}

    virtual MVOID setAELock(MBOOL  bIsAELock){};

    virtual MVOID setAWBLock(MBOOL  bIsAELock){};

    virtual MINT32 enableFlashQuickCalibration(MINT32 bEn) {return 0;}
    virtual MINT32 getFlashQuickCalibrationResult() {return 0;}

};

}; // namespace NS3A

#endif
