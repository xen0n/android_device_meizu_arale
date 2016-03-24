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
#define LOG_TAG "af_mgr_if"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <aaa_log.h>
#include <mtkcam/hal/sensor_hal.h>
//#include <faces.h>
//#include <mtkcam/featureio/aaa_hal_common.h>
//#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
//#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
//#include <mtkcam/drv/isp_reg.h>
//#include <mtkcam/drv/isp_drv.h>
//#include <sensor_hal.h>
#include <nvram_drv.h>
//#include <nvram_drv_mgr.h>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <mcu_drv.h>


#include <af_feature.h>
class NvramDrvBase;
using namespace android;

#include <mtkcam/algorithm/lib3a/af_algo_if.h>
//#include <CamDefs.h>
#include "af_mgr_if.h"
#include "af_mgr.h"


using namespace NS3A;
static  IAfMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAfMgr& IAfMgr::getInstance()
{
    return  singleton;
}
MRESULT IAfMgr::init(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 isInitMCU)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).init(i4SensorIdx, isInitMCU);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).init(i4SensorIdx, isInitMCU);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).init(i4SensorIdx, isInitMCU);
    return AfMgr::getInstance(ESensorDev_Main).init(i4SensorIdx, isInitMCU);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::uninit(MINT32 i4SensorDev, MINT32 isInitMCU)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).uninit(isInitMCU);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).uninit(isInitMCU);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).uninit(isInitMCU);

return AfMgr::getInstance(ESensorDev_Main).uninit(isInitMCU);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::triggerAF(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).triggerAF();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).triggerAF();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).triggerAF();

