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
#define LOG_TAG "ae_mgr_if"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <mtkcam/featureio/aaa_hal_if.h>
//#include <mtkcam/featureio/aaa_hal_common.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <camera_custom_AEPlinetable.h>
//#include <Modes.h>
//#include <faces.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
//#include <sensor_hal.h>
//#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>

#include <mtkcam/hal/IHalSensor.h>
using namespace NSCam;
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <kd_camera_feature.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"

using namespace NS3A;
using namespace NSIspTuning;

static  IAeMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAeMgr&
IAeMgr::
getInstance()
{
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::cameraPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).cameraPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).cameraPreviewInit(i4SensorIdx, rParam);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::camcorderPreviewInit(MINT32 i4SensorDev, MINT32 i4SensorIdx, Param_T &rParam)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).camcorderPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).camcorderPreviewInit(i4SensorIdx, rParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).camcorderPreviewInit(i4SensorIdx, rParam);

    return (ret_main | ret_sub | ret_main2);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// for come back to preview/video condition use
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::cameraPreviewReinit(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).cameraPreviewReinit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).cameraPreviewReinit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).cameraPreviewReinit();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::uninit(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).uninit();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void IAeMgr::setAeMeterAreaEn(MINT32 i4SensorDev, int en)
{

    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setAeMeterAreaEn(en);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setAeMeterAreaEn(en);
}

