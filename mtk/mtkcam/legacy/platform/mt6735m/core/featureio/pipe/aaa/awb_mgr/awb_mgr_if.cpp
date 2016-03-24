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
#define LOG_TAG "awb_mgr_if"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <aaa_hal.h>

#include <isp_tuning.h>
#include "awb_mgr_if.h"
#include "awb_mgr.h"

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbMgr&
IAwbMgr::
getInstance()
{
    static  IAwbMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraPreviewInit(i4SensorIdx, rParam);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
camcorderPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).camcorderPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).camcorderPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).camcorderPreviewInit(i4SensorIdx, rParam);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraCaptureInit(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraCaptureInit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraCaptureInit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraCaptureInit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
cameraPreviewReinit(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).cameraPreviewReinit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).cameraPreviewReinit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).cameraPreviewReinit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
uninit(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
isAWBEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).isAWBEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).isAWBEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).isAWBEnable();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBMode(MINT32 i4SensorDev, MINT32 i4NewAWBMode)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBMode(i4NewAWBMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBMode(i4NewAWBMode);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBMode();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBMode();
    }

    MY_ERR("Err IAwbMgr::getAWBMode()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setStrobeMode(MINT32 i4SensorDev, MINT32 i4NewStrobeMode)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setStrobeMode(i4NewStrobeMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setStrobeMode(i4NewStrobeMode);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getStrobeMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getStrobeMode();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getStrobeMode();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getStrobeMode();
    }

    MY_ERR("Err IAwbMgr::getStrobeMode()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setFlashAWBData(MINT32 i4SensorDev, FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setFlashAWBData(rFlashAwbData);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setFlashAWBData(rFlashAwbData);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBLock(MINT32 i4SensorDev, MBOOL bAWBLock)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBLock(bAWBLock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBLock(bAWBLock);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
enableAWB(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).enableAWB();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).enableAWB();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).enableAWB();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
disableAWB(MINT32 i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).disableAWB();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).disableAWB();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).disableAWB();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setAWBStatCropRegion(MINT32 i4SensorDev,
                     MINT32 i4SensorMode,
                     MINT32 i4CropOffsetX,
                     MINT32 i4CropOffsetY,
                     MINT32 i4CropRegionWidth,
                     MINT32 i4CropRegionHeight)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).setAWBStatCropRegion(i4SensorMode, i4CropOffsetX, i4CropOffsetY, i4CropRegionWidth, i4CropRegionHeight);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPvAWB(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doPvAWB(i4FrameCount, bAEStable, i4SceneLV, pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doAFAWB(MINT32 i4SensorDev, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doAFAWB(pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doAFAWB(pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doAFAWB(pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doPreCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doPreCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
doCapAWB(MINT32 i4SensorDev, MINT32 i4SceneLV, MVOID *pAWBStatBuf, MBOOL bApplyToHW)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AwbMgr::getInstance(ESensorDev_Main).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AwbMgr::getInstance(ESensorDev_Sub).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AwbMgr::getInstance(ESensorDev_MainSecond).doCapAWB(i4SceneLV, pAWBStatBuf, bApplyToHW);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
applyAWB(MINT32 i4SensorDev, AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).applyAWB(rAWBSyncOutput);
        return MTRUE;
    }

    MY_ERR("Err IAwbMgr::applyAWB()\n");

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getDebugInfo(MINT32 i4SensorDev, AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAWBDebugInfo, rAWBDebugData);
        return MTRUE;
    }

    MY_ERR("Err IAwbMgr::getDebugInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAWBCCT(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAWBCCT();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAWBCCT();
    }

    MY_ERR("Err IAwbMgr::getAWBCCT()\n");

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getASDInfo(MINT32 i4SensorDev, AWB_ASD_INFO_T &a_rAWBASDInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getASDInfo(a_rAWBASDInfo);
        return MTRUE;
    }

    MY_ERR("Err IAwbMgr::getASDInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getAWBOutput(MINT32 i4SensorDev, AWB_OUTPUT_T &a_rAWBOutput)
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getAWBOutput(a_rAWBOutput);
        return MTRUE;
    }

    MY_ERR("Err IAwbMgr::getAWBOutput()\n");

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
getAWBParentStat(MINT32 i4SensorDev, AWB_PARENT_BLK_STAT_T &a_rAWBParentState, MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY )
{
    if (i4SensorDev & ESensorDev_Main) {
        AwbMgr::getInstance(ESensorDev_Main).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        AwbMgr::getInstance(ESensorDev_Sub).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }
    if (i4SensorDev & ESensorDev_MainSecond) {
        AwbMgr::getInstance(ESensorDev_MainSecond).getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);
        return MTRUE;
    }

    return MFALSE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IAwbMgr::
setAFLV(MINT32 i4SensorDev, MINT32 i4AFLV)
{
    if (i4SensorDev & ESensorDev_Main)
        AwbMgr::getInstance(ESensorDev_Main).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_Sub)
        AwbMgr::getInstance(ESensorDev_Sub).setAFLV(i4AFLV);
    if (i4SensorDev & ESensorDev_MainSecond)
        AwbMgr::getInstance(ESensorDev_MainSecond).setAFLV(i4AFLV);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IAwbMgr::
getAFLV(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getAFLV();
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getAFLV();
    }

    MY_ERR("Err IAwbMgr::getAFLV()\n");

    return 0;
}


MBOOL IAwbMgr::SaveAwbMgrInfo(MINT32 i4SensorDev, const char * fname)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).SaveAwbMgrInfo(fname);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).SaveAwbMgrInfo(fname);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).SaveAwbMgrInfo(fname);
    }

    MY_ERR("Err IAwbMgr::SaveAwbMgrInfo()\n");

    return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