return AfMgr::getInstance(ESensorDev_Main).triggerAF();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFMode(MINT32 i4SensorDev,MINT32 a_eAFMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAFMode(a_eAFMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAFMode(a_eAFMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAFMode(a_eAFMode);

return AfMgr::getInstance(ESensorDev_Main).setAFMode(a_eAFMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFArea(MINT32 i4SensorDev, CameraFocusArea_T a_sAFArea)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAFArea( a_sAFArea);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAFArea( a_sAFArea);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAFArea( a_sAFArea);
return AfMgr::getInstance(ESensorDev_Main).setAFArea( a_sAFArea);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setCamMode(MINT32 i4SensorDev, MINT32 a_eCamMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setCamMode(a_eCamMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setCamMode(a_eCamMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setCamMode(a_eCamMode);
return AfMgr::getInstance(ESensorDev_Main).setCamMode(a_eCamMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setZoomWinInfo(MINT32 i4SensorDev,MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);

return AfMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setFDInfo(a_sFaces);

return AfMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces);
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sObtinfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setOTInfo(a_sObtinfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setOTInfo(a_sObtinfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setOTInfo(a_sObtinfo);

return AfMgr::getInstance(ESensorDev_Main).setOTInfo(a_sObtinfo);
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFMaxAreaNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFMaxAreaNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFMaxAreaNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFMaxAreaNum();

return AfMgr::getInstance(ESensorDev_Main).getAFMaxAreaNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMaxLensPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getMaxLensPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getMaxLensPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getMaxLensPos();
return AfMgr::getInstance(ESensorDev_Main).getMaxLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMinLensPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getMinLensPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getMinLensPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getMinLensPos();
return AfMgr::getInstance(ESensorDev_Main).getMinLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFBestPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFBestPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFBestPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFBestPos();

return AfMgr::getInstance(ESensorDev_Main).getAFBestPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFPos(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFPos();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFPos();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFPos();

return AfMgr::getInstance(ESensorDev_Main).getAFPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFStable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFStable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFStable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFStable();

return AfMgr::getInstance(ESensorDev_Main).getAFStable();
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableOffset(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableOffset();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableOffset();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableOffset();

return AfMgr::getInstance(ESensorDev_Main).getAFTableOffset();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableMacroIdx(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableMacroIdx();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableMacroIdx();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableMacroIdx();

return AfMgr::getInstance(ESensorDev_Main).getAFTableMacroIdx();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableIdxNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTableIdxNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTableIdxNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTableIdxNum();

return AfMgr::getInstance(ESensorDev_Main).getAFTableIdxNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IAfMgr::getAFTable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFTable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFTable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFTable();

return AfMgr::getInstance(ESensorDev_Main).getAFTable();
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setMFPos(MINT32 i4SensorDev, MINT32 a_i4Pos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setMFPos(a_i4Pos);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setMFPos(a_i4Pos);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setMFPos(a_i4Pos);

return AfMgr::getInstance(ESensorDev_Main).setMFPos(a_i4Pos);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFullScanstep(MINT32 i4SensorDev, MINT32 a_i4Step)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setFullScanstep(a_i4Step);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setFullScanstep(a_i4Step);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setFullScanstep(a_i4Step);

return AfMgr::getInstance(ESensorDev_Main).setFullScanstep(a_i4Step);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T IAfMgr::getFLKStat(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFLKStat();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFLKStat();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFLKStat();

return AfMgr::getInstance(ESensorDev_Main).getFLKStat();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID IAfMgr::setAE2AFInfo(MINT32 i4SensorDev,AE2AFInfo_T rAEInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setAE2AFInfo(rAEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setAE2AFInfo(rAEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setAE2AFInfo(rAEInfo);

return AfMgr::getInstance(ESensorDev_Main).setAE2AFInfo(rAEInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::getAFRefWin(MINT32 i4SensorDev, CameraArea_T &rWinSize)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAFRefWin(rWinSize);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAFRefWin(rWinSize);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAFRefWin(rWinSize);

return AfMgr::getInstance(ESensorDev_Main).getAFRefWin(rWinSize);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::doAF(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).doAF(pAFStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).doAF(pAFStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).doAF(pAFStatBuf);

return AfMgr::getInstance(ESensorDev_Main).doAF(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAF_IN_HSIZE(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setAF_IN_HSIZE();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setAF_IN_HSIZE();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setAF_IN_HSIZE();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::enableAF(MINT32 i4SensorDev, MINT32 a_i4En)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).enableAF(a_i4En);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).enableAF(a_i4En);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).enableAF(a_i4En);
return AfMgr::getInstance(ESensorDev_Main).enableAF(a_i4En);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocusFinish(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isFocusFinish();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isFocusFinish();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isFocusFinish();

return AfMgr::getInstance(ESensorDev_Main).isFocusFinish();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocused(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isFocused();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isFocused();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isFocused();

return AfMgr::getInstance(ESensorDev_Main).isFocused();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isLastFocusModeTAF(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).isLastFocusModeTAF();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).isLastFocusModeTAF();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).isLastFocusModeTAF();

return AfMgr::getInstance(ESensorDev_Main).isLastFocusModeTAF();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getDebugInfo(MINT32 i4SensorDev, AF_DEBUG_INFO_T &rAFDebugInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getDebugInfo(rAFDebugInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAFDebugInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAFDebugInfo);
return AfMgr::getInstance(ESensorDev_Main).getDebugInfo(rAFDebugInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAfMgr::setCallbacks(MINT32 i4SensorDev, I3ACallBack* cb)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setCallbacks(cb);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setCallbacks(cb);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setCallbacks(cb);

return AfMgr::getInstance(ESensorDev_Main).setCallbacks(cb);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SingleAF_CallbackNotify(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).SingleAF_CallbackNotify();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).SingleAF_CallbackNotify();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).SingleAF_CallbackNotify();
return AfMgr::getInstance(ESensorDev_Main).SingleAF_CallbackNotify();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setBestShotConfig(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).setBestShotConfig();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).setBestShotConfig();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).setBestShotConfig();
return AfMgr::getInstance(ESensorDev_Main).setBestShotConfig();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::calBestShotValue(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).calBestShotValue(pAFStatBuf);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).calBestShotValue(pAFStatBuf);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).calBestShotValue(pAFStatBuf);

return AfMgr::getInstance(ESensorDev_Main).calBestShotValue(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAfMgr::getBestShotValue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getBestShotValue();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getBestShotValue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getBestShotValue();

return AfMgr::getInstance(ESensorDev_Main).getBestShotValue();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setSGGPGN(MINT32 i4SensorDev, MINT32 i4SGG_PGAIN)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setSGGPGN(i4SGG_PGAIN);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setSGGPGN(i4SGG_PGAIN);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setSGGPGN(i4SGG_PGAIN);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::autoFocus(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).autoFocus();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).autoFocus();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).autoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::DoCallback(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).DoCallback();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).DoCallback();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).DoCallback();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::cancelAutoFocus(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).cancelAutoFocus();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).cancelAutoFocus();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).cancelAutoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::TimeOutHandle(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).TimeOutHandle();
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).TimeOutHandle();
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).TimeOutHandle();
}
MVOID IAfMgr::setAndroidServiceState(MINT32 i4SensorDev, MBOOL a_state)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setAndroidServiceState(a_state);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setAndroidServiceState(a_state);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setAndroidServiceState(a_state);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Depth AF API
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getDAFtbl(MINT32 i4SensorDev,MVOID ** ptbl)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getDAFtbl(ptbl);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getDAFtbl(ptbl);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getDAFtbl(ptbl);
    return AfMgr::getInstance(ESensorDev_Main).getDAFtbl(ptbl);
}
MVOID IAfMgr::setCurFrmNum(MINT32 i4SensorDev, MUINT32 frm_num)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setCurFrmNum(frm_num);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setCurFrmNum(frm_num);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setCurFrmNum(frm_num);
}
MINT32 IAfMgr::queryAFStatusByMagicNum(MINT32 i4SensorDev, MUINT32 i4MagicNum)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).queryAFStatusByMagicNum(i4MagicNum);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).queryAFStatusByMagicNum(i4MagicNum);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).queryAFStatusByMagicNum(i4MagicNum);

    return AfMgr::getInstance(ESensorDev_Main).queryAFStatusByMagicNum(i4MagicNum);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Camera 3.0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAperture(MINT32 i4SensorDev, MINT32 lens_aperture)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setAperture(lens_aperture);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setAperture(lens_aperture);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setAperture(lens_aperture);
}

