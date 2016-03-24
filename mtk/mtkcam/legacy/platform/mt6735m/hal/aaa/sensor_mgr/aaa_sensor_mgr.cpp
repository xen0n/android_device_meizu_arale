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
#define LOG_TAG "aaa_sensor_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <string.h>
//#include <cutils/pmem.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_param.h>
//#include <isp_drv.h>
#include <isp_tuning.h>
//#include "buf_mgr.h"

#include <aaa_sensor_mgr.h>

using namespace NS3Av3;
using namespace android;
using namespace NSCam;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr&
AAASensorMgr::
getInstance()
{
    static  AAASensorMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
AAASensorMgr()
    : m_pHalSensorObj(MNULL)
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_bFlickerState(MFALSE)
    , m_u4SensorFrameRate(0xFFFF)
{
    memset(&m_pSensorModeDelay, 0, 11*sizeof(MINT32));

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
~AAASensorMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
init(MINT32 const i4SensorIdx)
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[%s()] - E. m_Users: %d i4SensorIdx:%d\n", __FUNCTION__, m_Users, i4SensorIdx);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0) {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return ret;
    }

    // Sensor hal init
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    m_pHalSensorObj = pIHalSensorList->createSensor("aaa_sensor_mgr", i4SensorIdx);

    if(m_pHalSensorObj == NULL) {
        MY_ERR("[AAA Sensor Mgr] Can not create SensorHal obj\n");
    }

    android_atomic_inc(&m_Users);

    m_u4SensorFrameRate = 0xFFFF;

    MY_LOG("[%s()] - m_u4SensorFrameRate: %d \n", __FUNCTION__, m_u4SensorFrameRate);

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("debug.aaa_sensor_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    m_bFlickerState = 0;

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
uninit()
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0) {
        return ret;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) { // There is no more User after decrease one User
        if(m_pHalSensorObj) {
            m_pHalSensorObj->destroyInstance("aaa_sensor_mgr");
            m_pHalSensorObj = NULL;
        }
    } else {  // There are still some users.
        MY_LOG("Still %d users \n", m_Users);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorVCinfo(MINT32 i4SensorDev, SENSOR_VC_INFO_T *a_rSensorVCInfo)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MINT32 i4SensorSenraio = 0; //SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    SensorVCInfo rSensorVCInfo;


    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

//    MY_LOG_IF(m_bDebugEnable,"[getSensorVCinfo]  \n");
    MY_LOG("[getSensorVCinfo]  \n");

    // Get default frame rate
    if(i4SensorDev == ESensorDev_Main) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&rSensorVCInfo, (MUINTPTR)&i4SensorSenraio, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    a_rSensorVCInfo->u2VCModeSelect = rSensorVCInfo.ModeSelect;
    a_rSensorVCInfo->u2VCShutterRatio = rSensorVCInfo.EXPO_Ratio;
    a_rSensorVCInfo->u2VCOBvalue = rSensorVCInfo.ODValue;

    switch(rSensorVCInfo.RG_STATSMODE)
    {
        case 1:   // 8x8
            a_rSensorVCInfo->u2VCStatWidth = 8;
            a_rSensorVCInfo->u2VCStatHeight = 8;
            break;
        case 2:   // 4x4
            a_rSensorVCInfo->u2VCStatWidth = 4;
            a_rSensorVCInfo->u2VCStatHeight = 4;
            break;
        case 3:   // 1x1
            a_rSensorVCInfo->u2VCStatWidth = 1;
            a_rSensorVCInfo->u2VCStatHeight = 1;
            break;
        default:
        case 0:   // 16x16
            a_rSensorVCInfo->u2VCStatWidth = 16;
            a_rSensorVCInfo->u2VCStatHeight = 16;
            break;
    }

    MY_LOG("[getSensorVCinfo] Mode:%d ShutterRatio:%d OBvalue:%d Stat width:%d height:%d %d Error:%d \n", a_rSensorVCInfo->u2VCModeSelect, a_rSensorVCInfo->u2VCShutterRatio,
        a_rSensorVCInfo->u2VCOBvalue, a_rSensorVCInfo->u2VCStatWidth, a_rSensorVCInfo->u2VCStatHeight, rSensorVCInfo.RG_STATSMODE, err);

    return S_AAA_SENSOR_MGR_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorSyncinfo(MINT32 i4SensorDev, MINT32 *i4SutterDelay, MINT32 *i4SensorGainDelay, MINT32 *i4IspGainDelay, MINT32 *i4SensorModeDelay)
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();

    MY_LOG("[setSensorDev] Sensor m_eSensorDevId:%d\n", i4SensorDev);

    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    // Get sensor delay frame for sync
    *i4SutterDelay = rSensorStaticInfo.aeShutDelayFrame;
    *i4SensorGainDelay = rSensorStaticInfo.aeSensorGainDelayFrame;
    *i4IspGainDelay = rSensorStaticInfo.aeISPGainDelayFrame;
    *i4SensorModeDelay = rSensorStaticInfo.captureDelayFrame;
   m_pSensorModeDelay[0] = rSensorStaticInfo.previewDelayFrame;
   m_pSensorModeDelay[1] = rSensorStaticInfo.captureDelayFrame;
   m_pSensorModeDelay[2] = rSensorStaticInfo.videoDelayFrame;
   m_pSensorModeDelay[3] = rSensorStaticInfo.video1DelayFrame;
   m_pSensorModeDelay[4] = rSensorStaticInfo.video2DelayFrame;
   m_pSensorModeDelay[5] = rSensorStaticInfo.Custom1DelayFrame;
   m_pSensorModeDelay[6] = rSensorStaticInfo.Custom2DelayFrame;
   m_pSensorModeDelay[7] = rSensorStaticInfo.Custom3DelayFrame;
   m_pSensorModeDelay[8] = rSensorStaticInfo.Custom4DelayFrame;
   m_pSensorModeDelay[9] = rSensorStaticInfo.Custom5DelayFrame;
   m_pSensorModeDelay[10] = rSensorStaticInfo.previewDelayFrame;
    MY_LOG("[setSensorDev] Sensor delay frame Shutter:%d Gain:%d Isp:%d Capture delay:%d\n", *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay, *i4SensorModeDelay);
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorWidthHeight(MINT32 i4SensorDev, SENSOR_RES_INFO_T* a_rSensorResolution)
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    a_rSensorResolution->u2SensorPreviewWidth = rSensorStaticInfo.previewWidth;
    a_rSensorResolution->u2SensorPreviewHeight = rSensorStaticInfo.previewHeight;
    a_rSensorResolution->u2SensorVideoWidth = rSensorStaticInfo.videoWidth;
    a_rSensorResolution->u2SensorVideoHeight = rSensorStaticInfo.videoHeight;
    a_rSensorResolution->u2SensorFullWidth = rSensorStaticInfo.captureWidth;
    a_rSensorResolution->u2SensorFullHeight = rSensorStaticInfo.captureHeight;
    a_rSensorResolution->u2SensorVideo1Width = rSensorStaticInfo.video1Width;
    a_rSensorResolution->u2SensorVideo1Height = rSensorStaticInfo.video1Height;
    a_rSensorResolution->u2SensorVideo2Width = rSensorStaticInfo.video2Width;
    a_rSensorResolution->u2SensorVideo2Height = rSensorStaticInfo.video2Height;

    MY_LOG("[getSensorWidthHeight] Sensor id:%d Prv:%d %d Video:%d %d Cap:%d %d Video1:%d Video2:%d\n", i4SensorDev, a_rSensorResolution->u2SensorPreviewWidth,
                  a_rSensorResolution->u2SensorPreviewHeight, a_rSensorResolution->u2SensorVideoWidth, a_rSensorResolution->u2SensorVideoHeight,
                  a_rSensorResolution->u2SensorFullWidth, a_rSensorResolution->u2SensorFullHeight, a_rSensorResolution->u2SensorVideo1Width, a_rSensorResolution->u2SensorVideo1Height,
                  a_rSensorResolution->u2SensorVideo2Width, a_rSensorResolution->u2SensorVideo2Height);
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 rSensorMaxFrmRate[NSIspTuning::ESensorMode_NUM])
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    if(i4SensorDev == ESensorDev_Main) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_Sub) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Preview] = rSensorStaticInfo.previewFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Capture] = rSensorStaticInfo.captureFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Video] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_SlimVideo1] = rSensorStaticInfo.video1FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_SlimVideo2] = rSensorStaticInfo.video2FrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom1] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom2] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom3] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom4] = rSensorStaticInfo.videoFrameRate;
    rSensorMaxFrmRate[NSIspTuning::ESensorMode_Custom5] = rSensorStaticInfo.videoFrameRate;

    for (MUINT32 i = 0; i < NSIspTuning::ESensorMode_NUM; i++)
    {
        MY_LOG("[%s] SensorDev(%d) Mode(%d) MaxFPS(%d)", __FUNCTION__,
            i4SensorDev, i, rSensorMaxFrmRate[i]);
    }
    return S_AAA_SENSOR_MGR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpTime(MINT32 i4SensorDev, MUINT32 a_u4ExpTime)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorExpTime] i4SensorDev:%d a_u4ExpTime = %d \n", i4SensorDev, a_u4ExpTime);

    if (a_u4ExpTime == 0) {
        MY_ERR("setSensorExpTime() error: exposure time = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MUINTPTR)&a_u4ExpTime, 0, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }


    if(ret) {
        MY_ERR("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4ExpTime);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorGain(MINT32 i4SensorDev, MUINT32 a_u4SensorGain)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorGain] i4SensorDev:%d a_u4SensorGain = %d \n", i4SensorDev, a_u4SensorGain);

    if (a_u4SensorGain < 1024) {
        MY_ERR("setSensorGain() error: sensor gain:%d \n", a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_GAIN, (MUINTPTR)&a_u4SensorGain, 0, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        MY_ERR("Err SENSOR_CMD_SET_SENSOR_GAIN, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4SensorGain);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorFrameRate(MINT32 i4SensorDev, MUINT32 a_u4SensorFrameRate)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    MUINT32 u4SensorFrameRate;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorFrameRate] i4SensorDev:%d a_u4SensorFrameRate = %d \n", i4SensorDev, a_u4SensorFrameRate);

    // Set sensor frame rate
    u4SensorFrameRate = a_u4SensorFrameRate/10;    // 10 base frame rate from AE

    // Set frame rate
    if(u4SensorFrameRate == m_u4SensorFrameRate){
        MY_LOG_IF(m_bDebugEnable,"[setSensorFrameRate] The same main frame rate m_u4SensorFrameRate = %d \n", m_u4SensorFrameRate);
    } else {
        if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
        } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&u4SensorFrameRate, 0, 0);
        } else {
            MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }

        m_u4SensorFrameRate = u4SensorFrameRate;
    }

    if(ret) {
        MY_ERR("Err SENSOR_CMD_SET_VIDEO_FRAME_RATE, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, u4SensorFrameRate);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpLine(MINT32 i4SensorDev, MUINT32 a_u4ExpLine)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorExpTime] i4SensorDev:%d a_u4ExpLine = %d \n", i4SensorDev, a_u4ExpLine);

    if (a_u4ExpLine == 0) {
        MY_ERR("setSensorExpTime() error: exposure line = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure line
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MUINTPTR)&a_u4ExpLine, 0, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        MY_ERR("Err SENSOR_CMD_SET_SENSOR_EXP_LINE, Sensor dev:%d a_u4ExpTime:%d\n", i4SensorDev, a_u4ExpLine);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setFlickerFrameRateActive(MINT32 i4SensorDev, MBOOL a_bFlickerFPSAvtive)
{
    MY_LOG("setFlickerFrameRateActive en=%d",a_bFlickerFPSAvtive);
    MINT32 ret = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4FlickerInfo;

    if  (!m_pHalSensorObj) {
       // MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setFlickerFrameRateActive] a_bFlickerFPSAvtive = %d \n", a_bFlickerFPSAvtive);

    u4FlickerInfo = (MUINT32)a_bFlickerFPSAvtive;

    // Set frame rate
    if(a_bFlickerFPSAvtive == m_bFlickerState){
        MY_LOG_IF(m_bDebugEnable,"[setSensorFrameRate] The same main flicker active = %d \n", m_bFlickerState);
    } else {
        if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MUINTPTR)&u4FlickerInfo, 0, 0);
        } else {
            MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }

        m_bFlickerState = a_bFlickerFPSAvtive;
    }

    if(ret) {
        MY_ERR("Err SENSOR_CMD_SET_FLICKER_FRAME_RATE, Sensor dev:%d flicker status:%d\n", i4SensorDev, a_bFlickerFPSAvtive);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setPreviewParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setPreviewParams] Id:%d Shutter:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);

    m_u4SensorFrameRate = 0xFFFF;

    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret_shutter | ret_gain) {
        MY_ERR("Err setPreviewParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setCaptureParams(MINT32 i4SensorDev, MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setCaptureParams] Id:%d Shutter:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_SUB, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpTime(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpTime);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret_shutter | ret_gain) {
        MY_ERR("Err setCaptureParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpTime, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setPreviewLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setPreviewLineBaseParams] Id:%d Exp. Line:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);

    m_u4SensorFrameRate = 0xFFFF;

    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }


    if(ret_shutter | ret_gain) {
        MY_ERR("Err setPreviewLineBaseParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setCaptureLineBaseParams(MINT32 i4SensorDev, MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain)
{
    MINT32 ret_shutter, ret_gain;

    ret_shutter =  ret_gain = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setCaptureLinaeBaseParams] Id:%d Exp. Line:%d Sensor Gain:%d \n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);

    if(i4SensorDev == ESensorDev_Main) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_SUB, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_SUB, a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret_shutter = setSensorExpLine(NSCam::SENSOR_DEV_MAIN_2, a_u4ExpLine);
        ret_gain = setSensorGain(NSCam::SENSOR_DEV_MAIN_2, a_u4SensorGain);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }



    if(ret_shutter | ret_gain) {
        MY_ERR("Err setCaptureLinaeBaseParams, Sensor dev:%d shutter:%d gain:%d\n", i4SensorDev, a_u4ExpLine, a_u4SensorGain);
    }

    return (ret_shutter | ret_gain);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AAASensorMgr::setPreviewMaxFrameRate(MINT32 i4SensorDev, MUINT32 frameRate, MUINT32 u4SensorMode)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setPreviewMaxFrameRate] frameRate = %d \n", frameRate);

    if (frameRate == 0) {
        MY_ERR("setPreviewMaxFrameRate() error: frameRate = 0\n");
        return MHAL_INVALID_PARA;
    }

    if(frameRate == m_u4SensorFrameRate) {
        MY_LOG_IF(m_bDebugEnable,"[setPreviewMaxFrameRate] The same main frame rate m_u4SensorFrameRate = %d \n", m_u4SensorFrameRate);
    } else {
    // Set max frame rate
        MY_LOG("[setPreviewMaxFrameRate] frameRate = %d \n", frameRate);
        if(i4SensorDev == ESensorDev_Main) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
        } else if(i4SensorDev == ESensorDev_Sub) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
        } else if(i4SensorDev == ESensorDev_MainSecond) {
            ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frameRate, 0);
        } else {
            MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
            return E_AAA_SENSOR_NULL;
        }
        m_u4SensorFrameRate = frameRate;
    }

    if(ret) {
        MY_ERR("Err SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO, Sensor dev:%d frameRate:%d\n", i4SensorDev, frameRate);
    }

    return (ret);
}