getInitInputParam(MINT32 i4SensorDev, SYNC_CTC_INIT_PARAM &CCTInitParam, NORMAL_GAIN_INIT_INPUT_PARAM &NoramlGainInitParam)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).getInitInputParam(CCTInitParam, NoramlGainInitParam);
    }

    MY_ERR("Err IAwbMgr::getInitInputParam()\n");

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);
    }

    MY_ERR("Err IAwbMgr::setTGInfo()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAwbMgr::setAWBNvramIdx(MINT32 const i4SensorDev, MINT32 const i4AWBNvramIdx)
{
    if (i4SensorDev & ESensorDev_Main) {
        return AwbMgr::getInstance(ESensorDev_Main).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        return AwbMgr::getInstance(ESensorDev_Sub).setAWBNvramIdx(i4AWBNvramIdx);
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        return AwbMgr::getInstance(ESensorDev_MainSecond).setAWBNvramIdx(i4AWBNvramIdx);
    }

    MY_ERR("Err IAwbMgr::setAWBNvramIdx()\n");

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBEnable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBEnable();

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBDisable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBDisable();

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetEnableInfo(a_pEnableAWB, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetAWBGain(a_pAWBGain, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBGain(MINT32 i4SensorDev, MVOID *a_pAWBGain)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSetAWBGain(a_pAWBGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSetAWBGain(a_pAWBGain);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBApplyNVRAMParam(a_pAWBNVRAM);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetNVRAMParam(a_pAWBNVRAM, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetNVRAMParam(MINT32 i4SensorDev, AWB_NVRAM_IDX_T eIdx, MVOID *a_pAWBNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetNVRAMParam(eIdx, a_pAWBNVRAM);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSaveNVRAMParam();

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBSetAWBMode(MINT32 i4SensorDev, MINT32 a_AWBMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBSetAWBMode(a_AWBMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBSetAWBMode(a_AWBMode);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetAWBMode(MINT32 i4SensorDev, MINT32 *a_pAWBMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetAWBMode(a_pAWBMode, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBGetLightProb(MINT32 i4SensorDev, MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBGetLightProb(a_pAWBLightProb, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPAWBBypassCalibration(MINT32 i4SensorDev, MBOOL bBypassCalibration)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPAWBBypassCalibration(bBypassCalibration);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPAWBBypassCalibration(bBypassCalibration);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBApplyNVRAMParam(a_pFlashAWBNVRAM);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pFlashAWBNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBGetNVRAMParam(a_pFlashAWBNVRAM, a_pOutLen);

    return E_AWB_PARAMETER_ERROR;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAwbMgr::
CCTOPFlashAWBSaveNVRAMParam(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AwbMgr::getInstance(ESensorDev_Main).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        return AwbMgr::getInstance(ESensorDev_Sub).CCTOPFlashAWBSaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AwbMgr::getInstance(ESensorDev_MainSecond).CCTOPFlashAWBSaveNVRAMParam();

    return E_AWB_PARAMETER_ERROR;
}