MINT32 IAfMgr::getAperture(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getAperture();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getAperture();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getAperture();

    return AfMgr::getInstance(ESensorDev_Main).getAperture();
}
MVOID IAfMgr::setFocalLength(MINT32 i4SensorDev, MINT32 lens_focalLength)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setFocalLength(lens_focalLength);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setFocalLength(lens_focalLength);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setFocalLength(lens_focalLength);
}

MINT32 IAfMgr::getFocalLength(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocalLength();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocalLength();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocalLength();

    return AfMgr::getInstance(ESensorDev_Main).getFocalLength();
}

MVOID IAfMgr::setFocusDistance(MINT32 i4SensorDev, MINT32 lens_focusDistance)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setFocusDistance(lens_focusDistance);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setFocusDistance(lens_focusDistance);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setFocusDistance(lens_focusDistance);
}

MINT32 IAfMgr::getFocusDistance(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocusDistance();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocusDistance();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocusDistance();

    return AfMgr::getInstance(ESensorDev_Main).getFocusDistance();
}

MVOID IAfMgr::setOpticalStabilizationMode(MINT32 i4SensorDev, MINT32 ois_OnOff)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setOpticalStabilizationMode(ois_OnOff);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setOpticalStabilizationMode(ois_OnOff);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setOpticalStabilizationMode(ois_OnOff);
}

MINT32 IAfMgr::getOpticalStabilizationMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getOpticalStabilizationMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getOpticalStabilizationMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getOpticalStabilizationMode();

    return AfMgr::getInstance(ESensorDev_Main).getOpticalStabilizationMode();
}

MINT32 IAfMgr::getFocusRange(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getFocusRange();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getFocusRange();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getFocusRange();

    return AfMgr::getInstance(ESensorDev_Main).getFocusRange();
}
MINT32 IAfMgr::getLensState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getLensState();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getLensState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getLensState();

    return AfMgr::getInstance(ESensorDev_Main).getLensState();
}

