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

#ifndef _AAA_HAL_IF_H_
#define _AAA_HAL_IF_H_

#include <mtkcam/common.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/exif/IDbgInfoContainer.h>

#include <utils/threads.h>
#include <utils/List.h>

using namespace NSCam;
using namespace android;

class IBaseCamExif;

namespace NS3A
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class Hal3AIf {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AIf() {}
    virtual ~Hal3AIf() {}

private: // disable copy constructor and copy assignment operator
    Hal3AIf(const Hal3AIf&);
    Hal3AIf& operator=(const Hal3AIf&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3AIf* createInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID destroyInstance() {}
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0) {return MTRUE;}
    virtual MINT32 getErrorCode() const {return 0;}
    virtual MBOOL getParams(Param_T &rParam) const {return MTRUE;}
    virtual MBOOL setParams(Param_T const &rNewParam) {return MTRUE;}
    virtual MINT32 getResult(MUINT32 u4FrmId, Result_T& rResult) const {return 0;}
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam) {return MTRUE;}
    virtual MBOOL isReadyToCapture() const {return MTRUE;}
    virtual MBOOL autoFocus() {return MTRUE;}
    virtual MBOOL cancelAutoFocus() {return MTRUE;}
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height) {return MTRUE;}
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const {return MTRUE;}
    virtual MBOOL setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const {return MTRUE;}
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const {return 0;}
    /**
     * @brief add callbacks for 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 addCallbacks(I3ACallBack* cb) {return 0;}
    /**
     * @brief remove callbacks in 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 removeCallbacks(I3ACallBack* cb) {return 0;}

    virtual MBOOL setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr) {return MTRUE;}
    virtual MINT32 getExposureInfo(ExpSettingParam_T &strHDRInputSetting) {return 0;}
    virtual MINT32 getCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo) {return 0;}
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo) {return 0;}
    virtual MBOOL setFDInfo(MVOID* a_sFaces) {return MTRUE;}
    virtual MBOOL setOTInfo(MVOID* a_sOT) {return MTRUE;}
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo) {return 0;}
    virtual MINT32 isNeedFiringFlash(MBOOL bEnCal) {return 0;}
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo) {return MTRUE;}
    virtual MINT32 modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100) {return 0;}
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo) {return MTRUE;}
    virtual MVOID endContinuousShotJobs() {}
    virtual MVOID enterCaptureProcess() {}
    virtual MVOID exitCaptureProcess() {}
    virtual MUINT32 queryFramesPerCycle(MUINT32 fps) {return 1;}
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter) {return 0;}
    virtual MVOID setFDEnable(MBOOL bEnable) {}
    virtual MVOID setSensorMode(MINT32 i4SensorMode) {}
    virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode) {}
    virtual MINT32 SetAETargetMode(MUINT32 AeTargetMode) { return 0; }
    virtual MVOID setFrameId(MINT32 i4FrmId) {}
    MINT32 getFrameId() const {return 0;}

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2) {return 0;}
    virtual MINT32 getSensorType() const{return 0;}
    virtual MINT32 enableFlashQuickCalibration(MINT32 bEn) {return 0;}
    virtual MINT32 getFlashQuickCalibrationResult() {return 0;}
    virtual MVOID setAELock(MBOOL  bIsAELock){};
    virtual MVOID setAWBLock(MBOOL  bIsAELock){};

};

class CallBackSet : public I3ACallBack
{
public:
    CallBackSet();
    virtual             ~CallBackSet();

    virtual void        doNotifyCb (
                           int32_t _msgType,
                           int32_t _ext1,
                           int32_t _ext2,
                           int32_t _ext3,
                           MINTPTR _ext4 = 0
                        );

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        );

    virtual MINT32      addCallback(I3ACallBack* cb);
    virtual MINT32      removeCallback(I3ACallBack* cb);
private:
    List<I3ACallBack*>  m_CallBacks;
    Mutex               m_Mutex;
};

}; // namespace NS3A

#endif