MRESULT IAeMgr::setAEMeteringArea(MINT32 i4SensorDev, CameraMeteringArea_T const *sNewAEMeteringArea)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringArea(sNewAEMeteringArea);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringArea(sNewAEMeteringArea);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setFDenable(MINT32 i4SensorDev, MBOOL bFDenable)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFDenable(bFDenable);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFDenable(bFDenable);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT IAeMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setFDInfo(a_sFaces);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setFDInfo(a_sFaces);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sOT)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setOTInfo(a_sOT);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setOTInfo(a_sOT);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEEVCompIndex(MINT32 i4SensorDev, MINT32 i4NewEVIndex, MFLOAT fStep)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEEVCompIndex(i4NewEVIndex, fStep);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEEVCompIndex(i4NewEVIndex, fStep);

    return (ret_main | ret_sub | ret_main2);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 IAeMgr::getEVCompensateIndex(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getEVCompensateIndex();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getEVCompensateIndex();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMeteringMode(MINT32 i4SensorDev, MUINT32 u4NewAEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMeteringMode(u4NewAEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMeteringMode(u4NewAEMeteringMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEMeterMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeterMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeterMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeterMode();

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEISOSpeed(MINT32 i4SensorDev, MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEISOSpeed(u4NewAEISOSpeed);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEISOSpeed(u4NewAEISOSpeed);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEISOSpeedMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEISOSpeedMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEISOSpeedMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMinMaxFrameRate(MINT32 i4SensorDev, MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMinMaxFrameRate(i4NewAEMinFps, i4NewAEMaxFps);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEFLKMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEFlickerMode(u4NewAEFLKMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEFlickerMode(u4NewAEFLKMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEAutoFlickerMode(MINT32 i4SensorDev, MUINT32 u4NewAEAutoFLKMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEAutoFlickerMode(u4NewAEAutoFLKMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEAutoFlickerMode(u4NewAEAutoFLKMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAECamMode(MINT32 i4SensorDev, MUINT32 u4NewAECamMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAECamMode(u4NewAECamMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAECamMode(u4NewAECamMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEShotMode(MINT32 i4SensorDev, MUINT32 u4NewAEShotMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEShotMode(u4NewAEShotMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEShotMode(u4NewAEShotMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAELimiterMode(MINT32 i4SensorDev, MBOOL bAELimter)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAELimiterMode(bAELimter);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAELimiterMode(bAELimter);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSceneMode(MINT32 i4SensorDev, MUINT32 u4NewScene)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setSceneMode(u4NewScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setSceneMode(u4NewScene);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEScene(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEScene();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEScene();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEScene();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAEMode(MINT32 i4SensorDev, MUINT32 u4NewAEmode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAEMode(u4NewAEmode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAEMode(u4NewAEmode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEState(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEState();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEState();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEState();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setSensorMode(i4NewSensorMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setSensorMode(i4NewSensorMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateAEScenarioMode(MINT32 i4SensorDev, EIspProfile_T eIspProfile)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateAEScenarioMode(eIspProfile);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateAEScenarioMode(eIspProfile);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getSensorMode(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorMode();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorMode();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorMode();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAPAELock(MINT32 i4SensorDev, MBOOL bAPAELock)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAPAELock(bAPAELock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAPAELock(bAPAELock);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAFAELock(MINT32 i4SensorDev, MBOOL bAFAELock)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAFAELock(bAFAELock);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAFAELock(bAFAELock);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setZoomWinInfo(MINT32 i4SensorDev, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 IAeMgr::getAEMaxMeterAreaNum(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMaxMeterAreaNum();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMaxMeterAreaNum();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).enableAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).enableAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).enableAE();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::disableAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).disableAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).disableAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).disableAE();

    return (ret_main | ret_sub | ret_main2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAEmonitor(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPvAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPvAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPvAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPvAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPvAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPvAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPvAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAEmonitor(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MBOOL &bWillUpdateSensorbyI2C)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doAFAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doAFAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doAFAEmonitor(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule, bWillUpdateSensorbyI2C);

    return (ret_main | ret_sub | ret_main2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doAFAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doAFAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doAFAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doAFAE(i4FrameCount, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doPreCapAE(MINT32 i4SensorDev, MINT32 i4FrameCount, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doPreCapAE(i4FrameCount, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doPreCapAE(i4FrameCount, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doPreCapAE(i4FrameCount, bIsStrobeFired, pAEStatBuf, i4ActiveAEItem, u4AAOUpdate, bAAASchedule);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapAE(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doCapAE();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doCapAE();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doCapAE();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doBackAEInfo(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doBackAEInfo();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doBackAEInfo();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doBackAEInfo();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void IAeMgr::setRestore(MINT32 i4SensorDev, int frm)
{
    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setRestore(frm);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setRestore(frm);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setRestore(frm);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doRestoreAEInfo(MINT32 i4SensorDev, MBOOL bRestorePrvOnly)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doRestoreAEInfo(bRestorePrvOnly);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doRestoreAEInfo(bRestorePrvOnly);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doRestoreAEInfo(bRestorePrvOnly);

    return (ret_main | ret_sub | ret_main2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::doCapFlare(MINT32 i4SensorDev, MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).doCapFlare(pAEStatBuf, bIsStrobe);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).doCapFlare(pAEStatBuf, bIsStrobe);

    return (ret_main | ret_sub | ret_main2);
}

void IAeMgr::setExp(MINT32 i4SensorDev, int exp)
{
    //MINT32 ret_main, ret_sub, ret_main2;
    //ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        AeMgr::getInstance(ESensorDev_Main).setExp(exp);
    if (i4SensorDev & ESensorDev_Sub)
        AeMgr::getInstance(ESensorDev_Sub).setExp(exp);
    if (i4SensorDev & ESensorDev_MainSecond)
        AeMgr::getInstance(ESensorDev_MainSecond).setExp(exp);

    //return (ret_main | ret_sub | ret_main2);
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getLVvalue(isStrobeOn);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAOECompLVvalue(MINT32 i4SensorDev, MBOOL isStrobeOn)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAOECompLVvalue(isStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAOECompLVvalue(isStrobeOn);

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getCaptureLVvalue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCaptureLVvalue();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCaptureLVvalue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCaptureLVvalue();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getDebugInfo(MINT32 i4SensorDev, AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getDebugInfo(rAEDebugInfo, rAEPlineDebugInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAENvramData(MINT32 i4SensorDev, AE_NVRAM_T &rAENVRAM)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAENvramData(rAENVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAENvramData(rAENVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAENvramData(rAENVRAM);

    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getNvramData(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getNvramData(i4SensorDev);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getNvramData(i4SensorDev);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getNvramData(i4SensorDev);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCurrentPlineTable(MINT32 i4SensorDev, strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCurrentPlineTable(a_PrvAEPlineTable, a_CapAEPlineTable, a_StrobeAEPlineTable, a_StrobeAEPlineInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getSensorDeviceInfo(MINT32 i4SensorDev, AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getSensorDeviceInfo(a_rDeviceInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getSensorDeviceInfo(a_rDeviceInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsDoAEInPreAF(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsDoAEInPreAF();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsDoAEInPreAF();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsAEStable(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAEStable();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAEStable();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAEStable();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getBVvalue(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getBVvalue();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getBVvalue();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getBVvalue();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::IsStrobeBVTrigger(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsStrobeBVTrigger();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsStrobeBVTrigger();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsStrobeBVTrigger();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setStrobeMode(MINT32 i4SensorDev, MBOOL bIsStrobeOn)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setStrobeMode(bIsStrobeOn);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setStrobeMode(bIsStrobeOn);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getPreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getPreviewParams(a_rPreviewInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getPreviewParams(a_rPreviewInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getExposureInfo(MINT32 i4SensorDev, ExpSettingParam_T &strHDRInputSetting)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getExposureInfo(strHDRInputSetting);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getExposureInfo(strHDRInputSetting);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getCaptureParams(a_rCaptureInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updatePreviewParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updatePreviewParams(a_rPreviewInfo, i4AEidxNext);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateCaptureParams(MINT32 i4SensorDev, AE_MODE_CFG_T &a_rCaptureInfo)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateCaptureParams(a_rCaptureInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateCaptureParams(a_rCaptureInfo);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringYvalue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringYvalue(rWinSize, uYvalue);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringYvalue(rWinSize, uYvalue);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEMeteringBlockAreaValue(MINT32 i4SensorDev, AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEMeteringBlockAreaValue(rWinSize, uYvalue, u2YCnt);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getHDRCapInfo(MINT32 i4SensorDev, Hal3A_HDROutputParam_T & strHDROutputInfo)
{
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getRTParams(MINT32 i4SensorDev, FrameOutputParam_T &a_strFrameInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getRTParams(a_strFrameInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getRTParams(a_strFrameInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::getAECondition(MINT32 i4SensorDev, MUINT32 i4AECondition)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAECondition(i4AECondition);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAECondition(i4AECondition);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAECondition(i4AECondition);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getLCEPlineInfo(MINT32 i4SensorDev, LCEInfo_T &a_rLCEInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getLCEPlineInfo(a_rLCEInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getLCEPlineInfo(a_rLCEInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getLCEPlineInfo(a_rLCEInfo);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setAERotateDegree(MINT32 i4SensorDev, MINT32 i4RotateDegree)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).setAERotateDegree(i4RotateDegree);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).setAERotateDegree(i4RotateDegree);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::UpdateSensorISPParams(MINT32 i4SensorDev, AE_STATE_T eNewAEState)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).UpdateSensorISPParams(eNewAEState);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).UpdateSensorISPParams(eNewAEState);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 IAeMgr::getAEFaceDiffIndex(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEFaceDiffIndex();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEFaceDiffIndex();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEFaceDiffIndex();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorDelayInfo(MINT32 i4SensorDev, MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateSensorDelayInfo(i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getBrightnessValue(MINT32 i4SensorDev, MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getBrightnessValue(bFrameUpdate, i4Yvalue);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getBrightnessValue(bFrameUpdate, i4Yvalue);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getBrightnessValue(bFrameUpdate, i4Yvalue);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAEBlockYvalues(MINT32 i4SensorDev, MUINT8 *pYvalues, MUINT8 size)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEBlockYvalues(pYvalues, size);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEBlockYvalues(pYvalues, size);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEBlockYvalues(pYvalues, size);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::SetAETargetMode(MINT32 i4SensorDev, eAETARGETMODE eAETargetMode)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).SetAETargetMode(eAETargetMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).SetAETargetMode(eAETargetMode);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setVideoDynamicFrameRate(MINT32 i4SensorDev, MBOOL bVdoDynamicFps)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setVideoDynamicFrameRate(bVdoDynamicFps);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setVideoDynamicFrameRate(bVdoDynamicFps);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setVideoDynamicFrameRate(bVdoDynamicFps);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::getAESensorActiveCycle(MINT32 i4SensorDev, MINT32* i4ActiveCycle)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAESensorActiveCycle(i4ActiveCycle);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAESensorActiveCycle(i4ActiveCycle);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAESensorActiveCycle(i4ActiveCycle);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::IsNeedUpdateSensor(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsNeedUpdateSensor();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsNeedUpdateSensor();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsNeedUpdateSensor();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::updateSensorbyI2C(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).updateSensorbyI2C();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).updateSensorbyI2C();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setSensorDirectly(MINT32 i4SensorDev, CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setSensorDirectly(a_rCaptureInfo, bDirectlySetting);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEEnable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEEnable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEEnable();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEDisable(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEDisable();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEDisable();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAeMgr::CCTOPAEGetEnableInfo(MINT32 i4SensorDev, MINT32 *a_pEnableAE, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetEnableInfo(a_pEnableAE, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetAEScene(MINT32 i4SensorDev, MINT32 a_AEScene)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSceneMode(a_AEScene);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSceneMode(a_AEScene);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetAEScene(MINT32 i4SensorDev, MINT32 *a_pAEScene, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetAEScene(a_pAEScene, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetMeteringMode(MINT32 i4SensorDev, MINT32 a_AEMeteringMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetMeteringMode(a_AEMeteringMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetMeteringMode(a_AEMeteringMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyExpParam(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyExpParam(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetFlickerMode(MINT32 i4SensorDev, MINT32 a_AEFlickerMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetFlickerMode(a_AEFlickerMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetFlickerMode(a_AEFlickerMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetExpParam(MINT32 i4SensorDev, MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetExpParam(a_pAEExpParamIn, a_pAEExpParamOut, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetFlickerMode(MINT32 i4SensorDev, MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetFlickerMode(a_pAEFlickerMode, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetMeteringMode(MINT32 i4SensorDev, MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetMeteringMode(a_pAEMEteringMode, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyNVRAMParam(a_pAENVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyNVRAMParam(a_pAENVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyNVRAMParam(a_pAENVRAM);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetNVRAMParam(MINT32 i4SensorDev, MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetNVRAMParam(a_pAENVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESaveNVRAMParam(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESaveNVRAMParam();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESaveNVRAMParam();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetCurrentEV(MINT32 i4SensorDev, MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetCurrentEV(a_pAECurrentEV, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAELockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAELockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAELockExpSetting();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEUnLockExpSetting(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEUnLockExpSetting();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEUnLockExpSetting();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetIspOB(MINT32 i4SensorDev, MUINT32 *a_pIspOB, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetIspOB(a_pIspOB, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspOB(MINT32 i4SensorDev, MUINT32 a_IspOB)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetIspOB(a_IspOB);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetIspOB(a_IspOB);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetIspRAWGain(MINT32 i4SensorDev, MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetIspRAWGain(a_pIspRawGain, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetIspRAWGain(MINT32 i4SensorDev, MUINT32 a_IspRAWGain)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetIspRAWGain(a_IspRAWGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetIspRAWGain(a_IspRAWGain);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpTime(MINT32 i4SensorDev, MUINT32 a_ExpTime)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorExpTime(a_ExpTime);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorExpTime(a_ExpTime);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorExpLine(MINT32 i4SensorDev, MUINT32 a_ExpLine)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorExpLine(a_ExpLine);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorExpLine(a_ExpLine);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetSensorGain(MINT32 i4SensorDev, MUINT32 a_SensorGain)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetSensorGain(a_SensorGain);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetSensorGain(a_SensorGain);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESetCaptureMode(MINT32 i4SensorDev, MUINT32 a_CaptureMode)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESetCaptureMode(a_CaptureMode);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESetCaptureMode(a_CaptureMode);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOSetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOSetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOSetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOGetCaptureParams(MINT32 i4SensorDev, MVOID *a_pAEExpParam)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOGetCaptureParams(a_pAEExpParam);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOGetCaptureParams(a_pAEExpParam);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetFlareOffset(MINT32 i4SensorDev, MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetFlareOffset(a_FlareThres, a_pAEFlareOffset, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPSetAETargetValue(MINT32 i4SensorDev, MUINT32 u4AETargetValue)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPSetAETargetValue(u4AETargetValue);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPSetAETargetValue(u4AETargetValue);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::modifyAEPlineTableLimitation(MINT32 i4SensorDev, MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).modifyAEPlineTableLimitation(bEnable, bEquivalent, u4IncreaseISO_x100, u4IncreaseShutter_x100);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAESGG1Gain(MINT32 i4SensorDev, MUINT32 *pSGG1Gain)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAESGG1Gain(pSGG1Gain);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAESGG1Gain(pSGG1Gain);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAESGG1Gain(pSGG1Gain);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::enableAEOneShotControl(MINT32 i4SensorDev, MBOOL bAEControl)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).enableAEOneShotControl(bAEControl);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).enableAEOneShotControl(bAEControl);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::getAEPlineTable(MINT32 i4SensorDev, eAETableID eTableID, strAETable &a_AEPlineTable)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).getAEPlineTable(eTableID, a_AEPlineTable);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).getAEPlineTable(eTableID, a_AEPlineTable);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).getAEPlineTable(eTableID, a_AEPlineTable);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEApplyPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEApplyPlineNVRAM(a_pAEPlineNVRAM);

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAEGetPlineNVRAM(MINT32 i4SensorDev, MVOID *a_pAEPlineNVRAM, MUINT32 *a_pOutLen)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAEGetPlineNVRAM(a_pAEPlineNVRAM, a_pOutLen);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::CCTOPAESavePlineNVRAM(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).CCTOPAESavePlineNVRAM();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).CCTOPAESavePlineNVRAM();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::get3ACaptureDelayFrame(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).get3ACaptureDelayFrame();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).get3ACaptureDelayFrame();

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::IsMultiCapture(MINT32 i4SensorDev, MBOOL bMultiCap)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsMultiCapture(bMultiCap);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsMultiCapture(bMultiCap);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsMultiCapture(bMultiCap);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::IsAEContinueShot(MINT32 i4SensorDev, MBOOL bCShot)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).IsAEContinueShot(bCShot);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).IsAEContinueShot(bCShot);

    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAeMgr::updateCaptureShutterValue(MINT32 i4SensorDev)
{
    MINT32 ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = S_AE_OK;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AeMgr::getInstance(ESensorDev_Main).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AeMgr::getInstance(ESensorDev_Sub).updateCaptureShutterValue();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AeMgr::getInstance(ESensorDev_MainSecond).updateCaptureShutterValue();

    return (ret_main | ret_sub | ret_main2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::isLVChangeTooMuch(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).isLVChangeTooMuch();
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).isLVChangeTooMuch();
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).isLVChangeTooMuch();

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAeMgr::setEISon(MINT32 i4SensorDev, MBOOL bISEISon)
{
    if (i4SensorDev & ESensorDev_Main)
        return AeMgr::getInstance(ESensorDev_Main).setEISon(bISEISon);
    if (i4SensorDev & ESensorDev_Sub)
        return AeMgr::getInstance(ESensorDev_Sub).setEISon(bISEISon);
    if (i4SensorDev & ESensorDev_MainSecond)
        return AeMgr::getInstance(ESensorDev_MainSecond).setEISon(bISEISon);

    return MFALSE;
}

