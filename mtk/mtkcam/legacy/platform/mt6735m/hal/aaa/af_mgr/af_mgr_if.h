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
#ifndef _AF_MGR_IF_H_
#define _AF_MGR_IF_H_

#include <af_param.h>
#include <af_feature.h>
#include <mtkcam/algorithm/lib3a/af_algo_if.h>

namespace NS3A
{
    /*******************************************************************************
    *
    *******************************************************************************/
    class IAfMgr
    {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    Ctor/Dtor.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    private:    ////    Disallowed.
        //    Copy constructor is disallowed.
        IAfMgr(IAfMgr const&);
        //    Copy-assignment operator is disallowed.
        IAfMgr& operator=(IAfMgr const&);
    public:  ////
        IAfMgr() {};
        ~IAfMgr() {};

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
    static IAfMgr& getInstance();
    MRESULT init(MINT32 i4SensorDev, MINT32 i4SensorIdx,MINT32 isInitMCU=1);
    MRESULT uninit(MINT32 i4SensorDev,MINT32 isInitMCU=1);
    MRESULT triggerAF(MINT32 i4SensorDev);
    MRESULT setAFMode(MINT32 i4SensorDev,MINT32 a_eAFMode);
    EAfState_T  getAFState(MINT32 i4SensorDev);
    MRESULT setAFArea(MINT32 i4SensorDev, CameraFocusArea_T a_sAFArea);
    MRESULT setCamMode(MINT32 i4SensorDev, MINT32 a_eCamMode);
    MRESULT setZoomWinInfo(MINT32 i4SensorDev,MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces);
    MRESULT setOTInfo(MINT32 i4SensorDev, MVOID* a_sObtinfo);
    MINT32 getAFMaxAreaNum(MINT32 i4SensorDev);
    MINT32 getMaxLensPos(MINT32 i4SensorDev);
    MINT32 getMinLensPos(MINT32 i4SensorDev);
    MINT32 getAFBestPos(MINT32 i4SensorDev);
    MINT32 getAFPos(MINT32 i4SensorDev);
    MINT32 getAFStable(MINT32 i4SensorDev);
    MINT32 getAFTableOffset(MINT32 i4SensorDev);
    MINT32 getAFTableMacroIdx(MINT32 i4SensorDev);
    MINT32 getAFTableIdxNum(MINT32 i4SensorDev);
    MVOID* getAFTable(MINT32 i4SensorDev);
    MRESULT setMFPos(MINT32 i4SensorDev, MINT32 a_i4Pos);
    MRESULT setFullScanstep(MINT32 i4SensorDev, MINT32 a_i4Step);
    AF_FULL_STAT_T getFLKStat(MINT32 i4SensorDev);
    MVOID  setAE2AFInfo(MINT32 i4SensorDev,AE2AFInfo_T rAEInfo);
    MVOID  getAFRefWin(MINT32 i4SensorDev, CameraArea_T &rWinSize);
    MRESULT doAF(MINT32 i4SensorDev, MVOID *pAFStatBuf);
    MVOID doPDBuffer(MINT32 i4SensorDev, MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride);
    MINT32 PDPureRawInterval(MINT32 i4SensorDev);
    MRESULT doPDTask(MINT32 i4SensorDev);
    MVOID setAF_IN_HSIZE(MINT32 i4SensorDev);
    MRESULT enableAF(MINT32 i4SensorDev, MINT32 a_i4En);
    MINT32 isFocusFinish(MINT32 i4SensorDev);
    MINT32 isFocused(MINT32 i4SensorDev);
    MINT32 isLastFocusModeTAF(MINT32 i4SensorDev);
    MRESULT getDebugInfo(MINT32 i4SensorDev, AF_DEBUG_INFO_T &rAFDebugInfo);
    MBOOL setCallbacks(MINT32 i4SensorDev, I3ACallBack* cb);
    MRESULT SingleAF_CallbackNotify(MINT32 i4SensorDev);
    MRESULT setBestShotConfig(MINT32 i4SensorDev);
    MRESULT calBestShotValue(MINT32 i4SensorDev, MVOID *pAFStatBuf);
    MINT64 getBestShotValue(MINT32 i4SensorDev);
    MVOID setSGGPGN(MINT32 i4SensorDev, MINT32 i4SGG_PGAIN);
    MVOID autoFocus(MINT32 i4SensorDev);
    MVOID DoCallback(MINT32 i4SensorDev);
    MVOID cancelAutoFocus(MINT32 i4SensorDev);
    MVOID TimeOutHandle(MINT32 i4SensorDev);
    MVOID setAndroidServiceState(MINT32 i4SensorDev, MBOOL a_state);

    // Depth AF API
    MINT32 getDAFtbl(MINT32 i4SensorDev,MVOID ** ptbl);
    MVOID  setCurFrmNum(MINT32 i4SensorDev, MUINT32 frm_num);
    MINT32 queryAFStatusByMagicNum(MINT32 i4SensorDev, MUINT32 i4MagicNum);
    //Camera 3.0
    MVOID   setAperture(MINT32 i4SensorDev,MINT32 lens_aperture);
    MINT32  getAperture(MINT32 i4SensorDev);
    MVOID   setFocalLength(MINT32 i4SensorDev,MINT32 lens_focalLength);
    MINT32  getFocalLength(MINT32 i4SensorDev);
    MVOID   setFocusDistance(MINT32 i4SensorDev,MINT32 lens_focusDistance);
    MINT32  getFocusDistance(MINT32 i4SensorDev);
    MVOID   setOpticalStabilizationMode (MINT32 i4SensorDev,MINT32 ois_OnOff);
    MINT32  getOpticalStabilizationMode(MINT32 i4SensorDev);
    MINT32  getFocusRange(MINT32 i4SensorDev);
    MINT32  getLensState(MINT32 i4SensorDev);
    MVOID   setSharpnessMapMode(MINT32 i4SensorDev, MINT32 SharpMapOnOff);
    MINT32  getSharpnessMapMode(MINT32 i4SensorDev);
    MINT32  getMaxSharpnessMapValue(MINT32 i4SensorDev);
    MINT32  getSharpnessMapSize(MINT32 i4SensorDev);
    MVOID getSharpnessMap(MINT32 i4SensorDev, MVOID* a_sAFFullStat);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    CCT feature
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MRESULT CCTMCUNameinit(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    MRESULT CCTMCUNameuninit(MINT32 i4SensorDev);
    MINT32 CCTOPAFOpeartion(MINT32 i4SensorDev);
    MINT32 CCTOPMFOpeartion(MINT32 i4SensorDev,MINT32 a_i4MFpos);
    MINT32 CCTOPAFGetAFInfo(MINT32 i4SensorDev,MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetBestPos(MINT32 i4SensorDev,MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFCaliOperation(MINT32 i4SensorDev,MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFSetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange);
    MINT32 CCTOPAFGetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFApplyNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM);
    MINT32 CCTOPAFSaveNVRAMParam(MINT32 i4SensorDev);
    MINT32 CCTOPAFGetFV(MINT32 i4SensorDev,MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFEnable(MINT32 i4SensorDev);
    MINT32 CCTOPAFDisable(MINT32 i4SensorDev);
    MINT32 CCTOPAFGetEnableInfo(MINT32 i4SensorDev,MVOID *a_pEnableAF, MUINT32 *a_pOutLen);

    };
};    //    namespace NS3A
#endif // _AE_MGR_N3D_H_