MVOID IAfMgr::setSharpnessMapMode(MINT32 i4SensorDev, MINT32 SharpMapOnOff)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).setSharpnessMapMode(SharpMapOnOff);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).setSharpnessMapMode(SharpMapOnOff);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).setSharpnessMapMode(SharpMapOnOff);
}

MINT32 IAfMgr::getSharpnessMapMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getSharpnessMapMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getSharpnessMapMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getSharpnessMapMode();

    return AfMgr::getInstance(ESensorDev_Main).getSharpnessMapMode();
}

MINT32 IAfMgr::getMaxSharpnessMapValue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getMaxSharpnessMapValue();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getMaxSharpnessMapValue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getMaxSharpnessMapValue();

    return AfMgr::getInstance(ESensorDev_Main).getMaxSharpnessMapValue();
}
MINT32 IAfMgr::getSharpnessMapSize(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).getSharpnessMapSize();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).getSharpnessMapSize();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).getSharpnessMapSize();

    return AfMgr::getInstance(ESensorDev_Main).getSharpnessMapSize();
}
MVOID IAfMgr::getSharpnessMap(MINT32 i4SensorDev, MVOID* a_sAFFullStat)
{
    if (i4SensorDev & ESensorDev_Main)
        AfMgr::getInstance(ESensorDev_Main).getSharpnessMap(a_sAFFullStat);
    if (i4SensorDev & ESensorDev_Sub)
        AfMgr::getInstance(ESensorDev_Sub).getSharpnessMap(a_sAFFullStat);
    if (i4SensorDev & ESensorDev_MainSecond)
        AfMgr::getInstance(ESensorDev_MainSecond).getSharpnessMap(a_sAFFullStat);

    AfMgr::getInstance(ESensorDev_Main).getSharpnessMap(a_sAFFullStat);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//      CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameinit(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameinit(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTMCUNameinit(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTMCUNameinit(i4SensorIdx);
    return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameinit(i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameuninit(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameuninit();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTMCUNameuninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTMCUNameuninit();

return AfMgr::getInstance(ESensorDev_Main).CCTMCUNameuninit();
}
MINT32 IAfMgr::CCTOPAFOpeartion(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFOpeartion();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFOpeartion();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFOpeartion();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFOpeartion();
}
MINT32 IAfMgr::CCTOPMFOpeartion(MINT32 i4SensorDev,MINT32 a_i4MFpos)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPMFOpeartion(a_i4MFpos);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPMFOpeartion(a_i4MFpos);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPMFOpeartion(a_i4MFpos);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPMFOpeartion(a_i4MFpos);
}
MINT32 IAfMgr::CCTOPAFGetAFInfo(MINT32 i4SensorDev,MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetBestPos(MINT32 i4SensorDev,MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFCaliOperation(MINT32 i4SensorDev,MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFSetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetFocusRange(a_pFocusRange);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFSetFocusRange(a_pFocusRange);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFSetFocusRange(a_pFocusRange);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSetFocusRange(a_pFocusRange);
}
MINT32 IAfMgr::CCTOPAFGetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFApplyNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFApplyNVRAMParam(a_pAFNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFApplyNVRAMParam(a_pAFNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFApplyNVRAMParam(a_pAFNVRAM);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFApplyNVRAMParam(a_pAFNVRAM);
}
MINT32 IAfMgr::CCTOPAFSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFSaveNVRAMParam();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFSaveNVRAMParam();
}
MINT32 IAfMgr::CCTOPAFGetFV(MINT32 i4SensorDev,MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFEnable();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFEnable();
}
MINT32 IAfMgr::CCTOPAFDisable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFDisable();
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFDisable();

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFDisable();
}
MINT32 IAfMgr::CCTOPAFGetEnableInfo(MINT32 i4SensorDev,MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AfMgr::getInstance(ESensorDev_Sub).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AfMgr::getInstance(ESensorDev_MainSecond).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);

    return AfMgr::getInstance(ESensorDev_Main).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
}