MUINT32 AAASensorMgr::getPreviewDefaultFrameRate(MINT32 i4SensorDev, MUINT32 u4SensorMode)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 frmRate;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[getPreviewMaxFrameRate]  \n");

    // Get default frame rate
    if(i4SensorDev == ESensorDev_Main) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        err = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&u4SensorMode, (MUINTPTR)&frmRate, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    return frmRate;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
set2ShutterParams(MINT32 i4SensorDev, MUINT32 a_u4LEExpTime, MUINT32 a_u4SEExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable, "[setiVHDRParams] Id:%d Shutter:%d %d Sensor Gain:%d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);

    if ((a_u4LEExpTime == 0) || (a_u4SEExpTime == 0) || (a_u4SensorGain == 0)){
        MY_ERR("setiVHDRParams() error Id:%d Shutter:%d %d Sensor Gain:%d \n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN, (MUINTPTR)&a_u4LEExpTime, (MUINTPTR)&a_u4SEExpTime, (MUINTPTR)&a_u4SensorGain);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        MY_ERR("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d Shutter:%d %d Sensor Gain:%d\n", i4SensorDev, a_u4LEExpTime, a_u4SEExpTime, a_u4SensorGain);
    }

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorAWBGain(MINT32 i4SensorDev, strSensorAWBGain *strSensorAWBInfo)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG("[setSensorAWBGain] Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B, strSensorAWBInfo->u4GB);

    MY_LOG_IF(m_bDebugEnable, "[setSensorAWBGain] Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B, strSensorAWBInfo->u4GB);

    if ((strSensorAWBInfo->u4GB == 0) || (strSensorAWBInfo->u4R == 0) || (strSensorAWBInfo->u4B == 0) || (strSensorAWBInfo->u4GB == 0)){
        MY_ERR("setSensorAWBGain() error Id:%d GR:%d R:%d B:%d GB:%d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
        strSensorAWBInfo->u4GB);
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN, (MUINTPTR)strSensorAWBInfo, 0, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        MY_ERR("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d GR:%d %d R:%d %d B:%d %d GB:%d %d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
        strSensorAWBInfo->u4GB);
    }

    MY_LOG("[setSensorAWBGain1] Id:%d GR:%d %d R:%d %d B:%d %d GB:%d %d\n", i4SensorDev, strSensorAWBInfo->u4GR,
        strSensorAWBInfo->u4R, strSensorAWBInfo->u4B,
        strSensorAWBInfo->u4GB);

    return (ret);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getSensorModeDelay(MINT32 i4SensorMode,MINT32 *i4SensorModeDelay)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    *i4SensorModeDelay = m_pSensorModeDelay[i4SensorMode];

    MY_LOG("[getSensorModeDelay] SensorMode:%d SensorModeDelay:%d \n", i4SensorMode, *i4SensorModeDelay);

    return (ret);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
getRollingShutter(MINT32 i4SensorDev, MUINT32& tline, MUINT32& vsize) const
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;

    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    // Set exposure time
    if (i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER, (MUINTPTR)&tline, (MUINTPTR)&vsize, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable, "[%s] sensor(%d), tline(%d), vsize(%d)", __FUNCTION__, i4SensorDev, tline, vsize);

    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::setSensorOBLock(MINT32 i4SensorDev, MBOOL bLockSensorOB)
{
    MINT32 ret = S_AAA_SENSOR_MGR_OK;


    if  (!m_pHalSensorObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable, "[setSensorOBLock] Id:%d bLockSensorOB:%d\n", i4SensorDev, bLockSensorOB);

    // Set exposure time
    if(i4SensorDev == ESensorDev_Main) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else if(i4SensorDev == ESensorDev_Sub) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_SUB, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else if(i4SensorDev == ESensorDev_MainSecond) {
        ret = m_pHalSensorObj->sendCommand(NSCam::SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_OB_LOCK, (MUINTPTR)&bLockSensorOB, 0, 0);
    } else {
        MY_ERR("[%s()] Sensor type error:%d", __FUNCTION__, i4SensorDev);
        return E_AAA_SENSOR_NULL;
    }

    if(ret) {
        MY_ERR("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d bLockSensorOB:%d\n", i4SensorDev, bLockSensorOB);
    }

    return (ret);

}
